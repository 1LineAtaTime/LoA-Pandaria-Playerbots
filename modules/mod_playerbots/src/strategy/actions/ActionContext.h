#ifndef _PLAYERBOT_ACTIONCONTEXT_H
#define _PLAYERBOT_ACTIONCONTEXT_H

#include "AcceptDuelAction.h"
#include "AddLootAction.h"
#include "AttackActions.h"
#include "BattleGroundJoinAction.h"
#include "FreeBGJoinAction.h"       // #611 (AC ActionContext.h:196)
#include "CastCustomSpellAction.h"   // #611 (AC ActionContext.h:195,199)
#include "GuildCreateActions.h"      // #611 (AC ActionContext.h:204-208)
#include "EmoteAction.h"
#include "GreetAction.h"
#include "SuggestWhatToDoAction.h"  // #195/#322
#include "TalkAction.h"
#include "BattleGroundTactics.h"
#include "BgTacticsTB.h"
#include "ActionSayHello.h"
#include "AutoMaintenanceOnLevelupAction.h"
#include "CancelChannelAction.h"
#include "ChangeStrategyAction.h"
#include "CheckMailAction.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "GenericActions.h"
#include "GenericSpellActions.h"
#include "PetsAction.h"
#include "PositionAction.h"
#include "RacialActions.h"
#include "SetFocusHealTargetsAction.h"
#include "ShareQuestAction.h"
#include "RtiAction.h"
#include "MovementActions.h"
#include "TravelAction.h"
// #269/#272 (owner rule 2026-10-05: 1:1 incl. dormant code) - AC ActionContext.h includes these too.
#include "ChooseRpgTargetAction.h"
#include "ChooseTravelTargetAction.h"
#include "MoveToRpgTargetAction.h"
#include "MoveToTravelTargetAction.h"
#include "RpgAction.h"
#include "RpgSubActions.h"
#include "CheckMountStateAction.h"
#include "GatherAction.h"
#include "FishingAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "LootAction.h"
#include "NewRpgAction.h"
#include "NewRpgDuelSpot.h"  // #201 (#612)
#include "DropQuestAction.h"
#include "NonCombatActions.h"
#include "RandomBotUpdateAction.h"
#include "PullActions.h"
#include "ReachTargetActions.h"
#include "ResetInstancesAction.h"
#include "StayActions.h"
#include "TeleportToMasterAction.h"
#include "ImbueAction.h"
#include "UseItemAction.h"
#include "RevealGatheringItemAction.h"  // #377
#include "OpenItemAction.h"             // #535
#include "UnlockItemAction.h"           // #535
#include "VehicleActions.h"

