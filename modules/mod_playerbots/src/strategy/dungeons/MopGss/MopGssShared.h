/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun (MoP 5.4.8, map 962) dungeon strategy "mop-gss" - shared ids and helpers (epic #373,
// issue #655).
//
// AC (WotLK) has no strategy for this dungeon: it is MoP-only content, so this EXTENDS AC's Ai/Dungeon pattern
// (UB / HoS / MgT: Strategy + Triggers + Actions + Multipliers + ValueContext) and does not port an AC file.
// Pattern anchors: fork strategy/dungeons/UB/UBStrategy.cpp, MgT/MgT{Triggers,Actions,Multipliers,ValueContext}*.
//
// Sources for the intended mechanics (fetched 2026-10-06, evidence only):
//   Wowpedia MediaWiki API: "Saboteur Kip'tilak" (the player with Sabotage gets away from everyone; watch the
//   bomb explosion waves), "Striker Ga'dok" (Strafing Run, Acid Bombs, Mantid Strikers), "Commander Ri'mok" (deal
//   with the mantid adds; stay away or behind when he does Frenzied Assault), "Raigonn" (phase 1: DPS on the weak
//   spot, one DPS and the tank on the Swarmbringers; phase 2: move out of the way when he casts Fixate).
//   Wowhead MoP tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id>: 107268 Sabotage (bombs explode after
//   5 s, 148500 Fire to the victim and everyone within 2 yd, then four directional explosions), 107120 Frenzied
//   Assault (9 yd 90-degree cone, every 0.3 s for 6 s), 111723 Fixate (15 s, the target flees), 120559 Bombard
//   (Krik'thik Saboteur, 3 s cast), 111600 Screeching Swarm (1.5 s cast), 107118 Impervious Carapace (-99% damage
//   until the Weak Spot dies).
// Ids: this fork's scripts src/server/scripts/Pandaria/GateOfTheSettingSun/{boss_saboteur_kiptilak,
// boss_striker_gadok,boss_commander_rimok,boss_raigonn}.cpp and the MoP test DB creature_template (2026-10-06).
//
// Not covered, on purpose (see the issue comment): Prey Time / Impaling Strike (tank pickups, nothing to dodge),
// Acid Bombs (target a random player from above, no ground marker on this fork), the rope vehicles of the Raigonn
// approach (a descent, not a fight mechanic). The cannons, Battering Headbutt and Viscous Fluid ARE covered since the
// #655 pass of 2026-10-14 (below).

// Issue #781 additions (2026-10-11):
//   Kip'tilak - World in Flames (Wowpedia: "once at 60% and once again at 30%"; this core: 70% and 30%,
//     boss_saboteur_kiptilak.cpp:140-160): he plants five more Stable Munitions, and 1 s later EVERY munition in the
//     room explodes (:196-207); each explosion sends four bunnies 40 yd along the cardinal axes that burn players
//     within 2 yd every 0.5 s (npc_munition_explosion_bunny :252-340). One second is too short to react, so from
//     GSS_WIF_WINDOW % above a threshold the bots stand off both axis lanes of every munition in reach.
//   Ga'dok - the Krik'thik Strikers (59778) of every Strafing Run land on the platform and attack (boss_striker_gadok
//     .cpp:330-340, npc_krikthik_striker :585-630); Wowpedia: "Deal with Mantid Strikers during the phase". The
//     damage dealers kill them, the tank picks them up (MopDungeon add wave, AC AK Nadox pattern).

// #655 w58: the Krik'thik Bombardiers 56706 over Ri'mok's ramp keep arming random Flying Mantid Bombing Stalkers 56709
// (npc_krikthik_bombarder, gate_of_the_setting_sun.cpp:433-481: 106729 -> aura 106875, periodic 106874 = 47.5k Fire within
// 5 yd, Spell.dbc). w58 lost the whole group to them twice (the brazier packs, then at the Ri'mok stage with no attacker
// listed), so every bot stands off the armed stalkers ("mop gss bomb").
// Issue #655 pass 2 (2026-10-14), Ri'mok and Raigonn. Wowpedia MediaWiki API ("Commander Ri'mok", "Raigonn") and Wowhead
// MoP tooltip API (107122, 111671, 118685, 130772), both fetched 2026-10-14; ids from the 5.4.8 Spell.dbc /
// SpellEffect.dbc / VehicleSeat.dbc:
//   Ri'mok - Viscous Fluid "won't hurt you, but decrease your damage close to 0" (Wowpedia). Each stationary tick of his
//     Viscous Fluid channel 107078 (spell_rimok_viscous_fluid, boss_commander_rimok.cpp:418-444, skips the tick while he
//     moves) summons a Viscos Fluid Stalker 56883 (107077 effect SUMMON 56883); its aura 107088 puts 107122 (-10% damage
//     done per stack, 6 yd) on the players around it. The non-tanks stand out of the pools (melee: on a clear spot that
//     still reaches him); the tank walks him out of a pool (AC Ai/Dungeon/AC/ACActions.cpp:19-47
//     ShirrakTankPositionBossAction shape). The tank picks up the Swarmers / Saboteurs that jump down the ladders
//     (npc_add_generator :277-339; the #781 add wave, AC AK Nadox).
//   Raigonn - phase 1 (Impervious Carapace 107118): "The remaining DPS should jump into cannons, which will launch them
//     on top of Raigonn, and attack the weak spot" (Wowpedia). An Artillery 59819 (spellclick 46598; selectable only
//     while DATA_ARTILLERY_STATE is IN_PROGRESS, instance_gate_of_the_setting_sun.cpp:443-495) seats the clicker on the
//     Weak Spot 1.5 s later (vehicle_artillery::OnSpellClick, boss_raigonn.cpp:586-605). The Weak Spot is vehicle 1913
//     with two seats (VehicleSeat 10570 / 10877, flag CAN_ATTACK 0x4000) and the core lets its riders hit it
//     (Unit.cpp:11928). So a melee damage dealer rides a cannon (AC Ai/Dungeon/TOC/TOCActions.cpp:178-245 ToCMountAction:
//     nearest free, selectable, friendly vehicle; walk to it; HandleSpellClick) and then focuses the Weak Spot it sits on;
//     the ranged kill the Swarm Bringers first, then shoot the Weak Spot; the tank picks up the Protectorates and Swarm
//     Bringers. Battering Headbutt (111671 / 118685): 250k Physical within 10 yd of the door he rams; in phase 1 the bots
//     keep out of both impact zones (chargePos[0] / [2], boss_raigonn.cpp:85-91) while he is on his way there.

#ifndef PLAYERBOTS_MOPGSSSHARED_H
#define PLAYERBOTS_MOPGSSSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace MopGss
{
constexpr uint32 GSS_MAP_ID = 962;

// bosses
constexpr uint32 GSS_NPC_KIPTILAK = 56906;
constexpr uint32 GSS_NPC_GADOK = 56589;
constexpr uint32 GSS_NPC_RIMOK = 56636;
constexpr uint32 GSS_NPC_RAIGONN = 56877;

// Kip'tilak
constexpr uint32 GSS_NPC_STABLE_MUNITION = 56917;
constexpr uint32 GSS_NPC_BUNNY_N_M = 56911;  // the explosion bunnies that run a 40 yd line in one cardinal direction
constexpr uint32 GSS_NPC_BUNNY_S_M = 56918;
constexpr uint32 GSS_NPC_BUNNY_E_M = 56919;
constexpr uint32 GSS_NPC_BUNNY_W_M = 56920;
constexpr uint32 GSS_NPC_BUNNY_N_P = 59205;
constexpr uint32 GSS_NPC_BUNNY_S_P = 59206;
constexpr uint32 GSS_NPC_BUNNY_E_P = 59207;
constexpr uint32 GSS_NPC_BUNNY_W_P = 59208;

// Ga'dok: the world triggers that mark the strafing lanes
constexpr uint32 GSS_NPC_STALKER_NORTH_SOUTH = 56932;
constexpr uint32 GSS_NPC_STALKER_WEST_EAST = 56913;

// Ri'mok / Raigonn adds
constexpr uint32 GSS_NPC_KRIKTHIK_SABOTEUR = 60447;
constexpr uint32 GSS_NPC_WEAK_SPOT = 56895;
constexpr uint32 GSS_NPC_SWARM_BRINGER = 56930;
constexpr uint32 GSS_NPC_ENGULFING_WINDS = 56928;
constexpr uint32 GSS_NPC_KRIKTHIK_ENGULFER = 56912;  // flies 30 yd over the courtyard, REACT_PASSIVE (boss_raigonn.cpp:320, :496)
constexpr uint32 GSS_NPC_KRIKTHIK_STRIKER = 59778;
constexpr uint32 GSS_NPC_KRIKTHIK_SWARMER = 59835;
constexpr uint32 GSS_NPC_VISCOUS_FLUID = 56883;  // Viscos Fluid Stalker, summoned by 107077
constexpr uint32 GSS_NPC_PROTECTORATE = 56929;
constexpr uint32 GSS_NPC_ARTILLERY = 59819;      // the courtyard cannons (vehicle 2085, spellclick 46598)
constexpr uint32 GSS_NPC_BOMB_STALKER = 56709;   // Flying Mantid Bombing Stalker, ~33 of them on Ri'mok's ramp x 1267-1333
constexpr uint32 GSS_NPC_KRIKTHIK_BOMBARDIER = 56706;  // the 18 bombers over Ri'mok's ramp, each arms a bomb stalker every 1-5 s
constexpr uint32 GSS_NPC_FLAK_CANNON = 60415;         // Ga'dok's tower, at 1235.8,2313.6,430.9 and 1235.8,2295.3,430.9 (world.creature)

// spells (ids from the fork's boss scripts)
constexpr uint32 GSS_SPELL_SABOTAGE = 107268;
constexpr uint32 GSS_SPELL_MUNITION_EXPLOSION_AURA = 107216;  // on the explosion bunnies while they run
constexpr uint32 GSS_SPELL_STRAFING_RUN = 107342;
constexpr uint32 GSS_SPELL_STRAFING_RUN_DMG = 116298;         // put on the lane stalkers by the strafing script
constexpr uint32 GSS_SPELL_FRENZIED_ASSAULT = 107120;
constexpr uint32 GSS_SPELL_BOMBARD = 120559;                  // Krik'thik Saboteur
constexpr uint32 GSS_SPELL_FIXATE = 111723;
constexpr uint32 GSS_SPELL_SCREECHING_SWARM = 111600;         // Krik'thik Swarm Bringer
constexpr uint32 GSS_SPELL_IMPERVIOUS_CARAPACE = 107118;      // Raigonn phase 1, removed when the Weak Spot dies
constexpr uint32 GSS_SPELL_BOMB_AURA = 106875;                // armed bomb stalker: periodic 106874, 47.5k Fire within 5 yd
constexpr uint32 GSS_SPELL_FIRE_FLAK_CANNON = 116554;         // npc_spellclick_spells(60415): the click makes the cannon cast it on itself

// instance boss indexes (DataTypes, src/server/scripts/Pandaria/GateOfTheSettingSun/gate_of_the_setting_sun.h:27-30)
constexpr uint32 GSS_BOSS_GADOK = 1;
constexpr uint32 GSS_BOSS_RAIGONN = 3;

// tuning
constexpr float GSS_SCAN = 30.0f;
constexpr float GSS_SABOTAGE_SPREAD = 6.0f;      // the bomb hits everyone within 2 yd; keep a margin
constexpr float GSS_SABOTAGE_MUNITION = 9.0f;    // munitions chain-explode next to the victim
constexpr float GSS_LANE_HALF_WIDTH = 4.0f;      // the four directional explosions travel along the axes
constexpr float GSS_LANE_CLEAR = 5.5f;
constexpr float GSS_WINDS_SAFE_DIST = 7.0f;  // w58: 107279 (the tornado's tick) hits within 5 yd; 8.5 kept the bots dodging
constexpr float GSS_FIXATE_RUN_DIST = 30.0f;
constexpr float GSS_WIF_WINDOW = 5.0f;        // % of Kip'tilak's health above 70 / 30 at which the lanes are cleared
// #655 w68: and this much BELOW the threshold. World in Flames plants its 5 munitions at the crossing and blows every munition 1 s
// later (boss_saboteur_kiptilak.cpp:142-159, :196-207); the window used to close at the crossing, i.e. before the new munitions'
// lanes could be cleared (the tank died at 29%, 65 -> 0 in one probe, 107215 = 228k Fire per bunny tick, Wowhead 2026-10-15).
// The exploded munitions stay 2 s (DespawnOrUnsummon(2000)), so the hazard ends on its own once they are gone.
constexpr float GSS_WIF_AFTER = 3.0f;
constexpr float GSS_WIF_LANE_CLEAR = 4.0f;    // bunnies burn within 2 yd of their line; 2 yd margin
constexpr float GSS_WIF_LANE_LENGTH = 41.0f;  // the bunnies run 40 yd
constexpr float GSS_FLUID_CLEAR = 7.5f;        // 107122 reaches 6 yd around a pool; 1.5 yd margin
constexpr float GSS_FLUID_DRAG_CLEAR = 10.0f;  // the tank's spot: Ri'mok stands within his reach of it, out of every pool
constexpr float GSS_HEADBUTT_SAFE = 14.0f;     // Battering Headbutt hits within 10 yd of the impact (Wowhead 111671)
constexpr float GSS_WEAKSPOT_CLEAR_PCT = 20.0f;  // #655 w69: from this Weak Spot hp% the ground ranged clear the Protectorates first
constexpr float GSS_FLAK_SCAN = 500.0f;        // the cannon and the bombers are searched map-wide (the legs run 100-300 yd away)
constexpr uint32 GSS_FLAK_CLICK_MS = 1200;     // Fire Flak Cannon has a 0.5 s cooldown (SpellCooldowns.dbc 7969); 1.2 s keeps every click a fire
constexpr uint32 GSS_FLAK_GIVEUP_MS = 45000;   // an unreachable cannon: back to the normal strategy for this long
constexpr uint32 GSS_FLAK_MAX_MOVE_FAILS = 12;
constexpr float GSS_BOMB_CLEAR = 6.5f;         // 106874 radius 5 yd (SpellRadius), 1.5 yd margin
constexpr float GSS_HEADBUTT_INNER_X = 958.30f;  // chargePos[2] (boss_raigonn.cpp:89): the inner door, once Ri'mok is DONE
constexpr float GSS_HEADBUTT_INNER_Y = 2241.68f;
constexpr float GSS_HEADBUTT_OUTER_X = 958.30f;  // chargePos[0] (boss_raigonn.cpp:87): the outer gate, before that
constexpr float GSS_HEADBUTT_OUTER_Y = 2386.92f;

enum class HazardKind : uint8
{
    None = 0,
    SabotageSelf,
    SabotageCross,
    MunitionLane,
    Strafe,
    Frenzy,
    Fixate,
    Winds,
    WorldInFlames,
    Fluid,
    Headbutt,
    Bomb
};

// Where the bot must go to be safe, 2D; z is validated by the action.
struct Hazard
{
    HazardKind kind = HazardKind::None;
    float x = 0.0f;
    float y = 0.0f;
};

enum class InterruptGroup : uint8
{
    Saboteur = 0,
    SwarmBringer
};

// The most urgent hazard for this bot right now (kind None when there is none).
bool ComputeHazard(Player* bot, Hazard& out);

// DPS bots: Ri'mok himself (#655 w65, the Saboteurs are interrupted, not killed); at Raigonn the Weak Spot (ranged) and the
// Swarm Bringers, in phase 2 the leftover adds and then Raigonn (#655 w68). Tanks (#655 w65): Ri'mok while he hits somebody
// else; Raigonn for the whole of phase 2 (#655 w68); in Raigonn's phase 1 the add pile.
ObjectGuid ComputeFocusTarget(Player* bot);

// #655 w65: this tank's boss (Ri'mok, or Raigonn in phase 2) is engaged and hitting somebody else: the tank add wave of that
// fight waits and "mop gss focus target" takes the boss back (without the wait the two switch the tank's target every tick).
// #655 w68: Raigonn in phase 2 counts whoever he hits - the tank holds him for the whole phase (no new adds come then,
// boss_raigonn.cpp:188), so the add wave waits for the whole phase.
bool TankLostBoss(Player* bot, uint32 bossEntry);

// #655 w68: what the tank's own target pick ("tank target", exclusion type Tank) must skip at Raigonn - the Krik'thik Engulfers
// (30 yd up, REACT_PASSIVE: no aggro to take, so the smart tank pick ranks them first and never reaches them) and the Weak Spot
// (17 yd up on his back). Inserts nothing away from an engaged Raigonn.
void CollectTankExclusions(Player* bot, GuidSet& out);

// #655 Raigonn phase 1: Raigonn's body (56877) takes -99% (Impervious Carapace 107118) until the Weak Spot (56895) dies,
// so damage dealers must not target it. Inserts Raigonn's guid into `out` while he is in combat with the carapace up and
// a living Weak Spot is around (the Dps / Attacker / Tank exclusion types call this; #655 w65: the tank skips him too).
void CollectCarapaceExclusions(Player* bot, GuidSet& out);

// A hostile caster of an interrupt-worthy spell of this group within 40 yd of the bot, or nullptr.
Unit* FindInterruptTarget(Player* bot, InterruptGroup group);

// Does the bot know at least one interrupt spell (kick, pummel, wind shear, mind freeze, counterspell, ...).
bool HasInterruptSpell(PlayerbotAI* botAI);

// Casts the first available interrupt on `target`. True when a spell was started.
bool CastInterrupt(PlayerbotAI* botAI, Unit* target);

// #655 Ri'mok: the spot the tank walks him to, out of the Viscous Fluid (false: nothing to do).
bool ComputeTankDragSpot(Player* bot, float& x, float& y, float& z);

// #821 Ga'dok is DONE and Krik'thik Bombardiers (56706) are alive: the Flak Cannon (60415) a bot should fire. Only ONE
// bot of the group is elected (a non-tank, non-healer damage dealer, ranged first, then the lowest GUID); `cannon` is
// nullptr for everybody else and when nothing is left to shoot. `bombardiers` is the live count (0 once Ga'dok is not
// DONE). The cannon is a plain spellclick (VehicleId 0): the click makes the cannon cast 116554 on itself and
// npc_flak_cannon::SpellHit then shoots down 5 bombers (boss_striker_gadok.cpp:789-812).
struct FlakState
{
    Creature* cannon = nullptr;
    uint32 bombardiers = 0;
};

FlakState ComputeFlak(Player* bot);

// #655 Raigonn phase 1: the Artillery this melee damage dealer should ride to the Weak Spot, or nullptr.
Creature* FindCannon(Player* bot);

// #781 add wave at Ga'dok: the landed Krik'thik Strikers (the ones still flying in are pacified and not in combat).
inline MopDungeon::WaveSpec StrikerWave(MopDungeon::WaveRole role)
{
    return {GSS_NPC_GADOK, {GSS_NPC_KRIKTHIK_STRIKER}, role, 50.0f};
}

// #655 tank add waves: Ri'mok's Saboteurs / Swarmers; Raigonn's Swarm Bringers / Protectorates (his Engulfers fly 30 yd
// up and never melee, boss_raigonn.cpp:310-316 and :485-491).
inline MopDungeon::WaveSpec RimokAddWave(MopDungeon::WaveRole role)
{
    return {GSS_NPC_RIMOK, {GSS_NPC_KRIKTHIK_SABOTEUR, GSS_NPC_KRIKTHIK_SWARMER}, role, 45.0f};
}

inline MopDungeon::WaveSpec RaigonnAddWave(MopDungeon::WaveRole role)
{
    return {GSS_NPC_RAIGONN, {GSS_NPC_SWARM_BRINGER, GSS_NPC_PROTECTORATE}, role, 60.0f};
}
}

#endif
