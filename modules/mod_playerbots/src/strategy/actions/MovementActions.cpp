/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MovementActions.h"

#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <string>
#include <algorithm>
#include <unordered_map>

#include "Event.h"
#include "FleeManager.h"
#include "PositionValue.h"
#include "G3D/Vector3.h"
#include "GameObject.h"
#include "LastMovementValue.h"
#include "Map.h"
#include "MMapFactory.h"
#include "MotionMaster.h"
#include "MoveSplineInitArgs.h"
#include "MovementGenerator.h"
#include "ObjectDefines.h"
#include "ObjectGuid.h"
#include "PathGenerator.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "WorldPosition.h"
#include "PlayerbotSpec.h"
#include "Position.h"

#include "Random.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"

#include "Timer.h"
#include "Transport.h"
#include "Unit.h"
#include "Vehicle.h"
#include "WaypointMovementGenerator.h"
#include "Corpse.h"
#include "Group.h"
#include "PetsAction.h"

MovementAction::MovementAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name)
{
    bot = botAI->GetBot();
}

void MovementAction::ClearIdleState()
{
    context->GetValue<time_t>("stay time")->Set(0);
    context->GetValue<PositionMap&>("position")->Get()["random"].Reset();
}

void MovementAction::WaitForReach(float distance)
{
    float delay = 1000.0f * MoveDelay(distance);

    if (delay > sPlayerbotAIConfig->maxWaitForMove)
        delay = sPlayerbotAIConfig->maxWaitForMove;

    Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
    Unit* player = *botAI->GetAiObjectContext()->GetValue<Unit*>("enemy player target");
    if ((player || target) && delay > sPlayerbotAIConfig->globalCoolDown)
        delay = sPlayerbotAIConfig->globalCoolDown;

    if (delay < 0)
        delay = 0;

    botAI->SetNextCheckDelay((uint32)delay);
}

// #590: swim / water-walk flag refresh, called from PlayerbotAI::UpdateAIInternal.
// UpdateMovementState only runs when a movement action starts, so a bot that left the water and then stopped
// moving kept MOVEMENTFLAG_SWIMMING on dry land (swim speed, CanFishValue false). AC's core refreshes this
// itself; this fork's core does not. Same liquid query as UpdateMovementState, ~500 ms per bot.
void RefreshBotLiquidMovementFlags(Player* bot)
{
    static thread_local std::unordered_map<ObjectGuid, uint32> s_lastRefresh;
    uint32 const now = getMSTime();
    uint32& last = s_lastRefresh[bot->GetGUID()];
    if (last && getMSTimeDiff(last, now) < 500)
        return;
    last = now;

    if (!bot->IsAlive() || bot->GetTransport() || bot->GetVehicle() || bot->GetVehicleKit() ||
        bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) || bot->HasUnitMovementFlag(MOVEMENTFLAG_CAN_FLY) ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL) || bot->HasUnitState(UNIT_STATE_ROOT) || bot->isFrozen() ||
        bot->IsPolymorphed() || bot->IsBeingTeleported())
        return;

    ZLiquidStatus const liquidState = bot->GetLiquidStatus();
    bool const isSwimming = bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    bool const isWaterWalking = bot->HasUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);

    if (liquidState == LIQUID_MAP_NO_WATER)
    {
        if (isSwimming)
            bot->SetSwim(false);
        if (isWaterWalking)
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
    }
    else if ((liquidState & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)) && !isSwimming && !isWaterWalking &&
             !bot->HasWaterWalkAura())
        bot->SetSwim(true);
}

void MovementAction::UpdateMovementState()
{
    // AC parity (#134/#61): AC MovementAction::UpdateMovementState (Ai/Base/Actions/MovementActions.cpp).
    // The older version kept here only toggled MOVEMENTFLAG_FLYING while already above ground, so a bot on a
    // flying mount never took off: it rode the flying mount on the ground (CanFly=1, IsFlying=0), took
    // ground paths into navmesh pockets and never triggered the flyer logic (issue-61-wander-pocket-stall.md).
    // Not ported: AC's wasMovementRestricted member (an animation refresh after CC ends), which would need a
    // header change; flags are still left untouched while the bot is restricted, as on AC.
    bool const isCurrentlyRestricted = bot->HasUnitState(UNIT_STATE_LOST_CONTROL) ||
                                       bot->HasUnitState(UNIT_STATE_ROOT) || bot->isFrozen() ||
                                       bot->IsPolymorphed();
    if (isCurrentlyRestricted || !bot->IsAlive())
        return;

    Player* master = botAI ? botAI->GetMaster() : nullptr;
    ZLiquidStatus const liquidState = bot->GetLiquidStatus();
    float const gZ = bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    bool const onGroundZ = bot->GetPositionZ() < gZ + 1.0f;
    bool const wantsSwim = (liquidState & LIQUID_MAP_IN_WATER) || (liquidState & LIQUID_MAP_UNDER_WATER);
    bool const wantsFly = bot->HasIncreaseMountedFlightSpeedAura() || bot->HasFlyAura();
    bool const canWaterWalk = bot->HasWaterWalkAura();
    bool const isMasterFlying = master ? master->HasUnitMovementFlag(MOVEMENTFLAG_FLYING) : true;
    bool const isMasterSwimming = master ? master->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING) : true;
    bool const isFlying = bot->HasUnitMovementFlag(MOVEMENTFLAG_FLYING);
    bool const isSwimming = bot->HasUnitMovementFlag(MOVEMENTFLAG_SWIMMING);
    bool const isWaterWalking = bot->HasUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
    bool const hasGravityDisabled = bot->HasUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
    bool movementFlagsUpdated = false;
    uint32 const flagsBefore = bot->GetUnitMovementFlags();  // #747: see the SendMovementFlagUpdate guard below

    // handle water (AC: fragile logic, do not alter without testing every transition)
    if (liquidState != LIQUID_MAP_NO_WATER && !isFlying)
    {
        if (canWaterWalk && !isMasterSwimming && !isWaterWalking)
        {
            bot->SetSwim(false);
            bot->AddUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
            movementFlagsUpdated = true;
        }
        else if ((!canWaterWalk || isMasterSwimming) && isWaterWalking)
        {
            bot->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
            if (wantsSwim)
                bot->SetSwim(true);
            movementFlagsUpdated = true;
        }
        else if (!wantsSwim && isSwimming)
        {
            bot->SetSwim(false);
            movementFlagsUpdated = true;
        }
        else if (wantsSwim && !isSwimming && !isWaterWalking)
        {
            // Kept from the previous MoP version: enter swimming when in water (AC gets this from the core).
            bot->SetSwim(true);
            movementFlagsUpdated = true;
        }
    }

    // reset when not around water while swimming or water walking
    if (liquidState == LIQUID_MAP_NO_WATER && (isSwimming || isWaterWalking))
    {
        bot->SetSwim(false);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_WATERWALKING);
        movementFlagsUpdated = true;
    }

    // handle flying
    if (wantsFly && !isFlying && isMasterFlying)
    {
        bot->AddUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
        bot->AddUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
        bot->AddUnitMovementFlag(MOVEMENTFLAG_FLYING);
        movementFlagsUpdated = true;
    }
    else if (!wantsFly && !isWaterWalking && (isFlying || hasGravityDisabled))
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
        movementFlagsUpdated = true;
    }
    else if (!isMasterFlying && isFlying && onGroundZ)
    {
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_CAN_FLY);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_DISABLE_GRAVITY);
        bot->RemoveUnitMovementFlag(MOVEMENTFLAG_FLYING);
        movementFlagsUpdated = true;
    }

    // movement flags should only be updated between state changes, otherwise it breaks certain effects
    //
    // #747 (MoP protocol, beyond AC): a swim toggle in the middle of a spline (Twin Peaks river/lake crossings: MoveTo
    // runs this before IsWaitingForLastMove, so the running spline is not replaced) is already broadcast by
    // Unit::SetSwim as SMSG_SPLINE_MOVE_START/STOP_SWIM (Movement::PacketSender picks the spline opcode while
    // IsSplineEnabled()). The extra SMSG_PLAYER_MOVE carries no spline marker in 5.4.8 (AC's 3.3.5 heartbeat has
    // MOVEMENTFLAG_SPLINE_ENABLED), so observers would drop the spline, run the bot forward at swim/run speed until the
    // next SMSG_MONSTER_MOVE and then see it jump. Any other flag change (water walking, flying) is still sent.
    uint32 const flagsChanged = flagsBefore ^ bot->GetUnitMovementFlags();
    bool const swimOnlyMidSpline = bot->IsSplineEnabled() && !(flagsChanged & ~uint32(MOVEMENTFLAG_SWIMMING));
    if (movementFlagsUpdated && !swimOnlyMidSpline)
        bot->SendMovementFlagUpdate();
}
bool MovementAction::ChaseTo(WorldObject* obj, float distance, float angle)
{
    if (!IsMovingAllowed())
    {
        return false;
    }

    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!seat || !seat->CanControl())
            return false;

        // vehicle->GetMotionMaster()->Clear();
        vehicle->GetBase()->GetMotionMaster()->MoveChase((Unit*)obj, 30.0f);
        return true;
    }

    UpdateMovementState();

    if (!bot->IsStandState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    bot->GetMotionMaster()->MoveChase((Unit*)obj, distance);

    // TODO shouldnt this use "last movement" value?
    WaitForReach(bot->GetExactDist2d(obj) - distance);
    return true;
}

