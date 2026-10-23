/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - actions.

#ifndef PLAYERBOTS_MOPMSPACTIONS_H
#define PLAYERBOTS_MOPMSPACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MopMspShared.h"
#include "MovementActions.h"

// Walks to the point the "mop msp hazard" value computed: out of a cone / ring / axe / dervish, away from or to the
// Conflagrate carrier, onto the Meteor target.
class MopMspEscapeAction : public MovementAction
{
public:
    MopMspEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop msp escape") {}
    bool Execute(Event event) override;
};

// Casts the bot's interrupt on the caster FindInterruptTarget() found.
class MopMspInterruptAction : public Action
{
public:
    MopMspInterruptAction(PlayerbotAI* botAI, std::string const name, MopMsp::InterruptGroup group)
        : Action(botAI, name), _group(group)
    {
    }

    bool Execute(Event event) override;

private:
    MopMsp::InterruptGroup const _group;
};

// Attacks the protector named by the "mop msp focus target" value.
class MopMspFocusAction : public AttackAction
{
public:
    MopMspFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop msp focus target") {}
    bool Execute(Event event) override;
};

#endif
