/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/NewRpgBaseAction.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "NewRpgBaseAction.h"

#include <algorithm>
#include <cmath>
#include <list>
#include <set>    // #945
#include "GameObject.h"  // #945 mailbox errands
#include <mutex>  // #843 quest hub search, #945 town errands
#include <unordered_map>
#include <unordered_set>  // #975 navmesh reach fill
#include <vector>

#include "BotScenario.h"
#include "BudgetValues.h"  // #945: NeedMoneyFor
#include "Config.h"  // #945: sConfigMgr
#include "BroadcastHelper.h"
#include "DBCEnums.h"
#include "GossipDef.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "IVMapManager.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "NewRpgDuelSpot.h"  // #201 (#612): sDuelSpotMgr
#include "NewRpgRoamTransport.h"  // #218 (beyond AC)
#include "Map.h"
#include "MapDefines.h"
#include "MMapFactory.h"  // #975 quest hub navmesh reach
#include "PathGenerator.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "OutdoorPvPMgr.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "RandomPlayerbotMgr.h"
#include "StatsWeightCalculator.h"
#include "Timer.h"
#include "TravelMgr.h"   // #1004
#include "TravelNode.h"  // #1004: sTravelNodeMap.FindTaxiPath
#include "Unit.h"
#include "World.h"
#include "WorldSession.h"

// AC parity (#61 Dalaran finding): AC picks a turn-in giver with CanRewardQuest(quest, 0, false), which on AC
// runs the base checks AND the reward-item bag-space check. On this core the 3-arg overload skips the base
// checks and the 2-arg one skips bag space, so a bot with full bags re-selected the same giver forever
// Both are required here.
static bool CanRewardQuestAC(Player* bot, Quest const* quest)
{
    // AC's third argument is reward-choice index 0; this core takes the choice *item id*, so pass the first
    // choice item (0 matched nothing and skipped the bag check: turn-in livelock).
    uint32 const choiceItem = quest->GetRewChoiceItemsCount() > 0 ? quest->RewardChoiceItemId[0] : 0;
    return bot->CanRewardQuest(quest, false) && bot->CanRewardQuest(quest, choiceItem, false);
}

// #872: the #843 quest hub search keeps a per-bot "trip" flag; defined with the search further down.
namespace QuestHubSearch
{
void ClearTrip(Player* bot);
bool TakeTrip(Player* bot);
}  // namespace QuestHubSearch

// #945: MoveFarTo tells the town errand (defined further down) about a stuck teleport / an abandoned trip, for its end reason.
namespace TownErrand
{
void NoteStuckTeleport(Player* bot);
void NoteAbandon(Player* bot);
}  // namespace TownErrand

// MoP-only data compensation. On an INCOMPLETE path the core's smoothing
// can append the raw, unreachable destination as the last point (PathGenerator::FindSmoothPath end-step 1 yd
// height test), so GetActualEndPosition() is a point in the air and the spline flies the bot straight to it.
// AC runs the same code, but its navmesh fails that height test where the MoP mesh passes it. Real smoothing
// steps are <= 4 yd, so a longer last step is the invented point: end on the last real point instead.
static G3D::Vector3 ReachableEndPosition(PathGenerator const& path)
{
    Movement::PointsArray const& points = path.GetPath();
    if ((path.GetPathType() & PATHFIND_INCOMPLETE) && points.size() >= 2)
    {
        G3D::Vector3 const& last = points[points.size() - 1];
        G3D::Vector3 const& prev = points[points.size() - 2];
        if ((last - prev).length() > 8.0f)
            return prev;
    }
    return path.GetActualEndPosition();
}

// #814 (MoP-only compensation, beyond AC): where MoveFarTo's stuck teleport puts a GO_GRIND bot. AC teleports to the
// raw destination (AC NewRpgBaseAction.cpp:121 `bot->TeleportTo(dest)`), and a grind cell's x, y AND z are each
// rounded to the 50-yd lattice (TravelMgr.cpp:504-507 lround(pos / 50); AC TravelMgr.cpp:4812-4829), so the bot lands
// up to 25 yd under the terrain or in the air. AC walks out of that spot: its core gives a player whose start point
// has no polygon a straight NORMAL|NOT_USING_PATH shortcut (AC PathGenerator.cpp:280-296, `creature ? CanFly() :
// true`) and MoveTo hands it to MovePoint. This core returns NOPATH for players there (PathGenerator.cpp:211-243) and
// the module refuses airborne segments (#747 MovementAction::PathIsWalkable), so the bot never moves again and 90 s
// later the stuck teleport fires a second time (w52: 49 of MoP's 236 stuck teleports started on a lattice point a
// previous teleport had left the bot on, 46 of them 6+ yd off the navmesh; AC 8 of 65). So the landing point is the
// water or ground surface under the cell, and the cell's z becomes that surface too: GO_GRIND's arrival test is AC's
// 3-D "< 10 yd" (NewRpgAction.cpp GrindArrivalDist), which the raw lattice z would never pass from the ground.
// Camp, quest and every other destination are exact spawn / POI points and keep AC's raw teleport.
static WorldPosition StuckTeleportTarget(Player* bot, NewRpgInfo& info, WorldPosition const& dest)
{
    auto* grind = std::get_if<NewRpgInfo::GoGrind>(&info.data);
    Map const* map = bot->GetMap();
    if (!grind || !map || dest.GetMapId() != bot->GetMapId() || grind->pos.GetMapId() != dest.GetMapId() ||
        std::fabs(grind->pos.GetPositionX() - dest.GetPositionX()) > 0.1f ||
        std::fabs(grind->pos.GetPositionY() - dest.GetPositionY()) > 0.1f)
        return dest;

    float const x = dest.GetPositionX();
    float const y = dest.GetPositionY();
    float const z = dest.GetPositionZ();
    // A flyer's GrindMoveTarget is already on the ground: a surface within 3 yd under the point keeps it.
    float const local = map->GetWaterOrGroundLevel(bot->GetPhaseMask(), x, y, z, nullptr, false, bot->GetCollisionHeight());
    if (local > INVALID_HEIGHT && std::fabs(local - z) <= 3.0f)
        return dest;

    // Lattice rounding is at most 25 yd: take the water or ground surface found from 30 yd above the cell.
    float surface =
        map->GetWaterOrGroundLevel(bot->GetPhaseMask(), x, y, z + 30.0f, nullptr, false, bot->GetCollisionHeight());
    // #732: a surface more than 30 yd under the cell (the cell centre hangs over a drop: w54 Castinon at Hyjal
    // (5000,-2700,1500)) is still ground, and with nothing found from 30 yd above (the cell centre is inside a hill)
    // the top terrain or water surface is - AC's quest-POI z rule (NewRpgAction.cpp DoIncompleteQuest, MAX_HEIGHT).
    // The raw lattice point used to be kept in both cases: the bot hung in the air or in the rock, every path from
    // there failed, and the next stuck teleport started from it (w54: 9 of MoP's 175 started on a raw lattice z,
    // Castinon's 1500 among them). The water level only counts next to a valid terrain height (Map::GetWaterLevel
    // answers 0 when the grid is missing).
    if (surface <= INVALID_HEIGHT)
    {
        float const top = map->GetHeight(bot->GetPhaseMask(), x, y, MAX_HEIGHT);
        if (top > INVALID_HEIGHT)
            surface = std::max(top, map->GetWaterLevel(x, y));
    }
    if (surface <= INVALID_HEIGHT)
        return dest;

    WorldPosition const landing(dest.GetMapId(), x, y, surface + 0.5f, dest.GetOrientation());
    grind->pos = landing;
    return landing;
}

// #814 (beyond AC; the same gap is latent in AC: w52 Navlok, w54 Nore). MoveFarTo paths a flying bot over the ground
// navmesh like a walker. More than 7 yd above the mesh (or 50 yd, or off every polygon) PathGenerator answers a
// flyer with a NOT_USING_PATH shortcut (BuildPolyPath farFromPoly + CanFly) or, on this core, NOPATH for a player,
// and typeOk refuses both; on a roof or peak navmesh island the INCOMPLETE ends never close the gap. Either way the
// flyer hovers until the 90 s stuck teleport: 16 of the 36 probed MoP stuck teleports in w52 and 3 of 14 in w54
// were flying bots, most of them frozen in place for the 2 min before (AC 3 of 16). MoP bots fly from level 60 on
// every continent and in Pandaria (probe IsFlying 8.5% vs AC 2.2%, w54), so MoP pays this far more often. MoveTo
// flies a flyer in a straight line (generatePath = !IsFlying()), so the leg is pathFinderDis straight toward the
// destination, its end kept 3 yd over the terrain / water top there (the AC quest-POI MAX_HEIGHT height rule).
static bool FlyLegTarget(Player* bot, WorldPosition const& dest, float legDis, float& lx, float& ly, float& lz)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();
    float const dist = bot->GetExactDist(dest);
    Map const* map = bot->GetMap();
    if (!map || dist < 1.0f)
        return false;

    float const t = std::min(1.0f, legDis / dist);
    lx = bx + (dest.GetPositionX() - bx) * t;
    ly = by + (dest.GetPositionY() - by) * t;
    lz = bz + (dest.GetPositionZ() - bz) * t;
    float const top = map->GetHeight(bot->GetPhaseMask(), lx, ly, MAX_HEIGHT);
    if (top > INVALID_HEIGHT)
        lz = std::max(lz, std::max(top, map->GetWaterLevel(lx, ly)) + 3.0f);
    return true;
}


// MoP keeps quest progress per QuestObjective, but quest_poi.ObjectiveIndex keeps the 3.3.5 layout that AC's
// GetQuestPOIPosAndObjectiveIdx uses: creature/GO objectives are slots 0-3 and item objectives are
// QUEST_OBJECTIVES_COUNT (4) + item slot. StorageIndex counts within each type (a kill and an item objective can
// both be 0), so item objectives map to 4 + StorageIndex. Without this no item-objective POI ever matched and bots
// dropped every collect-item quest right after accepting it.
static int32 LegacyPoiIndex(QuestObjective const& objective)
{
    return objective.Type == QUEST_OBJECTIVE_ITEM ? 4 + int32(objective.StorageIndex) : int32(objective.StorageIndex);
}

bool NewRpgBaseAction::MoveFarTo(WorldPosition dest)
{
    if (dest == WorldPosition())
        return false;

    // Don't start a ground move while the bot is on a taxi: the MovePoint issued
    // below would fight the active flight spline. Returning true consumes the tick
    // instead of returning false, because the callers of MoveFarTo fall through to
    // MoveRandomNear, which is also a ground move.
    if (bot->IsInFlight())
        return true;

    NewRpgInfo& info = botAI->GetRpgInfo();
    if (dest != info.moveFarPos)
    {
        // clear stuck information if it's a new dest
        info.SetMoveFarTo(dest);
    }

    // #218 (BEYOND AC, MoP only - NewRpgRoamTransport.h): a destination on another continent (only the #218 camp
    // pick makes one; AC never leaves the map) or a trip already running is driven over the travel graph by boat,
    // zeppelin, elevator, tram, flight path or portal. Everything else falls through to AC's code unchanged.
    if (RoamTransport::Enabled())
    {
        bool const crossMap = dest.GetMapId() != bot->GetMapId();
        int32 const trip = RoamTransportMove(dest, crossMap, false);
        if (trip >= 0)
            return trip != 0;
        if (crossMap)
        {
            // No route (or the trip was abandoned): walking cannot reach another map, so drop the objective the way
            // AC drops a status it cannot serve (back to IDLE, a fresh roll on the next status update).
            TownErrand::NoteAbandon(bot);  // #945
            info.ChangeToIdle();
            return true;
        }
    }

    // performance optimization
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
    {
        return false;
    }

    // Let previously committed movement finish before recomputing.
    //
    // MoveTo internally caps its stored delay at maxWaitForMove
    // (default 5s), but a long path (200+ yd routed around a
    // mountain) takes 30+ seconds to walk. After 5s
    // IsWaitingForLastMove returns false and MoveFarTo re-enters.
    // Without this gate, MoveTo would call mm->Clear() and
    // reissue MovePoint from the new bot position - and from a new
    // position mmap's partial-path endpoint often differs, so the
    // bot gets clobbered mid-walk and ends up oscillating
    // around an unreachable destination.
    //
    // If the bot is still actively walking toward its last
    // committed point on the same map, just let the current spline
    // finish. The stuck counter below continues to track real
    // progress toward dest and triggers teleport recovery if the
    // committed paths genuinely aren't closing the gap.
    {
        LastMovement& lastMove = AI_VALUE(LastMovement&, "last movement");
        if (bot->isMoving() && lastMove.lastMoveToMapId == bot->GetMapId())
        {
            float remaining = bot->GetExactDist(lastMove.lastMoveToX, lastMove.lastMoveToY, lastMove.lastMoveToZ);
            if (remaining > 10.0f)
                return true;
        }
    }

    // stuck check
    float disToDest = bot->GetDistance(dest);
    // #218 (BEYOND AC, MoP only): 30 s without progress, try the travel graph once (per destination per 10 min). A
    // route with an elevator / tram / boat / zeppelin / flight / portal leg is followed instead of waiting for AC's
    // 90 s stuck teleport below; a plain walking route changes nothing and AC's check runs as before.
    // #847/#846 (w87): one stuck attempt is enough - a resting city bot ticks rarely and each attempt is a 30-40 s AC walk,
    // so '>= 4' started the trip only after ~250 s although the 30 s no-progress condition was met at ~69 s.
    if (RoamTransport::Enabled() && disToDest + 5.0f >= info.nearestMoveFarDis && info.stuckAttempts >= 1 &&
        GetMSTimeDiffToNow(info.stuckTs) >= 30 * IN_MILLISECONDS)
    {
        int32 const trip = RoamTransportMove(dest, true, true);
        if (trip >= 0)
        {
            info.stuckTs = getMSTime();
            info.stuckAttempts = 0;
            return trip != 0;
        }
    }
    // Require a meaningful improvement (5yd) to reset the stuck counter.
    // The old 1yd threshold was small enough that bots oscillating back
    // and forth around an obstacle would keep "making progress" forever
    // and never trigger the teleport recovery below.
    if (disToDest + 5.0f < info.nearestMoveFarDis)
    {
        info.nearestMoveFarDis = disToDest;
        info.stuckTs = getMSTime();
        info.stuckAttempts = 0;
    }
    else if (++info.stuckAttempts >= 5 && GetMSTimeDiffToNow(info.stuckTs) >= stuckTime)
    {
        // No meaningful progress toward dest for `stuckTime`: fall
        // back to teleporting directly so the bot can get on with
        // its RPG objective instead of oscillating indefinitely.
        info.stuckTs = getMSTime();
        info.stuckAttempts = 0;
        TownErrand::NoteStuckTeleport(bot);  // #945
        // #814: the logged "to" stays the destination (the field counters key on it); the landing z is logged apart,
        // and fly=1 marks a bot that was flying (the field run splits the count by flying vs ground).
        WorldPosition const landing = StuckTeleportTarget(bot, info, dest);
        TC_LOG_DEBUG("playerbots",
            "[New RPG] Teleport %s from (%f,%f,%f,%u) to (%f,%f,%f,%u) as it stuck when moving far - Zone: %u landz=%.1f fly=%d",
            bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId(),
            dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), dest.GetMapId(), bot->GetZoneId(),
            landing.GetPositionZ(), bot->IsFlying() ? 1 : 0);
        bot->RemoveAurasWithInterruptFlags(
            SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP));
        return bot->TeleportTo(landing);
    }

    float dis = bot->GetExactDist(dest);

    // #814 (beyond AC, see FlyLegTarget): one straight flight leg toward dest for a flying bot.
    auto flyLeg = [&](char const* why) -> bool
    {
        float lx = 0.0f, ly = 0.0f, lz = 0.0f;
        if (!FlyLegTarget(bot, dest, pathFinderDis, lx, ly, lz))
            return false;
        if (sPlayerbotAIConfig->debugBotNames.count(bot->GetName()) || BotScenario::WantsTrail(bot))
            TC_LOG_DEBUG("playerbots", "[MOVEFAR] %s fly from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f) (map %u) why=%s",
                         bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), lx, ly,
                         lz, bot->GetMapId(), why);
        return MoveTo(bot->GetMapId(), lx, ly, lz, false, false, false, true);
    };
    // A flyer that made no 5-yd progress for 30 s (it crawls on a roof / peak island) leaves it in a straight line.
    if (bot->IsFlying() && dis >= pathFinderDis && info.stuckAttempts >= 3 &&
        GetMSTimeDiffToNow(info.stuckTs) >= 30 * IN_MILLISECONDS)
        return flyLeg("stuck");

    if (dis < pathFinderDis)
    {
        if (sPlayerbotAIConfig->debugBotNames.count(bot->GetName()) || BotScenario::WantsTrail(bot))
            TC_LOG_DEBUG("playerbots",
                     "[MOVEFAR] %s direct from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f) (map %u)",
                     bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                     dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), bot->GetMapId());
        return MoveTo(dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false, false,
                      false, true);
    }

    const uint32 typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;

    // Primary strategy: ask mmap for a route to the TRUE destination.
    // If mmap can reach it directly (PATHFIND_NORMAL) or partially
    // (PATHFIND_INCOMPLETE - destinations beyond the smooth-path cap,
    // or where local geometry blocks the final step), walk to the furthest
    // reachable waypoint mmap computed. This lets bots follow the real
    // route around obstacles (mountains, cave walls, cliffs) instead of
    // trying to cut straight through. The spline system walks the whole
    // returned path smoothly, so subsequent ticks early-out via
    // IsWaitingForLastMove and no further PathGenerator calls fire until
    // the bot arrives.
    {
        PathGenerator path(bot);
        path.CalculatePath(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ());
        PathType type = path.GetPathType();
        bool canReach = !(type & (~typeOk));
        if (canReach)
        {
            G3D::Vector3 const endPos = ReachableEndPosition(path);
            // Only commit if the mmap endpoint actually makes progress
            // toward the destination. For pathological INCOMPLETE
            // results (e.g. disconnected polys that still report
            // INCOMPLETE) the endpoint can land right under the bot;
            // fall through to cone sampling in that case.
            float endDistToDest = dest.GetExactDist(endPos.x, endPos.y, endPos.z);
            if (endDistToDest + 5.0f < disToDest)
            {
                if (sPlayerbotAIConfig->debugBotNames.count(bot->GetName()) || BotScenario::WantsTrail(bot))
                    TC_LOG_DEBUG("playerbots",
                             "[MOVEFAR] %s commit from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f) type %u points %u (map %u)",
                             bot->GetName().c_str(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                             endPos.x, endPos.y, endPos.z, uint32(type), uint32(path.GetPath().size()), bot->GetMapId());
                return MoveTo(bot->GetMapId(), endPos.x, endPos.y, endPos.z, false, false, false, true);
            }
        }
    }

    // Fallback: mmap couldn't route to the destination. Sample the
    // forward cone for a reachable stepping stone so the bot keeps
    // moving and can try again from a new vantage point. Cap at 2
    // samples - we already spent one PathGenerator call above and at
    // thousands of bots every extra CalculatePath matters.
    float minDelta = M_PI;
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();
    const float baseAngle = bot->GetAngle(&dest);
    float rx, ry, rz;
    bool found = false;
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        float delta = (rand_norm() - 0.5f) * static_cast<float>(M_PI);  // +/-pi/2, forward cone
        float sampleDis = (0.5f + rand_norm() * 0.5f) * pathFinderDis;
        float angle = baseAngle + delta;
        float dx = x + cos(angle) * sampleDis;
        float dy = y + sin(angle) * sampleDis;
        float dz = z + 0.5f;
        PathGenerator path(bot);
        path.CalculatePath(dx, dy, dz);
        PathType type = path.GetPathType();
        bool canReach = !(type & (~typeOk));

        if (canReach && fabs(delta) <= minDelta)
        {
            found = true;
            // #61: a cone sample in the air over a valley
            // gives the same degenerate INCOMPLETE [bot, sample] path as the primary branch above.
            G3D::Vector3 const endPos = ReachableEndPosition(path);
            rx = endPos.x;
            ry = endPos.y;
            rz = endPos.z;
            minDelta = fabs(delta);
        }
    }
    if (found)
    {
        if (sPlayerbotAIConfig->debugBotNames.count(bot->GetName()) || BotScenario::WantsTrail(bot))
            TC_LOG_DEBUG("playerbots", "[MOVEFAR] %s cone from (%.1f,%.1f,%.1f) to (%.1f,%.1f,%.1f) (map %u)",
                         bot->GetName().c_str(), x, y, z, rx, ry, rz, bot->GetMapId());
        return MoveTo(bot->GetMapId(), rx, ry, rz, false, false, false, true);
    }
    // #814: no ground path from where the flyer hovers - fly instead of waiting for the stuck teleport.
    if (bot->IsFlying())
        return flyLeg("nopath");
    return false;
}

