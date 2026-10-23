#ifndef _PLAYERBOT_VALUECONTEXT_H
#define _PLAYERBOT_VALUECONTEXT_H

#include "ActiveSpellValue.h"
#include "AoeHealValues.h"
#include "AoeValues.h"
#include "AttackersValue.h"
#include "AttackerCountValues.h"
#include "AttackerWithoutAuraTargetValue.h"
#include "BudgetValues.h"
#include "CcTargetValue.h"
#include "CollisionValue.h"
#include "CurrentCcTargetValue.h"
#include "CurrentTargetValue.h"
#include "DistanceValue.h"
#include "DpsTargetValue.h"
#include "DuelTargetValue.h"
#include "EnemyHealerTargetValue.h"
#include "PvpRotationValues.h"  // #855 (epic #854): rotation-lane shared values
#include "EnemyPlayerValue.h"
#include "EstimatedLifetimeValue.h"
#include "Formations.h"
#include "GroupValues.h"
#include "GrindTargetValue.h"
#include "HasTotemValue.h"
#include "InvalidTargetValue.h"
#include "IsBehindValue.h"
#include "IsMovingValue.h"
#include "ItemCountValue.h"
#include "ItemForSpellValue.h"
#include "FishValues.h"
#include "ItemUsageValue.h"
#include "AlwaysLootListValue.h"
#include "LastMovementValue.h"
#include "LastSpellCastValue.h"
#include "LastSpellCastTimeValue.h"
#include "LfgValues.h"
#include "LootStrategyValue.h"
#include "LootValues.h"
#include "MaintenanceValues.h"
#include "ManaSaveLevelValue.h"
#include "MasterTargetValue.h"
#include "FlagCarrierValue.h"
#include "NearestAddsValue.h"
#include "NearestCorpsesValue.h"
#include "NearestGameObjects.h"
#include "NearestNpcsValue.h"
#include "NearestUnitsValue.h"
#include "NearestFriendlyPlayersValue.h"
#include "NewPlayerNearbyValue.h"
#include "PartyMemberWithoutAuraValue.h"
#include "PartyMemberWithoutItemValue.h"
#include "PartyMemberToDispelValue.h"
#include "PartyMemberSnaredTargetValue.h"
#include "PartyMemberToHealValue.h"
#include "TargetValue.h"
#include "PartyMemberToResurectValue.h"
#include "PetTargetValue.h"
#include "PositionValue.h"
#include "RTSCValues.h"  // #648
#include "QuestValues.h"
#include "SkipSpellsListValue.h"
#include "SnareTargetValue.h"
#include "StatsValues.h"
#include "SelfTargetValue.h"
#include "SpellCastUsefulValue.h"
#include "SpellIdValue.h"
#include "TankTargetValue.h"
#include "ThreatValues.h"
#include "TradeValues.h"
#include "PossibleRpgTargetsValue.h"
#include "PvpValues.h"   // #272: BgTypeValue / RpgBgTypeValue (AC Ai/Base/Value/PvpValues.h)
#include "RpgValues.h"   // #272: NextRpgActionValue (AC Ai/World/Rpg/RpgValues.h)
#include "PossibleTargetsValue.h"
#include "RandomBotUpdateValue.h"
#include "RangeValues.h"
#include "RtiValue.h"
#include "RtiTargetValue.h"
#include "WaitForAttackTimeValue.h"
#include "GuildValues.h"  // #611 (AC ValueContext.h:303,560)

