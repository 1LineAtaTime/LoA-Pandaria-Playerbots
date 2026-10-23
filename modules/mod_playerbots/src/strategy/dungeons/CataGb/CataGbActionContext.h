/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBACTIONCONTEXT_H
#define PLAYERBOTS_CATAGBACTIONCONTEXT_H

#include "Action.h"
#include "CataGbActions.h"
#include "NamedObjectContext.h"

class CataDungeonGbActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonGbActionContext()
    {
        creators["gb umbriss ground siege"] = &CataDungeonGbActionContext::gb_umbriss_ground_siege;
        creators["gb umbriss blitz"] = &CataDungeonGbActionContext::gb_umbriss_blitz;
        creators["gb throngus phalanx"] = &CataDungeonGbActionContext::gb_throngus_phalanx;
        creators["gb throngus cave in"] = &CataDungeonGbActionContext::gb_throngus_cave_in;
        creators["gb throngus fire patch"] = &CataDungeonGbActionContext::gb_throngus_fire_patch;
        creators["gb valiona breath"] = &CataDungeonGbActionContext::gb_valiona_breath;
        creators["gb drahga portal"] = &CataDungeonGbActionContext::gb_drahga_portal;
        creators["gb drahga kite"] = &CataDungeonGbActionContext::gb_drahga_kite;
        creators["gb drahga spirit"] = &CataDungeonGbActionContext::gb_drahga_spirit;
        creators["gb drahga seeping twilight"] = &CataDungeonGbActionContext::gb_drahga_seeping_twilight;
        creators["gb erudax gale"] = &CataDungeonGbActionContext::gb_erudax_gale;
        creators["gb erudax shadows"] = &CataDungeonGbActionContext::gb_erudax_shadows;
        creators["gb erudax corruptor"] = &CataDungeonGbActionContext::gb_erudax_corruptor;
    }

private:
    static Action* gb_umbriss_ground_siege(PlayerbotAI* botAI) { return new GbUmbrissGroundSiegeAction(botAI); }
    static Action* gb_umbriss_blitz(PlayerbotAI* botAI) { return new GbUmbrissBlitzAction(botAI); }
    static Action* gb_throngus_phalanx(PlayerbotAI* botAI) { return new GbThrongusPhalanxAction(botAI); }
    static Action* gb_throngus_cave_in(PlayerbotAI* botAI) { return new GbThrongusCaveInAction(botAI); }
    static Action* gb_throngus_fire_patch(PlayerbotAI* botAI) { return new GbThrongusFirePatchAction(botAI); }
    static Action* gb_valiona_breath(PlayerbotAI* botAI) { return new GbValionaBreathAction(botAI); }
    static Action* gb_drahga_portal(PlayerbotAI* botAI) { return new GbDrahgaPortalAction(botAI); }
    static Action* gb_drahga_kite(PlayerbotAI* botAI) { return new GbDrahgaKiteAction(botAI); }
    static Action* gb_drahga_spirit(PlayerbotAI* botAI) { return new GbDrahgaSpiritAction(botAI); }
    static Action* gb_drahga_seeping_twilight(PlayerbotAI* botAI) { return new GbDrahgaSeepingTwilightAction(botAI); }
    static Action* gb_erudax_gale(PlayerbotAI* botAI) { return new GbErudaxGaleAction(botAI); }
    static Action* gb_erudax_shadows(PlayerbotAI* botAI) { return new GbErudaxShadowsAction(botAI); }
    static Action* gb_erudax_corruptor(PlayerbotAI* botAI) { return new GbErudaxCorruptorAction(botAI); }
};

#endif