bool NewRpgBaseAction::MoveRandomNear(float moveStep, MovementPriority priority)
{
    if (IsWaitingForLastMove(priority))
        return false;

    Map* map = bot->GetMap();
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();

    // Previously: attempts = 1. A single random sample often landed in
    // water / blocked geometry / unreachable poly, the function returned
    // false, and the caller had no fallback - bot stood still. Retry a
    // handful of times with a fresh distance each loop so a bad roll
    // doesn't lock the bot in place.
    float bestHop = 0.0f, bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f;  // longest clamped candidate (#61)
    for (int attempt = 0; attempt < 8; ++attempt)
    {
        float distance, angle;
        // HARNESS-SPEC-botscenario-v2 G3.4: deterministic pick - a golden-angle sweep advanced by
        // the scenario's own pick counter instead of a random angle/distance, so repeated calls are
        // reproducible. No-op (false) unless a deterministic scenario is running for this bot.
        if (!BotScenario::NextRandomNearAngle(bot, moveStep, angle, distance))
        {
            distance = (0.4f + rand_norm() * 0.6f) * moveStep;
            angle = (float)rand_norm() * 2 * static_cast<float>(M_PI);
        }
        float dx = x + distance * cos(angle);
        float dy = y + distance * sin(angle);
        float dz = z;

        PathGenerator path(bot);
        path.CalculatePath(dx, dy, dz);
        PathType type = path.GetPathType();
        uint32 typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_FARFROMPOLY;
        bool canReach = !(type & (~typeOk));

        if (!canReach)
            continue;

        if (!map->CanReachPositionAndGetValidCoords(bot, dx, dy, dz))
            continue;

        if (map->IsInWater(bot->GetPhaseMask(), dx, dy, dz))
            continue;

        // #61: CanReachPositionAndGetValidCoords clamps the target at the first navmesh wall, so in a
        // pocket every direction can come back as a 0-2 yd hop and the first one used to win. Keep a
        // clamped candidate only as a fallback and prefer one that keeps at least 0.4 x moveStep (the
        // shortest distance the random pick itself would choose). Shared latent bug, fixed identically
        // on both cores.
        float const hop = std::sqrt((dx - x) * (dx - x) + (dy - y) * (dy - y));
        if (hop < 0.4f * moveStep)
        {
            if (hop > bestHop)
            {
                bestHop = hop;
                bestX = dx;
                bestY = dy;
                bestZ = dz;
            }
            continue;
        }

        bool moved = MoveTo(bot->GetMapId(), dx, dy, dz, false, false, false, true, priority);
        if (moved)
        {
            BotScenario::LogTarget(bot, "rnd", nullptr, dx, dy, dz, moveStep);
            return true;
        }
    }

    if (bestHop > 0.0f && MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, true, priority))
    {
        BotScenario::LogTarget(bot, "rnd", nullptr, bestX, bestY, bestZ, moveStep);
        return true;
    }

    // #759 / #814 (beyond AC; latent in AC too, see FlyLegTarget): a flyer hovering more than 7 yd over the navmesh
    // gets only refused samples above (NOT_USING_PATH / NOPATH), so WANDER_RANDOM left it in the air until the status
    // timed out. Fly it straight down to the ground or water surface
    // under it (MoveTo flies a flyer in a straight line); from there the samples find the mesh again.
    if (bot->IsFlying() && map)
    {
        float const ground =
            map->GetWaterOrGroundLevel(bot->GetPhaseMask(), x, y, z, nullptr, false, bot->GetCollisionHeight());
        if (ground > INVALID_HEIGHT && z - ground > 3.0f &&
            MoveTo(bot->GetMapId(), x, y, ground + 0.5f, false, false, false, true, priority))
        {
            BotScenario::LogTarget(bot, "rnd", nullptr, x, y, ground + 0.5f, moveStep);
            return true;
        }
    }

    return false;
}

bool NewRpgBaseAction::ForceToWait(uint32 duration, MovementPriority priority)
{
    AI_VALUE(LastMovement&, "last movement")
        .Set(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetOrientation(),
             duration, priority);
    return true;
}

/* ---------------------------------------------------------------------------------------------
 * NPC / quest interaction (ported from AC NewRpgBaseAction.cpp)
 *
 * This is what makes bots walk NPC to NPC, take and hand in quests, and head for quest
 * objectives, rather than only wandering. Two deliberate differences from upstream:
 *
 *  - Upstream drives quest accept / reward / abandon by hand-building CMSG packets and feeding
 *    them to the session handlers. Those packets are bit-packed on 5.4.8 (see
 *    QuestHandler.cpp), and CMSG_QUESTGIVER_CHOOSE_REWARD now carries an *item id* rather than a
 *    choice index. Calling the Player methods the handlers themselves call is both faithful in
 *    effect and immune to packet-layout drift.
 *  - MoP replaced the fixed RequiredNpcOrGo/RequiredItem arrays with the QuestObjective list, so
 *    objective progress is read through Player::IsQuestObjectiveComplete and the POI blobs are
 *    matched on QuestObjective::StorageIndex.
 * --------------------------------------------------------------------------------------------- */

bool NewRpgBaseAction::MoveWorldObjectTo(ObjectGuid guid, float distance)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return false;

    WorldObject* object = botAI->GetWorldObject(guid);
    if (!object)
        return false;

    float x = object->GetPositionX();
    float y = object->GetPositionY();
    float z = object->GetPositionZ();
    uint32 mapId = object->GetMapId();
    float angle = 0.f;

    if (!object->ToUnit() || !object->ToUnit()->isMoving())
        angle = object->GetAngle(bot) + (static_cast<float>(M_PI) * irand(-25, 25) / 100.0f);  // towards the target
    else
        angle = object->GetOrientation() +
                (static_cast<float>(M_PI) * irand(-25, 25) / 100.0f);  // in front of a moving target

    float rnd = rand_norm();
    x += cos(angle) * distance * rnd;
    y += sin(angle) * distance * rnd;

    // Source must be a Unit: Map::CheckCollisionAndGetValidCoords feeds it to PathGenerator, which
    // dereferences _sourceUnit. Passing a quest GameObject here crashed the worldserver (issue #62).
    if (!object->GetMap()->CheckCollisionAndGetValidCoords(bot, object->GetPositionX(), object->GetPositionY(),
                                                           object->GetPositionZ(), x, y, z))
    {
        x = object->GetPositionX();
        y = object->GetPositionY();
        z = object->GetPositionZ();
    }

    return MoveTo(mapId, x, y, z, false, false, false, true);
}

bool NewRpgBaseAction::InteractWithNpcOrGameObjectForQuest(ObjectGuid guid)
{
    WorldObject* object = ObjectAccessor::GetWorldObject(*bot, guid);
    if (!object || !bot->CanInteractWithQuestGiver(object))
        return false;

    bot->PrepareQuestMenu(guid);
    QuestMenu const& menu = bot->PlayerTalkClass->GetQuestMenu();
    if (menu.Empty())
        return true;

    bool accepted = false;  // #872
    for (uint32 idx = 0; idx < menu.GetMenuItemCount(); ++idx)
    {
        QuestMenuItem const& item = menu.GetItem(idx);
        Quest const* quest = sObjectMgr->GetQuestTemplate(item.QuestId);
        if (!quest)
            continue;

        QuestStatus const status = bot->GetQuestStatus(item.QuestId);

        if (status == QUEST_STATUS_NONE && bot->CanTakeQuest(quest, false) && bot->CanAddQuest(quest, false) &&
            IsQuestWorthDoing(quest) && IsQuestCapableDoing(quest))
        {
            accepted |= AcceptQuest(quest, guid);
            BroadcastHelper::BroadcastQuestAccepted(botAI, bot, quest);  // #322, AC NewRpgBaseAction.cpp:369
            botAI->GetRpgStatistic().questAccepted++;
            TC_LOG_DEBUG("playerbots", "[New RPG] %s accept quest %u", bot->GetName().c_str(), quest->GetQuestId());
        }

        if (status == QUEST_STATUS_COMPLETE && CanRewardQuestAC(bot, quest))
        {
            TurnInQuest(quest, guid);
            BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);  // #322, AC NewRpgBaseAction.cpp:381
            botAI->GetRpgStatistic().questRewarded++;
            TC_LOG_DEBUG("playerbots", "[New RPG] %s turned in quest %u", bot->GetName().c_str(), quest->GetQuestId());
        }
    }

    // #872, fix inside the #843 extension (BEYOND AC): a quest hub trip exists only to put a doable quest in the log.
    // Once the bot has taken one on that trip, end it at IDLE so AC's next draw sees DO_QUEST through AC's own check
    // (CheckRpgStatusAvailable). Before, the trip ran on as AC's GO_CAMP -> WANDER_NPC and the bot wandered the hub's
    // NPCs for statusWanderNpcDuration with the quest already in its log (w76: 5.8 % of all MoP time, npc_pp rows).
    // The trip flag is set only by the hub pick and cleared at every IDLE draw, so plain AC GO_CAMP / WANDER_NPC
    // statuses never end here.
    if (accepted)
    {
        NewRpgInfo& info = botAI->GetRpgInfo();
        NewRpgStatus const rpgStatus = info.GetStatus();
        if ((rpgStatus == RPG_GO_CAMP || rpgStatus == RPG_WANDER_NPC) && !BotScenario::IsStatusLocked(bot) &&
            QuestHubSearch::TakeTrip(bot))
        {
            TC_LOG_DEBUG("playerbots", "[New RPG] %s quest hub trip done (status %d) -> idle", bot->GetName().c_str(),
                         int(rpgStatus));
            info.ChangeToIdle();
        }
    }

    return true;
}

bool NewRpgBaseAction::CanInteractWithQuestGiver(Object* questGiver)
{
    // A variant of Player::CanInteractWithQuestGiver with the distance check removed and every
    // other check kept, so a bot can decide to walk to a quest giver it cannot yet reach.
    if (!questGiver)
        return false;

    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            ObjectGuid guid = questGiver->GetGUID();
            if (!guid)
                return false;

            if (!bot->IsInWorld() || bot->IsDuringRemoveFromWorld())
                return false;

            if (bot->IsInFlight())
                return false;

            Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*bot, guid);
            if (!creature)
                return false;

            // Deathstate checks
            if (!bot->IsAlive() &&
                !(creature->GetCreatureTemplate()->type_flags & CREATURE_TYPEFLAGS_GHOST))
                return false;

            if (!creature->IsAlive() &&
                !(creature->GetCreatureTemplate()->type_flags & CREATURE_TYPEFLAGS_DEAD_INTERACT))
                return false;

            if (!creature->HasFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_QUESTGIVER))
                return false;

            // not allow interaction under control
            if (creature->GetCharmerGUID())
                return false;

            if (creature->GetReactionTo(bot) <= REP_UNFRIENDLY)
                return false;

            return true;
        }
        case TYPEID_GAMEOBJECT:
        {
            if (GameObject* go = bot->GetMap()->GetGameObject(questGiver->GetGUID()))
            {
                if (go->GetGoType() == GAMEOBJECT_TYPE_QUESTGIVER)
                {
                    // Players cannot interact with gameobjects that use the "Point" icon
                    if (go->GetGOInfo()->IconName == "Point")
                        return false;

                    return true;
                }
            }

            return false;
        }
        default:
            break;
    }

    return false;
}

bool NewRpgBaseAction::IsWithinInteractionDist(Object* questGiver)
{
    // The other half of Player::CanInteractWithQuestGiver: only the distance check.
    if (!questGiver)
        return false;

    switch (questGiver->GetTypeId())
    {
        case TYPEID_UNIT:
        {
            ObjectGuid guid = questGiver->GetGUID();
            if (!guid)
                return false;

            Creature* creature = ObjectAccessor::GetCreatureOrPetOrVehicle(*bot, guid);
            if (!creature)
                return false;

            return creature->IsWithinDistInMap(bot, INTERACTION_DISTANCE);
        }
        case TYPEID_GAMEOBJECT:
        {
            if (GameObject* go = bot->GetMap()->GetGameObject(questGiver->GetGUID()))
                return go->IsWithinDistInMap(bot, INTERACTION_DISTANCE);

            return false;
        }
        default:
            break;
    }

    return false;
}

bool NewRpgBaseAction::AcceptQuest(Quest const* quest, ObjectGuid guid)
{
    if (!quest)
        return false;

    Object* questGiver = ObjectAccessor::GetWorldObject(*bot, guid);
    if (!questGiver)
        return false;

    bot->AddQuestAndCheckCompletion(quest, questGiver);
    return true;
}

bool NewRpgBaseAction::TurnInQuest(Quest const* quest, ObjectGuid guid)
{
    if (!quest)
        return false;

    uint32 const questId = quest->GetQuestId();

    if (bot->GetQuestRewardStatus(questId))
        return false;

    if (!bot->CanRewardQuest(quest, false))
        return false;

    Object* questGiver = ObjectAccessor::GetWorldObject(*bot, guid);
    if (!questGiver)
        return false;

    // On 5.4.8 the reward is identified by item id, not by choice index.
    uint32 rewardItemId = 0;
    if (quest->GetRewChoiceItemsCount() > 0)
    {
        uint32 bestIndex = BestRewardIndex(quest);
        rewardItemId = quest->RewardChoiceItemId[bestIndex];
    }

    if (!bot->CanRewardQuest(quest, rewardItemId, false))
        return false;

    bot->PlayDistanceSound(621);
    bot->RewardQuest(quest, rewardItemId, questGiver);
    return true;
}

