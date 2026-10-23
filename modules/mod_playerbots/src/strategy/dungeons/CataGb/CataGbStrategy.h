/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBSTRATEGY_H
#define PLAYERBOTS_CATAGBSTRATEGY_H

#include "Strategy.h"

// Grim Batol (map 670), issue #664. Mechanics and sources are listed per edge in CataGbTriggers.h.
class CataDungeonGbStrategy : public Strategy
{
public:
    CataDungeonGbStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-gb"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
