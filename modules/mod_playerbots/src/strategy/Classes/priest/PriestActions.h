/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PRIESTACTIONS_H
#define _PLAYERBOT_PRIESTACTIONS_H

#include "GenericSpellActions.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"

class PlayerbotAI;

// disc
BUFF_ACTION(CastPowerWordFortitudeAction, "power word: fortitude");
BUFF_PARTY_ACTION(CastPowerWordFortitudeOnPartyAction, "power word: fortitude");
BUFF_PARTY_ACTION(CastPrayerOfFortitudeOnPartyAction, "prayer of fortitude");
BUFF_ACTION(CastPowerWordShieldAction, "power word: shield");

BUFF_ACTION(CastInnerFireAction, "inner fire");
CURE_ACTION(CastDispelMagicAction, "dispel magic");
CURE_PARTY_ACTION(CastDispelMagicOnPartyAction, "dispel magic", DISPEL_MAGIC);
SPELL_ACTION(CastDispelMagicOnTargetAction, "dispel magic");
CC_ACTION(CastShackleUndeadAction, "shackle undead");
SPELL_ACTION_U(CastManaBurnAction, "mana burn",
               AI_VALUE2(uint8, "mana", "self target") < 50 && AI_VALUE2(uint8, "mana", "current target") >= 20);
BUFF_ACTION(CastLevitateAction, "levitate");
BUFF_ACTION(CastDivineSpiritAction, "divine spirit");
BUFF_PARTY_ACTION(CastDivineSpiritOnPartyAction, "divine spirit");
BUFF_PARTY_ACTION(CastPrayerOfSpiritOnPartyAction, "prayer of spirit");
// disc 2.4.3
SPELL_ACTION(CastMassDispelAction, "mass dispel");

// disc talents
BUFF_ACTION(CastPowerInfusionAction, "power infusion");
BUFF_PARTY_ACTION(CastPowerInfusionOnPartyAction, "power infusion");
BUFF_ACTION(CastInnerFocusAction, "inner focus");
// disc 2.4.3 talents
BUFF_ACTION(CastPainSuppressionAction, "pain suppression");
PROTECT_ACTION(CastPainSuppressionProtectAction, "pain suppression");

// holy
HEAL_ACTION(CastLesserHealAction, "lesser heal");
HEAL_ACTION(CastHealAction, "heal");
HEAL_ACTION(CastGreaterHealAction, "greater heal");
HEAL_ACTION(CastFlashHealAction, "flash heal");
HEAL_ACTION(CastRenewAction, "renew");
HEAL_PARTY_ACTION(CastLesserHealOnPartyAction, "lesser heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastHealOnPartyAction, "heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastGreaterHealOnPartyAction, "greater heal", 50.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastPowerWordShieldOnPartyAction, "power word: shield", 15.0f, HealingManaEfficiency::VERY_HIGH);
HEAL_PARTY_ACTION(CastFlashHealOnPartyAction, "flash heal", 15.0f, HealingManaEfficiency::LOW);
HEAL_PARTY_ACTION(CastRenewOnPartyAction, "renew", 15.0f, HealingManaEfficiency::VERY_HIGH);
// HEAL_PARTY_ACTION(CastPrayerOfMendingAction, "prayer of mending", 10.0f, HealingManaEfficiency::HIGH);
class CastPrayerOfMendingAction : public HealPartyMemberAction
{
public:
    CastPrayerOfMendingAction(PlayerbotAI* botAI) : HealPartyMemberAction(botAI, "prayer of mending", 10.0f, HealingManaEfficiency::HIGH, false) {}
};

HEAL_PARTY_ACTION(CastBindingHealAction, "binding heal", 15.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastPrayerOfHealingAction, "prayer of healing", 15.0f, HealingManaEfficiency::MEDIUM);
// AOE_HEAL_ACTION(CastCircleOfHealingAction, "circle of healing", 15.0f, HealingManaEfficiency::HIGH);
class CastCircleOfHealingAction : public HealPartyMemberAction
{
public:
    CastCircleOfHealingAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "circle of healing", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
};
// Lane E (#863, MoP rotation row 25721 "Lightwell(c)", self, in combat, always; 25717 on injured friendlies):
// Lightwell 126135 is a 3 min cooldown dest-location summon. The old CastAoeHealSpellAction wrapper was never reachable
// (no strategy edge), and its heal-candidate gate does not apply to a summon. Cast it at the bot's own position.
class CastLightwellAction : public CastSpellAction
{
public:
    CastLightwellAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "lightwell") {}
    std::string const GetTargetName() override { return "self target"; }
};

