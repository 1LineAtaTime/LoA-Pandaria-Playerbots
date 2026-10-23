/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// AC Ai/Dungeon/GD/GDTriggers.h (#556, epic #569)
#ifndef PLAYERBOTS_GDTRIGGERS_H
#define PLAYERBOTS_GDTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

enum GundrakIDs
{
    // Slad'ran
    SPELL_POISON_NOVA_N             = 55081,
    SPELL_POISON_NOVA_H             = 59842,
    SPELL_SNAKE_WRAP                = 55126,
    NPC_SNAKE_WRAP                  = 29742,
    NPC_SLADRAN_VIPER               = 29680,
    NPC_SLADRAN_CONSTRICTOR         = 29713,

    // Gal'darah
    SPELL_WHIRLING_SLASH_N          = 55250,
    SPELL_WHIRLING_SLASH_H          = 59824,
};

// 5.4.8 SPELL-ID RULE (#556, epic #569): a heroic cast keeps the SAME spell id on this core (per-difficulty
// SpellInfo, SpellMgr.cpp:471-476), and the fork boss scripts cast ONE id in every difficulty:
//  - Poison Nova: boss_slad_ran.cpp:27,164 DoCastVictim(SPELL_POISON_NOVA = 55081). AC's heroic id 59842 is never
//    cast here, so DUNGEON_MODE(55081, 59842) would make the heroic trigger/multiplier dead; both modes use _N.
//  - Whirling Slash: boss_gal_darah.cpp:31,187 DoCast(me, SPELL_WHIRLING_SLASH = 55250) in every difficulty
//    (only Puncture/Stampede have _H ids there); both modes use _N.
//  - Snake Wrap 55126 = fork SPELL_SNAKE_WRAP_SUMMON (boss_slad_ran.cpp:36,324,442), the aura that stays on the
//    wrapped player while the Snake Wrap NPC (29742) lives - what AC's HasAura check wants. Unchanged.
// The _H enumerators are kept for parity with AC.
#define SPELL_POISON_NOVA           SPELL_POISON_NOVA_N
#define SPELL_WHIRLING_SLASH        SPELL_WHIRLING_SLASH_N

namespace GundrakSladran
{
constexpr float STACK_LEASH_YD = 10.0f;
constexpr float STACK_CLOSE_TO_YD = 5.0f;
constexpr float TANK_PICKUP_YD = 12.0f;
constexpr uint32 SNAKE_WRAP_SCAN_INTERVAL = 200;

bool IsAdd(Unit* unit);
ObjectGuid CalculateAssignedSnakeWrap(PlayerbotAI* botAI);
Unit* GetAssignedSnakeWrap(PlayerbotAI* botAI);
Player* GetStackTank(PlayerbotAI* botAI);
Unit* GetTankHoldTarget(PlayerbotAI* botAI);
}

class SladranPoisonNovaTrigger : public Trigger
{
public:
    SladranPoisonNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "slad'ran poison nova") {}
    bool IsActive() override;
};

class SladranSnakeWrapTrigger : public Trigger
{
public:
    SladranSnakeWrapTrigger(PlayerbotAI* ai) : Trigger(ai, "slad'ran snake wrap") {}
    bool IsActive() override;
};

class SladranStackOnTankTrigger : public Trigger
{
public:
    SladranStackOnTankTrigger(PlayerbotAI* ai) : Trigger(ai, "slad'ran stack on tank") {}
    bool IsActive() override;
};

class SladranTankHoldTrigger : public Trigger
{
public:
    SladranTankHoldTrigger(PlayerbotAI* ai) : Trigger(ai, "slad'ran tank hold") {}
    bool IsActive() override;
};

class GaldarahWhirlingSlashTrigger : public Trigger
{
public:
    GaldarahWhirlingSlashTrigger(PlayerbotAI* ai) : Trigger(ai, "gal'darah whirling slash") {}
    bool IsActive() override;
};

#endif
