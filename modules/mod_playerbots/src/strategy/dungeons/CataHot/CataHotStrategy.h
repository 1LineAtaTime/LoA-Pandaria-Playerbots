/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight (map 940) strategy "cata-hot" - issue #673, epic #373. Pattern: AC Ai/Dungeon/UB/UBStrategy.h (target
// exclusions) and Ai/Dungeon/HoS/HoSStrategy.h. 5.4.8: HasTargetExclusions()/AppendTargetExclusions(GuidSet&,
// TargetValueExclusionType) match fork strategy/Strategy.h:61-65.

#ifndef PLAYERBOTS_CATAHOTSTRATEGY_H
#define PLAYERBOTS_CATAHOTSTRATEGY_H

#include "Strategy.h"

class CataDungeonHourOfTwilightStrategy : public Strategy
{
public:
    CataDungeonHourOfTwilightStrategy(PlayerbotAI* ai) : Strategy(ai) {}
    std::string const getName() override { return "cata-hot"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
