/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UKStrategy.h"
#include "Playerbots.h"
#include "UKMultipliers.h"

// AC Ai/Dungeon/UK/UKStrategy.cpp:10
void WotlkDungeonUKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Prince Keleseth
    triggers.push_back(new TriggerNode("keleseth frost tomb",
             NextAction::array(0, new NextAction("attack frost tomb", ACTION_RAID + 1), nullptr)));

    // Skarvald the Constructor & Dalronn the Controller
    triggers.push_back(new TriggerNode("dalronn priority",
             NextAction::array(0, new NextAction("attack dalronn", ACTION_RAID + 1), nullptr)));

    // Ingvar the Plunderer

    // No easy way to check LoS here, the pillars do not seem to count as gameobjects.
    // Not implemented for now, unsure if this is needed as a good group can probably burst through the boss
    // and just eat the debuff.
    // triggers.push_back(new TriggerNode("ingvar dreadful roar",
    //          NextAction::array(0, new NextAction("ingvar hide los", ACTION_RAID + 1), nullptr)));
    triggers.push_back(new TriggerNode("ingvar smash tank",
            NextAction::array(0, new NextAction("ingvar dodge smash", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("ingvar smash tank return",
            NextAction::array(0, new NextAction("ingvar smash return", ACTION_MOVE + 5), nullptr)));
    // Buggy... if not behind target, ai can get stuck running towards and away from target.
    // I think for ranged chars, a custom action should be added that doesn't attempt to run into melee.
    // This is a bandaid for now, needs to be improved.
    triggers.push_back(new TriggerNode("not behind ingvar",
            NextAction::array(0, new NextAction("set behind", ACTION_MOVE + 1), nullptr)));

}

// AC Ai/Dungeon/UK/UKStrategy.cpp:37
void WotlkDungeonUKStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new PrinceKelesethMultiplier(botAI));
    multipliers.push_back(new SkarvaldAndDalronnMultiplier(botAI));
    multipliers.push_back(new IngvarThePlundererMultiplier(botAI));
}
