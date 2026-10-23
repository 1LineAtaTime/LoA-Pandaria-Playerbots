/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/GD/GDMultipliers.h (#556, epic #569)
#ifndef PLAYERBOTS_GDMULTIPLIERS_H
#define PLAYERBOTS_GDMULTIPLIERS_H

#include "Multiplier.h"

class SladranMultiplier : public Multiplier
{
    public:
        SladranMultiplier(PlayerbotAI* ai) : Multiplier(ai, "slad'ran") {}

    public:
        float GetValue(Action* action) override;
};

class GaldarahMultiplier : public Multiplier
{
    public:
        GaldarahMultiplier(PlayerbotAI* ai) : Multiplier(ai, "gal'darah") {}

    public:
        float GetValue(Action* action) override;
};

#endif
