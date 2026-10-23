/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WARLOCKACTIONS_H
#define _PLAYERBOT_WARLOCKACTIONS_H

#include "GenericSpellActions.h"
#include "Playerbots.h"
#include "UseItemAction.h"

class PlayerbotAI;
class Unit;

// -- TALENTS
SPELL_ACTION(CastDarkRegenerationAction, "dark regeneration");
SPELL_ACTION(CastDemonicBreathAction, "demonic breath");
SPELL_ACTION(CastMortalCoilAction, "mortal coil");
BUFF_ACTION(CastSoulLinkAction, "soul link");
SPELL_ACTION(CastSacrificialPactAction, "sacrificial pact");
SPELL_ACTION(CastDarkBarginAction, "dark bargain");
SPELL_ACTION(CastBloodHorrorAction, "blood horror");
SPELL_ACTION(CastGrmoireOfSupremacyAction, "grimoire of supremacy");
SPELL_ACTION(CastGrimoireOfSacrificeAction, "grimoire of sacrifice");
SPELL_ACTION(CastMannorothFuryAction, "mannoroth's fury");
class CastShadowFuryAction : public CastSpellAction
{
public:
    CastShadowFuryAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "shadowfury") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};
// -- GENERIC
SPELL_ACTION(CastHealthFunnelAction, "health funnel");
// #693: AC WarlockActions.h:215-220 (CastSoulshatterAction with isUseful, incl. mod-playerbots#2855).
class CastSoulShatterAction : public CastSpellAction
{
public:
    CastSoulShatterAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "soulshatter") {}
    bool isUseful() override;
};
SPELL_ACTION(CastUnendingResolveAction, "unending resolve");
SPELL_ACTION(CastFelFlameAction, "fel flame");
BUFF_ACTION(CastDarkIntentAction, "dark intent");
// #521 (AC WarlockActions.h:32-55): Fel Armor 104938 (L38) and Unending Breath 5697 (L24) are both
// learnable in 5.4.8 (cls.py 2026-10-04). Demon Armor/Demon Skin actions stay unported: both spells
// are NOT-LEARNABLE in 5.4.8 (Fel Armor is the only armor spell).
class CastFelArmorAction : public CastBuffSpellAction
{
public:
    CastFelArmorAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fel armor") {}
};

class CastUnendingBreathAction : public CastBuffSpellAction
{
public:
    CastUnendingBreathAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "unending breath") {}
};

class CastUnendingBreathOnPartyAction : public BuffOnPartyAction
{
public:
    CastUnendingBreathOnPartyAction(PlayerbotAI* botAI) : BuffOnPartyAction(botAI, "unending breath") {}
};
SPELL_ACTION(CastTwilightWardAction, "twilight ward");

BEGIN_RANGED_SPELL_ACTION(CastShadowBoltAction, "shadow bolt")
END_SPELL_ACTION()

class CastDrainSoulAction : public CastSpellAction
{
public:
    CastDrainSoulAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "drain soul") {}

    bool isUseful() override;
};

class CastDrainLifeAction : public CastSpellAction
{
public:
    CastDrainLifeAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "drain life") {}
};

DEBUFF_ACTION(CastCurseOfWeaknessAction, "curse of enfeeblement");  // #521: successor, see GenericWarlockStrategy.cpp
DEBUFF_ACTION(CastCurseOfElementsAction, "curse of the elements");
DEBUFF_ACTION(CastCurseOfEnfeeblementAction, "curse of enfeeblement");

class CastCorruptionAction : public CastDebuffSpellAction
{
public:
    CastCorruptionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "corruption", true) {}
};
class CastCorruptionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastCorruptionOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "corruption", true) {}
};
class CastCreateHealthstoneAction : public CastBuffSpellAction
{
public:
    CastCreateHealthstoneAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "create healthstone") {}
};

// #163 S8 (AC parity, Ai/Class/Warlock/WarlockActions.h:96-127), corrected per research
// (s8-pet-control-and-class-spells.md B.1): patch 5.0.4 turned Soulstone into a **direct cast**
// (spell 20707) with no item and no "Create Soulstone" step at all (verified: 0 "Create Soulstone"
// rows in Spell.dbc). CastCreateSoulstoneAction is N/A and removed - UseSoulstoneAction below now
// casts the spell directly instead of using an item.

