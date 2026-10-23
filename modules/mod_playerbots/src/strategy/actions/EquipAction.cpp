/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/EquipAction.cpp for issue #188 (S6 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - AC builds a synthetic CMSG_AUTOEQUIP_ITEM_SLOT/CMSG_AUTOSTORE_BAG_ITEM WorldPacket and feeds it
 *    through WorldPackets::Item::AutoEquipItemSlot/bot->GetSession()->HandleAutoEquipItemSlotOpcode().
 *    Neither the typed WorldPackets::Item packet-struct classes nor an "ItemPackets.h" exist on this
 *    core (it still uses the older raw-WorldPacket handler signatures, e.g.
 *    Server/WorldSession.h:820 `void HandleAutoEquipItemSlotOpcode(WorldPacket& recvPacket)`), and
 *    that handler's own body (Handlers/ItemHandler.cpp:123, after its own bit-packed guid parse) does
 *    nothing more than `bot->SwapItem(item->GetPos(), dstpos)`. Calling Player::SwapItem() directly
 *    is therefore both simpler and lower-risk than constructing a fake packet (SwapItem is already
 *    used elsewhere in this very function, and by AC itself, for the bag-equip case) - see
 *    EquipToSlot() below.
 *  - chat->FormatItem(itemProto): no equivalent on this fork's ChatHelper (see ItemUsageValue.cpp's
 *    file header) - proto->Name1 is used for the "Equipping X" messages instead.
 *  - chat->parseItems(text): same gap - a local ParseItemIds() (plain numeric ids / "item:<id>" link
 *    segments, comma or space separated) is used instead, matching LootStrategyAction.cpp/
 *    LootRollAction.cpp's equivalent helpers.
 *  - INVTYPE_AMMO / bot->SetAmmo(): ammunition was removed from the game in Cataclysm (see
 *    ItemUsageValue.cpp's file header); this branch is dropped (proto->InventoryType == INVTYPE_AMMO
 *    can no longer occur for any real 5.4.8 item, so this is "AC drops unknown ones" territory).
 *  - CollectItemsVisitor (AC's "every item in bags" visitor) does not exist on this fork; a small
 *    local CollectAllItemsVisitor (IterateItemsVisitor) replaces it.
 *  - "item upgrade" AI value (ItemUpgradeValue, strategy/value/ItemUsageValue.{h,cpp}) is a new #188
 *    registration - see strategy/value/ValueContext.h.
 *  - sRandomPlayerbotMgr.IsRandomBot(bot) -> sRandomPlayerbotMgr->IsRandomBot(bot) (pointer singleton
 *    on this fork, see Manager/RandomPlayerbotMgr.h:220).
 *  - EquipUpgradesPacketAction::Execute() additionally parses the "trade status"/"item push result"
 *    packet payload in AC, purely as an early-exit optimization (skip the scan if the one item that
 *    triggered this can't be equipped). Both source packets are bit-packed on this core and "trade
 *    status" isn't even wired up as a trigger yet (trade is unported, see the design doc's S8);
 *    SelectInventoryItemsToEquip() already safely (idempotently) re-scans the whole bag, so the
 *    optimization is dropped rather than risk a silent bit-packed misparse for a pure perf gain -
 *    this action just always re-scans when triggered (by "item push result" or "loot roll won", see
 *    strategy/generic/WorldPacketHandlerStrategy.cpp).
 */

#include "EquipAction.h"

#include "Event.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "StatsWeightCalculator.h"

#include <cctype>
#include <cstring>

namespace
{
// 5.4.8: see the chat->parseItems() note in the file header.
ItemIds ParseItemIds(std::string const& text)
{
    ItemIds ids;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t itemPos = text.find("item:", pos);
        size_t digitsStart = std::string::npos;
        size_t searchFrom = pos;

        if (itemPos != std::string::npos)
        {
            digitsStart = itemPos + strlen("item:");
            searchFrom = digitsStart;
        }
        else
        {
            // No more item links - scan the remainder for bare numeric tokens.
            while (searchFrom < text.size() && !isdigit((unsigned char)text[searchFrom]))
                ++searchFrom;
            digitsStart = searchFrom;
        }

        if (digitsStart == std::string::npos || digitsStart >= text.size() || !isdigit((unsigned char)text[digitsStart]))
            break;

        size_t digitsEnd = digitsStart;
        while (digitsEnd < text.size() && isdigit((unsigned char)text[digitsEnd]))
            ++digitsEnd;

        ids.insert(uint32(atoi(text.substr(digitsStart, digitsEnd - digitsStart).c_str())));
        pos = digitsEnd;
    }

    return ids;
}

// 5.4.8: see the CollectItemsVisitor note in the file header.
class CollectAllItemsVisitor : public IterateItemsVisitor
{
public:
    bool Visit(Item* item) override
    {
        items.push_back(item);
        return true;
    }

    std::vector<Item*> items;
};

// 5.4.8: see the SwapItem note in the file header.
void EquipToSlot(Player* bot, Item* item, uint8 slot)
{
    uint16 dstpos = uint16(slot) | (uint16(INVENTORY_SLOT_BAG_0) << 8);
    if (item->GetPos() == dstpos)
        return;
    bot->SwapItem(item->GetPos(), dstpos);
}
}

bool EquipAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemIds ids = ParseItemIds(text);
    EquipItems(ids);
    return true;
}

