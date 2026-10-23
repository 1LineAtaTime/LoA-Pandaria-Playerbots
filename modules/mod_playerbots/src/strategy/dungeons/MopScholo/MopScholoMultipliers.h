/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#ifndef PLAYERBOTS_MOPSCHOLOMULTIPLIERS_H
#define PLAYERBOTS_MOPSCHOLOMULTIPLIERS_H

#include "Multiplier.h"

// While there is something to kill instead of the boss (Phylactery, real Jandice, Failed Students) the normal target
// pickers (assist / aoe / drop) are vetoed so they do not flip the bot back to the boss every tick (same idea as the
// fork's MgT strategy, MgTMultipliers.cpp:50-82).
class MopScholoFocusMultiplier : public Multiplier
{
public:
    MopScholoFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scholo focus target") {}
    float GetValue(Action* action) override;
};

// While Lilian's Soul chases this bot the only legal movement is running from it.
// #781: while Jandice casts / channels Wondrous Rapidity the tank makes no other move than the dodge (AC
// Ai/Dungeon/UK/UKMultipliers.cpp, tank part of IngvarThePlundererMultiplier: "Prevent movement actions during smash").
class MopScholoRapidityMultiplier : public Multiplier
{
public:
    MopScholoRapidityMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scholo wondrous rapidity") {}
    float GetValue(Action* action) override;
};

// #659 Chillheart's Ice Wave (AC Ai/Dungeon/HoS/HoSMultipliers.cpp:14-30 shape): while the wall is up, no movement
// other than outrunning it when the bot is inside its reach, and no chase of a target that stands inside it (a melee
// "reach" into the wall is an instant kill).
class MopScholoIceWallMultiplier : public Multiplier
{
public:
    MopScholoIceWallMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scholo ice wall") {}
    float GetValue(Action* action) override;
};

class MopScholoFixateMultiplier : public Multiplier
{
public:
    MopScholoFixateMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "scholo fixated") {}
    float GetValue(Action* action) override;
};

#endif
