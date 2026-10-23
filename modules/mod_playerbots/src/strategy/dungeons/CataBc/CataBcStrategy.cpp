/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#include "CataBcStrategy.h"
#include "AiObjectContext.h"
#include "CataBcMultipliers.h"
#include "Playerbots.h"

void CataDungeonBcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Rom'ogg Bonecrusher: The Skullcracker (237k to everything within 12 yd) is the one that kills the group
    triggers.push_back(new TriggerNode("cata bc skullcracker",
        NextAction::array(0, new NextAction("cata bc flee skullcracker", ACTION_EMERGENCY + 10), nullptr)));

    // Rom'ogg: Chains of Woe pull and root everyone at 66% / 33%; the chains have to be killed before the Skullcracker lands
    triggers.push_back(new TriggerNode("cata bc chains",
        NextAction::array(0, new NextAction("cata bc break chains", ACTION_EMERGENCY + 8), nullptr)));

    // Rom'ogg: Quake cracks the ground under each player; step off the crack before it ruptures
    triggers.push_back(new TriggerNode("cata bc quake",
        NextAction::array(0, new NextAction("cata bc leave quake", ACTION_EMERGENCY + 6), nullptr)));

    // Corla: standing in a beam stacks Evolution on the player (100 stacks transform it); step out at 60
    triggers.push_back(new TriggerNode("cata bc evolution",
        NextAction::array(0, new NextAction("cata bc leave beam", ACTION_EMERGENCY + 7), nullptr)));

    // Corla: Dark Command (2 s cast, fear + 30000 shadow damage) must be interrupted
    triggers.push_back(new TriggerNode("cata bc interrupt",
        NextAction::array(0, new NextAction("cata bc interrupt", ACTION_INTERRUPT + 5), nullptr)));

    // Beauty: Lava Drool puddles (pups) are 5 yd persistent fire
    triggers.push_back(new TriggerNode("cata bc lava drool",
        NextAction::array(0, new NextAction("cata bc leave lava drool", ACTION_EMERGENCY + 5), nullptr)));

    // Karsh Steelbender: only takes damage while heated in the central pillar; the tank walks him through it
    triggers.push_back(new TriggerNode("cata bc karsh pillar",
        NextAction::array(0, new NextAction("cata bc karsh pillar", ACTION_RAID + 5), nullptr)));
}

void CataDungeonBcStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new CataBcEvadeMultiplier(botAI));
    multipliers.push_back(new CataBcChainsMultiplier(botAI));
    multipliers.push_back(new CataBcKarshMultiplier(botAI));
}

// Same shape as fork strategy/dungeons/UB/UBStrategy.cpp AppendTargetExclusions (guids from a throttled value)
void CataDungeonBcStrategy::AppendTargetExclusions(GuidSet& exclusions, TargetValueExclusionType /*type*/)
{
    AiObjectContext* context = botAI->GetAiObjectContext();
    auto const& shadows = AI_VALUE_REF(GuidVector, "cata bc shadows");
    exclusions.insert(shadows.begin(), shadows.end());
}
