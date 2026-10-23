/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InventoryAction.h"

#include "Playerbots.h"

#include <algorithm>
#include <set>

namespace
{
// AC InventoryAction.cpp:119-138 (compare_items / compare_items_by_level): class desc, subclass asc, quality asc, ilvl desc.
bool compare_items(ItemTemplate const* proto1, ItemTemplate const* proto2)
{
    if (proto1->Class != proto2->Class)
        return proto1->Class > proto2->Class;

    if (proto1->SubClass != proto2->SubClass)
        return proto1->SubClass < proto2->SubClass;

    if (proto1->Quality != proto2->Quality)
        return proto1->Quality < proto2->Quality;

    if (proto1->ItemLevel != proto2->ItemLevel)
        return proto1->ItemLevel > proto2->ItemLevel;

    return false;
}

bool compare_items_by_level(Item const* item1, Item const* item2)
{
    return compare_items(item1->GetTemplate(), item2->GetTemplate());
}
}

// Ported from AC mod-playerbots InventoryAction.cpp:51-69 (IterateItemsInBags), unchanged logic.
// This fork has no PlayerbotFactory::IterateItems to route through, so the loop is written
// directly here instead (per #149's instructions, rather than inventing new shared plumbing).
// #188: added the ITERATE_ITEMS_IN_EQUIP branch (UnequipAction needs to find equipped items by id).
void InventoryAction::IterateItems(IterateItemsVisitor* visitor, uint32 mask)
{
    if (mask & ITERATE_ITEMS_IN_EQUIP)
        for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                if (!visitor->Visit(pItem))
                    return;

    // #258: AC InventoryAction::IterateItemsInBank (AC InventoryAction.cpp:84-116), reached through
    // AC's `mask == ITERATE_ITEMS_IN_BANK` test at AC InventoryAction.cpp:47-48. The bare bank mask
    // is the only one AC ever passes (parseItems(text, ITERATE_ITEMS_IN_BANK)), so the equality test
    // is kept 1:1 rather than widened to a bit test. It sits above the bag guard below because that
    // guard returns early - with mask == ITERATE_ITEMS_IN_BANK no bag/equip bit is set, so AC would
    // visit nothing but the bank, which the explicit return reproduces exactly.
    if (mask == ITERATE_ITEMS_IN_BANK)
    {
        for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                if (!visitor->Visit(pItem))
                    return;

        for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
            if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                    if (Item* pItem = pBag->GetItemByPos(j))
                        if (!visitor->Visit(pItem))
                            return;

        return;
    }

    if (!(mask & ITERATE_ITEMS_IN_BAGS))
        return;

    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!visitor->Visit(pItem))
                return;

    // 5.4.8: no keyring (removed in Cataclysm), so AC's KEYRING_SLOT loop has nothing to walk.

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (!visitor->Visit(pItem))
                        return;

    // #258: ITERATE_ITEMS_IN_BANK is handled above; BankAction.cpp still walks the bank through its
    // file-local ForEachItemInBank(), which this branch does not affect.
}

// #188 (S6 of epic #163): ItemUsageValue (and EquipAction/UnequipAction/LootRollAction) need "how
// many of item X do I have" / "find item X" for an arbitrary item id, not just the four consumable
// categories below. AC resolves this via chat->FormatItem(proto) (a formatted item-link qualifier)
// matched by its own visitor; this fork's ChatHelper has no such formatter. FindItemByIdVisitor
// (ItemVisitors.h) does a plain item-id match instead and is shared by all of those callers.

