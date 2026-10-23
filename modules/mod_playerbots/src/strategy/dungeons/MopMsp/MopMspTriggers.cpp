/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654). See MopMspShared.h for sources.

#include "MopMspTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

bool MopMspHazardTrigger::IsActive()
{
    auto const& hazard = AI_VALUE_REF(MopMsp::Hazard, "mop msp hazard");
    return hazard.kind == _kind;
}

bool MopMspInterruptTrigger::IsActive()
{
    if (!MopMsp::HasInterruptSpell(botAI))
        return false;

    Unit* target = MopMsp::FindInterruptTarget(bot, _group);
    return target && _gate.Allow(bot, target);
}

bool MopMspFocusTrigger::IsActive()
{
    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop msp focus target");
    if (focus.IsEmpty())
        return false;

    Unit* current = AI_VALUE(Unit*, "current target");
    return !current || current->GetGUID() != focus;
}
