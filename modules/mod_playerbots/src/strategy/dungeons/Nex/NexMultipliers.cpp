/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NexMultipliers.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "NexActions.h"
#include "NexTriggers.h"

// AC Ai/Dungeon/Nex/NexMultipliers.cpp:14 (FactionCommanderMultiplier::GetValue)
// 5.4.8: DUNGEON_DIFFICULTY_NORMAL/HEROIC exist (src/server/shared/DataStores/DBCEnums.h:411-412, see
// DungeonStrategyUtils.h). Faction handling kept as AC: Normal = the opposing-faction commander
// (27947/27949), Heroic = Kolurg (Alliance bots) / Stoutbeard (Horde bots).
float FactionCommanderMultiplier::GetValue(Action* action)
{
    Unit* boss = nullptr;
    uint8 faction = bot->GetTeamId();

    switch (bot->GetMap()->GetDifficulty())
    {
        case DUNGEON_DIFFICULTY_NORMAL:
            if (faction == TEAM_ALLIANCE)
            {
                boss = AI_VALUE2(Unit*, "find target", "horde commander");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = AI_VALUE2(Unit*, "find target", "alliance commander");
            }
            break;
        case DUNGEON_DIFFICULTY_HEROIC:
            if (faction == TEAM_ALLIANCE)
            {
                boss = AI_VALUE2(Unit*, "find target", "commander kolurg");
            }
            else //if (faction == TEAM_HORDE)
            {
                boss = AI_VALUE2(Unit*, "find target", "commander stoutbeard");
            }
            break;
        default:
            break;
    }
    if (boss && boss->HasUnitState(UNIT_STATE_CASTING) &&
        boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
    {
        // Prevent movement actions other than flee during a whirlwind, to prevent running back in early.
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromWhirlwindAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/Nex/NexMultipliers.cpp:56 (TelestraMultiplier::GetValue)
float TelestraMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand magus telestra");
    if (boss && boss->GetEntry() != NPC_TELESTRA)
    {
        // boss is split into clones, do not auto acquire target
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/Nex/NexMultipliers.cpp:70 (AnomalusMultiplier::GetValue)
float AnomalusMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anomalus");
    if (boss && boss->HasAura(BUFF_RIFT_SHIELD))
    {
        if (dynamic_cast<DpsAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/Nex/NexMultipliers.cpp:83 (OrmorokMultiplier::GetValue)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11).
float OrmorokMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    if (!boss) { return 1.0f; }

    // These are used for auto ranged repositioning, need to suppress so ranged dps don't ping-pong
    if (dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }
    // This boss is annoying and shuffles around a lot. Don't let tank move once fight has started.
    // Extra checks are to allow the tank to close distance and engage the boss initially
    if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<DodgeSpikesAction*>(action)
        && PlayerBotSpec::IsTank(bot) && bot->IsWithinMeleeRange(boss)
        && AI_VALUE2(bool, "facing", "current target"))
        {
            return 0.0f;
        }
    return 1.0f;
}
