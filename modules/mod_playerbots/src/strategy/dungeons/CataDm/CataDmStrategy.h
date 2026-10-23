/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (map 36, "cata-dm"). Epic #373, issue #667. See CataDmShared.h for sources.

#ifndef PLAYERBOTS_CATADMSTRATEGY_H
#define PLAYERBOTS_CATADMSTRATEGY_H

#include "Strategy.h"

class CataDungeonDmStrategy : public Strategy
{
public:
    CataDungeonDmStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-dm"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
