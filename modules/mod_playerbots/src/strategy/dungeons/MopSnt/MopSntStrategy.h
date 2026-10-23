/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple (map 1011) strategy "mop-snt" (epic #373, issue #656). MoP-only: no AC file to port, the
// shape is fork strategy/dungeons/UB/UBStrategy.h + MgT (value-backed escape / focus / interrupt).

#ifndef PLAYERBOTS_MOPSNTSTRATEGY_H
#define PLAYERBOTS_MOPSNTSTRATEGY_H

#include "Strategy.h"

class MopSntStrategy : public Strategy
{
public:
    MopSntStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mop-snt"; }

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
