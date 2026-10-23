/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/TravelStrategy.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #269.
 */

#include "TravelStrategy.h"
#include "Playerbots.h"

TravelStrategy::TravelStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

// 5.4.8 divergence (AC TravelStrategy.cpp:13-18): old NextAction::array API, same action and relevance.
NextAction** TravelStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("travel", 1.0f), nullptr);
}

// 5.4.8 divergence (AC TravelStrategy.cpp:20-36): old NextAction::array API; names and relevances
// (6.f, 1) identical to AC.
void TravelStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("no travel target", NextAction::array(0, new NextAction("choose travel target", 6.f), nullptr)));
    triggers.push_back(
        new TriggerNode("far from travel target", NextAction::array(0, new NextAction("move to travel target", 1), nullptr)));
}