// Base for the four "cast soulstone on X" actions. Keeps AC's target order, 30 yd + LoS check,
// HasSoulstoneAura check and the 2.5 s cross-call reservation; only the "use an item" step (AC's
// UseItem) is replaced with a direct botAI->CastSpell("soulstone", target).
class UseSoulstoneAction : public Action
{
public:
    UseSoulstoneAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}

    // #713: the caster-side CanCastSpell gate (mounted / GCD / moving / casting / cooldown). AC's UseItem path
    // only ran with a stone in the bags; the direct-cast spell has no such gate, so without this the trigger
    // retried the cast every tick (SPELL_FAILED_NOT_MOUNTED 67 / NOT_READY 70 thousands of times per BG).
    bool isPossible() override;

protected:
    bool CastSoulstoneOn(Unit* target);
    static bool HasSoulstoneAura(Unit* unit);
};

class UseSoulstoneSelfAction : public UseSoulstoneAction
{
public:
    UseSoulstoneSelfAction(PlayerbotAI* botAI) : UseSoulstoneAction(botAI, "soulstone self") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class UseSoulstoneMasterAction : public UseSoulstoneAction
{
public:
    UseSoulstoneMasterAction(PlayerbotAI* botAI) : UseSoulstoneAction(botAI, "soulstone master") {}

    bool isUseful() override;
    bool Execute(Event event) override;
};

class UseSoulstoneTankAction : public UseSoulstoneAction
{
public:
    UseSoulstoneTankAction(PlayerbotAI* botAI) : UseSoulstoneAction(botAI, "soulstone tank") {}

    bool Execute(Event event) override;
};

class UseSoulstoneHealerAction : public UseSoulstoneAction
{
public:
    UseSoulstoneHealerAction(PlayerbotAI* botAI) : UseSoulstoneAction(botAI, "soulstone healer") {}

    bool Execute(Event event) override;
};

class CastBanishAction : public CastBuffSpellAction
{
public:
    CastBanishAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "banish on cc") {}

    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
};
class CastRainOfFireAction : public CastSpellAction
{
public:
    CastRainOfFireAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "rain of fire") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};
class CastFearAction : public CastDebuffSpellAction
{
public:
    CastFearAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "fear") {}
};

class CastFearOnCcAction : public CastBuffSpellAction
{
public:
    CastFearOnCcAction(PlayerbotAI* botAI) : CastBuffSpellAction(botAI, "fear on cc") {}

    Value<Unit*>* GetTargetValue() override;
    bool Execute(Event event) override;
    bool isPossible() override;
    bool isUseful() override;
};

class CastLifeTapAction : public CastSpellAction
{
public:
    CastLifeTapAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "life tap") {}

    std::string const GetTargetName() override { return "self target"; }
    bool isUseful() override;
};

class CastSiphonLifeAction : public CastDebuffSpellAction
{
public:
    CastSiphonLifeAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "siphon life", true) {}
};

class CastSiphonLifeOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastSiphonLifeOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "siphon life") {}
};
class CastDemonicEmpowermentAction : public CastBuffSpellAction
{
public:
    CastDemonicEmpowermentAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "demonic empowerment") {}
    std::string const GetTargetName() override { return "pet target"; }
};

// -- Affliction
// #521: 5.x name is "soulburn" (74434, L19, cls.py 2026-10-04); "soul burn" never resolved. It is a self buff
// (consumes a Soul Shard, empowers Drain Life per Wowhead 74434), so a buff action: no recast while active.
BUFF_ACTION(CastSoulBurnAction, "soulburn");
SPELL_ACTION(CastDarkSoulMiseryAction, "dark soul: misery");
SPELL_ACTION(CastMaleficGraspAction, "malefic grasp");
class CastCurseOfAgonyAction : public CastDebuffSpellAction
{
public:
    CastCurseOfAgonyAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "agony", true) {}
};
class CastUnstableAfflictionAction : public CastDebuffSpellAction
{
public:
    CastUnstableAfflictionAction(PlayerbotAI* ai) : CastDebuffSpellAction(ai, "unstable affliction", true) {}
};
class CastUnstableAfflictionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastUnstableAfflictionOnAttackerAction(PlayerbotAI* ai)
        : CastDebuffSpellOnAttackerAction(ai, "unstable affliction", true)
    {
    }
};
class CastHauntAction : public CastDebuffSpellAction
{
public:
    CastHauntAction(PlayerbotAI* ai) : CastDebuffSpellAction(ai, "haunt", true, 0) {}
    bool isUseful() override
    {
        return CastDebuffSpellAction::isUseful() && !botAI->HasAura("haunt", GetTarget(), true, false);
    }
};
class CastSeedOfCorruptionAction : public CastDebuffSpellAction
{
public:
    CastSeedOfCorruptionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "seed of corruption", true, 0) {}
    bool isUseful() override
    {
        return CastDebuffSpellAction::isUseful() && !botAI->HasAura("corruption", GetTarget(), false, true);
    }
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

class CastSeedOfCorruptionOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastSeedOfCorruptionOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "seed of corruption", true, 0)
    {
    }
    bool isUseful() override
    {
        return CastDebuffSpellOnAttackerAction::isUseful() && !botAI->HasAura("corruption", GetTarget(), false, true);
    }
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};
class CastCurseOfAgonyOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastCurseOfAgonyOnAttackerAction(PlayerbotAI* botAI)
        : CastDebuffSpellOnAttackerAction(botAI, "agony", true)
    {
    }
};

// -- summons
BUFF_ACTION(CastSummonVoidwalkerAction, "summon voidwalker");
BUFF_ACTION(CastSummonFelguardAction, "summon felguard");
BUFF_ACTION(CastSummonFelhunterAction, "summon felhunter");
BUFF_ACTION(CastSummonImpAction, "summon imp");
BUFF_ACTION(CastSummonSuccubusAction, "summon succubus");
BUFF_ACTION(CastSummonDoomGuardAction, "summon doomguard");
BUFF_ACTION(CastSummonFelImpAction, "summon fel imp");
BUFF_ACTION(CastSummonVoidLordAction, "summon voidlord");
BUFF_ACTION(CastSummonShivarraAction, "summon shivarra");
BUFF_ACTION(CastSummonObserverAction, "summon observer");
BUFF_ACTION(CastSummonTerrorGuardAction, "summon terrorguard");
BUFF_ACTION(CastSummonWrathGuardGuardAction, "summon wrathguard");

class CastSummonInfernalAction : public CastSpellAction
{
public:
    CastSummonInfernalAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "summon infernal") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};
class CastSummonAbyssalAction : public CastSpellAction
{
public:
    CastSummonAbyssalAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "summon abyssal") {}
    ActionThreatType getThreatType() override { return ActionThreatType::Aoe; }
};

// -- Demono
SPELL_ACTION(CastDarkSoulKnowledgeAction, "dark soul: knowledge");
class CastHellfireAction : public CastMeleeSpellAction
{
public:
    CastHellfireAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "hellfire")
    {
        range = ATTACK_DISTANCE;
    }
};
class CastMetaImmolationAction : public CastMeleeSpellAction
{
public:
    CastMetaImmolationAction(PlayerbotAI* botAI) : CastMeleeSpellAction(botAI, "immolation aura")
    {
        range = ATTACK_DISTANCE;
    }
};
class CastHandOfGuldanAction : public CastSpellAction
{
public:
    // 5.4.8 spell name is "Hand of Gul'dan" (105174, Spell.dbc); the action key stays "hand of guldan". The old
    // qualifier never resolved ([SID] resolved=0, 253 lines / 23 demonology bots in w19), so isPossible was always false.
    CastHandOfGuldanAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "hand of gul'dan") {}
    bool Execute(Event event) override;
};
SPELL_ACTION(CastSoulFireAction, "soul fire");

BUFF_ACTION(CastMetamorphosisAction, "metamorphosis");
SPELL_ACTION(CastMetaChaosWaveAction, "chaos wave");
DEBUFF_ACTION(CastMetaDoomAction, "doom");
SPELL_ACTION(CastMetaChaosAction, "touch of chaos");
SPELL_ACTION(CastMetaVoidRayAction, "void ray");


// -- Destru
SPELL_ACTION(CastDarkSoulInstabilityAction, "dark soul: instability");
SPELL_ACTION(CastFireAndBrimeStoneAction, "fire and brimstone");
SPELL_ACTION(CastFlamesOfXorothAction, "flames of xoroth");
SPELL_ACTION(CastShadowBurnAction, "shadowburn");
class CastChaosBoltAction : public CastSpellAction
{
public:
    CastChaosBoltAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "chaos bolt") {}
    bool isUseful() override;
};
class CastImmolateAction : public CastDebuffSpellAction
{
public:
    CastImmolateAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "immolate", true) {}
    bool Execute(Event event) override;
};
class CastImmolateOnAttackerAction : public CastDebuffSpellOnAttackerAction
{
public:
    CastImmolateOnAttackerAction(PlayerbotAI* botAI) : CastDebuffSpellOnAttackerAction(botAI, "immolate", true) {}
};
class CastConflagrateAction : public CastSpellAction
{
public:
    CastConflagrateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "conflagrate") {}
    bool Execute(Event event) override;
};
class CastIncinerateAction : public CastSpellAction
{
public:
    CastIncinerateAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "incinerate") {}
    bool Execute(Event event) override;
};

