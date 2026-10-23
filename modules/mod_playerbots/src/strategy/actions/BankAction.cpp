/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BankAction.cpp (#258).
 *  - parseItems(text, ITERATE_ITEMS_IN_BANK) -> local ForEachItemInBank() + the same qualifier
 *    rules InventoryAction::Find() applies (numeric item id, else case-insensitive name substring);
 *    parseItems(text, ITERATE_ITEMS_IN_BAGS) -> InventoryAction::Find(text), unchanged.
 *  - chat->FormatItem -> ChatHelper::FormatItem (static on this fork).
 *  - PlayerbotAI::TellItems() -> file-local TellBankList() (see the header comment).
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault(...) -> AC's literal fallback string.
 *  - AC's BroadcastHelper is not ported (see #322); neither BankAction nor GuildBankAction has a
 *    BroadcastHelper call site, so nothing is omitted here.
 */

#include "BankAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "Item.h"
#include "ItemVisitors.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace
{
// AC InventoryAction::IterateItemsInBank-equivalent (AC InventoryAction.cpp:71-86 via the
// ITERATE_ITEMS_IN_BANK mask). Local because this fork's InventoryAction::IterateItems has no bank
// branch (strategy/actions/InventoryAction.cpp:39) and InventoryAction.h is not this slice's file.
template <class F>
void ForEachItemInBank(Player* bot, F&& f)
{
    for (uint32 i = BANK_SLOT_ITEM_START; i < BANK_SLOT_ITEM_END; ++i)
        if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            if (!f(pItem))
                return;

    for (uint32 i = BANK_SLOT_BAG_START; i < BANK_SLOT_BAG_END; ++i)
        if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                if (Item* pItem = pBag->GetItemByPos(j))
                    if (!f(pItem))
                        return;
}

// AC's bank-side parseItems() qualifier resolution: an item id, else a case-insensitive name match,
// both via AC's own visitors (ItemVisitors.h:179-202), matching InventoryAction::Find()'s last two
// branches. AC's reserved qualifiers (food/drink/potions) are meaningless against a bank and are
// not ported.
std::vector<Item*> ParseItemsInBank(Player* bot, std::string const qualifier)
{
    std::vector<Item*> found;

    if (uint32 const itemId = uint32(atoi(qualifier.c_str())))
    {
        FindItemByIdVisitor visitor(itemId);
        ForEachItemInBank(bot, [&](Item* item) { return visitor.Visit(item); });
        found = visitor.GetResult();
    }
    else if (!qualifier.empty())
    {
        FindNamedItemVisitor visitor(bot, qualifier);
        ForEachItemInBank(bot, [&](Item* item) { return visitor.Visit(item); });
        found = visitor.GetResult();
    }

    return found;
}

// AC's InventoryAction::TellItems() stand-in: proto->Name1 (id) xCount, plus a [soulbound] marker -
// the same simplification TradeStatusAction.cpp:168-188 already uses on this fork.
void TellBankList(PlayerbotAI* botAI, std::map<uint32, uint32> const& items, std::map<uint32, bool> const& soulbound)
{
    std::ostringstream out;
    for (auto const& kv : items)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(kv.first);
        if (!proto)
            continue;

        out << proto->Name1 << " (" << kv.first << ") x" << kv.second;

        auto sb = soulbound.find(kv.first);
        if (sb != soulbound.end() && sb->second)
            out << " [soulbound]";

        out << "; ";
    }

    if (!out.str().empty())
        botAI->TellMaster(out);
}
} // namespace

// AC BankAction.cpp:13-30
bool BankAction::Execute(Event event)
{
    std::string const text = event.getParam();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const npcGuid : npcs)
    {
        Unit* npc = botAI->GetUnit(npcGuid);
        if (!npc || !npc->HasNpcFlag(UNIT_NPC_FLAG_BANKER))
            continue;

        return ExecuteBank(text, npc);
    }

    // #258: PlayerbotTextMgr not ported - AC's literal fallback text is used verbatim
    // (AC BankAction.cpp:27-28).
    botAI->TellError("Cannot find banker nearby");
    return false;
}

// AC BankAction.cpp:32-67
bool BankAction::ExecuteBank(std::string const text, Unit* /*bank*/)
{
    if (text.empty() || text == "?")
    {
        ListItems();
        return true;
    }

    bool result = false;
    if (text[0] == '-')
    {
        std::vector<Item*> found = ParseItemsInBank(bot, text.substr(1));
        for (Item* item : found)
        {
            if (!item || !item->GetTemplate())
                continue;

            result &= Withdraw(item->GetTemplate()->ItemId);
        }
    }
    else
    {
        std::vector<Item*> found = Find(text);
        if (found.empty())
            return false;

        for (Item* item : found)
        {
            if (!item)
                continue;

            result &= Deposit(item);
        }
    }

    return result;
}

// AC BankAction.cpp:69-90
bool BankAction::Withdraw(uint32 itemid)
{
    Item* pItem = FindItemInBank(itemid);
    if (!pItem)
        return false;

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->StoreItem(dest, pItem, true);

    std::ostringstream out;
    out << "got " << ChatHelper::FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " from bank";
    botAI->TellMaster(out.str());
    return true;
}

// AC BankAction.cpp:92-110
bool BankAction::Deposit(Item* pItem)
{
    std::ostringstream out;

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanBankItem(NULL_BAG, NULL_SLOT, dest, pItem, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, pItem, nullptr);
        return false;
    }

    bot->RemoveItem(pItem->GetBagSlot(), pItem->GetSlot(), true);
    bot->BankItem(dest, pItem, true);

    out << "put " << ChatHelper::FormatItem(pItem->GetTemplate(), pItem->GetCount()) << " to bank";
    botAI->TellMaster(out.str());
    return true;
}

// AC BankAction.cpp:112-138
void BankAction::ListItems()
{
    botAI->TellMaster("=== Bank ===");

    std::map<uint32, uint32> items;
    std::map<uint32, bool> soulbound;
    ForEachItemInBank(bot, [&](Item* pItem)
    {
        if (!pItem || !pItem->GetTemplate())
            return true;

        items[pItem->GetTemplate()->ItemId] += pItem->GetCount();
        soulbound[pItem->GetTemplate()->ItemId] = pItem->IsSoulBound();
        return true;
    });

    TellBankList(botAI, items, soulbound);
}

// AC BankAction.cpp:140-175
Item* BankAction::FindItemInBank(uint32 itemId)
{
    Item* found = nullptr;

    ForEachItemInBank(bot, [&](Item* pItem)
    {
        if (!pItem)
            return true;

        ItemTemplate const* proto = pItem->GetTemplate();
        if (proto && proto->ItemId == itemId)
        {
            found = pItem;
            return false; // stop the walk - AC returns here too
        }

        return true;
    });

    return found;
}