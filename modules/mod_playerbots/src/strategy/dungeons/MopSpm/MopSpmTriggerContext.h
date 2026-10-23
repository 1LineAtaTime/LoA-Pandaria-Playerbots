/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm trigger context (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSPMTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopSpmTriggers.h"
#include "NamedObjectContext.h"

class MopSpmTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopSpmTriggerContext()
    {
        creators["mop spm danger"] = &MopSpmTriggerContext::danger;
        creators["mop spm parry stance"] = &MopSpmTriggerContext::parry_stance;
        creators["mop spm haze"] = &MopSpmTriggerContext::haze;
        creators["mop spm priority"] = &MopSpmTriggerContext::priority;
    }

private:
    static Trigger* danger(PlayerbotAI* botAI) { return new MopSpmDangerTrigger(botAI); }
    static Trigger* parry_stance(PlayerbotAI* botAI) { return new MopSpmParryTrigger(botAI); }
    static Trigger* haze(PlayerbotAI* botAI) { return new MopSpmHazeTrigger(botAI); }
    static Trigger* priority(PlayerbotAI* botAI) { return new MopSpmPriorityTrigger(botAI); }
};

#endif
