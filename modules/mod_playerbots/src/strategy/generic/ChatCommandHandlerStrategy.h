/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Strategy/ChatCommandHandlerStrategy (#173), trimmed to the
 * chat commands this batch actually ports (see ChatActionContext.h / ChatTriggerContext.h). AiFactory
 * already adds "+chat" to both the default combat and non-combat strategy lists (spec section 1
 * "Strategies AiFactory adds but MoP never registered" - AiFactory.cpp is untouched by this change);
 * registering "chat" here (StrategyContext.h) is what turns that from a silent no-op into a live
 * strategy. Later chat-command batches only add entries here plus new Chat*Context creators - no
 * further PlayerbotAI.h changes.
 */

#ifndef _PLAYERBOT_CHATCOMMANDHANDLERSTRATEGY_H
#define _PLAYERBOT_CHATCOMMANDHANDLERSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

class ChatCommandHandlerStrategy : public PassTroughStrategy
{
public:
    ChatCommandHandlerStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "chat"; }
};

#endif
