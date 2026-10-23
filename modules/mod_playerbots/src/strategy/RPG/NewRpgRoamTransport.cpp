/*
 * #218 / #528 (BEYOND AC, MoP only): roaming bots take boats, zeppelins, elevators, trams and flight paths on
 * their own. Design and AC references: NewRpgRoamTransport.h.
 *
 * A "trip" is a TravelNodeMap::getFullPath route (Travel/TravelNode.cpp) walked point by point:
 *   NODE_PATH / NODE_NODE / NODE_PREPATH : walk (MovementAction::MoveTo, as AC's MoveFarTo does)
 *   NODE_TRANSPORT pair (board, arrival) : wait at the dock, walk onto the deck, ride, walk off at the arrival dock
 *   NODE_FLIGHTPATH pair                 : walk to the flight master and fly (Player::ActivateTaxiPathTo; the taxi
 *                                          cheat of #528 makes every node known, AC PlayerbotAI.cpp:240)
 *   NODE_PORTAL / NODE_TELEPORT pair     : step to the portal, then teleport to its exit (what the old #86 graph
 *                                          walker did; AC's commented route code teleports too)
 * Boarding reuses the S9 (#206/#209) rules: an MO transport (boat/zeppelin) is boarded only while it is docked
 * (keyframe stop window, FollowActions.cpp IsDockedNow), a static one (elevator/tram) only while its floor is level
 * with the bot (the IsUnderStaticTransportDeck 4 yd ray), and the attach itself stops the bot first and then calls
 * Transport::AddPassenger(bot, true) (PlayerbotAI.cpp SeatBotOnMasterTransport, #223 P2). A masterless rider that
 * walks off is removed here as soon as the deck is no longer under it: PlayerbotAI's ambient check would otherwise
 * re-seat it from its own offset (#316 SnapToOwnDeckOffset) and the platform would carry it away (#586).
 *
 * State lives in a .cpp-local table keyed by bot GUID (no NewRpgInfo.h change), the same pattern as
 * FollowActions.cpp MasterStaticTransportMemo. A bot is only ever updated by one map thread, so after the locked
 * lookup its own entry is used without the lock; std::unordered_map keeps element addresses stable on rehash.
 */

#include "NewRpgRoamTransport.h"

#include "NewRpgBaseAction.h"

#include "Config.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GameObject.h"
#include "GameObjectModel.h"
#include "Map.h"
#include "MapManager.h"
#include "MotionMaster.h"
#include "MoveSpline.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PathGenerator.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Random.h"
#include "ReputationMgr.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Transport.h"
#include "TravelMgr.h"
#include "TransportMgr.h"
#include "TravelNode.h"
#include "TypeContainerVisitor.h"
#include "WorldPosition.h"

#include <algorithm>
#include <atomic>
#include <cmath>
#include <memory>
#include <mutex>
#include <set>
#include <shared_mutex>
#include <unordered_map>
#include <vector>

