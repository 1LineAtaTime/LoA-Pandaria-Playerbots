/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPACTIONCONTEXT_H
#define PLAYERBOTS_CATAVPACTIONCONTEXT_H

#include "Action.h"
#include "CataVpActions.h"
#include "NamedObjectContext.h"

class CataDungeonVpActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonVpActionContext()
    {
        creators["vp ertan ring"] = &CataDungeonVpActionContext::vp_ertan_ring;
        creators["vp altairus breath"] = &CataDungeonVpActionContext::vp_altairus_breath;
        creators["vp altairus twister"] = &CataDungeonVpActionContext::vp_altairus_twister;
        creators["vp altairus downwind"] = &CataDungeonVpActionContext::vp_altairus_downwind;
        creators["vp asaad stack"] = &CataDungeonVpActionContext::vp_asaad_stack;
        creators["vp slipstream"] = &CataDungeonVpActionContext::vp_slipstream;   // #692
    }

private:
    static Action* vp_ertan_ring(PlayerbotAI* botAI) { return new VpErtanRingAction(botAI); }
    static Action* vp_altairus_breath(PlayerbotAI* botAI) { return new VpAltairusBreathAction(botAI); }
    static Action* vp_altairus_twister(PlayerbotAI* botAI) { return new VpAltairusTwisterAction(botAI); }
    static Action* vp_altairus_downwind(PlayerbotAI* botAI) { return new VpAltairusDownwindAction(botAI); }
    static Action* vp_asaad_stack(PlayerbotAI* botAI) { return new VpAsaadStackAction(botAI); }
    static Action* vp_slipstream(PlayerbotAI* botAI) { return new VpSlipstreamAction(botAI); }
};

#endif
