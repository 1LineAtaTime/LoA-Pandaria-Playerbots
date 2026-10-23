/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINACTIONS_H
#define _PLAYERBOT_PALADINACTIONS_H

#include "AiObject.h"
#include "GenericSpellActions.h"
#include "SharedDefines.h"

class PlayerbotAI;
class Unit;

// seals
BUFF_ACTION(CastSealOfRighteousnessAction, "seal of righteousness");
BUFF_ACTION(CastSealOfCommandAction, "seal of command");
BUFF_ACTION(CastSealOfInsightAction, "seal of insight");

// seals - 5.4.8 remaining paladin seals (AC PaladinActions.h:15-19). AC's "seal of vengeance" is
// deliberately not ported: that spell was renamed to Seal of Corruption in patch 3.1, so 5.4.8
// spell names only know "seal of corruption" (registered below).
BUFF_ACTION(CastSealOfJusticeAction, "seal of justice");
BUFF_ACTION(CastSealOfLightAction, "seal of light");
BUFF_ACTION(CastSealOfWisdomAction, "seal of wisdom");
BUFF_ACTION(CastSealOfCorruptionAction, "seal of corruption");
// #525 (AC DpsPaladinStrategy.cpp:130 "seal of corruption"): Seal of Truth (31801) is the 5.x
// successor of Seal of Corruption/Vengeance (Wowhead 31801 tooltip 2026-10-04: "Replaces Seal of Command").
BUFF_ACTION(CastSealOfTruthAction, "seal of truth");

// judgements
// #525: the 5.4.8 spell is spelled "Judgment" (20271, Spell.dbc / Wowhead 2026-10-04). SpellIdValue
// matches names exactly, so the old "judgement" string never resolved and Judgment was never cast.
// The registered action key stays "judgement" (AC "judgement of light/wisdom" edges map onto it).
SPELL_ACTION(CastJudgementAction, "judgment");

// #525: Holy Power spenders with no AC equivalent (WotLK has no Holy Power).
// #735 (owner 2026-10-08: AC's methodology): PlayerbotAI::CanCastSpell checks with
// TRIGGERED_IGNORE_POWER_AND_REAGENT_COST (as AC does), so a spender's Holy Power cost is NOT checked there - a
// spender cast at 0 Holy Power failed SPELL_FAILED_NO_POWER (88). As AC gates its combo-point finishers (plain
// actions, AC RogueFinishingActions.h:14-42, driven by "combo points N available" triggers, AC
// CombatRogueStrategy.cpp:118-126), the spenders stay plain actions and the strategies fire them from the
// "holy power N available" / "divine purpose" triggers (PaladinTriggers.h).
// Templar's Verdict 85256: 3 Holy Power, Ret melee finisher (Wowhead 2026-10-04).
MELEE_ACTION(CastTemplarsVerdictAction, "templar's verdict");
// Inquisition 84963: Ret buff, +30% holy damage, consumes up to 3 Holy Power (Wowhead 2026-10-04).
BUFF_ACTION(CastInquisitionAction, "inquisition");

// auras
BUFF_ACTION(CastDevotionAuraAction, "devotion aura");
BUFF_ACTION(CastRetributionAuraAction, "retribution aura");
BUFF_ACTION(CastConcentrationAuraAction, "concentration aura");
BUFF_ACTION(CastCrusaderAuraAction, "crusader aura");
BUFF_ACTION(CastSanctityAuraAction, "sanctity aura");

// resistance auras (AC PaladinActions.h:33-35) - all three still exist in 5.4.8
BUFF_ACTION(CastShadowResistanceAuraAction, "shadow resistance aura");
BUFF_ACTION(CastFrostResistanceAuraAction, "frost resistance aura");
BUFF_ACTION(CastFireResistanceAuraAction, "fire resistance aura");
SPELL_ACTION(CastHolyShockAction, "holy shock");

// consecration
MELEE_ACTION(CastConsecrationAction, "consecration");

// repentance
SNARE_ACTION(CastRepentanceSnareAction, "repentance");
DEBUFF_ACTION(CastRepentanceAction, "repentance");
ENEMY_HEALER_ACTION(CastRepentanceOnHealerAction, "repentance");

// hamme of wrath
SPELL_ACTION(CastHammerOfWrathAction, "hammer of wrath");

// buffs
BUFF_ACTION(CastDivineFavorAction, "divine favor");

// blessings

// fury
BUFF_ACTION(CastRighteousFuryAction, "righteous fury");