class PlayerbotAI;
class ValueContext : public NamedObjectContext<UntypedValue>
{
public:
    ValueContext()
    {
        creators["health"] = &ValueContext::health;
        creators["rage"] = &ValueContext::rage;
        creators["energy"] = &ValueContext::energy;
        creators["chi"] = &ValueContext::chi;  // #194 (S8): Mistweaver Uplift-on-party Chi gate
        creators["mana"] = &ValueContext::mana;
        creators["mana save level"] = &ValueContext::mana_save_level;
        creators["combo"] = &ValueContext::combo;
        creators["dead"] = &ValueContext::dead;
        creators["pet dead"] = &ValueContext::pet_dead;
        creators["pet happy"] = &ValueContext::pet_happy;
        creators["has mana"] = &ValueContext::has_mana;
        creators["mounted"] = &ValueContext::mounted;
        creators["last movement"] = &ValueContext::last_movement;
        creators["stay time"] = &ValueContext::stay_time;
        creators["last taxi"] = &ValueContext::last_movement;
        creators["last area trigger"] = &ValueContext::last_movement;
        creators["combat"] = &ValueContext::combat;
        // AC parity (#141 item 5): WaitForAttackStrategy::ShouldWait() bookkeeping.
        creators["wait for attack time"] = &ValueContext::wait_for_attack_time;
        creators["combat start time"] = &ValueContext::combat_start_time;
        creators["bag space"] = &ValueContext::bag_space;
        creators["durability"] = &ValueContext::durability;
        creators["speed"] = &ValueContext::speed;
        creators["death count"] = &ValueContext::death_count;
        creators["experience"] = &ValueContext::experience;
        creators["disperse distance"] = &ValueContext::disperse_distance;
        creators["random bot update"] = &ValueContext::random_bot_update_value;
        creators["has totem"] = &ValueContext::has_totem;

        creators["distance"] = &ValueContext::distance;
        creators["moving"] = &ValueContext::moving;
        creators["swimming"] = &ValueContext::swimming;
        creators["behind"] = &ValueContext::behind;
        creators["facing"] = &ValueContext::facing;
        creators["balance"] = &ValueContext::balance;
        creators["attackers"] = &ValueContext::attackers;
        creators["attacker count"] = &ValueContext::attacker_count;
        creators["my attacker count"] = &ValueContext::my_attacker_count;
        creators["possible adds"] = &ValueContext::possible_adds;

        creators["rti target"] = &ValueContext::rti_target;
        creators["rti cc target"] = &ValueContext::rti_cc_target;
        creators["rti cc"] = &ValueContext::rti_cc;
        creators["rti"] = &ValueContext::rti;

        creators["prioritized targets"] = &ValueContext::prioritized_targets;
        creators["possible triggers"] = &ValueContext::possible_triggers;
        creators["possible targets"] = &ValueContext::possible_targets;
        // #272 (AC ValueContext.h:128): the old-rpg target scan. Dormant as in AC - only the old "rpg"
        // strategy's ChooseRpgTargetAction reads it.
        creators["possible rpg targets"] = &ValueContext::possible_rpg_targets;
        creators["possible new rpg targets"] = &ValueContext::possible_new_rpg_targets;
        creators["possible new rpg game objects"] = &ValueContext::possible_new_rpg_game_objects;
        creators["possible targets no los"] = &ValueContext::possible_targets_no_los;

        creators["position"] = &ValueContext::position;
        creators["current position"] = &ValueContext::current_position;
        creators["can move around"] = &ValueContext::can_move_around;

        creators["master target"] = &ValueContext::master;
        creators["all targets"] = &ValueContext::all_targets;
        creators["self target"] = &ValueContext::self_target;
        creators["pet target"] = &ValueContext::pet_target;
        creators["current target"] = &ValueContext::current_target;
        creators["old target"] = &ValueContext::old_target;
        creators["grind target"] = &ValueContext::grind_target;
        creators["pull target"] = &ValueContext::pull_target;
        // #408 (AC ValueContext.h): PullStrategy stores its in-flight pull target here.
        creators["pull strategy target"] = &ValueContext::pull_strategy_target;
        creators["talk target"] = &ValueContext::talk_target;  // #194 (S8): TalkAction/GreetAction
        creators["invalid target"] = &ValueContext::invalid_target;
        creators["nearest npcs"] = &ValueContext::nearest_npcs;
        // #545 (AC Ai/Base/ValueContext.h:115,319): used by the Phase 6 dungeon strategies
        creators["nearest hostile npcs"] = &ValueContext::nearest_hostile_npcs;
        creators["find target"] = &ValueContext::find_target;
        // Read by EnterVehicleAction::Execute (VehicleActions.cpp) and, as in AC, by PvpTriggers /
        // TOCActions / ICCActions_GSB. AI_VALUE has no null guard (Playerbots.h:26), so this
        // registration must exist before any "nearest vehicles" call site does.
        creators["nearest vehicles"] = &ValueContext::nearest_vehicles;
        creators["nearest friendly players"] = &ValueContext::nearest_friendly_players;
        creators["closest friendly players"] = &ValueContext::closest_friendly_players;
        creators["nearest enemy players"] = &ValueContext::nearest_enemy_players;
        creators["enemy player target"] = &ValueContext::enemy_player_target;
        // Referenced by ChooseTargetActions and the battleground triggers; without these two the
        // AI_VALUE macro dereferences a null pointer (see FlagCarrierValue.h).
        creators["enemy flag carrier"] = &ValueContext::enemy_flag_carrier;
        creators["team flag carrier"] = &ValueContext::team_flag_carrier;
        // Used by the battleground tactics; unregistered these are null dereferences, not misses.
        creators["bg role"] = &ValueContext::bg_role;
        creators["closest game objects"] = &ValueContext::closest_game_objects;
        creators["nearest game objects no los"] = &ValueContext::nearest_game_objects_no_los;
        creators["dps target"] = &ValueContext::dps_target;
        creators["dps aoe target"] = &ValueContext::dps_aoe_target;

        creators["can move around"] = &ValueContext::can_move_around;
        creators["should home bind"] = &ValueContext::should_home_bind;
        creators["should repair"] = &ValueContext::should_repair;
        creators["can repair"] = &ValueContext::can_repair;
        creators["should sell"] = &ValueContext::should_sell;
        creators["can sell"] = &ValueContext::can_sell;
        creators["can fight equal"] = &ValueContext::can_fight_equal;
        creators["can fight elite"] = &ValueContext::can_fight_elite;
        creators["can fight boss"] = &ValueContext::can_fight_boss;

        creators["formation"] = &ValueContext::formation;
        // #648 (AC ValueContext.h:311-314): RTSC / see spell values
        creators["see spell location"] = &ValueContext::see_spell_location;
        creators["RTSC selected"] = &ValueContext::RTSC_selected;
        creators["RTSC next spell action"] = &ValueContext::RTSC_next_spell_action;
        creators["RTSC saved location"] = &ValueContext::RTSC_saved_location;
        creators["range"] = &ValueContext::range;
        creators["group"] = &ValueContext::group;
        creators["group members"] = &ValueContext::group_members;
        creators["following party"] = &ValueContext::following_party;
        creators["near leader"] = &ValueContext::near_leader;
        creators["and"] = &ValueContext::and_value;
        creators["group count"] = &ValueContext::group_count;
        creators["group and"] = &ValueContext::group_and;
        creators["group or"] = &ValueContext::group_or;
        creators["group ready"] = &ValueContext::group_ready;

        creators["active spell"] = &ValueContext::active_spell;
        creators["spell id"] = &ValueContext::spell_id;
        creators["vehicle spell id"] = &ValueContext::vehicle_spell_id;
        // #149: was commented out, but HasItemForSpellTrigger/CastItemSpellAction/
        // SpellCastUsefulValue/PlayerbotAI.cpp all call AI_VALUE2(Item*, "item for spell", ...)
        // live; see ItemForSpellValue.h for why this is a neutral stub.
        creators["item for spell"] = &ValueContext::item_for_spell;
        // #412 (AC ValueContext.h:332-334): fishing.
        creators["can fish"] = &ValueContext::can_fish;
        creators["can use fishing bobber"] = &ValueContext::can_use_fishing_bobber;
        creators["fishing spot"] = &ValueContext::fishing_spot;
        creators["spell cast useful"] = &ValueContext::spell_cast_useful;
        creators["last spell cast"] = &ValueContext::last_spell_cast;
        creators["last spell cast time"] = &ValueContext::last_spell_cast_time;

        creators["estimated lifetime"] = &ValueContext::expected_lifetime;
        creators["estimated group dps"] = &ValueContext::expected_group_dps;

        creators["attacker without aura"] = &ValueContext::attacker_without_aura;
        creators["melee attacker without aura"] = &ValueContext::melee_attacker_without_aura;
        creators["party member without aura"] = &ValueContext::party_member_without_aura;
        creators["party member to heal"] = &ValueContext::party_member_to_heal;
        // #163 S8 (AC parity): lowest-mana healer, for HealerLowManaTrigger.
        creators["healer low mana"] = &ValueContext::healer_low_mana;
        // #163 S8 (AC parity): the snared party member Hand of Freedom (and any future cure-snare
        // action) should target.
        creators["party member snared target"] = &ValueContext::party_member_snared_target;
        // #163 S8 (AC parity): the manual-set list "focus heal targets" (chat command) writes to.
        creators["focus heal targets"] = &ValueContext::focus_heal_targets;
        creators["party member to resurrect"] = &ValueContext::party_member_to_resurrect;
        creators["party member to dispel"] = &ValueContext::party_member_to_dispel;
        creators["party member to protect"] = &ValueContext::party_member_to_protect;

        creators["main tank"] = &ValueContext::main_tank;
        creators["threat"] = &ValueContext::threat;
        creators["has aggro"] = &ValueContext::has_aggro;
        creators["tank target"] = &ValueContext::tank_target;

        creators["last flee angle"] = &ValueContext::last_flee_angle;
        creators["last flee timestamp"] = &ValueContext::last_flee_timestamp;
        creators["recently flee info"] = &ValueContext::recently_flee_info;

        // #149 (AC ValueContext.h, ItemCountValue.h): backs UseItemAction/EatAction/DrinkAction/
        // UseHealingPotion/UseManaPotion's item lookups. FindItemUsageVisitor::Accept() in
        // ItemVisitors.cpp already calls AI_VALUE2(ItemUsage, "item usage", ...), a *different*,
        // still-unregistered value this port does not add (no "usage <n>" qualifier is used by
        // food/drink/potions), so that call path stays dormant/unreachable as before.
        creators["inventory items"] = &ValueContext::inventory_items;
        creators["item count"] = &ValueContext::item_count;
        creators["petition signs"] = &ValueContext::petition_signs;  // #611 (AC ValueContext.h:303)

        // #149: 27 AI value names read by already-registered triggers/actions (has nearest adds,
        // aoe heal, snare, enemy healer target, cc, etc.) with no NamedObjectContext<UntypedValue>
        // creator, so AiObjectContext::GetValue<T> returned nullptr and AI_VALUE/GetTargetValue()
        // dereferenced it. See each header for whether the class is a full/adapted port or a
        // neutral stub for a subsystem this fork never ported (loot, travel, item usage, ...).
        creators["cc target"] = &ValueContext::cc_target;
        creators["current cc target"] = &ValueContext::current_cc_target;
        creators["snare target"] = &ValueContext::snare_target;
        creators["enemy healer target"] = &ValueContext::enemy_healer_target;
        // #855 (epic #854, rotation ports): shared values every class lane uses (PvpRotationValues.h).
        creators["enemy caster target"] = &ValueContext::enemy_caster_target;
        creators["is cc"] = &ValueContext::is_cc;
        creators["carrying objective"] = &ValueContext::carrying_objective;
        creators["in pvp instance"] = &ValueContext::in_pvp_instance;
        creators["duel target"] = &ValueContext::duel_target;
        creators["inside target"] = &ValueContext::inside_target;
        creators["nearest adds"] = &ValueContext::nearest_adds;
        creators["collision"] = &ValueContext::collision;
        creators["neglect threat"] = &ValueContext::neglect_threat;
        creators["new player nearby"] = &ValueContext::new_player_nearby;
        // Dependency of "new player nearby" (NewPlayerNearbyValue::Calculate()), not itself one of
        // the 27 names; see NewPlayerNearbyValue.h.
        creators["already seen players"] = &ValueContext::already_seen_players;
        creators["aoe heal"] = &ValueContext::aoe_heal;
        creators["aoe position"] = &ValueContext::aoe_position;
        // Dependency of "cc target" (FindTargetForCcStrategy::CheckAttacker()), not itself one of
        // the 27 names; see AoeValues.h.
        creators["aoe count"] = &ValueContext::aoe_count;
        creators["has area debuff"] = &ValueContext::has_area_debuff;
        creators["party member without item"] = &ValueContext::party_member_without_item;
        creators["party member without food"] = &ValueContext::party_member_without_food;
        creators["party member without water"] = &ValueContext::party_member_without_water;
        // #332 (AC ValueContext.h:222-224, 277-280, 288): AC's gold-budget value set. "repair cost"
        // and "free money for" were already registered here but resolved to hard-coded 0 stubs;
        // MaxGearRepairCostValue / TrainCostValue / MoneyNeededForValue / TotalMoneyNeededForValue /
        // ShouldGetMoneyValue did not exist at all, and CanTrainValue did not either.
        creators["max repair cost"] = &ValueContext::max_repair_cost;
        creators["repair cost"] = &ValueContext::repair_cost;
        creators["train cost"] = &ValueContext::train_cost;
        creators["money needed for"] = &ValueContext::money_needed_for;
        creators["total money needed for"] = &ValueContext::total_money_needed_for;
        creators["free money for"] = &ValueContext::free_money_for;
        creators["should get money"] = &ValueContext::should_get_money;
        creators["can train"] = &ValueContext::can_train;
        creators["rpg target"] = &ValueContext::rpg_target;
        creators["ignore rpg target"] = &ValueContext::ignore_rpg_target;
        // #272 (AC ValueContext.h:245): set/read only by the old-rpg actions/triggers/multiplier.
        creators["next rpg action"] = &ValueContext::next_rpg_action;
        creators["travel target"] = &ValueContext::travel_target;
        // #272 (AC ValueContext.h:259-260, 267): read only by the old-rpg queue-bg / home-bind
        // trigger+action pairs, DistanceValue's "home bind" qualifier and ShouldHomeBindValue (no live
        // reader on either core).
        creators["bg type"] = &ValueContext::bg_type;
        creators["rpg bg type"] = &ValueContext::rpg_bg_type;
        creators["home bind"] = &ValueContext::home_bind;
        creators["last long move"] = &ValueContext::last_long_move;
        creators["lfg proposal"] = &ValueContext::lfg_proposal;
        creators["loot target"] = &ValueContext::loot_target;
        creators["available loot"] = &ValueContext::available_loot;
        // #151 (AC ValueContext.h:111,132,176-181): the rest of the loot-stack value set.
        creators["can loot"] = &ValueContext::can_loot;
        creators["has available loot"] = &ValueContext::has_available_loot;
        creators["loot strategy"] = &ValueContext::loot_strategy;
        creators["always loot list"] = &ValueContext::always_loot_list;
        creators["nearest corpses"] = &ValueContext::nearest_corpses;
        creators["nearest game objects"] = &ValueContext::nearest_game_objects;
        creators["item usage"] = &ValueContext::item_usage;
        // #188 (S6 of epic #163): ItemUpgradeValue reuses ItemUsageValue's Calculate() helpers but
        // is queried separately by EquipAction::SelectInventoryItemsToEquip() (skips the quest/skill/
        // vendor checks and only asks "would equipping this be an upgrade").
        creators["item upgrade"] = &ValueContext::item_upgrade;
        // #193 (S7 of epic #163, AC ValueContext.h): the trade-giveaway item list TradeAction/
        // the (not-yet-ported) RPG trade-useful action would consume.
        // #272: AC ValueContext.h:309 registers this under "items useful to give" (read by AC
        // RpgTradeUsefulTrigger); the fork had used the class's default name "useful to give", which
        // nothing reads. Key aligned to AC.
        creators["items useful to give"] = &ValueContext::items_useful_to_give;
        creators["skip spells list"] = &ValueContext::skip_spells_list;

        // #353 (AC Ai/Base/Value/LootValues.h:70-76, registered at AC ValueContext.h:308): the
        // "has upgrade" predicate that Travel/TravelMgr.cpp BossTravelDestination::isActive() asks
        // for. Chain: "has upgrade" -> "entry loot usage" (here) -> "entry loot list" (here) ->
        // DropMapValue::GetLootTemplate() (LootValues.cpp) + AI_VALUE2(ItemUsage, "item usage", ..)
        // (ItemUsageValue.h, already registered below this block).
        creators["entry loot usage"] = &ValueContext::entry_loot_usage;
        creators["has upgrade"] = &ValueContext::has_upgrade;
        // AC registers "entry loot list" in SharedValueContext.h:60 because it is a global
        // per-entry loot-template lookup. On this fork it stays here so the whole "has upgrade"
        // chain lands atomically in one file; EntryLootListValue is a SingleCalculatedValue, so
        // Calculate() still runs once per (bot, entry) and costs one LootStore::GetLootFor() map
        // lookup. "drop map" (DropMapValue) is deliberately NOT registered: its only consumer,
        // AC's ItemDropListValue, is not ported (see the strategy/value/QuestValues.cpp header).
        creators["entry loot list"] = &ValueContext::entry_loot_list;
        // #257 slice H registration (AC ValueContext.h:270-275): the per-bot quest values. The
        // three global ones ("entry quest relation", "quest guidp map", "quest givers") live in
        // SharedValueContext, not here, exactly as in AC.
        creators["free quest log slots"] = &ValueContext::free_quest_log_slots;
        creators["dialog status"] = &ValueContext::dialog_status;
        creators["dialog status quest"] = &ValueContext::dialog_status_quest;
        creators["can accept quest npc"] = &ValueContext::can_accept_quest_npc;
        creators["can accept quest low level npc"] = &ValueContext::can_accept_quest_low_level_npc;
        creators["can turn in quest npc"] = &ValueContext::can_turn_in_quest_npc;
    }

private:
    static UntypedValue* free_quest_log_slots(PlayerbotAI* botAI) { return new FreeQuestLogSlotValue(botAI); }
    static UntypedValue* dialog_status(PlayerbotAI* botAI) { return new DialogStatusValue(botAI); }
    static UntypedValue* dialog_status_quest(PlayerbotAI* botAI) { return new DialogStatusQuestValue(botAI); }
    static UntypedValue* can_accept_quest_npc(PlayerbotAI* botAI) { return new CanAcceptQuestValue(botAI); }
    static UntypedValue* can_accept_quest_low_level_npc(PlayerbotAI* botAI)
    {
        return new CanAcceptQuestLowLevelValue(botAI);
    }
    static UntypedValue* can_turn_in_quest_npc(PlayerbotAI* botAI) { return new CanTurnInQuestValue(botAI); }

