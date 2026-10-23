/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm triggers (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMTRIGGERS_H
#define PLAYERBOTS_MOPSPMTRIGGERS_H

#include "Trigger.h"

class MopSpmDangerTrigger : public Trigger
{
public:
    MopSpmDangerTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop spm danger") {}
    bool IsActive() override;
};

// Master Snowdrift is in Parry Stance and the bot is fighting.
class MopSpmParryTrigger : public Trigger
{
public:
    MopSpmParryTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop spm parry stance") {}
    bool IsActive() override;
};

// Taran Zhu's Haze of Hate: the only cure is the Meditate extra-action button.
class MopSpmHazeTrigger : public Trigger
{
public:
    MopSpmHazeTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop spm haze") {}
    bool IsActive() override;
};

// A priority add is up and not yet carrying the group's skull (only the tracker bot reports it).
class MopSpmPriorityTrigger : public Trigger
{
public:
    MopSpmPriorityTrigger(PlayerbotAI* botAI) : Trigger(botAI, "mop spm priority") {}
    bool IsActive() override;
};

#endif