void EquipAction::EquipItems(ItemIds ids)
{
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        EquipItem(&visitor);
    }
}

// Return bagslot with smalest bag.
uint8 EquipAction::GetSmallestBagSlot()
{
    int8 curBag = 0;
    uint32 curSlots = 0;
    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag);
        if (pBag)
        {
            if (curBag > 0 && curSlots < pBag->GetBagSize())
                continue;

            curBag = bag;
            curSlots = pBag->GetBagSize();
        }
        else
            return bag;
    }

    return curBag;
}

void EquipAction::EquipItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    if (!items.empty())
        EquipItem(*items.begin());
}

void EquipAction::EquipItem(Item* item)
{
    uint8 bagIndex = item->GetBagSlot();
    uint8 slot = item->GetSlot();
    ItemTemplate const* itemProto = item->GetTemplate();
    uint32 itemId = itemProto->ItemId;
    uint8 invType = itemProto->InventoryType;

    // Handle bags first
    bool equippedBag = false;
    if (itemProto->Class == ITEM_CLASS_CONTAINER)
    {
        // Attempt to equip as a bag
        uint8 newBagSlot = GetSmallestBagSlot();

        if (newBagSlot > 0)
        {
            uint16 src = ((bagIndex << 8) | slot);
            uint16 dst = ((INVENTORY_SLOT_BAG_0 << 8) | newBagSlot);
            bot->SwapItem(src, dst);
            equippedBag = true;
        }
    }

    // If we didn't equip as a bag, try to equip as gear
    if (!equippedBag)
    {
        // Ranged weapons aren't handled by the rest of the weapon equip logic
        // Handle them early here to avoid issues.
        if (invType == INVTYPE_RANGED || invType == INVTYPE_THROWN || invType == INVTYPE_RANGEDRIGHT)
        {
            EquipToSlot(bot, item, EQUIPMENT_SLOT_RANGED);

            std::ostringstream out;
            out << "Equipping " << itemProto->Name1 << " in ranged slot";
            botAI->TellMaster(out);
            return;
        }

        uint8 dstSlot = botAI->FindEquipSlot(itemProto, NULL_SLOT, true);

        // Check if the item is a weapon and whether the bot can dual wield or use Titan Grip
        bool isWeapon = (itemProto->Class == ITEM_CLASS_WEAPON);
        bool canTitanGrip = bot->CanTitanGrip();
        bool canDualWield = bot->CanDualWield();

        bool isTwoHander = (invType == INVTYPE_2HWEAPON);
        bool isValidTGWeapon = false;
        if (canTitanGrip && isTwoHander)
        {
            // Titan Grip-valid 2H weapon subclasses: Axe2, Mace2, Sword2
            isValidTGWeapon = (itemProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                               itemProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                               itemProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2);
        }

        // Check if the main hand currently has a 2H weapon equipped
        Item* currentMHItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        bool have2HWeaponEquipped = (currentMHItem && currentMHItem->GetTemplate()->InventoryType == INVTYPE_2HWEAPON);

        bool canDualWieldOrTG = (canDualWield || isTwoHander);

        // If this is a weapon and we can dual wield or Titan Grip, check if we can improve main/off-hand setup
        if (isWeapon && canDualWieldOrTG)
        {
            // Fetch current main hand and offhand items
            Item* mainHandItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
            Item* offHandItem  = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

            // Set up the stats calculator once and reuse results for performance
            StatsWeightCalculator calculator(bot);
            calculator.SetItemSetBonus(false);
            calculator.SetOverflowPenalty(false);

            // Calculate item scores once and store them
            float newItemScore = calculator.CalculateItem(itemId, item->GetItemRandomPropertyId());
            float mainHandScore = mainHandItem
                ? calculator.CalculateItem(mainHandItem->GetTemplate()->ItemId, mainHandItem->GetItemRandomPropertyId()) : 0.0f;
            float offHandScore = offHandItem
                ? calculator.CalculateItem(offHandItem->GetTemplate()->ItemId, offHandItem->GetItemRandomPropertyId()) : 0.0f;

            // Determine where this weapon can go
            bool canGoMain = (invType == INVTYPE_WEAPON ||
                              invType == INVTYPE_WEAPONMAINHAND ||
                              isTwoHander);

            bool canTGOff = false;
            if (canTitanGrip && isTwoHander && isValidTGWeapon)
                canTGOff = true;

            bool canGoOff = (invType == INVTYPE_WEAPON ||
                             invType == INVTYPE_WEAPONOFFHAND ||
                             canTGOff);

            // Check if the main hand item can go to offhand if needed
            bool mainHandCanGoOff = false;
            if (mainHandItem)
            {
                ItemTemplate const* mhProto = mainHandItem->GetTemplate();
                bool mhIsValidTG = false;
                if (canTitanGrip && mhProto->InventoryType == INVTYPE_2HWEAPON)
                {
                    mhIsValidTG = (mhProto->SubClass == ITEM_SUBCLASS_WEAPON_AXE2 ||
                                   mhProto->SubClass == ITEM_SUBCLASS_WEAPON_MACE2 ||
                                   mhProto->SubClass == ITEM_SUBCLASS_WEAPON_SWORD2);
                }

                mainHandCanGoOff = (mhProto->InventoryType == INVTYPE_WEAPON ||
                                    mhProto->InventoryType == INVTYPE_WEAPONOFFHAND ||
                                    (mhProto->InventoryType == INVTYPE_2HWEAPON && mhIsValidTG));
            }

            // Priority 1: Replace main hand if the new weapon is strictly better
            // and if conditions allow (e.g. no conflicting 2H logic)
            bool betterThanMH = (newItemScore > mainHandScore);
            // If a one-handed weapon is better, we can still use it instead of a two-handed weapon
            bool mhConditionOK = (invType != INVTYPE_2HWEAPON ||
                      (isTwoHander && !canTitanGrip) ||
                      (canTitanGrip && isValidTGWeapon));

            if (canGoMain && betterThanMH && mhConditionOK)
            {
                // Equip new weapon in main hand
                EquipToSlot(bot, item, EQUIPMENT_SLOT_MAINHAND);

                // Try moving old main hand weapon to offhand if beneficial
                if (mainHandItem && mainHandCanGoOff && (!offHandItem || mainHandScore > offHandScore))
                {
                    ItemTemplate const* oldMHProto = mainHandItem->GetTemplate();

                    EquipToSlot(bot, mainHandItem, EQUIPMENT_SLOT_OFFHAND);

                    std::ostringstream moveMsg;
                    moveMsg << "Main hand upgrade found. Moving " << oldMHProto->Name1 << " to offhand";
                    botAI->TellMaster(moveMsg);
                }

                std::ostringstream out;
                out << "Equipping " << itemProto->Name1 << " in main hand";
                botAI->TellMaster(out);
                return;
            }

            // Priority 2: If not better than main hand, check if better than offhand
            else if (canGoOff && newItemScore > offHandScore)
            {
                // Equip in offhand
                EquipToSlot(bot, item, EQUIPMENT_SLOT_OFFHAND);

                std::ostringstream out;
                out << "Equipping " << itemProto->Name1 << " in offhand";
                botAI->TellMaster(out);
                return;
            }
            else
            {
                // No improvement, do nothing
                return;
            }
        }

        // If not a special dual-wield/TG scenario or no improvement found, fall back to original logic
        if (dstSlot == EQUIPMENT_SLOT_FINGER1 ||
            dstSlot == EQUIPMENT_SLOT_TRINKET1 ||
            (dstSlot == EQUIPMENT_SLOT_MAINHAND && canDualWield &&
                ((invType != INVTYPE_2HWEAPON && !have2HWeaponEquipped) || (canTitanGrip && isValidTGWeapon))))
        {
            // Handle ring/trinket dual-slot logic
            Item* const equippedItems[2] = {
                bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot),
                bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot + 1)
            };

            if (equippedItems[0])
            {
                if (equippedItems[1])
                {
                    // Both slots are full - pick the worst item to replace, but only if new item is better
                    StatsWeightCalculator calc(bot);
                    calc.SetItemSetBonus(false);
                    calc.SetOverflowPenalty(false);

                    // Calculate new item score with random properties
                    int32 newItemRandomProp = item->GetItemRandomPropertyId();
                    float newItemScore = calc.CalculateItem(itemId, newItemRandomProp);

                    // Calculate equipped items scores with random properties
                    int32 firstRandomProp = equippedItems[0]->GetItemRandomPropertyId();
                    int32 secondRandomProp = equippedItems[1]->GetItemRandomPropertyId();
                    float firstItemScore = calc.CalculateItem(equippedItems[0]->GetTemplate()->ItemId, firstRandomProp);
                    float secondItemScore = calc.CalculateItem(equippedItems[1]->GetTemplate()->ItemId, secondRandomProp);

                    // Determine which slot (if any) should be replaced
                    bool betterThanFirst = newItemScore > firstItemScore;
                    bool betterThanSecond = newItemScore > secondItemScore;

                    // Early return if new item is not better than either equipped item
                    if (!betterThanFirst && !betterThanSecond)
                        return;

                    if (betterThanFirst && betterThanSecond)
                    {
                        // New item is better than both - replace the worse of the two equipped items
                        if (firstItemScore > secondItemScore)
                            dstSlot++; // Replace second slot (worse)
                        // else: keep dstSlot as-is (replace first slot)
                    }
                    else if (betterThanSecond)
                        dstSlot++; // Only better than second slot - replace it
                }
                else
                {
                    // Second slot empty, use it
                    dstSlot++;
                }
            }
        }

        // Equip the item in the chosen slot
        EquipToSlot(bot, item, dstSlot);
    }

    std::ostringstream out;
    out << "Equipping " << itemProto->Name1;
    botAI->TellMaster(out);
}

