/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeAction.cpp for issue #193 (S7 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - chat->parseGameobjects(text): this fork's ChatHelper (AI/ChatHelper.h) has no item/player-link
 *    parser at all - not even the "Hfound:" gameobject-link format AC's own parseGameobjects() reads.
 *    A ".t"/".nt" chat command is always typed straight to the bot by whoever it should trade with, so
 *    this always targets botAI->GetMaster() directly, which is also AC's own fallback for exactly this
 *    case (`if (!player && botAI->GetMaster()) player = botAI->GetMaster();`).
 *  - chat->parseItems(text): same ChatHelper gap. A local ParseItemIds() (numeric item id, or an item
 *    link's "item:<id>" segment - same helper as LootStrategyAction.cpp/EquipAction.cpp) replaces it,
 *    resolved to actual bag items with the existing FindItemByIdVisitor (strategy/ItemVisitors.h) and
 *    InventoryAction::IterateItems() - so only an id/link, not an item name, can be traded by text.
 *  - chat->parseMoney(text): ChatHelper::parseMoney() (AI/ChatHelper.cpp, #335 = AC Bot/Cmd/ChatHelper.cpp:251-284)
 *    is called directly, as in AC. (#341: the file-local ParseMoney() this file carried before #335 is removed; it
 *    was a copy that lacked AC's "any other character zeroes the amount" branch, so a token like "x5g" read as 5g
 *    where AC gives 0.)
 *  - CMSG_INITIATE_TRADE bit-packs the target guid on this core
 *    (Handlers/TradeHandler.cpp:636-657, WorldSession::HandleInitiateTradeOpcode) instead of the plain
 *    8-byte guid AC's WotLK client sent (`packet << player->GetGUID();`); the packet built below
 *    writes the bits/bytes in the exact index order that handler reads them back in.
 *  - CMSG_SET_TRADE_GOLD reads a uint64 (Handlers/TradeHandler.cpp:786-797,
 *    WorldSession::HandleSetTradeGoldOpcode: `recvPacket >> gold;` where gold is uint64), not the
 *    uint32 AC's WotLK client sent.
 *  - CMSG_SET_TRADE_ITEM reads tradeSlot, then SLOT, then BAG, in that order
 *    (Handlers/TradeHandler.cpp:799-840: `recvPacket >> tradeSlot; recvPacket >> slot; recvPacket >>
 *    bag;`) - the reverse of AC's WotLK order (tradeSlot, bag, slot;
 *    `packet << (uint8)item->GetBagSlot(); packet << (uint8)item->GetSlot();`).
 *  - TradeAction::slots (AC TradeAction.h:25, a `static std::map<std::string, uint32>`) is declared in
 *    AC's header but never defined or referenced anywhere in TradeAction.cpp - dead in AC itself - and
 *    is not ported.
 */

#include "TradeAction.h"

#include "Event.h"
#include "ItemVisitors.h"
#include "Playerbots.h"
#include "TradeData.h"

#include <cctype>
#include <cstring>

namespace
{
// 5.4.8: see the chat->parseItems() note in the file header (LootStrategyAction.cpp/EquipAction.cpp
// pattern).
ItemIds ParseItemIds(std::string const& text)
{
    ItemIds ids;
    size_t pos = 0;
    while (pos < text.size())
    {
        size_t itemPos = text.find("item:", pos);
        size_t digitsStart;
        size_t searchFrom = pos;

        if (itemPos != std::string::npos)
        {
            digitsStart = itemPos + strlen("item:");
            searchFrom = digitsStart;
        }
        else
        {
            while (searchFrom < text.size() && !isdigit((unsigned char)text[searchFrom]))
                ++searchFrom;
            digitsStart = searchFrom;
        }

        if (digitsStart >= text.size() || !isdigit((unsigned char)text[digitsStart]))
            break;

        size_t digitsEnd = digitsStart;
        while (digitsEnd < text.size() && isdigit((unsigned char)text[digitsEnd]))
            ++digitsEnd;

        ids.insert(uint32(atoi(text.substr(digitsStart, digitsEnd - digitsStart).c_str())));
        pos = digitsEnd;
    }

    return ids;
}
}

bool TradeAction::Execute(Event event)
{
    std::string const text = event.getParam();

    // If text starts with any excluded prefix, don't process it further.
    for (auto const& prefix : sPlayerbotAIConfig->tradeActionExcludedPrefixes)
    {
        if (text.find(prefix) == 0)
            return false;
    }

    if (!bot->GetTrader())
    {
        // 5.4.8: see the chat->parseGameobjects() note in the file header.
        Player* player = botAI->GetMaster();
        if (!player)
            return false;

        if (!player->GetTrader())
        {
            // 5.4.8: see the CMSG_INITIATE_TRADE note in the file header. Bit order mirrors
            // WorldSession::HandleInitiateTradeOpcode's read order exactly.
            WorldPacket packet(CMSG_INITIATE_TRADE);
            ObjectGuid guid = player->GetGUID();
            packet.WriteBit(guid[5]);
            packet.WriteBit(guid[1]);
            packet.WriteBit(guid[4]);
            packet.WriteBit(guid[2]);
            packet.WriteBit(guid[3]);
            packet.WriteBit(guid[7]);
            packet.WriteBit(guid[0]);
            packet.WriteBit(guid[6]);
            packet.FlushBits();
            packet.WriteByteSeq(guid[4]);
            packet.WriteByteSeq(guid[6]);
            packet.WriteByteSeq(guid[2]);
            packet.WriteByteSeq(guid[0]);
            packet.WriteByteSeq(guid[3]);
            packet.WriteByteSeq(guid[7]);
            packet.WriteByteSeq(guid[5]);
            packet.WriteByteSeq(guid[1]);
            bot->GetSession()->HandleInitiateTradeOpcode(packet);
            return true;
        }
        else if (player->GetTrader() != bot)
            return false;
    }

    // 5.4.8: see the CMSG_SET_TRADE_GOLD note in the file header.
    uint32 copper = ChatHelper::parseMoney(text);
    if (copper > 0)
    {
        WorldPacket packet(CMSG_SET_TRADE_GOLD, 8);
        packet << uint64(copper);
        bot->GetSession()->HandleSetTradeGoldOpcode(packet);
    }

    size_t pos = text.rfind(" ");
    int count = pos != std::string::npos ? atoi(text.substr(pos + 1).c_str()) : 1;

    ItemIds ids = ParseItemIds(text);
    std::vector<Item*> found;
    for (uint32 id : ids)
    {
        FindItemByIdVisitor visitor(id);
        IterateItems(&visitor);
        for (Item* item : visitor.GetResult())
            found.push_back(item);
    }

    // AC's InventoryAction::parseItems falls back to a name match, so "t <item name>" works too.
    if (found.empty())
    {
        FindNamedItemVisitor visitor(bot, text);
        IterateItems(&visitor);
        found = visitor.GetResult();
    }

    if (found.empty())
        return false;

    uint32 traded = 0;
    for (Item* item : found)
    {
        if (!bot->GetTrader() || item->IsInTrade())
            continue;

        int8 slot = item->CanBeTraded() ? -1 : TRADE_SLOT_NONTRADED;
        if (TradeItem(item, slot) && slot != TRADE_SLOT_NONTRADED && ++traded >= uint32(count))
            break;
    }

    return true;
}

bool TradeAction::TradeItem(Item const* item, int8 slot)
{
    int8 tradeSlot = -1;
    Item* itemPtr = const_cast<Item*>(item);

    TradeData* pTrade = bot->GetTradeData();
    if ((slot >= 0 && slot < TRADE_SLOT_COUNT) && pTrade->GetItem(TradeSlots(slot)) == nullptr)
        tradeSlot = slot;

    if (slot == TRADE_SLOT_NONTRADED)
        pTrade->SetItem(TRADE_SLOT_NONTRADED, itemPtr);
    else
    {
        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT && tradeSlot == -1; i++)
        {
            if (pTrade->GetItem(TradeSlots(i)) == itemPtr)
            {
                tradeSlot = i;

                WorldPacket packet(CMSG_CLEAR_TRADE_ITEM, 1);
                packet << (uint8)tradeSlot;
                bot->GetSession()->HandleClearTradeItemOpcode(packet);
                pTrade->SetItem(TradeSlots(i), nullptr);
                return true;
            }
        }

        for (uint8 i = 0; i < TRADE_SLOT_TRADED_COUNT && tradeSlot == -1; i++)
        {
            if (pTrade->GetItem(TradeSlots(i)) == nullptr)
                tradeSlot = i;
        }
    }

    if (tradeSlot == -1)
        return false;

    // 5.4.8: see the CMSG_SET_TRADE_ITEM note in the file header - slot/bag order swapped vs AC.
    WorldPacket packet(CMSG_SET_TRADE_ITEM, 3);
    packet << (uint8)tradeSlot;
    packet << (uint8)item->GetSlot();
    packet << (uint8)item->GetBagSlot();
    bot->GetSession()->HandleSetTradeItemOpcode(packet);
    return true;
}
