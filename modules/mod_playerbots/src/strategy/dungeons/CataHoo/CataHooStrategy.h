/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOSTRATEGY_H
#define PLAYERBOTS_CATAHOOSTRATEGY_H

#include "Strategy.h"

// Halls of Origination (map 644), issue #665. Mechanics and sources are listed per edge in CataHooTriggers.h.
class CataDungeonHooStrategy : public Strategy
{
public:
    CataDungeonHooStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-hoo"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