bool MovementAction::ReachCombatTo(Unit* target, float distance)
{
    if (!IsMovingAllowed(target))
        return false;

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    // #756 (AC parity, AC MovementActions.cpp:789-844, ported 1:1): this fork had the older body without the
    // fleeing-target prediction and with a 10 yd half-step threshold (AC: 6 yd). A bot chasing a running player
    // (or riding after one before the dismount distance) aimed at where the target was, not where it is going.
    // Position::NormalizeOrientation / GetAngle, Map::CheckCollisionAndGetValidCoords, MOVEMENTFLAG_FORWARD and
    // M_PI/M_PI_2 (Common.h) exist on 5.4.8 with the same signatures.
    float targetOrientation = target->GetOrientation();

    float deltaAngle = Position::NormalizeOrientation(targetOrientation - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI;  // -PI..PI
    // if target is moving forward and moving far away, predict the position
    bool behind = fabs(deltaAngle) > M_PI_2;
    if (target->HasUnitMovementFlag(MOVEMENTFLAG_FORWARD) && behind)
    {
        float predictDis = std::min(3.0f, target->GetObjectSize() * 2);
        tx += cos(target->GetOrientation()) * predictDis;
        ty += sin(target->GetOrientation()) * predictDis;
        if (!target->GetMap()->CheckCollisionAndGetValidCoords(target, target->GetPositionX(), target->GetPositionY(),
                                                               target->GetPositionZ(), tx, ty, tz))
        {
            tx = target->GetPositionX();
            ty = target->GetPositionY();
            tz = target->GetPositionZ();
        }
    }
    float combatDistance = bot->GetCombatReach() + target->GetCombatReach();
    distance += combatDistance;

    if (bot->GetExactDist(tx, ty, tz) <= distance)
        return false;

    PathGenerator path(bot);
    path.CalculatePath(tx, ty, tz, false);
    PathType type = path.GetPathType();
    int typeOk = PATHFIND_NORMAL | PATHFIND_INCOMPLETE | PATHFIND_SHORTCUT;
    if (!(type & typeOk))
        return false;
    float shortenTo = distance;

    // Avoid walking too far when moving towards each other
    float disToGo = bot->GetExactDist(tx, ty, tz) - distance;
    if (disToGo >= 6.0f)  // #756: AC :834 (this fork had 10.0f)
        shortenTo = disToGo / 2 + distance;

    path.ShortenPathUntilDist(G3D::Vector3(tx, ty, tz), shortenTo);
    G3D::Vector3 endPos = path.GetPath().back();
    return MoveTo(target->GetMapId(), endPos.x, endPos.y, endPos.z, false, false, false, false,
        MovementPriority::MOVEMENT_COMBAT, true);
}

bool MovementAction::IsDuplicateMove(uint32 mapId, float x, float y, float z)
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    // heuristic 5s
    if (lastMove.msTime + sPlayerbotAIConfig->maxWaitForMove < getMSTime() ||
        lastMove.lastMoveShort.GetExactDist(x, y, z) > 0.01f)
        return false;

    return true;
}

bool MovementAction::IsWaitingForLastMove(MovementPriority priority)
{
    LastMovement& lastMove = *context->GetValue<LastMovement&>("last movement");

    if (priority > lastMove.priority)
        return false;

    // heuristic 5s
    if (lastMove.lastdelayTime + lastMove.msTime > getMSTime())
        return true;

    return false;
}

float MovementAction::MoveDelay(float distance, bool backwards)
{
    float speed;
    if (bot->isSwimming())
    {
        speed = backwards ? bot->GetSpeed(MOVE_SWIM_BACK) : bot->GetSpeed(MOVE_SWIM);
    }
    else if (bot->IsFlying())
    {
        speed = backwards ? bot->GetSpeed(MOVE_FLIGHT_BACK) : bot->GetSpeed(MOVE_FLIGHT);
    }
    else
    {
        speed = backwards ? bot->GetSpeed(MOVE_RUN_BACK) : bot->GetSpeed(MOVE_RUN);
    }
    float delay = distance / speed;
    return delay;
}

bool MovementAction::JumpTo(uint32 mapId, float x, float y, float z, MovementPriority priority)
{
    UpdateMovementState();

    if (!IsMovingAllowed())
        return false;

    if (IsDuplicateMove(mapId, x, y, z))
        return false;

    if (IsWaitingForLastMove(priority))
        return false;

    float const speed = bot->GetSpeed(MOVE_RUN);

    MotionMaster& mm = *bot->GetMotionMaster();
    mm.Clear();
    mm.MoveJump(x, y, z, speed, speed, 1);

    AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), 1000, priority);
    return true;
}

bool MovementAction::MoveTo(WorldObject* target, float distance, MovementPriority priority)
{
    if (!IsMovingAllowed(target))
        return false;

    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float bz = bot->GetPositionZ();

    float tx = target->GetPositionX();
    float ty = target->GetPositionY();
    float tz = target->GetPositionZ();

    float distanceToTarget = bot->GetDistance(target);
    float angle = bot->GetAngle(target);
    float needToGo = distanceToTarget - distance;

    float maxDistance = sPlayerbotAIConfig->spellDistance;
    if (needToGo > 0 && needToGo > maxDistance)
        needToGo = maxDistance;
    else if (needToGo < 0 && needToGo < -maxDistance)
        needToGo = -maxDistance;

    float dx = cos(angle) * needToGo + bx;
    float dy = sin(angle) * needToGo + by;
    float dz;  // = std::max(bz, tz); // calc accurate z position to avoid stuck
    if (distanceToTarget > CONTACT_DISTANCE)
    {
        dz = bz + (tz - bz) * (needToGo / distanceToTarget);
    }
    else
    {
        dz = tz;
    }
    return MoveTo(target->GetMapId(), dx, dy, dz, false, false, false, false, priority);
}

