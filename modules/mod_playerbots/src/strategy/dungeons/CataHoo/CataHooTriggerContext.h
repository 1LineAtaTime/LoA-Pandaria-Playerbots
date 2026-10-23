/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAHOOTRIGGERCONTEXT_H

#include "CataHooTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonHooTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonHooTriggerContext()
    {
        creators["hoo anhuur beacons"] = &CataDungeonHooTriggerContext::hoo_anhuur_beacons;
        creators["hoo anhuur pit return"] = &CataDungeonHooTriggerContext::hoo_anhuur_pit_return;
        creators["hoo anhuur searing light"] = &CataDungeonHooTriggerContext::hoo_anhuur_searing_light;
        creators["hoo anhuur reckoning"] = &CataDungeonHooTriggerContext::hoo_anhuur_reckoning;
        creators["hoo ptah quicksand"] = &CataDungeonHooTriggerContext::hoo_ptah_quicksand;
        creators["hoo anraphet alpha beam"] = &CataDungeonHooTriggerContext::hoo_anraphet_alpha_beam;
        creators["hoo isiset supernova"] = &CataDungeonHooTriggerContext::hoo_isiset_supernova;
        creators["hoo isiset image"] = &CataDungeonHooTriggerContext::hoo_isiset_image;
        creators["hoo ammunae pod"] = &CataDungeonHooTriggerContext::hoo_ammunae_pod;
        creators["hoo ammunae spore cloud"] = &CataDungeonHooTriggerContext::hoo_ammunae_spore_cloud;
        creators["hoo setesh priority"] = &CataDungeonHooTriggerContext::hoo_setesh_priority;
        creators["hoo rajh solar winds"] = &CataDungeonHooTriggerContext::hoo_rajh_solar_winds;
        creators["hoo setesh reign of chaos"] = &CataDungeonHooTriggerContext::hoo_setesh_reign_of_chaos;
        creators["hoo setesh chaos blast"] = &CataDungeonHooTriggerContext::hoo_setesh_chaos_blast;
        creators["hoo rajh sun orb"] = &CataDungeonHooTriggerContext::hoo_rajh_sun_orb;
        creators["hoo rajh inferno leap"] = &CataDungeonHooTriggerContext::hoo_rajh_inferno_leap;
    }

private:
    static Trigger* hoo_anhuur_beacons(PlayerbotAI* botAI) { return new HooAnhuurBeaconsTrigger(botAI); }
    static Trigger* hoo_anhuur_pit_return(PlayerbotAI* botAI) { return new HooAnhuurPitReturnTrigger(botAI); }
    static Trigger* hoo_anhuur_searing_light(PlayerbotAI* botAI) { return new HooAnhuurSearingLightTrigger(botAI); }
    static Trigger* hoo_anhuur_reckoning(PlayerbotAI* botAI) { return new HooAnhuurReckoningTrigger(botAI); }
    static Trigger* hoo_ptah_quicksand(PlayerbotAI* botAI) { return new HooPtahQuicksandTrigger(botAI); }
    static Trigger* hoo_anraphet_alpha_beam(PlayerbotAI* botAI) { return new HooAnraphetAlphaBeamTrigger(botAI); }
    static Trigger* hoo_isiset_supernova(PlayerbotAI* botAI) { return new HooIsisetSupernovaTrigger(botAI); }
    static Trigger* hoo_isiset_image(PlayerbotAI* botAI) { return new HooIsisetImageTrigger(botAI); }
    static Trigger* hoo_ammunae_pod(PlayerbotAI* botAI) { return new HooAmmunaePodTrigger(botAI); }
    static Trigger* hoo_ammunae_spore_cloud(PlayerbotAI* botAI) { return new HooAmmunaeSporeCloudTrigger(botAI); }
    static Trigger* hoo_setesh_priority(PlayerbotAI* botAI) { return new HooSeteshPriorityTrigger(botAI); }
    static Trigger* hoo_rajh_solar_winds(PlayerbotAI* botAI) { return new HooRajhSolarWindsTrigger(botAI); }
    static Trigger* hoo_setesh_reign_of_chaos(PlayerbotAI* botAI) { return new HooSeteshReignOfChaosTrigger(botAI); }
    static Trigger* hoo_setesh_chaos_blast(PlayerbotAI* botAI) { return new HooSeteshChaosBlastTrigger(botAI); }
    static Trigger* hoo_rajh_sun_orb(PlayerbotAI* botAI) { return new HooRajhSunOrbTrigger(botAI); }
    static Trigger* hoo_rajh_inferno_leap(PlayerbotAI* botAI) { return new HooRajhInfernoLeapTrigger(botAI); }
};

#endif
