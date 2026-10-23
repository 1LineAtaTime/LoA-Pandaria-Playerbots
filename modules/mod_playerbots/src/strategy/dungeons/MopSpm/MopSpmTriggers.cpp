/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm triggers (epic #373, issue #653).

#include "MopSpmTriggers.h"
#include "MopSpmShared.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Playerbots.h"

bool MopSpmDangerTrigger::IsActive()
{
    return MopSpm::IsInDanger(bot);
}

bool MopSpmParryTrigger::IsActive()
{
    return bot->IsInCombat() && MopSpm::IsParryStanceActive(bot);
}

bool MopSpmHazeTrigger::IsActive()
{
    return bot->IsInCombat() && MopSpm::NeedsMeditate(bot);
}

bool MopSpmPriorityTrigger::IsActive()
{
    if (bot->GetMapId() != MopSpm::MAP_SPM || !bot->IsInCombat() ||
        !EncounterHelpers::IsMechanicTrackerBot(bot, MopSpm::MAP_SPM))
    {
        return false;
    }

    Unit* target = MopSpm::GetPriorityMarkTarget(bot);
    if (!target)
        return false;

    Group* group = bot->GetGroup();
    return group && group->GetTargetIcon(MopSpm::SKULL_ICON) != target->GetGUID();
}
