/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "NexTriggers.h"
#include "AiObject.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"

// 5.4.8: AC botAI->IsTank/IsMelee/IsRangedDps(bot) -> static PlayerBotSpec::IsX(bot) (AI/PlayerbotSpec.h:11-19).

// AC Ai/Dungeon/Nex/NexTriggers.cpp:12 (FactionCommanderWhirlwindTrigger::IsActive)
bool FactionCommanderWhirlwindTrigger::IsActive()
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

    if (boss && boss->HasUnitState(UNIT_STATE_CASTING))
    {
        if (boss->FindCurrentSpellBySpellId(SPELL_WHIRLWIND))
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:53 (TelestraFirebombTrigger::IsActive)
bool TelestraFirebombTrigger::IsActive()
{
    if (PlayerBotSpec::IsMelee(bot)) { return false; }

    Unit* boss = AI_VALUE2(Unit*, "find target", "grand magus telestra");
    // Avoid split phase with the fake Telestra units, only match the true boss id
    return boss && boss->GetEntry() == NPC_TELESTRA;
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:62 (TelestraSplitPhaseTrigger::IsActive)
bool TelestraSplitPhaseTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "grand magus telestra");
    // Only match split phase with the fake Telestra units
    return boss && boss->GetEntry() != NPC_TELESTRA;
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:69 (ChaoticRiftTrigger::IsActive)
bool ChaoticRiftTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "anomalus");
    return boss && boss->HasAura(BUFF_RIFT_SHIELD);
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:75 (OrmorokSpikesTrigger::IsActive)
// 5.4.8 NOTE: this keys on gameobject 188537. AC's Ormorok script spawns it per spike creature
// (AC src/server/scripts/Northrend/Nexus/Nexus/boss_ormorok.cpp:174 SummonGameObject(GO_CRYSTAL_SPIKE ...)),
// but the fork's boss_ormorok.cpp only SummonCreature(NPC_CRYSTAL_SPIKE 27099) (line 135) and has no
// SummonGameObject, so on the fork this trigger never fires (the non-tank "ormorok stack" trigger still does).
bool OrmorokSpikesTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    if (!boss || !PlayerBotSpec::IsTank(bot)) { return false; }

    GuidVector objects = AI_VALUE(GuidVector, "closest game objects");
    for (auto i = objects.begin(); i != objects.end(); ++i)
    {
        GameObject* go = botAI->GetGameObject(*i);
        if (go && go->GetEntry() == GO_CRYSTAL_SPIKE)
        {
            return true;
        }
    }
    return false;
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:92 (OrmorokStackTrigger::IsActive)
bool OrmorokStackTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ormorok the tree-shaper");
    return (boss && !PlayerBotSpec::IsTank(bot));
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:98 (IntenseColdTrigger::IsActive)
// Aura looked up by name "intense cold" (fork spell 48094, boss_keristrasza.cpp:34,96).
bool IntenseColdTrigger::IsActive()
{
    // Adjust as needed - too much interrupting loses dps time,
    // but too many stacks is deadly. Assuming 3-5 is a good number to clear
    int stackThreshold = 5;
    Unit* boss = AI_VALUE2(Unit*, "find target", "keristrasza");
    return boss && botAI->GetAura("intense cold", bot, false, false, stackThreshold);
}

// AC Ai/Dungeon/Nex/NexTriggers.cpp:107 (KeristraszaPositioningTrigger::IsActive)
bool KeristraszaPositioningTrigger::IsActive()
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "keristrasza");
    // Include healers here for now, otherwise they stand in things
    return boss && !PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsRangedDps(bot);
    // return boss && botAI->IsMelee(bot) && !botAI->IsTank(bot);
}
