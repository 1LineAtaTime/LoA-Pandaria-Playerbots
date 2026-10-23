/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Monk class actions for the Legends of Azeroth Pandaria fork (issue #67).
 *
 * There is no upstream mod-playerbots monk context to port: AzerothCore is 3.3.5 and has no monk
 * class. These are built from the generic spell-action machinery this fork already has - each action
 * is resolved by the spell's English name through SpellIdValue (fixed in #64, which is why the enUS
 * DBCs are required). The spell list is the MoP 5.4.8 monk toolkit.
 */

#ifndef _PLAYERBOT_MONKACTIONS_H
#define _PLAYERBOT_MONKACTIONS_H

#include "AiObject.h"
#include "CureTriggers.h"
#include "GenericSpellActions.h"
#include "GenericTriggers.h"
#include "Player.h"
#include "SharedDefines.h"
#include "PlayerbotAI.h"
#include "ReachTargetActions.h"

// -- Stances / buffs (all three specs) --------------------------------------
BUFF_ACTION(CastStanceOfTheFierceTigerAction, "stance of the fierce tiger");
BUFF_ACTION(CastStanceOfTheSturdyOxAction, "stance of the sturdy ox");
BUFF_ACTION(CastStanceOfTheWiseSerpentAction, "stance of the wise serpent");
BUFF_ACTION(CastLegacyOfTheEmperorAction, "legacy of the emperor");
BUFF_ACTION(CastFortifyingBrewAction, "fortifying brew");

// #526 (AC Ai/Class/Warrior/WarriorTriggers.h:20-22 BattleStance/DefensiveStance/BerserkerStanceTrigger =
// BUFF_TRIGGER on the stance name, edge ACTION_HIGH+9/+10 in the spec strategy, AC TankWarriorStrategy.cpp:154-156,
// ArmsWarriorStrategy.cpp:102-104; AC DK presences are never triggered, so the warrior stance is the pattern that
// runs). 5.4.8: every monk spell other than Touch of Death needs a stance (SPELL_FAILED_ONLY_SHAPESHIFT, wave 10).
// Stance of the Fierce Tiger 103985 (skill L1, WW), Sturdy Ox 115069 (spec 268 L10, BM), Wise Serpent 115070
// (spec 270 L10, MW): cls.py + Wowhead mop-classic tooltip API, 2026-10-05.
BUFF_TRIGGER(StanceOfTheFierceTigerTrigger, "stance of the fierce tiger");
BUFF_TRIGGER(StanceOfTheSturdyOxTrigger, "stance of the sturdy ox");
BUFF_TRIGGER(StanceOfTheWiseSerpentTrigger, "stance of the wise serpent");

// -- Windwalker / shared melee ----------------------------------------------
MELEE_ACTION(CastJabAction, "jab");

// Tiger Palm 100787 (1 Chi): its Tiger Power buff (125359, 20 s, up to 3 stacks, -30% armor of the target) is
// an upkeep, not a filler - cast only when the buff is missing / under 3 stacks / near expiry, otherwise it
// out-prioritises Jab at Chi 1 and the bot never banks the 2 Chi Rising Sun Kick / Blackout Kick need.
// Wowhead tooltip 125359 / 100787, 2026-10-05; AC rogue upkeep shape (CombatRogueStrategy.cpp "slice and dice").
class CastTigerPalmAction : public CastMeleeSpellAction
{
public:
    CastTigerPalmAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "tiger palm") {}

    bool isUseful() override;
};
MELEE_ACTION(CastBlackoutKickAction, "blackout kick");
MELEE_ACTION(CastRisingSunKickAction, "rising sun kick");
MELEE_ACTION(CastFistsOfFuryAction, "fists of fury");
MELEE_ACTION(CastSpinningCraneKickAction, "spinning crane kick");
MELEE_ACTION(CastExpelHarmAction, "expel harm");
MELEE_ACTION(CastChiWaveAction, "chi wave");
REACH_ACTION(CastRollAction, "roll", 20.0f);

