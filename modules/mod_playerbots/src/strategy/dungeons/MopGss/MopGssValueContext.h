/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - the values behind the triggers. Same shape
// as fork strategy/dungeons/UB/UBValueContext.h and MgT/MgTValueContext.h: a CalculatedValue per question, so the
// triggers, the action and the multipliers all read one cached answer instead of scanning the grid themselves.

#ifndef PLAYERBOTS_MOPGSSVALUECONTEXT_H
#define PLAYERBOTS_MOPGSSVALUECONTEXT_H

#include "MopGssShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Unit.h"
#include "Value.h"

// "mop gss hazard": where this bot has to go right now (sabotage spread / lanes, strafing lane, cone, fixate, winds).
class MopGssHazardValue : public CalculatedValue<MopGss::Hazard>
{
public:
    MopGssHazardValue(PlayerbotAI* botAI) : CalculatedValue<MopGss::Hazard>(botAI, "mop gss hazard", 250) {}

protected:
    MopGss::Hazard Calculate() override
    {
        MopGss::Hazard hazard;
        MopGss::ComputeHazard(bot, hazard);
        return hazard;
    }
};

// "mop gss focus target": Saboteur at Ri'mok; Weak Spot / Swarm Bringer at Raigonn (empty otherwise).
class MopGssFocusTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    MopGssFocusTargetValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "mop gss focus target", 500) {}

protected:
    ObjectGuid Calculate() override { return MopGss::ComputeFocusTarget(bot); }
};

class MopGssValueContext : public NamedObjectContext<UntypedValue>
{
public:
    MopGssValueContext()
    {
        creators["mop gss hazard"] = &MopGssValueContext::mop_gss_hazard;
        creators["mop gss focus target"] = &MopGssValueContext::mop_gss_focus_target;
    }

private:
    static UntypedValue* mop_gss_hazard(PlayerbotAI* botAI) { return new MopGssHazardValue(botAI); }
    static UntypedValue* mop_gss_focus_target(PlayerbotAI* botAI) { return new MopGssFocusTargetValue(botAI); }
};

#endif
