/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_NEXACTIONS_H
#define PLAYERBOTS_NEXACTIONS_H

#include "Action.h"
#include "AttackActions.h"  // 5.4.8: AC "AttackAction.h" -> fork strategy/actions/AttackActions.h
#include "MovementActions.h"
#include "NexTriggers.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// AC Ai/Dungeon/Nex/NexActions.h:16
class MoveFromWhirlwindAction : public MovementAction
{
public:
    MoveFromWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "move from whirlwind") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/Nex/NexActions.h:23
class FirebombSpreadAction : public MovementAction
{
public:
    FirebombSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "firebomb spread") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/Nex/NexActions.h:30
class TelestraSplitTargetAction : public AttackAction
{
public:
    TelestraSplitTargetAction(PlayerbotAI* ai) : AttackAction(ai, "telestra split target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/Nex/NexActions.h:38
class ChaoticRiftTargetAction : public AttackAction
{
public:
    ChaoticRiftTargetAction(PlayerbotAI* ai) : AttackAction(ai, "chaotic rift target") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/Nex/NexActions.h:46
class DodgeSpikesAction : public MovementAction
{
public:
    DodgeSpikesAction(PlayerbotAI* ai) : MovementAction(ai, "dodge spikes") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/Nex/NexActions.h:54
class IntenseColdJumpAction : public MovementAction
{
public:
    IntenseColdJumpAction(PlayerbotAI* ai) : MovementAction(ai, "intense cold jump") {}
    bool Execute(Event event) override;
};

#endif
