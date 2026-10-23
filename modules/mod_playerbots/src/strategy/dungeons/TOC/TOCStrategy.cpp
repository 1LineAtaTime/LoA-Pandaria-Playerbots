/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "TOCStrategy.h"

// AC Ai/Dungeon/TOC/TOCStrategy.cpp:9
void WotlkDungeonToCStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("toc lance",
        NextAction::array(0, new NextAction("toc lance", ACTION_RAID + 5), nullptr)));
    triggers.push_back(new TriggerNode("toc ue lance",
        NextAction::array(0, new NextAction("toc ue lance", ACTION_RAID + 2), nullptr)));
    triggers.push_back(new TriggerNode("toc mount near",
        NextAction::array(0, new NextAction("toc mount", ACTION_RAID + 4), nullptr)));
    triggers.push_back(new TriggerNode("toc mounted",
        NextAction::array(0, new NextAction("toc mounted", ACTION_RAID + 6), nullptr)));
    triggers.push_back(new TriggerNode("toc eadric",
        NextAction::array(0, new NextAction("toc eadric", ACTION_RAID + 3), nullptr)));

}

// AC Ai/Dungeon/TOC/TOCStrategy.cpp:24
void WotlkDungeonToCStrategy::InitMultipliers(std::vector<Multiplier*>& /*multipliers*/)
{
}
