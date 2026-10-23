/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLACTIONS_H
#define PLAYERBOTS_HOLACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "HoLTriggers.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// AC Ai/Dungeon/HoL/HoLActions.h:16-17
const Position IONAR_TANK_POSITION = Position(1078.860f, -261.928f, 61.226f);
const Position DISPERSE_POSITION = Position(1161.152f, -261.584f, 53.223f);

// AC Ai/Dungeon/HoL/HoLActions.h:19
class BjarngrimTargetAction : public AttackAction
{
public:
    BjarngrimTargetAction(PlayerbotAI* ai) : AttackAction(ai, "bjarngrim target") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:26
class AvoidWhirlwindAction : public MovementAction
{
public:
    AvoidWhirlwindAction(PlayerbotAI* ai) : MovementAction(ai, "avoid whirlwind") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:33
class VolkhanTargetAction : public AttackAction
{
public:
    VolkhanTargetAction(PlayerbotAI* ai) : AttackAction(ai, "volkhan target") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:40
class StaticOverloadSpreadAction : public MovementAction
{
public:
    StaticOverloadSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "static overload spread") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:47
class BallLightningSpreadAction : public MovementAction
{
public:
    BallLightningSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "ball lightning spread") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:54
class IonarTankPositionAction : public MovementAction
{
public:
    IonarTankPositionAction(PlayerbotAI* ai) : MovementAction(ai, "ionar tank position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:62
class DispersePositionAction : public MovementAction
{
public:
    DispersePositionAction(PlayerbotAI* ai) : MovementAction(ai, "disperse position") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:70
class LokenStackAction : public MovementAction
{
public:
    LokenStackAction(PlayerbotAI* ai) : MovementAction(ai, "loken stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

// AC Ai/Dungeon/HoL/HoLActions.h:78
class AvoidLightningNovaAction : public MovementAction
{
public:
    AvoidLightningNovaAction(PlayerbotAI* ai) : MovementAction(ai, "avoid lightning nova") {}
    bool Execute(Event event) override;
};

#endif
