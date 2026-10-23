/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTMULTIPLIERS_H
#define PLAYERBOTS_CATATOTTMULTIPLIERS_H

#include "Multiplier.h"

// While a "get out now" mechanic applies to the bot, no other movement walks it back in
class CataTottEvadeMultiplier : public Multiplier
{
public:
    CataTottEvadeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata tott evade") {}
    float GetValue(Action* action) override;

private:
    // the danger test searches the grid; every movement action of a tick asks, so keep the answer for 100 ms
    uint32 _checkedAt = 0;
    bool _danger = false;
};

// DPS bots keep the focus target (Waterspout adds, Faceless Sappers) instead of re-assisting the boss
class CataTottFocusMultiplier : public Multiplier
{
public:
    CataTottFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata tott focus") {}
    float GetValue(Action* action) override;
};

// Ghur'sha's Absorb Magic turns spell damage into healing for him: casters wait the 3 seconds out
class CataTottAbsorbMagicMultiplier : public Multiplier
{
public:
    CataTottAbsorbMagicMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cata tott absorb magic") {}
    float GetValue(Action* action) override;
};

#endif
