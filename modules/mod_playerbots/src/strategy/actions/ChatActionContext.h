/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * New context (#173), mirroring AC mod-playerbots Ai/Base/ChatActionContext.h. Registers only the
 * chat-command actions ported in this pass; everything else in AC's 114-creator version (items,
 * quests, trade, loot, LFG, guild, debug, ...) is out of scope here - see the #173 report for the
 * full ported-vs-left-out list. Later chat-command batches add creators here without touching
 * PlayerbotAI.h again.
 */

#ifndef _PLAYERBOT_CHATACTIONCONTEXT_H
#define _PLAYERBOT_CHATACTIONCONTEXT_H

#include "AttackActions.h"
// #258 registration (AC ChatActionContext.h includes, lines 12,14,30-31,42,61-62,82).
#include "BankAction.h"
#include "BuyAction.h"
#include "CastCustomSpellAction.h"  // #611 (AC ChatActionContext.h:164-165)
#include "ChangeStrategyAction.h"
#include "TaxiAction.h"   // #767
#include "DebugAction.h"  // #611 build 13 (AC ChatActionContext.h:22)
#include "GossipHelloAction.h"
#include "GuildBankAction.h"
#include "MailAction.h"
#include "SellAction.h"
#include "SetHomeAction.h"  // #611 (AC ChatActionContext.h:149)
#include "UseAction.h"      // #611 (AC ChatActionContext.h:126,282)
#include "SendMailAction.h"
#include "TrainerAction.h"
#include "ChatShortcutActions.h"
#include "RtscAction.h"  // #648
#include "EquipAction.h"
#include "Formations.h"
#include "HelpAction.h"
#include "InviteToGroupAction.h"
#include "LeaveGroupAction.h"
#include "LootRollAction.h"
#include "LootStrategyAction.h"
#include "NamedObjectContext.h"
#include "NewRpgAction.h"  // #536: TellRpgStatusAction, StartRpgDoQuestAction
#include "PassLeadershipToMasterAction.h"
#include "PetsAction.h"
#include "DropQuestAction.h"
#include "ListQuestsActions.h"
#include "PositionAction.h"
#include "QueryQuestAction.h"
#include "ShareQuestAction.h"
#include "RangeAction.h"
#include "RepairAllAction.h"
#include "ResetAiAction.h"
#include "RtiAction.h"
#include "SaveManaAction.h"
#include "Stances.h"
#include "TradeAction.h"
#include "RewardAction.h"  // #258 (AC ChatActionContext.h)
#include "HireAction.h"    // #258
#include "UnequipAction.h"
#include "UnlockTradedItemAction.h"
#include "UseMeetingStoneAction.h"
#include "WhoAction.h"