bool MovementAction::MoveTo(uint32 mapId, float x, float y, float z, bool idle, bool react, bool normal_only,
    bool exact_waypoint, MovementPriority priority, bool lessDelay, bool backwards)
{
    UpdateMovementState();
    if (!IsMovingAllowed(mapId, x, y, z))
    {
        return false;
    }
    if (IsDuplicateMove(mapId, x, y, z))
    {
        return false;
    }
    if (IsWaitingForLastMove(priority))
    {
        return false;
    }
    bool generatePath = !bot->IsFlying() && !bot->isSwimming();
    // #747 (beyond AC, Cata/MoP-only map): Twin Peaks' routes cross the Horde lake and the river. With AC's
    // "no path while swimming" the bot glides in one straight line to a waypoint up to 5 points ahead, through
    // the bank or above the water. The 0726 navmesh has water-surface polys, so keep pathfinding in water there.
    if (!generatePath && !bot->IsFlying() && bot->GetMapId() == 726)
        generatePath = true;
    bool disableMoveSplinePath = sPlayerbotAIConfig->disableMoveSplinePath >= 2 ||
        (sPlayerbotAIConfig->disableMoveSplinePath == 1 && bot->InBattleground());
    if (Vehicle* vehicle = bot->GetVehicle())
    {
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        Unit* vehicleBase = vehicle->GetBase();
        // #150: AC parity - MoP had this inverted (CanFly instead of !CanFly), so ground
        // vehicles skipped pathfinding (drove into walls) and flying vehicles pathfound on the
        // ground mesh.
        generatePath = !vehicleBase || !vehicleBase->CanFly();
        if (!vehicleBase || !seat || !seat->CanControl())  // is passenger and cant move anyway
            return false;

        float distance = vehicleBase->GetExactDist(x, y, z);  // use vehicle distance, not bot
        if (distance > 0.01f)
        {
            MotionMaster& mm = *vehicleBase->GetMotionMaster();  // need to move vehicle, not bot
            mm.Clear();
            if (!backwards)
            {
                mm.MovePoint(0, x, y, z, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, z, generatePath);
            }
            float speed = backwards ? vehicleBase->GetSpeed(MOVE_RUN_BACK) : vehicleBase->GetSpeed(MOVE_RUN);
            float delay = 1000.0f * (distance / speed);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
    }
    else if (exact_waypoint || disableMoveSplinePath || !generatePath)
    {
        float distance = bot->GetExactDist(x, y, z);
        if (distance > 0.01f)
        {
            // #747 (diverges from AC Ai/Base/Actions/MovementActions.cpp:214-247, which hands the point to the core
            // unchecked): mm.MovePoint(..., generatePath=true) ends in the core's MoveSplineInit::MoveTo, which
            // answers PATHFIND_NOPATH with a straight 2-point spline (Spline/MoveSplineInit.cpp:175-192). A bot is a
            // player: it cannot fly through the air or through terrain, so refuse the move instead. Flying and
            // swimming bots (generatePath=false) keep AC's straight line.
            if (generatePath && !PathIsWalkable(x, y, z))
                return false;

            if (bot->IsSitState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            MotionMaster& mm = *bot->GetMotionMaster();
            mm.Clear();
            if (!backwards)
            {
                mm.MovePoint(0, x, y, z, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, z, generatePath);
            }
            float delay = 1000.0f * MoveDelay(distance, backwards);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            AI_VALUE(LastMovement&, "last movement").Set(mapId, x, y, z, bot->GetOrientation(), delay, priority);
            return true;
        }
    }
    else
    {
        float modifiedZ;
        Movement::PointsArray path = SearchForBestPath(x, y, z, modifiedZ, sPlayerbotAIConfig->maxMovementSearchTime, normal_only);
        if (modifiedZ == INVALID_HEIGHT)
        {
            return false;
        }
        float distance = bot->GetExactDist(x, y, modifiedZ);
        if (distance > 0.01f)
        {
            if (bot->IsSitState())
                bot->SetStandState(UNIT_STAND_STATE_STAND);

            MotionMaster& mm = *bot->GetMotionMaster();
            G3D::Vector3 endP = path.back();
            mm.Clear();
            // issue #61: SearchForBestPath just computed modifiedZ as the ground-snapped, walkable
            // Z for this destination - the raw z parameter can be off by a floor in multi-level city
            // buildings, sending the pathfinder onto the wrong floor's polygon and producing a long,
            // circuitous route that looks like backtracking. Use the corrected value, matching the
            // WotLK reference.
            //
            // #747 (beyond AC, map 726 only; AC MovementActions.cpp:247-262 always hands the point to MovePoint):
            // the Twin Peaks navmesh carries river-BED polygons, so Detour routes every water leg along the bed and
            // the bots swam, fought and cast at z -13..-15 under the surface (wave 20: 80% of the swim samples in
            // x1600-1680 y240-320, median z -13.2..-15.2; players swim at the surface). MovePoint would let the core
            // rebuild the same bed path, so when any point of the path lies under liquid the module launches the
            // path itself, lifted to just under the surface, through MotionMaster::MoveSmoothPath (an
            // EffectMovementGenerator, as MoveJump; the spline speed is still picked from the bot's movement flags,
            // i.e. swim speed while swimming). Dry paths and every other map take the MovePoint route as before.
            Movement::PointsArray const unraised = path;  // #747: to find where the raised (deep-water) part starts
            bool const surfaceSwim = !backwards && bot->GetMapId() == 726 && !bot->IsFlying() && path.size() >= 2 &&
                                     RaisePathToWaterSurface(path);
            // #747 (owner 2026-10-16, "moving extremely fast" in Twin Peaks): one spline has one speed, picked at launch
            // from the bot's movement flags (MoveSplineInit::Launch SelectSpeedType). A bot on the bank is not swimming,
            // so the whole river/lake crossing ran at its run (mount) speed - ~3x a swimmer - and the probe's JUMP row
            // (run speed x dt) could not see it. A bot that is not swimming yet ends this leg at the first deep-water
            // point; there UpdateMovementState sets swimming and the next MoveTo launches the swim leg at swim speed.
            float legX = x, legY = y, legZ = modifiedZ;
            if (surfaceSwim && !bot->isSwimming() && !bot->HasWaterWalkAura())
            {
                for (size_t i = 1; i + 1 < path.size(); ++i)
                {
                    if (path[i].z == unraised[i].z)
                        continue;
                    path.resize(i + 1);
                    legX = path.back().x;
                    legY = path.back().y;
                    legZ = path.back().z;
                    distance = bot->GetExactDist(legX, legY, legZ);
                    break;
                }
            }
            if (surfaceSwim)
            {
                std::vector<Position> points;
                points.reserve(path.size());
                for (G3D::Vector3 const& p : path)
                    points.emplace_back(p.x, p.y, p.z, 0.0f);
                mm.MoveSmoothPath(0, points.data(), points.size(), false, false);
            }
            else if (!backwards)
            {
                mm.MovePoint(0, x, y, modifiedZ, generatePath);
            }
            else
            {
                mm.MovePointBackwards(0, x, y, modifiedZ, generatePath);
            }
             float delay = 1000.0f * MoveDelay(distance, backwards);
            if (lessDelay)
            {
                delay -= botAI->GetReactDelay();
            }
            delay = std::max(.0f, delay);
            delay = std::min((float)sPlayerbotAIConfig->maxWaitForMove, delay);
            // #742 (AC parity, AC MovementActions.cpp:259-260): record the ground-snapped modifiedZ the bot was
            // actually sent to, not the caller's raw z. IsDuplicateMove compares the next request's raw z with this
            // value, so with the raw z stored every repeat of the same request was refused for maxWaitForMove. A
            // mounted bot whose reach-melee move was stopped ("reset objective force" StopMoving + MotionMaster
            // Clear, BattleGroundTactics.cpp) asks for the identical spot by a standing target on the next ticks;
            // here that was a duplicate and the bot sat still on its mount ticking "melee", AC re-issues the move.
            // #747: a deep-water leg cut above records its own end (the next request for the full destination is
            // then not a duplicate and is not held for maxWaitForMove at the water's edge).
            AI_VALUE(LastMovement&, "last movement").Set(mapId, legX, legY, legZ, bot->GetOrientation(), delay, priority);
            return true;
        }
    }
    return false;
}

bool MoveRandomAction::Execute(Event event)
{
    float distance = sPlayerbotAIConfig->tooCloseDistance + urand(10, 30);
    const float x = bot->GetPositionX();
    const float y = bot->GetPositionY();
    const float z = bot->GetPositionZ();
    int attempts = 5;
    Map* map = bot->GetMap();
    while (--attempts)
    {
        float angle = (float)rand_norm() * 2 * static_cast<float>(M_PI);
        float dx = x + distance * cos(angle);
        float dy = y + distance * sin(angle);
        float dz = z;
        if (!map->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
            dx, dy, dz))
            continue;

        if (map->IsInWater(bot->GetPhaseMask(), dx, dy, dz))
            continue;

        bool moved = MoveTo(bot->GetMapId(), dx, dy, dz, false, false, false, true);
        if (moved)
            return true;
    }

    return false;
}

bool MovementAction::MoveNear(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    float angle = GetFollowAngle();
    return MoveTo(mapId, x + cos(angle) * distance, y + sin(angle) * distance, z, false, false, false, false, priority);
}

bool MovementAction::MoveNear(WorldObject* target, float distance, MovementPriority priority)
{
    if (!target)
        return false;

    distance += target->GetCombatReach();

    float x = target->GetPositionX();
    float y = target->GetPositionY();
    float z = target->GetPositionZ();
    float followAngle = GetFollowAngle();

    for (float angle = followAngle; angle <= followAngle + static_cast<float>(2 * M_PI);
        angle += static_cast<float>(M_PI / 4.f))
    {
        float x = target->GetPositionX() + cos(angle) * distance;
        float y = target->GetPositionY() + sin(angle) * distance;
        float z = target->GetPositionZ();

        if (!bot->IsWithinLOS(x, y, z))
            continue;

        bool moved = MoveTo(target->GetMapId(), x, y, z, false, false, false, false, priority);
        if (moved)
            return true;
    }

    botAI->TellError("All paths not in LOS");
    return false;
}

float MovementAction::GetFollowAngle()
{
    Player* master = GetMaster();
    Group* group = master ? master->GetGroup() : bot->GetGroup();
    if (!group)
        return 0.0f;

    // Prevent bots with orphaned raid groups from dividing by 0, which freezes the server.
    // #435: AC parity (AC Ai/Base/Actions/MovementActions.cpp:853-856). A bot alone in its BG raid
    // (groupSize=1, no master) got 2*pi/0 = inf here; MoveNear's cos/sin(inf) made the target NaN
    // and Map::GetGrid tripped the getNGrid ASSERT.
    uint32 memberCount = group->GetMembersCount();
    if (memberCount <= 1)
        return 0.0f;

    uint32 index = 1;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        if (ref->GetSource() == master)
            continue;

        if (ref->GetSource() == bot)
            return 2 * M_PI / (memberCount - 1) * index;

        ++index;
    }

    return 0;
}

bool MoveRandomAction::isUseful()
{
    // AC MovementActions.cpp:2754 (issue #609): not while an old-rpg target is set.
    return !AI_VALUE(GuidPosition, "rpg target");
}

bool MovementAction::IsMovingAllowed(WorldObject* target)
{
    if (!target)
        return false;

    if (bot->GetMapId() != target->GetMapId())
        return false;

    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed(uint32 mapId, float x, float y, float z)
{
    return IsMovingAllowed();
}

bool MovementAction::IsMovingAllowed()
{
    // do not allow if not vehicle driver
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(true))
        return false;

    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() || bot->HasRootAura() || bot->HasSpiritOfRedemptionAura() ||
        bot->HasConfuseAura() || bot->IsCharmed() || bot->HasStunAura() ||
        bot->IsInFlight() || bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
    {
        return false;
    }

    return bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FLIGHT_MOTION_TYPE;
}

bool MoveRandomAction::isPossible()
{
    if (bot->IsInCombat() ||
        !AI_VALUE(bool, "can move around") ||
        !bot->CanFreeMove() ||
        !botAI->CanMove()) return false;

    return true;
}

bool MovementAction::Follow(Unit* target, float distance) { return Follow(target, distance, GetFollowAngle()); }
bool MovementAction::Follow(Unit* target, float distance, float angle)
{
    UpdateMovementState();

    if (!target)
        return false;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
        sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    // Move to target corpse if alive.
    if (!target->IsAlive() && bot->IsAlive() && target->GetGUID().IsPlayer())
    {
        Player* pTarget = (Player*)target;

        Corpse* corpse = pTarget->GetCorpse();

        if (corpse)
        {
            WorldPosition botPos(bot);
            WorldPosition cPos(corpse);

            if (botPos.fDist(cPos) > sPlayerbotAIConfig->spellDistance)
                return MoveTo(cPos.getMapId(), cPos.getX(), cPos.getY(), cPos.getZ());
        }
    }

    if (sServerFacade->IsDistanceGreaterOrEqualThan(sServerFacade->GetDistance2d(bot, target),
        sPlayerbotAIConfig->sightDistance))
    {
        if (target->GetGUID().IsPlayer())
        {
            Player* pTarget = (Player*)target;

            PlayerbotAI* targetBotAI = GET_PLAYERBOT_AI(pTarget);
            if (targetBotAI)  // Try to move to where the bot is going if it is closer and in the same direction.
            {
                WorldPosition botPos(bot);
                WorldPosition tarPos(target);
                WorldPosition longMove =
                    targetBotAI->GetAiObjectContext()->GetValue<WorldPosition>("last long move")->Get();

                if (longMove)
                {
                    float lDist = botPos.fDist(longMove);
                    float tDist = botPos.fDist(tarPos);
                    float ang = botPos.getAngleBetween(tarPos, longMove);
                    if ((lDist * 1.5 < tDist && ang < static_cast<float>(M_PI) / 2) ||
                        target->HasUnitState(UNIT_STATE_IN_FLIGHT))
                    {
                        return MoveTo(longMove.getMapId(), longMove.getX(), longMove.getY(), longMove.getZ());
                    }
                }
            }
            else
            {
                if (pTarget->HasUnitState(UNIT_STATE_IN_FLIGHT))  // Move to where the player is flying to.
                {
                    // #179 (AC MovementActions.cpp:1196-1209, spec section 3 "follow toward a flying
                    // master's taxi destination"): this was AC's `tMap.back()` on a plain vector;
                    // this core's TaxiPathNodeList is a Path<> (Movement/Waypoints/Path.h) with no
                    // back() - the previously commented-out placeholder also called
                    // GetTotalLength() (a distance in yards) as an array index, which was never
                    // valid. The last node is tMap[tMap.size() - 1], a TaxiPathNodeEntry const&
                    // (verified against DBCStructure.h and FlightPathMovementGenerator::GetPath()).
                    TaxiPathNodeList const& tMap = static_cast<FlightPathMovementGenerator*>(pTarget->GetMotionMaster()->top())->GetPath();
                    if (!tMap.empty())
                    {
                        TaxiPathNodeEntry const& tEnd = tMap[tMap.size() - 1];
                        return MoveTo(tEnd.MapId, tEnd.LocX, tEnd.LocY, tEnd.LocZ);
                    }
                }
            }
        }

        if (!target->HasUnitState(UNIT_STATE_IN_FLIGHT))
            return MoveTo(target, sPlayerbotAIConfig->followDistance);
    }

    if (sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
        sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    if (target->IsFriendlyTo(bot) && bot->IsMounted() && AI_VALUE(GuidVector, "all targets").empty())
        distance += angle;

    if (!bot->InBattleground() && sServerFacade->IsDistanceLessOrEqualThan(sServerFacade->GetDistance2d(bot, target),
        sPlayerbotAIConfig->followDistance))
    {
        // botAI->TellError("No need to follow");
        return false;
    }

    bot->HandleEmoteCommand(0);

    if (bot->IsSitState())
        bot->SetStandState(UNIT_STAND_STATE_STAND);

    if (bot->IsNonMeleeSpellCasted(true))
    {
        bot->CastStop();
        botAI->InterruptSpell();
    }

    // AI_VALUE(LastMovement&, "last movement").Set(target);
    ClearIdleState();

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FOLLOW_MOTION_TYPE)
    {
        Unit* currentTarget = sServerFacade->GetChaseTarget(bot);
        if (currentTarget && currentTarget->GetGUID() == target->GetGUID())
            return false;
    }

    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
        bot->GetMotionMaster()->Clear();

    bot->GetMotionMaster()->MoveFollow(target, distance, angle);
    return true;
}

