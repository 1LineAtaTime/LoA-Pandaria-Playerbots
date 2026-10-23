/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WarriorAIObjectContext.h"

#include "ArmsWarriorStrategy.h"
#include "FuryWarriorStrategy.h"
#include "GenericWarriorNonCombatStrategy.h"
#include "MeleeCombatStrategy.h"
#include "WarriorPullStrategy.h"
#include "NamedObjectContext.h"
#include "Playerbots.h"
#include "TankWarriorStrategy.h"
#include "WarriorActions.h"
#include "WarriorTriggers.h"

class WarriorStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarriorStrategyFactoryInternal()
    {
        creators["nc"] = &WarriorStrategyFactoryInternal::nc;
        // #408 (AC WarriorAiObjectContext.cpp:31)
        creators["pull"] = &WarriorStrategyFactoryInternal::pull;
        creators["aoe"] = &WarriorStrategyFactoryInternal::aoe;
        // #593 (AC Ai/Base/StrategyContext.h:105,139): "behind" (SetBehindCombatStrategy) was never registered, so
        // AiFactory's addStrategy("behind") for arms/fury was a silent no-op.
        creators["behind"] = &WarriorStrategyFactoryInternal::behind;
    }

private:
    static Strategy* nc(PlayerbotAI* botAI) { return new GenericWarriorNonCombatStrategy(botAI); }
    // #408 (AC WarriorAiObjectContext.cpp:31)
    static Strategy* pull(PlayerbotAI* botAI) { return new WarriorPullStrategy(botAI); }
    static Strategy* aoe(PlayerbotAI* botAI) { return new WarriorAoeStrategy(botAI); }
    static Strategy* behind(PlayerbotAI* botAI) { return new SetBehindCombatStrategy(botAI); }  // #593
};

class WarriorCombatStrategyFactoryInternal : public NamedObjectContext<Strategy>
{
public:
    WarriorCombatStrategyFactoryInternal() : NamedObjectContext<Strategy>(false, true)
    {
        creators["tank"] = &WarriorCombatStrategyFactoryInternal::tank;
        creators["arms"] = &WarriorCombatStrategyFactoryInternal::arms;
        creators["fury"] = &WarriorCombatStrategyFactoryInternal::fury;
    }
private:
    static Strategy* tank(PlayerbotAI* botAI) { return new TankWarriorStrategy(botAI); }
    static Strategy* arms(PlayerbotAI* botAI) { return new ArmsWarriorStrategy(botAI); }
    static Strategy* fury(PlayerbotAI* botAI) { return new FuryWarriorStrategy(botAI); }
};

