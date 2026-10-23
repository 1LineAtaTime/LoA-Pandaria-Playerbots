/*
 * #901 Tol Barad bot tactics (beyond AC). Spec: Wowpedia Tol Barad (PvP) - the attackers win by holding all three bases
 * (Ironclad Garrison, Warden's Vigil, Slagworks) and may also bring the spires down with Abandoned Siege Engines
 * (45344, Deploy Siege Mode 84974); the defenders split over the three bases and kill the engines. Capture is by presence
 * (BfCapturePoint, like Eye of the Storm), so a bot only has to stand inside the radius and survive.
 *
 * Pattern: AC's AB/EotS node logic (BattleGroundTactics.cpp selectObjective, "fewest friends, then nearest"), the DG/BFG
 * per-map tactics files, the IoC/SotA spellclick boarding primitive (BgTacticsIC.cpp Board: StopMoving, HandleSpellClick).
 *
 * Ownership is read from the capture-point flag pole's art kit (TolBaradCapturePoint::ChangeTeam sets 2 Alliance / 1 Horde
 * on a capture and 0 while contested). World states are packet-only on this core (Map::GetWorldState is not fed by the
 * battlefield), so they cannot be read bot-side. Art kit 0 is read as "not held": for an attacker it means go there, for a
 * defender it means a contested point - the same action either way.
 */
#include "BgTacticsTB.h"

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "BattlefieldTB.h"
#include "BattleGroundTactics.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "Log.h"
#include "Map.h"
#include "Playerbots.h"
#include "Vehicle.h"
#include "PathGenerator.h"
#include "MotionMaster.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <mutex>

namespace
{
    uint32 const TB_BATTLE_ID = 21;            // BATTLEFIELD_BATTLEID_TB (Battlefield.h:39)
    float const TB_CAPTURE_RADIUS = 18.0f;     // stand this close to the pole (inside the capture radius)
    float const TB_FRIEND_RADIUS = 30.0f;      // a friend this close to a base counts as already there
    float const TB_ENGINE_SEARCH = 200.0f;
    float const TB_ENGINE_BOARD = 8.0f;
    float const TB_DEFENDER_ENGINE_RANGE = 70.0f;   // a defender fights an enemy engine this close
    float const TB_SIEGE_DEPLOY_RANGE = 55.0f;      // TODO verify: spell 84974's real cannon range (siege mode)
    uint32 const TB_REEVAL_MS = 20000;

    std::mutex g_tbMutex;
    struct Pick { uint8 point; uint32 until; };
    std::map<ObjectGuid, Pick> g_pick;          // bot -> chosen base + re-evaluation time
    std::map<ObjectGuid, uint32> g_logAt;       // log throttle
    std::map<ObjectGuid, uint32> g_deployed;    // driver -> time siege mode was cast
    std::map<ObjectGuid, uint32> g_boardFail;   // bot -> time a spellclick board failed (30 s backoff)

    // #903: walk progress per bot (see the `walk` lambda in TolBaradPlayAction::Execute). `d` = 2D distance to the walk target
    // at the last sample: progress is measured toward the target, not as displacement, so shuffling around a navmesh island
    // (w99: 218.2 <-> 220.1 yd) no longer resets the stall counter.
    struct Prog { float d = 0; uint32 at = 0; uint8 stuck = 0; };
    std::map<ObjectGuid, Prog> g_prog;

