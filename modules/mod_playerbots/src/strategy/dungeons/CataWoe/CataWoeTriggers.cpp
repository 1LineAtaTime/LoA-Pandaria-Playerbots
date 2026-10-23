/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - triggers (issue #672, epic #373).

#include "CataWoeTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool CataWoeConditionTrigger::IsActive()
{
    return _condition(bot);
}

bool CataWoeHazardTrigger::IsActive()
{
    return WellOfEternity::InHazard(bot, _kind);
}
