/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655). See MopGssShared.h for sources.

#include "MopGssActions.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "LastMovementValue.h"
#include "Log.h"
#include "MotionMaster.h"
#include "ObjectDefines.h"
#include "Playerbots.h"
#include "Timer.h"
#include <algorithm>
#include <cmath>

bool MopGssEscapeAction::Execute(Event /*event*/)
{
    MopGss::Hazard const hazard = AI_VALUE(MopGss::Hazard, "mop gss hazard");
    if (hazard.kind == MopGss::HazardKind::None)
        return false;

    // w59 (#655): 197 of 262 Ri'mok escapes and 135 of 182 Raigonn escapes FAILED - a melee chase / reach move at
    // MOVEMENT_COMBAT was still running (IsWaitingForLastMove waits on an equal priority), so the bot stayed in the bomb /
    // pool / tornado. A hazard escape now overrides combat movement; a running escape is still waited for.
    // #655 w65: Clarinero's Viscous Fluid walk (a melee spot next to Ri'mok) was still running when Frenzied Assault began;
    // the Frenzy escape returned FAILED 8 times in a row on that wait and he stood in the cone 100 -> 24 -> 10 -> dead.
    // A walk for ANOTHER hazard (or any other forced walk: cannon, flak) is cut; only our own walk for this hazard is
    // waited for (same cut as MopSnt/MopSntActions.cpp:411-415).
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
    {
        LastMovement& last = AI_VALUE(LastMovement&, "last movement");
        bool const ours = hazard.kind == _movingKind &&
                          std::hypot(last.lastMoveToX - _movingX, last.lastMoveToY - _movingY) < 0.5f;
        if (ours)
            return false;

        last.clear();
    }

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const bz = bot->GetPositionZ();
    float const dist = std::hypot(hazard.x - bx, hazard.y - by);
    if (dist < 0.5f)
        return false;

    // The computed point first, then shorter steps and slightly turned ones: a wall on the straight line must not
    // leave the bot standing in the hazard. Collision check as in UB/UBActions.cpp:35.
    // #781: wider fan (+-1.2 rad) and a longer step - see the FAILED audit in the issue #781 report.
    float const baseAngle = std::atan2(hazard.y - by, hazard.x - bx);
    static float const turns[] = {0.0f, 0.6f, -0.6f, 1.2f, -1.2f};
    static float const scales[] = {1.0f, 0.7f, 1.4f, 0.45f};
    for (float const turn : turns)
    {
        for (float const scale : scales)
        {
            float const angle = baseAngle + turn;
            float dx = bx + std::cos(angle) * dist * scale;
            float dy = by + std::sin(angle) * dist * scale;
            float dz = bz;
            if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, dx, dy, dz))
                continue;

            if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true, MovementPriority::MOVEMENT_FORCED))
            {
                _movingKind = hazard.kind;
                _movingX = dx;
                _movingY = dy;
                return true;
            }
        }
    }

    return false;
}

bool MopGssInterruptAction::Execute(Event /*event*/)
{
    Unit* target = MopGss::FindInterruptTarget(bot, _group);
    if (!target)
        return false;

    return MopGss::CastInterrupt(botAI, target);
}

