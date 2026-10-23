/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun (map 962) strategy "mop-gss" (epic #373, issue #655). MoP-only: no AC file to port, the
// shape is fork strategy/dungeons/UB/UBStrategy.h + MgT (value-backed escape / focus / interrupt).

#ifndef PLAYERBOTS_MOPGSSSTRATEGY_H
#define PLAYERBOTS_MOPGSSSTRATEGY_H

#include "Strategy.h"

class MopGssStrategy : public Strategy
{
public:
    MopGssStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-gss"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    // #655 Raigonn phase 1: damage dealers leave his body (-99% while the carapace is up) and shoot the Weak Spot
    // (shape of CataLct/CataLctStrategy.h); #655 w65: the tank target skips him too (he never melees in phase 1).
    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;

private:
    uint32 _cacheTime = 0;
    bool _cacheValid = false;
    GuidSet _cache;
    GuidSet _tankCache;  // #655 w68: Tank exclusion type only (MopGss::CollectTankExclusions)
};

#endif
