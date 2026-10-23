/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm action context (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMACTIONCONTEXT_H
#define PLAYERBOTS_MOPSPMACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopSpmActions.h"

class MopSpmActionContext : public NamedObjectContext<Action>
{
public:
    MopSpmActionContext()
    {
        creators["mop spm avoid"] = &MopSpmActionContext::avoid;
        creators["mop spm parry stop"] = &MopSpmActionContext::parry_stop;
        creators["mop spm meditate"] = &MopSpmActionContext::meditate;
        creators["mop spm mark priority"] = &MopSpmActionContext::mark_priority;
    }

private:
    static Action* avoid(PlayerbotAI* botAI) { return new MopSpmAvoidAction(botAI); }
    static Action* parry_stop(PlayerbotAI* botAI) { return new MopSpmParryStopAction(botAI); }
    static Action* meditate(PlayerbotAI* botAI) { return new MopSpmMeditateAction(botAI); }
    static Action* mark_priority(PlayerbotAI* botAI) { return new MopSpmMarkPriorityAction(botAI); }
};

#endif
