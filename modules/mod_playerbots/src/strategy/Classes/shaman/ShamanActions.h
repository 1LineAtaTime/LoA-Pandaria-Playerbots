/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_SHAMANACTIONS_H
#define _PLAYERBOT_SHAMANACTIONS_H

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "SharedDefines.h"

class PlayerbotAI;

class CastLesserHealingWaveAction : public CastHealingSpellAction
{
public:
    CastLesserHealingWaveAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lesser healing wave") {}
};

class CastLesserHealingWaveOnPartyAction : public HealPartyMemberAction
{
public:
    CastLesserHealingWaveOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "lesser healing wave", 25.0f, HealingManaEfficiency::LOW)
    {
    }
};

class CastHealingWaveAction : public CastHealingSpellAction
{
public:
    CastHealingWaveAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "healing wave") {}
};

class CastHealingWaveOnPartyAction : public HealPartyMemberAction
{
public:
    CastHealingWaveOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "healing wave", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastChainHealAction : public HealPartyMemberAction
{
public:
    CastChainHealAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "chain heal", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastRiptideAction : public CastHealingSpellAction
{
public:
    CastRiptideAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "riptide") {}
};

class CastRiptideOnPartyAction : public HealPartyMemberAction
{
public:
    CastRiptideOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "riptide", 15.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
};

class CastEarthShieldAction : public CastBuffSpellAction
{
public:
    CastEarthShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "earth shield") {}
};

class CastEarthShieldOnPartyAction : public BuffOnPartyAction
{
public:
    CastEarthShieldOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "earth shield") {}
};

class CastWaterShieldAction : public CastBuffSpellAction
{
public:
    CastWaterShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water shield") {}
};

class CastLightningShieldAction : public CastBuffSpellAction
{
public:
    CastLightningShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "lightning shield") {}
};

class CastEarthlivingWeaponAction : public CastEnchantItemAction
{
public:
    CastEarthlivingWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "earthliving weapon") {}
};

class CastRockbiterWeaponAction : public CastEnchantItemAction
{
public:
    CastRockbiterWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "rockbiter weapon") {}
};

class CastFlametongueWeaponAction : public CastEnchantItemAction
{
public:
    CastFlametongueWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "flametongue weapon") {}
};

class CastFrostbrandWeaponAction : public CastEnchantItemAction
{
public:
    CastFrostbrandWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "frostbrand weapon") {}
};

class CastWindfuryWeaponAction : public CastEnchantItemAction
{
public:
    CastWindfuryWeaponAction(PlayerbotAI* botAI) : CastEnchantItemAction(botAI, "windfury weapon") {}
};

// #382 (AC GenericSpellActions.cpp:327-364 CastEnchantItemMainHandAction / CastEnchantItemOffHandAction, used by
// AC ShamanActions.h:33-72): the hand-less actions above resolve their weapon through the "item for spell" value,
// which is a nullptr stub in this fork (value/ItemForSpellValue.h), so CastEnchantItemAction::isPossible() is always
// false and the hand-less imbue actions never cast. These per-hand actions name the equipment slot directly and cast
// with that item as the target, exactly like AC. Kept in the shaman folder (shared GenericSpellActions is not ours).
class CastShamanEnchantMainHandAction : public CastSpellAction
{
public:
    CastShamanEnchantMainHandAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

    bool Execute(Event event) override;
    bool isPossible() override;
    std::string const GetTargetName() override { return "self target"; }
};

class CastShamanEnchantOffHandAction : public CastSpellAction
{
public:
    CastShamanEnchantOffHandAction(PlayerbotAI* botAI, std::string const spell) : CastSpellAction(botAI, spell) {}

    bool Execute(Event event) override;
    bool isPossible() override;
    std::string const GetTargetName() override { return "self target"; }
};

class CastRockbiterWeaponMainHandAction : public CastShamanEnchantMainHandAction
{
public:
    CastRockbiterWeaponMainHandAction(PlayerbotAI* botAI) : CastShamanEnchantMainHandAction(botAI, "rockbiter weapon") {}
};

