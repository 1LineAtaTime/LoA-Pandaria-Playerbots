/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UnlockItemAction.cpp (issue #535).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - PlayerbotAI::FindLockedItem (AC Bot/PlayerbotAI.cpp:5713-5748) is a file-local helper here
 *    (PlayerbotAI.h must not be edited); inventory walk = AC FindItemInInventory :5612-5644.
 *  - The Lockpicking skill (633) was removed in Cataclysm, so AC's bot->HasSkill(SKILL_LOCKPICKING)
 *    gate and GetSkillValue(SKILL_LOCKPICKING) are always false/0 here. Same adaptation as
 *    UnlockTradedItemAction.cpp (#193): the HasSkill gate becomes "is a rogue" and the skill is
 *    GetSkillValue(SKILL_LOCKPICKING) + level * 5 (the core's own Pick Lock rule,
 *    Spell::CanOpenLock, Spell.cpp:8702-8710).
 *  - Everything else (LockEntry/sLockStore/LOCK_KEY_SKILL/SkillByLockType, CastSpell(uint32, Unit*, Item*))
 *    matches this core.
 */

#include "UnlockItemAction.h"

#include "Bag.h"
#include "DBCStores.h"
#include "Item.h"
#include "Playerbots.h"

namespace
{
constexpr uint32 UNLOCK_ITEM_PICK_LOCK_SPELL_ID = 1804;  // AC UnlockItemAction.cpp:15

template <typename Pred>
Item* UnlockFindItemInInventory(Player* bot, Pred check)
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

// AC Bot/PlayerbotAI.cpp:5713-5748 PlayerbotAI::FindLockedItem.
Item* UnlockFindLockedItem(Player* bot)
{
    // 5.4.8: stands in for AC's HasSkill(SKILL_LOCKPICKING) gate (see file comment).
    if (bot->GetClass() != CLASS_ROGUE)
        return nullptr;

    return UnlockFindItemInInventory(bot,
        [bot](Item* item, ItemTemplate const* proto) -> bool
        {
            if (proto->LockID == 0)  // Ensure the item is actually locked
                return false;

            if (!item->IsLocked())  // Ensure item instance is locked
                return false;

            // Check if bot has enough Lockpicking skill
            LockEntry const* lockInfo = sLockStore.LookupEntry(proto->LockID);
            if (!lockInfo)
                return false;

            for (uint8 j = 0; j < 8; ++j)
            {
                if (lockInfo->Type[j] == LOCK_KEY_SKILL)
                {
                    uint32 skillId = SkillByLockType(LockType(lockInfo->Index[j]));
                    if (skillId == SKILL_LOCKPICKING)
                    {
                        uint32 requiredSkill = lockInfo->Skill[j];
                        uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING) + bot->GetLevel() * 5;
                        return botSkill >= requiredSkill;
                    }
                }
            }

            return false;
        });
}
}  // namespace

// AC UnlockItemAction.cpp:17-28.
bool UnlockItemAction::Execute(Event /*event*/)
{
    bool foundLockedItem = false;

    Item* item = UnlockFindLockedItem(bot);
    if (item)
    {
        UnlockItem(item);
        foundLockedItem = true;
    }

    return foundLockedItem;
}

// AC UnlockItemAction.cpp:30-41.
void UnlockItemAction::UnlockItem(Item* item)
{
    // Use CastSpell to unlock the item
    if (botAI->CastSpell(UNLOCK_ITEM_PICK_LOCK_SPELL_ID, bot, item))
    {
        // #535 proof row (beyond AC, log only): AC only tells the master, and a random bot has none.
        TC_LOG_DEBUG("playerbots", "[UNLOCKITEM] %s picks the lock of item=%u", bot->GetName().c_str(), item->GetEntry());
        std::ostringstream out;
        out << "Used Pick Lock on: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
        botAI->TellError("Failed to cast Pick Lock.");
}
