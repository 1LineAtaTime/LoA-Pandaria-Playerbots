/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_OCACTIONS_H
#define PLAYERBOTS_OCACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotAI.h"
#include "UseItemAction.h"

// AC Ai/Dungeon/OC/OCActions.h:16
// Inner-ring safe spots for Mage-Lord Urom's Empowered Arcane Explosion.
const Position uromSafePositions[3] =
{
    Position(1138.88f, 1052.22f, 508.36f),
    Position(1084.62f, 1079.71f, 508.36f),
    Position(1087.42f, 1020.132f, 508.36f)
};

// AC Ai/Dungeon/OC/OCActions.h:23
class AvoidUnstableSphereAction : public MovementAction
{
public:
    AvoidUnstableSphereAction(PlayerbotAI* ai) : MovementAction(ai, "avoid unstable sphere") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/OC/OCActions.h:30
class MountDrakeAction : public UseItemAction
{
public:
    MountDrakeAction(PlayerbotAI* ai) : UseItemAction(ai, "mount drake") {}
    bool Execute(Event event) override;
    bool isPossible() override;
};

// AC Ai/Dungeon/OC/OCActions.h:38
class DismountDrakeAction : public Action
{
public:
    DismountDrakeAction(PlayerbotAI* ai) : Action(ai, "dismount drake") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/OC/OCActions.h:45
class OccFlyDrakeAction : public MovementAction
{
public:
    OccFlyDrakeAction(PlayerbotAI* ai) : MovementAction(ai, "occ fly drake") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/OC/OCActions.h:52
class OccDrakeAttackAction : public Action
{
public:
    OccDrakeAttackAction(PlayerbotAI* botAI) : Action(botAI, "occ drake attack") {}
    bool Execute(Event event) override;

protected:
    Unit* vehicleBase;
    bool CastDrakeSpellAction(Unit* target, uint32 spellId, uint32 cooldown);
    bool AmberDrakeAction(Unit* target);
    bool EmeraldDrakeAction(Unit* target);
    bool RubyDrakeAction(Unit* target);
};

// AC Ai/Dungeon/OC/OCActions.h:66
class AvoidArcaneExplosionAction : public MovementAction
{
public:
    AvoidArcaneExplosionAction(PlayerbotAI* ai) : MovementAction(ai, "avoid arcane explosion") {}
    bool Execute(Event event) override;
};

// AC Ai/Dungeon/OC/OCActions.h:73
class TimeBombSpreadAction : public MovementAction
{
public:
    TimeBombSpreadAction(PlayerbotAI* ai) : MovementAction(ai, "time bomb spread") {}
    bool Execute(Event event) override;
};

#endif
