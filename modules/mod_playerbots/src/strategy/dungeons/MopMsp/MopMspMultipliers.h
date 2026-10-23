/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - multipliers.

#ifndef PLAYERBOTS_MOPMSPMULTIPLIERS_H
#define PLAYERBOTS_MOPMSPMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has a hazard to leave (or the meteor to stack on) no other movement action may pull it back
// (shape of HoS/HoSMultipliers.cpp:14 KrystallusMultiplier and UB/UBMultipliers.cpp:19).
class MopMspHazardMultiplier : public Multiplier
{
public:
    MopMspHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop msp hazard") {}
    float GetValue(Action* action) override;
};

// While a DPS bot has a protector to focus, the generic target pickers must not re-target it
// (shape of MgT/MgTMultipliers.cpp MgTFocusOrderMultiplier).
class MopMspFocusMultiplier : public Multiplier
{
public:
    MopMspFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop msp focus order") {}
    float GetValue(Action* action) override;
};

#endif
