/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/NearestCorpsesValue.cpp (issue #151). AC uses
 * Acore::UnitListSearcher + Cell::VisitObjects; this core's spelling is Trinity::UnitListSearcher +
 * Cell::VisitAllObjects (the same pair the old LootAction.cpp's FindLootableCorpse used).
 */

#include "NearestCorpsesValue.h"

#include "CellImpl.h"
#include "GridObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"

namespace
{
    // AC NearestCorpsesValue.cpp:12-21.
    class AnyDeadUnitInObjectRangeCheck
    {
    public:
        AnyDeadUnitInObjectRangeCheck(WorldObject const* obj, float /*range*/) : i_obj(obj) {}
        WorldObject const& GetFocusObject() const { return *i_obj; }
        bool operator()(Unit* u) { return !u->IsAlive(); }

    private:
        WorldObject const* i_obj;
    };
}

void NearestCorpsesValue::FindUnits(std::list<Unit*>& targets)
{
    AnyDeadUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<AnyDeadUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool NearestCorpsesValue::AcceptUnit(Unit* /*unit*/) { return true; }