uint32 NewRpgBaseAction::BestRewardIndex(Quest const* quest)
{
    if (!quest || quest->GetRewChoiceItemsCount() <= 1)
        return 0;

    // #531: AC NewRpgBaseAction.cpp:556-592 1:1. The issue #62 item-level stand-in is gone now that
    // "item usage" (strategy/value/ItemUsageValue.cpp) and StatsWeightCalculator (#188) exist. The
    // "item usage" qualifier is the item id as text - the fork's convention, as in
    // TalkToQuestGiverAction::BestRewards.
    ItemUsage bestUsage = ITEM_USAGE_NONE;
    for (uint32 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
    {
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", std::to_string(quest->RewardChoiceItemId[i]));
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
            bestUsage = ITEM_USAGE_EQUIP;
        else if (usage == ITEM_USAGE_BAD_EQUIP && bestUsage != ITEM_USAGE_EQUIP)
            bestUsage = usage;
        else if (usage != ITEM_USAGE_NONE && bestUsage == ITEM_USAGE_NONE)
            bestUsage = usage;
    }

    StatsWeightCalculator calc(bot);
    uint32 best = 0;
    float bestScore = 0;
    for (uint32 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
    {
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", std::to_string(quest->RewardChoiceItemId[i]));
        if (usage == bestUsage || usage == ITEM_USAGE_REPLACE)
        {
            float score = calc.CalculateItem(quest->RewardChoiceItemId[i]);
            if (score > bestScore)
            {
                bestScore = score;
                best = i;
            }
        }
    }

    return best;
}

bool NewRpgBaseAction::IsQuestWorthDoing(Quest const* quest)
{
    if (!quest)
        return false;

    // taxi-quest-stage-design.md G-Q1 / AC parity (AC NewRpgBaseAction.cpp:597): use the
    // player-adjusted quest level (Player::GetQuestLevel, Player.h:1697 - falls back to the bot's own
    // level when the quest scales, GetQuestLevel() < 0) instead of the raw template level. 1956 MoP
    // quests have QuestLevel == -1; with the raw value every one of them read as "quest level -1",
    // which is always <= bot level - LowLevelHideDiff and therefore never worth doing.
    bool isLowLevelQuest =
        int32(bot->GetLevel()) > (bot->GetQuestLevel(quest) + sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF));

    if (isLowLevelQuest)
        return false;

    if (quest->IsRepeatable())
        return false;

    if (quest->IsSeasonal())
        return false;

    return true;
}

bool NewRpgBaseAction::IsQuestCapableDoing(Quest const* quest)
{
    if (!quest)
        return false;

    // G-Q1 / AC parity (AC NewRpgBaseAction.cpp:613): same player-adjusted level as IsQuestWorthDoing.
    if (int32(bot->GetLevel()) + 3 < bot->GetQuestLevel(quest))
        return false;

    // NOTE (issue #62): do NOT test QuestType here. On 5.4.8 QuestType is a UI grouping
    // (0 turn-in, 1 with-max-level, 2 NORMAL, 3 task, 4/5 in-progress), not the WotLK
    // normal/elite/dungeon/pvp enum. "GetQuestType() != 0" therefore rejected 16077 of the 18028
    // quests in the world DB - every normal quest - so no bot ever took one (measured: quest
    // 25452 at level 36 came back worth=1 capable=0). Group content is filtered by suggested
    // players instead.
    // AC parity: AC's "quest->GetType() != 0" reads the QuestInfoID column (quest_template column 5 on
    // AC), i.e. group/dungeon/raid/pvp/escort/heroic quests. The same column exists on 5.4.8 (plus
    // scenario 98 / account 102), so the elite/dungeon filter is kept through GetQuestInfoID().
    if (quest->GetQuestInfoID() != 0)
        return false;

    if (quest->GetSuggestedPlayers() >= 2)
        return false;

    return true;
}

bool NewRpgBaseAction::OrganizeQuestLog()
{
    int32 freeSlotNum = 0;
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
        if (!bot->GetQuestSlotQuestId(i))
            freeSlotNum++;

    // it's ok if we have two more free slots
    if (freeSlotNum >= 2)
        return false;

    // Mirrors WorldSession::HandleQuestLogRemoveQuest (QuestHandler.cpp) rather than sending the
    // packet, so a dropped quest leaves no timed quest, quest items or achievement timer behind.
    auto dropQuest = [this](uint16 slot, uint32 questId)
    {
        TC_LOG_DEBUG("playerbots", "[New RPG] %s drop quest %u", bot->GetName().c_str(), questId);

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        QuestStatus oldStatus = bot->GetQuestStatus(questId);

        if (quest)
        {
            if (quest->HasSpecialFlag(QUEST_SPECIAL_FLAGS_TIMED))
                bot->RemoveTimedQuest(questId);

            if (quest->HasFlag(QUEST_FLAGS_FLAGS_PVP))
            {
                bot->pvpInfo.IsHostile = bot->pvpInfo.IsInHostileArea || bot->HasPvPForcingQuest();
                bot->UpdatePvPState();
            }
        }

        bot->DestroyQuestItems(questId);
        bot->RemoveActiveQuest(questId);
        bot->RemoveTimedAchievement(CRITERIA_START_TYPE_QUEST, questId);

        if (quest)
            sScriptMgr->OnQuestStatusChange(bot, quest, oldStatus, QUEST_STATUS_NONE);

        bot->SetQuestSlot(slot, 0);
        bot->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_QUEST_ABANDONED, 1);

        botAI->GetRpgStatistic().questDropped++;
    };

    int32 dropped = 0;

    // Quests that are not worth doing, not capable of doing, or already failed.
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = bot->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        if (!IsQuestWorthDoing(quest) || !IsQuestCapableDoing(quest) ||
            bot->GetQuestStatus(questId) == QUEST_STATUS_FAILED)
        {
            dropQuest(i, questId);
            dropped++;
        }
    }

    // drop at most 8 quests at once to avoid repeated accept and drop
    if (dropped >= 8)
        return true;

    // Festival/class quests and quests belonging to another zone.
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = bot->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        int32 const zoneOrSort = quest->GetZoneOrSort();
        if (zoneOrSort < 0 || (zoneOrSort > 0 && uint32(zoneOrSort) != bot->GetZoneId()))
        {
            dropQuest(i, questId);
            dropped++;
        }
    }

    if (dropped >= 8)
        return true;

    // Still full: clear the log.
    for (uint16 i = 0; i < MAX_QUEST_LOG_SIZE; ++i)
    {
        uint32 questId = bot->GetQuestSlotQuestId(i);
        if (!questId)
            continue;

        dropQuest(i, questId);
    }

    return true;
}

bool NewRpgBaseAction::SearchQuestGiverAndAcceptOrReward()
{
    // #975 (BEYOND AC, the #890 rule for every caller): no quest-giver detour while the bot's roam trip holds it for a
    // transport leg. #890 gated only GO_GRIND / GO_CAMP (NewRpgAction.cpp:513,533); DO_QUEST (NewRpgAction.cpp:927) and
    // WANDER_RANDOM / WANDER_NPC (794,802) still ran it first. Run mop1k-v10-1028: Guine and Peragon (DO_QUEST 11725,
    // POI (3522,5016)) waited for elevator 152614 on the Fizzcrank tower top; every tick the detour picked a giver
    // below (Abner Fizzletorque 25780 at (4125.4,5281.3,25.2), 48 yd west, 13 yd down: Brougos took his 11713 the
    // moment he reached the ground in v8) and MoveWorldObjectTo walked the bot to the island point nearest it, the west
    // corner (4172.5,5279.7,40.6). That cancelled the four standby walks and, 6 s after the trip's stuck-teleport onto
    // the standby point (4181.9,5281.3,39.6), had the bot back on the corner ("abandon dock-approach-stall", t=36 s /
    // 31 s). Estanza (GO_CAMP to the Valiance Keep inn, gated) boarded from the same corner in the same run; every
    // corner trip that never boarded in v8/v9/v10 (Brougos, Badeon, Estanza in v9, Guine, Peragon) was a DO_QUEST trip
    // to (3522,5016), all of them sent up there by the quest hub search ("giver 25590 quest 11725 dist 1"). AC has no
    // roam trip: outside a trip this changes nothing.
    if (RoamTransport::HoldsForTransportLeg(bot))
        return false;

    OrganizeQuestLog();

    if (ObjectGuid npcOrGo = ChooseNpcOrGameObjectToInteract(true, 80.0f))
    {
        WorldObject* object = ObjectAccessor::GetWorldObject(*bot, npcOrGo);

        // issue #61: .botscenario wander/free-mode target trace. BotScenario::LogTarget de-dupes
        // kind=qg on the target's guid itself, so staying at the same quest giver across several
        // ticks logs it once, not on every Execute().
        if (object)
            BotScenario::LogTarget(bot, "qg", object, object->GetPositionX(), object->GetPositionY(), object->GetPositionZ());

        if (object && bot->CanInteractWithQuestGiver(object))
        {
            InteractWithNpcOrGameObjectForQuest(npcOrGo);
            ForceToWait(5000);
            return true;
        }

        return MoveWorldObjectTo(npcOrGo);
    }

    return false;
}

ObjectGuid NewRpgBaseAction::ChooseNpcOrGameObjectToInteract(bool questgiverOnly, float distanceLimit)
{
    GuidVector possibleTargets = AI_VALUE(GuidVector, "possible new rpg targets");
    GuidVector possibleGameObjects = AI_VALUE(GuidVector, "possible new rpg game objects");

    if (possibleTargets.empty() && possibleGameObjects.empty())
        return ObjectGuid::Empty;

    WorldObject* nearestObject = nullptr;

    // taxi-quest-stage-design.md B.3 item 3: a .botscenario quest stage started with "pick=nearest"
    // scans the whole list instead of stopping at the first quest giver in list order, so the bot
    // takes the true nearest one - the harness's deterministic stand-in for AC's random giver-visit
    // order (AC ChooseNpcOrGameObjectToInteract, NewRpgBaseAction.cpp:779/797, has the same `break`).
    bool const pickNearest = BotScenario::PickNearest(bot);

    auto considerAll = [&](GuidVector const& guids)
    {
        for (ObjectGuid const& guid : guids)
        {
            WorldObject* object = ObjectAccessor::GetWorldObject(*bot, guid);

            if (!object || !object->IsInWorld())
                continue;

            if (distanceLimit && bot->GetDistance(object) > distanceLimit)
                continue;

            if (CanInteractWithQuestGiver(object) && HasQuestToAcceptOrReward(object))
            {
                if (!nearestObject || bot->GetExactDist(nearestObject) > bot->GetExactDist(object))
                    nearestObject = object;
                if (!pickNearest)
                    break;
            }
        }
    };

    considerAll(possibleTargets);
    considerAll(possibleGameObjects);

    if (nearestObject)
        return nearestObject->GetGUID();

    // No quest giver with anything to offer.
    if (questgiverOnly)
        return ObjectGuid::Empty;

    if (possibleTargets.empty())
        return ObjectGuid::Empty;

    // HARNESS-SPEC-botscenario-v2 G3.3: deterministic pick for this non-quest-giver random branch -
    // take the nearest candidate instead of a random one. possibleTargets isn't distance-sorted, so
    // this is a plain min-distance scan.
    if (BotScenario::PickNearest(bot))
    {
        WorldObject* nearest = nullptr;
        for (ObjectGuid const& guid : possibleTargets)
        {
            WorldObject* candidate = ObjectAccessor::GetCreatureOrPetOrVehicle(*bot, guid);
            if (!candidate)
                candidate = ObjectAccessor::GetGameObject(*bot, guid);

            if (!candidate || !candidate->IsInWorld())
                continue;

            if (!nearest || bot->GetExactDist(candidate) < bot->GetExactDist(nearest))
                nearest = candidate;
        }

        return nearest ? nearest->GetGUID() : ObjectGuid::Empty;
    }

    ObjectGuid guid = possibleTargets[urand(0, possibleTargets.size() - 1)];
    WorldObject* object = ObjectAccessor::GetCreatureOrPetOrVehicle(*bot, guid);
    if (!object)
        object = ObjectAccessor::GetGameObject(*bot, guid);

    if (object && object->IsInWorld())
        return object->GetGUID();

    return ObjectGuid::Empty;
}

bool NewRpgBaseAction::HasQuestToAcceptOrReward(WorldObject* object)
{
    if (!object)
        return false;

    ObjectGuid guid = object->GetGUID();
    bot->PrepareQuestMenu(guid);

    QuestMenu const& menu = bot->PlayerTalkClass->GetQuestMenu();
    if (menu.Empty())
        return false;

    bool const debugBot = !sPlayerbotAIConfig->debugBotNames.empty() && sPlayerbotAIConfig->debugBotNames.count(bot->GetName());
    if (debugBot)
        TC_LOG_DEBUG("playerbots", "[QUEST] %s giver=%s menu=%u", bot->GetName().c_str(), object->GetName().c_str(),
                     menu.GetMenuItemCount());

    // Rewards first: handing in is always preferable to taking another quest.
    for (uint32 idx = 0; idx < menu.GetMenuItemCount(); ++idx)
    {
        QuestMenuItem const& item = menu.GetItem(idx);
        Quest const* quest = sObjectMgr->GetQuestTemplate(item.QuestId);
        if (!quest)
            continue;

        if (bot->GetQuestStatus(item.QuestId) == QUEST_STATUS_COMPLETE && CanRewardQuestAC(bot, quest))
            return true;
    }

    for (uint32 idx = 0; idx < menu.GetMenuItemCount(); ++idx)
    {
        QuestMenuItem const& item = menu.GetItem(idx);
        Quest const* quest = sObjectMgr->GetQuestTemplate(item.QuestId);
        if (!quest)
            continue;

        // #72 perf: the #62 [QUEST] diagnostic below used to evaluate all four checks for every menu entry of every
        // nearby quest giver, for every bot, before the status test - CanTakeQuest/CanAddQuest walk the quest's
        // conditions, prerequisites, skill and bag space even for quests the bot already has or has done. AC
        // (Ai/World/Rpg/Action/NewRpgBaseAction.cpp:883-884) short-circuits on QUEST_STATUS_NONE first; so does this
        // now, and the four values are only computed for the watched DebugBotNames bots.
        if (debugBot)
            TC_LOG_DEBUG("playerbots", "[QUEST] %s quest=%u status=%d take=%d add=%d worth=%d capable=%d",
                         bot->GetName().c_str(), item.QuestId, int(bot->GetQuestStatus(item.QuestId)),
                         bot->CanTakeQuest(quest, false) ? 1 : 0, bot->CanAddQuest(quest, false) ? 1 : 0,
                         IsQuestWorthDoing(quest) ? 1 : 0, IsQuestCapableDoing(quest) ? 1 : 0);

        if (bot->GetQuestStatus(item.QuestId) == QUEST_STATUS_NONE && bot->CanTakeQuest(quest, false) &&
            bot->CanAddQuest(quest, false) && IsQuestWorthDoing(quest) && IsQuestCapableDoing(quest))
            return true;
    }

    return false;
}

namespace
{
    /// Random convex weights, so repeated calls spread bots over a POI area instead of stacking
    /// them all on its centroid.
    std::vector<float> GenerateRandomWeights(size_t n)
    {
        std::vector<float> weights(n);
        float sum = 0.0f;

        for (size_t i = 0; i < n; ++i)
        {
            weights[i] = float(rand_norm());
            sum += weights[i];
        }

        if (sum <= 0.0f)
            return std::vector<float>(n, 1.0f / float(n));

        for (size_t i = 0; i < n; ++i)
            weights[i] /= sum;

        return weights;
    }
}

bool NewRpgBaseAction::GetQuestPOIPosAndObjectiveIdx(uint32 questId, std::vector<POIInfo>& poiInfo, bool toComplete)
{
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return false;

    QuestPOIData const* poiData = sObjectMgr->GetQuestPOIData(questId);
    if (!poiData)
        return false;

    QuestStatus const status = bot->GetQuestStatus(questId);

    // Turn the blob's points into one position, and reject it if it is far away, off this zone,
    // or has no usable height.
    auto blobPosition = [this](QuestPOIBlobData const& blob, float& dx, float& dy) -> bool
    {
        if (blob.MapId != bot->GetMapId() || blob.Points.empty())
            return false;

        dx = 0.f;
        dy = 0.f;

        // issue #105: quest_poi_points can carry the INT32_MIN sentinel for a point that must not be
        // used (quest 24589 "Lord Jaraxxus Must Die!" has one). Averaging it in produces a position
        // outside the world, and any grid lookup on that position used to crash the worldserver.
        constexpr float worldLimit = 17066.0f;
        uint32 usedPoints = 0;

        // taxi-quest-stage-design.md B.3 item 3: a .botscenario quest stage started with
        // "pick=nearest" uses equal weights - the blob centroid - instead of a random convex
        // combination, so the harness POI point is deterministic (AC GetQuestPOIPosAndObjectiveIdx,
        // NewRpgBaseAction.cpp:929/1001, has the same random draw otherwise).
        std::vector<float> weights = BotScenario::PickNearest(bot)
            ? std::vector<float>(blob.Points.size(), 1.0f / float(blob.Points.size()))
            : GenerateRandomWeights(blob.Points.size());
        for (size_t i = 0; i < blob.Points.size(); ++i)
        {
            float const px = float(blob.Points[i].X);
            float const py = float(blob.Points[i].Y);
            if (std::fabs(px) > worldLimit || std::fabs(py) > worldLimit)
                continue;

            dx += px * weights[i];
            dy += py * weights[i];
            ++usedPoints;
        }

        if (!usedPoints)
            return false;

        if (bot->GetDistance2d(dx, dy) >= 1500.0f)
            return false;

        float dz = std::max(bot->GetMap()->GetHeight(bot->GetPhaseMask(), dx, dy, MAX_HEIGHT),
                            bot->GetMap()->GetWaterLevel(dx, dy));

        if (dz == INVALID_HEIGHT || dz == VMAP_INVALID_HEIGHT_VALUE)
            return false;

        if (bot->GetZoneId() != bot->GetMap()->GetZoneId(bot->GetPhaseMask(), dx, dy, dz))
            return false;

        return true;
    };

    if (toComplete && status == QUEST_STATUS_COMPLETE)
    {
        for (QuestPOIBlobData const& blob : poiData->Blobs)
        {
            // ObjectiveIndex -1 marks the hand-in location.
            if (blob.ObjectiveIndex != -1)
                continue;

            float dx = 0.f, dy = 0.f;
            if (!blobPosition(blob, dx, dy))
                continue;

            poiInfo.push_back({{dx, dy}, blob.ObjectiveIndex});
        }

        return !poiInfo.empty();
    }

    if (status != QUEST_STATUS_INCOMPLETE)
        return false;

    // MoP tracks progress per QuestObjective rather than through the 3.3.5 RequiredNpcOrGo /
    // RequiredItem arrays; the POI blobs key off the objective's StorageIndex.
    std::vector<int32> incompleteObjectiveIdx;
    for (QuestObjective const& objective : quest->GetObjectives())
    {
        if (objective.StorageIndex < 0)
            continue;

        if (!bot->IsQuestObjectiveComplete(quest, objective))
            incompleteObjectiveIdx.push_back(LegacyPoiIndex(objective));
    }

    if (incompleteObjectiveIdx.empty())
        return false;

    for (QuestPOIBlobData const& blob : poiData->Blobs)
    {
        if (std::find(incompleteObjectiveIdx.begin(), incompleteObjectiveIdx.end(), blob.ObjectiveIndex) ==
            incompleteObjectiveIdx.end())
            continue;

        float dx = 0.f, dy = 0.f;
        if (!blobPosition(blob, dx, dy))
            continue;

        poiInfo.push_back({{dx, dy}, blob.ObjectiveIndex});
    }

    return !poiInfo.empty();
}

