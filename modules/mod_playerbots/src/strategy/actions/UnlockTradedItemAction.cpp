/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UnlockTradedItemAction.cpp for issue #193 (S7 of epic
 * #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - bot->getClass() -> bot->GetClass() (this fork's Player capitalizes the accessor, see the S6
 *    "Known MoP API differences" notes e.g. strategy/actions/EquipAction.cpp).
 *  - 5.4.8: the Lockpicking skill (633) was removed from the game in Patch 4.0.1; SkillLine.dbc has no
 *    row 633 at all on this client, so bot->HasSkill(SKILL_LOCKPICKING) and
 *    bot->GetSkillValue(SKILL_LOCKPICKING) are always false/0 for every rogue. Retail's replacement
 *    rule (still what this core's own Spell::CanOpenLock, Spell.cpp:8702-8710, computes for the
 *    rogue-family Pick Lock spell) is: lock skill = GetSkillValue(633) + GetLevel()*5.
 *    CanUnlockItem() below therefore drops the HasSkill() gate and UnlockItem()'s skill read adds
 *    GetLevel()*5.
 *  - Everything else (TradeData::GetItem, Item::IsLocked/GetTemplate, LockEntry/sLockStore,
 *    SkillByLockType, botAI->CastSpell(uint32, Unit*, Item*)) matches this core's API exactly.
 */

#include "UnlockTradedItemAction.h"

#include "Playerbots.h"  // 5.4.8: Player/Item/DBC stores are not pulled in by PlayerbotAI.h here
#include "DBCStores.h"
#include "Item.h"
#include "SpellInfo.h"
#include "TradeData.h"

inline constexpr uint32_t PICK_LOCK_SPELL_ID = 1804;

bool UnlockTradedItemAction::Execute(Event /*event*/)
{
    Player* trader = bot->GetTrader();
    if (!trader)
        return false; // No active trade session

    TradeData* tradeData = trader->GetTradeData();
    if (!tradeData)
        return false; // No trade data available

    Item* lockbox = tradeData->GetItem(TRADE_SLOT_NONTRADED);
    if (!lockbox)
    {
        botAI->TellError("No item in the Do Not Trade slot.");
        return false;
    }

    if (!CanUnlockItem(lockbox))
    {
        botAI->TellError("Cannot unlock this item.");
        return false;
    }

    UnlockItem(lockbox);
    return true;
}

bool UnlockTradedItemAction::CanUnlockItem(Item* item)
{
    if (!item)
        return false;

    ItemTemplate const* itemTemplate = item->GetTemplate();
    if (!itemTemplate)
        return false;

    // Ensure the bot is a rogue. 5.4.8: no HasSkill(SKILL_LOCKPICKING) gate - see the file comment.
    if (bot->GetClass() != CLASS_ROGUE)
        return false;

    // Ensure the item is actually locked.
    if (itemTemplate->LockID == 0 || !item->IsLocked())
        return false;

    // Check if the bot's Lockpicking skill is high enough.
    uint32 lockId = itemTemplate->LockID;
    LockEntry const* lockInfo = sLockStore.LookupEntry(lockId);
    if (!lockInfo)
        return false;

    // 5.4.8: SKILL_LOCKPICKING no longer exists as a real skill; GetSkillValue() is 0 for it, and the
    // level-scaled bonus (the core's own Pick Lock rule, Spell.cpp:8702-8710) is added by hand.
    uint32 botSkill = bot->GetSkillValue(SKILL_LOCKPICKING) + bot->GetLevel() * 5;
    for (uint8 j = 0; j < 8; ++j)
    {
        if (lockInfo->Type[j] == LOCK_KEY_SKILL && SkillByLockType(LockType(lockInfo->Index[j])) == SKILL_LOCKPICKING)
        {
            uint32 requiredSkill = lockInfo->Skill[j];
            if (botSkill >= requiredSkill)
                return true;
            else
            {
                std::ostringstream out;
                out << "Lockpicking skill too low (" << botSkill << "/" << requiredSkill << ") to unlock: "
                    << item->GetTemplate()->Name1;
                botAI->TellMaster(out.str());
            }
        }
    }

    return false;
}

void UnlockTradedItemAction::UnlockItem(Item* item)
{
    if (!bot->HasSpell(PICK_LOCK_SPELL_ID))
    {
        botAI->TellError("Cannot unlock, Pick Lock spell is missing.");
        return;
    }

    // Use CastSpell to unlock the item.
    if (botAI->CastSpell(PICK_LOCK_SPELL_ID, bot->GetTrader(), item)) // Unit target is trader
    {
        std::ostringstream out;
        out << "Picking Lock on traded item: " << item->GetTemplate()->Name1;
        botAI->TellMaster(out.str());
    }
    else
    {
        botAI->TellError("Failed to cast Pick Lock.");
    }
}
