/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Mgr/Item/StatsCollector.cpp for issue #188 (S6 of epic #163).
 * 5.4.8 adaptations vs AC (WotLK):
 *  - proto->Block does not exist on 5.4.8 ItemTemplate (block value on this expansion comes only from
 *    the ITEM_MOD_BLOCK_VALUE item stat, handled below in CollectByItemStatType) - dropped.
 *  - proto->getFeralBonus()/Feral AP item field does not exist on 5.4.8 (Cataclysm removed the
 *    separate "feral attack power" item field; feral druids scale attack power from Agility only) -
 *    dropped.
 *  - proto->StatsCount does not exist; 5.4.8 ItemTemplate always has a fixed ItemStat[MAX_ITEM_PROTO_STATS]
 *    array with unused slots at ItemStatValue == 0, so we iterate the full array and skip zero values
 *    (the pattern used elsewhere in this core, e.g. ObjectMgr.cpp, Item.cpp).
 *  - proto->IsWeapon() does not exist on 5.4.8 ItemTemplate; replaced with Class == ITEM_CLASS_WEAPON.
 *  - AC's SpellProcEntry (WotLK-era) has uppercase fields (ProcFlags/Chance/Cooldown as Milliseconds/
 *    SpellFamilyName/SpellFamilyMask as flag96). This core's SpellProcEntry was rewritten with
 *    lowercase fields and different semantics (typeMask overrides ProcFlags, cooldown is uint32
 *    seconds, spellFamilyMask is flag128) - CollectSpellStats/CanBeTriggeredByType are adapted below.
 *  - MAX_COMBAT_RATING was renamed CR_MAX_COMBAT_RATING.
 *  - flag96 -> flag128 (this core added a 4th dword to spell class masks).
 *  - Added CR_MASTERY / CR_PVP_POWER handling in HandleApplyAura's SPELL_AURA_MOD_RATING loop and
 *    ITEM_MOD_MASTERY_RATING / ITEM_MOD_PVP_POWER in CollectByItemStatType (Cataclysm/MoP-only
 *    itemization, see StatsCollector.h).
 *  - SPELL_AURA_MOD_INCREASES_SPELL_PCT_TO_HIT (old spell-hit suppression debuff) was retired in
 *    4.3.4 (this core renumbers it SPELL_AURA_199, unused) - that term is dropped from overflow calc
 *    in StatsWeightCalculator, not here.
 *  - Class2SpellFamilyName()/IsHealingSpell() are local static ports of AC's PlayerbotAI statics of
 *    the same name (PlayerbotAI.h is off-limits for this issue); added a CLASS_MONK case.
 */

#include "StatsCollector.h"

#include "DBCStores.h"
#include "Player.h"  // 5.4.8: ItemTemplate lives in ItemPrototype.h, which is not self-contained
#include "SpellAuraDefines.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Util.h"

StatsCollector::StatsCollector(CollectorType type, int32 cls) : type_(type), cls_(cls) { Reset(); }

void StatsCollector::Reset()
{
    for (uint32 i = 0; i < STATS_TYPE_MAX; i++)
    {
        stats[i] = 0;
    }
}

void StatsCollector::CollectItemStats(ItemTemplate const* proto)
{
    if (proto->IsRangedWeapon())
    {
        float val = (proto->DamageMin + proto->DamageMax) * 1000 / 2 / proto->Delay;
        stats[STATS_TYPE_RANGED_DPS] += val;
    }
    else if (proto->Class == ITEM_CLASS_WEAPON)
    {
        float val = (proto->DamageMin + proto->DamageMax) * 1000 / 2 / proto->Delay;
        stats[STATS_TYPE_MELEE_DPS] += val;
        // 5.4.8: no separate "feral attack power" item field on this expansion (dropped, see file
        // header) - feral druids are scored by Agility/Attack Power like any other agility melee.
    }
    stats[STATS_TYPE_ARMOR] += proto->Armor;
    // 5.4.8: proto->Block does not exist (dropped, see file header); block value comes only from the
    // ITEM_MOD_BLOCK_VALUE item stat below.
    for (uint32 i = 0; i < MAX_ITEM_PROTO_STATS; i++)
    {
        _ItemStat const& stat = proto->ItemStat[i];
        if (!stat.ItemStatValue)
            continue;
        CollectByItemStatType(stat.ItemStatType, stat.ItemStatValue);
    }
    for (uint8 j = 0; j < MAX_ITEM_PROTO_SPELLS; j++)
    {
        switch (proto->Spells[j].SpellTrigger)
        {
            case ITEM_SPELLTRIGGER_ON_USE:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(proto->Spells[j].SpellCooldown));
                break;
            case ITEM_SPELLTRIGGER_ON_EQUIP:
                CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(0));
                break;
            case ITEM_SPELLTRIGGER_CHANCE_ON_HIT:
                if (type_ & CollectorType::MELEE)
                {
                    if (proto->SpellPPMRate > 0.01f)
                        CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(static_cast<int>(60000 / proto->SpellPPMRate)));
                    else
                        CollectSpellStats(proto->Spells[j].SpellId, 1.0f, Milliseconds(static_cast<int>(60000 / 1.8f)));  // Default PPM = 1.8
                }
                break;
            default:
                break;
        }
    }

    if (proto->socketBonus)
    {
        if (SpellItemEnchantmentEntry const* enchant = sSpellItemEnchantmentStore.LookupEntry(proto->socketBonus))
            CollectEnchantStats(enchant);
    }
}

