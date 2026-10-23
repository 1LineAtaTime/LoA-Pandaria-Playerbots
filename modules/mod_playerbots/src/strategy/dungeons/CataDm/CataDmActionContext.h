/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#ifndef PLAYERBOTS_CATADMACTIONCONTEXT_H
#define PLAYERBOTS_CATADMACTIONCONTEXT_H

#include "Action.h"
#include "CataDmActions.h"
#include "NamedObjectContext.h"

class CataDungeonDmActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonDmActionContext()
    {
        creators["dm leave blossom"] = &CataDungeonDmActionContext::dm_leave_blossom;
        creators["dm kill blossom"] = &CataDungeonDmActionContext::dm_kill_blossom;
        creators["dm leave sticky bomb"] = &CataDungeonDmActionContext::dm_leave_sticky_bomb;
        creators["dm spread chest bomb"] = &CataDungeonDmActionContext::dm_spread_chest_bomb;
        creators["dm attack helix"] = &CataDungeonDmActionContext::dm_attack_helix;
        creators["dm leave harvest"] = &CataDungeonDmActionContext::dm_leave_harvest;
        creators["dm leave overdrive"] = &CataDungeonDmActionContext::dm_leave_overdrive;
        creators["dm kill vapor"] = &CataDungeonDmActionContext::dm_kill_vapor;
        creators["dm kill vanessa add"] = &CataDungeonDmActionContext::dm_kill_vanessa_add;
        creators["dm dodge fire wall"] = &CataDungeonDmActionContext::dm_dodge_fire_wall;
        creators["dm avoid molten slag"] = &CataDungeonDmActionContext::dm_avoid_molten_slag;
        creators["dm kill molten slag"] = &CataDungeonDmActionContext::dm_kill_molten_slag;
        creators["dm attack reaper"] = &CataDungeonDmActionContext::dm_attack_reaper;
        creators["dm escape blaze"] = &CataDungeonDmActionContext::dm_escape_blaze;
        creators["dm leave powder keg"] = &CataDungeonDmActionContext::dm_leave_powder_keg;
        creators["dm click steam valve"] = &CataDungeonDmActionContext::dm_click_steam_valve;
        creators["dm escape nightmare hazard"] = &CataDungeonDmActionContext::dm_escape_nightmare_hazard;
        creators["dm save harrington"] = &CataDungeonDmActionContext::dm_save_harrington;
    }

private:
    static Action* dm_leave_blossom(PlayerbotAI* botAI) { return new DmLeaveBlossomAction(botAI); }
    static Action* dm_kill_blossom(PlayerbotAI* botAI) { return new DmKillBlossomAction(botAI); }
    static Action* dm_leave_sticky_bomb(PlayerbotAI* botAI) { return new DmLeaveStickyBombAction(botAI); }
    static Action* dm_spread_chest_bomb(PlayerbotAI* botAI) { return new DmSpreadChestBombAction(botAI); }
    static Action* dm_attack_helix(PlayerbotAI* botAI) { return new DmAttackHelixAction(botAI); }
    static Action* dm_leave_harvest(PlayerbotAI* botAI) { return new DmLeaveHarvestAction(botAI); }
    static Action* dm_leave_overdrive(PlayerbotAI* botAI) { return new DmLeaveOverdriveAction(botAI); }
    static Action* dm_kill_vapor(PlayerbotAI* botAI) { return new DmKillVaporAction(botAI); }
    static Action* dm_kill_vanessa_add(PlayerbotAI* botAI) { return new DmKillVanessaAddAction(botAI); }
    static Action* dm_dodge_fire_wall(PlayerbotAI* botAI) { return new DmDodgeFireWallAction(botAI); }
    static Action* dm_avoid_molten_slag(PlayerbotAI* botAI) { return new DmAvoidMoltenSlagAction(botAI); }
    static Action* dm_kill_molten_slag(PlayerbotAI* botAI) { return new DmKillMoltenSlagAction(botAI); }
    static Action* dm_attack_reaper(PlayerbotAI* botAI) { return new DmAttackReaperAction(botAI); }
    static Action* dm_escape_blaze(PlayerbotAI* botAI) { return new DmEscapeBlazeAction(botAI); }
    static Action* dm_leave_powder_keg(PlayerbotAI* botAI) { return new DmLeavePowderKegAction(botAI); }
    static Action* dm_click_steam_valve(PlayerbotAI* botAI) { return new DmClickSteamValveAction(botAI); }
    static Action* dm_escape_nightmare_hazard(PlayerbotAI* botAI) { return new DmEscapeNightmareHazardAction(botAI); }
    static Action* dm_save_harrington(PlayerbotAI* botAI) { return new DmSaveHarringtonAction(botAI); }
};

#endif
