/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// End Time (map 938, Cata 4.3 heroic, level 85) bot strategy "cata-et" - issue #671, epic #373 (MoP-only dungeon
// strategy lanes, lane L8). AC has no strategy for this dungeon; this follows AC's dungeon-strategy PATTERN
// (Ai/Dungeon/<Code>/<Code>{Strategy,Triggers,Actions,Multipliers}, e.g. HoS/PoS) with the fork's own ids.
//
// Mechanics research (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>&prop=wikitext):
//     "Echo of Baine", "Echo of Jaina", "Echo of Sylvanas", "Echo of Tyrande", "Murozond (tactics)", "End Time".
//   Wowhead MoP-classic tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>): 101602, 101627, 103241 style
//   cast-time / radius checks for the ids below.
// Real ids come from THIS core: boss scripts src/server/scripts/Kalimdor/CavernsOfTime/EndTime/*.cpp (the file:line of
// every id is next to it) and the MoP test world DB (creature_template / gameobject / npc_spellclick_spells).
//
// Script gaps (mechanics a retail guide lists but the fork's script never casts - NO edge written for them):
//   Baine   : Pulverize/Throw Totem are scripted, but Molten Axe/Fist only react to lava; no platform-sink logic beyond the
//             spell effect 101815 (boss_echo_of_baine.cpp).
//   Tyrande : Tears of Elune (30%) has no marker unit/dynobj to dodge: 102241 ticks 102242 every 1 s, each player rolls 50%
//             (spell_echo_of_tyrande_tears_of_elune_script) for missile 102243, which lands at a RANDOM point within 10 yd of
//             him (SpellEffect.dbc: TARGET_DEST_DEST_RANDOM, radius 10) at 5 yd/s from Tyrande, 4 yd blast 102244. Wowpedia
//             "Echo of Tyrande" (2026-10-15) plays it as unavoidable group damage ("healing will become more difficult").
//             Dark Moonlight (102414) IS a persistent dynobj (effect 27, 15 yd, 7-day duration, -50% cast speed); the ranged
//             band (ET_TYRANDE_RANGED_*, 39+ yd) already keeps every caster out of it.
//   Murozond: "Temporal Snapshot" is a stub (boss_murozond.cpp:96 "until Temporal Snapshot is properly implemented").

#ifndef PLAYERBOTS_CATAETSHARED_H
#define PLAYERBOTS_CATAETSHARED_H

#include "Define.h"
#include <functional>
#include <initializer_list>
#include <string>
#include <vector>

class Creature;
class GameObject;
class Player;
class PlayerbotAI;
class Unit;

