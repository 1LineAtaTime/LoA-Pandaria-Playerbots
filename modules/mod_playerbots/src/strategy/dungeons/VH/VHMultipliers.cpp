/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "VHMultipliers.h"
#include "ChooseTargetActions.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "VHActions.h"
#include "VHTriggers.h"

// AC Ai/Dungeon/VH/VHMultipliers.cpp:13
// 5.4.8: AC botAI->IsDps(bot) -> static PlayerBotSpec::IsDps(bot) (AI/PlayerbotSpec.h:13)
float ErekemMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "erekem");
    if (!boss || !PlayerBotSpec::IsDps(bot)) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action))
    {
        return 0.0f;
    }
    if (action->getThreatType() == Action::ActionThreatType::Aoe)
    {
        return 0.0f;
    }
    return 1.0f;
}

// AC Ai/Dungeon/VH/VHMultipliers.cpp:29
float IchoronMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ichoron");
    if (!boss) { return 1.0f; }

    if (dynamic_cast<DpsAssistAction*>(action)
        || dynamic_cast<TankAssistAction*>(action)
        || dynamic_cast<DropTargetAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}

// AC Ai/Dungeon/VH/VHMultipliers.cpp:43
float ZuramatMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "zuramat the obliterator");
    if (!boss) { return 1.0f; }

    if (bot->HasAura(SPELL_VOID_SHIFTED))
    {
        if (dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }
    }

    if (boss->HasAura(SPELL_SHROUD_OF_DARKNESS) && dynamic_cast<AttackAction*>(action))
    {
        return 0.0f;
    }
    return 1.0f;
}
