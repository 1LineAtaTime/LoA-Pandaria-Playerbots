/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UPMULTIPLIERS_H
#define PLAYERBOTS_UPMULTIPLIERS_H

#include "Multiplier.h"

// AC Ai/Dungeon/UP/UPMultipliers.h:12
class SkadiMultiplier : public Multiplier
{
    public:
        SkadiMultiplier(PlayerbotAI* ai) : Multiplier(ai, "skadi the ruthless") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/UP/UPMultipliers.h:21
class YmironMultiplier : public Multiplier
{
    public:
        YmironMultiplier(PlayerbotAI* ai) : Multiplier(ai, "king ymiron") {}

    public:
        float GetValue(Action* action) override;
};

#endif
