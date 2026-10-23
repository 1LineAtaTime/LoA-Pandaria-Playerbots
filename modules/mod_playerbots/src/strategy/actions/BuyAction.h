/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BuyAction.h for issue #258 (NPC interaction layer).
 * The "buy" chat command registration is deliberately NOT done here - a later slice owns every
 * *Context.h / AiFactory.cpp so that no file has two writers. See BuyAction.cpp for the 5.4.8
 * adaptations.
 *
 * AC declares two private TradeItem() helpers here (:30-31) that have no definition anywhere in AC
 * (dead declarations); they are not ported.
 */

#ifndef PLAYERBOTS_BUYACTION_H
#define PLAYERBOTS_BUYACTION_H

#include "InventoryAction.h"

class FindItemVisitor;
class ObjectGuid;
class Item;
class Player;
class PlayerbotAI;

// Both are *structs* in this core (Creature.h:143 VendorItem / :165 VendorItemData, and
// ItemPrototype.h ItemTemplate). The class-key must match or MSVC mangles the parameter type
// differently (struct -> "U", class -> "V") and you get an LNK2019 for a symbol that exists.
struct ItemTemplate;
struct VendorItemData;

class BuyAction : public InventoryAction
{
public:
    BuyAction(PlayerbotAI* botAI) : InventoryAction(botAI, "buy") {}

    bool Execute(Event event) override;

private:
    bool BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto);
};

#endif