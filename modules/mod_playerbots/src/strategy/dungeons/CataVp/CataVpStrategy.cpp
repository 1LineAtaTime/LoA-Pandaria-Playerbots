/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataVpStrategy.h"
#include "CataVpMultipliers.h"

void CataDungeonVpStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Grand Vizier Ertan (43878)
    triggers.push_back(new TriggerNode("vp ertan ring",
        NextAction::array(0, new NextAction("vp ertan ring", ACTION_EMERGENCY + 4), nullptr)));

    // Altairus (43873)
    triggers.push_back(new TriggerNode("vp altairus breath",
        NextAction::array(0, new NextAction("vp altairus breath", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("vp altairus twister",
        NextAction::array(0, new NextAction("vp altairus twister", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("vp altairus downwind",
        NextAction::array(0, new NextAction("vp altairus downwind", ACTION_RAID + 10), nullptr)));

    // Asaad (43875)
    triggers.push_back(new TriggerNode("vp asaad stack",
        NextAction::array(0, new NextAction("vp asaad stack", ACTION_EMERGENCY + 6), nullptr)));

    // #692 (BEYOND AC) Slipstream rides between the platforms: above combat and follow (the movement lock vetoes the follow
    // toward a leader in the air), below the boss mechanics (never both: a ride needs the platform's boss dead)
    triggers.push_back(new TriggerNode("vp slipstream",
        NextAction::array(0, new NextAction("vp slipstream", ACTION_EMERGENCY + 2), nullptr)));
}

void CataDungeonVpStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new VpMovementLockMultiplier(botAI));
}
