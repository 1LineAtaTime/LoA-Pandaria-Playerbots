/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) action context, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSActionContext.h.

#ifndef PLAYERBOTS_CATAZAACTIONCONTEXT_H
#define PLAYERBOTS_CATAZAACTIONCONTEXT_H

#include "Action.h"
#include "CataZaActions.h"
#include "NamedObjectContext.h"

class CataDungeonZulAmanActionContext : public NamedObjectContext<Action>
{
public:
    CataDungeonZulAmanActionContext()
    {
        creators["za join storm eye"] = &CataDungeonZulAmanActionContext::join_storm_eye;
        creators["za static spread"] = &CataDungeonZulAmanActionContext::static_spread;
        creators["za surge close"] = &CataDungeonZulAmanActionContext::surge_close;
        creators["za escape fire bombs"] = &CataDungeonZulAmanActionContext::escape_fire_bombs;
        creators["za escape flame breath"] = &CataDungeonZulAmanActionContext::escape_flame_breath;
        creators["za leave water totem"] = &CataDungeonZulAmanActionContext::leave_water_totem;
        creators["za escape malacrass ground"] = &CataDungeonZulAmanActionContext::escape_malacrass_ground;
        creators["za malacrass interrupt"] = &CataDungeonZulAmanActionContext::malacrass_interrupt;
        creators["za escape pillar of flame"] = &CataDungeonZulAmanActionContext::escape_pillar_of_flame;
        creators["za escape whirlwind"] = &CataDungeonZulAmanActionContext::escape_whirlwind;
        creators["za escape feather cyclone"] = &CataDungeonZulAmanActionContext::escape_feather_cyclone;
        creators["za focus target"] = &CataDungeonZulAmanActionContext::focus_target;
    }

private:
    static Action* join_storm_eye(PlayerbotAI* ai) { return new CataZaJoinStormEyeAction(ai); }
    static Action* static_spread(PlayerbotAI* ai) { return new CataZaSpreadAction(ai); }
    static Action* surge_close(PlayerbotAI* ai) { return new CataZaSurgeCloseAction(ai); }
    static Action* escape_fire_bombs(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape fire bombs", CataZulAman::HazardKind::FireBombs);
    }
    static Action* escape_flame_breath(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape flame breath", CataZulAman::HazardKind::FlameBreath);
    }
    static Action* leave_water_totem(PlayerbotAI* ai) { return new CataZaLeaveWaterTotemAction(ai); }
    static Action* escape_malacrass_ground(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape malacrass ground", CataZulAman::HazardKind::MalacrassGround);
    }
    static Action* malacrass_interrupt(PlayerbotAI* ai) { return new CataZaInterruptAction(ai); }
    static Action* escape_pillar_of_flame(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape pillar of flame", CataZulAman::HazardKind::PillarOfFlame);
    }
    static Action* escape_whirlwind(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape whirlwind", CataZulAman::HazardKind::DaakaraWhirlwind);
    }
    static Action* escape_feather_cyclone(PlayerbotAI* ai)
    {
        return new CataZaEscapeAction(ai, "za escape feather cyclone", CataZulAman::HazardKind::FeatherCyclone);
    }
    static Action* focus_target(PlayerbotAI* ai) { return new CataZaFocusAction(ai); }
};

#endif
