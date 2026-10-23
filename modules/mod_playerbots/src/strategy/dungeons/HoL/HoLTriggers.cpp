/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "HoLTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:11
bool StormforgedLieutenantTrigger::IsActive()
{
    // 5.4.8: botAI->IsDps(bot) -> static PlayerBotSpec::IsDps(bot) (AI/PlayerbotSpec.h:13)
    if (!PlayerBotSpec::IsDps(bot)) { return false; }

    // Target is not findable from threat table using AI_VALUE2(),
    // therefore need to search manually for the unit name
    GuidVector targets = AI_VALUE(GuidVector, "possible targets");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->IsInCombat() &&
            unit->GetEntry() == NPC_STORMFORGED_LIEUTENANT)
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:31
bool BjarngrimWhirlwindTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "general bjarngrim");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND_BJARNGRIM);
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:39
bool VolkhanTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "volkhan");
    return boss && !PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsHeal(bot);
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:45
bool IonarStaticOverloadTrigger::IsActive()
{
    GuidVector members = AI_VALUE(GuidVector, "group members");
    for (auto& member : members)
    {
        Unit* unit = botAI->GetUnit(member);
        if (unit && unit->HasAura(SPELL_STATIC_OVERLOAD))
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:59
bool IonarBallLightningTrigger::IsActive()
{
    if (PlayerBotSpec::IsMelee(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_BALL_LIGHTNING);
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:69
bool IonarTankAggroTrigger::IsActive()
{
    if (!PlayerBotSpec::IsTank(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }

    return AI_VALUE2(bool, "has aggro", "current target");
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:79
bool IonarDisperseTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ionar");
    if (!boss) { return false; }

    return !bot->CanSeeOrDetect(boss) || boss->FindCurrentSpellBySpellId(SPELL_DISPERSE);
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:87
bool LokenRangedTrigger::IsActive()
{
    return !PlayerBotSpec::IsMelee(bot) && AI_VALUE2(Unit*, "find target", "loken");
}

// AC Ai/Dungeon/HoL/HoLTriggers.cpp:92
bool LokenLightningNovaTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "loken");
    if (!boss) { return false; }

    return boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_LIGHTNING_NOVA);
}
