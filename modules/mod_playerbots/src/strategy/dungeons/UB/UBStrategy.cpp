/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "UBStrategy.h"
#include "Playerbots.h"
#include "UBMultipliers.h"

// AC Ai/Dungeon/UB/UBStrategy.cpp:11
// 5.4.8: TriggerNode syntax per fork strategy/generic/CombatStrategy.cpp:13.
void TbcDungeonUnderbogStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("ub foul spores",
        NextAction::array(0, new NextAction("ub retreat from foul spores", ACTION_EMERGENCY + 10), nullptr)));

    triggers.push_back(new TriggerNode("ub spore cloud danger",
        NextAction::array(0, new NextAction("ub vacate spore cloud", ACTION_EMERGENCY + 2), nullptr)));

    triggers.push_back(new TriggerNode("ub underbat lash",
        NextAction::array(0, new NextAction("ub clear underbat back", ACTION_EMERGENCY + 1), nullptr)));
}

// AC Ai/Dungeon/UB/UBStrategy.cpp:23
void TbcDungeonUnderbogStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new HungarfenFoulSporesMultiplier(botAI));
    multipliers.push_back(new HungarfenMushroomIgnoreMultiplier(botAI));
    multipliers.push_back(new UnderbatFacingMultiplier(botAI));
}

// AC Ai/Dungeon/UB/UBStrategy.cpp:30
void TbcDungeonUnderbogStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& mushrooms = AI_VALUE_REF(GuidVector, "ub mushrooms");
    exclusions.insert(mushrooms.begin(), mushrooms.end());
}