class PlayerbotAI;
class ActionContext : public NamedObjectContext<Action>
{
public:
    ActionContext()
    {
        creators["say hello"] = &ActionContext::hello;
        creators["move random"] = &ActionContext::move_random;

        creators["follow"] = &ActionContext::follow;
        creators["flee"] = &ActionContext::flee;
        creators["move from group"] = &ActionContext::move_from_group;
        creators["flee to master"] = &ActionContext::flee_to_master;
        creators["guard"] = &ActionContext::guard;
        // #177 (AC ActionContext.h:81,148,152): SetReturnPositionAction/ReturnAction/
        // ReturnToStayPositionAction classes already existed on this fork (or were added in #177);
        // only the creator registration was missing. "return"/"set return position" back AC's
        // ReturnStrategy, which AC itself never adds to any engine (verified dormant, see the #177
        // report) - registered here for name parity only, not wired into AiFactory.
        creators["set return position"] = &ActionContext::set_return_position;
        creators["return"] = &ActionContext::_return;
        creators["return to stay position"] = &ActionContext::return_to_stay_position;
        creators["runaway"] = &ActionContext::runaway;
        // #269/#272 (owner rule 2026-10-05: port AC 1:1 including dormant code). AC ActionContext.h:82-88.
        // Registered exactly as AC registers them; they stay dormant because, as in AC, nothing adds the
        // "rpg"/"travel" strategies by default (AC AiFactory.cpp:616-622 - see Factory/AiFactory.cpp).
        creators["rpg"] = &ActionContext::rpg;
        creators["crpg"] = &ActionContext::crpg;
        creators["choose rpg target"] = &ActionContext::choose_rpg_target;
        creators["move to rpg target"] = &ActionContext::move_to_rpg_target;
        creators["travel"] = &ActionContext::travel;
        creators["choose travel target"] = &ActionContext::choose_travel_target;
        creators["move to travel target"] = &ActionContext::move_to_travel_target;
        // #269: AC ActionContext.h:189-191 (dark portal; no TriggerNode drives them in AC either) and
        // AggressiveTargetAction (AC ChooseTargetActions.h:39-46). The dark-portal key is now AC's
        // "use dark portal azeroth" (it was "dark portal azeroth" here; nothing referenced it).
        creators["move to dark portal"] = &ActionContext::move_to_dark_portal;
        creators["move from dark portal"] = &ActionContext::move_from_dark_portal;
        creators["use dark portal azeroth"] = &ActionContext::use_dark_portal_azeroth;
        // #272/#201: AC ActionContext.h:245-267, the old-rpg sub-actions ("rpg duel" included). Dormant as
        // in AC: only RpgStrategy / RpgAction reach them, and nothing adds "rpg" by default.
        creators["rpg stay"] = &ActionContext::rpg_stay;
        creators["rpg work"] = &ActionContext::rpg_work;
        creators["rpg emote"] = &ActionContext::rpg_emote;
        creators["rpg cancel"] = &ActionContext::rpg_cancel;
        creators["rpg taxi"] = &ActionContext::rpg_taxi;
        creators["rpg discover"] = &ActionContext::rpg_discover;
        creators["rpg start quest"] = &ActionContext::rpg_start_quest;
        creators["rpg end quest"] = &ActionContext::rpg_end_quest;
        creators["rpg buy"] = &ActionContext::rpg_buy;
        creators["rpg sell"] = &ActionContext::rpg_sell;
        creators["rpg repair"] = &ActionContext::rpg_repair;
        creators["rpg train"] = &ActionContext::rpg_train;
        creators["rpg heal"] = &ActionContext::rpg_heal;
        creators["rpg home bind"] = &ActionContext::rpg_home_bind;
        creators["rpg queue bg"] = &ActionContext::rpg_queue_bg;
        creators["rpg buy petition"] = &ActionContext::rpg_buy_petition;
        creators["rpg use"] = &ActionContext::rpg_use;
        creators["rpg spell"] = &ActionContext::rpg_spell;
        creators["rpg craft"] = &ActionContext::rpg_craft;
        creators["rpg trade useful"] = &ActionContext::rpg_trade_useful;
        creators["rpg duel"] = &ActionContext::rpg_duel;
        creators["rpg mount anim"] = &ActionContext::rpg_mount_anim;
        // #611: the AC actions the old-rpg sub-actions call through DoSpecificAction (AC ActionContext.h:195-208).
        // Registration only: nothing drives them by default, as in AC.
        creators["cast random spell"] = &ActionContext::cast_random_spell;
        creators["free bg join"] = &ActionContext::free_bg_join;
        creators["craft random item"] = &ActionContext::craft_random_item;
        creators["buy petition"] = &ActionContext::buy_petition;
        creators["offer petition"] = &ActionContext::offer_petition;
        creators["offer petition nearby"] = &ActionContext::offer_petition_nearby;
        creators["turn in petition"] = &ActionContext::turn_in_petition;
        creators["buy tabard"] = &ActionContext::buy_tabard;
        creators["aggressive target"] = &ActionContext::aggressive_target;
        // #271 (AC ActionContext.h:89,100,104,105,153): the five MovementActions AC both HAS and REGISTERS, and this fork had neither
        // Registered with AC's exact names so the ported classes are reachable - before this they
        // were dead code, because an unregistered action silently does nothing (the failure mode #258 was
        // filed over). The factories live in the private block below.
        //
        // #271 ported nine classes in total; the other four (MoveInsideAction, RotateAroundTheCenterPointAction,
        // MoveAwayFromCreatureAction, MoveAwayFromPlayerWithDebuffAction) take constructor arguments and AC does
        // NOT register them either - they are base classes for encounter actions (e.g.
        // GrobbulusMoveCenterAction : MoveInsideAction, YoggSaronDeathOrbAction : MoveAwayFromCreatureAction).
        // Registering them here was a mistake caught by the build; do not reintroduce it.
        creators["flee with pet"] = &ActionContext::flee_with_pet;
        creators["rear flank"] = &ActionContext::rear_flank;
        creators["disperse set"] = &ActionContext::disperse_set;
        creators["move out of enemy contact"] = &ActionContext::move_out_of_enemy_contact;
        creators["move out of collision"] = &ActionContext::move_out_of_collision;
        creators["stay"] = &ActionContext::stay;
        creators["teleport to master"] = &ActionContext::teleport_to_master;
        creators["sit"] = &ActionContext::sit;
        // #149 (AC LeaveGroupAction.h:41): ported above, adapted to this fork's PlayerbotAI
        // (no GetGroupLeader(); see the class comment in LeaveGroupAction.h).
        creators["leave far away"] = &ActionContext::leave_far_away;

        creators["random bot update"] = &ActionContext::random_bot_update;
        creators["mark rti"] = &ActionContext::mark_rti;
        creators["attack rti target"] = &ActionContext::attack_rti_target;
        creators["auto maintenance on levelup"] = &ActionContext::auto_maintenance_on_levelup;

        creators["tank face"] = &ActionContext::tank_face;
        creators["tank assist"] = &ActionContext::tank_assist;
        creators["set facing"] = &ActionContext::set_facing;
        // #149 (AC MovementActions.h:220): ported above, mirrors TankFaceAction.
        creators["set behind"] = &ActionContext::set_behind;
        // #149 (AC MovementActions.h:105): ported above; only the "trigger NPC" case is ported -
        // see the class comment in MovementActions.h.
        creators["avoid aoe"] = &ActionContext::avoid_aoe;
        creators["drop target"] = &ActionContext::drop_target;
        creators["dps assist"] = &ActionContext::dps_assist;
        creators["dps aoe"] = &ActionContext::dps_aoe;
        creators["attack"] = &ActionContext::melee;
        creators["melee"] = &ActionContext::melee;
        // #149 (AC CombatActions.h:14,23): ported below (ChangeCombatStrategyAction already existed).
        creators["switch to melee"] = &ActionContext::switch_to_melee;
        creators["switch to ranged"] = &ActionContext::switch_to_ranged;
        creators["attack anything"] = &ActionContext::attack_anything;
        creators["attack enemy player"] = &ActionContext::attack_enemy_player;
        // #149 (AC ActionContext.h:102,107,228): CombatFormationMoveAction/CastShootAction/
        // AttackEnemyFlagCarrierAction already existed (CombatFormationMoveAction is TankFaceAction's
        // already-registered base class), only the direct registration was missing.
        creators["combat formation move"] = &ActionContext::combat_formation_move;
        creators["shoot"] = &ActionContext::shoot;
        creators["attack enemy flag carrier"] = &ActionContext::attack_enemy_flag_carrier;

        creators["toggle pet spell"] = &ActionContext::toggle_pet_spell;
        creators["pet attack"] = &ActionContext::pet_attack;

        creators["reach spell"] = &ActionContext::ReachSpell;
        creators["reach melee"] = &ActionContext::ReachMelee;
        creators["reach party member to heal"] = &ActionContext::reach_party_member_to_heal;
        creators["reach party member to resurrect"] = &ActionContext::reach_party_member_to_resurrect;

        // #408 (AC ActionContext.h): the pull request/perform/lifecycle actions. "reach pull" closes to
        // PullAction's range prerequisite, "return to pull position" to PullBackStrategy's trigger.
        creators["pull my target"] = &ActionContext::pull_my_target;
        creators["pull rti target"] = &ActionContext::pull_rti_target;
        creators["pull start"] = &ActionContext::pull_start;
        creators["pull action"] = &ActionContext::pull_action;
        creators["pull end"] = &ActionContext::pull_end;
        creators["end pull"] = &ActionContext::end_pull;
        creators["return to pull position"] = &ActionContext::return_to_pull_position;
        creators["reach pull"] = &ActionContext::reach_pull;
        creators["use random quest item"] = &ActionContext::use_random_quest_item;  // #257 (AC :198)
        // #149 (AC ActionContext.h:186-187): GroupStrategy.cpp already references these names; ported
        // onto Group::AddInvite()/AddLeaderInvite() instead of AC's hand-built CMSG_GROUP_INVITE
        // packet - see the class comment in InviteToGroupAction.h for why, and for why both are
        // restricted to recruiting other unowned bots only (never a real player).
        creators["invite nearby"] = &ActionContext::invite_nearby;
        creators["invite guild"] = &ActionContext::invite_guild;

        // new rpg (runtime roaming)
        creators["new rpg status update"] = &ActionContext::new_rpg_status_update;
        creators["new rpg go grind"] = &ActionContext::new_rpg_go_grind;
        creators["new rpg go camp"] = &ActionContext::new_rpg_go_camp;
        creators["new rpg wander random"] = &ActionContext::new_rpg_wander_random;
        creators["new rpg wander npc"] = &ActionContext::new_rpg_wander_npc;
        creators["new rpg do quest"] = &ActionContext::new_rpg_do_quest;

        // #257 slice H registration (AC ActionContext.h:183,209). These two QuestStrategies-era
        // action classes had no registration point on this fork at all.
        creators["auto share quest"] = &ActionContext::auto_share_quest;
        creators["clean quest log"] = &ActionContext::clean_quest_log;
        creators["new rpg travel flight"] = &ActionContext::new_rpg_travel_flight;
        creators["new rpg outdoor pvp"] = &ActionContext::new_rpg_outdoor_pvp;
        creators["new rpg duel spot"] = &ActionContext::new_rpg_duel_spot;  // #201 (#612): AC ActionContext.h:280 pattern

        // mount (issue #62): ride a ground mount for long journeys, dismount when it stops making sense.
        creators["check mount state"] = &ActionContext::check_mount_state;

        // loot (issue #62, extended #151)
        creators["loot"] = &ActionContext::loot;
        // #151 (AC ActionContext.h:127-150): the rest of the loot-stack actions.
        creators["add loot"] = &ActionContext::add_loot;
        creators["add all loot"] = &ActionContext::add_all_loot;
        creators["add gathering loot"] = &ActionContext::add_gathering_loot;
        creators["release loot"] = &ActionContext::release_loot;
        creators["move to loot"] = &ActionContext::move_to_loot;
        creators["open loot"] = &ActionContext::open_loot;

        // gather (issue #62): harvest herb/mining nodes.
        creators["gather"] = &ActionContext::gather;
        // #412 (AC ActionContext.h:210-214): fishing.
        creators["move near water"] = &ActionContext::move_near_water;
        creators["go fishing"] = &ActionContext::go_fishing;
        creators["use fishing bobber"] = &ActionContext::use_fishing_bobber;
        // #377 (AC ActionContext.h:160)
        creators["reveal gathering item"] = &ActionContext::reveal_gathering_item;
        // #535 (AC ChatActionContext.h:98-99; registered here, the header this lane owns - the action
        // context chain resolves the key identically)
        creators["open items"] = &ActionContext::open_items;
        creators["unlock items"] = &ActionContext::unlock_items;
        creators["end master fishing"] = &ActionContext::end_master_fishing;
        creators["remove bobber strategy"] = &ActionContext::remove_bobber_strategy;

        // #149 (AC ActionContext.h:119-122, NonCombatActions.h, UseItemAction.h): consumables,
        // ported onto the already-present (dead until now) ItemVisitors.h finders.
        creators["food"] = &ActionContext::food;
        creators["drink"] = &ActionContext::drink;
        // #226 (AC parity): AC ActionContext.h:118 registers "healthstone" -> UseItemAction
        // (AC ActionContext.h:365) immediately before "healing potion", and the matching ActionNode
        // is present on both trees (AC Bot/Engine/Strategy/Strategy.cpp:16,39-46 / MoP
        // strategy/Strategy.cpp:16,37-43) with the identical "healing potion" continuer. This port's
        // #149 consumables cut-down carried food/drink/healing potion/mana potion across but dropped
        // healthstone, so UsePotionsStrategy's "critical health" -> NextAction("healthstone", ...)
        // (AC Ai/Base/Strategy/UsePotionsStrategy.cpp:34 / MoP
        // strategy/generic/UsePotionsStrategy.cpp:8) pushed a node whose action factory lookup
        // returned null, and Engine::InitializeAction (strategy/Engine.cpp:554-563) logged
        // "A:healthstone - UNKNOWN" (Engine.cpp:176) on every tick the bot sat at critical health.
        creators["healthstone"] = &ActionContext::healthstone;
        creators["healing potion"] = &ActionContext::healing_potion;
        creators["mana potion"] = &ActionContext::mana_potion;
        // #434 (AC ActionContext.h:168-169): temporary weapon enchants (sharpening stone / weightstone,
        // wizard / mana oil). "apply poison" (AC :167) is not ported: rogue poisons are self-cast spells in 5.4.8.
        creators["apply stone"] = &ActionContext::apply_stone;
        creators["apply oil"] = &ActionContext::apply_oil;

        // #149: ResetInstancesAction ported from AC, adapted to this core's
        // WorldSession::HandleResetInstancesOpcode(WorldPacket&) signature.
        creators["reset instances"] = &ActionContext::reset_instances;

        // battlegrounds (issue #33)
        creators["bg join"] = &ActionContext::bg_join;
        creators["bg status"] = &ActionContext::bg_status;
        creators["bg status check"] = &ActionContext::bg_status_check;  // #396 AC WorldPacketActionContext.h:104 (next to "bg status", as on this fork)
        creators["disenchant random item"] = &ActionContext::disenchant_random_item;  // #611 AC ActionContext.h:201
        creators["enchant random item"] = &ActionContext::enchant_random_item;        // #611 AC ActionContext.h:202
        creators["bg leave"] = &ActionContext::bg_leave;
        creators["bg strategy check"] = &ActionContext::bg_strategy_check;
        // In-battleground objective play (#33 slice B). All of these are BGTactics under
        // different names; BGTactics::Execute switches on getName().
        creators["bg move to start"] = &ActionContext::bg_move_to_start;
        creators["bg move to objective"] = &ActionContext::bg_move_to_objective;
        creators["bg select objective"] = &ActionContext::bg_select_objective;
        creators["bg check objective"] = &ActionContext::bg_check_objective;
        creators["tb play"] = &ActionContext::tb_play;   // #901
        creators["bg reset objective force"] = &ActionContext::bg_reset_objective_force;
        creators["bg check flag"] = &ActionContext::bg_check_flag;
        creators["bg protect fc"] = &ActionContext::bg_protect_fc;
        creators["bg attack fc"] = &ActionContext::bg_attack_fc;
        creators["bg use buff"] = &ActionContext::bg_use_buff;

        // #346 (AC WorldPacketActionContext.h:109,177): arena positioning. ArenaTactics is its own class
        // and "battleground" is deliberately NOT put on an arena engine (AiFactory), so this name is the
        // only route to positioning inside an arena match. Without it the ported ArenaTactics is never
        // instantiated and the whole arena behaviour layer stays inert.
        creators["arena tactics"] = &ActionContext::arena_tactics;

        // #179 (AC ActionContext.h:232-233, spec section 3 "vehicles: enter/leave the master's
        // vehicle", after #150): the "nearest vehicles" auto-board branch is scoped out - see the
        // .cpp file comment.
        creators["enter vehicle"] = &ActionContext::enter_vehicle;
        creators["leave vehicle"] = &ActionContext::leave_vehicle;
        // #393 (AC ActionContext.h:234-243): the Isle of Conquest vehicle weapons (classes in GenericSpellActions.h;
        // they cast through PlayerbotAI::CanCastVehicleSpell/CastVehicleSpell, #581).
        creators["hurl boulder"] = &ActionContext::hurl_boulder;
        creators["ram"] = &ActionContext::ram;
        creators["steam rush"] = &ActionContext::steam_rush;
        creators["steam blast"] = &ActionContext::steam_blast;
        creators["napalm"] = &ActionContext::napalm;
        creators["fire cannon"] = &ActionContext::fire_cannon;
        creators["incendiary rocket"] = &ActionContext::incendiary_rocket;
        creators["rocket blast"] = &ActionContext::rocket_blast;
        creators["blade salvo"] = &ActionContext::blade_salvo;
        creators["glaive throw"] = &ActionContext::glaive_throw;

        // #163 S8 (AC parity, DuelStrategy.cpp): answer a duel request, attack the duel opponent is
        // already wired via AttackActions.h but was never registered.
        creators["accept duel"] = &ActionContext::accept_duel;
        creators["attack duel opponent"] = &ActionContext::attack_duel_opponent;

        // #163 S8 / #147 (AC parity): pet stance chat command + the "new pet" -> auto-stance wiring.
        creators["pet"] = &ActionContext::pet;
        creators["set pet stance"] = &ActionContext::set_pet_stance;

        // #163 S8 (AC parity): the "focus heal targets" chat command.
        creators["focus heal targets"] = &ActionContext::focus_heal_targets;

        // #163 S8 (AC parity): the racials strategy's actions, plus "use trinket" (the class existed,
        // GenericSpellActions.h, but had no creator).
        creators["arcane torrent"] = &ActionContext::arcane_torrent;
        creators["war stomp"] = &ActionContext::war_stomp;
        creators["berserking"] = &ActionContext::berserking;
        creators["every man for himself"] = &ActionContext::every_man_for_himself;
        creators["will of the forsaken"] = &ActionContext::will_of_the_forsaken;
        creators["stoneform"] = &ActionContext::stoneform;
        creators["escape artist"] = &ActionContext::escape_artist;
        creators["blood fury"] = &ActionContext::blood_fury;
        creators["gift of the naaru"] = &ActionContext::gift_of_the_naaru;
        creators["lifeblood"] = &ActionContext::lifeblood;
        creators["use trinket"] = &ActionContext::use_trinket;

        // #163 S8 (AC parity): receive-and-react-to-emote (EmoteStrategy.h).
        creators["emote"] = &ActionContext::emote;
        // #194 (S8, AC parity Ai/Base/Strategy/EmoteStrategy.cpp): idle-chatter talk + new-player greet,
        // both gated in EmoteStrategy.cpp (AiPlayerbot.RandomBotEmote / AiPlayerbot.EnableGreet).
        creators["talk"] = &ActionContext::talk;
        creators["greet"] = &ActionContext::greet;
        // #195/#322 (AC Ai/Base/ActionContext.h): world talk, wired by EmoteStrategy behind RandomBotTalk
        creators["suggest what to do"] = &ActionContext::suggest_what_to_do;
        creators["suggest dungeon"] = &ActionContext::suggest_dungeon;
        creators["suggest trade"] = &ActionContext::suggest_trade;

        // #258 registration (AC Ai/Base/ActionContext.h:158 "check mail" -> :301 CheckMailAction).
        // The only one of the nine #258 NPC-interaction actions AC registers in the *action*
        // context rather than the chat context; the other eight live in ChatActionContext.h.
        // AC never wires it to a live trigger either - WorldPacketHandlerStrategy.cpp:48
        // ("often" -> "check mail") is commented out on AC - so this is name parity only,
        // exactly as the three #177 names above.
        creators["check mail"] = &ActionContext::check_mail;

        // #423 (AC ActionContext.h:216 -> :335 CancelChannelAction, AC Ai/Base/Actions/CancelChannelAction.h).
        // This is NOT name parity only: GenericMageStrategy.cpp:265 pushes NextAction("cancel channel", 26.0f)
        // under the "blizzard channel check" trigger node and GenericDruidStrategy.cpp:188-190 has the
        // "hurricane channel check" edge deliberately withheld for the same reason, so with the action
        // unregistered Engine::InitializeAction logged "A:cancel channel - UNKNOWN" (the #258 failure mode).
        creators["cancel channel"] = &ActionContext::cancel_channel;
    }

private:
    static Action* hello(PlayerbotAI* botAI) { return new SayHelloAction(botAI); }
    static Action* move_random(PlayerbotAI* botAI) { return new MoveRandomAction(botAI); }

