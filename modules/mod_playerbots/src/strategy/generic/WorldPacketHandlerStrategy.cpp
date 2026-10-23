/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WorldPacketHandlerStrategy.h"

#include "Playerbots.h"

void WorldPacketHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    // AC WorldPacketHandlerStrategy.cpp:71-72: vote on pending group loot rolls every few ticks.
    triggers.push_back(new TriggerNode("very often", NextAction::array(0, new NextAction("loot roll", relevance), nullptr)));

    triggers.push_back(new TriggerNode("levelup", NextAction::array(0, new NextAction("auto maintenance on levelup", relevance + 3), nullptr)));

    // party handlers
    triggers.push_back(new TriggerNode("group invite", NextAction::array(0, new NextAction("accept invitation", relevance), nullptr)));
    // #258 (AC WorldPacketHandlerStrategy.cpp:22-25, :49-50). AC binds the two "not enough" packets to BUY_ERR_* enum values used as
    // opcode numbers (AC PlayerbotAI.cpp:302-303), so those nodes never fire on AC; kept dead the same way here (no handler).
    triggers.push_back(new TriggerNode("not enough money", NextAction::array(0, new NextAction("tell not enough money", relevance), nullptr)));
    triggers.push_back(new TriggerNode("not enough reputation", NextAction::array(0, new NextAction("tell not enough reputation", relevance), nullptr)));
    triggers.push_back(new TriggerNode("guild invite", NextAction::array(0, new NextAction("guild accept", relevance), nullptr)));
    triggers.push_back(new TriggerNode("petition offer", NextAction::array(0, new NextAction("petition sign", relevance), nullptr)));
    triggers.push_back(new TriggerNode("uninvite guid", NextAction::array(0, new NextAction("uninvite", relevance), nullptr)));
    triggers.push_back(new TriggerNode("uninvite", NextAction::array(0, new NextAction("uninvite", relevance), nullptr)));

    // #648 (AC WorldPacketHandlerStrategy.cpp:62): the master's CMSG_CAST_SPELL -> SeeSpellAction (RTSC, fish with master).
    triggers.push_back(new TriggerNode("see spell", NextAction::array(0, new NextAction("see spell", relevance), nullptr)));
    triggers.push_back(new TriggerNode("release spirit", NextAction::array(0, new NextAction("release", relevance), nullptr)));
    triggers.push_back(new TriggerNode("revive from corpse", NextAction::array(0, new NextAction("revive from corpse", relevance), nullptr)));

    // D4 (death-revive-parity.md F4): AC XpGainAction.cpp:16 resets "death count" to 0 on XP gain.
    triggers.push_back(new TriggerNode("bg status", NextAction::array(0, new NextAction("bg status", relevance), nullptr)));
    triggers.push_back(new TriggerNode("xpgain", NextAction::array(0, new NextAction("xp gain", relevance), nullptr)));

    // #151 (AC WorldPacketHandlerStrategy.cpp:40): SMSG_LOOT_RESPONSE -> "store loot".
    triggers.push_back(new TriggerNode("loot response", NextAction::array(0, new NextAction("store loot", relevance), nullptr)));

    // #179 (AC WorldPacketHandlerStrategy.cpp:30): a meeting-stone GameObject click summoning the bot.
    // #535 (AC WorldPacketHandlerStrategy.cpp:29-30): "add loot" comes first, then the meeting stone.
    triggers.push_back(new TriggerNode("use game object", NextAction::array(0, new NextAction("add loot", relevance), new NextAction("use meeting stone", relevance), nullptr)));

    // #258 (AC WorldPacketHandlerStrategy.cpp:32): the master's CMSG_GOSSIP_HELLO ->
    // "gossip hello" WorldPacketTrigger (already registered in WorldPacketTriggerContext.h,
    // AI/PlayerbotAI.cpp:235) -> TrainerAction, which trains the bot at the NPC being greeted.
    // AC's "often" -> "check mail" node (AC WorldPacketHandlerStrategy.cpp:48) is commented out
    // on AC itself, so no node is added for "check mail" - the action is name-parity only.
    triggers.push_back(new TriggerNode("gossip hello", NextAction::array(0, new NextAction("trainer", relevance), nullptr)));

    // #179 (AC WorldPacketHandlerStrategy.cpp:33-35): taxi with the master. "remember taxi" and
    // "taxi" both fire from the same trigger, same relevance, so they run on consecutive AI ticks
    // (Engine::DoNextAction stops at the first successful action) - exactly the AC-observed pacing.
    triggers.push_back(new TriggerNode("activate taxi", NextAction::array(0, new NextAction("remember taxi", relevance), new NextAction("taxi", relevance), nullptr)));

    // #179 (AC WorldPacketHandlerStrategy.cpp:38-39): area trigger / instance portal follow.
    triggers.push_back(new TriggerNode("area trigger", NextAction::array(0, new NextAction("reach area trigger", relevance), nullptr)));
    triggers.push_back(new TriggerNode("within area trigger", NextAction::array(0, new NextAction("area trigger", relevance), nullptr)));

    // #188 (S6 of epic #163, AC WorldPacketHandlerStrategy.cpp:41-46,72): auto-equip an upgrade when
    // loot is pushed into the bot's bags or a master-loot roll is won. "loot roll" itself is a 1:1
    // packet->action passthrough (see the constructor's `supported` list) since LootRollAction is
    // its own packet handler here.
    // #535 (AC :41-44): "unlock items", "open items" first; AC's "query item usage" is omitted (not
    // ported - needs GetSellMultiplier/GetBuyMultiplier and the full InventoryAction on this fork).
    triggers.push_back(new TriggerNode("item push result", NextAction::array(0, new NextAction("unlock items", relevance), new NextAction("open items", relevance), new NextAction("equip upgrades packet action", relevance), nullptr)));
    triggers.push_back(new TriggerNode("loot roll won", NextAction::array(0, new NextAction("equip upgrades packet action", relevance), nullptr)));

    // #193 (S7 of epic #163, AC WorldPacketHandlerStrategy.cpp:36-37): SMSG_TRADE_STATUS also
    // re-checks for a gear upgrade (a completed trade can hand the bot an item), same as loot.
    triggers.push_back(new TriggerNode("trade status", NextAction::array(0, new NextAction("accept trade", relevance), new NextAction("equip upgrades packet action", relevance), nullptr)));
    triggers.push_back(new TriggerNode("trade status extended", NextAction::array(0, new NextAction("trade status extended", relevance), nullptr)));

    // #210 S10 G2 (AC WorldPacketHandlerStrategy.cpp:51-52): the proposal packet's name ("lfg
    // proposal") differs from the action that answers it ("lfg accept"), so this needs its own node -
    // "lfg role check"/"lfg teleport" are same-name pass-throughs via `supported` below.
    triggers.push_back(new TriggerNode("lfg proposal", NextAction::array(0, new NextAction("lfg accept", relevance), nullptr)));

    // #210 S10 G4 (AC WorldPacketHandlerStrategy.cpp:47): matches AC's own explicit node for this one
    // (kept separate from `supported` even though the names match, same as upstream).
    triggers.push_back(new TriggerNode("ready check finished", NextAction::array(0, new NextAction("ready check finished", relevance), nullptr)));

    // #257 slice H (AC WorldPacketHandlerStrategy.cpp:45): QuestItemPushResultAction runs off the
    // item-push packet. AC declares this as a second, separate TriggerNode on the same trigger
    // (its first one, "unlock items"/"open items"/"query item usage", is not ported here).
    triggers.push_back(new TriggerNode("item push result", NextAction::array(0, new NextAction("quest item push result", relevance), nullptr)));

    // #257 (AC WorldPacketHandlerStrategy.cpp:69): the master's SMSG_QUESTGIVER_OFFER_REWARD (the turn-in
    // dialog) -> "questgiver quest details" -> TurnInQueryQuestAction hands the same quest in for the bot.
    triggers.push_back(new TriggerNode("questgiver quest details", NextAction::array(0, new NextAction("turn in query quest", relevance), nullptr)));
}

