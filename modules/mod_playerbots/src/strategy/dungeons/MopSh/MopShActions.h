/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#ifndef PLAYERBOTS_MOPSHACTIONS_H
#define PLAYERBOTS_MOPSHACTIONS_H

#include "Action.h"
#include "MovementActions.h"

// Base for the escape actions: one collision-checked, ground-snapped step at a world angle, issued at combat
// priority (same pattern as the fork's UB strategy, UBActions.cpp:27-45).
class MopShStepAction : public MovementAction
{
public:
    MopShStepAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    bool StepAtAngle(float angle, float distance);
};

class MopShLeaveKnifeLineAction : public MopShStepAction
{
public:
    MopShLeaveKnifeLineAction(PlayerbotAI* botAI) : MopShStepAction(botAI, "sh leave knife line") {}
    bool Execute(Event event) override;
};

class MopShFleeBladesAction : public MovementAction
{
public:
    MopShFleeBladesAction(PlayerbotAI* botAI) : MovementAction(botAI, "sh flee blades of light") {}
    bool Execute(Event event) override;
};

class MopShKoeglerInterruptAction : public Action
{
public:
    MopShKoeglerInterruptAction(PlayerbotAI* botAI) : Action(botAI, "sh koegler interrupt") {}
    bool Execute(Event event) override;
};

class MopShLeaveBurningBooksAction : public MovementAction
{
public:
    MopShLeaveBurningBooksAction(PlayerbotAI* botAI) : MovementAction(botAI, "sh leave burning books") {}
    bool Execute(Event event) override;
};

// #683: the elected bot (w106: nearest to the beam, the tank first) steps onto the Book Burner beam (Koegler -> Book Case line)
// and takes the bolt.
class MopShInterceptBookBurnerAction : public MovementAction
{
public:
    MopShInterceptBookBurnerAction(PlayerbotAI* botAI) : MovementAction(botAI, "sh intercept book burner") {}
    bool Execute(Event event) override;
};

class MopShBehindKoeglerAction : public MopShStepAction
{
public:
    MopShBehindKoeglerAction(PlayerbotAI* botAI) : MopShStepAction(botAI, "sh behind koegler") {}
    bool Execute(Event event) override;
};

#endif
