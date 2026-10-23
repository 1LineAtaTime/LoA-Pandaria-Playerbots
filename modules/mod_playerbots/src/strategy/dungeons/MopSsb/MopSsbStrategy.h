/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb strategy "mop-ssb" (epic #373, issue #652).

#ifndef PLAYERBOTS_MOPSSBSTRATEGY_H
#define PLAYERBOTS_MOPSSBSTRATEGY_H

#include "Strategy.h"

class MopSsbStrategy : public Strategy
{
public:
    MopSsbStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-ssb"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
