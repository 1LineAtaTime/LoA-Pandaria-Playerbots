/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity (map 939) strategy "cata-woe" - issue #672, epic #373. Pattern: AC Ai/Dungeon/UB/UBStrategy.h (target
// exclusions) and Ai/Dungeon/HoS/HoSStrategy.h. 5.4.8: HasTargetExclusions()/AppendTargetExclusions(GuidSet&,
// TargetValueExclusionType) match fork strategy/Strategy.h:61-65.

#ifndef PLAYERBOTS_CATAWOESTRATEGY_H
#define PLAYERBOTS_CATAWOESTRATEGY_H

#include "Strategy.h"

class CataDungeonWellOfEternityStrategy : public Strategy
{
public:
    CataDungeonWellOfEternityStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "cata-woe"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
