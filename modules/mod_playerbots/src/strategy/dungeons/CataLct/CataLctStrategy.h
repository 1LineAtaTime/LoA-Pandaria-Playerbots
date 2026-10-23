/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (map 755, "cata-lct"). Epic #373, issue #666. See CataLctShared.h for sources.

#ifndef PLAYERBOTS_CATALCTSTRATEGY_H
#define PLAYERBOTS_CATALCTSTRATEGY_H

#include "Strategy.h"

class CataDungeonLctStrategy : public Strategy
{
public:
    CataDungeonLctStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "cata-lct"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    // #819: the Blaze of the Heavens phoenix is not a damage dealer target while Barim lives (shape of CataHot/UB)
    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;

private:
    uint32 _cacheTime = 0;
    bool _cacheValid = false;
    GuidSet _cache;
    GuidSet _tankCache;  // #666: Tank exclusion type (the egg during Repentance only)
};

#endif
