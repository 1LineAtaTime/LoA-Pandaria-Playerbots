/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "DKAiObjectContext.h"

#include "BloodDKStrategy.h"
#include "DKActions.h"
#include "DKTriggers.h"
#include "DeathKnightPullStrategy.h"
#include "FrostDKStrategy.h"
#include "GenericDKNonCombatStrategy.h"
#include "GenericTriggers.h"
#include "Playerbots.h"

#include "UnholyDKStrategy.h"

class DeathKnightStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightStrategyFactoryInternal()
    {
        creators["nc"] = &DeathKnightStrategyFactoryInternal::nc;
        creators["pull"] = &DeathKnightStrategyFactoryInternal::pull;
        creators["frost aoe"] = &DeathKnightStrategyFactoryInternal::frost_aoe;
        creators["unholy aoe"] = &DeathKnightStrategyFactoryInternal::unholy_aoe;
        // #593 (AC Ai/Base/StrategyContext.h:105,139): "behind" (SetBehindCombatStrategy) was never registered, so
        // AiFactory's addStrategy("behind") for frost/unholy was a silent no-op.
        creators["behind"] = &DeathKnightStrategyFactoryInternal::behind;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericDKNonCombatStrategy(botAI); }
    static Strategy* pull(PlayerbotAI* botAI) { return new DeathKnightPullStrategy(botAI); }
    static Strategy* frost_aoe(PlayerbotAI* botAI) { return new FrostDKAoeStrategy(botAI); }
    static Strategy* unholy_aoe(PlayerbotAI* botAI) { return new UnholyDKAoeStrategy(botAI); }
    static Strategy* behind(PlayerbotAI* botAI) { return new SetBehindCombatStrategy(botAI); }  // #593
};

class DeathKnightCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &DeathKnightCombatStrategyFactoryInternal::blood;
        creators["blood"] = &DeathKnightCombatStrategyFactoryInternal::blood;
        creators["frost"] = &DeathKnightCombatStrategyFactoryInternal::frost_dps;
        creators["unholy"] = &DeathKnightCombatStrategyFactoryInternal::unholy_dps;
    }

private:
    static Strategy* frost_dps(PlayerbotAI* botAI) { return new FrostDKStrategy(botAI); }
    static Strategy* unholy_dps(PlayerbotAI* botAI) { return new UnholyDKStrategy(botAI); }
    static Strategy* tank(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
    static Strategy* blood(PlayerbotAI* botAI) { return new BloodDKStrategy(botAI); }
};

class DeathKnightDKBuffStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    DeathKnightDKBuffStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["bdps"] = &DeathKnightDKBuffStrategyFactoryInternal::bdps;
    }

private:
    static Strategy* bdps(PlayerbotAI* botAI) { return new DKBuffDpsStrategy(botAI); }
};

class DeathKnightTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    DeathKnightTriggerFactoryInternal()
    {
        // #149 registered the first 13 DK trigger names. #357 completes the set to full AC parity:
        // every remaining name DKAiObjectContext.cpp registers upstream is now registered here too,
        // in AC's order and with AC's exact strings.
        // 5.4.8: "pestilence glyph" is registered for AC name parity but is INERT - WotLK's Glyph of
        // Pestilence (63334, refreshes diseases) does not exist in Cataclysm; MoP's (58657) only widens
        // Pestilence's radius, so PestilenceGlyphTrigger::IsActive() returns false. See DKTriggers.h.
        // "no desolation" stays registered but is also always-true on 5.4.8 (WotLK Desolation talent gone).
        creators["icy touch 3s"] = &DeathKnightTriggerFactoryInternal::icy_touch_3s;
        creators["dd cd and icy touch 3s"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_icy_touch_3s;
        creators["plague strike 3s"] = &DeathKnightTriggerFactoryInternal::plague_strike_3s;
        creators["dd cd and plague strike 3s"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_plague_strike_3s;
        creators["no desolation"] = &DeathKnightTriggerFactoryInternal::no_desolation;
        creators["death and decay cooldown"] = &DeathKnightTriggerFactoryInternal::death_and_decay_cooldown;
        creators["dd cd and no desolation"] = &DeathKnightTriggerFactoryInternal::dd_cd_and_no_desolation;
        creators["mind freeze on enemy healer"] = &DeathKnightTriggerFactoryInternal::mind_freeze_on_enemy_healer;
        creators["no rune"] = &DeathKnightTriggerFactoryInternal::no_rune;
        // #149 (AC DKAiObjectContext.cpp:82-86): FrostDKStrategy.cpp / UnholyDKStrategy.cpp already
        // reference these names; only the registration was missing.
        creators["high blood rune"] = &DeathKnightTriggerFactoryInternal::high_blood_rune;
        creators["high frost rune"] = &DeathKnightTriggerFactoryInternal::high_frost_rune;
        creators["high unholy rune"] = &DeathKnightTriggerFactoryInternal::high_unholy_rune;
        creators["freezing fog"] = &DeathKnightTriggerFactoryInternal::freezing_fog;
        // #357: AC DKAiObjectContext.cpp:59-92 - the names this fork never registered.
        creators["bone shield"] = &DeathKnightTriggerFactoryInternal::bone_shield;
        creators["pestilence glyph"] = &DeathKnightTriggerFactoryInternal::pestilence_glyph;
        creators["blood strike"] = &DeathKnightTriggerFactoryInternal::blood_strike;
        creators["plague strike"] = &DeathKnightTriggerFactoryInternal::plague_strike;
        creators["plague strike on attacker"] = &DeathKnightTriggerFactoryInternal::plague_strike_on_attacker;
        creators["icy touch"] = &DeathKnightTriggerFactoryInternal::icy_touch;
        creators["death coil"] = &DeathKnightTriggerFactoryInternal::death_coil;
        creators["icy touch on attacker"] = &DeathKnightTriggerFactoryInternal::icy_touch_on_attacker;
        creators["horn of winter"] = &DeathKnightTriggerFactoryInternal::horn_of_winter;
        creators["mind freeze"] = &DeathKnightTriggerFactoryInternal::mind_freeze;
        creators["strangulate"] = &DeathKnightTriggerFactoryInternal::strangulate;
        creators["strangulate on enemy healer"] = &DeathKnightTriggerFactoryInternal::strangulate_on_enemy_healer;
        creators["blood tap"] = &DeathKnightTriggerFactoryInternal::blood_tap;
        creators["raise dead"] = &DeathKnightTriggerFactoryInternal::raise_dead;
        creators["chains of ice"] = &DeathKnightTriggerFactoryInternal::chains_of_ice;
        creators["pillar of frost"] = &DeathKnightTriggerFactoryInternal::pillar_of_frost;  // #601
        creators["army of the dead"] = &DeathKnightTriggerFactoryInternal::army_of_the_dead;
        creators["hysteria no cd"] = &DeathKnightTriggerFactoryInternal::hysteria_no_cd;
        // rot-lane-2 (#857): MoP rotation triggers (DKTriggers.h). Removed dead names: "improved icy talons",
        // "unbreakable armor" (NO-SUCH-NAME on 5.4.8, forkonly_verdicts.txt).
        creators["unholy frenzy"] = &DeathKnightTriggerFactoryInternal::unholy_frenzy;
        creators["dk outbreak"] = &DeathKnightTriggerFactoryInternal::dk_outbreak;
        creators["dk festering"] = &DeathKnightTriggerFactoryInternal::dk_festering;
        creators["dk blood boil"] = &DeathKnightTriggerFactoryInternal::dk_blood_boil;
        creators["dk frost strike proc"] = &DeathKnightTriggerFactoryInternal::dk_frost_strike_proc;
        creators["dk death coil proc"] = &DeathKnightTriggerFactoryInternal::dk_death_coil_proc;
        creators["dk chains of ice player"] = &DeathKnightTriggerFactoryInternal::dk_chains_of_ice_player;
        creators["dk pvp enemy caster"] = &DeathKnightTriggerFactoryInternal::dk_pvp_enemy_caster;
        // MoP rotation row 25015/25118/25220 (Icebound Fortitude, health <= 40), 25017 (Rune Tap <= 40), 25016 (Vampiric
        // Blood <= 90). HealthInRangeTrigger fires below the max value (strict <).
        creators["dk health 40"] = &DeathKnightTriggerFactoryInternal::dk_health_40;
        creators["dk health 90"] = &DeathKnightTriggerFactoryInternal::dk_health_90;
    }

private:
    static Trigger* icy_touch_3s(PlayerbotAI* botAI) { return new IcyTouch3sDebuffTrigger(botAI); }
    static Trigger* dd_cd_and_icy_touch_3s(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "icy touch 3s");
    }
    static Trigger* plague_strike_3s(PlayerbotAI* botAI) { return new PlagueStrike3sDebuffTrigger(botAI); }
    static Trigger* dd_cd_and_plague_strike_3s(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "plague strike 3s");
    }
    static Trigger* no_desolation(PlayerbotAI* botAI) { return new DesolationTrigger(botAI); }
    static Trigger* death_and_decay_cooldown(PlayerbotAI* botAI) { return new DeathAndDecayCooldownTrigger(botAI); }
    static Trigger* dd_cd_and_no_desolation(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "death and decay cooldown", "no desolation");
    }
    static Trigger* mind_freeze_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new MindFreezeOnEnemyHealerTrigger(botAI);
    }
    static Trigger* no_rune(PlayerbotAI* botAI) { return new NoRuneTrigger(botAI); }
    static Trigger* high_blood_rune(PlayerbotAI* botAI) { return new HighBloodRuneTrigger(botAI); }
    static Trigger* high_frost_rune(PlayerbotAI* botAI) { return new HighFrostRuneTrigger(botAI); }
    static Trigger* high_unholy_rune(PlayerbotAI* botAI) { return new HighUnholyRuneTrigger(botAI); }
    static Trigger* freezing_fog(PlayerbotAI* botAI) { return new FreezingFogTrigger(botAI); }
    static Trigger* bone_shield(PlayerbotAI* botAI) { return new BoneShieldTrigger(botAI); }
    static Trigger* pestilence_glyph(PlayerbotAI* botAI) { return new PestilenceGlyphTrigger(botAI); }
    static Trigger* blood_strike(PlayerbotAI* botAI) { return new BloodStrikeTrigger(botAI); }
    static Trigger* plague_strike(PlayerbotAI* botAI) { return new PlagueStrikeDebuffTrigger(botAI); }
    static Trigger* plague_strike_on_attacker(PlayerbotAI* botAI)
    {
        return new PlagueStrikeDebuffOnAttackerTrigger(botAI);
    }
    static Trigger* icy_touch(PlayerbotAI* botAI) { return new IcyTouchDebuffTrigger(botAI); }
    static Trigger* death_coil(PlayerbotAI* botAI) { return new DeathCoilTrigger(botAI); }
    static Trigger* icy_touch_on_attacker(PlayerbotAI* botAI)
    {
        return new IcyTouchDebuffOnAttackerTrigger(botAI);
    }
    static Trigger* horn_of_winter(PlayerbotAI* botAI) { return new HornOfWinterTrigger(botAI); }
    static Trigger* mind_freeze(PlayerbotAI* botAI) { return new MindFreezeInterruptSpellTrigger(botAI); }
    static Trigger* strangulate(PlayerbotAI* botAI) { return new StrangulateInterruptSpellTrigger(botAI); }
    static Trigger* strangulate_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new StrangulateOnEnemyHealerTrigger(botAI);
    }
    static Trigger* blood_tap(PlayerbotAI* botAI) { return new BloodTapTrigger(botAI); }
    static Trigger* raise_dead(PlayerbotAI* botAI) { return new RaiseDeadTrigger(botAI); }
    static Trigger* chains_of_ice(PlayerbotAI* botAI) { return new ChainsOfIceSnareTrigger(botAI); }
    static Trigger* pillar_of_frost(PlayerbotAI* botAI) { return new PillarOfFrostTrigger(botAI); }  // #601
    static Trigger* army_of_the_dead(PlayerbotAI* botAI) { return new ArmyOfTheDeadTrigger(botAI); }
    static Trigger* hysteria_no_cd(PlayerbotAI* botAI) { return new HysteriaNoCooldownTrigger(botAI); }
    static Trigger* unholy_frenzy(PlayerbotAI* botAI) { return new UnholyFrenzyTrigger(botAI); }
    static Trigger* dk_outbreak(PlayerbotAI* botAI) { return new DKOutbreakTrigger(botAI); }
    static Trigger* dk_festering(PlayerbotAI* botAI) { return new DKDiseaseExpiringTrigger(botAI, "dk festering", 2000); }
    static Trigger* dk_blood_boil(PlayerbotAI* botAI) { return new DKBloodBoilTrigger(botAI); }
    static Trigger* dk_frost_strike_proc(PlayerbotAI* botAI) { return new DKFrostStrikeProcTrigger(botAI); }
    static Trigger* dk_death_coil_proc(PlayerbotAI* botAI) { return new DKDeathCoilProcTrigger(botAI); }
    static Trigger* dk_chains_of_ice_player(PlayerbotAI* botAI) { return new DKChainsOfIcePlayerTrigger(botAI); }
    static Trigger* dk_pvp_enemy_caster(PlayerbotAI* botAI) { return new DKPvpEnemyCasterTrigger(botAI); }
    static Trigger* dk_health_40(PlayerbotAI* botAI) { return new HealthInRangeTrigger(botAI, "dk health 40", 40.0f); }
    static Trigger* dk_health_90(PlayerbotAI* botAI) { return new HealthInRangeTrigger(botAI, "dk health 90", 90.0f); }
};

class DeathKnightAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    DeathKnightAiObjectContextInternal()
    {
        // Unholy
        creators["bone shield"] = &DeathKnightAiObjectContextInternal::bone_shield;
        creators["plague strike"] = &DeathKnightAiObjectContextInternal::plague_strike;
        creators["plague strike on attacker"] = &DeathKnightAiObjectContextInternal::plague_strike_on_attacker;
        creators["death grip"] = &DeathKnightAiObjectContextInternal::death_grip;
        creators["death coil"] = &DeathKnightAiObjectContextInternal::death_coil;
        creators["death strike"] = &DeathKnightAiObjectContextInternal::death_strike;
        creators["unholy blight"] = &DeathKnightAiObjectContextInternal::unholy_blight;
        creators["scourge strike"] = &DeathKnightAiObjectContextInternal::scourge_strike;
        creators["death and decay"] = &DeathKnightAiObjectContextInternal::death_and_decay;
        creators["unholy presence"] = &DeathKnightAiObjectContextInternal::unholy_presence;
        creators["raise dead"] = &DeathKnightAiObjectContextInternal::raise_dead;
        creators["army of the dead"] = &DeathKnightAiObjectContextInternal::army_of_the_dead;
        creators["summon gargoyle"] = &DeathKnightAiObjectContextInternal::summon_gargoyle;
        creators["anti magic shell"] = &DeathKnightAiObjectContextInternal::anti_magic_shell;
        creators["anti magic zone"] = &DeathKnightAiObjectContextInternal::anti_magic_zone;
        creators["ghoul frenzy"] = &DeathKnightAiObjectContextInternal::ghoul_frenzy;
        creators["corpse explosion"] = &DeathKnightAiObjectContextInternal::corpse_explosion;
        // Frost
        creators["icy touch"] = &DeathKnightAiObjectContextInternal::icy_touch;
        creators["icy touch on attacker"] = &DeathKnightAiObjectContextInternal::icy_touch_on_attacker;
        creators["obliterate"] = &DeathKnightAiObjectContextInternal::obliterate;
        creators["howling blast"] = &DeathKnightAiObjectContextInternal::howling_blast;
        creators["frost strike"] = &DeathKnightAiObjectContextInternal::frost_strike;
        creators["chains of ice"] = &DeathKnightAiObjectContextInternal::chains_of_ice;
        creators["rune strike"] = &DeathKnightAiObjectContextInternal::rune_strike;
        // creators["icy clutch"] = &DeathKnightAiObjectContextInternal::icy_clutch;
        creators["horn of winter"] = &DeathKnightAiObjectContextInternal::horn_of_winter;
        creators["killing machine"] = &DeathKnightAiObjectContextInternal::killing_machine;
        creators["frost presence"] = &DeathKnightAiObjectContextInternal::frost_presence;
        creators["deathchill"] = &DeathKnightAiObjectContextInternal::deathchill;
        creators["icebound fortitude"] = &DeathKnightAiObjectContextInternal::icebound_fortitude;
        creators["mind freeze"] = &DeathKnightAiObjectContextInternal::mind_freeze;
        creators["empower rune weapon"] = &DeathKnightAiObjectContextInternal::empower_rune_weapon;
        creators["pillar of frost"] = &DeathKnightAiObjectContextInternal::pillar_of_frost;  // #601
        // blood
        creators["blood strike"] = &DeathKnightAiObjectContextInternal::blood_strike;
        creators["blood tap"] = &DeathKnightAiObjectContextInternal::blood_tap;
        creators["pestilence"] = &DeathKnightAiObjectContextInternal::pestilence;
        creators["strangulate"] = &DeathKnightAiObjectContextInternal::strangulate;
        creators["blood boil"] = &DeathKnightAiObjectContextInternal::blood_boil;
        creators["heart strike"] = &DeathKnightAiObjectContextInternal::heart_strike;
        creators["blood presence"] = &DeathKnightAiObjectContextInternal::blood_presence;
        creators["rune tap"] = &DeathKnightAiObjectContextInternal::rune_tap;
        creators["vampiric blood"] = &DeathKnightAiObjectContextInternal::vampiric_blood;
        creators["death pact"] = &DeathKnightAiObjectContextInternal::death_pact;
        // creators["hysteria"] = &DeathKnightAiObjectContextInternal::hysteria;
        creators["dancing rune weapon"] = &DeathKnightAiObjectContextInternal::dancing_rune_weapon;
        creators["dark command"] = &DeathKnightAiObjectContextInternal::dark_command;
        // was defined below but never registered, so "mind freeze on enemy healer" (GenericDKStrategy.cpp)
        // had no action to run even before this pass added its trigger.
        creators["mind freeze on enemy healer"] = &DeathKnightAiObjectContextInternal::mind_freeze_on_enemy_healer;
        // rot-lane-2 (#857): MoP rotation actions. Removed dead names: hungering cold, unbreakable armor,
        // improved icy talons, mark of blood, death rune mastery (forkonly_verdicts.txt; deathchill is kept because
        // Utils/EncounterHelpers.cpp:484 references CastDeathchillAction).
        creators["mind freeze on enemy caster"] = &DeathKnightAiObjectContextInternal::mind_freeze_on_enemy_caster;
        creators["strangulate on enemy caster"] = &DeathKnightAiObjectContextInternal::strangulate_on_enemy_caster;
        creators["festering strike"] = &DeathKnightAiObjectContextInternal::festering_strike;
        creators["outbreak"] = &DeathKnightAiObjectContextInternal::outbreak;
        creators["soul reaper"] = &DeathKnightAiObjectContextInternal::soul_reaper;
        creators["unholy frenzy"] = &DeathKnightAiObjectContextInternal::unholy_frenzy;
    }