class WarriorTriggerFactoryInternal : public NamedObjectContext<Trigger>
{
public:
    WarriorTriggerFactoryInternal()
    {
        creators["battle shout"] = &WarriorTriggerFactoryInternal::battle_shout;
        creators["commanding shout"] = &WarriorTriggerFactoryInternal::commanding_shout;
        creators["battle stance"] = &WarriorTriggerFactoryInternal::battle_stance;
        creators["defensive stance"] = &WarriorTriggerFactoryInternal::defensive_stance;
        creators["berserker stance"] = &WarriorTriggerFactoryInternal::berserker_stance;

        creators["victory rush"] = &WarriorTriggerFactoryInternal::victory_rush;
        creators["thunder clap"] = &WarriorTriggerFactoryInternal::thunder_clap;
        creators["sunder armor"] = &WarriorTriggerFactoryInternal::sunder_armor;
        creators["colossus smash"] = &WarriorTriggerFactoryInternal::colossus_smash;
        creators["disarm"] = &WarriorTriggerFactoryInternal::disarm;

        creators["pummel on enemy healer"] = &WarriorTriggerFactoryInternal::pummel_on_enemy_healer;
        creators["pummel"] = &WarriorTriggerFactoryInternal::pummel;

        // -- arms
        creators["overpower"] = &WarriorTriggerFactoryInternal::overpower;
        creators["mortal wounds"] = &WarriorTriggerFactoryInternal::mortal_wounds;
        creators["taste for blood"] = &WarriorTriggerFactoryInternal::taste_for_blood;

        // -- fury
        creators["raging blow"] = &WarriorTriggerFactoryInternal::raging_blow;
        creators["meat cleaver"] = &WarriorTriggerFactoryInternal::meat_cleaver;
        creators["bloodsurge"] = &WarriorTriggerFactoryInternal::bloodsurge;

        // -- prot
        creators["shield barrier"] = &WarriorTriggerFactoryInternal::shield_barrier;
        creators["shield block"] = &WarriorTriggerFactoryInternal::shield_block;
        creators["revenge"] = &WarriorTriggerFactoryInternal::revenge;
        creators["vigilance"] = &WarriorTriggerFactoryInternal::vigilance;
        // #163 S8 (AC parity)
        creators["taunt on snare target"] = &WarriorTriggerFactoryInternal::taunt_on_snare_target;

        // Issue #357 slice 10 - every name AC registers in
        // WarriorTriggerFactoryInternal (AC WarriorAiObjectContext.cpp:55-102) that
        // this fork did not. Keys are AC's exact strings.
        creators["hamstring"] = &WarriorTriggerFactoryInternal::hamstring;
        creators["death wish"] = &WarriorTriggerFactoryInternal::death_wish;
        creators["recklessness"] = &WarriorTriggerFactoryInternal::recklessness;
        creators["rend"] = &WarriorTriggerFactoryInternal::rend;
        creators["rend on attacker"] = &WarriorTriggerFactoryInternal::rend_on_attacker;
        creators["bloodrage"] = &WarriorTriggerFactoryInternal::bloodrage;
        creators["shield bash"] = &WarriorTriggerFactoryInternal::shield_bash;
        creators["shield bash on enemy healer"] = &WarriorTriggerFactoryInternal::shield_bash_on_enemy_healer;
        creators["concussion blow"] = &WarriorTriggerFactoryInternal::concussion_blow;
        creators["sword and board"] = &WarriorTriggerFactoryInternal::sword_and_board;
        creators["mocking blow"] = &WarriorTriggerFactoryInternal::mocking_blow;
        creators["rampage"] = &WarriorTriggerFactoryInternal::rampage;
        creators["mortal strike"] = &WarriorTriggerFactoryInternal::mortal_strike;
        creators["thunder clap on snare target"] = &WarriorTriggerFactoryInternal::thunder_clap_on_snare_target;
        creators["bloodthirst"] = &WarriorTriggerFactoryInternal::bloodthirst;
        creators["whirlwind"] = &WarriorTriggerFactoryInternal::whirlwind;
        creators["berserker rage"] = &WarriorTriggerFactoryInternal::berserker_rage;
        creators["intercept"] = &WarriorTriggerFactoryInternal::intercept;
        creators["intercept on enemy healer"] = &WarriorTriggerFactoryInternal::intercept_on_enemy_healer;
        creators["intercept on snare target"] = &WarriorTriggerFactoryInternal::intercept_on_snare_target;
        creators["intercept can cast"] = &WarriorTriggerFactoryInternal::intercept_can_cast;
        creators["intercept and far enemy"] = &WarriorTriggerFactoryInternal::intercept_and_far_enemy;
        creators["intercept and rage"] = &WarriorTriggerFactoryInternal::intercept_and_rage;
        creators["spell reflection"] = &WarriorTriggerFactoryInternal::spell_reflection;
        creators["sudden death"] = &WarriorTriggerFactoryInternal::sudden_death;
        creators["instant slam"] = &WarriorTriggerFactoryInternal::instant_slam;
        creators["shockwave"] = &WarriorTriggerFactoryInternal::shockwave;
        creators["shockwave on snare target"] = &WarriorTriggerFactoryInternal::shockwave_on_snare_target;
        creators["thunder clap and rage"] = &WarriorTriggerFactoryInternal::thunder_clap_and_rage;
        creators["shattering throw trigger"] = &WarriorTriggerFactoryInternal::shattering_throw_trigger;
        // #522 (AC Ai/Base/TriggerContext.h:73): not in this fork's shared TriggerContext.
        creators["fear sleep sap"] = &WarriorTriggerFactoryInternal::fear_sleep_sap;
        // Lane 11 (#866): MoP thresholds + always-on cooldowns.
        creators["warrior health 20"] = &WarriorTriggerFactoryInternal::warrior_health_20;
        creators["warrior health 40"] = &WarriorTriggerFactoryInternal::warrior_health_40;
        creators["warrior health 50"] = &WarriorTriggerFactoryInternal::warrior_health_50;
        creators["avatar"] = &WarriorTriggerFactoryInternal::avatar;
        creators["bloodbath buff"] = &WarriorTriggerFactoryInternal::bloodbath_buff;
        creators["skull banner can cast"] = &WarriorTriggerFactoryInternal::skull_banner_can_cast;
    }
private:
    static Trigger* battle_shout(PlayerbotAI* botAI) { return new BattleShoutTrigger(botAI); }
    static Trigger* commanding_shout(PlayerbotAI* botAI) { return new CommandingShoutTrigger(botAI); }
    static Trigger* defensive_stance(PlayerbotAI* botAI) { return new DefensiveStanceTrigger(botAI); }
    static Trigger* berserker_stance(PlayerbotAI* botAI) { return new BerserkerStanceTrigger(botAI); }
    static Trigger* battle_stance(PlayerbotAI* botAI) { return new BattleStanceTrigger(botAI); }