    // #903 (w99-t901 probes, 2026-10-22): several Tol Barad graveyards stand on navmesh islands - every path out is a one-
    // component PathGenerator INCOMPLETE (pathType 0x4) and MoveTo refuses it (PathTypeIsWalkable / SegmentLeavesSurface).
    // Measured: WorldSafeLocs Baradin Hold (-1191.35, 956.84, 151.14) and its spirit (-1247.42, 981.25, 155.35) - a bot sent
    // to the spirit walked back to the WorldSafeLocs spot and stopped; South Spire WorldSafeLocs (-1576.53, 853.33, 191.87)
    // and spirit (-1603.34, 874.29, 193.69); Warden's Vigil spirit (-1570.44, 1167.57, 159.50) -> stuck at (-1563, 1183, 162).
    // West Spire (191 yd up like South Spire) is not measured; the stall gate below keeps a wrong guess harmless.
    struct Island { float x, y, r, zMin; };
    Island const TB_ISLANDS[] =
    {
        { -1215.0f, 990.0f, 75.0f, 140.0f },   // Baradin Hold plateau (the only graveyard the defenders hold in the war)
        { -1590.0f, 864.0f, 45.0f, 178.0f },   // South Spire graveyard
        { -1567.0f, 1175.0f, 35.0f, 152.0f },  // Warden's Vigil graveyard
        { -1052.0f, 1494.0f, 45.0f, 178.0f },  // West Spire graveyard (unmeasured)
    };
    // Points measured on the connected mesh (a bot teleported there walked to its base in the same w99 war):
    // the ground at the foot of Baradin Hold (BattlefieldTB.h RandomQuestgiverPos) -> Ironclad Garrison; the Ironclad
    // Garrison, Slagworks and East Spire spirit-guide spots (BattlefieldTB.h TBGraveyards[1], [3], [6]).
    Position const TB_WALKABLE[] =
    {
        { -1228.93f, 975.04f, 121.72f, 5.97f },
        { -974.28f, 1089.47f, 132.99f, 5.90f },
        { -1343.32f, 565.24f, 139.04f, 1.66f },
        { -943.66f, 572.36f, 157.54f, 1.74f },
    };

    bool OnIsland(Player* bot)
    {
        for (Island const& i : TB_ISLANDS)
            if (bot->GetPositionZ() > i.zMin && bot->GetExactDist2d(i.x, i.y) < i.r)
                return true;
        return false;
    }

    Position const& NearestWalkable(Player* bot)
    {
        Position const* best = &TB_WALKABLE[0];
        for (Position const& p : TB_WALKABLE)
            if (bot->GetExactDist2d(&p) < bot->GetExactDist2d(best))
                best = &p;
        return *best;
    }

    bool Due(Player* bot, uint32 periodMs)
    {
        std::lock_guard<std::mutex> lock(g_tbMutex);
        uint32 const now = getMSTime();
        uint32& t = g_logAt[bot->GetGUID()];
        if (t && now - t < periodMs)
            return false;
        t = now;
        return true;
    }

    Battlefield* Tb()
    {
        return sBattlefieldMgr->GetBattlefieldByBattleId(TB_BATTLE_ID);
    }

    // 0 none/contested, 1 Horde, 2 Alliance (TB_GO_ARTKIT_*); -1 = pole not found (grid not loaded)
    int OwnerArtKit(Player* bot, Battlefield* bf, uint8 i)
    {
        TBCapturePointSpawnData const& cp = TBCapturePoints[i];
        std::list<GameObject*> poles;
        bot->GetGameObjectListWithEntryInGrid(poles, cp.entryFlagPole[bf->GetDefenderTeam()], 5000.0f);
        for (GameObject* go : poles)
            if (go->GetExactDist2d(cp.x, cp.y) < 15.0f)
                return go->GetGoArtKit();
        return -1;
    }

    // true when `team` holds base i
    bool Held(Player* bot, Battlefield* bf, uint8 i, TeamId team)
    {
        int const kit = OwnerArtKit(bot, bf, i);
        if (kit < 0)   // unknown: the defenders start by holding every base
            return team == bf->GetDefenderTeam();
        return kit == (team == TEAM_ALLIANCE ? TB_GO_ARTKIT_FLAG_ALLIANCE : TB_GO_ARTKIT_FLAG_HORDE);
    }

    uint32 FriendsAt(Player* bot, float x, float y)
    {
        uint32 n = 0;
        for (auto const& ref : bot->GetMap()->GetPlayers())
        {
            Player* p = ref.GetSource();
            if (p && p != bot && p->IsAlive() && p->GetTeamId() == bot->GetTeamId() && p->GetExactDist2d(x, y) <= TB_FRIEND_RADIUS)
                ++n;
        }
        return n;
    }