WorldPacketHandlerStrategy::WorldPacketHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    supported.push_back("uninvite");
    supported.push_back("bg status");

    // #179 (AC WorldPacketHandlerStrategy.cpp constructor): a 1:1 WorldPacketTrigger -> same-named
    // Action, via PassTroughStrategy::InitTriggers looping `supported`.
    supported.push_back("check mount state");
    supported.push_back("party command");

    // #188 (S6 of epic #163): as AC (WorldPacketHandlerStrategy.cpp:72,77), LootRollAction runs on the
    // "very often" tick and votes on every pending group roll; the "loot roll" packet trigger
    // (SMSG_LOOT_START_ROLL) just makes the first vote immediate.
    supported.push_back("loot roll");

    // #210 S10 G1/G3 (AC WorldPacketHandlerStrategy.cpp constructor): same-name packet -> action
    // pass-throughs for the LFG role check answer and the master's teleport in/out copy.
    supported.push_back("lfg role check");
    supported.push_back("lfg teleport");

    // #210 S10 G4 (AC WorldPacketHandlerStrategy.cpp constructor): same-name pass-through for the
    // ready-check confirm ("ready check finished" is a separate, explicit node above - AC parity).
    supported.push_back("ready check");

    // #257 slice H (AC WorldPacketHandlerStrategy.cpp:89,91,92,94): same-name packet -> action
    // pass-throughs for the bot's own quest-progress packets.
    supported.push_back("quest update add kill");
    supported.push_back("quest update failed");
    supported.push_back("quest update failed timer");
    supported.push_back("quest update complete");  // AC :93
    supported.push_back("confirm quest");
}
