/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_TOCACTIONS_H
#define PLAYERBOTS_TOCACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "EquipAction.h"
#include "Event.h"
#include "ItemCountValue.h"
#include "ItemUsageValue.h"
#include "LastMovementValue.h"
#include "MovementActions.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "StatsWeightCalculator.h"
#include "TOCTriggers.h"
// AC also includes ScriptedCreature.h here; nothing in the ToC strategy uses it, so it is dropped.

// AC Ai/Dungeon/TOC/TOCActions.h:27
class ToCLanceAction : public AttackAction
{
public:
    ToCLanceAction(PlayerbotAI* ai) : AttackAction(ai, "toc lance") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/TOC/TOCActions.h:34
class ToCUELanceAction : public AttackAction
{
public:
    ToCUELanceAction(PlayerbotAI* ai) : AttackAction(ai, "toc ue lance") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/TOC/TOCActions.h:41
class ToCMountedAction : public Action
{
public:
    ToCMountedAction(PlayerbotAI* botAI, std::string const name = "toc mounted")
        : Action(botAI, name) {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/TOC/TOCActions.h:49
class ToCMountAction : public MovementAction
{
public:
    ToCMountAction(PlayerbotAI* botAI, std::string const name = "toc mount")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

// AC Ai/Dungeon/TOC/TOCActions.h:58
class ToCEadricAction : public MovementAction
{
public:
    ToCEadricAction(PlayerbotAI* botAI, std::string const name = "toc eadric")
        : MovementAction(botAI, name) {}
    bool Execute(Event event) override;
};
#endif
