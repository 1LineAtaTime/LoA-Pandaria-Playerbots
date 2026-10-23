/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTTRIGGERCONTEXT_H
#define PLAYERBOTS_CATATOTTTRIGGERCONTEXT_H

#include "CataTottTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class CataDungeonTottTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonTottTriggerContext()
    {
        creators["cata tott geyser"] = &CataDungeonTottTriggerContext::cata_tott_geyser;
        creators["cata tott vortex"] = &CataDungeonTottTriggerContext::cata_tott_vortex;
        creators["cata tott lightning surge"] = &CataDungeonTottTriggerContext::cata_tott_lightning_surge;
        creators["cata tott nazjar adds"] = &CataDungeonTottTriggerContext::cata_tott_nazjar_adds;
        creators["cata tott fissure"] = &CataDungeonTottTriggerContext::cata_tott_fissure;
        creators["cata tott mind fog"] = &CataDungeonTottTriggerContext::cata_tott_mind_fog;
        creators["cata tott earth shards"] = &CataDungeonTottTriggerContext::cata_tott_earth_shards;
        creators["cata tott blight pool"] = &CataDungeonTottTriggerContext::cata_tott_blight_pool;
        creators["cata tott sappers"] = &CataDungeonTottTriggerContext::cata_tott_sappers;
        creators["cata tott neptulon adds"] = &CataDungeonTottTriggerContext::cata_tott_neptulon_adds;
        creators["cata tott ozumat"] = &CataDungeonTottTriggerContext::cata_tott_ozumat;
        creators["cata tott interrupt"] = &CataDungeonTottTriggerContext::cata_tott_interrupt;
    }

private:
    static Trigger* cata_tott_geyser(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott geyser", CataTott::HazardId::GEYSER);
    }

    static Trigger* cata_tott_vortex(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott vortex", CataTott::HazardId::VORTEX);
    }

    static Trigger* cata_tott_lightning_surge(PlayerbotAI* botAI) { return new CataTottSurgeTrigger(botAI); }

    static Trigger* cata_tott_nazjar_adds(PlayerbotAI* botAI)
    {
        return new CataTottFocusTrigger(botAI, "cata tott nazjar adds", CataTott::FocusId::NAZJAR_ADDS);
    }

    static Trigger* cata_tott_fissure(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott fissure", CataTott::HazardId::FISSURE);
    }

    static Trigger* cata_tott_mind_fog(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott mind fog", CataTott::HazardId::MIND_FOG);
    }

    static Trigger* cata_tott_earth_shards(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott earth shards", CataTott::HazardId::EARTH_SHARDS);
    }

    static Trigger* cata_tott_blight_pool(PlayerbotAI* botAI)
    {
        return new CataTottHazardTrigger(botAI, "cata tott blight pool", CataTott::HazardId::BLIGHT_POOL);
    }

    static Trigger* cata_tott_sappers(PlayerbotAI* botAI)
    {
        return new CataTottFocusTrigger(botAI, "cata tott sappers", CataTott::FocusId::SAPPERS);
    }

    static Trigger* cata_tott_neptulon_adds(PlayerbotAI* botAI)
    {
        return new CataTottFocusTrigger(botAI, "cata tott neptulon adds", CataTott::FocusId::NEPTULON_ADDS);
    }

    static Trigger* cata_tott_ozumat(PlayerbotAI* botAI)
    {
        return new CataTottFocusTrigger(botAI, "cata tott ozumat", CataTott::FocusId::OZUMAT);
    }

    static Trigger* cata_tott_interrupt(PlayerbotAI* botAI) { return new CataTottInterruptTrigger(botAI); }
};

#endif