namespace EndTime
{
constexpr uint32 ET_MAP_END_TIME = 938;

// ---- creatures (fork end_time.h:62-66 and the boss scripts) ------------------------------------------------------
constexpr uint32 ET_NPC_ECHO_OF_JAINA    = 54445;
constexpr uint32 ET_NPC_ECHO_OF_BAINE    = 54431;
constexpr uint32 ET_NPC_ECHO_OF_SYLVANAS = 54123;
constexpr uint32 ET_NPC_ECHO_OF_TYRANDE  = 54544;
constexpr uint32 ET_NPC_MUROZOND         = 54432;

constexpr uint32 ET_NPC_RISEN_GHOUL      = 54191;   // boss_echo_of_sylvanas.cpp:87
constexpr uint32 ET_NPC_BLIGHTED_ARROW   = 54403;   // boss_echo_of_sylvanas.cpp:89

constexpr uint32 ET_NPC_MOONLANCE_1      = 54574;   // boss_echo_of_tyrande.cpp:97-100
constexpr uint32 ET_NPC_MOONLANCE_2_1    = 54580;
constexpr uint32 ET_NPC_MOONLANCE_2_2    = 54581;
constexpr uint32 ET_NPC_MOONLANCE_2_3    = 54582;
constexpr uint32 ET_NPC_EYE_OF_ELUNE_1_E = 54594;   // boss_echo_of_tyrande.cpp:101-106
constexpr uint32 ET_NPC_EYE_OF_ELUNE_1_W = 54597;
constexpr uint32 ET_NPC_EYE_OF_ELUNE_2_E = 54939;
constexpr uint32 ET_NPC_EYE_OF_ELUNE_2_W = 54940;
constexpr uint32 ET_NPC_EYE_OF_ELUNE_3_E = 54941;
constexpr uint32 ET_NPC_EYE_OF_ELUNE_3_W = 54942;
constexpr uint32 ET_NPC_POOL_OF_MOONLIGHT = 54508;  // boss_echo_of_tyrande.cpp:112
// The five-pool event's adds (boss_echo_of_tyrande.cpp:113-117, summoned by EVENT_SUMMON_ADDS :363-383); every one spawns with
// "In Shadow" 101841 (creature_template_addon.auras of the MoP test world DB).
constexpr uint32 ET_NPC_TIME_TWISTED_SENTINEL     = 54512;
constexpr uint32 ET_NPC_TIME_TWISTED_HUNTRESS     = 54701;
constexpr uint32 ET_NPC_TIME_TWISTED_NIGHTSABER_1 = 54688;
constexpr uint32 ET_NPC_TIME_TWISTED_NIGHTSABER_2 = 54699;
constexpr uint32 ET_NPC_TIME_TWISTED_NIGHTSABER_3 = 54700;

constexpr uint32 ET_NPC_FLARECORE        = 54446;   // boss_echo_of_jaina.cpp:59
constexpr uint32 ET_NPC_FROST_BLADE      = 54494;   // boss_echo_of_jaina.cpp:60

constexpr uint32 ET_NPC_BAINES_TOTEM     = 54434;   // boss_echo_of_baine.cpp:54

// ---- game objects ------------------------------------------------------------------------------------------------
constexpr uint32 ET_GO_JAINA_FRAGMENT    = 209318;  // MoP world DB gameobject_template, ScriptName go_echo_of_jaina_jaina_staff_fragment
constexpr uint32 ET_GO_HOURGLASS         = 209249;  // end_time.h:50
constexpr uint32 ET_GO_PLATFORM_1        = 209670;  // end_time.h:53-56 (Baine's four platforms, type 33 destructible)
constexpr uint32 ET_GO_PLATFORM_2        = 209693;
constexpr uint32 ET_GO_PLATFORM_3        = 209694;
constexpr uint32 ET_GO_PLATFORM_4        = 209695;

// ---- spells ------------------------------------------------------------------------------------------------------
constexpr uint32 ET_SPELL_CALL_OF_THE_HIGHBORNE = 100686;  // boss_echo_of_sylvanas.cpp:43 (immune aura while the Call runs)
constexpr uint32 ET_SPELL_WRACKING_PAIN_AURA    = 101258;  // :36 (applied when a Risen Ghoul starts walking)
constexpr uint32 ET_SPELL_SEEPING_SHADOWS_DUMMY = 103175;  // :49 (applied when a Risen Ghoul is engaged)

constexpr uint32 ET_SPELL_MOONBOLT      = 102193;          // boss_echo_of_tyrande.cpp:51
constexpr uint32 ET_SPELL_STARDUST      = 102173;          // :54
constexpr uint32 ET_SPELL_IN_SHADOW     = 101841;          // :71 (Tyrande's aura until the moonlight-pool event is done)

constexpr uint32 ET_SPELL_FROSTBOLT_VOLLEY = 101810;       // boss_echo_of_jaina.cpp:39

// #842: the PLAYER's throw is 101603 (SpellHandler.cpp:1071 lets players cast it unlearned; 107837 ticks 101601 every 1 s,
// whose SPELL_AURA_MOD_NEXT_SPELL 363 points the client at 101603). 101603 = TRIGGER_MISSILE -> 101602 at the dest, and
// spell_linked_spell 101603 -> -107837 drops the held totem. 101602 (5% max-health damage + 6 s stun + 50% damage taken) cast
// directly never removed 107837, so the holder re-threw every GCD: w106 Baine x6 (42.8M) 99% -> 0% in 6 s.
constexpr uint32 ET_SPELL_THROW_TOTEM_BACK = 101603;       // SpellEffect.dbc: effect 32 TRIGGER_MISSILE -> 101602 (boss_echo_of_baine.cpp:40)
constexpr uint32 ET_SPELL_THROW_TOTEM_AURA = 107837;       // :41 (npc_spellclick_spells 54434 -> 107837)

constexpr uint32 ET_SPELL_INFINITE_BREATH      = 102569;   // boss_murozond.cpp:48
constexpr uint32 ET_SPELL_DISTORTION_BOMB_DMG  = 101984;   // :53 (SpellEffect.dbc: effect 27 PERSISTENT_AREA_AURA => dynobj)

// ---- geometry taken from the scripts / DB -------------------------------------------------------------------------
constexpr float ET_SYLVANAS_RING_DIST = 32.8f;             // boss_echo_of_sylvanas.cpp:103 (Risen Ghoul ring radius)
constexpr float ET_SYLVANAS_SACRIFICE_RADIUS = 11.0f;      // Wowpedia "Echo of Sylvanas": Sacrifice hits within 11 yd
// w62 (#671): the safe spot is IN the sector of a killed ghoul (no link, no out-of-bounds Wracking Pain there,
// IsPlayerOutOfBounds boss_echo_of_sylvanas.cpp:252-280) and outside Sacrifice. w61 stood 18 yd out with all eight ghouls alive:
// the tank died at (3826.7,898.5) 17 s into the Call, when the walking ghouls passed inside 18 yd.
constexpr float ET_SYLVANAS_SAFE_DIST = 24.0f;
constexpr float ET_SYLVANAS_BREACH_MIN_GAP = 1.05f;        // 60 degrees: a full ring has 45-degree gaps (spawnAngle, :106)
// w65 (#671): IsPlayerOutOfBounds (boss_echo_of_sylvanas.cpp:259-289) casts 101221 (50k, every 1 s) on a player farther from
// Sylvanas than the LIVE ghoul of his sector (spawnAngle/2 = 22.5 degrees either side, :271). The bots stay this much inside it.
constexpr float ET_SYLVANAS_SECTOR_HALF  = 0.49f;          // 28 degrees: the script's 22.5 plus a margin
constexpr float ET_SYLVANAS_INSIDE_LIMIT = 1.5f;           // re-step once the walking ghoul is this close to the bot's radius
constexpr float ET_SYLVANAS_INSIDE_STAND = 4.0f;           // stand this far inside the ghoul (combat reach 5, :514-515: still melee)
// w67 (#671): the Eyes of Elune orbit Tyrande for the rest of the fight at 16, 23 and 30 yd (one pair per cast, 20 s apart,
// boss_echo_of_tyrande.cpp:452-458 + eye Reset/MovementInform :600-680, cyclic spline 7.7 yd/s) and Piercing Gaze hits 6 yd
// round each eye (102183 radius 6). From the third cast on, 10-36 yd is covered; ranged bots hold 37.5-40 yd (heals: 40 yd +
// both combat reaches, IsWithinCombatRange), the healer on the tank's side.
// w68 (#671): the gaze area check adds the player's own size (IsWithinDist3d + combat reach 1.5), so the outer orbit reaches
// 37.5 yd: the mage took 52k at 35.8 yd (reach spell) and 50k at 38.3 yd. Band 39-42 yd: spells reach 40 + 1.5 + Tyrande's
// combat reach 1.8 (creature_model_info 39617), heals 40 + 1.5 + 1.5, so ET_TYRANDE_HEAL_REACH (2D) stays below the class
// "party member to heal out of spell range" trigger (2D minus both sizes > 38.5 + 0.5 = 42 yd).
// The healer's anchor is Tyrande's victim, every other ranged bot's anchor the group healer; a bot out of its anchor's reach
// walks ALONG the band ET_TYRANDE_ARC_STEP at a time (a 0.35 rad chord at 40 yd stays outside 39 yd): w68 the healer walked
// the chord to the tank's bearing, which swings 100+ degrees while the tank stands 1-3 yd from her, and crossed 22 yd.
constexpr float ET_TYRANDE_RANGED_MIN   = 39.0f;
constexpr float ET_TYRANDE_RANGED_STAND = 40.0f;
constexpr float ET_TYRANDE_RANGED_MAX   = 42.0f;
constexpr float ET_TYRANDE_HEAL_REACH   = 41.0f;
constexpr float ET_TYRANDE_ARC_STEP     = 0.35f;
// Inside the inner orbit (16 yd - 7.5 yd gaze reach): melee may stand here; a hazard escape near Tyrande stays here or in the band.
constexpr float ET_TYRANDE_INNER_SAFE   = 8.0f;
constexpr float ET_PLATFORM_TOP_OFFSET = 4.0f;             // Baine spawn z 131.3 vs platform GO z 127.3 (MoP world DB)

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
    BlightedArrows,   // Sylvanas ground blight (54403)
    Moonlance,        // Tyrande lances (54574, 54580-82)
    EyeOfElune,       // Tyrande orbiting eyes (54594, 54597, 54939-42)
    DistortionBomb,   // Murozond 8 yd dynobj (101984)
};

