/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb strategy "mop-ssb" (epic #373, issue #652).

#include "MopSsbStrategy.h"
#include "MopSsbMultipliers.h"
#include "Playerbots.h"

void MopSsbStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // Ground mechanics (Ground Pound, Rolling Barrels, Furlwind, Carrot Breath, lit Hoppers, Gushing Brew) go through
    // one danger trigger so the bot never steps out of one hazard into another.
    triggers.push_back(new TriggerNode("mop ssb danger",
        NextAction::array(0, new NextAction("mop ssb avoid", ACTION_EMERGENCY + 8), nullptr)));

    // Yan-Zhu: keep moving while Blackout Brew stacks climb.
    triggers.push_back(new TriggerNode("mop ssb blackout",
        NextAction::array(0, new NextAction("mop ssb blackout step", ACTION_EMERGENCY + 4), nullptr)));

    // #690 Yan-Zhu Stout pair: Carbonation (click a Fizzy Bubble, climb above the boss) and Wall of Suds (Sudsy hop).
    triggers.push_back(new TriggerNode("mop ssb fizzy",
        NextAction::array(0, new NextAction("mop ssb fizzy flight", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("mop ssb wall",
        NextAction::array(0, new NextAction("mop ssb wall jump", ACTION_EMERGENCY + 5), nullptr)));

    // #690 Yan-Zhu Ale pair: ranged DPS pop the Bubble Shield bubbles.
    triggers.push_back(new TriggerNode("mop ssb bubble",
        NextAction::array(0, new NextAction("mop ssb pop bubble", ACTION_RAID + 3), nullptr)));

    // Hoppers (Hoptallus) and Yeasty Brew Alementals (Yan-Zhu): skull mark, same level as MgT's focus marks.
    triggers.push_back(new TriggerNode("mop ssb priority",
        NextAction::array(0, new NextAction("mop ssb mark priority", ACTION_RAID + 2), nullptr)));
}

void MopSsbStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new MopSsbAvoidMultiplier(botAI));
}