    static Trigger* victory_rush(PlayerbotAI* botAI) { return new VictoryRushTrigger(botAI); }
    static Trigger* thunder_clap(PlayerbotAI* botAI) { return new ThunderClapTrigger(botAI); }
    static Trigger* sunder_armor(PlayerbotAI* botAI) { return new SunderArmorTrigger(botAI); }
    static Trigger* colossus_smash(PlayerbotAI* botAI) { return new ColossusSmashTrigger(botAI); }
    static Trigger* disarm(PlayerbotAI* botAI) { return new DisarmDebuffTrigger(botAI); }

    static Trigger* pummel(PlayerbotAI* botAI) { return new PummelInterruptSpellTrigger(botAI); }
    static Trigger* pummel_on_enemy_healer(PlayerbotAI* botAI) { return new PummelInterruptEnemyHealerSpellTrigger(botAI); }

    // -- arms
    static Trigger* overpower(PlayerbotAI* botAI) { return new OverpowerAvailableTrigger(botAI); }
    static Trigger* mortal_wounds(PlayerbotAI* botAI) { return new MortalWoundsDebuffTrigger(botAI); }
    static Trigger* taste_for_blood(PlayerbotAI* botAI) { return new TasteForBloodTrigger(botAI); }
    
    // -- fury
    static Trigger* raging_blow(PlayerbotAI* botAI) { return new RagingBlowAvailableTrigger(botAI); }
    static Trigger* meat_cleaver(PlayerbotAI* botAI) { return new MeatCleaverTrigger(botAI); }
    static Trigger* bloodsurge(PlayerbotAI* botAI) { return new BloodSurgeTrigger(botAI); }

    // -- prot
    static Trigger* shield_block(PlayerbotAI* botAI) { return new ShieldBlockTrigger(botAI); }
    static Trigger* shield_barrier(PlayerbotAI* botAI) { return new ShieldBarrierTrigger(botAI); }
    static Trigger* revenge(PlayerbotAI* botAI) { return new RevengeAvailableTrigger(botAI); }
    static Trigger* vigilance(PlayerbotAI* botAI) { return new VigilanceTrigger(botAI); }
    static Trigger* taunt_on_snare_target(PlayerbotAI* botAI) { return new TauntSnareTrigger(botAI); }

