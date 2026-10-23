/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSStrategy.h"
#include "CoSMultipliers.h"

// AC Ai/Dungeon/CoS/CoSStrategy.cpp:10
void WotlkDungeonCoSStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Meathook
    // Can tank this in a fixed position to allow healer to LoS the stun, probably not necessary

    // Salramm the Fleshcrafter
    triggers.push_back(new TriggerNode("explode ghoul",
        NextAction::array(0, new NextAction("explode ghoul spread", ACTION_MOVE + 5), nullptr)));

    // Chrono-Lord Epoch
    // Not sure if this actually works, I think I've seen him charge melee characters..?
    triggers.push_back(new TriggerNode("epoch ranged",
        NextAction::array(0, new NextAction("epoch stack", ACTION_MOVE + 5), nullptr)));

    // Mal'Ganis

    // Infinite Corruptor (Heroic only)
}

// AC Ai/Dungeon/CoS/CoSStrategy.cpp:29
void WotlkDungeonCoSStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new EpochMultiplier(botAI));
}