const Movement::PointsArray MovementAction::SearchForBestPath(float x, float y, float z, float& modified_z,
    int maxSearchCount, bool normal_only, float step)
{
    bool found = false;
    modified_z = INVALID_HEIGHT;
    // #747 (diverges from AC Ai/Base/Actions/MovementActions.cpp:1700-1760): AC snaps the destination with
    // GetMapHeight, which ignores liquid, so a waypoint on a water surface (Twin Peaks river legs: z -5.1 over a
    // -16 river bed) was sunk to the bed - 11 yd from any navmesh poly and under water, which is exactly the case
    // PathGenerator::BuildPolyPath (core PathGenerator.cpp:250-262) answers with the straight "swim shortcut"
    // (NORMAL|NOT_USING_PATH). The water-or-ground surface is where a swimming or walking bot actually ends up.
    // On dry land this is GetMapHeight (same GetHeight call), so ordinary walking is unchanged.
    //
    // Wave 25 (#747, 0726 mesh): that mesh has river-BED polygons and no water-surface polygons (our old
    // 0726 mesh: a surface poly at -5.08 at (1627,271.6); the current one: only the bed at -16.28), while a swimming bot
    // sits 0.8 yd under the surface (RaisePathToWaterSurface), 10+ yd above the nearest polygon. For a start or end
    // "far from poly and in water" PathGenerator::BuildPolyPath (:250-262; AC core PathGenerator.cpp:301-330 same)
    // builds no path at all but a straight NORMAL|NOT_USING_PATH shortcut, which MovePoint rides as one segment:
    // Haralas t=817314-823854, a 32-yd line from the river surface (1625.7,275.0,-5.1) up onto the bridge deck
    // (1594.3,281.1,0.1), mounted at run speed, 1-5 yd above the water; every Horde-side crossing at that ford the
    // same, and a bot pulled onto such a glider stood 2 yd over 11-yd-deep water (Aslandel). So on 726 the query
    // runs from the bed under the bot to the bed under the destination - where the polygons are - through the
    // two-point CalculatePath overload (AC PathGenerator.h:75), and RaisePathToWaterSurface lifts the result back to
    // the surface. A point not in liquid, and every other map, is handled exactly as before.
    // #437/#768 (AC parity): AC snaps with WorldObject::GetMapHeight, which probes from z + max(GetCollisionHeight(),
    // Z_OFFSET_FIND_HEIGHT 2.0) (AC Entities/Object/Object.cpp:3145-3151, shared/SharedDefines.h:27). This core's
    // GetMapWaterOrGroundLevel probes from z + Z_OFFSET_FIND_HEIGHT = 1.5 (Map.cpp:2534, SharedDefines.h:29). Blade's
    // Edge's Cata+ floor is 1.84 yd above AC's melee spot (ArenaTactics 6239.89,261.11,0.89: vmap floor 2.73 here,
    // 0.89 on AC), so the 1.5-yd probe started under the floor, the ADT is a hole there, the height came back invalid,
    // CalculatePath refused it, min_length stayed 0 and no candidate could beat it: MoveTo failed for every melee bot
    // and they never left the start room (wave 31b BE legs). Lift every probe to AC's height.
    float const probeLift = std::max(bot->GetCollisionHeight(), 2.0f) - Z_OFFSET_FIND_HEIGHT;
    bool destInLiquid = false;
    float tempZ = BedUnderLiquid(x, y, z, &destInLiquid);
    if (!destInLiquid)
        tempZ = bot->GetMapWaterOrGroundLevel(x, y, z + probeLift);
    float startX, startY, startZ;
    bot->GetPosition(startX, startY, startZ);
    startZ = BedUnderLiquid(startX, startY, startZ);
    PathGenerator gen(bot);
    gen.CalculatePath(startX, startY, startZ, x, y, tempZ);
    Movement::PointsArray result = gen.GetPath();
    float min_length = gen.getPathLength();
    // #747 (diverges from AC :1709-1748): AC tests the type with a bitmask, which also accepts NORMAL|NOPATH,
    // NORMAL|SHORT and the NOT_USING_PATH shortcuts (the core ORs those bits onto the type - PathGenerator.cpp:111,
    // 280, 617, 623 here, AC core MovementGenerators/PathGenerator.cpp:686-692). MoveTo then calls MovePoint, the
    // core re-runs PathGenerator, rejects on the NOPATH bit and glides the bot in a straight line through terrain
    // (Twin Peaks flag carriers crossing the river and the mid basin in the air, wave 19). A real NORMAL or
    // INCOMPLETE path passes exactly as before. A destination in liquid has one candidate z, the bed (#747).
    if (PathTypeIsWalkable(gen, bot) && (destInLiquid || std::abs(tempZ - z) < 0.5f))
    {
        modified_z = tempZ;
        return result;
    }
    // Start searching
    if (PathTypeIsWalkable(gen, bot))
    {
        modified_z = tempZ;
        found = true;
    }
    int count = 1;
    for (float delta = step; count < maxSearchCount / 2 + 1; count++, delta += step)
    {
        tempZ = bot->GetMapWaterOrGroundLevel(x, y, z + delta + probeLift);
        if (tempZ <= INVALID_HEIGHT)  // GetWaterOrGroundLevel reports no grid as VMAP_INVALID_HEIGHT_VALUE
        {
            continue;
        }
        PathGenerator gen(bot);
        gen.CalculatePath(startX, startY, startZ, x, y, tempZ);
        if (PathTypeIsWalkable(gen, bot) && gen.getPathLength() < min_length)
        {
            found = true;
            min_length = gen.getPathLength();
            result = gen.GetPath();
            modified_z = tempZ;
        }
    }
    for (float delta = -step; count < maxSearchCount; count++, delta -= step)
    {
        tempZ = bot->GetMapWaterOrGroundLevel(x, y, z + delta + probeLift);
        if (tempZ <= INVALID_HEIGHT)
        {
            continue;
        }
        PathGenerator gen(bot);
        gen.CalculatePath(startX, startY, startZ, x, y, tempZ);
        if (PathTypeIsWalkable(gen, bot) && gen.getPathLength() < min_length)
        {
            found = true;
            min_length = gen.getPathLength();
            result = gen.GetPath();
            modified_z = tempZ;
        }
    }
    if (!found && normal_only)
    {
        modified_z = INVALID_HEIGHT;
        return Movement::PointsArray{};
    }
    if (!found && !normal_only)
    {
        return result;
    }
    return result;
}

// #747: the core builds its own path in MotionMaster::MovePoint -> MoveSplineInit::MoveTo and, on the NOPATH bit,
// moves the unit in a straight 2-point spline to the destination (core Spline/MoveSplineInit.cpp:175-192, same in
// AC Spline/MoveSplineInit.cpp:221-237). This mirrors that decision so the module never orders a move the core will
// turn into a glide: NOPATH and SHORT (the smooth path overflowed MAX_POINT_PATH_LENGTH) are refused outright; a
// BuildShortcut flagged NOT_USING_PATH is kept when it is the "no navmesh here" case (maps or tiles without mmaps,
// UNIT_STATE_IGNORE_PATHFINDING) but refused when its straight segment leaves the water/ground surface by more than
// 3 yd - that is the "far from poly and in water -> CanSwim" shortcut of PathGenerator::BuildPolyPath (:250-262),
// which on Twin Peaks runs from the Horde flag-room floor straight down into the river bed. No AC equivalent
// (AC MovementActions.cpp has no path-type check before MovePoint); a plain NORMAL/INCOMPLETE path is unaffected.
//
// Wave 20 (#747, 0726 mesh): the same glide also comes out of a path the core accepts. When the start
// polygon is the only polygon Detour can reach (a navmesh island: both Twin Peaks mid graveyards, and the 3-triangle
// sliver tpJumpDown used to land on), FindSmoothPath takes its "same polygon" branch, targetPos is the RAW
// destination, and "Reached end of path" appends it as the final point (core PathGenerator.cpp:946-949 and
// 1001-1009, AC MovementGenerators/PathGenerator.cpp same code): a NORMAL|INCOMPLETE path whose last segment is a
// 300-yd straight line from the island edge to the Horde flag room (Andent t=916631, Bernond t=458427, ...).
// So every accepted path is also checked segment by segment: a smooth path advances SMOOTH_PATH_STEP_SIZE (4 yd)
// per point, so only a segment longer than 8 yd can be one of those appended lines or a shortcut, and only those
// are sampled against the water/ground surface. Map 726 has no off-mesh connections (all nine tiles: omCount 0),
// which are the one legitimate source of a long airborne segment.
namespace
{
    bool SegmentLeavesSurface(Map const* map, Unit const* unit, G3D::Vector3 const& a, G3D::Vector3 const& b)
    {
        float const len = (b - a).length();
        if (len <= 8.0f)
            return false;
        for (float d = 0.0f; d <= len; d += 4.0f)
        {
            G3D::Vector3 const p = a + (b - a) * (d / len);
            float ground = INVALID_HEIGHT;
            float const surface = map->GetWaterOrGroundLevel(unit->GetPhaseMask(), p.x, p.y, p.z, &ground, false,
                                                             unit->GetCollisionHeight());
            if (surface > INVALID_HEIGHT && p.z - surface > 3.0f)  // through the air
                return true;
            if (ground > INVALID_HEIGHT && ground - p.z > 3.0f)  // under a floor (water between bed and surface is fine)
                return true;
            // Map::GetHeight only reports the ADT surface for a point at or above it, so a point already under the
            // terrain comes back with no ground at all: ask the grid directly.
            float const terrain = map->GetGridHeight(p.x, p.y);
            if (terrain > INVALID_HEIGHT && terrain - p.z > 3.0f)  // through the terrain
                return true;
        }
        return false;
    }
}

