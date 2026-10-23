/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#include "CataBcActions.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Timer.h"
#include <cmath>

// Same fan-out as fork strategy/dungeons/UB/UBActions.cpp UBRetreatFromFoulSporesAction: the straight line away from
// the danger first, then +-22.5 / 45 / 67.5 / 90 degrees; Map::CheckCollisionAndGetValidCoords rejects a blocked line.
bool CataBcMoveAction::EvadeFrom(float cx, float cy, float safeRadius, MovementPriority priority, Unit* awayRef)
{
    float const px = bot->GetPositionX();
    float const py = bot->GetPositionY();
    float const current = std::hypot(px - cx, py - cy);
    if (current >= safeRadius)
        return false;

    float awayAngle = 0.0f;
    if (current > 0.5f)
        awayAngle = std::atan2(py - cy, px - cx);
    else if (awayRef)
        awayAngle = std::atan2(py - awayRef->GetPositionY(), px - awayRef->GetPositionX());
    else
        awayAngle = bot->GetOrientation() + float(M_PI);

    float const moveDist = safeRadius - current + 1.5f;
    for (float delta : { 0.0f, float(M_PI / 8), float(-M_PI / 8), float(M_PI / 4), float(-M_PI / 4),
                         float(3 * M_PI / 8), float(-3 * M_PI / 8), float(M_PI / 2), float(-M_PI / 2) })
    {
        float const angle = awayAngle + delta;
        float dx = px + std::cos(angle) * moveDist;
        float dy = py + std::sin(angle) * moveDist;
        float dz = bot->GetPositionZ();
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), dx, dy, dz))
            continue;

        if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true, priority))
            return true;
    }

    return false;
}

bool CataBcFleeSkullcrackerAction::Execute(Event /*event*/)
{
    Creature* boss = CataBc::GetRomogg(bot);
    if (!boss)
        return false;

    return EvadeFrom(boss->GetPositionX(), boss->GetPositionY(), CataBc::SKULLCRACKER_SAFE_RADIUS,
                     MovementPriority::MOVEMENT_FORCED, boss);
}

bool CataBcBreakChainsAction::Execute(Event /*event*/)
{
    Creature* chains = CataBc::GetChains(bot);
    if (!chains)
        return false;

    if (AI_VALUE(Unit*, "current target") == chains)
        return false;

    return Attack(chains);
}

bool CataBcLeaveQuakeAction::Execute(Event /*event*/)
{
    // The crack stays where the bot stood when the Quake aura landed: remember that spot for the aura's lifetime.
    if (!_haveOrigin || GetMSTimeDiffToNow(_originTime) > CataBc::QUAKE_ORIGIN_RESET_MS)
    {
        _originX = bot->GetPositionX();
        _originY = bot->GetPositionY();
        _originTime = getMSTime();
        _haveOrigin = true;
    }

    return EvadeFrom(_originX, _originY, CataBc::QUAKE_SAFE_RADIUS, MovementPriority::MOVEMENT_FORCED,
                     CataBc::GetRomogg(bot));
}

bool CataBcLeaveLavaDroolAction::Execute(Event /*event*/)
{
    float cx = 0.0f;
    float cy = 0.0f;
    if (!CataBc::GetLavaDroolDanger(bot, cx, cy))
        return false;

    return EvadeFrom(cx, cy, CataBc::LAVA_DROOL_SAFE_RADIUS);
}

bool CataBcLeaveBeamAction::Execute(Event /*event*/)
{
    float px = 0.0f;
    float py = 0.0f;
    if (!CataBc::GetNearestBeamPoint(bot, px, py))
        return false;

    return EvadeFrom(px, py, CataBc::BEAM_CLEAR_RADIUS, MovementPriority::MOVEMENT_FORCED);
}

bool CataBcInterruptAction::Execute(Event /*event*/)
{
    Unit* target = CataBc::GetInterruptTarget(bot);
    if (!target)
        return false;

    return CataBc::CastInterrupt(botAI, target);
}

bool CataBcKarshPillarAction::Execute(Event /*event*/)
{
    Creature* karsh = CataBc::GetKarsh(bot);
    if (!karsh || karsh->GetVictim() != bot)
        return false;

    float const cx = CataBc::KARSH_PILLAR_X;
    float const cy = CataBc::KARSH_PILLAR_Y;
    float const cz = CataBc::KARSH_PILLAR_Z;
    uint32 const stacks = CataBc::KarshSuperheatStacks(karsh);
    float const karshDist = karsh->GetDistance(cx, cy, cz);  // 3D, like the script's bHeat test

    if (stacks == 0)
    {
        // Cooled (-99% damage taken): already in the pillar means the armor is about to heat, nothing to do.
        if (karshDist <= CataBc::KARSH_PILLAR_RADIUS + 0.5f)
            return false;

        // Post on the far side of the pillar from Karsh, far enough that he stops outside it to melee.
        float tx = 0.0f;
        float ty = 0.0f;
        CataBc::GetKarshPost(karsh, tx, ty);
        float tz = bot->GetPositionZ();
        if (bot->GetExactDist2d(tx, ty) < 3.0f)
            return false;

        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), tx, ty, tz))
            return false;

        return MoveTo(bot->GetMapId(), tx, ty, tz, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
    }

    if (stacks >= CataBc::KARSH_MAX_STACKS)
    {
        if (karshDist > CataBc::KARSH_PILLAR_RADIUS + 1.0f)
            return false;

        return EvadeFrom(cx, cy, CataBc::KARSH_FAR_SIDE + 1.0f, MovementPriority::MOVEMENT_COMBAT, karsh);
    }

    return false;
}