    static UntypedValue* entry_loot_usage(PlayerbotAI* botAI) { return new EntryLootUsageValue(botAI); }
    static UntypedValue* has_upgrade(PlayerbotAI* botAI) { return new HasUpgradeValue(botAI); }
    static UntypedValue* entry_loot_list(PlayerbotAI* botAI) { return new EntryLootListValue(botAI); }
    static UntypedValue* last_movement(PlayerbotAI* botAI) { return new LastMovementValue(botAI); }
    static UntypedValue* stay_time(PlayerbotAI* botAI) { return new StayTimeValue(botAI); }
    static UntypedValue* mounted(PlayerbotAI* botAI) { return new IsMountedValue(botAI); }
    static UntypedValue* health(PlayerbotAI* botAI) { return new HealthValue(botAI); }
    static UntypedValue* rage(PlayerbotAI* botAI) { return new RageValue(botAI); }
    static UntypedValue* energy(PlayerbotAI* botAI) { return new EnergyValue(botAI); }
    static UntypedValue* chi(PlayerbotAI* botAI) { return new ChiValue(botAI); }
    static UntypedValue* mana(PlayerbotAI* botAI) { return new ManaValue(botAI); }
    static UntypedValue* mana_save_level(PlayerbotAI* botAI) { return new ManaSaveLevelValue(botAI); }
    static UntypedValue* combo(PlayerbotAI* botAI) { return new ComboPointsValue(botAI); }
    static UntypedValue* dead(PlayerbotAI* botAI) { return new IsDeadValue(botAI); }
    static UntypedValue* pet_dead(PlayerbotAI* botAI) { return new PetIsDeadValue(botAI); }
    static UntypedValue* pet_happy(PlayerbotAI* botAI) { return new PetIsHappyValue(botAI); }
    static UntypedValue* has_mana(PlayerbotAI* botAI) { return new HasManaValue(botAI); }   
    static UntypedValue* combat(PlayerbotAI* botAI) { return new IsInCombatValue(botAI); }
    static UntypedValue* wait_for_attack_time(PlayerbotAI* botAI) { return new WaitForAttackTimeValue(botAI); }
    static UntypedValue* combat_start_time(PlayerbotAI* botAI) { return new CombatStartTimeValue(botAI); }
    static UntypedValue* bag_space(PlayerbotAI* botAI) { return new BagSpaceValue(botAI); }
    static UntypedValue* durability(PlayerbotAI* botAI) { return new DurabilityValue(botAI); }
    static UntypedValue* speed(PlayerbotAI* botAI) { return new SpeedValue(botAI); }
    static UntypedValue* death_count(PlayerbotAI* botAI) { return new DeathCountValue(botAI); }
    static UntypedValue* experience(PlayerbotAI* botAI) { return new ExperienceValue(botAI); }
    static UntypedValue* disperse_distance(PlayerbotAI* ai) { return new DisperseDistanceValue(ai); }
    static UntypedValue* random_bot_update_value(PlayerbotAI* botAI) { return new RandomBotUpdateValue(botAI); }
    static UntypedValue* has_totem(PlayerbotAI* botAI) { return new HasTotemValue(botAI); }