namespace
{
    // ---- config (read once, like PlayerbotAI.cpp HasConfiguredTaxiCheat) ------------------------------------------
    bool CfgEnabled()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.RoamTransports", true);
        return v;
    }

    uint32 CfgCampChance()
    {
        static uint32 const v = std::min<uint32>(100, uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RoamTransportCampChance", 10))));
        return v;
    }

    // ---- budgets -----------------------------------------------------------------------------------------------
    constexpr uint32 kRouteBuildGapMs = 250;            // at most ~4 A* route builds per second, all bots together
    constexpr uint32 kRetryAfterFailMs = 10 * MINUTE * IN_MILLISECONDS;
    constexpr uint32 kWalkStallMs = 120 * IN_MILLISECONDS;
    constexpr uint32 kDockWaitMs = 12 * MINUTE * IN_MILLISECONDS;   // longest MoP boat/zeppelin round trip is ~6 min
    constexpr uint32 kBoardMs = 45 * IN_MILLISECONDS;
    constexpr uint32 kRideMs = 15 * MINUTE * IN_MILLISECONDS;
    constexpr uint32 kLeaveMs = 30 * IN_MILLISECONDS;
    constexpr uint32 kTaxiStartMs = 60 * IN_MILLISECONDS;
    constexpr float kArriveDist = 10.0f;                 // AC's GO_GRIND / GO_CAMP arrival distance
    constexpr float kPointReached = 4.0f;
    constexpr float kWalkLookAhead = 60.0f;
    constexpr float kDockRadius = 35.0f;
    constexpr float kDockHeight = 12.0f;                 // a landing more than this above / below is another level
    constexpr float kDeckFootprint = 8.0f;               // 2D reach of a deck from its transport origin (PickLeavePoint)
    // #848 (w68/w69: 4 "dock-wait-timeout" at the Borean elevator 152614, every wait "stop-too-short"): a static
    // platform's step-on / step-off is timed to the deck EDGE, where the attach (Board: DeckUnder) and the detach
    // (Leave: no deck under the bot) happen, plus one fast-react tick of margin. The old test timed the walk to the
    // deck point 3 yd inside the edge plus 1.5 s, which no stop under ~2.3 s can ever fit: TransportAnimation 152614
    // pauses 1333 ms at each landing (frames 0-1333 at z 0, 3333-4667 at z -12.83, period 6667). Boats and zeppelins
    // (60 s keyframe stops) keep the old margins.
    constexpr uint32 kStaticStepMarginMs = 500;
    constexpr float kStandbyFromEdge = 1.5f;             // the DockWait standby point, short of a static deck's edge
    // #890 (w88 Z3 Eraimo, Orgrimmar elevator 206610): a bot that is waiting goes back to walking only this much
    // farther out than kDockRadius (it entered DockWait at 34 yd on the ramp below and logged "wait" 15+ times).
    constexpr float kDockRadiusSlack = 10.0f;

    enum class TripPhase : uint8
    {
        Walk,
        DockWait,
        Board,
        Ride,
        Leave,
        Taxi
    };

    char const* PhaseName(TripPhase p)
    {
        switch (p)
        {
            case TripPhase::Walk: return "walk";
            case TripPhase::DockWait: return "wait";
            case TripPhase::Board: return "board";
            case TripPhase::Ride: return "ride";
            case TripPhase::Leave: return "leave";
            case TripPhase::Taxi: return "taxi";
        }
        return "?";
    }

    struct RoamTrip
    {
        bool active = false;
        WorldPosition dest;
        std::vector<PathNodePoint> pts;
        size_t idx = 0;                 // the point the bot is working toward
        TripPhase phase = TripPhase::Walk;
        uint32 startMs = 0;
        uint32 phaseMs = 0;
        uint32 progressMs = 0;
        float bestDist = 0.0f;
        uint32 transportEntry = 0;
        ObjectGuid transportGuid;
        float boardX = 0.0f, boardY = 0.0f, boardZ = 0.0f;
        float edgeX = 0.0f, edgeY = 0.0f;  // #848: where the deck starts on the bot's way to (boardX, boardY)
        bool boardMoveIssued = false;
        float leaveX = 0.0f, leaveY = 0.0f, leaveZ = 0.0f;
        uint32 legs = 0;                // transport / flight / portal legs completed
        uint32 failAtMs = 0;            // last abandon or empty route (retry gate)
        uint32 lookupMs = 0;            // last transport object-store lookup (DockWait)
        float dockX = 0.0f, dockY = 0.0f, dockZ = 0.0f;  // where the bot stood when it stepped toward the deck
        bool grind = false;             // the status that drives the trip is RPG_GO_GRIND (else RPG_GO_CAMP)
        uint32 hops = 0;                // re-plans after a route that ended on an intermediate map (same dest)
        uint32 routeStartMap = 0;       // the map the current route was built on
        uint32 prevMap = 0;             // the map the previous hop left from (a re-plan may not go back there)
        uint32 portalMs = 0;            // when a portal leg teleported the bot (ResumeAfterReset window)
        size_t walkTarget = 0;          // the last look-ahead point a walk MoveTo was issued for (wave 34 catch-up)
        bool flew = false;              // a Taxi-phase tick saw the bot in flight (wave 36 taxi-not-landed)
        uint32 driverT = 0;             // NewRpgInfo::startT of the status that last drove the trip (wave 41)
        int32 driverStatus = -1;        // ... and that status (NewRpgStatus)
        bool dockApproached = false;    // wave 51: the DockWait standby walk to a static platform's edge was issued
        uint8 waitWhy = 0;              // wave 51: DockWait reasons already logged ("wait-why"), bit per reason
        size_t stuckTpIdx = size_t(-1); // #847 (w87): the route index the trip already stuck-teleported at (once each)
        // #890: the dock wait of one leg keeps its start across re-entries (Walk <-> DockWait), so dock-wait-timeout fires.
        uint32 dockWaitStartMs = 0;     // first DockWait entry of this leg (0 = none yet)
        size_t dockWaitIdx = size_t(-1);  // ... the leg (route index) it belongs to
        uint32 dockWaitTripMs = 0;      // ... and the trip (startMs) it belongs to
        uint32 dockWaitAgain = 0;       // re-entries of that wait ("wait-again")
        bool levelApproached = false;   // #890: the walk up to a static platform's landing was issued in this wait
    };

    constexpr uint32 kMaxHops = 3;

    std::mutex g_tripMutex;
    std::unordered_map<ObjectGuid, RoamTrip> g_trips;
    std::atomic<uint32> g_lastRouteBuildMs{0};
    std::atomic<int32> g_activeTrips{0};

    // Fast path for MoveFarTo: with no trip running anywhere, a call that may not start one skips the locked lookup.
    void SetActive(RoamTrip& trip, bool on)
    {
        if (trip.active == on)
            return;
        trip.active = on;
        g_activeTrips.fetch_add(on ? 1 : -1, std::memory_order_relaxed);
    }

    RoamTrip& TripFor(Player* bot)
    {
        std::lock_guard<std::mutex> lock(g_tripMutex);
        return g_trips[bot->GetGUID()];
    }

    bool TakeRouteBuildSlot()
    {
        uint32 const now = getMSTime();
        uint32 last = g_lastRouteBuildMs.load();
        if (last && getMSTimeDiff(last, now) < kRouteBuildGapMs)
            return false;
        return g_lastRouteBuildMs.compare_exchange_strong(last, now ? now : 1);
    }

    bool IsWalkPoint(PathNodePoint const& p)
    {
        return p.type == NODE_PATH || p.type == NODE_NODE || p.type == NODE_PREPATH;
    }

    // A leg is a pair of consecutive points of the same special type and entry (TravelNodeMap::getFullPath adds the
    // departure and the arrival point of every transport / flight / portal link, TravelNode.cpp:1016-1024).
    bool IsLegStart(std::vector<PathNodePoint> const& pts, size_t i)
    {
        if (i + 1 >= pts.size())
            return false;
        PathNodePoint const& a = pts[i];
        PathNodePoint const& b = pts[i + 1];
        if (a.type != b.type || a.entry != b.entry)
            return false;
        return a.type == NODE_TRANSPORT || a.type == NODE_FLIGHTPATH || a.type == NODE_PORTAL || a.type == NODE_TELEPORT;
    }

    // #848: the route index of the trip's next leg (the first leg start at or after `from`) when it is a transport leg
    // of the transport the bot is a passenger of, else pts.size(). Only the NEXT leg counts: a later leg on the same
    // ship is never skipped to.
    size_t AboardLegIdx(std::vector<PathNodePoint> const& pts, size_t from, Player* bot)
    {
        Transport* on = bot->GetTransport();
        if (!on)
            return pts.size();
        for (size_t j = from; j + 1 < pts.size(); ++j)
        {
            if (!IsLegStart(pts, j))
                continue;
            return (pts[j].type == NODE_TRANSPORT && pts[j].entry == on->GetEntry()) ? j : pts.size();
        }
        return pts.size();
    }

    float Dist2d(Player* bot, WorldPosition const& p)
    {
        return bot->GetExactDist2d(p.GetPositionX(), p.GetPositionY());
    }

    float Dist3d(Player* bot, WorldPosition const& p)
    {
        return bot->GetExactDist(p.GetPositionX(), p.GetPositionY(), p.GetPositionZ());
    }

    // Wave 34 (#218, leg E up): a route point counts as reached only on the bot's own level. The 2D test alone took
    // the Mesa Elevator's top-landing node (-1286.2,189.7,130.1) as reached from the shaft roof at z 159.0 straight
    // above it ("arrive route-end (-1286.2,189.2,159.0)"). Graph path z is AC-era data, so allow a few yards of slack.
    constexpr float kPointReachedZ = 6.0f;
    bool PointReached(Player* bot, WorldPosition const& p)
    {
        return Dist2d(bot, p) < kPointReached && std::fabs(bot->GetPositionZ() - p.GetPositionZ()) < kPointReachedZ;
    }

    // Wave 51 (#218): a portal node is an area trigger's centre (AC graph data). The Deeprun Tram entrance node 1450
    // (-8346.5,514.0,92.6) is 8 yd above the floor: Emeronato (w50) stood 3.9 yd from it in 2D at z 84.4 for 120 s
    // and gave up (walk-stall point=165/168, the point before the portal leg). The core never fires an area trigger
    // for a bot anyway (the client sends CMSG_AREATRIGGER); the trip teleports itself. Below it within kDockHeight is
    // at the portal.
    bool PortalPointReached(Player* bot, WorldPosition const& p)
    {
        return Dist2d(bot, p) < kPointReached && std::fabs(bot->GetPositionZ() - p.GetPositionZ()) <= kDockHeight;
    }

    // The live transport GO of this entry on the bot's map closest to (x,y,z). Static (elevator/tram) transports are
    // never in a grid and MO transports are map-wide, so walk the map's object store - BotScenario.cpp's
    // StaticTransportFinder idiom, widened to both transport GO types.
    struct TransportByEntryFinder
    {
        uint32 entry;
        float x, y, z;
        Transport* best = nullptr;
        float bestD = 0.0f;
        template <class T> void Visit(std::unordered_map<ObjectGuid, T*>&) { }
        void Visit(std::unordered_map<ObjectGuid, GameObject*>& m)
        {
            for (auto const& p : m)
            {
                GameObject* go = p.second;
                if (!go || !go->IsInWorld() || go->GetEntry() != entry)
                    continue;
                if (go->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT && go->GetGoType() != GAMEOBJECT_TYPE_MO_TRANSPORT)
                    continue;
                if (!go->GetGUID().IsMOTransport())  // #141 F3: only a real Transport object
                    continue;
                float const d = go->GetExactDist(x, y, z);
                if (!best || d < bestD)
                {
                    best = go->ToTransport();
                    bestD = d;
                }
            }
        }
    };

    Transport* FindTransport(Player* bot, uint32 entry, WorldPosition const& nearPos)
    {
        if (!entry || !bot->GetMap())
            return nullptr;
        TransportByEntryFinder finder{entry, nearPos.GetPositionX(), nearPos.GetPositionY(), nearPos.GetPositionZ()};
        TypeContainerVisitor<TransportByEntryFinder, MapStoredObjectTypesContainer> visitor(finder);
        visitor.Visit(bot->GetMap()->GetObjectsStore());
        return finder.best;
    }

    // Same keyframe test as FollowActions.cpp IsDockedNow / PlayerbotAI.cpp IsMoTransportDockedNow (#587: only an MO
    // transport has keyframes).
    bool MoDockedNow(Transport* t)
    {
        if (!t || t->GetGoType() != GAMEOBJECT_TYPE_MO_TRANSPORT || !t->GetTransportTemplate() || !t->GetTransportPeriod())
            return false;
        uint32 const tm = t->GetTimer() % t->GetTransportPeriod();
        for (KeyFrame const& k : t->GetKeyFrames())
            if (tm >= k.ArriveTime && tm < k.DepartureTime)
                return true;
        return false;
    }

    // Milliseconds left in the MO transport's current stop window (0 = moving).
    uint32 MoDockLeftMs(Transport* t)
    {
        if (!t || t->GetGoType() != GAMEOBJECT_TYPE_MO_TRANSPORT || !t->GetTransportTemplate() || !t->GetTransportPeriod())
            return 0;
        uint32 const tm = t->GetTimer() % t->GetTransportPeriod();
        for (KeyFrame const& k : t->GetKeyFrames())
            if (tm >= k.ArriveTime && tm < k.DepartureTime)
                return k.DepartureTime - tm;
        return 0;
    }

    // FollowActions.cpp #589 StaticAnimPos / StaticPlatformStaysFor (copied: they live in that TU's anonymous
    // namespace). Does a static (type-11) platform stay where it is for at least `ms` more? An elevator pauses only
    // ~4-5 s at a landing, so a bot may step on or off only when that fits in what is left of the pause.
    bool StaticAnimPos(TransportAnimation const* anim, uint32 period, uint32 t, G3D::Vector3& out)
    {
        if (!anim || anim->Path.empty() || period < 2)
            return false;
        uint32 const realTimer = (t % period) % (period - 1);
        TransportAnimationEntry const* prev = nullptr;
        TransportAnimationEntry const* next = nullptr;
        for (auto itr = anim->Path.begin(); itr != anim->Path.end(); prev = (itr++)->second)
        {
            next = itr->second;
            if (realTimer <= next->TimeIndex)
                break;
        }
        if (prev == next)
            prev = nullptr;
        if (prev && next->TimeIndex > prev->TimeIndex)
            out = G3D::Vector3(prev->X, prev->Y, prev->Z).lerp(G3D::Vector3(next->X, next->Y, next->Z),
                float(realTimer - prev->TimeIndex) / float(next->TimeIndex - prev->TimeIndex));
        else
            out = G3D::Vector3(next->X, next->Y, next->Z);
        return true;
    }

    bool StaticPlatformStaysFor(Transport* t, uint32 ms)
    {
        if (!t || t->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
            return false;
        TransportAnimation const* anim = t->GetGOValue()->Transport.AnimationInfo;
        uint32 const period = t->GetTransportPeriod();
        if (!anim || anim->Path.empty() || period < 2)
            return true;
        uint32 const timer = t->GetTimer() % period;
        if (uint32 const pause = t->GetUInt32Value(GAMEOBJECT_FIELD_LEVEL))
            if (t->GetGoState() == GO_STATE_ACTIVE ? timer == pause : timer == period - 1)
                return true;
        G3D::Vector3 p0;
        if (!StaticAnimPos(anim, period, timer, p0))
            return true;
        uint32 const step = 100;
        for (uint32 dt = step; dt < ms + step; dt += step)
        {
            G3D::Vector3 p;
            StaticAnimPos(anim, period, timer + std::min(dt, ms), p);
            if ((p - p0).squaredLength() > 0.1f * 0.1f)
                return false;
        }
        return true;
    }

    // Will the platform still be here after `ms`? (step-on / step-off window)
    bool StaysFor(Transport* t, uint32 ms)
    {
        if (!t)
            return false;
        if (t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
            return MoDockLeftMs(t) >= ms;
        return StaticPlatformStaysFor(t, ms);
    }

    // Deck surface of t under (x, y), searched downward from fromZ for maxDown yards (GameObjectModel ray, the same
    // primitive as PlayerbotAI.cpp IsUnderStaticTransportDeck).
    bool DeckZAt(Transport* t, float x, float y, float fromZ, float maxDown, float& outZ)
    {
        if (!t || !t->m_model)
            return false;
        G3D::Ray const r(G3D::Vector3(x, y, fromZ), G3D::Vector3(0.0f, 0.0f, -1.0f));
        float dist = maxDown;
        if (!t->m_model->intersectRay(r, dist, true, t->GetPhaseMask(), VMAP::ModelIgnoreFlags::Nothing))
            return false;
        outZ = fromZ - dist;
        return true;
    }

    bool DeckUnder(Transport* t, Player* bot)
    {
        float z = 0.0f;
        return DeckZAt(t, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() + 2.0f, 4.0f, z);
    }

    // Is the platform at the dock the route boards at? MO: inside a keyframe stop and near the board node (the node IS
    // the transport origin at that stop, TravelMgr.cpp transport node generation). Static: the origin is at the node.
    //
    // Wave 51 (#218): the static-transport nodes come from the AC playerbots DB (playerbots_travelnode), not from this
    // core's animation. For the Telredor elevator 183177 (TransportAnimation 127203-127287, stops at local (0,0,0) for
    // 5 s, (3.72,0,123.23) for 0.33 s and (-17.26,-1.56,123.25) for 5 s) the AC nodes 1974 (282.0,5918.2,149.4) and
    // 1975 (303.0,5919.8,149.4) are the spawn MINUS the x/y offset, while Transport::Update (Transport.cpp:380-405)
    // adds it with the identity parent rotation this core gives 183177 (Transport.cpp:115, no sniff row): the
    // platform pauses at (268.5,5916.7,149.4), 34.6 yd from node 1975. The 4 yd test never passed: Blanuel waited
    // 867 s (dock-wait-timeout) and Ophele 650+ s at (294.5,5942.7,150.0) in w50. A static platform that is paused
    // on the node's level within kStaticStopReach of it is at that stop too. 25 yd on purpose: with the GO's own
    // rotation (0,0,-0.6947,0.7193) as parent rotation (what the client-side retail stop implies) stop B is at
    // (283.6,5935.4,149.4), 19.6 yd from node 1975 and 13 yd from where the bots wait; the identity-rotation spot
    // 34.6 yd off (likely inside the mushroom cap) is NOT accepted, it logs "wait-why stopped-elsewhere" instead.
    constexpr float kStaticStopReach = 25.0f;
    constexpr float kStopLevelZ = 3.0f;   // Scaffold Cars 149045 stops are 15-27 yd apart, elevators 60+ yd
    bool AtStop(Transport* t, WorldPosition const& node)
    {
        if (!t)
            return false;
        if (t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
            return MoDockedNow(t) && t->GetExactDist2d(node.GetPositionX(), node.GetPositionY()) < 30.0f;
        if (t->GetExactDist(node.GetPositionX(), node.GetPositionY(), node.GetPositionZ()) < 4.0f)
            return true;
        return std::fabs(t->GetPositionZ() - node.GetPositionZ()) <= kStopLevelZ &&
               t->GetExactDist2d(node.GetPositionX(), node.GetPositionY()) <= kStaticStopReach &&
               StaticPlatformStaysFor(t, 250);
    }

    // A deck point a bot standing at the dock can walk onto: march from the bot toward the transport origin until the
    // model is under the march point no more than 2.5 yd above / 8 yd below the bot's feet, then go 3 yd further in.
    // #848: (ex, ey) is where the deck starts on that line (the march point it was first found under), the spot where
    // Board attaches a bot walking onto a static platform.
    bool FindDeckPoint(Transport* t, Player* bot, float& ox, float& oy, float& oz, float& ex, float& ey)
    {
        float const bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
        // #218 (w90 Z4 Darcellis, Iron Eagle 175080 at the Orgrimmar tower): a boat / zeppelin deck is boarded from a
        // dock on its level. The static-platform window (2.5 yd above the feet down to 8 yd below) took a hull part 6.5 yd
        // under a zeppelin that was still settling ("board ... (1872.7,-4425.8,129.1)" from the 135.6 dock, stored local
        // z -27.78 where riders stand at -17.8): the rider hung under the hull and the ambient check dropped it at
        // Grom'gol. For an MO transport only a deck from 4 yd above to 4 yd below the feet counts; a higher one is
        // found on a later tick once the ship has settled. Static platforms keep their window.
        bool const mo = t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
        float const rayFrom = bz + (mo ? 4.0f : 2.5f);
        float const rayDown = mo ? 8.0f : 10.5f;
        float const cx = t->GetPositionX(), cy = t->GetPositionY();
        float const len = std::sqrt((cx - bx) * (cx - bx) + (cy - by) * (cy - by));
        if (len < 0.5f)
        {
            float z = 0.0f;
            if (!DeckZAt(t, bx, by, rayFrom, rayDown, z))
                return false;
            ox = bx; oy = by; oz = z;
            ex = bx; ey = by;
            return true;
        }
        float const ux = (cx - bx) / len, uy = (cy - by) / len;
        for (float s = 0.0f; s <= std::min(len, 45.0f); s += 1.0f)
        {
            float const px = bx + ux * s, py = by + uy * s;
            float z = 0.0f;
            if (!DeckZAt(t, px, py, rayFrom, rayDown, z))
                continue;
            float const inner = std::min(s + 3.0f, len);
            float iz = z;
            float const ix = bx + ux * inner, iy = by + uy * inner;
            if (!DeckZAt(t, ix, iy, rayFrom, rayDown, iz))
                iz = z;
            ox = ix; oy = iy; oz = iz;
            ex = px; ey = py;
            return true;
        }
        return false;
    }

    // #848: how far a rider standing on a static platform walks toward (tx, ty) before the deck is no longer under it
    // (Leave detaches there). The deck test is Leave's own (a 12 yd ray for a static platform). At most the distance to
    // the target.
    float DeckExitDist(Transport* t, Player* bot, float tx, float ty)
    {
        float const bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
        float const len = std::sqrt((tx - bx) * (tx - bx) + (ty - by) * (ty - by));
        if (len < 0.5f)
            return len;
        float const ux = (tx - bx) / len, uy = (ty - by) / len;
        for (float s = 0.5f; s < len; s += 0.5f)
        {
            float z = 0.0f;
            if (!DeckZAt(t, bx + ux * s, by + uy * s, bz + 2.0f, kDockHeight, z))
                return s;
        }
        return len;
    }

    // #848: the walk time a static step-on / step-off needs (see kStaticStepMarginMs).
    uint32 StaticStepMs(Player* bot, float dist)
    {
        float walkSpeed = std::max(2.5f, bot->GetSpeed(MOVE_RUN));
        if (bot->IsMounted())
            walkSpeed = std::min(walkSpeed, playerBaseMoveSpeed[MOVE_RUN]);  // Board dismounts for a static platform
        return uint32(dist / walkSpeed * IN_MILLISECONDS) + kStaticStepMarginMs;
    }

    // Where to step off at the arrival dock: the first following walk point on the bot's map that is at least 8 yd
    // (2D) from the transport origin and on dry ground; else the first following point; else the arrival node.
    //
    // Wave 34 (#218, leg E up): BuildRoute's path past the last node is that node alone (getPathTo(endPos, nullptr)
    // runs no pathfinding: WorldPosition.cpp getPathStepFrom returns {} without a bot, AC the same), so a route that
    // ends with a transport leg has no point off the deck. The fallback was the arrival node = the platform origin
    // in the shaft ("leave->(-1286.2,189.7,130.1)"): Stanne walked to the middle of the Mesa Elevator and stayed
    // aboard. With no route point off the deck, step off toward the trip's destination instead: the destination
    // itself when it is close and level with the landing, else a point 12 yd out from the origin on the ground.
    void PickLeavePoint(RoamTrip& trip, Player* bot, Transport* t, float& ox, float& oy, float& oz)
    {
        size_t const arrival = trip.idx + 1;
        WorldPosition const& arr = trip.pts[arrival].point;
        ox = arr.GetPositionX(); oy = arr.GetPositionY(); oz = arr.GetPositionZ();
        bool haveFallback = false;
        for (size_t i = arrival + 1; i < trip.pts.size() && i < arrival + 40; ++i)
        {
            WorldPosition const& p = trip.pts[i].point;
            if (p.GetMapId() != bot->GetMapId())
                break;
            if (!haveFallback)
            {
                ox = p.GetPositionX(); oy = p.GetPositionY(); oz = p.GetPositionZ();
                haveFallback = true;
            }
            if (t && t->GetExactDist2d(p.GetPositionX(), p.GetPositionY()) < kDeckFootprint)
                continue;
            if (bot->GetMap()->IsInWater(bot->GetPhaseMask(), p.GetPositionX(), p.GetPositionY(), p.GetPositionZ()))
                continue;
            ox = p.GetPositionX(); oy = p.GetPositionY(); oz = p.GetPositionZ();
            return;
        }

        // No route point off the deck: head for the destination (wave 34).
        if (!t)
            return;
        WorldPosition const& dest = trip.dest;
        float const ax = t->GetPositionX(), ay = t->GetPositionY(), az = arr.GetPositionZ();
        bool const destHere = dest.GetMapId() == bot->GetMapId();
        if (destHere)
        {
            float const dx = dest.GetPositionX() - ax, dy = dest.GetPositionY() - ay;
            float const dd = std::sqrt(dx * dx + dy * dy);
            if (dd < 0.5f)
                return;
            if (dd >= kDeckFootprint && dd <= 30.0f && std::fabs(dest.GetPositionZ() - az) <= kDockHeight &&
                !bot->GetMap()->IsInWater(bot->GetPhaseMask(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ()))
            {
                ox = dest.GetPositionX(); oy = dest.GetPositionY(); oz = dest.GetPositionZ();
                return;
            }
            // 12 yd out toward the destination, on the ground of the landing's level (searched from a few yards above
            // the arrival node, never from the roof of an elevator housing).
            float const sx = ax + dx / dd * 12.0f, sy = ay + dy / dd * 12.0f;
            float const gz = bot->GetMap()->GetHeight(bot->GetPhaseMask(), sx, sy, az + 6.0f, true, 6.0f + kDockHeight);
            float deckZ = 0.0f;
            if (std::fabs(gz - az) <= kDockHeight && !bot->GetMap()->IsInWater(bot->GetPhaseMask(), sx, sy, gz) &&
                !DeckZAt(t, sx, sy, gz + 2.0f, 4.0f, deckZ))
            {
                ox = sx; oy = sy; oz = gz;
                return;
            }
        }
        // Wave 51 (#218): a cross-map route now ends at the arrival dock (ValidateRoute end-far), and a boat's origin
        // is in the water, so the old fallback (the arrival node) walked the rider to the middle of the deck and it
        // never got off. Search a ring of dry ground on the landing's level around the platform, off the deck, nearest
        // the destination (or the bot when the destination is on another map).
        float bestScore = -1.0f;
        for (float r : {12.0f, 18.0f, 25.0f})
        {
            for (uint32 k = 0; k < 16; ++k)
            {
                float const a = float(k) * float(M_PI) / 8.0f;
                float const sx = ax + std::cos(a) * r, sy = ay + std::sin(a) * r;
                float const gz = bot->GetMap()->GetHeight(bot->GetPhaseMask(), sx, sy, az + 6.0f, true, 6.0f + kDockHeight);
                if (std::fabs(gz - az) > kDockHeight)
                    continue;
                if (bot->GetMap()->IsInWater(bot->GetPhaseMask(), sx, sy, gz))
                    continue;
                float deckZ = 0.0f;
                if (DeckZAt(t, sx, sy, gz + 2.0f, 4.0f, deckZ))
                    continue;
                float const score = destHere ? dest.GetExactDist(sx, sy, gz) : bot->GetExactDist2d(sx, sy);
                if (bestScore < 0.0f || score < bestScore)
                {
                    bestScore = score;
                    ox = sx; oy = sy; oz = gz;
                }
            }
            if (bestScore >= 0.0f)
                return;
        }
    }

    // Wave 34 (#218): points on the arrival deck (the arrival node and anything within its footprint on the same
    // level). Once the bot has stepped off they are behind it: walking to one would lead back into an elevator shaft.
    bool OnArrivalDeck(WorldPosition const& p, WorldPosition const& arr)
    {
        return p.GetMapId() == arr.GetMapId() &&
               p.GetExactDist2d(arr.GetPositionX(), arr.GetPositionY()) < kDeckFootprint &&
               std::fabs(p.GetPositionZ() - arr.GetPositionZ()) <= kDockHeight;
    }

    void StopHere(Player* bot)
    {
        if (MotionMaster* mm = bot->GetMotionMaster())
            mm->Clear(true);
        bot->StopMoving();
    }

    // At the trip's destination: on its map, off any transport, inside AC's GO_GRIND / GO_CAMP arrival distance (2D for
    // a flying bot, AC NewRpgAction.cpp GrindArrivalDist).
    bool AtDest(Player* bot, WorldPosition const& dest)
    {
        if (dest.GetMapId() != bot->GetMapId() || bot->GetTransport())
            return false;
        return (bot->IsFlying() ? Dist2d(bot, dest) : Dist3d(bot, dest)) < kArriveDist;
    }

    // Wave 41 (#218): every trip ends here, in exactly one "arrive" (why == nullptr) or "abandon <why>" line.
    // retryGate: an abandon blocks a new route to the same destination for kRetryAfterFailMs.
    void EndTrip(RoamTrip& trip, Player* bot, uint32 now, char const* why, bool retryGate = true)
    {
        uint32 const t = getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS;
        if (!why)
        {
            // Wave 37 (#218, elevator DOWN): a bot that stepped off its arrival stop between two ticks reaches here
            // still in the Leave phase, so the step-off was never logged. Log it before the arrival.
            if (trip.phase == TripPhase::Leave)
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s leave transport=%u map=%u (%.1f,%.1f,%.1f) t=%us", bot->GetName().c_str(),
                             trip.transportEntry, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                             bot->GetPositionZ(), t);
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s arrive map=%u (%.1f,%.1f,%.1f) legs=%u t=%us", bot->GetName().c_str(),
                         bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), trip.legs, t);
        }
        else
        {
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s abandon %s phase=%s point=%u/%u legs=%u map=%u (%.1f,%.1f,%.1f) dest=%u (%.1f,%.1f,%.1f) t=%us",
                         bot->GetName().c_str(), why, PhaseName(trip.phase), uint32(trip.idx), uint32(trip.pts.size()), trip.legs,
                         bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), trip.dest.GetMapId(),
                         trip.dest.GetPositionX(), trip.dest.GetPositionY(), trip.dest.GetPositionZ(), t);
            // Never leave an abandoned rider attached: masterless, the ambient check would keep it aboard for good (#586).
            if (Transport* tr = bot->GetTransport())
            {
                bool const wasMo = tr->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
                StopHere(bot);
                tr->RemovePassenger(bot, true);
                if (wasMo)
                    bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
            }
            if (retryGate)
                trip.failAtMs = now ? now : 1;
        }
        SetActive(trip, false);
        trip.pts.clear();
    }

    char const* StatusChangedReason(NewRpgStatus s)
    {
        switch (s)
        {
            case RPG_IDLE: return "status-idle";
            case RPG_REST: return "status-rest";
            case RPG_WANDER_RANDOM: return "status-wander-random";
            case RPG_WANDER_NPC: return "status-wander-npc";
            case RPG_GO_GRIND: return "status-go-grind";
            case RPG_GO_CAMP: return "status-go-camp";
            case RPG_DO_QUEST: return "status-do-quest";
            case RPG_TRAVEL_FLIGHT: return "status-travel-flight";
            case RPG_OUTDOOR_PVP: return "status-outdoor-pvp";
            case RPG_DUEL_SPOT: return "status-duel-spot";
            default: return "status-changed";
        }
    }

    // Wave 41 (#218): a trip is driven only by MoveFarTo calls from the RPG status that started it. 10 trips started,
    // none logged arrive or abandon in the run window:
    //  - Achris / Oneya landed 58-68 yd from their GO_CAMP hub and walked on; at 10 yd NewRpgStatusUpdateAction
    //    switched GO_CAMP -> WANDER_NPC (AC NewRpgAction.cpp:304-316) before the trip's own arrival test (route end
    //    within kPointReached, or kArriveDist on the next MoveFarTo) ever ran, so nothing called the trip again;
    //  - Ivander was refreshed ("teleport for level and refresh", PlayerbotAI::Reset -> RPG_IDLE) in the Taxi phase;
    //  - the scenario's Eleny stepped off the Mesa Elevator and the goto ended at 4.8 yd the same way.
    // Such a trip stayed active for good (and its Walk phase kept the bot at the base react delay). Called once per AI
    // tick (PlayerbotAI::GetReactDelay via WantsFastReact). Returns true when the trip was ended.
    bool EndUndrivenTrip(RoamTrip& trip, Player* bot)
    {
        uint32 const now = getMSTime();
        // MoveFarTo returns before RoamTransportMove while the bot is in flight, so the Taxi phase never saw it: note the
        // flight here (wave 36 taxi-not-landed test).
        if (trip.phase == TripPhase::Taxi && bot->IsInFlight())
        {
            trip.flew = true;
            trip.progressMs = now;
        }
        if (AtDest(bot, trip.dest))
        {
            EndTrip(trip, bot, now, nullptr);
            return true;
        }
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI || !trip.driverT)
            return false;
        NewRpgInfo const& info = botAI->GetRpgInfo();
        NewRpgStatus const status = info.GetStatus();
        if (info.startT == trip.driverT && int32(status) == trip.driverStatus)
            return false;
        // ResumeAfterReset (and BotScenario's re-arm) re-apply GO_GRIND / GO_CAMP to the trip's destination after a far
        // teleport: that status drives the same trip.
        WorldPosition const* pos = nullptr;
        if (auto const* g = std::get_if<NewRpgInfo::GoGrind>(&info.data))
            pos = &g->pos;
        else if (auto const* c = std::get_if<NewRpgInfo::GoCamp>(&info.data))
            pos = &c->pos;
        if (pos && pos->GetMapId() == trip.dest.GetMapId() &&
            pos->GetExactDist2d(trip.dest.GetPositionX(), trip.dest.GetPositionY()) < 1.0f)
        {
            trip.driverT = info.startT;
            trip.driverStatus = int32(status);
            return false;
        }
        // A rider is detached at a stop only, never in open water or an elevator shaft.
        if (Transport* t = bot->GetTransport())
            if (!StaysFor(t, 1000))
                return false;
        EndTrip(trip, bot, now, StatusChangedReason(status));
        return true;
    }

    // Returns the legs that make a trip worth following (transport, flight path, portal). A hearthstone leg
    // (NODE_TELEPORT, the getRoute 8690 seed) is counted apart and never qualifies on its own (wave 30, #218).
    size_t CountLegs(std::vector<PathNodePoint> const& pts, uint32& transportLegs, uint32& flightLegs, uint32& portalLegs,
                     uint32& hearthLegs)
    {
        transportLegs = flightLegs = portalLegs = hearthLegs = 0;
        for (size_t i = 0; i + 1 < pts.size(); ++i)
        {
            if (!IsLegStart(pts, i))
                continue;
            if (pts[i].type == NODE_TRANSPORT)
                ++transportLegs;
            else if (pts[i].type == NODE_FLIGHTPATH)
                ++flightLegs;
            else if (pts[i].type == NODE_PORTAL)
                ++portalLegs;
            else
                ++hearthLegs;
            ++i;
        }
        return transportLegs + flightLegs + portalLegs;
    }

    // Maps the travel graph has nodes on (built once the graph is loaded; Pandaria 870 has none). A bot on a map
    // without nodes gets no A* at all: the route could only start on another continent (wave 30 walk-wrong-map).
    bool MapHasTravelNodes(uint32 mapId)
    {
        static std::mutex mtx;
        static std::set<uint32> maps;
        static bool built = false;
        std::lock_guard<std::mutex> lock(mtx);
        if (!built)
        {
            std::shared_lock<std::shared_timed_mutex> guard(TravelNodeMap::instance().m_nMapMtx);
            for (TravelNode* n : TravelNodeMap::instance().getNodes())
                if (n)
                    maps.insert(n->getMapId());
            built = !maps.empty();  // an empty graph (not loaded yet) is read again next time
        }
        return maps.count(mapId) != 0;
    }

    // ---- wave 36 (#218): can this bot use the flight master at a flight leg's departure? ---------------------------
    // Two w36 trips walked to a flight master the bot could not use and abandoned there (taxi-no-flightmaster):
    //  - Babetta (Worgen) at The Shadow Vault (travel node 337, Morlia Doomwing 30314, phaseMask 1): spell_area 30181
    //    (area 4477, until quest 12896 / 12897 is done) is SPELL_AURA_PHASE misc 2, so the bot stands in phase 2 and
    //    never sees her.
    //  - Iwarco (Human) at Dun Niffelem (node 504, Halvdan 32571): phaseMask 4, and his faction template 2107 is
    //    Sons of Hodir (faction 1119, base reputation -42000, at war), so he is hidden AND hostile.
    // The travel graph's A* only checks the taxi nodes' mount per team (TravelNode.cpp getCost, AC the same). AC's own
    // flight-master pick filters by faction template only (AC Mgr/Travel/TravelMgr.cpp:4699-4701 forHorde /
    // forAlliance = !(hostileMask & team), MoP Travel/TravelMgr.cpp:626-627) and gives up on the spot when
    // FindNearestCreature - which is phase-aware - finds nobody (AC Ai/World/Rpg/Action/NewRpgAction.cpp:728-732,
    // MoP NewRpgAction.cpp:915-925). A route is built for a destination minutes away, so check up front: a flight
    // master spawned near the departure that passes AC's faction-template test, is not hostile by reputation (the core's
    // Unit::GetFactionReactionTo order: rank, capped at neutral when at war), and shares a phase with the phase the bot
    // will have there (zone / area spell_area SPELL_AURA_PHASE auras the bot fits, else PHASEMASK_NORMAL; PhaseMgr
    // GetPhaseMaskForSpawn). Spawns with a phase id / phase group are not judged (no mask to compare).
    struct FlightMasterSpawn
    {
        uint32 mapId;
        float x, y, z;
        uint32 entry;
        uint32 faction;
        uint32 phaseMask;
        bool phaseIds;
    };

    std::vector<FlightMasterSpawn> const& FlightMasterSpawns()
    {
        static std::vector<FlightMasterSpawn> spawns;
        static std::once_flag once;
        std::call_once(once, []()
        {
            for (auto const& p : sObjectMgr->GetAllCreatureData())
            {
                CreatureData const& d = p.second;
                CreatureTemplate const* t = sObjectMgr->GetCreatureTemplate(d.id);
                if (!t)
                    continue;
                uint32 npcflag = 0, unitFlags = 0, dynFlags = 0;
                ObjectMgr::ChooseCreatureFlags(t, npcflag, unitFlags, dynFlags, &d);
                if (!(npcflag & UNIT_NPC_FLAG_FLIGHTMASTER))
                    continue;
                spawns.push_back({d.mapId, d.posX, d.posY, d.posZ, d.id, t->faction, d.phaseMask,
                                  d.phaseid != 0 || d.phaseGroup != 0});
            }
        });
        return spawns;
    }

    uint32 ExpectedPhaseMask(Player* bot, uint32 mapId, float x, float y, float z)
    {
        uint32 zoneId = 0, areaId = 0;
        sMapMgr->GetZoneAndAreaId(PHASEMASK_NORMAL, zoneId, areaId, mapId, x, y, z);
        uint32 mask = 0;
        // Player::UpdateZoneDependentAuras / UpdateAreaDependentAuras (Player.cpp:26904-26947): the zone's autocast
        // spell_area rows, then the area's.
        auto addPhases = [&](uint32 id)
        {
            SpellAreaForAreaMapBounds const bounds = sSpellMgr->GetSpellAreaForAreaMapBounds(id);
            for (auto itr = bounds.first; itr != bounds.second; ++itr)
            {
                SpellArea const* sa = itr->second;
                if (!sa->autocast || !sa->IsFitToRequirements(bot, zoneId, areaId))
                    continue;
                SpellInfo const* info = sSpellMgr->GetSpellInfo(sa->spellId);
                if (!info)
                    continue;
                for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                    if (info->Effects[i].ApplyAuraName == SPELL_AURA_PHASE && info->Effects[i].MiscValue > 0)
                        mask |= uint32(info->Effects[i].MiscValue);
            }
        };
        if (zoneId)
            addPhases(zoneId);
        if (areaId && areaId != zoneId)
            addPhases(areaId);
        return mask ? mask : uint32(PHASEMASK_NORMAL);
    }

    bool FlightMasterUsable(Player* bot, WorldPosition const& dep)
    {
        uint32 const teamMask = bot->GetTeam() == ALLIANCE ? FACTION_MASK_ALLIANCE : FACTION_MASK_HORDE;
        uint32 expected = 0;  // computed on the first spawn that needs it
        for (FlightMasterSpawn const& fm : FlightMasterSpawns())
        {
            if (fm.mapId != dep.GetMapId() ||
                dep.GetExactDist(fm.x, fm.y, fm.z) > INTERACTION_DISTANCE * 3)  // the trip's own search radius
                continue;
            FactionTemplateEntry const* ft = sFactionTemplateStore.LookupEntry(fm.faction);
            if (!ft || (ft->hostileMask & teamMask))
                continue;  // AC's test
            if (FactionEntry const* fe = sFactionStore.LookupEntry(ft->faction))
                if (fe->CanHaveReputation())
                {
                    ReputationRank rank = bot->GetReputationMgr().GetRank(fe);
                    if (bot->GetReputationMgr().IsAtWar(fe))
                        rank = std::min(REP_NEUTRAL, rank);
                    if (rank <= REP_HOSTILE)
                        continue;
                }
            if (!fm.phaseIds)
            {
                if (!expected)
                    expected = ExpectedPhaseMask(bot, fm.mapId, fm.x, fm.y, fm.z);
                if (!(fm.phaseMask & expected))
                    continue;
            }
            return true;
        }
        return false;
    }

    // Wave 30 (#218): 947 of 1103 abandons were "walk-wrong-map ... point=0 t=0s" on Pandaria (map 870), whose travel
    // graph has no nodes: getFullPath then routes from the nearest nodes on ANOTHER continent, so no point before the
    // first leg was on the bot's map and the trip started at point 0 on the wrong map. A route is followed only when
    // it is coherent end to end: it starts on the bot's map within kStartReach of the bot, every walk point and every
    // leg departure lies on the map the bot is on at that point (a leg moves it to its arrival point's map), and it
    // ends on the destination's map within kEndReach of the destination. Returns nullptr when valid, else the reason.
    //
    // Wave 33 (#218): a cross-map route may end on an intermediate map (allowHop): AC's A* (TravelNode.cpp getRoute,
    // the "dataNode->getMapId() != start->getMapId() && isWalking()" exit) stops at the first walkable node on ANY
    // other map, so Northrend -> Kalimdor came back ending in the Eastern Kingdoms (51 "end-map" in wave 33). Such a
    // route is followed and re-planned at its end (kMaxHops), unless it would go back to the map the last hop left.
    // A transport leg whose departure is a landing straight above / below the walk point before it, while its
    // arrival is level with that point, runs the wrong way (an elevator's two landings share x/y) - "leg-reversed".
    constexpr float kStartReach = 80.0f;     // a first route point this close is walked as before (no path test)
    constexpr float kEndReach = 150.0f;
    constexpr float kStartPathMax = 600.0f;  // wave 35: longest navmesh walk to the first route point
    constexpr float kStartPathRatio = 3.0f;  // ... and at most this many times the straight line (plus 60 yd)

    // Wave 35 (#218): the w34 "start-far" cut at 80 yd straight line rejected every cross-continent route with a
    // transport leg (travel nodes are sparse: the first one is 100-450 yd off; w35 start-far 571->571 x27, 1->1 x11,
    // 1->571, 1->0, 530->1). BuildRoute cannot give the walk to it a navmesh path (no bot, see BuildRoute), so ask the
    // MMAP PathGenerator - the one MovementAction::MoveTo checks every walk with - from the bot to that point, once per
    // route build (route builds are rate-limited, kRouteBuildGapMs). Returns nullptr when the point is reachable on
    // foot and fills `out` with the path (the bot's own position first), else the reason. A partial path (the
    // 200-polygon / 200-point limits of PathGenerator) counts when it is long and closes most of the gap; the rest
    // is walked by the trip's own pathfinding MoveTo from where the partial path ends.
    char const* StartPath(Player* bot, WorldPosition const& target, bool dock, std::vector<WorldPosition>& out)
    {
        out.clear();
        float const straight = Dist3d(bot, target);
        PathGenerator gen(bot);
        gen.CalculatePath(target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(), false);
        PathType const type = gen.GetPathType();
        if (type & PATHFIND_NOT_USING_PATH)
            return nullptr;  // no navmesh here (or a flying bot): AC's straight line is all there is, as before
        if (!(type & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE)) || (type & (PATHFIND_NOPATH | PATHFIND_SHORT)))
            return "start-unreachable";
        float const len = gen.getPathLength();
        G3D::Vector3 const& e = gen.GetActualEndPosition();
        float const gap = target.GetExactDist(e.x, e.y, e.z);
        // A dock node is the transport origin (in the water / an elevator shaft): ending within the dock radius of it
        // is arriving. Any other route point is on the ground.
        float const gapOk = dock ? kDockRadius : 10.0f;
        float est = len;
        if (gap > gapOk)
        {
            if (len < 150.0f || gap > straight * 0.75f)
                return "start-unreachable";  // a short dead end (another navmesh island, a cliff top)
            est = len + gap;
        }
        if (est > kStartPathMax || (est > straight * kStartPathRatio && est - straight > 60.0f))
            return "start-unreachable";
        for (G3D::Vector3 const& v : gen.GetPath())
            out.emplace_back(bot->GetMapId(), v.x, v.y, v.z);
        return nullptr;
    }

    char const* ValidateRoute(std::vector<PathNodePoint> const& pts, Player* bot, WorldPosition const& dest, size_t& start,
                              bool allowHop, uint32 noHopMap, std::vector<WorldPosition>& startPath)
    {
        startPath.clear();
        // Wave 34 (#218): the route's first point is the bot's own position (BuildRoute's toStart is {startPos}: no
        // pathfinding without a bot, WorldPosition.cpp getPathStepFrom), so the nearest pre-leg point was always 0 yd
        // away and start-far never fired. Routes whose start node lay 100-445 yd off were followed and ended in
        // off-route at t=0 (Antavo 380 yd, Eldarion 445 yd) or walk-stall (Ureno 120, Oswalthar 103, Iley 198 yd).
        // Measure the reach to the first real route point instead.
        size_t const first = (pts.size() > 1 && pts[0].type == NODE_PREPATH && !IsLegStart(pts, 0) &&
                              pts[0].point.GetMapId() == bot->GetMapId() && Dist3d(bot, pts[0].point) < kPointReached)
                                 ? 1 : 0;
        start = first;
        float bestD = -1.0f;
        for (size_t i = first; i < pts.size(); ++i)
        {
            if (IsLegStart(pts, i))
                break;
            if (pts[i].point.GetMapId() != bot->GetMapId())
                continue;
            float const d = Dist3d(bot, pts[i].point);
            if (bestD < 0.0f || d < bestD)
            {
                bestD = d;
                start = i;
            }
        }
        if (bestD < 0.0f)
        {
            // No walk point on this map before the first leg: only valid when the route opens with a leg departing here.
            if (!IsLegStart(pts, first) || pts[first].point.GetMapId() != bot->GetMapId())
                return "start-off-map";
            start = first;
            bestD = Dist3d(bot, pts[first].point);
        }
        // Wave 35: beyond kStartReach the first point must be reachable on foot (StartPath, checked last: it is the
        // only expensive test) instead of the w34 hard "start-far" cut.
        bool const needStartPath = bestD > kStartReach;
        if (bestD > kStartPathMax)
            return "start-unreachable";  // no walk can be shorter than the straight line (w35: 1189-8308 yd)

        uint32 curMap = bot->GetMapId();
        // wave 51: a boat / zeppelin / elevator / tram leg in the followed part. #847 (w86 R3, Shrine of Two Moons ->
        // Orgrimmar): a portal leg counts too - A* stops at the portal's arrival node (3893, the Earthshrine 327 yd
        // above the hub), exactly as it stops at a boat's arrival dock, and the route was rejected "end-far".
        bool transportLeg = false;
        for (size_t i = start; i < pts.size(); ++i)
        {
            if (IsLegStart(pts, i))
            {
                if (pts[i].point.GetMapId() != curMap)
                    return "leg-map";
                if (pts[i].type == NODE_TRANSPORT || pts[i].type == NODE_PORTAL)
                    transportLeg = true;
                // Wave 36: a flight leg needs a flight master this bot can use (FlightMasterUsable). Only legs on the
                // bot's own map are judged: the area lookup must stay on the map this thread updates.
                if (pts[i].type == NODE_FLIGHTPATH && curMap == bot->GetMapId() && !FlightMasterUsable(bot, pts[i].point))
                    return "flightmaster-unusable";
                if (pts[i].type == NODE_TRANSPORT && i > 0 && IsWalkPoint(pts[i - 1]) &&
                    pts[i - 1].point.GetMapId() == curMap && pts[i + 1].point.GetMapId() == curMap)
                {
                    WorldPosition const& before = pts[i - 1].point;
                    WorldPosition const& dep = pts[i].point;
                    WorldPosition const& arr = pts[i + 1].point;
                    if (before.GetExactDist2d(dep.GetPositionX(), dep.GetPositionY()) < kDockRadius &&
                        arr.GetExactDist2d(dep.GetPositionX(), dep.GetPositionY()) < kDockRadius &&  // stacked landings
                        std::fabs(before.GetPositionZ() - dep.GetPositionZ()) > kDockHeight &&
                        std::fabs(before.GetPositionZ() - arr.GetPositionZ()) <= kDockHeight)
                        return "leg-reversed";
                }
                // #843 (wave 71p): a leg into an instance map is never a roam hop. The travel graph keeps dungeon
                // entrances as portal links, so a cross-map camp route from the Barrens took the Ragefire Chasm
                // entrance (areatrigger 2230, map 1 -> 389) as its "hop" (end-map=389): Cellietta then sat inside
                // the dungeon for 28 min, WANDER_NPC ping-ponging between its 3 NPCs (no route back: hop-back).
                if (pts[i + 1].point.GetMapId() != dest.GetMapId())
                    if (MapEntry const* arrEntry = sMapStore.LookupEntry(pts[i + 1].point.GetMapId()))
                        if (arrEntry->Instanceable())
                            return "leg-instance";
                curMap = pts[i + 1].point.GetMapId();
                ++i;
                continue;
            }
            if (pts[i].point.GetMapId() != curMap)
                return "walk-map";
        }
        if (curMap != dest.GetMapId())
        {
            if (!allowHop || curMap == bot->GetMapId())
                return "end-map";
            if (noHopMap && curMap == noHopMap)
                return "hop-back";
            // ends on an intermediate map: followed, then re-planned from there
            return needStartPath ? StartPath(bot, pts[start].point, IsLegStart(pts, start), startPath) : nullptr;
        }
        // Wave 51 (#218): A* (TravelNode.cpp getRoute, AC TravelNode.cpp:1260 the same) returns as soon as it pops the
        // first walkable node on any map other than the start's, so a cross-continent route always ends at the
        // arrival dock, not at the hub: w50 rejected every boat / zeppelin route that reached the hub's continent as
        // "end-far" (Gracie 0->1, Titandro 0->1, Berolandel 1->0, Artio and Caelar 530->1, all transport=1; w47 the
        // same). The leg is what the trip is for: such a route is followed to the arrival dock and AC's MoveFarTo
        // walks the rest on that map ("arrive route-end" returns -1). The same for a same-map route whose transport
        // leg is the only way out (Telredor 183177: nodes 1974/1975 have no walk links; Linevelina, Rentan w50).
        WorldPosition const& last = pts.back().point;
        if (last.GetExactDist(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ()) > kEndReach &&
            !(transportLeg && last.GetMapId() == dest.GetMapId()))
            return "end-far";
        return needStartPath ? StartPath(bot, pts[start].point, IsLegStart(pts, start), startPath) : nullptr;
    }

    // #846 (w86 org-down): a route that STARTS at a transport dock has no walk point before the leg (BuildRoute's toStart
    // is the bot's own position: no pathfinding without a bot), so the trip walks straight at the dock node. Elevator
    // nodes are the platform origin in the shaft: Stanne, on the Orgrimmar tower level at z 90.8, walked at the 206610
    // top landing node (1755.3,-4396.6,109.3) 18.5 yd above him - no navmesh point there - and gave up after 120 s
    // ("abandon dock-walk-stall phase=walk point=1/8"); the offline Detour path to the landing floor is 190 yd long.
    // The dock's own walk links know where its landing is: the first point of a walk link path that is off the
    // platform (>= 5 yd 2D from the node) and on the node's level. Returns the one nearest `from`.
    bool DockLandPoint(TravelNode* dock, WorldPosition const& from, WorldPosition& out)
    {
        float bestD = -1.0f;
        for (auto const& link : *dock->getLinks())
        {
            if (!link.second || link.second->getPathType() != TravelNodePathType::walk)
                continue;
            for (WorldPosition const& p : link.second->getPath())
            {
                if (p.GetMapId() != dock->getMapId() || std::fabs(p.GetPositionZ() - dock->getZ()) > kDockHeight)
                    break;
                if (p.GetExactDist2d(dock->getX(), dock->getY()) < 5.0f)
                    continue;
                float const d = from.GetExactDist(p.GetPositionX(), p.GetPositionY(), p.GetPositionZ());
                if (bestD < 0.0f || d < bestD)
                {
                    bestD = d;
                    out = p;
                }
                break;
            }
        }
        return bestD >= 0.0f;
    }

    // Wave 33 (#218, leg E down): TravelNodeMap::getRoute(startPos, endPos, ...) (TravelNode.cpp, AC TravelNode.cpp
    // the same) ranks the 5 start / end node candidates by WorldPosition::fDist, a 2D distance. An elevator's two
    // landings share x/y (Thunder Bluff Mesa Elevator 4170: nodes 1867 z=130.1 and 1868 z=68.8), so they tie and the
    // pick is arbitrary: from the top landing the route started at the bottom one ("[ROUTE] A* found a 2-node route
    // ... startDist=61") and the bot walked toward a dock 61 yd below it until dock-walk-stall. AC's code stays as it
    // is (AC never follows these routes); the trip builds its route with the same search, AC's getFullPath steps in
    // AC's order, but with the candidates ranked by 3D distance on the position's own map.
    TravelPath BuildRoute(WorldPosition startPos, WorldPosition endPos, Player* bot)
    {
        std::vector<WorldPosition> beginPath = endPos.getPathFromPath({startPos}, nullptr, 40);
        if (endPos.isPathTo(beginPath))
            return TravelPath(beginPath);

        TravelNodeMap& nodeMap = TravelNodeMap::instance();
        std::shared_lock<std::shared_timed_mutex> guard(nodeMap.m_nMapMtx);
        std::vector<TravelNode*> const nodes = nodeMap.getNodes();

        auto rank = [&nodes](WorldPosition pos)
        {
            std::vector<std::pair<float, TravelNode*>> v;
            for (TravelNode* n : nodes)
                if (n && n->getMapId() == pos.GetMapId())
                    v.emplace_back(n->getPosition()->distance(pos), n);
            size_t const k = std::min<size_t>(5, v.size());
            std::partial_sort(v.begin(), v.begin() + k, v.end(),
                              [](std::pair<float, TravelNode*> const& a, std::pair<float, TravelNode*> const& b)
                              { return a.first < b.first; });
            v.resize(k);
            return v;
        };
        std::vector<std::pair<float, TravelNode*>> const starts = rank(startPos);
        std::vector<std::pair<float, TravelNode*>> const ends = rank(endPos);

        // AC's candidate order (TravelNode.cpp getRoute: endI++, then the next start once endI > startI + 1).
        size_t si = 0, ei = 0;
        while (si < starts.size() && ei < ends.size())
        {
            TravelNode* startNode = starts[si].second;
            TravelNode* endNode = ends[ei].second;
            // #847: roamTrip - a same-map destination is routed on its own map (w86 R4 came back ending on map 732) and
            // no route enters a dungeon that is not the destination's (#846 org-up: "leg-instance" via Ragefire Chasm).
            TravelNodeRoute route = nodeMap.getRoute(startNode, endNode, bot, true);
            if (!route.isEmpty())
            {
                WorldPosition startNodePos = *startNode->getPosition();
                std::vector<WorldPosition> toStart = beginPath;
                float const maxStartDistance = startNode->isTransport() ? 20.0f : sPlayerbotAIConfig->targetPosRecalcDistance;
                if (!startNodePos.cropPathTo(toStart, maxStartDistance))
                    toStart = startPos.getPathTo(startNodePos, nullptr);
                // #846: the route opens with a transport leg at a dock on another level than the bot: walk to the
                // dock's landing first (DockLandPoint).
                std::vector<TravelNode*> const& rn = route.getNodes();
                if (rn.size() > 1 && startNode->hasLinkTo(rn[1]) &&
                    startNode->getPathTo(rn[1])->getPathType() == TravelNodePathType::transport &&
                    std::fabs(startPos.GetPositionZ() - startNodePos.GetPositionZ()) > kDockHeight)
                {
                    WorldPosition land;
                    if (DockLandPoint(startNode, startPos, land))
                    {
                        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s dock-landing node (%.1f,%.1f,%.1f) via (%.1f,%.1f,%.1f)",
                                     bot->GetName().c_str(), startNodePos.GetPositionX(), startNodePos.GetPositionY(),
                                     startNodePos.GetPositionZ(), land.GetPositionX(), land.GetPositionY(),
                                     land.GetPositionZ());
                        toStart.push_back(land);
                    }
                }
                std::vector<WorldPosition> toEnd = route.getNodes().back()->getPosition()->getPathTo(endPos, nullptr);
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s route %u nodes start-node (%.1f,%.1f,%.1f) %.0f yd end-node %.0f yd",
                             bot->GetName().c_str(), uint32(route.getNodes().size()), startNodePos.GetPositionX(), startNodePos.GetPositionY(),
                             startNodePos.GetPositionZ(), starts[si].first, ends[ei].first);
                return route.buildPath(toStart, toEnd, bot);
            }
            ++ei;
            if (ei > si + 1)
            {
                ++si;
                ei = 0;
            }
        }
        return TravelPath();
    }
}  // namespace