bool MovementAction::PathTypeIsWalkable(PathGenerator const& gen, Unit const* unit)
{
    PathType const type = gen.GetPathType();
    if (!(type & (PATHFIND_NORMAL | PATHFIND_INCOMPLETE)))
        return false;
    if (type & (PATHFIND_NOPATH | PATHFIND_SHORT))
        return false;

    // no navmesh for this map (or pathfinding switched off for the unit): the shortcut is all there is, as on AC
    if ((type & PATHFIND_NOT_USING_PATH) &&
        (unit->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING) ||
         !MMAP::MMapFactory::createOrGetMMapManager()->GetNavMesh(unit->GetMapId())))
        return true;

    // #747 (map 726, see SearchForBestPath): the swim shortcut is never the only way across a bed-only mesh, and the
    // core rides it as one straight segment above the water (the 3-yd sampling below let the ford glide through,
    // wave 25), so on Twin Peaks it is refused outright - the bed-to-bed query gives a real path instead.
    if ((type & PATHFIND_NOT_USING_PATH) && unit->GetMapId() == 726)
        return false;

    // #437/#768: arenas keep AC's movement (AC MovementActions.cpp:239-262 hands every NORMAL|INCOMPLETE path to
    // MovePoint). Dalaran Sewers' start pipes are navmesh islands (0617_29_30.mmtile: a 4-poly and an 8-poly component
    // with no link to the floor, same as AC's mesh). At the pipe tip the single-poly path appends the floor point and
    // AC's bots glide down that line; the segment check below refused it, MoveTo failed, and ArenaTactics' teleport
    // fallback (within 4 yd of the tip centre) missed the tip corner the path parks the bot on (6.5 yd away), so both
    // teams stood on their pipe tips for the whole match (wave 31b DS 2v2, leftstart dist 29.5/32.9 = the corners).
    if (unit->GetMap()->IsBattleArena())
        return true;

    Movement::PointsArray const& pts = gen.GetPath();
    Map const* map = unit->GetMap();
    for (size_t i = 1; i < pts.size(); ++i)
        if (SegmentLeavesSurface(map, unit, pts[i - 1], pts[i]))
            return false;
    return true;
}

bool MovementAction::PathIsWalkable(float x, float y, float z)
{
    if (bot->IsFlying())
        return true;
    PathGenerator gen(bot);
    gen.CalculatePath(x, y, z, false);
    return PathTypeIsWalkable(gen, bot);
}

// #747 (map 726, see MoveTo): every path point that lies more than 1.5 yd under a liquid surface is lifted to
// 0.8 yd under that surface - where a swimming player's position sits - so the spline follows the surface instead
// of the river bed. Point 0 is the bot's own position and is left alone (MoveSplineInit::Launch overwrites it);
// points on dry land, in shallow water and already near the surface are unchanged. Water-walking bots keep the
// surface itself. Returns true when at least one point moved, which is what makes MoveTo launch the path itself.
bool MovementAction::RaisePathToWaterSurface(Movement::PointsArray& path) const
{
    Map const* map = bot->GetMap();
    if (!map)
        return false;
    float const below = bot->HasWaterWalkAura() ? 0.0f : 0.8f;
    bool raised = false;
    for (size_t i = 1; i < path.size(); ++i)
    {
        G3D::Vector3& p = path[i];
        LiquidData liquid;
        ZLiquidStatus const status = map->GetLiquidStatus(bot->GetPhaseMask(), p.x, p.y, p.z, MAP_ALL_LIQUIDS, &liquid,
                                                          bot->GetCollisionHeight());
        if (!(status & (LIQUID_MAP_IN_WATER | LIQUID_MAP_UNDER_WATER)))
            continue;
        float const target = liquid.level - below;
        if (target - p.z <= 1.5f)
            continue;
        p.z = target;
        raised = true;
    }
    return raised;
}

// #747 (map 726, see SearchForBestPath): the bed under (x,y,z) when the point lies in liquid or at its surface (up
// to 1 yd above it - the river waypoints of vPaths_TP sit on the surface), else z unchanged. LiquidData::depth_level
// is the grid ground under ADT liquid and the WMO floor under WMO liquid (Map::GetLiquidStatus), which is where the
// navmesh has its polygons (bed -16.28 at (1627,271.6) for a depth_level of -16.6).
float MovementAction::BedUnderLiquid(float x, float y, float z, bool* inLiquid) const
{
    if (inLiquid)
        *inLiquid = false;
    if (bot->GetMapId() != 726)
        return z;
    Map const* map = bot->GetMap();
    if (!map)
        return z;
    LiquidData liquid;
    ZLiquidStatus const status = map->GetLiquidStatus(bot->GetPhaseMask(), x, y, z, MAP_ALL_LIQUIDS, &liquid,
                                                      bot->GetCollisionHeight());
    if (status == LIQUID_MAP_NO_WATER || z > liquid.level + 1.0f)
        return z;
    if (inLiquid)
        *inLiquid = true;
    return liquid.depth_level;
}

bool MovementAction::MoveAway(Unit* target, float distance, bool backwards)
{
    if (!target)
    {
        return false;
    }
    float init_angle = target->GetAngle(bot);
    for (float delta = 0; delta <= M_PI / 2; delta += M_PI / 8)
    {
        float angle = init_angle + delta;
        float dx = bot->GetPositionX() + cos(angle) * distance;
        float dy = bot->GetPositionY() + sin(angle) * distance;
        float dz = bot->GetPositionZ();
        bool exact = true;
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false, backwards))
        {
            return true;
        }
        if (delta == 0)
        {
            continue;
        }
        exact = true;
        angle = init_angle - delta;
        dx = bot->GetPositionX() + cos(angle) * distance;
        dy = bot->GetPositionY() + sin(angle) * distance;
        dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), dx, dy, dz))
        {
            // disable prediction if position is invalid
            dx = bot->GetPositionX() + cos(angle) * distance;
            dy = bot->GetPositionY() + sin(angle) * distance;
            dz = bot->GetPositionZ();
            exact = false;
        }
        if (MoveTo(target->GetMapId(), dx, dy, dz, false, false, true, exact, MovementPriority::MOVEMENT_COMBAT, false, backwards))
        {
            return true;
        }
    }
    return false;
}

bool MovementAction::Move(float angle, float distance)
{
    float x = bot->GetPositionX() + cos(angle) * distance;
    float y = bot->GetPositionY() + sin(angle) * distance;

    //TODO do we need GetMapWaterOrGroundLevel() if we're using CheckCollisionAndGetValidCoords() ?
    float z = bot->GetMapWaterOrGroundLevel(x, y, bot->GetPositionZ());
    if (z == -100000.0f || z == -200000.0f)
        z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
        bot->GetPositionZ(), x, y, z, false))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z);
}

// just calculates average position of group and runs away from that position
bool MovementAction::MoveFromGroup(float distance)
{
    if (Group* group = bot->GetGroup())
    {
        uint32 mapId = bot->GetMapId();
        float closestDist = FLT_MAX;
        float x = 0.0f;
        float y = 0.0f;
        uint32 count = 0;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* player = gref->GetSource();
            if (!player || player == bot || !player->IsAlive() || player->GetMapId() != mapId)
                continue;
            float dist = bot->GetDistance2d(player);
            if (closestDist > dist)
                closestDist = dist;
            x += player->GetPositionX();
            y += player->GetPositionY();
            count++;
        }

        if (count && closestDist < distance)
        {
            x /= count;
            y /= count;
            // x and y are now average position of the group members
            float angle = bot->GetAngle(x, y) + M_PI;
            return Move(angle, distance - closestDist);
        }
    }
    return false;
}