private:
    // Unholy
    static Action* bone_shield(PlayerbotAI* botAI) { return new CastBoneShieldAction(botAI); }
    static Action* plague_strike(PlayerbotAI* botAI) { return new CastPlagueStrikeAction(botAI); }
    static Action* plague_strike_on_attacker(PlayerbotAI* botAI) { return new CastPlagueStrikeOnAttackerAction(botAI); }
    static Action* death_grip(PlayerbotAI* botAI) { return new CastDeathGripAction(botAI); }
    static Action* death_coil(PlayerbotAI* botAI) { return new CastDeathCoilAction(botAI); }
    static Action* death_strike(PlayerbotAI* botAI) { return new CastDeathStrikeAction(botAI); }
    static Action* unholy_blight(PlayerbotAI* botAI) { return new CastUnholyBlightAction(botAI); }
    static Action* scourge_strike(PlayerbotAI* botAI) { return new CastScourgeStrikeAction(botAI); }
    static Action* death_and_decay(PlayerbotAI* botAI) { return new CastDeathAndDecayAction(botAI); }
    static Action* unholy_presence(PlayerbotAI* botAI) { return new CastUnholyPresenceAction(botAI); }
    static Action* raise_dead(PlayerbotAI* botAI) { return new CastRaiseDeadAction(botAI); }
    static Action* army_of_the_dead(PlayerbotAI* botAI) { return new CastArmyOfTheDeadAction(botAI); }
    static Action* summon_gargoyle(PlayerbotAI* botAI) { return new CastSummonGargoyleAction(botAI); }
    static Action* anti_magic_shell(PlayerbotAI* botAI) { return new CastAntiMagicShellAction(botAI); }
    static Action* anti_magic_zone(PlayerbotAI* botAI) { return new CastAntiMagicZoneAction(botAI); }
    static Action* ghoul_frenzy(PlayerbotAI* botAI) { return new CastGhoulFrenzyAction(botAI); }
    static Action* corpse_explosion(PlayerbotAI* botAI) { return new CastCorpseExplosionAction(botAI); }
    // Frost
    static Action* icy_touch(PlayerbotAI* botAI) { return new CastIcyTouchAction(botAI); }
    static Action* icy_touch_on_attacker(PlayerbotAI* botAI) { return new CastIcyTouchOnAttackerAction(botAI); }
    static Action* obliterate(PlayerbotAI* botAI) { return new CastObliterateAction(botAI); }
    static Action* howling_blast(PlayerbotAI* botAI) { return new CastHowlingBlastAction(botAI); }
    static Action* frost_strike(PlayerbotAI* botAI) { return new CastFrostStrikeAction(botAI); }
    static Action* chains_of_ice(PlayerbotAI* botAI) { return new CastChainsOfIceAction(botAI); }
    static Action* rune_strike(PlayerbotAI* botAI) { return new CastRuneStrikeAction(botAI); }
    // static Action* icy_clutch(PlayerbotAI* botAI) { return new CastIcyClutchAction(botAI); }
    static Action* horn_of_winter(PlayerbotAI* botAI) { return new CastHornOfWinterAction(botAI); }
    static Action* killing_machine(PlayerbotAI* botAI) { return new CastKillingMachineAction(botAI); }
    static Action* frost_presence(PlayerbotAI* botAI) { return new CastFrostPresenceAction(botAI); }
    static Action* deathchill(PlayerbotAI* botAI) { return new CastDeathchillAction(botAI); }
    static Action* icebound_fortitude(PlayerbotAI* botAI) { return new CastIceboundFortitudeAction(botAI); }
    static Action* mind_freeze(PlayerbotAI* botAI) { return new CastMindFreezeAction(botAI); }
    static Action* empower_rune_weapon(PlayerbotAI* botAI) { return new CastEmpowerRuneWeaponAction(botAI); }
    static Action* pillar_of_frost(PlayerbotAI* botAI) { return new CastPillarOfFrostAction(botAI); }  // #601
    // blood
    static Action* blood_strike(PlayerbotAI* botAI) { return new CastBloodStrikeAction(botAI); }
    static Action* blood_tap(PlayerbotAI* botAI) { return new CastBloodTapAction(botAI); }
    static Action* pestilence(PlayerbotAI* botAI) { return new CastPestilenceAction(botAI); }
    static Action* strangulate(PlayerbotAI* botAI) { return new CastStrangulateAction(botAI); }
    static Action* blood_boil(PlayerbotAI* botAI) { return new CastBloodBoilAction(botAI); }
    static Action* heart_strike(PlayerbotAI* botAI) { return new CastHeartStrikeAction(botAI); }
    static Action* blood_presence(PlayerbotAI* botAI) { return new CastBloodPresenceAction(botAI); }
    static Action* rune_tap(PlayerbotAI* botAI) { return new CastRuneTapAction(botAI); }
    static Action* vampiric_blood(PlayerbotAI* botAI) { return new CastVampiricBloodAction(botAI); }
    static Action* death_pact(PlayerbotAI* botAI) { return new CastDeathPactAction(botAI); }
    // static Action* hysteria(PlayerbotAI* botAI) { return new CastHysteriaAction(botAI); }
    static Action* dancing_rune_weapon(PlayerbotAI* botAI) { return new CastDancingRuneWeaponAction(botAI); }
    static Action* dark_command(PlayerbotAI* botAI) { return new CastDarkCommandAction(botAI); }
    static Action* mind_freeze_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new CastMindFreezeOnEnemyHealerAction(botAI);
    }
    static Action* mind_freeze_on_enemy_caster(PlayerbotAI* botAI)
    {
        return new CastMindFreezeOnEnemyCasterAction(botAI);
    }
    static Action* strangulate_on_enemy_caster(PlayerbotAI* botAI)
    {
        return new CastStrangulateOnEnemyCasterAction(botAI);
    }
    static Action* festering_strike(PlayerbotAI* botAI) { return new CastFesteringStrikeAction(botAI); }
    static Action* outbreak(PlayerbotAI* botAI) { return new CastOutbreakAction(botAI); }
    static Action* soul_reaper(PlayerbotAI* botAI) { return new CastSoulReaperAction(botAI); }
    static Action* unholy_frenzy(PlayerbotAI* botAI) { return new CastUnholyFrenzyAction(botAI); }
};

DKAiObjectContext::DKAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new DeathKnightStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightCombatStrategyFactoryInternal());
    strategyContexts.Add(new DeathKnightDKBuffStrategyFactoryInternal());
    actionContexts.Add(new DeathKnightAiObjectContextInternal());
    triggerContexts.Add(new DeathKnightTriggerFactoryInternal());
}
