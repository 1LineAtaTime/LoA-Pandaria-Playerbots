/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCSTRATEGY_H
#define PLAYERBOTS_CATABCSTRATEGY_H

#include "Strategy.h"

class CataDungeonBcStrategy : public Strategy
{
public:
    CataDungeonBcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-bc"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    // Shadows of Obsidius absorb every hit; nobody targets them
    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
