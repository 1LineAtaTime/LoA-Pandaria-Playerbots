#include "FollowActions.h"
#include "FishingAction.h"  // #647 kFishingDistanceFromMaster

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <cstdio>
#include <mutex>
#include <unordered_map>

#include "Event.h"
#include "Formations.h"
#include "GameObjectModel.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Transport.h"
#include "TransportMgr.h"

namespace
{
    // #206/#209 F3 (owner requirement, beyond AC): a static transport (elevator/tram) only counts as
    // under a unit when its floor is within ~2 yd below the unit's feet (a 4 yd ray). The core's
    // GetTransportForPos ray reaches ~30 yd down, which attached a bot standing in an empty shaft with
    // the gap as its transport offset (s9-elevator-boat-rootcause.md).
    bool IsUnderStaticTransportDeck(Transport* t, float x, float y, float z)
    {
        if (!t || !t->m_model)
            return false;
        G3D::Vector3 const v(x, y, z + 2.0f);
        G3D::Ray const r(v, G3D::Vector3(0.0f, 0.0f, -1.0f));
        float dist = 4.0f;
        return t->m_model->intersectRay(r, dist, false, t->GetPhaseMask(), VMAP::ModelIgnoreFlags::Nothing);
    }

    // #587: only a moving (MO) transport created by Transport::Create has keyframes. A static/local
    // transport (elevator/tram, Transport::CreateLocal) never sets _transportInfo, so its
    // GetKeyFrames() dereferences nullptr - and its GetTransportPeriod() is the animation's TotalTime,
    // i.e. non-zero, so a period check alone does not keep it out. Every keyframe walk in this file
    // goes through this gate. (AC never reaches this case: its FollowAction drops static transports
    // before any keyframe use, AC FollowActions.cpp "Ignore static transports".)
    bool HasKeyFrames(Transport* t)
    {
        return t && t->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT && t->GetTransportTemplate();
    }

    // #206 B1/B2 (owner requirement, beyond AC): is this MO transport (boat/zeppelin) docked right now?
    // Same keyframe test as the harness's DockedAt (BotScenario.cpp); Transport::IsMoving() is useless
    // here because it stays true for the whole route.
    bool IsDockedNow(Transport* t)
    {
        if (!HasKeyFrames(t) || !t->GetTransportPeriod())
            return false;
        uint32 const tm = t->GetTimer() % t->GetTransportPeriod();
        for (KeyFrame const& k : t->GetKeyFrames())
            if (tm >= k.ArriveTime && tm < k.DepartureTime)
                return true;
        return false;
    }

    // #589 (owner requirement #209, beyond AC): a static (type-11) platform's animation-space position at
    // path time t. The same frame lookup and lerp Transport::Update runs (Transport.cpp:357-375), minus the
    // constant origin and parent rotation, which cannot change whether two instants are the same spot.
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

    // #589: does this static platform stay where it is for at least `ms` more? An elevator pauses only
    // ~4-5 s at a landing (Thunder Bluff 4170: top 0..~4800 of a 30033 ms period), so a bot may step onto
    // it, or off it, only when that fits in what is left of the pause; otherwise it is left standing in an
    // open shaft. Pure function of the path timer for an unstoppable transport (pause=0, every entry seen so
    // far); a stoppable one parked at its stop (Transport.cpp:305-326) stays until its GO state flips, which
    // counts as staying. Unknown animation data keeps the pre-#589 rule (z gap only).
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