    static UntypedValue* position(PlayerbotAI* botAI) { return new PositionValue(botAI); }
    static UntypedValue* current_position(PlayerbotAI* botAI) { return new CurrentPositionValue(botAI); }
    static UntypedValue* behind(PlayerbotAI* botAI) { return new IsBehindValue(botAI); }
    static UntypedValue* facing(PlayerbotAI* botAI) { return new IsFacingValue(botAI); }
    static UntypedValue* moving(PlayerbotAI* botAI) { return new IsMovingValue(botAI); }
    static UntypedValue* swimming(PlayerbotAI* botAI) { return new IsSwimmingValue(botAI); }
    static UntypedValue* distance(PlayerbotAI* botAI) { return new DistanceValue(botAI); }
    static UntypedValue* rti_target(PlayerbotAI* botAI) { return new RtiTargetValue(botAI); }
    static UntypedValue* rti_cc_target(PlayerbotAI* botAI) { return new RtiCcTargetValue(botAI); }
    static UntypedValue* rti(PlayerbotAI* botAI) { return new RtiValue(botAI); }
    static UntypedValue* rti_cc(PlayerbotAI* botAI) { return new RtiCcValue(botAI); }

    static UntypedValue* balance(PlayerbotAI* botAI) { return new BalancePercentValue(botAI); }
    static UntypedValue* attackers(PlayerbotAI* botAI) { return new AttackersValue(botAI); }
    static UntypedValue* attacker_count(PlayerbotAI* botAI) { return new AttackerCountValue(botAI); }
    static UntypedValue* my_attacker_count(PlayerbotAI* botAI) { return new MyAttackerCountValue(botAI); }

