/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - actions (issue #673, epic #373). Pattern: AC Ai/Dungeon/HoS/HoSActions.h
// (MovementAction subclasses), AC Ai/Dungeon/PoS/PoSActions.cpp (FleePosition spread) and the focus attack of the fork's
// MgT strategy (MgTActions.cpp MgTFocusAction).

#ifndef PLAYERBOTS_CATAHOTACTIONS_H
#define PLAYERBOTS_CATAHOTACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "CataHotShared.h"
#include "MovementActions.h"

// Base of every moving action of this strategy; the hazard multiplier lets these through and blocks the rest.
class CataHotMovementAction : public MovementAction
{
public:
    CataHotMovementAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    bool StepTo(float x, float y, float z);
    bool LeaveHazards(std::vector<HourOfTwilight::Hazard> const& hazards);
};

// Leave a circular ground hazard of this kind (Choking Smoke Bomb, Purifying Light / Corrupting Twilight orbs).
class CataHotEscapeAction : public CataHotMovementAction
{
public:
    CataHotEscapeAction(PlayerbotAI* botAI, std::string const name, HourOfTwilight::HazardKind kind)
        : CataHotMovementAction(botAI, name), _kind(kind)
    {
    }

    bool Execute(Event event) override;

private:
    HourOfTwilight::HazardKind _kind;
};

// Benedictus: step sideways out of the Wave of Virtue / Wave of Twilight.
class CataHotWaveDodgeAction : public CataHotMovementAction
{
public:
    CataHotWaveDodgeAction(PlayerbotAI* botAI) : CataHotMovementAction(botAI, "hot dodge wave") {}
    bool Execute(Event event) override;
};

// Benedictus: a bot carrying Righteous / Twilight Shear moves away from the others (the pulse splashes 10 yd).
class CataHotShearSpreadAction : public CataHotMovementAction
{
public:
    CataHotShearSpreadAction(PlayerbotAI* botAI) : CataHotMovementAction(botAI, "hot spread shear") {}
    bool Execute(Event event) override;
};

// Asira (#696): a caster carrying Mark of Silence stands behind the tank on Asira's line, so the knife hits the tank.
class CataHotKnifeCoverAction : public CataHotMovementAction
{
public:
    CataHotKnifeCoverAction(PlayerbotAI* botAI) : CataHotMovementAction(botAI, "hot knife cover") {}
    bool Execute(Event event) override;
};

// Benedictus (#696): stack on Thrall inside his Water Shell while a Wave of Virtue walks (phase 1).
class CataHotWaterShellAction : public CataHotMovementAction
{
public:
    CataHotWaterShellAction(PlayerbotAI* botAI) : CataHotMovementAction(botAI, "hot enter water shell") {}
    bool Execute(Event event) override;
};

// Arcurion: attack the Icy Tomb that holds Thrall.
class CataHotKillTombAction : public AttackAction
{
public:
    CataHotKillTombAction(PlayerbotAI* botAI) : AttackAction(botAI, "hot kill icy tomb") {}
    bool Execute(Event event) override;
};

// Thrall's escorts (#673): attack the ambusher that is fighting Thrall.
class CataHotAssistThrallAction : public AttackAction
{
public:
    CataHotAssistThrallAction(PlayerbotAI* botAI) : AttackAction(botAI, "hot assist thrall") {}
    bool Execute(Event event) override;
};

#endif
