/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man, map 568, level 85) bot strategy "cata-za" (MoP-only lane L7, issue #669, epic #373).
// AC has no strategy for this dungeon, so this follows AC's dungeon-strategy PATTERN (Ai/Dungeon/HoS, UB, MgT):
// Strategy + Triggers + Actions + Multipliers + ActionContext/TriggerContext. No AC file is ported 1:1.
//
// Intended mechanics, sources (fetched 2026-10-06):
//  - Wowpedia MediaWiki API: Akil'zon (tactics), Nalorakk (tactics), Jan'alai (tactics), Halazzi (tactics),
//    Hex Lord Malacrass, Daakara, Zul'Aman (instance).
//  - Spell/NPC ids and radii: the fork's own boss scripts src/server/scripts/EasternKingdoms/ZulAman/*.cpp and
//    the 5.4.8 SpellEffect.dbc / SpellRadius.dbc (DynamicObject = effect 27 PERSISTENT_AREA_AURA).
// A mechanic the fork's script never casts is deliberately NOT given an edge (see the issue comment for the gaps).

#ifndef PLAYERBOTS_CATAZASHARED_H
#define PLAYERBOTS_CATAZASHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace CataZulAman
{
constexpr uint32 MAP_ZUL_AMAN = 568;

// Bosses (fork scripts: boss_akilzon.cpp zulaman.h:36-41)
constexpr uint32 NPC_AKILZON  = 23574;
constexpr uint32 NPC_NALORAKK = 23576;
constexpr uint32 NPC_JANALAI  = 23578;
constexpr uint32 NPC_HALAZZI  = 23577;
constexpr uint32 NPC_MALACRASS = 24239;
constexpr uint32 NPC_DAAKARA  = 23863;

// Adds and ground-effect creatures
constexpr uint32 NPC_AMANI_KIDNAPPER = 52638; // boss_akilzon.cpp:48 (carries a Plucked player)
constexpr uint32 NPC_FIRE_BOMB       = 23920; // boss_janalai.cpp:63
constexpr uint32 NPC_HATCHER_1       = 23818; // boss_janalai.cpp:64
constexpr uint32 NPC_HATCHER_2       = 24504; // boss_janalai.cpp:65
constexpr uint32 NPC_LIGHTNING_TOTEM = 24224; // boss_halazzi.cpp:61
constexpr uint32 NPC_WATER_TOTEM     = 52755; // boss_halazzi.cpp:62
constexpr uint32 NPC_ALYSON_ANTILE   = 24240; // boss_hexlord.cpp:99
constexpr uint32 NPC_SLITHER         = 24242;
constexpr uint32 NPC_GAZAKROTH       = 24244;
constexpr uint32 NPC_DARKHEART       = 24246;
constexpr uint32 NPC_PILLAR_OF_FLAME = 24187; // boss_daakara.cpp:112
constexpr uint32 NPC_FEATHER_VORTEX  = 24136; // #678 Daakara eagle: Feather Cyclone (boss_daakara.cpp NPC_FEATHER_VORTEX)
constexpr uint32 NPC_BLOOD_WORM      = 52827; // #678 Malacrass Death Knight kit (boss_hexlord.cpp NPC_BLOOD_WORM)

// Spells
constexpr uint32 SPELL_ELECTRICAL_STORM = 43648; // Akil'zon, aura on the lifted victim (boss_akilzon.cpp:39)
constexpr uint32 SPELL_ELECTRICAL_SAFE  = 44007; // persistent area aura (DynamicObject) around the victim, 10 yd
constexpr uint32 SPELL_SURGE            = 42402; // Nalorakk / Daakara bear: +500% physical damage taken debuff
constexpr uint32 SPELL_FIRE_BOMB_DAMAGE = 42630; // boss_janalai.cpp:36, radius 4 yd
constexpr uint32 SPELL_FLAME_BREATH     = 43140; // Jan'alai + Daakara (dragonhawk): cone cast, aura lays the fire line
constexpr uint32 SPELL_FLAME_BREATH_LINE = 97497; // persistent area aura (DynamicObject), 4 yd (boss_janalai.cpp:29)
constexpr uint32 SPELL_WHIRLWIND        = 17207; // Daakara phase 1: aura on the boss, periodic whirlwind 15576
constexpr uint32 SPELL_RAIN_OF_FIRE     = 43440; // Malacrass siphon (Warlock): persistent area aura, 5 yd
constexpr uint32 SPELL_CONSECRATION     = 43429; // Malacrass siphon (Paladin): persistent area aura, 10 yd
constexpr uint32 SPELL_DEATH_AND_DECAY  = 61603; // #678 Malacrass siphon (Death Knight): persistent area aura, 5 yd (SpellRadius 8)
constexpr uint32 SPELL_SMOKE_BOMB_AREA  = 97644; // #678 Malacrass siphon (Rogue): persistent area auras, 7 yd (SpellRadius 37)

// Malacrass heals / hard-hitting casts worth an interrupt (boss_hexlord.cpp:25-83)
constexpr uint32 SPELL_ALYSON_FLASH_HEAL = 43575;
constexpr uint32 SPELL_PR_HEAL           = 41372;
constexpr uint32 SPELL_PR_FLASH_HEAL     = 43431; // #678 Cata priest kit: 2 s cast, heals Malacrass for 20%
constexpr uint32 SPELL_PA_HOLY_LIGHT     = 43451;
constexpr uint32 SPELL_SH_HEALING_WAVE   = 43548;
constexpr uint32 SPELL_SH_CHAIN_LIGHTNING = 43435;
constexpr uint32 SPELL_MG_FROSTBOLT      = 43428;
constexpr uint32 SPELL_MG_FIREBALL       = 41383;
constexpr uint32 SPELL_VENOM_SPIT        = 43579;

// Geometry (yards / radians)
constexpr float BOSS_SCAN_RANGE      = 120.0f;
constexpr float HAZARD_SCAN_RANGE    = 80.0f;
constexpr float FLAME_BREATH_LENGTH  = 50.0f; // spell 43140 range 50 yd
constexpr float FLAME_BREATH_HALF_ANGLE = 0.70f; // ~40 deg: Wowpedia says a 60 deg cone, margin on top
constexpr float WHIRLWIND_RADIUS     = 9.0f;
constexpr float PILLAR_RADIUS        = 3.0f;
constexpr float SURGE_HOLD_DISTANCE  = 10.0f; // a Surge-debuffed bot stays this close so it is not the "most distant"
constexpr float SURGE_CLOSE_DISTANCE = 5.0f;  // centre distance the Surge close-in walks to (well inside SURGE_HOLD_DISTANCE)
constexpr float SPREAD_MIN_DISTANCE  = 13.0f; // Akil'zon Static Disruption 44008: 12 yd blast (SpellRadius idx 32) + a body width
constexpr float SPREAD_MAX_FROM_BOSS = 28.0f; // a spread spot stays on Akil'zon's platform, in range of him and the healer
constexpr float SPREAD_MAX_FROM_MEMBER = 21.0f; // ... and within this of every other member: any storm eye is ~2 pulses away
constexpr float WATER_TOTEM_RADIUS   = 10.0f; // Refreshing Stream 97502 heals everything within ~10 yd
constexpr float WATER_TOTEM_CLEAR    = 13.0f;
// #678 Daakara eagle. Storm 43121 (Feather Cyclone 43120 tick, 500 ms) has SpellEffect radius index 0: it hits a unit the cyclone
// touches (Wowpedia: "within 2 yds"). A cyclone that Sweeping Winds set moving runs straight to where a random player stood
// (boss_daakara.cpp npc_daakara_vortex DoAction); its path is a row of circles PATH_STEP apart.
constexpr float CYCLONE_RADIUS       = 4.0f;
constexpr float CYCLONE_PATH_STEP    = 3.0f;

constexpr float STAND_MARGIN = 0.5f; // a bot closer than this to a hazard edge counts as inside it
constexpr float CLEAR_MARGIN = 1.5f; // an escape spot is wanted at least this far outside every hazard

enum class HazardKind : uint8
{
    FireBombs,        // Jan'alai: armed Fire Bombs
    FlameBreath,      // Jan'alai / Daakara: the cone while cast, the fire line (DynamicObject) afterwards
    PillarOfFlame,    // Daakara dragonhawk phase
    DaakaraWhirlwind, // Daakara phase 1, non-tanks
    MalacrassGround,  // Malacrass siphoned Rain of Fire / Consecration / Death and Decay / Smoke Bomb (DynamicObjects)
    FeatherCyclone    // #678 Daakara eagle: Feather Cyclones and the line a moving one runs along
};

enum class HazardShape : uint8 { Circle, Cone };

struct Hazard
{
    HazardShape shape;
    float x;
    float y;
    float radius;    // circle radius, or cone length
    float angle;     // cone direction (radians, same frame as Position::GetAngle)
    float halfAngle; // cone half opening
};

// Distance by which (x, y) lies outside the hazard; negative when inside.
float HazardSlack(Hazard const& hazard, float x, float y);
float MinSlack(std::vector<Hazard> const& hazards, float x, float y);

// Max effect radius of a spell (SpellInfo::Effects[].CalcRadius, cf. UBShared.cpp MaxEffectRadius).
float SpellRadius(uint32 spellId, float fallback);

Creature* FindBoss(Player* bot, uint32 entry);

void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out);

// Ring search for the nearest spot clear of every hazard (pattern: MgTShared.cpp CollectHazardEscapes).
// With an anchor, a spot must also lie within anchorRange (2D) of it and on its level (|dz| <= 3): no spot down a staircase.
// With maxFromHazards > 0, a spot must also lie within that 2D distance of every hazard centre (a compact spread).
bool FindEscapeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, float& outX, float& outY,
                    float& outZ, Unit const* anchor = nullptr, float anchorRange = 0.0f, float maxFromHazards = 0.0f);