class CastDivineStormAction : public CastMeleeSpellAction
{
public:
    CastDivineStormAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "divine storm") {}
};

class CastCrusaderStrikeAction : public CastMeleeSpellAction
{
public:
    CastCrusaderStrikeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "crusader strike") {}
    bool isUseful() override;
};

class CastSealSpellAction : public CastBuffSpellAction
{
public:
    CastSealSpellAction(PlayerbotAI* botAI, std::string const name) : CastBuffSpellAction(botAI, name) {}

    bool isUseful() override;
};

class CastBlessingOfMightAction : public CastBuffSpellAction
{
public:
    CastBlessingOfMightAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of might") {}
};

class CastBlessingOnPartyAction : public BuffOnPartyAction
{
public:
    CastBlessingOnPartyAction(PlayerbotAI* botAI, std::string const name) : BuffOnPartyAction(botAI, name), name(name)
    {
    }

private:
    std::string name;
};

class CastBlessingOfMightOnPartyAction : public BuffOnPartyAction
{
public:
    CastBlessingOfMightOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "blessing of might") {}

    std::string const getName() override { return "blessing of might on party"; }
};

class CastBlessingOfKingsAction : public CastBuffSpellAction
{
public:
    CastBlessingOfKingsAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "blessing of kings") {}
};

class CastBlessingOfKingsOnPartyAction : public CastBlessingOnPartyAction
{
public:
    CastBlessingOfKingsOnPartyAction(PlayerbotAI* botAI) : CastBlessingOnPartyAction(botAI, "blessing of kings") {}

    std::string const getName() override { return "blessing of kings on party"; }
};

class CastHolyLightAction : public CastHealingSpellAction
{
public:
    CastHolyLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "holy light") {}
};

class CastHolyShockOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyShockOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "holy shock", 25.0f, HealingManaEfficiency::LOW)
    {
    }
};

class CastHolyLightOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyLightOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "holy light", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastFlashOfLightAction : public CastHealingSpellAction
{
public:
    CastFlashOfLightAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "flash of light") {}
};

// #521: "word of glory on party" was used by the nc strategy but never registered. Word of Glory 85673
// (L9, Holy Power heal; cls.py paladin) - same HealPartyMemberAction shape as Flash of Light below.
class CastWordOfGloryOnPartyAction : public HealPartyMemberAction
{
public:
    CastWordOfGloryOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "word of glory", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastFlashOfLightOnPartyAction : public HealPartyMemberAction
{
public:
    CastFlashOfLightOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "flash of light", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastLayOnHandsAction : public CastHealingSpellAction
{
public:
    CastLayOnHandsAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "lay on hands") {}
};

class CastLayOnHandsOnPartyAction : public HealPartyMemberAction
{
public:
    CastLayOnHandsOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "lay on hands") {}
};

class CastDivineProtectionAction : public CastBuffSpellAction
{
public:
    CastDivineProtectionAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine protection") {}
};

class CastDivineProtectionOnPartyAction : public HealPartyMemberAction
{
public:
    CastDivineProtectionOnPartyAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "divine protection") {}

    std::string const getName() override { return "divine protection on party"; }
};

class CastDivineShieldAction : public CastBuffSpellAction
{
public:
    CastDivineShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine shield") {}
    // rotation lane 6 (#861): prio group 5 - never while carrying a PvP objective.
    bool isUseful() override;
};

class CastHolyWrathAction : public CastMeleeSpellAction
{
public:
    CastHolyWrathAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "holy wrath") {}
};

class CastHammerOfJusticeAction : public CastMeleeSpellAction
{
public:
    CastHammerOfJusticeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of justice") {}
};

class CastHammerOfTheRighteousAction : public CastMeleeSpellAction
{
public:
    CastHammerOfTheRighteousAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hammer of the righteous") {}
};

class CastPurifyPoisonAction : public CastCureSpellAction
{
public:
    CastPurifyPoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") {}
};

class CastPurifyDiseaseAction : public CastCureSpellAction
{
public:
    CastPurifyDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "purify") {}
};

class CastPurifyPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifyPoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_POISON) {}

    std::string const getName() override { return "purify poison on party"; }
};

class CastPurifyDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastPurifyDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "purify", DISPEL_DISEASE) {}

    std::string const getName() override { return "purify disease on party"; }
};

