/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - actions (issue #672, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSActions.h
// (MovementAction subclasses) and the focus attack of the fork's MgT strategy (MgTActions.cpp MgTFocusAction).

#ifndef PLAYERBOTS_CATAWOEACTIONS_H
#define PLAYERBOTS_CATAWOEACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataWoeShared.h"
#include "MovementActions.h"

// Base of every moving action of this strategy; the hazard multiplier lets these through and blocks the rest.
class CataWoeMovementAction : public MovementAction
{
public:
    CataWoeMovementAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    bool StepTo(float x, float y, float z);
    bool LeaveHazards(std::vector<WellOfEternity::Hazard> const& hazards);
};

// Leave a circular ground hazard of this kind (Fel Flames, Eye of Peroth'arn, Arcane Bomb marker).
class CataWoeEscapeAction : public CataWoeMovementAction
{
public:
    CataWoeEscapeAction(PlayerbotAI* botAI, std::string const name, WellOfEternity::HazardKind kind)
        : CataWoeMovementAction(botAI, name), _kind(kind)
    {
    }

    bool Execute(Event event) override;

private:
    WellOfEternity::HazardKind _kind;
};

// Azshara: interrupt Total Obedience; a melee bot without a ready ranged interrupt walks up to her.
class CataWoeObedienceInterruptAction : public CataWoeMovementAction
{
public:
    CataWoeObedienceInterruptAction(PlayerbotAI* botAI) : CataWoeMovementAction(botAI, "woe interrupt total obedience") {}
    bool Execute(Event event) override;
};

// Azshara: kill the Hand of the Queen riding the puppet (#695).
class CataWoeKillHandAction : public AttackAction
{
public:
    CataWoeKillHandAction(PlayerbotAI* botAI) : AttackAction(botAI, "woe kill hand of the queen") {}
    bool Execute(Event event) override;
};

// Illidan escort: walk to the unlocked Portal Energy Focus and use it (#695 instance gate).
class CataWoeUsePortalFocusAction : public CataWoeMovementAction
{
public:
    CataWoeUsePortalFocusAction(PlayerbotAI* botAI) : CataWoeMovementAction(botAI, "woe use portal energy focus") {}
    bool Execute(Event event) override;
};

// Mannoroth: attack the nearest Dreadlord Debilitator.
class CataWoeKillDebilitatorAction : public AttackAction
{
public:
    CataWoeKillDebilitatorAction(PlayerbotAI* botAI) : AttackAction(botAI, "woe kill debilitator") {}
    bool Execute(Event event) override;
};

#endif
