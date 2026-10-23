/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSStrategy.h"
#include "PoSMultipliers.h"

// AC Ai/Dungeon/PoS/PoSStrategy.cpp:10
void WotlkDungeonPoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ick and krick",
        NextAction::array(0, new NextAction("ick and krick", ACTION_RAID + 5), nullptr)));

    triggers.push_back(new TriggerNode("tyrannus",
        NextAction::array(0, new NextAction("tyrannus", ACTION_RAID + 5), nullptr)));
}

// AC Ai/Dungeon/PoS/PoSStrategy.cpp:19
void WotlkDungeonPoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new IckAndKrickMultiplier(botAI));
}
