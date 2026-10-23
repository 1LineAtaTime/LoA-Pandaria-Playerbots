/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAMANTRIGGERS_H
#define _PLAYERBOT_SHAMANTRIGGERS_H

#include "CureTriggers.h"
#include "GenericTriggers.h"
#include "SharedDefines.h"
#include "Unit.h"
#include <ctime>

class PlayerbotAI;
class Item;

// Weapon imbues (Windfury/Flametongue/Frostbrand/Earthliving Weapon) are still applied as temporary
// weapon enchants in MoP (this did not change the way poisons/ammo did), so these check the same
// TEMP_ENCHANTMENT_SLOT item API AC uses.
class MainHandWeaponNoImbueTrigger : public BuffTrigger
{
public:
    MainHandWeaponNoImbueTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "main hand", 1) {}
    bool IsActive() override;

private:
    time_t lastWrongImbue = 0;
};

class OffHandWeaponNoImbueTrigger : public BuffTrigger
{
public:
    OffHandWeaponNoImbueTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "off hand", 1) {}
    bool IsActive() override;
};

// Maelstrom Weapon is still a real Enhancement mechanic in MoP (stacks reduce next spell's cast time).
class MaelstromWeaponTrigger : public HasAuraStackTrigger
{
public:
    MaelstromWeaponTrigger(PlayerbotAI* botAI, int stack = 5) : HasAuraStackTrigger(botAI, "maelstrom weapon", stack) {}
};

// Frost Shock still roots/slows in MoP.
SNARE_TRIGGER(FrostShockSnareTrigger, "frost shock");

// Cleanse Spirit is a single MoP shaman spell (like paladin's Cleanse), so these mirror the paladin
// cleanse triggers but point at "cleanse spirit" with the relevant dispel type.
class CleanseSpiritPoisonTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritPoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_POISON) {}
};

class CleanseSpiritDiseaseTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_DISEASE) {}
};

class CleanseSpiritCurseTrigger : public NeedCureTrigger
{
public:
    CleanseSpiritCurseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "cleanse spirit", DISPEL_CURSE) {}
};

class PartyMemberCleanseSpiritPoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritPoisonTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_POISON)
    {
    }
};

class PartyMemberCleanseSpiritDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritDiseaseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_DISEASE)
    {
    }
};

class PartyMemberCleanseSpiritCurseTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCleanseSpiritCurseTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "cleanse spirit", DISPEL_CURSE)
    {
    }
};

// #382 (MoP-only; Wowhead mop-classic tooltips 51886 / 77130, 2026-10-04): in 5.4.8 Cleanse Spirit removes only
// Curse effects; Restoration's Purify Spirit (77130) removes Curse and Magic. The poison/disease triggers above are
// therefore no longer wired to any edge (HasAuraToDispel is spell-agnostic and would make the bot spam a cast that
// cannot remove the aura); magic is handled by these Purify Spirit triggers, mirroring the paladin Cleanse magic ones.
class PurifySpiritMagicTrigger : public NeedCureTrigger
{
public:
    PurifySpiritMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "purify spirit", DISPEL_MAGIC) {}
};

class PartyMemberPurifySpiritMagicTrigger : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberPurifySpiritMagicTrigger(PlayerbotAI* botAI)
        : PartyMemberNeedCureTrigger(botAI, "purify spirit", DISPEL_MAGIC)
    {
    }
};

// #149: MoP's shaman still has separate "cure disease"/"cure poison" actions/cast spells (see
// ShamanActions.h CURE_ACTION/CURE_PARTY_ACTION and the actions already registered in
// ShamanAiObjectContext.cpp) alongside "cleanse spirit" above; GenericShamanStrategy.cpp /
// ShamanNonCombatStrategy.cpp already reference "party member cure disease"/"party member cure
// poison" as dead trigger names. Reuse the same generic PartyMemberNeedCureTrigger base the Priest
// ("party member cure disease") and Druid ("party member cure poison") versions use; "Sham" suffix
// avoids an ODR clash with those same-named classes in the other class folders (same convention as
// CastCurePoisonActionSham/CastCureDiseaseActionSham in ShamanActions.h).
class PartyMemberCureDiseaseTriggerSham : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCureDiseaseTriggerSham(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure disease", DISPEL_DISEASE) {}
};

class PartyMemberCurePoisonTriggerSham : public PartyMemberNeedCureTrigger
{
public:
    PartyMemberCurePoisonTriggerSham(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "cure poison", DISPEL_POISON) {}
};

