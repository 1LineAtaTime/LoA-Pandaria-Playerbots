/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#include "CataScActions.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>

// Same fan-out as fork strategy/dungeons/UB/UBActions.cpp UBRetreatFromFoulSporesAction: the straight line away from
// the danger first, then +-22.5 / 45 / 67.5 / 90 degrees; Map::CheckCollisionAndGetValidCoords rejects a blocked line.
bool CataScMoveAction::EvadeFrom(float cx, float cy, float safeRadius, MovementPriority priority, Unit* awayRef)
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

bool CataScMoveAction::LeaveCone(Unit* boss, float arc, MovementPriority priority)
{
    float const dist = std::max(bot->GetExactDist2d(boss), 5.0f);

    // which side of the boss's facing the bot is on: (-pi, pi]
    float rel = Position::NormalizeOrientation(boss->GetAngle(bot) - boss->GetOrientation());
    if (rel > float(M_PI))
        rel -= 2.0f * float(M_PI);

    // just outside the cone on the nearer side, at the same distance from the boss
    float const side = rel >= 0.0f ? 1.0f : -1.0f;
    float const angle = boss->GetOrientation() + side * (arc * 0.5f + 0.4f);
    float dx = boss->GetPositionX() + std::cos(angle) * dist;
    float dy = boss->GetPositionY() + std::sin(angle) * dist;
    float dz = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), dx, dy, dz))
        return false;

    return MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true, priority);
}

bool CataScLeaveHazardAction::Execute(Event /*event*/)
{
    Creature* hazard = CataSc::GetHazardCreature(bot, _hazard);
    if (!hazard)
        return false;

    return EvadeFrom(hazard->GetPositionX(), hazard->GetPositionY(), CataSc::HazardSafeRadius(_hazard), _priority,
                     hazard);
}

bool CataScLeaveDynHazardAction::Execute(Event /*event*/)
{
    float cx = 0.0f;
    float cy = 0.0f;
    if (!CataSc::GetDynHazard(bot, _hazard, cx, cy))
        return false;

    return EvadeFrom(cx, cy, CataSc::DynHazardSafeRadius(_hazard), _priority);
}

bool CataScFleeShatterAction::Execute(Event /*event*/)
{
    Creature* boss = CataSc::GetOzruk(bot);
    if (!boss)
        return false;

    return EvadeFrom(boss->GetPositionX(), boss->GetPositionY(), CataSc::SHATTER_SAFE_RADIUS,
                     MovementPriority::MOVEMENT_FORCED, boss);
}

bool CataScLeaveSlamConeAction::Execute(Event /*event*/)
{
    Creature* boss = CataSc::GetOzruk(bot);
    if (!boss)
        return false;

    return LeaveCone(boss, CataSc::GROUND_SLAM_ARC, MovementPriority::MOVEMENT_FORCED);
}

bool CataScLeaveSandBlastAction::Execute(Event /*event*/)
{
    Creature* boss = CataSc::GetSlabhide(bot);
    if (!boss)
        return false;

    return LeaveCone(boss, CataSc::SAND_BLAST_ARC);
}

bool CataScFocusAction::Execute(Event /*event*/)
{
    Unit* target = CataSc::GetFocusTarget(bot, _focus);
    if (!target)
        return false;

    if (EncounterHelpers::IsMechanicTrackerBot(bot, CataSc::MAP_ID))
        EncounterHelpers::MarkTargetWithSkull(bot, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}
