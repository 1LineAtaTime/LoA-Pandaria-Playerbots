/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs strategy "mop-tjs" (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSSTRATEGY_H
#define PLAYERBOTS_MOPTJSSTRATEGY_H

#include "Strategy.h"

class MopTjsStrategy : public Strategy
{
public:
    MopTjsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-tjs"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;

private:
    struct ExclusionCache
    {
        uint32 time = 0;
        bool valid = false;
        GuidSet guids;
    };

    // [0] tank target value, [1] DPS / attacker target values
    ExclusionCache _cache[2];
};

#endif
