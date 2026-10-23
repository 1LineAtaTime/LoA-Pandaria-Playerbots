/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" - Scholomance (map 1007, level 90 revamp). Epic #373, issue #659.
//
// AC WotLK mod-playerbots has no strategy for this dungeon, so it is authored on AC's dungeon-strategy PATTERN
// (AC Ai/Dungeon/HoS, UB, MgT: Strategy / Triggers / Actions / Multipliers / ActionContext / TriggerContext).
//
// Sources for the intended mechanics (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>): Instructor_Chillheart,
//     Jandice_Barov, Rattlegore, Lilian_Voss_(tactics), Darkmaster_Gandling - "Spells and abilities" / "Strategy".
//   Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for the spell texts below.
// Ids, radii and interruptibility are checked against THIS core: the fork's boss scripts
// (src/server/scripts/EasternKingdoms/Scholomance/*.cpp) and the 5.4.8 Spell/SpellEffect/SpellInterrupts DBCs.
//
// Edges (1-4 per boss; only mechanics that the fork's scripts really cast):
//   Instructor Chillheart - outrun the Ice Wave (#659, 2026-10-13), Frigid Grasp ground burst, Ice Wrath carrier
//                           spreads, phase 2: kill the Phylactery
//                           (the tome adds are untargetable), stay out of Arcane Bomb markers / Fire Tome paths
//   Jandice Barov         - kill the REAL Jandice in the illusion phases, Gravity Flux ground circle, interrupt
//                           Wondrous Rapidity
//   Rattlegore            - click a Bone Pile for Bone Armor (absorbs Bone Spike), Soulflame pools, interrupt Bone Spike
//   Lilian Voss           - Dark Blaze pools, spread 8 yd (Shadow Shiv), a bot fixated by Lilian's Soul keeps running
//   Darkmaster Gandling   - interrupt Incinerate / Immolate, kill the Failed Students while he channels Rise
//
// Script gaps (mechanics the fork does NOT implement, so no edge exists for them): see the issue comment.

// Issue #781 additions (2026-10-11):
//   Jandice / Rattlegore / Gandling interrupts: the wave 33 jandice run logged 563 FAILED of 578 "scholo priority
//     interrupt" - the [CAST] rejects were "cooldown" (every bot kept firing at the same cast after the first kick
//     landed) and "channeling". The ready interrupters now take turns (MopDungeon::InterruptGate), tank included.
//   Gandling's Rise!: the damage dealers already kill the Failed Students (FindFocusTarget); the tank now picks up the
//     students that are not on him (Wowpedia: "The students swarm the area, fighting anything they can").
//   Chillheart -> Phylactery and Jandice's real image (59184) are the existing FindFocusTarget switches.
//   Wondrous Rapidity (114062) is no longer interrupted: the tank steps behind Jandice instead (AC Ingvar "dodge
//   smash" pattern, see JandiceRapidityActive below).
//   Bone Spike (113999) and Gandling's Incinerate (113136) / Immolate (113141) are no longer priority interrupts
//   either: Wowpedia "Rattlegore" (2026-10-12) counters Bone Spike with the Bone Piles' Bone Armor (the existing
//   "scholo bone armor" edge) and Wowpedia "Darkmaster Gandling" (2026-10-11) only says "Gandling does not need to be
//   tanked normally, as he will only use Incinerate and Immolate"; the Wowhead MoP tooltips of 113999 / 113136 /
//   113141 (2026-10-12) give no interrupt instruction. With no sourced entry left, the "scholo priority interrupt"
//   edge is kept registered but undriven (its TriggerNode is commented out in MopScholoStrategy.cpp). The class
//   strategies' own generic interrupts still run.

