/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataHooStrategy.h"
#include "CataHooMultipliers.h"

void CataDungeonHooStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Temple Guardian Anhuur (39425)
    triggers.push_back(new TriggerNode("hoo anhuur beacons",
        NextAction::array(0, new NextAction("hoo anhuur beacons", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("hoo anhuur pit return",
        NextAction::array(0, new NextAction("hoo anhuur pit return", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode("hoo anhuur searing light",
        NextAction::array(0, new NextAction("hoo anhuur searing light", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("hoo anhuur reckoning",
        NextAction::array(0, new NextAction("hoo anhuur reckoning", ACTION_EMERGENCY + 3), nullptr)));

    // Earthrager Ptah (39428)
    triggers.push_back(new TriggerNode("hoo ptah quicksand",
        NextAction::array(0, new NextAction("hoo ptah quicksand", ACTION_EMERGENCY + 4), nullptr)));

    // Anraphet (39788)
    triggers.push_back(new TriggerNode("hoo anraphet alpha beam",
        NextAction::array(0, new NextAction("hoo anraphet alpha beam", ACTION_EMERGENCY + 4), nullptr)));

    // Isiset (39587)
    triggers.push_back(new TriggerNode("hoo isiset supernova",
        NextAction::array(0, new NextAction("hoo isiset supernova", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("hoo isiset image",
        NextAction::array(0, new NextAction("hoo isiset image", ACTION_RAID + 5), nullptr)));

    // Ammunae (39731)
    triggers.push_back(new TriggerNode("hoo ammunae pod",
        NextAction::array(0, new NextAction("hoo ammunae pod", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("hoo ammunae spore cloud",
        NextAction::array(0, new NextAction("hoo ammunae spore cloud", ACTION_EMERGENCY + 4), nullptr)));

    // Setesh (39732)
    triggers.push_back(new TriggerNode("hoo setesh priority",
        NextAction::array(0, new NextAction("hoo setesh priority", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("hoo setesh reign of chaos",
        NextAction::array(0, new NextAction("hoo setesh reign of chaos", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("hoo setesh chaos blast",
        NextAction::array(0, new NextAction("hoo setesh chaos blast", ACTION_EMERGENCY + 4), nullptr)));

    // Rajh (39378)
    triggers.push_back(new TriggerNode("hoo rajh solar winds",
        NextAction::array(0, new NextAction("hoo rajh solar winds", ACTION_EMERGENCY + 4), nullptr)));
    // #691: the orb and the leap land within 1.5-3.5 s, so they go before the vortex
    triggers.push_back(new TriggerNode("hoo rajh sun orb",
        NextAction::array(0, new NextAction("hoo rajh sun orb", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("hoo rajh inferno leap",
        NextAction::array(0, new NextAction("hoo rajh inferno leap", ACTION_EMERGENCY + 5), nullptr)));
}

void CataDungeonHooStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HooMovementLockMultiplier(botAI));
    multipliers.push_back(new HooSupernovaMultiplier(botAI));
}