bool NewRpgBaseAction::SelectRandomFlightTaxiNode(uint32& flightMasterEntry, WorldPosition& flightMasterPos,
                                                  std::vector<uint32>& path)
{
    TravelMgr::FlightMasterInfo const* info = sTravelMgr.GetNearestFlightMasterInfo(bot);
    if (!info)
        return false;

    std::vector<std::vector<uint32>> availablePaths = sTravelMgr.GetOptimalFlightDestinations(bot);
    if (availablePaths.empty())
        return false;

    flightMasterEntry = info->templateEntry;
    flightMasterPos = info->pos;
    path = availablePaths[urand(0, availablePaths.size() - 1)];

    TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s select random flight taxi node from:%u (node %u) to:%u (%u available)",
                 bot->GetName().c_str(), flightMasterEntry, path.front(), path.back(), uint32(availablePaths.size()));
    return true;
}

WorldPosition NewRpgBaseAction::SelectRandomGrindPos(Player* bot)
{
    // issue #97 guard, kept: unreachable outside a battleground, and "new rpg" is never on in one.
    if (bot->InBattleground())
        return WorldPosition();

    // Test harness: a scenario with "grind=x,y,z" sends both cores to the same point.
    WorldPosition scenarioPoint;
    if (BotScenario::GrindPoint(bot, scenarioPoint))
        return scenarioPoint;

    std::vector<WorldLocation> const& locs = sTravelMgr.GetLocsPerLevelCache(bot->GetLevel());
    float hiRange = 500.0f;
    float loRange = 2500.0f;
    if (bot->GetLevel() < 5)
    {
        hiRange /= 3;
        loRange /= 3;
    }
    std::vector<WorldLocation> lo_prepared_locs, hi_prepared_locs;

    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->Flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    for (auto& loc : locs)
    {
        if (bot->GetMapId() != loc.GetMapId())
            continue;

        if (bot->GetExactDist(loc) > 2500.0f)
            continue;

        if (!inCity && bot->GetMap()->GetZoneId(bot->GetPhaseMask(), loc.GetPositionX(), loc.GetPositionY(),
                                                loc.GetPositionZ()) != bot->GetZoneId())
            continue;

        // #707, MoP-only divergence (Cataclysm content): AC's grind-cell pick (Ai/World/Rpg/Action/NewRpgBaseAction.cpp
        // SelectRandomGrindPos) has no liquid check because 3.3.5 has no seabed mob camps. Cataclysm added them (Tanaris
        // sunken ships, Vashj'ir): "select random grind pos Map:1 X:-6050 Y:-3750 Z:-50" sent Talaster to the sea floor,
        // where it fought Southsea Treasure Hunters it could never reach for the rest of the owner session (3628 Judgment
        // casts failing OUT_OF_RANGE). The filter is AC's own one for the same cache in RandomTeleportByLocations
        // (Bot/RandomPlayerbotMgr.cpp:1692 `if (map->IsInWater(...)) continue;`; this fork's copy is
        // RandomPlayerbotMgr.cpp:604).
        if (bot->GetMap()->IsInWater(bot->GetPhaseMask(), loc.GetPositionX(), loc.GetPositionY(), loc.GetPositionZ()))
            continue;

        if (bot->GetExactDist(loc) < hiRange)
            hi_prepared_locs.push_back(loc);

        if (bot->GetExactDist(loc) < loRange)
            lo_prepared_locs.push_back(loc);
    }
    // HARNESS-SPEC-botscenario-v2 G3.1: deterministic pick - nearest of lo_prepared_locs, skipping
    // the 50/50 hi/lo roll.
    bool const pickNearest = BotScenario::PickNearest(bot);

    WorldPosition dest{};
    if (pickNearest)
    {
        for (WorldLocation const& loc : lo_prepared_locs)
            if (dest == WorldPosition() || bot->GetExactDist(loc) < bot->GetExactDist(dest))
                dest = loc;
    }
    else if (urand(1, 100) <= 50 && !hi_prepared_locs.empty())
        dest = hi_prepared_locs[urand(0, hi_prepared_locs.size() - 1)];
    else if (!lo_prepared_locs.empty())
        dest = lo_prepared_locs[urand(0, lo_prepared_locs.size() - 1)];

    TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s select random grind pos Map:%u X:%f Y:%f Z:%f (%u+%u available in %u)",
                 bot->GetName().c_str(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
                 uint32(hi_prepared_locs.size()), uint32(lo_prepared_locs.size() - hi_prepared_locs.size()),
                 uint32(locs.size()));
    return dest;
}

WorldPosition NewRpgBaseAction::SelectRandomCampPos(Player* bot)
{
    // AC parity (issue #61): AC NewRpgBaseAction.cpp:1051-1092. Camp destinations now come from
    // TravelMgr's innkeeper/flight-master hub cache instead of the RandomPlayerbotMgr city cache, so
    // the issue #86 "20% chance of a city on another continent" cross-map camp trip and the issue #96
    // enemy-capital guard (the faction filter on TravelMgr's hub build already excludes hostile NPCs)
    // are both dropped, matching AC exactly.
    if (bot->InBattleground())  // issue #97 guard, kept (see 4.8)
        return WorldPosition();

    // #218 (BEYOND AC, MoP only): RoamTransportCampChance percent of camp picks try a level-matched hub on another
    // continent the travel graph reaches (boat / zeppelin / portal). Never for a status a .botscenario pins or picks
    // deterministically, so the 1:1 scenario legs stay AC-identical.
    if (uint32 const pct = RoamTransport::CampChancePct())
    {
        if (!BotScenario::IsStatusLocked(bot) && !BotScenario::PickNearest(bot) && urand(1, 100) <= pct)
        {
            WorldPosition const farHub = RoamTransport::SelectCrossMapHub(bot);
            if (farHub != WorldPosition())
            {
                TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s select cross-map camp pos Map:%u X:%f Y:%f Z:%f (from map %u)",
                             bot->GetName().c_str(), farHub.GetMapId(), farHub.GetPositionX(), farHub.GetPositionY(),
                             farHub.GetPositionZ(), bot->GetMapId());
                return farHub;
            }
        }
    }

    std::vector<WorldLocation> const& locs = sTravelMgr.GetTravelHubs(bot);

    bool inCity = false;
    if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
    {
        if (zone->Flags & AREA_FLAG_CAPITAL)
            inCity = true;
    }

    std::vector<WorldLocation> prepared_locs;
    for (auto& loc : locs)
    {
        if (bot->GetMapId() != loc.GetMapId())
            continue;

        float range = bot->GetLevel() <= 5 ? 500.0f : 2500.0f;
        if (bot->GetExactDist(loc) > range)
            continue;

        if (bot->GetExactDist(loc) < 50.0f)
            continue;

        if (!inCity && bot->GetMap()->GetZoneId(bot->GetPhaseMask(), loc.GetPositionX(), loc.GetPositionY(),
                                                loc.GetPositionZ()) != bot->GetZoneId())
            continue;

        prepared_locs.push_back(loc);
    }
    // HARNESS-SPEC-botscenario-v2 G3.2: deterministic pick - nearest valid hub, the same way as
    // SelectRandomGrindPos above.
    bool const pickNearest = BotScenario::PickNearest(bot);

    WorldPosition dest{};
    if (pickNearest)
    {
        for (WorldLocation const& loc : prepared_locs)
            if (dest == WorldPosition() || bot->GetExactDist(loc) < bot->GetExactDist(dest))
                dest = loc;
    }
    else if (!prepared_locs.empty())
        dest = prepared_locs[urand(0, prepared_locs.size() - 1)];

    TC_LOG_DEBUG("playerbots", "[New RPG] Bot %s select random inn keeper pos Map:%u X:%f Y:%f Z:%f (%u available in %u)",
                 bot->GetName().c_str(), dest.GetMapId(), dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(),
                 uint32(prepared_locs.size()), uint32(locs.size()));
    return dest;
}

// =====================================================================================================================
// MoP/Cata extension, owner 2026-10-16, #843 (BEYOND AC): quest hub search.
// AC's quest flow (WotLK data) relies on each hub offering its level's quests at once. Cataclysm rebuilt the old zones
// as chains, so on this core a hub's givers often offer nothing until a chain starter elsewhere in the zone is done
// (Durotar: Razor Hill's Gar'Thok 25171/25173/25177 need 25168 from Sen'jin Village). AC's CheckRpgStatusAvailable then
// never finds DO_QUEST available and the bot falls back to WANDER_NPC (w71p: MoP DO_QUEST 32.8 % vs AC 46.3 %).
// AC's code path is kept: only when an IDLE bot has no doable quest in its log does DO_QUEST stay a candidate, and only
// if a quest giver in the bot's zone has a quest it can take now. Picking it sends the bot there with AC's GO_CAMP
// travel (MoveFarTo), and the quest is taken on arrival through AC's own accept path (SearchQuestGiverAndAcceptOrReward,
// GO_CAMP -> WANDER_NPC). Gate: AiPlayerbot.RpgQuestHubSearchEnable (0 = exactly AC). Runs only at the IDLE status
// pick (CheckRpgStatusAvailable + RandomChangeStatus, one search shared by both), never per tick.
// =====================================================================================================================
namespace QuestHubSearch
{
// A creature quest-giver spawn on a continent, indexed once per map from the spawn store.
struct StarterSpawn
{
    uint32 entry;
    uint32 spawnId;
    uint32 phaseMask;
    float x;
    float y;
    float z;
};

struct Pick
{
    uint32 entry = 0;
    uint32 spawnId = 0;
    uint32 questId = 0;
    WorldPosition pos{};
    float dist = 0.0f;
};

struct BotState
{
    bool hasPick = false;  // found by CheckRpgStatusAvailable, consumed by RandomChangeStatus in the same draw
    uint32 pickMs = 0;
    Pick pick;
    bool hasNone = false;  // the last search found nothing
    uint32 noneMs = 0;
    std::unordered_map<uint64, uint32> targeted;  // (spawnId << 32 | questId) -> getMSTime() of the trip
    bool onTrip = false;  // #872: set by MarkTargeted (hub pick -> GO_CAMP), cleared at the next IDLE draw or on accept
};

constexpr uint32 PICK_REUSE_MS = 2 * IN_MILLISECONDS;                  // check and draw run in the same Execute()
constexpr uint32 NONE_COOLDOWN_MS = 2 * MINUTE * IN_MILLISECONDS;      // no takeable quest here: AC behaviour meanwhile
constexpr uint32 RETARGET_COOLDOWN_MS = 30 * MINUTE * IN_MILLISECONDS; // never re-pick a giver/quest it just went to
constexpr size_t MAX_GIVERS_CHECKED = 24;                              // nearest spawns whose quests are evaluated
constexpr uint32 MAX_ZONE_LOOKUPS = 8;                                 // terrain zone lookups per search
constexpr uint32 MAX_REACH_CHECKS = 4;                                 // #975: uncached walk-reach checks per search
constexpr float REACH_TALK_DIST = 7.0f;                                // #975: a poly this close (3D) can talk to the giver
constexpr float REACH_ESCAPE_DIST = 80.0f;                             // #975: a region reaching this far is not an island
constexpr size_t REACH_FILL_CAP = 2048;                                // #975: polys one fill may visit (reached = open)

std::once_flag indexOnce;
std::unordered_map<uint32, std::vector<StarterSpawn>> spawnsByMap;  // written once under indexOnce, then read-only
std::mutex stateLock;
std::unordered_map<ObjectGuid, BotState> states;
std::mutex reachLock;                            // #975
std::unordered_map<uint32, bool> reachBySpawn;   // #975: spawnId -> walk-reachable (the navmesh is static)

// #975 (BEYOND AC, part of this extension): the search measured 2D distance and never asked whether the giver can be
// walked to. Run mop1k-v10-1028: Guine (inn below the Fizzcrank tower, Borean Tundra) "quest hub search -> giver 25590
// quest 11725 dist 1" - Fizzcrank Fullthrottle 25590 stands 13 yd up on the tower top (4178.3,5283.1,39.6), a 22-poly
// navmesh island whose only exit is elevator 152614; the bot flew up, took the quest and was then stuck on the island
// (Guine and Peragon in v10, Brougos in v8, Badeon and Estanza in v9). AC has no such check: its quest flow walks to
// quest givers within 80 yd 3D (AC Ai/World/Rpg/Action/NewRpgBaseAction.cpp:735,773 ChooseNpcOrGameObjectToInteract
// GetDistance) out of "possible new rpg targets", which skips line of sight (AC
// Ai/Base/Value/PossibleRpgTargetsValue.cpp:115-117, NearestUnitsValue ignoreLos = true); this search sends bots up to
// RpgQuestHubSearchRadius (2500 yd) away, so it skips givers that only a navmesh island can talk to.
// The rule is the navmesh region around the giver, not a path: the walkable polys (bot filter) within REACH_TALK_DIST
// (3D) of the spawn - the bot can talk to the giver from any of them (INTERACTION_DISTANCE 5 plus both combat reaches)
// - are flood-filled over the poly links; the giver is on an island when the fill ends before any visited poly has a
// vertex REACH_ESCAPE_DIST (3D) away from the spawn. Fizzcrank's tower top is a closed 24-poly region within 9.6 yd.
// The first version (paths toward the giver from 4 ground anchors 30 yd out, anchor height from Map::GetHeight)
// rejected reachable givers for every bot: Innkeeper Norman 6741 under Undercity (v11, anchors on the surface lake),
// Rin Duoctane 30490 in Dalaran (v12, anchors on the forest 460 yd below the city) and Muigin 9119 at Marshal's Refuge
// (v12, the spawn is 19 yd above the navmesh). Offline (a Python Detour reader over the test server's mmaps, every
// quest-giver spawn of the hub index on the continents and open-world maps): the closed regions are tower / platform
// tops (Fizzcrank, Archmage Vargoth's Violet Citadel retreat, the Wyrmrest Temple top, Image of Archmage Xylem), ship
// decks, regions the mmaps cut off (Maginor Dumas / Coridormi in Stormwind, 76 yd from any open poly; two Revantusk
// huts) and the underwater Vashj'ir bases (kept, see Submerged); Norman, Rin Duoctane and every city / town giver are
// open, Muigin has no poly within reach and is kept.
// Verdicts: Open and NoPoly keep the giver and are cached (the navmesh is static); Island skips it and is cached,
// except for a submerged giver (a swimmer reaches it: cached as kept) and for a bot standing inside the island itself
// (kept for that bot only, not cached); Unknown (no mmaps, a tile in reach of the spawn or at the fill's edge not
// loaded) keeps the giver without caching, as AC sends bots to givers without any check.
enum class Reach : uint8 { Open, NoPoly, Island, Unknown };

// The flood fill (see above). botInside: the bot's own poly is part of an Island. Called from the bot's map thread
// (the per-instance dtNavMeshQuery is the one PathGenerator uses there).
Reach NavmeshReach(Player* bot, StarterSpawn const& spawn, bool& botInside)
{
    botInside = false;
    MMAP::MMapManager* mgr = MMAP::MMapFactory::createOrGetMMapManager();
    dtNavMesh const* nav = mgr->GetNavMesh(bot->GetMapId());
    dtNavMeshQuery const* query = mgr->GetNavMeshQuery(bot->GetMapId(), bot->GetInstanceId());
    if (!nav || !query)
        return Reach::Unknown;

    float const center[3] = {spawn.y, spawn.z, spawn.x};  // Detour order
    // Every tile under the talk box must be loaded (tiles load with their grid), or "no poly" may only mean "not yet".
    for (uint32 i = 0; i < 4; ++i)
    {
        float const corner[3] = {spawn.y + ((i & 1) ? REACH_TALK_DIST : -REACH_TALK_DIST), spawn.z,
                                 spawn.x + ((i & 2) ? REACH_TALK_DIST : -REACH_TALK_DIST)};
        int tx = -1, ty = -1;
        nav->calcTileLoc(corner, &tx, &ty);
        if (tx < 0 || ty < 0 || !nav->getTileAt(tx, ty, 0))
            return Reach::Unknown;
    }

    dtQueryFilter filter;  // the bot filter of PathGenerator::CreateFilter
    filter.setIncludeFlags(NAV_GROUND | NAV_WATER);
    filter.setExcludeFlags(NAV_MAGMA | NAV_SLIME);

    float const ext[3] = {REACH_TALK_DIST, REACH_TALK_DIST, REACH_TALK_DIST};
    dtPolyRef nearPolys[128];
    int nearCount = 0;
    query->queryPolygons(center, ext, &filter, nearPolys, &nearCount, 128);  // a full buffer keeps the first 128

    std::vector<dtPolyRef> pending;
    std::unordered_set<dtPolyRef> seen;
    float const talkSq = REACH_TALK_DIST * REACH_TALK_DIST;
    for (int i = 0; i < nearCount; ++i)
    {
        float cp[3];
        if (dtStatusFailed(query->closestPointOnPoly(nearPolys[i], center, cp, nullptr)))
            continue;
        float const dx = cp[0] - center[0], dy = cp[1] - center[1], dz = cp[2] - center[2];
        if (dx * dx + dy * dy + dz * dz <= talkSq && seen.insert(nearPolys[i]).second)
            pending.push_back(nearPolys[i]);
    }
    if (pending.empty())
        return Reach::NoPoly;

    float const escapeSq = REACH_ESCAPE_DIST * REACH_ESCAPE_DIST;
    bool unloadedEdge = false;
    while (!pending.empty())
    {
        dtPolyRef const ref = pending.back();
        pending.pop_back();
        dtMeshTile const* tile = nullptr;
        dtPoly const* poly = nullptr;
        if (dtStatusFailed(nav->getTileAndPolyByRef(ref, &tile, &poly)))
            continue;

        for (uint8 v = 0; v < poly->vertCount; ++v)
        {
            float const* p = &tile->verts[poly->verts[v] * 3];
            float const dx = p[0] - center[0], dy = p[1] - center[1], dz = p[2] - center[2];
            if (dx * dx + dy * dy + dz * dz > escapeSq)
                return Reach::Open;
        }
        if (seen.size() >= REACH_FILL_CAP)
            return Reach::Open;

        uint32 linkedEdges = 0;
        for (unsigned int l = poly->firstLink; l != DT_NULL_LINK; l = tile->links[l].next)
        {
            dtLink const& link = tile->links[l];
            if (link.edge < DT_VERTS_PER_POLYGON)
                linkedEdges |= 1u << link.edge;
            if (!link.ref || seen.count(link.ref))
                continue;
            dtMeshTile const* nextTile = nullptr;
            dtPoly const* nextPoly = nullptr;
            nav->getTileAndPolyByRefUnsafe(link.ref, &nextTile, &nextPoly);
            // the flag test of dtQueryFilter::passFilter: without DT_VIRTUAL_QUERYFILTER Detour defines it inline in its own
            // .cpp, so a call from here does not link on GCC / Clang (Linux / macOS CI)
            if (!nextPoly || (nextPoly->flags & filter.getIncludeFlags()) == 0 || (nextPoly->flags & filter.getExcludeFlags()) != 0)
                continue;
            seen.insert(link.ref);
            pending.push_back(link.ref);
        }

        // A tile-border edge with no link toward a tile that is not loaded: the region may go on there.
        for (uint8 e = 0; e < poly->vertCount; ++e)
        {
            if (!(poly->neis[e] & DT_EXT_LINK) || (linkedEdges & (1u << e)))
                continue;
            int nx = tile->header->x, ny = tile->header->y;
            switch (poly->neis[e] & 0xff)
            {
                case 0: ++nx; break;
                case 2: ++ny; break;
                case 4: --nx; break;
                case 6: --ny; break;
                default: break;
            }
            if (!nav->getTileAt(nx, ny, 0))
                unloadedEdge = true;
        }
    }
    if (unloadedEdge)
        return Reach::Unknown;

    float const botPos[3] = {bot->GetPositionY(), bot->GetPositionZ(), bot->GetPositionX()};
    float const botExt[3] = {3.0f, 5.0f, 3.0f};  // PathGenerator::GetPolyByLocation's first box
    float botPoint[3];
    dtPolyRef botRef = 0;
    if (dtStatusSucceed(query->findNearestPoly(botPos, botExt, &filter, &botRef, botPoint)) && botRef)
        botInside = seen.count(botRef) != 0;
    return Reach::Island;
}

// An island giver under at least 2 yd of water (the Vashj'ir bases: their navmesh is closed, the sea above is not
// navmesh, a swimmer gets there).
bool Submerged(Player* bot, StarterSpawn const& spawn)
{
    return bot->GetMap()->IsInWater(bot->GetPhaseMask(), spawn.x, spawn.y, spawn.z + 2.0f);
}

// Returns false only for a giver on a navmesh island (see above). Called from the bot's map thread.
bool WalkReachable(Player* bot, StarterSpawn const& spawn, uint32& checks)
{
    {
        std::lock_guard<std::mutex> guard(reachLock);
        auto itr = reachBySpawn.find(spawn.spawnId);
        if (itr != reachBySpawn.end())
            return itr->second;
    }
    if (++checks > MAX_REACH_CHECKS)
        return false;  // this search spent its budget; the giver stays unjudged for the next one

    bool botInside = false;
    Reach const reach = NavmeshReach(bot, spawn, botInside);
    if (reach == Reach::Unknown)
        return true;  // not judged: keep the giver (AC sends bots to givers without any check)
    if (reach == Reach::Island && botInside)
        return true;  // this bot stands on the island itself; says nothing for the bots outside it

    bool const keep = reach != Reach::Island || Submerged(bot, spawn);
    {
        std::lock_guard<std::mutex> guard(reachLock);
        reachBySpawn[spawn.spawnId] = keep;
    }
    if (!keep)
        TC_LOG_DEBUG("playerbots",
                     "[New RPG] quest hub search skips giver %u spawn %u (%.1f,%.1f,%.1f map %u): navmesh island "
                     "(closed within %.0f yd)",
                     spawn.entry, spawn.spawnId, spawn.x, spawn.y, spawn.z, bot->GetMapId(), REACH_ESCAPE_DIST);
    return keep;
}

void BuildIndex()
{
    uint32 total = 0;
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        CreatureData const& data = itr.second;
        // Event-only and phase-id / phase-group spawns are not reliably there for a roaming bot.
        if (!data.dbData || data.gameEventId || data.phaseid || data.phaseGroup)
            continue;

        MapEntry const* mapEntry = sMapStore.LookupEntry(data.mapId);
        if (!mapEntry || mapEntry->Instanceable())
            continue;

        CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(data.id);
        if (!tmpl || !((tmpl->npcflag | data.npcflag) & UNIT_NPC_FLAG_QUESTGIVER))
            continue;

        QuestRelationResult const relations = sObjectMgr->GetCreatureQuestRelations(data.id);
        if (relations.begin() == relations.end())
            continue;

        spawnsByMap[data.mapId].push_back({data.id, itr.first, data.phaseMask, data.posX, data.posY, data.posZ});
        ++total;
    }

