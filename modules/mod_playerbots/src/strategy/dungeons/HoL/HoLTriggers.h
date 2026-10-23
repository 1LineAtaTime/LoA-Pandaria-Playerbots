/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOLTRIGGERS_H
#define PLAYERBOTS_HOLTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/HoL/HoLTriggers.h:15
enum HallsOfLightningIDs
{
    // General Bjarngrim
    NPC_STORMFORGED_LIEUTENANT      = 29240,
    // The fork's boss script DoCasts 52027 (src/server/scripts/Northrend/Ulduar/HallsOfLightning/boss_bjarngrim.cpp:40,316)
    SPELL_WHIRLWIND_BJARNGRIM       = 52027,

    // Ionar
    // 5.4.8 ADAPTATION: AC has SPELL_STATIC_OVERLOAD_N = 52658 / _H = 59795 and SPELL_BALL_LIGHTNING_N = 52780 /
    // _H = 59800 with #define X DUNGEON_MODE(bot, X_N, X_H) (AC HoLTriggers.h:16-19,34-35). The fork's boss
    // script DoCasts only the normal id in every difficulty (boss_ionar.cpp:260 SPELL_STATIC_OVERLOAD = 52658,
    // :269 SPELL_BALL_LIGHTNING = 52780; its _H constants at :28,:30 are never cast), and a heroic cast keeps the
    // same id with a difficulty SpellInfo (src/server/game/Spells/SpellMgr.cpp:471-476). So no DUNGEON_MODE is
    // needed and the macros become the normal ids. 52658 still applies a periodic-trigger aura on 5.4.8
    // (SpellEffect.dbc: id 52658, effect 6, aura 23, trigger 52659).
    SPELL_STATIC_OVERLOAD           = 52658,
    SPELL_BALL_LIGHTNING            = 52780,
    SPELL_DISPERSE                  = 52770,
    NPC_SPARK_OF_IONAR              = 28926,

    // Loken
    // The fork's boss script itself switches on difficulty (boss_loken.cpp:31-32,250
    // DoCast(me, DUNGEON_MODE(SPELL_LIGHTNING_NOVA_N, SPELL_LIGHTNING_NOVA_H))), so AC's DUNGEON_MODE is kept.
    SPELL_LIGHTNING_NOVA_N          = 52960,
    SPELL_LIGHTNING_NOVA_H          = 59835,
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:35
#define SPELL_LIGHTNING_NOVA        DUNGEON_MODE(bot, SPELL_LIGHTNING_NOVA_N, SPELL_LIGHTNING_NOVA_H)

// AC Ai/Dungeon/HoL/HoLTriggers.h:38
class StormforgedLieutenantTrigger : public Trigger
{
public:
    StormforgedLieutenantTrigger(PlayerbotAI* ai) : Trigger(ai, "stormforged lieutenant") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:45
class BjarngrimWhirlwindTrigger : public Trigger
{
public:
    BjarngrimWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "bjarngrim whirlwind") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:52
class VolkhanTrigger : public Trigger
{
public:
    VolkhanTrigger(PlayerbotAI* ai) : Trigger(ai, "volkhan") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:59
class IonarStaticOverloadTrigger : public Trigger
{
public:
    IonarStaticOverloadTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar static overload") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:66
class IonarBallLightningTrigger : public Trigger
{
public:
    IonarBallLightningTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar ball lightning spread") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:73
class IonarTankAggroTrigger : public Trigger
{
public:
    IonarTankAggroTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar tank aggro") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:80
class IonarDisperseTrigger : public Trigger
{
public:
    IonarDisperseTrigger(PlayerbotAI* ai) : Trigger(ai, "ionar disperse") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:87
class LokenRangedTrigger : public Trigger
{
public:
    LokenRangedTrigger(PlayerbotAI* ai) : Trigger(ai, "loken ranged") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoL/HoLTriggers.h:94
class LokenLightningNovaTrigger : public Trigger
{
public:
    LokenLightningNovaTrigger(PlayerbotAI* ai) : Trigger(ai, "lightning nova") {}
    bool IsActive() override;
};

#endif
