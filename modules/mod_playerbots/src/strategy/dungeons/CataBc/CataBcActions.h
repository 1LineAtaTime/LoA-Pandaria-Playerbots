/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#ifndef PLAYERBOTS_CATABCACTIONS_H
#define PLAYERBOTS_CATABCACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataBcShared.h"
#include "MovementActions.h"

// Base of every movement action of this dungeon: the evade multiplier recognises them by the "cata bc " name prefix
// and lets them through while it blocks formation / follow / reach movement.
class CataBcMoveAction : public MovementAction
{
public:
    CataBcMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Step out of a circle of radius safeRadius around (cx, cy): try the straight line away first, then fan out.
    // awayRef is only used when the bot stands on the centre itself (no direction to run along).
    bool EvadeFrom(float cx, float cy, float safeRadius,
                   MovementPriority priority = MovementPriority::MOVEMENT_COMBAT, Unit* awayRef = nullptr);
};

class CataBcFleeSkullcrackerAction : public CataBcMoveAction
{
public:
    CataBcFleeSkullcrackerAction(PlayerbotAI* botAI) : CataBcMoveAction(botAI, "cata bc flee skullcracker") {}
    bool Execute(Event event) override;
};

class CataBcBreakChainsAction : public AttackAction
{
public:
    CataBcBreakChainsAction(PlayerbotAI* botAI) : AttackAction(botAI, "cata bc break chains") {}
    bool Execute(Event event) override;
};

class CataBcLeaveQuakeAction : public CataBcMoveAction
{
public:
    CataBcLeaveQuakeAction(PlayerbotAI* botAI) : CataBcMoveAction(botAI, "cata bc leave quake") {}
    bool Execute(Event event) override;

private:
    float _originX = 0.0f;
    float _originY = 0.0f;
    uint32 _originTime = 0;
    bool _haveOrigin = false;
};

class CataBcLeaveLavaDroolAction : public CataBcMoveAction
{
public:
    CataBcLeaveLavaDroolAction(PlayerbotAI* botAI) : CataBcMoveAction(botAI, "cata bc leave lava drool") {}
    bool Execute(Event event) override;
};

class CataBcLeaveBeamAction : public CataBcMoveAction
{
public:
    CataBcLeaveBeamAction(PlayerbotAI* botAI) : CataBcMoveAction(botAI, "cata bc leave beam") {}
    bool Execute(Event event) override;
};

class CataBcInterruptAction : public Action
{
public:
    CataBcInterruptAction(PlayerbotAI* botAI) : Action(botAI, "cata bc interrupt") {}
    bool Execute(Event event) override;
};

// Karsh is nearly immune (Quicksilver Armor, -99% damage) until he stands in the molten stream in the middle of the
// room, and every second he spends there stacks Superheated Quicksilver (more Heat Wave damage).
// Cooled: the tank takes a post on the far side of the pillar so Karsh walks through it.
// Too hot: the tank walks out of the pillar so Karsh follows.
class CataBcKarshPillarAction : public CataBcMoveAction
{
public:
    CataBcKarshPillarAction(PlayerbotAI* botAI) : CataBcMoveAction(botAI, "cata bc karsh pillar") {}
    bool Execute(Event event) override;
};

#endif