void StatsCollector::CollectSpellStats(uint32 spellId, float multiplier, Milliseconds spellCooldown)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);

    if (!spellInfo)
        return;

    if (SpecialSpellFilter(spellId))
        return;

    // Form-restricted item auras (e.g., feral attack power, idol boosts) should be considered by Druids only.
    if (spellInfo->Stances && cls_ != CLASS_DRUID)
        return;

    SpellProcEntry const* eventEntry = sSpellMgr->GetSpellProcEntry(spellInfo->Id);

    // 5.4.8: this core's SpellProcEntry::cooldown is uint32 seconds, not AC's Milliseconds field.
    Milliseconds triggerCooldown = (eventEntry && eventEntry->cooldown) ? Milliseconds(eventEntry->cooldown * 1000) : 0ms;

    bool canNextTrigger = true;

    uint32 procFlags;
    uint32 procChance;
    // 5.4.8: SpellProcEntry::typeMask overrides SpellInfo::ProcFlags (AC's field was named ProcFlags).
    if (eventEntry && eventEntry->typeMask)
        procFlags = eventEntry->typeMask;
    else
        procFlags = spellInfo->ProcFlags;

    // 5.4.8: SpellProcEntry::chance (lowercase) overrides SpellInfo::ProcChance (AC's field was Chance).
    if (eventEntry && eventEntry->chance)
        procChance = uint32(eventEntry->chance);
    else
        procChance = spellInfo->ProcChance;
    bool lowChance = procChance <= 5;

    if (lowChance || (procFlags && !CanBeTriggeredByType(spellInfo, procFlags)))
        canNextTrigger = false;

    if (spellInfo->StackAmount)
    {
        // Heuristic multiplier for spell with stackAmount since high stackAmount may not be available
        if (spellInfo->StackAmount <= 1)
            multiplier *= spellInfo->StackAmount * 1;
        else if (spellInfo->StackAmount <= 5)
            multiplier *= 1 + (spellInfo->StackAmount - 1) * 0.75;
        else if (spellInfo->StackAmount <= 10)
            multiplier *= 4 + (spellInfo->StackAmount - 5) * 0.6;
        else if (spellInfo->StackAmount <= 20)
            multiplier *= 7 + (spellInfo->StackAmount - 10) * 0.4;
        else
            multiplier *= 11;
    }

    for (int i = 0; i < MAX_SPELL_EFFECTS; i++)
    {
        SpellEffectInfo const& effectInfo = spellInfo->Effects[i];
        if (!effectInfo.Effect)
            continue;
        switch (effectInfo.Effect)
        {
            case SPELL_EFFECT_APPLY_AURA:
            {
                if (spellInfo->SpellFamilyName && /*effectInfo.ApplyAuraName != SPELL_AURA_DUMMY &&*/
                    effectInfo.ApplyAuraName != SPELL_AURA_PROC_TRIGGER_SPELL)
                {
                    if (!CheckSpellValidation(spellInfo->SpellFamilyName, effectInfo.SpellClassMask))
                        return;

                    // Some dummy effects cannot be recognized, make some bonus to identify
                    stats[STATS_TYPE_BONUS] += 1;
                }

                /// @todo Handle negative spell
                if (!spellInfo->IsPositive())
                    break;

                float coverage;
                if (spellCooldown.count() <= 2000 || spellInfo->GetDuration() == -1)
                    coverage = 1.0f;
                else
                    coverage =
                        std::min(1.0f, (float)spellInfo->GetDuration() / (spellInfo->GetDuration() + spellCooldown.count()));

                multiplier *= coverage;
                HandleApplyAura(effectInfo, multiplier, canNextTrigger, triggerCooldown);
                break;
            }
            case SPELL_EFFECT_HEAL:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                float transfer_multiplier = 1;
                stats[STATS_TYPE_HEAL_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_ENERGIZE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                if (effectInfo.MiscValue != POWER_MANA)
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                float transfer_multiplier = 0.2;
                stats[STATS_TYPE_MANA_REGENERATION] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                break;
            }
            case SPELL_EFFECT_SCHOOL_DAMAGE:
            {
                /// @todo Handle spell without cooldown
                if (!spellCooldown.count())
                    break;
                float normalizedCd = std::max((float)spellCooldown.count() / 1000, 5.0f);
                int32 val = AverageValue(effectInfo);
                if (type_ & (CollectorType::MELEE | CollectorType::RANGED))
                {
                    float transfer_multiplier = 1;
                    stats[STATS_TYPE_ATTACK_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                }
                else if (type_ & CollectorType::SPELL_DMG)
                {
                    float transfer_multiplier = 0.5;
                    stats[STATS_TYPE_SPELL_POWER] += (float)val / normalizedCd * multiplier * transfer_multiplier;
                }
                break;
            }
            default:
                break;
        }
    }
}

