/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace (map 994) strategy "mop-msp" (epic #373, issue #654). MoP-only: no AC file to port, the shape
// is fork strategy/dungeons/UB/UBStrategy.h (target exclusions) + MgT (value-backed escape/focus/interrupt).

#ifndef PLAYERBOTS_MOPMSPSTRATEGY_H
#define PLAYERBOTS_MOPMSPSTRATEGY_H

#include "Strategy.h"

class MopMspStrategy : public Strategy
{
public:
    MopMspStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-msp"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;

    bool HasTargetExclusions() const override { return true; }
    void AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType type) override;
};

#endif