    static UntypedValue* possible_adds(PlayerbotAI* botAI) { return new PossibleAddsValue(botAI); }
    static UntypedValue* prioritized_targets(PlayerbotAI* botAI) { return new PrioritizedTargetsValue(botAI); }

    static UntypedValue* master(PlayerbotAI* botAI) { return new MasterTargetValue(botAI); }
    static UntypedValue* all_targets(PlayerbotAI* botAI) { return new AllTargetsValue(botAI); }
    static UntypedValue* self_target(PlayerbotAI* botAI) { return new SelfTargetValue(botAI); }
    static UntypedValue* pet_target(PlayerbotAI* botAI) { return new PetTargetValue(botAI); }
    static UntypedValue* current_target(PlayerbotAI* botAI) { return new CurrentTargetValue(botAI); }
    static UntypedValue* possible_triggers(PlayerbotAI* botAI) { return new PossibleTriggersValue(botAI); }
    static UntypedValue* possible_targets(PlayerbotAI* botAI) { return new PossibleTargetsValue(botAI); }
    static UntypedValue* possible_targets_no_los(PlayerbotAI* botAI) { return new PossibleTargetsValue(botAI, "possible targets", sPlayerbotAIConfig->sightDistance, true); }
    static UntypedValue* possible_rpg_targets(PlayerbotAI* botAI) { return new PossibleRpgTargetsValue(botAI); }
    static UntypedValue* possible_new_rpg_targets(PlayerbotAI* botAI) { return new PossibleNewRpgTargetsValue(botAI); }
    static UntypedValue* possible_new_rpg_game_objects(PlayerbotAI* botAI) { return new PossibleNewRpgGameObjectsValue(botAI); }
    static UntypedValue* old_target(PlayerbotAI* botAI) { return new CurrentTargetValue(botAI); }
    static UntypedValue* grind_target(PlayerbotAI* botAI) { return new GrindTargetValue(botAI); }
    static UntypedValue* pull_target(PlayerbotAI* botAI) { return new PullTargetValue(botAI); }
    // #408 (AC ValueContext.h)
    static UntypedValue* pull_strategy_target(PlayerbotAI* botAI) { return new PullStrategyTargetValue(botAI); }    static UntypedValue* talk_target(PlayerbotAI* botAI) { return new TalkTargetValue(botAI); }
    static UntypedValue* focus_heal_targets(PlayerbotAI* botAI) { return new FocusHealTargetValue(botAI); }
    static UntypedValue* invalid_target(PlayerbotAI* botAI) { return new InvalidTargetValue(botAI); }
    static UntypedValue* nearest_npcs(PlayerbotAI* botAI) { return new NearestNpcsValue(botAI); }
    static UntypedValue* nearest_hostile_npcs(PlayerbotAI* botAI) { return new NearestHostileNpcsValue(botAI); }  // AC ValueContext.h:426
    static UntypedValue* find_target(PlayerbotAI* botAI) { return new FindTargetValue(botAI); }  // AC ValueContext.h:576
    static UntypedValue* nearest_vehicles(PlayerbotAI* botAI) { return new NearestVehiclesValue(botAI); }
    static UntypedValue* enemy_flag_carrier(PlayerbotAI* botAI) { return new FlagCarrierValue(botAI, false, true); }
    static UntypedValue* team_flag_carrier(PlayerbotAI* botAI) { return new FlagCarrierValue(botAI, true, true); }
    static UntypedValue* bg_role(PlayerbotAI* botAI) { return new BgRoleValue(botAI); }
    static UntypedValue* closest_game_objects(PlayerbotAI* botAI)
    {
        return new NearestGameObjects(botAI, INTERACTION_DISTANCE);
    }
    static UntypedValue* nearest_game_objects_no_los(PlayerbotAI* botAI)
    {
        return new NearestGameObjects(botAI, sPlayerbotAIConfig->sightDistance, true);
    }
    static UntypedValue* nearest_friendly_players(PlayerbotAI* botAI) { return new NearestFriendlyPlayersValue(botAI); }
    static UntypedValue* closest_friendly_players(PlayerbotAI* botAI) { return new NearestFriendlyPlayersValue(botAI, INTERACTION_DISTANCE); }
    static UntypedValue* nearest_enemy_players(PlayerbotAI* botAI) { return new NearestEnemyPlayersValue(botAI); }
    static UntypedValue* enemy_player_target(PlayerbotAI* botAI) { return new EnemyPlayerValue(botAI); }
    static UntypedValue* dps_target(PlayerbotAI* botAI) { return new DpsTargetValue(botAI); }
    static UntypedValue* dps_aoe_target(PlayerbotAI* botAI) { return new DpsAoeTargetValue(botAI); }

