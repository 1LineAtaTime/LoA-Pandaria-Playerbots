/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETACTIONCONTEXT_H
#define _PLAYERBOT_WORLDPACKETACTIONCONTEXT_H


#include "NamedObjectContext.h"

#include "AcceptInvitationAction.h"
#include "AcceptQuestAction.h"
#include "EquipAction.h"
#include "LfgActions.h"
#include "LootAction.h"
#include "LootRollAction.h"
#include "PassLeadershipToMasterAction.h"
#include "QuestAction.h"
#include "QuestConfirmAcceptAction.h"
#include "ReadyCheckAction.h"
#include "ReleaseSpiritAction.h"
#include "ReviveFromCorpseAction.h"
#include "SeeSpellAction.h"
#include "AreaTriggerAction.h"
#include "LeaveGroupAction.h"
#include "RememberTaxiAction.h"
#include "TaxiAction.h"
#include "TalkToQuestGiverAction.h"
#include "TradeStatusAction.h"
#include "TradeStatusExtendedAction.h"
#include "UseMeetingStoneAction.h"
#include "XpGainAction.h"
#include "TellMasterAction.h"    // #258
#include "GuildAcceptAction.h"   // #258
#include "PetitionSignAction.h"  // #258

class PlayerbotAI;

class WorldPacketActionContext : public NamedObjectContext<Action>
{
public:
    WorldPacketActionContext()
    {
        // party handlers
        creators["accept invitation"] = &WorldPacketActionContext::accept_invitation;
        creators["uninvite"] = &WorldPacketActionContext::uninvite;
        // #178/#179 (AC LeaveGroupAction.h:34-38): react to SMSG_PARTY_COMMAND_RESULT the instant
        // the bot's own master leaves the party (see LeaveGroupAction.cpp for the .cpp-local
        // FindNewMasterForPartyCommand() this fork uses instead of AC's PlayerbotAI::FindNewMaster()).
        creators["party command"] = &WorldPacketActionContext::party_command;

        // death
        creators["revive from corpse"] = &WorldPacketActionContext::revive_from_corpse;
        creators["find corpse"] = &WorldPacketActionContext::find_corpse;
        creators["auto release"] = &WorldPacketActionContext::auto_release;

        // issue #99: these three classes existed but were never registered anywhere, so
        // DoSpecificAction("spirit healer") / "repop" always returned false and the
        // "can self resurrect" and "falling far" triggers were inert.
        creators["spirit healer"] = &WorldPacketActionContext::spirit_healer;
        creators["repop"] = &WorldPacketActionContext::repop;
        creators["self resurrect"] = &WorldPacketActionContext::self_resurrect;

        // #149 (AC WorldPacketActionContext.h:66 / ChatActionContext.h:120): AcceptResurrectAction ported
        // above; ReleaseSpiritAction already existed and defaults to the name "release" (its "auto
        // release" sibling was the only name registered before this).
        creators["accept resurrect"] = &WorldPacketActionContext::accept_resurrect;
        creators["release"] = &WorldPacketActionContext::release;

        // D4 (death-revive-parity.md F4): AC XpGainAction.cpp:16 resets "death count" to 0 on XP gain.
        creators["xp gain"] = &WorldPacketActionContext::xp_gain;

        // #151 (AC WorldPacketActionContext.h:74,132): fired on the "loot response" WorldPacket
        // trigger (SMSG_LOOT_RESPONSE).
        creators["store loot"] = &WorldPacketActionContext::store_loot;

        // #179 (AC WorldPacketActionContext.h): taxi with the master. AC registers "taxi" itself in
        // ChatActionContext.h; both are merged into the same lookup, so registering it here instead
        // (alongside its packet-driven sibling) makes no behavioural difference.
        creators["remember taxi"] = &WorldPacketActionContext::remember_taxi;
        creators["taxi"] = &WorldPacketActionContext::taxi;

        // #179 (AC WorldPacketActionContext.h): area trigger / instance portal follow.
        creators["reach area trigger"] = &WorldPacketActionContext::reach_area_trigger;
        creators["area trigger"] = &WorldPacketActionContext::area_trigger;

        // #178/#179: UseMeetingStoneAction was ported in #177 but never registered (needed this
        // batch's packet-context headers). SummonAction (the "summon" chat command) already had a
        // registration point of its own in ChatActionContext.h and needs none here.
        creators["use meeting stone"] = &WorldPacketActionContext::use_meeting_stone;

        // #188 (S6 of epic #163): loot rolls (SMSG_LOOT_START_ROLL - see LootRollAction.cpp for why
        // this fork's version is itself the packet handler, unlike AC's tick-based LootRollAction) and
        // auto-equip-on-loot/roll-won (fired from "item push result"/"loot roll won", see
        // WorldPacketHandlerStrategy.cpp).
        creators["loot roll"] = &WorldPacketActionContext::loot_roll;
        creators["equip upgrades packet action"] = &WorldPacketActionContext::equip_upgrades_packet_action;

        // #193 (S7 of epic #163, AC WorldPacketActionContext.h:72-73): SMSG_TRADE_STATUS/
        // SMSG_TRADE_STATUS_EXTENDED (TradeStatusAction/TradeStatusExtendedAction).
        creators["accept trade"] = &WorldPacketActionContext::accept_trade;
        creators["trade status extended"] = &WorldPacketActionContext::trade_status_extended;

        // #210 S10 G1-G3 (AC WorldPacketActionContext.h): LFG role check answer, proposal accept and
        // the master's teleport in/out copy. See LfgActions.cpp for the 5.4.8 packet notes.
        // #142 (AC WorldPacketActionContext.h:111,114): the random-bot self-queue pair.
        creators["lfg join"] = &WorldPacketActionContext::lfg_join;
        creators["lfg leave"] = &WorldPacketActionContext::lfg_leave;
        creators["lfg role check"] = &WorldPacketActionContext::lfg_role_check;
        creators["lfg accept"] = &WorldPacketActionContext::lfg_accept;
        creators["lfg teleport"] = &WorldPacketActionContext::lfg_teleport;
        creators["see spell"] = &WorldPacketActionContext::see_spell;  // #648, AC WorldPacketActionContext.h:116
        // #210 S10 G11 registration (AC WorldPacketActionContext.h:56,145): same GiveLeaderAction
        // "give leader" already uses (ChatActionContext.h), with AC's dungeon-specific message.
        creators["give leader in dungeon"] = &WorldPacketActionContext::give_leader_in_dungeon;

        // #210 S10 G4 (AC WorldPacketActionContext.h:96-97): ready check confirm + report, and the
        // re-confirm on "ready check finished".
        creators["ready check"] = &WorldPacketActionContext::ready_check;
        creators["ready check finished"] = &WorldPacketActionContext::ready_check_finished;
        // #628 (AC WorldPacketActionContext.h:98-99)
        creators["force rebuff"] = &WorldPacketActionContext::force_rebuff;
        creators["ready reply"] = &WorldPacketActionContext::ready_reply;

        // #257 slice H registration (AC WorldPacketActionContext.h:79-91). The master-commanded
        // quest layer; every name below is the exact string AC uses.
        //
        // "quest update complete" (AC:89) binds SMSG_QUEST_UPDATE_COMPLETE and "turn in query quest"
        // (AC:90) answers SMSG_QUESTGIVER_OFFER_REWARD (via the "questgiver quest details" trigger);
        // both exist in 5.4.8 and are registered below.
        creators["talk to quest giver"] = &WorldPacketActionContext::turn_in_quest;
        creators["accept quest"] = &WorldPacketActionContext::accept_quest;
        creators["confirm quest"] = &WorldPacketActionContext::confirm_quest;
        creators["accept all quests"] = &WorldPacketActionContext::accept_all_quests;
        creators["accept quest share"] = &WorldPacketActionContext::accept_quest_share;
        creators["quest update add kill"] = &WorldPacketActionContext::quest_update_add_kill;
        creators["quest update add item"] = &WorldPacketActionContext::quest_update_add_item;
        creators["quest update failed"] = &WorldPacketActionContext::quest_update_failed;
        creators["quest update failed timer"] = &WorldPacketActionContext::quest_update_failed_timer;
        creators["quest item push result"] = &WorldPacketActionContext::quest_item_push_result;
        creators["quest update complete"] = &WorldPacketActionContext::quest_update_complete;  // AC :89
        creators["turn in query quest"] = &WorldPacketActionContext::turn_in_query_quest;      // AC :90

        // #257 slice H: QuestConfirmAcceptAction is a 5.4.8-only class (AC drives the same
        // CMSG_QUEST_CONFIRM_ACCEPT answer through ConfirmQuestAction) and has no AC creator name.
        // Registered under the name its own constructor declares so the class is reachable.
        creators["quest confirm accept"] = &WorldPacketActionContext::quest_confirm_accept;
        creators["tell not enough money"] = &WorldPacketActionContext::tell_not_enough_money;            // #258 AC WorldPacketActionContext.h
        creators["tell not enough reputation"] = &WorldPacketActionContext::tell_not_enough_reputation;
        creators["guild accept"] = &WorldPacketActionContext::guild_accept;
        creators["petition sign"] = &WorldPacketActionContext::petition_sign;
    }

private:
    static Action* accept_invitation(PlayerbotAI* botAI) { return new AcceptInvitationAction(botAI); }
    static Action* uninvite(PlayerbotAI* botAI) { return new UninviteAction(botAI); }
    static Action* party_command(PlayerbotAI* botAI) { return new PartyCommandAction(botAI); }

