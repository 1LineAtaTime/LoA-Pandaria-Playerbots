/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanActions.h"

#include "AiFactory.h"
#include "Item.h"
#include "Playerbots.h"
#include "Totem.h"

// Wave-74 fix (#858), see the class comment in ShamanActions.h.
std::string const CastUnleashElementsAction::GetTargetName()
{
    return AiFactory::GetPlayerSpecTab(bot) == Specializations::SPEC_SHAMAN_RESTORATION ? "self target" : "current target";
}

bool CastTotemAction::isUseful()
{
    if (needLifeTime > 0.1f && AI_VALUE(uint8, "attacker count") < 3)
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target)
        {
            return false;
        }
        float dps = AI_VALUE(float, "estimated group dps");
        if (target->GetHealth() / dps < needLifeTime)
        {
            return false;
        }
    }
    return CastBuffSpellAction::isUseful() && !AI_VALUE2(bool, "has totem", name) && !botAI->HasAura(buff, bot);
}

bool CastManaSpringTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "healing stream totem");
}

bool CastFlametongueTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "magma totem") &&
           !botAI->HasAura("totem of wrath", bot);
}

bool CastSearingTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "flametongue totem");
}

bool CastMagmaTotemAction::isUseful() {
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !bot->IsWithinMeleeRange(target))
        return false;

    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", name); 
}

bool CastFireNovaAction::isUseful() {
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;
    Creature* fireTotem = bot->GetMap()->GetCreature(bot->m_SummonSlot[1]);
    if (!fireTotem)
        return false;
    
    if (target->GetDistance(fireTotem) > 8.0f)
        return false;
    
    return CastMeleeSpellAction::isUseful(); 
}

bool CastCleansingTotemAction::isUseful()
{
    return CastTotemAction::isUseful() && !AI_VALUE2(bool, "has totem", "mana tide totem");
}

// #382 (AC GenericSpellActions.cpp:327-364): per-hand weapon imbues. AC's isPossible() rejects a missing item, a
// miscellaneous weapon, a fishing pole (main hand only in AC) and an item that already carries a temporary enchant,
// then asks CanCastSpell with the item as the target. 5.4.8 spells the constant ITEM_SUBCLASS_WEAPON_MISCELLANEOUS
// (same value as AC's ITEM_SUBCLASS_WEAPON_MISC, see actions/SellAction.cpp:106-108).
bool CastShamanEnchantMainHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastShamanEnchantMainHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISCELLANEOUS ||
        item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

bool CastShamanEnchantOffHandAction::Execute(Event /*event*/)
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    return item && botAI->CastSpell(spell, bot, item);
}

bool CastShamanEnchantOffHandAction::isPossible()
{
    Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);
    if (!item || item->GetTemplate()->SubClass == ITEM_SUBCLASS_WEAPON_MISCELLANEOUS ||
        item->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
    {
        return false;
    }

    return botAI->CanCastSpell(spell, bot, item);
}

// #408 (AC parity, AC ShamanActions.cpp:76-91): Spirit Walk for guardian spirit wolves.
bool CastSpiritWalkAction::Execute(Event /*event*/)
{
    constexpr uint32 SPIRIT_WOLF = 29264;
    constexpr uint32 SPIRIT_WALK_SPELL = 58875;

    for (Unit* unit : bot->m_Controlled)
    {
        if (unit->GetEntry() == SPIRIT_WOLF && unit->HasSpell(SPIRIT_WALK_SPELL))
        {
            unit->CastSpell(unit, SPIRIT_WALK_SPELL, false);
            return true;
        }
    }

    return false;
}

// #431 (AC parity, AC ShamanActions.cpp:94-117 SetTotemAction::Execute): see the class comment in ShamanActions.h
// for why the totem is cast instead of written to the (non-existent on MoP) totem bar.
bool SetTotemAction::isUseful()
{
    return botAI->CanCastSpell(totemName, bot);
}

bool SetTotemAction::Execute(Event /*event*/)
{
    return botAI->CastSpell(totemName, bot);
}
