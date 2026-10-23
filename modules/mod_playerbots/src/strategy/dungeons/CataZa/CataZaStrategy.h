/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) strategy "cata-za", MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSStrategy.h.

#ifndef PLAYERBOTS_CATAZASTRATEGY_H
#define PLAYERBOTS_CATAZASTRATEGY_H

#include "Strategy.h"

class CataDungeonZulAmanStrategy : public Strategy
{
public:
    CataDungeonZulAmanStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "cata-za"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
