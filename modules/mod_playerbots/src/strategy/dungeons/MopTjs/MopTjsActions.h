/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs actions (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSACTIONS_H
#define PLAYERBOTS_MOPTJSACTIONS_H

#include "AttackActions.h"
#include "MovementActions.h"

// Steps to the nearest walkable point outside every live ground hazard (see MopTjsShared.h).
class MopTjsAvoidAction : public MovementAction
{
public:
    MopTjsAvoidAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop tjs avoid") {}
    bool Execute(Event event) override;
};

// #815: attack the add MopTjs::FindLivingWater() names (AC UKActions.cpp:11-31 AttackFrostTombAction shape); walk
// toward it first while it is out of sight, since AttackAction::Attack refuses a target out of line of sight.
class MopTjsAttackLivingWaterAction : public AttackAction
{
public:
    MopTjsAttackLivingWaterAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop tjs attack living water") {}
    bool Execute(Event event) override;
};

// #651: attack the Lorewalker Stonestep enemy MopTjs::FindLorewalkerTarget() names (same shape as the living water action).
class MopTjsAttackLorewalkerAction : public AttackAction
{
public:
    MopTjsAttackLorewalkerAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop tjs attack lorewalker") {}
    bool Execute(Event event) override;
};

#endif