SPELL_ACTION(CastSmiteAction, "smite");
SPELL_ACTION(CastHolyNovaAction, "holy nova");

// AC: RESS_ACTION(CastResurrectionAction, "resurrection"). #790: Resurrection 2006 cannot be cast in the Spirit of
// Redemption form (SPELL_FAILED_NOT_SHAPESHIFT 71, which CanCastSpell lets through like AC), and in 5.4.8 every Holy
// priest has the spirit (20711 baseline). The "party member dead" edge fired every tick of the 15 s spirit after a
// wipe (wave 35 SM Durand, Gineriana: 55 PREREQ/FAILED on the dead master). Same guard as AC's
// IsDisallowedShapeshiftForm (Ai/Base/Actions/NonCombatActions.cpp:27-30), on the macro's class otherwise unchanged.
class CastResurrectionAction : public ResurrectPartyMemberAction
{
public:
    CastResurrectionAction(PlayerbotAI* botAI) : ResurrectPartyMemberAction(botAI, "resurrection") {}

    bool isUseful() override { return !bot->HasSpiritOfRedemptionAura() && ResurrectPartyMemberAction::isUseful(); }
};

CURE_ACTION(CastCureDiseaseAction, "cure disease");
CURE_PARTY_ACTION(CastCureDiseaseOnPartyAction, "cure disease", DISPEL_DISEASE);
CURE_ACTION(CastAbolishDiseaseAction, "abolish disease");
CURE_PARTY_ACTION(CastAbolishDiseaseOnPartyAction, "abolish disease", DISPEL_DISEASE);
// #524: cure disease / abolish disease do not exist in 5.4.8 (cls.py priest: NOT-LEARNABLE / NO-SUCH-NAME). Purify
// (527, Disc/Holy L22) "Dispels harmful effects on the target, removing all Magic and Disease effects"
// (Wowhead mop-classic tooltip spell 527, 2026-10-04). Used by PriestCureStrategy for the disease edges.
CURE_ACTION(CastPurifyAction, "purify");
CURE_PARTY_ACTION(CastPurifyOnPartyAction, "purify", DISPEL_DISEASE);
// #524: the Magic half of Purify for party members (replaces friendly "dispel magic on party", see
// PriestCureStrategy).
CURE_PARTY_ACTION(CastPurifyMagicOnPartyAction, "purify", DISPEL_MAGIC);

// DEBUFF_CHECKISOWNER_ACTION(CastHolyFireAction, "holy fire");
class CastHolyFireAction : public CastDebuffSpellAction
{
public:
    CastHolyFireAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "holy fire", true, 0.0f) {}
};

// shadow 2.4.3
// BUFF_ACTION(CastShadowfiendAction, "shadowfiend");
SPELL_ACTION(CastShadowWordDeathAction, "shadow word: death");

// shadow
DEBUFF_CHECKISOWNER_ACTION(CastPowerWordPainAction, "shadow word: pain");
DEBUFF_ENEMY_ACTION(CastPowerWordPainOnAttackerAction, "shadow word: pain");
SPELL_ACTION(CastMindBlastAction, "mind blast");
// Psychic Scream (8122): PBAoE fear. Lane 8 / #863 (MoP rotation row 503 class + 25800/25801 shadow, prio group
// 2/10 and 0/11: target within 3-4 yd). AC only had the action registered (its "enemy is close" edge is commented out);
// MoP 5.4.8 fires it on enemy PLAYERS only, so it is gated on a BG/arena map, a player target and the lane-0 "is cc"
// guard (never fear what is already stunned/feared/rooted). isUseful is defined in PriestActions.cpp.
class CastPsychicScreamAction : public CastSpellAction
{
public:
    CastPsychicScreamAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "psychic scream") {}
    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

// Void Tendrils (108920, MoP talent): PBAoE root. MoP rotation row 25802 (shadow, group 0/10): current target
// within 7 yd. Same pvp / player / "is cc" gate as Psychic Scream.
class CastVoidTendrilsAction : public CastSpellAction
{
public:
    CastVoidTendrilsAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "void tendrils") {}
    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