    // #209 (owner requirement, beyond AC): a static (type-11) transport whose path moves it sideways - the
    // Deeprun Tram cars (MoP 218203-218208 / AC 176080-176085, ~2480 yd along y) - rather than up/down a
    // fixed shaft like an elevator. The elevator rules below do not fit it: its origin is not its floor
    // (218204: deck 11.31 yd below the origin, so origin z never came within the 8 yd landing tolerance of a
    // bot on the platform), and GetStationaryX/Y() is only its first station. Animation-space XY extent of
    // the path; an elevator's is ~0.
    bool IsHorizontalStaticTransport(Transport* t)
    {
        if (!t || t->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
            return false;
        TransportAnimation const* anim = t->GetGOValue()->Transport.AnimationInfo;
        if (!anim || anim->Path.empty())
            return false;
        float minX = 0.0f, maxX = 0.0f, minY = 0.0f, maxY = 0.0f;
        bool first = true;
        for (auto const& node : anim->Path)
        {
            TransportAnimationEntry const* e = node.second;
            if (!e)
                continue;
            if (first)
            {
                minX = maxX = e->X;
                minY = maxY = e->Y;
                first = false;
                continue;
            }
            minX = std::min(minX, e->X);
            maxX = std::max(maxX, e->X);
            minY = std::min(minY, e->Y);
            maxY = std::max(maxY, e->Y);
        }
        return !first && (maxX - minX > 20.0f || maxY - minY > 20.0f);
    }

    // #209: the boarding-assist radius of FollowAction::Execute below; a tram car only counts as "at the bot's
    // station" while its rider-master is within it.
    float const kStaticBoardingAssistDistance = 60.0f;

    // #209 (w21b): a tram rider steps off at the station its master is at. That station test used the 60 yd
    // boarding radius, but the Stormwind platform exit (68.0,2490.9) is 63.5 yd from the car centre and up to
    // 66.7 yd from a rider at the deck's far side (s9wo-mop-tram-w21b-0749: Carius x=1.3, Aleneya x=3.7), so a
    // rider whose master waits there would hold on the car at every dock and ride on. The two stations are ~2480 yd
    // apart, so a wide radius still tells them apart; the level and dwell tests stay as they were.
    float const kTramStationRadius = 250.0f;

    // #589: time the walk onto/off a static platform needs on top of the walk itself: the next follow
    // tick that boards (~0.2 s) or the 1 s ambient check that detaches, plus slack.
    uint32 const kStaticStepMarginMs = 1000;
    // #589 round 2: boarding only needs the next follow tick once the bot is on the deck (FOLLOW314 lines
    // come every ~200 ms), so its margin is smaller than the step-off one, which waits on the 1 s ambient
    // check (#586).
    uint32 const kStaticBoardMarginMs = 600;

    // #589 round 2: 2D distance from the bot to the nearest deck point of a static platform, walking from the
    // platform's centre toward the bot until the deck ray (IsUnderStaticTransportDeck) misses - the same
    // walk FindBoardingPointOnTransport does from the master. Round 1 used the distance to the centre, which
    // overstated the walk by a whole deck radius and refused a reachable window (s9wo-mop-elev-w10-0903:
    // static-notparked at timer 3173 for Carius 7.2 yd from the centre, ~3.4 yd from the deck, with ~1.6 s
    // of the pause left). Falls back to the centre distance when no deck point is found.
    // #209: levelZ is the deck height to probe at - the platform's own z for an elevator (origin = floor), the
    // rider-master's feet for a tram car (origin 11.31 yd above its floor).
    float StaticDeckDist2d(Transport* t, Player* bot, float levelZ)
    {
        float const cx = t->GetPositionX();
        float const cy = t->GetPositionY();
        float const bx = bot->GetPositionX();
        float const by = bot->GetPositionY();
        float const d = std::sqrt((bx - cx) * (bx - cx) + (by - cy) * (by - cy));
        if (d < 0.5f)
            return d;
        float const pz = std::max(bot->GetPositionZ(), levelZ);
        int32 const steps = std::clamp(static_cast<int32>(d / 0.5f), 1, 60);
        float edge = -1.0f;
        for (int32 i = 0; i <= steps; ++i)
        {
            float const f = static_cast<float>(i) / static_cast<float>(steps);
            if (!IsUnderStaticTransportDeck(t, cx + (bx - cx) * f, cy + (by - cy) * f, pz))
                break;
            edge = d * f;
        }
        return edge < 0.0f ? d : d - edge;
    }

    // #589: the static transport each bot last saw its master on. After the master walks off at another
    // landing, FollowAction no longer resolves any transport for it, and the plain follow's target is
    // across the open shaft: the path from the landing edge degenerates and the spline goes straight
    // down (s9wo-mop-elev-w9-0649: Carius z 130.3 -> 69.7 in 9 s at run speed, mgen=8). Remembering the
    // elevator lets the bot wait for it and ride it instead. Same per-bot file-local map + mutex pattern
    // as AI/PlayerbotAI.cpp g_nextTransportCheck (no header change); bounded by the bot roster.
    struct MasterStaticTransportMemo
    {
        ObjectGuid transport;
        uint32 msTime = 0;
    };
    std::unordered_map<ObjectGuid::LowType, MasterStaticTransportMemo> g_masterStaticTransport;
    std::mutex g_masterStaticTransportMutex;
    uint32 const kStaticLinkMemoMs = 5 * MINUTE * IN_MILLISECONDS;

    void RememberMasterStaticTransport(Player* bot, Transport* t)
    {
        std::lock_guard<std::mutex> lock(g_masterStaticTransportMutex);
        MasterStaticTransportMemo& m = g_masterStaticTransport[bot->GetGUID().GetCounter()];
        m.transport = t->GetGUID();
        m.msTime = getMSTime();
    }

    // #589: the elevator that links this bot to a master who has just left it at another landing: the
    // one the master was last seen on, still in world on the bot's map, the bot not aboard it, both
    // near its shaft (bot <= 15 yd, master <= 40 yd: inside the 60 yd boarding assist radius below) and
    // on different levels (> 8 yd, the landing tolerance the static branch uses).
    Transport* LinkedStaticTransport(Player* bot, Player* master, Map* map)
    {
        ObjectGuid guid;
        {
            std::lock_guard<std::mutex> lock(g_masterStaticTransportMutex);
            auto const it = g_masterStaticTransport.find(bot->GetGUID().GetCounter());
            if (it == g_masterStaticTransport.end() || getMSTimeDiff(it->second.msTime, getMSTime()) > kStaticLinkMemoMs)
                return nullptr;
            guid = it->second.transport;
        }
        Transport* t = map ? map->GetTransport(guid) : nullptr;
        if (!t || !t->IsInWorld() || t->GetMap() != bot->GetMap() || t->GetGoType() != GAMEOBJECT_TYPE_TRANSPORT)
            return nullptr;
        // #209: elevators only - a tram car has no shaft to wait at (its stations are ~2480 yd apart).
        if (IsHorizontalStaticTransport(t))
            return nullptr;
        if (bot->GetTransport() == t)
            return nullptr;
        if (std::fabs(bot->GetPositionZ() - master->GetPositionZ()) <= 8.0f)
            return nullptr;
        float const sx = t->GetStationaryX();
        float const sy = t->GetStationaryY();
        if (bot->GetExactDist2d(sx, sy) > 15.0f || master->GetExactDist2d(sx, sy) > 40.0f)
            return nullptr;
        return t;
    }

    // #179 (AC FollowActions.cpp:23-96, ported almost verbatim - only the core calls differ, noted
    // inline). Probes a few Z offsets, matching AC's tolerance for the ~2 yd the raycast in
    // Map::GetTransportForPos starts above the query point.
    Transport* GetTransportForPosTolerant(Map* map, WorldObject* ref, uint32 phaseMask, float x, float y, float z)
    {
        if (!map || !ref)
            return nullptr;

        std::array<float, 4> const probes = { z, z + 0.5f, z + 1.5f, z - 0.5f };
        for (float const pz : probes)
        {
            Transport* t = map->GetTransportForPos(phaseMask, x, y, pz, ref);
            // #141 F3 parity (crash hazard): reject anything that is not a real Mo_Transport-guided
            // Transport - see AI/PlayerbotAI.cpp's ambient transport check for the same guard.
            if (t && !t->GetGUID().IsMOTransport())
                t = nullptr;
            if (t && t->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && !IsUnderStaticTransportDeck(t, x, y, pz))
                t = nullptr;
            if (t)
                return t;
        }

        return nullptr;
    }

    // Attempts to find a point on the leader's transport that is closer to the bot, by probing along
    // the segment from master -> bot and returning the last point that is still detected as being on
    // the expected transport.
    bool FindBoardingPointOnTransport(Map* map, Transport* expectedTransport, WorldObject* ref,
        float masterX, float masterY, float masterZ,
        float botX, float botY, float botZ,
        float& outX, float& outY, float& outZ)
    {
        if (!map || !expectedTransport || !ref)
            return false;

        uint32 const phaseMask = ref->GetPhaseMask();

        // Ensure master is actually detected on that transport (tolerant).
        if (GetTransportForPosTolerant(map, ref, phaseMask, masterX, masterY, masterZ) != expectedTransport)
            return false;

        // The raycast in GetTransportForPos starts at (z + 2). Probe with a safe Z.
        float const probeZ = std::max(masterZ, botZ);

        // Adaptive step count: small platforms need tighter sampling.
        float const dx2 = botX - masterX;
        float const dy2 = botY - masterY;
        float const dist2d = std::sqrt(dx2 * dx2 + dy2 * dy2);
        int32 const steps = std::clamp(static_cast<int32>(dist2d / 0.75f), 10, 28);

        float const dx = (botX - masterX) / static_cast<float>(steps);
        float const dy = (botY - masterY) / static_cast<float>(steps);

        float lastX = masterX;
        float lastY = masterY;
        bool found = false;

        for (int32 i = 1; i <= steps; ++i)
        {
            float const px = masterX + dx * i;
            float const py = masterY + dy * i;

            Transport* const t = GetTransportForPosTolerant(map, ref, phaseMask, px, py, probeZ);
            if (t != expectedTransport)
                break;

            lastX = px;
            lastY = py;
            found = true;
        }

        if (!found)
            return false;

        outX = lastX;
        outY = lastY;
        outZ = masterZ; // keep deck-level Z to encourage stepping onto the platform/boat
        return true;
    }

    // AC's Unit::StopMovingOnCurrentPos() (AC Unit.cpp) does not exist on this core; this mirrors
    // the .cpp-local stand-in already used by the ambient transport check (AI/PlayerbotAI.cpp's
    // StopOnCurrentPos, #141 F5): a zero-length spline in the bot's current frame, so a board shows
    // up to observers even when the previous spline was already finalized.
    void StopOnCurrentPos(Player* bot)
    {
        bot->ClearUnitState(UNIT_STATE_MOVING);
        if (!bot->IsInWorld())
            return;

        Movement::MoveSplineInit init(bot);
        init.MoveTo(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), false, true);
        init.SetFacing(bot->GetOrientation());
        init.Launch();
    }

