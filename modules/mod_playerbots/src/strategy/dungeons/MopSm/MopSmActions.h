/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMACTIONS_H
#define PLAYERBOTS_MOPSMACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"

// Step out of Spirit Gale: one collision-checked, ground-snapped step at a world angle, issued at combat priority
// (same pattern as the fork's UB strategy, UBActions.cpp:27-45).
class MopSmLeaveSpiritGaleAction : public MovementAction
{
public:
    MopSmLeaveSpiritGaleAction(PlayerbotAI* botAI) : MovementAction(botAI, "sm leave spirit gale") {}
    bool Execute(Event event) override;

private:
    bool StepAtAngle(float angle, float distance);
};

class MopSmPriorityInterruptAction : public Action
{
public:
    MopSmPriorityInterruptAction(PlayerbotAI* botAI) : Action(botAI, "sm priority interrupt") {}
    bool Execute(Event event) override;
};

// #684: dispel Whitemane's Power Word: Shield (AC BT ReliquaryOfSoulsSpellstealRuneShieldAction shape, BTActions.h:254).
class MopSmDispelWhitemaneShieldAction : public Action
{
public:
    MopSmDispelWhitemaneShieldAction(PlayerbotAI* botAI) : Action(botAI, "sm dispel whitemane shield") {}
    bool Execute(Event event) override;
};

class MopSmFocusAddAction : public AttackAction
{
public:
    MopSmFocusAddAction(PlayerbotAI* botAI) : AttackAction(botAI, "sm focus add") {}
    bool Execute(Event event) override;
};

class MopSmLeaveFirestormAction : public MovementAction
{
public:
    MopSmLeaveFirestormAction(PlayerbotAI* botAI) : MovementAction(botAI, "sm leave firestorm kick") {}
    bool Execute(Event event) override;
};

class MopSmLeaveScorchedEarthAction : public MovementAction
{
public:
    MopSmLeaveScorchedEarthAction(PlayerbotAI* botAI) : MovementAction(botAI, "sm leave scorched earth") {}
    bool Execute(Event event) override;
};

#endif
