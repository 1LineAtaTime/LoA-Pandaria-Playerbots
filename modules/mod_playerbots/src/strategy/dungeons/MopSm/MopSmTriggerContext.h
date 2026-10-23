/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#ifndef PLAYERBOTS_MOPSMTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPSMTRIGGERCONTEXT_H

#include "MopDungeonShared.h"
#include "MopSmShared.h"
#include "MopSmTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class MopDungeonScarletMonasteryTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopDungeonScarletMonasteryTriggerContext()
    {
        creators["sm in spirit gale"] = &MopDungeonScarletMonasteryTriggerContext::sm_in_spirit_gale;
        creators["sm priority interrupt"] = &MopDungeonScarletMonasteryTriggerContext::sm_priority_interrupt;
        creators["sm focus add"] = &MopDungeonScarletMonasteryTriggerContext::sm_focus_add;
        creators["sm firestorm kick"] = &MopDungeonScarletMonasteryTriggerContext::sm_firestorm_kick;
        creators["sm in scorched earth"] = &MopDungeonScarletMonasteryTriggerContext::sm_in_scorched_earth;
        creators["sm tank thalnos adds"] = &MopDungeonScarletMonasteryTriggerContext::sm_tank_thalnos_adds;
        creators["sm switch to whitemane"] = &MopDungeonScarletMonasteryTriggerContext::sm_switch_to_whitemane;
        creators["sm whitemane shield"] = &MopDungeonScarletMonasteryTriggerContext::sm_whitemane_shield;
    }

private:
    static Trigger* sm_in_spirit_gale(PlayerbotAI* botAI) { return new MopSmInSpiritGaleTrigger(botAI); }
    static Trigger* sm_priority_interrupt(PlayerbotAI* botAI) { return new MopSmPriorityInterruptTrigger(botAI); }
    static Trigger* sm_focus_add(PlayerbotAI* botAI) { return new MopSmFocusAddTrigger(botAI); }
    static Trigger* sm_firestorm_kick(PlayerbotAI* botAI) { return new MopSmFirestormKickTrigger(botAI); }
    static Trigger* sm_in_scorched_earth(PlayerbotAI* botAI) { return new MopSmScorchedEarthTrigger(botAI); }
    static Trigger* sm_whitemane_shield(PlayerbotAI* botAI) { return new MopSmWhitemaneShieldTrigger(botAI); }

    static Trigger* sm_tank_thalnos_adds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sm tank thalnos adds", MopScarletMonastery::ThalnosTankWave());
    }

    static Trigger* sm_switch_to_whitemane(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "sm switch to whitemane", MopScarletMonastery::WhitemaneSwitch());
    }
};

#endif