    TC_LOG_INFO("playerbots", "[New RPG] quest hub search index: %u quest-giver spawns on %u maps", total,
                uint32(spawnsByMap.size()));
}

bool Enabled(Player* bot)
{
    // Off in a deterministic .botscenario (pick=nearest) so those 1:1 legs stay AC-identical (#218 precedent).
    return sPlayerbotAIConfig->rpgQuestHubSearchEnable && sPlayerbotAIConfig->rpgQuestHubSearchRadius > 0.0f &&
           !BotScenario::PickNearest(bot) && bot->GetMap() && !bot->GetMap()->Instanceable();
}

// canTake(Quest const*): the bot can take this quest now - the same test HasQuestToAcceptOrReward applies at the giver.
template <class CanTake>
bool Search(Player* bot, std::unordered_map<uint64, uint32> const& targeted, CanTake const& canTake, Pick& out)
{
    std::call_once(indexOnce, BuildIndex);

    auto mapItr = spawnsByMap.find(bot->GetMapId());
    if (mapItr == spawnsByMap.end())
        return false;

    float const radius = sPlayerbotAIConfig->rpgQuestHubSearchRadius;
    float const radiusSq = radius * radius;
    uint32 const botPhase = bot->GetPhaseMask();

    std::vector<std::pair<float, StarterSpawn const*>> nearby;
    for (StarterSpawn const& spawn : mapItr->second)
    {
        if (!(spawn.phaseMask & botPhase))
            continue;

        // #975: 3D, like AC's quest-giver distance (GetDistance); the 2D "dist 1" hid a giver 13 yd up a tower.
        float const dx = spawn.x - bot->GetPositionX();
        float const dy = spawn.y - bot->GetPositionY();
        float const dz = spawn.z - bot->GetPositionZ();
        float const distSq = dx * dx + dy * dy + dz * dz;
        if (distSq > radiusSq)
            continue;

        nearby.emplace_back(distSq, &spawn);
    }

    size_t const count = std::min(nearby.size(), MAX_GIVERS_CHECKED);
    std::partial_sort(nearby.begin(), nearby.begin() + count, nearby.end(),
                      [](std::pair<float, StarterSpawn const*> const& a, std::pair<float, StarterSpawn const*> const& b)
                      { return a.first < b.first; });

    FactionTemplateEntry const* botFaction = bot->GetFactionTemplateEntry();
    uint32 const botZone = bot->GetZoneId();
    uint32 zoneLookups = 0;
    uint32 reachChecks = 0;  // #975

    for (size_t i = 0; i < count; ++i)
    {
        StarterSpawn const& spawn = *nearby[i].second;

        if (botFaction)
        {
            CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(spawn.entry);
            FactionTemplateEntry const* giverFaction = tmpl ? sFactionTemplateStore.LookupEntry(tmpl->faction) : nullptr;
            if (giverFaction && giverFaction->IsHostileTo(*botFaction))
                continue;
        }

        uint32 questId = 0;
        for (uint32 relQuestId : sObjectMgr->GetCreatureQuestRelations(spawn.entry))
        {
            if (targeted.count((uint64(spawn.spawnId) << 32) | relQuestId))
                continue;

            Quest const* quest = sObjectMgr->GetQuestTemplate(relQuestId);
            if (quest && canTake(quest))
            {
                questId = relQuestId;
                break;
            }
        }

        if (!questId)
            continue;

        // Same zone only, like AC's GO_CAMP pick (SelectRandomCampPos); leaving the zone stays TRAVEL_FLIGHT's job.
        if (++zoneLookups > MAX_ZONE_LOOKUPS)
            break;
        if (bot->GetMap()->GetZoneId(botPhase, spawn.x, spawn.y, spawn.z) != botZone)
            continue;

        // #975: only a giver the navmesh joins to the ground around it (see WalkReachable).
        if (!WalkReachable(bot, spawn, reachChecks))
            continue;

        out.entry = spawn.entry;
        out.spawnId = spawn.spawnId;
        out.questId = questId;
        out.pos = WorldPosition(bot->GetMapId(), spawn.x, spawn.y, spawn.z);
        out.dist = std::sqrt(nearby[i].first);
        return true;
    }

    return false;
}

template <class CanTake>
bool Find(Player* bot, CanTake const& canTake, Pick& out)
{
    std::unordered_map<uint64, uint32> targeted;
    {
        std::lock_guard<std::mutex> guard(stateLock);
        BotState& state = states[bot->GetGUID()];

        if (state.hasPick && GetMSTimeDiffToNow(state.pickMs) < PICK_REUSE_MS)
        {
            out = state.pick;
            return true;
        }
        state.hasPick = false;

        if (state.hasNone && GetMSTimeDiffToNow(state.noneMs) < NONE_COOLDOWN_MS)
            return false;

        for (auto it = state.targeted.begin(); it != state.targeted.end();)
        {
            if (GetMSTimeDiffToNow(it->second) >= RETARGET_COOLDOWN_MS)
                it = state.targeted.erase(it);
            else
                ++it;
        }
        targeted = state.targeted;
    }

    // The search runs outside the lock: it reads only the bot (this thread) and load-time data.
    bool const found = Search(bot, targeted, canTake, out);

    std::lock_guard<std::mutex> guard(stateLock);
    BotState& state = states[bot->GetGUID()];
    if (found)
    {
        state.hasPick = true;
        state.pickMs = getMSTime();
        state.pick = out;
        state.hasNone = false;
    }
    else
    {
        state.hasNone = true;
        state.noneMs = getMSTime();
        TC_LOG_DEBUG("playerbots", "[New RPG] %s quest hub search -> none (map %u zone %u level %u)",
                     bot->GetName().c_str(), bot->GetMapId(), bot->GetZoneId(), uint32(bot->GetLevel()));
    }
    return found;
}

void MarkTargeted(Player* bot, Pick const& pick)
{
    std::lock_guard<std::mutex> guard(stateLock);
    BotState& state = states[bot->GetGUID()];
    state.targeted[(uint64(pick.spawnId) << 32) | pick.questId] = getMSTime();
    state.hasPick = false;
    state.onTrip = true;  // #872
}

// #872: any new IDLE draw ends a trip, so the flag never outlives the GO_CAMP -> WANDER_NPC the hub pick started.
void ClearTrip(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr != states.end())
        itr->second.onTrip = false;
}

// #872: true (and the trip ends) when the bot is on a hub trip.
bool TakeTrip(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr == states.end() || !itr->second.onTrip)
        return false;
    itr->second.onTrip = false;
    return true;
}
}  // namespace QuestHubSearch
// ============================== end MoP/Cata extension #843 (quest hub search) ==============================

