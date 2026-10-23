/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCMULTIPLIERS_H
#define PLAYERBOTS_OCMULTIPLIERS_H

#include "Multiplier.h"
#include "Unit.h"

// AC Ai/Dungeon/OC/OCMultipliers.h:13
// Platform coordinates: identical to the MoP world.creature rows for Mage-Lord Urom (guid 86823,
// 1177.47 937.722 527.405 o 2.21657, map 578); the fork's boss_urom.cpp uses z > 518.63 for the same split.
const float uromCoords[4][4] =
{   // Platform coordinates
    {1177.47f, 937.722f, 527.405f, 2.21657f},
    {968.66f, 1042.53f, 527.32f, 0.077f},
    {1164.02f, 1170.85f, 527.321f, 3.66f},
    {1118.31f, 1080.377f, 508.361f, 4.25f}      // Inner ring, actual boss fight
};

// AC Ai/Dungeon/OC/OCMultipliers.h:21
class MountingDrakeMultiplier : public Multiplier
{
    public:
        MountingDrakeMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mounting drake") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/OC/OCMultipliers.h:30
class OccFlyingMultiplier : public Multiplier
{
    public:
        OccFlyingMultiplier(PlayerbotAI* ai) : Multiplier(ai, "occ flying drake") {}

    public:
        float GetValue(Action* action) override;
};

// AC Ai/Dungeon/OC/OCMultipliers.h:39
class UromMultiplier : public Multiplier
{
    public:
        UromMultiplier(PlayerbotAI* ai) : Multiplier(ai, "mage-lord urom") {}

    public:
        float GetValue(Action* action) override;
    protected:
        uint8 GetPhaseByCurrentPosition(Unit* boss);
};

// AC Ai/Dungeon/OC/OCMultipliers.h:50
class EregosMultiplier : public Multiplier
{
    public:
        EregosMultiplier(PlayerbotAI* ai) : Multiplier(ai, "ley-guardian eregos") {}

    public:
        float GetValue(Action* action) override;
};

#endif