// -- Brewmaster -------------------------------------------------------------
MELEE_ACTION(CastKegSmashAction, "keg smash");
MELEE_ACTION(CastBreathOfFireAction, "breath of fire");
MELEE_ACTION(CastDizzyingHazeAction, "dizzying haze");
MELEE_ACTION(CastGuardAction, "guard");

// Purifying Brew 119582 (spec 268 L75, 1 Chi) clears Stagger: only worth casting while a Moderate/Heavy Stagger
// (124274/124273) is on the bot, otherwise it burns Chi for nothing (Wowhead tooltip 124274, 2026-10-05).
class CastPurifyingBrewAction : public CastBuffSpellAction
{
public:
    CastPurifyingBrewAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "purifying brew") {}

    bool isUseful() override;
};

// Shuffle upkeep: Blackout Kick 100784 (2 Chi) applies Shuffle 115307 (Wowhead 2026-10-05); registered under the
// name "shuffle" so it can sit in the Brewmaster list without colliding with Windwalker's "blackout kick" spender.
class CastShuffleAction : public CastMeleeSpellAction
{
public:
    CastShuffleAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "blackout kick") {}

    std::string const getName() override { return "shuffle"; }
    bool isUseful() override;
};
// #862 (MoP rotation row 26809 / 26916, prio group 5): Elusive Brew 115308 is consumed from the stack aura
// 128939, which carries the SAME NAME ("Elusive Brew"). BUFF_ACTION/CastAuraSpellAction::isUseful refuses while the bot
// has an aura named "elusive brew", so the old BUFF_ACTION could never fire once a stack existed (same trap as Tigereye
// Brew below). Plain self-target cast; ElusiveBrewTrigger (below) carries the >= 7 stacks gate.
class CastElusiveBrewAction : public CastSpellAction
{
public:
    CastElusiveBrewAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "elusive brew") {}

    std::string const GetTargetName() override { return "self target"; }
};

// -- Mistweaver -------------------------------------------------------------
HEAL_ACTION(CastRenewingMistAction, "renewing mist");
HEAL_ACTION(CastEnvelopingMistAction, "enveloping mist");
HEAL_ACTION(CastSurgingMistAction, "surging mist");
HEAL_ACTION(CastSoothingMistAction, "soothing mist");
HEAL_ACTION(CastUpliftAction, "uplift");
CURE_ACTION(CastDetoxAction, "detox");

// #163 S8 (MoP-only, no AC spec - designed in AC's healer pattern per the class strategies above).
// Revival: raid-wide heal, self-cast, same shape as CastTranquilityAction (DruidActions.h)/
// CastDivineHymnAction. Verified against Spell.dbc: "Revival" is spell id 115310.
AOE_HEAL_ACTION(CastRevivalAction, "revival", 15.0f, HealingManaEfficiency::MEDIUM);

// Life Cocoon: single-target defensive cooldown cast on a party member, same shape as
// CastGuardianSpiritOnPartyAction (PriestActions.h). Verified against Spell.dbc: "Life Cocoon" is
// spell id 116849 (124990 is the same spell's Glyph-modified rank entry).
HEAL_PARTY_ACTION(CastLifeCocoonOnPartyAction, "life cocoon", 40.0f, HealingManaEfficiency::MEDIUM);

// "On party" variants of the two portable HoTs, so a Mistweaver can top off someone other than its
// current target - same shape as Druid's CastRejuvenationOnPartyAction/regrowth-on-party.
HEAL_PARTY_ACTION(CastRenewingMistOnPartyAction, "renewing mist", 15.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastEnvelopingMistOnPartyAction, "enveloping mist", 25.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastSurgingMistOnPartyAction, "surging mist", 20.0f, HealingManaEfficiency::MEDIUM);