    // Issue #357 slice 10 - factories shaped exactly like AC
    // WarriorAiObjectContext.cpp:110-171.
    static Trigger* hamstring(PlayerbotAI* botAI) { return new HamstringTrigger(botAI); }
    static Trigger* death_wish(PlayerbotAI* botAI) { return new DeathWishTrigger(botAI); }
    static Trigger* recklessness(PlayerbotAI* botAI) { return new RecklessnessTrigger(botAI); }
    static Trigger* rend(PlayerbotAI* botAI) { return new RendDebuffTrigger(botAI); }
    static Trigger* rend_on_attacker(PlayerbotAI* botAI) { return new RendDebuffOnAttackerTrigger(botAI); }
    static Trigger* bloodrage(PlayerbotAI* botAI) { return new BloodrageBuffTrigger(botAI); }
    static Trigger* shield_bash(PlayerbotAI* botAI) { return new ShieldBashInterruptSpellTrigger(botAI); }
    static Trigger* shield_bash_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new ShieldBashInterruptEnemyHealerSpellTrigger(botAI);
    }
    static Trigger* concussion_blow(PlayerbotAI* botAI) { return new ConcussionBlowTrigger(botAI); }
    static Trigger* sword_and_board(PlayerbotAI* botAI) { return new SwordAndBoardTrigger(botAI); }
    static Trigger* mocking_blow(PlayerbotAI* botAI) { return new MockingBlowTrigger(botAI); }
    static Trigger* rampage(PlayerbotAI* botAI) { return new RampageAvailableTrigger(botAI); }
    static Trigger* mortal_strike(PlayerbotAI* botAI) { return new MortalStrikeDebuffTrigger(botAI); }
    static Trigger* thunder_clap_on_snare_target(PlayerbotAI* botAI) { return new ThunderClapSnareTrigger(botAI); }
    static Trigger* bloodthirst(PlayerbotAI* botAI) { return new BloodthirstBuffTrigger(botAI); }
    static Trigger* whirlwind(PlayerbotAI* botAI) { return new WhirlwindTrigger(botAI); }
    static Trigger* berserker_rage(PlayerbotAI* botAI) { return new BerserkerRageBuffTrigger(botAI); }
    static Trigger* intercept(PlayerbotAI* botAI) { return new InterceptInterruptSpellTrigger(botAI); }
    static Trigger* intercept_on_enemy_healer(PlayerbotAI* botAI)
    {
        return new InterceptInterruptEnemyHealerSpellTrigger(botAI);
    }
    static Trigger* intercept_on_snare_target(PlayerbotAI* botAI) { return new InterceptSnareTrigger(botAI); }
    static Trigger* intercept_can_cast(PlayerbotAI* botAI) { return new InterceptCanCastTrigger(botAI); }
    static Trigger* intercept_and_far_enemy(PlayerbotAI* botAI)
    {
        // AC's exact string (AC WarriorAiObjectContext.cpp:133). AC's own base
        // context only registers "enemy out of melee" (AC Ai/Base/TriggerContext.h:110),
        // so this resolves to null there too and TwoTriggers::IsActive() returns false -
        // ported unchanged rather than silently "fixed".
        return new TwoTriggers(botAI, "enemy is out of melee", "intercept can cast");
    }
    static Trigger* intercept_and_rage(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "intercept and far enemy", "light rage available");
    }
    static Trigger* thunder_clap_and_rage(PlayerbotAI* botAI)
    {
        return new TwoTriggers(botAI, "thunder clap", "light rage available");
    }
    static Trigger* spell_reflection(PlayerbotAI* botAI) { return new SpellReflectionTrigger(botAI); }
    static Trigger* sudden_death(PlayerbotAI* botAI) { return new SuddenDeathTrigger(botAI); }
    static Trigger* instant_slam(PlayerbotAI* botAI) { return new SlamInstantTrigger(botAI); }
    static Trigger* shockwave(PlayerbotAI* botAI) { return new ShockwaveTrigger(botAI); }
    static Trigger* shockwave_on_snare_target(PlayerbotAI* botAI) { return new ShockwaveSnareTrigger(botAI); }
    static Trigger* shattering_throw_trigger(PlayerbotAI* botAI) { return new ShatteringThrowTrigger(botAI); }
    static Trigger* fear_sleep_sap(PlayerbotAI* botAI) { return new WarriorFearSleepSapTrigger(botAI); }
    static Trigger* warrior_health_20(PlayerbotAI* botAI) { return new WarriorHealth20Trigger(botAI); }
    static Trigger* warrior_health_40(PlayerbotAI* botAI) { return new WarriorHealth40Trigger(botAI); }
    static Trigger* warrior_health_50(PlayerbotAI* botAI) { return new WarriorHealth50Trigger(botAI); }
    static Trigger* avatar(PlayerbotAI* botAI) { return new AvatarTrigger(botAI); }
    static Trigger* bloodbath_buff(PlayerbotAI* botAI) { return new BloodbathBuffTrigger(botAI); }
    static Trigger* skull_banner_can_cast(PlayerbotAI* botAI) { return new SkullBannerCanCastTrigger(botAI); }
};

