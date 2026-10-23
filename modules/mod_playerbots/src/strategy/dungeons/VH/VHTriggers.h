/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_VHTRIGGERS_H
#define PLAYERBOTS_VHTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/VH/VHTriggers.h:15
enum VioletHoldIDs
{
    // Ichoron
    // 5.4.8 check: the fork's boss script DoCasts 59820 (src/server/scripts/Northrend/VioletHold/boss_ichoron.cpp:23,126); same id as AC.
    SPELL_DRAINED                      = 59820,
    NPC_ICHOR_GLOBULE                  = 29321,

    // Zuramat the Obliterator
    // 5.4.8 check: fork constants src/server/scripts/Northrend/VioletHold/boss_zuramat.cpp:31 (VOID_SHIFTED 54343),
    // :25-26 (SHROUD_OF_DARKNESS 54524 / _H 59745); same ids as AC.
    SPELL_VOID_SHIFTED                 = 54343,
    SPELL_SHROUD_OF_DARKNESS_N         = 54524,
    SPELL_SHROUD_OF_DARKNESS_H         = 59745,
    NPC_VOID_SENTRY                    = 29364,
};

// AC Ai/Dungeon/VH/VHTriggers.h:28
// 5.4.8 SPELL-ID RULE: the fork's boss script itself switches to the _H id in heroic
// (src/server/scripts/Northrend/VioletHold/boss_zuramat.cpp:133,
// DoCast(me, DUNGEON_MODE(SPELL_SHROUD_OF_DARKNESS, SPELL_SHROUD_OF_DARKNESS_H))),
// so DUNGEON_MODE is kept as in AC (SpellMgr.cpp:471-476 does not make it redundant here).
#define SPELL_SHROUD_OF_DARKNESS    DUNGEON_MODE(bot, SPELL_SHROUD_OF_DARKNESS_N, SPELL_SHROUD_OF_DARKNESS_H)

// AC Ai/Dungeon/VH/VHTriggers.h:30
class ErekemTargetTrigger : public Trigger
{
public:
    ErekemTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "erekem target") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/VH/VHTriggers.h:37
class IchoronTargetTrigger : public Trigger
{
public:
    IchoronTargetTrigger(PlayerbotAI* ai) : Trigger(ai, "ichoron target") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/VH/VHTriggers.h:44
class VoidShiftTrigger : public Trigger
{
public:
    VoidShiftTrigger(PlayerbotAI* ai) : Trigger(ai, "void shift") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/VH/VHTriggers.h:51
class ShroudOfDarknessTrigger : public Trigger
{
public:
    ShroudOfDarknessTrigger(PlayerbotAI* ai) : Trigger(ai, "shroud of darkness") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/VH/VHTriggers.h:58
class CyanigosaPositioningTrigger : public Trigger
{
public:
    CyanigosaPositioningTrigger(PlayerbotAI* ai) : Trigger(ai, "cyanigosa positioning") {}
    bool IsActive() override;
};

#endif
