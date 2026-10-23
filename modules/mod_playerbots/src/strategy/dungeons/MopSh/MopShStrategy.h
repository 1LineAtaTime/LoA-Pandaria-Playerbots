/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHSTRATEGY_H
#define PLAYERBOTS_MOPSHSTRATEGY_H

#include "Strategy.h"

class MopDungeonScarletHallsStrategy : public Strategy
{
public:
    MopDungeonScarletHallsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "mop-sh"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