class WarriorAiObjectContextInternal : public NamedObjectContext<Action>
{
public:
    WarriorAiObjectContextInternal()
    {
        // -- buffs
        creators["battle shout"] = &WarriorAiObjectContextInternal::battle_shout;
        creators["commanding shout"] = &WarriorAiObjectContextInternal::commanding_shout;
        creators["battle stance"] = &WarriorAiObjectContextInternal::battle_stance;
        creators["berserker stance"] = &WarriorAiObjectContextInternal::berserker_stance;
        creators["defensive stance"] = &WarriorAiObjectContextInternal::defensive_stance;

        // -- general
        creators["charge"] = &WarriorAiObjectContextInternal::charge;
        creators["heroic strike"] = &WarriorAiObjectContextInternal::heroic_strike;
        creators["heroic leap"] = &WarriorAiObjectContextInternal::heroic_leap;
        creators["victory rush"] = &WarriorAiObjectContextInternal::victory_rush;
        creators["execute"] = &WarriorAiObjectContextInternal::execute;
        creators["sunder armor"] = &WarriorAiObjectContextInternal::sunder_armor;

        creators["bloodbath"] = &WarriorAiObjectContextInternal::bloodbath;
        creators["berserker rage"] = &WarriorAiObjectContextInternal::berserker_rage;
        creators["rallying cry"] = &WarriorAiObjectContextInternal::rallying_cry;
        creators["shield wall"] = &WarriorAiObjectContextInternal::shield_wall;
        creators["recklessness"] = &WarriorAiObjectContextInternal::recklessness;
        creators["colossus smash"] = &WarriorAiObjectContextInternal::colossus_smash;
        creators["disarm"] = &WarriorAiObjectContextInternal::disarm;
        creators["taunt"] = &WarriorAiObjectContextInternal::taunt;
        // #163 S8 (AC parity)
        creators["taunt on snare target"] = &WarriorAiObjectContextInternal::taunt_on_snare_target;
        creators["mocking banner"] = &WarriorAiObjectContextInternal::mocking_banner;

        // -- aoe
        creators["thunder clap"] = &WarriorAiObjectContextInternal::thunder_clap;
        creators["whirlwind"] = &WarriorAiObjectContextInternal::whirlwind;
        creators["cleave"] = &WarriorAiObjectContextInternal::cleave;
        creators["bladestorm"] = &WarriorAiObjectContextInternal::bladestorm;
        // #522: key was "dragonroar" (never matched the spell name "Dragon Roar", and the aoe strategy
        // fired "dragon_roar", which no context created - audit "Action not visible"). Spell lookup is an
        // exact lower-cased name match (strategy/value/SpellIdValue.cpp), so the key is now "dragon roar".
        creators["dragon roar"] = &WarriorAiObjectContextInternal::dragonroar;
        creators["shockwave"] = &WarriorAiObjectContextInternal::shockwave;

        // -- arms
        creators["mortal strike"] = &WarriorAiObjectContextInternal::mortal_strike;
        creators["slam"] = &WarriorAiObjectContextInternal::slam;
        creators["overpower"] = &WarriorAiObjectContextInternal::overpower;
        creators["die by the sword"] = &WarriorAiObjectContextInternal::die_by_the_sword;
        creators["sweeping strikes"] = &WarriorAiObjectContextInternal::sweeping_strikes;
        creators["demoralizing banner"] = &WarriorAiObjectContextInternal::demoralizing_banner;

        // -- fury
        creators["skull banner"] = &WarriorAiObjectContextInternal::skull_banner;
        creators["raging blow"] = &WarriorAiObjectContextInternal::raging_blow;
        creators["bloodthirst"] = &WarriorAiObjectContextInternal::bloodthirst;
        creators["wild strike"] = &WarriorAiObjectContextInternal::wild_strike;

        // -- prot
        creators["demoralizing shout"] = &WarriorAiObjectContextInternal::demoralizing_shout;
        creators["devastate"] = &WarriorAiObjectContextInternal::devastate;
        creators["last stand"] = &WarriorAiObjectContextInternal::last_stand;
        creators["revenge"] = &WarriorAiObjectContextInternal::revenge;
        creators["shield barrier"] = &WarriorAiObjectContextInternal::shield_barrier;
        creators["shield block"] = &WarriorAiObjectContextInternal::shield_block;
        creators["shield slam"] = &WarriorAiObjectContextInternal::shield_slam;
        creators["enraged regeneration"] = &WarriorAiObjectContextInternal::enraged_regeneration;
        creators["vigilance"] = &WarriorAiObjectContextInternal::vigilance;
        creators["storm bolt"] = &WarriorAiObjectContextInternal::storm_bolt;
        creators["mocking banner"] = &WarriorAiObjectContextInternal::mocking_banner;
        // Issue #357 slice 10: AC registers "pummel" (AC WarriorAiObjectContext.cpp:226)
        // and GenericWarriorStrategy.cpp:78 fires NextAction("pummel"); no Action
        // context in this fork created it before.
        creators["pummel"] = &WarriorAiObjectContextInternal::pummel;

        // #522 (AC WarriorAiObjectContext.cpp:192,198-200,214,229-233,238-240,270-310): AC action keys this
        // fork lacked, all learnable in 5.4.8 (cls.py 2026-10-04).
        creators["pummel on enemy healer"] = &WarriorAiObjectContextInternal::pummel_on_enemy_healer;
        creators["hamstring"] = &WarriorAiObjectContextInternal::hamstring;
        creators["intimidating shout"] = &WarriorAiObjectContextInternal::intimidating_shout;
        creators["demoralizing shout without life time check"] =
            &WarriorAiObjectContextInternal::demoralizing_shout_without_life_time_check;
        creators["shockwave on snare target"] = &WarriorAiObjectContextInternal::shockwave_on_snare_target;
        creators["piercing howl"] = &WarriorAiObjectContextInternal::piercing_howl;
        creators["intervene"] = &WarriorAiObjectContextInternal::intervene;
        creators["spell reflection"] = &WarriorAiObjectContextInternal::spell_reflection;
        creators["thunder clap on snare target"] = &WarriorAiObjectContextInternal::thunder_clap_on_snare_target;
        creators["heroic throw"] = &WarriorAiObjectContextInternal::heroic_throw;
        creators["heroic throw on snare target"] = &WarriorAiObjectContextInternal::heroic_throw_on_snare_target;
        creators["shattering throw"] = &WarriorAiObjectContextInternal::shattering_throw;
        // Lane 11 (#866): Avatar + enemy-caster interrupts.
        creators["avatar"] = &WarriorAiObjectContextInternal::avatar;
        creators["pummel on enemy caster"] = &WarriorAiObjectContextInternal::pummel_on_enemy_caster;
        creators["disrupting shout on enemy caster"] = &WarriorAiObjectContextInternal::disrupting_shout_on_enemy_caster;
        creators["intimidating shout on enemy caster"] =
            &WarriorAiObjectContextInternal::intimidating_shout_on_enemy_caster;
    }
private:
    // -- buffs
    static Action* battle_shout(PlayerbotAI* botAI) { return new CastBattleShoutAction(botAI); }
    static Action* commanding_shout(PlayerbotAI* botAI) { return new CastCommandingShoutAction(botAI); }
    static Action* defensive_stance(PlayerbotAI* botAI) { return new CastDefensiveStanceAction(botAI); }
    static Action* battle_stance(PlayerbotAI* botAI) { return new CastBattleStanceAction(botAI); }
    static Action* berserker_stance(PlayerbotAI* botAI) { return new CastBerserkerStanceAction(botAI); }

