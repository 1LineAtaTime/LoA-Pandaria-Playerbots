/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#ifndef PLAYERBOTS_CATASCACTIONS_H
#define PLAYERBOTS_CATASCACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataScShared.h"
#include "MovementActions.h"

// Base of every movement action of this dungeon: the evade multiplier recognises them by the "cata sc " name
// prefix and lets them through while it blocks formation / follow / reach movement.
class CataScMoveAction : public MovementAction
{
public:
    CataScMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Step out of a circle of radius safeRadius around (cx, cy): try the straight line away first, then fan out.
    // awayRef is only used when the bot stands on the centre itself (no direction to run along).
    bool EvadeFrom(float cx, float cy, float safeRadius,
                   MovementPriority priority = MovementPriority::MOVEMENT_COMBAT, Unit* awayRef = nullptr);

    // Step sideways out of the frontal cone (full width arc, radians) of a unit, on the nearer side.
    bool LeaveCone(Unit* boss, float arc, MovementPriority priority = MovementPriority::MOVEMENT_COMBAT);
};

// Leave the nearest ground effect creature of one kind
class CataScLeaveHazardAction : public CataScMoveAction
{
public:
    CataScLeaveHazardAction(PlayerbotAI* botAI, std::string const name, CataSc::HazardId hazard,
                            MovementPriority priority = MovementPriority::MOVEMENT_COMBAT)
        : CataScMoveAction(botAI, name), _hazard(hazard), _priority(priority)
    {
    }

    bool Execute(Event event) override;

private:
    CataSc::HazardId _hazard;
    MovementPriority _priority;
};

// Leave the nearest DynamicObject of one kind
class CataScLeaveDynHazardAction : public CataScMoveAction
{
public:
    CataScLeaveDynHazardAction(PlayerbotAI* botAI, std::string const name, CataSc::DynHazardId hazard,
                               MovementPriority priority = MovementPriority::MOVEMENT_COMBAT)
        : CataScMoveAction(botAI, name), _hazard(hazard), _priority(priority)
    {
    }

    bool Execute(Event event) override;

private:
    CataSc::DynHazardId _hazard;
    MovementPriority _priority;
};

class CataScFleeShatterAction : public CataScMoveAction
{
public:
    CataScFleeShatterAction(PlayerbotAI* botAI) : CataScMoveAction(botAI, "cata sc flee shatter") {}
    bool Execute(Event event) override;
};

class CataScLeaveSlamConeAction : public CataScMoveAction
{
public:
    CataScLeaveSlamConeAction(PlayerbotAI* botAI) : CataScMoveAction(botAI, "cata sc leave slam cone") {}
    bool Execute(Event event) override;
};

class CataScLeaveSandBlastAction : public CataScMoveAction
{
public:
    CataScLeaveSandBlastAction(PlayerbotAI* botAI) : CataScMoveAction(botAI, "cata sc leave sand blast") {}
    bool Execute(Event event) override;
};

// Attack the focus target (and let the tracker bot mark it with skull so the group assist follows)
class CataScFocusAction : public AttackAction
{
public:
    CataScFocusAction(PlayerbotAI* botAI, std::string const name, CataSc::FocusId focus)
        : AttackAction(botAI, name), _focus(focus)
    {
    }

    bool Execute(Event event) override;

private:
    CataSc::FocusId _focus;
};

#endif
