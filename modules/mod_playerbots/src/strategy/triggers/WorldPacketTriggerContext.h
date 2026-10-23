/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WORLDPACKETTRIGGERCONTEXT_H
#define _PLAYERBOT_WORLDPACKETTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "WorldPacketTrigger.h"
#include "WithinAreaTrigger.h"

class WorldPacketTriggerContext : public NamedObjectContext<Trigger>
{
public:
    WorldPacketTriggerContext()
    {
        creators["group invite"] = &WorldPacketTriggerContext::group_invite;
        creators["uninvite"] = &WorldPacketTriggerContext::uninvite;
        creators["uninvite guid"] = &WorldPacketTriggerContext::uninvite_guid;
        creators["group destroyed"] = &WorldPacketTriggerContext::group_destroyed;

        // #178/#179 (AC WorldPacketTriggerContext.h): master-leaves-party (PartyCommandAction) and
        // the mount-matching re-check on the master's speed change (CheckMountStateAction, #134).
        creators["party command"] = &WorldPacketTriggerContext::party_command;
        creators["check mount state"] = &WorldPacketTriggerContext::check_mount_state;

        // #179 (AC WorldPacketTriggerContext.h): taxi with the master (5.4.8 CMSG_ACTIVATE_TAXI(_EXPRESS)
        // parse in RememberTaxiAction.cpp) and area trigger / instance portal follow.
        creators["activate taxi"] = &WorldPacketTriggerContext::activate_taxi;
        creators["area trigger"] = &WorldPacketTriggerContext::area_trigger;
        creators["within area trigger"] = &WorldPacketTriggerContext::within_area_trigger;

        creators["release spirit"] = &WorldPacketTriggerContext::release_spirit;
        creators["revive from corpse"] = &WorldPacketTriggerContext::revive_from_corpse;

        creators["levelup"] = &WorldPacketTriggerContext::levelup;

        // #149 (AC WorldPacketTriggerContext.h:28): fires on the incoming SMSG_RESURRECT_REQUEST so
        // DeadStrategy can schedule "accept resurrect".
        creators["resurrect request"] = &WorldPacketTriggerContext::resurrect_request;

        // #163 S8 (AC parity, DuelStrategy.cpp): fires on the incoming SMSG_DUEL_REQUESTED so
        // DuelStrategy can schedule "accept duel".
        creators["duel requested"] = &WorldPacketTriggerContext::duel_requested;
        // #163 S8 (AC parity): the receiving side of EmoteStrategy.
        creators["receive text emote"] = &WorldPacketTriggerContext::receive_text_emote;
        creators["receive emote"] = &WorldPacketTriggerContext::receive_emote;

        // D4 (death-revive-parity.md F4): fires on SMSG_LOG_XPGAIN so WorldPacketHandlerStrategy can
        // schedule "xp gain" (AC XpGainAction.cpp:16 resets "death count" to 0).
        creators["xpgain"] = &WorldPacketTriggerContext::xpgain;

        // #151 (AC WorldPacketTriggerContext.h:35,113): fires on SMSG_LOOT_RESPONSE so
        // WorldPacketHandlerStrategy can schedule "store loot" (AI/PlayerbotAI.cpp's handler
        // registration, WorldPacketHandlerStrategy.cpp).
        creators["loot response"] = &WorldPacketTriggerContext::loot_response;
        creators["bg status"] = &WorldPacketTriggerContext::bg_status;

        // #188 (S6 of epic #163): loot rolls (SMSG_LOOT_START_ROLL, registered as "loot roll" - see
        // LootRollAction.cpp) and the two auto-equip-on-loot triggers (SMSG_ITEM_PUSH_RESULT,
        // SMSG_LOOT_ROLL_WON).
        creators["loot roll"] = &WorldPacketTriggerContext::loot_roll;
        creators["item push result"] = &WorldPacketTriggerContext::item_push_result;
        creators["loot roll won"] = &WorldPacketTriggerContext::loot_roll_won;

        // #193 (S7 of epic #163, AC WorldPacketTriggerContext.h:33-34): SMSG_TRADE_STATUS/
        // SMSG_TRADE_STATUS_EXTENDED, registered as "trade status"/"trade status extended"
        // (see strategy/generic/WorldPacketHandlerStrategy.cpp for the trigger -> action wiring).
        creators["trade status"] = &WorldPacketTriggerContext::trade_status;
        creators["trade status extended"] = &WorldPacketTriggerContext::trade_status_extended;

        // #210 S10 G1-G3 (AC WorldPacketTriggerContext.h): SMSG_LFG_ROLE_CHECK_UPDATE / _PROPOSAL_UPDATE
        // (botOutgoing) and CMSG_LFG_TELEPORT (masterIncoming) - see AI/PlayerbotAI.cpp for the opcode
        // registration and LfgActions.cpp for the 5.4.8 packet notes.
        creators["lfg role check"] = &WorldPacketTriggerContext::lfg_role_check;
        creators["lfg proposal"] = &WorldPacketTriggerContext::lfg_proposal;
        creators["lfg teleport"] = &WorldPacketTriggerContext::lfg_teleport;
        // #648 (AC WorldPacketTriggerContext.h:71): the master's CMSG_CAST_SPELL (AI/PlayerbotAI.cpp masterIncoming).
        creators["see spell"] = &WorldPacketTriggerContext::seespell;

        // #210 S10 G4 (AC WorldPacketTriggerContext.h): SMSG_RAID_READY_CHECK (masterOutgoing) and
        // SMSG_RAID_READY_CHECK_COMPLETED (masterOutgoing) - AI/PlayerbotAI.cpp has the opcode
        // registration, ReadyCheckAction.cpp the 5.4.8 packet notes.
        creators["ready check"] = &WorldPacketTriggerContext::ready_check;
        creators["ready check finished"] = &WorldPacketTriggerContext::ready_check_finished;

        // #257 slice H registration (AC WorldPacketTriggerContext.h:38-48): the quest packet
        // triggers. AC registers every one of these as a plain WorldPacketTrigger - there is no
        // AC QuestTriggers.h/.cpp file, so none is ported here either.
        // "quest update complete" (AC:47) binds SMSG_QUEST_UPDATE_COMPLETE (AI/PlayerbotAI.cpp).
        creators["quest update complete"] = &WorldPacketTriggerContext::quest_update_complete;
        creators["complete quest"] = &WorldPacketTriggerContext::complete_quest;
        creators["accept quest"] = &WorldPacketTriggerContext::accept_quest;
        creators["confirm quest"] = &WorldPacketTriggerContext::quest_confirm_accept;
        creators["quest share"] = &WorldPacketTriggerContext::quest_share;
        creators["quest update add kill"] = &WorldPacketTriggerContext::quest_update_add_kill;
        creators["quest update add item"] = &WorldPacketTriggerContext::quest_update_add_item;
        creators["quest update failed"] = &WorldPacketTriggerContext::quest_update_failed;
        creators["quest update failed timer"] = &WorldPacketTriggerContext::quest_update_failed_timer;
        creators["questgiver quest details"] = &WorldPacketTriggerContext::questgiver_quest_details;

        // #257 slice H (AC WorldPacketTriggerContext.h:20,26): AI/PlayerbotAI.cpp:165,235 already
        // route CMSG_GAMEOBJ_USE and CMSG_GOSSIP_HELLO under these two names, but neither trigger
        // had a registration point, so both handlers were dead. DefaultQuestStrategy and
        // AcceptAllQuestsStrategy hang "talk to quest giver" off exactly these two triggers.
        creators["gossip hello"] = &WorldPacketTriggerContext::gossip_hello;
        creators["use game object"] = &WorldPacketTriggerContext::use_game_object;
        creators["not enough money"] = &WorldPacketTriggerContext::not_enough_money;            // #258 AC WorldPacketTriggerContext.h
        creators["not enough reputation"] = &WorldPacketTriggerContext::not_enough_reputation;
        creators["guild invite"] = &WorldPacketTriggerContext::guild_invite;
        creators["petition offer"] = &WorldPacketTriggerContext::petition_offer;
    }

private:
    static Trigger* group_invite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group invite"); }
    static Trigger* uninvite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "uninvite"); }
    static Trigger* uninvite_guid(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "uninvite guid"); }
    static Trigger* group_destroyed(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "group destroyed"); }

    static Trigger* party_command(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "party command"); }
    static Trigger* check_mount_state(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "check mount state"); }

    static Trigger* activate_taxi(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "activate taxi"); }
    static Trigger* area_trigger(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "area trigger"); }
    static Trigger* within_area_trigger(PlayerbotAI* botAI) { return new WithinAreaTrigger(botAI); }

    static Trigger* release_spirit(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "release spirit"); }
    static Trigger* revive_from_corpse(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "revive from corpse"); }

    static Trigger* levelup(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "levelup"); }
    static Trigger* resurrect_request(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "resurrect request"); }
    static Trigger* duel_requested(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "duel requested"); }
    static Trigger* receive_text_emote(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "receive text emote"); }
    static Trigger* receive_emote(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "receive emote"); }
    static Trigger* xpgain(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "xpgain"); }
    static Trigger* loot_response(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "loot response"); }
    static Trigger* bg_status(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "bg status"); }

    static Trigger* loot_roll(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "loot roll"); }
    static Trigger* item_push_result(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "item push result"); }
    static Trigger* loot_roll_won(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "loot roll won"); }

    static Trigger* trade_status(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "trade status"); }
    static Trigger* trade_status_extended(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "trade status extended"); }

    static Trigger* lfg_role_check(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg role check"); }
    static Trigger* lfg_proposal(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg proposal"); }
    static Trigger* lfg_teleport(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "lfg teleport"); }
    static Trigger* seespell(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "see spell"); }  // #648, AC :133

    static Trigger* ready_check(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "ready check"); }
    static Trigger* ready_check_finished(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "ready check finished"); }

    static Trigger* quest_update_add_kill(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add kill"); }
    static Trigger* quest_update_add_item(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update add item"); }
    static Trigger* quest_update_failed(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed"); }
    static Trigger* quest_update_failed_timer(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update failed timer"); }
    static Trigger* quest_update_complete(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest update complete"); }  // AC :105
    static Trigger* complete_quest(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "complete quest"); }
    static Trigger* accept_quest(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "accept quest"); }
    static Trigger* quest_confirm_accept(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "confirm quest"); }
    static Trigger* quest_share(PlayerbotAI* ai) { return new WorldPacketTrigger(ai, "quest share"); }
    static Trigger* questgiver_quest_details(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "questgiver quest details"); }

    static Trigger* gossip_hello(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "gossip hello"); }
    static Trigger* use_game_object(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "use game object"); }
    static Trigger* not_enough_money(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "not enough money"); }
    static Trigger* not_enough_reputation(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "not enough reputation"); }
    static Trigger* guild_invite(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "guild invite"); }
    static Trigger* petition_offer(PlayerbotAI* botAI) { return new WorldPacketTrigger(botAI, "petition offer"); }
};

#endif