#ifndef PLAYERBOTS_MOPSCHOLOSHARED_H
#define PLAYERBOTS_MOPSCHOLOSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"
#include <string>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace MopScholomance
{
constexpr uint32 MAP_SCHOLOMANCE = 1007;

// Creature entries: scholomance.h:145-196 (fork), identical in the MoP test world DB.
constexpr uint32 NPC_INSTRUCTOR_CHILLHEART = 58633;
constexpr uint32 NPC_PHYLACTERY            = 58664;
constexpr uint32 NPC_FROSTGRASP_STALKER    = 57860;
constexpr uint32 NPC_ICE_WALL_STALKER      = 68291;   // scholomance.h:154, Ice Wave origin (east end of the room)
constexpr uint32 NPC_ICE_WALL_ROW          = 62731;   // scholomance.h:156 NPC_ICE_WALL_MOVE, one wall row = 15 of them
constexpr uint32 NPC_ARCANE_BOMB           = 58753;   // ground marker the falling Arcane Bomb detonates on
constexpr uint32 NPC_ANTONIDAS_GUIDE       = 66240;   // Fire Tome: burns everything within 2 yd of it
constexpr uint32 NPC_JANDICE_BAROV         = 59184;   // the real boss
constexpr uint32 NPC_JANDICE_ILLUSION      = 59220;   // the 14 decoys
constexpr uint32 NPC_GRAVITY_FLUX_STALKER  = 300001; // #685: must match scholomance.h
constexpr uint32 NPC_RATTLEGORE            = 59153;
constexpr uint32 NPC_BONE_PILE             = 59304;
constexpr uint32 NPC_SOULFLAME_STALKER     = 45979;
constexpr uint32 NPC_LILIAN_VOSS           = 59200;
constexpr uint32 NPC_LILIAN_SOUL           = 58791;
constexpr uint32 NPC_DARK_BLAZE            = 58780;
constexpr uint32 NPC_GRANDMASTER_GANDLING  = 59080;   // the boss (58875 is the intro NPC)
constexpr uint32 NPC_FAILED_STUDENT        = 59078;   // summoned by Rise (113144, SpellEffect misc 59078)
constexpr uint32 NPC_FRESH_TEST_SUBJECT    = 59099;   // #806 classroom: friendly, carries Explosive Pain (faction 35)
constexpr uint32 NPC_EXPIRED_TEST_SUBJECT  = 59100;   // #806 classroom: hostile, the ones that attack the lesson's victim

// Spell ids: boss_instructor_chillheart.cpp:26-37, scholomance.h:117, boss_jandice_barov.cpp:27-31,
// boss_rattlegore.cpp:26-35, boss_lilian_voss.cpp:27-37, boss_grandmaster_gandling.cpp:26-37.
// A heroic cast keeps the same id with a per-difficulty SpellInfo (SpellMgr.cpp:471-476).
constexpr uint32 SPELL_ICE_WRATH           = 111610;  // aura on a player: 2.5 yd frost pulse on his neighbours, 10 s
constexpr uint32 SPELL_FRIGID_GRASP        = 111239;  // aura on the Frost Grasp Stalker, bursts (114886, 3.5 yd) after 4 s
constexpr uint32 SPELL_ARCANE_BOMB_MARKER  = 113848;  // cosmetic aura the script puts on the ground marker
constexpr uint32 SPELL_FIRE_TOME           = 111574;  // aura on a Fire Tome (periodic Burn, 2 yd)
constexpr uint32 SPELL_WONDROUS_RAPIDITY   = 114062;  // 1.5 s cast, interruptible (flags 0x29), then a 6 s cone barrage
constexpr uint32 SPELL_BONE_SPIKE          = 113999;  // 1 s cast, interruptible (flags 0x29), 300k + knockback
constexpr uint32 SPELL_BONE_ARMOR          = 113996;  // absorbs one Bone Spike; granted by clicking a Bone Pile
constexpr uint32 SPELL_SOULFLAME           = 114007;  // aura on a Soulflame stalker; the pool grows with its scale
constexpr uint32 SPELL_DARK_BLAZE_FIRE     = 111587;  // aura on a Dark Blaze pool (2 yd, 30 s)
constexpr uint32 SPELL_FIXATE_ANGER        = 115350;  // aura on the player Lilian's Soul chases
constexpr uint32 SPELL_INCINERATE          = 113136;  // 2 s cast, interruptible
constexpr uint32 SPELL_IMMOLATE            = 113141;  // 2 s cast, interruptible
constexpr uint32 SPELL_RISE                = 113143;  // 26 s channel, boss takes 50% less damage while it runs
constexpr uint32 SPELL_EXPLOSIVE_PAIN      = 113280;  // #806 aura on a Fresh Test Subject; removed (dispel) -> 113312, 8 yd, Expired only

// A damaging stalker the bot must stay out of.
struct Hazard
{
    uint32 entry;       // creature entry of the ground-effect carrier
    uint32 aura;        // aura it must carry to be live (0 = any)
    float radius;       // damage radius
    float scaleRadius;  // extra radius per unit of the carrier's object scale (Soulflame grows with scale)
    float margin;       // extra clearance the bot aims for
};

inline constexpr Hazard HAZARD_FRIGID_GRASP   = { NPC_FROSTGRASP_STALKER,   SPELL_FRIGID_GRASP,       3.5f, 0.0f, 2.0f };
inline constexpr Hazard HAZARD_ARCANE_BOMB    = { NPC_ARCANE_BOMB,          SPELL_ARCANE_BOMB_MARKER, 2.0f, 0.0f, 1.5f };
inline constexpr Hazard HAZARD_FIRE_TOME      = { NPC_ANTONIDAS_GUIDE,      SPELL_FIRE_TOME,          2.0f, 0.0f, 2.5f };
inline constexpr Hazard HAZARD_GRAVITY_FLUX   = { NPC_GRAVITY_FLUX_STALKER, 0,                        3.0f, 0.0f, 2.5f };
// spell_soul_flame (boss_rattlegore.cpp:348-367): targets are the players within 2.7 + scale yd of the stalker.
inline constexpr Hazard HAZARD_SOULFLAME      = { NPC_SOULFLAME_STALKER,    SPELL_SOULFLAME,          2.7f, 1.0f, 2.0f };
inline constexpr Hazard HAZARD_DARK_BLAZE     = { NPC_DARK_BLAZE,           SPELL_DARK_BLAZE_FIRE,    2.0f, 0.0f, 1.5f };

constexpr float BOSS_SCAN_RANGE     = 100.0f;
constexpr float TARGET_SCAN_RANGE   = 60.0f;
constexpr float BONE_PILE_RANGE     = 45.0f;
constexpr float ICE_WRATH_SPREAD    = 5.0f;    // pulse radius 2.5 yd: be this far from every neighbour
constexpr float LILIAN_SPREAD       = 9.0f;    // Shadow Shiv hits everything within 8 yd of her landing
constexpr float FIXATE_SAFE_RANGE   = 20.0f;   // the Soul is slow: keep this much room and keep moving
constexpr float FIXATE_REACT_RANGE  = 24.0f;

// #659 Ice Wave (Wowpedia MediaWiki API "Instructor Chillheart", fetched 2026-10-13: "summons a wave of ice that
// slowly sweeps across the room, instantly killing any player it touches"; "The ice spreads from the east end of the
// room"; Wowhead MoP tooltip 111231 "Ice Wall: A deadly wall of ice instantly kills enemies", fetched 2026-10-13).
// This core: npc_ice_wall_stalker (boss_instructor_chillheart.cpp:386-477) lays a row at the stalker on engage and a
// new row every 7 s, 4 yd further along +Y (EVENT_MOVE_WALL, :458-466); every row creature casts 122449 -> 122482 ->
// areatrigger 348 = sat_ice_wall (:711-741), an IAreaTriggerOnce with spell_areatrigger_template.Radius 15 that casts
// the instakill 111231 on the first living player within 15 yd (2D, AreaTrigger.cpp:40-48). Rows live 7 s.
constexpr float ICE_WALL_REACH      = 15.0f;
constexpr float ICE_WALL_MARGIN     = 5.0f;    // one 4 yd wall step + reaction time

// #659 (2026-10-13) Reliquary geometry, from the navmesh (testserver data-mop-extract/mmaps/1007_31_31 + 1007_32_31,
// dtsim 3 yd grid and paths): the main floor is x 169-235, y 3-38, z 119.3; north of y ~39 there is only floor in the
// two side lobes (x < 172 / x > 232, y 42-54), whose staircases climb to the landing (x 193-211, y 47-53, z 132.1); the
// entrance corridor runs north from the landing (x 199-202 at y 54-62, x 193-210 at y 65-107). One area (6066) covers it
// all. So "walk +Y" stops at the north wall from y ~38 on, and the way out is the stairs (main floor -> landing ~70 yd).
// Chillheart evades 53 yd (+ her object size) from her spawn 200.4,6.1,119.3 (EnterEvadeIfOutOfCombatArea(diff, 53.0f),
// boss_instructor_chillheart.cpp:289; ScriptedCreature.cpp:529-648 uses GetDistance(home)): landing 46.2 yd, corridor
// mouth (200.6,56) 51.5 yd, (200.5,59) 54.4 yd. Her evade calls Reset(), which destroys the wall (:95-96, CreatureAI.cpp:329).
// Kill line = newest row y + 15 (rows y = -6.6 + 4(n-1) at 7n s, the first at the stalker y -5.3): 9.7 at engage, 16.4 at
// 21 s, 28.4 at 42 s, 40.4 at 63 s, 48.4 at 77 s, 52.4 at 84 s, 56.4 at 91 s: the melee window ends at ~90 s.
// The only way up from the main floor goes through the west / east lobe mouths (dtsim path 200,35 -> landing: via
// 171,31 and 171,43, 66-77 yd, ~10 s). Once the safe line + 2 passes RELIQUARY_FLOOR_ESCAPE_Y (front row 5.4 at 28 s,
// kill line 20.4) every bot still on the main floor heads for the landing, while the lobe mouths are >= 10 yd clear.
constexpr float RELIQUARY_FLOOR_ESCAPE_Y = 24.0f;
constexpr float RELIQUARY_MAIN_FLOOR_MAX_Z = 126.0f;  // below this a bot is on the main floor / a lobe, not upstairs
constexpr float RELIQUARY_MAIN_FLOOR_MAX_Y = 42.0f;   // ... and south of the lobes
constexpr float RELIQUARY_LANDING_X     = 201.5f;
constexpr float RELIQUARY_LANDING_Y     = 50.5f;
constexpr float RELIQUARY_LANDING_MAX_Y = 53.0f;
constexpr float RELIQUARY_UPPER_Z       = 132.1f;
constexpr float RELIQUARY_CORRIDOR_X    = 200.6f;
constexpr float RELIQUARY_MOUTH_Y       = 56.0f;   // the last tank spot inside Chillheart's leash
constexpr float RELIQUARY_CORRIDOR_MAX_Y = 105.0f;
// #659 Jandice: the passage mouth in the north-west corner of her hall (navmesh x 260-266, y 54-66, z 113.5): 23.1 yd
// from her spawn, 29.9+ yd from every Candlestick Mage 59467 and 33.2 yd from the skeleton candle 59481 (282.5,29.7),
// the Boneweaver hall walled off (w44: the old stage 250,68 sat 15.5 yd from Boneweaver 240.6,80.1, whose Woven
// Boneguards 59242 were the tank's first target 2 s after landing - 332 target ticks, 10-16 attackers).
constexpr float JANDICE_SPOT_X = 263.0f;
constexpr float JANDICE_SPOT_Y = 56.5f;
constexpr float JANDICE_SPOT_Z = 113.5f;
constexpr float TANK_SPOT_SLACK = 3.0f;

// #806 Harsh Lesson (Wowpedia MediaWiki API "Darkmaster Gandling", fetched 2026-10-22): "teleports a random player to one of
// his study rooms"; "Healers should dispel the friendly Fresh Test Subjects of their Explosive Pain to kill the Expired Test
// Subjects. Damage dealers and tanks can just kill their way out"; Explosive Pain "explodes when dispelled, inflicting
// 400 Shadow damage to Expired Test Subjects within 8 yards". Core: boss_grandmaster_gandling.cpp (EVENT_HARSH_LESSON
// teleports to a Teleport Location stalker 59167; npc_gandling_room_stalker opens the room's door when no Expired Test
// Subject is left within 45 yd and |dz| < 5; spell_explosive_pain(_proc) filter the blast to Expired subjects).
constexpr float LESSON_ROOM_RANGE   = 30.0f;   // a classroom is a single room: subjects within this are "my room"
constexpr float LESSON_ROOM_DZ      = 5.0f;    // same floor test as the core's door stalker
constexpr float LESSON_BLAST_RADIUS = 8.0f;    // 113312
constexpr float LESSON_RETURN_RANGE = 35.0f;   // farther than this from Gandling, with the room cleared: walk back
constexpr float LESSON_BOSS_SCAN    = 200.0f;  // the core scans its Teleport Location stalkers within 200 yd of him

// Alive NPC by entry within BOSS_SCAN_RANGE ("find target" needs the unit on the bot's threat list; entry lookups do not).
Creature* FindBoss(Player* bot, uint32 entry);

// Nearest live hazard carrier closer than its radius + margin (3D distance: Fire Tomes hover 7 yd up); clearDistance =
// the distance the bot must reach.
Creature* FindNearestHazard(Player* bot, Hazard const& hazard, float& clearDistance);

// The NPC `entry` that has this bot on its threat list (AC "find target" semantics, TargetValue.cpp FindTargetValue,
// matched by entry because Jandice's illusions share her name); nullptr when the bot is not fighting it.
Creature* FindEngagedBoss(Player* bot, uint32 entry);

// #659 Ice Wave: the Y line the bot must reach (front row + ICE_WALL_REACH + ICE_WALL_MARGIN; the wall walks +Y) while
// Chillheart's first phase is on; 0 when no wall is up. `shortfall` = how far the bot still is from it (<= 0 = clear).
float IceWallSafeY(Player* bot, float& shortfall);

// #659: where a bot outruns the wall to (safe line + 2): straight +Y while that is short of RELIQUARY_FLOOR_ESCAPE_Y,
// then the landing, then the corridor (the navmesh path climbs the side-lobe stairs). Geometry above. IceWallSafeY's
// shortfall also counts a bot on the main floor as short of the landing once the floor is closing.
void IceWallOutrunSpot(Player* bot, float safeY, float& x, float& y, float& z);

// #659: the spot the tank holds its boss on, false when none: Chillheart while the wall is up -> the landing, then the
// corridor mouth once the safe line passes the landing (she follows her victim; the tank pulls her out of the wall
// and spends the move at the start of the fight instead of a 70 yd trek mid-fight); Jandice outside her illusion
// phases -> the passage mouth (JANDICE_SPOT_*), so the fight stays out of the candle crowd's reach.
bool TankSpot(Player* bot, float& x, float& y, float& z);

// Nearest other living group member on the bot's map within `within` yd (2D); nullptr when none.
Player* FindNearestGroupMember(Player* bot, float within);

// Interrupt priority of a cast: 1 = most urgent, 0 = not on the list (#781: no sourced entry is left, see above).
uint8 InterruptPriority(uint32 spellId);
uint8 CurrentCastPriority(Unit* caster);

// Jandice / Rattlegore / Gandling, whichever is casting the most urgent interruptible spell; nullptr when none.
Unit* FindInterruptTarget(Player* bot);

// Cast the bot's interrupt (or hard control) on target; returns the spell name used, empty when none was possible.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);

// The unit this bot should be killing instead of the boss, or nullptr:
//   Chillheart phase 2: the Phylactery once it is attackable (every non-healer, the tank included);
//   Jandice illusion phases: the real Jandice (entry 59184; the clones are 59220) for every non-healer;
//   #659 Chillheart's first lesson while the Ice Wave is up: Chillheart, damage dealers only (Wowpedia: the wall
//     "gives players roughly two minutes to kill the lich"; here ~90 s, so no damage goes to the Risen Guards);
//   #659 Lilian Voss stage 2: Lilian's Soul (58791) while it can still be hit, every non-healer (her body is
//     unselectable until the Soul "dies"; Wowpedia "Lilian Voss (tactics)" 2026-10-14: "unload on the spirit");
//   Gandling while he channels Rise: a Failed Student, damage dealers only (#659: the current one, else the most damaged).
Unit* FindFocusTarget(Player* bot);

// #806: Gandling is in combat and a live Expired Test Subject (hostile, same floor) stands within LESSON_ROOM_RANGE:
// the bot is in a Harsh Lesson classroom.
bool InHarshLessonRoom(Player* bot);

// #806: the Fresh Test Subject (friendly, carries Explosive Pain, in line of sight) whose dispel blast hits the most live
// Expired Test Subjects (at least one); nullptr when none. `hits` = how many.
Creature* FindExplosivePainSubject(Player* bot, uint32* hits = nullptr);

// #806: nearest live Expired Test Subject of the bot's classroom; nullptr outside a lesson.
Unit* FindLessonTarget(Player* bot);

// #806: the bot knows a friendly-target dispel (Purify / Cleanse / Detox / Nature's Cure / Purify Spirit / Cleanse Spirit).
bool HasFriendlyDispel(PlayerbotAI* botAI);

// #806: cast the bot's friendly dispel on target; true when a cast started.
bool CastFriendlyDispel(PlayerbotAI* botAI, Unit* target);

// #806: Gandling is in combat, this bot is far from him (> LESSON_RETURN_RANGE) and no Expired subject is left within
// LESSON_ROOM_RANGE: the classroom is clear and its door open, so the bot walks back to him. `gandling` is set when true.
bool LessonExitNeeded(Player* bot, Creature*& gandling);

// A visible Bone Pile within BONE_PILE_RANGE, nearest first, skipping a pile next to a burning Soulflame (#659);
// nullptr when none.
Creature* FindBonePile(Player* bot);

// Rattlegore is engaged and the bot carries no Bone Armor.
bool NeedsBoneArmor(Player* bot);

// The bot is fixated by Lilian's Soul and the Soul is within FIXATE_REACT_RANGE.
bool FixatedBySoul(Player* bot, Creature*& soul);

// Sources (owner rule 2026-10-12: MoP-only mechanics follow the intended design):
//   Wowpedia MediaWiki API, page "Jandice Barov" (fetched 2026-10-12): Wondrous Rapidity is type=Tank - "unleashing
//     a barrage of attacks ... in a cone in front of her"; Strategy: "When she starts casting Wondrous Rapidity, the
//     tank can move behind her ..., as she will not move to keep the tank targeted after the cast or the channel has
//     started."
//   Wowhead MoP tooltip API nether.wowhead.com/mop-classic/tooltip/spell/114062 (fetched 2026-10-12): "1.5 sec cast",
//     "9 sec cooldown", "Attacks hit for 170625 to 179375 Arcane damage in a cone in front of the caster".
//   Neither says to interrupt it, so it is a tank sidestep, not an interrupt.
// #781: Jandice (59184) is casting or channelling Wondrous Rapidity; `boss` is set when true.
// #659: only a Jandice this bot is fighting counts (FindEngagedBoss, AC's "find target"): she stands 89 yd from
// Chillheart and 46 yd from Rattlegore, inside the 100 yd entry scan.
bool JandiceRapidityActive(Player* bot, Creature*& boss);

// #781 add wave: the tank picks up the Failed Students of Gandling's Rise! that are not on him.
inline MopDungeon::WaveSpec GandlingTankWave()
{
    return {NPC_GRANDMASTER_GANDLING, {NPC_FAILED_STUDENT}, MopDungeon::WaveRole::Tank, TARGET_SCAN_RANGE};
}
}

#endif
