/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb trigger context (epic #373, issue #652).

#ifndef PLAYERBOTS_MOPSSBTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSSBTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopSsbTriggers.h"
#include "NamedObjectContext.h"

class MopSsbTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopSsbTriggerContext()
    {
        creators["mop ssb danger"] = &MopSsbTriggerContext::danger;
        creators["mop ssb blackout"] = &MopSsbTriggerContext::blackout;
        creators["mop ssb priority"] = &MopSsbTriggerContext::priority;
        creators["mop ssb fizzy"] = &MopSsbTriggerContext::fizzy;
        creators["mop ssb wall"] = &MopSsbTriggerContext::wall;
        creators["mop ssb bubble"] = &MopSsbTriggerContext::bubble;
    }

private:
    static Trigger* danger(PlayerbotAI* botAI) { return new MopSsbDangerTrigger(botAI); }
    static Trigger* blackout(PlayerbotAI* botAI) { return new MopSsbBlackoutTrigger(botAI); }
    static Trigger* priority(PlayerbotAI* botAI) { return new MopSsbPriorityTrigger(botAI); }
    static Trigger* fizzy(PlayerbotAI* botAI) { return new MopSsbFizzyTrigger(botAI); }
    static Trigger* wall(PlayerbotAI* botAI) { return new MopSsbWallTrigger(botAI); }
    static Trigger* bubble(PlayerbotAI* botAI) { return new MopSsbBubbleTrigger(botAI); }
};

#endif
