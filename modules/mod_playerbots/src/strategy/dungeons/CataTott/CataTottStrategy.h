/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTSTRATEGY_H
#define PLAYERBOTS_CATATOTTSTRATEGY_H

#include "Strategy.h"

class CataDungeonTottStrategy : public Strategy
{
public:
    CataDungeonTottStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-tott"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