// Ported from AC mod-playerbots InventoryAction.cpp:211-398 (parseItems), cut down to the four
// qualifiers #149's actions need. AC also resolves item ids/names/quality/class, outfits, mounts,
// pets, ammo, recipes and quest items via ChatHelper and several FindXVisitor classes unused
// elsewhere on this fork - not ported, see the class comment in InventoryAction.h.
std::vector<Item*> InventoryAction::Find(std::string const qualifier, uint32 mask)
{
    std::vector<Item*> found;

    // #883 (consolidates the copy #257/Q2 put in SellAction.cpp): AC InventoryAction::parseItems resolves item
    // links FIRST (AC InventoryAction.cpp:222-239): chat->parseItems(text) -> FindItemByIdVisitor per id in `mask`,
    // de-duplicated through a set and sorted by compare_items_by_level. Any text carrying a "Hitem:<id>" link
    // returns here and never reaches the qualifier branches, exactly as in AC.
    if (ItemIds const ids = ChatHelper::parseItems(qualifier); !ids.empty())
    {
        std::set<Item*> linked;
        for (uint32 const id : ids)
        {
            FindItemByIdVisitor visitor(id);
            IterateItems(&visitor, mask);
            linked.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        }

        found.assign(linked.begin(), linked.end());
        std::sort(found.begin(), found.end(), compare_items_by_level);
        return found;
    }

    if (qualifier == "food" || qualifier == "conjured food")
    {
        FindFoodVisitor visitor(bot, 11, qualifier == "conjured food");
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (qualifier == "drink" || qualifier == "water" || qualifier == "conjured drink" ||
             qualifier == "conjured water")
    {
        FindFoodVisitor visitor(bot, 59, qualifier == "conjured drink" || qualifier == "conjured water");
        IterateItems(&visitor);
        found = visitor.GetResult();

        // AC falls back to food (spell category 11) if no drink is found - some low-level bots
        // only carry food.
        if (found.empty())
        {
            FindFoodVisitor foodVisitor(bot, 11);
            IterateItems(&foodVisitor);
            found = foodVisitor.GetResult();
        }
    }
    else if (qualifier == "quest")
    {
        // #257 (AC InventoryAction.cpp:316-321): the quest-class items in the bags, for "use random quest item".
        FindQuestItemVisitor visitor(bot);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (qualifier == "mount")
    {
        // #530 (AC InventoryAction.cpp:277-282): a mount item (Reins/Whistle...) in the bags; FindMountVisitor exists
        // in ItemVisitors.h:289 (AC Mgr/Item/ItemVisitors.h:374).
        FindMountVisitor visitor(bot);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (qualifier == "mana potion")
    {
        FindPotionVisitor visitor(bot, SPELL_EFFECT_ENERGIZE);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (qualifier == "healing potion")
    {
        FindPotionVisitor visitor(bot, SPELL_EFFECT_HEAL);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (uint32 const itemId = uint32(atoi(qualifier.c_str())))
    {
        // #188: a plain numeric qualifier is an item id (see the FindItemByIdVisitor comment above).
        FindItemByIdVisitor visitor(itemId);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }
    else if (qualifier.rfind("usage ", 0) == 0)
    {
        // #258: "usage <n>" branch, ported verbatim from AC InventoryAction.cpp:330-343. AC's
        // comment is kept because the two details in it are load-bearing: "usage <n>" is only
        // meaningful at the *start* of the qualifier (which is how AC's isReservedQualifier() reads
        // it - matching it anywhere handed stoi() the text at a fixed offset instead of the number,
        // so "bandage usage 3" landed in stoi("ge usage 3")), and the digits are capped at nine so
        // the value always fits an int.
        //
        // This is the qualifier AC's CanSellValue (AC Ai/Base/Value/MaintenanceValues.cpp:41-45)
        // depends on: "item count" for "usage 12" (ITEM_USAGE_VENDOR) and "usage 10" (ITEM_USAGE_AH).
        // The fork's truncated port dropped the branch, so both strings fell through to the bare-name
        // FindNamedItemVisitor below, matched nothing, and made "can sell" permanently false - bots
        // never sold their trade goods, which is the whole point of the maintenance strategy.
        std::string const usage = qualifier.substr(6, qualifier.find(' ', 6) - 6);
        if (!usage.empty() && usage.size() <= 9 && usage.find_first_not_of("0123456789") == std::string::npos)
        {
            FindItemUsageVisitor visitor(bot, ItemUsage(std::stoi(usage)));
            IterateItems(&visitor);
            found = visitor.GetResult();
        }
    }
    else if (!qualifier.empty())
    {
        // #226 (AC parity): AC's last resort in InventoryAction::parseItems is a bare item *name*
        // lookup - AC mod-playerbots Ai/Base/Actions/InventoryAction.cpp:345-350:
        //
        //     if (!isReservedQualifier(text))
        //     {
        //         FindNamedItemVisitor visitor(bot, text);
        //         IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);
        //         found.insert(visitor.GetResult().begin(), visitor.GetResult().end());
        //     }
        //
        // This fork's truncated port dropped that branch, so *every* named-item qualifier resolved to
        // an empty vector. That silently broke every ItemCountTrigger: AI_VALUE2(uint32, "item count",
        // item) was permanently 0, so ItemCountTrigger::IsActive() (GenericTriggers.cpp:415, same
        // one-liner as AC's :417) was permanently true. The clearest live symptom was the warlock's
        // "no healthstone" trigger (WarlockTriggers.cpp HasHealthstoneTrigger ->
        // WarlockConjuredItemTrigger, and GenericWarlockNonCombatStrategy.cpp:67 -> "create healthstone"):
        // the bot conjured a healthstone, never counted it, and re-conjured every AI tick.
        //
        // AC's isReservedQualifier() guard is not needed here: this is the final else of a chain that
        // already consumed every reserved qualifier (food/drink/mana potion/healing potion) and every
        // numeric item id above. FindNamedItemVisitor is AC's own visitor, already ported on this fork
        // (ItemVisitors.h:179-191, case-insensitive substring match on ItemTemplate::Name1) and used by
        // TradeAction.cpp:193.
        FindNamedItemVisitor visitor(bot, qualifier);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }

    return found;
}