    static Action* follow(PlayerbotAI* botAI) { return new FollowAction(botAI); }
    static Action* flee(PlayerbotAI* botAI) { return new FleeAction(botAI); }
    static Action* move_from_group(PlayerbotAI* botAI) { return new MoveFromGroupAction(botAI); }
    static Action* flee_to_master(PlayerbotAI* botAI) { return new FleeToMasterAction(botAI); }
    static Action* guard(PlayerbotAI* botAI) { return new GuardAction(botAI); }
    static Action* set_return_position(PlayerbotAI* botAI) { return new SetReturnPositionAction(botAI); }
    static Action* _return(PlayerbotAI* botAI) { return new ReturnAction(botAI); }
    static Action* return_to_stay_position(PlayerbotAI* botAI) { return new ReturnToStayPositionAction(botAI); }
    static Action* stay(PlayerbotAI* botAI) { return new StayAction(botAI); }
    static Action* teleport_to_master(PlayerbotAI* botAI) { return new TeleportToMasterAction(botAI); }
    static Action* sit(PlayerbotAI* botAI) { return new SitAction(botAI); }
    static Action* runaway(PlayerbotAI* botAI) { return new RunAwayAction(botAI); }
    // #269
    static Action* rpg(PlayerbotAI* botAI) { return new RpgAction(botAI); }
    static Action* crpg(PlayerbotAI* botAI) { return new CRpgAction(botAI); }
    static Action* choose_rpg_target(PlayerbotAI* botAI) { return new ChooseRpgTargetAction(botAI); }
    static Action* move_to_rpg_target(PlayerbotAI* botAI) { return new MoveToRpgTargetAction(botAI); }
    static Action* travel(PlayerbotAI* botAI) { return new TravelAction(botAI); }
    static Action* choose_travel_target(PlayerbotAI* botAI) { return new ChooseTravelTargetAction(botAI); }
    static Action* move_to_travel_target(PlayerbotAI* botAI) { return new MoveToTravelTargetAction(botAI); }
    static Action* move_to_dark_portal(PlayerbotAI* botAI) { return new MoveToDarkPortalAction(botAI); }
    static Action* use_dark_portal_azeroth(PlayerbotAI* botAI) { return new DarkPortalAzerothAction(botAI); }
    static Action* rpg_stay(PlayerbotAI* botAI) { return new RpgStayAction(botAI); }
    static Action* rpg_work(PlayerbotAI* botAI) { return new RpgWorkAction(botAI); }
    static Action* rpg_emote(PlayerbotAI* botAI) { return new RpgEmoteAction(botAI); }
    static Action* rpg_cancel(PlayerbotAI* botAI) { return new RpgCancelAction(botAI); }
    static Action* rpg_taxi(PlayerbotAI* botAI) { return new RpgTaxiAction(botAI); }
    static Action* rpg_discover(PlayerbotAI* botAI) { return new RpgDiscoverAction(botAI); }
    static Action* rpg_start_quest(PlayerbotAI* botAI) { return new RpgStartQuestAction(botAI); }
    static Action* rpg_end_quest(PlayerbotAI* botAI) { return new RpgEndQuestAction(botAI); }
    static Action* rpg_buy(PlayerbotAI* botAI) { return new RpgBuyAction(botAI); }
    static Action* rpg_sell(PlayerbotAI* botAI) { return new RpgSellAction(botAI); }
    static Action* rpg_repair(PlayerbotAI* botAI) { return new RpgRepairAction(botAI); }
    static Action* rpg_train(PlayerbotAI* botAI) { return new RpgTrainAction(botAI); }
    static Action* rpg_heal(PlayerbotAI* botAI) { return new RpgHealAction(botAI); }
    static Action* rpg_home_bind(PlayerbotAI* botAI) { return new RpgHomeBindAction(botAI); }
    static Action* rpg_queue_bg(PlayerbotAI* botAI) { return new RpgQueueBgAction(botAI); }
    static Action* rpg_buy_petition(PlayerbotAI* botAI) { return new RpgBuyPetitionAction(botAI); }
    static Action* rpg_use(PlayerbotAI* botAI) { return new RpgUseAction(botAI); }
    static Action* rpg_spell(PlayerbotAI* botAI) { return new RpgSpellAction(botAI); }
    static Action* rpg_craft(PlayerbotAI* botAI) { return new RpgCraftAction(botAI); }
    static Action* rpg_trade_useful(PlayerbotAI* botAI) { return new RpgTradeUsefulAction(botAI); }
    static Action* rpg_duel(PlayerbotAI* botAI) { return new RpgDuelAction(botAI); }
    static Action* rpg_mount_anim(PlayerbotAI* botAI) { return new RpgMountAnimAction(botAI); }
    static Action* cast_random_spell(PlayerbotAI* botAI) { return new CastRandomSpellAction(botAI); }
    static Action* free_bg_join(PlayerbotAI* botAI) { return new FreeBGJoinAction(botAI); }
    static Action* craft_random_item(PlayerbotAI* botAI) { return new CraftRandomItemAction(botAI); }
    static Action* buy_petition(PlayerbotAI* botAI) { return new BuyPetitionAction(botAI); }
    static Action* offer_petition(PlayerbotAI* botAI) { return new PetitionOfferAction(botAI); }
    static Action* offer_petition_nearby(PlayerbotAI* botAI) { return new PetitionOfferNearbyAction(botAI); }
    static Action* turn_in_petition(PlayerbotAI* botAI) { return new PetitionTurnInAction(botAI); }
    static Action* buy_tabard(PlayerbotAI* botAI) { return new BuyTabardAction(botAI); }
    static Action* move_from_dark_portal(PlayerbotAI* botAI) { return new MoveFromDarkPortalAction(botAI); }
    static Action* aggressive_target(PlayerbotAI* botAI) { return new AggressiveTargetAction(botAI); }
    // #271 - the nine MovementActions ported from AC (AC ActionContext.h:297,318,322,323,366 + the
    // Naxx/Raid-file entries AC keeps for move_inside/rotate/move_away_from_*).
    static Action* flee_with_pet(PlayerbotAI* botAI) { return new FleeWithPetAction(botAI); }
    static Action* rear_flank(PlayerbotAI* botAI) { return new RearFlankAction(botAI); }
    static Action* disperse_set(PlayerbotAI* botAI) { return new DisperseSetAction(botAI); }
    static Action* move_out_of_enemy_contact(PlayerbotAI* botAI) { return new MoveOutOfEnemyContactAction(botAI); }
    static Action* move_out_of_collision(PlayerbotAI* botAI) { return new MoveOutOfCollisionAction(botAI); }
    static Action* leave_far_away(PlayerbotAI* botAI) { return new LeaveFarAwayAction(botAI); }

