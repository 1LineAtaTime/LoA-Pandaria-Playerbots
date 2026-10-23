/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSACTIONS_H
#define PLAYERBOTS_COSACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CoSTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

// AC Ai/Dungeon/CoS/CoSActions.h:17
class ExplodeGhoulSpreadAction : public MovementAction
{
public:
    ExplodeGhoulSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "explode ghoul spread") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/CoS/CoSActions.h:24
class EpochStackAction : public MovementAction
{
public:
    EpochStackAction(PlayerbotAI* ai) : MovementAction(ai, "epoch stack") {}
    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
