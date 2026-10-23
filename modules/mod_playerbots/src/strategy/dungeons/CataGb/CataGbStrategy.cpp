/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataGbStrategy.h"
#include "CataGbMultipliers.h"

void CataDungeonGbStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // General Umbriss (39625)
    triggers.push_back(new TriggerNode("gb umbriss ground siege",
        NextAction::array(0, new NextAction("gb umbriss ground siege", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("gb umbriss blitz",
        NextAction::array(0, new NextAction("gb umbriss blitz", ACTION_EMERGENCY + 6), nullptr)));

    // Forgemaster Throngus (40177)
    triggers.push_back(new TriggerNode("gb throngus phalanx",
        NextAction::array(0, new NextAction("gb throngus phalanx", ACTION_RAID + 10), nullptr)));
    triggers.push_back(new TriggerNode("gb throngus cave in",
        NextAction::array(0, new NextAction("gb throngus cave in", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("gb throngus fire patch",
        NextAction::array(0, new NextAction("gb throngus fire patch", ACTION_EMERGENCY + 4), nullptr)));

    // Drahga Shadowburner (40319)
    triggers.push_back(new TriggerNode("gb drahga portal",
        NextAction::array(0, new NextAction("gb drahga portal", ACTION_EMERGENCY + 3), nullptr)));
    triggers.push_back(new TriggerNode("gb drahga kite",
        NextAction::array(0, new NextAction("gb drahga kite", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("gb drahga spirit",
        NextAction::array(0, new NextAction("gb drahga spirit", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("gb drahga seeping twilight",
        NextAction::array(0, new NextAction("gb drahga seeping twilight", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("gb valiona breath",
        NextAction::array(0, new NextAction("gb valiona breath", ACTION_EMERGENCY + 6), nullptr)));

    // Erudax (40484)
    triggers.push_back(new TriggerNode("gb erudax gale",
        NextAction::array(0, new NextAction("gb erudax gale", ACTION_EMERGENCY + 4), nullptr)));
    triggers.push_back(new TriggerNode("gb erudax shadows",
        NextAction::array(0, new NextAction("gb erudax shadows", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("gb erudax corruptor",
        NextAction::array(0, new NextAction("gb erudax corruptor", ACTION_RAID + 5), nullptr)));
}

void CataDungeonGbStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new GbMovementLockMultiplier(botAI));
    multipliers.push_back(new GbCaveInApproachMultiplier(botAI));
}