    static Action* random_bot_update(PlayerbotAI* botAI) { return new RandomBotUpdateAction(botAI); }
    static Action* mark_rti(PlayerbotAI* botAI) { return new MarkRtiAction(botAI); }
    static Action* attack_rti_target(PlayerbotAI* botAI) { return new AttackRtiTargetAction(botAI); }
    static Action* auto_maintenance_on_levelup(PlayerbotAI* botAI) { return new AutoMaintenanceOnLevelupAction(botAI); }

    static Action* tank_face(PlayerbotAI* botAI) { return new TankFaceAction(botAI); }
    static Action* tank_assist(PlayerbotAI* botAI) { return new TankAssistAction(botAI); }
    static Action* set_facing(PlayerbotAI* botAI) { return new SetFacingTargetAction(botAI); }
    static Action* set_behind(PlayerbotAI* botAI) { return new SetBehindTargetAction(botAI); }
    static Action* avoid_aoe(PlayerbotAI* botAI) { return new AvoidAoeAction(botAI); }
    static Action* drop_target(PlayerbotAI* botAI) { return new DropTargetAction(botAI); }
    static Action* dps_assist(PlayerbotAI* botAI) { return new DpsAssistAction(botAI); }
    static Action* dps_aoe(PlayerbotAI* botAI) { return new DpsAoeAction(botAI); }
    static Action* melee(PlayerbotAI* botAI) { return new MeleeAction(botAI); }
    static Action* switch_to_melee(PlayerbotAI* botAI) { return new SwitchToMeleeAction(botAI); }
    static Action* switch_to_ranged(PlayerbotAI* botAI) { return new SwitchToRangedAction(botAI); }
    static Action* attack_anything(PlayerbotAI* botAI) { return new AttackAnythingAction(botAI); }
    static Action* attack_enemy_player(PlayerbotAI* botAI) { return new AttackEnemyPlayerAction(botAI); }
    static Action* combat_formation_move(PlayerbotAI* botAI) { return new CombatFormationMoveAction(botAI); }
    static Action* shoot(PlayerbotAI* botAI) { return new CastShootAction(botAI); }
    static Action* attack_enemy_flag_carrier(PlayerbotAI* botAI) { return new AttackEnemyFlagCarrierAction(botAI); }