    // IsOverWater: is the TERRAIN under (x,y) submerged? outGroundZ receives the sampled ground Z.
    bool IsOverWater(Map* map, uint32 phaseMask, float x, float y, float z, float& outGroundZ)
    {
        outGroundZ = INVALID_HEIGHT;
        if (!map)
            return false;
        float const groundZ = map->GetHeight(x, y, z, true, DEFAULT_HEIGHT_SEARCH);
        if (groundZ <= INVALID_HEIGHT)
            return false;
        outGroundZ = groundZ;
        return map->IsUnderWater(phaseMask, x, y, groundZ);
    }

    // FindDryLandWaitPoint: pick a dry-land point near the transport's nearest stop node where the bot can
    // wait for the boat to return. Returns false when nothing usable was found.
    bool FindDryLandWaitPoint(Transport* t, Map* map, uint32 phaseMask, float botX, float botY, float botZ,
                              float& outX, float& outY, float& outZ)
    {
        if (!HasKeyFrames(t) || !map) // #587
            return false;

        // Nearest STOP frame (DepartureTime > ArriveTime - the same stop test the core's own
        // IsStopFrame reads off the taxi node flags, but taken from the frame timings so a stop
        // that was authored without the flag is still found).
        KeyFrame const* nearest = nullptr;
        float nearestDist2 = 0.0f;
        for (KeyFrame const& k : t->GetKeyFrames())
        {
            if (k.DepartureTime <= k.ArriveTime || !k.Node)
                continue;
            float const dx = k.Node->LocX - botX;
            float const dy = k.Node->LocY - botY;
            float const d2 = dx * dx + dy * dy;
            if (!nearest || d2 < nearestDist2)
            {
                nearest = &k;
                nearestDist2 = d2;
            }
        }

        if (!nearest)
            return false;

        // Spiral outward from the stop node (fixed loop, no RNG): the first candidate whose sampled
        // ground is real and NOT underwater is the closest dry patch to where the boat stops.
        float const radii[] = { 0.0f, 4.0f, 8.0f, 12.0f, 18.0f, 25.0f, 35.0f };
        float const nodeX = nearest->Node->LocX;
        float const nodeY = nearest->Node->LocY;
        float const k45 = 0.70710678f;
        for (float const r : radii)
        {
            for (uint32 b = 0; b < 8; ++b)
            {
                // 8 bearings, 45 degrees apart, computed without trig so the probe cost is trivial.
                float const ux = (b == 0 ? 1.0f : b == 1 ? k45 : b == 2 ? 0.0f : b == 3 ? -k45 :
                                  b == 4 ? -1.0f : b == 5 ? -k45 : b == 6 ? 0.0f : k45);
                float const uy = (b == 0 ? 0.0f : b == 1 ? k45 : b == 2 ? 1.0f : b == 3 ? k45 :
                                  b == 4 ? 0.0f : b == 5 ? -k45 : b == 6 ? -1.0f : -k45);
                float const x = nodeX + ux * r;
                float const y = nodeY + uy * r;

                float groundZ = INVALID_HEIGHT;
                if (IsOverWater(map, phaseMask, x, y, botZ, groundZ))
                    continue;
                if (groundZ <= INVALID_HEIGHT)
                    continue;

                outX = x;
                outY = y;
                outZ = groundZ;
                return true;
            }
        }

        return false;
    }
}

