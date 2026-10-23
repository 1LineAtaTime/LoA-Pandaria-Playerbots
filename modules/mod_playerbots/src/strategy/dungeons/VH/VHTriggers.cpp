/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// AC Ai/Dungeon/VH/VHTriggers.cpp:11
// 5.4.8: AC botAI->IsDps/IsHeal/IsTank/IsRangedDps(bot) -> static PlayerBotSpec::IsX(bot) (AI/PlayerbotSpec.h:11-18)
bool ErekemTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss) { return false; }

    return PlayerBotSpec::IsDps(bot);
}

// AC Ai/Dungeon/VH/VHTriggers.cpp:19
bool IchoronTargetTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return false; }

    return !PlayerBotSpec::IsHeal(bot);
}

// AC Ai/Dungeon/VH/VHTriggers.cpp:27
bool VoidShiftTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    return bot->HasAura(SPELL_VOID_SHIFTED) && !PlayerBotSpec::IsHeal(bot);
}

// AC Ai/Dungeon/VH/VHTriggers.cpp:35
bool ShroudOfDarknessTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return false; }

    return boss->HasAura(SPELL_SHROUD_OF_DARKNESS);
}

// AC Ai/Dungeon/VH/VHTriggers.cpp:43
bool CyanigosaPositioningTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "cyanigosa");
    if (!boss) { return false; }

    // Include healers here for now, otherwise they stand in things
    return !PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsRangedDps(bot);
    // return botAI->IsMelee(bot) && !botAI->IsTank(bot);
}
