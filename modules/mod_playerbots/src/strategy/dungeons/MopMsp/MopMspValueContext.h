/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654) - the values behind the triggers. Same shape as
// fork strategy/dungeons/UB/UBValueContext.h and MgT/MgTValueContext.h: a CalculatedValue per question, so the
// triggers, the action and the multipliers all read one cached answer instead of scanning the grid themselves.

#ifndef PLAYERBOTS_MOPMSPVALUECONTEXT_H
#define PLAYERBOTS_MOPMSPVALUECONTEXT_H

#include "MopMspShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Unit.h"
#include "Value.h"

// "mop msp hazard": where this bot has to go right now (cone / ring / axe / dervish / spread / meteor stack).
class MopMspHazardValue : public CalculatedValue<MopMsp::Hazard>
{
public:
    MopMspHazardValue(PlayerbotAI* botAI) : CalculatedValue<MopMsp::Hazard>(botAI, "mop msp hazard", 250) {}

protected:
    MopMsp::Hazard Calculate() override
    {
        MopMsp::Hazard hazard;
        MopMsp::ComputeHazard(bot, hazard);
        return hazard;
    }
};

// "mop msp focus target": the Glintrok protector the DPS bots must kill next (empty outside Gekkan's room).
class MopMspFocusTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    MopMspFocusTargetValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "mop msp focus target", 500) {}

protected:
    ObjectGuid Calculate() override { return MopMsp::ComputeFocusTarget(bot); }
};

// "mop msp exclusions": guids no target value may pick (Mu'Shiba).
class MopMspExclusionsValue : public CalculatedValue<GuidVector>
{
public:
    MopMspExclusionsValue(PlayerbotAI* botAI) : CalculatedValue<GuidVector>(botAI, "mop msp exclusions", 1000) {}

protected:
    GuidVector Calculate() override { return MopMsp::ComputeExclusions(bot); }
};

class MopMspValueContext : public NamedObjectContext<UntypedValue>
{
public:
    MopMspValueContext()
    {
        creators["mop msp hazard"] = &MopMspValueContext::mop_msp_hazard;
        creators["mop msp focus target"] = &MopMspValueContext::mop_msp_focus_target;
        creators["mop msp exclusions"] = &MopMspValueContext::mop_msp_exclusions;
    }

private:
    static UntypedValue* mop_msp_hazard(PlayerbotAI* botAI) { return new MopMspHazardValue(botAI); }
    static UntypedValue* mop_msp_focus_target(PlayerbotAI* botAI) { return new MopMspFocusTargetValue(botAI); }
    static UntypedValue* mop_msp_exclusions(PlayerbotAI* botAI) { return new MopMspExclusionsValue(botAI); }
};

#endif