// Mana Tea: mana-regen cooldown, same trigger shape AC uses for Innervate/Mana Tide ("medium mana").
// Verified against Spell.dbc: spell id 115294.
// #862 (MoP rotation row 27013, prio group 2: "mana % <= 50 AND has aura Mana Tea (115867), >= 2 stacks"): the
// stack aura 115867 is named "Mana Tea" like the spell, so BUFF_ACTION (CastAuraSpellAction::isUseful) never cast it with
// stacks banked. Self-target cast gated on >= 2 stacks; the mana gate is the existing "medium mana" trigger edge.
class CastManaTeaAction : public CastSpellAction
{
public:
    CastManaTeaAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "mana tea") {}

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

// #194 (S8, MoP-only, no AC spec): Thunder Focus Tea, the "empower the next heal" cooldown from the
// research doc B.7 heal ladder (id 116680; 1 Chi, 45 s cooldown, empowers the next Surging Mist or
// Uplift). Same shape as Resto Shaman/Druid's "nature's swiftness"-style empower-cooldown BUFF_ACTION.
BUFF_ACTION(CastThunderFocusTeaAction, "thunder focus tea");

// Chi-gated "uplift on party": Uplift costs 2 Chi (SpellPower.dbc) and heals every target carrying the
// caster's Renewing Mist, so it is only worth attempting with Chi actually banked - the fork's own
// "chi" value (StatsValues.h/.cpp), read the same way RageValue/EnergyValue read their power types.
// Hand-written instead of the HEAL_PARTY_ACTION macro so isUseful() can add that check (MonkActions.cpp).
class CastUpliftOnPartyAction : public HealPartyMemberAction
{
public:
    CastUpliftOnPartyAction(PlayerbotAI* botAI)
        : HealPartyMemberAction(botAI, "uplift", 15.0f, HealingManaEfficiency::MEDIUM)
    {
    }

    bool isUseful() override;
};

// ---------------------------------------------------------------------------------------------
// #526 / #140 / #388: the class trigger + action layer every AC class has (Monk is MoP-only, so AC
// has no monk file; each block names the AC file whose pattern it copies and the 5.4.8 spell source:
// Wowhead tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id>, fetched 2026-10-04, and
// cls.py learnability check).
// ---------------------------------------------------------------------------------------------

// Interrupt. Spear Hand Strike 116705 (skill, L32; interrupts + 4 s school lockout, 15 s cd, melee
// range). Pattern: AC Rogue kick (Ai/Class/Rogue/Strategy/AssassinationRogueStrategy.cpp:172-183:
// "kick" + "kick on enemy healer"), AC Warrior pummel (FuryWarriorStrategy.cpp:107-117).
INTERRUPT_TRIGGER(SpearHandStrikeInterruptTrigger, "spear hand strike");
INTERRUPT_HEALER_TRIGGER(SpearHandStrikeEnemyHealerTrigger, "spear hand strike");
SPELL_ACTION(CastSpearHandStrikeAction, "spear hand strike");
ENEMY_HEALER_ACTION(CastSpearHandStrikeOnEnemyHealerAction, "spear hand strike");

// Taunt (Brewmaster). Provoke 115546 (skill, L14; 40 yd, 8 s cd, target rushes the monk). Pattern: AC
// TankWarriorStrategy.cpp:219-229 ("lose aggro" -> "taunt", "taunt on snare target"); same useful()
// guard as MoP CastTauntAction (WarriorActions.h:114): only when the target is not already on us.
SPELL_ACTION_U(CastProvokeAction, "provoke", GetTarget() && !GetTarget()->IsPlayer() && GetTarget()->GetTarget() != bot->GetGUID());   // #835: never on a player
SNARE_TRIGGER(ProvokeSnareTrigger, "provoke");
// #835 (w74 WSG: warrior "taunt on snare target" landed Taunt on a player): the snare target can be a fleeing
// player; a taunt is never cast on players.
class CastProvokeOnSnareTargetAction : public CastSnareSpellAction
{
public:
    CastProvokeOnSnareTargetAction(PlayerbotAI* botAI) : CastSnareSpellAction(botAI, "provoke") {}
    bool isUseful() override { Unit* t = GetTarget(); return t && !t->IsPlayer() && CastSnareSpellAction::isUseful(); }
};

