/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOMULTIPLIERS_H
#define PLAYERBOTS_CATAHOOMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has to get out of (or into) a spot because of one of this dungeon's mechanics, every other movement
// action is vetoed so the usual combat positioning cannot walk it straight back. The state is looked up at most every
// 150 ms: the engine asks a multiplier once per queued action.
class HooMovementLockMultiplier : public Multiplier
{
public:
    HooMovementLockMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hoo movement lock") {}
    float GetValue(Action* action) override;

private:
    uint32 lastCheck = 0;
    bool locked = false;
};

// Isiset's Supernova blinds whoever has her in the front arc when the 3 s cast ends. While she casts it a non-tank bot
// must not attack or cast at her (that would turn it back towards her), so those actions are vetoed; healing goes on.
class HooSupernovaMultiplier : public Multiplier
{
public:
    HooSupernovaMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "hoo supernova") {}
    float GetValue(Action* action) override;

private:
    uint32 lastCheck = 0;
    bool casting = false;
};

#endif
