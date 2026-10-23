/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UKACTIONS_H
#define PLAYERBOTS_UKACTIONS_H

#include "Action.h"
#include "MovementActions.h"
#include "AttackActions.h"  // 5.4.8: AC "AttackAction.h" -> fork strategy/actions/AttackActions.h
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "UKTriggers.h"

// AC Ai/Dungeon/UK/UKActions.h class AttackFrostTombAction
class AttackFrostTombAction : public AttackAction
{
public:
    AttackFrostTombAction(PlayerbotAI* ai) : AttackAction(ai, "attack frost tomb") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/UK/UKActions.h class AttackDalronnAction
class AttackDalronnAction : public AttackAction
{
public:
    AttackDalronnAction(PlayerbotAI* ai) : AttackAction(ai, "attack dalronn") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/UK/UKActions.h class IngvarDodgeSmashAction
class IngvarDodgeSmashAction : public MovementAction
{
public:
    IngvarDodgeSmashAction(PlayerbotAI* ai) : MovementAction(ai, "ingvar dodge smash") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/UK/UKActions.h class IngvarSmashReturnAction
class IngvarSmashReturnAction : public MovementAction
{
public:
    IngvarSmashReturnAction(PlayerbotAI* ai) : MovementAction(ai, "ingvar smash return") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