// Crowd control. Paralysis 115078 (skill, L44; 20 yd, incapacitate 40 s, breaks on damage). Pattern:
// AC Rogue sap (Ai/Class/Rogue/Strategy/CombatRogueStrategy.cpp:394-397 "sap" cc node; AC
// RogueTriggers.h:99 SapTrigger : HasCcTargetTrigger).
CC_TRIGGER(ParalysisTrigger, "paralysis");
CC_ACTION(CastParalysisAction, "paralysis");

// Party buffs, out of combat. Legacy of the Emperor 115921 (skill, L22: +5% Str/Agi/Int, raid-wide
// when cast on a group member) and Legacy of the White Tiger 116781 (Windwalker spec 269, L81: +5%
// crit, raid-wide). cls.py confirms both; Brewmaster/Mistweaver simply cannot cast the latter.
// Pattern: AC Mage nc "arcane intellect" / "arcane intellect on party"
// (Ai/Class/Mage/Strategy/GenericMageNonCombatStrategy.cpp:38,56), shaped like MoP's
// ArcaneBrillianceOnPartyTrigger/CastArcaneBrillianceOnPartyAction (MageTriggers.h:24, MageActions.h:175).
// #704: check interval 4 * 2000 like AC's party/raid stat buffs (AC Ai/Class/Mage/MageTriggers.h:22-23
// ArcaneIntellectOnPartyTrigger, :29 ArcaneIntellectTrigger; AC Ai/Class/Druid/DruidTriggers.h:27
// MarkOfTheWildOnPartyTrigger; MoP MageTriggers.h:26 ArcaneBrillianceOnPartyTrigger). The BUFF_TRIGGER /
// BUFF_PARTY_TRIGGER macros use interval 1, i.e. every tick.
class LegacyOfTheEmperorTrigger : public BuffTrigger
{
public:
    LegacyOfTheEmperorTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "legacy of the emperor", 4 * 2000) {}
};

class LegacyOfTheEmperorOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    LegacyOfTheEmperorOnPartyTrigger(PlayerbotAI* botAI) : BuffOnPartyTrigger(botAI, "legacy of the emperor", 4 * 2000) {}
};

BUFF_ACTION(CastLegacyOfTheWhiteTigerAction, "legacy of the white tiger");

class LegacyOfTheWhiteTigerTrigger : public BuffTrigger
{
public:
    LegacyOfTheWhiteTigerTrigger(PlayerbotAI* botAI) : BuffTrigger(botAI, "legacy of the white tiger", 4 * 2000) {}
};

class LegacyOfTheWhiteTigerOnPartyTrigger : public BuffOnPartyTrigger
{
public:
    LegacyOfTheWhiteTigerOnPartyTrigger(PlayerbotAI* botAI)
        : BuffOnPartyTrigger(botAI, "legacy of the white tiger", 4 * 2000)
    {
    }
};
BUFF_PARTY_ACTION(CastLegacyOfTheEmperorOnPartyAction, "legacy of the emperor");
BUFF_PARTY_ACTION(CastLegacyOfTheWhiteTigerOnPartyAction, "legacy of the white tiger");

// Boost. Tigereye Brew 116740 (Windwalker spec 269, L56: +6% damage and healing per consumed stack,
// 15 s, 5 s cd) and Touch of Death 115080 (skill, L22, 3 Chi, 1.5 min cd: "only usable on creatures
// that have equal or less health than you, or on players at or below 10%"). Pattern: AC RogueBoostStrategy
// (AC Ai/Class/Rogue/Strategy/CombatRogueStrategy.cpp:373-375: "adrenaline rush" BoostTrigger).
// The stock BoostTrigger is not usable for Tigereye Brew: the stacks aura (125195) shares the spell's
// name, so BuffTrigger's "no aura" test would never pass once a stack exists; the trigger therefore
// uses "can the spell be cast" (which already requires stacks) plus BoostTrigger's balance gate.
// Both IsActive() bodies are in MonkActions.cpp (AI_VALUE cannot bind from a header TU).
class TigereyeBrewTrigger : public SpellCanBeCastTrigger
{
public:
    TigereyeBrewTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "tigereye brew") {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

class TouchOfDeathTrigger : public SpellCanBeCastTrigger
{
public:
    TouchOfDeathTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "touch of death") {}

    bool IsActive() override;
};