    static Action* toggle_pet_spell(PlayerbotAI* ai) { return new TogglePetSpellAutoCastAction(ai); }
    static Action* pet_attack(PlayerbotAI* ai) { return new PetAttackAction(ai); }

    static Action* ReachSpell(PlayerbotAI* botAI) { return new ReachSpellAction(botAI); }
    static Action* ReachMelee(PlayerbotAI* botAI) { return new ReachMeleeAction(botAI); }
    static Action* reach_party_member_to_heal(PlayerbotAI* botAI) { return new ReachPartyMemberToHealAction(botAI); }
    static Action* reach_party_member_to_resurrect(PlayerbotAI* botAI) { return new ReachPartyMemberToResurrectAction(botAI); }

    // new rpg (runtime roaming)
    static Action* new_rpg_status_update(PlayerbotAI* botAI) { return new NewRpgStatusUpdateAction(botAI); }
    static Action* new_rpg_go_grind(PlayerbotAI* botAI) { return new NewRpgGoGrindAction(botAI); }
    static Action* new_rpg_go_camp(PlayerbotAI* botAI) { return new NewRpgGoCampAction(botAI); }
    static Action* new_rpg_wander_random(PlayerbotAI* botAI) { return new NewRpgWanderRandomAction(botAI); }
    static Action* new_rpg_wander_npc(PlayerbotAI* botAI) { return new NewRpgWanderNpcAction(botAI); }
    static Action* new_rpg_do_quest(PlayerbotAI* botAI) { return new NewRpgDoQuestAction(botAI); }

