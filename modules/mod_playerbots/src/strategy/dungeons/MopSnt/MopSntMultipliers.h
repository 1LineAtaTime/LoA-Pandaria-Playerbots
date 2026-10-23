/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - multipliers.

#ifndef PLAYERBOTS_MOPSNTMULTIPLIERS_H
#define PLAYERBOTS_MOPSNTMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has a hazard to leave no other movement action may pull it back
// (shape of HoS/HoSMultipliers.cpp:14 KrystallusMultiplier and UB/UBMultipliers.cpp:19).
class MopSntHazardMultiplier : public Multiplier
{
public:
    MopSntHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop snt hazard") {}
    float GetValue(Action* action) override;
};

// While a DPS bot has a Sap Globule to focus, the generic target pickers must not re-target it
// (shape of MgT/MgTMultipliers.cpp MgTFocusOrderMultiplier).
class MopSntFocusMultiplier : public Multiplier
{
public:
    MopSntFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop snt focus order") {}
    float GetValue(Action* action) override;
};

#endif
