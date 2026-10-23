/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MageTriggers.h"

// 5.4.8 include-order: DynamicObject.h derives from GridObject<DynamicObject> but only includes
// Object.h, and Map.h does NOT pull GridObject.h either - GridObject.h is reached only via Creature.h
// (Creature.h:28) and GameObject.h. So DynamicObject.h does not compile unless one of those came first
// (C7568/C3770 "argument list missing after assumed function template 'GridObject'"). This matches the
// core's own precedent, e.g. ObjectAccessor.cpp includes Creature.h before DynamicObject.h.
#include "Creature.h"
#include "DynamicObject.h"
#include "Group.h"
#include "MageActions.h"
#include "Player.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "Spell.h"
#include "SpellAuraEffects.h"
#include "Value.h"

bool ArcaneBrillianceTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool RuneOfPowerTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("rune of power", GetTarget());
}

bool MageArmorTrigger::IsActive()
{
    return 
        (botAI->GetBot()->GetSpecialization() != Specializations::SPEC_MAGE_FIRE && bot->GetGroup() ||
        botAI->GetBot()->GetSpecialization() == Specializations::SPEC_MAGE_ARCANE) &&
        !botAI->HasAura("mage armor", bot);
}

bool FrostArmorTrigger::IsActive()
{
    return (botAI->GetBot()->GetSpecialization() == Specializations::SPEC_MAGE_FROST && !botAI->HasAura("frost armor", bot) && !bot->GetGroup());
}

bool MoltenArmorTrigger::IsActive()
{
    return (botAI->GetBot()->GetSpecialization() == Specializations::SPEC_MAGE_FIRE && !botAI->HasAura("molten armor", bot));
}

bool FrostNovaOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return botAI->HasAura(spell, target);
}

bool IcyVeinsTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return !botAI->HasAura("icy veins", target);
}

bool FingersOfFrostSingleTrigger::IsActive()
{
    // AC: "Fingers of Frost "stack" count is always 1. The value is instead stored in the charges." (WotLK 44544.)
    // #790: this 5.4.8 core keeps Fingers of Frost 44544 ("Limit 2 charges", Wowhead mop-classic tooltip 44544,
    // 2026-10-12) as a STACK count - spell_mage.cpp spell_mage_ice_lance / spell_mage_deep_freez consume it with
    // ModStackAmount(-1) - so GetCharges() was always 0 and this "last charge" edge never fired (wave 35: no
    // "T:fingers of frost single" at all). Same test on the stack amount.
    Aura* aura = botAI->GetAura("fingers of frost", bot, false, true, -1);
    return (aura && aura->GetStackAmount() == 1);
}

bool SlowNotOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
    {
        return false;
    }
    return !botAI->HasAura(spell, target);
}

bool HeatingUpTrigger::IsActive()
{
    Aura* aura = botAI->GetAura("heating up", bot, false, false, -1);
    return aura != nullptr;
}
// ---------------------------------------------------------------------------
// AC WotLK parity (#357 slice 8) - ported 1:1 from
// modules/mod-playerbots/src/Ai/Class/Mage/MageTriggers.cpp in the AC WotLK tree.
// ---------------------------------------------------------------------------

bool NoManaGemTrigger::IsActive()
{
    // #385: AC's six WotLK gem ids (33312/22044/8008/8007/5513/5514) do not exist as conjured items in
    // 5.4.8; Conjure Mana Gem (759) makes the single Mana Gem 36799 (Wowhead mop-classic, 2026-10-04).
    static const std::vector<uint32> gemIds = {
        36799   // Mana Gem
    };

    for (uint32 gemId : gemIds)
    {
        if (bot->GetItemCount(gemId, false) > 0)  // false = only in bags
            return false;
    }
    return true;
}

bool ArcaneIntellectTrigger::IsActive()
{
    return BuffTrigger::IsActive() && !botAI->HasAura("arcane brilliance", GetTarget());
}

