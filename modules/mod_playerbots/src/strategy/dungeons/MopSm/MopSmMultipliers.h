/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMMULTIPLIERS_H
#define PLAYERBOTS_MOPSMMULTIPLIERS_H

#include "Multiplier.h"

// While Korloff spins Firestorm Kick the only legal movement near him is the leave action: melee must not walk back in.
class MopSmFirestormMultiplier : public Multiplier
{
public:
    MopSmFirestormMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sm firestorm kick") {}
    float GetValue(Action* action) override;
};

// While there is an add to kill first, the damage dealers' normal target pickers (assist / aoe / drop) are vetoed so
// they do not flip the bot back to the boss every tick (same idea as the fork's MgT strategy, MgTMultipliers.cpp:50-82).
class MopSmFocusMultiplier : public Multiplier
{
public:
    MopSmFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sm focus add") {}
    float GetValue(Action* action) override;
};

// While Whitemane casts Scarlet Resurrection no interrupt / hard-control action may run. Wowpedia: the cast is
// uninterruptible; since #684 the script gives her interrupt immunity for it and recasts it until it lands, so a kick
// there is a wasted interrupt (before #684 it froze the fight).
// #781: while the bot stands in Korloff's Scorched Earth fire, only "sm leave scorched earth" may move it (same shape
// as MopSmFirestormMultiplier / AC HoSMultipliers.cpp:14-45).
class MopSmScorchedEarthMultiplier : public Multiplier
{
public:
    MopSmScorchedEarthMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sm scorched earth") {}
    float GetValue(Action* action) override;
};

class MopSmReviveGuardMultiplier : public Multiplier
{
public:
    MopSmReviveGuardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "sm scarlet resurrection") {}
    float GetValue(Action* action) override;
};

#endif
