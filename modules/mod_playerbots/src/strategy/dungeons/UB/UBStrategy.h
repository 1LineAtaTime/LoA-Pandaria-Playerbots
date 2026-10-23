/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UBSTRATEGY_H
#define PLAYERBOTS_UBSTRATEGY_H

#include "Strategy.h"

// AC Ai/Dungeon/UB/UBStrategy.h:12
// 5.4.8: HasTargetExclusions()/AppendTargetExclusions(GuidSet&, TargetValueExclusionType) match fork
// strategy/Strategy.h:61-65 (GuidSet = std::set<ObjectGuid>, src/server/game/Entities/Object/ObjectGuid.h:300).
class TbcDungeonUnderbogStrategy : public Strategy
{
public:
    TbcDungeonUnderbogStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tbc-ub"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
