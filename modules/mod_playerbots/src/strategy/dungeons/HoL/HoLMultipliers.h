/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLMULTIPLIERS_H
#define PLAYERBOTS_HOLMULTIPLIERS_H

#include "Multiplier.h"

// AC Ai/Dungeon/HoL/HoLMultipliers.h:12
class BjarngrimMultiplier : public Multiplier
{
    public:
        BjarngrimMultiplier(PlayerbotAI* ai) : Multiplier(ai, "general bjarngrim") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/HoL/HoLMultipliers.h:21
class VolkhanMultiplier : public Multiplier
{
    public:
        VolkhanMultiplier(PlayerbotAI* ai) : Multiplier(ai, "volkhan") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/HoL/HoLMultipliers.h:30
class IonarMultiplier : public Multiplier
{
    public:
        IonarMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ionar") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/HoL/HoLMultipliers.h:39
class LokenMultiplier : public Multiplier
{
    public:
        LokenMultiplier(PlayerbotAI* ai) : Multiplier(ai, "loken") {}

    public:
        float GetValue(Action* action) override;
};

#endif