class ChatActionContext : public NamedObjectContext<Action>
{
public:
    ChatActionContext()
    {
        creators["who"] = &ChatActionContext::who;
        // #611 (AC ChatActionContext.h:149,164-165): the chat actions the old-rpg sub-actions call through DoSpecificAction.
        creators["use"] = &ChatActionContext::use;
        creators["reward"] = &ChatActionContext::reward;  // #258 (AC ChatActionContext.h:132)
        creators["hire"] = &ChatActionContext::hire;      // #258 (AC :187)
        creators["home"] = &ChatActionContext::home;
        creators["debug"] = &ChatActionContext::debug;   // #611 build 13 (AC ChatActionContext.h:183-184); no chat trigger drives it yet
        creators["cdebug"] = &ChatActionContext::debug;
        creators["cast custom spell"] = &ChatActionContext::cast_custom_spell;
        creators["cast custom nc spell"] = &ChatActionContext::cast_custom_nc_spell;
        creators["help"] = &ChatActionContext::help;
        creators["co"] = &ChatActionContext::co;
        creators["nc"] = &ChatActionContext::nc;
        creators["de"] = &ChatActionContext::dead;
        creators["rti"] = &ChatActionContext::rti;
        creators["follow chat shortcut"] = &ChatActionContext::follow_chat_shortcut;
        creators["stay chat shortcut"] = &ChatActionContext::stay_chat_shortcut;
        creators["move from group chat shortcut"] = &ChatActionContext::move_from_group_chat_shortcut;

        // #177 (group-functionality-parity.md section 3.1a "Movement/state" + "Strategy" rows)
        creators["flee chat shortcut"] = &ChatActionContext::flee_chat_shortcut;
        creators["runaway chat shortcut"] = &ChatActionContext::runaway_chat_shortcut;
        creators["grind chat shortcut"] = &ChatActionContext::grind_chat_shortcut;
        creators["tank attack chat shortcut"] = &ChatActionContext::tank_attack_chat_shortcut;
        creators["max dps chat shortcut"] = &ChatActionContext::max_dps_chat_shortcut;
        creators["attack my target"] = &ChatActionContext::attack_my_target;
        creators["save mana"] = &ChatActionContext::save_mana;
        creators["formation"] = &ChatActionContext::formation;
        creators["range"] = &ChatActionContext::range;
        creators["stance"] = &ChatActionContext::stance;
        creators["position"] = &ChatActionContext::position;
        creators["summon"] = &ChatActionContext::summon;
        creators["reset botAI"] = &ChatActionContext::reset_ai;
        creators["invite"] = &ChatActionContext::invite;
        creators["leave"] = &ChatActionContext::leave;
        creators["join"] = &ChatActionContext::join;
        creators["give leader"] = &ChatActionContext::give_leader;

        // #188 (S6 of epic #163): loot/gear chat commands (AC ChatActionContext.h:117 for "ll").
        creators["repair"] = &ChatActionContext::repair;
        creators["ll"] = &ChatActionContext::ll;
        creators["equip"] = &ChatActionContext::equip;
        creators["unequip"] = &ChatActionContext::unequip;
        // 5.4.8: named "dice roll" not "roll" - the Monk ability "Roll" (a gap-closer spell) already
        // owns the name "roll" in MonkActionFactoryInternal (strategy/Classes/monk/MonkAiObjectContext.cpp)
        // and is checked after the shared ChatActionContext (AiObjectContext::GetAction() returns the
        // first match, and the base AiObjectContext registers ChatActionContext before
        // MonkAiObjectContext adds its own factory) - reusing "roll" here would have silently made
        // Monk bots unable to ever cast Roll.
        creators["dice roll"] = &ChatActionContext::roll;
        creators["equip upgrade"] = &ChatActionContext::equip_upgrade;

        // #193 (S7 of epic #163, AC ChatActionContext.h:129,218): the "t"/"nt" trade commands
        // (both map to "trade" - see ChatCommandHandlerStrategy.cpp) and the rogue "unlock traded
        // item" helper TradeStatusExtendedAction schedules.
        creators["trade"] = &ChatActionContext::trade;
        creators["unlock traded item"] = &ChatActionContext::unlock_traded_item;

        // #194 (S8, AC ChatActionContext.h:208-209): chat-context "pet"/"pet attack", separate from
        // the combat ActionContext's PetAttackAction registration for the same "pet attack" name.
        creators["pet"] = &ChatActionContext::pet;
        creators["pet attack"] = &ChatActionContext::pet_attack;

        // #257 slice H registration (AC ChatActionContext.h:103,112,113,114,115): the four quest
        // chat commands plus the quest-log cleaner.
        creators["drop"] = &ChatActionContext::drop;
        creators["share"] = &ChatActionContext::share;
        creators["quests"] = &ChatActionContext::quests;
        creators["query quest"] = &ChatActionContext::query_quest;
        creators["clean quest log"] = &ChatActionContext::clean_quest_log;

        // #258 registration (AC ChatActionContext.h:130-131,139,154-155,163,180-181). AC puts
        // all eight NPC-interaction chat commands here, in its chat context and not in its
        // ActionContext; only "check mail" (ActionContext.h:158) is an action-context name.
        creators["sell"] = &ChatActionContext::sell;
        creators["buy"] = &ChatActionContext::buy;
        creators["trainer"] = &ChatActionContext::trainer;
        creators["gb"] = &ChatActionContext::gb;
        creators["bank"] = &ChatActionContext::bank;
        creators["gossip hello"] = &ChatActionContext::gossip_hello;
        creators["sendmail"] = &ChatActionContext::sendmail;
        creators["mail"] = &ChatActionContext::mail;
        // #536: AC ChatActionContext.h:109-110
        creators["rpg status"] = &ChatActionContext::rpg_status;
        creators["taxi"] = &ChatActionContext::taxi;   // #767 (AC ChatActionContext.h:123)
        creators["rpg do quest"] = &ChatActionContext::rpg_do_quest;
        creators["rtsc"] = &ChatActionContext::rtsc;  // #648 (AC ChatActionContext.h:197)
    }

private:
    static Action* who(PlayerbotAI* botAI) { return new WhoAction(botAI); }
    static Action* taxi(PlayerbotAI* botAI) { return new TaxiAction(botAI); }   // #767 (AC :285)
    static Action* rpg_status(PlayerbotAI* botAI) { return new TellRpgStatusAction(botAI); }  // #536, AC :301
    static Action* rpg_do_quest(PlayerbotAI* botAI) { return new StartRpgDoQuestAction(botAI); }  // #536, AC :302
    static Action* help(PlayerbotAI* botAI) { return new HelpAction(botAI); }
    static Action* co(PlayerbotAI* botAI) { return new ChangeCombatStrategyAction(botAI); }
    static Action* nc(PlayerbotAI* botAI) { return new ChangeNonCombatStrategyAction(botAI); }
    static Action* dead(PlayerbotAI* botAI) { return new ChangeDeadStrategyAction(botAI); }
    static Action* rti(PlayerbotAI* botAI) { return new RtiAction(botAI); }
    static Action* follow_chat_shortcut(PlayerbotAI* botAI) { return new FollowChatShortcutAction(botAI); }
    static Action* stay_chat_shortcut(PlayerbotAI* botAI) { return new StayChatShortcutAction(botAI); }
    static Action* move_from_group_chat_shortcut(PlayerbotAI* botAI) { return new MoveFromGroupChatShortcutAction(botAI); }