    // -- generic
    static Action* charge(PlayerbotAI* botAI) { return new CastChargeAction(botAI); }
    static Action* heroic_strike(PlayerbotAI* botAI) { return new CastHeroicStrikeAction(botAI); }
    static Action* heroic_leap(PlayerbotAI* botAI) { return new CastHeroicLeapAction(botAI); }
    static Action* victory_rush(PlayerbotAI* botAI) { return new CastVictoryRushAction(botAI); }
    static Action* execute(PlayerbotAI* botAI) { return new CastExecuteAction(botAI); }
    static Action* sunder_armor(PlayerbotAI* botAI) { return new CastSunderArmorAction(botAI); }
    static Action* bloodbath(PlayerbotAI* botAI) { return new CastBloodbathAction(botAI); }
    static Action* berserker_rage(PlayerbotAI* botAI) { return new CastBerserkerRageAction(botAI); }
    static Action* rallying_cry(PlayerbotAI* botAI) { return new CastRallyingCryAction(botAI); }
    static Action* shield_wall(PlayerbotAI* botAI) { return new CastShieldWallAction(botAI); }
    static Action* recklessness(PlayerbotAI* botAI) { return new CastRecklessnessAction(botAI); }
    static Action* die_by_the_sword(PlayerbotAI* botAI) { return new CastDieByTheSwordAction(botAI); }
    static Action* colossus_smash(PlayerbotAI* botAI) { return new CastColossusSmashAction(botAI); }
    static Action* disarm(PlayerbotAI* botAI) { return new CastDisarmAction(botAI); }
    static Action* taunt(PlayerbotAI* botAI) { return new CastTauntAction(botAI); }
    static Action* taunt_on_snare_target(PlayerbotAI* botAI) { return new CastTauntOnSnareTargetAction(botAI); }
    static Action* mocking_banner(PlayerbotAI* botAI) { return new CastMockingBannerAction(botAI); }
    // Issue #357 slice 10: AC WarriorAiObjectContext.cpp:226 (MELEE_ACTION, AC WarriorActions.h:92).
    static Action* pummel(PlayerbotAI* botAI) { return new CastPummelAction(botAI); }
    // #522: AC WarriorAiObjectContext.cpp:227,270-310.
    static Action* pummel_on_enemy_healer(PlayerbotAI* botAI) { return new CastPummelOnEnemyHealerAction(botAI); }
    static Action* hamstring(PlayerbotAI* botAI) { return new CastHamstringAction(botAI); }
    static Action* intimidating_shout(PlayerbotAI* botAI) { return new CastIntimidatingShoutAction(botAI); }
    static Action* demoralizing_shout_without_life_time_check(PlayerbotAI* botAI)
    {
        return new CastDemoralizingShoutWithoutLifeTimeCheckAction(botAI);
    }
    static Action* shockwave_on_snare_target(PlayerbotAI* botAI) { return new CastShockwaveSnareAction(botAI); }
    static Action* piercing_howl(PlayerbotAI* botAI) { return new CastPiercingHowlAction(botAI); }
    static Action* intervene(PlayerbotAI* botAI) { return new CastInterveneAction(botAI); }
    static Action* spell_reflection(PlayerbotAI* botAI) { return new CastSpellReflectionAction(botAI); }
    static Action* thunder_clap_on_snare_target(PlayerbotAI* botAI) { return new CastThunderClapSnareAction(botAI); }
    static Action* heroic_throw(PlayerbotAI* botAI) { return new CastHeroicThrowAction(botAI); }
    static Action* heroic_throw_on_snare_target(PlayerbotAI* botAI) { return new CastHeroicThrowSnareAction(botAI); }
    static Action* shattering_throw(PlayerbotAI* botAI) { return new CastShatteringThrowAction(botAI); }
    static Action* avatar(PlayerbotAI* botAI) { return new CastAvatarAction(botAI); }
    static Action* pummel_on_enemy_caster(PlayerbotAI* botAI) { return new CastPummelOnEnemyCasterAction(botAI); }
    static Action* disrupting_shout_on_enemy_caster(PlayerbotAI* botAI)
    {
        return new CastDisruptingShoutOnEnemyCasterAction(botAI);
    }
    static Action* intimidating_shout_on_enemy_caster(PlayerbotAI* botAI)
    {
        return new CastIntimidatingShoutOnEnemyCasterAction(botAI);
    }

