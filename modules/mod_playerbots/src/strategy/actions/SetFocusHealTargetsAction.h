#ifndef _PLAYERBOT_SETFOCUSHEALTARGETSACTION_H
#define _PLAYERBOT_SETFOCUSHEALTARGETSACTION_H

#include "Action.h"

class PlayerbotAI;

// AC parity (Ai/Base/Actions/SetFocusHealTargetsAction.{h,cpp}): chat command restricting a healer
// to a hand-picked list of group members ("focus heal targets +name" / "-name" / "clear" / "?").
// Plain strings replace AC's PlayerbotTextMgr (this fork has no text-template system).
class SetFocusHealTargetsAction : public Action
{
public:
    SetFocusHealTargetsAction(PlayerbotAI* botAI) : Action(botAI, "focus heal targets") {}

    bool Execute(Event event) override;
};

#endif