class CastFlametongueWeaponMainHandAction : public CastShamanEnchantMainHandAction
{
public:
    CastFlametongueWeaponMainHandAction(PlayerbotAI* botAI)
        : CastShamanEnchantMainHandAction(botAI, "flametongue weapon")
    {
    }
};

class CastFlametongueWeaponOffHandAction : public CastShamanEnchantOffHandAction
{
public:
    CastFlametongueWeaponOffHandAction(PlayerbotAI* botAI) : CastShamanEnchantOffHandAction(botAI, "flametongue weapon")
    {
    }
};

class CastEarthlivingWeaponMainHandAction : public CastShamanEnchantMainHandAction
{
public:
    CastEarthlivingWeaponMainHandAction(PlayerbotAI* botAI)
        : CastShamanEnchantMainHandAction(botAI, "earthliving weapon")
    {
    }
};

class CastWindfuryWeaponMainHandAction : public CastShamanEnchantMainHandAction
{
public:
    CastWindfuryWeaponMainHandAction(PlayerbotAI* botAI) : CastShamanEnchantMainHandAction(botAI, "windfury weapon") {}
};

// #382 (MoP-only; Wowhead mop-classic tooltip 8004, 2026-10-04): Healing Surge is the 5.x fast heal that replaces
// Lesser Healing Wave (cls.py shaman: 'lesser healing wave' NOT-LEARNABLE, 'healing surge' YES 8004/skill/L7).
// Same estAmount / efficiency as the Lesser Healing Wave actions it replaces.
class CastHealingSurgeAction : public CastHealingSpellAction
{
public:
    CastHealingSurgeAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "healing surge") {}
};

class CastHealingSurgeOnPartyAction : public HealPartyMemberAction
{
public:
    CastHealingSurgeOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "healing surge", 25.0f, HealingManaEfficiency::LOW)
    {
    }
};

// #382 (MoP-only; Wowhead mop-classic tooltip 77130, 2026-10-04): Purify Spirit (Restoration, level 18) "Removes all
// Curse and Magic effects from a friendly target". Mirrors the paladin Cleanse magic actions.
class CastPurifySpiritAction : public CastCureSpellAction
{
public:
    CastPurifySpiritAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify spirit") {}
};

class CastPurifySpiritMagicOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifySpiritMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify spirit", DISPEL_MAGIC)
    {
    }

    std::string const getName() override { return "purify spirit magic on party"; }
};

class CastPurifySpiritCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifySpiritCurseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify spirit", DISPEL_CURSE)
    {
    }

    std::string const getName() override { return "purify spirit curse on party"; }
};

class CastTotemAction : public CastBuffSpellAction
{
public:
    CastTotemAction(PlayerbotAI* botAI, std::string const spell, std::string const buffName = "",
                    float needLifeTime = 8.0f)
        : CastBuffSpellAction(botAI, spell), needLifeTime(needLifeTime)
    {
        if (buffName == "")
        {
            buff = spell;
        }
        else
        {
            buff = buffName;
        }
    }

    bool isUseful() override;

protected:
    float needLifeTime;
    std::string buff;
};

class CastStoneskinTotemAction : public CastTotemAction
{
public:
    CastStoneskinTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "stoneskin totem") {}
};

class CastEarthbindTotemAction : public CastTotemAction
{
public:
    CastEarthbindTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "earthbind totem") {}
};

class CastStrengthOfEarthTotemAction : public CastTotemAction
{
public:
    CastStrengthOfEarthTotemAction(PlayerbotAI* botAI)
        : CastTotemAction(botAI, "strength of earth totem", "strength of earth", 20.0f)
    {
    }
};

class CastManaSpringTotemAction : public CastTotemAction
{
public:
    CastManaSpringTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "mana spring totem", "mana spring", 0.0f) {}

    bool isUseful() override;
};

class CastManaTideTotemAction : public CastTotemAction
{
public:
    CastManaTideTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "mana tide totem") {}

    std::string const GetTargetName() override { return "self target"; }
};

class CastHealingStreamTotemAction : public CastTotemAction
{
public:
    CastHealingStreamTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "healing stream totem") {}
};

