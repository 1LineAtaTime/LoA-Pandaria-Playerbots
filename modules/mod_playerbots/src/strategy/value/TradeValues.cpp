/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeValues.cpp for issue #193 (S7 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - IsRealPlayer(Player*): AC's global free helper (any player, not bot-controlled) does not exist on
 *    this fork - only the no-argument PlayerbotAI::IsRealPlayer() member ("is *this* bot itself a real
 *    player's own alt") does, which is a different question. `!GET_PLAYERBOT_AI(x)` ("x has no
 *    PlayerbotAI at all") is the fork-wide substitute used everywhere else this AC concept comes up
 *    (see TradeStatusAction.cpp's traderIsGameClientPlayer).
 *  - AC fetches myItems through `AI_VALUE2(vector<Item*>, "inventory items", "usage " + usage)`
 *    (InventoryItemValue + InventoryAction::Find()). This fork's InventoryAction::Find()
 *    (strategy/actions/InventoryAction.cpp) only resolves the four
 *    food/drink/potion qualifiers AC's own #149 port needed and a bare item id - it has no "usage N"
 *    branch (see ValueContext.h's own note next to the "inventory items" registration), so that
 *    lookup would silently return nothing on this fork. This action instead collects the bot's bag
 *    items directly (same loop as InventoryAction::IterateItems(), inlined here rather than via
 *    multiple inheritance - see the next point) and classifies each one with the already-working
 *    "item usage" value (by item id), matching AC's actual filter (ITEM_USAGE_NONE/VENDOR/AH/
 *    DISENCHANT) without needing the unported "usage N" qualifier.
 *  - Not based on InventoryAction (unlike most other item-scanning actions ported so far): this value
 *    would need IterateItems() *and* unambiguous bot/botAI/context access (GetTradeData(), GetMaster(),
 *    AI_VALUE/PAI_VALUE2) in the same function. ItemCountValue.cpp already documents that combining
 *    CalculatedValue<T> and InventoryAction as two separate AiNamedObject base chains makes bot/botAI/
 *    context ambiguous when referenced unqualified - workable there because Calculate() only needs
 *    qualifier and Find(), not workable here. The bag/backpack loop is inlined instead, so this class
 *    keeps CalculatedValue<T>/Qualified as its only base (single AiNamedObject chain, same shape as
 *    ItemUsageValue).
 */

#include "TradeValues.h"

#include <algorithm>

#include "ItemVisitors.h"
#include "Playerbots.h"
#include "TradeData.h"
#include "WorldPosition.h"

std::vector<Item*> ItemsUsefulToGiveValue::Calculate()
{
    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");
    Player* player = guidP.GetPlayer();

    std::vector<Item*> giveItems;

    // 5.4.8: see the IsRealPlayer() note in the file header. The null check on `player` is a small
    // defensive addition - AC does not have it, relying on GuidPosition::GetPlayer() and
    // GET_PLAYERBOT_AI(nullptr) both being safe to call with no target set.
    if (!player || !GET_PLAYERBOT_AI(botAI->GetMaster()) || !GET_PLAYERBOT_AI(player))
        return giveItems;

    static std::vector<ItemUsage> const myUsages = {ITEM_USAGE_NONE, ITEM_USAGE_VENDOR, ITEM_USAGE_AH,
                                                     ITEM_USAGE_DISENCHANT};

    // 5.4.8: see the "inventory items"/"usage N" note in the file header - collects the bot's bag
    // items directly instead (same slots InventoryAction::IterateItems() walks by default).
    std::vector<Item*> myItems;
    for (uint32 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            myItems.push_back(item);

    for (uint32 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* item = pBag->GetItemByPos(j))
                    myItems.push_back(item);

    std::reverse(myItems.begin(), myItems.end());

    for (Item* item : myItems)
    {
        if (!item->CanBeTraded())
            continue;

        if (bot->GetTradeData() && bot->GetTradeData()->HasItem(item->GetGUID()))
            continue;

        ItemUsage myUsage = AI_VALUE2(ItemUsage, "item usage", std::to_string(item->GetEntry()));
        if (std::find(myUsages.begin(), myUsages.end(), myUsage) == myUsages.end())
            continue;

        ItemUsage otherUsage = PAI_VALUE2(ItemUsage, "item usage", std::to_string(item->GetEntry()));

        if (std::find(myUsages.begin(), myUsages.end(), otherUsage) == myUsages.end())
            giveItems.push_back(item);
    }

    return giveItems;
}
