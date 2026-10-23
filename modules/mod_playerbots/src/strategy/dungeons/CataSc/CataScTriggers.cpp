/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#include "CataScTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Playerbots.h"

bool CataScHazardTrigger::IsActive()
{
    return CataSc::GetHazardCreature(bot, _hazard) != nullptr;
}

bool CataScDynHazardTrigger::IsActive()
{
    float cx = 0.0f;
    float cy = 0.0f;
    return CataSc::GetDynHazard(bot, _hazard, cx, cy);
}

bool CataScShatterTrigger::IsActive()
{
    return CataSc::ShatterDanger(bot);
}

bool CataScGroundSlamTrigger::IsActive()
{
    return CataSc::GroundSlamDanger(bot);
}

bool CataScSandBlastTrigger::IsActive()
{
    return CataSc::SandBlastDanger(bot);
}

bool CataScFocusTrigger::IsActive()
{
    if (!CataSc::IsFocusDps(bot))
        return false;

    return CataSc::GetFocusTarget(bot, _focus) != nullptr;
}
