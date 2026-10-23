/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm actions (epic #373, issue #653).

#ifndef PLAYERBOTS_MOPSPMACTIONS_H
#define PLAYERBOTS_MOPSPMACTIONS_H

#include "Action.h"
#include "MovementActions.h"

// Steps to the nearest walkable point outside every live ground hazard (see MopSpmShared.h).
class MopSpmAvoidAction : public MovementAction
{
public:
    MopSpmAvoidAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop spm avoid") {}
    bool Execute(Event event) override;
};

// Parry Stance: drop the melee swing / auto shot / harmful cast in progress. The multiplier keeps the bot from
// starting a new attack until the stance is over.
class MopSpmParryStopAction : public Action
{
public:
    MopSpmParryStopAction(PlayerbotAI* botAI) : Action(botAI, "mop spm parry stop") {}
    bool Execute(Event event) override;
};

// Haze of Hate: press the Meditate extra-action button (5 s stun, Hatred back to 0).
class MopSpmMeditateAction : public Action
{
public:
    MopSpmMeditateAction(PlayerbotAI* botAI) : Action(botAI, "mop spm meditate") {}
    bool Execute(Event event) override;
};

class MopSpmMarkPriorityAction : public Action
{
public:
    MopSpmMarkPriorityAction(PlayerbotAI* botAI) : Action(botAI, "mop spm mark priority") {}
    bool Execute(Event event) override;
};

#endif
