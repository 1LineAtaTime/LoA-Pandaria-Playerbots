/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

// AC Ai/Dungeon/DTK/DTKTriggers.cpp:11 (CorpseExplodeTrigger::IsActive)
bool CorpseExplodeTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "trollgore");
    if (!boss) { return false; }

    float distance = 6.0f;  // 5 unit radius, 1 unit added as buffer
    GuidVector corpses = AI_VALUE(GuidVector, "nearest corpses");
    for (auto i = corpses.begin(); i != corpses.end(); ++i)
    {
        Unit* unit = botAI->GetUnit(*i);
        if (unit && unit->GetEntry() == NPC_DRAKKARI_INVADER)
        {
            if (bot->GetExactDist2d(unit) < distance)
            {
                return true;
            }
        }
    }
    return false;
}

// AC Ai/Dungeon/DTK/DTKTriggers.cpp:32 (ArcaneFieldTrigger::IsActive)
bool ArcaneFieldTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
    if (boss)
    {
        return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_ARCANE_FIELD);
    }
    return false;
}

// AC Ai/Dungeon/DTK/DTKTriggers.cpp:44-60 (CrystalHandlerTrigger::IsActive) is commented out in AC; kept as a comment.
// bool CrystalHandlerTrigger::IsActive()
// {
//     Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
//     if (!boss) { return false; }

//     // Target is not findable from threat table using AI_VALUE2(),
//     // therefore need to search manually for the unit name
//     GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

//     for (auto i = targets.begin(); i != targets.end(); ++i)
//     {
//         Unit* unit = botAI->GetUnit(*i);
//         if (unit && unit->GetEntry() == NPC_CRYSTAL_HANDLER)
//         {
//             return true;
//         }
//     }
//     return false;
// }

// AC Ai/Dungeon/DTK/DTKTriggers.cpp:62 (GiftOfTharonjaTrigger::IsActive)
bool GiftOfTharonjaTrigger::IsActive()
{
    return bot->HasAura(SPELL_GIFT_OF_THARONJA);
}
