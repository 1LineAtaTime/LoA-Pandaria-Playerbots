/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#include "CataSfkStrategy.h"
#include "CataSfkMultipliers.h"

void CataDungeonSfkStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("sfk pain and suffering",
        NextAction::array(0, new NextAction("sfk interrupt pain and suffering", ACTION_INTERRUPT + 5), nullptr)));

    triggers.push_back(new TriggerNode("sfk worgen spirit",
        NextAction::array(0, new NextAction("sfk kill worgen spirit", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("sfk worgen channel",
        NextAction::array(0, new NextAction("sfk interrupt worgen", ACTION_INTERRUPT + 5), nullptr)));

    triggers.push_back(new TriggerNode("sfk desecration",
        NextAction::array(0, new NextAction("sfk leave desecration", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("sfk unholy empowerment",
        NextAction::array(0, new NextAction("sfk interrupt empowerment", ACTION_INTERRUPT + 6), nullptr)));

    triggers.push_back(new TriggerNode("sfk shield of the perfidious",
        NextAction::array(0, new NextAction("sfk leave shield cone", ACTION_EMERGENCY + 4), nullptr)));

    triggers.push_back(new TriggerNode("sfk poisonous mixture",
        NextAction::array(0, new NextAction("sfk leave poison mixture", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("sfk walden keep moving",
        NextAction::array(0, new NextAction("sfk keep moving", ACTION_EMERGENCY + 3), nullptr)));

    // AC UldStrategy.cpp:128-130 (hodir biting cold -> hodir biting cold jump, ACTION_RAID)
    triggers.push_back(new TriggerNode("sfk toxic coagulant",
        NextAction::array(0, new NextAction("sfk coagulant jump", ACTION_RAID), nullptr)));

    triggers.push_back(new TriggerNode("sfk cursed bullets",
        NextAction::array(0, new NextAction("sfk interrupt cursed bullets", ACTION_INTERRUPT + 5), nullptr)));

    triggers.push_back(new TriggerNode("sfk pistol barrage",
        NextAction::array(0, new NextAction("sfk leave barrage cone", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("sfk bloodthirsty ghoul",
        NextAction::array(0, new NextAction("sfk kill ghoul", ACTION_RAID + 2), nullptr)));
}

void CataDungeonSfkStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new SfkAvoidMultiplier(botAI));
}
