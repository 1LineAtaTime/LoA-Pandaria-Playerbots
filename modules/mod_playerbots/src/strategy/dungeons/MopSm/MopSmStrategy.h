/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMSTRATEGY_H
#define PLAYERBOTS_MOPSMSTRATEGY_H

#include "Strategy.h"

class MopDungeonScarletMonasteryStrategy : public Strategy
{
public:
    MopDungeonScarletMonasteryStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "mop-sm"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
