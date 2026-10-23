/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/ItemCountValue.h/.cpp (#149). Registers the
 * "inventory items" (a std::vector<Item*>) and "item count" (a uint32) AI values used by
 * UseItemAction/EatAction/DrinkAction/UseHealingPotion/UseManaPotion to find and count items by
 * qualifier ("food", "drink", "healing potion", "mana potion" - see InventoryAction::Find()).
 */

#ifndef _PLAYERBOT_ITEMCOUNTVALUE_H
#define _PLAYERBOT_ITEMCOUNTVALUE_H

#include "InventoryAction.h"
#include "NamedObjectContext.h"
#include "Value.h"

class Item;
class PlayerbotAI;

class InventoryItemValueBase : public InventoryAction
{
public:
    InventoryItemValueBase(PlayerbotAI* botAI) : InventoryAction(botAI, "empty") {}

    bool Execute(Event /*event*/) override { return false; }
};

class ItemCountValue : public Uint32CalculatedValue, public Qualified, public InventoryItemValueBase
{
public:
    ItemCountValue(PlayerbotAI* botAI, std::string const name = "item count")
        : Uint32CalculatedValue(botAI, name), InventoryItemValueBase(botAI)
    {
    }

    uint32 Calculate() override;
};

class InventoryItemValue : public CalculatedValue<std::vector<Item*>>, public Qualified, public InventoryItemValueBase
{
public:
    InventoryItemValue(PlayerbotAI* botAI, std::string const name = "inventory items")
        : CalculatedValue<std::vector<Item*>>(botAI, name), InventoryItemValueBase(botAI)
    {
    }

    std::vector<Item*> Calculate() override;
};

#endif