    static Action* auto_share_quest(PlayerbotAI* ai) { return new AutoShareQuestAction(ai); }
    static Action* clean_quest_log(PlayerbotAI* ai) { return new CleanQuestLogAction(ai); }
    static Action* new_rpg_travel_flight(PlayerbotAI* botAI) { return new NewRpgTravelFlightAction(botAI); }
    static Action* new_rpg_outdoor_pvp(PlayerbotAI* botAI) { return new NewRpgOutdoorPvpAction(botAI); }
    static Action* new_rpg_duel_spot(PlayerbotAI* botAI) { return new NewRpgDuelSpotAction(botAI); }  // #201 (#612): AC :486 pattern

    // mount (issue #62)
    static Action* check_mount_state(PlayerbotAI* botAI) { return new CheckMountStateAction(botAI); }

    // loot (issue #62, extended #151)
    static Action* loot(PlayerbotAI* botAI) { return new LootAction(botAI); }
    static Action* add_loot(PlayerbotAI* botAI) { return new AddLootAction(botAI); }
    static Action* add_all_loot(PlayerbotAI* botAI) { return new AddAllLootAction(botAI); }
    static Action* add_gathering_loot(PlayerbotAI* botAI) { return new AddGatheringLootAction(botAI); }
    static Action* release_loot(PlayerbotAI* botAI) { return new ReleaseLootAction(botAI); }
    static Action* move_to_loot(PlayerbotAI* botAI) { return new MoveToLootAction(botAI); }
    static Action* open_loot(PlayerbotAI* botAI) { return new OpenLootAction(botAI); }

