/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time (map 938) strategy "cata-et" - issue #671, epic #373. AC has no End Time strategy; this is AC's dungeon-strategy
// pattern (Ai/Dungeon/HoS/HoSStrategy.cpp:9-33) with edges for the mechanics that wipe a pull. Sources: see CataEtShared.h.
//
// Boss x edges (all four echoes are covered; the instance only offers two of them per run, end_time_teleport.cpp:66-110):
//   Echo of Sylvanas : kill ONE Risen Ghoul / stand in its breach / avoid Blighted Arrows / spread ranged
//   Echo of Tyrande  : stand in the Pool of Moonlight during the add event / dodge Moonlance / dodge Eyes of Elune / interrupt
//   Echo of Jaina    : collect the 16 staff fragments / dodge Frost Blades / stomp or avoid the Flarecore / interrupt Volley
//   Echo of Baine    : swim out of the lava to a platform / click and throw the totem back
//   Murozond         : leave Distortion Bombs / leave the Infinite Breath cone / press the Hourglass of Time

#include "CataEtStrategy.h"
#include "CataEtMultipliers.h"
#include "CataEtShared.h"

void CataDungeonEndTimeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Echo of Sylvanas
    triggers.push_back(new TriggerNode("et sylvanas stay inside",
        NextAction::array(0, new NextAction("et sylvanas stay inside", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(new TriggerNode("et sylvanas safe spot",
        NextAction::array(0, new NextAction("et sylvanas safe spot", ACTION_EMERGENCY + 8), nullptr)));
    triggers.push_back(new TriggerNode("et blighted arrows",
        NextAction::array(0, new NextAction("et leave blighted arrows", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("et sylvanas kill ghoul",
        NextAction::array(0, new NextAction("et sylvanas kill ghoul", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("et ranged spread",
        NextAction::array(0, new NextAction("et ranged spread", ACTION_RAID + 3), nullptr)));

    // Echo of Tyrande
    triggers.push_back(new TriggerNode("et moonlance",
        NextAction::array(0, new NextAction("et leave moonlance", ACTION_EMERGENCY + 7), nullptr)));
    triggers.push_back(new TriggerNode("et eye of elune",
        NextAction::array(0, new NextAction("et leave eye of elune", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("et tyrande interrupt",
        NextAction::array(0, new NextAction("et tyrande interrupt", ACTION_INTERRUPT + 5), nullptr)));
    triggers.push_back(new TriggerNode("et tyrande ranged band",
        NextAction::array(0, new NextAction("et tyrande ranged band", ACTION_RAID + 4), nullptr)));
    triggers.push_back(new TriggerNode("et moonlight pool",
        NextAction::array(0, new NextAction("et enter moonlight pool", ACTION_RAID + 1), nullptr)));

    // Echo of Jaina
    triggers.push_back(new TriggerNode("et frost blades",
        NextAction::array(0, new NextAction("et dodge frost blades", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(new TriggerNode("et flarecore",
        NextAction::array(0, new NextAction("et handle flarecore", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("et jaina interrupt",
        NextAction::array(0, new NextAction("et jaina interrupt", ACTION_INTERRUPT + 5), nullptr)));
    triggers.push_back(new TriggerNode("et jaina fragments",
        NextAction::array(0, new NextAction("et collect fragment", ACTION_HIGH + 2), nullptr)));

    // Echo of Baine
    triggers.push_back(new TriggerNode("et baine in lava",
        NextAction::array(0, new NextAction("et baine leave lava", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("et baine totem",
        NextAction::array(0, new NextAction("et baine totem", ACTION_RAID + 4), nullptr)));

    // Murozond
    triggers.push_back(new TriggerNode("et distortion bomb",
        NextAction::array(0, new NextAction("et leave distortion bomb", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("et infinite breath",
        NextAction::array(0, new NextAction("et dodge infinite breath", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("et hourglass",
        NextAction::array(0, new NextAction("et use hourglass", ACTION_RAID + 6), nullptr)));
}

void CataDungeonEndTimeStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataEtHazardMultiplier(botAI));
    multipliers.push_back(new CataEtSylvanasCallMultiplier(botAI));
    multipliers.push_back(new CataEtTyrandeEventMultiplier(botAI));
    multipliers.push_back(new CataEtTyrandeKickMultiplier(botAI));
    multipliers.push_back(new CataEtTyrandeBandMultiplier(botAI));
    multipliers.push_back(new CataEtHourglassMultiplier(botAI));
}
