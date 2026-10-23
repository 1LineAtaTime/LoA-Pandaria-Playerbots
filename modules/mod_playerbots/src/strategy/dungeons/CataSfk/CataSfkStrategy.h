/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (map 33, "cata-sfk"). Epic #373, issue #668. See CataSfkShared.h for sources.

#ifndef PLAYERBOTS_CATASFKSTRATEGY_H
#define PLAYERBOTS_CATASFKSTRATEGY_H

#include "Strategy.h"

class CataDungeonSfkStrategy : public Strategy
{
public:
    CataDungeonSfkStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-sfk"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