// Not a BUFF_ACTION: CastAuraSpellAction::isUseful() refuses while the bot has an aura NAMED "tigereye brew",
// and the stack aura 125195 carries that name, so a buff action could never fire with stacks banked.
class CastTigereyeBrewAction : public CastSpellAction
{
public:
    CastTigereyeBrewAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "tigereye brew") {}

    std::string const GetTargetName() override { return "self target"; }
};
MELEE_ACTION(CastTouchOfDeathAction, "touch of death");

// Defensive. Fortifying Brew 115203 (skill, L24: +20% health, -20% damage taken, 20 s, 3 min cd) is
// CastFortifyingBrewAction above; trigger = the generic "low health" (AC GenericDKStrategy.cpp:68,
// "low health" -> "icebound fortitude").

// Cure. Detox 115450 (skill, L20: removes Poison and Disease from a friendly target; Mistweaver's
// Internal Medicine adds Magic - tooltip "[Internal Medicine: Magic] Poison and Disease effects").
// Pattern: AC PaladinCureStrategy (Ai/Class/Paladin/Strategy/GenericPaladinStrategy.cpp:38-55) and the
// MoP paladin ports of its triggers/actions (PaladinTriggers.h:158-191, PaladinActions.h:254-290).
// Self cure is the existing CastDetoxAction ("detox"); that single action serves all three dispel types.
class DetoxCurePoisonTrigger : public NeedCureTrigger
{
public:
    DetoxCurePoisonTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "detox", DISPEL_POISON) {}
};

class DetoxCureDiseaseTrigger : public NeedCureTrigger
{
public:
    DetoxCureDiseaseTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "detox", DISPEL_DISEASE) {}
};

class DetoxCureMagicTrigger : public NeedCureTrigger
{
public:
    DetoxCureMagicTrigger(PlayerbotAI* botAI) : NeedCureTrigger(botAI, "detox", DISPEL_MAGIC) {}
};

class DetoxPartyMemberCurePoisonTrigger : public PartyMemberNeedCureTrigger
{
public:
    DetoxPartyMemberCurePoisonTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "detox", DISPEL_POISON) {}
};

class DetoxPartyMemberCureDiseaseTrigger : public PartyMemberNeedCureTrigger
{
public:
    DetoxPartyMemberCureDiseaseTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "detox", DISPEL_DISEASE) {}
};

class DetoxPartyMemberCureMagicTrigger : public PartyMemberNeedCureTrigger
{
public:
    DetoxPartyMemberCureMagicTrigger(PlayerbotAI* botAI) : PartyMemberNeedCureTrigger(botAI, "detox", DISPEL_MAGIC) {}
};

class CastDetoxPoisonOnPartyAction : public CurePartyMemberAction
{
public:
    CastDetoxPoisonOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "detox", DISPEL_POISON) {}

    std::string const getName() override { return "detox poison on party"; }
};

class CastDetoxDiseaseOnPartyAction : public CurePartyMemberAction
{
public:
    CastDetoxDiseaseOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "detox", DISPEL_DISEASE) {}

    std::string const getName() override { return "detox disease on party"; }
};

class CastDetoxMagicOnPartyAction : public CurePartyMemberAction
{
public:
    CastDetoxMagicOnPartyAction(PlayerbotAI* botAI) : CurePartyMemberAction(botAI, "detox", DISPEL_MAGIC) {}

