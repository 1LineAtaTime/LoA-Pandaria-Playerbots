/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeAction.{h,cpp} for issue #193 (S7 of epic #163).
 * Registered as the "t"/"nt" chat commands (ChatActionContext.h/ChatTriggerContext.h), matching AC
 * (Ai/Base/Strategy/ChatCommandHandlerStrategy.cpp:44-45). See TradeAction.cpp for the 5.4.8
 * adaptations.
 */

#ifndef PLAYERBOTS_TRADEACTION_H
#define PLAYERBOTS_TRADEACTION_H

#include "InventoryAction.h"

class Item;
class PlayerbotAI;

class TradeAction : public InventoryAction
{
public:
    TradeAction(PlayerbotAI* botAI) : InventoryAction(botAI, "trade") {}

    bool Execute(Event event) override;

private:
    bool TradeItem(Item const* item, int8 slot);
};

#endif
