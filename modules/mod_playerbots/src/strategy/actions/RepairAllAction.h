/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RepairAllAction.{h,cpp} for issue #188 (S6 of epic
 * #163). Registered as the "repair" chat command (ChatActionContext.h/ChatTriggerContext.h), matching
 * AC. See RepairAllAction.cpp for 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_REPAIRALLACTION_H
#define PLAYERBOTS_REPAIRALLACTION_H

#include "Action.h"

class PlayerbotAI;

class RepairAllAction : public Action
{
public:
    RepairAllAction(PlayerbotAI* botAI) : Action(botAI, "repair") {}

    bool Execute(Event event) override;
};

#endif
