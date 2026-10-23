/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"

// AC Ai/Dungeon/FoS/FoSTriggers.cpp:11
bool MoveFromBronjahmTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (!boss->FindCurrentSpellBySpellId(SPELL_CORRUPT_SOUL))
        return false;

    if (!bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

// AC Ai/Dungeon/FoS/FoSTriggers.cpp:26
bool SwitchToSoulFragment::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    Unit* corruptedSoul = bot->FindNearestCreature(NPC_CORRUPTED_SOUL_FRAGMENT, 50.0f);
    bool activeSoulExists = corruptedSoul && corruptedSoul->IsAlive();

    if (!activeSoulExists)
        return false;

    return true;
}

// AC Ai/Dungeon/FoS/FoSTriggers.cpp:41
bool BronjahmPositionTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "bronjahm");
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
        return false;

    return true;
}

// AC Ai/Dungeon/FoS/FoSTriggers.cpp:53
bool DevourerOfSoulsTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "devourer of souls");
    if (!boss)
        return false;

    return true;
}
