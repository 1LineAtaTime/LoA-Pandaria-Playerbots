/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time bot strategy "cata-et" - triggers (issue #671, epic #373).

#include "CataEtTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool CataEtConditionTrigger::IsActive()
{
    return _condition(bot);
}

bool CataEtHazardTrigger::IsActive()
{
    return EndTime::InHazard(bot, _kind);
}
