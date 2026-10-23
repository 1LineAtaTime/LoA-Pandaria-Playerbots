/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - actions (issue #671, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSActions.h
// (MovementAction subclasses) and AC Ai/Dungeon/PoS/PoSActions.cpp (FleePosition spread).

#ifndef PLAYERBOTS_CATAETACTIONS_H
#define PLAYERBOTS_CATAETACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataEtShared.h"
#include "MovementActions.h"

// Base of every moving action of this strategy; the hazard multiplier lets these through and blocks the rest.
class CataEtMovementAction : public MovementAction
{
public:
    CataEtMovementAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    bool StepTo(float x, float y, float z);
    bool LeaveHazards(std::vector<EndTime::Hazard> const& hazards);
};

// Leave a circular ground hazard of this kind (blight, lance, eye, distortion bomb).
class CataEtEscapeAction : public CataEtMovementAction
{
public:
    CataEtEscapeAction(PlayerbotAI* botAI, std::string const name, EndTime::HazardKind kind)
        : CataEtMovementAction(botAI, name), _kind(kind)
    {
    }

    bool Execute(Event event) override;

private:
    EndTime::HazardKind _kind;
};

// Jaina: sidestep the three Frost Blades.
class CataEtBladeDodgeAction : public CataEtMovementAction
{
public:
    CataEtBladeDodgeAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et dodge frost blades") {}
    bool Execute(Event event) override;
};

// Jaina: the stomper runs onto the Flarecore ember, everybody else keeps clear of it.
class CataEtFlarecoreAction : public CataEtMovementAction
{
public:
    CataEtFlarecoreAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et handle flarecore") {}
    bool Execute(Event event) override;
};

// Sylvanas: stand in the sector of the killed Risen Ghoul (the breach), outside Sacrifice.
class CataEtSafeSpotAction : public CataEtMovementAction
{
public:
    CataEtSafeSpotAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et sylvanas safe spot") {}
    bool Execute(Event event) override;
};

// Sylvanas: during the Call stay closer to her than the live Risen Ghoul of the bot's sector (out-of-bounds Wracking Pain 101221).
class CataEtSylvanasInsideAction : public CataEtMovementAction
{
public:
    CataEtSylvanasInsideAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et sylvanas stay inside") {}
    bool Execute(Event event) override;
};

// Sylvanas: ranged bots keep 10 yd apart (Black Arrow / Unholy Shot splash).
class CataEtSpreadAction : public CataEtMovementAction
{
public:
    CataEtSpreadAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et ranged spread") {}
    bool Execute(Event event) override;
};

// Tyrande: stand in the current Pool of Moonlight while the adds event runs.
class CataEtMoonlightPoolAction : public CataEtMovementAction
{
public:
    CataEtMoonlightPoolAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et enter moonlight pool") {}
    bool Execute(Event event) override;
};

// Tyrande: ranged bots stand outside the outer Eye of Elune orbit (37.5-40 yd), the healer on the tank's side.
class CataEtTyrandeRangedBandAction : public CataEtMovementAction
{
public:
    CataEtTyrandeRangedBandAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et tyrande ranged band") {}
    bool Execute(Event event) override;
};

// Jaina: walk to the nearest unused staff fragment and use it.
class CataEtFragmentAction : public CataEtMovementAction
{
public:
    CataEtFragmentAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et collect fragment") {}
    bool Execute(Event event) override;
};

// Baine: swim out of the lava onto the nearest intact platform.
class CataEtLavaAction : public CataEtMovementAction
{
public:
    CataEtLavaAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et baine leave lava") {}
    bool Execute(Event event) override;
};

// Baine: click the totem, then throw it back at him.
class CataEtTotemAction : public CataEtMovementAction
{
public:
    CataEtTotemAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et baine totem") {}
    bool Execute(Event event) override;
};

// Murozond: leave the Infinite Breath cone.
class CataEtBreathAction : public CataEtMovementAction
{
public:
    CataEtBreathAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et dodge infinite breath") {}
    bool Execute(Event event) override;
};

// Murozond: press the Hourglass of Time (rewinds the encounter state of the group, up to five times).
class CataEtHourglassAction : public CataEtMovementAction
{
public:
    CataEtHourglassAction(PlayerbotAI* botAI) : CataEtMovementAction(botAI, "et use hourglass") {}
    bool Execute(Event event) override;

private:
    uint32 _lastUse = 0;
};

// Sylvanas: every non-healer attacks the designated Risen Ghoul until it dies and opens a breach in the ring.
class CataEtKillGhoulAction : public AttackAction
{
public:
    CataEtKillGhoulAction(PlayerbotAI* botAI) : AttackAction(botAI, "et sylvanas kill ghoul") {}
    bool Execute(Event event) override;
};

// Interrupt the cast a boss-specific getter reports.
class CataEtInterruptAction : public Action
{
public:
    using TargetGetter = Unit* (*)(Player*);

    CataEtInterruptAction(PlayerbotAI* botAI, std::string const name, TargetGetter getter)
        : Action(botAI, name), _getter(getter)
    {
    }

    bool Execute(Event event) override;

private:
    TargetGetter _getter;
};

#endif
