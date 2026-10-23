/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#include "MopSmTriggers.h"
#include "MopSmShared.h"
#include "Creature.h"
#include "Playerbots.h"

using namespace MopScarletMonastery;

bool MopSmInSpiritGaleTrigger::IsActive()
{
    return bot->HasAura(SPELL_SPIRIT_GALE_AURA);
}

bool MopSmPriorityInterruptTrigger::IsActive()
{
    Unit* target = FindInterruptTarget(bot);
    return target && _gate.Allow(bot, target);
}

bool MopSmFocusAddTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    return FindFocusAdd(bot) != nullptr;
}

bool MopSmFirestormKickTrigger::IsActive()
{
    Creature* boss = nullptr;
    return FirestormKickNear(bot, boss);
}

bool MopSmWhitemaneShieldTrigger::IsActive()
{
    if (!bot->IsInCombat() || !HasShieldDispel(botAI))
        return false;

    return FindShieldedWhitemane(bot) != nullptr;
}

bool MopSmScorchedEarthTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    float clearDistance = 0.0f;
    return FindNearestHazard(bot, HAZARD_SCORCHED_EARTH, clearDistance) != nullptr;
}