// Akil'zon Electrical Storm: the victim is lifted and a 10 yd "eye" (DynamicObject 44007) forms under it;
// every other player outside the eye takes the storm damage. Returns the nearest eye centre when the bot is
// neither the victim nor already inside the eye.
bool GetStormEye(Player* bot, float& centreX, float& centreY, float& centreZ, float& radius);
bool IsStormActive(Player* bot);

// Akil'zon Static Disruption (instant, 12 yd blast on a random player, stacking +25% nature damage taken).
// Not while an Electrical Storm is up (everybody stacks in the eye then).
bool NeedsSpread(Player* bot, Player*& nearest);
// One SPREAD_MIN_DISTANCE circle around every other living group member on the map.
void CollectSpreadHazards(Player* bot, std::vector<Hazard>& out);

// A bot carrying Surge (Nalorakk / Daakara bear form) is returned the boss it should stand next to.
Unit* GetSurgeAnchor(Player* bot);

// Halazzi: tank only, the boss stands inside a Water Totem's healing range.
Unit* GetWaterTotemToLeave(Player* bot);

// Priority focus target for non-tank DPS (Kidnapper, Hatchers, Lightning Totem, Malacrass servants); for the tank, Akil'zon.
Unit* GetFocusTarget(Player* bot);

// Malacrass: a caster (boss or servant) currently casting a heal / hard cast, best (lowest) priority first.
Unit* GetInterruptTarget(Player* bot);

// Dedicated interrupt (kick, pummel, ...), first that can be cast on the target.
bool CastDedicatedInterrupt(PlayerbotAI* botAI, Unit* target);
}

#endif