bool FollowAction::Execute(Event event)
{
    Formation* formation = AI_VALUE(Formation*, "formation");
    std::string const target = formation->GetTargetName();

    // #179 (AC FollowActions.cpp:21-206, spec section 3 "follow onto a moving transport"): board a
    // boat/zeppelin the master is riding, or move to meet it. AC explicitly excludes static
    // transports (elevators/trams) here ("only keep boats/zeppelins") and only rides them via the
    // ambient per-tick transport check (#141, AI/PlayerbotAI.cpp). #209 (owner requirement, beyond
    // AC) extends this fork's own copy of the block below to also actively approach/board/wait for
    // static transports - see the #209 comment just below.
    Player* master = botAI->GetMaster();
    // #587 hardening (beyond AC, which tests the map id only): also require the SAME Map instance, so the
    // transport lookups below never read another instance's Map (and its transports) from this bot's
    // map-update thread. Every same-instance case behaves exactly as before.
    if (master && master->IsInWorld() && bot->IsInWorld() && bot->GetMapId() == master->GetMapId() &&
        master->GetMap() == bot->GetMap())
    {
        Map* map = master->GetMap();
        uint32 const mapId = bot->GetMapId();
        Transport* transport = nullptr;
        bool masterOnTransport = false;

        if (master->GetTransport())
        {
            transport = master->GetTransport();
            masterOnTransport = true;
        }
        else if (map)
        {
            transport = GetTransportForPosTolerant(map, master, master->GetPhaseMask(),
                master->GetPositionX(), master->GetPositionY(), master->GetPositionZ());
            masterOnTransport = (transport != nullptr);
        }

        // #589 (owner requirement #209, beyond AC): the master left an elevator at another landing. Keep
        // driving the static branches below for that elevator (masterOnTransport stays false), so the bot
        // waits at its landing and rides the next cycle, instead of the plain follow down/up the shaft.
        if (!transport && map)
            transport = LinkedStaticTransport(bot, master, map);

        // #209 (owner requirement, beyond AC): AC ignores static transports (elevators/trams)
        // here and only rides them through the ambient per-tick check (AI/PlayerbotAI.cpp,
        // #141); this fork additionally drives an active approach/board/wait cycle for them, so
        // a grouped bot follows the master onto an elevator, not just a boat/zeppelin. The
        // #141 F3 guard inside GetTransportForPosTolerant (IsMOTransport(), a GUID-type check) is
        // unrelated to this and stays as-is: every real Transport object - moving (MO) or local
        // (static/elevator) - is created with a HighGuid::Mo_Transport guid
        // (Transport.cpp:74 CreateLocal, :211 Create), so that guard already lets static
        // transports through; it only rejects a plain type-11 GameObject that was never actually
        // instantiated as a Transport (e.g. GameObject.cpp:230-233's entry-216666 case reached via
        // GetTransportForPos loop 2's FindNearestGameObjectOfType -> GameObject::ToTransport
        // reinterpret_cast), which would be unsafe to call Transport methods on. Ported to AC
        // identically (its own equivalent guard sits in the same GetTransportForPosTolerant here).
        // #587 hardening (defensive; not the #587 crash itself): a transport is only usable while it is in
        // world on the bot's own Map - an MO transport changes Map on its continent hop. Done BEFORE the
        // type flags below, so a dropped transport can never leave isMoTransport/isStaticTransport set.
        if (transport && (!transport->IsInWorld() || transport->GetMap() != bot->GetMap()))
        {
            transport = nullptr;
            masterOnTransport = false;
        }
        bool const isMoTransport = transport && transport->GetGoType() == GAMEOBJECT_TYPE_MO_TRANSPORT;
        bool const isStaticTransport = transport && transport->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT;
        if (transport && !isMoTransport && !isStaticTransport)
            transport = nullptr;
        if (isStaticTransport && masterOnTransport)
            RememberMasterStaticTransport(bot, transport); // #589

        bool const dockedNow = isMoTransport && IsDockedNow(transport);

        // ---- #314 TEMPORARY MEASUREMENT INSTRUMENTATION - NOT PRODUCT CODE, REMOVE AFTER THE RUN ----
        // One line per follow-assist tick for the case under test: the assist's own IsDockedNow
        // answer, the branch this assist actually took, and the bot's own position/flags/motion - so
        // that a movement into the water can be attributed to this assist or to something else. Cost is
        // bounded: the lambda only runs on a tick that ends in a transport branch (i.e. the master is on
        // a transport), and it writes to the playerbots debug logger the neighbouring action files
        // already use (e.g. strategy/actions/CheckMountStateAction.cpp:339).
        // #587: the per-keyframe `frames=` dump (DockedAtProbe/DockedFramesProbe) is removed - it walked
        // GetKeyFrames() on static transports too, whose keyframe template is nullptr (crash #587).
        // #316 QA B1: gate the probe's own reads on the log level it writes to, so live bots pay nothing
        // while the "playerbots" category is off.
        bool const probeEnabled = sLog->ShouldLog("playerbots", LOG_LEVEL_DEBUG);
        uint32 const probePeriod = (probeEnabled && transport) ? transport->GetTransportPeriod() : 0u;
        uint32 const probeTimer = probePeriod ? (transport->GetTimer() % probePeriod) : 0u;
        // #316: the move target is passed IN rather than captured - destX/destY/destZ are locals of the
        // boarding-distance block below, so a lambda defined here cannot see them. `hasTarget` (2.0f is
        // not a real target for this probe: the deck/pier targets are always well outside it) gates the
        // three fields, so the emitted dstx/dsty/dstz are only meaningful when hasTarget=1.
        // #316: `dist2d` is passed IN rather than captured - it is a local of the flag block at
        // :378 (declared before the `if (inAssist)`), so a lambda defined earlier cannot see it.
        // -1.0f is the sentinel for "not measured on this tick", so a reader can never mistake a
        // branch that has no distance for one measured at zero.
        auto assistProbe = [&](char const* branch, bool hasTarget = false,
                               float tx = -2.0f, float ty = -2.0f, float tz = -2.0f,
                               float dist2d = -1.0f)
        {
            int mgen = 19; // NULL_MOTION_TYPE normalisation, matching the harness trace (BotScenario.cpp:3294-3298)
            if (bot->GetMotionMaster())
            {
                mgen = int(bot->GetMotionMaster()->GetCurrentMovementGeneratorType());
                if (mgen == NULL_MOTION_TYPE)
                    mgen = 19;
                else if (mgen == SPLINE_CHAIN_MOTION_TYPE || mgen == FACE_MOTION_TYPE)
                    mgen += 100;
            }
            TC_LOG_DEBUG("playerbots",
                "[FOLLOW314] %s branch=%s tguid=%u entry=%u mo=%d static=%d mOnT=%d botT=%u "
                "dockednow=%d timer=%u period=%u x=%.1f y=%.1f z=%.1f flags=%u moving=%d mgen=%d "
                "hasTarget=%d dstx=%.1f dsty=%.1f dstz=%.1f dist2d=%.1f",
                bot->GetName().c_str(), branch,
                transport ? (uint32)transport->GetGUID().GetCounter() : 0u,
                transport ? transport->GetEntry() : 0u,
                isMoTransport ? 1 : 0, isStaticTransport ? 1 : 0, masterOnTransport ? 1 : 0,
                bot->GetTransport() ? (uint32)bot->GetTransport()->GetGUID().GetCounter() : 0u,
                dockedNow ? 1 : 0, probeTimer, probePeriod,
                bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                bot->GetUnitFlags(), bot->isMoving() ? 1 : 0, mgen,
                hasTarget ? 1 : 0, tx, ty, tz, dist2d);
        };
        // ---- end #314 temporary measurement instrumentation ----

        // #206 B1 (owner requirement, beyond AC): the master is aboard a boat/zeppelin that has left the
        // dock without the bot - hold at the dock instead of chasing it over water or off the navmesh
        // (s9-elevator-boat-rootcause.md, boat). The bot boards on the next docked visit.
        if (isMoTransport && masterOnTransport && bot->GetTransport() != transport && !dockedNow)
        {
            float waitX = 0.0f, waitY = 0.0f, waitZ = 0.0f;
            if (FindDryLandWaitPoint(transport, map, bot->GetPhaseMask(),
                    bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), waitX, waitY, waitZ))
            {
                float const dx = bot->GetPositionX() - waitX;
                float const dy = bot->GetPositionY() - waitY;
                float const dz = bot->GetPositionZ() - waitZ;
                bool const atWait = (dx * dx + dy * dy + dz * dz) <= (3.0f * 3.0f);
                if (atWait)
                {
                    if (bot->isMoving())
                        bot->StopMoving();
                    assistProbe("dock-wait");
                    return true;
                }
                if (MotionMaster* mm = bot->GetMotionMaster())
                    mm->MovePoint(0, waitX, waitY, waitZ, /*generatePath*/ true);
                assistProbe("dock-approach", true, waitX, waitY, waitZ);
                ClearIdleState();
                return true;
            }

            // No dry land derivable: do NOT fall through to a follow whose destination is a hull in open
            // water. Hold here instead, unless the bot is already in water (then it can still swim).
            assistProbe("dock-nodryland");
            if (!bot->IsInWater())
            {
                if (bot->isMoving())
                    bot->StopMoving();
                assistProbe("dock-waterguard", true, master->GetPositionX(), master->GetPositionY(), master->GetPositionZ());
                return true;
            }
        }

        // A static transport (elevator/tram) cycles continuously - every entry seen so far has
        // pause=0, so unlike a boat's docked stop there is no window where "wait here, it will
        // come to you" is safe for an arbitrary vertical gap. Only approach/board it while its
        // platform is near the bot's own current Z (its "landing"); otherwise hold at the shaft's
        // fixed XY (GetStationaryX/Y() - an elevator does not move horizontally), at the bot's own
        // Z, instead of chasing the master's position on a platform the bot cannot path to (that
        // produced the "unreachable follow" spin observed in #206 S9 round 5).
        if (isStaticTransport && bot->GetTransport() != transport)
        {
            float const landingGapTolerance = 8.0f; // matches the tolerance s9-elev-run.sh polls with
            // #589: "near the bot's Z" is not enough - the platform also has to stay parked there long
            // enough for the walk onto it (round 2: the bot's 2D distance to the deck at run speed, plus
            // kStaticBoardMarginMs). While it is still arriving, or leaves too soon, the bot holds
            // and rides the next cycle instead of stepping into the shaft as the platform drops away
            // (s9wo-mop-elev-w9-0649: assist-move at timer 6654, ~2 s after the ~4.8 s top pause ended). A
            // bot already standing on the deck boards regardless (the board check below).
            // #209: a tram car (IsHorizontalStaticTransport) is "at the bot's landing" when the bot is level with
            // the master standing on its deck and within the boarding-assist radius of it - not by origin z (the
            // car's origin is 11.31 yd above its floor, so the elevator test never held at the station and held
            // at random tunnel points instead: static-notparked at dist2d 2231 in s9wo-mop-tram-w13r-1039).
            bool const tram = IsHorizontalStaticTransport(transport);
            float const levelZ = (tram && masterOnTransport) ? master->GetPositionZ() : transport->GetPositionZ();
            bool const atLanding = std::fabs(levelZ - bot->GetPositionZ()) <= landingGapTolerance &&
                (!tram || (masterOnTransport &&
                    sServerFacade->GetDistance2d(bot, master) <= kStaticBoardingAssistDistance));
            bool const onDeck = IsUnderStaticTransportDeck(transport, bot->GetPositionX(), bot->GetPositionY(),
                bot->GetPositionZ());
            // The deck walk only matters (and only costs its rays) while the platform is at this landing.
            float const deckDist = (atLanding && !onDeck) ? StaticDeckDist2d(transport, bot, levelZ) : -1.0f;
            uint32 const boardNeedMs = uint32(1000.0f * std::max(deckDist, 0.0f) /
                std::max(bot->GetSpeed(MOVE_RUN), 1.0f)) + kStaticBoardMarginMs;
            if (!onDeck && (!atLanding || !StaticPlatformStaysFor(transport, boardNeedMs)))
            {
                // #209: a tram has no shaft to hold beside - GetStationaryX/Y() is its first station only, and the
                // elevator hold walked the bots ~12 yd from the car centre, one of them off the platform edge
                // (Aleneya z -6.9). Hold where the bot stands; it boards when the car is back with the master on it.
                if (tram)
                {
                    if (bot->isMoving())
                        bot->StopMoving();
                    assistProbe(atLanding ? "tram-notparked" : "tram-hold", false, -2.0f, -2.0f, -2.0f, deckDist);
                    return true;
                }
                // Hold on the bot's own landing, a few yards short of the shaft, with pathfinding:
                // the shaft itself is open air at whichever landing the platform has left.
                float const holdDist = 10.0f;
                float const shaftX = transport->GetStationaryX();
                float const shaftY = transport->GetStationaryY();
                float const dx = bot->GetPositionX() - shaftX;
                float const dy = bot->GetPositionY() - shaftY;
                float const d = std::sqrt(dx * dx + dy * dy);
                if (d <= holdDist + 2.0f)
                {
                    if (bot->isMoving())
                        bot->StopMoving();
                    // #314 temporary probe (#589 split; round 2 logs the deck distance in dist2d when parked here)
                    assistProbe(atLanding ? "static-notparked" : "static-hold", false, -2.0f, -2.0f, -2.0f, deckDist);
                    return true;
                }
                float const waitX = shaftX + dx / d * holdDist;
                float const waitY = shaftY + dy / d * holdDist;
                float const waitZ = bot->GetPositionZ();

                if (IsMovingAllowed() && !IsDuplicateMove(mapId, waitX, waitY, waitZ))
                {
                    if (bot->IsSitState())
                        bot->SetStandState(UNIT_STAND_STATE_STAND);

                    if (MotionMaster* mm = bot->GetMotionMaster())
                        mm->MovePoint(0, waitX, waitY, waitZ, true);

                    ClearIdleState();
                }
                assistProbe("static-wait"); // #314 temporary probe
                return true;
            }
        }

        // #206/#209 F2 (owner requirement, beyond AC): a bot riding an elevator/tram the master is not on
        // keeps IGNORE_PATHFINDING (F1), so a follow move would be a straight line through the air - hold
        // until the platform is level with the master.
        // #589: ...and parked there long enough to step off it (at most one deck radius, ~8 yd, toward the
        // master, plus kStaticStepMarginMs for the 1 s ambient check that detaches the rider, #586). "Level"
        // alone released the bot up to 8 yd above the master's landing while the platform still moved.
        if (Transport* botTransport = bot->GetTransport())
        {
            // #209: a tram rider's level is its own feet (the car origin is 11.31 yd above the deck, which held
            // a rider on the car at every station), and it only steps off at the station its master is at.
            bool const botTram = IsHorizontalStaticTransport(botTransport);
            float stepOffDist = std::min(sServerFacade->GetDistance2d(bot, master), 8.0f);
            // #209 w29: an elevator rider only has to clear the DECK EDGE toward the master, not reach the master.
            // Charging the whole bot-master distance (up to 8 yd = 2.1 s at run speed + 1 s margin) held a rider that
            // stood 1-2 yd from the edge once less than ~2.1 s of the ~5.3 s bottom pause was left, and it rode back
            // up (s9wo-mop-elev-w29-2211 Carius, master off at timer ~17100; AC has no hold and both AC bots stepped off
            // the same pause, s9wo-ac-elev-w29-2211). Same deck ray as StaticDeckDist2d, walked from the bot.
            if (!botTram && botTransport->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && stepOffDist > 0.5f)
            {
                float const bx = bot->GetPositionX();
                float const by = bot->GetPositionY();
                float const ddx = master->GetPositionX() - bx;
                float const ddy = master->GetPositionY() - by;
                float const dd = std::sqrt(ddx * ddx + ddy * ddy);
                if (dd > 0.5f)
                {
                    float edge = stepOffDist;
                    for (float s = 0.5f; s <= stepOffDist; s += 0.5f)
                        if (!IsUnderStaticTransportDeck(botTransport, bx + ddx / dd * s, by + ddy / dd * s,
                                std::max(bot->GetPositionZ(), botTransport->GetPositionZ())))
                        {
                            edge = s;
                            break;
                        }
                    stepOffDist = edge;
                }
            }
            uint32 const stepOffNeedMs = uint32(1000.0f * stepOffDist / std::max(bot->GetSpeed(MOVE_RUN), 1.0f)) +
                kStaticStepMarginMs;
            float const botLevelZ = botTram ? bot->GetPositionZ() : botTransport->GetPositionZ();
            if (botTransport->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && botTransport != transport &&
                (std::fabs(botLevelZ - master->GetPositionZ()) > 8.0f ||
                 (botTram && sServerFacade->GetDistance2d(bot, master) > kTramStationRadius) ||
                 !StaticPlatformStaysFor(botTransport, stepOffNeedMs)))
            {
                if (bot->isMoving())
                    bot->StopMoving();
                assistProbe("bot-static-hold"); // #314 temporary probe
                return true;
            }
        }

        if (transport && map && bot->GetTransport() != transport)
        {
            // #209: probe a tram car at its rider-master's deck height, not at its origin (11.31 yd above the
            // floor): the 4 yd deck ray from origin z + 2 never reached the deck, so a bot on it never boarded here.
            float const boardLevelZ = (masterOnTransport && IsHorizontalStaticTransport(transport))
                ? master->GetPositionZ() : transport->GetPositionZ();
            float const botProbeZ = std::max(bot->GetPositionZ(), boardLevelZ);
            Transport* botSurfaceTransport = GetTransportForPosTolerant(map, bot, bot->GetPhaseMask(),
                bot->GetPositionX(), bot->GetPositionY(), botProbeZ);

            if (botSurfaceTransport == transport)
            {
                transport->AddPassenger(bot, true);
                StopOnCurrentPos(bot);
                assistProbe("board"); // #314 temporary probe
                return true;
            }

            float const boardingAssistDistance = 60.0f;
            float const dist2d = sServerFacade->GetDistance2d(bot, master);
            bool const inAssist = sServerFacade->IsDistanceLessOrEqualThan(dist2d, boardingAssistDistance);

            if (inAssist)
            {
                float destX = masterOnTransport ? master->GetPositionX() : transport->GetPositionX();
                float destY = masterOnTransport ? master->GetPositionY() : transport->GetPositionY();
                float destZ = masterOnTransport ? master->GetPositionZ() : transport->GetPositionZ();
                float edgeX = 0.0f;
                float edgeY = 0.0f;
                float edgeZ = 0.0f;

                // #206 B2: true only when a real deck point was derived below, so destZ is a deck
                // surface and not the masterOnTransport fallback. The straight-line assist gate at
                // the bottom of this block depends on this flag.
                bool hasBoardingPoint = false;
                if (masterOnTransport &&
                    FindBoardingPointOnTransport(map, transport, master,
                        master->GetPositionX(), master->GetPositionY(), master->GetPositionZ(),
                        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        edgeX, edgeY, edgeZ))
                {
                    destX = edgeX;
                    destY = edgeY;
                    destZ = edgeZ;
                    hasBoardingPoint = true;
                }

                MovementPriority const priority = botAI->GetState() == BOT_STATE_COMBAT
                    ? MovementPriority::MOVEMENT_COMBAT
                    : MovementPriority::MOVEMENT_NORMAL;

                // #206 B2 (owner requirement, beyond AC): the straight-line (generatePath=false) assist
                // move only while a boat/zeppelin is docked AND a deck-anchored boarding point was
                // actually derived; otherwise normal follow handles it (no climbing from the beach or
                // walking on air after it).
                //
                // The gate is hasBoardingPoint, NOT a bot-vs-destZ distance test. When
                // FindBoardingPointOnTransport fails, destZ is still the masterOnTransport fallback set
                // above - the master's own feet - which is not a deck surface, so a distance measured
                // against it can sit inside tolerance while the real deck gap is large. Measured on
                // boat 20808 at the Booty Bay pier: master z 3.0, bot pier z 6.8 gave
                // |6.8 - 3.0| = 3.8, inside the old 4.0f tolerance, while the real deck sat at z 0.2 -
                // a 6.6 yd drop. The assist fired and the generatePath=false line below walked the bot
                // straight off the 6.8 yd pier and into the water (observed z -2.7). With no boarding
                // point there is no deck value to measure against and nothing to walk a straight line
                // to, so there must be no straight-line assist at all. Where a boarding point does
                // exist, destZ is the genuine deck (assigned above) and this gate behaves as the
                // original distance test did.
                bool const canBoardAssist = !isMoTransport ||
                    (IsDockedNow(transport) && hasBoardingPoint);
                bool const movingAllowed = IsMovingAllowed();
                bool const dupMove = IsDuplicateMove(mapId, destX, destY, destZ);
                bool const waiting = IsWaitingForLastMove(priority);

                if (movingAllowed && !dupMove && !waiting && canBoardAssist)
                {
                    if (bot->IsSitState())
                        bot->SetStandState(UNIT_STAND_STATE_STAND);

                    bot->CastStop();

                    if (MotionMaster* mm = bot->GetMotionMaster())
                        mm->MovePoint(0, destX, destY, destZ, false);
                    else
                    {
                        assistProbe("assist-nomotionmaster"); // #314 temporary probe
                        return false;
                    }

                    float delay = 1000.0f * MoveDelay(bot->GetExactDist(destX, destY, destZ));
                    delay = std::clamp(delay, 0.0f, static_cast<float>(sPlayerbotAIConfig->maxWaitForMove));

                    AI_VALUE(LastMovement&, "last movement")
                        .Set(mapId, destX, destY, destZ, bot->GetOrientation(), delay, priority);
                    ClearIdleState();
                    assistProbe("assist-move", true, destX, destY, destZ, dist2d); // #314/#316 temporary probe
                    return true;
                }

                // #314 temporary probe: reached the boarding-assist distance but the straight-line
                // assist was gated off, so control falls through to the normal follow below - the
                // prime suspect for a bot walking off the deck into the water while the master is
                // already gone. hasBoardingPoint/canBoardAssist are the deciding flags.
                assistProbe(canBoardAssist ? "fallthrough-gated" : "fallthrough-noboard",
                            false, -2.0f, -2.0f, -2.0f, dist2d);
            }
            else
            {
                assistProbe("fallthrough-far", false, -2.0f, -2.0f, -2.0f, dist2d);
            }

            // #589: linked elevator (the master left it at another landing, see LinkedStaticTransport): the
            // plain follow below would target the master across the open shaft. Hold instead; a running
            // assist move keeps going, and the next tick boards or re-checks the platform. Never true for an
            // MO transport, which is only ever resolved from the master.
            if (!masterOnTransport)
                return true;
        }
        else if (transport)
        {
            assistProbe(bot->GetTransport() == transport ? "fallthrough-aboard" : "fallthrough-nomap");
        }
    }
    // end transport handling

    bool moved = false;
    if (!target.empty())
    {
        // #316: never walk a bot that is on dry land into water. The transport block's hold only covers the
        // boat-away window; on a DOCKED window control falls through to the plain follow, whose destination is
        // the master's XY - over water while the master rides - and that is what walked the bot off the shore
        // (measured z 22.3 -> -57.2, mgen=8 throughout). Refuse such a move and hold instead.
        // NOTE: `map` from the transport block above is scoped to that block; this guard runs after it, so
        // take the map from the BOT here (a bot in water with no map is not a case worth guarding).
        // #316: never walk a bot that is on dry land into water. The transport block's hold only covers the
        // boat-away window; on a DOCKED window control falls through to the plain follow, whose destination is
        // the master's XY - over water while the master rides - and that is what walked the bot off the shore
        // (measured z 22.3 -> -57.2, mgen=8 throughout). Refuse such a move and hold instead.
        //
        // N1: this is deliberately NOT narrowed to "the master is on a transport". Two narrowed forms were
        // built and BOTH broke the fix (master->GetTransport() -> z min -57.8; the block's own
        // masterOnTransport -> z min -57.5, guard fires 164 vs 1193): the descent happens in a window where
        // the master is not detected as riding, so narrowing removes exactly the coverage that matters.
        // The generic form is the one that is verified to work. The narrower question ("does this also stop
        // a bot following a merely-swimming master?") is tracked separately rather than guessed at here.
        Map* guardMap = bot ? bot->GetMap() : nullptr;
        // #882: the narrower question from N1 - a master who is himself SWIMMING (in the water, not on a transport: a deck
        // is above the surface, so this never holds while riding) is followed into the water as AC does.
        Player* const guardMaster = botAI->GetMaster();
        bool const masterSwimming = guardMaster && !guardMaster->GetTransport() && (guardMaster->IsInWater() || guardMaster->IsUnderWater());
        if (bot && !masterSwimming && !bot->IsInWater() && !bot->IsUnderWater() && guardMap)
        {
            // destination the follow would move to: the follow target's position, else the formation location
            float destX = bot->GetPositionX();
            float destY = bot->GetPositionY();
            float destZ = bot->GetPositionZ();
            bool haveDest = false;
            if (Unit* ft = AI_VALUE(Unit*, target))
            {
                destX = ft->GetPositionX(); destY = ft->GetPositionY(); destZ = ft->GetPositionZ();
                haveDest = true;
            }
            else
            {
                Formation* f = AI_VALUE(Formation*, "formation");
                if (f) { WorldLocation loc = f->GetLocation(); destX = loc.m_positionX; destY = loc.m_positionY; destZ = loc.m_positionZ; haveDest = true; }
            }
            if (haveDest)
            {
                float groundZ = INVALID_HEIGHT;
                if (IsOverWater(guardMap, bot->GetPhaseMask(), destX, destY, destZ, groundZ))
                {
                    // destination terrain is submerged AND the bot is on dry land: do not walk into the sea.
                    // #316: this site is OUTSIDE the transport block, so the #314 `assistProbe` lambda is not
                    // in scope here - log directly in the same [FOLLOW314] shape so the branch stays observable
                    // and a reader can still count it beside the other branches.
                    if (bot->isMoving())
                        bot->StopMoving();
                    TC_LOG_DEBUG("playerbots",
                        "[FOLLOW314] %s branch=water-guard-follow tguid=0 entry=0 mo=0 static=0 mOnT=0 botT=%u "
                        "dockednow=0 timer=0 period=0 x=%.1f y=%.1f z=%.1f flags=%u moving=%d mgen=%d "
                        "hasTarget=1 dstx=%.1f dsty=%.1f dstz=%.1f dist2d=-1.0",
                        bot->GetName().c_str(),
                        bot->GetTransport() ? (uint32)bot->GetTransport()->GetGUID().GetCounter() : 0u,
                        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        bot->GetUnitFlags(), bot->isMoving() ? 1 : 0,
                        bot->GetMotionMaster() ? int(bot->GetMotionMaster()->GetCurrentMovementGeneratorType()) : 19,
                        destX, destY, destZ);
                    return true;
                }
            }
        }

        moved = Follow(AI_VALUE(Unit*, target));
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || loc.GetMapId() == -1)
            return false;

        // #316: the SAME water guard as the targeted branch above, for the no-target path. This branch is
        // the one that actually ran on the measured leg (`target` empty -> formation location, which is the
        // master's XY out over the water while the master rides) and it MoveTo's with no walkability test,
        // walking the bot off the shore (z 22.4 -> -57.3 with mgen=8 throughout). Refusing here is what
        // closes the entry; without it the guard above is unreachable in exactly the failing scenario.
        {
            // #316: the SAME generic guard as the targeted branch above (see its N1 note - narrowing this
            // to a transport check was measured to break the fix).
            Map* guardMap = bot ? bot->GetMap() : nullptr;
            Player* const guardMaster = botAI->GetMaster();  // #882: see the targeted branch above
            bool const masterSwimming = guardMaster && !guardMaster->GetTransport() && (guardMaster->IsInWater() || guardMaster->IsUnderWater());
            if (bot && !masterSwimming && !bot->IsInWater() && !bot->IsUnderWater() && guardMap)
            {
                float groundZ = INVALID_HEIGHT;
                if (IsOverWater(guardMap, bot->GetPhaseMask(),
                        loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), groundZ))
                {
                    if (bot->isMoving())
                        bot->StopMoving();
                    TC_LOG_DEBUG("playerbots",
                        "[FOLLOW314] %s branch=water-guard-follow tguid=0 entry=0 mo=0 static=0 mOnT=0 botT=%u "
                        "dockednow=0 timer=0 period=0 x=%.1f y=%.1f z=%.1f flags=%u moving=%d mgen=%d "
                        "hasTarget=0 dstx=%.1f dsty=%.1f dstz=%.1f dist2d=-1.0",
                        bot->GetName().c_str(),
                        bot->GetTransport() ? (uint32)bot->GetTransport()->GetGUID().GetCounter() : 0u,
                        bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                        bot->GetUnitFlags(), bot->isMoving() ? 1 : 0,
                        bot->GetMotionMaster() ? int(bot->GetMotionMaster()->GetCurrentMovementGeneratorType()) : 19,
                        loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
                    return true;
                }
            }
        }

        // #209 w29 (owner requirement, beyond AC): a passenger of a static transport (elevator/tram) must not be sent
        // THROUGH its own floor. The formation location takes its z from the map height at that XY
        // (Formations.cpp NearFormation: CheckCollisionAndGetValidCoords / UpdateAllowedPositionZ), and a type-11
        // platform is not in the map height data, so over the deck that is the shaft pit / track bed ~9-10 yd below.
        // The passenger keeps IGNORE_PATHFINDING (Transport.cpp:607-609), so MovePoint is a straight line to that z:
        // s9wo-mop-elev-w29-2211 Carius walked 68.8 -> 59.3 under the bottom landing deck right after the master stepped
        // off, the #589 bot-static-hold below then held it there and it rode back up; tram w28 Aleneya z -13.9 under a
        // -4.1 deck. AC has the same formation z (AC Formations.cpp:107-127) but no static hold, so its bot climbs out on
        // the next tick (s9wo-ac-elev-w29-2211 Crartarl 59.3 -> 70.7). Keep the destination at the deck/landing level.
        if (Transport* botT = bot->GetTransport())
        {
            float const floorZ = bot->GetPositionZ();
            if (botT->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && loc.GetPositionZ() < floorZ - 2.0f)
            {
                float z = floorZ;
                // master stepped off onto the landing at (about) this level: aim at its feet
                if (master && !master->GetTransport() && std::fabs(master->GetPositionZ() - floorZ) < 3.0f)
                    z = master->GetPositionZ();
                loc = WorldLocation(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), z, loc.GetOrientation());
            }
        }

        MovementPriority priority = botAI->GetState() == BOT_STATE_COMBAT ? MovementPriority::MOVEMENT_COMBAT : MovementPriority::MOVEMENT_NORMAL;
        moved = MoveTo(loc.GetMapId(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ(), false, false, false,
            true, priority, true);
    }

    if (Pet* pet = bot->GetPet())
    {
        //botAI->PetFollow();
    }

    return moved;
}

