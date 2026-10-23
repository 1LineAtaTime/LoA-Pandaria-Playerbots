/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - triggers (issue #673, epic #373).

#include "CataHotTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool CataHotConditionTrigger::IsActive()
{
    return _condition(bot);
}

bool CataHotHazardTrigger::IsActive()
{
    return HourOfTwilight::InHazard(bot, _kind);
}