// Halo (120517 holy / 120644 shadow, MoP talent): ring around the caster. MoP rotation row 505 (class, group
// 2/0): >= 3 group members below 30% or >= 3 enemies within 30 yd. The name resolves to whichever the bot knows.
class CastHaloAction : public CastSpellAction
{
public:
    CastHaloAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "halo") {}
    std::string const GetTargetName() override { return "self target"; }
};

// Cascade (127632 / 121135, MoP talent): bouncing bolt. MoP rotation row 25803 (shadow, group 0/10, always).
SPELL_ACTION(CastCascadeAction, "cascade");

// Penance (47540) on the enemy: MoP rotation row 25604 (disc, group 0/3, always). The heal version is
// "penance on party" below.
SPELL_ACTION(CastPenanceAction, "penance");

// Silence on any casting enemy (lane 0 contract: CastSpellOnEnemyCasterAction + "enemy caster in range"). MoP
// shadow priority groups 10/2 and 10/1. AC only silences the current target / an enemy healer, which
// stay. PvP maps only, like the other MoP PvP guards.
class CastSilenceOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastSilenceOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "silence") {}
    bool isUseful() override;
};
DEBUFF_ACTION(CastMindSootheAction, "mind soothe");
class CastFadeAction : public CastBuffSpellAction
{
public:
    CastFadeAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fade") {}
    bool isUseful() override;
};
BUFF_ACTION(CastShadowProtectionAction, "shadow protection");
BUFF_PARTY_ACTION(CastShadowProtectionOnPartyAction, "shadow protection");
BUFF_PARTY_ACTION(CastPrayerOfShadowProtectionAction, "prayer of shadow protection");

// shadow talents
SPELL_ACTION(CastMindFlayAction, "mind flay");
DEBUFF_ACTION(CastVampiricEmbraceAction, "vampiric embrace");
BUFF_ACTION(CastShadowformAction, "shadowform");
SPELL_ACTION(CastSilenceAction, "silence");
ENEMY_HEALER_ACTION(CastSilenceOnEnemyHealerAction, "silence");
// shadow talents 2.4.3
DEBUFF_CHECKISOWNER_ACTION(CastVampiricTouchAction, "vampiric touch");
DEBUFF_ENEMY_ACTION(CastVampiricTouchOnAttackerAction, "vampiric touch");
// racials
// #735: 5.4.8 Devouring Plague (2944) costs Shadow Orbs (SpellPower.dbc power type 13; Wowhead MoP tooltip
// 2026-10-08: "Consumes all of the caster's Shadow Orbs"). CanCastSpell skips the cost check
// (TRIGGERED_IGNORE_POWER_AND_REAGENT_COST), so it is gated AC-style by the "shadow orbs 3 available" trigger in
// ShadowPriestDebuffStrategy (PriestTriggers.h), not here - as AC's Rupture is a plain debuff action fired from
// "combo points 5 available" (AC RogueFinishingActions.h:32-36, CombatRogueStrategy.cpp:118-126).
DEBUFF_CHECKISOWNER_ACTION(CastDevouringPlagueAction, "devouring plague");
BUFF_ACTION(CastTouchOfWeaknessAction, "touch of weakness");
DEBUFF_ACTION(CastHexOfWeaknessAction, "hex of weakness");
BUFF_ACTION(CastShadowguardAction, "shadowguard");
HEAL_ACTION(CastDesperatePrayerAction, "desperate prayer");
BUFF_ACTION(CastFearWardAction, "fear ward");
BUFF_PARTY_ACTION(CastFearWardOnPartyAction, "fear ward");
// #524 (AC PriestActions.h:239): fear ward on the main tank.
class CastFearWardOnMainTankAction : public BuffOnMainTankAction
{
public:
    CastFearWardOnMainTankAction(PlayerbotAI* botAI) : BuffOnMainTankAction(botAI, "fear ward") {}
};
SPELL_ACTION_U(CastStarshardsAction, "starshards",
               (AI_VALUE2(uint8, "mana", "self target") > 50 && AI_VALUE(Unit*, "current target") &&
                AI_VALUE2(float, "distance", "current target") > 15.0f));
BUFF_ACTION(CastElunesGraceAction, "elune's grace");
BUFF_ACTION(CastFeedbackAction, "feedback");
BUFF_ACTION(CastSymbolOfHopeAction, "symbol of hope");
SPELL_ACTION(CastConsumeMagicAction, "consume magic");
SNARE_ACTION(CastChastiseAction, "chastise");

