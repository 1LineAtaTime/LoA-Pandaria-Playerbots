/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time (map 938) strategy "cata-et" - issue #671, epic #373. Pattern: AC Ai/Dungeon/HoS/HoSStrategy.h.

#ifndef PLAYERBOTS_CATAETSTRATEGY_H
#define PLAYERBOTS_CATAETSTRATEGY_H

#include "Strategy.h"

class CataDungeonEndTimeStrategy : public Strategy
{
public:
    CataDungeonEndTimeStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "cata-et"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