    static Action* flee_chat_shortcut(PlayerbotAI* botAI) { return new FleeChatShortcutAction(botAI); }
    static Action* runaway_chat_shortcut(PlayerbotAI* botAI) { return new GoawayChatShortcutAction(botAI); }
    static Action* grind_chat_shortcut(PlayerbotAI* botAI) { return new GrindChatShortcutAction(botAI); }
    static Action* tank_attack_chat_shortcut(PlayerbotAI* botAI) { return new TankAttackChatShortcutAction(botAI); }
    static Action* max_dps_chat_shortcut(PlayerbotAI* botAI) { return new MaxDpsChatShortcutAction(botAI); }
    static Action* attack_my_target(PlayerbotAI* botAI) { return new AttackMyTargetAction(botAI); }
    static Action* save_mana(PlayerbotAI* botAI) { return new SaveManaAction(botAI); }
    static Action* formation(PlayerbotAI* botAI) { return new SetFormationAction(botAI); }
    static Action* range(PlayerbotAI* botAI) { return new RangeAction(botAI); }
    static Action* stance(PlayerbotAI* botAI) { return new SetStanceAction(botAI); }
    static Action* position(PlayerbotAI* botAI) { return new PositionAction(botAI); }
    static Action* summon(PlayerbotAI* botAI) { return new SummonAction(botAI); }
    static Action* reset_ai(PlayerbotAI* botAI) { return new ResetAiAction(botAI); }
    static Action* invite(PlayerbotAI* botAI) { return new InviteToGroupAction(botAI); }
    static Action* leave(PlayerbotAI* botAI) { return new LeaveGroupAction(botAI); }
    static Action* join(PlayerbotAI* botAI) { return new JoinGroupAction(botAI); }
    static Action* give_leader(PlayerbotAI* botAI) { return new GiveLeaderAction(botAI); }

    static Action* repair(PlayerbotAI* botAI) { return new RepairAllAction(botAI); }
    static Action* ll(PlayerbotAI* botAI) { return new LootStrategyAction(botAI); }
    static Action* equip(PlayerbotAI* botAI) { return new EquipAction(botAI); }
    static Action* unequip(PlayerbotAI* botAI) { return new UnequipAction(botAI); }
    static Action* roll(PlayerbotAI* botAI) { return new RollAction(botAI); }
    static Action* equip_upgrade(PlayerbotAI* botAI) { return new EquipUpgradeAction(botAI); }

    static Action* trade(PlayerbotAI* botAI) { return new TradeAction(botAI); }
    static Action* unlock_traded_item(PlayerbotAI* botAI) { return new UnlockTradedItemAction(botAI); }

    static Action* pet(PlayerbotAI* botAI) { return new PetsAction(botAI); }
    static Action* pet_attack(PlayerbotAI* botAI) { return new PetsAction(botAI, "attack"); }

    static Action* drop(PlayerbotAI* botAI) { return new DropQuestAction(botAI); }
    static Action* share(PlayerbotAI* botAI) { return new ShareQuestAction(botAI); }
    static Action* quests(PlayerbotAI* botAI) { return new ListQuestsAction(botAI); }
    static Action* query_quest(PlayerbotAI* botAI) { return new QueryQuestAction(botAI); }
    static Action* clean_quest_log(PlayerbotAI* botAI) { return new CleanQuestLogAction(botAI); }

    static Action* sell(PlayerbotAI* botAI) { return new SellAction(botAI); }
    static Action* buy(PlayerbotAI* botAI) { return new BuyAction(botAI); }
    static Action* trainer(PlayerbotAI* botAI) { return new TrainerAction(botAI); }
    static Action* gb(PlayerbotAI* botAI) { return new GuildBankAction(botAI); }
    static Action* bank(PlayerbotAI* botAI) { return new BankAction(botAI); }
    static Action* gossip_hello(PlayerbotAI* botAI) { return new GossipHelloAction(botAI); }
    static Action* sendmail(PlayerbotAI* botAI) { return new SendMailAction(botAI); }
    static Action* mail(PlayerbotAI* botAI) { return new MailAction(botAI); }
    static Action* use(PlayerbotAI* botAI) { return new UseAction(botAI); }
    static Action* reward(PlayerbotAI* botAI) { return new RewardAction(botAI); }
    static Action* hire(PlayerbotAI* botAI) { return new HireAction(botAI); }
    static Action* home(PlayerbotAI* botAI) { return new SetHomeAction(botAI); }
    static Action* debug(PlayerbotAI* botAI) { return new DebugAction(botAI); }   // AC ChatActionContext.h:225
    static Action* cast_custom_spell(PlayerbotAI* botAI) { return new CastCustomSpellAction(botAI); }
    static Action* cast_custom_nc_spell(PlayerbotAI* botAI) { return new CastCustomNcSpellAction(botAI); }
    static Action* rtsc(PlayerbotAI* botAI) { return new RTSCAction(botAI); }  // #648
};

#endif