class CastHandOfReckoningAction : public CastSpellAction
{
public:
    // #525: the 5.4.8 taunt is "Reckoning" (62124; "Hand of Reckoning" is the WotLK name and does not
    // exist in 5.4.8 Spell.dbc - cls.py 2026-10-04; Wowhead 62124). Action key stays "hand of reckoning".
    CastHandOfReckoningAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "reckoning") {}
    // #835: never taunt a player (BG lose-aggro edge can see the old target, then dps assist switches to a player).
    bool isUseful() override { Unit* t = GetTarget(); return t && !t->IsPlayer() && CastSpellAction::isUseful(); }
};

class CastRighteousDefenseAction : public CastSpellAction
{
public:
    CastRighteousDefenseAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "righteous defense") {}
    virtual Unit* GetTarget() override;
};

class CastCleansePoisonAction : public CastCureSpellAction
{
public:
    CastCleansePoisonAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

class CastCleanseDiseaseAction : public CastCureSpellAction
{
public:
    CastCleanseDiseaseAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

// #383: "cleanse magic" is not a 5.4.8 spell name (cls.py: 55598 NOT-LEARNABLE) - Cleanse Magic merged into
// Cleanse (4987, L20) in 4.0.1, so every cleanse action here already casts "cleanse" (DISPEL_* picks the type).
class CastCleanseMagicAction : public CastCureSpellAction
{
public:
    CastCleanseMagicAction(PlayerbotAI* botAI) : CastCureSpellAction(botAI, "cleanse") {}
};

class CastCleansePoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleansePoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_POISON) {}

    std::string const getName() override { return "cleanse poison on party"; }
};

class CastCleanseDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_DISEASE) {}

    std::string const getName() override { return "cleanse disease on party"; }
};

class CastCleanseMagicOnPartyAction : public CurePartyMemberAction
{
public:
    CastCleanseMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "cleanse", DISPEL_MAGIC) {}

    std::string const getName() override { return "cleanse magic on party"; }
};

BEGIN_SPELL_ACTION(CastAvengersShieldAction, "avenger's shield")
END_SPELL_ACTION()

BEGIN_SPELL_ACTION(CastExorcismAction, "exorcism")
END_SPELL_ACTION()

class CastHolyShieldAction : public CastBuffSpellAction
{
public:
    CastHolyShieldAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "holy shield") {}
};

class CastRedemptionAction : public ResurrectPartyMemberAction
{
public:
    CastRedemptionAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "redemption") {}
};

class CastHammerOfJusticeOnEnemyHealerAction : public CastSpellOnEnemyHealerAction
{
public:
    CastHammerOfJusticeOnEnemyHealerAction(PlayerbotAI* botAI)
        : CastSpellOnEnemyHealerAction(botAI, "hammer of justice")
    {
    }
};

class CastHammerOfJusticeSnareAction : public CastSnareSpellAction
{
public:
    CastHammerOfJusticeSnareAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "hammer of justice") {}
};

// #357 slice 6 (AC PaladinActions.h:352) - 1:1
class CastSenseUndeadAction : public CastBuffSpellAction
{
public:
    CastSenseUndeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "sense undead") {}
};

class CastTurnUndeadAction : public CastBuffSpellAction
{
public:
    // #383 (AC PaladinActions.h Turn Undead): Turn Undead was replaced by Turn Evil (10326, L46) in
    // patch 4.0.1 (cls.py 2026-10-04; Wowhead 10326 "Undead, Aberration or Demon target flees").
    CastTurnUndeadAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "turn evil") {}

    Value<Unit*>* GetTargetValue() override;
};

class CastBlessingOfProtectionProtectAction : public CastProtectSpellAction
{
public:
    // #383 (AC GenericPaladinStrategy.cpp "protect party member"): Blessing of Protection became Hand of
    // Protection (1022, L48) in patch 4.0.1 (cls.py 2026-10-04; Wowhead 1022). Action key unchanged.
    CastBlessingOfProtectionProtectAction(PlayerbotAI* botAI) : CastProtectSpellAction(botAI, "hand of protection") {}

    std::string const getName() override { return "blessing of protection on party"; }
};

// #163 S8 (AC parity, Ai/Class/Paladin/Actions/PaladinActions.h:363-368): free the first snared
// party member (or self, if the bot itself is snared and ungrouped/no one else can help). AC also
// tracks "any Paladin Hand already on this target from this caster" via a shared Hand-family helper
// (HasAnyPaladinHandFromCaster) this fork doesn't have (Hand of Protection/Salvation/Sacrifice
// aren't ported); substituted with a plain "does the target already have hand of freedom" check.
class CastHandOfFreedomOnPartyAction : public CastBuffSpellAction
{
public:
    CastHandOfFreedomOnPartyAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "hand of freedom") {}

    std::string const getName() override { return "hand of freedom on party"; }
    Unit* GetTarget() override;
    Value<Unit*>* GetTargetValue() override;
    bool isUseful() override;
};

