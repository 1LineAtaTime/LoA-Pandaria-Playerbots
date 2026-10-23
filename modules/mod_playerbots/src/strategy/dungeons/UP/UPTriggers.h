/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_UPTRIGGERS_H
#define PLAYERBOTS_UPTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/UP/UPTriggers.h:15
enum UtgardePinnacleIDs
{
    // Skadi the Ruthless
    // 5.4.8 ADAPTATION: AC has SPELL_FREEZING_CLOUD_N = 47579 / _H = 60020 and SPELL_SKADI_WHIRLWIND_N = 50228 /
    // _H = 59322 with #define X DUNGEON_MODE(bot, X_N, X_H) (AC UPTriggers.h:16-17,20-21,30-31). The fork's
    // boss script casts a single id in every difficulty: SPELL_FREEZING_CLOUD = 47579 (src/server/scripts/
    // Northrend/UtgardeKeep/UtgardePinnacle/boss_skadi.cpp:36,691,753) and SPELL_WHIRLWIND = 50228
    // (boss_skadi.cpp:32,291); there is no heroic id in the script. A heroic cast keeps the same id with a
    // difficulty SpellInfo (src/server/game/Spells/SpellMgr.cpp:471-476), so the macros are the normal ids.
    // 47579 / 50228 still carry their auras on 5.4.8 (SpellEffect.dbc: 47579 effect 6 aura 4; 50228 effect 6
    // aura 23/33/138).
    SPELL_FREEZING_CLOUD            = 47579,
    // Fork: Grauf DoCasts 47592 (SPELL_FREEZING_CLOUD_RIGHT_PERIODIC, boss_skadi.cpp:38,434); same id as AC.
    // (The left breath uses 47590, boss_skadi.cpp:39,415, which AC does not check either.)
    SPELL_FREEZING_CLOUD_BREATH     = 47592,
    NPC_BREATH_TRIGGER              = 28351,
    SPELL_SKADI_WHIRLWIND           = 50228,

    // King Ymiron
    // The fork script itself switches on difficulty (boss_ymiron.cpp:23-24,245
    // DoCast(me, DUNGEON_MODE(SPELL_BANE, SPELL_BANE_H))), so AC's DUNGEON_MODE is kept.
    SPELL_BANE_N                    = 48294,
    SPELL_BANE_H                    = 59301,
};

// AC Ai/Dungeon/UP/UPTriggers.h:32
#define SPELL_BANE                  DUNGEON_MODE(bot, SPELL_BANE_N, SPELL_BANE_H)

// const float SKADI_BREATH_CENTRELINE = -512.46875f;

// AC Ai/Dungeon/UP/UPTriggers.h:36
class SkadiFreezingCloudTrigger : public Trigger
{
public:
    SkadiFreezingCloudTrigger(PlayerbotAI* ai) : Trigger(ai, "skadi freezing cloud") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UP/UPTriggers.h:43
class SkadiWhirlwindTrigger : public Trigger
{
public:
    SkadiWhirlwindTrigger(PlayerbotAI* ai) : Trigger(ai, "skadi whirlwind") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/UP/UPTriggers.h:50
class YmironBaneTrigger : public Trigger
{
public:
    YmironBaneTrigger(PlayerbotAI* ai) : Trigger(ai, "ymiron bane") {}
    bool IsActive() override;
};

#endif