bool FollowAction::isUseful()
{
    // move from group takes priority over follow as it's added and removed automatically
    // (without removing/adding follow)
    if (botAI->HasStrategy("move from group", BOT_STATE_COMBAT) ||
        botAI->HasStrategy("move from group", BOT_STATE_NON_COMBAT))
        return false;

    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
        return false;

    Formation* formation = AI_VALUE(Formation*, "formation");
    if (!formation)
        return false;

    std::string const target = formation->GetTargetName();

    Unit* fTarget = nullptr;
    if (!target.empty())
        fTarget = AI_VALUE(Unit*, target);
    else
        fTarget = AI_VALUE(Unit*, "master target");

    if (fTarget)
    {
        if (fTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return false;

        if (!CanDeadFollow(fTarget))
            return false;

        if (fTarget->GetGUID() == bot->GetGUID())
            return false;
    }

    float distance = 0.f;
    if (!target.empty())
    {
        distance = AI_VALUE2(float, "distance", target);
    }
    else
    {
        WorldLocation loc = formation->GetLocation();
        if (Formation::IsNullLocation(loc) || bot->GetMapId() != loc.GetMapId())
            return false;

        distance = bot->GetDistance(loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ());
    }
    // #647 AC FollowActions.cpp:287-288: while "master fishing", only follow beyond fishingDistanceFromMaster.
    if (botAI->HasStrategy("master fishing", BOT_STATE_NON_COMBAT))
        return sServerFacade->IsDistanceGreaterThan(distance, kFishingDistanceFromMaster);

    // #209 w29b (owner requirement, beyond AC; AC's isUseful has no transport case, AC FollowActions.cpp:236-290): the
    // master is riding an elevator/tram and this bot is not on it. A bot that already stands within the follow distance
    // never ran Execute, so the static-transport boarding branches there never saw it: s9wo-mop-tram-w29b-2323 Aleneya
    // stopped 5.6 yd from the master at (10.1,9.2), just off the car deck, and logged no [FOLLOW314] line for the whole
    // 13.8 s dwell (first one: tram-notparked at timer 708, after departure) while Carius, whose formation spot was on
    // the deck, boarded. Keep the follow useful inside the boarding-assist radius so Execute decides (board / wait /
    // hold) on every tick.
    if (Player* m = botAI->GetMaster())
        if (Transport* mt = m->GetTransport())
            if (mt->GetGoType() == GAMEOBJECT_TYPE_TRANSPORT && bot->GetTransport() != mt && bot->GetMap() == m->GetMap() &&
                bot->GetExactDist2d(m) <= kStaticBoardingAssistDistance)
                return true;

    return sServerFacade->IsDistanceGreaterThan(distance, formation->GetMaxDistance());
}

bool FollowAction::CanDeadFollow(Unit* target)
{
    // Move to corpse when dead and player is alive or not a ghost.
    if (!bot->IsAlive() && (target->IsAlive() || !target->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST)))
        return false;

    return true;
}

