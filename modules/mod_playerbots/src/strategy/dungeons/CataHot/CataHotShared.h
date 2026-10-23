/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight (map 940, Cata 4.3 heroic, level 85) bot strategy "cata-hot" - issue #673, epic #373 (MoP-only dungeon
// strategy lanes, lane L8). AC has no strategy for this dungeon; this follows AC's dungeon-strategy PATTERN
// (Ai/Dungeon/<Code>/<Code>{Strategy,Triggers,Actions,Multipliers}, e.g. HoS/PoS) with the fork's own ids.
//
// Mechanics research (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>&prop=wikitext):
//     "Arcurion", "Asira Dawnslayer", "Archbishop Benedictus (tactics)", "Hour of Twilight".
//   Wowhead MoP-classic tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>): 103558 (8 yd radius smoke), 103684.
// Real ids come from THIS core: boss scripts src/server/scripts/Kalimdor/CavernsOfTime/HourofTwilight/*.cpp (file:line next
// to every id) and the MoP test world DB (creature_template / creature spawns).
//
// #696 (2026-10-23, the real encounters; Wowpedia MediaWiki API + Wowhead tooltip API fetched 2026-10-22):
//   Arcurion  : the glyphs are the Frozen Servitors' Icy Boulder (Wowhead 102198 "3 sec cast", 102199 2.5 yd). The servitor now
//               marks the spot with a World Trigger (22515) and lands the boulder on it after the cast (boss_arcurion.cpp
//               npc_arcurion_frozen_servitor) -> "hot icy boulder" leaves the marker.
//   Asira     : Mark of Silence -> Throw Knife hits the FIRST player between Asira and the marked caster (spell script
//               FilterTargets, IsInBetween 1 yd) -> "hot mark of silence": a marked caster stands behind the tank on Asira's line
//               (Wowpedia: "position yourself so that a non-marked player (preferably the tank) is between you and Asira").
//   Benedictus: Thrall's Water Shell (self channel 103688 -> 103744 pulse, 4 yd) exempts from the Wave of Virtue (phase 1 only) ->
//               "hot water shell" stacks on Thrall 54971 while a Wave of Virtue walks; Wave of Twilight (phase 2) is dodged.
//               Benedictus is script-spawned (no creature row in the MoP world DB): the driver runs the
//               real chain (Asira dead -> Life Warden -> Thrall 54634 escort through the faceless -> Benedictus walks in).

#ifndef PLAYERBOTS_CATAHOTSHARED_H
#define PLAYERBOTS_CATAHOTSHARED_H

