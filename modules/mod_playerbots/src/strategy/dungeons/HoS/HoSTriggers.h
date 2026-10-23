/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_HOSTRIGGERS_H
#define PLAYERBOTS_HOSTRIGGERS_H

#include "DungeonStrategyUtils.h"
#include "GenericTriggers.h"
#include "PlayerbotAIConfig.h"
#include "Trigger.h"

// AC Ai/Dungeon/HoS/HoSTriggers.h:13
enum HallsOfStoneIDs
{
    // Krystallus
    SPELL_GROUND_SLAM               = 50827,
    DEBUFF_GROUND_SLAM              = 50833,

    // Sjonnir The Ironshaper
    // 5.4.8 ADAPTATION: AC has SPELL_LIGHTNING_RING_N = 50840 / _H = 59848 and
    // #define SPELL_LIGHTNING_RING DUNGEON_MODE(bot, N, H) (AC HoSTriggers.h:23-27).
    // The fork's boss script casts a single id in every difficulty: it DoCasts SPELL_LIGHTING_RING = 51849
    // (src/server/scripts/Northrend/Ulduar/HallsOfStone/boss_sjonnir.cpp:24,160); 50840 is only the
    // unused "_1" constant (boss_sjonnir.cpp:25). Heroic keeps the same id with a difficulty SpellInfo
    // (src/server/game/Spells/SpellMgr.cpp:471-476), so no DUNGEON_MODE is needed.
    // Reference behaviour (lead, #550): AC's own script casts 50840 every 40 s (AC
    // src/server/scripts/Northrend/Ulduar/HallsOfStone/boss_sjonnir.cpp:200-203) and 51849 only below 20% hp
    // (:181-183); the fork's script casts 51849 every 30-35 s (fork boss_sjonnir.cpp:102,158-160). Watching the
    // ring the fork's boss actually casts keeps AC's "avoid the periodic ring" behaviour.
    SPELL_LIGHTNING_RING            = 51849,
};

// AC Ai/Dungeon/HoS/HoSTriggers.h:27
class KrystallusGroundSlamTrigger : public Trigger
{
public:
    KrystallusGroundSlamTrigger(PlayerbotAI* ai) : Trigger(ai, "krystallus ground slam") {}
    bool IsActive() override;
};

// AC Ai/Dungeon/HoS/HoSTriggers.h:34
class SjonnirLightningRingTrigger : public Trigger
{
public:
    SjonnirLightningRingTrigger(PlayerbotAI* ai) : Trigger(ai, "sjonnir lightning ring") {}
    bool IsActive() override;
};

#endif
