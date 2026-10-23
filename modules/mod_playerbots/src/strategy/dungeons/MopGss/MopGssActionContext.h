/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - action context.

#ifndef PLAYERBOTS_MOPGSSACTIONCONTEXT_H
#define PLAYERBOTS_MOPGSSACTIONCONTEXT_H

#include "Action.h"
#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopGssActions.h"
#include "MopGssShared.h"

class MopGssActionContext : public NamedObjectContext<Action>
{
public:
    MopGssActionContext()
    {
        creators["mop gss escape"] = &MopGssActionContext::mop_gss_escape;
        creators["mop gss saboteur interrupt"] = &MopGssActionContext::mop_gss_saboteur_interrupt;
        creators["mop gss swarm interrupt"] = &MopGssActionContext::mop_gss_swarm_interrupt;
        creators["mop gss focus target"] = &MopGssActionContext::mop_gss_focus_target;
        creators["mop gss strikers"] = &MopGssActionContext::mop_gss_strikers;
        creators["mop gss tank strikers"] = &MopGssActionContext::mop_gss_tank_strikers;
        creators["mop gss tank fluid"] = &MopGssActionContext::mop_gss_tank_fluid;
        creators["mop gss cannon"] = &MopGssActionContext::mop_gss_cannon;
        creators["mop gss flak"] = &MopGssActionContext::mop_gss_flak;
        creators["mop gss tank rimok adds"] = &MopGssActionContext::mop_gss_tank_rimok_adds;
        creators["mop gss tank raigonn adds"] = &MopGssActionContext::mop_gss_tank_raigonn_adds;
    }

private:
    static Action* mop_gss_escape(PlayerbotAI* botAI) { return new MopGssEscapeAction(botAI); }

    static Action* mop_gss_saboteur_interrupt(PlayerbotAI* botAI)
    {
        return new MopGssInterruptAction(botAI, "mop gss saboteur interrupt", MopGss::InterruptGroup::Saboteur);
    }

    static Action* mop_gss_swarm_interrupt(PlayerbotAI* botAI)
    {
        return new MopGssInterruptAction(botAI, "mop gss swarm interrupt", MopGss::InterruptGroup::SwarmBringer);
    }

    static Action* mop_gss_focus_target(PlayerbotAI* botAI) { return new MopGssFocusAction(botAI); }

    static Action* mop_gss_strikers(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop gss strikers", MopGss::StrikerWave(MopDungeon::WaveRole::Dps));
    }

    static Action* mop_gss_tank_strikers(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop gss tank strikers",
                                             MopGss::StrikerWave(MopDungeon::WaveRole::Tank));
    }

    static Action* mop_gss_tank_fluid(PlayerbotAI* botAI) { return new MopGssTankFluidAction(botAI); }

    static Action* mop_gss_cannon(PlayerbotAI* botAI) { return new MopGssCannonAction(botAI); }

    static Action* mop_gss_flak(PlayerbotAI* botAI) { return new MopGssFlakAction(botAI); }

    static Action* mop_gss_tank_rimok_adds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop gss tank rimok adds",
                                             MopGss::RimokAddWave(MopDungeon::WaveRole::Tank));
    }

    static Action* mop_gss_tank_raigonn_adds(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveAction(botAI, "mop gss tank raigonn adds",
                                             MopGss::RaigonnAddWave(MopDungeon::WaveRole::Tank));
    }
};

#endif
