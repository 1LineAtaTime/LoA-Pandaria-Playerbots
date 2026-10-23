/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (map 755, "cata-lct"). Epic #373, issue #666.
// AC WotLK has no Cata dungeon, so there is no AC file to cite: this follows AC's dungeon-strategy PATTERN
// (Strategy / Triggers / Actions / Multipliers / contexts, cf. fork strategy/dungeons/HoS, UB, FoS, MgT) and uses
// the REAL ids of THIS core. Ids: the fork's boss scripts
// (src/server/scripts/Kalimdor/LostCityOfTheTolvir/boss_*.cpp, lost_city_of_the_tolvir.h), the MoP test world DB
// (creature_template ScriptName / entry) and 5.4.8 SpellEffect.dbc (summon entries, radii).
// Intended mechanics (read 2026-10-06): Wowpedia MediaWiki API, pages General Husam, Lockmaw, High Prophet Barim,
// Siamat, Augh (Abilities + Strategy sections).
// Only mechanics the fork's scripts actually drive have an edge here. #680 (2026-10-23) re-enabled Husam's Shockwave and Bad
// Intentions (TC 4.3.4 port, boss_general_husam.cpp), so they have edges now.
//   Husam    : 3 edges - get out of an armed Mystic Trap / a detonating trap (7 yd, 83171); step out of the four Shockwave
//              lines (4 yd, 83454); come down off the wall after a Bad Intentions throw
//   Siamat   : + failsafe (#820) - a bot that fell off the platform (destructible GO 205365, no navmesh) is put back on it
//   Lockmaw  : 2 edges - leave the rear arc while Dust Flail runs; kill Frenzied Crocolisks first
//   Barim    : 5 edges - leave Heaven's Fury (43801), kill Soul Fragments before they merge, leave Blaze fire (heroic),
//              Repentance: leave Hallowed Ground (13 yd around Barim), everyone on the Harbinger of Darkness (43927)
//   Siamat   : 3 edges - leave Cloud Burst / Tempest Storm, kill the Servants while Deflecting Winds (-90%) is up,
//              leave a Servant that is about to Lightning Charge (heroic)

#ifndef PLAYERBOTS_CATALCTSHARED_H
#define PLAYERBOTS_CATALCTSHARED_H

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