    static UntypedValue* can_move_around(PlayerbotAI* botAI) { return new CanMoveAroundValue(botAI); }
    static UntypedValue* should_home_bind(PlayerbotAI* botAI) { return new ShouldHomeBindValue(botAI); }
    static UntypedValue* should_repair(PlayerbotAI* botAI) { return new ShouldRepairValue(botAI); }
    static UntypedValue* can_repair(PlayerbotAI* botAI) { return new CanRepairValue(botAI); }
    static UntypedValue* should_sell(PlayerbotAI* botAI) { return new ShouldSellValue(botAI); }
    static UntypedValue* can_sell(PlayerbotAI* botAI) { return new CanSellValue(botAI); }
    static UntypedValue* can_fight_equal(PlayerbotAI* botAI) { return new CanFightEqualValue(botAI); }
    static UntypedValue* can_fight_elite(PlayerbotAI* botAI) { return new CanFightEliteValue(botAI); }
    static UntypedValue* can_fight_boss(PlayerbotAI* botAI) { return new CanFightBossValue(botAI); }

    static UntypedValue* formation(PlayerbotAI* botAI) { return new FormationValue(botAI); }
    static UntypedValue* see_spell_location(PlayerbotAI* botAI) { return new SeeSpellLocationValue(botAI); }  // #648
    static UntypedValue* RTSC_selected(PlayerbotAI* botAI) { return new RTSCSelectedValue(botAI); }
    static UntypedValue* RTSC_next_spell_action(PlayerbotAI* botAI) { return new RTSCNextSpellActionValue(botAI); }
    static UntypedValue* RTSC_saved_location(PlayerbotAI* botAI) { return new RTSCSavedLocationValue(botAI); }
    static UntypedValue* range(PlayerbotAI* botAI) { return new RangeValue(botAI); }
    static UntypedValue* group_members(PlayerbotAI* botAI) { return new GroupMembersValue(botAI); }
    static UntypedValue* following_party(PlayerbotAI* botAI) { return new IsFollowingPartyValue(botAI); }
    static UntypedValue* near_leader(PlayerbotAI* botAI) { return new IsNearLeaderValue(botAI); }
    static UntypedValue* group(PlayerbotAI* botAI) { return new IsInGroupValue(botAI); }
    static UntypedValue* and_value(PlayerbotAI* botAI) { return new BoolANDValue(botAI); }
    static UntypedValue* group_count(PlayerbotAI* botAI) { return new GroupBoolCountValue(botAI); }
    static UntypedValue* group_and(PlayerbotAI* botAI) { return new GroupBoolANDValue(botAI); }
    static UntypedValue* group_or(PlayerbotAI* botAI) { return new GroupBoolORValue(botAI); }
    static UntypedValue* group_ready(PlayerbotAI* botAI) { return new GroupReadyValue(botAI); }

