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
#include <mutex>  // #843 quest hub search
#include <unordered_map>
#include <vector>

#include "BotScenario.h"
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

std::once_flag indexOnce;
std::unordered_map<uint32, std::vector<StarterSpawn>> spawnsByMap;  // written once under indexOnce, then read-only
std::mutex stateLock;
std::unordered_map<ObjectGuid, BotState> states;

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

        float const dx = spawn.x - bot->GetPositionX();
        float const dy = spawn.y - bot->GetPositionY();
        float const distSq = dx * dx + dy * dy;
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
