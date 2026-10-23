/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UnequipAction.cpp for issue #188 (S6 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - AC builds a synthetic CMSG_AUTOSTORE_BAG_ITEM WorldPacket and feeds it through
 *    WorldPackets::Item::AutoStoreBagItem/bot->GetSession()->HandleAutoStoreBagItemOpcode(). Neither
 *    exists on this core (see EquipAction.cpp's file header for the same gap); that handler's body
 *    (Handlers/ItemHandler.cpp:969) is a plain, non-bit-packed
 *    `recvData >> srcSlot >> srcBag >> dstBag;` followed by CanUnequipItem/CanStoreItem/RemoveItem/
 *    StoreItem calls with no other side effects worth reproducing via a fake packet, so those Player
 *    methods are called directly instead - see UnequipItem() below.
 *  - chat->parseItems(text)/chat->parseSlot(name): no equivalent on this fork's ChatHelper. Item-id/
 *    link parsing uses the same local ParseItemIds() as EquipAction.cpp/LootStrategyAction.cpp;
 *    AC's comma-separated *slot name* fallback ("unequip head,legs") has no parser to port and is
 *    dropped - flagged as an AC convenience feature not ported for #188.
 *  - chat->FormatItem(item->GetTemplate()): no equivalent - item->GetTemplate()->Name1 is used for
 *    the "X unequipped" message instead.
 *  - IterateItems(visitor, ITERATE_ALL_ITEMS) needs to see equipped items, not just bags -
 *    InventoryAction::IterateItems() was extended with an ITERATE_ITEMS_IN_EQUIP branch for this (see
 *    InventoryAction.h/.cpp); ITERATE_ITEMS_IN_BANK is still not implemented (unequip only needs
 *    equipped + bags).
 */

#include "UnequipAction.h"

#include "Event.h"
#include "Playerbots.h"

#include <algorithm>
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
        size_t digitsStart;

        if (itemPos != std::string::npos)
            digitsStart = itemPos + strlen("item:");
        else
        {
            digitsStart = pos;
            while (digitsStart < text.size() && !isdigit((unsigned char)text[digitsStart]))
                ++digitsStart;
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

bool UnequipAction::Execute(Event event)
{
    std::string const text = event.getParam();

    ItemIds ids = ParseItemIds(text);
    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        UnequipItem(&visitor);
    }

    return true;
}

void UnequipAction::UnequipItem(FindItemVisitor* visitor)
{
    IterateItems(visitor, ITERATE_ALL_ITEMS);
    std::vector<Item*> items = visitor->GetResult();
    if (!items.empty())
    {
        // Prefer equipped item
        auto equipped = std::find_if(items.begin(), items.end(),
                                     [](Item* item)
                                     {
                                         uint8 bag = item->GetBagSlot();
                                         return bag == INVENTORY_SLOT_BAG_0 && item->GetSlot() < EQUIPMENT_SLOT_END;
                                     });

        if (equipped != items.end())
            UnequipItem(*equipped);
        else
            UnequipItem(*items.begin());
    }
}

void UnequipAction::UnequipItem(Item* item)
{
    // 5.4.8: see the CMSG_AUTOSTORE_BAG_ITEM note in the file header - this mirrors
    // WorldSession::HandleAutoStoreBagItemOpcode's body directly instead of faking that packet.
    uint16 src = item->GetPos();

    if (Player::IsEquipmentPos(src) || Player::IsBagPos(src))
    {
        InventoryResult msg = bot->CanUnequipItem(src, !Player::IsBagPos(src));
        if (msg != EQUIP_ERR_OK)
        {
            bot->SendEquipError(msg, item, nullptr);
            return;
        }
    }

    ItemPosCountVec dest;
    InventoryResult msg = bot->CanStoreItem(NULL_BAG, NULL_SLOT, dest, item, false);
    if (msg != EQUIP_ERR_OK)
    {
        bot->SendEquipError(msg, item, nullptr);
        return;
    }

    std::string const itemName = item->GetTemplate()->Name1;

    bot->RemoveItem(item->GetBagSlot(), item->GetSlot(), true);
    bot->StoreItem(dest, item, true);

    std::ostringstream out;
    out << itemName << " unequipped";
    botAI->TellMaster(out);
}
