/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - the values behind the triggers. Same shape as
// fork strategy/dungeons/UB/UBValueContext.h and MgT/MgTValueContext.h: a CalculatedValue per question, so the
// triggers, the actions and the multipliers all read one cached answer instead of scanning the grid themselves.

#ifndef PLAYERBOTS_MOPSNTVALUECONTEXT_H
#define PLAYERBOTS_MOPSNTVALUECONTEXT_H

#include "MopSntShared.h"
#include "NamedObjectContext.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Unit.h"
#include "Value.h"

// "mop snt hazard": where this bot has to go right now (Thousand Blades, Dashing Strike / Blade Rush lines, armed
// bombs, Caustic Pitch).
class MopSntHazardValue : public CalculatedValue<MopSnt::Hazard>
{
public:
    MopSntHazardValue(PlayerbotAI* botAI) : CalculatedValue<MopSnt::Hazard>(botAI, "mop snt hazard", 250) {}

protected:
    MopSnt::Hazard Calculate() override
    {
        MopSnt::Hazard hazard;
        MopSnt::ComputeHazard(bot, hazard);
        return hazard;
    }
};

// "mop snt focus target": the Sap Globule the DPS bots must kill next (empty away from Jin'bak).
class MopSntFocusTargetValue : public CalculatedValue<ObjectGuid>
{
public:
    MopSntFocusTargetValue(PlayerbotAI* botAI) : CalculatedValue<ObjectGuid>(botAI, "mop snt focus target", 500) {}

protected:
    ObjectGuid Calculate() override { return MopSnt::ComputeFocusTarget(bot); }
};

class MopSntValueContext : public NamedObjectContext<UntypedValue>
{
public:
    MopSntValueContext()
    {
        creators["mop snt hazard"] = &MopSntValueContext::mop_snt_hazard;
        creators["mop snt focus target"] = &MopSntValueContext::mop_snt_focus_target;
    }

private:
    static UntypedValue* mop_snt_hazard(PlayerbotAI* botAI) { return new MopSntHazardValue(botAI); }
    static UntypedValue* mop_snt_focus_target(PlayerbotAI* botAI) { return new MopSntFocusTargetValue(botAI); }
};

#endif