    static Action* find_corpse(PlayerbotAI* botAI) { return new FindCorpseAction(botAI); }
    static Action* auto_release(PlayerbotAI* botAI) { return new AutoReleaseSpiritAction(botAI); }
    static Action* revive_from_corpse(PlayerbotAI* botAI) { return new ReviveFromCorpseAction(botAI); }
    static Action* spirit_healer(PlayerbotAI* botAI) { return new SpiritHealerAction(botAI); }
    static Action* repop(PlayerbotAI* botAI) { return new RepopAction(botAI); }
    static Action* self_resurrect(PlayerbotAI* botAI) { return new SelfResurrectAction(botAI); }
    static Action* accept_resurrect(PlayerbotAI* botAI) { return new AcceptResurrectAction(botAI); }
    static Action* release(PlayerbotAI* botAI) { return new ReleaseSpiritAction(botAI); }
    static Action* xp_gain(PlayerbotAI* botAI) { return new XpGainAction(botAI); }
    static Action* store_loot(PlayerbotAI* botAI) { return new StoreLootAction(botAI); }

    static Action* remember_taxi(PlayerbotAI* botAI) { return new RememberTaxiAction(botAI); }
    static Action* taxi(PlayerbotAI* botAI) { return new TaxiAction(botAI); }

    static Action* reach_area_trigger(PlayerbotAI* botAI) { return new ReachAreaTriggerAction(botAI); }
    static Action* area_trigger(PlayerbotAI* botAI) { return new AreaTriggerAction(botAI); }