    // AB/EotS node choice: the base that is not ours (attacker) / not safely held (defender), fewest friends already there,
    // then nearest. Sticky for TB_REEVAL_MS so the raid does not oscillate.
    int PickBase(Player* bot, Battlefield* bf)
    {
        bool const attacker = bot->GetTeamId() == bf->GetAttackerTeam();
        TeamId const me = bot->GetTeamId();

        {
            std::lock_guard<std::mutex> lock(g_tbMutex);
            auto itr = g_pick.find(bot->GetGUID());
            if (itr != g_pick.end() && getMSTime() < itr->second.until)
                return itr->second.point;
        }

        int best = -1;
        float bestScore = 1e9f;
        for (uint8 i = 0; i < TB_BASE_COUNT; ++i)
        {
            TBCapturePointSpawnData const& cp = TBCapturePoints[i];
            bool const held = Held(bot, bf, i, me);
            float score = bot->GetExactDist2d(cp.x, cp.y) / 10.0f + FriendsAt(bot, cp.x, cp.y) * 30.0f;
            if (attacker)
            {
                if (held)
                    score += 500.0f;           // already ours: only if nothing else is left
            }
            else
            {
                if (!held)
                    score -= 80.0f;            // defenders prefer a contested/lost base
                else
                    score += 20.0f;            // held: still wants a guard, spread by the friend count
            }
            if (score < bestScore)
            {
                bestScore = score;
                best = i;
            }
        }

        if (best >= 0)
        {
            std::lock_guard<std::mutex> lock(g_tbMutex);
            g_pick[bot->GetGUID()] = { uint8(best), getMSTime() + TB_REEVAL_MS };
        }
        return best;
    }

    // anyFaction: boarding by the attackers - the core hands every engine to the DEFENDER faction (BattlefieldTB.cpp:609), so the old
    // "friendly engine" filter (hostile=false) rejected every engine for an attacker and no bot ever tried (#901 zero [TB] board lines).
    Creature* NearestEngine(Player* bot, bool hostile, bool wantFree, float range, bool anyFaction = false)
    {
        std::list<Creature*> engines;
        bot->GetCreatureListWithEntryInGrid(engines, NPC_ABANDONED_SIEGE_ENGINE, range);
        Creature* best = nullptr;
        float bestDist = range;
        for (Creature* c : engines)
        {
            if (!c->IsAlive() || !c->IsVehicle())
                continue;
            if (!anyFaction && hostile != bot->IsValidAttackTarget(c))
                continue;
            if (wantFree && (!c->GetVehicleKit() || !c->GetVehicleKit()->HasEmptySeat(0)))
                continue;
            float const d = bot->GetExactDist(c);
            if (d < bestDist)
            {
                bestDist = d;
                best = c;
            }
        }
        return best;
    }

    // #903: siege-driver election. build 100: `guid % 4 == 0` picked no bot among the ten census attackers (guids 22..2007: %4 =
    // 2,3,1,3,2,2,2,1,1,2), so step 4 never ran and the log has zero '[TB] ... board' lines. Rank the attackers on the map by guid
    // instead: the lowest ceil(n/4) (at least one) are drivers, so a raid of any size gets engines.
    bool IsSiegeDriver(Player* bot)
    {
        uint32 n = 0, below = 0;
        uint64 const mine = bot->GetGUID().GetCounter();
        for (auto const& ref : bot->GetMap()->GetPlayers())
        {
            Player* p = ref.GetSource();
            if (!p || p->GetTeamId() != bot->GetTeamId() || !GET_PLAYERBOT_AI(p))
                continue;
            ++n;
            if (p->GetGUID().GetCounter() < mine)
                ++below;
        }
        return below < std::max<uint32>(1, (n + 3) / 4);
    }

    // nearest spire the defenders still have standing (the attackers' siege target), -1 if none
    int NearestSpire(Player* bot, Unit* from)
    {
        int best = -1;
        float bestDist = 1e9f;
        for (uint8 i = 0; i < TB_TOWERS_COUNT; ++i)
        {
            GameObject* go = bot->FindNearestGameObject(TBTowers[i].entry, 5000.0f);
            if (go && go->GetDestructibleState() == GO_DESTRUCTIBLE_DESTROYED)
                continue;
            float const d = from->GetExactDist2d(TBTowers[i].x, TBTowers[i].y);
            if (d < bestDist)
            {
                bestDist = d;
                best = i;
            }
        }
        return best;
    }
}

