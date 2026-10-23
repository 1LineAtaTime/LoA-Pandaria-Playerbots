/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Strategy/LootNonCombatStrategy.cpp:10-16 (issue #151). AC's
 * 4th line ("timer" -> "add gathering loot" 5.0, GatherStrategy::InitTriggers) is in
 * NonCombatStrategy.cpp (#156) - see the class comment in LootNonCombatStrategy.h.
 */

#include "LootNonCombatStrategy.h"

#include "Playerbots.h"

void LootNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("loot available", NextAction::array(0, new NextAction("loot", 6.0f), nullptr)));
    triggers.push_back(new TriggerNode("far from loot target", NextAction::array(0, new NextAction("move to loot", 7.0f), nullptr)));
    triggers.push_back(new TriggerNode("can loot", NextAction::array(0, new NextAction("open loot", 8.0f), nullptr)));
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("add all loot", 5.0f), nullptr)));
}

// AC LootNonCombatStrategy.cpp:25-29 (issue #377).
void RevealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("reveal gathering item", 50.0f), nullptr)));
}

// AC LootNonCombatStrategy.cpp:31-36 (issue #412).
void UseBobberStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("can use fishing bobber", NextAction::array(0, new NextAction("use fishing bobber", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("remove bobber strategy", 20.0f), nullptr)));
}

// AC NonCombatStrategy.cpp:37-67 (issue #412). "very often" is the fork's trigger of that name (TriggerContext.h).
void MasterFishingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("very often", NextAction::array(0, new NextAction("move near water", 10.0f), nullptr)));
    triggers.push_back(new TriggerNode("very often", NextAction::array(0, new NextAction("go fishing", 10.0f), nullptr)));
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("end master fishing", 12.0f), new NextAction("equip upgrades packet action", 6.0f), nullptr)));
}