namespace RoamTransport
{
    bool Enabled()
    {
        return CfgEnabled();
    }

    uint32 CampChancePct()
    {
        return CfgEnabled() ? CfgCampChance() : 0;
    }

    WorldPosition SelectCrossMapHub(Player* bot)
    {
        if (!bot || !CfgEnabled())
            return WorldPosition();

        std::vector<WorldLocation> const& hubs = sTravelMgr.GetTravelHubs(bot);
        if (hubs.empty())
            return WorldPosition();

        WorldPosition const botPos(bot);
        std::vector<uint32> const& botMaps = sPlayerbotAIConfig->randomBotMaps;

        // Sample a few hubs (random order) instead of scoring all of them: mapTransDistance walks the transfer list.
        std::vector<size_t> order(hubs.size());
        for (size_t i = 0; i < order.size(); ++i)
            order[i] = i;
        for (size_t i = order.size(); i > 1; --i)
            std::swap(order[i - 1], order[urand(0, uint32(i - 1))]);

        uint32 tried = 0;
        for (size_t oi : order)
        {
            WorldLocation const& loc = hubs[oi];
            if (loc.GetMapId() == bot->GetMapId())
                continue;
            if (!botMaps.empty() && std::find(botMaps.begin(), botMaps.end(), loc.GetMapId()) == botMaps.end())
                continue;
            if (!MapHasTravelNodes(bot->GetMapId()) || !MapHasTravelNodes(loc.GetMapId()))  // wave 30: no graph there
                continue;
            MapEntry const* me = sMapStore.LookupEntry(loc.GetMapId());
            if (!me || !me->IsContinent())
                continue;
            if (++tried > 8)
                break;
            WorldPosition const hub(loc);
            if (sTravelMgr.fastMapTransDistance(botPos, hub) >= 200000.0f)  // no map transfer in the travel graph
                continue;
            return hub;
        }
        return WorldPosition();
    }