class CastCleansingTotemAction : public CastTotemAction
{
public:
    CastCleansingTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "cleansing totem") {}
    virtual bool isUseful();
};

class CastFlametongueTotemAction : public CastTotemAction
{
public:
    CastFlametongueTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "flametongue totem", "", 20.0f) {}

    bool isUseful() override;
};

class CastWindfuryTotemAction : public CastTotemAction
{
public:
    CastWindfuryTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "windfury totem") {}
};

class CastGraceOfAirTotemAction : public CastTotemAction
{
public:
    CastGraceOfAirTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "grace of air totem") {}
};

class CastSearingTotemAction : public CastTotemAction
{
public:
    CastSearingTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "searing totem", "", 0.0f) {}

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

class CastMagmaTotemAction : public CastTotemAction
{
public:
    CastMagmaTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "magma totem", "", 0.0f) {}

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

class CastFireNovaAction : public CastMeleeSpellAction
{
public:
    CastFireNovaAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "fire nova") {}
    bool isUseful() override;
};

class CastWindShearAction : public CastSpellAction
{
public:
    CastWindShearAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "wind shear") {}
};

class CastAncestralSpiritAction : public ResurrectPartyMemberAction
{
public:
    CastAncestralSpiritAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "ancestral spirit") {}
};

class CastPurgeAction : public CastSpellAction
{
public:
    CastPurgeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "purge") {}
};

class CastStormstrikeAction : public CastMeleeSpellAction
{
public:
    CastStormstrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "stormstrike") {}
};

class CastLavaLashAction : public CastMeleeSpellAction
{
public:
    CastLavaLashAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "lava lash") {}
};

class CastWaterBreathingAction : public CastBuffSpellAction
{
public:
    CastWaterBreathingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water breathing") {}
};

class CastWaterWalkingAction : public CastBuffSpellAction
{
public:
    CastWaterWalkingAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "water walking") {}
};

class CastWaterBreathingOnPartyAction : public BuffOnPartyAction
{
public:
    CastWaterBreathingOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "water breathing") {}
};

class CastWaterWalkingOnPartyAction : public BuffOnPartyAction
{
public:
    CastWaterWalkingOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "water walking") {}
};

class CastCleanseSpiritAction : public CastCureSpellAction
{
public:
    CastCleanseSpiritAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse spirit") {}
};

class CastCleanseSpiritPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritPoisonOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_POISON)
    {
    }

    std::string const getName() override { return "cleanse spirit poison on party"; }
};

class CastCleanseSpiritCurseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritCurseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_CURSE)
    {
    }

    std::string const getName() override { return "cleanse spirit curse on party"; }
};

class CastCleanseSpiritDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseSpiritDiseaseOnPartyAction(PlayerbotAI* botAI)
        : CurePartyMemberAction(botAI, "cleanse spirit", DISPEL_DISEASE)
    {
    }

    std::string const getName() override { return "cleanse spirit disease on party"; }
};

class CastFlameShockAction : public CastDebuffSpellAction
{
public:
    CastFlameShockAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "flame shock", true, 6.0f) {}
};

class CastEarthShockAction : public CastSpellAction
{
public:
    CastEarthShockAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "earth shock") {}
};

class CastFrostShockAction : public CastSnareSpellAction
{
public:
    CastFrostShockAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "frost shock") {}
};

class CastChainLightningAction : public CastSpellAction
{
public:
    CastChainLightningAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chain lightning") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastLightningBoltAction : public CastSpellAction
{
public:
    CastLightningBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "lightning bolt") {}
};

class CastThunderstormAction : public CastSpellAction
{
public:
    CastThunderstormAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "thunderstorm") {}
};

class CastHeroismAction : public CastBuffSpellAction
{
public:
    CastHeroismAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "heroism") {}
};

class CastBloodlustAction : public CastBuffSpellAction
{
public:
    CastBloodlustAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "bloodlust") {}
};

class CastElementalMasteryAction : public CastBuffSpellAction
{
public:
    CastElementalMasteryAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "elemental mastery") {}
};

class CastWindShearOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastWindShearOnEnemyHealerAction(PlayerbotAI* botAI) : CastSpellOnEnemyHealerAction(botAI, "wind shear") {}
};

CURE_ACTION(CastCurePoisonActionSham, "cure disease");
CURE_PARTY_ACTION(CastCurePoisonOnPartyActionSham, "cure poison", DISPEL_POISON);

CURE_ACTION(CastCureDiseaseActionSham, "cure disease");
CURE_PARTY_ACTION(CastCureDiseaseOnPartyActionSham, "cure disease", DISPEL_DISEASE);

class CastLavaBurstAction : public CastSpellAction
{
public:
    CastLavaBurstAction(PlayerbotAI* ai) : CastSpellAction(ai, "lava burst") {}
};

class CastEarthShieldOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastEarthShieldOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "earth shield", false) {}
};

class CastTotemOfWrathAction : public CastTotemAction
{
public:
    CastTotemOfWrathAction(PlayerbotAI* ai) : CastTotemAction(ai, "totem of wrath") {}
    virtual std::string const GetTargetName() override { return "self target"; }
    virtual bool isUseful() override { return CastTotemAction::isUseful(); }
};

class CastFireElementalTotemAction : public CastTotemAction
{
public:
    CastFireElementalTotemAction(PlayerbotAI* ai) : CastTotemAction(ai, "fire elemental totem", "", 0.0f) {}
    virtual std::string const GetTargetName() override { return "self target"; }
    virtual bool isUseful() override { return CastTotemAction::isUseful(); }
};

class CastFireElementalTotemMeleeAction : public CastTotemAction
{
public:
    CastFireElementalTotemMeleeAction(PlayerbotAI* ai) : CastTotemAction(ai, "fire elemental totem", "", 0.0f) {}
    virtual std::string const GetTargetName() override { return "self target"; }
    virtual bool isUseful() override
    {
        Unit* target = AI_VALUE(Unit*, "current target");
        if (!target || !bot->IsWithinMeleeRange(target))
            return false;
        return CastTotemAction::isUseful();
    }
};

class CastWrathOfAirTotemAction : public CastTotemAction
{
public:
    CastWrathOfAirTotemAction(PlayerbotAI* ai) : CastTotemAction(ai, "wrath of air totem", "wrath of air totem") {}
};

class CastShamanisticRageAction : public CastBuffSpellAction
{
public:
    CastShamanisticRageAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shamanistic rage") {}
};

class CastFeralSpiritAction : public CastSpellAction
{
public:
    CastFeralSpiritAction(PlayerbotAI* ai) : CastSpellAction(ai, "feral spirit") {}
};

// #408 (AC parity, AC ShamanActions.h:432-437): Tremor Totem (earth slot).
class CastTremorTotemAction : public CastTotemAction
{
public:
    CastTremorTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "tremor totem") {}
};

// #408 (AC parity, AC ShamanActions.h:446-452): Stoneclaw Totem (earth slot, low-level fallback).
class CastStoneclawTotemAction : public CastTotemAction
{
public:
    CastStoneclawTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "stoneclaw totem") {}
};

// #382 (MoP-only; Wowhead mop-classic tooltip 108270, 2026-10-04; cls.py shaman 'stoneclaw totem' NO-SUCH-NAME,
// 'stone bulwark totem' YES 108270/talent): Stoneclaw Totem no longer exists in 5.4.8. Stone Bulwark Totem is the
// 5.x earth-slot totem that grants the shaman a damage-absorb shield, the same emergency role (Stoneclaw absorbed
// damage in WotLK), so it replaces Stoneclaw on the AC "low health" edge. It is a talent: unknown -> not castable.
class CastStoneBulwarkTotemAction : public CastTotemAction
{
public:
    CastStoneBulwarkTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "stone bulwark totem") {}
};

// #408 (AC parity, AC ShamanActions.h:485-490): Frost Resistance Totem (fire slot).
class CastFrostResistanceTotemAction : public CastTotemAction
{
public:
    CastFrostResistanceTotemAction(PlayerbotAI* botAI)
        : CastTotemAction(botAI, "frost resistance totem", "frost resistance")
    {
    }
};

