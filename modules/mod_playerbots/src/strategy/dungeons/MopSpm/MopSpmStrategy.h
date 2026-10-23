/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm strategy "mop-spm" (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMSTRATEGY_H
#define PLAYERBOTS_MOPSPMSTRATEGY_H

#include "Strategy.h"

class MopSpmStrategy : public Strategy
{
public:
    MopSpmStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-spm"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;

private:
    uint32 _cacheTime = 0;
    bool _cacheValid = false;
    GuidSet _cache;
};

#endif
