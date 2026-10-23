/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs triggers (epic #373, issue #651).

#include "MopTjsTriggers.h"
#include "MopTjsShared.h"
#include "Playerbots.h"

bool MopTjsDangerTrigger::IsActive()
{
    return MopTjs::IsInDanger(bot);
}

bool MopTjsLivingWaterTrigger::IsActive()
{
    Unit* water = MopTjs::FindLivingWater(bot);
    if (!water)
        return false;

    return AI_VALUE(Unit*, "current target") != water || !bot->IsWithinLOSInMap(water);
}

bool MopTjsLorewalkerTrigger::IsActive()
{
    if (!MopTjs::FindLorewalkerTarget(bot))
        return false;

    // A bot already on a live event enemy keeps it (the Intensity switch is the DPS target exclusion's job).
    return !MopTjs::IsLorewalkerEnemy(AI_VALUE(Unit*, "current target"));
}
