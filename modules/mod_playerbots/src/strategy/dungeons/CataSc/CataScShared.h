/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (map 725, "cata-sc"). Epic #373, issue #662.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids come from the fork's boss scripts (src/server/scripts/Maelstrom/TheStonecore/*.cpp)
// and the MoP test world DB.
// Intended mechanics (read 2026-10-06):
//   Wowpedia MediaWiki API: Corborus, Slabhide, Ozruk, High Priestess Azil (Abilities + Strategy sections);
//   Wowhead tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for cast times, radii and spell text.
// Only mechanics the fork's scripts actually drive have an edge here (see the lane report for the script gaps).

#ifndef PLAYERBOTS_CATASCSHARED_H
#define PLAYERBOTS_CATASCSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Common.h"
#include "Position.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace CataSc
{
constexpr uint32 MAP_ID = 725;

// Bosses and adds (fork the_stonecore.h:66-69, boss_*.cpp enums; MoP world DB creature_template)
constexpr uint32 NPC_CORBORUS           = 43438;
constexpr uint32 NPC_SLABHIDE           = 43214;
constexpr uint32 NPC_OZRUK              = 42188;
constexpr uint32 NPC_AZIL               = 42333;
constexpr uint32 NPC_CRYSTAL_SHARD      = 49267;  // boss_corborus.cpp:50 (heroic Crystal Barrage adds)
constexpr uint32 NPC_LAVA_FISSURE       = 43242;  // boss_slabhide.cpp:45
constexpr uint32 NPC_GRAVITY_WELL       = 42499;  // boss_high_priestess_azil.cpp:54
constexpr uint32 NPC_DEVOUT_FOLLOWER    = 42428;  // :53

// Spells (fork boss scripts; every id verified against the script that casts or applies it)
constexpr uint32 SPELL_GROUND_SLAM      = 78903;  // boss_ozruk.cpp:31, 3 s cast, frontal, 4 yd at the impact point
constexpr uint32 SPELL_SHATTER          = 78807;  // :32, 3 s cast, 15 yd around Ozruk
constexpr uint32 SPELL_STALACTITE_AREA  = 80654;  // boss_slabhide.cpp:28, the warning circle (persistent area aura)
constexpr uint32 SPELL_CRYSTAL_BARRAGE  = 86881;  // boss_corborus.cpp:23, 4 s channel, 5 yd persistent area on the target
constexpr uint32 SPELL_ARCANE_SHIELD    = 79050;  // boss_high_priestess_azil.cpp:31 (Energy Shield: -75% damage taken)

// Geometry
constexpr float HAZARD_MARGIN           = 1.0f;   // a bot counts as inside at radius + margin
constexpr float HAZARD_SAFE_EXTRA       = 1.5f;   // and runs to radius + this
constexpr float SHATTER_RADIUS          = 15.0f;  // Wowhead 78807: "all enemies within 15 yards"
constexpr float SHATTER_SAFE_RADIUS     = 17.5f;
constexpr float GROUND_SLAM_ARC         = 1.5708f;  // 90 deg in front of Ozruk: the 4 yd impact point lands there
constexpr float GROUND_SLAM_RANGE       = 16.0f;    // (heroic Rupture lines are not scripted on the fork: Rupture Controller 49597 has no AI)
constexpr float SAND_BLAST_ARC          = 1.3963f;  // 80 deg: Wowpedia says 60 deg cone, 15 yd; margin for the turn
constexpr float SAND_BLAST_RANGE        = 17.0f;
constexpr float SCAN_RANGE              = 80.0f;

// Ground effects that exist as creatures for the lifetime of the effect
enum class HazardId : uint8
{
    LAVA_FISSURE,  // Slabhide: crack under a random player, erupts after 5 s (3 s heroic), then a lava pool (5 yd)
    GRAVITY_WELL,  // Azil: pulls everything within 10 yd towards its centre and burns it
    COUNT
};

// Ground effects that exist as DynamicObjects of a spell
enum class DynHazardId : uint8
{
    STALACTITE,       // Slabhide: warning circle, the stalactite lands on it 6 s later (19k + knockback)
    CRYSTAL_BARRAGE,  // Corborus: 4 s of 9.5k per half second within 5 yd of the target's position
    COUNT
};

// What the DPS bots drop everything for
enum class FocusId : uint8
{
    CRYSTAL_SHARDS,   // Corborus heroic: shards walk at a player and explode (20k + 25% damage taken)
    AZIL_FOLLOWERS,   // Azil's Energy Shield phase: she takes -75% damage, the followers are what dies
    COUNT
};

// Cheap lookups ------------------------------------------------------------------------------------------------
Creature* FindCreature(Player* bot, uint32 entry, float range);
bool IsCasting(Unit* unit, uint32 spellId);

// The nearest live hazard creature of this kind the bot is standing in (or next to); nullptr when clear.
Creature* GetHazardCreature(Player* bot, HazardId id);
float HazardSafeRadius(HazardId id);

// The nearest DynamicObject of this kind the bot is standing in (or next to).
bool GetDynHazard(Player* bot, DynHazardId id, float& cx, float& cy);
float DynHazardSafeRadius(DynHazardId id);

// Ozruk
Creature* GetOzruk(Player* bot);
bool ShatterDanger(Player* bot);
bool GroundSlamDanger(Player* bot);

// Slabhide
Creature* GetSlabhide(Player* bot);
bool SandBlastDanger(Player* bot);
// True when the bot is inside the frontal arc (full width in radians) of the unit, within range
bool InFrontArc(Player* bot, Unit* boss, float arc, float range);

// Focus targets
bool IsFocusDps(Player* bot);
Unit* GetFocusTarget(Player* bot, FocusId id);
Unit* GetAnyFocusTarget(Player* bot);

// True while one of this dungeon's "get out of the way now" mechanics applies to the bot; the evade multiplier
// then keeps formation / follow / reach movement from walking the bot back in.
bool InEvadeDanger(Player* bot);
}

#endif