// ---- generic helpers (same code in every dungeon of this lane) -----------------------------------------------------
void CollectCreatures(Player* bot, std::initializer_list<uint32> entries, float radius, std::vector<Creature*>& out);
bool InsideAnyHazard(std::vector<Hazard> const& hazards, float x, float y);
bool FindClearSpot(Player* bot, std::function<float(float, float)> const& margin, float& outX, float& outY, float& outZ);
bool FindHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ);
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);
bool IsInterruptActionName(std::string const& name);              // a class interrupt action ("kick", "kick on enemy healer", ...)
void CollectBotMembers(Player* bot, std::vector<Player*>& out);   // alive playerbot members on the bot's map, bot included
Player* GetDesignatedBot(Player* bot);                            // tank first, then lowest guid

// ---- hazards -------------------------------------------------------------------------------------------------------
void CollectHazards(Player* bot, HazardKind kind, std::vector<Hazard>& out);
bool InHazard(Player* bot, HazardKind kind);

// ---- Echo of Sylvanas ----------------------------------------------------------------------------------------------
Creature* GetSylvanas(Player* bot);
bool SylvanasCallActive(Player* bot);
void CollectRisenGhouls(Player* bot, std::vector<Creature*>& out);
Creature* GetDesignatedGhoul(std::vector<Creature*> const& ghouls);   // lowest guid: every bot agrees, no AoE flapping
bool GetSylvanasBreachAngle(Creature* sylvanas, std::vector<Creature*> const& ghouls, float& outAngle);
bool GetSylvanasSafeSpot(Player* bot, float& outX, float& outY, float& outZ);
bool NeedsSylvanasSafeSpot(Player* bot);
bool GetSylvanasInsideSpot(Player* bot, float& outX, float& outY, float& outZ);   // false: the bot is inside its sector's ghoul
bool NeedsSylvanasInside(Player* bot);
bool NeedsGhoulKill(Player* bot);
bool NeedsRangedSpread(Player* bot);

