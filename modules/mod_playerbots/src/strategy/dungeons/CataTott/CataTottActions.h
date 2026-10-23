/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTACTIONS_H
#define PLAYERBOTS_CATATOTTACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataTottShared.h"
#include "MovementActions.h"

// Base of every movement action of this dungeon: the evade multiplier recognises them by the "cata tott " name
// prefix and lets them through while it blocks formation / follow / reach movement.
class CataTottMoveAction : public MovementAction
{
public:
    CataTottMoveAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    // Step out of a circle of radius safeRadius around (cx, cy): try the straight line away first, then fan out.
    // awayRef is only used when the bot stands on the centre itself (no direction to run along).
    bool EvadeFrom(float cx, float cy, float safeRadius,
                   MovementPriority priority = MovementPriority::MOVEMENT_COMBAT, Unit* awayRef = nullptr);
};

// Leave the nearest ground effect of one kind
class CataTottLeaveHazardAction : public CataTottMoveAction
{
public:
    CataTottLeaveHazardAction(PlayerbotAI* botAI, std::string const name, CataTott::HazardId hazard,
                              MovementPriority priority = MovementPriority::MOVEMENT_COMBAT)
        : CataTottMoveAction(botAI, name), _hazard(hazard), _priority(priority)
    {
    }

    bool Execute(Event event) override;

private:
    CataTott::HazardId _hazard;
    MovementPriority _priority;
};

// Lightning Surge: move 8 yd+ away from the nearest group member
class CataTottSpreadSurgeAction : public CataTottMoveAction
{
public:
    CataTottSpreadSurgeAction(PlayerbotAI* botAI) : CataTottMoveAction(botAI, "cata tott spread surge") {}
    bool Execute(Event event) override;
};

// Attack the focus target (and let the tracker bot mark it with skull so the group assist follows)
class CataTottFocusAction : public AttackAction
{
public:
    CataTottFocusAction(PlayerbotAI* botAI, std::string const name, CataTott::FocusId focus)
        : AttackAction(botAI, name), _focus(focus)
    {
    }

    bool Execute(Event event) override;

private:
    CataTott::FocusId _focus;
};

class CataTottInterruptAction : public Action
{
public:
    CataTottInterruptAction(PlayerbotAI* botAI) : Action(botAI, "cata tott interrupt") {}
    bool Execute(Event event) override;
};

#endif
