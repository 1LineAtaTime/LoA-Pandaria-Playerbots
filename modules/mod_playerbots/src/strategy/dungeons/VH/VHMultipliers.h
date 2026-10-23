/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHMULTIPLIERS_H
#define PLAYERBOTS_VHMULTIPLIERS_H

#include "Multiplier.h"

// AC Ai/Dungeon/VH/VHMultipliers.h:12
class ErekemMultiplier : public Multiplier
{
    public:
        ErekemMultiplier(PlayerbotAI* ai) : Multiplier(ai, "erekem") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/VH/VHMultipliers.h:21
class IchoronMultiplier : public Multiplier
{
    public:
        IchoronMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ichoron") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/VH/VHMultipliers.h:30
class ZuramatMultiplier : public Multiplier
{
    public:
        ZuramatMultiplier(PlayerbotAI* ai) : Multiplier(ai, "zuramat the obliterator") {}

    public:
        float GetValue(Action* action) override;
};

#endif
