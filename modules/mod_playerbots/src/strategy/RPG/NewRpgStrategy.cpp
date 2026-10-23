/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Strategy/NewRpgStrategy.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "NewRpgStrategy.h"

NewRpgStrategy::NewRpgStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

NextAction** NewRpgStrategy::getDefaultActions()
{
    // the relevance should be greater than grind/default actions
    return NextAction::array(0, new NextAction("new rpg status update", 11.0f), nullptr);
}

void NewRpgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("go grind status",
            NextAction::array(0, new NextAction("new rpg go grind", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("go camp status",
            NextAction::array(0, new NextAction("new rpg go camp", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("wander random status",
            NextAction::array(0, new NextAction("new rpg wander random", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("wander npc status",
            NextAction::array(0, new NextAction("new rpg wander npc", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("do quest status",
            NextAction::array(0, new NextAction("new rpg do quest", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("travel flight status",
            NextAction::array(0, new NextAction("new rpg travel flight", 3.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("outdoor pvp status",
            NextAction::array(0, new NextAction("new rpg outdoor pvp", 3.0f), nullptr)));

    // #201 (#612): AC NewRpgStrategy.cpp:70-75 (outdoor pvp node) pattern.
    triggers.push_back(
        new TriggerNode("duel spot status",
            NextAction::array(0, new NextAction("new rpg duel spot", 3.0f), nullptr)));
}

void NewRpgStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}
