/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - trigger context.

#ifndef PLAYERBOTS_MOPGSSTRIGGERCONTEXT_H
#define PLAYERBOTS_MOPGSSTRIGGERCONTEXT_H

#include "AiObjectContext.h"
#include "MopDungeonShared.h"
#include "MopGssTriggers.h"
#include "TriggerContext.h"

class MopGssTriggerContext : public NamedObjectContext<Trigger>
{
public:
    MopGssTriggerContext()
    {
        creators["mop gss sabotage self"] = &MopGssTriggerContext::mop_gss_sabotage_self;
        creators["mop gss sabotage cross"] = &MopGssTriggerContext::mop_gss_sabotage_cross;
        creators["mop gss munition lane"] = &MopGssTriggerContext::mop_gss_munition_lane;
        creators["mop gss strafe"] = &MopGssTriggerContext::mop_gss_strafe;
        creators["mop gss frenzy"] = &MopGssTriggerContext::mop_gss_frenzy;
        creators["mop gss fixate"] = &MopGssTriggerContext::mop_gss_fixate;
        creators["mop gss winds"] = &MopGssTriggerContext::mop_gss_winds;
        creators["mop gss saboteur interrupt"] = &MopGssTriggerContext::mop_gss_saboteur_interrupt;
        creators["mop gss swarm interrupt"] = &MopGssTriggerContext::mop_gss_swarm_interrupt;
        creators["mop gss focus target"] = &MopGssTriggerContext::mop_gss_focus_target;
        creators["mop gss world in flames"] = &MopGssTriggerContext::mop_gss_world_in_flames;
        creators["mop gss strikers"] = &MopGssTriggerContext::mop_gss_strikers;
        creators["mop gss tank strikers"] = &MopGssTriggerContext::mop_gss_tank_strikers;
        creators["mop gss fluid"] = &MopGssTriggerContext::mop_gss_fluid;
        creators["mop gss headbutt"] = &MopGssTriggerContext::mop_gss_headbutt;
        creators["mop gss bomb"] = &MopGssTriggerContext::mop_gss_bomb;
        creators["mop gss tank fluid"] = &MopGssTriggerContext::mop_gss_tank_fluid;
        creators["mop gss cannon"] = &MopGssTriggerContext::mop_gss_cannon;
        creators["mop gss flak"] = &MopGssTriggerContext::mop_gss_flak;
        creators["mop gss tank rimok adds"] = &MopGssTriggerContext::mop_gss_tank_rimok_adds;
        creators["mop gss tank raigonn adds"] = &MopGssTriggerContext::mop_gss_tank_raigonn_adds;
    }

private:
    static Trigger* mop_gss_sabotage_self(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss sabotage self", MopGss::HazardKind::SabotageSelf);
    }

    static Trigger* mop_gss_sabotage_cross(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss sabotage cross", MopGss::HazardKind::SabotageCross);
    }

    static Trigger* mop_gss_munition_lane(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss munition lane", MopGss::HazardKind::MunitionLane);
    }

    static Trigger* mop_gss_strafe(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss strafe", MopGss::HazardKind::Strafe);
    }

    static Trigger* mop_gss_frenzy(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss frenzy", MopGss::HazardKind::Frenzy);
    }

    static Trigger* mop_gss_fixate(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss fixate", MopGss::HazardKind::Fixate);
    }

    static Trigger* mop_gss_winds(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss winds", MopGss::HazardKind::Winds);
    }

    static Trigger* mop_gss_saboteur_interrupt(PlayerbotAI* botAI)
    {
        return new MopGssInterruptTrigger(botAI, "mop gss saboteur interrupt", MopGss::InterruptGroup::Saboteur);
    }

    static Trigger* mop_gss_swarm_interrupt(PlayerbotAI* botAI)
    {
        return new MopGssInterruptTrigger(botAI, "mop gss swarm interrupt", MopGss::InterruptGroup::SwarmBringer);
    }

    static Trigger* mop_gss_focus_target(PlayerbotAI* botAI) { return new MopGssFocusTrigger(botAI); }

    static Trigger* mop_gss_world_in_flames(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss world in flames", MopGss::HazardKind::WorldInFlames);
    }

    static Trigger* mop_gss_strikers(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "mop gss strikers", MopGss::StrikerWave(MopDungeon::WaveRole::Dps));
    }

    static Trigger* mop_gss_tank_strikers(PlayerbotAI* botAI)
    {
        return new MopDungeon::MopWaveTrigger(botAI, "mop gss tank strikers",
                                              MopGss::StrikerWave(MopDungeon::WaveRole::Tank));
    }

    static Trigger* mop_gss_fluid(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss fluid", MopGss::HazardKind::Fluid);
    }

    static Trigger* mop_gss_headbutt(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss headbutt", MopGss::HazardKind::Headbutt);
    }

    static Trigger* mop_gss_bomb(PlayerbotAI* botAI)
    {
        return new MopGssHazardTrigger(botAI, "mop gss bomb", MopGss::HazardKind::Bomb);
    }

    static Trigger* mop_gss_tank_fluid(PlayerbotAI* botAI) { return new MopGssTankFluidTrigger(botAI); }

    static Trigger* mop_gss_cannon(PlayerbotAI* botAI) { return new MopGssCannonTrigger(botAI); }

    static Trigger* mop_gss_flak(PlayerbotAI* botAI) { return new MopGssFlakTrigger(botAI); }

    static Trigger* mop_gss_tank_rimok_adds(PlayerbotAI* botAI)
    {
        return new MopGssTankAddsTrigger(botAI, "mop gss tank rimok adds", MopGss::RimokAddWave(MopDungeon::WaveRole::Tank),
                                         MopGss::GSS_NPC_RIMOK);
    }

    static Trigger* mop_gss_tank_raigonn_adds(PlayerbotAI* botAI)
    {
        return new MopGssTankAddsTrigger(botAI, "mop gss tank raigonn adds",
                                         MopGss::RaigonnAddWave(MopDungeon::WaveRole::Tank), MopGss::GSS_NPC_RAIGONN);
    }
};

#endif