    void Forget(Player* bot)
    {
        if (!bot)
            return;
        std::lock_guard<std::mutex> lock(g_tripMutex);
        auto itr = g_trips.find(bot->GetGUID());
        if (itr == g_trips.end())
            return;
        SetActive(itr->second, false);
        g_trips.erase(itr);
    }

    // Wave 33 (#218, leg E up): Thunder Bluff is a rest area, so PlayerbotAI::GetReactDelay (AC parity, dynamic
    // react delay) spaced Eleny's AI ticks 4-19 s apart (verbose "--- AI Tick ---" at t=95/106/124/128/147/153/158/
    // 162/174 s). The Mesa Elevator pauses 5 s at a landing (TransportAnimation 4170: 15000-20000 ms at -61.24) and
    // boarding takes two ticks inside that pause (step toward the deck, then attach): the step came at 162 s, the
    // attach check at 174 s, long after the platform had gone up. These phases get AC's base react delay.
    bool WantsFastReact(Player* bot)
    {
        if (!bot || g_activeTrips.load(std::memory_order_relaxed) <= 0)
            return false;
        RoamTrip* trip = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_tripMutex);
            auto itr = g_trips.find(bot->GetGUID());
            if (itr == g_trips.end() || !itr->second.active)
                return false;
            trip = &itr->second;  // own entry, this bot's map thread only (see the file header)
        }
        // Wave 41 (#218): end a trip that arrived or lost its status between two MoveFarTo calls.
        if (EndUndrivenTrip(*trip, bot))
            return false;
        TripPhase const p = trip->phase;
        // MoP DIVERGENCE (beyond AC; owner decision 2026-10-12, #218 wave 34): the Walk phase of an active trip gets
        // the base react delay too. AC's resting (city) bot ticks every 2-20 s; on the Thunder Bluff mesa Stanne
        // overshot route points between ticks and walked the 1870->1867 path back and forth for 5 min without ever
        // reaching the elevator. Bots without an active trip are not in g_trips as active and keep AC's tick rate.
        return p == TripPhase::Walk || p == TripPhase::DockWait || p == TripPhase::Board || p == TripPhase::Ride ||
               p == TripPhase::Leave;
    }

    bool ResumeAfterReset(Player* bot, WorldPosition& dest, bool& grind)
    {
        if (!bot || g_activeTrips.load(std::memory_order_relaxed) <= 0)
            return false;
        std::lock_guard<std::mutex> lock(g_tripMutex);
        auto itr = g_trips.find(bot->GetGUID());
        if (itr == g_trips.end() || !itr->second.active)
            return false;
        RoamTrip const& trip = itr->second;
        // Only a reset the trip itself caused: a map-changing ride, or the far teleport of a portal leg just taken.
        bool const riding = trip.phase == TripPhase::Ride || trip.phase == TripPhase::Leave;
        bool const portal = trip.portalMs && getMSTimeDiff(trip.portalMs, getMSTime()) < 10 * IN_MILLISECONDS;
        // #848 (w69: Stallian and Ancaranon rode Stormwind's Pride 190536 from Valiance Keep to Stormwind Harbor, but
        // both trips ended "abandon status-idle|status-rest phase=wait ... map=0"): PlayerbotAI's ambient deck check
        // (AC parity) attached them while they waited on the dock, and no RPG tick ran between that attach and the
        // ship's map change (BotActiveAlone throttling - "Bot X is on a transport" was their only line), so the trip
        // was still in DockWait when the far teleport reset the status. A bot that is a passenger of its next leg's
        // transport is on the trip's ride, whatever phase the trip last saw: RoamTransportMove then logs the board
        // (via=ambient) and rides it to the arrival dock (ride-arrived / leave).
        bool const aboard = trip.phase != TripPhase::Taxi && AboardLegIdx(trip.pts, trip.idx, bot) < trip.pts.size();
        if (!riding && !portal && !aboard)
            return false;
        dest = trip.dest;
        grind = trip.grind;
        return true;
    }

    // #218 (lane Z3, 2026-10-21): the zeppelin leg of an ordinary roaming bot timed out while the harness-driven
    // ones (always active, BotScenario::Driving) arrived. Flavelyn (build 88) boarded The Cloudkisser 181689 at
    // Vengeance Landing, rode it for 15 min and logged "abandon ride-timeout phase=ride" at the boarding spot
    // (1979.8,-6091.9,67.8): the zeppelin was docked at Vengeance Landing again, with her at the same deck offset.
    // Bapheri, an ordinary bot riding the same zeppelin in that window, got ride-arrived at Undercity; harness bots
    // Endak and Helah (t218-zeppelin) did both directions in 88 / 104 s. The Ride step that notices the far stop runs
    // from "new rpg go camp|go grind" (relevance 3), and PlayerbotAI::AllowActive (AC parity: BotActiveAlone 10, a
    // bot hash reshuffled every BotActiveAloneDurationSeconds = 30 s) leaves ~90% of lone bots in minimal ticks -
    // relevance >= 100 only, then PassiveDelay 10 s (PlayerbotAI.cpp DoNextAction). A 60 s zeppelin stop (TaxiPathNode
    // path 737 nodes 3 and 10, delay 60) spans two 30 s windows, so a rider catches it only ~19% of the time and
    // otherwise rides the loop back. WantsFastReact only shortens the react delay of a tick that runs at all.
    // True while the trip is on a transport leg (dock wait, boarding, riding, stepping off) or its bot is already a
    // passenger of the next leg's transport (the ambient attach, #848): PlayerbotAI::AllowActive keeps such a bot
    // active, as AC keeps a bot in a battleground / in combat active. BEYOND AC (AC has no roam trip). The walk and
    // taxi phases keep AC's throttling. Pure query (no EndUndrivenTrip side effect: AllowActive caches its answer).
    // AllowActive can run for this bot from a group member's thread (PARTY_ACTIVITY), so only the phase byte is read
    // here, never the route vector its own map thread may be rebuilding; a walking bot that is aboard any transport
    // counts (its next tick rides that leg, or drops the attachment).
    bool OnTransportLeg(Player* bot)
    {
        if (!bot || g_activeTrips.load(std::memory_order_relaxed) <= 0)
            return false;
        TripPhase phase = TripPhase::Walk;
        {
            std::lock_guard<std::mutex> lock(g_tripMutex);
            auto itr = g_trips.find(bot->GetGUID());
            if (itr == g_trips.end() || !itr->second.active)
                return false;
            phase = itr->second.phase;
        }
        switch (phase)
        {
            case TripPhase::DockWait:
            case TripPhase::Board:
            case TripPhase::Ride:
            case TripPhase::Leave:
                return true;
            case TripPhase::Walk:
                return bot->GetTransport() != nullptr;
            default:
                return false;
        }
    }

    // #890 (w90 Z3 Debora, Z4 Darcellis, t890-ramp-probe Galanie: "[WANDER] <bot> kind=qg entry=55382 ... tz=101.8"): AC's
    // GO_GRIND / GO_CAMP Execute first walks to any quest giver within 80 yd (SearchQuestGiverAndAcceptOrReward). Below
    // the Orgrimmar tower that is Gamon-like NPC 55382 on the upper level (z 101.8), reachable only by the elevator the
    // trip is waiting for: every such tick walked the bot up the ramp to (1744,-4335,55), the nearest navmesh point, and
    // the trip walked it back ("wait-again" 12-17 times). True while the bot's roam trip is at a transport leg
    // (dock wait, boarding, riding, stepping off) or walking its last kDockRadius + kDockRadiusSlack yards to one: the
    // quest-giver detour waits until the leg is done. BEYOND AC (AC has no roam trip; outside a trip nothing changes).
    // Called from the bot's own action (own map thread), so the route vector may be read.
    bool HoldsForTransportLeg(Player* bot)
    {
        if (!bot || g_activeTrips.load(std::memory_order_relaxed) <= 0)
            return false;
        RoamTrip* trip = nullptr;
        {
            std::lock_guard<std::mutex> lock(g_tripMutex);
            auto itr = g_trips.find(bot->GetGUID());
            if (itr == g_trips.end() || !itr->second.active)
                return false;
            trip = &itr->second;
        }
        switch (trip->phase)
        {
            case TripPhase::DockWait:
            case TripPhase::Board:
            case TripPhase::Ride:
            case TripPhase::Leave:
                return true;
            case TripPhase::Walk:
            {
                if (bot->GetTransport())
                    return true;
                for (size_t j = trip->idx; j + 1 < trip->pts.size(); ++j)
                {
                    if (!IsLegStart(trip->pts, j))
                        continue;
                    PathNodePoint const& leg = trip->pts[j];
                    return leg.type == NODE_TRANSPORT && leg.point.GetMapId() == bot->GetMapId() &&
                           Dist2d(bot, leg.point) <= kDockRadius + kDockRadiusSlack;
                }
                return false;
            }
            default:
                return false;
        }
    }

    // #889 (BEYOND AC: AC ignores static transports, ac_static_transport_expected): a type-11 platform that will not
    // stay where it is for the next 250 ms is between two landings. Same TransportAnimation test as DockWait / Board /
    // Leave (StaticPlatformStaysFor). Read-only on the transport; called from the rider's own map thread.
    bool StaticPlatformMoving(Transport* t)
    {
        return t && t->IsInWorld() && t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && !StaticPlatformStaysFor(t, 250);
    }
}  // namespace RoamTransport

