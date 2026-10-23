/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/Mech/MechStrategy.cpp (#566, epic #569)
#include "MechStrategy.h"
#include "MechMultipliers.h"
#include "MechTriggers.h"

void TbcDungeonMechanarStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sepethrea kite flame", NextAction::array(0, new NextAction("sepethrea kite flame", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("sepethrea avoid flame", NextAction::array(0, new NextAction("sepethrea avoid flame", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("sepethrea trail", NextAction::array(0, new NextAction("sepethrea avoid trail", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("sepethrea focus boss", NextAction::array(0, new NextAction("sepethrea focus boss", ACTION_RAID + 2), nullptr)));
}

void TbcDungeonMechanarStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SepethreaKiteFlameMultiplier(botAI));
    multipliers.push_back(new SepethreaFocusBossMultiplier(botAI));
}
