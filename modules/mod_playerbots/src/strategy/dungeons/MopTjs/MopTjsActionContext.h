/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs action context (epic #373, issue #651).

#ifndef PLAYERBOTS_MOPTJSACTIONCONTEXT_H
#define PLAYERBOTS_MOPTJSACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopTjsActions.h"
#include "MopTjsShared.h"

class MopTjsActionContext : public NamedObjectContext<Action>
{
public:
    MopTjsActionContext()
    {
        creators["mop tjs avoid"] = &MopTjsActionContext::avoid;
        creators["mop tjs switch to yulon"] = &MopTjsActionContext::switch_to_yulon;
        creators["mop tjs attack living water"] = &MopTjsActionContext::attack_living_water;
        creators["mop tjs attack lorewalker"] = &MopTjsActionContext::attack_lorewalker;
    }

private:
    static Action* avoid(PlayerbotAI* botAI) { return new MopTjsAvoidAction(botAI); }
    static Action* attack_living_water(PlayerbotAI* botAI) { return new MopTjsAttackLivingWaterAction(botAI); }
    static Action* attack_lorewalker(PlayerbotAI* botAI) { return new MopTjsAttackLorewalkerAction(botAI); }

    static Action* switch_to_yulon(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop tjs switch to yulon", MopTjs::YulonSwitch());
    }
};

#endif
