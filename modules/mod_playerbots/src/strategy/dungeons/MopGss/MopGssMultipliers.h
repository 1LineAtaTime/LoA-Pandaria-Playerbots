/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - multipliers.

#ifndef PLAYERBOTS_MOPGSSMULTIPLIERS_H
#define PLAYERBOTS_MOPGSSMULTIPLIERS_H

#include "Multiplier.h"

// While the bot has a hazard to leave no other movement action may pull it back
// (shape of HoS/HoSMultipliers.cpp:14 KrystallusMultiplier and UB/UBMultipliers.cpp:19).
class MopGssHazardMultiplier : public Multiplier
{
public:
    MopGssHazardMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop gss hazard") {}
    float GetValue(Action* action) override;
};

// While a DPS bot has an add / weak spot to focus, the generic target pickers must not re-target it
// (shape of MgT/MgTMultipliers.cpp MgTFocusOrderMultiplier).
class MopGssFocusMultiplier : public Multiplier
{
public:
    MopGssFocusMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop gss focus order") {}
    float GetValue(Action* action) override;
};

// #655 w59: while a melee damage dealer should ride a cannon, no other movement / reach action may take it back to
// Raigonn's body (the rogue chased him for 420 s and never reached an Artillery). The FindCannon answer is kept 500 ms.
class MopGssCannonMultiplier : public Multiplier
{
public:
    MopGssCannonMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop gss cannon order") {}
    float GetValue(Action* action) override;

private:
    uint32 _checkedAt = 0;
    bool _wantCannon = false;
};

// #655 w69: no "tank face" on Raigonn (AC precedent: BT/BTMultipliers.cpp:466, BWL/BWLMultipliers.cpp:38 veto TankFaceAction
// per boss). His combat reach is 22.68 yd (creature_model_info 39519), so TankFaceAction's near point (GetNearPoint at the
// summed reach, 108 degrees off the group, MovementActions.cpp:1843-1850) is a 30+ yd walk: in w69 the tank walked
// 946.6,2291.8 -> 941.7,2323.0 with no swing while Raigonn and two Protectorates hit him, and died 8 s into phase 2. Raigonn
// has no frontal cone (Wowpedia "Raigonn": Stomp hits everyone near him), so facing him away from the group buys nothing.
class MopGssTankFaceMultiplier : public Multiplier
{
public:
    MopGssTankFaceMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "mop gss tank face") {}
    float GetValue(Action* action) override;
};

#endif