    // -- aoe
    static Action* whirlwind(PlayerbotAI* botAI) { return new CastWhirlWindAction(botAI); }
    static Action* thunder_clap(PlayerbotAI* botAI) { return new CastThunderClapAction(botAI); }
    static Action* cleave(PlayerbotAI* botAI) { return new CastCleaveAction(botAI); }
    static Action* bladestorm(PlayerbotAI* botAI) { return new CastBladestormAction(botAI); }
    static Action* dragonroar(PlayerbotAI* botAI) { return new CastDragonroarAction(botAI); }
    static Action* shockwave(PlayerbotAI* botAI) { return new CastShockwaveAction(botAI); }

    // -- ARMS
    static Action* demoralizing_banner(PlayerbotAI* botAI) { return new CastDemoralizingBannerAction(botAI); }
    static Action* mortal_strike(PlayerbotAI* botAI) { return new CastMortalStrikeAction(botAI); }
    static Action* slam(PlayerbotAI* botAI) { return new CastSlamAction(botAI); }
    static Action* overpower(PlayerbotAI* botAI) { return new CastOverPowerAction(botAI); }    
    static Action* sweeping_strikes(PlayerbotAI* botAI) { return new CastSweepingStrikesAction(botAI); }
    
    // -- FURY
    static Action* skull_banner(PlayerbotAI* botAI) { return new CastSkullBannerAction(botAI); }
    static Action* raging_blow(PlayerbotAI* botAI) { return new CastRagingBlowAction(botAI); }
    static Action* bloodthirst(PlayerbotAI* botAI) { return new CastBloodThirstAction(botAI); }
    static Action* wild_strike(PlayerbotAI* botAI) { return new CastWildStrikeAction(botAI); }