// ---- Echo of Tyrande -----------------------------------------------------------------------------------------------
Creature* GetTyrande(Player* bot);
Unit* GetTyrandeInterruptTarget(Player* bot);
bool TyrandeNeedsInterrupt(Player* bot);
Creature* GetMoonlightPool(Player* bot);
bool NeedsMoonlightPool(Player* bot);
bool TyrandeEventActive(Player* bot);   // In Shadow on Tyrande and a pool or an event add is up
bool TyrandeFightActive(Player* bot);   // Tyrande in combat and attackable (In Shadow gone)
bool GetTyrandeRangedSpot(Player* bot, float& outX, float& outY, float& outZ);   // false: a ranged bot already outside the Eyes
bool NeedsTyrandeRangedBand(Player* bot);
// Hazard escape that, during the Tyrande fight, lands inside ET_TYRANDE_INNER_SAFE or in the band, never in the Eye orbits;
// falls back to the plain FindHazardEscape when no such spot is in reach.
bool FindTyrandeHazardEscape(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ);

// ---- Echo of Jaina -------------------------------------------------------------------------------------------------
Creature* GetJaina(Player* bot);
Unit* GetJainaInterruptTarget(Player* bot);
bool JainaNeedsInterrupt(Player* bot);
GameObject* FindJainaFragment(Player* bot);   // the unused fragment nearest to the designated bot: the group walks as one
bool NeedsFragments(Player* bot);
bool InBladePath(Player* bot);
bool GetBladeEscape(Player* bot, float& outX, float& outY, float& outZ);
Creature* GetFlarecore(Player* bot);
Player* GetFlarecoreStomper(Player* bot, Creature* ember);
bool FlarecoreActive(Player* bot);
bool NeedsFlarecoreAction(Player* bot);   // the stomper, or a bot standing within 8.5 yd of the ember

// ---- Echo of Baine -------------------------------------------------------------------------------------------------
Creature* GetBaine(Player* bot);
bool BaineFightActive(Player* bot);
GameObject* FindIntactPlatform(Player* bot);
bool NeedsLavaEscape(Player* bot);
Creature* GetBaineTotem(Player* bot);
Player* GetTotemClicker(Player* bot, Creature* totem);
bool NeedsTotemThrow(Player* bot);

// ---- Murozond ------------------------------------------------------------------------------------------------------
Creature* GetMurozond(Player* bot);
bool InInfiniteBreath(Player* bot);
bool GetBreathEscape(Player* bot, float& outX, float& outY, float& outZ);
bool NeedsHourglass(Player* bot);
}

#endif