// =====================================================================================================================
// #945 (BEYOND AC, this fork only): town errands. AC bots reach repairers, vendors, trainers, auctioneers, bankers and
// stable masters only by chance (WANDER_NPC picks a random NPC within 150 yd, AC PossibleRpgTargetsValue.cpp:88-108).
// Same pattern as the quest hub search above: at the IDLE status draw (RandomChangeStatus) a bot with a need - repair,
// sell, profession training near its skill cap - or, with RpgTownErrandChance %, a random errand (auctioneer, banker,
// own-class trainer, stable master for hunters, guild banker for guilded bots) travels to a matching NPC with AC's
// GO_CAMP (MoveFarTo). On the GO_CAMP arrival NewRpgAction.cpp seeds WANDER_NPC with that NPC, whose arrival code does
// the work (sell/buy/repair as AC test-staging PR #2949, trainer learn) and stays RpgTownErrandStayMin..Max seconds at
// the pure-visit NPCs. No new status, no NewRpgInfo change. Gate: AiPlayerbot.RpgTownErrandEnable (0 = exactly AC);
// off in a deterministic .botscenario (pick=nearest). Config is read once through sConfigMgr, like CfgCampChance in
// NewRpgRoamTransport.cpp, so PlayerbotAIConfig.h is not touched.
// =====================================================================================================================
namespace TownErrand
{
enum Cat : uint32
{
    CAT_REPAIR = 0x01,
    CAT_VENDOR = 0x02,
    CAT_PROF = 0x04,       // profession trainer (npc_trainer rows for a profession skill, trainerType 2)
    CAT_AUCTION = 0x08,
    CAT_BANK = 0x10,
    CAT_CLASS = 0x20,      // class trainer: creature_template.trainer_class (MoP class trainers carry npcflag 1/3 only)
    CAT_STABLE = 0x40,
    CAT_GUILDBANK = 0x80,
    CAT_MAILBOX = 0x100,   // GAMEOBJECT_TYPE_MAILBOX spawn (a gameobject, not a creature)
};

// Creature and gameobject spawn ids overlap, so a gameobject spawn is keyed (targeted / npcLoad / Errand::spawnId) with this bit.
constexpr uint32 GO_KEY_BIT = 0x80000000u;

enum Need : uint32
{
    NEED_NONE = 0,
    NEED_REPAIR = 1,
    NEED_SELL = 2,
    NEED_PROF = 3,
    NEED_ERRAND = 4,
};

struct Spawn
{
    uint32 entry;
    uint32 spawnId;
    uint32 phaseMask;
    float x;
    float y;
    float z;
    float o;                 // orientation: the errand stand point is in front of the NPC
    uint32 cats;
    uint8 trainerClass;
    uint32 profSkill;        // CAT_PROF: the profession (skill id) the trainer teaches
    bool isGo = false;       // gameobject spawn (CAT_MAILBOX); spawnId then carries GO_KEY_BIT
};

struct Errand
{
    bool active = false;
    uint32 need = NEED_NONE;
    uint32 cat = 0;
    uint32 entry = 0;
    uint32 spawnId = 0;
    WorldPosition pos{};     // the NPC spawn
    WorldPosition stand{};   // GO_CAMP target: ~STAND_DIST yd in front of the NPC
    bool sawDeath = false;   // end-reason hints, set while the errand is active
    bool sawCombat = false;
    bool teleported = false; // MoveFarTo's stuck teleport fired
    bool abandoned = false;  // MoveFarTo dropped the trip
    ObjectGuid npc;          // set on the GO_CAMP arrival
    uint32 startMs = 0;      // pick time
    uint32 arriveMs = 0;     // GO_CAMP arrival (WANDER_NPC seeded)
    uint32 reachedMs = 0;    // interaction distance reached
    uint32 stayMs = 0;
    // #1004 (BEYOND AC): far legs. legMs restarts the timeout when a flight lands; budgetMs is the time allowed for the
    // current leg (pick -> landing, then landing -> done), scaled with the distance.
    uint32 legMs = 0;
    uint32 budgetMs = 0;
    uint32 zoneId = 0;           // zone of the NPC
    uint32 zoneMs = 0;           // first time the bot was seen in that zone
    bool flightPending = false;  // a flight to the NPC's town is planned / running
    uint32 landedMs = 0;
    uint32 flightFmEntry = 0;
    WorldPosition flightFmPos{};
    std::vector<uint32> flightPath;
};

struct BotState
{
    bool init = false;
    uint32 lastMs = 0;       // last errand pick (cooldown start); staggered at first sight
    bool hasNone = false;
    uint32 noneMs = 0;
    std::unordered_map<uint32, uint32> targeted;  // spawnId -> getMSTime() of the pick
    bool profValid = false;  // per-bot cache of "a trainer of a known profession offers a spell learnable now"
    bool profUseful = false;
    uint32 profMs = 0;
    Errand errand;
};

constexpr uint32 NONE_COOLDOWN_MS = 2 * MINUTE * IN_MILLISECONDS;
constexpr uint32 RETARGET_COOLDOWN_MS = 30 * MINUTE * IN_MILLISECONDS;
// #1004 (BEYOND AC): the time allowed for one leg scales with its distance: base + dist / speed, capped. v16 soak: the old
// fixed 25 min timed out 57-68% of the auction errands beyond 1000 yd (3500 yd on foot, with combat and other states between).
constexpr uint32 LEG_BASE_MS = 10 * MINUTE * IN_MILLISECONDS;
constexpr float LEG_SPEED = 3.0f;                                      // yd/s, an average that includes detours and fights
constexpr uint32 LEG_CAP_MS = 45 * MINUTE * IN_MILLISECONDS;
constexpr uint32 FLIGHT_ALLOW_MS = 6 * MINUTE * IN_MILLISECONDS;       // pick -> landing allowance on top of the walk to the flight master
uint32 LegBudgetMs(float dist, uint32 extraMs = 0)
{
    uint32 const ms = LEG_BASE_MS + extraMs + uint32(dist / LEG_SPEED) * IN_MILLISECONDS;
    return std::min(ms, LEG_CAP_MS);
}
constexpr uint32 REACH_TIMEOUT_MS = 3 * MINUTE * IN_MILLISECONDS;     // arrival at the camp pos -> interaction distance
// #945 (2 h run at 1000 bots, 2026-10-28): how long an errand keeps its bot out of AC's activity rotation (KeepsActive).
// Throttled (BotActiveAlone 10: "new rpg" actions run only in a bot's ~10% active 30 s windows), Zinor walked 82 yd to
// banker 44853 in ~16 min and then slept 14 min at 4 yd from it until the 25 min timeout. Active, a 1200 yd far-ring
// walk takes ~3-5 min; past this cap the errand falls back to the rotation (and still ends by ERRAND_MAX_MS).
constexpr uint32 ACTIVE_CAP_MS = 10 * MINUTE * IN_MILLISECONDS;
constexpr size_t MAX_CHECKED = 40;                                    // nearest spawns whose extra test is evaluated
constexpr size_t NEAREST_N = 3;
constexpr float STAND_DIST = 4.0f;                                    // GO_CAMP stand point in front of the NPC (TravelMgr hubs)

std::once_flag indexOnce;
std::unordered_map<uint32, std::vector<Spawn>> spawnsByMap;  // written once under indexOnce, then read-only
std::mutex stateLock;
std::unordered_map<ObjectGuid, BotState> states;

bool CfgEnable()
{
    static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.RpgTownErrandEnable", true);
    return v;
}
float CfgRadius()
{
    static float const v = float(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandRadius", 1200)));
    return v;
}

// Owner 2026-10-28 ("more should go to the auction house"): auction houses stand only in capitals and a few goblin towns,
// so a drawn auction errand searches this far (outer ring) before falling through to the next category - with 1200 yd
// the first weighted run picked mailboxes 44 times and auctioneers 3 times out of 72 errands.
float CfgAuctionRadius()
{
    static float const v = float(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandAuctionRadius", 4000)));
    return v;
}
// #1004 (BEYOND AC): a pick farther than this (yd) flies to the flight node nearest the NPC first (the bot's existing
// TRAVEL_FLIGHT), then walks the rest. 0 = never fly.
float CfgFlightMin()
{
    static float const v = float(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandFlightMinDistance", 800)));
    return v;
}
float CfgNearRadius()
{
    static float const v = float(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandNearRadius", 400)));
    return v;
}
uint32 CfgCooldownMs()
{
    static uint32 const v = uint32(std::min(3600 * 24, std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandCooldown", 1800)))) * IN_MILLISECONDS;
    return v;
}
uint32 CfgChance()
{
    static uint32 const v = uint32(std::min(100, std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandChance", 10))));
    return v;
}
uint32 CfgStayMinS()
{
    static uint32 const v = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandStayMin", 15)));
    return v;
}
uint32 CfgStayMaxS()
{
    static uint32 const v = std::max(CfgStayMinS(), uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandStayMax", 45))));
    return v;
}
uint32 CfgMaxBotsPerNpc()
{
    static uint32 const v = uint32(std::max(1, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandMaxBotsPerNpc", 3)));
    return v;
}

// Owner request 2026-10-28: a random errand first draws ONE category by these weights (0 = never), not an NPC by abundance.
uint32 CfgCategoryWeight(uint32 cat)
{
    static uint32 const weights[9] = {
        0,  // repair (a need, not a random errand)
        0,  // vendor (a need)
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightProfession", 25))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightAuction", 40))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightBank", 20))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightClassTrainer", 5))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightStable", 3))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightGuildBank", 2))),
        uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RpgTownErrandWeightMailbox", 6)))};
    for (uint32 i = 0; i < 9; ++i)
        if (cat == (1u << i))
            return weights[i];
    return 0;
}

// primary professions a trainer visit can raise
constexpr uint32 PRIMARY_PROFESSIONS[] = {SKILL_ALCHEMY,      SKILL_BLACKSMITHING, SKILL_ENCHANTING,  SKILL_ENGINEERING,
                                          SKILL_HERBALISM,    SKILL_INSCRIPTION,   SKILL_JEWELCRAFTING, SKILL_LEATHERWORKING,
                                          SKILL_MINING,       SKILL_SKINNING,      SKILL_TAILORING};
// secondary skills with trainers
constexpr uint32 SECONDARY_PROFESSIONS[] = {SKILL_COOKING, SKILL_FIRST_AID, SKILL_FISHING};

bool IsProfessionSkill(uint32 skill)
{
    return std::find(std::begin(PRIMARY_PROFESSIONS), std::end(PRIMARY_PROFESSIONS), skill) != std::end(PRIMARY_PROFESSIONS) ||
           std::find(std::begin(SECONDARY_PROFESSIONS), std::end(SECONDARY_PROFESSIONS), skill) != std::end(SECONDARY_PROFESSIONS);
}

char const* NeedName(uint32 need)
{
    switch (need)
    {
        case NEED_REPAIR: return "repair";
        case NEED_SELL: return "sell";
        case NEED_PROF: return "profession";
        case NEED_ERRAND: return "errand";
        default: return "none";
    }
}

void BuildIndex()
{
    uint32 counts[9] = {};
    uint32 total = 0;
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        CreatureData const& data = itr.second;
        // Event-only and phase-id / phase-group spawns are not reliably there for a roaming bot (as the quest hub index).
        if (!data.dbData || data.gameEventId || data.phaseid || data.phaseGroup)
            continue;

        MapEntry const* mapEntry = sMapStore.LookupEntry(data.mapId);
        if (!mapEntry || mapEntry->Instanceable())
            continue;

        CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(data.id);
        if (!tmpl)
            continue;

        uint32 const flags = tmpl->npcflag | data.npcflag;
        uint32 cats = 0;
        if (flags & UNIT_NPC_FLAG_REPAIR)
            cats |= CAT_REPAIR;
        if (flags & UNIT_NPC_FLAG_VENDOR)
            cats |= CAT_VENDOR;
        if (flags & UNIT_NPC_FLAG_AUCTIONEER)
            cats |= CAT_AUCTION;
        if (flags & UNIT_NPC_FLAG_BANKER)
            cats |= CAT_BANK;
        if (flags & UNIT_NPC_FLAG_STABLEMASTER)
            cats |= CAT_STABLE;
        if (flags & UNIT_NPC_FLAG_GUILD_BANKER)
            cats |= CAT_GUILDBANK;
        if (tmpl->trainer_class)
            cats |= CAT_CLASS;
        uint32 profSkill = 0;
        if ((flags & (UNIT_NPC_FLAG_TRAINER | UNIT_NPC_FLAG_TRAINER_PROFESSION)))
        {
            TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(data.id);
            if (trainer && trainer->trainerType == 2)
            {
                // the profession it teaches: the first profession skill any of its spells requires
                for (auto const& spellItr : trainer->spellList)
                {
                    if (IsProfessionSkill(spellItr.second.reqSkill))
                    {
                        profSkill = spellItr.second.reqSkill;
                        break;
                    }
                }
                if (profSkill)
                    cats |= CAT_PROF;
            }
        }

        if (!cats)
            continue;

        spawnsByMap[data.mapId].push_back({data.id, itr.first, data.phaseMask, data.posX, data.posY, data.posZ, data.orientation,
                                           cats, uint8(tmpl->trainer_class), profSkill});
        ++total;
        for (uint32 i = 0; i < 9; ++i)
            if (cats & (1u << i))
                ++counts[i];
    }

    // Mailboxes (GAMEOBJECT_TYPE_MAILBOX) are gameobject spawns: same filters as the creature spawns above.
    for (auto const& itr : sObjectMgr->GetAllGOData())
    {
        GameObjectData const& data = itr.second;
        if (!data.dbData || data.gameEventId || data.phaseid || data.phaseGroup)
            continue;

        MapEntry const* mapEntry = sMapStore.LookupEntry(data.mapid);
        if (!mapEntry || mapEntry->Instanceable())
            continue;

        GameObjectTemplate const* tmpl = sObjectMgr->GetGameObjectTemplate(data.id);
        if (!tmpl || tmpl->type != GAMEOBJECT_TYPE_MAILBOX)
            continue;

        Spawn spawn{data.id, itr.first | GO_KEY_BIT, data.phaseMask, data.posX, data.posY, data.posZ, data.orientation,
                    uint32(CAT_MAILBOX), 0, 0};
        spawn.isGo = true;
        spawnsByMap[data.mapid].push_back(spawn);
        ++total;
        ++counts[8];
    }

    TC_LOG_INFO("playerbots",
                "[New RPG] town errand index: %u spawns on %u maps (repair %u vendor %u profession %u auction %u bank %u class %u "
                "stable %u guildbank %u mailbox %u)",
                total, uint32(spawnsByMap.size()), counts[0], counts[1], counts[2], counts[3], counts[4], counts[5], counts[6],
                counts[7], counts[8]);
}

bool Enabled(Player* bot)
{
    return CfgEnable() && CfgRadius() > 0.0f && !BotScenario::PickNearest(bot) && bot->GetMap() &&
           !bot->GetMap()->Instanceable() && !bot->InBattleground();
}

// Professions the bot knows (primary or cooking / first aid / fishing), as a skill id list. #945 follow-up: the old trigger
// ("within 25 of the skill cap") was almost never true - random bots get skill value = max = level * 5 and every eligible
// trainer spell from BotFactory InitAvailableSpells at randomize / refresh / level-up - so the trigger is now "a trainer of a
// known profession offers something learnable now" (ProfessionLearnableNow, cached per bot).
std::vector<uint32> KnownProfessions(Player* bot)
{
    std::vector<uint32> out;
    for (uint32 skill : PRIMARY_PROFESSIONS)
        if (bot->HasSkill(skill))
            out.push_back(skill);
    for (uint32 skill : SECONDARY_PROFESSIONS)
        if (bot->HasSkill(skill))
            out.push_back(skill);
    return out;
}

// True when the trainer teaches the bot a rank of one of `skills` it can learn and afford right now.
bool ProfessionTrainerUseful(PlayerbotAI* botAI, Player* bot, uint32 entry, std::vector<uint32> const& skills)
{
    TrainerSpellData const* trainer = sObjectMgr->GetNpcTrainerSpells(entry);
    if (!trainer)
        return false;

    uint32 const money = botAI->GetAiObjectContext()->GetValue<uint32>("free money for", std::to_string(uint32(NeedMoneyFor::spells)))->Get();
    for (auto const& itr : trainer->spellList)
    {
        TrainerSpell const& spell = itr.second;
        if (!spell.reqSkill || std::find(skills.begin(), skills.end(), spell.reqSkill) == skills.end())
            continue;
        if (spell.spellCost <= money && bot->GetTrainerSpellState(&spell) == TRAINER_SPELL_GREEN)
            return true;
    }
    return false;
}

// True when a trainer of one of `skills` on the bot's map (within the errand radius) offers a spell learnable now. The
// answer is cached per bot for RpgTownErrandCooldown (TryStart is the only caller and is itself cooldown-gated).
bool ProfessionLearnableNow(PlayerbotAI* botAI, Player* bot, std::vector<uint32> const& skills)
{
    if (skills.empty())
        return false;
    {
        std::lock_guard<std::mutex> guard(stateLock);
        BotState& state = states[bot->GetGUID()];
        if (state.profValid && GetMSTimeDiffToNow(state.profMs) < CfgCooldownMs())
            return state.profUseful;
    }

    std::call_once(indexOnce, BuildIndex);
    bool useful = false;
    auto mapItr = spawnsByMap.find(bot->GetMapId());
    if (mapItr != spawnsByMap.end())
    {
        float const radiusSq = CfgRadius() * CfgRadius();
        std::set<uint32> checkedEntries;
        for (Spawn const& spawn : mapItr->second)
        {
            if (!(spawn.cats & CAT_PROF) || std::find(skills.begin(), skills.end(), spawn.profSkill) == skills.end() ||
                !(spawn.phaseMask & bot->GetPhaseMask()))
                continue;
            float const dx = spawn.x - bot->GetPositionX();
            float const dy = spawn.y - bot->GetPositionY();
            if (dx * dx + dy * dy > radiusSq || !checkedEntries.insert(spawn.entry).second)
                continue;
            if (ProfessionTrainerUseful(botAI, bot, spawn.entry, skills))
            {
                useful = true;
                break;
            }
        }
    }

    std::lock_guard<std::mutex> guard(stateLock);
    BotState& state = states[bot->GetGUID()];
    state.profValid = true;
    state.profUseful = useful;
    state.profMs = getMSTime();
    return useful;
}

struct Pick
{
    Spawn const* spawn = nullptr;
    uint32 cat = 0;
    float dist = 0.0f;
};

// `wanted`: categories the need accepts. The nearest NEAREST_N eligible spawns are candidates, one is picked at random.
bool Search(PlayerbotAI* botAI, Player* bot, uint32 need, uint32 wanted, std::vector<uint32> const& profSkills,
            std::unordered_map<uint32, uint32> const& targeted, std::unordered_map<uint32, uint32> const& npcLoad, float radius,
            Pick& out)
{
    std::call_once(indexOnce, BuildIndex);

    auto mapItr = spawnsByMap.find(bot->GetMapId());
    if (mapItr == spawnsByMap.end())
        return false;

    float const radiusSq = radius * radius;
    uint32 const botPhase = bot->GetPhaseMask();
    uint8 const botClass = bot->GetClass();
    bool const guilded = bot->GetGuildId() != 0;
    uint32 const perNpc = CfgMaxBotsPerNpc();

    // Hostile spawns are dropped here, before the nearest-N cut: in a contested zone (Dragonblight, Hellfire) the
    // nearest MAX_CHECKED repairers or vendors could all be the other faction's, and the search found nothing at all
    // (combined run 2026-10-27: 15 "no matching NPC" for 3 picks).
    FactionTemplateEntry const* botFaction = bot->GetFactionTemplateEntry();
    std::vector<std::pair<float, Spawn const*>> nearby;
    for (Spawn const& spawn : mapItr->second)
    {
        if (!(spawn.phaseMask & botPhase))
            continue;

        uint32 cats = spawn.cats & wanted;
        // class trainer: only the bot's own class; stable master: hunters; guild bank: guilded bots
        if ((cats & CAT_CLASS) && spawn.trainerClass != botClass)
            cats &= ~uint32(CAT_CLASS);
        // profession trainer: only a profession the bot knows (profSkills is empty when it knows none)
        if ((cats & CAT_PROF) && std::find(profSkills.begin(), profSkills.end(), spawn.profSkill) == profSkills.end())
            cats &= ~uint32(CAT_PROF);
        if ((cats & CAT_STABLE) && botClass != CLASS_HUNTER)
            cats &= ~uint32(CAT_STABLE);
        if ((cats & CAT_GUILDBANK) && !guilded)
            cats &= ~uint32(CAT_GUILDBANK);
        if (!cats)
            continue;

        float const dx = spawn.x - bot->GetPositionX();
        float const dy = spawn.y - bot->GetPositionY();
        float const distSq = dx * dx + dy * dy;
        if (distSq > radiusSq || distSq < 25.0f * 25.0f)  // already standing at it: GO_CAMP would be a no-op
            continue;
        if (targeted.count(spawn.spawnId))
            continue;
        auto load = npcLoad.find(spawn.spawnId);
        if (load != npcLoad.end() && load->second >= perNpc)
            continue;
        if (botFaction && !spawn.isGo)
        {
            CreatureTemplate const* tmpl = sObjectMgr->GetCreatureTemplate(spawn.entry);
            FactionTemplateEntry const* faction = tmpl ? sFactionTemplateStore.LookupEntry(tmpl->faction) : nullptr;
            if (faction && faction->IsHostileTo(*botFaction))
                continue;
        }

        nearby.emplace_back(distSq, &spawn);
    }

    size_t const count = std::min(nearby.size(), MAX_CHECKED);
    std::partial_sort(nearby.begin(), nearby.begin() + count, nearby.end(),
                      [](std::pair<float, Spawn const*> const& a, std::pair<float, Spawn const*> const& b)
                      { return a.first < b.first; });

    std::vector<std::pair<float, Spawn const*>> candidates;
    for (size_t i = 0; i < count && candidates.size() < NEAREST_N; ++i)
    {
        Spawn const& spawn = *nearby[i].second;

        if (need == NEED_PROF && !ProfessionTrainerUseful(botAI, bot, spawn.entry, profSkills))
            continue;

        if (bot->GetMap()->IsInWater(botPhase, spawn.x, spawn.y, spawn.z))
            continue;

        candidates.push_back(nearby[i]);
    }

    if (candidates.empty())
        return false;

    auto const& chosen = candidates[urand(0, uint32(candidates.size() - 1))];
    out.spawn = chosen.second;
    out.dist = std::sqrt(chosen.first);
    uint32 const cats = chosen.second->cats & wanted;
    // the category that explains the visit, most specific first
    for (uint32 bit : {CAT_PROF, CAT_REPAIR, CAT_VENDOR, CAT_AUCTION, CAT_BANK, CAT_CLASS, CAT_STABLE, CAT_GUILDBANK, CAT_MAILBOX})
    {
        if (cats & bit)
        {
            out.cat = bit;
            break;
        }
    }
    return true;
}

char const* StatusName(NewRpgStatus status)
{
    switch (status)
    {
        case RPG_IDLE: return "IDLE";
        case RPG_GO_GRIND: return "GO_GRIND";
        case RPG_GO_CAMP: return "GO_CAMP";
        case RPG_WANDER_NPC: return "WANDER_NPC";
        case RPG_WANDER_RANDOM: return "WANDER_RANDOM";
        case RPG_REST: return "REST";
        case RPG_DO_QUEST: return "DO_QUEST";
        case RPG_TRAVEL_FLIGHT: return "TRAVEL_FLIGHT";
        case RPG_OUTDOOR_PVP: return "OUTDOOR_PVP";
        case RPG_DUEL_SPOT: return "DUEL_SPOT";
        default: return "?";
    }
}

// Ends the active errand without `done` and logs why (stateLock held). Completion rate = done / pick; the end lines say where
// the rest went. reason: status-changed-to X | combat | death | reset | stuck-teleport | trip-abandon | unreachable | timeout |
// replaced. #945 (2026-10-28): reached= (interaction distance seen) and since-arrive= (s from the GO_CAMP arrival) say
// where an arrived errand stopped.
void EndErrand(Player* bot, Errand& errand, char const* reason, char const* extra = "")
{
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s end reason=%s%s need=%s npc=%u age=%u s arrived=%d reached=%d since-arrive=%u s "
                 "travel=%s zone-at=%d s landed-at=%d s budget=%u s",
                 bot->GetName().c_str(), reason, extra, NeedName(errand.need), errand.entry,
                 GetMSTimeDiffToNow(errand.startMs) / IN_MILLISECONDS, errand.arriveMs ? 1 : 0, errand.reachedMs ? 1 : 0,
                 errand.arriveMs ? GetMSTimeDiffToNow(errand.arriveMs) / IN_MILLISECONDS : 0,
                 errand.landedMs ? "flight" : errand.flightFmEntry ? "flight-planned" : "walk",
                 errand.zoneMs ? int(getMSTimeDiff(errand.startMs, errand.zoneMs) / IN_MILLISECONDS) : -1,
                 errand.landedMs ? int(getMSTimeDiff(errand.startMs, errand.landedMs) / IN_MILLISECONDS) : -1,
                 errand.budgetMs / IN_MILLISECONDS);
    errand.active = false;
}

// The end reason when the status left GO_CAMP / WANDER_NPC (or an IDLE draw found the errand still active).
// reset: the status is the IDLE of a freshly built NewRpgInfo (startT 0), i.e. PlayerbotAI::Reset(true) - the
// RandomPlayerbotMgr teleport for level / refresh / revive teleport (AC Bot/RandomPlayerbotMgr.cpp:1588-1597, 1604-1613).
void EndForStatus(Player* bot, Errand& errand, NewRpgStatus status, bool reset = false)
{
    if (errand.sawDeath || !bot->IsAlive())
        EndErrand(bot, errand, "death");
    else if (reset)
        EndErrand(bot, errand, "reset");
    else if (errand.teleported)
        EndErrand(bot, errand, "stuck-teleport");
    else if (errand.abandoned)
        EndErrand(bot, errand, "trip-abandon");
    else if (errand.sawCombat)
        EndErrand(bot, errand, "combat");
    else if (status == RPG_IDLE && errand.arriveMs)
        EndErrand(bot, errand, "replaced");
    else
        EndErrand(bot, errand, "status-changed-to ", StatusName(status));
}

void NoteStuckTeleport(Player* bot)
{
    if (!CfgEnable())
        return;
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr != states.end() && itr->second.errand.active)
        itr->second.errand.teleported = true;
}

void NoteAbandon(Player* bot)
{
    if (!CfgEnable())
        return;
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr != states.end() && itr->second.errand.active)
        itr->second.errand.abandoned = true;
}

// Every NewRpgStatusUpdateAction tick (NewRpgAction.cpp): while an errand is active its status must stay GO_CAMP (walking)
// or WANDER_NPC (at the NPC). Anything else ends it, with the reason logged; the pick time keeps the normal cooldown.
void Watch(Player* bot, NewRpgStatus status)
{
    if (!CfgEnable())
        return;
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr == states.end() || !itr->second.errand.active)
        return;
    Errand& errand = itr->second.errand;
    if (!bot->IsAlive())
        errand.sawDeath = true;
    if (bot->IsInCombat())
        errand.sawCombat = true;
    if (GetMSTimeDiffToNow(errand.legMs) > errand.budgetMs)
    {
        EndErrand(bot, errand, "timeout");
        return;
    }
    if (!errand.zoneMs && errand.zoneId && bot->GetZoneId() == errand.zoneId)
    {
        errand.zoneMs = getMSTime();
        TC_LOG_DEBUG("playerbots", "[ERRAND] %s zone-arrival npc=%u zone=%u after %u s", bot->GetName().c_str(), errand.entry,
                     errand.zoneId, GetMSTimeDiffToNow(errand.startMs) / IN_MILLISECONDS);
    }
    if (status == RPG_GO_CAMP || status == RPG_WANDER_NPC || (status == RPG_TRAVEL_FLIGHT && errand.flightPending))
        return;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    EndForStatus(bot, errand, status, status == RPG_IDLE && botAI && botAI->GetRpgInfo().startT == 0);
}

// #945 (BEYOND AC, 2026-10-28): PlayerbotAI::AllowActive keeps a bot on a running errand active for its first
// ACTIVE_CAP_MS (through RoamTransport::OnTransportLeg, the activity hook AllowActive already has for roam trips), as AC
// keeps a bot in combat / a BG / an LFG queue active. Throttled, the errand's walk, its WANDER_NPC seed and the work at
// the NPC only ran in the ~10% active windows: 2 h run, 80 of 150 errand ends were the 25 min timeout (36 of the 61
// never-arrived ones were >500 yd far-ring picks), 43 "replaced" (the WANDER_NPC 5 min timer of AC NewRpgAction.cpp:327
// fired in the first active window after a sleep at the NPC). Bots without an errand keep AC's rotation. May run from a
// group member's thread (PARTY_ACTIVITY): only reads this bot's entry under stateLock.
bool KeepsActive(Player* bot)
{
    if (!CfgEnable())
        return false;
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    return itr != states.end() && itr->second.errand.active && GetMSTimeDiffToNow(itr->second.errand.legMs) < ACTIVE_CAP_MS;
}

// Called at every IDLE draw: a new draw ends any errand the previous one started (Watch normally logged it already).
void Clear(Player* bot)
{
    if (!CfgEnable())
        return;
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr != states.end() && itr->second.errand.active)
        EndForStatus(bot, itr->second.errand, RPG_IDLE);
}