// Totem presence checks.
//
// AC (Ai/Class/Shaman/ShamanTriggers.cpp:287-447) checks every element through one of the shaman's
// four summon slots rather than by scanning totem names: bot->m_SummonSlot[SUMMON_SLOT_TOTEM_<ELEMENT>]
// resolved to the creature standing in it, its summon spell read from UNIT_CREATED_BY_SPELL, plus a
// 30 yard range check and a "does the slot hold what the active element strategy wants" test.
//
// That model exists in this fork as well - Unit.h:1008-1021 for the SummonSlot enum,
// Unit.h:2083 for the std::array<ObjectGuid, SUMMON_SLOT_MAX> m_SummonSlot storage and
// Unit.h:1207/2508 for IsTotem()/ToTotem() - so these triggers are ported per slot. The only genuine
// core gap is Player::GetActionButton: AC reads the totem bar (TOTEM_BAR_SLOT_FIRE/EARTH/WATER/AIR =
// 132..135 in Strategy/TotemsShamanStrategy.h:17-20) through it in SetTotemTrigger::IsActive only, and
// this fork exposes ActionButtonPACKET/ActionButtonUpdateState (Player.h:89,351-356) with no accessor.
// The element's summon slot stands in for the bar slot there, which keeps AC's element grouping.
class NoFireTotemTrigger : public Trigger
{
public:
    NoFireTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no fire totem") {}
    bool IsActive() override;
};

class NoWaterTotemTrigger : public Trigger
{
public:
    NoWaterTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no water totem") {}
    bool IsActive() override;
};

class NoAirTotemTrigger : public Trigger
{
public:
    NoAirTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no air totem") {}
    bool IsActive() override;
};

class NoEarthTotemTrigger : public Trigger
{
public:
    NoEarthTotemTrigger(PlayerbotAI* botAI) : Trigger(botAI, "no earth totem") {}
    bool IsActive() override;
};

// Buff triggers
class WaterShieldTrigger : public BuffTrigger
{
public:
    WaterShieldTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water shield") {}
};

class LightningShieldTrigger : public BuffTrigger
{
public:
    LightningShieldTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "lightning shield") {}
};

class WaterWalkingTrigger : public BuffTrigger
{
public:
    WaterWalkingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water walking", 7) {}
    bool IsActive() override;
};

class WaterBreathingTrigger : public BuffTrigger
{
public:
    WaterBreathingTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "water breathing", 5 * 2000) {}
    bool IsActive() override;
};

class WaterWalkingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterWalkingOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "water walking on party", 2 * 2000) {}
    bool IsActive() override;
};

class WaterBreathingOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    WaterBreathingOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "water breathing on party", 2 * 2000) {}
    bool IsActive() override;
};

// Boost triggers
class HeroismTrigger : public BoostTrigger
{
public:
    HeroismTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "heroism") {}
};

class BloodlustTrigger : public BoostTrigger
{
public:
    BloodlustTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "bloodlust") {}
};

class ElementalMasteryTrigger : public BuffTrigger
{
public:
    ElementalMasteryTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "elemental mastery") {}
    bool IsActive() override;
};

class SpiritWalkTrigger : public Trigger
{
public:
    SpiritWalkTrigger(PlayerbotAI* botAI) : Trigger(botAI, "spirit walk ready") {}
    bool IsActive() override;

private:
    time_t lastSpiritWalkTime = 0;
};

class FireElementalTotemTrigger : public BoostTrigger
{
public:
    FireElementalTotemTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "fire elemental totem") {}
};

// CC, Interrupt, and Dispel triggers
class WindShearInterruptSpellTrigger : public InterruptSpellTrigger
{
public:
    WindShearInterruptSpellTrigger(PlayerbotAI* botAI) : InterruptSpellTrigger(botAI, "wind shear") {}
};

class WindShearInterruptEnemyHealerSpellTrigger : public InterruptEnemyHealerTrigger
{
public:
    WindShearInterruptEnemyHealerSpellTrigger(PlayerbotAI* botAI) : InterruptEnemyHealerTrigger(botAI, "wind shear") {}
};

class PurgeTrigger : public TargetAuraDispelTrigger
{
public:
    PurgeTrigger(PlayerbotAI* botAI) : TargetAuraDispelTrigger(botAI, "purge", DISPEL_MAGIC) {}
};

