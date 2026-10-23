/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BuyAction.cpp for issue #258 (NPC interaction layer).
 * Gives bots a vendor buy path so they can top up reagents / food / ammo / gear.
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *
 *  - chat->FormatItem(proto) -> ChatHelper::FormatItem(proto): this fork's ChatHelper formatters are
 *    static (AI/ChatHelper.h:48, landed by #257/#322).
 *
 *  - chat->parseItems(link) -> ChatHelper::parseItems(link) (#331): the ChatHelper member landed in
 *    AI/ChatHelper.h, replacing the file-local ParseItemLinks() this file carried. That copy was a
 *    verbatim reproduction of AC Bot/Cmd/ChatHelper.cpp:286-310, so buy-by-link behaviour is
 *    unchanged - only the number of copies went from two to one.
 *
 *  - NeedMoneyFor and the budget values were file-local workarounds here until #332 ported AC's own
 *    Ai/Base/Value/BudgetValues.{h,cpp} to strategy/value/BudgetValues.{h,cpp}. Both the file-local
 *    enum copy and the file-local FreeMoneyFor() are gone; this file now includes "BudgetValues.h"
 *    and uses AC's enum and AC's AI_VALUE2(uint32, "free money for", ...) comparison verbatim. A bot
 *    now spends its money minus the reserve TotalMoneyNeededForValue holds for the purposes that
 *    outrank this one, which is what the purchase gate has always been meant to test.
 *
 *  - botAI->HasCheat(BotCheatMask::gold) (AC :233-239): the whole bot-cheat-mask feature is dormant
 *    on this fork (PlayerbotAI.h:319 has "cheatMask" commented out; there is no
 *    HasCheat()/BotCheatMask), exactly as recorded in RepairAllAction.cpp:10-15. The temporary
 *    "give the bot 10000000 copper" step is omitted - the purchase then costs the bot its real
 *    money, which is what a real player pays anyway. Flagged as an AC feature that cannot be
 *    ported without touching PlayerbotAI.h.
 *
 *  - VendorItemData / VendorItem / GetVendorItems() / BuyItemFromVendorSlot() all exist on this core
 *    with the same signatures AC uses (Creature.h:143-190, Creature.h:371, Player.h:1646), so that
 *    half of the port is unchanged. WorldPackets::Item::SellItem is NOT used here at all - the buy
 *    path goes through Player::BuyItemFromVendorSlot directly, as in AC.
 *
 *  - BroadcastHelper: not ported to this fork (see #322). AC's BuyAction has no BroadcastHelper
 *    call; nothing omitted here.
 *
 *  - PlayerbotTextMgr: not ported to this fork (see #322). AC's BuyAction reports no failure through
 *    it either - its two user-visible strings are literals ("Nobody sells ... nearby", the module's
 *    own error path), reproduced verbatim below.
 */

#include "BuyAction.h"

// #332: AC's own NeedMoneyFor enum and budget values (was a file-local copy of the enum).
#include "BudgetValues.h"
#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "StatsWeightCalculator.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Playerbots.h"

#include <algorithm>
#include <sstream>
#include <unordered_map>

bool BuyAction::Execute(Event event)
{
    bool buyUseful = false;
    ItemIds itemIds;
    std::string const link = event.getParam();

    if (link == "vendor")
        buyUseful = true;
    else
    {
        itemIds = ChatHelper::parseItems(link);
    }

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    bool vendored = false;
    bool result = false;
    for (GuidVector::iterator i = vendors.begin(); i != vendors.end(); ++i)
    {
        ObjectGuid vendorguid = *i;
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        vendored = true;

        if (buyUseful)
        {
            // Items are evaluated from high-level to low level.
            // For each item the bot checks again if an item is usefull.
            // Bot will buy until no usefull items are left.

            VendorItemData const* tItems = pCreature->GetVendorItems();
            if (!tItems)
                continue;

            VendorItemList m_items_sorted = tItems->m_items;

            m_items_sorted.erase(std::remove_if(m_items_sorted.begin(), m_items_sorted.end(),
                                                [](VendorItem* i)
                                                {
                                                    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(i->item);
                                                    return !proto;
                                                }),
                                 m_items_sorted.end());

            if (m_items_sorted.empty())
                continue;

            StatsWeightCalculator calculator(bot);
            calculator.SetItemSetBonus(false);
            calculator.SetOverflowPenalty(false);

            std::sort(m_items_sorted.begin(), m_items_sorted.end(),
                      [&calculator](VendorItem* i, VendorItem* j)
                      {
                          ItemTemplate const* item1 = sObjectMgr->GetItemTemplate(i->item);
                          ItemTemplate const* item2 = sObjectMgr->GetItemTemplate(j->item);

                          if (!item1 || !item2)
                              return false;

                          float score1 = calculator.CalculateItem(item1->ItemId);
                          float score2 = calculator.CalculateItem(item2->ItemId);

                          // Fallback to itemlevel if either score is 0
                          if (score1 == 0 || score2 == 0)
                          {
                              score1 = item1->ItemLevel;
                              score2 = item2->ItemLevel;
                          }
                          return score1 > score2;  // Sort in descending order (highest score first)
                      });

            std::unordered_map<uint32, float> bestPurchasedItemScore;  // Track best item score per InventoryType

            for (auto& tItem : m_items_sorted)
            {
                uint32 maxPurchases = 1;  // Default to buying once
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(tItem->item);
                if (!proto)
                    continue;

                if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_PROJECTILE)
                {
                    maxPurchases = 10;  // Allow up to 10 purchases if it's a consumable or projectile
                }

                for (uint32 i = 0; i < maxPurchases; i++)
                {
                    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", tItem->item);

                    uint32 invType = proto->InventoryType;

                    // Calculate item score
                    float newScore = calculator.CalculateItem(proto->ItemId);

                    // Skip if we already bought a better item for this slot
                    if (bestPurchasedItemScore.find(invType) != bestPurchasedItemScore.end() &&
                        bestPurchasedItemScore[invType] > newScore)
                    {
                        break;  // Skip lower-scoring items
                    }

                    // Check the bot's currently equipped item for this slot
                    uint8 dstSlot = botAI->FindEquipSlot(proto, NULL_SLOT, true);
                    Item* oldItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, dstSlot);

                    float oldScore = 0.0f;
                    if (oldItem)
                    {
                        ItemTemplate const* oldItemProto = oldItem->GetTemplate();
                        if (oldItemProto)
                            oldScore = calculator.CalculateItem(oldItemProto->ItemId);
                    }

                    // Skip if the bot already has a better or equal item equipped
                    if (oldScore > newScore)
                        break;

                    uint32 price = proto->BuyPrice;
                    price = uint32(floor(price * bot->GetReputationPriceDiscount(pCreature)));

                    NeedMoneyFor needMoneyFor = NeedMoneyFor::none;
                    switch (usage)
                    {
                        case ITEM_USAGE_REPLACE:
                        case ITEM_USAGE_EQUIP:
                        case ITEM_USAGE_BAD_EQUIP:
                        case ITEM_USAGE_BROKEN_EQUIP:
                            needMoneyFor = NeedMoneyFor::gear;
                            break;
                        case ITEM_USAGE_AMMO:
                            needMoneyFor = NeedMoneyFor::ammo;
                            break;
                        case ITEM_USAGE_QUEST:
                            needMoneyFor = NeedMoneyFor::anything;
                            break;
                        case ITEM_USAGE_USE:
                            needMoneyFor = NeedMoneyFor::consumables;
                            break;
                        case ITEM_USAGE_SKILL:
                            needMoneyFor = NeedMoneyFor::tradeskill;
                            break;
                        default:
                            break;
                    }

                    if (needMoneyFor == NeedMoneyFor::none)
                        break;

                    // AC BuyAction.cpp:167, verbatim. The budget is real as of #332 - see the file header.
                    if (AI_VALUE2(uint32, "free money for", uint32(needMoneyFor)) < price)
                        break;

                    if (!BuyItem(tItems, vendorguid, proto))
                        break;

                    // Store the best item score per InventoryType
                    bestPurchasedItemScore[invType] = newScore;

                    if (needMoneyFor == NeedMoneyFor::gear)
                    {
                        botAI->DoSpecificAction("equip upgrades packet action");
                    }
                }
            }
        }
        else
        {
            if (itemIds.empty())
                return false;

            for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
            {
                uint32 itemId = *i;
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                result |= BuyItem(pCreature->GetVendorItems(), vendorguid, proto);

                if (!result)
                {
                    std::ostringstream out;
                    out << "Nobody sells " << ChatHelper::FormatItem(proto) << " nearby";
                    botAI->TellMaster(out.str());
                    continue;
                }

                ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemId);
                if (usage == ITEM_USAGE_REPLACE || usage == ITEM_USAGE_EQUIP ||
                    usage == ITEM_USAGE_BAD_EQUIP || usage == ITEM_USAGE_BROKEN_EQUIP)
                {
                    botAI->DoSpecificAction("equip upgrades packet action");
                    break;
                }
            }
        }
    }

    return vendored;
}

// AC BuyAction.cpp:219-254
bool BuyAction::BuyItem(VendorItemData const* tItems, ObjectGuid vendorguid, ItemTemplate const* proto)
{
    if (!tItems || !proto)
        return false;

    uint32 itemId = proto->ItemId;
    uint32 oldCount = bot->GetItemCount(itemId, false);

    for (uint32 slot = 0; slot < tItems->GetItemCount(); ++slot)
    {
        if (tItems->GetItem(slot)->item != itemId)
            continue;

        // #258: AC's botAI->HasCheat(BotCheatMask::gold) { bot->SetMoney(10000000); } before the
        // purchase and SetMoney(botMoney) after it are omitted - see the file header.

        bot->BuyItemFromVendorSlot(vendorguid, slot, itemId, 1, NULL_BAG, NULL_SLOT);

        uint32 newCount = bot->GetItemCount(itemId, false);
        if (newCount > oldCount)
        {
            std::ostringstream out;
            out << "Buying " << ChatHelper::FormatItem(proto);
            botAI->TellMaster(out.str());
            return true;
        }

        return false;
    }

    return false;
}