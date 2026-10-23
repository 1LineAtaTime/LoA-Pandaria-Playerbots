/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOACTIONCONTEXT_H
#define PLAYERBOTS_CATAHOOACTIONCONTEXT_H

#include "Action.h"
#include "CataHooActions.h"
#include "NamedObjectContext.h"

class CataDungeonHooActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonHooActionContext()
    {
        creators["hoo anhuur beacons"] = &CataDungeonHooActionContext::hoo_anhuur_beacons;
        creators["hoo anhuur pit return"] = &CataDungeonHooActionContext::hoo_anhuur_pit_return;
        creators["hoo anhuur searing light"] = &CataDungeonHooActionContext::hoo_anhuur_searing_light;
        creators["hoo anhuur reckoning"] = &CataDungeonHooActionContext::hoo_anhuur_reckoning;
        creators["hoo ptah quicksand"] = &CataDungeonHooActionContext::hoo_ptah_quicksand;
        creators["hoo anraphet alpha beam"] = &CataDungeonHooActionContext::hoo_anraphet_alpha_beam;
        creators["hoo isiset supernova"] = &CataDungeonHooActionContext::hoo_isiset_supernova;
        creators["hoo isiset image"] = &CataDungeonHooActionContext::hoo_isiset_image;
        creators["hoo ammunae pod"] = &CataDungeonHooActionContext::hoo_ammunae_pod;
        creators["hoo ammunae spore cloud"] = &CataDungeonHooActionContext::hoo_ammunae_spore_cloud;
        creators["hoo setesh priority"] = &CataDungeonHooActionContext::hoo_setesh_priority;
        creators["hoo rajh solar winds"] = &CataDungeonHooActionContext::hoo_rajh_solar_winds;
        creators["hoo setesh reign of chaos"] = &CataDungeonHooActionContext::hoo_setesh_reign_of_chaos;
        creators["hoo setesh chaos blast"] = &CataDungeonHooActionContext::hoo_setesh_chaos_blast;
        creators["hoo rajh sun orb"] = &CataDungeonHooActionContext::hoo_rajh_sun_orb;
        creators["hoo rajh inferno leap"] = &CataDungeonHooActionContext::hoo_rajh_inferno_leap;
    }

private:
    static Action* hoo_anhuur_beacons(PlayerbotAI* botAI) { return new HooAnhuurBeaconsAction(botAI); }
    static Action* hoo_anhuur_pit_return(PlayerbotAI* botAI) { return new HooAnhuurPitReturnAction(botAI); }
    static Action* hoo_anhuur_searing_light(PlayerbotAI* botAI) { return new HooAnhuurSearingLightAction(botAI); }
    static Action* hoo_anhuur_reckoning(PlayerbotAI* botAI) { return new HooAnhuurReckoningAction(botAI); }
    static Action* hoo_ptah_quicksand(PlayerbotAI* botAI) { return new HooPtahQuicksandAction(botAI); }
    static Action* hoo_anraphet_alpha_beam(PlayerbotAI* botAI) { return new HooAnraphetAlphaBeamAction(botAI); }
    static Action* hoo_isiset_supernova(PlayerbotAI* botAI) { return new HooIsisetSupernovaAction(botAI); }
    static Action* hoo_isiset_image(PlayerbotAI* botAI) { return new HooIsisetImageAction(botAI); }
    static Action* hoo_ammunae_pod(PlayerbotAI* botAI) { return new HooAmmunaePodAction(botAI); }
    static Action* hoo_ammunae_spore_cloud(PlayerbotAI* botAI) { return new HooAmmunaeSporeCloudAction(botAI); }
    static Action* hoo_setesh_priority(PlayerbotAI* botAI) { return new HooSeteshPriorityAction(botAI); }
    static Action* hoo_rajh_solar_winds(PlayerbotAI* botAI) { return new HooRajhSolarWindsAction(botAI); }
    static Action* hoo_setesh_reign_of_chaos(PlayerbotAI* botAI) { return new HooSeteshReignOfChaosAction(botAI); }
    static Action* hoo_setesh_chaos_blast(PlayerbotAI* botAI) { return new HooSeteshChaosBlastAction(botAI); }
    static Action* hoo_rajh_sun_orb(PlayerbotAI* botAI) { return new HooRajhSunOrbAction(botAI); }
    static Action* hoo_rajh_inferno_leap(PlayerbotAI* botAI) { return new HooRajhInfernoLeapAction(botAI); }
};

#endif