void StatsCollector::CollectEnchantStats(SpellItemEnchantmentEntry const* enchant, uint32 default_enchant_amount)
{
    for (int s = 0; s < MAX_ITEM_ENCHANTMENT_EFFECTS; ++s)
    {
        uint32 enchant_display_type = enchant->Type[s];
        uint32 enchant_amount = enchant->Amount[s];
        uint32 enchant_spell_id = enchant->SpellID[s];

        if (SpecialEnchantFilter(enchant_spell_id))
            continue;

        switch (enchant_display_type)
        {
            case ITEM_ENCHANTMENT_TYPE_COMBAT_SPELL:
            {
                if (type_ & CollectorType::MELEE)
                    CollectSpellStats(enchant_spell_id, 0.25f);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_EQUIP_SPELL:
            {
                CollectSpellStats(enchant_spell_id, 1.0f);
                break;
            }
            case ITEM_ENCHANTMENT_TYPE_STAT:
            {
                // for item random suffix
                if (!enchant_amount)
                    enchant_amount = default_enchant_amount;

                if (!enchant_amount)
                {
                    break;
                }
                CollectByItemStatType(enchant_spell_id, enchant_amount);
                break;
            }
            default:
                break;
        }
    }
}

/// @todo Special case for some spell that hard to calculate, like trinket, relic, etc.
bool StatsCollector::SpecialSpellFilter(uint32 spellId)
{
    // trinket
    switch (spellId)
    {
        case 60764:  // Totem of Splintering
            if (type_ & (CollectorType::SPELL))
                return true;
            break;
        case 27521:  // Insightful Earthstorm Diamond
            stats[STATS_TYPE_MANA_REGENERATION] += 20;
            return true;
        case 55381:  // Insightful Earthsiege Diamond
            stats[STATS_TYPE_MANA_REGENERATION] += 40;
            return true;
        case 39442:  // Darkmoon Card: Wrath
            if (!(type_ & CollectorType::SPELL_HEAL))
                stats[STATS_TYPE_CRIT] += 50;
            return true;
        case 59620:  // Berserk
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_ATTACK_POWER] += 120;
            return true;
        case 67702:  // Death's Verdict
            stats[STATS_TYPE_ATTACK_POWER] += 225;
            return true;
        case 67771:  // Death's Verdict (heroic)
            stats[STATS_TYPE_ATTACK_POWER] += 260;
            return true;
        case 71406:  // Tiny Abomination in a Jar
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 600;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 150;
            return true;
        case 71545:  // Tiny Abomination in a Jar (heroic)
            if (cls_ == CLASS_PALADIN)
                stats[STATS_TYPE_ATTACK_POWER] += 800;
            else
                stats[STATS_TYPE_ATTACK_POWER] += 200;
            return true;
        case 71519:  // Deathbringer's Will
            stats[STATS_TYPE_ATTACK_POWER] += 350;
            return true;
        case 71562:  // Deathbringer's Will (heroic)
            stats[STATS_TYPE_ATTACK_POWER] += 400;
            return true;
        case 71602:  // Dislodged Foreign Object
            /// @todo The item can be triggered by heal spell, which mismatch with it's description
            /// Noticing that heroic item can not be triggered, probably a bug to report to AC
            if (type_ & CollectorType::SPELL_HEAL)
                return true;
            break;
        case 71903:  // Shadowmourne
            stats[STATS_TYPE_STRENGTH] += 200;
            return true;
        default:
            break;
    }
    // These are all WotLK-tier trinkets/weapons; kept verbatim from AC for diff parity even though
    // none of them exist on 5.4.8 (harmless dead branches - the spellId simply never matches).
    return false;
}