// Damage and Debuff triggers
class ShockTrigger : public DebuffTrigger
{
public:
    ShockTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "earth shock", 1, true) {}
    bool IsActive() override;
};

class FlameShockTrigger : public DebuffTrigger
{
public:
    FlameShockTrigger(PlayerbotAI* botAI) : DebuffTrigger(botAI, "flame shock", 1, true, 6.0f) {}
    bool IsActive() override;
};

class EarthShockExecuteTrigger : public Trigger
{
public:
    EarthShockExecuteTrigger(PlayerbotAI* botAI) : Trigger(botAI, "earth shock execute") {}
    bool IsActive() override;
};

class MaelstromWeapon5AndMediumAoeTrigger : public TwoTriggers
{
public:
    MaelstromWeapon5AndMediumAoeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "maelstrom weapon 5", "medium aoe") {}
};

class MaelstromWeapon4AndMediumAoeTrigger : public TwoTriggers
{
public:
    MaelstromWeapon4AndMediumAoeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "maelstrom weapon 4", "medium aoe") {}
};

class ChainLightningNoCdTrigger : public SpellNoCooldownTrigger
{
public:
    ChainLightningNoCdTrigger(PlayerbotAI* botAI) : SpellNoCooldownTrigger(botAI, "chain lightning") {}
};

// Healing triggers
class EarthShieldOnMainTankTrigger : public BuffOnMainTankTrigger
{
public:
    EarthShieldOnMainTankTrigger(PlayerbotAI* botAI) : BuffOnMainTankTrigger(botAI, "earth shield", false) {}
};

// Totem triggers
class CallOfTheElementsTrigger : public Trigger
{
public:
    CallOfTheElementsTrigger(PlayerbotAI* botAI) : Trigger(botAI, "call of the elements") {}
    bool IsActive() override;
};

class TotemicRecallTrigger : public Trigger
{
public:
    TotemicRecallTrigger(PlayerbotAI* botAI) : Trigger(botAI, "totemic recall") {}
    bool IsActive() override;
};

class CallOfTheElementsAndEnemyWithinMeleeTrigger : public TwoTriggers
{
public:
    CallOfTheElementsAndEnemyWithinMeleeTrigger(PlayerbotAI* botAI) : TwoTriggers(botAI, "call of the elements", "enemy within melee") {}
};

// Set Strategy Assigned Totems
//
// AC (Ai/Class/Shaman/ShamanTriggers.h:327-344 and .cpp:449-473) gives all seventeen "set <totem>"
// triggers one shared shape: each carries its own totem spell-id table plus the TOTEM_BAR_SLOT_*
// index of the element slot that totem belongs to, and IsActive() reports "the totem assigned to that
// bar slot is not the highest rank of my totem the bot knows".
//
// This fork has no Player::GetActionButton accessor, so the bar read is replaced by the summon slot the
// core keeps for that same element (Unit.h:1008-1021: SUMMON_SLOT_TOTEM_FIRE/EARTH/WATER/AIR = 1/2/3/4;
// storage at Unit.h:2083). The element grouping below is AC's verbatim - earth = slot 2, fire = slot 1,
// water = slot 3, air = slot 4. The standing totem is matched by summon creature name instead of by
// AC's WotLK spell-id tables, which MoP re-numbered for every shaman totem; the creature name is the
// same identity the rest of this folder already uses (ShamanActions.h casts totems by name, and
// HasTotemValue.cpp:20 compares the same summon creature name).
class SetTotemTrigger : public Trigger
{
public:
    SetTotemTrigger(PlayerbotAI* botAI, char const* totemName, char const* summonTotemName, uint8 elementSlot)
        : Trigger(botAI, std::string("set ") + totemName)
        , totemName(totemName)
        , summonTotemName(summonTotemName)
        , elementSlot(elementSlot)
    {
    }

    bool IsActive() override;

private:
    char const* totemName;
    char const* summonTotemName;
    uint8 elementSlot;
};

class SetStrengthOfEarthTotemTrigger : public SetTotemTrigger
{
public:
    SetStrengthOfEarthTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "strength of earth totem", "Strength of Earth Totem", SUMMON_SLOT_TOTEM_EARTH) {}
};

class SetStoneskinTotemTrigger : public SetTotemTrigger
{
public:
    SetStoneskinTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "stoneskin totem", "Stoneskin Totem", SUMMON_SLOT_TOTEM_EARTH) {}
};