// #408 (AC parity, AC ShamanActions.h:549-554): Fire Resistance Totem (water slot).
class CastFireResistanceTotemAction : public CastTotemAction
{
public:
    CastFireResistanceTotemAction(PlayerbotAI* botAI)
        : CastTotemAction(botAI, "fire resistance totem", "fire resistance")
    {
    }
};

// #408 (AC parity, AC ShamanActions.h:570-575): Nature Resistance Totem (air slot).
class CastNatureResistanceTotemAction : public CastTotemAction
{
public:
    CastNatureResistanceTotemAction(PlayerbotAI* botAI)
        : CastTotemAction(botAI, "nature resistance totem", "nature resistance")
    {
    }
};

// #408 (AC parity, AC ShamanActions.h:556-561): Grounding Totem (air slot, low-level fallback).
class CastGroundingTotemAction : public CastTotemAction
{
public:
    CastGroundingTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "grounding totem") {}
};

// #408 (AC parity, AC ShamanActions.h:404-416): Call of the Elements + Totemic Recall.
class CastCallOfTheElementsAction : public CastSpellAction
{
public:
    CastCallOfTheElementsAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "call of the elements") {}
};

class CastTotemicRecallAction : public CastSpellAction
{
public:
    CastTotemicRecallAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "totemic recall") {}
};

// #408 (AC parity, AC ShamanActions.h:147-153): Spirit Walk (guardian wolf spell).
class CastSpiritWalkAction : public Action
{
public:
    CastSpiritWalkAction(PlayerbotAI* botAI) : Action(botAI, "spirit walk") {}

    bool Execute(Event event) override;
};

// #431 (AC parity, AC ShamanActions.h:577-710): "set <totem>" actions used by the per-totem strategies.
// AC's SetTotemAction writes the highest-rank totem spell the bot knows onto the Call of the Elements totem bar
// (bot->addActionButton(TOTEM_BAR_SLOT_*, ...)). MoP 5.4.8 has no such bar: MAX_ACTION_BUTTONS is 132
// (Player.h:~414), so AC's slots 132-135 are rejected by Player::IsActionButtonDataValid. The same intent -
// "make this element's slot hold the totem the strategy assigned" - is carried out here by casting the totem into
// its element summon slot, which replaces any other totem standing there. SetTotemTrigger (ShamanTriggers.cpp)
// stays inactive once the slot holds the assigned totem, so the action does not repeat.
class SetTotemAction : public Action
{
public:
    SetTotemAction(PlayerbotAI* botAI, std::string const& totemName) : Action(botAI, "set " + totemName), totemName(totemName) {}

    bool isUseful() override;
    bool Execute(Event event) override;

private:
    std::string totemName;
};

class SetStrengthOfEarthTotemAction : public SetTotemAction
{
public:
    SetStrengthOfEarthTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "strength of earth totem") {}
};

class SetStoneskinTotemAction : public SetTotemAction
{
public:
    SetStoneskinTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "stoneskin totem") {}
};

class SetTremorTotemAction : public SetTotemAction
{
public:
    SetTremorTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "tremor totem") {}
};

class SetEarthbindTotemAction : public SetTotemAction
{
public:
    SetEarthbindTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "earthbind totem") {}
};

class SetSearingTotemAction : public SetTotemAction
{
public:
    SetSearingTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "searing totem") {}
};

class SetMagmaTotemAction : public SetTotemAction
{
public:
    SetMagmaTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "magma totem") {}
};

class SetFlametongueTotemAction : public SetTotemAction
{
public:
    SetFlametongueTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "flametongue totem") {}
};

class SetTotemOfWrathAction : public SetTotemAction
{
public:
    SetTotemOfWrathAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "totem of wrath") {}
};

class SetFrostResistanceTotemAction : public SetTotemAction
{
public:
    SetFrostResistanceTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "frost resistance totem") {}
};

class SetHealingStreamTotemAction : public SetTotemAction
{
public:
    SetHealingStreamTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "healing stream totem") {}
};

