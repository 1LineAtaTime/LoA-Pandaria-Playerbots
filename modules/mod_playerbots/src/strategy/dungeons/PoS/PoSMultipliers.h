/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_POSMULTIPLIERS_H
#define PLAYERBOTS_POSMULTIPLIERS_H

#include "Multiplier.h"

// AC Ai/Dungeon/PoS/PoSMultipliers.h:12
class IckAndKrickMultiplier : public Multiplier
{
    public:
    IckAndKrickMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ick and krick") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/PoS/PoSMultipliers.h:21
class GarfrostMultiplier : public Multiplier
{
public:
    GarfrostMultiplier(PlayerbotAI* ai) : Multiplier(ai, "garfrost") { }

    float GetValue(Action* action) override;
};

#endif
