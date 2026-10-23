/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAHOOSHARED_H
#define PLAYERBOTS_CATAHOOSHARED_H

#include "Define.h"
#include <vector>

class Creature;
class GameObject;
class Player;
class Unit;

namespace CataHoo
{
constexpr float PI_F = 3.14159265358979f;

// A circle on the ground (2D) the bot has to stay out of.
struct Hazard
{
    float x;
    float y;
    float radius;
};

// Optional bound for escape spots (radius <= 0 means no bound), e.g. the edge of a platform.
struct Leash
{
    float x;
    float y;
    float radius;
};

Creature* FindNearestAlive(Player* bot, uint32 entry, float range);
void CollectAlive(Player* bot, std::vector<uint32> const& entries, float range, std::vector<Creature*>& out);
// Alive and in combat, so a boss that has not been pulled yet never triggers anything.
Creature* FindEngagedBoss(Player* bot, uint32 entry, float range = 200.0f);
// The one among several candidates with the lowest guid: every bot picks the same unit, so marks never flip-flop.
Creature* LowestGuid(std::vector<Creature*> const& units);
bool IsCasting(Unit* caster, uint32 spellId);
// The unit a caster's current cast is aimed at (nullptr when it has none).
Unit* GetCastTarget(Player* bot, Unit* caster, uint32 spellId);
// Group members that are alive, in the world and on the bot's map.
void CollectGroupMembers(Player* bot, std::vector<Player*>& out, bool includeSelf);
bool OthersCentroid(Player* bot, float& cx, float& cy);

float AngleDiff(float a, float b);  // signed (a - b) in (-pi, pi]
float GroundZ(Player* bot, float x, float y);
bool InsideHazard(Player* bot, std::vector<Hazard> const& hazards, float margin);
// Nearest reachable spot (rings of 3 yd around the bot, 16 directions) outside every hazard.
bool FindSafeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, Leash const& leash,
                  float maxTravel, float& outX, float& outY);
// Is the bot's own step to (x, y) free of collisions?
bool ClearStep(Player* bot, float x, float y);

bool InsideCone(float originX, float originY, float axis, float halfAngle, float length, float x, float y);
// If the bot stands inside the cone (widened by angleMargin), a spot just outside its edge on the same radius,
// when it can be reached within maxMove yards.
bool FindConeExit(Player* bot, float originX, float originY, float axis, float halfAngle, float length,
                  float angleMargin, float maxMove, float& outX, float& outY);
}

