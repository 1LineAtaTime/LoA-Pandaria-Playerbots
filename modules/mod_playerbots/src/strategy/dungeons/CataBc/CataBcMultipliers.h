/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCMULTIPLIERS_H
#define PLAYERBOTS_CATABCMULTIPLIERS_H

#include "Multiplier.h"

// While a "get out now" mechanic applies to the bot, no other movement walks it back in
class CataBcEvadeMultiplier : public Multiplier
{
public:
    CataBcEvadeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata bc evade") {}
    float GetValue(Action* action) override;

private:
    // the danger test searches the grid; every movement action of a tick asks, so keep the answer for 100 ms
    uint32 _checkedAt = 0;
    bool _danger = false;
};

// Rom'ogg: a chained bot keeps its target on the Chains of Woe until they are gone
class CataBcChainsMultiplier : public Multiplier
{
public:
    CataBcChainsMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata bc chains") {}
    float GetValue(Action* action) override;
};

// Karsh: the tank holds its post on the far side of the pillar until Karsh has walked through it
class CataBcKarshMultiplier : public Multiplier
{
public:
    CataBcKarshMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata bc karsh") {}
    float GetValue(Action* action) override;

private:
    // the hold is capped: a Karsh that never closes in (stuck, leashed) must not freeze the tank
    uint32 _holdSince = 0;
};

#endif
