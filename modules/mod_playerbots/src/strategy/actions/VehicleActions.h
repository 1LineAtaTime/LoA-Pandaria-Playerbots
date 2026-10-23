/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/VehicleActions.h (#179, spec section 3 "vehicles:
 * enter/leave the master's vehicle"). This header is at FULL AC parity: both classes, both
 * Execute() overrides and EnterVehicleAction::EnterVehicle(Unit*, bool) are declared, with AC's
 * ctor defaults ("enter vehicle" / "leave vehicle"). It forward-declares class Unit - Unit IS a
 * class in the 5.4.8 core (Unit.h:1056), so the key matches - and drops AC's unused
 * #include "Event.h" (Event arrives via MovementActions.h, which this header already includes).
 * Both files are now at FULL AC parity: AC's "nearest vehicles" auto-board-any-friendly-vehicle
 * branch (Ai/Base/Actions/VehicleActions.cpp:42-72) was ported in #273 together with its
 * NearestVehiclesValue, and it is NOT IoC-specific - AC also reads the same value from
 * PvpTriggers.cpp:307, TOCActions.cpp:185 and ICCActions_GSB.cpp:114. See the .cpp comment.
 */

#ifndef _PLAYERBOT_VEHICLEACTIONS_H
#define _PLAYERBOT_VEHICLEACTIONS_H

#include "MovementActions.h"

class PlayerbotAI;
class Unit;

class EnterVehicleAction : public MovementAction
{
public:
    EnterVehicleAction(PlayerbotAI* botAI, std::string const name = "enter vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;

protected:
    bool EnterVehicle(Unit* vehicleBase, bool moveIfFar);
};

class LeaveVehicleAction : public MovementAction
{
public:
    LeaveVehicleAction(PlayerbotAI* botAI, std::string const name = "leave vehicle") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
