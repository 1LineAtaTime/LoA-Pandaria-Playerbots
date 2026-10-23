/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple strategy "mop-snt" (epic #373, issue #656) - action context.

#ifndef PLAYERBOTS_MOPSNTACTIONCONTEXT_H
#define PLAYERBOTS_MOPSNTACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopSntActions.h"

class MopSntActionContext : public NamedObjectContext<Action>
{
public:
    MopSntActionContext()
    {
        creators["mop snt escape"] = &MopSntActionContext::mop_snt_escape;
        creators["mop snt neronok interrupt"] = &MopSntActionContext::mop_snt_neronok_interrupt;
        creators["mop snt focus target"] = &MopSntActionContext::mop_snt_focus_target;
        creators["mop snt siege bomb"] = &MopSntActionContext::mop_snt_siege_bomb;
        creators["mop snt tar keg"] = &MopSntActionContext::mop_snt_tar_keg;
        creators["mop snt soak puddle"] = &MopSntActionContext::mop_snt_soak_puddle;
        creators["mop snt tank reinforcements"] = &MopSntActionContext::mop_snt_tank_reinforcements;
        creators["mop snt resin"] = &MopSntActionContext::mop_snt_resin;
        creators["mop snt neronok approach"] = &MopSntActionContext::mop_snt_neronok_approach;
    }

private:
    static Action* mop_snt_escape(PlayerbotAI* botAI) { return new MopSntEscapeAction(botAI); }

    static Action* mop_snt_neronok_interrupt(PlayerbotAI* botAI)
    {
        return new MopSntInterruptAction(botAI, "mop snt neronok interrupt", MopSnt::InterruptGroup::Neronok);
    }

    static Action* mop_snt_focus_target(PlayerbotAI* botAI) { return new MopSntFocusAction(botAI); }
    static Action* mop_snt_siege_bomb(PlayerbotAI* botAI) { return new MopSntSiegeBombAction(botAI); }
    static Action* mop_snt_tar_keg(PlayerbotAI* botAI) { return new MopSntTarKegAction(botAI); }
    static Action* mop_snt_soak_puddle(PlayerbotAI* botAI) { return new MopSntSoakPuddleAction(botAI); }
    static Action* mop_snt_resin(PlayerbotAI* botAI) { return new MopSntResinHopAction(botAI); }
    static Action* mop_snt_neronok_approach(PlayerbotAI* botAI) { return new MopSntNeronokApproachAction(botAI); }

    static Action* mop_snt_tank_reinforcements(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop snt tank reinforcements", MopSnt::ReinforcementsWave());
    }
};

#endif