class SetTremorTotemTrigger : public SetTotemTrigger
{
public:
    SetTremorTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "tremor totem", "Tremor Totem", SUMMON_SLOT_TOTEM_EARTH) {}
};

class SetEarthbindTotemTrigger : public SetTotemTrigger
{
public:
    SetEarthbindTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "earthbind totem", "Earthbind Totem", SUMMON_SLOT_TOTEM_EARTH) {}
};

class SetSearingTotemTrigger : public SetTotemTrigger
{
public:
    SetSearingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "searing totem", "Searing Totem", SUMMON_SLOT_TOTEM_FIRE) {}
};

class SetMagmaTotemTrigger : public SetTotemTrigger
{
public:
    SetMagmaTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "magma totem", "Magma Totem", SUMMON_SLOT_TOTEM_FIRE) {}
};

class SetFlametongueTotemTrigger : public SetTotemTrigger
{
public:
    SetFlametongueTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "flametongue totem", "Flametongue Totem", SUMMON_SLOT_TOTEM_FIRE) {}
};

class SetTotemOfWrathTrigger : public SetTotemTrigger
{
public:
    SetTotemOfWrathTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "totem of wrath", "Totem of Wrath", SUMMON_SLOT_TOTEM_FIRE) {}
};

class SetFrostResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFrostResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "frost resistance totem", "Frost Resistance Totem", SUMMON_SLOT_TOTEM_FIRE) {}
};

class SetHealingStreamTotemTrigger : public SetTotemTrigger
{
public:
    SetHealingStreamTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "healing stream totem", "Healing Stream Totem", SUMMON_SLOT_TOTEM_WATER) {}
};

class SetManaSpringTotemTrigger : public SetTotemTrigger
{
public:
    SetManaSpringTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "mana spring totem", "Mana Spring Totem", SUMMON_SLOT_TOTEM_WATER) {}
};

class SetCleansingTotemTrigger : public SetTotemTrigger
{
public:
    SetCleansingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "cleansing totem", "Cleansing Totem", SUMMON_SLOT_TOTEM_WATER) {}
};

class SetFireResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetFireResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "fire resistance totem", "Fire Resistance Totem", SUMMON_SLOT_TOTEM_WATER) {}
};

class SetWrathOfAirTotemTrigger : public SetTotemTrigger
{
public:
    SetWrathOfAirTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "wrath of air totem", "Wrath of Air Totem", SUMMON_SLOT_TOTEM_AIR) {}
};

class SetWindfuryTotemTrigger : public SetTotemTrigger
{
public:
    SetWindfuryTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "windfury totem", "Windfury Totem", SUMMON_SLOT_TOTEM_AIR) {}
};

class SetNatureResistanceTotemTrigger : public SetTotemTrigger
{
public:
    SetNatureResistanceTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "nature resistance totem", "Nature Resistance Totem", SUMMON_SLOT_TOTEM_AIR) {}
};

class SetGroundingTotemTrigger : public SetTotemTrigger
{
public:
    SetGroundingTotemTrigger(PlayerbotAI* botAI)
        : SetTotemTrigger(botAI, "grounding totem", "Grounding Totem", SUMMON_SLOT_TOTEM_AIR) {}
};

// ---- Rotation lane 3 (#858, epic #854).

// (group 10 interrupt on any casting enemy): battleground / arena only so the
// open world and dungeons keep AC's target / enemy-healer Wind Shear edges. Uses the lane 0 values
// "in pvp instance" and "enemy caster target" (#855).
class PvpEnemyCasterTrigger : public Trigger
{
public:
    PvpEnemyCasterTrigger(PlayerbotAI* botAI) : Trigger(botAI, "pvp enemy caster in range") {}
    bool IsActive() override;
};

// Ascendance 114049 and Ancestral Swiftness 16188: offensive
// cooldowns, same BoostTrigger gate as AC's Fire Elemental Totem ("boost" strategy).
class AscendanceTrigger : public BoostTrigger
{
public:
    AscendanceTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "ascendance") {}
};

class AncestralSwiftnessTrigger : public BoostTrigger
{
public:
    AncestralSwiftnessTrigger(PlayerbotAI* botAI) : BoostTrigger(botAI, "ancestral swiftness") {}
};

// (Restoration Ascendance with >= 3 group members below 45 % health).
class RestoAscendanceTrigger : public Trigger
{
public:
    RestoAscendanceTrigger(PlayerbotAI* botAI) : Trigger(botAI, "resto ascendance") {}
    bool IsActive() override;
};

#endif
