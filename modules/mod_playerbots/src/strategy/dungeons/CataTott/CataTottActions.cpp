/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#include "CataTottActions.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <cmath>

// Same fan-out as fork strategy/dungeons/UB/UBActions.cpp UBRetreatFromFoulSporesAction: the straight line away from
// the danger first, then +-22.5 / 45 / 67.5 / 90 degrees; Map::CheckCollisionAndGetValidCoords rejects a blocked line.
bool CataTottMoveAction::EvadeFrom(float cx, float cy, float safeRadius, MovementPriority priority, Unit* awayRef)
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

bool CataTottLeaveHazardAction::Execute(Event /*event*/)
{
    Creature* hazard = CataTott::GetHazardCreature(bot, _hazard);
    if (!hazard)
        return false;

    return EvadeFrom(hazard->GetPositionX(), hazard->GetPositionY(), CataTott::HazardSafeRadius(_hazard), _priority,
                     hazard);
}

bool CataTottSpreadSurgeAction::Execute(Event /*event*/)
{
    Player* closest = CataTott::GetNearestGroupMember(bot, CataTott::SURGE_RADIUS + CataTott::HAZARD_MARGIN);
    if (!closest)
        return false;

    return EvadeFrom(closest->GetPositionX(), closest->GetPositionY(),
                     CataTott::SURGE_RADIUS + CataTott::HAZARD_MARGIN, MovementPriority::MOVEMENT_COMBAT, closest);
}

bool CataTottFocusAction::Execute(Event /*event*/)
{
    Unit* target = CataTott::GetFocusTarget(bot, _focus);
    if (!target)
        return false;

    if (EncounterHelpers::IsMechanicTrackerBot(bot, CataTott::MAP_ID))
        EncounterHelpers::MarkTargetWithSkull(bot, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool CataTottInterruptAction::Execute(Event /*event*/)
{
    Unit* target = CataTott::GetInterruptTarget(bot);
    if (!target)
        return false;

    return CataTott::CastInterrupt(botAI, target);
}