// Halls of Origination (map 644, strategy "cata-hoo") - issue #665.
// Intended mechanics: Wowpedia (fetched 2026-10-06) Temple_Guardian_Anhuur, Earthrager_Ptah, Anraphet, Isiset, Ammunae,
// Setesh, Rajh; Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>, 2026-10-06). Ids and
// behaviour are the ones of THIS core's scripts: src/server/scripts/Kalimdor/HallsOfOrigination/boss_*.cpp and
// halls_of_origination.h.
namespace CataHoo
{
// MoP test DB creature_template (world DB) entries
constexpr uint32 NPC_TEMPLE_GUARDIAN_ANHUUR = 39425;   // halls_of_origination.h:55
constexpr uint32 NPC_EARTHRAGER_PTAH = 39428;          // halls_of_origination.h:56
constexpr uint32 NPC_ANRAPHET = 39788;                 // halls_of_origination.h:57
constexpr uint32 NPC_ISISET = 39587;                   // halls_of_origination.h:58
constexpr uint32 NPC_AMMUNAE = 39731;                  // halls_of_origination.h:59
constexpr uint32 NPC_SETESH = 39732;                   // halls_of_origination.h:60
constexpr uint32 NPC_RAJH = 39378;                     // halls_of_origination.h:61
constexpr uint32 NPC_SEARING_LIGHT = 40283;            // halls_of_origination.h:82, hits 4 yd for 10 s (summoned by 75115, boss_temple_guardian_anhuur.cpp:226)
constexpr uint32 NPC_ALPHA_BEAM = 41144;               // halls_of_origination.h:84
constexpr uint32 NPC_SOLAR_WINDS_1 = 39634;            // boss_rajh.cpp:61
constexpr uint32 NPC_SOLAR_WINDS_2 = 39635;            // boss_rajh.cpp:62
constexpr uint32 NPC_SOLAR_WINDS_3 = 47922;            // boss_rajh.cpp:63
constexpr uint32 NPC_ASTRAL_RAIN = 39720;              // boss_isiset.cpp:59, Mirror Image of Astral Rain
constexpr uint32 NPC_CELESTIAL_CALL = 39721;           // boss_isiset.cpp:60
constexpr uint32 NPC_VEIL_OF_SKY = 39722;              // boss_isiset.cpp:61
constexpr uint32 NPC_SEEDLING_POD = 40550;             // boss_ammunae.cpp:66
constexpr uint32 NPC_SEEDLING_POD_2 = 40716;           // boss_ammunae.cpp:70
constexpr uint32 NPC_SPORE = 40585;                    // boss_ammunae.cpp:69
constexpr uint32 NPC_VOID_SEEKER = 41371;              // boss_setesh.cpp:49
constexpr uint32 NPC_CHAOS_PORTAL = 41055;             // boss_setesh.cpp:51
// #691 (summoned by the ported mechanics in boss_earthrager_ptah.cpp / boss_setesh.cpp / boss_rajh.cpp)
constexpr uint32 NPC_QUICKSAND_POOL = 40503;           // Ptah EVENT_QUICKSAND, a pool near a player every 10 s while submerged
constexpr uint32 NPC_REIGN_OF_CHAOS = 41168;           // Setesh Reign of Chaos bubble, 5 s
constexpr uint32 NPC_CHAOS_BLAST = 41041;              // Setesh Chaos Blast: lands 5 s after it appears (aura 76681), then a void zone
constexpr uint32 NPC_ORB_OF_SUN = 40835;               // Rajh Sun Orb: 15 yd above a player, lands 3.5 s later
constexpr uint32 NPC_INFERNO_LEAP = 47040;             // Rajh Inferno Leap landing marker, he lands on it 1.5 s later
constexpr uint32 GO_BEACON_LEFT = 203133;              // boss_temple_guardian_anhuur.cpp:70
constexpr uint32 GO_BEACON_RIGHT = 203136;             // boss_temple_guardian_anhuur.cpp:71

// Spell ids as cast by the fork's scripts
constexpr uint32 SPELL_SHIELD_OF_LIGHT = 74938;        // boss_temple_guardian_anhuur.cpp:38
constexpr uint32 SPELL_DIVINE_RECKONING = 75592;       // boss_temple_guardian_anhuur.cpp:32 (aura on the target, 8 s)
constexpr uint32 SPELL_QUICKSAND = 75546;              // boss_earthrager_ptah.cpp:36, self aura while submerged (:191)
constexpr uint32 SPELL_SUPERNOVA = 74136;              // boss_isiset.cpp:31, cast :220-224 (3 s)
constexpr uint32 SPELL_SPORE_CLOUD = 75701;            // boss_ammunae.cpp:50
constexpr uint32 SPELL_SOLAR_WINDS_AURA = 74107;       // boss_rajh.cpp:36, on the vortex (:235)
// #691: the damage spells whose core-applied radius (SpellInfo, after the SpellMgr.cpp overrides) the bots keep out of
constexpr uint32 SPELL_QUICKSAND_DMG = 75547;          // pool pulse (DBC RadiusIndex 26 = 4 yd)
constexpr uint32 SPELL_QUICKSAND_SLOW = 75548;         // pool slow (6 yd)
constexpr uint32 SPELL_REIGN_OF_CHAOS_DMG = 77030;     // effect 1: 6000 within 2 yd
constexpr uint32 SPELL_CHAOS_BLAST = 76681;            // impact (effect 0), carried by a landed Chaos Blast
constexpr uint32 SPELL_CHAOS_BURN = 76684;             // the void zone pulse (effect 0)
constexpr uint32 SPELL_BLAZING_INFERNO = 76195;        // Sun Orb impact (effect 1, 13 yd)
constexpr uint32 SPELL_INFERNO_LEAP = 87647;           // Inferno Leap landing (effect 0, 10 yd)
constexpr uint32 SPELL_NOXIOUS_SPORES = 75702;         // Ammunae spore cloud damage (effect 0)

// Anhuur: Searing Light (npc 40283) burns 9.7k/s within 4 yd (SpellRadius 26 on 75117); Divine Reckoning 75592 pulses 75591
// after 8 s on everyone within 15 yd of its target (SpellRadius 18; 42.75-47.25k siphoned, the boss heals 5x). The platform
// is roughly 20 yd across (boss spawn -640.7 334.9 77.8): the escape spots are leashed to it so nobody walks off the edge.
constexpr float PLATFORM_X = -640.5f;                  // bosspos (boss_temple_guardian_anhuur.cpp:99)
constexpr float PLATFORM_Y = 334.9f;
constexpr float PLATFORM_RADIUS = 24.0f;
constexpr float SEARING_LIGHT_RADIUS = 4.0f;
constexpr float SEARING_LIGHT_MARGIN = 1.5f;
constexpr float RECKONING_RADIUS = 15.0f;
constexpr float RECKONING_MARGIN = 1.0f;
// The beacons are on the pit floor (z 54.6), the boss and the group on the platform (z 77.8); at 67% and 34% he raises
// the Shield of Light (immune to damage and to interrupts) until both beacons have been used.
constexpr float BEACON_USE_RANGE = 4.5f;
constexpr float BEACON_DROP_HEIGHT = 8.0f;     // the bot is this far above the beacon: it has to get down first
constexpr float PIT_DEPTH = 12.0f;             // the bot is this far below the boss: it is in the pit

// Ptah, Tumultuous Earthstorm: Quicksand 75546 pulses 75547 (19.5-20.5k and a 50% slow) every 2 s. #691: it is carried by
// the pools (npc 40503) the script now summons near the players; the boss-aura form (the fork's old stand-in, 4 yd round
// the submerged boss) is still recognised. A pool hurts within the larger of 75547 / 75548 (6 yd on this core; Wowpedia 7).
constexpr float QUICKSAND_RADIUS = 4.0f;
constexpr float QUICKSAND_MARGIN = 2.5f;
constexpr float QUICKSAND_POOL_RADIUS = 7.0f;  // fallback only (Wowpedia Earthrager_Ptah "7 yard radius pools")
constexpr float QUICKSAND_POOL_MARGIN = 1.5f;

// #691 Setesh: Reign of Chaos bubbles (Wowpedia "2 yard radius bubbles ... 6000 every second", 5 s) and Chaos Blast (Wowpedia
// "inflicting 24375-25625 ... within 12 yards", then "a 10 yard radius void zone"; the core applies the DBC RadiusIndex, 4 yd,
// to both 76681 and 76684). Radii are read from SpellInfo; the Wowpedia values are the fallback.
constexpr float REIGN_OF_CHAOS_RADIUS = 2.0f;
constexpr float CHAOS_BLAST_RADIUS = 12.0f;
constexpr float CHAOS_BURN_RADIUS = 10.0f;
constexpr float CHAOS_MARGIN = 1.5f;

// #691 Rajh: the Sun Orb lands for 76195 (13 yd, Wowpedia "within 13 yards"), Inferno Leap lands for 87647 (10 yd). Escape
// spots also stay out of the Solar Winds and are leashed to the room.
constexpr float SUN_ORB_RADIUS = 13.0f;
constexpr float INFERNO_LEAP_RADIUS = 10.0f;
constexpr float RAJH_HAZARD_MARGIN = 1.5f;

// Anraphet: Alpha Beams leave void zones of 5 yd (Wowpedia Anraphet; Wowhead 76184), npc 41144.
constexpr float ALPHA_BEAM_RADIUS = 5.0f;
constexpr float ALPHA_BEAM_MARGIN = 2.0f;

// Isiset: Supernova 74136 (3 s cast) blinds every player that has her in its front arc; the script's filter is
// !unit->isInFront(caster, 2.5f) (boss_isiset.cpp:575-581). Stay out of it with a margin.
constexpr float SUPERNOVA_ARC = 3.0f;

// Ammunae: a popped Spore leaves Noxious Spores (75702) of 6 yd for 30 s (Wowpedia Ammunae; Wowhead 75702 "within 6 yards").
// #691: the core applies another radius - DBC RadiusIndex 23 = 40 yd, overridden to 13 = 10 yd at SpellMgr.cpp:4207 - so the
// bots read the live one from SpellInfo; 6 yd is the fallback.
constexpr float SPORE_CLOUD_RADIUS = 6.0f;
constexpr float SPORE_CLOUD_MARGIN = 1.5f;

// Rajh: the Solar Winds vortex pulses 74108 (9.6-11.2k and a knock back) within 15 yd of itself every 1.5 s (SpellRadius 18
// on 74108, aura 74107 on the vortex). The room is centred on pointcenter (boss_rajh.cpp:51).
constexpr float SOLAR_WINDS_RADIUS = 15.0f;
constexpr float SOLAR_WINDS_MARGIN = 1.5f;
constexpr float RAJH_ROOM_X = -319.68f;
constexpr float RAJH_ROOM_Y = 193.42f;
constexpr float RAJH_ROOM_RADIUS = 38.0f;

// Each pair: Needed() is true exactly when Target() has somewhere to send the bot (triggers and the movement lock use
// it), Target() picks the spot.
bool SearingLightNeeded(Player* bot);
bool SearingLightTarget(Player* bot, float& x, float& y);
bool ReckoningNeeded(Player* bot);
bool ReckoningTarget(Player* bot, float& x, float& y);
bool QuicksandNeeded(Player* bot);
bool QuicksandTarget(Player* bot, float& x, float& y);
bool AlphaBeamNeeded(Player* bot);
bool AlphaBeamTarget(Player* bot, float& x, float& y);
bool SporeCloudNeeded(Player* bot);
bool SporeCloudTarget(Player* bot, float& x, float& y);
bool SolarWindsNeeded(Player* bot);
bool SolarWindsTarget(Player* bot, float& x, float& y);
// #691
bool ReignOfChaosNeeded(Player* bot);
bool ReignOfChaosTarget(Player* bot, float& x, float& y);
bool ChaosBlastNeeded(Player* bot);
bool ChaosBlastTarget(Player* bot, float& x, float& y);
bool SunOrbNeeded(Player* bot);
bool SunOrbTarget(Player* bot, float& x, float& y);
bool InfernoLeapNeeded(Player* bot);
bool InfernoLeapTarget(Player* bot, float& x, float& y);
// The radius one effect of a spell is applied with on this core (SpellInfo, so SpellMgr overrides count); fallback if none.
float EffectRadius(uint32 spellId, uint8 effIndex, float fallback);

// Anhuur's beacons: the beacon this bot has to use right now (nullptr when it has no duty or the boss is not shielded).
// The two duty bots are the first two non-tank bots of the group, damage dealers before healers, lowest guid first.
bool BeaconNeeded(Player* bot);
bool BeaconPlan(Player* bot, GameObject*& beacon);
// A bot left in the pit once the shield is down jumps back up to the boss.
bool PitReturnNeeded(Player* bot);
bool PitReturnTarget(Player* bot, float& x, float& y, float& z);

// Isiset's Supernova: this bot is not a tank and the boss is casting it.
bool SupernovaCasting(Player* bot);
bool SupernovaNeeded(Player* bot);
float SupernovaFacing(Player* bot);

bool AnyMovementLock(Player* bot);

// Add priorities: the unit to skull, nullptr when none (or the boss is not engaged).
Creature* PriorityImage(Player* bot);
Creature* PriorityPod(Player* bot);
Creature* PrioritySetesh(Player* bot);
}

#endif
