/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/OpenItemAction.cpp (issue #535).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - PlayerbotAI::FindOpenableItem (AC Bot/PlayerbotAI.cpp:5703-5711) and FindItemInInventory
 *    (AC :5612-5644) do not exist here and PlayerbotAI.h must not be edited, so they are the
 *    file-local helpers below. The predicate is handed the Item itself instead of AC's
 *    bot->GetItemByEntry(entry) lookup (which could test a different stack of the same entry).
 *  - CMSG_OPEN_ITEM on 5.4.8 is the plain two-byte "bagIndex, slot" read
 *    (WorldSession::HandleOpenItemOpcode, SpellHandler.cpp:500-520), so AC's packet is valid unchanged.
 *  - ITEM_FLAG_HAS_LOOT is ItemTemplate::Flags & ITEM_FLAG_HAS_LOOT (ItemPrototype.h:106) here.
 */

#include "OpenItemAction.h"

#include "Bag.h"
#include "Item.h"
#include "LootObjectStack.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
// AC Bot/PlayerbotAI.cpp:5612-5644 PlayerbotAI::FindItemInInventory (Item-based predicate, see header).
template <typename Pred>
Item* OpenFindItemInInventory(Player* bot, Pred check)
{
    for (uint8 slot = INVENTORY_SLOT_ITEM_START; slot < INVENTORY_SLOT_ITEM_END; ++slot)
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
        {
            ItemTemplate const* proto = pItem->GetTemplate();
            if (proto && bot->CanUseItem(proto) == EQUIP_ERR_OK && check(pItem, proto))
                return pItem;
        }
    }

    for (uint8 bag = INVENTORY_SLOT_BAG_START; bag < INVENTORY_SLOT_BAG_END; ++bag)
    {
        if (Bag const* const pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, bag))
        {
            for (uint8 slot = 0; slot < pBag->GetBagSize(); ++slot)
            {
                if (Item* const pItem = bot->GetItemByPos(bag, slot))
                {
                    ItemTemplate const* proto = pItem->GetTemplate();
                    if (proto && bot->CanUseItem(proto) == EQUIP_ERR_OK && check(pItem, proto))
                        return pItem;
                }
            }
        }
    }

    return nullptr;
}

// AC Bot/PlayerbotAI.cpp:5703-5711 PlayerbotAI::FindOpenableItem.
Item* OpenFindOpenableItem(Player* bot)
{
    return OpenFindItemInInventory(bot,
        [](Item* item, ItemTemplate const* proto) -> bool
        { return (proto->Flags & ITEM_FLAG_HAS_LOOT) && (proto->LockID == 0 || !item->IsLocked()); });
}
}  // namespace

// AC OpenItemAction.cpp:16-30.
bool OpenItemAction::Execute(Event /*event*/)
{
    bool foundOpenable = false;

    Item* item = OpenFindOpenableItem(bot);
    if (item)
    {
        uint8 bag = item->GetBagSlot();  // Retrieves the bag slot (0 for main inventory)
        uint8 slot = item->GetSlot();    // Retrieves the actual slot inside the bag

        OpenItem(item, bag, slot);
        foundOpenable = true;
    }

    return foundOpenable;
}

// AC OpenItemAction.cpp:32-47.
void OpenItemAction::OpenItem(Item* item, uint8 bag, uint8 slot)
{
    // #535 proof row (beyond AC, log only): AC only tells the master, and a random bot has none.
    TC_LOG_DEBUG("playerbots", "[OPENITEM] %s opens item=%u bag=%u slot=%u", bot->GetName().c_str(), item->GetEntry(),
                 uint32(bag), uint32(slot));

    WorldPacket packet(CMSG_OPEN_ITEM);
    packet << bag << slot;
    bot->GetSession()->HandleOpenItemOpcode(packet);

    // Store the item GUID as the loot target
    LootObject lootObject;
    lootObject.guid = item->GetGUID();
    botAI->GetAiObjectContext()->GetValue<LootObject>("loot target")->Set(lootObject);

    std::ostringstream out;
    out << "Opened item: " << item->GetTemplate()->Name1;
    botAI->TellMaster(out.str());
}