bool MovementAction::Flee(Unit* target)
{
    // Restored from AC MovementActions.cpp:1336-1436 (#177, #162). Adapted to this fork's API:
    //  - sPlayerbotAIConfig is a pointer here (sPlayerbotAIConfig-> instead of AC's sPlayerbotAIConfig.).
    //  - sServerFacade->... instead of AC's ServerFacade::instance()....
    //  - PlayerBotSpec::IsTank/IsHeal/IsRanged (this fork's static helper) instead of AC's
    //    botAI->IsTank/IsHeal/IsRanged.
    //  - AiPlayerbot.FleeingEnabled has no field on this fork yet (PlayerbotAIConfig.h is locked for
    //    this batch); hard-coded to AC's own default (true) below instead of a config read.
    //  - target->GetThreatManager().GetCurrentVictim() replaces AC's target->GetThreatMgr().
    //    getCurrentVictim(): this core's ThreatManager::GetCurrentVictim() already returns the victim
    //    Unit* directly (see src/server/game/Combat/ThreatManager.h), not a 3.3.5-style
    //    HostileReference, so the old commented-out draft's "ref->getTarget() == bot" is replaced with
    //    a direct Unit* compare.
    static bool const fleeingEnabled = true; // AiPlayerbot.FleeingEnabled default (AC PlayerbotAIConfig.cpp:408)

    Player* master = GetMaster();
    if (!target)
        target = master;

    if (!target)
        return false;

    if (!fleeingEnabled)
        return false;

    if (!IsMovingAllowed())
    {
        botAI->TellError("I am stuck while fleeing");
        return false;
    }

    bool foundFlee = false;
    time_t lastFlee = AI_VALUE(LastMovement&, "last movement").lastFlee;
    time_t now = time(0);
    uint32 fleeDelay = urand(2, sPlayerbotAIConfig->returnDelay / 1000);

    if (lastFlee)
    {
        if ((now - lastFlee) <= fleeDelay)
        {
            return false;
        }
    }

    Unit* currentVictim = target->GetThreatManager().GetCurrentVictim();
    if (currentVictim && currentVictim == bot)  // bot is target - try to flee to tank or master
    {
        if (Group* group = bot->GetGroup())
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = sPlayerbotAIConfig->sightDistance;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if (PlayerBotSpec::IsTank(player))
                {
                    float distanceToTank = sServerFacade->GetDistance2d(bot, player);
                    if (distanceToTank < fleeDistance)
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToTank;
                    }
                }
            }

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master)
            {
                foundFlee = MoveNear(master);
            }
        }
    }
    else  // bot is not targeted, try to flee dps/healers
    {
        bool isHealer = PlayerBotSpec::IsHeal(bot);
        bool needHealer = !isHealer && AI_VALUE2(uint8, "health", "self target") < 50;
        bool isRanged = PlayerBotSpec::IsRanged(bot);

        Group* group = bot->GetGroup();
        if (group)
        {
            Unit* fleeTarget = nullptr;
            float fleeDistance = botAI->GetRange("shoot") * 1.5f;
            Unit* spareTarget = nullptr;
            float spareDistance = botAI->GetRange("shoot") * 2.0f;
            std::vector<Unit*> possibleTargets;

            for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
            {
                Player* player = gref->GetSource();
                if (!player || player == bot || !player->IsAlive())
                    continue;

                if ((isHealer && PlayerBotSpec::IsHeal(player)) || needHealer)
                {
                    float distanceToHealer = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                    if (distanceToHealer < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                        (needHealer || player->IsWithinLOSInMap(target)))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToHealer;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                else if (isRanged && PlayerBotSpec::IsRanged(player))
                {
                    float distanceToRanged = sServerFacade->GetDistance2d(bot, player);
                    float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                    if (distanceToRanged < fleeDistance &&
                        distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                        player->IsWithinLOSInMap(target))
                    {
                        fleeTarget = player;
                        fleeDistance = distanceToRanged;
                        possibleTargets.push_back(fleeTarget);
                    }
                }
                // remember any group member in case no one else found
                float distanceToFlee = sServerFacade->GetDistance2d(bot, player);
                float distanceToTarget = sServerFacade->GetDistance2d(player, target);
                if (distanceToFlee < spareDistance &&
                    distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance) &&
                    player->IsWithinLOSInMap(target))
                {
                    spareTarget = player;
                    spareDistance = distanceToFlee;
                    possibleTargets.push_back(fleeTarget);
                }
            }

            if (!possibleTargets.empty())
                fleeTarget = possibleTargets[urand(0, possibleTargets.size() - 1)];

            if (!fleeTarget)
                fleeTarget = spareTarget;

            if (fleeTarget)
                foundFlee = MoveNear(fleeTarget);

            if ((!fleeTarget || !foundFlee) && master && master->IsAlive() && master->IsWithinLOSInMap(target))
            {
                float distanceToTarget = sServerFacade->GetDistance2d(master, target);
                if (distanceToTarget > (botAI->GetRange("shoot") / 2 + sPlayerbotAIConfig->followDistance))
                    foundFlee = MoveNear(master);
            }
        }
    }

    if ((foundFlee || lastFlee) && bot->GetGroup())
    {
        if (!lastFlee)
        {
            AI_VALUE(LastMovement&, "last movement").lastFlee = now;
        }
        else
        {
            if ((now - lastFlee) > fleeDelay)
            {
                AI_VALUE(LastMovement&, "last movement").lastFlee = 0;
            }
            else
                return false;
        }
    }

    FleeManager manager(bot, botAI->GetRange("flee"), bot->GetAngle(target) + M_PI);
    if (!manager.isUseful())
        return false;

    float rx, ry, rz;
    if (!manager.CalculateDestination(&rx, &ry, &rz))
    {
        botAI->TellError("Nowhere to flee");
        return false;
    }

    bool result = MoveTo(target->GetMapId(), rx, ry, rz);

    if (result)
        AI_VALUE(LastMovement&, "last movement").lastFlee = time(nullptr);

    return result;
}

Position MovementAction::BestPositionForMeleeToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        bool isTanking = (currentTarget->CanFreeMove()) && (currentTarget->GetVictim() == bot);
        float angle = bot->GetAngle(currentTarget);
        float angleLeft = angle + (float)M_PI / 2;
        float angleRight = angle - (float)M_PI / 2;
        possibleAngles.push_back({ angleLeft, false });
        possibleAngles.push_back({ angleRight, false });
        possibleAngles.push_back({ angle, true });
        if (isTanking)
        {
            possibleAngles.push_back({ angle + (float)M_PI, false });
            possibleAngles.push_back({ bot->GetAngle(&pos) - (float)M_PI, false });
        }
    }
    else
    {
        float angleTo = bot->GetAngle(&pos) - (float)M_PI;
        possibleAngles.push_back({ angleTo, false });
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig->fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{ dx, dy, dz };
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() >
            sPlayerbotAIConfig->tooCloseDistance &&
            bot->IsWithinMeleeRange(currentTarget))
        {
            continue;
        }
        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

Position MovementAction::BestPositionForRangedToFlee(Position pos, float radius)
{
    Unit* currentTarget = AI_VALUE(Unit*, "current target");
    std::vector<CheckAngle> possibleAngles;
    float angleToTarget = 0.0f;
    float angleFleeFromCenter = bot->GetAngle(&pos) - (float)M_PI;
    if (currentTarget)
    {
        // Normally, move to left or right is the best position
        angleToTarget = bot->GetAngle(currentTarget);
        float angleLeft = angleToTarget + (float)M_PI / 2;
        float angleRight = angleToTarget - (float)M_PI / 2;
        possibleAngles.push_back({ angleLeft, false });
        possibleAngles.push_back({ angleRight, false });
        possibleAngles.push_back({ angleToTarget + (float)M_PI, true });
        possibleAngles.push_back({ angleToTarget, true });
        possibleAngles.push_back({ angleFleeFromCenter, true });
    }
    else
    {
        possibleAngles.push_back({ angleFleeFromCenter, false });
    }
    float farestDis = 0.0f;
    Position bestPos;
    for (CheckAngle& checkAngle : possibleAngles)
    {
        float angle = checkAngle.angle;
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        if (!CheckLastFlee(angle, infoList))
        {
            continue;
        }
        bool strict = checkAngle.strict;
        float fleeDis = std::min(radius + 1.0f, sPlayerbotAIConfig->fleeDistance);
        float dx = bot->GetPositionX() + cos(angle) * fleeDis;
        float dy = bot->GetPositionY() + sin(angle) * fleeDis;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
            bot->GetPositionZ(), dx, dy, dz))
        {
            continue;
        }
        Position fleePos{ dx, dy, dz };
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() > sPlayerbotAIConfig->spellDistance)
        {
            continue;
        }
        if (strict && currentTarget &&
            fleePos.GetExactDist(currentTarget) - currentTarget->GetCombatReach() <
            (sPlayerbotAIConfig->tooCloseDistance))
        {
            continue;
        }

        if (pos.GetExactDist(fleePos) > farestDis)
        {
            farestDis = pos.GetExactDist(fleePos);
            bestPos = fleePos;
        }
    }
    if (farestDis > 0.0f)
    {
        return bestPos;
    }
    return Position();
}

bool MovementAction::FleePosition(Position pos, float radius, uint32 minInterval)
{
    std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");

    if (!infoList.empty() && infoList.back().timestamp + minInterval > getMSTime())
        return false;

    Position bestPos;
    if (PlayerBotSpec::IsMelee(bot))
    {
        bestPos = BestPositionForMeleeToFlee(pos, radius);
    }
    else
    {
        bestPos = BestPositionForRangedToFlee(pos, radius);
    }
    if (bestPos != Position())
    {
        if (MoveTo(bot->GetMapId(), bestPos.GetPositionX(), bestPos.GetPositionY(), bestPos.GetPositionZ(), false,
            false, true, false, MovementPriority::MOVEMENT_COMBAT))
        {
            uint32 curTS = getMSTime();
            while (!infoList.empty())
            {
                if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
                {
                    infoList.pop_front();
                }
                else
                {
                    break;
                }
            }
            infoList.push_back({ pos, radius, bot->GetAngle(&bestPos), curTS });
            return true;
        }
    }
    return false;
}

bool MovementAction::CheckLastFlee(float curAngle, std::list<FleeInfo>& infoList)
{
    uint32 curTS = getMSTime();
    curAngle = Position::NormalizeOrientation(curAngle);
    while (!infoList.empty())
    {
        if (infoList.size() > 10 || infoList.front().timestamp + 5000 < curTS)
        {
            infoList.pop_front();
        }
        else
        {
            break;
        }
    }
    for (FleeInfo& info : infoList)
    {
        // more than 5 sec
        if (info.timestamp + 5000 < curTS)
        {
            continue;
        }
        float revAngle = Position::NormalizeOrientation(info.angle + M_PI);
        // angle too close
        if (fabs(revAngle - curAngle) < M_PI / 4)
        {
            return false;
        }
    }
    return true;
}

bool RunAwayAction::Execute(Event event) { return Flee(AI_VALUE(Unit*, "master target")); }

bool SetFacingTargetAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return true;

    sServerFacade->SetFacingTo(bot, target);
    botAI->SetNextCheckDelay(sPlayerbotAIConfig->reactDelay);
    return true;
}

bool SetFacingTargetAction::isUseful()
{
    return !AI_VALUE2(bool, "facing", "current target");
}

bool SetFacingTargetAction::isPossible()
{
    if (bot->isFrozen() || bot->IsPolymorphed() || (bot->isDead() && !bot->HasPlayerFlag(PLAYER_FLAGS_GHOST)) ||
        bot->IsBeingTeleported() || bot->HasConfuseAura() || bot->IsCharmed() ||
        bot->HasStunAura() || bot->IsInFlight() ||
        bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
        return false;

    return true;
}

bool MoveFromGroupAction::Execute(Event event)
{
    float distance = atoi(event.getParam().c_str());
    if (!distance)
        distance = 20.0f; // flee distance from config is too small for this
    return MoveFromGroup(distance);
}

bool FleeAction::Execute(Event event)
{
    return MoveAway(AI_VALUE(Unit*, "current target"), sPlayerbotAIConfig->fleeDistance, true);
}

bool FleeAction::isUseful()
{
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }

    // Restored from AC MovementActions.cpp:1813 (#177): only useful once the enemy has closed to
    // melee range - a ranged bot backing off before that is "kiting", not "fleeing".
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->IsInWorld() && !bot->IsWithinMeleeRange(target))
        return false;

    return true;
}

