/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#include "CataScStrategy.h"
#include "CataScMultipliers.h"
#include "Playerbots.h"

void CataDungeonScStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Corborus: Crystal Barrage drops 9.5k per half second on a 5 yd circle under the target for 4 s
    triggers.push_back(new TriggerNode("cata sc crystal barrage",
        NextAction::array(0, new NextAction("cata sc leave crystal barrage", ACTION_EMERGENCY + 8), nullptr)));

    // Corborus, heroic: the Crystal Shards the barrage leaves behind explode on the first player they reach
    triggers.push_back(new TriggerNode("cata sc crystal shards",
        NextAction::array(0, new NextAction("cata sc kill crystal shards", ACTION_RAID + 3), nullptr)));

    // Slabhide: Lava Fissure (crack, then eruption and a lava pool) under a random player
    triggers.push_back(new TriggerNode("cata sc lava fissure",
        NextAction::array(0, new NextAction("cata sc leave lava fissure", ACTION_EMERGENCY + 7), nullptr)));

    // Slabhide: stalactites land on a marked circle (19k + knockback), in his air phase and in the corridor before him
    triggers.push_back(new TriggerNode("cata sc stalactite",
        NextAction::array(0, new NextAction("cata sc leave stalactite", ACTION_EMERGENCY + 9), nullptr)));

    // Slabhide: Sand Blast cone is meant for the tank only
    triggers.push_back(new TriggerNode("cata sc sand blast",
        NextAction::array(0, new NextAction("cata sc leave sand blast", ACTION_EMERGENCY + 3), nullptr)));

    // Ozruk: Shatter (3 s cast, 15 yd around him) one-shots most players
    triggers.push_back(new TriggerNode("cata sc shatter",
        NextAction::array(0, new NextAction("cata sc flee shatter", ACTION_EMERGENCY + 10), nullptr)));

    // Ozruk: Ground Slam (3 s cast) lands in front of him
    triggers.push_back(new TriggerNode("cata sc ground slam",
        NextAction::array(0, new NextAction("cata sc leave slam cone", ACTION_EMERGENCY + 8), nullptr)));

    // High Priestess Azil: Gravity Well pulls and burns everything within 10 yd
    triggers.push_back(new TriggerNode("cata sc gravity well",
        NextAction::array(0, new NextAction("cata sc leave gravity well", ACTION_EMERGENCY + 6), nullptr)));

    // High Priestess Azil: Energy Shield phase (-75% damage taken), the Devout Followers are what dies
    triggers.push_back(new TriggerNode("cata sc azil followers",
        NextAction::array(0, new NextAction("cata sc kill followers", ACTION_RAID + 2), nullptr)));
}

void CataDungeonScStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataScEvadeMultiplier(botAI));
    multipliers.push_back(new CataScFocusMultiplier(botAI));
}
