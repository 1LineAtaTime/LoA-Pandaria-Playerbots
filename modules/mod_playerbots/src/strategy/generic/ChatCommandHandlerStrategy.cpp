/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatCommandHandlerStrategy.h"

#include "Playerbots.h"

// Commands where the trigger name differs from the action name.
void ChatCommandHandlerStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("follow",
        NextAction::array(0, new NextAction("follow chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("stay",
        NextAction::array(0, new NextAction("stay chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("move from group",
        NextAction::array(0, new NextAction("move from group chat shortcut", relevance), nullptr)));

    // #177 (group-functionality-parity.md section 3.1a "Movement/state" + "Strategy" rows)
    triggers.push_back(new TriggerNode("flee",
        NextAction::array(0, new NextAction("flee chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("runaway",
        NextAction::array(0, new NextAction("runaway chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("grind",
        NextAction::array(0, new NextAction("grind chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("tank attack",
        NextAction::array(0, new NextAction("tank attack chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("max dps",
        NextAction::array(0, new NextAction("max dps chat shortcut", relevance), nullptr)));
    // 5.4.8: AC's "roll" chat command runs the "dice roll" action; the Monk ability owns the
    // action name "roll" (see ChatActionContext.h).
    triggers.push_back(new TriggerNode("roll",
        NextAction::array(0, new NextAction("dice roll", relevance), nullptr)));
    triggers.push_back(new TriggerNode("attack",
        NextAction::array(0, new NextAction("attack my target", relevance), nullptr)));

    // #258 (AC ChatCommandHandlerStrategy.cpp:46-47 "s"/"b", :58-59 "talk"). AC's "talk" chat
    // command runs GossipHelloAction before the quest-giver action; both names now exist.
    triggers.push_back(new TriggerNode("s",
        NextAction::array(0, new NextAction("sell", relevance), nullptr)));
    triggers.push_back(new TriggerNode("b",
        NextAction::array(0, new NextAction("buy", relevance), nullptr)));
    triggers.push_back(new TriggerNode("r",  // #258 AC :48
        NextAction::array(0, new NextAction("reward", relevance), nullptr)));
    triggers.push_back(new TriggerNode("talk",
        NextAction::array(0, new NextAction("gossip hello", relevance), new NextAction("talk to quest giver", relevance), nullptr)));

    // #193 (S7 of epic #163, AC ChatCommandHandlerStrategy.cpp:44-45): "t"/"nt" both run "trade".
    triggers.push_back(new TriggerNode("t",
        NextAction::array(0, new NextAction("trade", relevance), nullptr)));
    triggers.push_back(new TriggerNode("nt",
        NextAction::array(0, new NextAction("trade", relevance), nullptr)));

    // #163 S8 (AC parity, ChatCommandHandlerStrategy.cpp:80).
    triggers.push_back(new TriggerNode("focus heal",
        NextAction::array(0, new NextAction("focus heal targets", relevance), nullptr)));

    // #210 S10 G4 (AC ChatCommandHandlerStrategy.cpp:73): "ready" runs "ready check" directly (name
    // differs from the trigger, so it needs its own node instead of `supported`).
    triggers.push_back(new TriggerNode("ready",
        NextAction::array(0, new NextAction("ready check", relevance), nullptr)));

    // #146 (AC ChatCommandHandlerStrategy.cpp:32-34): "q" runs "query quest" then "query item usage". The
    // item-usage query action is not ported on this fork (no "query item usage" in any action context), so
    // only the quest half is wired; add the second NextAction when that action lands.
    triggers.push_back(new TriggerNode("q",
        NextAction::array(0, new NextAction("query quest", relevance), nullptr)));

    // #628 (AC ChatCommandHandlerStrategy.cpp:74): "rebuff" opens a force-rebuff pass.
    triggers.push_back(new TriggerNode("rebuff",
        NextAction::array(0, new NextAction("force rebuff", relevance), nullptr)));

    // #246 (AC ChatCommandHandlerStrategy.cpp:78): "disperse enable|disable|increase|decrease|set <d>" -> "disperse set".
    triggers.push_back(new TriggerNode("disperse",
        NextAction::array(0, new NextAction("disperse set", relevance), nullptr)));

    // #379 (AC ChatCommandHandlerStrategy.cpp:70-72): "pull" / "pull back" request a pull of the master's
    // target, "pull rti" of the raid-icon target. PullRequestAction (PullActions.cpp) then arms the
    // tank's "pull" strategy (PullStrategy.cpp "pull start" -> "pull action" -> "pull end").
    triggers.push_back(new TriggerNode("pull",
        NextAction::array(0, new NextAction("pull my target", relevance), nullptr)));
    triggers.push_back(new TriggerNode("pull back",
        NextAction::array(0, new NextAction("pull my target", relevance), nullptr)));
    triggers.push_back(new TriggerNode("pull rti",
        NextAction::array(0, new NextAction("pull rti target", relevance), nullptr)));
}

// Commands where the trigger name == the action name.
ChatCommandHandlerStrategy::ChatCommandHandlerStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI)
{
    supported.push_back("who");
    supported.push_back("help");
    supported.push_back("co");
    supported.push_back("nc");
    supported.push_back("de");
    supported.push_back("rti");

    // #177
    supported.push_back("save mana");
    supported.push_back("formation");
    supported.push_back("range");
    supported.push_back("stance");
    supported.push_back("position");
    supported.push_back("summon");
    supported.push_back("reset botAI");
    supported.push_back("invite");
    supported.push_back("leave");
    supported.push_back("join");
    supported.push_back("give leader");

    // #188 (S6 of epic #163): loot/gear chat commands (ChatActionContext.h:117 in AC for "ll";
    // RepairAllAction/EquipAction/UnequipAction/RollAction/EquipUpgradeAction are the same
    // trigger-name-equals-action-name shape as the rest of this list).
    supported.push_back("repair");
    supported.push_back("ll");
    supported.push_back("equip");
    supported.push_back("unequip");
    supported.push_back("equip upgrade");

    // #193 (S7 of epic #163, AC ChatCommandHandlerStrategy.cpp constructor): the rogue
    // "unlock traded item" helper is also directly typeable, matching AC.
    supported.push_back("unlock traded item");

    // #194 (S8): "pet"/"pet attack" chat commands (AC ChatCommandHandlerStrategy.cpp:175-176).
    supported.push_back("pet");
    supported.push_back("pet attack");

    // #258 (AC ChatCommandHandlerStrategy.cpp:112,125-126,144-145): trigger name == action name,
    // so PassTroughStrategy::InitTriggers wires them one-for-one like the rest of the list.
    supported.push_back("trainer");
    supported.push_back("gb");
    supported.push_back("bank");
    supported.push_back("sendmail");
    supported.push_back("mail");

    // #363 follow-up: AC Ai/Base/Strategy/ChatCommandHandlerStrategy.cpp:137-143. The only entry
    // point for the Druid "cancel <form>" actions (PR #370) - AC gives them no TriggerNode anywhere.
    supported.push_back("cancel tree form");
    supported.push_back("cancel travel form");
    supported.push_back("cancel bear form");
    supported.push_back("cancel dire bear form");
    supported.push_back("cancel cat form");
    supported.push_back("cancel moonkin form");
    supported.push_back("cancel aquatic form");
    supported.push_back("debug");   // #611 AC ChatCommandHandlerStrategy.cpp:148
    supported.push_back("cdebug");  // #611 AC :149
    supported.push_back("taxi");          // #767 AC ChatCommandHandlerStrategy.cpp:104
    // #146 (AC ChatCommandHandlerStrategy.cpp:88,98,99): quest chat commands, trigger name == action name.
    supported.push_back("quests");
    supported.push_back("drop");
    supported.push_back("share");
    supported.push_back("rpg status");    // #536 AC ChatCommandHandlerStrategy.cpp:95
    supported.push_back("rpg do quest");  // #536 AC :96
    supported.push_back("rtsc");  // #648 AC ChatCommandHandlerStrategy.cpp:164
    supported.push_back("hire");  // #258 AC :152
}