#include "Define.h"
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace HourOfTwilight
{
constexpr uint32 HOT_MAP_HOUR_OF_TWILIGHT = 940;

// ---- creatures (fork hour_of_twilight.h and the boss scripts) -------------------------------------------------------
constexpr uint32 HOT_NPC_THRALL_1            = 54548;   // hour_of_twilight.h:174 (canyon escort to Arcurion)
constexpr uint32 HOT_NPC_THRALL_2            = 55779;   // :175 (Thrall at Arcurion)
constexpr uint32 HOT_NPC_THRALL_3            = 54972;   // :176 (escort through Asira's trash waves)
constexpr uint32 HOT_NPC_THRALL_4            = 54634;   // :177 (escort through the Benedictus faceless)
constexpr uint32 HOT_NPC_THRALL_5           = 54971;   // hour_of_twilight.h:178 (Thrall during the Benedictus fight)
constexpr uint32 HOT_NPC_ARCURION            = 54590;   // :183
constexpr uint32 HOT_NPC_ASIRA               = 54968;   // :184
constexpr uint32 HOT_NPC_BENEDICTUS          = 54938;   // :185
constexpr uint32 HOT_NPC_FROZEN_SERVITOR     = 54600;   // :199
constexpr uint32 HOT_NPC_ICY_TOMB            = 54995;   // :201

constexpr uint32 HOT_NPC_PURIFYING_LIGHT     = 55377;   // boss_archbishop_benedictus.cpp:110 (phase 1 orb)
constexpr uint32 HOT_NPC_WAVE_OF_VIRTUE      = 55441;   // :112
constexpr uint32 HOT_NPC_TWILIGHT_BLAST      = 55468;   // :114 (phase 2 orb; the script reuses this entry for the Corrupting Twilight orbs)
constexpr uint32 HOT_NPC_WAVE_OF_TWILIGHT    = 55469;   // :115
constexpr uint32 HOT_NPC_GLYPH_MARKER        = 22515;   // hour_of_twilight.h NPC_WORLD_TRIGGER_HOT (Icy Boulder glyph, summoned by 54600)

// ---- spells ------------------------------------------------------------------------------------------------------
constexpr uint32 HOT_SPELL_CHOKING_SMOKE_BOMB = 103558;  // boss_asira_dawnslayer.cpp:37 (SpellEffect.dbc: effect 27 PERSISTENT_AREA_AURA => dynobj)
constexpr uint32 HOT_SPELL_RIGHTEOUS_SHEAR    = 103151;  // boss_archbishop_benedictus.cpp:39 (SPELL_RIGHTEOUS_SNEAR_AURA)
constexpr uint32 HOT_SPELL_TWILIGHT_SHEAR     = 103363;  // :64 (SPELL_TWILIGHT_SNEAR_AURA)
constexpr uint32 HOT_SPELL_PURIFIED_LIGHT     = 103654;  // :51 (aura of a landed Purifying Light orb)
constexpr uint32 HOT_SPELL_TWILIGHT_ORB       = 103774;  // :76 (aura of a landed Corrupting Twilight orb)
constexpr uint32 HOT_SPELL_WATER_SHIELD       = 103688;  // hour_of_twilight.cpp:90 (Thrall's own channel)
constexpr uint32 HOT_SPELL_WATER_SHELL_BUFF   = 103744;  // boss_archbishop_benedictus.cpp SPELL_WATER_SHELL_BUFF (on players in the shell)
constexpr uint32 HOT_SPELL_MARK_OF_SILENCE    = 102726;  // boss_asira_dawnslayer.cpp:34

// ---- geometry taken from the scripts -----------------------------------------------------------------------------
constexpr float HOT_WAVE_DESPAWN_X = 3508.25f;           // hour_of_twilight.h:34 WaveDespawnPoint (the wave walks here)
constexpr float HOT_WAVE_DESPAWN_Y = 268.58f;

// A circular ground hazard: a bot inside 'stand' is hurt, a bot that reaches 'clear' is safe.
struct Hazard
{
    float x;
    float y;
    float stand;
    float clear;
};

enum class HazardKind : uint8
{
    SmokeBomb,   // Asira's Choking Smoke Bomb, 8 yd dynobj (103558)
    LightOrb,    // Benedictus Purifying Light / Corrupting Twilight orbs in flight or landed (55377, 55468)
    IcyBoulder,  // Arcurion: a Frozen Servitor's Icy Boulder glyph (22515 marker, #696)
};

// ---- generic helpers (same code in every dungeon of this lane) -----------------------------------------------------
void CollectCreatures(Player* bot, std::initializer_list<uint32> entries, float radius, std::vector<Creature*>& out);
bool InsideAnyHazard(std::vector<Hazard> const& hazards, float x, float y);
bool FindClearSpot(Player* bot, std::function<float(float, float)> const& margin, float& outX, float& outY, float& outZ);
bool FindHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ);
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);
void CollectBotMembers(Player* bot, std::vector<Player*>& out);   // alive playerbot members on the bot's map, bot included
Player* GetDesignatedBot(Player* bot);                            // tank first, then lowest guid

// ---- hazards -------------------------------------------------------------------------------------------------------
void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out);
bool InHazard(Player* bot, HazardKind kind);

// ---- Arcurion ------------------------------------------------------------------------------------------------------
Creature* GetArcurion(Player* bot);
Creature* GetIcyTomb(Player* bot);
bool NeedsIcyTombKill(Player* bot);
void CollectServitors(Player* bot, std::vector<Creature*>& out);

// ---- Thrall's escorts ------------------------------------------------------------------------------------------------
Unit* GetEscortAmbusher(Player* bot);
bool NeedsEscortAssist(Player* bot);

// ---- Asira Dawnslayer ----------------------------------------------------------------------------------------------
bool NeedsKnifeCover(Player* bot);
bool GetKnifeCoverSpot(Player* bot, float& outX, float& outY, float& outZ);

// ---- Archbishop Benedictus -----------------------------------------------------------------------------------------
Creature* GetShellThrall(Player* bot);   // Thrall 54971 able to shell while a Wave of Virtue walks (phase 1), else nullptr
bool NeedsWaterShell(Player* bot);
bool InWavePath(Player* bot);
bool GetWaveEscape(Player* bot, float& outX, float& outY, float& outZ);
bool NeedsShearSpread(Player* bot);
Player* GetNearestShearVictimNeighbour(Player* bot);
}

#endif
