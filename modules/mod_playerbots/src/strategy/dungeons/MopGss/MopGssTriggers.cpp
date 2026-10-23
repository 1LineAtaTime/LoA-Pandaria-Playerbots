/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655). See MopGssShared.h for sources.

#include "MopGssTriggers.h"
#include "AiObjectContext.h"
#include "Log.h"
#include "Playerbots.h"

bool MopGssHazardTrigger::IsActive()
{
    auto const& hazard = AI_VALUE_REF(MopGss::Hazard, "mop gss hazard");
    return hazard.kind == _kind;
}

bool MopGssInterruptTrigger::IsActive()
{
    if (!MopGss::HasInterruptSpell(botAI))
        return false;

    return MopGss::FindInterruptTarget(bot, _group) != nullptr;
}

bool MopGssFocusTrigger::IsActive()
{
    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop gss focus target");
    if (focus.IsEmpty())
        return false;

    Unit* current = AI_VALUE(Unit*, "current target");
    return !current || current->GetGUID() != focus;
}

bool MopGssTankFluidTrigger::IsActive()
{
    float x, y, z;
    return MopGss::ComputeTankDragSpot(bot, x, y, z);
}

bool MopGssCannonTrigger::IsActive() { return MopGss::FindCannon(bot) != nullptr; }

bool MopGssFlakTrigger::IsActive()
{
    MopGss::FlakState const flak = MopGss::ComputeFlak(bot);
    bool const active = flak.cannon != nullptr;
    // the Trigger runs once a second, so this line is throttled by itself
    if (_wasActive && !active)
    {
        TC_LOG_DEBUG("playerbots", "[GSSFLAK] %s %s alive=%u", bot->GetName().c_str(),
                     flak.bombardiers ? "stop" : "none-left", flak.bombardiers);
    }

    _wasActive = active;
    return active;
}