class CastRemoveShadowformAction : public Action
{
public:
    CastRemoveShadowformAction(PlayerbotAI* botAI) : Action(botAI, "remove shadowform") {}

    bool isUseful() override;
    bool isPossible() override;
    bool Execute(Event event) override;
};

class CastDispersionAction : public CastSpellAction
{
public:
    CastDispersionAction(PlayerbotAI* ai) : CastSpellAction(ai, "dispersion") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastPenanceOnPartyAction : public HealPartyMemberAction
{
public:
    CastPenanceOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "penance", 25.0f, HealingManaEfficiency::HIGH)
    {
    }
};

class CastHymnOfHopeAction : public CastSpellAction
{
public:
    CastHymnOfHopeAction(PlayerbotAI* ai) : CastSpellAction(ai, "hymn of hope") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastDivineHymnAction : public CastSpellAction
{
public:
    CastDivineHymnAction(PlayerbotAI* ai) : CastSpellAction(ai, "divine hymn") {}
    virtual std::string const GetTargetName() { return "self target"; }
};

class CastShadowfiendAction : public CastSpellAction
{
public:
    CastShadowfiendAction(PlayerbotAI* ai) : CastSpellAction(ai, "shadowfiend") {}
    virtual std::string const GetTargetName() { return "current target"; }
};

class CastPowerWordShieldOnAlmostFullHealthBelowAction : public HealPartyMemberAction
{
public:
    CastPowerWordShieldOnAlmostFullHealthBelowAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "power word: shield", 15.0f, HealingManaEfficiency::HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastPowerWordShieldOnNotFullAction : public HealPartyMemberAction
{
public:
    CastPowerWordShieldOnNotFullAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "power word: shield", 5.0f, HealingManaEfficiency::HIGH)
    {
    }
    bool isUseful() override;
    Unit* GetTarget() override;
};

class CastMindSearAction : public CastSpellAction
{
public:
    CastMindSearAction(PlayerbotAI* ai) : CastSpellAction(ai, "mind sear") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastGuardianSpiritOnPartyAction : public HealPartyMemberAction
{
public:
    CastGuardianSpiritOnPartyAction(PlayerbotAI* ai) : HealPartyMemberAction(ai, "guardian spirit", 40.0f, HealingManaEfficiency::MEDIUM) {}
};

// #812 MoP-only (no AC counterpart; WotLK has no Chakra / Holy Word). Holy stance: +single-target healing, refreshes
// Renew, and turns Holy Word: Chastise into Holy Word: Serenity (81208 effect 2 = aura 332 OVERRIDE_ACTIONBAR_SPELLS,
// amount 88684). Self buff, same shape as CastShadowformAction.
BUFF_ACTION(CastChakraSerenityAction, "chakra: serenity");

// Lane 8 / #863 (MoP rotation row 25715 + 25723, holy): the AoE stance, used while >= 3 group members within
// 30 yd are below 80% (see ChakraSanctuaryTrigger); Chakra: Serenity is kept otherwise (25724).
BUFF_ACTION(CastChakraSanctuaryAction, "chakra: sanctuary");

// Lane 8 / #863 (MoP rotation row 25716): Holy Word: Sanctuary (88685) is Holy Word: Chastise (88625) under
// the Chakra: Sanctuary aura, exactly as Serenity above. Only useful while that aura is up.
class CastHolyWordSanctuaryOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyWordSanctuaryOnPartyAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "holy word: chastise", 25.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
    std::string const getName() override { return "holy word: sanctuary on party"; }
    bool isUseful() override;
};

// #812 MoP-only: Holy Word: Serenity (88684, 2% base mana, instant, 10 s cd; Wowhead mop-classic 88684 read
// 2026-10-14) is not learned - the bot knows Holy Word: Chastise (88625, spec257 L10) and the Chakra: Serenity aura
// swaps it, the client way (PlayerbotAI BotOverrideSpell, #597). So the action casts "holy word: chastise" and is only
// useful while Chakra: Serenity is up (without it 88625 is the hostile Chastise).
class CastHolyWordSerenityOnPartyAction : public HealPartyMemberAction
{
public:
    CastHolyWordSerenityOnPartyAction(PlayerbotAI* ai)
        : HealPartyMemberAction(ai, "holy word: chastise", 25.0f, HealingManaEfficiency::VERY_HIGH)
    {
    }
    std::string const getName() override { return "holy word: serenity on party"; }
    bool isUseful() override;
};

#endif
