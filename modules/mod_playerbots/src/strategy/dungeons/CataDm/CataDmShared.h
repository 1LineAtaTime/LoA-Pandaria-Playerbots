/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines, revamped (map 36, "cata-dm"). Epic #373, issue #667.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids: the fork's boss scripts (src/server/scripts/EasternKingdoms/Deadmines/boss_*.cpp,
// deadmines.h), the MoP test world DB (creature_template ScriptName / entry) and 5.4.8 SpellEffect.dbc (summon
// entries, radii). Intended mechanics (read 2026-10-06): Wowpedia MediaWiki API, pages Glubtok, Helix Gearbreaker,
// Foe Reaper 5000, Admiral Ripsnarl, "Captain" Cookie, Vanessa VanCleef (tactics).
// Only mechanics the fork's scripts actually drive have an edge here. #681 / #826 (2026-10-23) ported Glubtok's heroic Fire Wall,
// Foe Reaper's heroic Molten Slag, Helix's Throw Helix / stage-two Leap, Vanessa's detonations (Fiery Blaze, ropes), Powder Explosion
// and the Nightmare Gauntlet into the fork's scripts (port of The-Cataclysm-Preservation-Project TrinityCore 4.3.4 e503f96). Still no
// script (no edge): the Prototype Reaper vehicle (boss_foereaper5000.cpp; a player-driven vehicle the bots do not take). "Captain"
// Cookie has no edge: the fork's Cookie never attacks, and his Rotten Aura / cauldron fire tick for 8 (SpellEffect.dbc 89735 / 89262).
//   Glubtok  : 3 edges - leave a Fire / Frost Blossom landing (5 yd), kill the heroic blossom adds, dodge the heroic Fire Wall
//   Helix    : 3 edges - leave a Sticky Bomb (3 yd), spread a Chest Bomb (6 yd, heroic), focus the Oaf then Helix (#667)
//   Foe Reaper: 4 edges - leave the Harvest target (10 yd sweep), leave the boss during Overdrive (10 yd), keep out of a heroic
//              Molten Slag's 10 yd shield, ranged kill the slags
//   Ripsnarl : 1 edge  - kill the Vapors at once (heroic: they grow every 8 s)
//   Vanessa  : 3 edges - kill her adds (heroic only boss), Blood Wizard first; take a rope off the burning deck; leave her at 1%
//   Nightmare: 3 edges - click the Steam Valves from the trap, escape flames / icicles / spark chains, save the Harringtons

#ifndef PLAYERBOTS_CATADMSHARED_H
#define PLAYERBOTS_CATADMSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include "Common.h"
#include "Position.h"
#include <initializer_list>
#include <string>
#include <vector>

class Creature;
class Player;
class PlayerbotAI;
class Spell;
class Unit;