    // gather (issue #62)
    static Action* gather(PlayerbotAI* botAI) { return new GatherAction(botAI); }
    // #412 (AC ActionContext.h:417-421)
    static Action* move_near_water(PlayerbotAI* botAI) { return new MoveNearWaterAction(botAI); }
    static Action* go_fishing(PlayerbotAI* botAI) { return new FishingAction(botAI); }
    static Action* use_fishing_bobber(PlayerbotAI* botAI) { return new UseBobberAction(botAI); }
    static Action* reveal_gathering_item(PlayerbotAI* botAI) { return new RevealGatheringItemAction(botAI); }
    static Action* open_items(PlayerbotAI* botAI) { return new OpenItemAction(botAI); }
    static Action* unlock_items(PlayerbotAI* botAI) { return new UnlockItemAction(botAI); }
    static Action* end_master_fishing(PlayerbotAI* botAI) { return new EndMasterFishingAction(botAI); }
    static Action* remove_bobber_strategy(PlayerbotAI* botAI) { return new RemoveBobberStrategyAction(botAI); }

    // #149: group self-organizing (unowned bots only, see InviteToGroupAction.h)
    static Action* invite_nearby(PlayerbotAI* botAI) { return new InviteNearbyToGroupAction(botAI); }
    static Action* invite_guild(PlayerbotAI* botAI) { return new InviteGuildToGroupAction(botAI); }

    // #149: consumables
    static Action* food(PlayerbotAI* botAI) { return new EatAction(botAI); }
    static Action* drink(PlayerbotAI* botAI) { return new DrinkAction(botAI); }
    static Action* healing_potion(PlayerbotAI* botAI) { return new UseHealingPotion(botAI); }
    // #226 (AC parity): AC ActionContext.h:365, placed after healing_potion exactly as on AC.
    // The item itself is resolved by UseItemAction::Execute through
    // AI_VALUE2(std::vector<Item*>, "inventory items", name) -> InventoryAction::Find(name),
    // i.e. the same named-item resolver path.
    static Action* healthstone(PlayerbotAI* botAI) { return new UseItemAction(botAI, "healthstone"); }
    static Action* mana_potion(PlayerbotAI* botAI) { return new UseManaPotion(botAI); }
    // #434 (AC ActionContext.h:376-377)
    static Action* apply_oil(PlayerbotAI* botAI) { return new ImbueWithOilAction(botAI); }
    static Action* apply_stone(PlayerbotAI* botAI) { return new ImbueWithStoneAction(botAI); }

    static Action* reset_instances(PlayerbotAI* botAI) { return new ResetInstancesAction(botAI); }

    // battlegrounds (issue #33)
    static Action* bg_join(PlayerbotAI* botAI) { return new BGJoinAction(botAI); }
    static Action* bg_status(PlayerbotAI* botAI) { return new BGStatusAction(botAI); }
    static Action* bg_status_check(PlayerbotAI* botAI) { return new BGStatusCheckAction(botAI); }
    static Action* disenchant_random_item(PlayerbotAI* botAI) { return new DisEnchantRandomItemAction(botAI); }
    static Action* enchant_random_item(PlayerbotAI* botAI) { return new EnchantRandomItemAction(botAI); }
    static Action* bg_leave(PlayerbotAI* botAI) { return new BGLeaveAction(botAI); }
    static Action* bg_strategy_check(PlayerbotAI* botAI) { return new BGStrategyCheckAction(botAI); }