bool CombatFormationMoveAction::isUseful()
{
    if (getMSTime() - moveInterval < lastMoveTimer)
    {
        return false;
    }
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    return true;
}

bool CombatFormationMoveAction::Execute(Event event)
{
    // Restored from AC MovementActions.cpp:2274-2289 (#230/D1): the body had been gutted - the
    // FleePosition() call was commented out and lastMoveTimer never set, so the action could
    // only ever return false. The "stay" guards are AC's (:2277-2278).
    float dis = AI_VALUE(float, "disperse distance");
    if (dis <= 0.0f || (!bot->IsInCombat() && botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT)) ||
        (bot->IsInCombat() && botAI->HasStrategy("stay", BOT_STATE_COMBAT)))
        return false;
    Player* playerToLeave = NearestGroupMember(dis);
    if (playerToLeave && bot->GetExactDist(playerToLeave) < dis)
    {
        if (FleePosition(playerToLeave->GetPosition(), dis))
        {
            lastMoveTimer = getMSTime();
        }
    }
    return false;
}

Position CombatFormationMoveAction::AverageGroupPos(float dis, bool ranged, bool self)
{
    float averageX = 0, averageY = 0, averageZ = 0;
    int cnt = 0;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return Position();
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerBotSpec::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            sServerFacade->GetDistance2d(bot, member) > dis)
            continue;

        averageX += member->GetPositionX();
        averageY += member->GetPositionY();
        averageZ += member->GetPositionZ();
    }
    averageX /= cnt;
    averageY /= cnt;
    averageZ /= cnt;
    return Position(averageX, averageY, averageZ);
}

float CombatFormationMoveAction::AverageGroupAngle(Unit* from, bool ranged, bool self)
{
    Group* group = bot->GetGroup();
    if (!from || !group)
    {
        return 0.0f;
    }
    // float average = 0.0f;
    float sumX = 0.0f;
    float sumY = 0.0f;
    int cnt = 0;
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member)
            continue;

        if (!self && member == bot)
            continue;

        if (ranged && !PlayerBotSpec::IsRanged(member))
            continue;

        if (!member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->IsCharmed() ||
            sServerFacade->GetDistance2d(bot, member) > sPlayerbotAIConfig->sightDistance)
            continue;

        cnt++;
        sumX += member->GetPositionX() - from->GetPositionX();
        sumY += member->GetPositionY() - from->GetPositionY();
    }
    if (cnt == 0)
        return 0.0f;

    // unnecessary division
    // sumX /= cnt;
    // sumY /= cnt;

    return atan2(sumY, sumX);
}

Position CombatFormationMoveAction::GetNearestPosition(const std::vector<Position>& positions)
{
    Position result;
    for (const Position& pos : positions)
    {
        if (bot->GetExactDist(pos) < bot->GetExactDist(result))
            result = pos;
    }
    return result;
}

Player* CombatFormationMoveAction::NearestGroupMember(float dis)
{
    float nearestDis = 10000.0f;
    Player* result = nullptr;
    Group* group = bot->GetGroup();
    if (!group)
    {
        return result;
    }
    Group::MemberSlotList const& groupSlot = group->GetMemberSlots();
    for (Group::member_citerator itr = groupSlot.begin(); itr != groupSlot.end(); itr++)
    {
        Player* member = ObjectAccessor::FindPlayer(itr->guid);
        if (!member || !member->IsAlive() || member == bot || member->GetMapId() != bot->GetMapId() ||
            member->IsCharmed() || sServerFacade->GetDistance2d(bot, member) > dis)
            continue;
        if (nearestDis > bot->GetExactDist(member))
        {
            result = member;
            nearestDis = bot->GetExactDist(member);
        }
    }
    return result;
}

bool TankFaceAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (!bot->GetGroup())
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    if (!AI_VALUE2(bool, "has aggro", "current target"))
        return false;

    float averageAngle = AverageGroupAngle(target, true);

    if (averageAngle == 0.0f)
        return false;

    float deltaAngle = Position::NormalizeOrientation(averageAngle - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI; // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(averageAngle + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(averageAngle - M_PI * 3 / 5);

    // if dist < bot->GetMeleeRange(target) / 2, target will move backward
    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() - target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        /// @todo: movement control now is a mess, prepare to rewrite
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

// Ported from AC mod-playerbots MovementActions.cpp (#149); mirrors TankFaceAction::Execute above.
bool SetBehindTargetAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    if (target->GetVictim() == bot)
        return false;

    if (!bot->IsWithinMeleeRange(target) || target->isMoving())
        return false;

    float deltaAngle = Position::NormalizeOrientation(target->GetOrientation() - target->GetAngle(bot));
    if (deltaAngle > M_PI)
        deltaAngle -= 2.0f * M_PI; // -PI..PI

    float tolerable = M_PI_2;

    if (fabs(deltaAngle) > tolerable)
        return false;

    float goodAngle1 = Position::NormalizeOrientation(target->GetOrientation() + M_PI * 3 / 5);
    float goodAngle2 = Position::NormalizeOrientation(target->GetOrientation() - M_PI * 3 / 5);

    float dist = std::max(bot->GetExactDist(target), bot->GetMeleeRange(target) / 2) - bot->GetCombatReach() - target->GetCombatReach();
    std::vector<Position> availablePos;
    float x, y, z;
    target->GetNearPoint(bot, x, y, z, dist, goodAngle1);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    target->GetNearPoint(bot, x, y, z, dist, goodAngle2);
    if (bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
        x, y, z))
    {
        std::list<FleeInfo>& infoList = AI_VALUE(std::list<FleeInfo>&, "recently flee info");
        Position pos(x, y, z);
        float angle = bot->GetAngle(&pos);
        if (CheckLastFlee(angle, infoList))
        {
            availablePos.push_back(Position(x, y, z));
        }
    }
    if (availablePos.empty())
        return false;
    Position nearest = GetNearestPosition(availablePos);
    return MoveTo(bot->GetMapId(), nearest.GetPositionX(), nearest.GetPositionY(), nearest.GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

// Ported from AC mod-playerbots MovementActions.cpp:1832-1839 (#149). See the class comment in
// MovementActions.h for why only the "trigger NPC" case (AvoidUnitWithDamageAura) is ported.
bool AvoidAoeAction::isUseful()
{
    if (getMSTime() - moveInterval < uint32(lastMoveTimer))
        return false;

    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    return !triggers.empty();
}

bool AvoidAoeAction::Execute(Event /*event*/)
{
    // Case: Trigger npc (e.g. Lesser shadow fissure). AC's other two cases (dynamic-object aura,
    // damaging trap gameobject) are not ported - see the class comment in MovementActions.h.
    return AvoidUnitWithDamageAura();
}

// Ported from AC mod-playerbots MovementActions.cpp:1986-2054 (#149), unchanged logic - the
// underlying "possible triggers" value and FleePosition()/aoeAvoidSpellWhitelist/maxAoeAvoidRadius
// config keys already exist identically on this fork.
bool AvoidAoeAction::AvoidUnitWithDamageAura()
{
    GuidVector triggers = AI_VALUE(GuidVector, "possible triggers");
    if (triggers.empty())
    {
        return false;
    }
    for (ObjectGuid& guid : triggers)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsInWorld())
        {
            continue;
        }
        if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
        {
            return false;
        }
        Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
            unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
        for (Unit::AuraEffectList const& list : {aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell})
        {
            for (auto i = list.begin(); i != list.end(); ++i)
            {
                AuraEffect* aurEff = *i;
                SpellInfo const* spellInfo = aurEff->GetSpellInfo();
                if (!spellInfo)
                    continue;
                SpellInfo const* triggerSpellInfo =
                    sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
                if (!triggerSpellInfo)
                    continue;
                if (sPlayerbotAIConfig->aoeAvoidSpellWhitelist.find(triggerSpellInfo->Id) !=
                    sPlayerbotAIConfig->aoeAvoidSpellWhitelist.end())
                    return false;
                for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
                {
                    if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                    {
                        float radius = triggerSpellInfo->Effects[j].CalcRadius();
                        if (bot->GetDistance(unit) > radius)
                        {
                            break;
                        }
                        if (!radius || radius > sPlayerbotAIConfig->maxAoeAvoidRadius)
                            continue;
                        std::ostringstream name;
                        name << triggerSpellInfo->SpellName[LOCALE_enUS];
                        if (FleePosition(unit->GetPosition(), radius))
                        {
                            if (sPlayerbotAIConfig->tellWhenAvoidAoe && lastTellTimer < time(NULL) - 10)
                            {
                                lastTellTimer = time(NULL);
                                lastMoveTimer = getMSTime();
                                std::ostringstream out;
                                out << "I'm avoiding " << name.str() << " (" << triggerSpellInfo->Id << ")"
                                    << " Radius " << radius << " - [Unit Trigger]";
                                bot->Say(out.str(), LANG_UNIVERSAL);
                            }
                        }
                    }
                }
            }
        }
    }
    return false;
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:1824-1830 (class AC MovementActions.h:97-103).
// 5.4.8 divergence: AC's botAI->PetFollow() does not exist on this fork; it was ported as the free
// helper PlayerbotPetFollow(Player*) in PetsAction.h:14 (issue #194, S8) - identical call, no args.
// ---------------------------------------------------------------------------
bool FleeWithPetAction::Execute(Event /*event*/)
{
    if (bot->GetPet())
        PlayerbotPetFollow(bot);

    return Flee(AI_VALUE(Unit*, "current target"));
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2474-2487 (class AC MovementActions.h:153-173).
// 5.4.8 divergence: Unit::HasInArc() does not exist on this core. Same geometry expressed with the
// two primitives SetBehindTargetAction already uses on this fork (MovementActions.cpp:1607-1609):
// the normalised (orientation - GetAngle) delta.
// ---------------------------------------------------------------------------
bool RearFlankAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float delta = Position::NormalizeOrientation(target->GetOrientation() - target->GetAngle(bot));

    // Need to double the front angle check to account for mirrored angle.
    bool inFront = std::fabs(delta) <= minAngle;
    // Rear check does not need to double this angle as the logic is inverted
    // and we are subtracting from 2pi.
    bool inRear = std::fabs(delta) > (M_PI - maxAngle / 2.0f);

    return inFront || inRear;
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2489-2514.
// 5.4.8 divergence: Position::RelocatePolarOffset() does not exist on this core and the closest
// member, RelocateOffset(angle, distance), is RELATIVE to the stored orientation. The polar form is
// inlined verbatim (absolute angle from the anchor point) so the flank geometry is unchanged.
// ---------------------------------------------------------------------------
bool RearFlankAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float angle = frand(minAngle, maxAngle);
    float baseDistance = bot->GetMeleeRange(target) * 0.5f;
    float offset = baseDistance + distance;
    Position leftFlank = target->GetPosition();
    Position rightFlank = target->GetPosition();
    Position* destination = nullptr;
    leftFlank.Relocate(leftFlank.GetPositionX() + cos(angle) * offset, leftFlank.GetPositionY() + sin(angle) * offset,
                       leftFlank.GetPositionZ());
    rightFlank.Relocate(rightFlank.GetPositionX() + cos(-angle) * offset,
                        rightFlank.GetPositionY() + sin(-angle) * offset, rightFlank.GetPositionZ());

    if (bot->GetExactDist2d(leftFlank) < bot->GetExactDist2d(rightFlank))
    {
        destination = &leftFlank;
    }
    else
    {
        destination = &rightFlank;
    }

    return MoveTo(bot->GetMapId(), destination->GetPositionX(), destination->GetPositionY(),
                  destination->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2516-2598 (class AC MovementActions.h:175-183).
// The "disperse distance" AI value already exists on this fork (Value.h:378, registered in
// ValueContext.h:106). 5.4.8 divergence: botAI->IsMelee(bot) is a free static on this fork
// (PlayerbotSpec.h:15).
// ---------------------------------------------------------------------------
bool DisperseSetAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "disable")
    {
        RESET_AI_VALUE(float, "disperse distance");
        botAI->TellMasterNoFacing("Disable disperse");
        return true;
    }
    if (text == "enable" || text == "reset")
    {
        if (PlayerBotSpec::IsMelee(bot))
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_MELEE);
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", DEFAULT_DISPERSE_DISTANCE_RANGED);
        }
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        out << "Enable disperse distance " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "increase")
    {
        float dis = AI_VALUE(float, "disperse distance");
        std::ostringstream out;
        if (dis <= 0.0f)
        {
            out << "Enable disperse first";
            botAI->TellMasterNoFacing(out.str());
            return true;
        }
        dis += 1.0f;
        SET_AI_VALUE(float, "disperse distance", dis);
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text == "decrease")
    {
        float dis = AI_VALUE(float, "disperse distance");
        dis -= 1.0f;
        if (dis <= 0.0f)
        {
            dis += 1.0f;
        }
        SET_AI_VALUE(float, "disperse distance", dis);
        std::ostringstream out;
        out << "Increase disperse distance to " << std::setprecision(2) << dis;
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    if (text.starts_with("set"))
    {
        float dis = -1.0f;
        ;
        sscanf(text.c_str(), "set %f", &dis);
        std::ostringstream out;
        if (dis < 0 || dis > 100.0f)
        {
            out << "Invalid disperse distance " << std::setprecision(2) << dis;
        }
        else
        {
            SET_AI_VALUE(float, "disperse distance", dis);
            out << "Set disperse distance to " << std::setprecision(2) << dis;
        }
        botAI->TellMasterNoFacing(out.str());
        return true;
    }
    std::ostringstream out;
    out << "Usage: disperse [enable | disable | increase | decrease | set {distance}]";
    float dis = AI_VALUE(float, "disperse distance");
    if (dis > 0.0f)
    {
        out << "(Current disperse distance: " << std::setprecision(2) << dis << ")";
    }
    botAI->TellMasterNoFacing(out.str());
    return true;
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2611-2620 (class AC MovementActions.h:201-208).
// Both AI values exist on this fork ("inside target" = DistanceValue.h:20).
// ---------------------------------------------------------------------------
bool MoveOutOfEnemyContactAction::Execute(Event /*event*/)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    return MoveTo(target, sPlayerbotAIConfig->contactDistance);
}

