/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_FOSACTIONS_H
#define PLAYERBOTS_FOSACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "FoSTriggers.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// AC Ai/Dungeon/FoS/FoSActions.h:16
const Position BRONJAHM_TANK_POSITION = Position(5297.920f, 2506.698f, 686.068f);

// AC Ai/Dungeon/FoS/FoSActions.h:18
class MoveFromBronjahmAction : public MovementAction
{
public:
    MoveFromBronjahmAction(PlayerbotAI* ai) : MovementAction(ai, "move from bronjahm") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/FoS/FoSActions.h:25
class AttackCorruptedSoulFragmentAction : public AttackAction
{
public:
    AttackCorruptedSoulFragmentAction(PlayerbotAI* ai) : AttackAction(ai, "attack corrupted soul fragment") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/FoS/FoSActions.h:32
class BronjahmGroupPositionAction : public AttackAction
{
public:
    BronjahmGroupPositionAction(PlayerbotAI* ai) : AttackAction(ai, "bronjahm group position") {}

    bool Execute(Event event) override;

    bool isUseful() override;
};

// AC Ai/Dungeon/FoS/FoSActions.h:42
class DevourerOfSoulsAction : public AttackAction
{
public:
    DevourerOfSoulsAction(PlayerbotAI * ai, float distance = 10.0f, float delta_angle = M_PI / 8)
        : AttackAction(ai, "devourer of souls")
    {
        this->distance = distance;
        this->delta_angle = delta_angle;
    }
    virtual bool Execute(Event event);

protected:
    float distance, delta_angle;
};

#endif