    // #346 (AC WorldPacketActionContext.h:109,177). See the creators[] entry above.
    static Action* arena_tactics(PlayerbotAI* botAI) { return new ArenaTactics(botAI); }
    static Action* bg_move_to_start(PlayerbotAI* botAI) { return new BGTactics(botAI, "move to start"); }
    static Action* bg_move_to_objective(PlayerbotAI* botAI) { return new BGTactics(botAI, "move to objective"); }
    static Action* bg_select_objective(PlayerbotAI* botAI) { return new BGTactics(botAI, "select objective"); }
    static Action* bg_check_objective(PlayerbotAI* botAI) { return new BGTactics(botAI, "check objective"); }
    static Action* tb_play(PlayerbotAI* botAI) { return new TolBaradPlayAction(botAI); }
    static Action* bg_reset_objective_force(PlayerbotAI* botAI) { return new BGTactics(botAI, "reset objective force"); }
    static Action* bg_check_flag(PlayerbotAI* botAI) { return new BGTactics(botAI, "check flag"); }
    static Action* bg_protect_fc(PlayerbotAI* botAI) { return new BGTactics(botAI, "protect fc"); }
    static Action* bg_attack_fc(PlayerbotAI* botAI) { return new BGTactics(botAI, "attack fc"); }
    static Action* bg_use_buff(PlayerbotAI* botAI) { return new BGTactics(botAI, "use buff"); }

    static Action* enter_vehicle(PlayerbotAI* botAI) { return new EnterVehicleAction(botAI); }
    static Action* leave_vehicle(PlayerbotAI* botAI) { return new LeaveVehicleAction(botAI); }
    static Action* hurl_boulder(PlayerbotAI* botAI) { return new CastHurlBoulderAction(botAI); }
    static Action* ram(PlayerbotAI* botAI) { return new CastRamAction(botAI); }
    static Action* steam_blast(PlayerbotAI* botAI) { return new CastSteamBlastAction(botAI); }
    static Action* steam_rush(PlayerbotAI* botAI) { return new CastSteamRushAction(botAI); }
    static Action* napalm(PlayerbotAI* botAI) { return new CastNapalmAction(botAI); }
    static Action* fire_cannon(PlayerbotAI* botAI) { return new CastFireCannonAction(botAI); }
    static Action* incendiary_rocket(PlayerbotAI* botAI) { return new CastIncendiaryRocketAction(botAI); }
    static Action* rocket_blast(PlayerbotAI* botAI) { return new CastRocketBlastAction(botAI); }
    static Action* glaive_throw(PlayerbotAI* botAI) { return new CastGlaiveThrowAction(botAI); }
    static Action* blade_salvo(PlayerbotAI* botAI) { return new CastBladeSalvoAction(botAI); }

    static Action* accept_duel(PlayerbotAI* botAI) { return new AcceptDuelAction(botAI); }
    static Action* attack_duel_opponent(PlayerbotAI* botAI) { return new AttackDuelOpponentAction(botAI); }

    static Action* pet(PlayerbotAI* botAI) { return new PetsAction(botAI); }
    static Action* set_pet_stance(PlayerbotAI* botAI) { return new SetPetStanceAction(botAI); }
    static Action* focus_heal_targets(PlayerbotAI* botAI) { return new SetFocusHealTargetsAction(botAI); }

    static Action* arcane_torrent(PlayerbotAI* botAI) { return new CastArcaneTorrentAction(botAI); }
    static Action* war_stomp(PlayerbotAI* botAI) { return new CastWarStompAction(botAI); }
    static Action* berserking(PlayerbotAI* botAI) { return new CastBerserkingAction(botAI); }
    static Action* every_man_for_himself(PlayerbotAI* botAI) { return new CastEveryManForHimselfAction(botAI); }
    static Action* will_of_the_forsaken(PlayerbotAI* botAI) { return new CastWillOfTheForsakenAction(botAI); }
    static Action* stoneform(PlayerbotAI* botAI) { return new CastStoneformAction(botAI); }
    static Action* escape_artist(PlayerbotAI* botAI) { return new CastEscapeArtistAction(botAI); }
    static Action* blood_fury(PlayerbotAI* botAI) { return new CastBloodFuryAction(botAI); }
    static Action* gift_of_the_naaru(PlayerbotAI* botAI) { return new CastGiftOfTheNaaruAction(botAI); }
    static Action* lifeblood(PlayerbotAI* botAI) { return new CastLifebloodAction(botAI); }
    static Action* use_trinket(PlayerbotAI* botAI) { return new UseTrinketAction(botAI); }
    static Action* emote(PlayerbotAI* botAI) { return new EmoteAction(botAI); }
    static Action* talk(PlayerbotAI* botAI) { return new TalkAction(botAI); }
    static Action* greet(PlayerbotAI* botAI) { return new GreetAction(botAI); }
    static Action* suggest_what_to_do(PlayerbotAI* botAI) { return new SuggestWhatToDoAction(botAI); }
    static Action* suggest_dungeon(PlayerbotAI* botAI) { return new SuggestDungeonAction(botAI); }
    static Action* suggest_trade(PlayerbotAI* botAI) { return new SuggestTradeAction(botAI); }
    static Action* check_mail(PlayerbotAI* botAI) { return new CheckMailAction(botAI); }

    // #423 (AC ActionContext.h:335)
    static Action* cancel_channel(PlayerbotAI* botAI) { return new CancelChannelAction(botAI); }

    // #408 (AC ActionContext.h)
    static Action* pull_my_target(PlayerbotAI* botAI) { return new PullMyTargetAction(botAI); }
    static Action* pull_rti_target(PlayerbotAI* botAI) { return new PullRtiTargetAction(botAI); }
    static Action* pull_start(PlayerbotAI* botAI) { return new PullStartAction(botAI); }
    static Action* pull_action(PlayerbotAI* botAI) { return new PullAction(botAI); }
    static Action* pull_end(PlayerbotAI* botAI) { return new PullEndAction(botAI); }
    static Action* end_pull(PlayerbotAI* botAI) { return new ChangeCombatStrategyAction(botAI, "-pull"); }
    static Action* return_to_pull_position(PlayerbotAI* botAI) { return new ReturnToPullPositionAction(botAI); }
    static Action* reach_pull(PlayerbotAI* botAI) { return new ReachPullAction(botAI); }
    static Action* use_random_quest_item(PlayerbotAI* botAI) { return new UseRandomQuestItem(botAI); }  // #257 (AC ActionContext.h:198)
};

#endif
