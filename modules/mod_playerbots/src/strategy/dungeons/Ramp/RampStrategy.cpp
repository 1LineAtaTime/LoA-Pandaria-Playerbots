/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/Ramp/RampStrategy.cpp (#568, epic #569)
#include "RampStrategy.h"
#include "RampMultipliers.h"

void TbcDungeonHellfireRampartsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Watchkeeper Gargolmar
    triggers.push_back(new TriggerNode("gargolmar hellfire watchers are active",
                                       NextAction::array(0, new NextAction("gargolmar mark hellfire watchers", ACTION_RAID), nullptr)));

    // Omor the Unscarred
    triggers.push_back(new TriggerNode("omor treachery aura",
                                       NextAction::array(0, new NextAction("omor treachery aura flee from players", ACTION_EMERGENCY + 1), nullptr)));

    triggers.push_back(new TriggerNode("omor tank has treachery aura",
                                       NextAction::array(0, new NextAction("omor treachery aura flee from tank", ACTION_EMERGENCY + 1), nullptr)));

    triggers.push_back(new TriggerNode("omor ranged spread",
                                        NextAction::array(0, new NextAction("omor ranged spread", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("omor fiendish hound is active",
                                        NextAction::array(0, new NextAction("omor mark fiendish hound", ACTION_RAID), nullptr)));

    // Vazruden & Nazan
    triggers.push_back(new TriggerNode("vazruden tank position boss",
                                        NextAction::array(0, new NextAction("vazruden tank position boss", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("vazruden boss is active",
                                        NextAction::array(0, new NextAction("vazruden mark boss", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("nazan boss tremor totem",
                                        NextAction::array(0, new NextAction("nazan set tremor totem", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("nazan boss fire resistance totem",
                                        NextAction::array(0, new NextAction("nazan set fire resistance totem", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("nazan boss fire resistance aura",
                                       NextAction::array(0, new NextAction("nazan set fire resistance aura", ACTION_RAID), nullptr)));
}

void TbcDungeonHellfireRampartsStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new OmorTreacheryAuraFleeFromPlayersMultiplier(botAI));

    multipliers.push_back(new NazanSetTremorTotemMultiplier(botAI));

    multipliers.push_back(new NazanSetFireResistanceTotemMultiplier(botAI));

    multipliers.push_back(new NazanSetFireResistanceAuraMultiplier(botAI));
}