    std::string const getName() override { return "detox magic on party"; }
};

// ---------------------------------------------------------------------------------------------
// #862 (epic #854, rotation lane 7): MoP monk rotations.
// Monk is MoP-only (no AC class): AC style (trigger + action + strategy node), MoP row ids cited per port.
// All spell names verified against the 5.4.8 Spell.dbc (spellname.py mop): the SpellIdValue match is an exact name.
// ---------------------------------------------------------------------------------------------

// Self-target cast (statues, Rushing Jade Wind, Chi/Energizing Brew): CastSpellAction defaults to "current target".
class CastMonkSelfSpellAction : public CastSpellAction
{
public:
    CastMonkSelfSpellAction(PlayerbotAI* botAI, std::string const& spell) : CastSpellAction(botAI, spell) {}

    std::string const GetTargetName() override { return "self target"; }
};

// MoP rotation row 26815 (BM, group 5, health <= 40): Dampen Harm 122278 (talent). Fires from the generic "low health" edge.
BUFF_ACTION(CastDampenHarmAction, "dampen harm");

// MoP rotation row 26812 (BM, group 5) Summon Black Ox Statue 115315 / 27006 (MW, group 2) Summon Jade Serpent Statue 115313.
// The triggers also require that no statue of the bot is standing (creature entries 61146 / 60849,
// world creature_template), otherwise it would be re-summoned on every spell cooldown.
class CastSummonBlackOxStatueAction : public CastMonkSelfSpellAction
{
public:
    CastSummonBlackOxStatueAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "summon black ox statue") {}
};

class CastSummonJadeSerpentStatueAction : public CastMonkSelfSpellAction
{
public:
    CastSummonJadeSerpentStatueAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "summon jade serpent statue")
    {
    }
};

class MonkStatueTrigger : public SpellCanBeCastTrigger
{
public:
    MonkStatueTrigger(PlayerbotAI* botAI, std::string const& spell, uint32 statueEntry)
        : SpellCanBeCastTrigger(botAI, spell), statueEntry(statueEntry)
    {
    }

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;

private:
    uint32 statueEntry;
};

class BlackOxStatueTrigger : public MonkStatueTrigger
{
public:
    BlackOxStatueTrigger(PlayerbotAI* botAI) : MonkStatueTrigger(botAI, "summon black ox statue", 61146) {}
};

class JadeSerpentStatueTrigger : public MonkStatueTrigger
{
public:
    JadeSerpentStatueTrigger(PlayerbotAI* botAI) : MonkStatueTrigger(botAI, "summon jade serpent statue", 60849) {}
};

// MoP rotation row 26809 (BM) / 26916 (WW), group 5: Elusive Brew when >= 7 stacks of 128939 and the buff 115308 is not up.
class ElusiveBrewTrigger : public SpellCanBeCastTrigger
{
public:
    ElusiveBrewTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "elusive brew") {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

// MoP rotation row 1001 (class, group 2: Chi <= 2) and 26915 (WW, group 5: Energy <= 30%): Chi Brew 115399 (talent).
class ChiBrewTrigger : public SpellCanBeCastTrigger
{
public:
    ChiBrewTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "chi brew") {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

class CastChiBrewAction : public CastMonkSelfSpellAction
{
public:
    CastChiBrewAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "chi brew") {}
};

// MoP rotation row 26914 (WW, group 5): Energizing Brew 115288. MoP 5.4.8 lists it as "always"; it only restores Energy, so the
// edge hangs on the existing "low energy" trigger (LowEnergyTrigger, threshold 30) instead of being cast at full Energy.
class CastEnergizingBrewAction : public CastMonkSelfSpellAction
{
public:
    CastEnergizingBrewAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "energizing brew") {}
};

