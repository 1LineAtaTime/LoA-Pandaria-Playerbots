/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#ifndef PLAYERBOTS_CATASCMULTIPLIERS_H
#define PLAYERBOTS_CATASCMULTIPLIERS_H

#include "Multiplier.h"

// While a "get out now" mechanic applies to the bot, no other movement walks it back in
class CataScEvadeMultiplier : public Multiplier
{
public:
    CataScEvadeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata sc evade") {}
    float GetValue(Action* action) override;

private:
    // the danger test searches the grid; every movement action of a tick asks, so keep the answer for 100 ms
    uint32 _checkedAt = 0;
    bool _danger = false;
};

// DPS bots keep the focus target (Crystal Shards, Devout Followers) instead of re-assisting the boss
class CataScFocusMultiplier : public Multiplier
{
public:
    CataScFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata sc focus") {}
    float GetValue(Action* action) override;
};

#endif
