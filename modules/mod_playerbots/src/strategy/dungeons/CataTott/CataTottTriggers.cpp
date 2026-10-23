/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#include "CataTottTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

bool CataTottHazardTrigger::IsActive()
{
    return CataTott::GetHazardCreature(bot, _hazard) != nullptr;
}

bool CataTottSurgeTrigger::IsActive()
{
    // Only worth moving when somebody is actually inside the 8 yd blast
    return bot->IsInCombat() && CataTott::HasLightningSurge(bot) &&
           CataTott::GetNearestGroupMember(bot, CataTott::SURGE_RADIUS + CataTott::HAZARD_MARGIN) != nullptr;
}

bool CataTottFocusTrigger::IsActive()
{
    // the adds on Neptulon must be tanked too (Wowpedia "Ozumat", Tactics: "must all be tanked and killed")
    bool const tankJoins = _focus == CataTott::FocusId::NEPTULON_ADDS && PlayerBotSpec::IsTank(bot);
    if (!CataTott::IsFocusDps(bot) && !tankJoins)
        return false;

    return CataTott::GetFocusTarget(bot, _focus) != nullptr;
}

bool CataTottInterruptTrigger::IsActive()
{
    return bot->IsInCombat() && CataTott::GetInterruptTarget(bot) != nullptr;
}
