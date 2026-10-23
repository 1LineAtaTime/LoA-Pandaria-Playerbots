/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - actions.

#ifndef PLAYERBOTS_MOPGSSACTIONS_H
#define PLAYERBOTS_MOPGSSACTIONS_H

#include "Action.h"
#include "AttackActions.h"
#include "MopGssShared.h"
#include "MovementActions.h"

// Walks to the point the "mop gss hazard" value computed: out of a cone / lane / strafing line, away from the
// winds, away from the group while sabotaged, away from Raigonn while fixated.
class MopGssEscapeAction : public MovementAction
{
public:
    MopGssEscapeAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop gss escape") {}
    bool Execute(Event event) override;

private:
    MopGss::HazardKind _movingKind = MopGss::HazardKind::None;  // the hazard the running escape walk is for (#655 w65)
    float _movingX = 0.0f;                                       // and its end point (tells it from a cannon / flak walk)
    float _movingY = 0.0f;
};

// Casts the bot's interrupt on the caster FindInterruptTarget() found.
class MopGssInterruptAction : public Action
{
public:
    MopGssInterruptAction(PlayerbotAI* botAI, std::string const name, MopGss::InterruptGroup group)
        : Action(botAI, name), _group(group)
    {
    }

    bool Execute(Event event) override;

private:
    MopGss::InterruptGroup const _group;
};

// Attacks the add / weak spot named by the "mop gss focus target" value.
class MopGssFocusAction : public AttackAction
{
public:
    MopGssFocusAction(PlayerbotAI* botAI) : AttackAction(botAI, "mop gss focus target") {}
    bool Execute(Event event) override;
};

// #655 the tank walks Ri'mok out of the Viscous Fluid (AC Ai/Dungeon/AC/ACActions.cpp:19-47 ShirrakTankPositionBossAction).
class MopGssTankFluidAction : public MovementAction
{
public:
    MopGssTankFluidAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop gss tank fluid") {}
    bool Execute(Event event) override;
};

// #655 a melee damage dealer rides a courtyard cannon onto Raigonn's Weak Spot (AC Ai/Dungeon/TOC/TOCActions.cpp:178-245
// ToCMountAction + EnterVehicle).
class MopGssCannonAction : public MovementAction
{
public:
    MopGssCannonAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop gss cannon") {}
    bool Execute(Event event) override;
};

// #821 after Ga'dok the elected bot walks to the Flak Cannon (60415) on his tower and clicks it every ~1.2 s: each click
// makes the cannon cast Fire Flak Cannon 116554 on itself and npc_flak_cannon::SpellHit shoots down 5 Krik'thik
// Bombardiers. The cannon is a plain spellclick (VehicleId 0, npc_spellclick_spells 60415 -> 116554), so there is no
// seat to board or leave. Same reach / click model as MopGssCannonAction.
class MopGssFlakAction : public MovementAction
{
public:
    MopGssFlakAction(PlayerbotAI* botAI) : MovementAction(botAI, "mop gss flak") {}
    bool Execute(Event event) override;

private:
    uint32 _lastClickAt = 0;
    uint32 _clicks = 0;
    uint32 _moveFails = 0;
    uint32 _giveUpAt = 0;
    uint32 _lastMoveLogAt = 0;
    uint32 _bestAt = 0;
    float _bestDist = 1.0e9f;
    bool _boarded = false;
};

#endif
