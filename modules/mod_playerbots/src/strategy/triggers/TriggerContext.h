/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TRIGGERCONTEXT_H
#define _PLAYERBOT_TRIGGERCONTEXT_H


#include "NamedObjectContext.h"
#include "GenericTriggers.h"
#include "PullTriggers.h"
#include "LfgTriggers.h"
#include "LootTriggers.h"
#include "FishingTriggers.h"
#include "PvpTriggers.h"
#include "HealthTriggers.h"
#include "NewRpgTriggers.h"
#include "RpgTriggers.h"     // #272/#201 (AC TriggerContext.h:23)
#include "TravelTriggers.h"  // #269 (AC TriggerContext.h:26)
#include "RangeTriggers.h"
#include "Player.h"

class PlayerbotAI;

/// Ported from AC mod-playerbots GenericTriggers.h (SelfResurrectTrigger). Upstream's field is
/// named PLAYER_SELF_RES_SPELL; this MoP core calls it PLAYER_FIELD_SELF_RES_SPELL.
class SelfResurrectTrigger : public Trigger
{
public:
    SelfResurrectTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can self resurrect") {}

    bool IsActive() override { return !bot->IsAlive() && bot->GetUInt32Value(PLAYER_FIELD_SELF_RES_SPELL); }
};

/// #521 (AC Ai/Base/Trigger/GenericTriggers.h:243-247 BeingAttackedTrigger, registered at AC
/// Ai/Base/TriggerContext.h:127): true while at least one enemy attacks the bot. Shared by every class
/// (replaces the class-local mage/priest copies). Defined here, not in GenericTriggers.h, to keep the
/// rebuild small.
class BeingAttackedTrigger : public MyAttackerCountTrigger
{
public:
    BeingAttackedTrigger(PlayerbotAI* botAI) : MyAttackerCountTrigger(botAI, 1) {}
    std::string const getName() override { return "being attacked"; }
};

