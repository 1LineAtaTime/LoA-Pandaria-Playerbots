/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Siege of Niuzao Temple (MoP 5.4.8, map 1011) dungeon strategy "mop-snt" - shared ids and helpers (epic #373,
// issue #656).
//
// AC (WotLK) has no strategy for this dungeon: it is MoP-only content, so this EXTENDS AC's Ai/Dungeon pattern
// (UB / HoS / MgT / TOC: Strategy + Triggers + Actions + Multipliers + ValueContext; TOC for the spellclick
// primitive) and does not port an AC file.
// Pattern anchors: fork strategy/dungeons/UB/UBStrategy.cpp, MgT/MgT{Triggers,Actions,Multipliers,ValueContext}*,
// TOC/TOCActions.cpp:218-262 (Unit::HandleSpellClick on a friendly vehicle).
//
// Sources for the intended mechanics (fetched 2026-10-06, evidence only):
//   Wowpedia MediaWiki API: "Vizier Jin'bak" (kill the 3 Sap Globules before they reach the pool), "Commander
//   Vo'jak" (move out of his charging path; move away from Vo'jak when he casts Thousand Blades), "General
//   Pa'valak" (move away from the weapon and out of his charging path; DPS should use the bombs to get the Bulwark
//   shield down; bombs arm after 3 s and explode after another 3 s), "Wing Leader Ner'onok" (move out of the
//   yellow pools; the first player to reach him after he lands must interrupt Gusting Winds).
//   Wowhead MoP tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id>: 120759 Thousand Blades (4 s
//   channel, whirlwind on nearby enemies), 120789 Dashing Strike (charge that damages in a line and knocks the
//   target back), 124283 Blade Rush (175000 within 4 yd at the sword, then a charge: 277500-322500 within 8 yd),
//   119476 Bulwark (absorbs 9000000), 119388 Siege Explosive (picks up a bomb, "Throw the bomb!", 10 s), 119703
//   Detonate (277500-322500 Fire), 121443 Caustic Pitch (40000 Nature per second, -50% speed), 121282 Gusting Winds
//   (4 s channel).
// Ids: this fork's scripts src/server/scripts/Pandaria/SiegeOfNiuzaoTemple/{boss_vizier_jinbak,
// boss_commander_vojak,boss_general_pavalak,boss_wing_leader_neronok}.cpp and the MoP test DB creature_template and
// npc_spellclick_spells (61452 -> 119388), checked 2026-10-06.
//
// Not covered, on purpose (see the issue comment): the Amberwing's Bombard circle (an instant spell, no cast to
// read), the tar-keg wave defence (Li Chu / Lo Chu throw the kegs).
//
// Issue #687 (2026-10-13) - Ner'onok's Quick-Dry Resin (121447). Wowhead MoP tooltip API
// (nether.wowhead.com/mop-classic/tooltip/spell/121447): "As the resin hardens it inflicts 7500 Fire damage every 1 sec.
// If this process completes the target becomes encased in resin and cannot move or act for 6 sec", buff text "Jump to
// prevent the resin from hardening!". Wowpedia MediaWiki API, page "Wing Leader Ner'onok": "Jumping counteracts this
// effect and if the target breaks free they become Invigorated" / "Jump if he casts Quick-Dry Resin on you."
// Build 39+ drains the victim's alternate power on every take-off (boss_wing_leader_neronok.cpp spell_quick_dry_resin,
// a MotionMaster::MoveJump or a client jump), so the bots hop with MoveJump about once a second ("mop snt resin").
//
// Issue #781 additions (2026-10-11):
//   Jin'bak - Sap Puddle soak. Detonate (120001, boss_vizier_jinbak.cpp:206-209) hits for 8500 (heroic 25500) per
//     Grow stack of the puddle (spell_jinbak_detonate, :449-461) and every player tick of Sap Residue inside the
//     puddle (radius 4 yd x puddle scale, spell_sap_residue :414-421) takes 3 stacks off (npc_sap_puddle :374-381).
//     Sources (MoP-only mechanic, no AC counterpart - owner rule 2026-10-12: follow the intended design):
//       Wowpedia MediaWiki API, page "Vizier Jin'bak" (fetched 2026-10-11): "DPS needs to move in to the puddle to
//       lower it's size, do not stand too long in it" and "Sap Residue ... The effect stacks".
//       Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>, fetched 2026-10-12): 120001
//       Detonate "Channeled (5 sec cast) ... inflicting damage based on the size of the Sap Puddle. The Sap Puddle
//       shrinks every time an enemy walks into it"; 119941 Sap Residue "Inflicts 8000 Nature damage every 1 sec for
//       10 sec" (stacking).
//     So the DPS step in while their own Sap Residue is low and leave at SNT_RESIDUE_MAX stacks ("do not stand too
//     long in it": the stacking 10 s DoT is the limit the guide warns about).
//   Pa'valak - Bulwark (119476: 4000000 normal / 6000000 default, aura 69 school absorb, SpellEffect.dbc) is broken
//     by the thrown Siege Explosive: the bomb is a ground-targeted throw (119393: TRIGGER_MISSILE to TARGET_DEST_DEST
//     -> 119395 Detonate, 400000 normal / 500000 default Fire around the point), not a unit-targeted cast; the carry
//     aura 119388 re-applies the 119386 next-spell override every 1 s for 10 s, the throw removes it
//     (spell_siege_explosive_detonate, boss_general_pavalak.cpp:556-580). One throw per pick-up. Every DPS without a
//     bomb hits Pa'valak (the shield absorbs all schools); the tank picks up the Sik'thik Soldiers (61448) the four
//     Reinforcements Summoners (61483, aura 119781: one soldier per 15 s each) send in.
//   Ner'onok - Gusting Winds is only interruptible while it runs (boss_wing_leader_neronok.cpp:383-396); after a
//     flight he lands at the far end of the bridge, so a bot with a ready interrupt runs to him when nobody is in
//     range (Wowpedia: "The first person that reaches him needs to interrupt his casting").

#ifndef PLAYERBOTS_MOPSNTSHARED_H
#define PLAYERBOTS_MOPSNTSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace MopSnt
{
constexpr uint32 SNT_MAP_ID = 1011;

// bosses and adds
constexpr uint32 SNT_NPC_JINBAK = 61567;
constexpr uint32 SNT_NPC_SAP_GLOBULE = 61623;
constexpr uint32 SNT_NPC_VOJAK = 61634;
constexpr uint32 SNT_NPC_PAVALAK = 61485;
constexpr uint32 SNT_NPC_BLADE_RUSH_STALKER = 63720;
constexpr uint32 SNT_NPC_SIEGE_EXPLOSIVE = 61452;
constexpr uint32 SNT_NPC_NERONOK = 62205;
constexpr uint32 SNT_NPC_SIKTHIK_SOLDIER = 61448;   // Pa'valak's reinforcements (119781 -> 119706, misc 61448)
constexpr uint32 SNT_NPC_SAP_PUDDLE = 61613;
// #656 w53 Vo'jak waves (boss_commander_vojak.cpp:57-60): Demolishers explode when damaged or killed - Unstable Blast,
// 42750 to everything within 16 yd (Wowpedia "Commander Vo'jak", MediaWiki API, fetched 2026-10-14: "Demolishers blow up
// with 1 hit and blow up any enemys that stand near them, shoot them before they reach the top")
constexpr uint32 SNT_NPC_SIKTHIK_DEMOLISHER = 61670;
constexpr float SNT_DEMOLISHER_BLAST = 16.0f;
// #656 w54 Vo'jak tar kegs (boss_commander_vojak.cpp npc_mantid_tar_keg :1364-1449; 5.4.8 Spell*.dbc): Li / Lo Chu jump a keg
// 61817 onto a Barrel Target, where it turns clickable (123215 sparkle + UNIT_NPC_FLAG_SPELLCLICK); the spellclick
// (npc_spellclick_spells 61817 -> 123032 "Carrying Caustic Tar") gives the 10 s next-spell override 123035 -> 123039, a
// ground-targeted TRIGGER_MISSILE (range 40) that summons the tar pool 61579 (123112, 45 s) casting 120270: 23749 Nature per
// tick, -50% speed, +10% damage taken, stacking. Wowpedia "Commander Vo'jak" (MediaWiki API, fetched 2026-10-14): "Throw the
// kegs of tar near each other and in the path that the army walks through", "be careful not to hit your party members",
// "Use a keg of tar on him to remove his Rising Speed buff".
constexpr uint32 SNT_NPC_SIKTHIK_SWARMER = 63106;
constexpr uint32 SNT_NPC_SIKTHIK_WARRIOR = 61701;
constexpr uint32 SNT_NPC_MANTID_TAR_KEG = 61817;
constexpr uint32 SNT_SPELL_CARRYING_TAR = 123032;
constexpr uint32 SNT_SPELL_TAR_THROW = 123039;
constexpr uint32 SNT_SPELL_KEG_ACTIVE = 123215;
constexpr float SNT_TAR_SAFE = 8.0f;  // no group member this close to the throw point

// spells (ids from the fork's boss scripts)
constexpr uint32 SNT_SPELL_THOUSAND_BLADES = 120759;
constexpr uint32 SNT_SPELL_DASHING_STRIKE = 120789;
constexpr uint32 SNT_SPELL_BULWARK = 119476;
constexpr uint32 SNT_SPELL_BOMB_ARMED = 119702;
constexpr uint32 SNT_SPELL_BOMB_DETONATE = 119703;
constexpr uint32 SNT_SPELL_SIEGE_EXPLOSIVE_CARRY = 119388;  // on the player who picked a bomb up (spellclick)
constexpr uint32 SNT_SPELL_SIEGE_EXPLOSIVE_THROW = 119393;  // "Throw the bomb!" (ground-targeted)
constexpr uint32 SNT_SPELL_SIEGE_EXPLOSIVE_OVERRIDE = 119386;  // next-spell override the throw consumes
constexpr uint32 SNT_SPELL_CAUSTIC_PITCH = 121443;
constexpr uint32 SNT_SPELL_GUSTING_WINDS = 121282;
constexpr uint32 SNT_SPELL_SAP_GROW = 119942;          // stacking scale aura on the Sap Puddle
constexpr uint32 SNT_SPELL_SAP_RESIDUE_DOT = 119941;   // stacking DoT on a player standing in the puddle
constexpr uint32 SNT_SPELL_QUICK_DRY_RESIN = 121447;   // Ner'onok's resin: "Jump to prevent the resin from hardening!"
constexpr uint32 SNT_SPELL_ENCASED_IN_RESIN = 121448;  // the resin hardened: 6 s stun, nothing left to do

// #687 Quick-Dry Resin hop: a real short jump (MoveJump, ~0.3 s in the air), about once a second while the aura is up
constexpr float SNT_RESIN_HOP_DIST = 2.0f;
constexpr uint32 SNT_RESIN_HOP_INTERVAL = 1000;

// tuning
constexpr float SNT_SCAN = 30.0f;
constexpr float SNT_BLADE_RUSH_DISC = 9.5f;   // 175000 within 4 yd at the sword, 277500+ within 8 yd at the end
constexpr float SNT_BLADE_RUSH_LANE = 3.5f;
// the sword stalker is a 15 s summon (boss_general_pavalak.cpp:233) and the charge is over after ~6 s, so only a
// stalker whose summon timer still has more than this left counts as a live Blade Rush
constexpr uint32 SNT_BLADE_RUSH_MIN_TIMER = 8500;
constexpr float SNT_PUDDLE_BASE_RADIUS = 4.0f;    // spell_sap_residue: 4 yd x the puddle's object scale
constexpr uint32 SNT_SOAK_MIN_GROW = 3;           // a puddle smaller than this is not worth the Residue
constexpr uint32 SNT_RESIDUE_MAX = 3;             // own Sap Residue stacks at which a bot leaves the puddle
constexpr float SNT_SOAK_MIN_HEALTH = 60.0f;
// #656: farthest the tank steps from the puddle centre to leave it (Jin'bak evades beyond 30 yd from it,
// boss_vizier_jinbak.cpp:272; he stands a melee reach beyond his tank)
constexpr float SNT_JINBAK_TANK_MAX_DIST = 24.0f;
// #656 w51: melee DPS take a Sap Globule only within this distance of the puddle centre (the NW / N / E Resin Flake packs stand
// 46-57 yd from it, aggro ~20 yd); farther ones are the ranged DPS's (ComputeFocusTarget)
constexpr float SNT_GLOBULE_MELEE_LEASH = 22.0f;
// The bridge: after a flight (70% / 40%) he lands at the other end, bridgePos[0] (1889.0, 5177.2) <-> bridgePos[1]
// (1808.1, 5251.8) in boss_wing_leader_neronok.cpp:20-24 - 110 yd apart, ~120 yd from a bot standing behind the
// take-off point. #656: 100 yd (the old chase range, also AiPlayerbot.SightDistance used by AttackersValue.cpp:81/109)
// lost him across the bridge.
constexpr float SNT_NERONOK_CHASE_RANGE = 140.0f;
constexpr float SNT_NERONOK_REGROUP_DIST = 40.0f;  // farther than this from the landed Ner'onok: walk the bridge
constexpr float SNT_NERONOK_APPROACH_DIST = 20.0f; // ... to here; the generic reach / attack actions take over
constexpr float SNT_NERONOK_TANK_LANDING_DIST = 3.0f;  // #656 w50: the tank waits this close to his landing point
// #656 w48 healer leash (MopMspShared.h MSP_HEALER_LEASH_*): Ner'onok's victim farther than this (2D) or out of LOS
constexpr float SNT_HEALER_LEASH_MAX = 30.0f;
constexpr float SNT_HEALER_LEASH_DIST = 12.0f;  // ... walk back to this far from it (or half the distance)
// #656 w50: Ner'onok's bridge ends (boss_wing_leader_neronok.cpp:20-24 bridgePos, flight altitude) and the deck z (DB 131.25).
// The 70% flight lands at bridgePos[1], the 40% flight at bridgePos[0] (:131 MovePoint(bridgePos[phase]), :241 ++phase/--phase).
constexpr float SNT_BRIDGE_END0_X = 1888.998f;
constexpr float SNT_BRIDGE_END0_Y = 5177.15f;
constexpr float SNT_BRIDGE_END1_X = 1808.063f;
constexpr float SNT_BRIDGE_END1_Y = 5251.847f;
// #656 w50: the escape keeps walking to its own point while the hazard still names (about) the same spot
constexpr float SNT_ESCAPE_SAME_SPOT = 3.0f;
// #656 w50: a Siege Explosive (61452, summoned by 119377 for 20000 ms, 5.4.8 SpellEffect/SpellMisc.dbc) arms 5 s after its
// summon (npc_pavalak_siege_explosive: 1000 + 1000 + 1000 + 2000 ms, boss_general_pavalak.cpp:384-406) and loses its spellclick
constexpr uint32 SNT_BOMB_SUMMON_MS = 20000;
constexpr uint32 SNT_BOMB_ARM_MS = 5000;
constexpr uint32 SNT_BOMB_CLICK_MARGIN_MS = 750;  // the click itself + one AI tick

enum class HazardKind : uint8
{
    None = 0,
    Blades,
    Dash,
    BladeRush,
    Bomb,
    Pitch,
    Puddle,
    HealerLeash
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
    Neronok = 0
};

// The most urgent hazard for this bot right now (kind None when there is none).
bool ComputeHazard(Player* bot, Hazard& out);

// DPS bots at Jin'bak: the Sap Globule nearest to the bot (they crawl to the pool and make the detonation worse).
ObjectGuid ComputeFocusTarget(Player* bot);

// A hostile caster of an interrupt-worthy spell of this group within `range` yd of the bot, or nullptr.
Unit* FindInterruptTarget(Player* bot, InterruptGroup group, float range = 40.0f);

// Does the bot know at least one interrupt spell (kick, pummel, wind shear, mind freeze, counterspell, ...).
bool HasInterruptSpell(PlayerbotAI* botAI);

// Casts the first available interrupt on `target`. True when a spell was started.
bool CastInterrupt(PlayerbotAI* botAI, Unit* target);

// Pa'valak is behind his Bulwark (65% / 35%): General Pa'valak, or nullptr.
Creature* GetBulwarkedPavalak(Player* bot);

// A Siege Explosive that is not armed yet and can still be picked up (spellclick), within 45 yd of the bot.
Creature* FindClickableBomb(Player* bot);

// #781 add wave: the tank picks up the Sik'thik Soldiers Pa'valak's Reinforcements send in (not the ones already on him).
inline MopDungeon::WaveSpec ReinforcementsWave()
{
    return {SNT_NPC_PAVALAK, {SNT_NPC_SIKTHIK_SOLDIER}, MopDungeon::WaveRole::Tank, 60.0f};
}

// #656: Ner'onok has landed (not REACT_PASSIVE: boss_wing_leader_neronok.cpp:132 sets it for the flight, :254-256
// lifts it on EVENT_RE_ENGAGE) more than SNT_NERONOK_REGROUP_DIST from the bot, else nullptr. He has no combat
// movement (SetCombatMovement(false), :74), so the group has to cross the bridge to him (Wowpedia "Wing Leader
// Ner'onok": the first person that reaches him interrupts).
Creature* FindNeronokAcrossBridge(Player* bot);

// #656 w50: where the bot has to walk to be at Ner'onok when he can be fought again: his position once he has landed
// (as FindNeronokAcrossBridge), or the bridge end he is flying to while he is in the air (Wowpedia "Wing Leader
// Ner'onok", MediaWiki API, fetched 2026-10-14: "Chase Ner'onok once he lifts off and flies to the other side of the
// bridge"). False when he is not in the fight, when he has landed within SNT_NERONOK_REGROUP_DIST of the bot, or when
// he is in the air and the bot already stands where the approach stops (tank SNT_NERONOK_TANK_LANDING_DIST, others
// SNT_NERONOK_APPROACH_DIST from that end).
bool NeronokRegroupPoint(Player* bot, Creature*& neronok, float& x, float& y, bool& flying);

// #656 w50: milliseconds left of `caster`'s current cast / channel of `spellId` (0 when it is not casting it).
uint32 CastTimeLeftMs(Unit* caster, uint32 spellId);

// #656 w50: can `bot` (interrupt range `range`) get in range of `caster` before its cast of `spellId` ends, at run speed.
bool CanReachBeforeCastEnds(Player* bot, Unit* caster, uint32 spellId, float range);

// #656 w54: a Mantid Tar Keg that can be clicked now (on its Barrel Target), within 40 yd, else nullptr.
Creature* FindClickableKeg(Player* bot);

// #656 w54: where a carried keg goes - a Vo'jak / Sik'thik Warrior / Swarmer / Demolisher within the throw range with no
// group member near it (a Demolisher: none within its Unstable Blast), the bigger targets first; nullptr when none.
Unit* FindTarTarget(Player* bot);

// #656 w50: milliseconds this Siege Explosive can still be clicked before it arms (SNT_BOMB_ARM_MS when unknown).
uint32 BombClickableLeftMs(Creature* bomb);

// Jin'bak's Sap Puddle while Jin'bak is engaged within 70 yd, else nullptr.
Creature* FindSapPuddle(Player* bot);

// Current damage radius of the puddle (spell_sap_residue: 4 yd x object scale).
float SapPuddleRadius(Creature* puddle);

// Grow stacks of the puddle / Sap Residue stacks on a player.
uint32 SapGrowStacks(Creature* puddle);
uint32 SapResidueStacks(Player* bot);

// A DPS bot that should walk into the puddle now (see the #781 note above); `puddle` is set when true.
bool WantsToSoakPuddle(Player* bot, Creature*& puddle);
}

#endif