// rot-fix-F (wave 74): the "low energy" edge (threshold 30) never fired - a level-90 Windwalker on a dummy never drops below
// ~60 Energy (probe pwr=61..100), so Energizing Brew 115288 (MoP rotation row 26914, "always", 1 min cd) was never cast. Cast it as
// soon as any real Energy deficit exists (<= 75%) while the spell is ready; Windwalker only, like the MoP row.
class EnergizingBrewTrigger : public SpellCanBeCastTrigger
{
public:
    EnergizingBrewTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "energizing brew") {}

    std::string const GetTargetName() override { return "self target"; }
    bool IsActive() override;
};

// rot-fix-F: Chi Wave 115098 (MoP rotation row 27016 "Chi Wave Victim", Mistweaver group 2: current target, always). The Windwalker
// "chi wave" action is a MELEE_ACTION (melee-range gate), wrong for a Mistweaver standing at heal range; Chi Wave is a
// 40 yd ranged bolt that bounces to allies, so the Mistweaver uses this plain target cast.
class CastChiWaveTargetAction : public CastSpellAction
{
public:
    CastChiWaveTargetAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chi wave") {}

    std::string const getName() override { return "chi wave target"; }
};

// MoP rotation row 1002 (class, group 3): Rushing Jade Wind 116847 (talent) when >= 3 enemies within 10 yd (the "medium aoe" edge).
class CastRushingJadeWindAction : public CastMonkSelfSpellAction
{
public:
    CastRushingJadeWindAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "rushing jade wind") {}
};

// MoP rotation row 26900 (WW, group 0 prio 20, "always" on the current target): Touch of Karma 122470.
SPELL_ACTION(CastTouchOfKarmaAction, "touch of karma");

class TouchOfKarmaTrigger : public SpellCanBeCastTrigger
{
public:
    TouchOfKarmaTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "touch of karma") {}
};

// MoP rotation row 1003 (class, group 3, "always" on the current target): Invoke Xuen, the White Tiger 123904 (talent).
SPELL_ACTION(CastInvokeXuenAction, "invoke xuen, the white tiger");

class InvokeXuenTrigger : public SpellCanBeCastTrigger
{
public:
    InvokeXuenTrigger(PlayerbotAI* botAI) : SpellCanBeCastTrigger(botAI, "invoke xuen, the white tiger") {}
};

// MoP rotation row 1000 (class, group 2, injured friendlies <= 95%): Zen Sphere 124081 (talent, HoT). Chi Wave heal: MoP
// 27017 (MW, group 2, target <= 80%). Both are "on party" heals on the party-member-to-heal value.
HEAL_PARTY_ACTION(CastZenSphereOnPartyAction, "zen sphere", 15.0f, HealingManaEfficiency::MEDIUM);
HEAL_PARTY_ACTION(CastChiWaveOnPartyAction, "chi wave", 15.0f, HealingManaEfficiency::MEDIUM);

// MoP rotation row 18/19 (class, group 10): Spear Hand Strike on ANY enemy caster (victim row 19 + "each valid enemy" row 18).
// Base class and the "enemy caster in range" trigger are lane 0 (#855, shared contract). Spear Hand Strike is melee
// range, the shared value looks 30 yd out, hence the range test in isUseful.
class CastSpearHandStrikeOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastSpearHandStrikeOnEnemyCasterAction(PlayerbotAI* botAI)
        : CastSpellOnEnemyCasterAction(botAI, "spear hand strike")
    {
    }

    std::string const getName() override { return "spear hand strike on enemy caster"; }
    bool isUseful() override;
};

// MoP rotation row 1010 (class, group 0): Roll 109132 when the way ahead is >= 11 yd and the bot is not mounted. Used here as
// a PvP gap closer only (shared "in pvp instance" value), open world / dungeon behaviour stays as it was. "roll" stays
// the REACH_ACTION registered above (unused by any strategy); this one is the gated edge.
class CastRollGapAction : public CastMonkSelfSpellAction
{
public:
    CastRollGapAction(PlayerbotAI* botAI) : CastMonkSelfSpellAction(botAI, "roll") {}

    std::string const getName() override { return "roll gap"; }
    bool isUseful() override;
};

#endif
