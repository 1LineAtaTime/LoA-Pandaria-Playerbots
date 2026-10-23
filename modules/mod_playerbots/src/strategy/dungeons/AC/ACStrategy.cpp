/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/AC/ACStrategy.cpp (#564, epic #569)

#include "ACStrategy.h"
#include "ACMultipliers.h"
#include "ACTriggers.h"
#include "Playerbots.h"

void TbcDungeonAuchenaiCryptsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Shirrak The Dead Watcher
    triggers.push_back(new TriggerNode("shirrak tank position boss",
        NextAction::array(0, new NextAction("shirrak tank position boss", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("shirrak flee focus fire",
        NextAction::array(0, new NextAction("shirrak flee focus fire", ACTION_EMERGENCY + 10), nullptr)));

    triggers.push_back(new TriggerNode("shirrak ranged keep distance",
        NextAction::array(0, new NextAction("shirrak ranged keep distance", ACTION_RAID), nullptr)));
}

void TbcDungeonAuchenaiCryptsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new ShirrakFleeFocusFireMultiplier(botAI));
}
