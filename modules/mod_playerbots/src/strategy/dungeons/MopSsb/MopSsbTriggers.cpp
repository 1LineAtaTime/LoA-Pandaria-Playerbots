/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb triggers (epic #373, issue #652).

#include "MopSsbTriggers.h"
#include "MopSsbShared.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Playerbots.h"

bool MopSsbDangerTrigger::IsActive()
{
    return MopSsb::IsInDanger(bot);
}

bool MopSsbBlackoutTrigger::IsActive()
{
    return MopSsb::NeedsBlackoutStep(bot);
}

bool MopSsbFizzyTrigger::IsActive()
{
    return MopSsb::NeedsFizzyFlight(bot);
}

bool MopSsbWallTrigger::IsActive()
{
    return MopSsb::FindWallToJump(bot) != nullptr;
}

bool MopSsbBubbleTrigger::IsActive()
{
    Unit* bubble = MopSsb::FindBubbleShieldTarget(bot);
    return bubble && AI_VALUE(Unit*, "current target") != bubble;
}

bool MopSsbPriorityTrigger::IsActive()
{
    if (bot->GetMapId() != MopSsb::MAP_SSB || !bot->IsInCombat() ||
        !EncounterHelpers::IsMechanicTrackerBot(bot, MopSsb::MAP_SSB))
    {
        return false;
    }

    Unit* target = MopSsb::GetPriorityMarkTarget(bot);
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    return group && group->GetTargetIcon(MopSsb::SKULL_ICON) != target->GetGUID();
}