class SetManaSpringTotemAction : public SetTotemAction
{
public:
    SetManaSpringTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "mana spring totem") {}
};

class SetCleansingTotemAction : public SetTotemAction
{
public:
    SetCleansingTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "cleansing totem") {}
};

class SetFireResistanceTotemAction : public SetTotemAction
{
public:
    SetFireResistanceTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "fire resistance totem") {}
};

class SetWrathOfAirTotemAction : public SetTotemAction
{
public:
    SetWrathOfAirTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "wrath of air totem") {}
};

class SetWindfuryTotemAction : public SetTotemAction
{
public:
    SetWindfuryTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "windfury totem") {}
};

class SetNatureResistanceTotemAction : public SetTotemAction
{
public:
    SetNatureResistanceTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "nature resistance totem") {}
};

class SetGroundingTotemAction : public SetTotemAction
{
public:
    SetGroundingTotemAction(PlayerbotAI* botAI) : SetTotemAction(botAI, "grounding totem") {}
};

// ---- Rotation lane 3 (#858, epic #854): MoP shaman rows, AC style.

// (spec Elemental Blast, 117014).
class CastElementalBlastAction : public CastSpellAction
{
public:
    CastElementalBlastAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "elemental blast") {}
};

// (Unleash Elements 73680).
// Wave-74 fix (#858): the 5.4.8 SpellScript (spell_shaman.cpp spell_sha_unleash_elements, CheckEnchant) wants an ENEMY
// target for the Flametongue / Windfury / Frostbrand / Rockbiter imbues (elemental, enhancement) and a FRIENDLY target for
// Earthliving (restoration); a self cast with a hostile imbue returned SPELL_FAILED_BAD_TARGETS (13) 13x in the w74 ele leg.
// Restoration keeps the MoP self target, the damage specs cast on the current target.
class CastUnleashElementsAction : public CastSpellAction
{
public:
    CastUnleashElementsAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "unleash elements") {}
    std::string const GetTargetName() override;
};

// (Earthquake 61882, ground spell at the current target, >= 4 enemies within 8 yd).
class CastEarthquakeAction : public CastSpellAction
{
public:
    CastEarthquakeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "earthquake") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

// (Ascendance 114049, self).
class CastAscendanceAction : public CastBuffSpellAction
{
public:
    CastAscendanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ascendance") {}
};

// (class Ancestral Swiftness 16188).
class CastAncestralSwiftnessAction : public CastBuffSpellAction
{
public:
    CastAncestralSwiftnessAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ancestral swiftness") {}
};

// (Ancestral Guidance 108281 at health <= 50 %).
class CastAncestralGuidanceAction : public CastBuffSpellAction
{
public:
    CastAncestralGuidanceAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "ancestral guidance") {}
};

// (Healing Tide Totem 108280, water slot, at health <= 50 %).
class CastHealingTideTotemAction : public CastTotemAction
{
public:
    CastHealingTideTotemAction(PlayerbotAI* botAI) : CastTotemAction(botAI, "healing tide totem", "", 0.0f) {}
    std::string const GetTargetName() override { return "self target"; }
};

// (class Primal Strike 73899, levels 1-9 filler).
class CastPrimalStrikeAction : public CastMeleeSpellAction
{
public:
    CastPrimalStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "primal strike") {}
};

// (Healing Rain 73920, ground heal on hurt group members).
class CastHealingRainAction : public CastAoeHealSpellAction
{
public:
    CastHealingRainAction(PlayerbotAI* botAI)
        : CastAoeHealSpellAction(botAI, "healing rain", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

// (Greater Healing Wave 77472 at target health <= 60 %).
class CastGreaterHealingWaveOnPartyAction : public HealPartyMemberAction
{
public:
    CastGreaterHealingWaveOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "greater healing wave", 70.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

// (Wind Shear on the victim and on any casting enemy in range). Lane 0 shared base
// class (GenericSpellActions.h, issue #855) targets the "enemy caster target" value.
class CastWindShearOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastWindShearOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "wind shear") {}
};

#endif
