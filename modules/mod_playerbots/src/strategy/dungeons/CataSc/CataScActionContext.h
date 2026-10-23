/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#ifndef PLAYERBOTS_CATASCACTIONCONTEXT_H
#define PLAYERBOTS_CATASCACTIONCONTEXT_H

#include "Action.h"
#include "CataScActions.h"
#include "NamedObjectContext.h"

class CataDungeonScActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonScActionContext()
    {
        creators["cata sc leave crystal barrage"] = &CataDungeonScActionContext::cata_sc_leave_crystal_barrage;
        creators["cata sc kill crystal shards"] = &CataDungeonScActionContext::cata_sc_kill_crystal_shards;
        creators["cata sc leave lava fissure"] = &CataDungeonScActionContext::cata_sc_leave_lava_fissure;
        creators["cata sc leave stalactite"] = &CataDungeonScActionContext::cata_sc_leave_stalactite;
        creators["cata sc leave sand blast"] = &CataDungeonScActionContext::cata_sc_leave_sand_blast;
        creators["cata sc flee shatter"] = &CataDungeonScActionContext::cata_sc_flee_shatter;
        creators["cata sc leave slam cone"] = &CataDungeonScActionContext::cata_sc_leave_slam_cone;
        creators["cata sc leave gravity well"] = &CataDungeonScActionContext::cata_sc_leave_gravity_well;
        creators["cata sc kill followers"] = &CataDungeonScActionContext::cata_sc_kill_followers;
    }

private:
    static Action* cata_sc_leave_crystal_barrage(PlayerbotAI* botAI)
    {
        return new CataScLeaveDynHazardAction(botAI, "cata sc leave crystal barrage",
                                              CataSc::DynHazardId::CRYSTAL_BARRAGE);
    }

    static Action* cata_sc_kill_crystal_shards(PlayerbotAI* botAI)
    {
        return new CataScFocusAction(botAI, "cata sc kill crystal shards", CataSc::FocusId::CRYSTAL_SHARDS);
    }

    static Action* cata_sc_leave_lava_fissure(PlayerbotAI* botAI)
    {
        return new CataScLeaveHazardAction(botAI, "cata sc leave lava fissure", CataSc::HazardId::LAVA_FISSURE);
    }

    // The stalactite is lethal-ish (19k + knockback) and lands on a timer, so it overrides movement under way
    static Action* cata_sc_leave_stalactite(PlayerbotAI* botAI)
    {
        return new CataScLeaveDynHazardAction(botAI, "cata sc leave stalactite", CataSc::DynHazardId::STALACTITE,
                                              MovementPriority::MOVEMENT_FORCED);
    }

    static Action* cata_sc_leave_sand_blast(PlayerbotAI* botAI) { return new CataScLeaveSandBlastAction(botAI); }
    static Action* cata_sc_flee_shatter(PlayerbotAI* botAI) { return new CataScFleeShatterAction(botAI); }
    static Action* cata_sc_leave_slam_cone(PlayerbotAI* botAI) { return new CataScLeaveSlamConeAction(botAI); }

    static Action* cata_sc_leave_gravity_well(PlayerbotAI* botAI)
    {
        return new CataScLeaveHazardAction(botAI, "cata sc leave gravity well", CataSc::HazardId::GRAVITY_WELL);
    }

    static Action* cata_sc_kill_followers(PlayerbotAI* botAI)
    {
        return new CataScFocusAction(botAI, "cata sc kill followers", CataSc::FocusId::AZIL_FOLLOWERS);
    }
};

#endif
