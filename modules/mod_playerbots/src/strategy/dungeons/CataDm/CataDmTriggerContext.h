/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#ifndef PLAYERBOTS_CATADMTRIGGERCONTEXT_H
#define PLAYERBOTS_CATADMTRIGGERCONTEXT_H

#include "NamedObjectContext.h"
#include "CataDmTriggers.h"

class CataDungeonDmTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonDmTriggerContext()
    {
        creators["dm blossom landing"] = &CataDungeonDmTriggerContext::dm_blossom_landing;
        creators["dm blossom add"] = &CataDungeonDmTriggerContext::dm_blossom_add;
        creators["dm sticky bomb"] = &CataDungeonDmTriggerContext::dm_sticky_bomb;
        creators["dm chest bomb"] = &CataDungeonDmTriggerContext::dm_chest_bomb;
        creators["dm helix focus"] = &CataDungeonDmTriggerContext::dm_helix_focus;
        creators["dm harvest"] = &CataDungeonDmTriggerContext::dm_harvest;
        creators["dm overdrive"] = &CataDungeonDmTriggerContext::dm_overdrive;
        creators["dm vapor"] = &CataDungeonDmTriggerContext::dm_vapor;
        creators["dm vanessa add"] = &CataDungeonDmTriggerContext::dm_vanessa_add;
        creators["dm fire wall"] = &CataDungeonDmTriggerContext::dm_fire_wall;
        creators["dm molten slag"] = &CataDungeonDmTriggerContext::dm_molten_slag;
        creators["dm slag add"] = &CataDungeonDmTriggerContext::dm_slag_add;
        creators["dm reaper focus"] = &CataDungeonDmTriggerContext::dm_reaper_focus;
        creators["dm vanessa blaze"] = &CataDungeonDmTriggerContext::dm_vanessa_blaze;
        creators["dm powder keg"] = &CataDungeonDmTriggerContext::dm_powder_keg;
        creators["dm steam valve"] = &CataDungeonDmTriggerContext::dm_steam_valve;
        creators["dm nightmare hazard"] = &CataDungeonDmTriggerContext::dm_nightmare_hazard;
        creators["dm harrington"] = &CataDungeonDmTriggerContext::dm_harrington;
    }

private:
    static Trigger* dm_blossom_landing(PlayerbotAI* botAI) { return new DmBlossomLandingTrigger(botAI); }
    static Trigger* dm_blossom_add(PlayerbotAI* botAI) { return new DmBlossomAddTrigger(botAI); }
    static Trigger* dm_sticky_bomb(PlayerbotAI* botAI) { return new DmStickyBombTrigger(botAI); }
    static Trigger* dm_chest_bomb(PlayerbotAI* botAI) { return new DmChestBombTrigger(botAI); }
    static Trigger* dm_helix_focus(PlayerbotAI* botAI) { return new DmHelixFocusTrigger(botAI); }
    static Trigger* dm_harvest(PlayerbotAI* botAI) { return new DmHarvestTrigger(botAI); }
    static Trigger* dm_overdrive(PlayerbotAI* botAI) { return new DmOverdriveTrigger(botAI); }
    static Trigger* dm_vapor(PlayerbotAI* botAI) { return new DmVaporTrigger(botAI); }
    static Trigger* dm_vanessa_add(PlayerbotAI* botAI) { return new DmVanessaAddTrigger(botAI); }
    static Trigger* dm_fire_wall(PlayerbotAI* botAI) { return new DmFireWallTrigger(botAI); }
    static Trigger* dm_molten_slag(PlayerbotAI* botAI) { return new DmMoltenSlagTrigger(botAI); }
    static Trigger* dm_slag_add(PlayerbotAI* botAI) { return new DmSlagAddTrigger(botAI); }
    static Trigger* dm_reaper_focus(PlayerbotAI* botAI) { return new DmReaperFocusTrigger(botAI); }
    static Trigger* dm_vanessa_blaze(PlayerbotAI* botAI) { return new DmVanessaBlazeTrigger(botAI); }
    static Trigger* dm_powder_keg(PlayerbotAI* botAI) { return new DmPowderKegTrigger(botAI); }
    static Trigger* dm_steam_valve(PlayerbotAI* botAI) { return new DmSteamValveTrigger(botAI); }
    static Trigger* dm_nightmare_hazard(PlayerbotAI* botAI) { return new DmNightmareHazardTrigger(botAI); }
    static Trigger* dm_harrington(PlayerbotAI* botAI) { return new DmHarringtonTrigger(botAI); }
};

#endif
