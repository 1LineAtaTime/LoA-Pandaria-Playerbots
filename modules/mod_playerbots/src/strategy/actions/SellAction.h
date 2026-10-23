/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/SellAction.h for issue #258 (NPC interaction layer).
 * The "sell" chat command registration is deliberately NOT done here - a later slice owns every
 * *Context.h / AiFactory.cpp so that no file has two writers. See SellAction.cpp for the 5.4.8
 * adaptations.
 */

#ifndef PLAYERBOTS_SELLACTION_H
#define PLAYERBOTS_SELLACTION_H

#include "InventoryAction.h"

class AiObjectContext;
class FindItemVisitor;
class Item;
class PlayerbotAI;

class SellAction : public InventoryAction
{
public:
    SellAction(PlayerbotAI* botAI, std::string const name = "sell") : InventoryAction(botAI, name) {}

    bool Execute(Event event) override;
    void Sell(FindItemVisitor* visitor);
    void Sell(Item* item);
};

#endif