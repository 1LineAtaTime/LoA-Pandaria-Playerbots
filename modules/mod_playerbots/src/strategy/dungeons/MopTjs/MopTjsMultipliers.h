/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs multipliers (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSMULTIPLIERS_H
#define PLAYERBOTS_MOPTJSMULTIPLIERS_H

#include "Multiplier.h"

// While the bot stands in a live ground hazard only the avoid action may move it (AC HoSMultipliers.cpp:14-45 pattern).
class MopTjsAvoidMultiplier : public Multiplier
{
public:
    MopTjsAvoidMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop tjs avoid") {}
    float GetValue(Action* action) override;

private:
    uint32 _nextCheck = 0;
    bool _inDanger = false;
};

#endif