    static UntypedValue* active_spell(PlayerbotAI* botAI) { return new ActiveSpellValue(botAI); }
    static UntypedValue* spell_id(PlayerbotAI* botAI) { return new SpellIdValue(botAI); }
    static UntypedValue* vehicle_spell_id(PlayerbotAI* botAI) { return new VehicleSpellIdValue(botAI); }
    //static UntypedValue* inventory_item(PlayerbotAI* botAI) { return new InventoryItemValue(botAI); }
    static UntypedValue* last_spell_cast(PlayerbotAI* botAI) { return new LastSpellCastValue(botAI); }
    static UntypedValue* last_spell_cast_time(PlayerbotAI* botAI) { return new LastSpellCastTimeValue(botAI); }
    static UntypedValue* spell_cast_useful(PlayerbotAI* botAI) { return new SpellCastUsefulValue(botAI); }

    static UntypedValue* expected_lifetime(PlayerbotAI* ai) { return new EstimatedLifetimeValue(ai); }
    static UntypedValue* expected_group_dps(PlayerbotAI* ai) { return new EstimatedGroupDpsValue(ai); }

    static UntypedValue* attacker_without_aura(PlayerbotAI* botAI) { return new AttackerWithoutAuraTargetValue(botAI); }
    static UntypedValue* melee_attacker_without_aura(PlayerbotAI* botAI) { return new MeleeAttackerWithoutAuraTargetValue(botAI); }
    static UntypedValue* party_member_without_aura(PlayerbotAI* botAI) { return new PartyMemberWithoutAuraValue(botAI); }
    static UntypedValue* party_member_to_heal(PlayerbotAI* botAI) { return new PartyMemberToHeal(botAI); }
    static UntypedValue* healer_low_mana(PlayerbotAI* botAI) { return new HealerLowMana(botAI); }
    static UntypedValue* party_member_snared_target(PlayerbotAI* botAI) { return new PartyMemberSnaredTargetValue(botAI); }
    static UntypedValue* party_member_to_resurrect(PlayerbotAI* botAI) { return new PartyMemberToResurrect(botAI); }
    static UntypedValue* party_member_to_dispel(PlayerbotAI* botAI) { return new PartyMemberToDispel(botAI); }
    static UntypedValue* party_member_to_protect(PlayerbotAI* botAI) { return new PartyMemberToProtect(botAI); }

    static UntypedValue* main_tank(PlayerbotAI* ai) { return new PartyMemberMainTankValue(ai); }
    static UntypedValue* tank_target(PlayerbotAI* botAI) { return new TankTargetValue(botAI); }
    static UntypedValue* threat(PlayerbotAI* botAI) { return new ThreatValue(botAI); }
    static UntypedValue* has_aggro(PlayerbotAI* botAI) { return new HasAggroValue(botAI); }

    static UntypedValue* last_flee_angle(PlayerbotAI* ai) { return new LastFleeAngleValue(ai); }
    static UntypedValue* last_flee_timestamp(PlayerbotAI* ai) { return new LastFleeTimestampValue(ai); }
    static UntypedValue* recently_flee_info(PlayerbotAI* ai) { return new RecentlyFleeInfo(ai); }

    static UntypedValue* inventory_items(PlayerbotAI* botAI) { return new InventoryItemValue(botAI); }
    static UntypedValue* item_count(PlayerbotAI* botAI) { return new ItemCountValue(botAI); }
    static UntypedValue* petition_signs(PlayerbotAI* botAI) { return new PetitionSignsValue(botAI); }  // #611 (AC :560)

