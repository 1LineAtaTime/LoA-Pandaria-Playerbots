/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#ifndef PLAYERBOTS_CATALCTACTIONCONTEXT_H
#define PLAYERBOTS_CATALCTACTIONCONTEXT_H

#include "Action.h"
#include "CataLctActions.h"
#include "NamedObjectContext.h"

class CataDungeonLctActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonLctActionContext()
    {
        creators["lct avoid land mine"] = &CataDungeonLctActionContext::lct_avoid_land_mine;
        creators["lct avoid shockwave"] = &CataDungeonLctActionContext::lct_avoid_shockwave;
        creators["lct drop from wall"] = &CataDungeonLctActionContext::lct_drop_from_wall;
        creators["lct return to platform"] = &CataDungeonLctActionContext::lct_return_to_platform;
        creators["lct leave dust flail"] = &CataDungeonLctActionContext::lct_leave_dust_flail;
        creators["lct kill crocolisk"] = &CataDungeonLctActionContext::lct_kill_crocolisk;
        creators["lct avoid heavens fury"] = &CataDungeonLctActionContext::lct_avoid_heavens_fury;
        creators["lct kill soul fragment"] = &CataDungeonLctActionContext::lct_kill_soul_fragment;
        creators["lct avoid hallowed ground"] = &CataDungeonLctActionContext::lct_avoid_hallowed_ground;
        creators["lct kill harbinger"] = &CataDungeonLctActionContext::lct_kill_harbinger;
        creators["lct avoid blaze fire"] = &CataDungeonLctActionContext::lct_avoid_blaze_fire;
        creators["lct avoid siamat storm"] = &CataDungeonLctActionContext::lct_avoid_siamat_storm;
        creators["lct kill servant"] = &CataDungeonLctActionContext::lct_kill_servant;
        creators["lct avoid lightning charge"] = &CataDungeonLctActionContext::lct_avoid_lightning_charge;
    }

private:
    static Action* lct_avoid_land_mine(PlayerbotAI* botAI) { return new LctAvoidLandMineAction(botAI); }
    static Action* lct_avoid_shockwave(PlayerbotAI* botAI) { return new LctAvoidShockwaveAction(botAI); }
    static Action* lct_drop_from_wall(PlayerbotAI* botAI) { return new LctDropFromWallAction(botAI); }
    static Action* lct_return_to_platform(PlayerbotAI* botAI) { return new LctReturnToPlatformAction(botAI); }
    static Action* lct_leave_dust_flail(PlayerbotAI* botAI) { return new LctLeaveDustFlailAction(botAI); }
    static Action* lct_kill_crocolisk(PlayerbotAI* botAI) { return new LctKillCrocoliskAction(botAI); }
    static Action* lct_avoid_heavens_fury(PlayerbotAI* botAI) { return new LctAvoidHeavensFuryAction(botAI); }
    static Action* lct_kill_soul_fragment(PlayerbotAI* botAI) { return new LctKillSoulFragmentAction(botAI); }
    static Action* lct_avoid_hallowed_ground(PlayerbotAI* botAI) { return new LctAvoidHallowedGroundAction(botAI); }
    static Action* lct_kill_harbinger(PlayerbotAI* botAI) { return new LctKillHarbingerAction(botAI); }
    static Action* lct_avoid_blaze_fire(PlayerbotAI* botAI) { return new LctAvoidBlazeFireAction(botAI); }
    static Action* lct_avoid_siamat_storm(PlayerbotAI* botAI) { return new LctAvoidSiamatStormAction(botAI); }
    static Action* lct_kill_servant(PlayerbotAI* botAI) { return new LctKillServantAction(botAI); }
    static Action* lct_avoid_lightning_charge(PlayerbotAI* botAI) { return new LctAvoidLightningChargeAction(botAI); }
};

#endif