    static Action* use_meeting_stone(PlayerbotAI* botAI) { return new UseMeetingStoneAction(botAI); }

    static Action* loot_roll(PlayerbotAI* botAI) { return new LootRollAction(botAI); }
    static Action* equip_upgrades_packet_action(PlayerbotAI* botAI) { return new EquipUpgradesPacketAction(botAI); }

    static Action* accept_trade(PlayerbotAI* botAI) { return new TradeStatusAction(botAI); }
    static Action* trade_status_extended(PlayerbotAI* botAI) { return new TradeStatusExtendedAction(botAI); }

    static Action* lfg_join(PlayerbotAI* botAI) { return new LfgJoinAction(botAI); }
    static Action* lfg_leave(PlayerbotAI* botAI) { return new LfgLeaveAction(botAI); }
    static Action* lfg_role_check(PlayerbotAI* botAI) { return new LfgRoleCheckAction(botAI); }
    static Action* lfg_accept(PlayerbotAI* botAI) { return new LfgAcceptAction(botAI); }
    static Action* lfg_teleport(PlayerbotAI* botAI) { return new LfgTeleportAction(botAI); }
    static Action* see_spell(PlayerbotAI* botAI) { return new SeeSpellAction(botAI); }  // #648
    static Action* give_leader_in_dungeon(PlayerbotAI* botAI)
    {
        return new GiveLeaderAction(botAI, "I don't know this dungeon, lead the way!");
    }

    static Action* ready_check(PlayerbotAI* botAI) { return new ReadyCheckAction(botAI); }
    static Action* ready_check_finished(PlayerbotAI* botAI) { return new FinishReadyCheckAction(botAI); }
    static Action* force_rebuff(PlayerbotAI* botAI) { return new ForceRebuffAction(botAI); }  // AC :129
    static Action* ready_reply(PlayerbotAI* botAI) { return new ReadyReplyAction(botAI); }  // AC :130

    static Action* turn_in_quest(PlayerbotAI* botAI) { return new TalkToQuestGiverAction(botAI); }
    static Action* accept_quest(PlayerbotAI* botAI) { return new AcceptQuestAction(botAI); }
    static Action* confirm_quest(PlayerbotAI* botAI) { return new ConfirmQuestAction(botAI); }
    static Action* accept_all_quests(PlayerbotAI* botAI) { return new AcceptAllQuestsAction(botAI); }
    static Action* accept_quest_share(PlayerbotAI* botAI) { return new AcceptQuestShareAction(botAI); }
    static Action* quest_update_add_kill(PlayerbotAI* ai) { return new QuestUpdateAddKillAction(ai); }
    static Action* quest_update_add_item(PlayerbotAI* ai) { return new QuestUpdateAddItemAction(ai); }
    static Action* quest_update_complete(PlayerbotAI* botAI) { return new QuestUpdateCompleteAction(botAI); }  // AC :159
    static Action* turn_in_query_quest(PlayerbotAI* botAI) { return new TurnInQueryQuestAction(botAI); }  // AC :167
    static Action* quest_update_failed(PlayerbotAI* ai) { return new QuestUpdateFailedAction(ai); }
    static Action* quest_update_failed_timer(PlayerbotAI* ai) { return new QuestUpdateFailedTimerAction(ai); }
    static Action* quest_item_push_result(PlayerbotAI* ai) { return new QuestItemPushResultAction(ai); }
    static Action* quest_confirm_accept(PlayerbotAI* botAI) { return new QuestConfirmAcceptAction(botAI); }
    static Action* tell_not_enough_money(PlayerbotAI* botAI) { return new TellMasterAction(botAI, "Not enough money"); }
    static Action* tell_not_enough_reputation(PlayerbotAI* botAI) { return new TellMasterAction(botAI, "Not enough reputation"); }
    static Action* guild_accept(PlayerbotAI* botAI) { return new GuildAcceptAction(botAI); }
    static Action* petition_sign(PlayerbotAI* botAI) { return new PetitionSignAction(botAI); }};

#endif
