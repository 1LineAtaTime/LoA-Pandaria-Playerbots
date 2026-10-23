/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Mgr/Item/StatsWeightCalculator.{h,cpp} for issue #188 (S6 of epic
 * #163). 5.4.8: AC's `int tab` is a WotLK talent-tab index (0-2); this fork identifies specs with the
 * `Specializations` enum (SharedDefines.h) via AiFactory::GetPlayerSpecTab(), so `tab` is that type
 * here. See StatsWeightCalculator.cpp for the full tab->Specializations mapping and the new Mastery /
 * PvP Power / Monk weights.
 */

#ifndef PLAYERBOTS_STATSWEIGHTCALCULATOR_H
#define PLAYERBOTS_STATSWEIGHTCALCULATOR_H

#include "Player.h"
#include "SharedDefines.h"
#include "StatsCollector.h"

#define ITEM_SUBCLASS_MASK_SINGLE_HAND                                                                        \
    ((1 << ITEM_SUBCLASS_WEAPON_AXE) | (1 << ITEM_SUBCLASS_WEAPON_MACE) | (1 << ITEM_SUBCLASS_WEAPON_SWORD) | \
     (1 << ITEM_SUBCLASS_WEAPON_DAGGER) | (1 << ITEM_SUBCLASS_WEAPON_FIST_WEAPON))

enum StatsOverflowThreshold
{
    SPELL_HIT_OVERFLOW = 14,
    MELEE_HIT_OVERFLOW = 8,
    RANGED_HIT_OVERFLOW = 8,
    EXPERTISE_OVERFLOW = 26,
    DEFENSE_OVERFLOW = 140,
    ARMOR_PENETRATION_OVERFLOW = 100
};

class StatsWeightCalculator
{
public:
    StatsWeightCalculator(Player* player);
    void Reset();
    float CalculateItem(uint32 itemId, int32 randomPropertyId = 0, int32 slot = -1);
    float CalculateEnchant(uint32 enchantId);
    int32 PickBestRandomPropertyId(uint32 itemId);

    void SetOverflowPenalty(bool apply) { enable_overflow_penalty_ = apply; }
    void SetItemSetBonus(bool apply) { enable_item_set_bonus_ = apply; }
    void SetPvpSpec(bool isPvp) { pvpSpec_ = isPvp; }
    void SetExcludeResilience(bool exclude) { exclude_resilience_ = exclude; }

    private:
    void GenerateWeights(Player* player);
    void GenerateBasicWeights(Player* player);
    void GenerateAdditionalWeights(Player* player);

    void CalculateRandomProperty(int32 randomPropertyId, uint32 itemId);
    void CalculateItemSetMod(Player* player, ItemTemplate const* proto);
    void CalculateSocketBonus(Player* player, ItemTemplate const* proto);

    void CalculateItemTypePenalty(ItemTemplate const* proto);
    float ApplyPreferredSpecWeapons(ItemTemplate const* proto, int32 slot);

    bool NotBestArmorType(uint32 item_subclass_armor);

    void ApplyOverflowPenalty(Player* player);
    void ApplyWeightFinetune(Player* player);

private:
    Player* player_;
    CollectorType type_;
    CollectorType hitOverflowType_;
    std::unique_ptr<StatsCollector> collector_;
    uint8 cls;
    uint8 lvl;
    // 5.4.8: was `int tab` (WotLK talent-tab index) in AC; this fork has one spec enum per class
    // instead of 3 generic tabs (see .cpp for the mapping table).
    Specializations tab;
    bool enable_overflow_penalty_;
    bool enable_item_set_bonus_;

    float weight_;
    float stats_weights_[STATS_TYPE_MAX];
    bool pvpSpec_ = false;
    bool exclude_resilience_ = false;
};

#endif
