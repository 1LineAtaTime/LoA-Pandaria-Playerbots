/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - action context.

#ifndef PLAYERBOTS_MOPMSPACTIONCONTEXT_H
#define PLAYERBOTS_MOPMSPACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopMspActions.h"

class MopMspActionContext : public NamedObjectContext<Action>
{
public:
    MopMspActionContext()
    {
        creators["mop msp escape"] = &MopMspActionContext::mop_msp_escape;
        creators["mop msp ming interrupt"] = &MopMspActionContext::mop_msp_ming_interrupt;
        creators["mop msp gekkan interrupt"] = &MopMspActionContext::mop_msp_gekkan_interrupt;
        creators["mop msp focus target"] = &MopMspActionContext::mop_msp_focus_target;
    }

private:
    static Action* mop_msp_escape(PlayerbotAI* botAI) { return new MopMspEscapeAction(botAI); }

    static Action* mop_msp_ming_interrupt(PlayerbotAI* botAI)
    {
        return new MopMspInterruptAction(botAI, "mop msp ming interrupt", MopMsp::InterruptGroup::Ming);
    }

    static Action* mop_msp_gekkan_interrupt(PlayerbotAI* botAI)
    {
        return new MopMspInterruptAction(botAI, "mop msp gekkan interrupt", MopMsp::InterruptGroup::Gekkan);
    }

    static Action* mop_msp_focus_target(PlayerbotAI* botAI) { return new MopMspFocusAction(botAI); }
};

#endif