namespace CataLct
{
constexpr uint32 MAP_ID = 755;

// Bosses (fork lost_city_of_the_tolvir.h:35-39) and the creatures the mechanics spawn (MoP world DB creature_template)
constexpr uint32 NPC_HUSAM                 = 44577;
constexpr uint32 NPC_LOCKMAW               = 43614;
constexpr uint32 NPC_BARIM                 = 43612;
constexpr uint32 NPC_SIAMAT                = 44819;
constexpr uint32 NPC_LAND_MINE             = 44796;  // npc_land_mine (boss_general_husam.cpp:263), the mine itself
constexpr uint32 NPC_FRENZIED_CROCOLISK    = 43658;  // npc_frenzied_croc (boss_lockmaw_augh.cpp:45)
constexpr uint32 NPC_HEAVENS_FURY          = 43801;  // summoned by 81939 (SpellEffect.dbc eff0 misc 43801), auras 81940/81941
constexpr uint32 NPC_SOUL_FRAGMENT         = 43934;  // npc_lstv_soul_fragment (boss_high_prophet_barim.cpp:63)
constexpr uint32 NPC_HARBINGER_OF_DARKNESS = 43927;  // npc_harbinger_of_darkness, summoned by 82203 (SpellEffect.dbc eff0 misc 43927)
constexpr uint32 NPC_BLAZE_FIRE            = 48907;  // summoned by 91189 (heroic fire patch), aura 91195
constexpr uint32 NPC_BLAZE_OF_THE_HEAVENS  = 48906;  // npc_blaze_of_the_heavens (boss_high_prophet_barim.cpp:379), the heroic phoenix / its egg
constexpr uint32 NPC_CLOUD_BURST           = 44541;  // npc_cloud_burst (boss_siamat.cpp:501, summoned by 83790)
constexpr uint32 NPC_TEMPEST_STORM         = 44713;  // boss_siamat.cpp:54, auras 84512 / 83406 / 83406
constexpr uint32 NPC_SERVANT_OF_SIAMAT_1   = 45259;  // 84547 (Static Shock 1) summons it
constexpr uint32 NPC_SERVANT_OF_SIAMAT_2   = 45268;  // 84553
constexpr uint32 NPC_SERVANT_OF_SIAMAT_3   = 45269;  // 84554, boss_siamat.cpp:55 NPC_SETVANT_OF_SIAMAT
constexpr uint32 NPC_SHOCKWAVE_STALKER     = 44711;  // #680: summoned by 83131-83134, runs 40 yd out from Husam (npc_shockwave_stalker)
constexpr uint32 NPC_SHOCKWAVE_VISUAL      = 44712;  // #680: the line's trail (83128), 83454 4 yd 4.6 s after it appears

// Spells (every id is cast / applied by a fork script)
constexpr uint32 SPELL_TOLVIR_LAND_MINE_VISUAL = 83110;  // boss_general_husam.cpp:29, added when the mine arms
constexpr uint32 SPELL_LAND_MINE_COUNTDOWN     = 85523;  // :30 SPELL_TOLVIR_LAND_MIVE_PERIODIC, 5 s countdown aura -> 83171
constexpr uint32 SPELL_DUST_FLAIL              = 81642;  // boss_lockmaw_augh.cpp:24, self aura on Lockmaw (dust flail script)
constexpr uint32 SPELL_DUST_FLAIL_AURA         = 81646;  // :25, periodic trigger of 81644 (7 yd, behind the boss)
constexpr uint32 SPELL_DEFLECTING_WINDS        = 84589;  // boss_siamat.cpp:26, -90% damage taken on Siamat in stage one
constexpr uint32 SPELL_SHOCKWAVE               = 83445;  // #680: Husam's 5 s cast while the four stalkers run out
constexpr uint32 SPELL_SHOCKWAVE_ARMED         = 83127;  // #680: aura on a Shockwave Visual until 7 s (damage at 4.6 s)
constexpr uint32 SPELL_REPENTANCE_START        = 82320;  // boss_high_prophet_barim.cpp:30, self aura on Barim for the whole Repentance stage
                                                         // (eff0 immune to all schools, eff1 every 1 s 88814 Hallowed Ground)

// Geometry (SpellEffect.dbc radii cross-checked with Wowpedia)
constexpr float MINE_ARMED_RADIUS        = 3.5f;   // npc_land_mine triggers at 1.4 yd (boss_general_husam.cpp:324)
constexpr float MINE_BLAST_RADIUS        = 8.0f;   // 83171: 7 yd, + margin; only while the countdown aura runs
constexpr float HEAVENS_FURY_RADIUS      = 5.0f;   // 4 yd (Wowpedia) + margin
constexpr float HALLOWED_GROUND_RADIUS   = 14.5f;  // 88814: 12 yd here (SpellMgr.cpp:4287-4289 RadiusEntry 32), Wowpedia 13 yd; + margin
constexpr float BLAZE_FIRE_RADIUS        = 4.5f;   // 91196: 3 yd + margin
constexpr float CLOUD_BURST_RADIUS       = 6.0f;   // 83051: 5 yd + margin
constexpr float TEMPEST_STORM_RADIUS     = 7.0f;   // 83446: 6 yd + margin
constexpr float LIGHTNING_CHARGE_RADIUS  = 11.0f;  // Wowpedia: 10 yd
constexpr float FLEE_MARGIN              = 1.5f;
constexpr float DUST_FLAIL_RANGE         = 9.0f;   // 81644: 7 yd
constexpr float DUST_FLAIL_REAR_ARC      = 1.48f;  // 85 deg each side of the tail
constexpr float SCAN_RANGE               = 40.0f;
constexpr float FOCUS_RANGE              = 40.0f;
constexpr float SERVANT_CHARGE_HP_PCT    = 6.0f;   // the fork forces the servant to 1 hp before the charge (boss_siamat.cpp:349)
// #680 Shockwave: four lines from Husam (N/S/E/W of his facing), 40 yd run from 4 yd out, 83454 hits 4 yd around each visual
constexpr float SHOCKWAVE_HALF_WIDTH     = 5.5f;   // 4 yd + margin
constexpr float SHOCKWAVE_SAFE_OFFSET    = 7.0f;   // where a bot steps to, measured from the line
constexpr float SHOCKWAVE_LENGTH         = 46.0f;  // 4 + 40 yd + margin
constexpr float SHOCKWAVE_NEAR_RADIUS    = 9.5f;   // inside ~10 yd the lines are too close: go to the 45 deg gap at this radius
// #680 Bad Intentions: a thrown bot is ejected at its Bad Intentions Target (44586, z ~28, 12.6 yd above Husam's floor; navmesh
// mmtile_probe: no polygon under any of the 8 spawns). A client falls from there. A bot stays in the air, out of reach of the
// floor mechanics.
constexpr float THROWN_HEIGHT            = 4.0f;   // above Husam's z = still on the wall
constexpr float THROWN_STEP              = 5.0f;   // the floor point: this far toward Husam
// #820 Siamat platform: the destructible GO 205365 has no navmesh surface (mmaps 0755: nearest polygon z 0.2 under z ~35.5).
// A point move or a knock-back plans on the ground mesh 35 yd below. w104 siamat.log: the tank and the mage dropped
// 35 -> 0 in ~4 probes on mgen 8 (point) and mgen 16 (effect/knock-back).
constexpr float SIAMAT_HOME_X            = -10948.5f;  // Siamat's spawn (MoP test world creature 330941)
constexpr float SIAMAT_HOME_Y            = -1399.8f;
constexpr float PLATFORM_AREA_RADIUS     = 54.0f;      // TC 4.3.4 instance boundary for DATA_SIAMAT (CircleBoundary 54 yd)
constexpr float PLATFORM_FALL_Z          = 30.5f;      // the floor is z 35.5-36; below this a bot has fallen off
constexpr float PLATFORM_RETURN_X        = -10940.5f;  // the driver's Siamat staging spot: bots stood there at z 35.9 (w104 probes)
constexpr float PLATFORM_RETURN_Y        = -1385.9f;
constexpr float PLATFORM_RETURN_Z        = 35.9f;
constexpr uint32 PLATFORM_RETURN_COOLDOWN_MS = 3000;

// Deflection of a lateral escape point from the source's facing (see LateralPoint)
constexpr float LATERAL_ANGLE = 1.396f;  // 80 deg

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
// bot already fights a target of equal or higher priority.
Unit* PickFocus(PlayerbotAI* botAI, Player* bot, std::initializer_list<uint32> priority, float maxDist);
// The nearest creature of `casterEntries` currently casting one of `spellIds`, or nullptr.
Unit* InterruptCaster(Player* bot, std::initializer_list<uint32> casterEntries, std::initializer_list<uint32> spellIds,
                      float range);
// Casts the first ready class interrupt on `target`; returns its name, or an empty string.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);

// Edges -------------------------------------------------------------------------------------------------------
// General Husam: an armed Tol'vir Land Mine within 3.5 yd (it explodes on a player within 1.4 yd) or any mine whose
// 5 s detonation countdown runs (Detonate Traps, heroic) within 8 yd. Flee point = the mine.
bool LandMineDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// General Husam, Shockwave (#680): the bot stands within 5.5 yd of one of the four lines - a running stalker's ray while Husam
// casts 83445, or an armed Shockwave Visual. Wowpedia "General Husam" Strategy: "Everyone should run out of them". Fills the
// escape point: perpendicular out of the line, or (within ~10 yd of Husam, where the lines are too close) the 45 deg gap
// between two lines at 9.5 yd.
bool ShockwaveDanger(PlayerbotAI* botAI, Player* bot, float& x, float& y, float& z);

// General Husam, Bad Intentions (#680): the bot was thrown onto a wall and left in the air above Husam's floor. Fills a floor
// point 5 yd toward Husam at his z (Wowpedia: "they are stunned as they slide down the wall back onto the ground").
bool ThrownOnWall(PlayerbotAI* botAI, Player* bot, float& x, float& y, float& z);

// Siamat (#820): the bot fell off Siamat's platform during the fight (z < 30.5 within 54 yd of his spawn, Siamat alive and in
// combat, the bot not in a vehicle). The return is a teleport, AC's pattern for a deck/terrace with no navmesh:
// ICCActions_GSB.cpp:199-211 "Z-axis correction: teleport bot back if fallen below deck" and KaraActions.cpp:1527-1534
// NightbaneTeleportBackToTerraceAction.
bool FellOffPlatform(PlayerbotAI* botAI, Player* bot);

// Lockmaw: while Dust Flail runs (self aura 81642 / 81646) a non-tank bot in the 7 yd arc behind his tail steps beside him.
// Returns Lockmaw.
Unit* DustFlailBoss(PlayerbotAI* botAI, Player* bot);

// Lockmaw: damage dealers kill the Frenzied Crocolisks (Scent of Blood sends four after one player) before Lockmaw.
Unit* CrocoliskFocus(PlayerbotAI* botAI, Player* bot);

// High Prophet Barim: stand inside no Heaven's Fury (the 43801 stalker deals 13-16k every 1.5 s within 4 yd for 9 s).
bool HeavensFuryDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// High Prophet Barim, Repentance: damage dealers kill the Soul Fragments before one reaches the Harbinger of Darkness
// (Merged Souls heals it for ~41k and enlarges it by 20%). Same idiom as AC FoSTriggers.h SwitchToSoulFragment.
Unit* SoulFragmentFocus(PlayerbotAI* botAI, Player* bot);

// High Prophet Barim, Repentance (#666 w68): stand outside Hallowed Ground. Barim's 82320 aura casts 88814 every second for
// the whole stage (7.4k normal / 9.6k heroic Holy, 12-13 yd); w68 the melee bots went back to their target next to Barim after the
// knock-back and died there (rogue 9 yd, tank 10.6 yd from Barim, ~10k/s; the mage at 25 yd lost ~1.5k/s). Wowpedia Strategy:
// "Everyone needs to stay out of the glowing ground around Barim". Flee point = Barim.
bool HallowedGroundDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// High Prophet Barim, Repentance: every bot (tank included: Wowpedia "the tank needs to immediately taunt it") attacks the
// Harbinger of Darkness - killing it ends the stage (boss_high_prophet_barim.cpp npc_harbinger_of_darkness DamageTaken ->
// ACTION_REPENTANCE_DONE). Soul Fragments stay first for the damage dealers (their action ranks higher).
Unit* HarbingerFocus(PlayerbotAI* botAI, Player* bot);

// High Prophet Barim, heroic (#819): the Blaze of the Heavens phoenix (and its egg) is no damage dealer's target while Barim is
// alive, in combat and attackable. Wowpedia "High Prophet Barim" (MediaWiki API 2026-10-14), Heroic Mode: the phoenix "can and
// should be kited away" / "neutralized by simply allowing it to remain in egg form"; the damage dealers stay on the boss. The
// phoenix burns itself down to an egg by itself (95248 self-immolation, boss_high_prophet_barim.cpp DamageTaken keeps it at
// 1 hp), so leaving it alone costs nothing. #666 w68: Repentance too - the old rule lifted the exclusion while Barim carried
// 82320, so all four bots (tank included) hit the egg (it heals 27.6k/s, 95276 eff2) for the whole stage and only the dead
// priest wanded the Harbinger (100% -> 73%); wipe at 49%. During Repentance the tank skips the egg as well (`tankOut`).
// Fills `out` (Dps / Attacker types) and `tankOut` (Tank type) with the phoenix guids within SCAN_RANGE.
void CollectBlazeExclusions(Player* bot, GuidSet& out, GuidSet& tankOut);

// High Prophet Barim, heroic: leave the 3 yd fire patches the Blaze of the Heavens drops (7-8k per 1.5 s).
bool BlazeFireDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Siamat: leave a Cloud Burst ring (5 yd, 9-10k and a knock-up each second) and the Tempest Storms (6 yd, ~6k per
// second) that the Minions of Siamat leave behind.
bool SiamatStormDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);

// Siamat, stage one: Siamat takes 10% damage (Deflecting Winds, -90%) and stage two only starts when the Servants of
// Siamat are dead, so damage dealers put the Servants first.
Unit* ServantFocus(PlayerbotAI* botAI, Player* bot);

// Siamat, heroic: a Servant of Siamat at ~1 hp casts Lightning Charge (10 yd: 10-12k and +33% damage taken per
// charge); get out of its 10 yd before it dies.
bool LightningChargeDanger(PlayerbotAI* botAI, Player* bot, Position& pos, float& radius);


// True while one of this dungeon's "get out of the way now" mechanics applies to the bot (the avoid multiplier then
// keeps follow / reach / chase movement from walking the bot straight back in).
bool InDanger(PlayerbotAI* botAI, Player* bot);
}

#endif
