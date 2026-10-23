/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654). See MopMspShared.h for sources.

#include "MopMspActions.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include <cmath>

bool MopMspEscapeAction::Execute(Event /*event*/)
{
    MopMsp::Hazard const hazard = AI_VALUE(MopMsp::Hazard, "mop msp hazard");
    if (hazard.kind == MopMsp::HazardKind::None)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

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

    // #654 w47: a healer dodging an axe / a ring takes the full-length step (turn <= 0.6 rad, which still clears the
    // radial hazard) that ends nearest Xin's victim, so it stays in heal range (AC Dungeon/Mech/MechActions.cpp:216-218).
    bool const radial = hazard.kind == MopMsp::HazardKind::Axe || hazard.kind == MopMsp::HazardKind::FireRing;
    if (Unit* tank = radial ? MopMsp::HealerLeashTarget(bot) : nullptr)
    {
        bool found = false;
        float bestX = 0.0f, bestY = 0.0f, bestZ = 0.0f, bestDist = 0.0f;
        for (float const turn : {0.0f, 0.6f, -0.6f})
        {
            for (float const scale : {1.0f, 1.4f})
            {
                float dx = bx + std::cos(baseAngle + turn) * dist * scale;
                float dy = by + std::sin(baseAngle + turn) * dist * scale;
                float dz = bz;
                if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bx, by, bz, dx, dy, dz))
                    continue;

                float const tankDist = tank->GetExactDist2d(dx, dy);
                if (!found || tankDist < bestDist)
                {
                    found = true;
                    bestX = dx;
                    bestY = dy;
                    bestZ = dz;
                    bestDist = tankDist;
                }
            }
        }

        if (found && MoveTo(bot->GetMapId(), bestX, bestY, bestZ, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
            return true;
    }
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

            if (MoveTo(bot->GetMapId(), dx, dy, dz, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
                return true;
        }
    }

    return false;
}

bool MopMspInterruptAction::Execute(Event /*event*/)
{
    Unit* target = MopMsp::FindInterruptTarget(bot, _group);
    if (!target)
        return false;

    return MopMsp::CastInterrupt(botAI, target);
}

bool MopMspFocusAction::Execute(Event /*event*/)
{
    Unit* target = botAI->GetUnit(AI_VALUE(ObjectGuid, "mop msp focus target"));
    if (!target || AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}