    // -- PROT
    static Action* demoralizing_shout(PlayerbotAI* botAI) { return new CastDemoralizingShoutAction(botAI); }
    static Action* devastate(PlayerbotAI* botAI) { return new CastDevastateAction(botAI); }
    static Action* last_stand(PlayerbotAI* botAI) { return new CastLastStandAction(botAI); }
    static Action* revenge(PlayerbotAI* botAI) { return new CastRevengeAction(botAI); }
    static Action* shield_barrier(PlayerbotAI* botAI) { return new CastShieldBarrierAction(botAI); }
    static Action* shield_block(PlayerbotAI* botAI) { return new CastShieldBlockAction(botAI); }
    static Action* shield_slam(PlayerbotAI* botAI) { return new CastShieldSlamAction(botAI); }
    static Action* enraged_regeneration(PlayerbotAI* botAI) { return new CastEnragedRegenerationAction(botAI); }
    static Action* vigilance(PlayerbotAI* botAI) { return new CastVigilanceAction(botAI); }
    static Action* storm_bolt(PlayerbotAI* botAI) { return new CastStormBoltAction(botAI); }
};

WarriorAiObjectContext::WarriorAiObjectContext(PlayerbotAI* botAI) : AiObjectContext(botAI)
{
    strategyContexts.Add(new WarriorStrategyFactoryInternal());
    strategyContexts.Add(new WarriorCombatStrategyFactoryInternal());
    actionContexts.Add(new WarriorAiObjectContextInternal());
    triggerContexts.Add(new WarriorTriggerFactoryInternal());
}
