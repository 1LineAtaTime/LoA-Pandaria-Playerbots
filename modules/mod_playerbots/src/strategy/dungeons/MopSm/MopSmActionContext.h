/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMACTIONCONTEXT_H
#define PLAYERBOTS_MOPSMACTIONCONTEXT_H

#include "Action.h"
#include "MopDungeonShared.h"
#include "MopSmActions.h"
#include "MopSmShared.h"
#include "NamedObjectContext.h"

class MopDungeonScarletMonasteryActionContext : public NamedObjectContext<Action>
{
public:
    MopDungeonScarletMonasteryActionContext()
    {
        creators["sm leave spirit gale"] = &MopDungeonScarletMonasteryActionContext::sm_leave_spirit_gale;
        creators["sm priority interrupt"] = &MopDungeonScarletMonasteryActionContext::sm_priority_interrupt;
        creators["sm focus add"] = &MopDungeonScarletMonasteryActionContext::sm_focus_add;
        creators["sm leave firestorm kick"] = &MopDungeonScarletMonasteryActionContext::sm_leave_firestorm_kick;
        creators["sm leave scorched earth"] = &MopDungeonScarletMonasteryActionContext::sm_leave_scorched_earth;
        creators["sm tank thalnos adds"] = &MopDungeonScarletMonasteryActionContext::sm_tank_thalnos_adds;
        creators["sm switch to whitemane"] = &MopDungeonScarletMonasteryActionContext::sm_switch_to_whitemane;
        creators["sm dispel whitemane shield"] = &MopDungeonScarletMonasteryActionContext::sm_dispel_whitemane_shield;
    }

private:
    static Action* sm_leave_spirit_gale(PlayerbotAI* botAI) { return new MopSmLeaveSpiritGaleAction(botAI); }
    static Action* sm_priority_interrupt(PlayerbotAI* botAI) { return new MopSmPriorityInterruptAction(botAI); }
    static Action* sm_focus_add(PlayerbotAI* botAI) { return new MopSmFocusAddAction(botAI); }
    static Action* sm_leave_firestorm_kick(PlayerbotAI* botAI) { return new MopSmLeaveFirestormAction(botAI); }
    static Action* sm_leave_scorched_earth(PlayerbotAI* botAI) { return new MopSmLeaveScorchedEarthAction(botAI); }
    static Action* sm_dispel_whitemane_shield(PlayerbotAI* botAI) { return new MopSmDispelWhitemaneShieldAction(botAI); }

    static Action* sm_tank_thalnos_adds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sm tank thalnos adds", MopScarletMonastery::ThalnosTankWave());
    }

    static Action* sm_switch_to_whitemane(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "sm switch to whitemane", MopScarletMonastery::WhitemaneSwitch());
    }
};

#endif
