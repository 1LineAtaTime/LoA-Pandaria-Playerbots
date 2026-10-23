/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKStrategy.h"
#include "Playerbots.h"
#include "DTKMultipliers.h"

// 5.4.8: AC `{ NextAction("a", P) }` -> fork NextAction::array(0, new NextAction("a", P), nullptr)
// (strategy/generic/CombatStrategy.cpp:13).
// AC Ai/Dungeon/DTK/DTKStrategy.cpp:10
void WotlkDungeonDTKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Trollgore
    triggers.push_back(new TriggerNode("corpse explode",
        NextAction::array(0, new NextAction("corpse explode spread", ACTION_MOVE + 5), nullptr)));

    // Novos the Summoner
    // TODO: Can be improved - it's a pretty easy fight but complex to program, revisit if needed
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("avoid arcane field", ACTION_MOVE + 5), nullptr)));
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("novos positioning", ACTION_MOVE + 4), nullptr)));
    triggers.push_back(new TriggerNode("arcane field",
        NextAction::array(0, new NextAction("novos target priority", ACTION_NORMAL + 1), nullptr)));

    // King Dred
    // TODO: Fear ward / tremor totem, or general anti-fear strat development

    //The Prophet Tharon'ja
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("touch of life", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("bone armor", ACTION_NORMAL + 4), nullptr)));
    // Run ranged chars (who would normally stand at range) into melee, to dps in skeleton form
    triggers.push_back(new TriggerNode("tharon'ja out of melee",
        NextAction::array(0, new NextAction("reach melee", ACTION_NORMAL + 3), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("taunt", ACTION_NORMAL + 2), nullptr)));
    triggers.push_back(new TriggerNode("gift of tharon'ja",
        NextAction::array(0, new NextAction("slaying strike", ACTION_NORMAL + 2), nullptr)));
}

// AC Ai/Dungeon/DTK/DTKStrategy.cpp:42
void WotlkDungeonDTKStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new NovosMultiplier(botAI));
    multipliers.push_back(new TharonjaMultiplier(botAI));
}