// -- #408 (AC parity, Ai/Class/Warlock/WarlockActions.h:103-127, :209-234, :359-399, :467-470).
// AC's UseSpellItemAction base lived in Ai/Base/Actions/UseItemAction.h; this module's
// UseItemAction is already self-only and has no selfOnly ctor arg, so the base is re-declared
// here (warlock-local, no shared header change / no extra rebuild).
class UseSpellItemAction : public UseItemAction
{
public:
    UseSpellItemAction(PlayerbotAI* botAI, std::string const name) : UseItemAction(botAI, name) {}

    bool isUseful() override;
};

// Utility spells (AC WarlockActions.h:209)
class CastShadowWardAction : public CastBuffSpellAction
{
public:
    CastShadowWardAction(PlayerbotAI* ai) : CastBuffSpellAction(ai, "shadow ward") {}
};

class DemonChargeAction : public CastSpellAction
{
public:
    DemonChargeAction(PlayerbotAI* ai) : CastSpellAction(ai, "demon charge") {}
};

// Curses (AC WarlockActions.h:359-399). All four bypass CastDebuffSpellAction's TTL check so a
// permanent curse is not treated as expired - identical to AC.
class CastCurseOfDoomAction : public CastDebuffSpellAction
{
public:
    CastCurseOfDoomAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of doom", true, 0) {}
    bool isUseful() override { return CastAuraSpellAction::isUseful(); }
};

class CastCurseOfExhaustionAction : public CastDebuffSpellAction
{
public:
    CastCurseOfExhaustionAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of exhaustion") {}
    bool isUseful() override { return CastAuraSpellAction::isUseful(); }
};

class CastCurseOfTonguesAction : public CastDebuffSpellAction
{
public:
    CastCurseOfTonguesAction(PlayerbotAI* botAI) : CastDebuffSpellAction(botAI, "curse of enfeeblement") {}  // #521: 5.0.4 successor
    bool isUseful() override { return CastAuraSpellAction::isUseful(); }
};

// CastCurseOfWeaknessAction already exists above (DEBUFF_ACTION, "curse of weakness"); only its
// action-factory key was missing.

// Tank spells (AC WarlockActions.h:467)
class CastSearingPainAction : public CastSpellAction
{
public:
    CastSearingPainAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "searing pain") {}
};

// ---- #864 / epic #854 (rotation lane 9): MoP warlock rotation port ------------------------------------------
// (class 9, prio group 10): Mortal Coil 6789 on a casting enemy (the "enemy caster
// target" value, lane 0 #855). Own getName so it never collides with the plain "mortal coil" action.
class CastMortalCoilOnEnemyCasterAction : public CastSpellOnEnemyCasterAction
{
public:
    CastMortalCoilOnEnemyCasterAction(PlayerbotAI* botAI) : CastSpellOnEnemyCasterAction(botAI, "mortal coil") {}
    std::string const getName() override { return "mortal coil on enemy caster"; }
};

// Optical Blast 115781 (pet entry 58964) + Spell Lock 19647 (pet entry 417), both
// group 10 and non-blocking: the PET casts. PlayerbotAI::CastSpell cannot do this (it toggles pet autocast for any
// spell the pet knows), so the action casts directly from the pet with Unit::CastSpell(victim, spellId, false).
class CastPetInterruptAction : public Action
{
public:
    CastPetInterruptAction(PlayerbotAI* botAI) : Action(botAI, "pet interrupt") {}
    bool isUseful() override;
    bool Execute(Event event) override;
};

// (Destruction, group 0 prio 9): Havoc 80240 on an enemy that is NOT the primary
// target. The stock "attacker without aura" value picks the highest-health attacker, i.e. usually the primary
// target itself, so the target is selected here (WarlockHavocTarget).
Unit* WarlockHavocTarget(PlayerbotAI* botAI);
class CastHavocAction : public CastSpellAction
{
public:
    CastHavocAction(PlayerbotAI* botAI) : CastSpellAction(botAI, "havoc") {}
    Unit* GetTarget() override { return WarlockHavocTarget(botAI); }
    std::string const getName() override { return "havoc"; }
};
#endif