namespace TolBaradBots
{
    bool InWar(Player* bot)
    {
        if (!bot || !bot->IsInWorld() || bot->GetMapId() != BATTLEFIELD_TB_MAPID)
            return false;
        Battlefield* bf = Tb();
        if (!bf || !bf->IsWarTime())
            return false;
        Group* group = bot->GetGroup();
        return group && group->isBFGroup();
    }

    // #900: a bot in ANY battlefield raid during that battlefield's war (Wintergrasp too) stays and fights there.
    bool InAnyWar(Player* bot)
    {
        if (InWar(bot))
            return true;
        if (!bot || !bot->IsInWorld())
            return false;
        Group* group = bot->GetGroup();
        if (!group || !group->isBFGroup())
            return false;
        Battlefield* bf = sBattlefieldMgr->GetBattlefieldToZoneId(bot->GetZoneId());
        return bf && bf->IsWarTime();
    }

    // #903: where a census bot enters the Tol Barad war (BattlefieldBots.cpp). Battlefield::GetClosestGraveYard hands out the
    // WorldSafeLocs spot, measured from the bot's position on ANOTHER map, and the two it gave in w99 (South Spire for the
    // attackers, Baradin Hold - the defenders' only war graveyard, BattlefieldTB.cpp:581) are navmesh islands. Defenders
    // enter at the foot of Baradin Hold; attackers (who hold every other graveyard in the war) at the Ironclad Garrison,
    // Slagworks or East Spire graveyard, spread by guid. All four points walked to a base in w99.
    void EntryPoint(Player* bot, Position& pos)
    {
        Battlefield* bf = Tb();
        if (bf && bot->GetTeamId() == bf->GetDefenderTeam())
            pos = TB_WALKABLE[0];
        else
            pos = TB_WALKABLE[1 + bot->GetGUID().GetCounter() % 3];
    }

    // #900 (beyond AC): generalised from the Tol Barad-only swap. "swapped" = bots whose roaming strategies are off for a war.
    void Sync(Player* bot)
    {
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (!botAI)
            return;

        static std::set<ObjectGuid> swapped;
        bool const tb = InWar(bot);
        bool const want = InAnyWar(bot);
        bool has;
        {
            std::lock_guard<std::mutex> lock(g_tbMutex);
            has = swapped.count(bot->GetGUID()) != 0;
        }
        // an engine rebuild during the war brings "new rpg" back: swap again
        if (want && has && !botAI->HasStrategy("new rpg", BOT_STATE_NON_COMBAT))
            return;
        if (!want && !has)
            return;

        if (want)
        {
            {
                std::lock_guard<std::mutex> lock(g_tbMutex);
                swapped.insert(bot->GetGUID());
            }
            botAI->ChangeStrategy(tb ? "+tolbarad,+pvp,-new rpg,-gather,-rpg,-grind,-travel" : "+pvp,-new rpg,-gather,-rpg,-grind,-travel",
                                  BOT_STATE_NON_COMBAT);
            TC_LOG_DEBUG("playerbots", "[BF] %s in the war -> %s", bot->GetName().c_str(), tb ? "tolbarad strategy" : "stay and fight (pvp)");
        }
        else
        {
            {
                std::lock_guard<std::mutex> lock(g_tbMutex);
                swapped.erase(bot->GetGUID());
                g_pick.erase(bot->GetGUID());
                g_deployed.erase(bot->GetGUID());
                g_boardFail.erase(bot->GetGUID());
                g_prog.erase(bot->GetGUID());
            }
            botAI->ResetStrategies();
            TC_LOG_DEBUG("playerbots", "[BF] %s left the war -> strategies rebuilt", bot->GetName().c_str());
        }
    }
}

void TolBaradStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Non-combat engine only: in combat the normal combat engine fights, and BGTactics::CaptureThreat below pulls a bot
    // into a fight the moment something hits it on its base.
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("tb play", ACTION_BG), nullptr)));
}

