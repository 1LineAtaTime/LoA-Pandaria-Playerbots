/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm multipliers (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMMULTIPLIERS_H
#define PLAYERBOTS_MOPSPMMULTIPLIERS_H

#include "Multiplier.h"

// While the bot stands in a live ground hazard only the avoid action may move it (AC HoSMultipliers.cpp:14-45 pattern).
class MopSpmAvoidMultiplier : public Multiplier
{
public:
    MopSpmAvoidMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop spm avoid") {}
    float GetValue(Action* action) override;

private:
    uint32 _nextCheck = 0;
    bool _inDanger = false;
};

// Master Snowdrift's Parry Stance (AC VHMultipliers.cpp:43-61 pattern): no new melee attack or harmful cast on the
// boss while he stands in it. Heals, buffs and movement stay available.
class MopSpmParryMultiplier : public Multiplier
{
public:
    MopSpmParryMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop spm parry stance") {}
    float GetValue(Action* action) override;

private:
    uint32 _nextCheck = 0;
    bool _parry = false;
};

#endif
