/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/CollisionValue.cpp) into
 * the Legends of Azeroth Pandaria fork (issue #149).
 */

#include "CollisionValue.h"

#include "CellImpl.h"
#include "GridObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "ServerFacade.h"

bool CollisionValue::Calculate()
{
    Unit* target = AI_VALUE(Unit*, qualifier);
    if (!target)
        return false;

    std::list<Unit*> targets;
    float range = sPlayerbotAIConfig->contactDistance;
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, range);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);

    for (Unit* other : targets)
    {
        if (bot == other)
            continue;

        float dist = sServerFacade->GetDistance2d(bot, other->GetPositionX(), other->GetPositionY());
        if (sServerFacade->IsDistanceLessThan(dist, other->GetCombatReach()))
            return true;
    }

    return false;
}