class CastDivinePleaAction : public CastBuffSpellAction
{
public:
    CastDivinePleaAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "divine plea") {}
};

class ShieldOfRighteousnessAction : public CastMeleeSpellAction
{
public:
    // #525: the 5.4.8 spell is "Shield of the Righteous" (53600, 3 Holy Power; Wowhead 2026-10-04).
    // "shield of righteousness" was the WotLK name and never resolved. Action key unchanged.
    ShieldOfRighteousnessAction(PlayerbotAI* ai) : CastMeleeSpellAction(ai, "shield of the righteous") {}
};

class CastBeaconOfLightOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastBeaconOfLightOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "beacon of light", true) {}
};

class CastSacredShieldOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastSacredShieldOnMainTankAction(PlayerbotAI* ai) : BuffOnMainTankAction(ai, "sacred shield", false) {}
};

class CastAvengingWrathAction : public CastBuffSpellAction
{
public:
    CastAvengingWrathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "avenging wrath") {}
};

class CastDivineIlluminationAction : public CastBuffSpellAction
{
public:
    CastDivineIlluminationAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine illumination") {}
};

class CastDivineSacrificeAction : public CastBuffSpellAction
{
public:
    CastDivineSacrificeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "divine sacrifice") {}
    bool isUseful() override;
};

class CastCancelDivineSacrificeAction : public Action
{
public:
    CastCancelDivineSacrificeAction(PlayerbotAI* botAI) : Action(botAI, "cancel divine sacrifice") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// ---- rotation lane 6 (#861, epic #854): MoP paladin rotations ----

// Rebuke 96231. Prio 10/2 on the current target and 10/1 on any casting enemy. AC has no
// Rebuke (WotLK Paladin has no interrupt); the action was registered as a node only and no strategy used it.
class CastRebukeAction : public CastMeleeSpellAction
{
public:
    CastRebukeAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "rebuke") {}
};

// shared contract (lane 0 #855): CastSpellOnEnemyCasterAction targets the "enemy caster target" value.
class CastRebukeOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastRebukeOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "rebuke") {}
};

// Cooldowns (self buffs). Holy Avenger 105809 (Ret, row 7016), Guardian of Ancient Kings (Holy 86669 row 6503,
// Prot/Ret 86698 rows 6617/7021; the name resolves per spec), Ardent Defender 31850 (Prot, row 6618).
BUFF_ACTION(CastHolyAvengerAction, "holy avenger");
BUFF_ACTION(CastGuardianOfAncientKingsAction, "guardian of ancient kings");
BUFF_ACTION(CastArdentDefenderAction, "ardent defender");

// Execution Sentence 114157. Ret: on the current target (row 7005). Holy: on an injured tank <= 60% (row 6506), below.
SPELL_ACTION(CastExecutionSentenceAction, "execution sentence");

// Holy Power heals. Eternal Flame 114163 costs 3 Holy Power (or a Divine Purpose proc, aura 90174). As in #735 the cast
// check ignores the power cost, so the gate lives here (holy and prot).
class CastEternalFlameAction : public CastHealingSpellAction
{
public:
    CastEternalFlameAction(PlayerbotAI* botAI) : CastHealingSpellAction(botAI, "eternal flame") {}
    bool isUseful() override;
};

class CastEternalFlameOnPartyAction : public HealPartyMemberAction
{
public:
    CastEternalFlameOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "eternal flame", 25.0f, HealingManaEfficiency::HIGH)
    {
    }
    bool isUseful() override;
};

// Divine Light 82326 (row 6512) and Holy Radiance 82327 (row 6510, 3+ members < 50% within 10 yd of the target).
class CastDivineLightOnPartyAction : public HealPartyMemberAction
{
public:
    CastDivineLightOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "divine light", 50.0f, HealingManaEfficiency::MEDIUM)
    {
    }
};

class CastHolyRadianceOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyRadianceOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "holy radiance", 30.0f, HealingManaEfficiency::LOW)
    {
    }
};

// Execution Sentence on the tank (role tank and health <= 60%).
class CastExecutionSentenceOnPartyAction : public HealPartyMemberAction
{
public:
    CastExecutionSentenceOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "execution sentence", 25.0f, HealingManaEfficiency::LOW)
    {
    }
    bool isUseful() override;
};

#endif