// #1004 (BEYOND AC): a far pick flies to the flight node nearest the NPC first, the way a player gets to a capital. Uses
// the bot's own TRAVEL_FLIGHT (walk to the nearest flight master, SendLearnNewTaxiNode, ActivateTaxiPathTo), only with a
// fixed destination instead of the random one. Worth it only when the walk to the flight master plus the walk from the
// destination node is well under the direct walk, and every node on the route has a mount for the bot's team (#887).
bool PlanFlight(Player* bot, Pick const& pick, Errand& errand)
{
    float const minDist = CfgFlightMin();
    if (minDist <= 0.0f || pick.dist <= minDist)
        return false;
    TravelMgr::FlightMasterInfo const* fm = sTravelMgr.GetNearestFlightMasterInfo(bot);
    if (!fm || !fm->taxiNodeId)
        return false;
    uint32 const team = bot->GetTeam();
    uint32 const mountIdx = team == ALLIANCE ? 1 : 0;
    TaxiNodesEntry const* fromNode = sTaxiNodesStore.LookupEntry(fm->taxiNodeId);
    if (!fromNode || !fromNode->MountCreatureID[mountIdx])
        return false;
    float const fmDist = bot->GetExactDist2d(fm->pos.GetPositionX(), fm->pos.GetPositionY());

    uint32 bestNode = 0;
    float bestDist = 0.0f;
    for (uint32 nodeId : sTravelMgr.GetFlightNodesInZone(errand.zoneId, team, fm->taxiNodeId))
    {
        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
        if (!node || node->map_id != bot->GetMapId() || !node->MountCreatureID[mountIdx])
            continue;
        float const d = std::hypot(node->x - pick.spawn->x, node->y - pick.spawn->y);
        if (!bestNode || d < bestDist)
        {
            bestNode = nodeId;
            bestDist = d;
        }
    }
    if (!bestNode || fmDist + bestDist > pick.dist * 0.6f)
        return false;

    std::vector<uint32> path = sTravelNodeMap.FindTaxiPath(fm->taxiNodeId, bestNode);
    if (path.empty())
        return false;
    for (uint32 nodeId : path)
    {
        TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
        if (!node || !node->MountCreatureID[mountIdx])
            return false;
    }

    errand.flightPending = true;
    errand.flightFmEntry = fm->templateEntry;
    errand.flightFmPos = fm->pos;
    errand.flightPath = std::move(path);
    errand.budgetMs = LegBudgetMs(fmDist + bestDist, FLIGHT_ALLOW_MS);
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s flight planned fm=%u node %u -> %u fm-dist=%.0f node-to-npc=%.0f hops=%u", bot->GetName().c_str(),
                 fm->templateEntry, fm->taxiNodeId, bestNode, fmDist, bestDist, uint32(errand.flightPath.size()));
    return true;
}

// #1004: the flight to take for the errand TryStart just picked (the caller then runs TRAVEL_FLIGHT instead of GO_CAMP).
bool TakeFlight(Player* bot, uint32& fmEntry, WorldPosition& fmPos, std::vector<uint32>& path)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr == states.end() || !itr->second.errand.active || !itr->second.errand.flightPending || itr->second.errand.landedMs)
        return false;
    Errand& errand = itr->second.errand;
    fmEntry = errand.flightFmEntry;
    fmPos = errand.flightFmPos;
    path = errand.flightPath;
    return true;
}

// #1004: TRAVEL_FLIGHT landed (NewRpgAction.cpp): true and `stand` set when it was an errand's flight, which resumes as
// GO_CAMP to the NPC with a fresh time budget for the remaining walk.
bool OnFlightLanded(Player* bot, WorldPosition& stand)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr == states.end() || !itr->second.errand.active || !itr->second.errand.flightPending)
        return false;
    Errand& errand = itr->second.errand;
    errand.flightPending = false;
    errand.landedMs = getMSTime();
    errand.legMs = errand.landedMs;
    float const left = errand.stand.getMapId() == bot->GetMapId()
                           ? bot->GetExactDist2d(errand.stand.GetPositionX(), errand.stand.GetPositionY()) : 0.0f;
    errand.budgetMs = LegBudgetMs(left);
    stand = errand.stand;
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s flight landed npc=%u after %u s, %.0f yd left, budget %u s", bot->GetName().c_str(), errand.entry,
                 GetMSTimeDiffToNow(errand.startMs) / IN_MILLISECONDS, left, errand.budgetMs / IN_MILLISECONDS);
    return true;
}

// Picks an errand for this IDLE draw; on true `pos` is the stand point to GO_CAMP to.
bool TryStart(PlayerbotAI* botAI, Player* bot, WorldPosition& pos)
{
    if (!Enabled(bot))
        return false;

    std::unordered_map<uint32, uint32> targeted;
    std::unordered_map<uint32, uint32> npcLoad;
    {
        std::lock_guard<std::mutex> guard(stateLock);
        BotState& state = states[bot->GetGUID()];
        // #945: an errand whose GO_CAMP is still running is never replaced by a new search
        if (state.errand.active)
            return false;
        if (!state.init)
        {
            // stagger the first errand over one cooldown so a freshly started server does not send everyone at once
            state.init = true;
            state.lastMs = getMSTime() - urand(0, CfgCooldownMs() / IN_MILLISECONDS) * IN_MILLISECONDS;
        }
        if (GetMSTimeDiffToNow(state.lastMs) < CfgCooldownMs())
            return false;
        if (state.hasNone && GetMSTimeDiffToNow(state.noneMs) < NONE_COOLDOWN_MS)
            return false;

        for (auto it = state.targeted.begin(); it != state.targeted.end();)
        {
            if (GetMSTimeDiffToNow(it->second) >= RETARGET_COOLDOWN_MS)
                it = state.targeted.erase(it);
            else
                ++it;
        }
        targeted = state.targeted;
        for (auto const& other : states)
            if (other.second.errand.active)
                ++npcLoad[other.second.errand.spawnId];
    }

    // Needs, first that holds and finds an NPC wins. Values are the bot's own (outside the lock).
    PlayerbotAI* ai = botAI;
    AiObjectContext* ctx = ai->GetAiObjectContext();
    std::vector<std::pair<uint32, uint32>> plan;  // (need, wanted categories)
    if (ctx->GetValue<bool>("should repair")->Get() && ctx->GetValue<bool>("can repair")->Get())
        plan.emplace_back(NEED_REPAIR, CAT_REPAIR);
    if (ctx->GetValue<bool>("should sell")->Get() && ctx->GetValue<bool>("can sell")->Get())
        plan.emplace_back(NEED_SELL, CAT_VENDOR);
    // profession trainers of the known professions: a need when something is learnable now, else a random-errand category
    std::vector<uint32> profSkills = KnownProfessions(bot);
    if (ProfessionLearnableNow(botAI, bot, profSkills))
        plan.emplace_back(NEED_PROF, CAT_PROF);
    if (urand(1, 100) <= CfgChance())
    {
        // Owner request 2026-10-28: draw ONE category by RpgTownErrandWeight*, then (near ring, far ring) search just that
        // category; when it finds nothing the loop below falls to the next category drawn by weight. Drawn without
        // replacement, so the plan is the whole weighted order. Stable masters only for hunters, guild banks only for
        // guilded bots, profession trainers only when the bot knows a profession.
        std::vector<uint32> cats = {CAT_AUCTION, CAT_PROF, CAT_BANK, CAT_MAILBOX, CAT_CLASS};
        if (bot->GetClass() == CLASS_HUNTER)
            cats.push_back(CAT_STABLE);
        if (bot->GetGuildId())
            cats.push_back(CAT_GUILDBANK);
        if (profSkills.empty())
            cats.erase(std::remove(cats.begin(), cats.end(), uint32(CAT_PROF)), cats.end());
        while (!cats.empty())
        {
            uint32 weightSum = 0;
            for (uint32 cat : cats)
                weightSum += CfgCategoryWeight(cat);
            if (!weightSum)
                break;
            uint32 roll = urand(1, weightSum);
            size_t idx = 0;
            for (; idx < cats.size(); ++idx)
            {
                uint32 const w = CfgCategoryWeight(cats[idx]);
                if (roll <= w)
                    break;
                roll -= w;
            }
            if (idx >= cats.size())
                break;
            // Owner order 2026-10-28 (auction house, then profession trainer, bank, mailbox): a mailbox only when the
            // draw picked it first, never as a fallback - every town has one, so falling through to it made mailboxes
            // as common as auctioneers (42 vs 40 in 40 min at 1000 bots) although their weight is 15 against 40.
            if (cats[idx] != CAT_MAILBOX || plan.empty() || plan.back().first != NEED_ERRAND)
                plan.emplace_back(NEED_ERRAND, cats[idx]);
            cats.erase(cats.begin() + idx);
        }
    }

    // Rings: the local town first (RpgTownErrandNearRadius), widen to RpgTownErrandRadius only when nothing qualifies.
    float const maxRadius = CfgRadius();
    float const nearRadius = std::min(CfgNearRadius(), maxRadius);
    Pick pick;
    uint32 need = NEED_NONE;
    bool farRing = false;
    for (auto const& entry : plan)
    {
        if (nearRadius > 0.0f && Search(botAI, bot, entry.first, entry.second, profSkills, targeted, npcLoad, nearRadius, pick))
        {
            need = entry.first;
            break;
        }
        // Banks stand only in capitals and a few towns, like auction houses: search them as far (v16 census: bank 11 vs mailbox
        // 16 per hour, because a drawn bank errand mostly found none in RpgTownErrandRadius and fell through).
        float const outerRadius = (entry.second == CAT_AUCTION || entry.second == CAT_BANK) ? std::max(maxRadius, CfgAuctionRadius()) : maxRadius;
        if (nearRadius < outerRadius && Search(botAI, bot, entry.first, entry.second, profSkills, targeted, npcLoad, outerRadius, pick))
        {
            need = entry.first;
            farRing = true;
            break;
        }
    }

    std::lock_guard<std::mutex> guard(stateLock);
    BotState& state = states[bot->GetGUID()];
    if (need == NEED_NONE)
    {
        if (!plan.empty())
        {
            state.hasNone = true;
            state.noneMs = getMSTime();
            TC_LOG_DEBUG("playerbots", "[ERRAND] %s give-up: no matching NPC (needs %u, map %u zone %u)",
                         bot->GetName().c_str(), uint32(plan.size()), bot->GetMapId(), bot->GetZoneId());
        }
        return false;
    }

    state.hasNone = false;
    state.lastMs = getMSTime();
    state.targeted[pick.spawn->spawnId] = state.lastMs;
    Errand& errand = state.errand;
    errand = Errand();
    errand.active = true;
    errand.need = need;
    errand.cat = pick.cat;
    errand.entry = pick.spawn->entry;
    errand.spawnId = pick.spawn->spawnId;
    errand.pos = WorldPosition(bot->GetMapId(), pick.spawn->x, pick.spawn->y, pick.spawn->z);
    // Stand point ~STAND_DIST yd in front of the NPC along its orientation (as the TravelMgr hub entries: x + cos(o) * d),
    // z from the spawn + 0.5, so GO_CAMP's 10 yd arrival test is met; the spawn itself when that point is in water.
    float const sx = pick.spawn->x + std::cos(pick.spawn->o) * STAND_DIST;
    float const sy = pick.spawn->y + std::sin(pick.spawn->o) * STAND_DIST;
    float const sz = pick.spawn->z + 0.5f;
    errand.stand = bot->GetMap()->IsInWater(bot->GetPhaseMask(), sx, sy, pick.spawn->z) ? errand.pos
                                                                                          : WorldPosition(bot->GetMapId(), sx, sy, sz);
    errand.startMs = state.lastMs;
    errand.legMs = errand.startMs;
    errand.budgetMs = LegBudgetMs(pick.dist);  // #1004: scaled with the distance (base 10 min + dist / 3 yd/s, cap 45 min)
    errand.zoneId = bot->GetMap()->GetZoneId(bot->GetPhaseMask(), pick.spawn->x, pick.spawn->y, pick.spawn->z);
    if (bot->GetZoneId() == errand.zoneId)
        errand.zoneMs = errand.startMs;
    PlanFlight(bot, pick, errand);
    errand.stayMs = (need == NEED_ERRAND ? urand(CfgStayMinS(), CfgStayMaxS()) : urand(8, 15)) * IN_MILLISECONDS;
    pos = errand.stand;

    CreatureTemplate const* tmpl = pick.spawn->isGo ? nullptr : sObjectMgr->GetCreatureTemplate(errand.entry);
    GameObjectTemplate const* goTmpl = pick.spawn->isGo ? sObjectMgr->GetGameObjectTemplate(errand.entry) : nullptr;
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s pick need=%s npc=%u (%s) cat=0x%x dist=%.0f ring=%s stay=%us travel=%s budget=%u s", bot->GetName().c_str(),
                 NeedName(need), errand.entry, tmpl ? tmpl->Name.c_str() : goTmpl ? goTmpl->name.c_str() : "?", pick.cat, pick.dist, farRing ? "far" : "near",
                 errand.stayMs / IN_MILLISECONDS, errand.flightPending ? "flight" : "walk", errand.budgetMs / IN_MILLISECONDS);
    return true;
}

