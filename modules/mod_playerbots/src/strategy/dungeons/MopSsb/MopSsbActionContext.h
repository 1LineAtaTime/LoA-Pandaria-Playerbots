/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb action context (epic #373, issue #652).

#ifndef PLAYERBOTS_MOPSSBACTIONCONTEXT_H
#define PLAYERBOTS_MOPSSBACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopSsbActions.h"

class MopSsbActionContext : public NamedObjectContext<Action>
{
public:
    MopSsbActionContext()
    {
        creators["mop ssb avoid"] = &MopSsbActionContext::avoid;
        creators["mop ssb blackout step"] = &MopSsbActionContext::blackout_step;
        creators["mop ssb mark priority"] = &MopSsbActionContext::mark_priority;
        creators["mop ssb fizzy flight"] = &MopSsbActionContext::fizzy_flight;
        creators["mop ssb wall jump"] = &MopSsbActionContext::wall_jump;
        creators["mop ssb pop bubble"] = &MopSsbActionContext::pop_bubble;
    }

private:
    static Action* avoid(PlayerbotAI* botAI) { return new MopSsbAvoidAction(botAI); }
    static Action* blackout_step(PlayerbotAI* botAI) { return new MopSsbBlackoutStepAction(botAI); }
    static Action* mark_priority(PlayerbotAI* botAI) { return new MopSsbMarkPriorityAction(botAI); }
    static Action* fizzy_flight(PlayerbotAI* botAI) { return new MopSsbFizzyFlightAction(botAI); }
    static Action* wall_jump(PlayerbotAI* botAI) { return new MopSsbWallJumpAction(botAI); }
    static Action* pop_bubble(PlayerbotAI* botAI) { return new MopSsbBubbleAction(botAI); }
};

#endif