class TriggerContext : public NamedObjectContext<Trigger>
{
public:
    TriggerContext()
    {
        creators["timer"] = &TriggerContext::Timer;
        creators["random"] = &TriggerContext::Random;
        creators["seldom"] = &TriggerContext::seldom;
        creators["often"] = &TriggerContext::often;
        creators["very often"] = &TriggerContext::very_often;  // AC TriggerContext.h:46 (#188 loot roll)
        creators["force rebuff pending"] = &TriggerContext::force_rebuff_pending;  // AC TriggerContext.h:48 (#628)
        creators["always trigger"] = &TriggerContext::always;
        creators["random bot update"] = &TriggerContext::random_bot_update_trigger;
        // #194 (S8): NewPlayerNearbyTrigger/NewPlayerNearbyValue already existed (issue #149) but were
        // never wired into creators[] - needed for EmoteStrategy's "new player nearby" -> "greet".
        creators["new player nearby"] = &TriggerContext::new_player_nearby;

        creators["target critical health"] = &TriggerContext::TargetCriticalHealth;

        creators["critical health"] = &TriggerContext::CriticalHealth;
        // #163 S8 (AC parity)
        creators["healer low mana"] = &TriggerContext::HealerLowMana;
        creators["low health"] = &TriggerContext::LowHealth;
        creators["medium health"] = &TriggerContext::MediumHealth;
        creators["almost full health"] = &TriggerContext::AlmostFullHealth;

        creators["tank assist"] = &TriggerContext::TankAssist;
        creators["lose aggro"] = &TriggerContext::LoseAggro;
        creators["has aggro"] = &TriggerContext::HasAggro;
        creators["being attacked"] = &TriggerContext::BeingAttacked;  // #521 (AC TriggerContext.h:127)
        creators["medium threat"] = &TriggerContext::MediumThreat;
        creators["low tank threat"] = &TriggerContext::low_tank_threat;

        creators["low mana"] = &TriggerContext::LowMana;
        // #163 S8 (AC parity): the generic triggers RacialsStrategy needs.
        creators["low energy"] = &TriggerContext::LowEnergy;
        creators["generic boost"] = &TriggerContext::GenericBoost;
        creators["loss of control"] = &TriggerContext::LossOfControl;
        creators["fear charm sleep"] = &TriggerContext::FearCharmSleep;
        creators["poison disease bleed"] = &TriggerContext::PoisonDiseaseBleed;
        creators["movement impaired"] = &TriggerContext::MovementImpaired;
        creators["medium mana"] = &TriggerContext::MediumMana;
        creators["high mana"] = &TriggerContext::HighMana;
        creators["almost full mana"] = &TriggerContext::AlmostFullMana;
        creators["enough mana"] = &TriggerContext::EnoughMana;

        creators["light rage available"] = &TriggerContext::LightRageAvailable;
        creators["medium rage available"] = &TriggerContext::MediumRageAvailable;
        creators["high rage available"] = &TriggerContext::HighRageAvailable;

        creators["light energy available"] = &TriggerContext::LightEnergyAvailable;
        creators["medium energy available"] = &TriggerContext::MediumEnergyAvailable;
        creators["high energy available"] = &TriggerContext::HighEnergyAvailable;
        creators["almost full energy available"] = &TriggerContext::AlmostFullEnergyAvailable;

        creators["party member critical health"] = &TriggerContext::PartyMemberCriticalHealth;
        creators["party member low health"] = &TriggerContext::PartyMemberLowHealth;
        creators["party member medium health"] = &TriggerContext::PartyMemberMediumHealth;
        creators["party member almost full health"] = &TriggerContext::PartyMemberAlmostFullHealth;

        // #177 (AC TriggerContext.h:36,38): ReturnTrigger already existed on this fork; only the
        // registration (and ReturnToStayPositionTrigger itself, new in #177) were missing.
        creators["return"] = &TriggerContext::_return;
        creators["return to stay position"] = &TriggerContext::return_to_stay_position;
        creators["collision"] = &TriggerContext::collision;  // #271 AC TriggerContext.h:39
        creators["dead"] = &TriggerContext::Dead;
        // AC TriggerContext.h:200. CombatStrategy (and the monk strategies) schedule "check mount state"
        // on this trigger; without the registration a bot that mounted and then entered combat never
        // dismounted, and Unit::Attack refuses a mounted player, so it stood with a target forever (#134).
        creators["mounted"] = &TriggerContext::mounted;
        creators["corpse near"] = &TriggerContext::corpse_near;
        // #149 (AC Base/Trigger/LootTriggers.h): LootAvailableTrigger is generic on AC, so every
        // class sees it (e.g. DK's UnholyDKStrategy.cpp corpse-explosion-on-loot line). Defined in
        // GenericTriggers.h; see the class comment there for why it does not need AC's
        // LootObjectStack/"has available loot" value.
        creators["loot available"] = &TriggerContext::loot_available;
        // #151 (AC TriggerContext.h:155-156): the rest of the loot-stack triggers.
        creators["far from loot target"] = &TriggerContext::far_from_loot_target;
        creators["can loot"] = &TriggerContext::can_loot;
        // #412 (AC TriggerContext.h:248-249): fishing.
        creators["can fish"] = &TriggerContext::can_fish;
        creators["can use fishing bobber"] = &TriggerContext::can_use_fishing_bobber;
        // #149: PartyMemberDeadTrigger/CombatPartyMemberDeadTrigger already existed in HealthTriggers.h
        // (and their factory methods below already existed), they were just never wired into creators[],
        // so DeadStrategy/CombatStrategy could never schedule a resurrect on either name.
        creators["party member dead"] = &TriggerContext::PartyMemberDead;
        creators["combat party member dead"] = &TriggerContext::CombatPartyMemberDead;
        // Ported from AC mod-playerbots TriggerContext.h:212 (falling far) and :247 (can self
        // resurrect); see DeadStrategy.cpp. IsFallingFarTrigger lives in GenericTriggers.h,
        // SelfResurrectTrigger is defined above.
        creators["falling far"] = &TriggerContext::falling_far;
        creators["can self resurrect"] = &TriggerContext::can_self_resurrect;

        creators["light aoe"] = &TriggerContext::LightAoe;
        creators["medium aoe"] = &TriggerContext::MediumAoe;
        creators["high aoe"] = &TriggerContext::HighAoe;

        // #149 (AC TriggerContext.h:160-166, :105-106): AoeHealTrigger/AoeInGroupTrigger/
        // HealerShouldAttackTrigger and TwoTriggers already existed in HealthTriggers.h/GenericTriggers.h,
        // only the registration was missing, so heal strategies never scheduled AoE heals.
        creators["critical aoe heal"] = &TriggerContext::critical_aoe_heal;
        creators["medium aoe heal"] = &TriggerContext::medium_aoe_heal;
        // #163 S8 (AC parity, TriggerContext.h:161,163): same shape as critical/medium above.
        creators["low aoe heal"] = &TriggerContext::low_aoe_heal;
        creators["almost full aoe heal"] = &TriggerContext::almost_full_aoe_heal;
        creators["group heal setting"] = &TriggerContext::group_heal_setting;
        creators["medium group heal setting"] = &TriggerContext::medium_group_heal_setting;
        creators["healer should attack"] = &TriggerContext::healer_should_attack;
        creators["medium aoe and healer should attack"] = &TriggerContext::medium_aoe_and_healer_should_attack;

        // #149 (AC GenericTriggers.h:105-138): ComboPointsAvailableTrigger/ComboPointsNotFullTrigger/
        // TargetWithComboPointsLowerHealTrigger ported below (StatAvailable, "combo" value and
        // "estimated group dps" value already existed on this fork).
        creators["combo points 3 available"] = &TriggerContext::combo_points_3_available;
        creators["combo points 4 available"] = &TriggerContext::combo_points_4_available;
        creators["combo points 5 available"] = &TriggerContext::combo_points_5_available;
        // BEYOND-AC: AC registers only "combo points 3/4/5 available", not the bare name.
        creators["combo points available"] = &TriggerContext::combo_points_available;
        creators["combo points not full"] = &TriggerContext::combo_points_not_full;
        creators["combo points not full and high energy"] = &TriggerContext::combo_points_not_full_and_high_energy;
        creators["target with combo points almost dead"] = &TriggerContext::target_with_combo_points_almost_dead;

        // #149 (AC StuckTriggers.cpp:144-170): CombatStuckTrigger ported below; "current position" and
        // "combat" values already existed on this fork.
        creators["combat stuck"] = &TriggerContext::combat_stuck;

        // #149 (AC GenericTriggers.h, SitTrigger/IsSwimmingTrigger/HasNearestAddsTrigger): these classes
        // already existed (StayTimeTrigger-based / ported), only the registration was missing.
        creators["sit"] = &TriggerContext::sit;
        creators["swimming"] = &TriggerContext::swimming;
        creators["has nearest adds"] = &TriggerContext::has_nearest_adds;

        creators["panic"] = &TriggerContext::panic;
        creators["outnumbered"] = &TriggerContext::outnumbered;
        creators["behind target"] = &TriggerContext::behind_target;
        creators["not behind target"] = &TriggerContext::not_behind_target;
        creators["not facing target"] = &TriggerContext::not_facing_target;

        creators["enemy out of melee"] = &TriggerContext::EnemyOutOfMelee;
        creators["enemy out of spell"] = &TriggerContext::EnemyOutOfSpell;
        creators["enemy too close for spell"] = &TriggerContext::enemy_too_close_for_spell;
        creators["enemy too close for shoot"] = &TriggerContext::enemy_too_close_for_shoot;
        creators["enemy too close for auto shot"] = &TriggerContext::enemy_too_close_for_auto_shot;
        creators["enemy too close for melee"] = &TriggerContext::enemy_too_close_for_melee;
        creators["enemy is close"] = &TriggerContext::enemy_is_close;
        creators["enemy within melee"] = &TriggerContext::enemy_within_melee;
        creators["not dps target active"] = &TriggerContext::not_dps_target_active;
        creators["not dps aoe target active"] = &TriggerContext::not_dps_aoe_target_active;
        creators["enemy player near"] = &TriggerContext::enemy_player_near;
        // #855 (epic #854, rotation ports): shared triggers every class lane uses (GenericTriggers.h).
        creators["enemy caster in range"] = &TriggerContext::enemy_caster_in_range;
        creators["bot is cc"] = &TriggerContext::bot_is_cc;
        creators["enemy player in melee range"] = &TriggerContext::enemy_player_in_melee_range;

        creators["party member to heal out of spell range"] = &TriggerContext::party_member_to_heal_out_of_spell_range;
        creators["no target"] = &TriggerContext::NoTarget;

        // #408 (AC TriggerContext.h): the two PullStrategy trigger nodes plus PullBackStrategy's.
        creators["pull start"] = &TriggerContext::pull_start;
        creators["pull end"] = &TriggerContext::pull_end;
        creators["return to pull position"] = &TriggerContext::return_to_pull_position;
        creators["target changed"] = &TriggerContext::target_changed;
        creators["invalid target"] = &TriggerContext::invalid_target;

        creators["no attackers"] = &TriggerContext::NoAttackers;
        creators["duel opponent untargeted"] = &TriggerContext::DuelOpponentUntargeted;  // #868
        creators["has attackers"] = &TriggerContext::has_attackers;
        creators["no possible targets"] = &TriggerContext::no_possible_targets;
        creators["possible adds"] = &TriggerContext::possible_adds;

        creators["no pet"] = &TriggerContext::no_pet;
        // #163 S8 / #147 (AC parity): fires once when a new pet/guardian appears, for pet-stance wiring.
        creators["new pet"] = &TriggerContext::new_pet;
        // #163 S8 (AC parity): no raid-target icon assigned, for the "mark rti" strategy.
        creators["no rti target"] = &TriggerContext::no_rti_target;
        // #163 S8 gap fix (a): NoFoodTrigger/NoDrinkTrigger existed but were never registered.
        creators["no food"] = &TriggerContext::no_food;
        creators["no drink"] = &TriggerContext::no_drink;
        creators["has pet"] = &TriggerContext::has_pet;
        creators["pet attack"] = &TriggerContext::pet_attack;

        creators["protect party member"] = &TriggerContext::protect_party_member;

        // new rpg (runtime roaming) status triggers
        creators["go grind status"] = &TriggerContext::go_grind_status;
        creators["go camp status"] = &TriggerContext::go_camp_status;
        creators["wander random status"] = &TriggerContext::wander_random_status;
        creators["wander npc status"] = &TriggerContext::wander_npc_status;
        creators["do quest status"] = &TriggerContext::do_quest_status;
        creators["travel flight status"] = &TriggerContext::travel_flight_status;
        creators["outdoor pvp status"] = &TriggerContext::outdoor_pvp_status;
        creators["duel spot status"] = &TriggerContext::duel_spot_status;  // #201 (#612): AC TriggerContext.h:246 pattern

        // Battleground triggers (#33 slice B). The in-battleground strategies fire on these.
        creators["bg waiting"] = &TriggerContext::bg_waiting;
        creators["bg active"] = &TriggerContext::bg_active;
        creators["bg invite active"] = &TriggerContext::bg_invite_active;
        creators["bg queue dead"] = &TriggerContext::bg_queue_dead;
        creators["player has flag"] = &TriggerContext::player_has_flag;
        creators["enemy flagcarrier near"] = &TriggerContext::enemy_flagcarrier_near;
        creators["team flagcarrier near"] = &TriggerContext::team_flagcarrier_near;
        creators["timer bg"] = &TriggerContext::timer_bg;
        creators["alliance no snowfall gy"] = &TriggerContext::alliance_no_snowfall_gy;
        // #393 (AC TriggerContext.h:207-208): Isle of Conquest vehicle triggers.
        creators["vehicle near"] = &TriggerContext::vehicle_near;
        creators["in vehicle"] = &TriggerContext::in_vehicle;

        // #210 S10 (AC TriggerContext.h): the "lfg" strategy's own triggers (LfgStrategy.cpp). Dormant
        // until AiFactory adds "lfg" to an engine (#142) - see LfgStrategy.h.
        creators["lfg proposal active"] = &TriggerContext::lfg_proposal_active;
        creators["unknown dungeon"] = &TriggerContext::unknown_dungeon;

        // #269/#272/#201 (owner rule 2026-10-05: port AC 1:1 including dormant code). Registered exactly as
        // AC registers them. Dormant as in AC: only the "rpg" / "travel" strategies name these triggers, and
        // nothing adds those strategies by default (AC AiFactory.cpp:616-622). "rpg duel" and the three
        // dark-portal triggers are named by no TriggerNode at all, in AC either.
        // AC TriggerContext.h:175-180
        creators["no rpg target"] = &TriggerContext::no_rpg_target;
        creators["has rpg target"] = &TriggerContext::has_rpg_target;
        creators["far from rpg target"] = &TriggerContext::far_from_rpg_target;
        creators["near rpg target"] = &TriggerContext::near_rpg_target;
        creators["no travel target"] = &TriggerContext::no_travel_target;
        creators["far from travel target"] = &TriggerContext::far_from_travel_target;
        // AC TriggerContext.h:202-205 - move to/enter dark portal if near
        creators["near dark portal"] = &TriggerContext::near_dark_portal;
        creators["at dark portal azeroth"] = &TriggerContext::at_dark_portal_azeroth;
        creators["at dark portal outland"] = &TriggerContext::at_dark_portal_outland;
        // AC TriggerContext.h:222-239
        creators["rpg"] = &TriggerContext::rpg;
        creators["rpg taxi"] = &TriggerContext::rpg_taxi;
        creators["rpg discover"] = &TriggerContext::rpg_discover;
        creators["rpg start quest"] = &TriggerContext::rpg_start_quest;
        creators["rpg end quest"] = &TriggerContext::rpg_end_quest;
        creators["rpg buy"] = &TriggerContext::rpg_buy;
        creators["rpg sell"] = &TriggerContext::rpg_sell;
        creators["rpg repair"] = &TriggerContext::rpg_repair;
        creators["rpg train"] = &TriggerContext::rpg_train;
        creators["rpg heal"] = &TriggerContext::rpg_heal;
        creators["rpg home bind"] = &TriggerContext::rpg_home_bind;
        creators["rpg queue bg"] = &TriggerContext::rpg_queue_bg;
        creators["rpg buy petition"] = &TriggerContext::rpg_buy_petition;
        creators["rpg use"] = &TriggerContext::rpg_use;
        creators["rpg spell"] = &TriggerContext::rpg_spell;
        creators["rpg craft"] = &TriggerContext::rpg_craft;
        creators["rpg trade useful"] = &TriggerContext::rpg_trade_useful;
        creators["rpg duel"] = &TriggerContext::rpg_duel;
    }

private:
    static Trigger* Timer(PlayerbotAI* botAI) { return new TimerTrigger(botAI); }
    static Trigger* Random(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "random", 20); }
    static Trigger* seldom(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "seldom", 300); }
    static Trigger* often(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "often", 5); }
    static Trigger* very_often(PlayerbotAI* botAI) { return new RandomTrigger(botAI, "often", 3); }
    static Trigger* always(PlayerbotAI* botAI) { return new AlwaysTrigger(botAI); }
    static Trigger* random_bot_update_trigger(PlayerbotAI* botAI) { return new RandomBotUpdateTrigger(botAI); }
    static Trigger* new_player_nearby(PlayerbotAI* botAI) { return new NewPlayerNearbyTrigger(botAI); }

    static Trigger* LowHealth(PlayerbotAI* botAI) { return new LowHealthTrigger(botAI); }
    static Trigger* MediumHealth(PlayerbotAI* botAI) { return new MediumHealthTrigger(botAI); }
    static Trigger* AlmostFullHealth(PlayerbotAI* botAI) { return new AlmostFullHealthTrigger(botAI); }
    static Trigger* CriticalHealth(PlayerbotAI* botAI) { return new CriticalHealthTrigger(botAI); }
    static Trigger* HealerLowMana(PlayerbotAI* botAI) { return new HealerLowManaTrigger(botAI); }
    static Trigger* LowEnergy(PlayerbotAI* botAI) { return new LowEnergyTrigger(botAI); }
    static Trigger* GenericBoost(PlayerbotAI* botAI) { return new GenericBoostTrigger(botAI); }
    static Trigger* LossOfControl(PlayerbotAI* botAI) { return new LossOfControlTrigger(botAI); }
    static Trigger* FearCharmSleep(PlayerbotAI* botAI) { return new FearCharmSleepTrigger(botAI); }
    static Trigger* PoisonDiseaseBleed(PlayerbotAI* botAI) { return new PoisonDiseaseBleedTrigger(botAI); }
    static Trigger* MovementImpaired(PlayerbotAI* botAI) { return new MovementImpairedTrigger(botAI); }
    static Trigger* TargetCriticalHealth(PlayerbotAI* botAI) { return new TargetCriticalHealthTrigger(botAI); }
    static Trigger* PartyMemberDead(PlayerbotAI* botAI) { return new PartyMemberDeadTrigger(botAI); }
    static Trigger* CombatPartyMemberDead(PlayerbotAI* botAI) { return new CombatPartyMemberDeadTrigger(botAI); }
    static Trigger* PartyMemberLowHealth(PlayerbotAI* botAI) { return new PartyMemberLowHealthTrigger(botAI); }
    static Trigger* PartyMemberMediumHealth(PlayerbotAI* botAI) { return new PartyMemberMediumHealthTrigger(botAI); }
    static Trigger* PartyMemberAlmostFullHealth(PlayerbotAI* botAI) { return new PartyMemberAlmostFullHealthTrigger(botAI); }
    static Trigger* LightRageAvailable(PlayerbotAI* botAI) { return new LightRageAvailableTrigger(botAI); }
    static Trigger* MediumRageAvailable(PlayerbotAI* botAI) { return new MediumRageAvailableTrigger(botAI); }
    static Trigger* HighRageAvailable(PlayerbotAI* botAI) { return new HighRageAvailableTrigger(botAI); }
    static Trigger* LightEnergyAvailable(PlayerbotAI* botAI) { return new LightEnergyAvailableTrigger(botAI); }
    static Trigger* MediumEnergyAvailable(PlayerbotAI* botAI) { return new MediumEnergyAvailableTrigger(botAI); }
    static Trigger* HighEnergyAvailable(PlayerbotAI* botAI) { return new HighEnergyAvailableTrigger(botAI); }
    static Trigger* AlmostFullEnergyAvailable(PlayerbotAI* botAI) { return new EnergyAvailable(botAI, 90); }
    static Trigger* generic_boost(PlayerbotAI* botAI) { return new GenericBoostTrigger(botAI); }
    static Trigger* PartyMemberCriticalHealth(PlayerbotAI* botAI) { return new PartyMemberCriticalHealthTrigger(botAI); }
    static Trigger* LowMana(PlayerbotAI* botAI) { return new LowManaTrigger(botAI); }
    static Trigger* MediumMana(PlayerbotAI* botAI) { return new MediumManaTrigger(botAI); }
    static Trigger* HighMana(PlayerbotAI* botAI) { return new HighManaTrigger(botAI); }
    static Trigger* AlmostFullMana(PlayerbotAI* botAI) { return new AlmostFullManaTrigger(botAI); }
    static Trigger* EnoughMana(PlayerbotAI* botAI) { return new EnoughManaTrigger(botAI); }

    static Trigger* TankAssist(PlayerbotAI* botAI) { return new TankAssistTrigger(botAI); }
    static Trigger* LoseAggro(PlayerbotAI* botAI) { return new LoseAggroTrigger(botAI); }
    static Trigger* HasAggro(PlayerbotAI* botAI) { return new HasAggroTrigger(botAI); }
    static Trigger* BeingAttacked(PlayerbotAI* botAI) { return new BeingAttackedTrigger(botAI); }
    static Trigger* MediumThreat(PlayerbotAI* botAI) { return new MediumThreatTrigger(botAI); }
    static Trigger* low_tank_threat(PlayerbotAI* botAI) { return new LowTankThreatTrigger(botAI); }

    static Trigger* _return(PlayerbotAI* botAI) { return new ReturnTrigger(botAI); }
    static Trigger* return_to_stay_position(PlayerbotAI* botAI) { return new ReturnToStayPositionTrigger(botAI); }
    static Trigger* Dead(PlayerbotAI* botAI) { return new DeadTrigger(botAI); }
    static Trigger* mounted(PlayerbotAI* botAI) { return new IsMountedTrigger(botAI); }
    static Trigger* corpse_near(PlayerbotAI* botAI) { return new CorpseNearTrigger(botAI); }
    static Trigger* loot_available(PlayerbotAI* botAI) { return new LootAvailableTrigger(botAI); }
    static Trigger* far_from_loot_target(PlayerbotAI* botAI) { return new FarFromCurrentLootTrigger(botAI); }
    static Trigger* can_loot(PlayerbotAI* botAI) { return new CanLootTrigger(botAI); }
    static Trigger* can_fish(PlayerbotAI* botAI) { return new CanFishTrigger(botAI); }
    static Trigger* can_use_fishing_bobber(PlayerbotAI* botAI) { return new CanUseFishingBobberTrigger(botAI); }
    static Trigger* falling_far(PlayerbotAI* botAI) { return new IsFallingFarTrigger(botAI); }
    static Trigger* can_self_resurrect(PlayerbotAI* botAI) { return new SelfResurrectTrigger(botAI); }

    static Trigger* LightAoe(PlayerbotAI* botAI) { return new LightAoeTrigger(botAI); }
    static Trigger* MediumAoe(PlayerbotAI* botAI) { return new MediumAoeTrigger(botAI); }
    static Trigger* HighAoe(PlayerbotAI* botAI) { return new HighAoeTrigger(botAI); }

    static Trigger* critical_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "critical aoe heal", "critical", 2); }
    static Trigger* medium_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "medium aoe heal", "medium", 2); }
    static Trigger* low_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "low aoe heal", "low", 2); }
    static Trigger* almost_full_aoe_heal(PlayerbotAI* botAI) { return new AoeHealTrigger(botAI, "almost full aoe heal", "almost full", 2); }
    static Trigger* group_heal_setting(PlayerbotAI* ai) { return new AoeInGroupTrigger(ai, "group heal setting", "almost full"); }
    static Trigger* medium_group_heal_setting(PlayerbotAI* ai) { return new AoeInGroupTrigger(ai, "medium group heal setting", "medium"); }
    static Trigger* healer_should_attack(PlayerbotAI* botAI) { return new HealerShouldAttackTrigger(botAI); }
    static Trigger* medium_aoe_and_healer_should_attack(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "medium aoe", "healer should attack"); }

    static Trigger* combo_points_3_available(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI, 3); }
    static Trigger* combo_points_4_available(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI, 4); }
    static Trigger* combo_points_5_available(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI, 5); }
    static Trigger* combo_points_available(PlayerbotAI* botAI) { return new ComboPointsAvailableTrigger(botAI, 5); }
    static Trigger* combo_points_not_full(PlayerbotAI* botAI) { return new ComboPointsNotFullTrigger(botAI); }
    static Trigger* combo_points_not_full_and_high_energy(PlayerbotAI* botAI) { return new TwoTriggers(botAI, "combo points not full", "high energy available"); }
    static Trigger* target_with_combo_points_almost_dead(PlayerbotAI* ai) { return new TargetWithComboPointsLowerHealTrigger(ai, 3, 3.0f); }

    static Trigger* combat_stuck(PlayerbotAI* botAI) { return new CombatStuckTrigger(botAI); }

    static Trigger* sit(PlayerbotAI* botAI) { return new SitTrigger(botAI); }
    static Trigger* swimming(PlayerbotAI* botAI) { return new IsSwimmingTrigger(botAI); }
    static Trigger* has_nearest_adds(PlayerbotAI* botAI) { return new HasNearestAddsTrigger(botAI); }

    static Trigger* panic(PlayerbotAI* botAI) { return new PanicTrigger(botAI); }
    static Trigger* outnumbered(PlayerbotAI* botAI) { return new OutNumberedTrigger(botAI); }

    static Trigger* behind_target(PlayerbotAI* botAI) { return new IsBehindTargetTrigger(botAI); }
    static Trigger* not_behind_target(PlayerbotAI* botAI) { return new IsNotBehindTargetTrigger(botAI); }
    static Trigger* not_facing_target(PlayerbotAI* botAI) { return new IsNotFacingTargetTrigger(botAI); }
    static Trigger* EnemyOutOfMelee(PlayerbotAI* botAI) { return new EnemyOutOfMeleeTrigger(botAI); }
    static Trigger* EnemyOutOfSpell(PlayerbotAI* botAI) { return new EnemyOutOfSpellRangeTrigger(botAI); }
    static Trigger* enemy_too_close_for_spell(PlayerbotAI* botAI) { return new EnemyTooCloseForSpellTrigger(botAI); }
    static Trigger* enemy_too_close_for_auto_shot(PlayerbotAI* botAI) { return new EnemyTooCloseForAutoShotTrigger(botAI); }
    static Trigger* enemy_too_close_for_shoot(PlayerbotAI* botAI) { return new EnemyTooCloseForShootTrigger(botAI); }
    static Trigger* enemy_too_close_for_melee(PlayerbotAI* botAI) { return new EnemyTooCloseForMeleeTrigger(botAI); }
    static Trigger* enemy_is_close(PlayerbotAI* botAI) { return new EnemyIsCloseTrigger(botAI); }
    static Trigger* enemy_within_melee(PlayerbotAI* botAI) { return new EnemyWithinMeleeTrigger(botAI); }
    static Trigger* not_dps_target_active(PlayerbotAI* botAI) { return new NotDpsTargetActiveTrigger(botAI); }
    static Trigger* not_dps_aoe_target_active(PlayerbotAI* botAI) { return new NotDpsAoeTargetActiveTrigger(botAI); }
    static Trigger* enemy_player_near(PlayerbotAI* botAI) { return new EnemyPlayerNear(botAI); }

    static Trigger* party_member_to_heal_out_of_spell_range(PlayerbotAI* botAI) { return new PartyMemberToHealOutOfSpellRangeTrigger(botAI); }
    static Trigger* target_changed(PlayerbotAI* botAI) { return new TargetChangedTrigger(botAI); }
    static Trigger* NoTarget(PlayerbotAI* botAI) { return new NoTargetTrigger(botAI); }
    static Trigger* invalid_target(PlayerbotAI* botAI) { return new InvalidTargetTrigger(botAI); }

    static Trigger* NoAttackers(PlayerbotAI* botAI) { return new NoAttackersTrigger(botAI); }
    static Trigger* DuelOpponentUntargeted(PlayerbotAI* botAI) { return new DuelOpponentUntargetedTrigger(botAI); }
    static Trigger* has_attackers(PlayerbotAI* botAI) { return new HasAttackersTrigger(botAI); }
    static Trigger* no_possible_targets(PlayerbotAI* botAI) { return new NoPossibleTargetsTrigger(botAI); }
    static Trigger* possible_adds(PlayerbotAI* botAI) { return new PossibleAddsTrigger(botAI); }

    static Trigger* no_pet(PlayerbotAI* botAI) { return new NoPetTrigger(botAI); }
    static Trigger* new_pet(PlayerbotAI* botAI) { return new NewPetTrigger(botAI); }
    static Trigger* no_rti_target(PlayerbotAI* botAI) { return new NoRtiTrigger(botAI); }
    static Trigger* no_food(PlayerbotAI* botAI) { return new NoFoodTrigger(botAI); }
    static Trigger* no_drink(PlayerbotAI* botAI) { return new NoDrinkTrigger(botAI); }
    static Trigger* has_pet(PlayerbotAI* botAI) { return new HasPetTrigger(botAI); }
    static Trigger* pet_attack(PlayerbotAI* botAI) { return new PetAttackTrigger(botAI); }

    static Trigger* protect_party_member(PlayerbotAI* botAI) { return new ProtectPartyMemberTrigger(botAI); }

    // new rpg (runtime roaming) status triggers
    static Trigger* go_grind_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_GO_GRIND); }
    static Trigger* go_camp_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_GO_CAMP); }
    static Trigger* wander_random_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_WANDER_RANDOM); }
    static Trigger* wander_npc_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_WANDER_NPC); }
    static Trigger* do_quest_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_DO_QUEST); }
    static Trigger* bg_waiting(PlayerbotAI* botAI) { return new BgWaitingTrigger(botAI); }
    static Trigger* bg_active(PlayerbotAI* botAI) { return new BgActiveTrigger(botAI); }
    static Trigger* bg_invite_active(PlayerbotAI* botAI) { return new BgInviteActiveTrigger(botAI); }
    static Trigger* bg_queue_dead(PlayerbotAI* botAI) { return new BgQueueDeadTrigger(botAI); }
    static Trigger* player_has_flag(PlayerbotAI* botAI) { return new PlayerHasFlag(botAI); }
    static Trigger* enemy_caster_in_range(PlayerbotAI* botAI) { return new EnemyCasterInRangeTrigger(botAI); }
    static Trigger* bot_is_cc(PlayerbotAI* botAI) { return new BotIsCcTrigger(botAI); }
    static Trigger* enemy_player_in_melee_range(PlayerbotAI* botAI) { return new EnemyPlayerInMeleeRangeTrigger(botAI); }
    static Trigger* enemy_flagcarrier_near(PlayerbotAI* botAI) { return new EnemyFlagCarrierNear(botAI); }
    static Trigger* team_flagcarrier_near(PlayerbotAI* botAI) { return new TeamFlagCarrierNear(botAI); }
    static Trigger* timer_bg(PlayerbotAI* botAI) { return new TimerBGTrigger(botAI); }
    static Trigger* alliance_no_snowfall_gy(PlayerbotAI* botAI) { return new AllianceNoSnowfallGY(botAI); }
    static Trigger* in_vehicle(PlayerbotAI* botAI) { return new InVehicleTrigger(botAI); }
    static Trigger* vehicle_near(PlayerbotAI* botAI) { return new VehicleNearTrigger(botAI); }
    static Trigger* travel_flight_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_TRAVEL_FLIGHT); }
    static Trigger* outdoor_pvp_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_OUTDOOR_PVP); }
    static Trigger* duel_spot_status(PlayerbotAI* botAI) { return new NewRpgStatusTrigger(botAI, RPG_DUEL_SPOT); }  // #201 (#612): AC :461 pattern

    static Trigger* lfg_proposal_active(PlayerbotAI* botAI) { return new LfgProposalActiveTrigger(botAI); }
    static Trigger* unknown_dungeon(PlayerbotAI* botAI) { return new UnknownDungeonTrigger(botAI); }

    // #408 (AC TriggerContext.h)
    static Trigger* pull_start(PlayerbotAI* botAI) { return new PullStartTrigger(botAI); }
    static Trigger* pull_end(PlayerbotAI* botAI) { return new PullEndTrigger(botAI); }
    static Trigger* return_to_pull_position(PlayerbotAI* botAI) { return new ReturnToPullPositionTrigger(botAI); }

    // #269/#272/#201: AC TriggerContext.h:261-266, :421-425, :437-454.
    static Trigger* no_rpg_target(PlayerbotAI* botAI) { return new NoRpgTargetTrigger(botAI); }
    static Trigger* has_rpg_target(PlayerbotAI* botAI) { return new HasRpgTargetTrigger(botAI); }
    static Trigger* collision(PlayerbotAI* botAI) { return new CollisionTrigger(botAI); }  // #271 AC :268
    static Trigger* far_from_rpg_target(PlayerbotAI* botAI) { return new FarFromRpgTargetTrigger(botAI); }
    static Trigger* near_rpg_target(PlayerbotAI* botAI) { return new NearRpgTargetTrigger(botAI); }
    static Trigger* no_travel_target(PlayerbotAI* botAI) { return new NoTravelTargetTrigger(botAI); }
    static Trigger* far_from_travel_target(PlayerbotAI* botAI) { return new FarFromTravelTargetTrigger(botAI); }
    static Trigger* near_dark_portal(PlayerbotAI* botAI) { return new NearDarkPortalTrigger(botAI); }
    static Trigger* at_dark_portal_azeroth(PlayerbotAI* botAI) { return new AtDarkPortalAzerothTrigger(botAI); }
    static Trigger* at_dark_portal_outland(PlayerbotAI* botAI) { return new AtDarkPortalOutlandTrigger(botAI); }
    static Trigger* rpg(PlayerbotAI* botAI) { return new RpgTrigger(botAI); }
    static Trigger* rpg_taxi(PlayerbotAI* botAI) { return new RpgTaxiTrigger(botAI); }
    static Trigger* rpg_discover(PlayerbotAI* botAI) { return new RpgDiscoverTrigger(botAI); }
    static Trigger* rpg_start_quest(PlayerbotAI* botAI) { return new RpgStartQuestTrigger(botAI); }
    static Trigger* rpg_end_quest(PlayerbotAI* botAI) { return new RpgEndQuestTrigger(botAI); }
    static Trigger* rpg_buy(PlayerbotAI* botAI) { return new RpgBuyTrigger(botAI); }
    static Trigger* rpg_sell(PlayerbotAI* botAI) { return new RpgSellTrigger(botAI); }
    static Trigger* rpg_repair(PlayerbotAI* botAI) { return new RpgRepairTrigger(botAI); }
    static Trigger* rpg_train(PlayerbotAI* botAI) { return new RpgTrainTrigger(botAI); }
    static Trigger* rpg_heal(PlayerbotAI* botAI) { return new RpgHealTrigger(botAI); }
    static Trigger* rpg_home_bind(PlayerbotAI* botAI) { return new RpgHomeBindTrigger(botAI); }
    static Trigger* rpg_queue_bg(PlayerbotAI* botAI) { return new RpgQueueBGTrigger(botAI); }
    static Trigger* rpg_buy_petition(PlayerbotAI* botAI) { return new RpgBuyPetitionTrigger(botAI); }
    static Trigger* rpg_use(PlayerbotAI* botAI) { return new RpgUseTrigger(botAI); }
    // AC TriggerContext.h:451: AC creates RpgUseTrigger here, not RpgSpellTrigger - carried verbatim.
    static Trigger* rpg_spell(PlayerbotAI* botAI) { return new RpgUseTrigger(botAI); }
    static Trigger* rpg_craft(PlayerbotAI* botAI) { return new RpgCraftTrigger(botAI); }
    static Trigger* rpg_trade_useful(PlayerbotAI* botAI) { return new RpgTradeUsefulTrigger(botAI); }
    static Trigger* rpg_duel(PlayerbotAI* botAI) { return new RpgDuelTrigger(botAI); }
    static Trigger* force_rebuff_pending(PlayerbotAI* botAI) { return new ForceRebuffPendingTrigger(botAI); }  // AC :341
};

#endif