bool TolBaradPlayAction::Execute(Event /*event*/)
{
    if (!TolBaradBots::InWar(bot) || !bot->IsAlive())
        return false;

    Battlefield* bf = Tb();
    TeamId const me = bot->GetTeamId();
    bool const attacker = me == bf->GetAttackerTeam();

    // #903: build 98 (w98-t901) - every attacker stood at its start for the whole war (the same 'dist=268.4' / '218.2' in every log
    // line, six bots on one spot), so a plain MoveTo failed for the whole route: MovementAction::MoveTo -> SearchForBestPath refuses
    // an INCOMPLETE/SHORT/NOPATH path or one whose segment leaves the surface (PathTypeIsWalkable) and returns false with no log.
    // `walk` = MoveTo + (a) a diagnostic of why it failed (path type/length, z), (b) progress tracking: after 6 s without 4 yd of
    // progress it escalates - MoveTo to a waypoint <= 60 yd along the line (rotating +-45/90 deg to get round an obstacle, ground-
    // snapped), and after six stalled samples hands the target to the core MovePoint (pathfinding, straight glide on NOPATH).
    auto walk = [&](float x, float y, float z) -> bool
    {
        uint32 const now = getMSTime();
        float const dTarget = bot->GetExactDist2d(x, y);
        uint8 stuck;
        {
            std::lock_guard<std::mutex> lock(g_tbMutex);
            Prog& p = g_prog[bot->GetGUID()];
            if (!p.at || now - p.at >= 6000)
            {
                // #903: stalled = not 4 yd closer to the target than 6 s ago (w99: island shuffles reset the old displacement test)
                bool const stalled = p.at && p.d - dTarget < 4.0f && !bot->IsInCombat();
                p.stuck = stalled ? std::min<uint8>(p.stuck + 1, 20) : 0;
                p.d = dTarget;
                p.at = now;
            }
            stuck = p.stuck;
        }

        // #903: on a graveyard navmesh island no path leaves (INCOMPLETE), so neither MoveTo, the waypoints nor the core
        // MovePoint (it rides the same partial path to the island edge) gets the bot off. After 12 s without progress, put it
        // on the nearest measured point of the connected mesh - the same relocation the resurrection itself is.
        if (stuck >= 2 && !bot->GetVehicle() && OnIsland(bot))
        {
            Position const& to = NearestWalkable(bot);
            TC_LOG_DEBUG("playerbots", "[TB] %s walk: graveyard island at (%.0f, %.0f, %.1f), stalled x%u -> (%.0f, %.0f, %.1f)",
                         bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), stuck,
                         to.GetPositionX(), to.GetPositionY(), to.GetPositionZ());
            bot->NearTeleportTo(to.GetPositionX(), to.GetPositionY(), to.GetPositionZ(), to.GetOrientation());
            std::lock_guard<std::mutex> lock(g_tbMutex);
            g_prog.erase(bot->GetGUID());
            return true;
        }

        bool ok;
        if (stuck == 0)
            ok = MoveTo(bot->GetMapId(), x, y, z);
        else if (stuck >= 6)
        {
            bot->GetMotionMaster()->MovePoint(0, x, y, z, true);
            ok = true;
            if (Due(bot, 10000))
                TC_LOG_DEBUG("playerbots", "[TB] %s walk: stalled x%u, core MovePoint to (%.0f, %.0f, %.0f)", bot->GetName().c_str(),
                             stuck, x, y, z);
        }
        else
        {
            static float const angles[] = { 0.0f, 0.785f, -0.785f, 1.571f, -1.571f };
            float const dx = x - bot->GetPositionX(), dy = y - bot->GetPositionY();
            float const d = std::sqrt(dx * dx + dy * dy);
            float const step = std::min(d, 60.0f);
            float const a = std::atan2(dy, dx) + angles[(stuck - 1) % 5];
            float const wx = bot->GetPositionX() + std::cos(a) * step;
            float const wy = bot->GetPositionY() + std::sin(a) * step;
            float const wz = bot->GetMapWaterOrGroundLevel(wx, wy, bot->GetPositionZ() + 2.0f);
            ok = wz > INVALID_HEIGHT && MoveTo(bot->GetMapId(), wx, wy, wz);
            if (Due(bot, 10000))
                TC_LOG_DEBUG("playerbots", "[TB] %s walk: stalled x%u, waypoint (%.0f, %.0f, %.1f) ok=%d", bot->GetName().c_str(), stuck,
                             wx, wy, wz, ok);
        }

        if (!ok && Due(bot, 10000))
        {
            PathGenerator gen(bot);
            gen.CalculatePath(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z);
            TC_LOG_DEBUG("playerbots", "[TB] %s walk: MoveTo (%.0f, %.0f, %.1f) refused, pathType=0x%x len=%.0f from (%.0f, %.0f, %.1f) stuck=%u",
                         bot->GetName().c_str(), x, y, z, uint32(gen.GetPathType()), gen.getPathLength(), bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ(), stuck);
        }
        return ok;
    };

    // 1. Fight whatever is hitting us (BG pattern, #754).
    if (!bot->GetVehicle())
        if (Unit* threat = BGTactics::CaptureThreat(bot))
            if (BGTactics::EngageThreat(bot, threat))
                return true;

    // 2. Driving a siege engine: attackers only.
    if (Unit* base = bot->GetVehicleBase())
    {
        if (base->GetEntry() != NPC_ABANDONED_SIEGE_ENGINE)
            return false;
        int const spire = NearestSpire(bot, base);
        if (spire < 0)
            return false;
        TBTowerInfo const& t = TBTowers[spire];
        float const dist = base->GetExactDist2d(t.x, t.y);
        // #903: the real range comes from the spell data when it has one (84974 itself may carry none: then the 55 yd guess stands)
        float deployRange = TB_SIEGE_DEPLOY_RANGE;
        if (SpellInfo const* si = sSpellMgr->GetSpellInfo(SPELL_DEPLOY_SIEGE_MODE))
            if (float const r = si->GetMaxRange(false))
                deployRange = std::min(TB_SIEGE_DEPLOY_RANGE, r * 0.9f);
        if (dist <= deployRange)
        {
            bool first;
            {
                std::lock_guard<std::mutex> lock(g_tbMutex);
                first = g_deployed.emplace(bot->GetGUID(), getMSTime()).second;
            }
            if (first)
            {
                bool const ok = botAI->CastVehicleSpell(SPELL_DEPLOY_SIEGE_MODE, base);
                TC_LOG_DEBUG("playerbots", "[TB] %s deploy siege mode (84974) at spire %d dist=%.1f range=%.1f ok=%d",
                             bot->GetName().c_str(), spire, dist, deployRange, ok);
            }
            // TODO: leave siege mode (SPELL_LEAVE_SIEGE_MODE 85078) and re-target once the spire is down.
            return true;
        }
        {
            std::lock_guard<std::mutex> lock(g_tbMutex);
            g_deployed.erase(bot->GetGUID());
        }
        if (Due(bot, 10000))
            TC_LOG_DEBUG("playerbots", "[TB] %s driving to spire %d dist=%.1f", bot->GetName().c_str(), spire, dist);
        // MoveTo clears the engine's motion on every call: skip it while the engine is already travelling
        if (base->HasUnitState(UNIT_STATE_MOVING))
            return true;
        bool const drove = MoveTo(bot->GetMapId(), t.x, t.y, t.z);
        if (!drove && Due(bot, 10000))
        {
            Vehicle* veh = bot->GetVehicle();
            VehicleSeatEntry const* seat = veh ? veh->GetSeatForPassenger(bot) : nullptr;
            TC_LOG_DEBUG("playerbots", "[TB] %s drive: MoveTo refused (seat canControl=%d) engine at (%.0f, %.0f, %.0f)",
                         bot->GetName().c_str(), seat && seat->CanControl(), base->GetPositionX(), base->GetPositionY(), base->GetPositionZ());
        }
        return drove;
    }

    // 3. Defenders kill enemy engines in range.
    if (!attacker)
        if (Creature* engine = NearestEngine(bot, true, false, TB_DEFENDER_ENGINE_RANGE))
            if (BGTactics::EngageThreat(bot, engine))
            {
                if (Due(bot, 10000))
                    TC_LOG_DEBUG("playerbots", "[TB] %s attacks siege engine dist=%.1f", bot->GetName().c_str(),
                                 bot->GetExactDist(engine));
                return true;
            }

    // 4. Attackers: one in four (by guid) is a siege driver and boards a free engine (any faction: the core gives them the defender
    //    faction) while a spire still stands; the rest take the bases. A failed board backs off 30 s so the bot does not freeze.
    bool boardBackoff = false;
    {
        std::lock_guard<std::mutex> lock(g_tbMutex);
        auto itr = g_boardFail.find(bot->GetGUID());
        boardBackoff = itr != g_boardFail.end() && getMSTime() - itr->second < 30000;
    }
    if (attacker && !boardBackoff && IsSiegeDriver(bot) && NearestSpire(bot, bot) >= 0)
    {
        Creature* engine = NearestEngine(bot, false, true, TB_ENGINE_SEARCH, true);
        if (!engine && Due(bot, 15000))
            TC_LOG_DEBUG("playerbots", "[TB] %s board: no free siege engine within %.0f", bot->GetName().c_str(), TB_ENGINE_SEARCH);
        if (!engine)
        {
            // #901 (w97c: 63 'no free siege engine within 200', 0 boardings in a 15 min war): the engines stand at fixed spawn
            // points (BattlefieldTB.h TBAbandonedSiegeEngineSpawnData), mostly > 200 yd from the bases the attackers hold and
            // in grids nobody has loaded, so the grid search never sees them. Walk to the nearest spawn point and look again.
            Position const* spawn = nullptr;
            float spawnDist = 0.0f;
            for (Position const& p : TBAbandonedSiegeEngineSpawnData)
            {
                float const d = bot->GetExactDist(&p);
                if (!spawn || d < spawnDist)
                {
                    spawn = &p;
                    spawnDist = d;
                }
            }
            if (spawn && spawnDist > 25.0f)
            {
                if (Due(bot, 10000))
                    TC_LOG_DEBUG("playerbots", "[TB] %s board: walking to engine spawn (%.0f, %.0f) dist=%.1f", bot->GetName().c_str(),
                                 spawn->GetPositionX(), spawn->GetPositionY(), spawnDist);
                return walk(spawn->GetPositionX(), spawn->GetPositionY(), spawn->GetPositionZ());
            }
        }
        if (engine)
        {
            if (bot->GetExactDist(engine) > TB_ENGINE_BOARD)
            {
                if (Due(bot, 10000))
                    TC_LOG_DEBUG("playerbots", "[TB] %s board: walking to engine %u dist=%.1f faction=%u", bot->GetName().c_str(),
                                 engine->GetGUID().GetCounter(), bot->GetExactDist(engine), engine->GetFaction());
                return walk(engine->GetPositionX(), engine->GetPositionY(), engine->GetPositionZ());
            }

            // Board like IoC/SotA: stand still, spellclick, drop the mount (BgTacticsIC.cpp Board).
            if (bot->IsMounted())
                bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            bool const clicked = engine->HandleSpellClick(bot, 0);
            bool const ok = bot->IsOnVehicle(engine);
            TC_LOG_DEBUG("playerbots", "[TB] %s board siege engine clicked=%d onVehicle=%d faction=%u", bot->GetName().c_str(), clicked, ok,
                         engine->GetFaction());
            if (ok || clicked)
                return true;   // the enter-vehicle aura lands next tick; step 2 then drives
            std::lock_guard<std::mutex> lock(g_tbMutex);
            g_boardFail[bot->GetGUID()] = getMSTime();
        }
    }

    // 5. Bases: pick one, go there, stand in the radius (presence capture).
    int const base = PickBase(bot, bf);
    if (base < 0)
        return false;
    TBCapturePointSpawnData const& cp = TBCapturePoints[base];
    float const dist = bot->GetExactDist2d(cp.x, cp.y);
    if (Due(bot, 15000))
        TC_LOG_DEBUG("playerbots", "[TB] %s %s -> base %d (%s) dist=%.1f held=%d friends=%u", bot->GetName().c_str(),
                     attacker ? "attacker" : "defender", base, TBGraveyardNames[base + 1], dist,
                     Held(bot, bf, uint8(base), me), FriendsAt(bot, cp.x, cp.y));

    if (dist > TB_CAPTURE_RADIUS)
    {
        if (dist < 60.0f && bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        return walk(cp.x, cp.y, cp.z);
    }

    return false;   // in the radius: stand and let the combat/pvp strategies fight
}