namespace CataDm
{
constexpr uint32 MAP_ID = 36;

// Bosses (fork deadmines.h:31-37) and the creatures the mechanics spawn (MoP world DB creature_template)
constexpr uint32 NPC_GLUBTOK              = 47162;
constexpr uint32 NPC_HELIX_GEARBREAKER    = 47296;  // deadmines.h:32, rides the Oaf (vehicle_template_accessory 47297 -> 47296)
constexpr uint32 NPC_LUMBERING_OAF        = 47297;  // deadmines.h:33
constexpr uint32 NPC_FOE_REAPER_5000      = 43778;
constexpr uint32 NPC_FIRE_BLOSSOM         = 48957;  // boss_glubtok.cpp:66 (heroic: spawned by 91301)
constexpr uint32 NPC_FROST_BLOSSOM        = 48958;  // boss_glubtok.cpp:67 (heroic: spawned by 91302)
constexpr uint32 NPC_STICKY_BOMB          = 47314;  // boss_helix_gearbreaker.cpp:50
constexpr uint32 NPC_TARGETING_BUNNY      = 47468;  // boss_foereaper5000.cpp:49, the Harvest destination
constexpr uint32 NPC_VAPOR                = 47714;  // boss_admiral_ripsnarl.cpp:67
constexpr uint32 NPC_DEFIAS_ENFORCER      = 49850;  // boss_vanessa_vancleef.cpp:66
constexpr uint32 NPC_DEFIAS_SHADOWGUARD   = 49852;
constexpr uint32 NPC_DEFIAS_BLOOD_WIZARD  = 49854;

// Spells (every id is cast / applied by a fork script)
constexpr uint32 SPELL_FIRE_BLOSSOM       = 88129;  // boss_glubtok.cpp:41, cast on a random player; lands (88173, 5 yd) 2 s later
constexpr uint32 SPELL_FROST_BLOSSOM      = 88169;  // :42, lands as 88177 (5 yd)
constexpr uint32 SPELL_CHEST_BOMB         = 88352;  // boss_helix_gearbreaker.cpp:38, 10 s aura on the target, explodes (88250, 6 yd)
constexpr uint32 SPELL_OVERDRIVE          = 88481;  // boss_foereaper5000.cpp:42, 10 s self aura (88484: 10 yd every second)

// Geometry (SpellEffect.dbc radii cross-checked with Wowpedia)
constexpr float BLOSSOM_RADIUS            = 6.0f;   // 88173 / 88177: 5 yd + margin
constexpr float STICKY_BOMB_RADIUS        = 4.0f;   // 88974: 3 yd + margin
constexpr float CHEST_BOMB_RADIUS         = 7.0f;   // 88250: 6 yd + margin
constexpr float HARVEST_RADIUS            = 11.0f;  // 88521 Harvest Sweep: 10 yd + margin
constexpr float OVERDRIVE_RADIUS          = 11.0f;  // 88484: 10 yd + margin
constexpr float FLEE_MARGIN               = 1.5f;
constexpr float SCAN_RANGE                = 40.0f;
constexpr float FOCUS_RANGE               = 40.0f;
constexpr float ESCAPE_SCAN               = 30.0f;  // hazards considered when picking an escape spot (FindBestSpot rings reach 19 yd)
constexpr float ESCAPE_MARGIN_CAP         = 1.5f;   // a spot this far beyond every safe distance counts as fully safe

// Deflection of a lateral escape point from the source's facing (see LateralPoint)
constexpr float LATERAL_ANGLE = 1.396f;  // 80 deg

// ---- #681 / #826 heroic mechanics ported into the fork's scripts (2026-10-23; port source: The-Cataclysm-Preservation-Project
// TrinityCore 4.3.4 e503f96 Deadmines scripts; intended design: Wowpedia MediaWiki API "Glubtok", "Foe Reaper 5000", "Vanessa
// VanCleef (tactics)", read 2026-10-23). Ids: the fork's scripts (deadmines.h, boss_*.cpp) and 5.4.8 SpellEffect.dbc radii.
// Glubtok heroic Fire Wall (boss_glubtok.cpp SummonFireWall): 28 dummies on one line through the pivot, turning clockwise.
constexpr uint32 NPC_FIRE_WALL_DUMMY      = 48976;
constexpr uint32 SPELL_FIRE_WALL          = 91398;  // periodic 1 s -> 91397, 46250 Fire within 3 yd of each dummy
constexpr float FIRE_WALL_PIVOT_X         = -193.287f;  // deadmines.h fireWallPivotPos (the platter spawn, creature 326770)
constexpr float FIRE_WALL_PIVOT_Y         = -441.953f;
constexpr float FIRE_WALL_PERIOD          = 20.0f;      // s per turn, deadmines.h FIRE_WALL_PERIOD (CPP 0.8 yd cyclic path at 0.25 yd/s)
constexpr float FIRE_WALL_HALF_WIDTH      = 3.0f;       // 91397 radius index 15
constexpr float FIRE_WALL_WARN            = 3.0f;       // s: a wall this close in time sends the bot ahead
constexpr float FIRE_WALL_LEAD            = 1.5f;       // s the wall turns while the bot walks
constexpr float FIRE_WALL_MIN_RADIUS      = 6.0f;       // nearer the pivot the 1.5 / 4.5 yd dummies leave no gap
constexpr float FIRE_WALL_MAX_RADIUS      = 15.0f;      // arc speed 4.7 yd/s at 15 yd (a bot runs 7)
// Foe Reaper heroic Molten Slag (boss_foereaper5000.cpp): Molten Shield 91815 -> 91819, 18500 Fire within 10 yd every 2 s
constexpr uint32 NPC_MOLTEN_SLAG          = 49229;
constexpr float MOLTEN_SHIELD_RADIUS      = 10.0f;
// Vanessa VanCleef (boss_vanessa_vancleef.cpp)
constexpr uint32 NPC_VANESSA_VANCLEEF     = 49541;
constexpr uint32 NPC_GPB_JMF              = 45979;  // the entrance fires
constexpr uint32 NPC_NIGHTMARE_FIRE_BUNNY = 51594;  // the detonation fires (and Glubtok's nightmare flames)
constexpr uint32 NPC_VANESSA_ROPE         = 49550;  // vehicle 1399, spellclick 46598
constexpr uint32 SPELL_FIERY_BLAZE        = 93484;  // periodic 1 s -> 93485, 46250 Fire within 5 yd
constexpr uint32 SPELL_ROPE_CLICK_ME      = 95527;
constexpr uint32 SPELL_COSMETIC_BOMB      = 96280;  // the 1% Powder Explosion countdown (96283, 185000 Fire within 10 yd)
constexpr float FIERY_BLAZE_RADIUS        = 5.0f;
constexpr float POWDER_RADIUS             = 10.0f;
constexpr float ROPE_CLICK_RANGE          = 4.0f;   // a player's interaction reach
constexpr float ROPE_SCAN                 = 50.0f;
constexpr uint32 ROPE_RIDE_COOLDOWN_MS    = 15000;  // after a bot was last seen on a rope it takes no other (it lands in burning fires)
// the Nightmare Gauntlet (instance_deadmines.cpp)
constexpr uint32 NPC_TRAP_BUNNY           = 49454;
constexpr uint32 NPC_STEAM_VALVE          = 49457;
constexpr uint32 NPC_COLLAPSING_ICICLE    = 49481;  // 92201 -> Ice Shards 92203, 47500 Frost within 7 yd, 1.7 s after it appears
constexpr uint32 NPC_LIGHTNING_PLATTER    = 49520;  // addon 95519 Spark
constexpr uint32 NPC_LIGHTNING_STALKER    = 49521;  // addon 92277 Spark -> 92278, 46250 within 2 yd every 0.3 s
constexpr uint32 NPC_ENRAGED_WORGEN       = 49532;
constexpr uint32 NPC_JAMES_HARRINGTON     = 49539;
constexpr uint32 SPELL_ADRENALINE         = 92604;  // a saved Harrington's +50% speed (60 s): every player has it in Ripsnarl's nightmare
// #826 w107: James is summoned up on the ship (-83.2,-775.0,26.9), ~85 yd from Erik's dock and out of its sight
constexpr float JAMES_SCAN                = 150.0f;
constexpr float JAMES_REACH               = 30.0f;  // within this and in sight the bot fights; farther it first walks up to the ship
constexpr float JAMES_APPROACH_X          = -89.2f; // the ship ramp 6.8 yd W of James (navmesh-checked, t6xx-dungeon-dm.sh staging point)
constexpr float JAMES_APPROACH_Y          = -778.0f;
constexpr float JAMES_APPROACH_Z          = 25.46f;
constexpr float JAMES_APPROACH_ARRIVED    = 3.0f;
constexpr uint32 SPELL_NIGHTMARE_FLAMES   = 92169;  // on 51594: 92171, 46250 Fire within 5 yd every 1 s
constexpr uint32 SPELL_SPARK              = 92277;
constexpr uint32 SPELL_SPARK_PLATTER      = 95519;
constexpr float NIGHTMARE_FLAMES_RADIUS   = 5.0f;
constexpr float ICE_SHARDS_RADIUS         = 7.0f;
constexpr float SPARK_RADIUS              = 2.0f;
constexpr float VALVE_RANGE               = 20.0f;  // the valves stand 5-7 yd around the trap

// Generic helpers (same code in CataLct / CataDm / CataSfk; each folder stands alone) -----------------------------
// Alive creatures of any of `entries` within `range` of the bot (grid search, same call as UBShared.cpp:94).
std::vector<Creature*> FindCreatures(Player* bot, std::initializer_list<uint32> entries, float range);
// The nearest of those within `maxDist` (2D), or nullptr.
Creature* NearestCreature(Player* bot, std::initializer_list<uint32> entries, float range, float maxDist);
// The current cast (generic or channeled) of `caster` if it is one of `spellIds`, else nullptr.
Spell* FindCast(Unit* caster, std::initializer_list<uint32> spellIds);
// True while `unit` casts one of `spellIds`, or carries the aura of that id (channels live as a self aura).
bool IsCastingOrAura(Unit* unit, std::initializer_list<uint32> spellIds);
// `unit` lies inside the cone of `source`'s facing (rear == true: facing + 180 deg) within `maxDist`.
bool InCone(Unit* source, Unit* unit, float halfAngle, float maxDist, bool rear);
// A point beside `source` (80 deg off its facing, on the side the bot already stands) - outside a front cone of
// half angle < 80 deg and outside a rear cone of half angle < 100 deg.
bool LateralPoint(Player* bot, Unit* source, float& x, float& y, float& z);
// The highest-priority alive attackable unit (entries listed in priority order) within maxDist, or nullptr when the
// bot already fights a target of equal or higher priority. engagedOnly: only units already in combat (#667 w68: the
// Helix focus pulled the idle Oaf from the trash gate).
Unit* PickFocus(PlayerbotAI* botAI, Player* bot, std::initializer_list<uint32> priority, float maxDist,
                bool engagedOnly = false);
// The nearest creature of `casterEntries` currently casting one of `spellIds`, or nullptr.
Unit* InterruptCaster(Player* bot, std::initializer_list<uint32> casterEntries, std::initializer_list<uint32> spellIds,
                      float range);
// Casts the first ready class interrupt on `target`; returns its name, or an empty string.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);

// Edges -------------------------------------------------------------------------------------------------------
// Glubtok, stage two: while he casts Fire / Frost Blossom at a player, everyone within 5 yd of that player (the
// target included) leaves; the blossom lands on the target's spot 2 s later (knock-up / slow). Flee point = the target.
bool BlossomLanding(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Glubtok, heroic: the Fire / Frost Blossom elementals that rise where a blossom landed have very low health and add up
// quickly; damage dealers kill them at once (Wowpedia: "get rid of them as soon as possible").
Unit* BlossomAddFocus(PlayerbotAI* botAI, Player* bot);

// Helix Gearbreaker: leave a Sticky Bomb (it drops at a player's feet, arms after ~7 s and explodes on a player within
// ~1.4 yd or after 18 more seconds; 3 yd knock-back).
bool StickyBombDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Helix Gearbreaker, heroic: a bot carrying the Chest Bomb (10 s aura, then 46-54k to everyone within 6 yd) leaves the
// group; everyone else leaves a carrier. Flee point = the nearest other group member / the nearest carrier.
bool ChestBombDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Helix Gearbreaker (#667 w68): one escape spot clear of EVERY bomb hazard at once - the live Sticky Bombs (5.5 yd) and the
// Chest Bomb carriers (the carrier: every other member; 8.5 yd) - sampled with MopDungeon::FindBestSpot (the GSS fix for
// the same multi-hazard ping-pong, MopGssShared.cpp:494-509). False when the bot is clear already or no walkable spot is
// better. FleePosition() only steps straight away from ONE point: a bomb dropped at the bot's own feet gives atan2(0,0) = 0,
// i.e. always due west, and without a current target it has no other angle, so a bot against the west wall never got out.
bool BombEscapeSpot(Player* bot, float& x, float& y, float& z);
// True while (x, y) is still clear of every bomb hazard (an escape walk in progress is kept while this holds).
bool BombSpotSafe(Player* bot, float x, float y);

// Helix Gearbreaker (#667): stage one is fought on the Lumbering Oaf (Helix rides it, unselectable), stage two on Helix once
// the Oaf is down (Wowpedia "Helix Gearbreaker", MediaWiki API 2026-10-14). The heroic Helix' Crew (49136-49139) sit on the
// overhead log and only throw Sticky Bombs - the tank chased them up there in w34 and the group wiped. Everyone but the healers
// keeps the Oaf / Helix as the target (the unselectable Helix and the "dead" Oaf drop out of "possible targets" by themselves,
// AttackersValue.cpp:169-172), the same shape as AC's Novos target priority (AC DTKActions.cpp:65, DTKStrategy.cpp:23).
Unit* HelixFocus(PlayerbotAI* botAI, Player* bot);

// Foe Reaper 5000: leave the Harvest destination (the targeting bunny 47468); the Reaper runs there and Harvest Sweep
// hits 184-215 within 10 yd and knocks back.
bool HarvestDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Foe Reaper 5000: while Overdrive (10 s self aura, 10 yd damage every second, the boss then runs wildly at double
// speed) is up everyone, the tank included, stays out of 10 yd of the boss. Flee point = the boss.
bool OverdriveDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Admiral Ripsnarl: kill the Vapors of the fog phases at once (heroic: unkilled they grow every 8 s through Swirling /
// Condensing / Freezing Vapor and Coalesce). Everyone but the healers, the tank included (the boss is gone then).
Unit* VaporFocus(PlayerbotAI* botAI, Player* bot);

// Vanessa VanCleef (heroic): above 50% she calls a Defias Enforcer, Shadowguard and Blood Wizard every 60 s (#681); damage
// dealers kill them (Blood Wizard first: Ragezone / Bloodbolt), before Vanessa.
Unit* VanessaAddFocus(PlayerbotAI* botAI, Player* bot);


// ---- #681 / #826 ---------------------------------------------------------------------------------------------
// Glubtok heroic Fire Wall: a line of fire through the pivot that turns clockwise once every 20 s. The wall is due over the bot
// within FIRE_WALL_WARN s (or is on it) -> a spot perpendicular to the wall on the bot's side (the middle of the wedge between
// the approaching half and the receding one, 5 s from either), a little further round for the turn during the walk, at 6-15 yd
// from the pivot (Wowpedia: "slowly rotate around the room ... It is easy to avoid").
bool FireWallDanger(Player* bot);
bool FireWallSpot(Player* bot, float& x, float& y, float& z);

// Foe Reaper heroic Molten Slag: everyone keeps out of a slag's 10 yd Molten Shield (the slag is slower than a player, so a
// fixated bot kites it); the ranged damage dealers kill the slags from range (Health_mod 2, Wowpedia: low health).
bool MoltenSlagDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);
Unit* MoltenSlagFocus(PlayerbotAI* botAI, Player* bot);
// #681 w106: the tank keeps the engaged Foe Reaper. A slag fixating a party member drew "tank assist" off the boss (the tank's
// [PROBE] target: 41 ticks on slags 49229, 32 on the Reaper), the Reaper then cut down the rogue and the tank died chasing slags.
// Wowpedia "Foe Reaper 5000" (heroic): the slags are held by the Prototype Reaper's pilot, not the tank; "It is not necessary to kill
// the adds before the boss dies". Same pattern as "dm helix focus" (engaged units only).
Unit* ReaperTankFocus(PlayerbotAI* botAI, Player* bot);

// Vanessa VanCleef 50% / 25% detonations: the deck burns (Fiery Blaze 5 yd); the way out is a rope at the side of the ship
// (Wowpedia: "Players must escape using the ropes"). The bot walks to the nearest free rope once the fires are on the deck and
// takes it when they blaze (a rope swing lasts ~9.7 s; the deck's fires go out 11 s after they appear, 6.2 s after they blaze,
// so an earlier swing lands in them). Also any Fiery Blaze within 5 yd (the entrance fires, a late bot).
Creature* VanessaRope(Player* bot);
bool VanessaBlazeActive(Player* bot);   // a detonation fire blazes within ROPE_SCAN
bool VanessaFiresOnDeck(Player* bot);   // detonation fires exist within ROPE_SCAN (blazing or about to)
bool VanessaBlazeDanger(Player* bot);

// Vanessa VanCleef at 1%: the Powder Explosion (10 yd); everyone leaves her (Wowpedia: "just go to the edge of the boat").
bool PowderKegDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Nightmare Gauntlet, Vanessa's trap: a bot seated on the trap (49454) clicks the nearest free Steam Valve.
Creature* SteamValveToClick(Player* bot);

// Nightmare Gauntlet ground hazards: Glubtok's nightmare flames (5 yd), a Collapsing Icicle (7 yd), the spark chains (2 yd) and
// any Fiery Blaze (5 yd). One escape spot clear of all of them (MopDungeon::FindBestSpot, like BombEscapeSpot).
bool GroundHazardDanger(Player* bot);
bool GroundHazardEscapeSpot(Player* bot, float& x, float& y, float& z);

// Nightmare Gauntlet, Ripsnarl's nightmare: kill the Enraged Worgen attacking Emme / Erik Harrington and James Harrington (Calissa
// chokes on his ship) at once - everyone but the healers (Wowpedia: "There is no tanking required here").
// #826 w107: James out of reach (beyond JAMES_REACH or out of sight) is returned to every role, healers included: the action then
// walks the bot up to the ship (JamesApproachSpot) instead of attacking.
Unit* HarringtonFocus(PlayerbotAI* botAI, Player* bot);
// #826 w107 (nm-james FAIL, "Calissa Harrington died" 34 s after the spawn): James is summoned on the ship, ~85 yd from Erik's
// dock, beyond FOCUS_RANGE and out of sight, so no bot picked him up; the group stood on the dock (follow, master 0 yd) until the
// driver's teleport landed 24 s into Calissa's 33 s (Choking 92608, 3% a second). Wowpedia "Vanessa VanCleef (tactics)": "After
// saving him, continue up to the ship ... You must kill James before he drops his wife". True, with the ramp spot next to James,
// while the bot still has to walk up there.
bool JamesApproachSpot(Player* bot, float& x, float& y, float& z);

// True while one of this dungeon's "get out of the way now" mechanics applies to the bot (the avoid multiplier then
// keeps follow / reach / chase movement from walking the bot straight back in).
bool InDanger(PlayerbotAI* botAI, Player* bot);
}

#endif
