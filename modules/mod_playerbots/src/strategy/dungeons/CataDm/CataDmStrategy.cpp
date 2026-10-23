/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#include "CataDmStrategy.h"
#include "CataDmMultipliers.h"

void CataDungeonDmStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("dm blossom landing",
        NextAction::array(0, new NextAction("dm leave blossom", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("dm blossom add",
        NextAction::array(0, new NextAction("dm kill blossom", ACTION_RAID + 2), nullptr)));

    triggers.push_back(new TriggerNode("dm sticky bomb",
        NextAction::array(0, new NextAction("dm leave sticky bomb", ACTION_EMERGENCY + 6), nullptr)));

    triggers.push_back(new TriggerNode("dm chest bomb",
        NextAction::array(0, new NextAction("dm spread chest bomb", ACTION_EMERGENCY + 7), nullptr)));

    // #667: stage one = the Oaf, stage two = Helix; below the bomb dodges, above the generic target choice (AC DTKStrategy.cpp:23)
    triggers.push_back(new TriggerNode("dm helix focus",
        NextAction::array(0, new NextAction("dm attack helix", ACTION_RAID + 1), nullptr)));

    triggers.push_back(new TriggerNode("dm harvest",
        NextAction::array(0, new NextAction("dm leave harvest", ACTION_EMERGENCY + 7), nullptr)));

    triggers.push_back(new TriggerNode("dm overdrive",
        NextAction::array(0, new NextAction("dm leave overdrive", ACTION_EMERGENCY + 5), nullptr)));

    triggers.push_back(new TriggerNode("dm vapor",
        NextAction::array(0, new NextAction("dm kill vapor", ACTION_RAID + 3), nullptr)));

    triggers.push_back(new TriggerNode("dm vanessa add",
        NextAction::array(0, new NextAction("dm kill vanessa add", ACTION_RAID + 2), nullptr)));
    // #681 / #826 (2026-10-23): the heroic mechanics ported into the fork's Deadmines scripts and the Nightmare Gauntlet
    triggers.push_back(new TriggerNode("dm steam valve",
        NextAction::array(0, new NextAction("dm click steam valve", ACTION_EMERGENCY + 9), nullptr)));
    triggers.push_back(new TriggerNode("dm powder keg",
        NextAction::array(0, new NextAction("dm leave powder keg", ACTION_EMERGENCY + 8), nullptr)));
    triggers.push_back(new TriggerNode("dm vanessa blaze",
        NextAction::array(0, new NextAction("dm escape blaze", ACTION_EMERGENCY + 7), nullptr)));
    triggers.push_back(new TriggerNode("dm fire wall",
        NextAction::array(0, new NextAction("dm dodge fire wall", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("dm nightmare hazard",
        NextAction::array(0, new NextAction("dm escape nightmare hazard", ACTION_EMERGENCY + 6), nullptr)));
    triggers.push_back(new TriggerNode("dm molten slag",
        NextAction::array(0, new NextAction("dm avoid molten slag", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("dm harrington",
        NextAction::array(0, new NextAction("dm save harrington", ACTION_RAID + 3), nullptr)));
    triggers.push_back(new TriggerNode("dm slag add",
        NextAction::array(0, new NextAction("dm kill molten slag", ACTION_RAID + 2), nullptr)));
    // #681 w106: the tank's own target choice, at the "dm helix focus" rank (above "tank assist", below every dodge)
    triggers.push_back(new TriggerNode("dm reaper focus",
        NextAction::array(0, new NextAction("dm attack reaper", ACTION_RAID + 1), nullptr)));
}

void CataDungeonDmStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new DmAvoidMultiplier(botAI));
}
