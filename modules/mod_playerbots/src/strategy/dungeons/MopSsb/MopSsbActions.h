/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb actions (epic #373, issue #652).

#ifndef PLAYERBOTS_MOPSSBACTIONS_H
#define PLAYERBOTS_MOPSSBACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MovementActions.h"

// Steps to the nearest walkable point outside every live ground hazard (see MopSsbShared.h).
class MopSsbAvoidAction : public MovementAction
{
public:
    MopSsbAvoidAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop ssb avoid") {}
    bool Execute(Event event) override;
};

// Yan-Zhu's Blackout Brew loses a stack for every second the bot is moving or jumping and gains one for every second
// it stands still; ten stacks stun for 10 s. Runs an arc around the boss, alternating direction.
class MopSsbBlackoutStepAction : public MovementAction
{
public:
    MopSsbBlackoutStepAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop ssb blackout step") {}
    bool Execute(Event event) override;

private:
    float _direction = 1.0f;
};

// #690 Carbonation: click the nearest Fizzy Bubble (spellclick 114459, 20 s of flight), then climb above the boss so the
// 4.9 yd "close to the ground" rule of Carbonation (spell_carbonation) no longer applies.
class MopSsbFizzyFlightAction : public MovementAction
{
public:
    MopSsbFizzyFlightAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop ssb fizzy flight") {}
    bool Execute(Event event) override;
};

// #690 Wall of Suds: hop with the Sudsy aura as the wall walks in (SudsyAreaController turns any jump into a high leap).
class MopSsbWallJumpAction : public MovementAction
{
public:
    MopSsbWallJumpAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop ssb wall jump") {}
    bool Execute(Event event) override;

private:
    float _side = 1.0f;
};

// #690 Bubble Shield: ranged non-healers pop the bubbles (the script puts them on every player's threat list).
class MopSsbBubbleAction : public AttackAction
{
public:
    MopSsbBubbleAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop ssb pop bubble") {}
    bool Execute(Event event) override;
};

class MopSsbMarkPriorityAction : public Action
{
public:
    MopSsbMarkPriorityAction(PlayerbotAI* botAI) : Action(botAI, "mop ssb mark priority") {}
    bool Execute(Event event) override;
};

#endif