bool FleeToMasterAction::Execute(Event event)
{
    Unit* fTarget = AI_VALUE(Unit*, "master target");
    bool canFollow = Follow(fTarget);
    if (!canFollow)
    {
        // botAI->SetNextCheckDelay(5000);
        return false;
    }

    WorldPosition targetPos(fTarget);
    WorldPosition bosPos(bot);
    float distance = bosPos.fDist(targetPos);

    if (distance < sPlayerbotAIConfig->reactDistance * 3)
    {
        if (!urand(0, 3))
            botAI->TellMaster("I am close, wait for me!");
    }
    else if (distance < 1000)
    {
        if (!urand(0, 10))
            botAI->TellMaster("I heading to your position.");
    }
    else if (!urand(0, 20))
        botAI->TellMaster("I am traveling to your position.");

    botAI->SetNextCheckDelay(3000);

    return true;
}

bool FleeToMasterAction::isUseful()
{
    if (!botAI->GetGroupMaster())
        return false;

    if (botAI->GetGroupMaster() == bot)
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && botAI->GetGroupMaster()->GetTarget() == target->GetGUID())
        return false;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;

    Unit* fTarget = AI_VALUE(Unit*, "master target");

    if (!CanDeadFollow(fTarget))
        return false;

    return true;
}