    // #149
    static UntypedValue* cc_target(PlayerbotAI* botAI) { return new CcTargetValue(botAI); }
    static UntypedValue* current_cc_target(PlayerbotAI* botAI) { return new CurrentCcTargetValue(botAI); }
    static UntypedValue* snare_target(PlayerbotAI* botAI) { return new SnareTargetValue(botAI); }
    static UntypedValue* enemy_healer_target(PlayerbotAI* botAI) { return new EnemyHealerTargetValue(botAI); }
    static UntypedValue* enemy_caster_target(PlayerbotAI* botAI) { return new EnemyCasterTargetValue(botAI); }
    static UntypedValue* is_cc(PlayerbotAI* botAI) { return new IsCcValue(botAI); }
    static UntypedValue* carrying_objective(PlayerbotAI* botAI) { return new CarryingObjectiveValue(botAI); }
    static UntypedValue* in_pvp_instance(PlayerbotAI* botAI) { return new InPvpInstanceValue(botAI); }
    static UntypedValue* duel_target(PlayerbotAI* botAI) { return new DuelTargetValue(botAI); }
    static UntypedValue* inside_target(PlayerbotAI* botAI) { return new InsideTargetValue(botAI); }
    static UntypedValue* nearest_adds(PlayerbotAI* botAI) { return new NearestAddsValue(botAI); }
    static UntypedValue* collision(PlayerbotAI* botAI) { return new CollisionValue(botAI); }
    static UntypedValue* neglect_threat(PlayerbotAI* botAI) { return new NeglectThreatResetValue(botAI); }
    static UntypedValue* new_player_nearby(PlayerbotAI* botAI) { return new NewPlayerNearbyValue(botAI); }
    static UntypedValue* already_seen_players(PlayerbotAI* botAI) { return new AlreadySeenPlayersValue(botAI); }
    static UntypedValue* aoe_heal(PlayerbotAI* botAI) { return new AoeHealValue(botAI); }
    static UntypedValue* aoe_position(PlayerbotAI* botAI) { return new AoePositionValue(botAI); }
    static UntypedValue* aoe_count(PlayerbotAI* botAI) { return new AoeCountValue(botAI); }
    static UntypedValue* has_area_debuff(PlayerbotAI* botAI) { return new HasAreaDebuffValue(botAI); }
    static UntypedValue* party_member_without_item(PlayerbotAI* botAI) { return new PartyMemberWithoutItemValue(botAI); }
    static UntypedValue* party_member_without_food(PlayerbotAI* botAI) { return new PartyMemberWithoutFoodValue(botAI); }
    static UntypedValue* party_member_without_water(PlayerbotAI* botAI) { return new PartyMemberWithoutWaterValue(botAI); }
    static UntypedValue* max_repair_cost(PlayerbotAI* botAI) { return new MaxGearRepairCostValue(botAI); }
    static UntypedValue* repair_cost(PlayerbotAI* botAI) { return new RepairCostValue(botAI); }
    static UntypedValue* train_cost(PlayerbotAI* botAI) { return new TrainCostValue(botAI); }
    static UntypedValue* money_needed_for(PlayerbotAI* botAI) { return new MoneyNeededForValue(botAI); }
    static UntypedValue* total_money_needed_for(PlayerbotAI* botAI) { return new TotalMoneyNeededForValue(botAI); }
    static UntypedValue* free_money_for(PlayerbotAI* botAI) { return new FreeMoneyForValue(botAI); }
    static UntypedValue* should_get_money(PlayerbotAI* botAI) { return new ShouldGetMoneyValue(botAI); }
    static UntypedValue* can_train(PlayerbotAI* botAI) { return new CanTrainValue(botAI); }
    static UntypedValue* rpg_target(PlayerbotAI* botAI) { return new RpgTargetValue(botAI); }
    static UntypedValue* ignore_rpg_target(PlayerbotAI* botAI) { return new IgnoreRpgTargetValue(botAI); }
    static UntypedValue* next_rpg_action(PlayerbotAI* botAI) { return new NextRpgActionValue(botAI); }
    static UntypedValue* travel_target(PlayerbotAI* botAI) { return new TravelTargetValue(botAI); }
    static UntypedValue* bg_type(PlayerbotAI* botAI) { return new BgTypeValue(botAI); }
    static UntypedValue* rpg_bg_type(PlayerbotAI* botAI) { return new RpgBgTypeValue(botAI); }
    static UntypedValue* home_bind(PlayerbotAI* botAI) { return new HomeBindValue(botAI); }
    static UntypedValue* last_long_move(PlayerbotAI* botAI) { return new LastLongMoveValue(botAI); }
    static UntypedValue* lfg_proposal(PlayerbotAI* botAI) { return new LfgProposalValue(botAI); }
    static UntypedValue* loot_target(PlayerbotAI* botAI) { return new LootTargetValue(botAI); }
    static UntypedValue* available_loot(PlayerbotAI* botAI) { return new AvailableLootValue(botAI); }
    static UntypedValue* can_loot(PlayerbotAI* botAI) { return new CanLootValue(botAI); }
    static UntypedValue* has_available_loot(PlayerbotAI* botAI) { return new HasAvailableLootValue(botAI); }
    static UntypedValue* loot_strategy(PlayerbotAI* botAI) { return new LootStrategyValue(botAI); }
    static UntypedValue* always_loot_list(PlayerbotAI* botAI) { return new AlwaysLootListValue(botAI); }
    static UntypedValue* nearest_corpses(PlayerbotAI* botAI) { return new NearestCorpsesValue(botAI); }
    static UntypedValue* nearest_game_objects(PlayerbotAI* botAI) { return new NearestGameObjects(botAI); }
    static UntypedValue* item_usage(PlayerbotAI* botAI) { return new ItemUsageValue(botAI); }
    static UntypedValue* item_upgrade(PlayerbotAI* botAI) { return new ItemUpgradeValue(botAI); }
    static UntypedValue* items_useful_to_give(PlayerbotAI* botAI) { return new ItemsUsefulToGiveValue(botAI); }
    static UntypedValue* skip_spells_list(PlayerbotAI* botAI) { return new SkipSpellsListValue(botAI); }
    static UntypedValue* item_for_spell(PlayerbotAI* botAI) { return new ItemForSpellValue(botAI); }
    static UntypedValue* can_fish(PlayerbotAI* botAI) { return new CanFishValue(botAI); }
    static UntypedValue* can_use_fishing_bobber(PlayerbotAI* botAI) { return new CanUseFishingBobberValue(botAI); }
    static UntypedValue* fishing_spot(PlayerbotAI* botAI) { return new FishingSpotValue(botAI); }
};

#endif
