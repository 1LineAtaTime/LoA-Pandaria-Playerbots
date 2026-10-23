/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/LootStrategyAction.{h,cpp} for issue #188 (S6 of epic
 * #163). Registered as the "ll" chat command (ChatActionContext.h/ChatTriggerContext.h), matching AC
 * (`ChatActionContext.h:117`). See LootStrategyAction.cpp for 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_LOOTSTRATEGYACTION_H
#define PLAYERBOTS_LOOTSTRATEGYACTION_H

#include "Action.h"

class PlayerbotAI;

class LootStrategyAction : public Action
{
public:
    LootStrategyAction(PlayerbotAI* botAI) : Action(botAI, "ll") {}

    bool Execute(Event event) override;
};

#endif