bool StatsCollector::SpecialEnchantFilter(uint32 enchantSpellId)
{
    switch (enchantSpellId)
    {
        case 64440:
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_PARRY] += 50;
            }
            return true;
        case 53365:  // Rune of the Fallen Crusader
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_STRENGTH] += 75;
            }
            return true;
        case 62157:  // Rune of the Stoneskin Gargoyle
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_DEFENSE] += 25;
                stats[STATS_TYPE_STAMINA] += 50;
            }
            return true;
        case 64571:  // Blood draining
            if (type_ & CollectorType::MELEE)
            {
                stats[STATS_TYPE_STAMINA] += 50;
            }
            return true;
        default:
            break;
    }
    return false;
}

bool StatsCollector::CanBeTriggeredByType(SpellInfo const* spellInfo, uint32 procFlags, bool strict)
{
    SpellProcEntry const* eventEntry = sSpellMgr->GetSpellProcEntry(spellInfo->Id);
    uint32 spellFamilyName = 0;
    if (eventEntry)
    {
        // 5.4.8: lowercase spellFamilyName/spellFamilyMask (flag128), see file header.
        spellFamilyName = eventEntry->spellFamilyName;
        flag128 spellFamilyMask = eventEntry->spellFamilyMask;
        if (spellFamilyName != 0)
        {
            if (!CheckSpellValidation(spellFamilyName, spellFamilyMask, strict))
                return false;
        }
    }

    uint32 triggerMask = TAKEN_HIT_PROC_FLAG_MASK;  // Generic trigger mask
    switch (type_)
    {
        case CollectorType::MELEE_DMG:
        {
            triggerMask |= MELEE_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::MELEE_TANK:
        {
            triggerMask |= MELEE_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PERIODIC_PROC_FLAG_MASK;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::RANGED:
        {
            triggerMask |= RANGED_PROC_FLAG_MASK;
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::SPELL_DMG:
        {
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            // Healing spell cannot trigger
            triggerMask &= ~PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_POS;
            triggerMask &= ~PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_POS;
            if (procFlags & triggerMask)
                return true;
            break;
        }
        case CollectorType::SPELL_HEAL:
        {
            triggerMask |= SPELL_PROC_FLAG_MASK;
            triggerMask |= PROC_FLAG_DONE_PERIODIC;
            // Dmg spell should not trigger
            triggerMask &= ~PROC_FLAG_DONE_SPELL_NONE_DMG_CLASS_NEG;
            triggerMask &= ~PROC_FLAG_DONE_SPELL_MAGIC_DMG_CLASS_NEG;
            if (!spellFamilyName)
                triggerMask &=
                    ~PROC_FLAG_DONE_PERIODIC;  // spellFamilyName = 0 and PROC_FLAG_DONE_PERIODIC -> it is a dmg spell
            if (procFlags & triggerMask)
                return true;
            break;
        }
        default:
            break;
    }
    return false;
}

void StatsCollector::CollectByItemStatType(uint32 itemStatType, int32 val)
{
    switch (itemStatType)
    {
        case ITEM_MOD_MANA:
            stats[STATS_TYPE_MANA_REGENERATION] += (float)val / 10;
            break;
        case ITEM_MOD_HEALTH:
            stats[STATS_TYPE_STAMINA] += (float)val / 15;
            break;
        case ITEM_MOD_AGILITY:
            stats[STATS_TYPE_AGILITY] += val;
            break;
        case ITEM_MOD_STRENGTH:
            stats[STATS_TYPE_STRENGTH] += val;
            break;
        case ITEM_MOD_INTELLECT:
            stats[STATS_TYPE_INTELLECT] += val;
            break;
        case ITEM_MOD_SPIRIT:
            stats[STATS_TYPE_SPIRIT] += val;
            break;
        case ITEM_MOD_STAMINA:
            stats[STATS_TYPE_STAMINA] += val;
            break;
        case ITEM_MOD_DEFENSE_SKILL_RATING:
            // 5.4.8: Defense Rating was removed from itemization in Cataclysm; no item on this
            // expansion sets this stat, kept for AC diff parity (see file header).
            stats[STATS_TYPE_DEFENSE] += val;
            break;
        case ITEM_MOD_DODGE_RATING:
            stats[STATS_TYPE_DODGE] += val;
            break;
        case ITEM_MOD_PARRY_RATING:
            stats[STATS_TYPE_PARRY] += val;
            break;
        case ITEM_MOD_BLOCK_RATING:
            stats[STATS_TYPE_BLOCK_RATING] += val;
            break;
        case ITEM_MOD_HIT_MELEE_RATING:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_HIT_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
                stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_CRIT_MELEE_RATING:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_CRIT_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
                stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_HASTE_MELEE_RATING:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_RANGED_RATING:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HASTE_SPELL_RATING:
            if (type_ & CollectorType::SPELL)
                stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_HIT_RATING:
            stats[STATS_TYPE_HIT] += val;
            break;
        case ITEM_MOD_CRIT_RATING:
            stats[STATS_TYPE_CRIT] += val;
            break;
        case ITEM_MOD_RESILIENCE_RATING:
            stats[STATS_TYPE_RESILIENCE] += val;
            break;
        case ITEM_MOD_HASTE_RATING:
            stats[STATS_TYPE_HASTE] += val;
            break;
        case ITEM_MOD_EXPERTISE_RATING:
            stats[STATS_TYPE_EXPERTISE] += val;
            break;
        case ITEM_MOD_ATTACK_POWER:
            stats[STATS_TYPE_ATTACK_POWER] += val;
            break;
        case ITEM_MOD_RANGED_ATTACK_POWER:
            if (type_ == CollectorType::RANGED)
                stats[STATS_TYPE_ATTACK_POWER] += val;
            break;
        case ITEM_MOD_MANA_REGENERATION:
            stats[STATS_TYPE_MANA_REGENERATION] += val;
            break;
        case ITEM_MOD_ARMOR_PENETRATION_RATING:
            // 5.4.8: Armor Penetration was removed from itemization in Cataclysm (replaced by
            // Mastery); no item on this expansion sets this stat, kept for AC diff parity.
            stats[STATS_TYPE_ARMOR_PENETRATION] += val;
            break;
        case ITEM_MOD_SPELL_POWER:
            stats[STATS_TYPE_SPELL_POWER] += val;
            stats[STATS_TYPE_HEAL_POWER] += val;
            break;
        case ITEM_MOD_HEALTH_REGEN:
            stats[STATS_TYPE_HEALTH_REGENERATION] += val;
            break;
        case ITEM_MOD_SPELL_PENETRATION:
            stats[STATS_TYPE_SPELL_PENETRATION] += val;
            break;
        case ITEM_MOD_BLOCK_VALUE:
            stats[STATS_TYPE_BLOCK_VALUE] += val;
            break;
        // 5.4.8: Cataclysm/MoP-only secondary stats, no WotLK analogue in AC (see StatsCollector.h).
        case ITEM_MOD_MASTERY_RATING:
            stats[STATS_TYPE_MASTERY] += val;
            break;
        case ITEM_MOD_PVP_POWER:
            stats[STATS_TYPE_PVP_POWER] += val;
            break;
        default:
            break;
    }
}

void StatsCollector::HandleApplyAura(SpellEffectInfo const& effectInfo, float multiplier, bool canNextTrigger,
                                     Milliseconds triggerCooldown)
{
    if (effectInfo.Effect != SPELL_EFFECT_APPLY_AURA)
        return;

    int32 val = AverageValue(effectInfo);

    switch (effectInfo.ApplyAuraName)
    {
        case SPELL_AURA_MOD_DAMAGE_DONE:
        {
            int32 schoolType = effectInfo.MiscValue;
            if (schoolType & SPELL_SCHOOL_MASK_NORMAL)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            if ((schoolType & SPELL_SCHOOL_MASK_MAGIC) == SPELL_SCHOOL_MASK_MAGIC)
                stats[STATS_TYPE_SPELL_POWER] += val * multiplier;
            break;
        }
        case SPELL_AURA_MOD_HEALING_DONE:
            stats[STATS_TYPE_HEAL_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_INCREASE_HEALTH:
            stats[STATS_TYPE_STAMINA] += val * multiplier / 15;
            break;
        case SPELL_AURA_SCHOOL_ABSORB:
        {
            int32 schoolType = effectInfo.MiscValue;
            if (schoolType & SPELL_SCHOOL_MASK_NORMAL)
                stats[STATS_TYPE_STAMINA] += val * multiplier / 15;
            break;
        }
        case SPELL_AURA_MOD_ATTACK_POWER:
            if (type_ & CollectorType::MELEE)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_RANGED_ATTACK_POWER:
            if (type_ & CollectorType::RANGED)
                stats[STATS_TYPE_ATTACK_POWER] += val * multiplier;
            break;
        case SPELL_AURA_MOD_SHIELD_BLOCKVALUE:
            stats[STATS_TYPE_BLOCK_VALUE] += val * multiplier;
            break;
        case SPELL_AURA_MOD_STAT:
        {
            int32 statType = effectInfo.MiscValue;
            switch (statType)
            {
                case STAT_STRENGTH:
                    stats[STATS_TYPE_STRENGTH] += val * multiplier;
                    break;
                case STAT_AGILITY:
                    stats[STATS_TYPE_AGILITY] += val * multiplier;
                    break;
                case STAT_STAMINA:
                    stats[STATS_TYPE_STAMINA] += val * multiplier;
                    break;
                case STAT_INTELLECT:
                    stats[STATS_TYPE_INTELLECT] += val * multiplier;
                    break;
                case STAT_SPIRIT:
                    stats[STATS_TYPE_SPIRIT] += val * multiplier;
                    break;
                case -1:  // Stat all
                    stats[STATS_TYPE_STRENGTH] += val * multiplier;
                    stats[STATS_TYPE_AGILITY] += val * multiplier;
                    stats[STATS_TYPE_STAMINA] += val * multiplier;
                    stats[STATS_TYPE_INTELLECT] += val * multiplier;
                    stats[STATS_TYPE_SPIRIT] += val * multiplier;
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_MOD_RESISTANCE:
        {
            int32 statType = effectInfo.MiscValue;
            if (statType & SPELL_SCHOOL_MASK_NORMAL)  // physical
                stats[STATS_TYPE_ARMOR] += val * multiplier;
            break;
        }
        case SPELL_AURA_MOD_RATING:
        {
            for (uint32 rating = CR_WEAPON_SKILL; rating < CR_MAX_COMBAT_RATING; ++rating)
            {
                if (effectInfo.MiscValue & (1 << rating))
                {
                    switch (rating)
                    {
                        case CR_DEFENSE_SKILL:
                            stats[STATS_TYPE_DEFENSE] += val * multiplier;
                            break;
                        case CR_DODGE:
                            stats[STATS_TYPE_DODGE] += val * multiplier;
                            break;
                        case CR_PARRY:
                            stats[STATS_TYPE_PARRY] += val * multiplier;
                            break;
                        case CR_BLOCK:
                            stats[STATS_TYPE_BLOCK_RATING] += val * multiplier;
                            break;
                        case CR_HIT_MELEE:
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_HIT_SPELL:
                            if (type_ & CollectorType::SPELL)
                                stats[STATS_TYPE_HIT] += val * multiplier;
                            break;
                        case CR_CRIT_MELEE:
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_CRIT_SPELL:
                            if (type_ & CollectorType::SPELL)
                                stats[STATS_TYPE_CRIT] += val * multiplier;
                            break;
                        case CR_HASTE_MELEE:
                            if (type_ & CollectorType::MELEE)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_RANGED:
                            if (type_ & CollectorType::RANGED)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_HASTE_SPELL:
                            if (type_ & CollectorType::SPELL)
                                stats[STATS_TYPE_HASTE] += val * multiplier;
                            break;
                        case CR_EXPERTISE:
                            stats[STATS_TYPE_EXPERTISE] += val * multiplier;
                            break;
                        case CR_ARMOR_PENETRATION:
                            // 5.4.8: dead on this expansion (see file header), kept for parity.
                            stats[STATS_TYPE_ARMOR_PENETRATION] += val * multiplier;
                            break;
                        // 5.4.8: Cataclysm/MoP-only ratings, no WotLK analogue in AC.
                        case CR_MASTERY:
                            stats[STATS_TYPE_MASTERY] += val * multiplier;
                            break;
                        case CR_PVP_POWER:
                            stats[STATS_TYPE_PVP_POWER] += val * multiplier;
                            break;
                        default:
                            break;
                    }
                }
            }
            break;
        }
        case SPELL_AURA_MOD_POWER_REGEN:
        {
            int32 powerType = effectInfo.MiscValue;
            switch (powerType)
            {
                case POWER_MANA:
                    stats[STATS_TYPE_MANA_REGENERATION] += val * multiplier;
                    break;
                default:
                    break;
            }
            break;
        }
        case SPELL_AURA_PROC_TRIGGER_SPELL:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, multiplier, triggerCooldown);
            break;
        }
        case SPELL_AURA_PERIODIC_TRIGGER_SPELL:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, multiplier, triggerCooldown);
            break;
        }
        case SPELL_AURA_ADD_TARGET_TRIGGER:
        {
            if (canNextTrigger)
                CollectSpellStats(effectInfo.TriggerSpell, multiplier, triggerCooldown);
            break;
        }
        case SPELL_AURA_MOD_CRIT_DAMAGE_BONUS:
        {
            if (type_ != CollectorType::SPELL_HEAL)
            {
                int32 statType = effectInfo.MiscValue;
                if (statType & SPELL_SCHOOL_MASK_NORMAL)  // physical
                    stats[STATS_TYPE_CRIT] += 30 * val * multiplier;
            }
            break;
        }
        default:
            break;
    }
}

float StatsCollector::AverageValue(SpellEffectInfo const& effectInfo)
{
    // float basePointsPerLevel = effectInfo.RealPointsPerLevel; //not used, line marked for removal.
    float basePoints = effectInfo.BasePoints;
    int32 randomPoints = effectInfo.DieSides;

    switch (randomPoints)
    {
        case 0:
            break;
        case 1:
            basePoints += 1;
            break;
        default:
            float randvalue = (1 + randomPoints) / 2.0f;
            basePoints += randvalue;
            break;
    }
    return basePoints;
}

bool StatsCollector::CheckSpellValidation(uint32 spellFamilyName, flag128 spelFalimyFlags, bool strict)
{
    if (StatsCollector::Class2SpellFamilyName(cls_) != spellFamilyName)
        return false;

    bool isHealingSpell = StatsCollector::IsHealingSpell(spellFamilyName, spelFalimyFlags);
    // strict to healer
    if (strict && (type_ & CollectorType::SPELL_HEAL))
    {
        return isHealingSpell;
    }

    if (!(type_ & CollectorType::SPELL_HEAL) && isHealingSpell)
        return false;

    // spells for caster/melee/tank are ambiguous
    if (cls_ == CLASS_DRUID && spellFamilyName == SPELLFAMILY_DRUID && (type_ & CollectorType::MELEE))
    {
        uint32 castingFlagsA = 0x4 | 0x2 | 0x1 | 0x200000;  // starfire | moonfire | wrath | insect swarm
        uint32 castingFlagsB = 0x0;
        uint32 castingFlagsC = 0x0;
        flag128 invalidFlags = {castingFlagsA, castingFlagsB, castingFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_PALADIN && spellFamilyName == SPELLFAMILY_PALADIN && (type_ & CollectorType::MELEE_TANK))
    {
        uint32 retributionFlagsA = 0x0;
        uint32 retributionFlagsB = 0x8000;  // crusader strike
        uint32 retributionFlagsC = 0x0;
        flag128 invalidFlags = {retributionFlagsA, retributionFlagsB, retributionFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_PALADIN && spellFamilyName == SPELLFAMILY_PALADIN && (type_ & CollectorType::MELEE_DMG))
    {
        uint32 retributionFlagsA = 0x0;
        uint32 retributionFlagsB = 0x100000 | 0x40;  // shield of righteouness | holy shield
        uint32 retributionFlagsC = 0x0;
        flag128 invalidFlags = {retributionFlagsA, retributionFlagsB, retributionFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_SHAMAN && spellFamilyName == SPELLFAMILY_SHAMAN && (type_ & CollectorType::SPELL_DMG))
    {
        uint32 meleeFlagsA = 0x0;
        uint32 meleeFlagsB = 0x1000010;  // stromstrike
        uint32 meleeFlagsC = 0x4;        // lava lash
        flag128 invalidFlags = {meleeFlagsA, meleeFlagsB, meleeFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    if (cls_ == CLASS_SHAMAN && spellFamilyName == SPELLFAMILY_SHAMAN && (type_ & CollectorType::MELEE_DMG))
    {
        uint32 casterFlagsA = 0x0;
        uint32 casterFlagsB = 0x1000;  // lava burst
        uint32 casterFlagsC = 0x0;
        flag128 invalidFlags = {casterFlagsA, casterFlagsB, casterFlagsC};
        if (spelFalimyFlags & invalidFlags)
            return false;
    }

    return true;
}

SpellFamilyNames StatsCollector::Class2SpellFamilyName(uint8 cls)
{
    switch (cls)
    {
        case CLASS_WARRIOR:
            return SPELLFAMILY_WARRIOR;
        case CLASS_PALADIN:
            return SPELLFAMILY_PALADIN;
        case CLASS_HUNTER:
            return SPELLFAMILY_HUNTER;
        case CLASS_ROGUE:
            return SPELLFAMILY_ROGUE;
        case CLASS_PRIEST:
            return SPELLFAMILY_PRIEST;
        case CLASS_DEATH_KNIGHT:
            return SPELLFAMILY_DEATHKNIGHT;
        case CLASS_SHAMAN:
            return SPELLFAMILY_SHAMAN;
        case CLASS_MAGE:
            return SPELLFAMILY_MAGE;
        case CLASS_WARLOCK:
            return SPELLFAMILY_WARLOCK;
        case CLASS_DRUID:
            return SPELLFAMILY_DRUID;
        // 5.4.8: Monk, no WotLK analogue in AC.
        case CLASS_MONK:
            return SPELLFAMILY_MONK;
        default:
            break;
    }
    return SPELLFAMILY_GENERIC;
}

bool StatsCollector::IsHealingSpell(uint32 spellFamilyName, flag128 spellFamilyFlags)
{
    if (!spellFamilyName)
        return false;
    flag128 healingFlags;
    switch (spellFamilyName)
    {
        case SPELLFAMILY_DRUID:
        {
            uint32 healingFlagsA = 0x10 | 0x40 | 0x20 | 0x80;  // rejuvenation | regrowth | healing touch | tranquility
            uint32 healingFlagsB = 0x4000000 | 0x2000000 | 0x2 | 0x10;  // wild growth | nourish | swiftmend | lifebloom
            uint32 healingFlagsC = 0x0;
            healingFlags = {healingFlagsA, healingFlagsB, healingFlagsC};
            break;
        }
        case SPELLFAMILY_PALADIN:
        {
            uint32 healingFlagsA = 0x80000000 | 0x40000000 | 0x8000 |
                                   0x80000;  // holy light | flash of light | lay on hands | judgement of light
            uint32 healingFlagsB = 0x10000;  // holy shock
            uint32 healingFlagsC = 0x0;
            healingFlags = {healingFlagsA, healingFlagsB, healingFlagsC};
            break;
        }
        case SPELLFAMILY_SHAMAN:
        {
            uint32 healingFlagsA = 0x80 | 0x40 | 0x100;  // lesser healing wave | healing wave | chain heal
            uint32 healingFlagsB = 0x400;                // earth shield
            uint32 healingFlagsC = 0x10;                 // riptide
            healingFlags = {healingFlagsA, healingFlagsB, healingFlagsC};
            break;
        }
        case SPELLFAMILY_PRIEST:
        {
            uint32 healingFlagsA = 0x40 | 0x200 | 0x40000 | 0x1000 | 0x800 | 0x400 |
                                   0x10000000;  // renew | prayer of healing | lesser heal | greater heal | flash heal |
                                                // heal | circle of healing
            uint32 healingFlagsB = 0x800000 | 0x20 | 0x4;  // penance | prayer of mending | binding heal
            uint32 healingFlagsC = 0x0;
            healingFlags = {healingFlagsA, healingFlagsB, healingFlagsC};
            break;
        }
        // 5.4.8: Monk healing (Mistweaver) is not classified here - Monk spec detection in
        // StatsWeightCalculator uses AiFactory::GetPlayerSpecTab() directly instead of relying on
        // spell-family heuristics, so this is not on the hot path for Monk gear scoring. Flag for a
        // follow-up if Monk item-proc scoring turns out to matter in practice.
        default:
            break;
    }
    return spellFamilyFlags & healingFlags;
}
