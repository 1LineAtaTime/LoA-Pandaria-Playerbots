/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_AKTRIGGERS_H
#define PLAYERBOTS_AKTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/AK/AKTriggers.h:15
enum OldKingdomIDs
{
    // Elder Nadox
    // AC value kept (AC AKTriggers.h:18). Unused by any AC trigger/action. NOTE: the fork's Nadox script uses
    // SPELL_GUARDIAN_AURA = 56151 (src/server/scripts/Northrend/AzjolNerub/Ahnkahet/boss_elder_nadox.cpp:192).
    BUFF_GUARDIAN_AURA                 = 56153,

    // Jedoga Shadowseeker
    NPC_TWILIGHT_VOLUNTEER             = 30385,

    // Forgotten One(s)
    // 5.4.8 SPELL-ID DECISION (AC AKTriggers.h:24-28 unchanged): the Forgotten One (30414) is SmartAI in the
    // fork DB with both ids (smart_scripts entryorguid 30414 actions 11 -> 60833 and 60848, identical to
    // acore_world), not a C++ boss script, so DUNGEON_MODE is kept. SPELL_SHADOW_CRASH is only referenced in
    // commented-out code (AC AKActions.cpp:61), so it has no runtime effect either way.
    SPELL_SHADOW_CRASH_N               = 60833,
    SPELL_SHADOW_CRASH_H               = 60848,
};

#define SPELL_SHADOW_CRASH             DUNGEON_MODE(bot, SPELL_SHADOW_CRASH_N, SPELL_SHADOW_CRASH_H)

// AC Ai/Dungeon/AK/AKTriggers.h:30
class NadoxGuardianTrigger : public Trigger
{
public:
    NadoxGuardianTrigger(PlayerbotAI* ai) : Trigger(ai, "elder nadox guardian") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/AK/AKTriggers.h:37
class JedogaVolunteerTrigger : public Trigger
{
public:
    JedogaVolunteerTrigger(PlayerbotAI* ai) : Trigger(ai, "jedoga volunteer") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/AK/AKTriggers.h:44
class ShadowCrashTrigger : public Trigger
{
public:
    ShadowCrashTrigger(PlayerbotAI* ai) : Trigger(ai, "shadow crash") {}
    bool IsActive() override;
};

#endif