bool NoFocusMagicTrigger::IsActive()
{
    constexpr uint32 SPELL_FOCUS_MAGIC = 54646;
    if (!bot->HasSpell(SPELL_FOCUS_MAGIC))
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive())
            continue;

        if (member->HasAura(SPELL_FOCUS_MAGIC, bot->GetGUID()))
            return false;
    }
    return true;
}

bool DeepFreezeCooldownTrigger::IsActive()
{
    constexpr uint32 SPELL_DEEP_FREEZE = 44572;
    return !bot->HasSpell(SPELL_DEEP_FREEZE) || SpellCooldownTrigger::IsActive();
}

const std::unordered_set<uint32> FlamestrikeNearbyTrigger::FLAMESTRIKE_SPELL_IDS = {
    2120, 2121, 8422, 8423, 10215, 10216, 27086, 42925, 42926
};

bool FlamestrikeNearbyTrigger::IsActive()
{
    for (uint32 spellId : FLAMESTRIKE_SPELL_IDS)
    {
        Aura* aura = bot->GetAura(spellId, bot->GetGUID());
        if (!aura)
            continue;

        DynamicObject* dynObj = aura->GetDynobjOwner();
        if (!dynObj)
            continue;

        float dist = bot->GetDistance2d(dynObj->GetPositionX(), dynObj->GetPositionY());
        if (dist <= radius)
            return true;
    }
    return false;
}

bool ImprovedScorchTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    static const uint32 ImprovedScorchExclusiveDebuffs[] = {// Shadow Mastery
                                                            17794, 17797, 17798, 17799, 17800,
                                                            // Winter's Chill
                                                            12579,
                                                            // Improved Scorch
                                                            22959};

    for (uint32 spellId : ImprovedScorchExclusiveDebuffs)
    {
        if (target->HasAura(spellId))
            return false;
    }

    return DebuffTrigger::IsActive();
}

const std::unordered_set<uint32> BlizzardChannelCheckTrigger::BLIZZARD_SPELL_IDS = {
    10,     // Blizzard Rank 1
    6141,   // Blizzard Rank 2
    8427,   // Blizzard Rank 3
    10185,  // Blizzard Rank 4
    10186,  // Blizzard Rank 5
    10187,  // Blizzard Rank 6
    27085,  // Blizzard Rank 7
    42938,  // Blizzard Rank 8
    42939   // Blizzard Rank 9
};

bool BlizzardChannelCheckTrigger::IsActive()
{
    if (Spell* spell = bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL); spell && BLIZZARD_SPELL_IDS.count(spell->m_spellInfo->Id))
    {
        uint8 attackerCount = AI_VALUE(uint8, "attacker count");
        return attackerCount < minEnemies;
    }

    return false;
}

bool FrostbiteOnTargetTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !target->IsAlive() || !target->IsInWorld())
        return false;

    return botAI->HasAura(spell, target);
}

bool NoFirestarterStrategyTrigger::IsActive()
{
    return !botAI->HasStrategy("firestarter", BOT_STATE_COMBAT);
}

// Rotation lane 10 (#865): PvP-gated triggers, see MageTriggers.h.
bool MagePvpEnemyPlayerInMeleeRangeTrigger::IsActive()
{
    if (!AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("enemy player in melee range");
    return t && t->IsActive();
}

bool MagePvpEnemyCasterInRangeTrigger::IsActive()
{
    if (!AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("enemy caster in range");
    return t && t->IsActive();
}

bool MageIceBlockHealthTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return bot->GetGroup() && bot->GetHealthPct() <= 10.0f;  // MoP rule: <= 10 % in a group

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("critical health");  // AC GenericMageStrategy.cpp:100
    return t && t->IsActive();
}

// Rotation fix lane H (#868): see MageTriggers.h.
bool MageMediumThreatTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("medium threat");
    return t && t->IsActive();
}

bool MageFleeCloseTrigger::IsActive()
{
    if (AI_VALUE(bool, "in pvp instance"))
        return false;

    Trigger* t = botAI->GetAiObjectContext()->GetTrigger("enemy too close for spell");
    return t && t->IsActive();
}
