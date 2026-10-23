/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPSTRATEGY_H
#define PLAYERBOTS_CATAVPSTRATEGY_H

#include "Strategy.h"

// The Vortex Pinnacle (map 657), issue #663. Mechanics and sources are listed per edge in CataVpTriggers.h.
class CataDungeonVpStrategy : public Strategy
{
public:
    CataDungeonVpStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-vp"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
