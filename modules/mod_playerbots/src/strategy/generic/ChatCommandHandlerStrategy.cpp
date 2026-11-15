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

    // AC ChatCommandHandlerStrategy.cpp:32-34: "q" runs "query quest" then "query item usage".
    triggers.push_back(new TriggerNode("q",
        NextAction::array(0, new NextAction("query quest", relevance), new NextAction("query item usage", relevance), nullptr)));

    // AC ChatCommandHandlerStrategy.cpp:79: "qi" runs only the item-usage query.
    triggers.push_back(new TriggerNode("qi",
        NextAction::array(0, new NextAction("query item usage", relevance), nullptr)));

    // AC ChatCommandHandlerStrategy.cpp:30,35-43,50,60-64,66,68-69: commands whose trigger name differs from the action
    // name. The actions already exist in the action contexts; these nodes are what makes them reachable from chat.
    triggers.push_back(new TriggerNode("rep",
        NextAction::array(0, new NextAction("reputation", relevance), nullptr)));
    triggers.push_back(new TriggerNode("add all loot",
        NextAction::array(0, new NextAction("add all loot", relevance), new NextAction("loot", relevance), nullptr)));
    triggers.push_back(new TriggerNode("u",
        NextAction::array(0, new NextAction("use", relevance), nullptr)));
    triggers.push_back(new TriggerNode("c",
        NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    triggers.push_back(new TriggerNode("items",
        NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    triggers.push_back(new TriggerNode("inv",
        NextAction::array(0, new NextAction("item count", relevance), nullptr)));
    // AC maps "e"/"ue" to "equip"/"unequip" (AC :42-43). This fork also keeps the full words in `supported`.
    triggers.push_back(new TriggerNode("e",
        NextAction::array(0, new NextAction("equip", relevance), nullptr)));
    triggers.push_back(new TriggerNode("ue",
        NextAction::array(0, new NextAction("unequip", relevance), nullptr)));
    triggers.push_back(new TriggerNode("accept",
        NextAction::array(0, new NextAction("accept quest", relevance), nullptr)));
    triggers.push_back(new TriggerNode("enter vehicle",
        NextAction::array(0, new NextAction("enter vehicle", relevance), nullptr)));
    triggers.push_back(new TriggerNode("leave vehicle",
        NextAction::array(0, new NextAction("leave vehicle", relevance), nullptr)));
    triggers.push_back(new TriggerNode("cast",
        NextAction::array(0, new NextAction("cast custom spell", relevance), nullptr)));
    triggers.push_back(new TriggerNode("castnc",
        NextAction::array(0, new NextAction("cast custom nc spell", relevance), nullptr)));
    triggers.push_back(new TriggerNode("revive",
        NextAction::array(0, new NextAction("spirit healer", relevance), nullptr)));
    // AC :66: "warning" (queued by PlayerbotAI::HandleCommand for a raid warning) runs the runaway shortcut.
    triggers.push_back(new TriggerNode("warning",
        NextAction::array(0, new NextAction("runaway chat shortcut", relevance), nullptr)));
    triggers.push_back(new TriggerNode("attackers",
        NextAction::array(0, new NextAction("tell attackers", relevance), nullptr)));
    triggers.push_back(new TriggerNode("target",
        NextAction::array(0, new NextAction("tell target", relevance), nullptr)));

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
    supported.push_back("talents");  // AC ChatCommandHandlerStrategy.cpp:107
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
    supported.push_back("lfg");  // AC ChatCommandHandlerStrategy.cpp:128
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
    // AC ChatCommandHandlerStrategy.cpp:159-163: guild management chat commands.
    supported.push_back("ginvite");
    supported.push_back("guild promote");
    supported.push_back("guild demote");
    supported.push_back("guild remove");
    supported.push_back("guild leave");
    supported.push_back("rtsc");  // #648 AC ChatCommandHandlerStrategy.cpp:164
    supported.push_back("hire");  // #258 AC :152

    // AC ChatCommandHandlerStrategy.cpp:89,91,102,108,119,122,147,165,168-169,177: trigger name == action name.
    supported.push_back("stats");
    supported.push_back("reputation");
    supported.push_back("release");
    supported.push_back("spells");
    supported.push_back("home");
    supported.push_back("emote");
    supported.push_back("go");
    supported.push_back("drink");
    supported.push_back("open items");
    supported.push_back("unlock items");
    supported.push_back("wait for attack time");
    // AC ChatCommandHandlerStrategy.cpp:113-116 (TrainerAction.h): maintenance, remove glyph, autogear, autogear bis.
    supported.push_back("maintenance");
    supported.push_back("remove glyph");
    supported.push_back("autogear");
    supported.push_back("autogear bis");
}
