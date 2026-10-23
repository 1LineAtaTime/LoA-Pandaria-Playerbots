/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_COSTRIGGERS_H
#define PLAYERBOTS_COSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/CoS/CoSTriggers.h:15
enum CullingOfStratholmeIDs
{
    // Salramm the Fleshcrafter
    // 5.4.8 check: fork src/server/scripts/Kalimdor/CavernsOfTime/CullingOfStratholme/boss_salramm_the_fleshcrafter.cpp:41; same id as AC.
    NPC_GHOUL_MINION                   = 27733,

    // AC CoSTriggers.cpp:22 compares against NPC_RISEN_GHOUL, which AC takes from core PetDefines.h:121
    // (26125, the Death Knight pet ghoul, NOT Salramm's 27733 Ghoul Minion). The fork has no such constant
    // (only TemporarySummon.h:94 IsPetGhoul() == 26125), so it is declared here with AC's value to keep AC's
    // behaviour 1:1. Consequence (same in AC): "explode ghoul" only fires for a nearby corpse of entry 26125.
    NPC_RISEN_GHOUL                    = 26125,
};

// AC Ai/Dungeon/CoS/CoSTriggers.h:21
class ExplodeGhoulTrigger : public Trigger
{
public:
    ExplodeGhoulTrigger(PlayerbotAI* ai) : Trigger(ai, "explode ghoul") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/CoS/CoSTriggers.h:28
class EpochRangedTrigger : public Trigger
{
public:
    EpochRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "chrono-lord epoch ranged") {}
    bool IsActive() override;
};

#endif
