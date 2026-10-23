/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs actions (epic #373, issue #651).

#include "MopTjsActions.h"
#include "MopTjsShared.h"
#include "Playerbots.h"
#include <vector>

bool MopTjsAvoidAction::Execute(Event /*event*/)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    std::vector<MopTjs::Hazard> hazards;
    MopTjs::CollectHazards(bot, hazards);
    if (hazards.empty() ||
        !MopTjs::InAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), MopTjs::DANGER_GRACE))
    {
        return false;
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!MopTjs::FindSafePoint(bot, hazards, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
                  false);
}

bool MopTjsAttackLivingWaterAction::Execute(Event /*event*/)
{
    Unit* water = MopTjs::FindLivingWater(bot);
    if (!water)
        return false;

    if (!bot->IsWithinLOSInMap(water))
    {
        if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
            return false;

        float const dist = PlayerBotSpec::IsRanged(bot) ? MopTjs::LIVING_WATER_RANGED_DIST : 2.0f;
        return MoveTo(water, dist, MovementPriority::MOVEMENT_COMBAT);
    }

    if (AI_VALUE(Unit*, "current target") == water)
        return false;

    return Attack(water);
}

bool MopTjsAttackLorewalkerAction::Execute(Event /*event*/)
{
    Unit* enemy = MopTjs::FindLorewalkerTarget(bot);
    if (!enemy || AI_VALUE(Unit*, "current target") == enemy)
        return false;

    if (!bot->IsWithinLOSInMap(enemy))
    {
        if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
            return false;

        float const dist = PlayerBotSpec::IsRanged(bot) ? MopTjs::LIVING_WATER_RANGED_DIST : 2.0f;
        return MoveTo(enemy, dist, MovementPriority::MOVEMENT_COMBAT);
    }

    return Attack(enemy);
}
