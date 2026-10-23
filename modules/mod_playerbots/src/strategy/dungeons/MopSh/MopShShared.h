/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" - Scarlet Halls (map 1001, level 90 revamp). Epic #373, issue #657.
//
// AC WotLK mod-playerbots has no strategy for this dungeon, so it is authored on AC's dungeon-strategy PATTERN
// (AC Ai/Dungeon/HoS, UB, MgT: Strategy / Triggers / Actions / Multipliers / ActionContext / TriggerContext).
//
// Sources for the intended mechanics (fetched 2026-10-06):
//   Wowpedia MediaWiki API (wowpedia.fandom.com/api.php?action=parse&page=<title>): Houndmaster_Braun,
//     Armsmaster_Harlan, Flameweaver_Koegler - "Spells and abilities" / "Strategy" sections.
//   Wowhead MoP tooltip API (nether.wowhead.com/mop-classic/tooltip/spell/<id>) for the spell texts below.
// Ids, radii and interruptibility are checked against THIS core: the fork's boss scripts
// (src/server/scripts/EasternKingdoms/ScarletHalls/*.cpp) and the 5.4.8 Spell/SpellEffect/SpellInterrupts DBCs.
//
// Edges (1-4 per boss; only mechanics that the fork's scripts really cast):
//   Houndmaster Braun    - Piercing Throw (114004): leave the knife line (Bloody Mess bleed hits everyone in the path)
//   Armsmaster Harlan    - Blades of Light (111216): flee the spinning boss; the vortex pulls in and kills
//   Flameweaver Koegler  - interrupt Pyroblast / Book Burner / Greater Dragon's Breath / Fireball Volley,
//                          leave burning book cases (15 yd, 195k per 2 s), get behind him during the breath
//
// Script gaps (mechanics the fork does NOT implement, so no edge exists for them): see the issue comment.

// Issue #781 additions (2026-10-11):
//   Braun  - Call Dog at 90/80/70/60% (boss_houndmaster_braun.cpp:177-183): an Obedient Hound (59309) turns hostile
//            (it is NON_ATTACKABLE until ACTION_ACTIVATE_DOG, :258-300). Wowpedia: "Quickly take out the dog then go
//            back to Braun". DPS kill it, the tank picks it up (MopDungeon add wave, AC AK Nadox pattern).
//   Harlan - Call for Help every 20-25 s summons two Scarlet Defenders (58998, boss_armsmaster_harlan.cpp:236-241).
//            Wowpedia: Heavy Armor - 90% less damage from area effects until 600 (heroic 60000) damage breaks it, so
//            the DPS single-target them (AoE vetoed while a defender lives) and the tank picks them up.
//          - Blades of Light: the old flee kept 16 yd from Harlan's CURRENT position only, but he runs a fixed route
//            (BladeStormPoints, scarlet_halls.h:43-60) at run speed and pulls everyone within 8 yd (111394), so a
//            bot clear of him could stand on the route ahead. The flee now scores spots against his position AND the
//            route (MopDungeon::FindBestSpot). #657 w43: the whole route, not the next three segments - it covers the
//            whole room, so "clear of the next three" bots were carried off seconds later (see BladesSafety).

#ifndef PLAYERBOTS_MOPSHSHARED_H
#define PLAYERBOTS_MOPSHSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"
#include <string>

class Creature;
class Player;
class PlayerbotAI;
class Unit;

