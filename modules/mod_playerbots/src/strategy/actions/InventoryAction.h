/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/InventoryAction.h/.cpp (#149), cut down to what
 * "food"/"drink"/"healing potion"/"mana potion" need: bag iteration plus a qualifier lookup for
 * those four names. AC's InventoryAction also resolves item ids/names, item quality, item class,
 * outfits, mounts, pets, ammo, recipes, quest items and an "item usage" qualifier via
 * ChatHelper::parseItems/parseItemQuality/parseItemClass and several FindXVisitor classes this
 * fork does not use anywhere - none of that is ported, only Find() and the four qualifiers #149
 * actually needs. AC's IterateItems() also offers ITERATE_ITEMS_IN_EQUIP/ITERATE_ITEMS_IN_BANK
 * masks via PlayerbotFactory::IterateItems (a different AC class not accessible here); this fork
 * only needed the bag case for #149.
 *
 * #188 (S6 of epic #163): UnequipAction needs to find an item id among *equipped* items too (that
 * is the whole point of unequip), so IterateItems() now takes an IterateItemsMask (ItemVisitors.h)
 * defaulting to ITERATE_ITEMS_IN_BAGS (unchanged behaviour for every existing #149 caller).
 * ITERATE_ITEMS_IN_BANK is still not implemented - no #188 caller needs it.
 */

#ifndef _PLAYERBOT_INVENTORYACTION_H
#define _PLAYERBOT_INVENTORYACTION_H

#include "Action.h"
#include "ItemVisitors.h"

#include <map>
#include <string>
#include <vector>

class Item;
class PlayerbotAI;
struct ItemTemplate;

class InventoryAction : public Action
{
public:
    InventoryAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

protected:
    // Walks the bot's bags (default) and/or equipped slots (ITERATE_ITEMS_IN_EQUIP), calling
    // visitor->Visit(item) for each item found (stopping early if Visit() returns false). Mirrors
    // AC's InventoryAction::IterateItemsInBags()/IterateItemsInEquip(). ITERATE_ITEMS_IN_BANK is not
    // ported (see the class comment).
    void IterateItems(IterateItemsVisitor* visitor, uint32 mask = ITERATE_ITEMS_IN_BAGS);

    // Resolves a qualifier string to matching bag items, mirroring AC's
    // InventoryAction::parseItems() cut down to the qualifiers this fork's actions need:
    // "food"/"conjured food", "drink"/"water"/"conjured drink"/"conjured water",
    // "healing potion", "mana potion".
    // #883: the item-link branch of AC's parseItems (AC InventoryAction.cpp:222-239) lives here, searched in `mask`
    // (AC's parseItems(text, mask); default bags, like every other branch of Find()).
    std::vector<Item*> Find(std::string const qualifier, uint32 mask = ITERATE_ITEMS_IN_BAGS);

    // AC Ai/Base/Actions/InventoryAction.h:23-25: the full chat item resolver and the item listing helpers that
    // the "c"/"items"/"inv" command (TellItemCountAction) needs. parseItems is AC's InventoryAction::parseItems
    // (AC InventoryAction.cpp:211-398) without the outfit lookup and the "materials"/"recipe all" qualifiers, whose
    // visitors and outfit store are not part of this module (see InventoryAction.cpp).
    void TellItems(std::map<uint32, uint32> items, std::map<uint32, bool> soulbound);
    void TellItem(ItemTemplate const* proto, uint32 count, bool soulbound);
    std::vector<Item*> parseItems(std::string const text, uint32 mask = ITERATE_ALL_ITEMS);
};

#endif