// GO_CAMP arrival (NewRpgAction.cpp): true and `npc` set when this camp pos was an errand and the NPC is there.
bool OnArrive(Player* bot, ObjectGuid& npc)
{
    uint32 entry = 0;
    uint32 spawnId = 0;
    {
        std::lock_guard<std::mutex> guard(stateLock);
        auto itr = states.find(bot->GetGUID());
        if (itr == states.end() || !itr->second.errand.active)
            return false;
        Errand& errand = itr->second.errand;
        if (GetMSTimeDiffToNow(errand.legMs) > errand.budgetMs)
        {
            EndErrand(bot, errand, "timeout");
            return false;
        }
        if (errand.pos.getMapId() != bot->GetMapId() || bot->GetExactDist(errand.pos) > 40.0f)
        {
            EndErrand(bot, errand, "unreachable", " (camp pos reached away from the npc)");
            return false;
        }
        entry = errand.entry;
        spawnId = errand.spawnId;
    }

    WorldObject* found = nullptr;
    if (spawnId & GO_KEY_BIT)
    {
        // gameobject errand (mailbox): the same spawn, else any gameobject of the entry in range
        std::list<GameObject*> gos;
        bot->GetGameObjectListWithEntryInGrid(gos, entry, 60.0f);
        for (GameObject* go : gos)
        {
            if (!go || !go->IsInWorld())
                continue;
            if (go->GetDBTableGUIDLow() == (spawnId & ~GO_KEY_BIT))
            {
                found = go;
                break;
            }
            if (!found)
                found = go;
        }
    }
    else
    {
        for (Creature* creature : bot->FindNearestCreatures(entry, 60.0f))
        {
            if (!creature || !creature->IsAlive() || !creature->IsInWorld())
                continue;
            if (creature->GetSpawnId() == spawnId)
            {
                found = creature;
                break;
            }
            if (!found)
                found = creature;
        }
    }

    std::lock_guard<std::mutex> guard(stateLock);
    Errand& errand = states[bot->GetGUID()].errand;
    if (!found)
    {
        TC_LOG_DEBUG("playerbots", "[ERRAND] %s give-up: npc/go %u not found at the camp pos (dead or despawned)",
                     bot->GetName().c_str(), entry);
        EndErrand(bot, errand, "unreachable", " (npc gone)");
        return false;
    }

    errand.npc = found->GetGUID();
    errand.arriveMs = getMSTime();
    npc = errand.npc;
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s arrival need=%s npc=%u cat=0x%x dist=%.1f", bot->GetName().c_str(), NeedName(errand.need),
                 entry, errand.cat, bot->GetExactDist(found));
    return true;
}

// True while `npc` is this bot's active errand target.
bool IsErrandNpc(Player* bot, ObjectGuid npc)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    return itr != states.end() && itr->second.errand.active && itr->second.errand.npc == npc;
}

// The errand was seeded but the bot never got to the NPC (blocked path, NPC gone): end it.
bool Expired(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    auto itr = states.find(bot->GetGUID());
    if (itr == states.end() || !itr->second.errand.active || !itr->second.errand.arriveMs)
        return false;
    Errand& errand = itr->second.errand;
    if (!errand.reachedMs && GetMSTimeDiffToNow(errand.arriveMs) > REACH_TIMEOUT_MS)
    {
        TC_LOG_DEBUG("playerbots", "[ERRAND] %s give-up: could not reach npc %u within %u s", bot->GetName().c_str(), errand.entry,
                     REACH_TIMEOUT_MS / IN_MILLISECONDS);
        EndErrand(bot, errand, "timeout", " (reach)");
        return true;
    }
    return false;
}

// Interaction distance reached: remember it; returns the need so the caller knows which work to do.
uint32 MarkReached(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    Errand& errand = states[bot->GetGUID()].errand;
    errand.reachedMs = getMSTime();
    return errand.need;
}

uint32 StayMs(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    return states[bot->GetGUID()].errand.stayMs;
}

uint32 CurrentCat(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    return states[bot->GetGUID()].errand.cat;
}

void Finish(Player* bot)
{
    std::lock_guard<std::mutex> guard(stateLock);
    Errand& errand = states[bot->GetGUID()].errand;
    TC_LOG_DEBUG("playerbots", "[ERRAND] %s done need=%s npc=%u stayed %u s", bot->GetName().c_str(), NeedName(errand.need),
                 errand.entry, (errand.reachedMs ? GetMSTimeDiffToNow(errand.reachedMs) : 0) / IN_MILLISECONDS);
    errand.active = false;
}
}  // namespace TownErrand
// ============================== end #945 (town errands) ==============================

bool NewRpgBaseAction::RandomChangeStatus(std::vector<NewRpgStatus> candidateStatus)
{
    // Mirrors AC NewRpgBaseAction::RandomChangeStatus: pick from the candidate
    // statuses by RpgStatusProbWeight. RandomBotRpgChance is deliberately not
    // consulted here - in AC it drives RandomPlayerbotMgr's teleport/strategy
    // scheduling, not the RPG status machine.

    // HARNESS-SPEC-botscenario-v2 G2: .botscenario allow-mask - only the listed statuses may be
    // drawn (the other weights count as 0). No-op unless a scenario is running with a mask set.
    BotScenario::FilterStatuses(bot, candidateStatus);

    // #872 (#843 extension): a new draw ends any quest hub trip the previous one started.
    if (sPlayerbotAIConfig->rpgQuestHubSearchEnable)
        QuestHubSearch::ClearTrip(bot);

    // #945 (BEYOND AC): a new draw ends any town errand the previous one started; then a bot with a need (repair, sell,
    // profession rank) or, by chance, a random errand goes to a matching NPC with the GO_CAMP travel. The GO_CAMP arrival
    // (NewRpgAction.cpp) seeds WANDER_NPC with that NPC. Only at the IDLE draw (GO_CAMP is a candidate there).
    TownErrand::Clear(bot);
    if (std::find(candidateStatus.begin(), candidateStatus.end(), RPG_GO_CAMP) != candidateStatus.end() &&
        !BotScenario::IsStatusLocked(bot))
    {
        WorldPosition errandPos;
        if (TownErrand::TryStart(botAI, bot, errandPos))
        {
            uint32 flightFm = 0;
            WorldPosition flightFmPos;
            std::vector<uint32> flightPath;
            if (TownErrand::TakeFlight(bot, flightFm, flightFmPos, flightPath))  // #1004: far pick, fly first
            {
                botAI->GetRpgInfo().ChangeToTravelFlight(flightFm, flightFmPos, std::move(flightPath));
                return true;
            }
            botAI->GetRpgInfo().ChangeToGoCamp(errandPos);
            BotScenario::LogTarget(bot, "camp", nullptr, errandPos.GetPositionX(), errandPos.GetPositionY(),
                                   errandPos.GetPositionZ());
            return true;
        }
    }

    std::vector<NewRpgStatus> availableStatus;
    uint32 probSum = 0;
    for (NewRpgStatus status : candidateStatus)
    {
        if (sPlayerbotAIConfig->RpgStatusProbWeight[status] == 0)
            continue;

        if (CheckRpgStatusAvailable(status))
        {
            availableStatus.push_back(status);
            probSum += sPlayerbotAIConfig->RpgStatusProbWeight[status];
        }
    }

    // AC parity (issue #61): AC has no data-free fallback here - REST is always a candidate, so the
    // available list is never actually empty. Removed per the #61 port spec (section 4.1).

    // Safety check. Default to "rest" if all RPG weights = 0
    if (availableStatus.empty() || probSum == 0)
    {
        botAI->GetRpgInfo().ChangeToRest();
        bot->SetStandState(UNIT_STAND_STATE_SIT);
        return true;
    }
    uint32 rand = urand(1, probSum);
    uint32 accumulate = 0;
    NewRpgStatus chosenStatus = RPG_STATUS_END;
    for (NewRpgStatus status : availableStatus)
    {
        accumulate += sPlayerbotAIConfig->RpgStatusProbWeight[status];
        if (accumulate >= rand)
        {
            chosenStatus = status;
            break;
        }
    }

    switch (chosenStatus)
    {
        case RPG_WANDER_RANDOM:
        {
            botAI->GetRpgInfo().ChangeToWanderRandom();
            return true;
        }
        case RPG_WANDER_NPC:
        {
            botAI->GetRpgInfo().ChangeToWanderNpc();
            return true;
        }
        case RPG_GO_GRIND:
        {
            WorldPosition pos = SelectRandomGrindPos(bot);
            if (pos != WorldPosition())
            {
                botAI->GetRpgInfo().ChangeToGoGrind(pos);
                BotScenario::LogTarget(bot, "grind", nullptr, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                return true;
            }
            return false;
        }
        case RPG_GO_CAMP:
        {
            WorldPosition pos = SelectRandomCampPos(bot);
            if (pos != WorldPosition())
            {
                botAI->GetRpgInfo().ChangeToGoCamp(pos);
                BotScenario::LogTarget(bot, "camp", nullptr, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
                return true;
            }
            return false;
        }
        case RPG_DO_QUEST:
        {
            std::vector<uint32> availableQuests;
            for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 questId = bot->GetQuestSlotQuestId(slot);
                if (!questId)
                    continue;

                if (botAI->GetLowPriorityQuests().count(questId))
                    continue;

                std::vector<POIInfo> poiInfo;
                if (GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
                    availableQuests.push_back(questId);
            }

            if (!availableQuests.empty())
            {
                uint32 questId = availableQuests[urand(0, availableQuests.size() - 1)];
                if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
                {
                    botAI->GetRpgInfo().ChangeToDoQuest(questId, quest);
                    return true;
                }
            }

            // MoP/Cata extension, owner 2026-10-16, #843 (see QuestHubSearch above): no doable quest in the log, so
            // go and take one - GO_CAMP travel to the giver, accepted there by AC's SearchQuestGiverAndAcceptOrReward.
            if (availableQuests.empty() && QuestHubSearch::Enabled(bot))
            {
                auto canTake = [this](Quest const* quest)
                {
                    return bot->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_NONE && bot->CanTakeQuest(quest, false) &&
                           bot->CanAddQuest(quest, false) && IsQuestWorthDoing(quest) && IsQuestCapableDoing(quest);
                };
                QuestHubSearch::Pick pick;
                if (QuestHubSearch::Find(bot, canTake, pick))
                {
                    QuestHubSearch::MarkTargeted(bot, pick);
                    botAI->GetRpgInfo().ChangeToGoCamp(pick.pos);
                    BotScenario::LogTarget(bot, "camp", nullptr, pick.pos.GetPositionX(), pick.pos.GetPositionY(),
                                           pick.pos.GetPositionZ());
                    TC_LOG_DEBUG("playerbots", "[New RPG] %s quest hub search -> giver %u quest %u dist %.0f",
                                 bot->GetName().c_str(), pick.entry, pick.questId, pick.dist);
                    return true;
                }
            }

            return false;
        }
        case RPG_TRAVEL_FLIGHT:
        {
            uint32 flightMasterEntry = 0;
            WorldPosition flightMasterPos;
            std::vector<uint32> path;
            if (SelectRandomFlightTaxiNode(flightMasterEntry, flightMasterPos, path))
            {
                botAI->GetRpgInfo().ChangeToTravelFlight(flightMasterEntry, flightMasterPos, path);
                return true;
            }

            return false;
        }
        case RPG_IDLE:
        {
            botAI->GetRpgInfo().ChangeToIdle();
            return true;
        }
        case RPG_REST:
        {
            botAI->GetRpgInfo().ChangeToRest();
            bot->SetStandState(UNIT_STAND_STATE_SIT);
            return true;
        }
        case RPG_OUTDOOR_PVP:
        {
            // The outdoor-pvp sub-action lives in NewRpgAction.cpp; leaving capturePointSpawnId at
            // 0 makes the action pick its own objective on the first tick it runs.
            botAI->GetRpgInfo().ChangeToOutdoorPvp();
            return true;
        }
        case RPG_DUEL_SPOT:
        {
            // #201 (#612): AC NewRpgBaseAction.cpp:1319-1322 (outdoor pvp) pattern.
            int32 idx = sDuelSpotMgr->SelectSpotFor(bot);
            if (idx < 0)
                return false;
            botAI->GetRpgInfo().ChangeToDuelSpot(uint32(idx));
            return true;
        }
        default:
        {
            botAI->GetRpgInfo().ChangeToRest();
            bot->SetStandState(UNIT_STAND_STATE_SIT);
            return true;
        }
    }
    return false;
}

namespace
{
// MoP 5.4.8 ships no src/server/game/Maps/AreaDefines.h, so AC's AREA_NAGRAND is not available
// by name; 3518 is the Nagrand zone id (Halaa, OutdoorPvPNA).
constexpr uint32 AREA_NAGRAND = 3518;
} // namespace

bool NewRpgBaseAction::CheckRpgStatusAvailable(NewRpgStatus status)
{
    // issue #97: the world-roaming layer must not run inside a battleground. It picks grind/camp
    // targets from the world farm cache, and inside a BG it selected spots on the battleground map
    // itself ("[New RPG] Bot Catha select random grind pos Map:489 X:949.9 Y:1459.2"), which walked
    // bots across the whole map - through the closed gates during WAIT_JOIN. "new rpg" is also
    // removed from the strategy set on entering a match (BGStrategyCheckAction); this is the central
    // guard so no stray call can start a world objective inside a match either.
    if (bot->InBattleground())
        return false;
    switch (status)
    {
        case RPG_IDLE:
        case RPG_REST:
            return true;
        case RPG_WANDER_RANDOM:
        {
            Unit* target = AI_VALUE(Unit*, "grind target");
            return target != nullptr;
        }
        case RPG_GO_GRIND:
        {
            WorldPosition pos = SelectRandomGrindPos(bot);
            return pos != WorldPosition();
        }
        case RPG_GO_CAMP:
        {
            WorldPosition pos = SelectRandomCampPos(bot);
            return pos != WorldPosition();
        }
        case RPG_WANDER_NPC:
        {
            // Upstream wants a handful of NPCs around before it is worth walking between them.
            GuidVector possibleTargets = AI_VALUE(GuidVector, "possible new rpg targets");
            return possibleTargets.size() >= 3;
        }
        case RPG_DO_QUEST:
        {
            for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
            {
                uint32 questId = bot->GetQuestSlotQuestId(slot);
                if (!questId)
                    continue;

                if (botAI->GetLowPriorityQuests().count(questId))
                    continue;

                std::vector<POIInfo> poiInfo;
                if (GetQuestPOIPosAndObjectiveIdx(questId, poiInfo, true))
                    return true;
            }

            // MoP/Cata extension, owner 2026-10-16, #843 (see QuestHubSearch above): DO_QUEST stays a candidate when a
            // giver in the bot's zone has a quest it can take now (RandomChangeStatus then travels there).
            if (QuestHubSearch::Enabled(bot))
            {
                auto canTake = [this](Quest const* quest)
                {
                    return bot->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_NONE && bot->CanTakeQuest(quest, false) &&
                           bot->CanAddQuest(quest, false) && IsQuestWorthDoing(quest) && IsQuestCapableDoing(quest);
                };
                QuestHubSearch::Pick pick;
                if (QuestHubSearch::Find(bot, canTake, pick))
                    return true;
            }

            return false;
        }
        case RPG_TRAVEL_FLIGHT:
        {
            uint32 flightMasterEntry = 0;
            WorldPosition flightMasterPos;
            std::vector<uint32> path;
            return SelectRandomFlightTaxiNode(flightMasterEntry, flightMasterPos, path);
        }
        case RPG_OUTDOOR_PVP:
        {
            if (!bot->IsPvP())
                return false;
            uint32 zoneId = bot->GetZoneId();
            if (zoneId == AREA_NAGRAND)
                return false;

            OutdoorPvP* outdoorPvP = sOutdoorPvPMgr->GetOutdoorPvPToZoneId(zoneId);
            return outdoorPvP != nullptr;
        }
        case RPG_DUEL_SPOT:
        {
            // #201 (#612): AC NewRpgBaseAction.cpp:1385-1395 (outdoor pvp) pattern.
            return sPlayerbotAIConfig->duelSpotEnable && botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT) &&
                   sDuelSpotMgr->SelectSpotFor(bot) >= 0;
        }
        default:
            return false;
    }
    return false;
}
