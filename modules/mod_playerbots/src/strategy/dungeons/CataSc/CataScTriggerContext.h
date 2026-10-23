/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#ifndef PLAYERBOTS_CATASCTRIGGERCONTEXT_H
#define PLAYERBOTS_CATASCTRIGGERCONTEXT_H

#include "CataScTriggers.h"
#include "NamedObjectContext.h"
#include "TriggerContext.h"

class CataDungeonScTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonScTriggerContext()
    {
        creators["cata sc crystal barrage"] = &CataDungeonScTriggerContext::cata_sc_crystal_barrage;
        creators["cata sc crystal shards"] = &CataDungeonScTriggerContext::cata_sc_crystal_shards;
        creators["cata sc lava fissure"] = &CataDungeonScTriggerContext::cata_sc_lava_fissure;
        creators["cata sc stalactite"] = &CataDungeonScTriggerContext::cata_sc_stalactite;
        creators["cata sc sand blast"] = &CataDungeonScTriggerContext::cata_sc_sand_blast;
        creators["cata sc shatter"] = &CataDungeonScTriggerContext::cata_sc_shatter;
        creators["cata sc ground slam"] = &CataDungeonScTriggerContext::cata_sc_ground_slam;
        creators["cata sc gravity well"] = &CataDungeonScTriggerContext::cata_sc_gravity_well;
        creators["cata sc azil followers"] = &CataDungeonScTriggerContext::cata_sc_azil_followers;
    }

private:
    static Trigger* cata_sc_crystal_barrage(PlayerbotAI* botAI)
    {
        return new CataScDynHazardTrigger(botAI, "cata sc crystal barrage", CataSc::DynHazardId::CRYSTAL_BARRAGE);
    }

    static Trigger* cata_sc_crystal_shards(PlayerbotAI* botAI)
    {
        return new CataScFocusTrigger(botAI, "cata sc crystal shards", CataSc::FocusId::CRYSTAL_SHARDS);
    }

    static Trigger* cata_sc_lava_fissure(PlayerbotAI* botAI)
    {
        return new CataScHazardTrigger(botAI, "cata sc lava fissure", CataSc::HazardId::LAVA_FISSURE);
    }

    static Trigger* cata_sc_stalactite(PlayerbotAI* botAI)
    {
        return new CataScDynHazardTrigger(botAI, "cata sc stalactite", CataSc::DynHazardId::STALACTITE);
    }

    static Trigger* cata_sc_sand_blast(PlayerbotAI* botAI) { return new CataScSandBlastTrigger(botAI); }
    static Trigger* cata_sc_shatter(PlayerbotAI* botAI) { return new CataScShatterTrigger(botAI); }
    static Trigger* cata_sc_ground_slam(PlayerbotAI* botAI) { return new CataScGroundSlamTrigger(botAI); }

    static Trigger* cata_sc_gravity_well(PlayerbotAI* botAI)
    {
        return new CataScHazardTrigger(botAI, "cata sc gravity well", CataSc::HazardId::GRAVITY_WELL);
    }

    static Trigger* cata_sc_azil_followers(PlayerbotAI* botAI)
    {
        return new CataScFocusTrigger(botAI, "cata sc azil followers", CataSc::FocusId::AZIL_FOLLOWERS);
    }
};

#endif
