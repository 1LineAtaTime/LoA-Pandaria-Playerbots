/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// AC Ai/Dungeon/CoS/CoSTriggers.cpp:11
bool ExplodeGhoulTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "salramm the fleshcrafter");
    if (!boss) { return false; }

    float distance = 10.0f;
    float distanceExtra = 2.0f;
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_RISEN_GHOUL)
        {
            if (bot->GetExactDist2d(unit) < distance + distanceExtra)
            {
                return true;
            }
        }
    }
    return false;
}

// AC Ai/Dungeon/CoS/CoSTriggers.cpp:33
// 5.4.8: AC botAI->IsMelee(bot) -> static PlayerBotSpec::IsMelee(bot) (AI/PlayerbotSpec.h:15)
bool EpochRangedTrigger::IsActive()
{
    return !PlayerBotSpec::IsMelee(bot) && AI_VALUE2(Unit*, "find target", "chrono-lord epoch");
}