namespace MopScarletHalls
{
constexpr uint32 MAP_SCARLET_HALLS = 1001;

// Creature entries: scarlet_halls.h:99-112 (fork), identical in the MoP test world DB.
constexpr uint32 NPC_HOUNDMASTER_BRAUN    = 59303;
constexpr uint32 NPC_ARMSMASTER_HARLAN    = 58632;
constexpr uint32 NPC_FLAMEWEAVER_KOEGLER  = 59150;
constexpr uint32 NPC_BOOK_CASE            = 59155;
constexpr uint32 NPC_OBEDIENT_HOUND       = 59309;   // scarlet_halls.h:102 (NPC_OBEDIEND_HOUND)
constexpr uint32 NPC_SCARLET_DEFENDER     = 58998;   // scarlet_halls.h:107

// Spell ids: boss_houndmaster_braun.cpp:24, boss_armsmaster_harlan.cpp:30-32, boss_flameweaver_koegler.cpp:24-30.
// A heroic cast keeps the same id with a per-difficulty SpellInfo (SpellMgr.cpp:471-476).
constexpr uint32 SPELL_PIERCING_THROW          = 114004;  // 2 s channel, 100 yd, random enemy
constexpr uint32 SPELL_BLADES_OF_LIGHT         = 111216;  // 6 s cast, then the vortex aura on Harlan (script removes it at EVENT_BACK)
constexpr uint32 SPELL_BLADES_OF_LIGHT_GRAB    = 112955;  // vehicle-grab aura on a player Harlan carried away
constexpr uint32 SPELL_BOOK_BURNER             = 113364;  // 3 s cast, interruptible (SpellInterrupts flags 0x9)
constexpr uint32 SPELL_BURNING_BOOKS           = 113616;  // aura on a lit Book Case, 30 s, periodic 113620
constexpr uint32 SPELL_GREATER_DRAGONS_BREATH  = 113641;  // 2 s cast + 10 s channel, 50 yd cone, channel interruptible (0x1008)
constexpr uint32 SPELL_PYROBLAST               = 113690;  // 3 s cast, interruptible
constexpr uint32 SPELL_FIREBALL_VOLLEY         = 113691;  // 2.5 s cast, 35 yd, interruptible

// Piercing Throw: Bloody Mess is applied to everyone within 3 yd of the line boss->target
// (spell_bloody_mess, boss_houndmaster_braun.cpp:387: IsInBetween(caster, target, 3.0f)).
constexpr float KNIFE_LINE_HALF_WIDTH = 3.0f;
constexpr float KNIFE_LINE_CLEARANCE  = 2.0f;

// Blades of Light: 50% weapon damage + pull within 8 yd (SpellEffect 111394, radius 8) while Harlan runs the room
// at run speed along BladeStormPoints (scarlet_halls.h:43-60), so keep a wide berth.
constexpr float BLADES_SAFE_DISTANCE = 16.0f;
constexpr float BLADES_PATH_CLEAR    = 11.0f;   // 8 yd pull radius + 3 yd: distance to keep from his route

// #657 (w50): escape spots in the entrance corridor west of the room, the only ground clear of the whole route (navmesh
// 1001_*: corridor floor x 1156-1177 at y 440-446; (1170,444) is 18.5 yd from the nearest route segment, (1164,444)
// 24.5 yd, so BladesSafety >= 1.15 wherever Harlan is). Wowpedia "Armsmaster Harlan" (MediaWiki API, 2026-10-14): tank
// him upstairs by the entrance and keep out of his path while he spins. A bot down in the pit has no clear spot within
// FindBestSpot's 19 yd (w50: 12/12 candidates beyond the south wall), so it walks the path-found way out to these
// (AC MgTEscapeAction pattern, AC MgTActions.cpp:19-33: fixed escape spots tried in order).
struct BladesEscapeSpot
{
    float x;
    float y;
    float z;
};
constexpr BladesEscapeSpot BLADES_ESCAPE_SPOTS[] = {{1170.0f, 444.0f, 12.6f}, {1164.0f, 444.0f, 12.7f}};

// Burning Books radius 15 yd normal / 20 yd heroic (113620 SpellEffect, "Burning Books ... within 15 yards").
constexpr float BURNING_BOOKS_RADIUS         = 15.0f;
constexpr float BURNING_BOOKS_RADIUS_HEROIC  = 20.0f;
constexpr float BURNING_BOOKS_MARGIN         = 1.5f;
constexpr float BURNING_BOOKS_SCAN_RANGE     = 40.0f;

// Greater Dragon's Breath: 50 yd cone in front of Koegler (113653 SpellEffect). Orient the bot behind him; only bots
// close enough to make it in time react (the breath channel is also interrupted, see InterruptPriority).
constexpr float BREATH_BEHIND_DISTANCE = 4.0f;
constexpr float BREATH_REACT_RANGE     = 30.0f;
constexpr float BREATH_FRONT_HALF_ANGLE = 1.05f;  // 60 deg: the 90 deg cone (Spell.cpp:1142) plus a margin

constexpr float BOSS_SCAN_RANGE = 100.0f;

// Alive boss/NPC by entry within BOSS_SCAN_RANGE. "find target" needs the unit on the bot's threat list, which Harlan
// leaves during Blades of Light (ResetAllThreat, boss_armsmaster_harlan.cpp:216), so entry lookups are used instead.
Creature* FindBoss(Player* bot, uint32 entry);

// Interrupt priority of a cast on Koegler: 1 = most urgent, 0 = not on the list.
uint8 InterruptPriority(uint32 spellId);
uint8 CurrentCastPriority(Unit* caster);

// Cast the bot's interrupt (or hard control) on target; returns the spell name used, empty when none was possible.
std::string CastInterrupt(PlayerbotAI* botAI, Unit* target);

// Harlan is winding up / running Blades of Light and this bot is not already carried by it.
bool BladesOfLightActive(Player* bot);

// #781/#657: how safe (x, y) is from Blades of Light: min(distance to Harlan / BLADES_SAFE_DISTANCE, distance to any
// segment of his route / BLADES_PATH_CLEAR). 1 or more is safe (in practice: the entrance corridor west of the room).
float BladesSafety(Creature* harlan, float x, float y);

// #781 add waves: Braun's Obedient Hounds and Harlan's Scarlet Defenders.
inline MopDungeon::WaveSpec HoundWave(MopDungeon::WaveRole role)
{
    return {NPC_HOUNDMASTER_BRAUN, {NPC_OBEDIENT_HOUND}, role, 60.0f};
}

inline MopDungeon::WaveSpec DefenderWave(MopDungeon::WaveRole role)
{
    return {NPC_ARMSMASTER_HARLAN, {NPC_SCARLET_DEFENDER}, role, 60.0f};
}

// A lit Book Case closer than its (heroic-aware) radius plus margin; clearDistance = distance the bot must reach.
Creature* FindBurningBookcase(Player* bot, float& clearDistance);

// #683 Book Burner (113364, 3 s cast): "Book Burner needs to be intercepted by the tank, so only one person has to take the
// damage ... You will see a red beam on the floor towards where he's going to shoot. Just stand on the beam to intercept his
// fireball" (Wowpedia MediaWiki API "Flameweaver Koegler", fetched 2026-10-22). The core (boss_flameweaver_koegler.cpp, spell
// script spell_koegler_book_burner) shields every unit with a living player within BOOK_BURNER_BEAM_HALF_WIDTH of the line
// Koegler -> unit, and lights only the aimed Book Case when nobody intercepts.
constexpr float BOOK_BURNER_BEAM_HALF_WIDTH = 2.0f;   // = the core's IsInBetween width
// #683 w106: the tank charged a Scarlet Scholar 20-30 yd from Koegler and never reached the beam inside the 3 s cast (2 of 2 bolts
// lit their case). The bot nearest the beam now takes it ("Players may intercept the fiery bolt", Wowpedia), the tank first when
// it is within BOOK_BURNER_TANK_SLACK of the nearest; each walks to its own nearest point of the beam, not a fixed spot.
constexpr float BOOK_BURNER_MIN_FROM_BOSS   = 2.5f;   // the beam point is at least this far from Koegler (not inside his model)
constexpr float BOOK_BURNER_MIN_FROM_CASE   = 2.0f;   // ... and short of the case (IsInBetween wants dist(bot, boss) < dist(boss, case))
constexpr float BOOK_BURNER_TANK_SLACK      = 3.0f;   // yd the tank may be farther from the beam than the nearest bot and still go
constexpr float BOOK_BURNER_ELECT_RANGE     = 60.0f;  // the core's interceptor scan (GetPlayerListInGrid 60 yd)

// Koegler is casting Book Burner at a Book Case; `boss` and `bookcase` are set when true.
bool BookBurnerAimed(Player* bot, Unit*& boss, Unit*& bookcase);
// #683 w106: the nearest point (x, y) of the Book Burner beam to `who`, and the distance to it.
float BookBurnerBeamPoint(Unit* who, Unit* boss, Unit* bookcase, float& x, float& y);
// #683 w106: who covers this bolt - a living group member already on the beam (a real player counts), else the elected bot (the
// nearest to the beam; the tank when within BOOK_BURNER_TANK_SLACK of it). Every bot elects the same member.
Player* BookBurnerInterceptor(Player* bot, Unit* boss, Unit* bookcase);

// Koegler is channelling Greater Dragon's Breath and the bot stands in the front arc within BREATH_REACT_RANGE.
bool InDragonBreathCone(Player* bot, Unit*& boss);
}

#endif
