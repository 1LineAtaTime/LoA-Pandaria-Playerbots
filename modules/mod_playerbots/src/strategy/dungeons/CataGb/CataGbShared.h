/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAGBSHARED_H
#define PLAYERBOTS_CATAGBSHARED_H

#include "Define.h"
#include <vector>

class Creature;
class GameObject;
class Player;
class Unit;

namespace CataGb
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

// Grim Batol (map 670, strategy "cata-gb") - issue #664.
// Intended mechanics: Wowpedia (fetched 2026-10-06) General_Umbriss, Forgemaster_Throngus, Drahga_Shadowburner, Erudax;
// Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>, 2026-10-06). Ids and behaviour are the
// ones of THIS core's scripts: src/server/scripts/EasternKingdoms/GrimBatol/boss_*.cpp and grim_batol.h.
namespace CataGb
{
// MoP test DB creature_template (world DB) entries
constexpr uint32 NPC_GENERAL_UMBRISS = 39625;          // grim_batol.h:65
constexpr uint32 NPC_FORGEMASTER_THRONGUS = 40177;     // grim_batol.h:66
constexpr uint32 NPC_DRAHGA_SHADOWBURNER = 40319;      // grim_batol.h:67
constexpr uint32 NPC_ERUDAX = 40484;                   // grim_batol.h:68
constexpr uint32 NPC_GROUND_SIEGE_STALKER = 40030;     // boss_general_umbriss.cpp:57, summoned on the target (6 s, :185)
constexpr uint32 NPC_BLITZ_STALKER = 40040;            // boss_general_umbriss.cpp:58, summoned on the target (6 s, :197)
constexpr uint32 NPC_INVOCATION_OF_FLAME_STALKER = 40355;  // boss_drahga_shadowburner.cpp:58
constexpr uint32 NPC_INVOKED_FLAMING_SPIRIT = 40357;       // boss_drahga_shadowburner.cpp:59
constexpr uint32 NPC_SEEPING_TWILIGHT = 40365;             // boss_drahga_shadowburner.cpp:61
constexpr uint32 NPC_SHADOW_GALE_STALKER = 40567;      // boss_erudax.cpp:58, the eye of the gale (summoned :194)
constexpr uint32 NPC_FACELESS_CORRUPTOR = 40600;       // boss_erudax.cpp:53
constexpr uint32 NPC_FACELESS_CORRUPTOR_H = 48844;     // boss_erudax.cpp:54
// #688 (2026-10-23): mechanics the core scripts gained (boss_forgemaster_throngus.cpp / boss_drahga_shadowburner.cpp)
constexpr uint32 NPC_CAVE_IN_STALKER = 40228;          // Throngus Mighty Stomp: summoned on a random player, Cave In 74990 for 240 s
constexpr uint32 NPC_FIRE_PATCH = 48711;               // Throngus heroic mace phase (Encumbered -> 90743 -> 90751), Lava Patch 90754
constexpr uint32 NPC_VALIONA = 40320;                  // boss_drahga_shadowburner.cpp NPC_VALIONA
constexpr uint32 NPC_DEVOURING_FLAMES = 48798;         // heroic: the stalker on the player Valiona channels Devouring Flames at
constexpr uint32 SPELL_FLAMING_FIXATE = 82850;         // aura on the player an Invoked Flaming Spirit chases
constexpr uint32 SPELL_VALIONAS_FLAME = 75321;         // 2 s cast at a random player, 40 yd 40 degree cone (Wowpedia)

// Spell ids as cast by the fork's scripts
constexpr uint32 SPELL_GROUND_SIEGE = 74634;           // boss_general_umbriss.cpp:36, cast :185-193 (DoCast on the stalker, 2 s cast)
constexpr uint32 SPELL_BLITZ = 74670;                  // boss_general_umbriss.cpp:37, cast :197-209 (DoCast on the stalker, 3 s cast)
constexpr uint32 SPELL_PERSONAL_PHALANX = 74908;       // boss_forgemaster_throngus.cpp:50, self aura in the shield phase (:215-221)
constexpr uint32 SPELL_SHADOW_GALE = 75664;            // boss_erudax.cpp:39 (:194-207)
constexpr uint32 SPELL_BINDING_SHADOWS = 79466;        // boss_erudax.cpp:33 (DoCast on a random non-tank player, :232-260)

// Umbriss. Ground Siege 74634: 2 s cast, no ConeAngle in SpellTargetRestrictions.dbc so the core's 90 degree default
// for creature casters applies (Spell.cpp:1142), 20 yd, 58.5-61.5k and a 4 s stun; Wowpedia: "face the location of a
// random player and unleash a tremor towards it". Blitz 74670: 3 s cast, charges the stalker's spot and hits 97.5-102.5k
// with a knock up within ~6 yd (Wowpedia Umbriss: "all players near the targeted player must also move").
constexpr float GROUND_SIEGE_HALF_ANGLE = 0.7854f + 0.10f;   // 45 degrees + margin
constexpr float GROUND_SIEGE_LENGTH = 22.0f;
constexpr float CONE_ANGLE_MARGIN = 0.10f;
constexpr float CONE_MAX_MOVE = 14.0f;
constexpr float BLITZ_RADIUS = 8.0f;
constexpr float BLITZ_MARGIN = 1.5f;

// Throngus, shield phase: Personal Phalanx 74908 pulses 74909 on enemies in a 120 degree cone in FRONT of him (-99%
// damage they deal, Spell effect 303; heroic Flaming Shield 90819 is a 14.6-15.4k/s fire cone there too). Stay out of it.
constexpr float PHALANX_HALF_ANGLE = 1.31f;   // 75 degrees: the 60 degree half cone plus margin
constexpr float PHALANX_AIM = 1.75f;          // walk round to ~100 degrees off his facing
constexpr float PHALANX_RANGE = 62.0f;        // heroic Flaming Shield 90830 reaches 60 yd
// #688 Throngus: Mighty Stomp -> Cave In (Wowpedia: "a random player in a 5 yard radius area", 3.8-4.2k every second; 74986
// radius 5 yd in SpellRadius.dbc); heroic Lava Patch 90754 radius 5 yd (8.3-8.7k every second).
constexpr float CAVE_IN_RADIUS = 5.0f;
constexpr float CAVE_IN_MARGIN = 1.5f;
// #688 w106 (14 hits from 2 cave-ins, bar 8): Throngus's combat reach is 7 yd (creature_model_info 33429), so he hits the tank
// from ~9.8 yd and never moved when the tank stepped 7 yd from the stalker: he kept standing on the circle and tank face / reach
// melee / charge / heroic leap / killing spree took the melee back in (tank 5 hits, rogue 5, mage 3). The tank now drags him:
// while his center is within radius + margin + CAVE_IN_BOSS_SLACK of a circle and the tank is closer than radius + margin +
// CAVE_IN_BOSS_SLACK + his melee gap (his reach + the tank's), it walks that far out, so he follows to outside it ("Tanks should
// move Throngus should these cave-ins appear on the melee", Wowpedia Forgemaster_Throngus). Nobody else closes in through or into
// a circle meanwhile (GbCaveInApproachMultiplier).
constexpr float CAVE_IN_BOSS_SLACK = 2.0f;
constexpr float CAVE_IN_DRAG_TRAVEL = 24.0f;
constexpr float FIRE_PATCH_RADIUS = 5.0f;
constexpr float FIRE_PATCH_MARGIN = 1.5f;
// #688 Valiona: Valiona's Flame "a 40 yard long and 40 degree wide cone ... in the direction of a random player" (2 s cast);
// heroic Devouring Flames "a massive 150 degree cone in front of Valiona" for 5 s, aimed at the 48798 stalker.
constexpr float VALIONA_FLAME_HALF_ANGLE = 0.3491f + 0.10f;
constexpr float VALIONA_FLAME_LENGTH = 42.0f;
constexpr float DEVOURING_HALF_ANGLE = 1.309f + 0.10f;
constexpr float DEVOURING_LENGTH = 60.0f;
constexpr float VALIONA_CONE_MAX_MOVE = 30.0f;

// Drahga: Invocation of Flame 75222 opens a portal for 6 s that hits 4.4-4.6k within 4 yd (Wowpedia) and spawns a
// Flaming Spirit that chases a random player and goes Supernova on arrival (75238: 151-159k in 10 yd, then a burning DoT).
// Valiona's Shredding Swipe leaves Seeping Twilight (npc 40365 aura 75274 -> 75317: 9.3-9.7k and -50% speed) 10 yd wide.
constexpr float FLAME_PORTAL_RADIUS = 4.0f;
constexpr float FLAME_PORTAL_MARGIN = 2.0f;
constexpr float SPIRIT_KITE_RADIUS = 14.0f;    // the spirit's supernova is 10 yd; run before it gets this close
constexpr float SEEPING_TWILIGHT_RADIUS = 10.0f;
constexpr float SEEPING_TWILIGHT_MARGIN = 1.5f;

// Erudax: the script zeroes Shadow Gale's damage for a player within 5 yd of the stalker (boss_erudax.cpp:483);
// Binding Shadows 79466 (1.5 s cast, 7.8-8.2k/s drain and a 6 s root) hits everyone within 8 yd of its target.
constexpr float SHADOW_GALE_ARRIVE = 3.0f;
constexpr float BINDING_SHADOWS_RADIUS = 8.0f;
constexpr float BINDING_SHADOWS_MARGIN = 1.5f;
constexpr float BINDING_SHADOWS_RUN = 11.0f;

// Each pair: Needed() is true exactly when Target() has somewhere to send the bot (triggers and the movement lock use
// it), Target() picks the spot. Both are cheap enough to run every tick; the lock caches them for 150 ms.
bool GroundSiegeNeeded(Player* bot);
bool GroundSiegeTarget(Player* bot, float& x, float& y);
bool BlitzNeeded(Player* bot);
bool BlitzTarget(Player* bot, float& x, float& y);
bool PhalanxNeeded(Player* bot);
bool PhalanxTarget(Player* bot, float& x, float& y);
bool CaveInNeeded(Player* bot);
bool CaveInTarget(Player* bot, float& x, float& y);
bool FirePatchNeeded(Player* bot);
bool FirePatchTarget(Player* bot, float& x, float& y);
bool ValionaBreathNeeded(Player* bot);
bool ValionaBreathTarget(Player* bot, float& x, float& y);
bool FlamePortalNeeded(Player* bot);
bool FlamePortalTarget(Player* bot, float& x, float& y);
bool SpiritKiteNeeded(Player* bot);
bool SpiritKiteTarget(Player* bot, float& x, float& y);
bool SeepingTwilightNeeded(Player* bot);
bool SeepingTwilightTarget(Player* bot, float& x, float& y);
bool ShadowGaleNeeded(Player* bot);
bool ShadowGaleTarget(Player* bot, float& x, float& y);
bool BindingShadowsNeeded(Player* bot);
bool BindingShadowsTarget(Player* bot, float& x, float& y);
bool AnyMovementLock(Player* bot);
// #688 w106: Throngus (engaged, not in the shield phase) stands within radius + margin + extra of a Cave In circle (not the
// heroic Fire Patches, which trail him); `boss` is set whenever he is engaged.
bool ThrongusAtCaveIn(Player* bot, Creature*& boss, float extra = CAVE_IN_BOSS_SLACK);
// #688 w106: (x, y) lies within radius + margin of a Cave In / Fire Patch circle.
bool InCaveInCircle(Player* bot, float x, float y);
// #688 w106: closing in on the boss to melee range (behind: to the spot behind his back) would end in a Cave In, or walk through
// one the bot is not standing in.
bool CaveInApproachBlocked(Player* bot, Unit* boss, bool behind = false);

// Add priorities: the lowest-guid unit of the kind, nullptr when the boss is not engaged or none is alive.
Creature* FlamingSpirit(Player* bot);
Creature* FacelessCorruptor(Player* bot);
}

#endif
