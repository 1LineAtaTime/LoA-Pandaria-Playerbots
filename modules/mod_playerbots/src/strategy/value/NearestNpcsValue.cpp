/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NearestNpcsValue.cpp) into
 * the Legends of Azeroth Pandaria fork.
 */

#include "NearestNpcsValue.h"

#include "GridObject.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "Vehicle.h"

void NearestNpcsValue::FindUnits(std::list<Unit*>& targets)
{
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool NearestNpcsValue::AcceptUnit(Unit* unit)
{
    return unit && unit->GetTypeId() != TYPEID_PLAYER;
}

// AC Ai/Base/Value/NearestNpcsValue.cpp:23-39 (#545); Acore:: searchers -> Trinity::, VisitObjects -> VisitAllObjects
// as NearestNpcsValue::FindUnits above.
void NearestHostileNpcsValue::FindUnits(std::list<Unit*>& targets)
{
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool NearestHostileNpcsValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsInWorld() || unit->IsDuringRemoveFromWorld())
        return false;

    if (unit->IsPlayer())
        return false;

    return unit->IsHostileTo(bot);
}

void NearestVehiclesValue::FindUnits(std::list<Unit*>& targets)
{
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool NearestVehiclesValue::AcceptUnit(Unit* unit)
{
    if (!unit || !unit->IsVehicle() || !unit->IsAlive())
        return false;

    Vehicle* veh = unit->GetVehicleKit();
    if (!veh || !veh->GetAvailableSeatCount())
        return false;

    return true;
}
