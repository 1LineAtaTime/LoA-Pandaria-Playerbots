/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/GD/GDStrategy.cpp (#556, epic #569)
#include "GDStrategy.h"
#include "GDMultipliers.h"

void WotlkDungeonGDStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Moorabi

    // Drakkari Colossus

    // Slad'ran
    // TODO: Might need to add target priority for heroic on the snakes or to burn down boss.
    // Will re-test in heroic, decent dps groups should be able to blast him down with no funky strats.
    triggers.push_back(new TriggerNode("poison nova",
        NextAction::array(0, new NextAction("avoid poison nova", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("snake wrap",
        NextAction::array(0, new NextAction("attack snake wrap", ACTION_RAID + 4), nullptr)));
    triggers.push_back(new TriggerNode("slad'ran stack on tank",
        NextAction::array(0, new NextAction("slad'ran stack on tank", ACTION_RAID + 1), nullptr)));
    triggers.push_back(new TriggerNode("slad'ran tank hold",
        NextAction::array(0, new NextAction("slad'ran tank hold", ACTION_RAID + 1), nullptr)));

    // Gal'darah
    triggers.push_back(new TriggerNode("whirling slash",
        NextAction::array(0, new NextAction("avoid whirling slash", ACTION_RAID + 5), nullptr)));

    // Eck the Ferocious (Heroic only)
}

void WotlkDungeonGDStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SladranMultiplier(botAI));
    multipliers.push_back(new GaldarahMultiplier(botAI));
}
