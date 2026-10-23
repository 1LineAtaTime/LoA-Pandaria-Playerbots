/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#ifndef PLAYERBOTS_CATAVPSHARED_H
#define PLAYERBOTS_CATAVPSHARED_H

#include "Define.h"
#include <vector>

class Creature;
class GameObject;
class Player;
class Unit;

namespace CataVp
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

// The Vortex Pinnacle (map 657, strategy "cata-vp") - issue #663.
// Intended mechanics: Wowpedia (fetched 2026-10-06) Grand_Vizier_Ertan, Altairus, Asaad, Vortex_Pinnacle; Wowhead
// MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>, 2026-10-06). Ids and behaviour are the ones of
// THIS core's scripts: src/server/scripts/Kalimdor/TheVortexPinnacle/boss_*.cpp and the_vortex_pinnacle.h.
namespace CataVp
{
// MoP test DB creature_template (world DB) entries
constexpr uint32 NPC_GRAND_VIZIER_ERTAN = 43878;   // boss_grand_vizier_ertan
constexpr uint32 NPC_ALTAIRUS = 43873;             // boss_altairus
constexpr uint32 NPC_ASAAD = 43875;                // boss_asaad
constexpr uint32 NPC_ERTAN_VORTEX = 46007;         // the 8 cyclones of Ertan's "Cyclone Shield" ring (the_vortex_pinnacle.h:66)
constexpr uint32 NPC_AIR_CURRENT = 47305;          // Altairus: carries the wind direction in its orientation
constexpr uint32 NPC_ALTAIRUS_TWISTER = 47342;     // Altairus heroic "Twisting Winds"
constexpr uint32 NPC_UNSTABLE_GROUNDING_FIELD = 46492;  // Asaad: spawned for the whole grounding-field phase

// Spell ids as cast by the fork's scripts
constexpr uint32 SPELL_DOWNWIND_OF_ALTAIRUS = 88286;    // boss_altairus.cpp:26, AddAura in EVENT_CHECK_FACING :202
constexpr uint32 SPELL_CHILLING_BREATH = 88308;         // boss_altairus.cpp:28 (DoCast on a random player)
constexpr uint32 SPELL_SUPREMACY_OF_THE_STORM = 86930;  // boss_asaad.cpp:35 (EVENT_FIELD_5, :225)
constexpr uint32 SPELL_UNSTABLE_GROUNDING_FIELD = 86911;  // boss_asaad.cpp:32 (EVENT_FIELD, 20 s channel, :185)

// Ertan: the ring of 8 cyclones circles her at ~35.8 yd (npc_ertan_vortex dist, boss_grand_vizier_ertan.cpp:233) and
// every 2 s Storm's Edge hits each player standing farther from her than a random cyclone (:178-202). Every 20-25 s the
// cyclones are called in for 14-17 s (:113, :170-176, :241-253); they walk in at 3 yd/s (npc 46007 speed_run 0.428571).
// w29 (#663): the whole group stacks under her for the WHOLE fight, not only once the ring is called in. Wowpedia
// Grand_Vizier_Ertan Strategy (MediaWiki API, read 2026-10-09): "It is possible to avoid the whirlwind damage and debuff
// if you stack directly underneath the Boss." Waiting at ring-9 yd until the nearest cyclone is within 16 yd left the bots
// in the path of the incoming cyclones (-85% speed) and outside the shrinking ring under Storm's Edge every 2 s
// (w27: 6 of 8 deaths and boss at 56%; w17: 560/832 dead ticks, boss at 7%).
// w64 (#663): the called-in ring stopped ~5 yd from her, inside the 5.5 yd reach of every cyclone (Cyclone Shield 86292 radius
// 4 yd, SpellMgr.cpp override, + 1.5 yd player combat reach), so all 8 cyclones hit each stacked bot every second (wipe at 27%).
// The script now stops the ring 9 yd from her (VORTEX_PULLED_IN_DIST): a 3.5 yd pocket under her. One threshold for both ring
// states (the 4 yd "ring out" hysteresis would sit 5 yd from a pulled-in cyclone, inside its reach), and every other movement
// (flee, blink, formation, avoid aoe) is vetoed while she is engaged (CataVpMultipliers.cpp) - w64: the frost mage fled and
// blinked 17-20 yd out of the stack 8 times and took Storm's Edge and the incoming cyclones there.
constexpr float ERTAN_STACK_ARRIVE = 2.5f;      // stay within this of her: 1 yd inside the 3.5 yd pocket of the pulled-in ring
constexpr float ERTAN_VORTEX_TOUCH = 5.5f;      // a cyclone this close to the bot hits it (86292: 4 yd + 1.5 yd combat reach)

// Altairus: Downwind of Altairus (-100% attack and cast speed) is given by the script (boss_altairus.cpp:129-205) to
// every player whose FACING is within 1 rad of the opposite of the air current's orientation; a bot that faces the boss
// is therefore upwind when it stands on the air current's side of the boss, within 1 rad of its heading.
constexpr float WIND_WEDGE_NEEDED = 0.55f;   // bearing (boss -> bot) farther than this from the heading = move
constexpr float WIND_WEDGE_AIM = 0.35f;      // ... and aim for this far from the heading, inside the 1.0 rad wedge
constexpr float ALTAIRUS_CENTER_X = -1183.97f;   // AlrairusCenter (the_vortex_pinnacle.h:47)
constexpr float ALTAIRUS_CENTER_Y = 51.14f;
constexpr float ALTAIRUS_PLATFORM_RADIUS = 40.0f;  // the script blasts a victim farther than 55 yd (boss_altairus.cpp:158)
// Chilling Breath: 2 s cast, 120 degree cone (SpellTargetRestrictions.dbc), 47.5-52.5k frost, boss turns to its target.
constexpr float BREATH_HALF_ANGLE = 1.0472f;
constexpr float BREATH_LENGTH = 85.0f;
constexpr float BREATH_ANGLE_MARGIN = 0.12f;
constexpr float BREATH_MAX_MOVE = 15.0f;
// Twisting Winds (heroic): Twister Aura 88313 pulses 88314 (11.7-12.3k + knock up) in 7 yd every 0.5 s.
constexpr float TWISTER_RADIUS = 7.0f;
constexpr float TWISTER_MARGIN = 1.5f;

// Asaad: during the grounding-field phase (Wowpedia Asaad Strategy) he teleports above the field and channels
// Supremacy of the Storm on everyone; the script zeroes its damage for players within 5 yd of him
// (boss_asaad.cpp:297). The field triangle is fieldPos[0..2] (boss_asaad.cpp:69), he hovers at fieldPos[3].
constexpr float ASAAD_STACK_X = -639.337f;   // centroid of fieldPos[0..2]
constexpr float ASAAD_STACK_Y = 487.263f;
constexpr float ASAAD_LIFTED_Z = 652.0f;     // he is at 646.6 on the ground, 656.63 above the field
constexpr float ASAAD_STACK_ARRIVE = 2.5f;

// Each pair: Needed() is true exactly when Target() has somewhere to send the bot (triggers and the movement lock use
// it), Target() picks the spot. Both are cheap enough to run every tick; the lock caches them for 150 ms.
bool ErtanRingNeeded(Player* bot);
bool ErtanRingTarget(Player* bot, float& x, float& y);
// Ertan is engaged and her cyclones are up: the whole group holds the stack under her.
bool ErtanStackActive(Player* bot);
bool WindNeeded(Player* bot);
bool WindTarget(Player* bot, float& x, float& y);
bool BreathNeeded(Player* bot);
bool BreathTarget(Player* bot, float& x, float& y);
bool TwisterNeeded(Player* bot);
bool TwisterTarget(Player* bot, float& x, float& y);
bool StackNeeded(Player* bot);
bool StackTarget(Player* bot, float& x, float& y);

// #692 (BEYOND AC - AC WotLK has no Cata dungeon; owner: take it on): the Slipstream rides between the three platforms.
// A grouped bot follows its leader onto the slipstream the way a player does - it walks to the platform's slipstream and
// clicks it (Unit::HandleSpellClick -> the_vortex_pinnacle.cpp npc_vortex_pinnacle_slipsteamAI::OnSpellClick summons one
// carrier per rider and flies it to the landing zone), the moving-platform follow rule (walk on, never teleport). It boards
// when the leader rides a carrier that lands farther on than the bot stands, or stands on a later platform; a leader bot
// (no other leader) boards on its own when it stands by a slipstream whose next boss is still alive.
constexpr uint32 NPC_SLIPSTREAM = 45455;                 // the_vortex_pinnacle.h NPC_SLIPSTREAM
constexpr uint32 SLIPSTREAM_DATA_CARRIER = 2;            // the_vortex_pinnacle.cpp TYPE_SLIPSTREAM_CARRIER: a carrier's GetData = route + 1
constexpr uint32 BOSS_DATA_ERTAN = 1;                    // the_vortex_pinnacle.h enum Data (InstanceScript::GetBossState ids)
constexpr uint32 BOSS_DATA_ALTAIRUS = 2;
constexpr uint32 BOSS_DATA_ASAAD = 3;
constexpr float SLIPSTREAM_CLICK_RANGE = 4.0f;           // IsWithinDistInMap (surface) range of the click; the script allows 10
constexpr float SLIPSTREAM_SEARCH_RANGE = 40.0f;         // the slipstream creature is looked up only this close
constexpr float SLIPSTREAM_LEADER_RANGE = 30.0f;         // a leader bot boards on its own only this close to the slipstream
// 0 = entrance + Ertan, 1 = Altairus, 2 = Asaad: the nearest of the anchors (creature spawn clusters of map 657).
int32 PlatformOf(float x, float y, float z);
// The slipstream of the bot's platform (its x, y, z) when the bot has to ride it, and the spawned slipstream itself once
// the bot is within SLIPSTREAM_SEARCH_RANGE of it (nullptr before).
bool SlipstreamTarget(Player* bot, float& x, float& y, float& z, Creature*& slipstream);
bool SlipstreamNeeded(Player* bot);

bool AnyMovementLock(Player* bot);
}

#endif