int32 NewRpgBaseAction::RoamTransportMove(WorldPosition const& dest, bool mayStart, bool needSpecialLeg)
{
    if (!CfgEnabled() || dest == WorldPosition())
        return -1;
    if (!mayStart && g_activeTrips.load(std::memory_order_relaxed) <= 0)
        return -1;

    RoamTrip& trip = TripFor(bot);
    uint32 const now = getMSTime();

    auto abandon = [&](char const* why) -> int32
    {
        EndTrip(trip, bot, now, why);
        return -1;
    };

    // #847 (w87 R2): a trip walk that made no progress for kWalkStallMs gets AC's MoveFarTo stuck recovery once per
    // route point - the trip replaces MoveFarTo for this bot, so it keeps MoveFarTo's own fallback: "No meaningful
    // progress toward dest for stuckTime: teleport" (AC Ai/World/Rpg/Action/NewRpgBaseAction.cpp:107-124, MoP
    // NewRpgBaseAction.cpp:272-290), with the stalled walk's target (a route point) as the destination and AC's log
    // line, so the field counters see it as the stuck teleport it is. Omarano (w87 R2) was led by AC's quest-giver
    // detour (NewRpgGoGrindAction::Execute -> SearchQuestGiverAndAcceptOrReward, 80 yd) from the last walk point
    // before the Honeydew portal (2001.5,-4693.1) to the Hallow's End NPC Spoops (15309, (2064.5,-4672.0,25.5)),
    // whose spot is a 15-poly navmesh pocket (z 28, x 2037-2083, y -4686..-4648) with no link to the main mesh: every
    // path to the portal 32 yd away ended on the pocket rim, and 115 s later the trip gave up ("abandon
    // portal-walk-stall ... point=57/60", status IDLE). A second stall at the same point still abandons.
    auto stall = [&](char const* why, WorldPosition const& target) -> int32
    {
        if (trip.stuckTpIdx == trip.idx || target.GetMapId() != bot->GetMapId() || bot->IsInFlight() ||
            bot->GetTransport() || bot->GetVehicle() || bot->IsBeingTeleported())
            return abandon(why);
        trip.stuckTpIdx = trip.idx;
        trip.progressMs = trip.phaseMs = now;
        trip.bestDist = -1.0f;
        trip.portalMs = now ? now : 1;  // a teleport may reset the bot's RPG status: ResumeAfterReset resumes the trip
        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s stuck-teleport %s phase=%s point=%u/%u map=%u (%.1f,%.1f,%.1f) -> (%.1f,%.1f,%.1f) t=%us",
                     bot->GetName().c_str(), why, PhaseName(trip.phase), uint32(trip.idx), uint32(trip.pts.size()),
                     bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                     target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                     getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
        TC_LOG_DEBUG("playerbots",
                     "[New RPG] Teleport %s from (%f,%f,%f,%u) to (%f,%f,%f,%u) as it stuck when moving far - Zone: %u landz=%.1f fly=%d",
                     bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                     bot->GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                     target.GetMapId(), bot->GetZoneId(), target.GetPositionZ(), bot->IsFlying() ? 1 : 0);
        StopHere(bot);
        bot->RemoveAurasWithInterruptFlags(
            SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP));
        return bot->TeleportTo(target.GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                               bot->GetOrientation()) ? 1 : 0;
    };

    // A trip for another destination is stale (the status moved on). Wave 41: it was dropped without a line; end it
    // as an arrival when the bot stands at the old destination, else as an abandon that does not gate the old one.
    if (trip.active && trip.dest != dest)
        EndTrip(trip, bot, now, AtDest(bot, trip.dest) ? nullptr : "dest-changed", false);

    if (!trip.active)
    {
        if (!mayStart)
            return -1;
        // Wave 35 (#218): 27 of 50 cross-map camp picks ended here without a line (MoveFarTo then drops the
        // objective), so the census could not say why. A cross-map destination reaches this once per pick (MoveFarTo
        // goes back to IDLE on -1), so name the reason for those; same-map stuck-hook calls stay quiet.
        bool const xMapPick = dest.GetMapId() != bot->GetMapId();
        auto quietNoRoute = [&](char const* why) -> int32
        {
            if (xMapPick)
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s noroute %s map=%u (%.1f,%.1f,%.1f) -> map=%u (%.1f,%.1f,%.1f)",
                             bot->GetName().c_str(), why, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                             bot->GetPositionZ(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(),
                             dest.GetPositionZ());
            return -1;
        };
        if (trip.failAtMs && getMSTimeDiff(trip.failAtMs, now) < kRetryAfterFailMs && trip.dest == dest)
            return quietNoRoute("retry-gate");
        if (bot->IsInFlight() || bot->GetTransport() || bot->GetVehicle() || bot->IsBeingTeleported())
            return quietNoRoute("busy");
        if (!MapHasTravelNodes(bot->GetMapId()) || !MapHasTravelNodes(dest.GetMapId()))
        {
            trip.dest = dest;
            trip.failAtMs = now ? now : 1;
            return quietNoRoute("no-graph");
        }
        if (!TakeRouteBuildSlot())
            return needSpecialLeg ? -1 : 0;  // cross-map: wait for a slot instead of letting AC's code path to it

        WorldPosition const botPos(bot);
        TravelPath path = BuildRoute(botPos, dest, bot);
        std::vector<PathNodePoint> pts = path.getPath();
        uint32 tl = 0, fl = 0, pl = 0, hl = 0;
        size_t const special = CountLegs(pts, tl, fl, pl, hl);
        if (trip.dest != dest)
        {
            trip.hops = 0;
            trip.prevMap = 0;
        }
        trip.dest = dest;
        size_t start = 0;
        std::vector<WorldPosition> startPath;
        bool const crossMap = dest.GetMapId() != bot->GetMapId();
        // Every route must have a transport / flight / portal leg (a same-map walk is AC's MoveFarTo's job, a
        // cross-map destination cannot be walked), must not hearth to reach a same-map destination, and must be coherent.
        char const* why = pts.empty() ? "empty"
                        : !special ? "no-leg"
                        : (hl && dest.GetMapId() == bot->GetMapId()) ? "hearth"
                        : ValidateRoute(pts, bot, dest, start, crossMap && trip.hops < kMaxHops, trip.prevMap, startPath);
        if (why)
        {
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s noroute %s map=%u (%.1f,%.1f,%.1f) -> map=%u (%.1f,%.1f,%.1f) points=%u transport=%u flight=%u portal=%u hearth=%u first-map=%u",
                         bot->GetName().c_str(), why, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                         bot->GetPositionZ(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
                         uint32(pts.size()), tl, fl, pl, hl, pts.empty() ? 0u : uint32(pts.front().point.GetMapId()));
            trip.failAtMs = now ? now : 1;
            return -1;
        }

        // Wave 35 (#218): a far first route point was proven reachable on foot (StartPath). Its navmesh path replaces
        // the route's opening (the bot's own position up to that point), so the start leg is walked point by point with
        // the usual look-ahead, catch-up and stall guards instead of as one straight line to a node 100-600 yd away.
        uint32 const startPathPoints = uint32(startPath.size());
        if (!startPath.empty())
        {
            std::vector<PathNodePoint> spliced;
            spliced.reserve(startPath.size() + pts.size() - start);
            for (WorldPosition const& w : startPath)
                spliced.push_back(PathNodePoint{w, NODE_PREPATH, 0});
            spliced.insert(spliced.end(), pts.begin() + start, pts.end());
            pts = std::move(spliced);
            start = 0;
        }

        SetActive(trip, true);
        trip.pts = std::move(pts);
        trip.idx = start;
        trip.walkTarget = start;
        trip.phase = TripPhase::Walk;
        trip.startMs = trip.phaseMs = trip.progressMs = now;
        trip.bestDist = -1.0f;
        trip.legs = 0;
        trip.transportEntry = 0;
        trip.transportGuid = ObjectGuid::Empty;
        trip.boardMoveIssued = false;
        trip.lookupMs = 0;
        trip.portalMs = 0;
        trip.stuckTpIdx = size_t(-1);
        trip.routeStartMap = bot->GetMapId();
        trip.grind = botAI->GetRpgInfo().GetStatus() == RPG_GO_GRIND;
        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s start map=%u zone=%u (%.1f,%.1f,%.1f) -> map=%u (%.1f,%.1f,%.1f) points=%u transport=%u flight=%u portal=%u hop=%u end-map=%u startpath=%u",
                     bot->GetName().c_str(), bot->GetMapId(), bot->GetZoneId(), bot->GetPositionX(), bot->GetPositionY(),
                     bot->GetPositionZ(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
                     uint32(trip.pts.size()), tl, fl, pl, trip.hops, uint32(trip.pts.back().point.GetMapId()), startPathPoints);
    }

    // Wave 41 (#218): the status that drives this call owns the trip (EndUndrivenTrip ends it once that status is gone).
    {
        NewRpgInfo const& info = botAI->GetRpgInfo();
        trip.driverT = info.startT;
        trip.driverStatus = int32(info.GetStatus());
    }

    // ---- arrived? -------------------------------------------------------------------------------------------------
    if (AtDest(bot, dest))
    {
        EndTrip(trip, bot, now, nullptr);
        return -1;  // AC's MoveFarTo finishes the last yards
    }

    if (trip.idx >= trip.pts.size())
    {
        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s arrive route-end map=%u (%.1f,%.1f,%.1f) legs=%u t=%us", bot->GetName().c_str(),
                     bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), trip.legs,
                     getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
        SetActive(trip, false);
        trip.pts.clear();
        if (dest.GetMapId() == bot->GetMapId())
            return -1;
        // A route accepted as a hop (ValidateRoute allowHop) ends on an intermediate map: re-plan from here on the
        // next call (MoveFarTo passes mayStart for a cross-map destination), at most kMaxHops times per destination.
        if (bot->GetMapId() != trip.routeStartMap && trip.hops < kMaxHops)
        {
            ++trip.hops;
            trip.prevMap = trip.routeStartMap;
            trip.failAtMs = 0;
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s hop %u map %u -> %u, re-planning to map=%u", bot->GetName().c_str(),
                         trip.hops, trip.routeStartMap, bot->GetMapId(), dest.GetMapId());
            return 1;
        }
        trip.failAtMs = now ? now : 1;  // route ended on the wrong map: no rebuild loop, MoveFarTo drops the objective
        return -1;
    }

    if (bot->IsBeingTeleported())
        return 1;

    // #848: the ambient deck check may have seated the bot on its next leg's transport while the trip still walked to
    // the dock (Walk phase, idx on a walk point before the leg). Jump to that leg, so the transport branch below rides
    // it ("board ... via=ambient") instead of the walking branch dropping the bot off the deck as a leftover attachment.
    if (trip.phase == TripPhase::Walk || trip.phase == TripPhase::DockWait || trip.phase == TripPhase::Board)
    {
        size_t const legIdx = AboardLegIdx(trip.pts, trip.idx, bot);
        if (legIdx < trip.pts.size() && legIdx != trip.idx)
        {
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s aboard transport=%u point=%u->%u phase=%s map=%u (%.1f,%.1f,%.1f)",
                         bot->GetName().c_str(), trip.pts[legIdx].entry, uint32(trip.idx), uint32(legIdx),
                         PhaseName(trip.phase), bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                         bot->GetPositionZ());
            trip.idx = legIdx;
            trip.walkTarget = legIdx;
        }
    }

    // ---- a special leg starts at idx --------------------------------------------------------------------------------
    if (IsLegStart(trip.pts, trip.idx))
    {
        PathNodePoint const& from = trip.pts[trip.idx];
        PathNodePoint const& to = trip.pts[trip.idx + 1];

        // -- portal / hearthstone: step onto it, then teleport to its exit
        if (from.type == NODE_PORTAL || from.type == NODE_TELEPORT)
        {
            if (from.point.GetMapId() == bot->GetMapId() && !PortalPointReached(bot, from.point) && from.type == NODE_PORTAL)
            {
                if (getMSTimeDiff(trip.progressMs, now) > kWalkStallMs)
                    return stall("portal-walk-stall", from.point);
                return MoveTo(from.point.GetMapId(), from.point.GetPositionX(), from.point.GetPositionY(),
                              from.point.GetPositionZ(), false, false, false, true) ? 1 : 0;
            }
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s portal type=%s entry=%u map=%u -> map=%u (%.1f,%.1f,%.1f)",
                         bot->GetName().c_str(), from.type == NODE_PORTAL ? "portal" : "teleport", from.entry,
                         bot->GetMapId(), to.point.GetMapId(), to.point.GetPositionX(), to.point.GetPositionY(),
                         to.point.GetPositionZ());
            trip.idx += 2;
            trip.legs++;
            trip.phase = TripPhase::Walk;
            trip.progressMs = trip.phaseMs = now;
            trip.bestDist = -1.0f;
            trip.portalMs = now ? now : 1;  // a far teleport resets the bot's RPG status: ResumeAfterReset
            bot->RemoveAurasWithInterruptFlags(
                SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP));
            return bot->TeleportTo(to.point.GetMapId(), to.point.GetPositionX(), to.point.GetPositionY(),
                                   to.point.GetPositionZ(), bot->GetOrientation()) ? 1 : 0;
        }

        // -- flight path: walk to the flight master node, fly, resume at the arrival node
        if (from.type == NODE_FLIGHTPATH)
        {
            if (trip.phase == TripPhase::Taxi)
            {
                if (bot->IsInFlight())
                {
                    trip.flew = true;
                    trip.progressMs = now;
                    return 1;
                }
                if (getMSTimeDiff(trip.phaseMs, now) < 3 * IN_MILLISECONDS)
                    return 1;  // the flight spline has not started yet
                // landed (or never took off). Wave 36 (#218): Stanne flew Zim'Torga -> The Argent Stand (path 979),
                // landed, and his next MoveFarTo came ~2 min later after the RPG quest actions had walked him 70 yd
                // to the quest givers (accept 12507/12565, turn in 12507): "taxi-not-landed ... (5476.0,-2622.5)" 25 yd
                // from his destination. A flight that was seen in the air and is over has landed; only the 60 yd test
                // is left for a flight too short to be seen by any tick.
                if (to.point.GetMapId() == bot->GetMapId() && (trip.flew || Dist2d(bot, to.point) < 60.0f))
                {
                    TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s landed path=%u map=%u (%.1f,%.1f,%.1f)", bot->GetName().c_str(),
                                 from.entry, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                    trip.idx += 2;
                    trip.legs++;
                    trip.phase = TripPhase::Walk;
                    trip.progressMs = trip.phaseMs = now;
                    trip.bestDist = -1.0f;
                    return 1;
                }
                if (getMSTimeDiff(trip.phaseMs, now) > kTaxiStartMs)
                    return abandon("taxi-not-landed");
                return 1;
            }

            if (from.point.GetMapId() != bot->GetMapId())
                return abandon("taxi-wrong-map");
            // Wave 35 (#786): the trip activated the taxi from up to 10 yd (2 x INTERACTION_DISTANCE) of the travel
            // node, which is the flight master's spawn, with no flight master (npc = nullptr). Player::
            // ActivateTaxiPathTo (Player.cpp:23446-23459) measures 2 x INTERACTION_DISTANCE from the DBC TaxiNodes
            // position instead, and that lies 2.3-4.7 yd beyond the NPC. All 7 w35 "taxi-activate-failed" stood
            // 11.1-13.2 yd from it (Krasus' Landing node 310 x4, Zim'Torga 307, Undercity 11, Bouldercrag's Refuge
            // 327) and got ERR_TAXI_TOO_FAR_AWAY. With Walk fast react (w34) the tick now lands on the first step
            // inside 10 yd, so w35 exposed it.
            // AC's NewRpgTravelFlightAction::Execute (AC Ai/World/Rpg/Action/NewRpgAction.cpp:725-745, MoP
            // NewRpgAction.cpp:911-940) walks to INTERACTION_DISTANCE of the flight master, finds the creature, removes
            // the shapeshift, dismounts, learns the node from it (SendLearnNewTaxiNode) and activates with it. Do
            // exactly that. The creature is the live flight master nearest the node (the travel node has no entry).
            Creature* flightMaster = nullptr;
            {
                float bestD = 0.0f;
                for (ObjectGuid const& guid : AI_VALUE(GuidVector, "nearest npcs"))
                {
                    Unit* u = botAI->GetUnit(guid);
                    Creature* c = u ? u->ToCreature() : nullptr;
                    if (!c || !c->IsAlive() || !c->HasNpcFlag(UNIT_NPC_FLAG_FLIGHTMASTER) || c->IsHostileTo(bot))
                        continue;
                    float const d = c->GetExactDist(from.point.GetPositionX(), from.point.GetPositionY(),
                                                    from.point.GetPositionZ());
                    if (d > INTERACTION_DISTANCE * 3 || (flightMaster && d >= bestD))
                        continue;
                    flightMaster = c;
                    bestD = d;
                }
            }
            TaxiPathEntry const* tp = sTaxiPathStore.LookupEntry(from.entry);
            if (!tp)
                return abandon("taxi-no-path");
            TaxiNodesEntry const* tn = sTaxiNodesStore.LookupEntry(tp->from);
            float const reach = flightMaster ? bot->GetDistance(flightMaster)
                                             : bot->GetDistance(from.point.GetPositionX(), from.point.GetPositionY(),
                                                                from.point.GetPositionZ());
            // GetDistance(npc) subtracts both bounding radii, so "in reach" can still leave the bot ~11 yd from the DBC
            // node behind a large flight master: also stay inside the core's own 2 x INTERACTION_DISTANCE of it.
            bool const nodeFar = tn && tn->map_id == bot->GetMapId() &&
                                 bot->GetExactDist(tn->x, tn->y, tn->z) > INTERACTION_DISTANCE * 2 - 1.0f;
            if (reach > INTERACTION_DISTANCE || (flightMaster && nodeFar))
            {
                if (getMSTimeDiff(trip.progressMs, now) > kWalkStallMs)
                    return stall("taxi-walk-stall", from.point);  // the node is the flight master's spawn
                float const d = Dist3d(bot, from.point);
                if (trip.bestDist < 0.0f || d + 3.0f < trip.bestDist)
                {
                    trip.bestDist = d;
                    trip.progressMs = now;
                }
                if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
                    return 0;
                if (flightMaster)
                    return MoveTo(flightMaster->GetMapId(), flightMaster->GetPositionX(), flightMaster->GetPositionY(),
                                  flightMaster->GetPositionZ(), false, false, false, true) ? 1 : 0;
                return MoveTo(from.point.GetMapId(), from.point.GetPositionX(), from.point.GetPositionY(),
                              from.point.GetPositionZ(), false, false, false, true) ? 1 : 0;
            }
            if (!flightMaster)
                return abandon("taxi-no-flightmaster");  // AC: no live flight master at the spot -> give up

            botAI->RemoveShapeshift();
            if (bot->IsMounted())
                bot->Dismount();
            bot->GetSession()->SendLearnNewTaxiNode(flightMaster);
            std::vector<uint32> nodes = {tp->from, tp->to};
            if (!bot->ActivateTaxiPathTo(nodes, flightMaster, 0))
            {
                // The core only answers the client (SendActivateTaxiReply): log the usual suspects for the census.
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s taxi refused path=%u node %u -> %u npc=%u dbcNodeDist=%.1f mounted=%u combat=%u casting=%u money=%u cost=%u",
                             bot->GetName().c_str(), from.entry, tp->from, tp->to, flightMaster->GetEntry(),
                             tn ? bot->GetExactDist(tn->x, tn->y, tn->z) : -1.0f, bot->IsMounted() ? 1u : 0u,
                             bot->IsInCombat() ? 1u : 0u, bot->IsNonMeleeSpellCasted(false) ? 1u : 0u,
                             uint32(bot->GetMoney()), tp->price);
                return abandon("taxi-activate-failed");
            }
            TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s taxi path=%u node %u -> %u map=%u (%.1f,%.1f,%.1f)", bot->GetName().c_str(),
                         from.entry, tp->from, tp->to, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                         bot->GetPositionZ());
            trip.phase = TripPhase::Taxi;
            trip.flew = false;
            trip.phaseMs = trip.progressMs = now;
            return 1;
        }

        // -- transport (boat / zeppelin / elevator / tram)
        Transport* t = nullptr;
        if (!trip.transportGuid.IsEmpty())
            t = bot->GetMap() ? bot->GetMap()->GetTransport(trip.transportGuid) : nullptr;

        // PlayerbotAI's ambient check (AC parity) may already have attached the bot while it waited on the dock - its
        // 30 yd hull ray (#206 BotBesideMasterOffPlatform notes). Aboard the right transport is aboard: ride it.
        // #879 (w86): not in the Board phase - Board's own AddPassenger is seen on the next tick by Board itself
        // ("board ..." without via). Here it was logged "via=ambient" (Terido, Ratchet 20808, z 0.1), so the t879
        // evaluator's R5 counted the trip's own attach as an ambient one.
        if (Transport* on = bot->GetTransport())
        {
            if (on->GetEntry() == from.entry && (trip.phase == TripPhase::Walk || trip.phase == TripPhase::DockWait ||
                                                 (trip.phase == TripPhase::Board && on != t)))
            {
                trip.transportGuid = on->GetGUID();
                trip.transportEntry = from.entry;
                t = on;
                StopHere(bot);
                trip.phase = TripPhase::Ride;
                trip.phaseMs = trip.progressMs = now;
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s board transport=%u type=%u map=%u (%.1f,%.1f,%.1f) via=ambient",
                             bot->GetName().c_str(), on->GetEntry(), uint32(on->GetGoType()), bot->GetMapId(),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                return 1;
            }
        }

        switch (trip.phase)
        {
            case TripPhase::Walk:
            case TripPhase::DockWait:
            {
                if (from.point.GetMapId() != bot->GetMapId())
                    return abandon("dock-wrong-map");
                // Walk to the dock: the land point before the board node, or the board node itself.
                float const d2 = Dist2d(bot, from.point);
                // #890: a waiting bot leaves the wait only kDockRadiusSlack beyond the radius it entered at (hysteresis).
                float const dockRadius = trip.phase == TripPhase::DockWait ? kDockRadius + kDockRadiusSlack : kDockRadius;
                if (d2 > dockRadius || std::fabs(bot->GetPositionZ() - from.point.GetPositionZ()) > 12.0f)
                {
                    trip.phase = TripPhase::Walk;
                    WorldPosition target = from.point;
                    if (trip.idx > 0 && IsWalkPoint(trip.pts[trip.idx - 1]) &&
                        trip.pts[trip.idx - 1].point.GetMapId() == bot->GetMapId())
                        target = trip.pts[trip.idx - 1].point;
                    float const d = Dist3d(bot, target);
                    if (trip.bestDist < 0.0f || d + 3.0f < trip.bestDist)
                    {
                        trip.bestDist = d;
                        trip.progressMs = now;
                    }
                    if (getMSTimeDiff(trip.progressMs, now) > kWalkStallMs)
                        // #847: only to the land point before the dock (the board node itself is the deck / the
                        // ship's origin at its stop, in the shaft or on the water while the transport is away)
                        return target != from.point ? stall("dock-walk-stall", target) : abandon("dock-walk-stall");
                    if (d < kPointReached)
                    {
                        // At the last land point but still outside the dock radius (a long pier): head for the node.
                        target = from.point;
                    }
                    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
                        return 0;
                    return MoveTo(target.GetMapId(), target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(),
                                  false, false, false, true) ? 1 : 0;
                }

                if (trip.phase != TripPhase::DockWait)
                {
                    // #890 (w88 Z3 Eraimo, 206610: "wait" 15+ times at the same spot): every re-entry restarted the 12 min
                    // timer, so dock-wait-timeout never fired. A re-entry for the same leg of the same trip keeps the
                    // first entry's start, its wait-why bits and its one dock approach, and logs "wait-again".
                    bool const again = trip.dockWaitStartMs && trip.dockWaitIdx == trip.idx && trip.dockWaitTripMs == trip.startMs;
                    trip.phase = TripPhase::DockWait;
                    trip.phaseMs = now;
                    // #890 (w90 Z3 Debora / Z4 Darcellis, "wait-again n=1..17"): a re-entry no longer restarts the
                    // dock-walk-stall clock either, so a bot that keeps being pulled off its dock gets the trip's
                    // one stuck teleport to the land point 120 s after its last real progress (it boarded right
                    // after that teleport in w88 and w90).
                    if (!again)
                        trip.progressMs = now;
                    trip.transportEntry = from.entry;
                    StopHere(bot);
                    if (!again)
                    {
                        trip.dockWaitStartMs = now ? now : 1;
                        trip.dockWaitIdx = trip.idx;
                        trip.dockWaitTripMs = trip.startMs;
                        trip.dockWaitAgain = 0;
                        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s wait transport=%u map=%u (%.1f,%.1f,%.1f)", bot->GetName().c_str(),
                                     from.entry, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                        trip.dockApproached = false;
                        trip.levelApproached = false;
                        trip.waitWhy = 0;
                    }
                    else
                    {
                        ++trip.dockWaitAgain;
                        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s wait-again n=%u transport=%u map=%u (%.1f,%.1f,%.1f) waited=%us",
                                     bot->GetName().c_str(), trip.dockWaitAgain, from.entry, bot->GetMapId(), bot->GetPositionX(),
                                     bot->GetPositionY(), bot->GetPositionZ(), getMSTimeDiff(trip.dockWaitStartMs, now) / IN_MILLISECONDS);
                    }
                }
                if (getMSTimeDiff(trip.dockWaitStartMs ? trip.dockWaitStartMs : trip.phaseMs, now) > kDockWaitMs)
                    return abandon("dock-wait-timeout");

                // Wave 51 (#218): w50's two dock waits (Blanuel 867 s, Ophele 650+ s, Telredor 183177) and w47's
                // Charlest (720 s, Scaffold Cars 149045) ended without one line saying why. Log each reason the wait
                // goes on for once per wait: not-found, stopped-elsewhere (the platform is paused / docked but not at
                // this stop: node vs platform position), no-deck-point, stop-too-short (the step-on walk does not fit).
                auto waitWhy = [&](uint8 bit, char const* why, Transport* tr, uint32 needMs, uint32 leftMs)
                {
                    if (trip.waitWhy & bit)
                        return;
                    trip.waitWhy |= bit;
                    TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s wait-why %s transport=%u type=%u bot=(%.1f,%.1f,%.1f) node=(%.1f,%.1f,%.1f) platform=(%.1f,%.1f,%.1f) d3=%.1f dz=%.1f need=%ums left=%ums t=%us",
                                 bot->GetName().c_str(), why, from.entry, tr ? uint32(tr->GetGoType()) : 0u, bot->GetPositionX(),
                                 bot->GetPositionY(), bot->GetPositionZ(), from.point.GetPositionX(), from.point.GetPositionY(),
                                 from.point.GetPositionZ(), tr ? tr->GetPositionX() : 0.0f, tr ? tr->GetPositionY() : 0.0f,
                                 tr ? tr->GetPositionZ() : 0.0f,
                                 tr ? tr->GetExactDist(from.point.GetPositionX(), from.point.GetPositionY(), from.point.GetPositionZ()) : -1.0f,
                                 tr ? tr->GetPositionZ() - from.point.GetPositionZ() : 0.0f, needMs, leftMs,
                                 getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
                };

                // Look the transport up every ~2 s (object-store walk).
                if (!t || !t->IsInWorld())
                {
                    if (trip.lookupMs && getMSTimeDiff(trip.lookupMs, now) < 2 * IN_MILLISECONDS)
                        return 1;
                    trip.lookupMs = now ? now : 1;
                    t = FindTransport(bot, from.entry, from.point);
                    trip.transportGuid = t ? t->GetGUID() : ObjectGuid::Empty;
                    if (!t)
                        waitWhy(1, "not-found", nullptr, 0, 0);
                }
                if (!t || !AtStop(t, from.point))
                {
                    // Paused near this dock (MO: docked within 200 yd; static: paused on this level) but not "at stop".
                    if (t && (t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT
                                  ? MoDockedNow(t) && t->GetExactDist2d(from.point.GetPositionX(), from.point.GetPositionY()) < 200.0f
                                  : std::fabs(t->GetPositionZ() - from.point.GetPositionZ()) <= kDockHeight && StaticPlatformStaysFor(t, 250)))
                        waitWhy(2, "stopped-elsewhere", t, 0, 0);
                    return 1;  // keep waiting on the dock
                }

                float dx = 0.0f, dy = 0.0f, dz = 0.0f, ex = 0.0f, ey = 0.0f;
                if (!FindDeckPoint(t, bot, dx, dy, dz, ex, ey))
                {
                    waitWhy(4, "no-deck-point", t, 0, 0);
                    // #890 (w88 Z3 Eraimo at (1738.7,-4367.9,39.6), 34 yd from 206610 on the ramp below its landing,
                    // platform z 45.5): FindDeckPoint only looks 2.5 yd above the bot's feet and down, so a static deck
                    // above a bot that waits on a lower level is never found and the bot waited for nothing. While the
                    // platform is at this stop, walk once (navmesh) to a standby point on its landing: kDeckFootprint +
                    // kStandbyFromEdge from the platform origin toward the bot, on ground within 4 yd of the platform's
                    // level, dry and not over the deck. The next ticks find the deck point from there.
                    if (t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && !trip.levelApproached)
                    {
                        float const px = t->GetPositionX(), py = t->GetPositionY(), pz = t->GetPositionZ();
                        float const toPlatform = bot->GetExactDist2d(px, py);
                        float const reach = kDeckFootprint + kStandbyFromEdge;
                        if (toPlatform > reach + 1.0f)
                        {
                            float const sx = px + (bot->GetPositionX() - px) / toPlatform * reach;
                            float const sy = py + (bot->GetPositionY() - py) / toPlatform * reach;
                            float const gz = bot->GetMap()->GetHeight(bot->GetPhaseMask(), sx, sy, pz + 3.0f, true, 8.0f);
                            float deckZ = 0.0f;
                            bool const ok = gz > INVALID_HEIGHT && std::fabs(gz - pz) <= 4.0f &&
                                            !bot->GetMap()->IsInWater(bot->GetPhaseMask(), sx, sy, gz) &&
                                            !DeckZAt(t, sx, sy, gz + 2.0f, 4.0f, deckZ);
                            // w90: the walk was logged "go" but MoveTo can refuse it (the last move's wait, a duplicate
                            // target, PathIsWalkable); the one try per wait is only spent once it really started.
                            bool moved = false;
                            if (ok)
                            {
                                if (bot->IsSitState())
                                    bot->SetStandState(UNIT_STAND_STATE_STAND);
                                moved = MoveTo(bot->GetMapId(), sx, sy, gz, false, false, false, true);
                            }
                            if (!ok || moved)
                            {
                                trip.levelApproached = true;
                                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s dock-approach-level %s transport=%u from=(%.1f,%.1f,%.1f) to=(%.1f,%.1f,%.1f) platform=(%.1f,%.1f,%.1f)",
                                             bot->GetName().c_str(), ok ? "go" : "no-ground", from.entry, bot->GetPositionX(),
                                             bot->GetPositionY(), bot->GetPositionZ(), sx, sy, gz, px, py, pz);
                            }
                        }
                    }
                    return 1;
                }
                bool const isStatic = t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT;
                // Step on only if the walk fits in what is left of the stop (an elevator pauses ~4-5 s: never walk
                // into an open shaft, #589). Board dismounts a bot for a static platform, so time that walk on foot
                // (wave 33: the check used the 14 yd/s mount speed, the walk then ran at 7).
                // #848: a static platform is timed to its edge (StaticStepMs), a boat / zeppelin as before.
                float walkSpeed = std::max(2.5f, bot->GetSpeed(MOVE_RUN));
                if (bot->IsMounted() && isStatic)
                    walkSpeed = std::min(walkSpeed, playerBaseMoveSpeed[MOVE_RUN]);
                uint32 const needMs = isStatic ? StaticStepMs(bot, bot->GetExactDist2d(ex, ey))
                                               : uint32(bot->GetExactDist2d(dx, dy) / walkSpeed * IN_MILLISECONDS) + 1500;
                if (!StaysFor(t, needMs))
                {
                    waitWhy(8, "stop-too-short", t, needMs, t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT ? MoDockLeftMs(t) : 0u);
                    // Wave 51 (#218): the wait starts at the first point inside kDockRadius (35 yd), and a static stop is
                    // short (Scaffold Cars 149045: 3.2-3.6 s, TransportAnimation 185198-185241; Telredor 5 s), so a walk
                    // of 30 yd (5.8 s with the margin, Charlest w47) never fits. While the platform is here, walk once
                    // to a standby point on the dock kStandbyFromDeck short of the deck point (2.5 yd short of the deck
                    // edge): on ground level with the bot, dry, and not on the deck (so not in the shaft later).
                    // #848: the standby point is kStandbyFromEdge short of the deck's edge (was 5.5 yd short of the deck
                    // point = 2.5 yd short of the edge); from there the 152614 step-on needs ~0.7 s of its 1.33 s pause.
                    float const toDeck = bot->GetExactDist2d(dx, dy);
                    float const toEdge = bot->GetExactDist2d(ex, ey);
                    if (isStatic && !trip.dockApproached && toDeck > 0.5f && toEdge > kStandbyFromEdge + 1.0f)
                    {
                        float const ux = (dx - bot->GetPositionX()) / toDeck, uy = (dy - bot->GetPositionY()) / toDeck;
                        float const back = toDeck - toEdge + kStandbyFromEdge;  // standby distance short of the deck point
                        float const sx = dx - ux * back, sy = dy - uy * back;
                        float const gz = bot->GetMap()->GetHeight(bot->GetPhaseMask(), sx, sy, bot->GetPositionZ() + 3.0f, true, 8.0f);
                        float deckZ = 0.0f;
                        bool const ok = std::fabs(gz - bot->GetPositionZ()) <= 3.0f &&
                                        !bot->GetMap()->IsInWater(bot->GetPhaseMask(), sx, sy, gz) &&
                                        !DeckZAt(t, sx, sy, gz + 2.0f, 4.0f, deckZ);
                        trip.dockApproached = true;  // once per wait, whatever the outcome
                        TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s dock-approach %s transport=%u from=(%.1f,%.1f,%.1f) to=(%.1f,%.1f,%.1f) deck=%.1f yd need=%ums",
                                     bot->GetName().c_str(), ok ? "go" : "no-ground", from.entry, bot->GetPositionX(),
                                     bot->GetPositionY(), bot->GetPositionZ(), sx, sy, gz, toDeck, needMs);
                        if (ok)
                        {
                            if (bot->IsSitState())
                                bot->SetStandState(UNIT_STAND_STATE_STAND);
                            MoveTo(bot->GetMapId(), sx, sy, gz, false, false, false, true);
                        }
                    }
                    return 1;
                }
                trip.boardX = dx;
                trip.boardY = dy;
                trip.boardZ = dz;
                trip.edgeX = ex;
                trip.edgeY = ey;
                trip.dockX = bot->GetPositionX();
                trip.dockY = bot->GetPositionY();
                trip.dockZ = bot->GetPositionZ();
                trip.boardMoveIssued = false;
                trip.phase = TripPhase::Board;
                trip.phaseMs = now;
                [[fallthrough]];
            }
            case TripPhase::Board:
            {
                if (!t || !t->IsInWorld())
                {
                    trip.phase = TripPhase::DockWait;
                    return 1;
                }
                if (bot->GetTransport() == t)
                {
                    StopHere(bot);
                    trip.phase = TripPhase::Ride;
                    trip.phaseMs = trip.progressMs = now;
                    TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s board transport=%u type=%u map=%u (%.1f,%.1f,%.1f)", bot->GetName().c_str(),
                                 t->GetEntry(), uint32(t->GetGoType()), bot->GetMapId(), bot->GetPositionX(),
                                 bot->GetPositionY(), bot->GetPositionZ());
                    return 1;
                }
                // It left (or we could not get on): back to the dock and wait for the next one. A bot that had
                // already walked over a static platform's footprint would stand in the open shaft (wave 33: Eleny at
                // (-1286.7,189.0,66.2) under the departed Mesa Elevator), so it steps back to where it started.
                auto backToDock = [&]()
                {
                    StopHere(bot);
                    trip.phase = TripPhase::DockWait;
                    trip.progressMs = now;
                    if (t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && trip.boardMoveIssued &&
                        bot->GetExactDist2d(trip.dockX, trip.dockY) > 1.0f)
                        bot->GetMotionMaster()->MovePoint(0, trip.dockX, trip.dockY, trip.dockZ, false);
                    trip.boardMoveIssued = false;
                    return 1;
                };
                bool const atStop = AtStop(t, from.point);
                if (!atStop || getMSTimeDiff(trip.phaseMs, now) > kBoardMs)
                    return backToDock();
                bool const staticDeck = t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT;
                // #848: a static platform is left in time when the rest of the walk to its edge fits (DockWait's own
                // test); the flat 800 ms sent a bot back from a 1.33 s stop it was about to make.
                uint32 const stayMs = staticDeck ? StaticStepMs(bot, bot->GetExactDist2d(trip.edgeX, trip.edgeY)) : 800;
                if (!DeckUnder(t, bot) && !StaysFor(t, stayMs))
                    return backToDock();  // it is leaving and we are still on the dock: stay off, wait for the next one
                float const toDeck = bot->GetExactDist2d(trip.boardX, trip.boardY);
                // #848: on a static platform the attach happens as soon as the deck is under the bot (at its edge).
                if (toDeck < 1.5f || (staticDeck && DeckUnder(t, bot)) ||
                    (trip.boardMoveIssued && !bot->isMoving() && DeckUnder(t, bot)))
                {
                    if (DeckUnder(t, bot))
                    {
                        // #223 P2: stop BEFORE boarding, then attach with the offset of where the bot stands.
                        StopHere(bot);
                        t->AddPassenger(bot, true);
                        return 1;  // next tick sees GetTransport() == t
                    }
                }
                if (!trip.boardMoveIssued || !bot->isMoving())
                {
                    if (bot->IsSitState())
                        bot->SetStandState(UNIT_STAND_STATE_STAND);
                    // #218 (w90 Z4 Darcellis hovered at the zeppelin dock with "fly,mounted" and flew down into the hull):
                    // a flying mount is left before any transport, a ground mount only before a static platform.
                    if (bot->IsMounted() && (t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT || bot->IsFlying() || bot->CanFly()))
                        bot->Dismount();
                    // Straight line (no navmesh on a deck) - the S9 boarding assist (FollowActions.cpp #206 B2).
                    bot->GetMotionMaster()->MovePoint(0, trip.boardX, trip.boardY, trip.boardZ, false);
                    trip.boardMoveIssued = true;
                }
                return 1;
            }
            case TripPhase::Ride:
            {
                Transport* riding = bot->GetTransport();
                if (!riding)
                {
                    // Fell off / was removed. If the deck is still right here (docked), try once more, else re-route.
                    if (t && t->IsInWorld() && AtStop(t, from.point))
                    {
                        trip.phase = TripPhase::DockWait;
                        return 1;
                    }
                    return abandon("ride-lost-transport");
                }
                if (bot->isMoving() && !bot->movespline->Finalized())
                    StopHere(bot);
                // Arrived at the far stop?
                bool arrived = false;
                if (riding->GetMapId() == to.point.GetMapId())
                {
                    if (riding->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT)
                        arrived = MoDockedNow(riding) &&
                                  riding->GetExactDist2d(to.point.GetPositionX(), to.point.GetPositionY()) < 30.0f;
                    else
                        arrived = AtStop(riding, to.point);  // wave 51: the node may not be the platform origin
                }
                // #218 (lane Z3): the timeout is tested after the arrival, never before it. t218-zeppelin w88 UNDRIVEN Z6:
                // Flaos rode the Iron Eagle 175080 Grom'gol -> Orgrimmar -> Grom'gol -> Orgrimmar and logged "abandon
                // ride-timeout ... map=1 (1872.6,-4424.4,135.0)" on a tick when the zeppelin WAS docked at the arrival stop.
                if (!arrived && getMSTimeDiff(trip.phaseMs, now) > kRideMs)
                    return abandon("ride-timeout");
                if (!arrived)
                    return 1;
                PickLeavePoint(trip, bot, riding, trip.leaveX, trip.leaveY, trip.leaveZ);
                {
                    float const walkSpeed = std::max(2.5f, bot->GetSpeed(MOVE_RUN));
                    float const offDist = std::min(12.0f, bot->GetExactDist2d(trip.leaveX, trip.leaveY));
                    // #848: a static platform only has to be walked off (to where Leave detaches the rider), timed like
                    // the step-on; the 12 yd + 1 s budget never fit the 1.33 s stops of the Borean elevator 152614.
                    uint32 const offMs = riding->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT
                                             ? StaticStepMs(bot, DeckExitDist(riding, bot, trip.leaveX, trip.leaveY))
                                             : uint32(offDist / walkSpeed * IN_MILLISECONDS) + 1000;
                    if (!StaysFor(riding, offMs))
                        return 1;  // not enough of the stop left: ride on and step off at the next stop here
                }
                trip.phase = TripPhase::Leave;
                trip.phaseMs = now;
                TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s ride-arrived transport=%u map=%u (%.1f,%.1f,%.1f) leave->(%.1f,%.1f,%.1f) t=%us",
                             bot->GetName().c_str(), riding->GetEntry(), bot->GetMapId(), bot->GetPositionX(),
                             bot->GetPositionY(), bot->GetPositionZ(), trip.leaveX, trip.leaveY, trip.leaveZ,
                             getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
                bot->GetMotionMaster()->MovePoint(0, trip.leaveX, trip.leaveY, trip.leaveZ, false);
                return 1;
            }
            case TripPhase::Leave:
            {
                Transport* riding = bot->GetTransport();
                // Wave 34 (#218, leg E up): only a platform still at the arrival stop can be walked off. Stanne stood
                // on the Mesa Elevator when it left the top landing; the deck ray lost it as it went down and he was
                // detached in mid-air at z 128.4 ("leave ... (-1286.2,189.7,128.4)"), then pathed onto the shaft
                // roof at z 159.0. A rider whose platform has left stays aboard and rides the next round.
                if (riding && !AtStop(riding, to.point))
                {
                    TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s leave-missed transport=%u map=%u (%.1f,%.1f,%.1f) t=%us",
                                 bot->GetName().c_str(), riding->GetEntry(), bot->GetMapId(), bot->GetPositionX(),
                                 bot->GetPositionY(), bot->GetPositionZ(), getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
                    trip.phase = TripPhase::Ride;
                    trip.phaseMs = now;
                    StopHere(bot);
                    return 1;
                }
                // Off the deck = no deck under the bot's x/y. For a static platform look further down (12 yd): a bot
                // still over a platform that has started to sink is carried, not off (wave 34, the same Stanne case).
                float deckZ = 0.0f;
                if (riding && !DeckZAt(riding, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ() + 2.0f,
                                       riding->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT ? kDockHeight : 4.0f, deckZ))
                {
                    // Off the deck: detach now (a masterless rider is otherwise re-seated by the ambient check, #586).
                    bool const wasMo = riding->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
                    StopHere(bot);
                    riding->RemovePassenger(bot, true);
                    if (wasMo)
                        bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
                    bot->GetMotionMaster()->MovePoint(0, trip.leaveX, trip.leaveY, trip.leaveZ, true);
                    riding = nullptr;
                }
                if (!riding)
                {
                    TC_LOG_DEBUG("playerbots", "[ROAMTRIP] %s leave transport=%u map=%u (%.1f,%.1f,%.1f) t=%us", bot->GetName().c_str(),
                                 trip.transportEntry, bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(),
                                 bot->GetPositionZ(), getMSTimeDiff(trip.startMs, now) / IN_MILLISECONDS);
                    // Wave 34 (#218): the points that follow on the arrival deck are behind the bot now (the UP
                    // route's only one was the shaft centre (-1286.2,189.7,130.1)): skip them, never walk back on.
                    WorldPosition const arrPos = to.point;
                    trip.idx += 2;
                    while (trip.idx < trip.pts.size() && !IsLegStart(trip.pts, trip.idx) &&
                           OnArrivalDeck(trip.pts[trip.idx].point, arrPos))
                        ++trip.idx;
                    trip.walkTarget = trip.idx;
                    trip.legs++;
                    trip.phase = TripPhase::Walk;
                    trip.transportGuid = ObjectGuid::Empty;
                    trip.progressMs = trip.phaseMs = now;
                    trip.bestDist = -1.0f;
                    return 1;
                }
                if (getMSTimeDiff(trip.phaseMs, now) > kLeaveMs)
                {
                    // Could not walk off before it left again: stay aboard, ride the next round.
                    trip.phase = TripPhase::Ride;
                    trip.phaseMs = now;
                    StopHere(bot);
                    return 1;
                }
                if (!bot->isMoving())
                    bot->GetMotionMaster()->MovePoint(0, trip.leaveX, trip.leaveY, trip.leaveZ, false);
                return 1;
            }
            default:
                break;
        }
        return 1;
    }

    // ---- walking leg ------------------------------------------------------------------------------------------------
    trip.phase = TripPhase::Walk;
    if (Transport* on = bot->GetTransport())
    {
        // Walking, so not riding: a leftover attachment (the ambient check re-seating a rider that just stepped off,
        // #316) is dropped here, as AC's ambient check drops it.
        bool const wasMo = on->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
        StopHere(bot);
        on->RemovePassenger(bot, true);
        if (wasMo)
            bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        return 1;
    }

    // Wave 34 (#218, leg E down): catch up with the look-ahead. A resting bot ticks only every 2-20 s (AC's
    // GetReactDelay) and MoveTo sends it up to kWalkLookAhead yards ahead, so it walked past route point 1 between
    // ticks and never stood within kPointReached of it at a tick. Every later look-ahead was anchored at that stale
    // point and sent Stanne back along the path (stops at path points 8, 7, 6, 1, 9, 7, 2 over 5 min, never "wait").
    // The bot has passed everything up to the point of the last look-ahead window it is now closest to.
    if (trip.walkTarget > trip.idx && trip.walkTarget < trip.pts.size())
    {
        size_t best = trip.idx;
        float bestD = Dist3d(bot, trip.pts[trip.idx].point);
        for (size_t j = trip.idx + 1; j <= trip.walkTarget; ++j)
        {
            if (IsLegStart(trip.pts, j) || !IsWalkPoint(trip.pts[j]) || trip.pts[j].point.GetMapId() != bot->GetMapId())
                break;
            float const dj = Dist3d(bot, trip.pts[j].point);
            if (dj < bestD)
            {
                bestD = dj;
                best = j;
            }
        }
        if (best > trip.idx)
        {
            trip.idx = best;
            trip.progressMs = now;
            trip.bestDist = -1.0f;
        }
    }

    // Skip reached points (wave 34: on the bot's own level, PointReached). Wave 51: the walk point right before a
    // portal leg is the portal node itself, an area-trigger centre up in the air (PortalPointReached).
    while (trip.idx < trip.pts.size() && !IsLegStart(trip.pts, trip.idx) &&
           trip.pts[trip.idx].point.GetMapId() == bot->GetMapId() &&
           (PointReached(bot, trip.pts[trip.idx].point) ||
            (trip.idx + 1 < trip.pts.size() && IsLegStart(trip.pts, trip.idx + 1) &&
             (trip.pts[trip.idx + 1].type == NODE_PORTAL || trip.pts[trip.idx + 1].type == NODE_TELEPORT) &&
             PortalPointReached(bot, trip.pts[trip.idx].point))))
    {
        ++trip.idx;
        trip.progressMs = now;
        trip.bestDist = -1.0f;
    }
    if (trip.idx >= trip.pts.size() || IsLegStart(trip.pts, trip.idx))
        return 1;  // the next tick handles the route end / the leg

    WorldPosition const& cur = trip.pts[trip.idx].point;
    if (cur.GetMapId() != bot->GetMapId())
        return abandon("walk-wrong-map");
    // Wave 35: a validated start walk may begin up to kStartPathMax yards from its end (a partial navmesh path).
    if (Dist3d(bot, cur) > kStartPathMax)
        return abandon("off-route");

    // Look ahead along consecutive walk points on this map, up to kWalkLookAhead yards of path.
    size_t target = trip.idx;
    float acc = Dist3d(bot, cur);
    for (size_t j = trip.idx + 1; j < trip.pts.size(); ++j)
    {
        if (IsLegStart(trip.pts, j) || !IsWalkPoint(trip.pts[j]) || trip.pts[j].point.GetMapId() != bot->GetMapId())
            break;
        acc += trip.pts[j - 1].point.distance(trip.pts[j].point);
        if (acc > kWalkLookAhead)
            break;
        target = j;
    }
    WorldPosition const& tp = trip.pts[target].point;
    float const d = Dist3d(bot, tp);
    if (trip.bestDist < 0.0f || d + 3.0f < trip.bestDist)
    {
        trip.bestDist = d;
        trip.progressMs = now;
    }
    if (getMSTimeDiff(trip.progressMs, now) > kWalkStallMs)
        return stall("walk-stall", tp);

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return 0;
    if (MoveTo(tp.GetMapId(), tp.GetPositionX(), tp.GetPositionY(), tp.GetPositionZ(), false, false, false, true))
    {
        trip.walkTarget = target;
        return 1;
    }
    trip.walkTarget = trip.idx;
    // Wave 30 walk-stall: the look-ahead point can be unreachable from here (graph paths are AC-era geometry); retry
    // with the next route point before the stall timer gives the trip up.
    if (target != trip.idx)
        return MoveTo(cur.GetMapId(), cur.GetPositionX(), cur.GetPositionY(), cur.GetPositionZ(), false, false, false, true) ? 1 : 0;
    return 0;
}
