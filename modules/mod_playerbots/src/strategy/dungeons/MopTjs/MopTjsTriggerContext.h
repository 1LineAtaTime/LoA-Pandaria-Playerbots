/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs trigger context (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPTJSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopTjsShared.h"
#include "MopTjsTriggers.h"
#include "NamedObjectContext.h"

class MopTjsTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopTjsTriggerContext()
    {
        creators["mop tjs danger"] = &MopTjsTriggerContext::danger;
        creators["mop tjs switch to yulon"] = &MopTjsTriggerContext::switch_to_yulon;
        creators["mop tjs living water"] = &MopTjsTriggerContext::living_water;
        creators["mop tjs lorewalker"] = &MopTjsTriggerContext::lorewalker;
    }

private:
    static Trigger* danger(PlayerbotAI* botAI) { return new MopTjsDangerTrigger(botAI); }
    static Trigger* living_water(PlayerbotAI* botAI) { return new MopTjsLivingWaterTrigger(botAI); }
    static Trigger* lorewalker(PlayerbotAI* botAI) { return new MopTjsLorewalkerTrigger(botAI); }

    static Trigger* switch_to_yulon(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "mop tjs switch to yulon", MopTjs::YulonSwitch());
    }
};

#endif
