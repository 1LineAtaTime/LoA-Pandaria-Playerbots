/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#ifndef PLAYERBOTS_CATATOTTACTIONCONTEXT_H
#define PLAYERBOTS_CATATOTTACTIONCONTEXT_H

#include "Action.h"
#include "CataTottActions.h"
#include "NamedObjectContext.h"

class CataDungeonTottActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonTottActionContext()
    {
        creators["cata tott leave geyser"] = &CataDungeonTottActionContext::cata_tott_leave_geyser;
        creators["cata tott leave vortex"] = &CataDungeonTottActionContext::cata_tott_leave_vortex;
        creators["cata tott spread surge"] = &CataDungeonTottActionContext::cata_tott_spread_surge;
        creators["cata tott kill nazjar adds"] = &CataDungeonTottActionContext::cata_tott_kill_nazjar_adds;
        creators["cata tott leave fissure"] = &CataDungeonTottActionContext::cata_tott_leave_fissure;
        creators["cata tott leave mind fog"] = &CataDungeonTottActionContext::cata_tott_leave_mind_fog;
        creators["cata tott leave earth shards"] = &CataDungeonTottActionContext::cata_tott_leave_earth_shards;
        creators["cata tott leave blight pool"] = &CataDungeonTottActionContext::cata_tott_leave_blight_pool;
        creators["cata tott kill sappers"] = &CataDungeonTottActionContext::cata_tott_kill_sappers;
        creators["cata tott kill neptulon adds"] = &CataDungeonTottActionContext::cata_tott_kill_neptulon_adds;
        creators["cata tott attack ozumat"] = &CataDungeonTottActionContext::cata_tott_attack_ozumat;
        creators["cata tott interrupt"] = &CataDungeonTottActionContext::cata_tott_interrupt;
    }

private:
    // Geyser: lethal (23k + knockback), so it overrides any movement that is already under way
    static Action* cata_tott_leave_geyser(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave geyser", CataTott::HazardId::GEYSER,
                                             MovementPriority::MOVEMENT_FORCED);
    }

    static Action* cata_tott_leave_vortex(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave vortex", CataTott::HazardId::VORTEX,
                                             MovementPriority::MOVEMENT_FORCED);
    }

    static Action* cata_tott_spread_surge(PlayerbotAI* botAI) { return new CataTottSpreadSurgeAction(botAI); }

    static Action* cata_tott_kill_nazjar_adds(PlayerbotAI* botAI)
    {
        return new CataTottFocusAction(botAI, "cata tott kill nazjar adds", CataTott::FocusId::NAZJAR_ADDS);
    }

    static Action* cata_tott_leave_fissure(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave fissure", CataTott::HazardId::FISSURE);
    }

    static Action* cata_tott_leave_mind_fog(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave mind fog", CataTott::HazardId::MIND_FOG);
    }

    static Action* cata_tott_leave_earth_shards(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave earth shards", CataTott::HazardId::EARTH_SHARDS);
    }

    static Action* cata_tott_leave_blight_pool(PlayerbotAI* botAI)
    {
        return new CataTottLeaveHazardAction(botAI, "cata tott leave blight pool", CataTott::HazardId::BLIGHT_POOL);
    }

    static Action* cata_tott_kill_sappers(PlayerbotAI* botAI)
    {
        return new CataTottFocusAction(botAI, "cata tott kill sappers", CataTott::FocusId::SAPPERS);
    }

    static Action* cata_tott_kill_neptulon_adds(PlayerbotAI* botAI)
    {
        return new CataTottFocusAction(botAI, "cata tott kill neptulon adds", CataTott::FocusId::NEPTULON_ADDS);
    }

    static Action* cata_tott_attack_ozumat(PlayerbotAI* botAI)
    {
        return new CataTottFocusAction(botAI, "cata tott attack ozumat", CataTott::FocusId::OZUMAT);
    }

    static Action* cata_tott_interrupt(PlayerbotAI* botAI) { return new CataTottInterruptAction(botAI); }
};

#endif