bool MopGssFocusAction::Execute(Event /*event*/)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "mop gss focus target"));
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool MopGssTankFluidAction::Execute(Event /*event*/)
{
    float x, y, z;
    if (!MopGss::ComputeTankDragSpot(bot, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MopGssCannonAction::Execute(Event /*event*/)
{
    Creature* cannon = MopGss::FindCannon(bot);
    if (!cannon)
        return false;

    // w60: the Artillery model is big (creature_model_info 41105: combat_reach 12, bounding 1.24) and the 0962 navmesh ends
    // ~15 yd short of its centre: the rogue walked to 919.1,2298.8 (cannon 59819 at 907.0,2308.5, 15.6 yd), every later
    // MoveTo towards the 2 yd point returned false (316 FAILED), and he stood there for the whole 329 s pull. The
    // client lets a player click a unit within its reach (the server's CMSG_SPELLCLICK has no range check,
    // SpellHandler.cpp:1429-1467), so the bot clicks from the cannon's combat reach + the interaction distance, and walks
    // only to its combat reach.
    float const clickRange = INTERACTION_DISTANCE + cannon->GetCombatReach();
    // w61 (#818 lane finding): MoveTo stops on the SURFACE distance (both combat reaches subtracted, Object.cpp:1643), so the
    // click test must use the same measure or the bot parks just outside an exact-distance threshold and never clicks.
    (void)clickRange;
    if (!bot->IsWithinDistInMap(cannon, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(cannon, 2.0f, MovementPriority::MOVEMENT_FORCED);  // w59: 115 FAILED at COMBAT

    botAI->RemoveShapeshift();
    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    // 5.4.8 Unit::HandleSpellClick(Unit* clicker, int8 seatId = -1), as TOC/TOCActions.cpp:257. The cannon's
    // OnSpellClick moves the rider onto the Weak Spot 1.5 s later (boss_raigonn.cpp:586-605).
    cannon->HandleSpellClick(bot);
    return bot->IsOnVehicle(cannon);
}

bool MopGssFlakAction::Execute(Event /*event*/)
{
    uint32 const now = getMSTime();
    if (_giveUpAt)
    {
        if (getMSTimeDiff(_giveUpAt, now) < MopGss::GSS_FLAK_GIVEUP_MS)
            return false;

        _giveUpAt = 0;
        _moveFails = 0;
        _bestDist = 1.0e9f;
    }

    MopGss::FlakState const flak = MopGss::ComputeFlak(bot);
    Creature* cannon = flak.cannon;
    if (!cannon)
    {
        _boarded = false;
        _bestDist = 1.0e9f;
        return false;
    }

    // #821 the Flak Cannon 60415 is a plain spellclick (npc_spellclick_spells 60415 -> 116554, VehicleId 0, cast_flags 0:
    // Unit::HandleSpellClick makes the CANNON cast 116554 on itself, Unit.cpp:18146-18190), so there is nothing to ride:
    // "board" is the first click in reach and "exit" is the end of the clicks. Reach model as MopGssCannonAction: the
    // server's CMSG_SPELLCLICK has no range check (SpellHandler.cpp:1429-1467), the bot walks to the cannon and clicks
    // from the client's interaction distance.
    float const clickRange = INTERACTION_DISTANCE + cannon->GetCombatReach();
    float const dist = bot->GetExactDist(cannon);
    (void)clickRange;
    if (!bot->IsWithinDistInMap(cannon, INTERACTION_DISTANCE - 1.0f))   // surface distance, the measure MoveTo uses (#818)
    {
        _boarded = false;
        auto const giveUp = [&](char const* why) {
            TC_LOG_DEBUG("playerbots", "[GSSFLAK] %s giveup why=%s cannon=%u dist=%.1f fails=%u at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), why, cannon->GetGUID().GetCounter(), dist, _moveFails,
                         bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            _giveUpAt = now ? now : 1;
            return false;
        };

        // no progress towards the cannon for 20 s (a navmesh that ends short of it): back to the normal strategy
        if (dist < _bestDist - 1.0f)
        {
            _bestDist = dist;
            _bestAt = now;
        }
        else if (getMSTimeDiff(_bestAt, now) > 20000)
            return giveUp("no-progress");

        if (IsWaitingForLastMove(MovementPriority::MOVEMENT_FORCED))
            return true;

        if (MoveTo(cannon, 2.0f, MovementPriority::MOVEMENT_FORCED))
        {
            _moveFails = 0;
            if (!_lastMoveLogAt || getMSTimeDiff(_lastMoveLogAt, now) >= 5000)
            {
                _lastMoveLogAt = now ? now : 1;
                TC_LOG_DEBUG("playerbots", "[GSSFLAK] %s walk cannon=%u dist=%.1f alive=%u at=%.1f,%.1f,%.1f",
                             bot->GetName().c_str(), cannon->GetGUID().GetCounter(), dist, flak.bombardiers,
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            }

            return true;
        }

        if (++_moveFails >= MopGss::GSS_FLAK_MAX_MOVE_FAILS)
            return giveUp("move-failed");

        return false;
    }

    _bestDist = 1.0e9f;
    _moveFails = 0;
    if (!_boarded)
    {
        _boarded = true;
        TC_LOG_DEBUG("playerbots", "[GSSFLAK] %s board cannon=%u dist=%.1f alive=%u", bot->GetName().c_str(),
                     cannon->GetGUID().GetCounter(), dist, flak.bombardiers);
    }

    if (_lastClickAt && getMSTimeDiff(_lastClickAt, now) < MopGss::GSS_FLAK_CLICK_MS)
        return true;  // hold the spot between two clicks

    botAI->RemoveShapeshift();
    if (bot->isMoving())
    {
        bot->GetMotionMaster()->Clear();
        bot->StopMoving();
    }

    _lastClickAt = now ? now : 1;
    ++_clicks;
    bool const ok = cannon->HandleSpellClick(bot);
    TC_LOG_DEBUG("playerbots", "[GSSFLAK] %s fire n=%u cannon=%u spell=%u ok=%u alive=%u dist=%.1f", bot->GetName().c_str(),
                 _clicks, cannon->GetGUID().GetCounter(), MopGss::GSS_SPELL_FIRE_FLAK_CANNON, ok ? 1 : 0,
                 flak.bombardiers, dist);
    return ok;
}
