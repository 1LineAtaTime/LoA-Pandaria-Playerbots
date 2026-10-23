/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) trigger context, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSTriggerContext.h.

#ifndef PLAYERBOTS_CATAZATRIGGERCONTEXT_H
#define PLAYERBOTS_CATAZATRIGGERCONTEXT_H

#include "CataZaTriggers.h"
#include "NamedObjectContext.h"

class CataDungeonZulAmanTriggerContext : public NamedObjectContext<Trigger>
{
public:
    CataDungeonZulAmanTriggerContext()
    {
        creators["za electrical storm"] = &CataDungeonZulAmanTriggerContext::electrical_storm;
        creators["za static spread"] = &CataDungeonZulAmanTriggerContext::static_spread;
        creators["za surge debuff"] = &CataDungeonZulAmanTriggerContext::surge_debuff;
        creators["za fire bombs"] = &CataDungeonZulAmanTriggerContext::fire_bombs;
        creators["za flame breath"] = &CataDungeonZulAmanTriggerContext::flame_breath;
        creators["za water totem"] = &CataDungeonZulAmanTriggerContext::water_totem;
        creators["za malacrass ground"] = &CataDungeonZulAmanTriggerContext::malacrass_ground;
        creators["za malacrass interrupt"] = &CataDungeonZulAmanTriggerContext::malacrass_interrupt;
        creators["za pillar of flame"] = &CataDungeonZulAmanTriggerContext::pillar_of_flame;
        creators["za whirlwind"] = &CataDungeonZulAmanTriggerContext::whirlwind;
        creators["za feather cyclone"] = &CataDungeonZulAmanTriggerContext::feather_cyclone;
        creators["za focus target"] = &CataDungeonZulAmanTriggerContext::focus_target;
    }

private:
    static Trigger* electrical_storm(PlayerbotAI* ai) { return new CataZaStormTrigger(ai); }
    static Trigger* static_spread(PlayerbotAI* ai) { return new CataZaSpreadTrigger(ai); }
    static Trigger* surge_debuff(PlayerbotAI* ai) { return new CataZaSurgeTrigger(ai); }
    static Trigger* fire_bombs(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za fire bombs", CataZulAman::HazardKind::FireBombs);
    }
    static Trigger* flame_breath(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za flame breath", CataZulAman::HazardKind::FlameBreath);
    }
    static Trigger* water_totem(PlayerbotAI* ai) { return new CataZaWaterTotemTrigger(ai); }
    static Trigger* malacrass_ground(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za malacrass ground", CataZulAman::HazardKind::MalacrassGround);
    }
    static Trigger* malacrass_interrupt(PlayerbotAI* ai) { return new CataZaInterruptTrigger(ai); }
    static Trigger* pillar_of_flame(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za pillar of flame", CataZulAman::HazardKind::PillarOfFlame);
    }
    static Trigger* whirlwind(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za whirlwind", CataZulAman::HazardKind::DaakaraWhirlwind);
    }
    static Trigger* feather_cyclone(PlayerbotAI* ai)
    {
        return new CataZaHazardTrigger(ai, "za feather cyclone", CataZulAman::HazardKind::FeatherCyclone);
    }
    static Trigger* focus_target(PlayerbotAI* ai) { return new CataZaFocusTrigger(ai); }
};

#endif
