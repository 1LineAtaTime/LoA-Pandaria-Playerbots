/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/LfgStrategy.cpp) into the
 * Legends of Azeroth Pandaria fork (#210 S10 G11 registration). See LfgStrategy.h for what is/isn't
 * ported in this batch.
 */

#include "LfgStrategy.h"

#include "Playerbots.h"

void LfgStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #142 (AC LfgStrategy.cpp:11-13): "random" -> "lfg join", "seldom" -> "lfg leave".
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("lfg join", relevance), nullptr)));
    triggers.push_back(new TriggerNode("seldom", NextAction::array(0, new NextAction("lfg leave", relevance), nullptr)));
    triggers.push_back(new TriggerNode(
        "unknown dungeon", NextAction::array(0, new NextAction("give leader in dungeon", relevance), nullptr)));
}

LfgStrategy::LfgStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}
