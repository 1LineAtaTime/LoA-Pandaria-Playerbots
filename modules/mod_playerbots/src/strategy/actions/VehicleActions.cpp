/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/VehicleActions.cpp (#179). Deviations from AC:
 *  - `ServerFacade::instance().GetDistance2d` -> this fork's `sServerFacade->GetDistance2d` singleton.
 *  - AC's auto-board-any-nearby-friendly-vehicle branch (AC lines 42-72) is ported 1:1 (#273). It is
 *    NOT IoC-specific: AC reads AI_VALUE(GuidVector, "nearest vehicles") from PvpTriggers.cpp:307,
 *    TOCActions.cpp:185 and ICCActions_GSB.cpp:114 as well, and VehicleActions.cpp:42 is simply the
 *    only reader in this fork so far. Its value, NearestVehiclesValue, and the "nearest vehicles"
 *    creator were added alongside it (strategy/value/NearestNpcsValue.{h,cpp} and
 *    strategy/value/ValueContext.h), 1:1 with AC Ai/Base/Value/NearestNpcsValue.h:41-52 and
 *    Ai/Base/ValueContext.h:117-118. Order matters: AI_VALUE expands to
 *    context->GetValue<T>(name)->Get() with NO null guard (Playerbots.h:26), so the creator must be
 *    registered before any call site dereferences it, or this is a guaranteed crash, not dead code.
 *  - the IoC entry exclusions port verbatim: BattlegroundIC.h DOES exist in 5.4.8 at
 *    src/server/game/Battlegrounds/Zones/BattlegroundIC.h, with NPC_KEEP_CANNON = 34944 (:38) and
 *    NPC_CATAPULT = 34793 (:44), the same values AC uses. An earlier revision of this comment
 *    wrongly claimed IoC did not exist in 5.4.8; that claim was false and is now removed. AC's inline
 *    comment on the IsVehicleInUse() check, which asserted this code is used only by the IoC
 *    vehicles, is dropped for the same reason.
 *  - depends on #150 (MovementAction::MoveTo's vehicle generatePath condition was inverted vs AC;
 *    fixed alongside this batch in the same file, MovementActions.cpp) so a ground vehicle the bot
 *    steers after boarding actually pathfinds.
 */

#include "VehicleActions.h"

#include "BattlegroundIC.h"
#include "Event.h"
#include "ObjectDefines.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include "Unit.h"
#include "Vehicle.h"
#include "WorldSession.h"

bool EnterVehicleAction::Execute(Event event)
{
    // do not switch vehicles yet
    if (bot->GetVehicle())
        return false;

    Player* master = botAI->GetMaster();
    // Triggered by the master targeting a vehicle (the "enter vehicle" command).
    if (event.getOwner() && master && master->GetTarget())
    {
        Unit* vehicleBase = botAI->GetUnit(master->GetTarget());
        if (!vehicleBase)
            return false;

        Vehicle* veh = vehicleBase->GetVehicleKit();
        if (vehicleBase->IsVehicle() && veh && veh->GetAvailableSeatCount())
            return EnterVehicle(vehicleBase, false);

        return false;
    }

    GuidVector npcs = AI_VALUE(GuidVector, "nearest vehicles");
    for (GuidVector::iterator i = npcs.begin(); i != npcs.end(); i++)
    {
        Unit* vehicleBase = botAI->GetUnit(*i);
        if (!vehicleBase)
            continue;

        if (vehicleBase->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
            continue;

        // dont let them get in the cannons as they'll stay forever and do nothing useful
        // dont let them in catapult they cant use them at all
        if (NPC_KEEP_CANNON == vehicleBase->GetEntry() || NPC_CATAPULT == vehicleBase->GetEntry())
            continue;

        if (!vehicleBase->IsFriendlyTo(bot))
            continue;

        if (!vehicleBase->GetVehicleKit()->GetAvailableSeatCount())
            continue;

        // this will avoid adding passengers (which dont really do much for the vehicles that do
        // have a passenger role); AC's comment here said "which is the only place this code is
        // used" - that was IoC-only and is false, this value is shared (see the file header).
        if (vehicleBase->GetVehicleKit()->IsVehicleInUse())
            continue;

        if (EnterVehicle(vehicleBase, true))
            return true;
    }

    return false;
}

bool EnterVehicleAction::EnterVehicle(Unit* vehicleBase, bool moveIfFar)
{
    float dist = sServerFacade->GetDistance2d(bot, vehicleBase);
    if (dist > 40.0f)
        return false;

    if (dist > INTERACTION_DISTANCE && !moveIfFar)
        return false;

    if (dist > INTERACTION_DISTANCE)
        return MoveTo(vehicleBase);

    // Use HandleSpellClick instead of Unit::EnterVehicle to handle a special vehicle script.
    vehicleBase->HandleSpellClick(bot);

    if (!bot->IsOnVehicle(vehicleBase))
        return false;

    // AC PR mod-playerbots#2856 (merged 2026-10-06, Ai/Base/Actions/VehicleActions.cpp:93-97): a siege position
    // is only for the vehicle it was set for: the tactics set one for this vehicle
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo siegePos = posMap["bg siege"];
    siegePos.Reset();
    posMap["bg siege"] = siegePos;

    // Dismount, because a bot can be told to enter a vehicle while mounted.
    WorldPacket emptyPacket;
    bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    return true;
}

bool LeaveVehicleAction::Execute(Event /*event*/)
{
    Vehicle* myVehicle = bot->GetVehicle();
    if (!myVehicle)
        return false;

    VehicleSeatEntry const* seat = myVehicle->GetSeatForPassenger(bot);
    if (!seat || !seat->CanEnterOrExit())
        return false;

    WorldPacket p;
    bot->GetSession()->HandleRequestVehicleExit(p);

    return true;
}