ItemIds EquipAction::SelectInventoryItemsToEquip()
{
    CollectAllItemsVisitor visitor;
    IterateItems(&visitor, ITERATE_ITEMS_IN_BAGS);

    ItemIds items;
    for (auto i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        Item* item = *i;
        if (!item)
            continue;

        ItemTemplate const* itemTemplate = item->GetTemplate();
        if (!itemTemplate)
            continue;

        //TODO Expand to Glyphs and Gems, that can be placed in equipment
        //Pre-filter non-equipable items
        if (itemTemplate->InventoryType == INVTYPE_NON_EQUIP)
            continue;

        int32 randomProperty = item->GetItemRandomPropertyId();
        uint32 itemId = item->GetTemplate()->ItemId;
        std::string itemUsageParam;
        if (randomProperty != 0)
            itemUsageParam = std::to_string(itemId) + "," + std::to_string(randomProperty);
        else
            itemUsageParam = std::to_string(itemId);

        ItemUsage usage = AI_VALUE2(ItemUsage, "item upgrade", itemUsageParam);
        if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_BAD_EQUIP)
            items.insert(itemId);
    }
    return items;
}

bool EquipUpgradesPacketAction::Execute(Event /*event*/)
{
    if (!sPlayerbotAIConfig->autoEquipUpgradeLoot && !sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    ItemIds items = SelectInventoryItemsToEquip();
    EquipItems(items);
    return true;
}

bool EquipUpgradeAction::Execute(Event /*event*/)
{
    ItemIds items = SelectInventoryItemsToEquip();
    EquipItems(items);
    return true;
}
