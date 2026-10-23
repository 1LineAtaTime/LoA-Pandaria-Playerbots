/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) strategy "cata-za", MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSStrategy.cpp
// (trigger -> action per mechanic) with MgTStrategy.cpp priorities. Boss mechanics: see CataZaShared.h for sources.

#include "CataZaStrategy.h"
#include "CataZaMultipliers.h"
#include "Playerbots.h"

void CataDungeonZulAmanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Akil'zon: Electrical Storm lifts a player; everyone else must stand in the 10 yd eye under them.
    triggers.push_back(new TriggerNode("za electrical storm",
        NextAction::array(0, new NextAction("za join storm eye", ACTION_EMERGENCY + 6), nullptr)));

    // Akil'zon: Static Disruption is an instant 12 yd blast on a random player (stacking +25% nature taken).
    triggers.push_back(new TriggerNode("za static spread",
        NextAction::array(0, new NextAction("za static spread", ACTION_MOVE + 5), nullptr)));

    // Nalorakk (troll form) and Daakara (bear): Surge hits the most distant player, +500% physical taken for 20 s.
    triggers.push_back(new TriggerNode("za surge debuff",
        NextAction::array(0, new NextAction("za surge close", ACTION_EMERGENCY + 1), nullptr)));

    // Jan'alai: Fire Bombs, then the Flame Breath cone and the fire line it leaves.
    triggers.push_back(new TriggerNode("za fire bombs",
        NextAction::array(0, new NextAction("za escape fire bombs", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("za flame breath",
        NextAction::array(0, new NextAction("za escape flame breath", ACTION_EMERGENCY + 4), nullptr)));

    // Halazzi: drag the boss out of the Water Totem's healing area.
    triggers.push_back(new TriggerNode("za water totem",
        NextAction::array(0, new NextAction("za leave water totem", ACTION_MOVE + 6), nullptr)));

    // Hex Lord Malacrass: siphoned Rain of Fire / Consecration on the ground, interrupt the heals.
    triggers.push_back(new TriggerNode("za malacrass ground",
        NextAction::array(0, new NextAction("za escape malacrass ground", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("za malacrass interrupt",
        NextAction::array(0, new NextAction("za malacrass interrupt", ACTION_INTERRUPT + 5), nullptr)));

    // Daakara: Column of Fire (dragonhawk) and Whirlwind (phase 1).
    triggers.push_back(new TriggerNode("za pillar of flame",
        NextAction::array(0, new NextAction("za escape pillar of flame", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("za whirlwind",
        NextAction::array(0, new NextAction("za escape whirlwind", ACTION_EMERGENCY + 1), nullptr)));

    // #678 Daakara eagle: Feather Cyclones (Storm on touch) and the straight line a Sweeping Winds cyclone runs along.
    triggers.push_back(new TriggerNode("za feather cyclone",
        NextAction::array(0, new NextAction("za escape feather cyclone", ACTION_EMERGENCY + 2), nullptr)));

    // Kidnapper (Akil'zon), Hatchers (Jan'alai), Lightning Totem (Halazzi), Malacrass servants.
    triggers.push_back(new TriggerNode("za focus target",
        NextAction::array(0, new NextAction("za focus target", ACTION_RAID + 2), nullptr)));
}

void CataDungeonZulAmanStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataZaHazardMultiplier(botAI));
    multipliers.push_back(new CataZaStormMultiplier(botAI));
    multipliers.push_back(new CataZaSpreadMultiplier(botAI));
    multipliers.push_back(new CataZaSurgeMultiplier(botAI));
    multipliers.push_back(new CataZaFocusMultiplier(botAI));
}
