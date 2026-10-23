/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAVPTRIGGERCONTEXT_H

#include "CataVpTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonVpTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonVpTriggerContext()
    {
        creators["vp ertan ring"] = &CataDungeonVpTriggerContext::vp_ertan_ring;
        creators["vp altairus breath"] = &CataDungeonVpTriggerContext::vp_altairus_breath;
        creators["vp altairus twister"] = &CataDungeonVpTriggerContext::vp_altairus_twister;
        creators["vp altairus downwind"] = &CataDungeonVpTriggerContext::vp_altairus_downwind;
        creators["vp asaad stack"] = &CataDungeonVpTriggerContext::vp_asaad_stack;
        creators["vp slipstream"] = &CataDungeonVpTriggerContext::vp_slipstream;   // #692
    }

private:
    static Trigger* vp_ertan_ring(PlayerbotAI* botAI) { return new VpErtanRingTrigger(botAI); }
    static Trigger* vp_altairus_breath(PlayerbotAI* botAI) { return new VpAltairusBreathTrigger(botAI); }
    static Trigger* vp_altairus_twister(PlayerbotAI* botAI) { return new VpAltairusTwisterTrigger(botAI); }
    static Trigger* vp_altairus_downwind(PlayerbotAI* botAI) { return new VpAltairusDownwindTrigger(botAI); }
    static Trigger* vp_asaad_stack(PlayerbotAI* botAI) { return new VpAsaadStackTrigger(botAI); }
    static Trigger* vp_slipstream(PlayerbotAI* botAI) { return new VpSlipstreamTrigger(botAI); }
};

#endif