bool MoveOutOfEnemyContactAction::isUseful() { return AI_VALUE2(bool, "inside target", "current target"); }

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2708-2724 (class AC MovementActions.h:228-235).
// 5.4.8 divergence: AC reads the "nearest friendly players" GuidVector through
// botAI->GetAiObjectContext()->GetValue<GuidVector>(...); this fork's AI_VALUE2 macro
// (Playerbots.h:27) is the same lookup by name, and NearestFriendlyPlayersValue is an
// ObjectGuidListCalculatedValue (NearestUnitsValue.h:15-24), so Get() is a GuidVector.
// ---------------------------------------------------------------------------
bool MoveOutOfCollisionAction::Execute(Event /*event*/)
{
    float angle = M_PI * 2000 / frand(1.f, 1000.f);
    float distance = sPlayerbotAIConfig->followDistance;
    return MoveTo(bot->GetMapId(), bot->GetPositionX() + cos(angle) * distance,
                  bot->GetPositionY() + sin(angle) * distance, bot->GetPositionZ());
}

bool MoveOutOfCollisionAction::isUseful()
{
    // do not avoid collision on vehicle
    if (botAI->IsInVehicle())
        return false;

    return AI_VALUE2(bool, "collision", "self target") &&
           AI_VALUE(GuidVector, "nearest friendly players").size() < 15;
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:1628-1635 and 2756 (class AC
// MovementActions.h:246-259).
// ---------------------------------------------------------------------------
bool MovementAction::MoveInside(uint32 mapId, float x, float y, float z, float distance, MovementPriority priority)
{
    if (bot->GetDistance2d(x, y) <= distance)
    {
        return false;
    }
    return MoveNear(mapId, x, y, z, distance, priority);
}

bool MoveInsideAction::Execute(Event /*event*/)
{
    return MoveInside(bot->GetMapId(), x, y, bot->GetPositionZ(), distance);
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2758-2768 (class AC MovementActions.h:261-290).
// ---------------------------------------------------------------------------
bool RotateAroundTheCenterPointAction::Execute(Event /*event*/)
{
    uint32 next_point = GetCurrWaypoint();
    if (MoveTo(bot->GetMapId(), waypoints[next_point].first, waypoints[next_point].second, bot->GetPositionZ(), false,
               false, false, false, MovementPriority::MOVEMENT_COMBAT))
    {
        call_counters += 1;
        return true;
    }
    return false;
}

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2778-2856 (class AC MovementActions.h:300-315).
// "nearest npcs" is a GuidVector on this fork (NearestNpcsValue.h:22, NearestUnitsValue.h:15).
// ---------------------------------------------------------------------------
bool MoveAwayFromCreatureAction::Execute(Event /*event*/)
{
    GuidVector targets = AI_VALUE(GuidVector, "nearest npcs");

    // Find all creatures with the specified Id
    std::vector<Unit*> creatures;
    for (auto const& guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (unit && (alive && unit->IsAlive()) && unit->GetEntry() == creatureId)
        {
            creatures.push_back(unit);
        }
    }

    if (creatures.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= 30.0f; distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromCreatures = true;
            float minCreatureDist = std::numeric_limits<float>::max();
            for (Unit* creature : creatures)
            {
                float dist = creature->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromCreatures = false;
                    break;
                }
                if (dist < minCreatureDist)
                {
                    minCreatureDist = dist;
                }
            }

            if (isSafeFromCreatures && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any creature. Higher is better.
                if (minCreatureDist > maxSafetyScore)
                {
                    maxSafetyScore = minCreatureDist;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT);
    }

    return false;
}

bool MoveAwayFromCreatureAction::isPossible() { return bot->CanFreeMove(); }

// ---------------------------------------------------------------------------
// Ported from AC mod-playerbots MovementActions.cpp:2858-2939 (class AC MovementActions.h:317-331).
// ---------------------------------------------------------------------------
bool MoveAwayFromPlayerWithDebuffAction::Execute(Event /*event*/)
{
    Group* const group = bot->GetGroup();

    if (!group)
        return false;

    std::vector<Player*> debuffedPlayers;

    for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
    {
        Player* player = gref->GetSource();
        if (player && player->IsAlive() && player->HasAura(spellId))
        {
            debuffedPlayers.push_back(player);
        }
    }

    if (debuffedPlayers.empty())
    {
        return false;
    }

    // Search for a safe position
    const int directions = 8;
    const float increment = 3.0f;
    float bestX = bot->GetPositionX();
    float bestY = bot->GetPositionY();
    float bestZ = bot->GetPositionZ();
    float maxSafetyScore = -1.0f;

    for (int i = 0; i < directions; ++i)
    {
        float angle = (i * 2 * M_PI) / directions;
        for (float distance = increment; distance <= (range + 5.0f); distance += increment)
        {
            float moveX = bot->GetPositionX() + distance * cos(angle);
            float moveY = bot->GetPositionY() + distance * sin(angle);
            float moveZ = bot->GetPositionZ();

            // Check creature distance constraints
            bool isSafeFromDebuffedPlayer = true;
            float minDebuffedPlayerDistance = std::numeric_limits<float>::max();
            for (Unit* debuffedPlayer : debuffedPlayers)
            {
                float dist = debuffedPlayer->GetExactDist2d(moveX, moveY);
                if (dist < range)
                {
                    isSafeFromDebuffedPlayer = false;
                    break;
                }
                if (dist < minDebuffedPlayerDistance)
                {
                    minDebuffedPlayerDistance = dist;
                }
            }

            if (isSafeFromDebuffedPlayer && bot->IsWithinLOS(moveX, moveY, moveZ))
            {
                // A simple safety score: the minimum distance to any debuffed player. Higher is better.
                if (minDebuffedPlayerDistance > maxSafetyScore)
                {
                    maxSafetyScore = minDebuffedPlayerDistance;
                    bestX = moveX;
                    bestY = moveY;
                    bestZ = moveZ;
                }
            }
        }
    }

    // Move to the best position found
    if (maxSafetyScore > 0.0f)
    {
        return MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true);
    }

    return false;
}

bool MoveAwayFromPlayerWithDebuffAction::isPossible() { return bot->CanFreeMove(); }
