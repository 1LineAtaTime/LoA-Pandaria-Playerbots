/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHMULTIPLIERS_H
#define PLAYERBOTS_MOPSHMULTIPLIERS_H

#include "Multiplier.h"

// While Harlan spins Blades of Light the only legal movement is the flee action: chasing the boss (or walking back
// to a melee spot) would put the bot in the vortex.
class MopShBladesMultiplier : public Multiplier
{
public:
    MopShBladesMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sh blades of light") {}
    float GetValue(Action* action) override;
};

// While Koegler channels the breath at a bot standing in the front arc the only legal movement is the step behind him.
class MopShDragonBreathMultiplier : public Multiplier
{
public:
    MopShDragonBreathMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sh dragon breath") {}
    float GetValue(Action* action) override;
};

// #683 w106: the elected Book Burner interceptor on the beam stays there until the bolt lands - no reach / tank-face / formation
// move takes it off (only the escapes and the intercept step itself may move it).
class MopShBookBurnerMultiplier : public Multiplier
{
public:
    MopShBookBurnerMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sh book burner") {}
    float GetValue(Action* action) override;
};

#endif
