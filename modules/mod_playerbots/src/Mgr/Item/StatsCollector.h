/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Mgr/Item/StatsCollector.{h,cpp} for issue #188 (S6 of epic #163).
 * 5.4.8 notes:
 *  - Added STATS_TYPE_MASTERY and STATS_TYPE_PVP_POWER: Cataclysm/MoP-only itemization that has no
 *    WotLK analogue in AC.
 *  - STATS_TYPE_DEFENSE and STATS_TYPE_ARMOR_PENETRATION are kept (for AC diff parity / array layout)
 *    even though Defense Rating and Armor Penetration were removed from itemization in Cataclysm
 *    (ITEM_MOD_DEFENSE_SKILL_RATING / ITEM_MOD_ARMOR_PENETRATION_RATING still exist as enum values on
 *    5.4.8 but no item or aura on this expansion sets them, so these buckets are always 0) - this is
 *    "the way AC drops unknown ones": no special-casing needed, the value is simply never populated.
 *  - Class2SpellFamilyName()/IsHealingSpell() are ported here as StatsCollector statics (AC has them
 *    on PlayerbotAI, which is off-limits to edit for this issue - see PlayerbotAI.h:603-604 in AC).
 */

#ifndef _PLAYERBOT_STATSCOLLECTOR_H
#define _PLAYERBOT_STATSCOLLECTOR_H

struct ItemTemplate;
#include "SharedDefines.h"
#include "SpellInfo.h"

enum StatsType : uint8
{
    // Basic stats
    STATS_TYPE_AGILITY = 0,
    STATS_TYPE_STRENGTH,
    STATS_TYPE_INTELLECT,
    STATS_TYPE_SPIRIT,
    STATS_TYPE_STAMINA,
    STATS_TYPE_HIT,
    STATS_TYPE_CRIT,
    STATS_TYPE_HASTE,
    // Stats for tank
    STATS_TYPE_ARMOR,
    STATS_TYPE_DEFENSE,
    STATS_TYPE_DODGE,
    STATS_TYPE_PARRY,
    STATS_TYPE_BLOCK_VALUE,
    STATS_TYPE_BLOCK_RATING,
    STATS_TYPE_RESILIENCE,
    STATS_TYPE_HEALTH_REGENERATION,
    // Stats for spell damage
    STATS_TYPE_SPELL_POWER,
    STATS_TYPE_SPELL_PENETRATION,
    // Stats for heal
    STATS_TYPE_HEAL_POWER,
    STATS_TYPE_MANA_REGENERATION,
    // Stats for physical damage and melee
    STATS_TYPE_ATTACK_POWER,
    STATS_TYPE_ARMOR_PENETRATION,
    STATS_TYPE_EXPERTISE,
    // Stats for weapon dps
    STATS_TYPE_MELEE_DPS,
    STATS_TYPE_RANGED_DPS,
    // 5.4.8: Cataclysm/MoP-only secondary stats, no WotLK analogue in AC.
    STATS_TYPE_MASTERY,
    STATS_TYPE_PVP_POWER,
    // Bonus for unrecognized stats
    STATS_TYPE_BONUS,
    STATS_TYPE_MAX = 28
};

enum CollectorType : uint8
{
    MELEE_DMG = 1,
    MELEE_TANK = 2,
    RANGED = 4,
    SPELL_DMG = 8,
    SPELL_HEAL = 16,
    MELEE = MELEE_DMG | MELEE_TANK,
    SPELL = SPELL_DMG | SPELL_HEAL
};

class StatsCollector
{
public:
    StatsCollector(CollectorType type, int32 cls = -1);
    StatsCollector(StatsCollector& stats) = default;
    void Reset();
    void CollectItemStats(ItemTemplate const* proto);
    void CollectSpellStats(uint32 spellId, float multiplier = 1.0f, Milliseconds spellCooldown = -1ms);
    void CollectEnchantStats(SpellItemEnchantmentEntry const* enchant, uint32 default_enchant_amount = 0);
    bool CanBeTriggeredByType(SpellInfo const* spellInfo, uint32 procFlags, bool strict = true);
    bool CheckSpellValidation(uint32 spellFamilyName, flag128 spelFalimyFlags, bool strict = true);

    // 5.4.8: local ports of AC's PlayerbotAI::Class2SpellFamilyName / PlayerbotAI::IsHealingSpell
    // (PlayerbotAI.h is off-limits for this issue - see class comment above).
    static SpellFamilyNames Class2SpellFamilyName(uint8 cls);
    static bool IsHealingSpell(uint32 spellFamilyName, flag128 spellFamilyFlags);

public:
    float stats[STATS_TYPE_MAX];

private:
    void CollectByItemStatType(uint32 itemStatType, int32 val);
    bool SpecialSpellFilter(uint32 spellId);
    bool SpecialEnchantFilter(uint32 enchantSpellId);

    void HandleApplyAura(SpellEffectInfo const& effectInfo, float multiplier, bool canNextTrigger,
                         Milliseconds triggerCooldown);
    float AverageValue(SpellEffectInfo const& effectInfo);

private:
    CollectorType type_;
    uint32 cls_;
};

#endif
