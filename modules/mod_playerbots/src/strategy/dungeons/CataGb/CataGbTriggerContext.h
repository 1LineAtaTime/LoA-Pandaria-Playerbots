/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBTRIGGERCONTEXT_H
#define PLAYERBOTS_CATAGBTRIGGERCONTEXT_H

#include "CataGbTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonGbTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonGbTriggerContext()
    {
        creators["gb umbriss ground siege"] = &CataDungeonGbTriggerContext::gb_umbriss_ground_siege;
        creators["gb umbriss blitz"] = &CataDungeonGbTriggerContext::gb_umbriss_blitz;
        creators["gb throngus phalanx"] = &CataDungeonGbTriggerContext::gb_throngus_phalanx;
        creators["gb throngus cave in"] = &CataDungeonGbTriggerContext::gb_throngus_cave_in;
        creators["gb throngus fire patch"] = &CataDungeonGbTriggerContext::gb_throngus_fire_patch;
        creators["gb valiona breath"] = &CataDungeonGbTriggerContext::gb_valiona_breath;
        creators["gb drahga portal"] = &CataDungeonGbTriggerContext::gb_drahga_portal;
        creators["gb drahga kite"] = &CataDungeonGbTriggerContext::gb_drahga_kite;
        creators["gb drahga spirit"] = &CataDungeonGbTriggerContext::gb_drahga_spirit;
        creators["gb drahga seeping twilight"] = &CataDungeonGbTriggerContext::gb_drahga_seeping_twilight;
        creators["gb erudax gale"] = &CataDungeonGbTriggerContext::gb_erudax_gale;
        creators["gb erudax shadows"] = &CataDungeonGbTriggerContext::gb_erudax_shadows;
        creators["gb erudax corruptor"] = &CataDungeonGbTriggerContext::gb_erudax_corruptor;
    }

private:
    static Trigger* gb_umbriss_ground_siege(PlayerbotAI* botAI) { return new GbUmbrissGroundSiegeTrigger(botAI); }
    static Trigger* gb_umbriss_blitz(PlayerbotAI* botAI) { return new GbUmbrissBlitzTrigger(botAI); }
    static Trigger* gb_throngus_phalanx(PlayerbotAI* botAI) { return new GbThrongusPhalanxTrigger(botAI); }
    static Trigger* gb_throngus_cave_in(PlayerbotAI* botAI) { return new GbThrongusCaveInTrigger(botAI); }
    static Trigger* gb_throngus_fire_patch(PlayerbotAI* botAI) { return new GbThrongusFirePatchTrigger(botAI); }
    static Trigger* gb_valiona_breath(PlayerbotAI* botAI) { return new GbValionaBreathTrigger(botAI); }
    static Trigger* gb_drahga_portal(PlayerbotAI* botAI) { return new GbDrahgaPortalTrigger(botAI); }
    static Trigger* gb_drahga_kite(PlayerbotAI* botAI) { return new GbDrahgaKiteTrigger(botAI); }
    static Trigger* gb_drahga_spirit(PlayerbotAI* botAI) { return new GbDrahgaSpiritTrigger(botAI); }
    static Trigger* gb_drahga_seeping_twilight(PlayerbotAI* botAI) { return new GbDrahgaSeepingTwilightTrigger(botAI); }
    static Trigger* gb_erudax_gale(PlayerbotAI* botAI) { return new GbErudaxGaleTrigger(botAI); }
    static Trigger* gb_erudax_shadows(PlayerbotAI* botAI) { return new GbErudaxShadowsTrigger(botAI); }
    static Trigger* gb_erudax_corruptor(PlayerbotAI* botAI) { return new GbErudaxCorruptorTrigger(botAI); }
};

#endif
