/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBMULTIPLIERS_H
#define PLAYERBOTS_CATAGBMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has to get out of (or into) a spot because of one of this dungeon's mechanics, every other movement
// action is vetoed so the usual combat positioning cannot walk it straight back. The state is looked up at most every
// 150 ms: the engine asks a multiplier once per queued action.
class GbMovementLockMultiplier : public Multiplier
{
public:
    GbMovementLockMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gb movement lock") {}
    float GetValue(Action* action) override;

private:
    uint32 lastCheck = 0;
    bool locked = false;
};

// #688 w106: while Throngus stands at a Cave In / Fire Patch circle (CataGbShared.h ThrongusAtCaveIn) nobody closes in on him -
// reach melee / spell, tank face, behind, formation, the gap closers (charge, heroic leap, intercept, feral charge, shadowstep,
// roll, killing spree) - and nobody follows a master who stands in a circle. The tank drags him out ("gb throngus cave in").
class GbCaveInApproachMultiplier : public Multiplier
{
public:
    GbCaveInApproachMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "gb cave in approach") {}
    float GetValue(Action* action) override;

private:
    uint32 lastCheck = 0;
    bool approachBlocked = false;  // the bot's straight walk to Throngus's melee range ends in / crosses a circle
    bool behindBlocked = false;    // ... the walk to the spot behind his back
    bool ringInCircle = false;     // a circle reaches into his melee ring (killing spree lands anywhere on it)
    bool masterInCircle = false;
};

#endif
