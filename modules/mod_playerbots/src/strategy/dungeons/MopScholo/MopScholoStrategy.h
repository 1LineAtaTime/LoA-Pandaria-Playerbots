/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOSTRATEGY_H
#define PLAYERBOTS_MOPSCHOLOSTRATEGY_H

#include "Strategy.h"

class MopDungeonScholomanceStrategy : public Strategy
{
public:
    MopDungeonScholomanceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "mop-scholo"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
