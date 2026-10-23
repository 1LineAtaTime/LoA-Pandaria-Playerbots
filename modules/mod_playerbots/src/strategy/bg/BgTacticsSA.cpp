/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Strand of the Ancients (map 607) play beyond AC mod-playerbots - issue #717 (epic #205), following the Isle of
// Conquest pattern (BgTacticsIC.cpp, #675, build 14).
//
// AC parity is the queue/join path only (AC BattleGroundJoinAction.cpp:477/:847, RandomPlayerbotMgr.cpp:1244):
// AC has no SotA case in BGTactics, AC AiFactory.cpp:689-690 keeps "battleground" off SotA, and AC
// Bot/PlayerbotAI.cpp:4181 says the siege shot is "implemented for IoC (extend for SotA if needed)". Everything
// here therefore extends AC's own pattern, the way BgTacticsIC.cpp does for the IoC parts AC leaves out:
// SelectObjective sets the "bg objective" (and, for a demolisher, the "bg siege") position the way AC
// selectObjective does, BGTactics walks/drives there, and AtObjective acts once the bot is there, the way AC atFlag
// does. The vehicle weapons fire through the existing AC path: StrandStrategy's "in vehicle" triggers run "hurl
// boulder" / "rocket blast" (CastVehicleSpellAction -> PlayerbotAI::CanCastVehicleSpell/CastVehicleSpell, #581),
// and PlayerbotAI's IsSiegeShot already recognises the SotA Hurl Boulder (below), so PlayerbotAI.cpp is unchanged.
//
// Mechanics and ids (sources, 2026-10-06):
//   - warcraft.wiki.gg "Strand of the Ancients" (MediaWiki API wikitext): two rounds of up to 10 minutes, sides
//     swap; attackers start on two boats, defenders at the South Graveyard; gates fall in walls (Green/Blue, then
//     Red/Purple, then Yellow, then the Chamber of Ancient Relics); "ALWAYS DRIVE THEM TO THE GATE. NEVER USE A
//     TANK TO ATTACK PLAYERS"; "capture the Eastern and Western Graveyards as soon as possible" (more
//     demolishers); graveyards cannot be retaken by defenders; defenders "make use of the turrets", "take out
//     demolishers", "defend your east and west GY's"; seaforium charges are attacker-only, 2 s to place, explode
//     after 10 s; defenders can defuse them.
//   - Wowhead tooltip API (nether.wowhead.com/tooltip/spell, dataEnv 15): 52338 Hurl Boulder (unlimited range,
//     instant, 2 s cd, 1000 siege damage within 10 yd), 49872 Rocket Blast (10-80 yd, 2 s cd), 52410 Place
//     Seaforium Charge (2 s cast, detonates after 10 s), 52415 Carrying Seaforium.
//   - MoP DBC (testserver data-mop-extract): 52338 = TRIGGER_MISSILE at DEST_TRAJ (89) -> 52339 with
//     GAMEOBJECT_DAMAGE (87), the same shape as the IoC Demolisher's 67440 -> 67439, so IsSiegeShot (PlayerbotAI.cpp,
//     AC PR #2856) already treats it as a gate shot; 49872 -> 51673 is SCHOOL_DAMAGE only (a gunner never siege
//     shots). 52410 = SUMMON_OBJECT 190752 at DEST_CASTER_FRONT + remove 52415; 52408 Seaforium Blast
//     GAMEOBJECT_DAMAGE 4500 in 10 yd. 52415 = OVERRIDE_SPELLS (the carried charge is an aura, not an item).
//     Vehicle 158 (Demolisher) seat 2021 CAN_CONTROL|CAN_CAST, not FIXED_POSITION; vehicle 453 (Antipersonnel
//     Cannon) is FIXED_POSITION with a CAN_CONTROL|CAN_CAST seat 4906 (so, as for the IoC keep cannon, a gunner is
//     "not the driver" to PlayerbotAI::IsInVehicle(true)). Lock 1479 (banners) opens with 21651
//     SPELL_CAPTURE_BANNER (lock type 17), the same lock as the IoC banners AC captures with it.
//   - Core: BattlegroundSA.{h,cpp} - gates slots 0-5 (Green 0, Yellow 1, Blue 2, Red 3, Purple 4, Ancient 5),
//     relic 6, flags 25-27, bomb piles 28-95; cannons 0-9 in gate pairs, demolishers 15-22 (beach 15-18 are
//     NOT_SELECTABLE until the warm-up ends, workshop 19-22 spawn on the east/west GY capture); Attackers is public;
//     PostUpdateImpl teleports attackers still on a boat or in the water to the beach at 59 s of each warm-up;
//     ResetObjs (round swap) re-creates every object and sets STATUS_WAIT_JOIN; EventPlayerClickedOnFlag (west/east
//     need Green OR Blue down, south needs a first- AND a second-wall gate); EventPlayerUsedGO (relic: Ancient gate
//     destroyed, used by an attacker). world DB: npc_spellclick_spells 28781 -> 60683, 27894 -> 60682;
//     creature_template 28781 spell1 52338 (no Ram on this core), 27894 spell1 49872; gameobject_template 190753
//     SPELLCASTER 52415 (1 charge, mount allowed).
//
// Objective state comes from the battleground's own objects - the same values the client's world states show:
//   gate world state 3620/3623/3617/3614/3638/3849 (1 intact, 2 damaged, 3 destroyed) == the gate GO's destructible
//   state (EventPlayerDamagedGO/DestroyGate set both from it); the graveyard world states 3632-3637 == the entry of
//   the flag GO in the slot (CaptureGraveyard re-adds it as 191306/191308/191310 for the Alliance, one less for the
//   Horde); BG_SA_ALLY_ATTACKS/HORDE_ATTACKS == BattlegroundSA::Attackers. The core keeps no readable world-state
//   store (Battleground::UpdateWorldState only sends packets), so this is the read-back.
//
// Roles: the "bg role" value (0-9, pinned to i % 10 by the 1:1 harness).
//   attackers: 0-3 demolisher drivers, 4-6 seaforium carriers, 7-8 graveyard cappers, 9 escort (and the fallback
//              for every role: escort a driven demolisher, else push the next gate);
//   defenders: 0-3 Antipersonnel Cannon gunners (front gates), 4 west-GY guard, 5 east-GY guard, 6-9 demolisher
//              hunters, then gate holders.
// Every attacker walks to the relic once the Chamber gate is down; every defender falls back to it.
// #737 (owner 2026-10-08): lanes - attackers split west (Green, Purple) / east (Blue, Red) by a role rank, 5:5, 7:3 to
// the lane whose first gate is down, and a bot switches to that breach when it is nearer (AttackerLane); drivers
// prefer their lane's demolishers and take the workshop pair too, cappers without a banner and the escort re-board a
// free demolisher within 60 yd; defenders split by role parity, climb the wall stairs to the cannon platform and jump
// down instead of walking through a closed gate, and come back through the gate's Defender's Portal (DefenderDetour;
// the portal works on this core through spell_scripts 54640 -> 54643, see SA_SPELL_DEFENDER_PORTAL_LOCK).
// #736 (owner 2026-10-08): on-foot objectives at a gate stand in front of it on the bot's own side (GateSide), never
// at the gate centre. Evidence: lane, detour, wall-jump, portal, portal-tp / portal-stay.
// Owner report 2026-10-08 (build 21), cited against warcraft.wiki.gg "Strand of the Ancients" (MediaWiki API,
// revid 6865830, 2026-09-08):
//   #750 preparation posts. "your team is free to move on the map during the initial countdown ... The defense
//        absolutely must use the time to get in to position"; "Right at the start of the game, you want to have people
//        on the turrets for both the blue and the green gates" (the page sends the rest to the beach; the owner wants
//        them at the front gates). StartSpot now does the wall jump itself during WAIT_JOIN (before, the jump waited
//        for STATUS_IN_PROGRESS in AtObjective, so every defender stood on the Purple/Red platform - "stops at the
//        inner gates" - until the match began), the gunners 0-3 each take their own front cannon (PreferredCannon:
//        role 0/2 Green gun 1/2, role 1/3 Blue gun 5/6) and board it during the warm-up, the rest hold behind their
//        lane's front gate, spread along it. Evidence: prep-post post=turret|gate (post=none at round start).
//   #751 defenders go for the demolishers: "Demolishers also take priority to kill over players, unless they were
//        hindering your ability to attack the demolisher"; "always attack demolishers as soon as possible, before they
//        are in range of a gate"; "try to take out demolishers unless a rusher gets your attention". The old hand-off
//        set "current target", which no trigger reads; DefenderEngage hands the demolisher over the way the game does
//        ("prioritized targets" -> attackers -> dps target, BotScenario.cpp) and attacks it as AttackAction::Attack
//        does - unless a player is attacking the bot: then that player first, then back to the demolisher.
//        Evidence: def-target.
//   #753 a destroyed gate is dropped as the siege target at once: DriveDemolisher (every tick, before the "in vehicle"
//        Hurl Boulder) moves "bg siege" off a gate that is down (or no longer the lane's) to the next one and drives on.
//        Before, "bg siege" stayed on the dead gate until the random "often" check objective re-ran SelectAttacker, and
//        every shot turned the demolisher back to face it (PlayerbotAI::CastVehicleSpell SetFacingTo) - fire, a few
//        steps, fire. Evidence: siege-retarget.
//   #752 never past an intact gate: the navmesh has no gates, so a driver sent on to Yellow after the other lane broke
//        through drove through the closed Purple gate. EffectiveGate makes the first intact gate on the mover's path
//        the target (siege, charge, hold) and drives to a point in front of it. Evidence: drive-blocked.
//   #736 boats: the core no longer teleports riders to the docks; BoatStep keeps an attacker aboard until its boat is
//        docked, then takes it off onto the dock (RemovePassenger + jump). Evidence: board-boat, boat, disembark.
//   #754 the SotA capture fights an enemy player near the banner first (AC atFlag); a gunner leaving its cannon (or
//        the round swap) drops the vehicle target the cannon aim left behind. Evidence: clear-target.
// Not done (core gap): defusing a planted charge - lock 1819 opens with spell 1843, but Spell::EffectOpenLock only
// despawns a trap for 1842 ("idk how it must works, hack for Massive Seaforium Charge"), so a defuse would do
// nothing on this core. Seaforium "on attacking vehicles" is not in the design (attacker-only, building damage).
//
// Evidence lines (TC_LOG_DEBUG "playerbots", MoP-only - AC has none of this): "[SOTA] <event> ..." with event
// boat, round, round-swap, board, drive, siege-park, gate-hp, gate-down, gy-owned, capture, bomb-pickup,
// bomb-plant, cannon-target, leave-cannon, leave-demolisher, relic, result, prep-post, def-target, siege-retarget,
// drive-blocked, board-boat, disembark, clear-target, drive-roll, vehicle-hit, demolisher-dead, ram-target.
// #765 a driven demolisher rams (Ram 60206) the enemy in front of it: RamTarget picks the unit the Ram cone will hit,
//      CastVehicleSpellAction ("ram") casts on it instead of "current target", which a driver never has. Evidence:
//      ram-target.
// #736 (owner 2026-10-14): non-gunner defenders hold a beach post per lane while its front gate stands (BeachPost,
// prep-post post=beach) and hunt over the beach (HuntRange); a demolisher gets a bot driver at
// AiPlayerbot.BgSotaVehicleDriveChance (DriveAllowed - beyond AC, see there).

#include "BattleGroundTactics.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundSA.h"
#include "BgTacticsSA.h"
#include "CellImpl.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Config.h"
#include "Creature.h"
#include "GameObject.h"
#include "Map.h"
#include "MotionMaster.h"
#include "MoveSplineInit.h"
#include "PathGenerator.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "Random.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Transport.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <atomic>
#include <cmath>
#include <cstring>
#include <map>
#include <mutex>
#include <utility>
#include <vector>

namespace
{
    std::atomic<bool> g_sotaEnabled{true};
    std::atomic<int32> g_sotaDriveChanceOverride{-1};  // #736(c): `.botscenario - sotadrive <pct>`; -1 = the config key

    constexpr uint32 SA_SPELL_CARRYING_SEAFORIUM = 52415;  // the carried charge (aura, OVERRIDE_SPELLS)
    constexpr uint32 SA_SPELL_PLACE_SEAFORIUM = 52410;     // 2 s cast, summons 190752 in front of the caster

    constexpr uint32 SA_ROLE_DRIVER_LAST = 3;    // attackers 0-3: demolisher drivers
    constexpr uint32 SA_ROLE_BOMB_FIRST = 4;     // attackers 4-6: seaforium carriers
    constexpr uint32 SA_ROLE_BOMB_LAST = 6;
    constexpr uint32 SA_ROLE_GY_FIRST = 7;       // attackers 7-8: graveyard cappers
    constexpr uint32 SA_ROLE_GY_LAST = 8;
    constexpr uint32 SA_ROLE_CANNON_LAST = 3;    // defenders 0-3: cannon gunners
    constexpr uint32 SA_ROLE_GUARD_WEST = 4;     // defenders 4/5: graveyard guards
    constexpr uint32 SA_ROLE_GUARD_EAST = 5;

    constexpr float SA_OFFSHORE_X = 1700.0f;     // the boats lie at x 2574-2683; the beach ends at x ~1620
    constexpr float SA_SIEGE_RANGE = 35.0f;      // a demolisher parks this close to its gate and fires
    // #765: Ram (60206, 5.4.8 DBC) knocks back and damages the enemies in a 12 yd cone in front of the demolisher
    // (effects 0/1 TARGET_UNIT_CONE_ENEMY_104, SpellRadius 32 = 12 yd; range 15 yd, SpellRange 11). A creature caster's
    // cone is M_PI / 2 wide when the spell has no SpellTargetRestrictions row (Spell::SelectImplicitConeTargets).
    constexpr float SA_RAM_REACH = 12.0f;
    constexpr float SA_RAM_CONE = static_cast<float>(M_PI) / 2.0f;
    // #736 (owner 2026-10-08): a carrier walks to SA_PLANT_STAND in front of the gate on its own side (GateSide), and
    // BGTactics::moveToObjective stops within 4 yd of an objective (BattleGroundTactics.cpp moveToObjective), so it
    // plants from up to SA_PLANT_RANGE. 52410 summons the charge 3 yd in front (SpellEffect radius index 15) and the
    // 52408 blast (10 yd, radius index 13) is measured to the gate's model bounds (Spell.cpp
    // WorldObjectSpellAreaTargetCheck -> GameObject::IsAtInteractDistance), so 10 yd still reaches the gate.
    constexpr float SA_PLANT_STAND = 6.0f;
    constexpr float SA_PLANT_RANGE = SA_PLANT_STAND + 4.0f;
    constexpr float SA_GATE_HOLD = 8.0f;         // escorts / gate holders stand this far in front of the gate
    constexpr float SA_NEAR_FLAG = 40.0f;        // any attacker this close to a capturable banner takes it
    constexpr float SA_HUNT_RANGE = 120.0f;      // enemy demolishers this close to a front gate are hunted
    constexpr float SA_CANNON_MIN = 10.0f;       // Rocket Blast 10-80 yd (Wowhead); 75 keeps a margin
    constexpr float SA_CANNON_MAX = 75.0f;
    constexpr float SA_CANNON_REACH = INTERACTION_DISTANCE + 2.0f;  // cannons stand on the wall-top platforms
    constexpr float SA_RELIC_REACH = 5.0f;       // 2D, the relic hangs 18 yd above the chamber floor

    // #737: lanes, second-wave demolishers, the defenders' wall routes.
    constexpr float SA_REBOARD_RANGE = 60.0f;        // a non-driver attacker takes a free demolisher this close
    constexpr float SA_OTHER_LANE_PENALTY = 150.0f;  // a demolisher/cannon on the other lane counts this much farther
    constexpr float SA_LANE_SPLIT_Y = -55.0f;        // west lane (Green, Purple) y > this; east (Blue, Red) below
    constexpr float SA_GATE_CROSS = 10.0f;           // a path crossing a gate's plane this close (2D) goes through it
    constexpr float SA_PLATFORM_REACH = 3.0f;        // 2D, on the wall platform: jump
    constexpr uint32 SA_DETOUR_RECHECK_MS = 4000;    // path test spacing per defender
    constexpr uint32 SA_PORTAL_SETTLE_MS = 4000;     // a portal use that leaves the bot in place this long = no teleport

    // #750/#751/#753 (owner 2026-10-08).
    constexpr float SA_PREP_POST_REACH = 5.0f;   // 2D, a gate holder this close to its spot is at its post
    constexpr float SA_HOLD_SPREAD = 3.0f;       // gate holders stand -3/0/+3 yd along the gate (not on one point)
    constexpr float SA_DEF_ENGAGE = 45.0f;       // a defender attacks an enemy demolisher this close (LOS, no gate between)
    constexpr float SA_DEF_THREAT = 40.0f;       // an enemy player this close targeting the bot is fought first
    constexpr float SA_SIEGE_GATE_MATCH = 20.0f; // 2D, the "bg siege" point belongs to the gate this close to it
    constexpr uint32 SA_DEF_ENGAGE_MS = 1000;    // DefenderEngage evaluation spacing per bot

    // #736 defenders on the beach (owner 2026-10-14). warcraft.wiki.gg "Strand of the Ancients" ===Defensive===: turret
    // crews at the Blue and Green gates, "the rest" on the beach split between the two landing docks, destroying the
    // demolishers as soon as possible. A first-wall gate hunts this far (Green/Blue -> the beach demolisher spawns
    // BattlegroundSA.h BG_SA_NpcSpawnlocs 15-18 are 205-211 yd off, the dock landings 201-205 yd); the inner gates keep
    // SA_HUNT_RANGE.
    constexpr float SA_FRONT_HUNT_RANGE = 260.0f;
    // Beach post per defender lane (0 west, 1 east), navmesh-checked 2026-10-14 (mmtile_probe.py --scan and dtsim.py
    // find_path over testserver data-mop-extract mmaps 0607_*): both on GROUND polys with ground +/-6 yd around (the
    // research spot west (1493,103) is a rock top at z 18, holes beside it - moved to (1496,94)); reached from the
    // Green / Blue wall-jump landings (93 / 75 yd corridor), from both docks and from all four beach demolisher spawns
    // with no gate plane crossed; the South Graveyard reaches them only over Purple+Green / Red+Blue (the wall routes).
    // 78-133 yd from the demolisher spawns of their dock.
    Position const SA_BEACH_POST[2] = {Position(1496.0f, 94.0f, 11.0f), Position(1503.0f, -189.0f, 6.4f)};

    // #752/#736 (core lane findings 2026-10-08).
    constexpr float SA_DRIVE_STAND = 25.0f;        // a demolisher drives to this far in front of its gate (own side)
    constexpr uint32 SA_BLOCK_RECHECK_MS = 3000;   // EffectiveGate path test spacing per bot
    constexpr float SA_BOAT_DOCKED_REACH = 5.0f;   // 2D, a boat this close to its dock spot is docked

    // #750 wave-20 fix.
    constexpr uint32 SA_MOUNT_WAIT_MS = 4000;      // a jump waits this long at most for a mount that would cut it
    constexpr float SA_LANDING_RESCUE = 15.0f;     // 2D, a bot stuck this close below a landing hops onto it
    constexpr float SA_SPLINE_HALF_SPAN = 250.0f;  // MONSTER_MOVE packs a linear path's points in 11-bit 0.25 yd offsets

    // Lane per bg role. Ranks interleave the role groups (drivers 0-3, carriers 4-6, cappers 7-8, escort 9) so
    // any west share keeps every group split: share 5 -> west roles 0,2,4,6,8; share 7 adds 9 and 7.
    uint8 const SA_LANE_RANK[10] = {0, 9, 1, 8, 2, 7, 3, 6, 4, 5};

    // Gates in BG_SA_Objects slot order. `ground` is the navmesh point at the gate (mmtile_probe.py on the
    // test-server map-607 mmaps, tol 4 yd; the Chamber gate GO sits 8 yd above its floor). side 0 = west (Green,
    // Purple), 1 = east (Blue, Red), -1 = the centre line. Guns are the BG_SA_GUN_* pair on that gate's wall
    // (BG_SA_NpcSpawnlocs order: Green, Yellow, Blue, Red, Purple).
    struct SaGate
    {
        uint32 slot;
        char const* name;
        Position ground;
        int8 side;
        uint32 gunA;
        uint32 gunB;
    };

    constexpr uint32 SA_NO_GUN = uint32(-1);

    SaGate const SA_GATES[6] = {
        {BG_SA_GREEN_GATE, "green", Position(1411.57f, 108.163f, 28.956f), 0, BG_SA_GUN_1, BG_SA_GUN_2},
        {BG_SA_YELLOW_GATE, "yellow", Position(1055.452f, -108.1f, 82.268f), -1, BG_SA_GUN_3, BG_SA_GUN_4},
        {BG_SA_BLUE_GATE, "blue", Position(1431.341f, -219.437f, 30.979f), 1, BG_SA_GUN_5, BG_SA_GUN_6},
        {BG_SA_RED_GATE, "red", Position(1227.667f, -212.555f, 55.340f), 1, BG_SA_GUN_7, BG_SA_GUN_8},
        {BG_SA_PURPLE_GATE, "purple", Position(1214.681f, 81.21f, 53.588f), 0, BG_SA_GUN_9, BG_SA_GUN_10},
        {BG_SA_ANCIENT_GATE, "ancient", Position(878.555f, -108.2f, 109.661f), -1, SA_NO_GUN, SA_NO_GUN}};

    Position const SA_RELIC_FLOOR(836.5f, -108.8f, 111.975f);  // under the Titan Relic (navmesh)

    // #736: how far from this gate an enemy demolisher is hunted (the first wall covers the beach)
    float HuntRange(SaGate const* gate)
    {
        return gate->slot == BG_SA_GREEN_GATE || gate->slot == BG_SA_BLUE_GATE ? SA_FRONT_HUNT_RANGE : SA_HUNT_RANGE;
    }

    // #737 defender routes past a closed gate (the mmaps carry no gate, so a navmesh path to the far side goes
    // straight through it - and the gate does not block a bot's spline either). Out: the stairs to the cannon
    // platform beside the gate, then a jump down in front of the cannon (seaward). In: the gate's Defender's Portal
    // (192819, BG_SA_ObjSpawnlocs "Portal" rows: each stands on the seaward side of its gate; AC
    // BattlegroundSA::DefendersPortalTeleport sends the user to SOTADefPortalDest, behind the gate; this core reaches
    // the same spots through 54640 -> 54643 -> the nearest 23472 trigger, see SA_SPELL_DEFENDER_PORTAL_LOCK).
    // Platform = the navmesh top of the stairs at the BG_SA_GUN_* spot, landing = the ground below it outside;
    // both from mmtile_probe.py/dtsim on testserver data-mop-extract map 607 (2026-10-08): the south graveyard
    // (1209.7,-65.2,70.1) reaches each platform without crossing a gate (265/259 yd purple/red), each landing
    // reaches its lane's front gate without crossing the wall it was dropped from, and the landing is NOT
    // connected back to its platform except through that gate (it is outside).
    struct SaWallRoute
    {
        uint32 gateSlot;
        Position platform;
        Position landing;
    };

    SaWallRoute const SA_WALL_ROUTES[4] = {
        {BG_SA_PURPLE_GATE, Position(1212.9f, 58.9f, 64.8f), Position(1224.0f, 53.5f, 51.13f)},     // gun 10
        {BG_SA_RED_GATE, Position(1230.3f, -188.4f, 67.0f), Position(1241.1f, -183.2f, 49.3f)},     // gun 7
        {BG_SA_GREEN_GATE, Position(1404.4f, 85.3f, 41.4f), Position(1413.0f, 76.0f, 25.7f)},       // gun 2
        {BG_SA_BLUE_GATE, Position(1420.9f, -198.0f, 42.4f), Position(1428.4f, -186.2f, 30.13f)}};  // gun 5

    SaWallRoute const* WallRouteOf(uint32 gateSlot)
    {
        for (SaWallRoute const& route : SA_WALL_ROUTES)
            if (route.gateSlot == gateSlot)
                return &route;

        return nullptr;
    }

    // The Defender's Portal in front of a gate (BG_SA_PORTAL_DEFFENDER_* slot order: blue, green, yellow, purple, red).
    uint32 PortalSlotOf(uint32 gateSlot)
    {
        switch (gateSlot)
        {
            case BG_SA_BLUE_GATE: return BG_SA_PORTAL_DEFFENDER_BLUE;
            case BG_SA_GREEN_GATE: return BG_SA_PORTAL_DEFFENDER_GREEN;
            case BG_SA_YELLOW_GATE: return BG_SA_PORTAL_DEFFENDER_YELLOW;
            case BG_SA_PURPLE_GATE: return BG_SA_PORTAL_DEFFENDER_PURPLE;
            case BG_SA_RED_GATE: return BG_SA_PORTAL_DEFFENDER_RED;
            default: return uint32(-1);
        }
    }

    // Graveyard banners: CaptureGraveyard re-adds the flag GO as the Alliance entry or one less for the Horde.
    struct SaFlag
    {
        uint32 slot;
        uint32 allianceEntry;
        char const* name;  // the core's "west/east/south graveyard" announcements
    };

    SaFlag const SA_FLAGS[3] = {{BG_SA_LEFT_FLAG, 191308, "west"},
                                {BG_SA_RIGHT_FLAG, 191306, "east"},
                                {BG_SA_CENTRAL_FLAG, 191310, "south"}};

    TeamId Other(TeamId team) { return team == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE; }
    uint32 FactionOf(TeamId team) { return BG_SA_Factions[team == TEAM_ALLIANCE ? 0 : 1]; }
    char const* TeamName(TeamId team) { return team == TEAM_ALLIANCE ? "alliance" : "horde"; }

    // Quiet slot reads (Battleground::GetBGCreature/GetBGObject log an ERROR per empty slot, and the workshop
    // demolishers and the bomb piles stay empty until their graveyard is taken) - as BgTacticsIC.cpp.
    Creature* SlotCreature(BattlegroundSA* bg, uint32 slot)
    {
        if (slot >= bg->BgCreatures.size() || bg->BgCreatures[slot].IsEmpty())
            return nullptr;

        BattlegroundMap* map = bg->FindBgMap();
        return map ? map->GetCreature(bg->BgCreatures[slot]) : nullptr;
    }

    GameObject* SlotObject(BattlegroundSA* bg, uint32 slot)
    {
        if (slot >= bg->BgObjects.size() || bg->BgObjects[slot].IsEmpty())
            return nullptr;

        BattlegroundMap* map = bg->FindBgMap();
        return map ? map->GetGameObject(bg->BgObjects[slot]) : nullptr;
    }

    // Gate world state 3 (destroyed). A missing GO (between ResetObjs' delete and re-add) counts as standing.
    // #753: building health 0 counts too (GameObject::ModifyHealth sets GO_DESTRUCTIBLE_DESTROYED from it).
    bool GateDown(BattlegroundSA* bg, uint32 slot)
    {
        GameObject* gate = SlotObject(bg, slot);
        return gate && (gate->GetDestructibleState() == GO_DESTRUCTIBLE_DESTROYED ||
                        (gate->GetGOValue()->Building.MaxHealth && !gate->GetGOValue()->Building.Health));
    }

    struct SaState
    {
        bool down[6];
        bool westThrough;  // Green and Purple down: the west lane reaches the courtyard
        bool eastThrough;  // Blue and Red down
        bool inner;        // either lane is through: the Yellow gate (then the Chamber) is next
        bool relic;        // the Chamber gate is down: the relic can be taken
    };

    SaState ReadState(BattlegroundSA* bg)
    {
        SaState s;
        for (uint32 i = 0; i < 6; ++i)
            s.down[i] = GateDown(bg, i);

        s.westThrough = s.down[BG_SA_GREEN_GATE] && s.down[BG_SA_PURPLE_GATE];
        s.eastThrough = s.down[BG_SA_BLUE_GATE] && s.down[BG_SA_RED_GATE];
        s.inner = s.westThrough || s.eastThrough;
        s.relic = s.down[BG_SA_ANCIENT_GATE];
        return s;
    }

    // The gates the attackers must break next (the defenders' front): one per lane while neither lane is through
    // (Green/Blue, then Purple/Red - a second-wall gate is only reachable through the first-wall gate of its own
    // lane), then Yellow, then the Chamber. Empty once the Chamber is down.
    std::vector<SaGate const*> NextGates(SaState const& s)
    {
        std::vector<SaGate const*> gates;
        if (s.inner)
        {
            if (!s.down[BG_SA_YELLOW_GATE])
                gates.push_back(&SA_GATES[BG_SA_YELLOW_GATE]);
            else if (!s.down[BG_SA_ANCIENT_GATE])
                gates.push_back(&SA_GATES[BG_SA_ANCIENT_GATE]);
            return gates;
        }

        gates.push_back(&SA_GATES[s.down[BG_SA_GREEN_GATE] ? BG_SA_PURPLE_GATE : BG_SA_GREEN_GATE]);
        gates.push_back(&SA_GATES[s.down[BG_SA_BLUE_GATE] ? BG_SA_RED_GATE : BG_SA_BLUE_GATE]);
        return gates;
    }

    SaGate const* NearestGate(std::vector<SaGate const*> const& gates, WorldObject const* from)
    {
        SaGate const* best = nullptr;
        float bestDist = 0.0f;
        for (SaGate const* gate : gates)
        {
            float const dist = from->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY());
            if (!best || dist < bestDist)
            {
                best = gate;
                bestDist = dist;
            }
        }

        return best;
    }

    // #737 lanes. 0 = west (Green, then Purple), 1 = east (Blue, then Red); -1 once a lane is through (Yellow, then
    // the Chamber, one gate for everyone).
    int8 LaneAt(float y) { return y > SA_LANE_SPLIT_Y ? 0 : 1; }

    SaGate const* LaneGate(SaState const& s, int8 lane)
    {
        if (s.inner || lane < 0)
        {
            std::vector<SaGate const*> const gates = NextGates(s);
            return gates.empty() ? nullptr : gates.front();
        }

        if (lane == 0)
            return &SA_GATES[s.down[BG_SA_GREEN_GATE] ? BG_SA_PURPLE_GATE : BG_SA_GREEN_GATE];

        return &SA_GATES[s.down[BG_SA_BLUE_GATE] ? BG_SA_RED_GATE : BG_SA_BLUE_GATE];
    }

    // Owner 2026-10-08: attackers spread over both lanes with weighted randomness, and prefer an already broken gate
    // when it is closer instead of breaking the opposite one. The draw is the bg role (random 0-9 for a live bot,
    // pinned i % 10 by the 1:1 harness - so deterministic there): 5 of 10 ranks go west while both first-wall gates
    // stand, 7 of 10 to the lane whose first gate is down; and a bot of the other lane switches when the next gate
    // behind that breach is nearer to it than its own lane's gate.
    int8 AttackerLane(WorldObject const* from, uint32 role, SaState const& s)
    {
        if (s.inner)
            return -1;

        bool const westOpen = s.down[BG_SA_GREEN_GATE];
        bool const eastOpen = s.down[BG_SA_BLUE_GATE];
        uint32 westShare = 5;
        if (westOpen && !eastOpen)
            westShare = 7;
        else if (eastOpen && !westOpen)
            westShare = 3;

        int8 lane = SA_LANE_RANK[role % 10] < westShare ? 0 : 1;
        if (westOpen != eastOpen)
        {
            int8 const open = westOpen ? 0 : 1;
            if (lane != open)
            {
                SaGate const* mine = LaneGate(s, lane);
                SaGate const* breach = LaneGate(s, open);
                if (from->GetExactDist2d(breach->ground.GetPositionX(), breach->ground.GetPositionY()) <
                    from->GetExactDist2d(mine->ground.GetPositionX(), mine->ground.GetPositionY()))
                    lane = open;
            }
        }

        return lane;
    }

    // Defenders: even roles hold the west lane, odd the east (gunners, hunters, gate holders alike).
    int8 DefenderLane(uint32 role, SaState const& s) { return s.inner ? -1 : int8(role % 2); }

    // #736 (owner 2026-10-14): the beach post of a defender lane while that lane's first-wall gate (Green / Blue) stands,
    // spread -SA_HOLD_SPREAD / 0 / +SA_HOLD_SPREAD along x by role (as the gate posts). False once the gate is down.
    bool BeachPost(SaState const& s, int8 lane, uint32 role, Position& post)
    {
        if (lane < 0 || lane > 1 || s.inner || s.down[lane == 0 ? BG_SA_GREEN_GATE : BG_SA_BLUE_GATE])
            return false;

        float const lateral = (float((role / 2) % 3) - 1.0f) * SA_HOLD_SPREAD;
        post.Relocate(SA_BEACH_POST[lane].GetPositionX() + lateral, SA_BEACH_POST[lane].GetPositionY(),
                      SA_BEACH_POST[lane].GetPositionZ());
        return true;
    }

    // Where a straight path crosses an intact gate: the first segment whose ends lie on opposite sides of the gate's
    // plane (the gate GO faces the sea: positive = seaward) with the crossing point within SA_GATE_CROSS of the gate.
    // `outside` = the side the bot starts on.
    struct SaCrossing
    {
        SaGate const* gate = nullptr;
        bool outside = false;
    };

    // One straight segment a -> b against every intact gate's plane (FirstClosedCrossing's test, #737; #751 uses it
    // for the straight line bot -> demolisher).
    SaCrossing SegmentClosedCrossing(BattlegroundSA* bg, SaState const& s, G3D::Vector3 const& a, G3D::Vector3 const& b)
    {
        SaCrossing result;
        for (SaGate const& info : SA_GATES)
        {
            if (s.down[info.slot])
                continue;

            GameObject* gate = SlotObject(bg, info.slot);
            if (!gate)
                continue;

            float const fx = std::cos(gate->GetOrientation());
            float const fy = std::sin(gate->GetOrientation());
            float const da = (a.x - gate->GetPositionX()) * fx + (a.y - gate->GetPositionY()) * fy;
            float const db = (b.x - gate->GetPositionX()) * fx + (b.y - gate->GetPositionY()) * fy;
            if (da * db >= 0.0f)
                continue;

            float const t = da / (da - db);
            float const px = a.x + (b.x - a.x) * t;
            float const py = a.y + (b.y - a.y) * t;
            float const pz = a.z + (b.z - a.z) * t;
            if (gate->GetExactDist2d(px, py) > SA_GATE_CROSS || std::fabs(pz - info.ground.GetPositionZ()) > 15.0f)
                continue;

            result.gate = &info;
            result.outside = da > 0.0f;
            return result;
        }

        return result;
    }

    // #752: `mover` = whatever moves (the bot, or the demolisher it drives - the path a driven vehicle follows).
    SaCrossing FirstClosedCrossing(WorldObject const* mover, BattlegroundSA* bg, SaState const& s, Position const& dest)
    {
        SaCrossing result;
        PathGenerator path(mover);
        path.CalculatePath(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), false);
        if (path.GetPathType() & PATHFIND_NOPATH)
            return result;

        Movement::PointsArray const& points = path.GetPath();
        for (size_t i = 1; i < points.size(); ++i)
        {
            result = SegmentClosedCrossing(bg, s, points[i - 1], points[i]);
            if (result.gate)
                return result;
        }

        return result;
    }

    // #736 (owner 2026-10-08): SaGate::ground is the gate's centre, and the navmesh has no gate in it (gates are
    // destructible GOs, only in the dynamic vmap tree), so a path to the centre - or a jitter around it - runs into or
    // through a closed gate. An on-foot bot stands `offset` yards in front of the gate on the side it is on instead:
    // along the gate's facing (the GO orientation, BG_SA_ObjSpawnlocs[slot][3] before the GO exists; it points out to
    // sea), the same plane FirstClosedCrossing tests. Being on the bot's own side, the objective never crosses that
    // gate; any OTHER closed gate on the way is DefenderDetour's job (wall stairs out, the Defender's Portal in).
    // #750: `lateral` moves the spot along the gate (perpendicular to its facing), so the holders of one gate do not
    // all stand on one point.
    Position GateSide(BattlegroundSA* bg, SaGate const* gate, WorldObject const* from, float offset, float lateral = 0.0f)
    {
        GameObject* go = SlotObject(bg, gate->slot);
        float const o = go ? go->GetOrientation() : BG_SA_ObjSpawnlocs[gate->slot][3];
        float const nx = std::cos(o);
        float const ny = std::sin(o);
        float const dx = from->GetPositionX() - gate->ground.GetPositionX();
        float const dy = from->GetPositionY() - gate->ground.GetPositionY();
        float const side = dx * nx + dy * ny >= 0.0f ? offset : -offset;
        return Position(gate->ground.GetPositionX() + side * nx - lateral * ny,
                        gate->ground.GetPositionY() + side * ny + lateral * nx, gate->ground.GetPositionZ());
    }

    // #753: the gate a "bg siege" point was set for (SelectAttacker sets it to the gate GO's position), or nullptr.
    SaGate const* GateAt(BattlegroundSA* bg, float x, float y)
    {
        for (SaGate const& info : SA_GATES)
        {
            GameObject* go = SlotObject(bg, info.slot);
            float const dist = go ? go->GetExactDist2d(x, y)
                                  : info.ground.GetExactDist2d(x, y);
            if (dist <= SA_SIEGE_GATE_MATCH)
                return &info;
        }

        return nullptr;
    }

    TeamId FlagOwner(BattlegroundSA* bg, SaFlag const& flag)
    {
        GameObject* go = SlotObject(bg, flag.slot);
        if (!go)
            return TEAM_NEUTRAL;

        if (go->GetEntry() == flag.allianceEntry)
            return TEAM_ALLIANCE;

        return go->GetEntry() == flag.allianceEntry - 1 ? TEAM_HORDE : TEAM_NEUTRAL;
    }

    // EventPlayerClickedOnFlag's gate test, narrowed to what the attackers can walk to: the west banner behind
    // Green, the east banner behind Blue, the south banner once a lane is through.
    bool FlagOpen(SaState const& s, SaFlag const& flag)
    {
        if (flag.slot == BG_SA_LEFT_FLAG)
            return s.down[BG_SA_GREEN_GATE];
        if (flag.slot == BG_SA_RIGHT_FLAG)
            return s.down[BG_SA_BLUE_GATE];
        return s.inner;
    }

    // Nearest banner the attackers can take now (still the defenders', its gates down).
    SaFlag const* CapturableFlag(Player* bot, BattlegroundSA* bg, SaState const& s, float maxDist)
    {
        SaFlag const* best = nullptr;
        float bestDist = maxDist;
        for (SaFlag const& flag : SA_FLAGS)
        {
            TeamId const owner = FlagOwner(bg, flag);
            if (owner == TEAM_NEUTRAL || owner == bg->Attackers || !FlagOpen(s, flag))
                continue;

            GameObject* go = SlotObject(bg, flag.slot);
            if (!go || !go->isSpawned())
                continue;

            float const dist = bot->GetExactDist(go);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = &flag;
            }
        }

        return best;
    }

    std::mutex g_sotaLogMutex;
    std::map<std::pair<uint32, uint32>, uint32> g_sotaLogTimes;  // (bg instance, key) -> getMSTime()

    // true once per everyMs (0 = once per instance) for one (battleground instance, key) - as BgTacticsIC.cpp.
    bool LogDue(uint32 instanceId, uint32 key, uint32 everyMs)
    {
        std::lock_guard<std::mutex> lock(g_sotaLogMutex);
        uint32 const now = getMSTime();
        auto itr = g_sotaLogTimes.find(std::make_pair(instanceId, key));
        if (itr == g_sotaLogTimes.end())
        {
            g_sotaLogTimes[std::make_pair(instanceId, key)] = now;
            return true;
        }

        if (!everyMs || getMSTimeDiff(itr->second, now) < everyMs)
            return false;

        itr->second = now;
        return true;
    }

    // #750 (wave 20): with MapUpdate.Threads > 1 a battleground map is updated by whichever worker thread takes it,
    // so per-bot state kept thread_local was per WORKER: each "once" line came once per thread (120 prep-post lines
    // for 20 defender-rounds), a wall jump dropped only that thread's detour cache, a priority set on one thread was
    // never cleared on another, a disembark was redone on the next thread. All per-bot state of this file is shared,
    // guarded by this one mutex (recursive: the helpers nest). Never held across a path calculation or a transport
    // call.
    std::recursive_mutex g_sotaState;
    using SaLock = std::lock_guard<std::recursive_mutex>;

    // Per-bot retry spacing - as BgTacticsIC.cpp, but shared across map threads (#750).
    bool BotDue(Player* bot, uint32 key, uint32 everyMs)
    {
        static std::map<std::pair<ObjectGuid, uint32>, uint32> s_last;
        SaLock lock(g_sotaState);
        uint32 const now = getMSTime();
        auto itr = s_last.find(std::make_pair(bot->GetGUID(), key));
        if (itr != s_last.end() && getMSTimeDiff(itr->second, now) < everyMs)
            return false;

        s_last[std::make_pair(bot->GetGUID(), key)] = now;
        return true;
    }

    // Where each driver boarded its demolisher (the drive-distance evidence). Guarded by g_sotaState.
    std::map<ObjectGuid, Position> s_boardPos;

    void ForgetBoardPos(Player* bot)
    {
        SaLock lock(g_sotaState);
        s_boardPos.erase(bot->GetGUID());
    }

    void SetPos(PositionMap& posMap, char const* key, Position const& p, uint32 mapId)
    {
        PositionInfo info = posMap[key];
        info.Set(p.GetPositionX(), p.GetPositionY(), p.GetPositionZ(), mapId);
        posMap[key] = info;
    }

    void ResetPos(AiObjectContext* context, char const* key)
    {
        PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
        PositionInfo info = posMap[key];
        info.Reset();
        posMap[key] = info;
    }

    Position Jitter(Position const& p, float range)
    {
        return Position(p.GetPositionX() + frand(-range, range), p.GetPositionY() + frand(-range, range),
                        p.GetPositionZ());
    }

    void Dismount(Player* bot)
    {
        // as AC's atFlag before a capture
        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }

    bool UsableVehicle(Creature* vehicle, uint32 faction)
    {
        return vehicle && vehicle->IsAlive() && vehicle->GetFaction() == faction && vehicle->GetVehicleKit() &&
               !vehicle->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
    }

    Player* DriverOf(Creature* vehicle)
    {
        Unit* driver = vehicle->GetVehicleKit() ? vehicle->GetVehicleKit()->GetPassenger(0) : nullptr;
        return driver && driver->GetTypeId() == TYPEID_PLAYER ? driver->ToPlayer() : nullptr;
    }

    bool DriveAllowed(BattlegroundSA* bg, Creature* vehicle);  // #736(c), defined with the vehicle-life state below

    // Nearest of our demolishers (beach or workshop - the workshop pair spawns when the east/west graveyard falls)
    // with a free driver seat, within maxDist. #737: one on the other lane counts SA_OTHER_LANE_PENALTY farther
    // (lane -1: no preference), so the drivers of both lanes each take their own side's demolishers.
    // #736(c): only a demolisher whose drive roll allows a bot driver (DriveAllowed).
    Creature* FreeDemolisher(Player* bot, BattlegroundSA* bg, TeamId team, float maxDist, int8 lane = -1)
    {
        Creature* best = nullptr;
        float bestScore = 0.0f;
        for (uint32 slot = BG_SA_DEMOLISHER_1; slot <= BG_SA_DEMOLISHER_8; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!UsableVehicle(vehicle, FactionOf(team)) || !vehicle->GetVehicleKit()->HasEmptySeat(0))
                continue;

            float const dist = bot->GetExactDist(vehicle);
            if (dist >= maxDist || !DriveAllowed(bg, vehicle))
                continue;

            float const score = dist + (lane >= 0 && LaneAt(vehicle->GetPositionY()) != lane ? SA_OTHER_LANE_PENALTY : 0.0f);
            if (!best || score < bestScore)
            {
                bestScore = score;
                best = vehicle;
            }
        }

        return best;
    }

    // Nearest demolisher of `team` with a player driving it; `nearGates` limits it to those within HuntRange of
    // one of those gates (the defenders' hunt; #736: 260 yd for the first wall, the beach).
    // #737: `lane` >= 0 keeps only the demolishers on that lane (escorts and hunters stay on their own side).
    Creature* DrivenDemolisher(Player* bot, BattlegroundSA* bg, TeamId team, float maxDist,
                               std::vector<SaGate const*> const* nearGates, int8 lane = -1)
    {
        Creature* best = nullptr;
        float bestDist = maxDist;
        for (uint32 slot = BG_SA_DEMOLISHER_1; slot <= BG_SA_DEMOLISHER_8; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!UsableVehicle(vehicle, FactionOf(team)) || !DriverOf(vehicle))
                continue;

            if (lane >= 0 && LaneAt(vehicle->GetPositionY()) != lane)
                continue;

            if (nearGates)
            {
                bool close = false;
                for (SaGate const* gate : *nearGates)
                    if (vehicle->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY()) < HuntRange(gate))
                        close = true;

                if (!close)
                    continue;
            }

            float const dist = bot->GetExactDist(vehicle);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = vehicle;
            }
        }

        return best;
    }

    // Nearest free Antipersonnel Cannon on the wall of a front gate (one the attackers still have to break).
    // #737: `lane` >= 0 prefers the cannons of that lane's gate (SA_OTHER_LANE_PENALTY), so the gunners split.
    // #750 w21: `laneOnly` skips the other lane's cannons altogether (warm-up only, see PreferredCannon).
    Creature* FreeCannon(Player* bot, BattlegroundSA* bg, TeamId team, SaState const& s, float maxDist, int8 lane = -1,
                         bool laneOnly = false)
    {
        Creature* best = nullptr;
        float bestDist = maxDist + SA_OTHER_LANE_PENALTY;
        for (SaGate const* gate : NextGates(s))
        {
            if (laneOnly && lane >= 0 && gate->side >= 0 && gate->side != lane)
                continue;

            for (uint32 slot : {gate->gunA, gate->gunB})
            {
                if (slot == SA_NO_GUN)
                    continue;

                Creature* cannon = SlotCreature(bg, slot);
                if (!UsableVehicle(cannon, FactionOf(team)) || cannon->GetVehicleKit()->IsVehicleInUse() ||
                    !cannon->GetVehicleKit()->GetAvailableSeatCount())
                    continue;

                float const dist = bot->GetExactDist(cannon);
                if (dist >= maxDist)
                    continue;

                float const score = dist + (lane >= 0 && gate->side >= 0 && gate->side != lane ? SA_OTHER_LANE_PENALTY : 0.0f);
                if (score < bestDist)
                {
                    bestDist = score;
                    best = cannon;
                }
            }
        }

        return best;
    }

    // The gate whose wall carries this cannon, or nullptr.
    SaGate const* GateOfCannon(BattlegroundSA* bg, Unit const* cannon)
    {
        for (SaGate const& gate : SA_GATES)
            for (uint32 slot : {gate.gunA, gate.gunB})
                if (slot != SA_NO_GUN && slot < bg->BgCreatures.size() && bg->BgCreatures[slot] == cannon->GetGUID())
                    return &gate;

        return nullptr;
    }

    // #750 (owner 2026-10-08): the four gunners man the four front cannons, one each - role 0/2 the west gate's gun
    // A/B (Green: BG_SA_GUN_1/2), role 1/3 the east gate's (Blue: BG_SA_GUN_5/6); on the Yellow wall role parity picks
    // gun 3/4. Deterministic (the bg role is pinned by the 1:1 harness), so two gunners never race for one cannon.
    // Taken or gone -> the nearest free front cannon of the lane, as before (FreeCannon).
    // #750 w21 (`laneOnly`, the warm-up): live bots draw their bg role at random, so a match has more gunner roles than
    // front cannons; the extra gunners fell back to the OTHER lane's free cannon and crossed the courtyard during the
    // warm-up (Arander/Aston/Adrio, role 3 at the blue wall, sent to a green cannon: 283-317 yd left at round start).
    // During the warm-up a gunner without a free cannon of its own lane holds its lane's gate post instead.
    Creature* PreferredCannon(Player* bot, BattlegroundSA* bg, TeamId team, SaState const& s, uint32 role, float maxDist,
                              bool laneOnly = false)
    {
        int8 const lane = DefenderLane(role, s);
        SaGate const* gate = LaneGate(s, lane);
        if (gate && gate->gunA != SA_NO_GUN)
        {
            uint32 const pick = lane >= 0 ? (role / 2) % 2 : role % 2;
            Creature* cannon = SlotCreature(bg, pick == 0 ? gate->gunA : gate->gunB);
            if (UsableVehicle(cannon, FactionOf(team)) && !cannon->GetVehicleKit()->IsVehicleInUse() &&
                cannon->GetVehicleKit()->GetAvailableSeatCount() && bot->GetExactDist(cannon) < maxDist)
                return cannon;
        }

        return FreeCannon(bot, bg, team, s, maxDist, lane, laneOnly);
    }

    GameObject* NearestBombPile(Player* bot, BattlegroundSA* bg, float maxDist)
    {
        GameObject* best = nullptr;
        float bestDist = maxDist;
        for (uint32 slot = BG_SA_BOMB; slot < BG_SA_MAXOBJ; ++slot)
        {
            GameObject* go = SlotObject(bg, slot);
            if (!go || !go->isSpawned())
                continue;

            float const dist = bot->GetExactDist(go);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = go;
            }
        }

        return best;
    }

    bool Offshore(Player* bot) { return bot->GetPositionX() > SA_OFFSHORE_X; }

    bool CarryingCharge(Player* bot) { return bot->HasAura(SA_SPELL_CARRYING_SEAFORIUM); }

    // #736 boats (core lane 2026-10-08): the core no longer teleports boat riders to the docks at 59 s / 53 s
    // (BattlegroundSA::RescueStrandedAttackers only rescues a non-rider out at sea). The boats carry the attackers to
    // the dock and stay there; a rider must walk off itself. It cannot just MoveTo: Transport::UpdatePassengerPositions
    // puts a passenger back at its deck offset every update, the deck is not on the navmesh, and PlayerbotAI's 1 s
    // deck probe re-seats an unattached bot standing on a deck (SnapToOwnDeckOffset). So, as BgTacticsIC's
    // DropFromGunship leaves the gunship: RemovePassenger, clear IGNORE_PATHFINDING (MoP keeps it for an MO transport
    // rider, PlayerbotAI.cpp #141 F1), and a jump onto the dock (PlayerbotAI never attaches a bot mid-jump, #675).
    //   Docked spot of each boat (slot BG_SA_BOAT_ONE = east, BG_SA_BOAT_TWO = west): spawn (BG_SA_ObjSpawnlocs) +
    //   the TransportAnimation.dbc offset at the dock frame (t 60067: 193182/193184 (1077.10,-744.35),
    //   193183/193185 (954.11,949.83); testserver data-mop-extract) turned by the parent rotation ResetObjs sets
    //   ({0,0,1,~0} = 180 deg about z, Transport::Update) -> (1602.6,-82.5) / (1619.9,31.4). Landing = the core's
    //   own dock points (RescueStrandedAttackers / TeleportAttackerToShip after launch), ~23 yd from those spots.
    Position const SA_BOAT_DOCKED[2] = {Position(1602.6f, -82.5f, 0.0f), Position(1619.9f, 31.4f, 0.0f)};
    Position const SA_DOCK_LANDING[2] = {Position(1598.571f, -106.671f, 8.873f), Position(1607.596f, 49.766f, 7.5795f)};

    // 0 / 1 = the SotA boat (BG_SA_BOAT_ONE / BG_SA_BOAT_TWO) `transport` is, -1 = none of them.
    int32 SotaBoatIndex(BattlegroundSA* bg, GameObject const* transport)
    {
        if (!transport)
            return -1;

        for (uint32 i = 0; i < 2; ++i)
        {
            uint32 const slot = BG_SA_BOAT_ONE + i;
            if (slot < bg->BgObjects.size() && !bg->BgObjects[slot].IsEmpty() && bg->BgObjects[slot] == transport->GetGUID())
                return int32(i);
        }

        return -1;
    }

    // At its dock spot, or (should that spot be off) standing still >= 3 s on the beach side of the open sea - the
    // boats only ever stop at their ocean spot (x > 2500) or at the dock.
    bool BoatDocked(GameObject* boat, int32 index)
    {
        if (boat->GetExactDist2d(SA_BOAT_DOCKED[index].GetPositionX(), SA_BOAT_DOCKED[index].GetPositionY()) <=
            SA_BOAT_DOCKED_REACH)
            return true;

        static std::map<ObjectGuid, std::pair<Position, uint32>> s_still;  // boat -> (pos, still since)
        SaLock lock(g_sotaState);
        uint32 const now = getMSTime();
        Position const pos(boat->GetPositionX(), boat->GetPositionY(), boat->GetPositionZ());
        auto itr = s_still.find(boat->GetGUID());
        if (itr == s_still.end() || itr->second.first.GetExactDist2d(&pos) > 0.3f)
        {
            s_still[boat->GetGUID()] = std::make_pair(pos, now);
            return false;
        }

        return pos.GetPositionX() < 1800.0f && getMSTimeDiff(itr->second.second, now) >= 3000;
    }

    // true once per bot per round after its disembark (a re-attach to the docked hull is then undone, not ridden)
    // #736 (t736 wave 20): one round of one match. Battleground instance ids are reused by later matches, so
    // (instance, attackers) alone matched the PREVIOUS match: a rider marked "disembarked" there counted as done the
    // moment it was put on the boat in the next one (Eranord "disembark ... t=2175 again=1" at the ocean spot; Jesto,
    // Vievelia, Cateritney, Estevin, Erentian the same in round 2). `since` = when the battleground began
    // (getMSTime() - Battleground::GetElapsedTime(), steady through both rounds), so a later match never matches.
    struct SaRound
    {
        uint32 instance = 0;
        TeamId attackers = TEAM_NEUTRAL;
        uint32 since = 0;
    };

    SaRound RoundOf(BattlegroundSA* bg)
    {
        SaRound round;
        round.instance = bg->GetInstanceID();
        round.attackers = bg->Attackers;
        round.since = getMSTime() - bg->GetElapsedTime();
        return round;
    }

    bool SameRound(SaRound const& a, SaRound const& b)
    {
        return a.instance == b.instance && a.attackers == b.attackers &&
               std::min(a.since - b.since, b.since - a.since) < 10000;  // uint32 wrap-safe; tick jitter is ms
    }

    std::map<ObjectGuid, SaRound> s_disembarked;  // guarded by g_sotaState

    bool DisembarkedThisRound(Player* bot, BattlegroundSA* bg)
    {
        SaLock lock(g_sotaState);
        auto itr = s_disembarked.find(bot->GetGUID());
        return itr != s_disembarked.end() && SameRound(itr->second, RoundOf(bg));
    }

    // #736(c) - BEYOND AC (AC has no SotA tactics at all, see the file head; owner 2026-10-14: "each vehicle at 70% of
    // getting a bot driving it, so sometimes one is free for a player"). One deterministic roll per (battleground
    // instance, round, demolisher guid, demolisher life): a demolisher keeps its guid through
    // BattlegroundSA::UpdateDemolisherSpawns' Relocate + Respawn (BattlegroundSA.cpp:1181+, 30 s after its death), so the
    // life count goes up when ObserveDemolishers sees it dead and the roll is dropped then. Chance =
    // AiPlayerbot.BgSotaVehicleDriveChance (default 70), or the harness override (`.botscenario - sotadrive <pct>`).
    // The same per-life state carries the evidence: the first hit on it (vehicle-hit) and its death (demolisher-dead).
    struct SaVehLife
    {
        SaRound round;
        uint32 life = 0;      // deaths seen this round
        bool dead = false;
        bool rolled = false;
        bool allowed = true;
        bool hit = false;     // vehicle-hit logged for this life
        bool driven = false;  // had a driver this life
    };

    std::map<ObjectGuid, SaVehLife> s_vehLife;  // guarded by g_sotaState

    // The vehicle's state for this round (a new round - or a later match on a reused instance id - starts it over).
    SaVehLife& VehLifeOf(Creature* vehicle, SaRound const& round)
    {
        SaVehLife& state = s_vehLife[vehicle->GetGUID()];
        if (!SameRound(state.round, round))
        {
            state = SaVehLife();
            state.round = round;
        }

        return state;
    }

    uint32 DriveChance()
    {
        int32 const forced = g_sotaDriveChanceOverride.load();
        int32 const chance = forced >= 0 ? forced : sConfigMgr->GetIntDefault("AiPlayerbot.BgSotaVehicleDriveChance", 70);
        return uint32(std::max(0, std::min(100, chance)));
    }

    bool DriveAllowed(BattlegroundSA* bg, Creature* vehicle)
    {
        SaRound const round = RoundOf(bg);
        uint32 roll = 0;
        uint32 chance = 0;
        uint32 life = 0;
        bool allowed = true;
        {
            SaLock lock(g_sotaState);
            SaVehLife& state = VehLifeOf(vehicle, round);
            if (state.rolled)
                return state.allowed;

            uint32 h = round.instance * 2654435761u;
            h ^= (uint32(round.attackers) + 1u) * 40503u;
            h ^= uint32(vehicle->GetGUID().GetCounter()) * 2246822519u;
            h ^= (state.life + 1u) * 3266489917u;
            h ^= h >> 15;
            h *= 2246822519u;
            h ^= h >> 13;
            roll = h % 100u;
            chance = DriveChance();
            allowed = roll < chance;
            life = state.life;
            state.rolled = true;
            state.allowed = allowed;
        }

        TC_LOG_DEBUG("playerbots", "[SOTA] drive-roll vguid=%u life=%u roll=%u chance=%u bots=%u at=%.1f,%.1f,%.1f t=%u",
                     uint32(vehicle->GetGUID().GetCounter()), life, roll, chance, uint32(allowed), vehicle->GetPositionX(),
                     vehicle->GetPositionY(), vehicle->GetPositionZ(), bg->GetElapsedTime());
        return allowed;
    }

    // Distance (2D) from `unit` to the nearest gate of the battleground (destroyed or not) and that gate.
    float NearestGateDist(Unit const* unit, SaGate const*& nearest)
    {
        float best = 0.0f;
        nearest = nullptr;
        for (SaGate const& gate : SA_GATES)
        {
            float const dist = unit->GetExactDist2d(gate.ground.GetPositionX(), gate.ground.GetPositionY());
            if (!nearest || dist < best)
            {
                best = dist;
                nearest = &gate;
            }
        }

        return best;
    }

    // #736 evidence (MoP-only): the first damage on each attacker demolisher life (vehicle-hit - the defenders' first
    // hit; the t575 row compares it with the first demolisher board) and each demolisher death with its distance from
    // the nearest gate (demolisher-dead - killed on the beach, not at a gate, is the defence the wiki asks for).
    void ObserveDemolishers(BattlegroundSA* bg)
    {
        SaRound const round = RoundOf(bg);
        for (uint32 slot = BG_SA_DEMOLISHER_1; slot <= BG_SA_DEMOLISHER_8; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!vehicle)
                continue;

            bool const alive = vehicle->IsAlive();
            Player* const driver = alive && vehicle->GetVehicleKit() ? DriverOf(vehicle) : nullptr;
            SaLock lock(g_sotaState);
            SaVehLife& state = VehLifeOf(vehicle, round);
            if (!alive)
            {
                if (state.dead)
                    continue;

                SaGate const* gate = nullptr;
                float const gateDist = NearestGateDist(vehicle, gate);
                TC_LOG_DEBUG("playerbots", "[SOTA] demolisher-dead vguid=%u life=%u driven=%u hit=%u gate=%s gatedist=%.1f at=%.1f,%.1f,%.1f t=%u",
                             uint32(vehicle->GetGUID().GetCounter()), state.life, uint32(state.driven),
                             uint32(state.hit), gate ? gate->name : "-", gateDist, vehicle->GetPositionX(),
                             vehicle->GetPositionY(), vehicle->GetPositionZ(), bg->GetElapsedTime());
                state.dead = true;
                ++state.life;  // the respawned demolisher rolls again (DriveAllowed)
                state.rolled = false;
                state.hit = false;
                state.driven = false;
                continue;
            }

            state.dead = false;
            if (driver)
                state.driven = true;

            if (!state.hit && vehicle->GetHealth() < vehicle->GetMaxHealth())
            {
                state.hit = true;
                SaGate const* gate = nullptr;
                float const gateDist = NearestGateDist(vehicle, gate);
                TC_LOG_DEBUG("playerbots", "[SOTA] vehicle-hit vguid=%u life=%u hp=%.0f driver=%s gate=%s gatedist=%.1f at=%.1f,%.1f,%.1f t=%u",
                             uint32(vehicle->GetGUID().GetCounter()), state.life, vehicle->GetHealthPct(),
                             driver ? driver->GetName().c_str() : "-", gate ? gate->name : "-", gateDist,
                             vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ(),
                             bg->GetElapsedTime());
            }
        }
    }

    // #736/#750 (wave 20): CheckMountStateAction mounts an unmounted bot out of combat, and its StopMoving cuts a jump
    // in the air (Adrio: disembark jump at t=60180, "mounting spell=35025" the next AI tick, still on the hull at
    // 62004 and re-attached by the deck probe; the wall jumps the same). So a jump goes once the bot is mounted, or
    // when no mount is coming during it: before its mount moment in the warm-up (CheckMountStateAction.cpp: a bot
    // mounts only in its own 1-30 s before the start, guid x instance hash - mirrored here), in combat (no mounting),
    // or after SA_MOUNT_WAIT_MS of waiting. Never dismount for a jump.
    bool MountWontCut(Player* bot, BattlegroundSA* bg)
    {
        if (bot->IsMounted() || bot->IsInCombat())
            return true;

        if (bg->GetStatus() == STATUS_WAIT_JOIN)
        {
            uint32 const h = (bot->GetGUID().GetCounter() * 2654435761u) ^ (bg->GetInstanceID() * 40503u);
            int32 const mountAtMs = 1000 + int32(h % 29001u);
            if (bg->GetStartDelayTime() > mountAtMs + int32(SA_MOUNT_WAIT_MS))
                return true;  // its mount moment is still ahead, after this jump
        }

        static std::map<ObjectGuid, uint32> s_waitSince;
        SaLock lock(g_sotaState);
        uint32 const now = getMSTime();
        auto itr = s_waitSince.find(bot->GetGUID());
        if (itr == s_waitSince.end() || getMSTimeDiff(itr->second, now) > 3 * SA_MOUNT_WAIT_MS)
        {
            s_waitSince[bot->GetGUID()] = now;  // a new wait
            return false;
        }

        return getMSTimeDiff(itr->second, now) >= SA_MOUNT_WAIT_MS;
    }

    // An attacker on a SotA boat: hold aboard while it sails; once it is docked, leave it onto the dock. Returns true
    // when the bot is (still) a boat passenger this tick - the caller then holds; `acted` = it left the boat now.
    // Also boards a bot that stands on a boat deck at sea but is not a passenger yet (the core drops attackers onto
    // the boats with slow fall, TeleportAttackerToShip; PlayerbotAI's deck probe runs once a second - the same probe,
    // done now, so the bot is a passenger before the boat sails off under it).
    bool BoatStep(Player* bot, BattlegroundSA* bg, char const* phase, bool& acted)
    {
        acted = false;
        Transport* transport = bot->GetTransport();
        int32 const boat = SotaBoatIndex(bg, transport);
        bool const done = DisembarkedThisRound(bot, bg);

        if (boat < 0)
        {
            if (transport || done || !Offshore(bot) || bot->IsInWater() || bot->GetVehicle() || !BotDue(bot, 51, 1000))
                return false;

            Transport* deck = bot->GetMap()->GetTransportForPos(bot->GetPhaseMask(), bot->GetPositionX(),
                                                                bot->GetPositionY(), bot->GetPositionZ(), bot);
            int32 const index = SotaBoatIndex(bg, deck);  // GUID match only: never trusts a non-transport GO cast
            if (index < 0)
                return false;

            // #736 (t736 wave 20): never attach a bot with a live spline. A bot dropped on the deck and not yet a
            // passenger stands 15 yd over the water, so BGTactics' #747 check gives it a MoveFall; attached mid-fall,
            // Unit::UpdateSplinePosition read the world-space fall spline as a deck offset and put the rider ~2750 yd
            // from its boat (Standen: board-boat at 2577.0,980.3,13.4, then "boat at=3185.0,3061.6", t736
            // max-dist-to-boat=2767 = |(2577,980)|). As PlayerbotAI's own deck check: stop, attach, then a zero-length
            // spline in the deck frame (StopOnCurrentPos, #141 F5 - MoveSplineInit uses the transport offset).
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            deck->AddPassenger(bot, true);
            {
                Movement::MoveSplineInit init(bot);
                init.MoveTo(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), false, true);
                init.SetFacing(bot->GetOrientation());
                init.Launch();
            }
            TC_LOG_DEBUG("playerbots", "[SOTA] board-boat bot=%s boat=%d at=%.1f,%.1f,%.1f t=%u", bot->GetName().c_str(),
                         index, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bg->GetElapsedTime());
            return true;
        }

        // #736 (t736 wave 20): only ever off a DOCKED boat - the "already disembarked" re-detach (again=1) skipped this
        // test and dropped riders in the open sea (fell=1/2, t736 aboard 69-77%). A boat waiting at its ocean spot
        // (x > 2500, the core's 0-60 s round-1 wait) is not docked: BoatDocked needs its dock spot, or a stop at x < 1800.
        if (!BoatDocked(transport, boat))
        {
            if (BotDue(bot, 36, 60000))
                TC_LOG_DEBUG("playerbots", "[SOTA] boat bot=%s at=%.1f,%.1f,%.1f transport=1 boat=%d", bot->GetName().c_str(),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), boat);
            return true;  // ride: stay aboard until the boat is docked
        }

        // #736 (wave 20): wait on the docked boat for a mount that would cut the jump; never dismount for it
        if (!MountWontCut(bot, bg) || !BotDue(bot, 50, 2000))
            return true;

        Position const from(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        transport->RemovePassenger(bot, true);
        bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        // also when re-attached after a cut jump (again=1): jump off again - with no jump the hull kept it (wave 20:
        // Boromar re-attached 4 times at 1602.0,-94.7)
        Position const land = Jitter(SA_DOCK_LANDING[boat], 1.5f);
        float const dist = bot->GetExactDist2d(land.GetPositionX(), land.GetPositionY());
        float const speedXY = std::max(bot->GetSpeed(MOVE_RUN), dist / 2.0f);  // at most ~2 s in the air
        bot->GetMotionMaster()->MoveJump(land.GetPositionX(), land.GetPositionY(), land.GetPositionZ(), speedXY, 6.0f);
        {
            SaLock lock(g_sotaState);
            s_disembarked[bot->GetGUID()] = RoundOf(bg);
        }

        acted = true;
        TC_LOG_DEBUG("playerbots", "[SOTA] disembark bot=%s boat=%d phase=%s at=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f dist=%.1f t=%u again=%u mounted=%u",
                     bot->GetName().c_str(), boat, phase, from.GetPositionX(), from.GetPositionY(), from.GetPositionZ(),
                     land.GetPositionX(), land.GetPositionY(), land.GetPositionZ(), dist, bg->GetElapsedTime(),
                     uint32(done), uint32(bot->IsMounted()));
        return false;
    }

    // #751: the enemy demolisher each defender handed to its AI through "prioritized targets" (map-thread local, as
    // BotDue). Only that entry is ever cleared again, so a priority someone else set (a harness, "attack my target")
    // is left alone.
    std::map<ObjectGuid, ObjectGuid> s_defVehicle;  // guarded by g_sotaState

    ObjectGuid DefVehicleOf(Player* bot)
    {
        SaLock lock(g_sotaState);
        auto itr = s_defVehicle.find(bot->GetGUID());
        return itr != s_defVehicle.end() ? itr->second : ObjectGuid::Empty;
    }

    void DropVehiclePriority(Player* bot, AiObjectContext* context)
    {
        SaLock lock(g_sotaState);
        auto itr = s_defVehicle.find(bot->GetGUID());
        if (itr == s_defVehicle.end())
            return;

        GuidVector const prio = context->GetValue<GuidVector>("prioritized targets")->Get();
        if (prio.size() == 1 && prio.front() == itr->second)
            context->GetValue<GuidVector>("prioritized targets")->Reset();

        s_defVehicle.erase(itr);
    }

    // #754(b): AimCannon aims the cannon by setting "current target" to an enemy demolisher (or a player up to 75 yd
    // off), and DefenderEngage prioritises a demolisher. Once the bot is off that cannon (gate down, relic) or the
    // round swaps, a vehicle target left there is stale - the bot on foot would chase a demolisher far away, or one
    // that is its own side's now. Clear it (and the attack on it) and the priority this file set.
    void ClearVehicleTarget(Player* bot, AiObjectContext* context, char const* why)
    {
        DropVehiclePriority(bot, context);
        Unit* target = context->GetValue<Unit*>("current target")->Get();
        if (!target || !target->IsVehicle())
            return;

        if (bot->GetVictim() == target)
            bot->AttackStop();

        context->GetValue<Unit*>("current target")->Set(nullptr);
        TC_LOG_DEBUG("playerbots", "[SOTA] clear-target bot=%s target=%u why=%s", bot->GetName().c_str(), target->GetEntry(),
                     why);
    }

    void LeaveVehicle(Player* bot, AiObjectContext* context, char const* what, char const* why)
    {
        if (!BotDue(bot, 30, 2000))
            return;

        WorldPacket packet;
        bot->GetSession()->HandleRequestVehicleExit(packet);
        ForgetBoardPos(bot);
        ClearVehicleTarget(bot, context, what);  // #754(b)
        TC_LOG_DEBUG("playerbots", "[SOTA] leave-%s bot=%s why=%s at=%.1f,%.1f,%.1f", what, bot->GetName().c_str(), why,
                     bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    }

    // AC EnterVehicleAction::EnterVehicle (AC VehicleActions.cpp:78-98): spellclick, then cancel the mount the bot
    // may have entered on. The siege position belongs to the vehicle it was set for (AC PR #2856) - as BgTacticsIC.cpp.
    bool Board(Player* bot, AiObjectContext* context, Creature* vehicle, char const* what)
    {
        // #737 MoP adapter: board standing still. The board runs from "check flag" while the walk to the vehicle is
        // still a running point move, and a rooted passenger's MotionMaster is never updated on this core
        // (MotionMaster.cpp:303), so that move would keep UNIT_STATE_ROAMING_MOVE for the whole ride and BGTactics'
        // IsStopped() gate would never let the driver drive (see BGTactics::Execute "move to objective").
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        vehicle->HandleSpellClick(bot, 0);
        if (!bot->IsOnVehicle(vehicle))
            return false;

        ResetPos(context, "bg siege");
        ResetPos(context, "bg objective");
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        {
            SaLock lock(g_sotaState);
            s_boardPos[bot->GetGUID()] = Position(vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ());
        }
        // #736: vguid (matches drive-roll / vehicle-hit / demolisher-dead) and t (battleground elapsed ms)
        Battleground* const bg = bot->GetBattleground();
        TC_LOG_DEBUG("playerbots", "[SOTA] board bot=%s what=%s vehicle=%u vguid=%u at=%.1f,%.1f,%.1f t=%u", bot->GetName().c_str(),
                     what, vehicle->GetEntry(), uint32(vehicle->GetGUID().GetCounter()), vehicle->GetPositionX(),
                     vehicle->GetPositionY(), vehicle->GetPositionZ(), bg ? bg->GetElapsedTime() : 0u);
        return true;
    }

    bool UseObject(Player* bot, GameObject* go, char const* what)
    {
        Dismount(bot);
        bot->StopMoving();
        // MoP adapter, as BGTactics::IsLockedInsideKeep: CMSG_GAMEOBJ_USE is bit-packed on this core, so call what
        // HandleGameObjectUseOpcode calls after parsing it (Handlers/SpellHandler.cpp).
        go->Use(bot);
        TC_LOG_DEBUG("playerbots", "[SOTA] %s bot=%s go=%u at=%.1f,%.1f,%.1f", what, bot->GetName().c_str(), go->GetEntry(),
                     go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        return true;
    }

    // AC atFlag's banner capture (BattleGroundTactics.cpp AV/AB/IC case): 21651 on the banner GO.
    bool Capture(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, GameObject* flag, char const* name)
    {
        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_CAPTURE_BANNER);
        if (!spellInfo || flag->GetGoState() != GO_STATE_READY || !bot->CanUseBattlegroundObject(flag))
            return false;

        // #754, AC atFlag AB/IC engage-before-capture (AC BattleGroundTactics.cpp:3706-3722, MoP :4411): an enemy
        // player within 2 x INTERACTION_DISTANCE (AC flagRange for AV/AB/IC, AC :3613) of the banner is fought first
        Unit* enemy = context->GetValue<Unit*>("enemy player target")->Get();
        if (enemy && enemy->IsAlive() && enemy->GetDistance(flag) < INTERACTION_DISTANCE * 2.0f)
        {
            context->GetValue<Unit*>("current target")->Set(enemy);
            return false;
        }

        // #754 (beyond AC, owner 2026-10-08): ... and so is whoever is hitting the bot (BGTactics::CaptureThreat, any live
        // attacker, or a PvP opponent within 50 yd - not an enemy merely nearby); the capture resumes once the threat is gone.
        if (Unit* threat = BGTactics::CaptureThreat(bot))
        {
            // #754 (owner 2026-10-08) keep the current target while it is alive and attacking the bot, else the threat;
            // engage it (#756: setting the value alone left a bot hit by a pet/guardian idle at the flag)
            Unit* const cur = context->GetValue<Unit*>("current target")->Get();
            Unit* const fight = (cur && cur->IsAlive() && cur->GetVictim() == bot) ? cur : threat;
            if (!BGTactics::EngageThreat(bot, fight))
                context->GetValue<Unit*>("current target")->Set(fight);
            return false;
        }

        if (!BotDue(bot, 31, 2000))
            return false;

        Dismount(bot);
        if (bot->IsInDisallowedMountForm())
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        bot->StopMoving();

        Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
        spell->m_targets.SetGOTarget(flag);
        spell->prepare(&spell->m_targets);
        botAI->WaitForSpellCast(spell);
        TC_LOG_DEBUG("playerbots", "[SOTA] capture bot=%s gy=%s go=%u", bot->GetName().c_str(), name, flag->GetEntry());
        return true;
    }

    // Place Seaforium Charge (the carried charge is the 52415 aura; 52410 is its override-bar spell): face the gate,
    // stand still, 2 s cast.
    bool PlantCharge(Player* bot, GameObject* gate, SaGate const& info)
    {
        if (!BotDue(bot, 32, 3000))
            return false;

        Dismount(bot);
        bot->StopMoving();
        bot->SetFacingToObject(gate);
        bot->CastSpell(bot, SA_SPELL_PLACE_SEAFORIUM, false);
        TC_LOG_DEBUG("playerbots", "[SOTA] bomb-plant bot=%s gate=%s hp=%u/%u at=%.1f,%.1f,%.1f", bot->GetName().c_str(),
                     info.name, gate->GetGOValue()->Building.Health, gate->GetGOValue()->Building.MaxHealth,
                     bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    // #750: true once per bot per round (instance, attackers) - one prep-post line each.
    bool PrepPostDue(Player* bot, BattlegroundSA* bg)
    {
        static std::map<ObjectGuid, SaRound> s_posted;
        SaLock lock(g_sotaState);
        SaRound const now = RoundOf(bg);
        auto itr = s_posted.find(bot->GetGUID());
        if (itr != s_posted.end() && SameRound(itr->second, now))
            return false;

        s_posted[bot->GetGUID()] = now;
        return true;
    }

    // A new round (ResetObjs re-creates every object and swaps Attackers): drop the old objective and siege
    // positions, which point at the other side's work.
    void CheckRoundSwap(Player* bot, AiObjectContext* context, BattlegroundSA* bg)
    {
        static std::map<ObjectGuid, std::pair<uint32, TeamId>> s_seen;  // bot -> (instance, attackers)
        SaLock lock(g_sotaState);
        std::pair<uint32, TeamId> const now = std::make_pair(bg->GetInstanceID(), bg->Attackers);
        auto itr = s_seen.find(bot->GetGUID());
        if (itr == s_seen.end())
        {
            s_seen[bot->GetGUID()] = now;
            return;
        }

        if (itr->second == now)
            return;

        bool const sameMatch = itr->second.first == now.first;
        itr->second = now;
        ResetPos(context, "bg objective");
        ResetPos(context, "bg siege");
        ForgetBoardPos(bot);
        ClearVehicleTarget(bot, context, "round-swap");  // #751/#754(b): last round's enemy vehicles may be ours now
        if (sameMatch && LogDue(bg->GetInstanceID(), 40 + uint32(bg->Attackers), 0))
            TC_LOG_DEBUG("playerbots", "[SOTA] round-swap instance=%u attackers=%s", bg->GetInstanceID(),
                         TeamName(bg->Attackers));
    }

    // Evidence for the 1:1 (MoP-only): the round's attackers, gate health while a demolisher sieges it, gate
    // destruction and graveyard ownership - one line per event per match, whichever bot sees it first.
    void ObserveMatch(Player* bot, AiObjectContext* context, BattlegroundSA* bg)
    {
        uint32 const instanceId = bg->GetInstanceID();

        if (LogDue(instanceId, 50 + uint32(bg->Attackers), 0))
            TC_LOG_DEBUG("playerbots", "[SOTA] round instance=%u attackers=%s", instanceId, TeamName(bg->Attackers));

        // #736: demolisher hits / deaths (and the drive-roll reset on a death), whichever bot comes first each 500 ms
        if (LogDue(instanceId, 3, 500))
            ObserveDemolishers(bg);

        if (bot->GetVehicle())
        {
            PositionInfo siege = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
            if (siege.isSet())
            {
                for (SaGate const& info : SA_GATES)
                {
                    GameObject* gate = SlotObject(bg, info.slot);
                    if (!gate || gate->GetExactDist2d(siege.x, siege.y) > 20.0f)
                        continue;

                    if (LogDue(instanceId, gate->GetGUID().GetCounter(), 15000))
                        TC_LOG_DEBUG("playerbots", "[SOTA] gate-hp gate=%s hp=%u/%u state=%u by=%s vehicle=%u dist=%.1f",
                                     info.name, gate->GetGOValue()->Building.Health, gate->GetGOValue()->Building.MaxHealth,
                                     uint32(gate->GetDestructibleState()), bot->GetName().c_str(),
                                     bot->GetVehicleBase() ? bot->GetVehicleBase()->GetEntry() : 0,
                                     bot->GetExactDist2d(gate->GetPositionX(), gate->GetPositionY()));
                    break;
                }
            }
        }

        if (!LogDue(instanceId, 1, 5000))
            return;

        // keys 1000000+ / 2000000+ carry the GO's guid counter: ResetObjs re-creates the objects each round
        for (SaGate const& info : SA_GATES)
            if (GameObject* gate = SlotObject(bg, info.slot))
                if (gate->GetDestructibleState() == GO_DESTRUCTIBLE_DESTROYED &&
                    LogDue(instanceId, 1000000 + gate->GetGUID().GetCounter(), 0))
                    TC_LOG_DEBUG("playerbots", "[SOTA] gate-down gate=%s attackers=%s instance=%u", info.name,
                                 TeamName(bg->Attackers), instanceId);

        for (SaFlag const& flag : SA_FLAGS)
            if (GameObject* go = SlotObject(bg, flag.slot))
                if (LogDue(instanceId, 2000000 + go->GetGUID().GetCounter(), 0))
                    TC_LOG_DEBUG("playerbots", "[SOTA] gy-owned gy=%s owner=%s attackers=%s instance=%u", flag.name,
                                 FlagOwner(bg, flag) == TEAM_ALLIANCE ? "alliance" : "horde", TeamName(bg->Attackers),
                                 instanceId);
    }

    // #752 (core lane, 2026-10-08): a bot-moved unit follows the mmaps path and the navmesh has no gates (PathGenerator
    // only tests gate collision when it shortens a path, PathGenerator.cpp:1297), so a driver sent to a gate behind an
    // INTACT one drives straight through it - typically the east lane breaks through first, every lane becomes "inner"
    // (Yellow), and the west drivers still between the walls roll through the closed Purple opening (192 drive samples
    // at x<1200, 40<y<200 with gate=yellow). Never go past an intact gate: if the path from `mover` to its stand point
    // in front of `gate` crosses another intact gate (FirstClosedCrossing, the gate-plane test DefenderDetour uses),
    // that gate is the one to break - the siege/charge/hold target and the cut destination. Path test cached per bot
    // (SA_BLOCK_RECHECK_MS, and at once when the wanted gate changes).
    struct SaBlock
    {
        uint32 instance = 0;
        int32 want = -1;
        int32 block = -1;
    };

    std::map<ObjectGuid, SaBlock> s_block;  // guarded by g_sotaState

    SaGate const* EffectiveGate(Player* bot, WorldObject const* mover, BattlegroundSA* bg, SaState const& s,
                                SaGate const* gate, float standOffset)
    {
        if (!gate)
            return nullptr;

        SaBlock cached;
        {
            SaLock lock(g_sotaState);
            cached = s_block[bot->GetGUID()];
        }

        bool const same = cached.instance == bg->GetInstanceID() && cached.want == int32(gate->slot);
        if (!same || BotDue(bot, 48, SA_BLOCK_RECHECK_MS))
        {
            SaCrossing const crossing = FirstClosedCrossing(mover, bg, s, GateSide(bg, gate, mover, standOffset));
            int32 const block = crossing.gate && crossing.gate != gate ? int32(crossing.gate->slot) : -1;
            if (block >= 0 && (!same || block != cached.block || BotDue(bot, 49, 15000)))
            {
                Position const cut = GateSide(bg, &SA_GATES[block], mover, standOffset);
                TC_LOG_DEBUG("playerbots", "[SOTA] drive-blocked bot=%s gate=%s want=%s dest-cut=%.1f,%.1f vehicle=%u at=%.1f,%.1f,%.1f",
                             bot->GetName().c_str(), SA_GATES[block].name, gate->name, cut.GetPositionX(),
                             cut.GetPositionY(), bot->GetVehicleBase() ? bot->GetVehicleBase()->GetEntry() : 0u,
                             mover->GetPositionX(), mover->GetPositionY(), mover->GetPositionZ());
            }

            cached.instance = bg->GetInstanceID();
            cached.want = int32(gate->slot);
            cached.block = block;
            SaLock lock(g_sotaState);
            s_block[bot->GetGUID()] = cached;
        }

        if (cached.block >= 0 && !s.down[cached.block])
            return &SA_GATES[cached.block];

        return gate;
    }

    // Driver: park SA_SIEGE_RANGE from the gate and let Hurl Boulder (IsSiegeShot) work on it.
    void DriveDemolisher(Player* bot, AiObjectContext* context, BattlegroundSA* bg, Unit* base, SaState const& s,
                         uint32 role)
    {
        // #737: the bot's lane, not the nearest gate; #752: never past an intact gate on the way
        SaGate const* gate = EffectiveGate(bot, base, bg, s, LaneGate(s, AttackerLane(base, role, s)), SA_DRIVE_STAND);

        // #753 (owner 2026-10-08): the siege position still on a gate that is down (or no longer this lane's gate) is
        // moved to the gate to break now, at once - before this tick's Hurl Boulder (IsSiegeShot aims at "bg siege")
        // fires at the rubble and turns the demolisher back to it. The objective goes with it, so the driver drives on.
        PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
        PositionInfo siege = posMap["bg siege"];
        if (siege.isSet())
        {
            SaGate const* aimed = GateAt(bg, siege.x, siege.y);
            if (aimed && aimed != gate)
            {
                GameObject* next = gate ? SlotObject(bg, gate->slot) : nullptr;
                float const nextDist =
                    gate ? base->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY()) : 0.0f;
                if (next)
                {
                    SetPos(posMap, "bg siege", *next, bot->GetMapId());
                    SetPos(posMap, "bg objective",
                           nextDist <= SA_SIEGE_RANGE ? Position(*base) : GateSide(bg, gate, base, SA_DRIVE_STAND),
                           bot->GetMapId());  // #752: in front of the gate, never its centre
                }
                else
                {
                    ResetPos(context, "bg siege");
                    ResetPos(context, "bg objective");
                }

                TC_LOG_DEBUG("playerbots", "[SOTA] siege-retarget bot=%s from=%s to=%s why=%s dist=%.1f at=%.1f,%.1f,%.1f",
                             bot->GetName().c_str(), aimed->name, next ? gate->name : "none",
                             s.down[aimed->slot] ? "gate-down" : "lane", nextDist, base->GetPositionX(),
                             base->GetPositionY(), base->GetPositionZ());
            }
        }

        if (!gate)
            return;

        float const dist = base->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY());
        if (BotDue(bot, 33, 10000))
        {
            float moved = -1.0f;
            {
                SaLock lock(g_sotaState);
                auto itr = s_boardPos.find(bot->GetGUID());
                if (itr != s_boardPos.end())
                    moved = base->GetExactDist2d(itr->second.GetPositionX(), itr->second.GetPositionY());
            }
            TC_LOG_DEBUG("playerbots", "[SOTA] drive bot=%s vehicle=%u at=%.1f,%.1f,%.1f moved=%.1f gate=%s gatedist=%.1f hp=%.0f",
                         bot->GetName().c_str(), base->GetEntry(), base->GetPositionX(), base->GetPositionY(),
                         base->GetPositionZ(), moved, gate->name, dist, base->GetHealthPct());
        }

        if (dist > SA_SIEGE_RANGE)
            return;

        if (base->isMoving())
            base->StopMoving();

        // hold here: BGTactics then sees the objective reached and does not drive on into the gate
        SetPos(posMap, "bg objective", *base, bot->GetMapId());
        if (BotDue(bot, 34, 30000))
            TC_LOG_DEBUG("playerbots", "[SOTA] siege-park bot=%s gate=%s dist=%.1f", bot->GetName().c_str(), gate->name, dist);
    }

    // Gunner: aim the cannon (Rocket Blast fires at "current target") at the nearest enemy demolisher in range,
    // else at the nearest attacking player in range ("pick off rushers").
    void AimCannon(Player* bot, AiObjectContext* context, BattlegroundSA* bg, Unit* cannon)
    {
        Unit* target = nullptr;
        float bestDist = SA_CANNON_MAX;
        for (uint32 slot = BG_SA_DEMOLISHER_1; slot <= BG_SA_DEMOLISHER_8; ++slot)
        {
            Creature* demolisher = SlotCreature(bg, slot);
            if (!UsableVehicle(demolisher, FactionOf(bg->Attackers)))
                continue;

            float const dist = cannon->GetExactDist2d(demolisher);
            if (dist >= SA_CANNON_MIN && dist < bestDist)
            {
                bestDist = dist;
                target = demolisher;
            }
        }

        if (!target)
        {
            Unit* enemy = context->GetValue<Unit*>("enemy player target")->Get();
            if (enemy && enemy->IsAlive())
            {
                float const dist = cannon->GetExactDist2d(enemy);
                if (dist >= SA_CANNON_MIN && dist < SA_CANNON_MAX)
                    target = enemy;
            }
        }

        if (!target)
            return;

        context->GetValue<Unit*>("current target")->Set(target);  // CastVehicleSpellAction reads "current target"
        if (BotDue(bot, 35, 10000))
            TC_LOG_DEBUG("playerbots", "[SOTA] cannon-target bot=%s target=%s entry=%u dist=%.1f hp=%.0f",
                         bot->GetName().c_str(), target->GetName().c_str(), target->GetEntry(),
                         cannon->GetExactDist2d(target), target->GetHealthPct());
    }

    // #751: an enemy player hindering this defender - one auto-attacking it (Unit::Attack puts every bot attacker in
    // getAttackers(), melee or not) or an enemy player near it with the bot as its target. Vehicles (a driven demolisher
    // also counts as player-controlled) are not "players attacking" here.
    Unit* PlayerThreat(Player* bot, PlayerbotAI* botAI, AiObjectContext* context)
    {
        for (Unit* attacker : bot->getAttackers())
            if (attacker && attacker->IsAlive() && !attacker->IsVehicle() && !attacker->GetVehicle() &&
                attacker->GetCharmerOrOwnerPlayerOrPlayerItself())
                return attacker;

        for (ObjectGuid const guid : context->GetValue<GuidVector>("nearest enemy players")->Get())
        {
            Unit* unit = botAI->GetUnit(guid);
            Player* player = unit ? unit->ToPlayer() : nullptr;
            if (player && player->IsAlive() && !player->GetVehicle() && player->IsInCombat() &&
                player->GetTarget() == bot->GetGUID() && bot->GetExactDist(player) < SA_DEF_THREAT)
                return player;
        }

        return nullptr;
    }

    // #751: an enemy demolisher worth attacking now: alive and selectable, within HuntRange of a gate the attackers
    // must break next (approaching or at it), within SA_DEF_ENGAGE of the bot, in sight, and no intact gate on the
    // straight line between them (the navmesh has no gates: a melee chase would walk through it - the objective layer
    // routes the bot out over the wall first, DefenderDetour). Driven ones first (an empty one is not hitting a gate),
    // then the nearest.
    Creature* EngageableDemolisher(Player* bot, BattlegroundSA* bg, TeamId team, SaState const& s)
    {
        std::vector<SaGate const*> const gates = NextGates(s);
        Creature* best = nullptr;
        float bestScore = 0.0f;
        for (uint32 slot = BG_SA_DEMOLISHER_1; slot <= BG_SA_DEMOLISHER_8; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!UsableVehicle(vehicle, FactionOf(Other(team))))
                continue;

            // the core decides whether a (driven or empty) demolisher can be attacked by this bot right now
            if (!bot->IsValidAttackTarget(vehicle))
                continue;

            float const dist = bot->GetExactDist(vehicle);
            if (dist >= SA_DEF_ENGAGE)
                continue;

            bool nearGate = false;
            for (SaGate const* gate : gates)  // #736: the first wall's range covers the beach (HuntRange)
                if (vehicle->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY()) < HuntRange(gate))
                    nearGate = true;

            if (!nearGate)
                continue;

            G3D::Vector3 const from(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            G3D::Vector3 const to(vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ());
            if (SegmentClosedCrossing(bg, s, from, to).gate || !bot->IsWithinLOSInMap(vehicle))
                continue;

            float const score = dist + (DriverOf(vehicle) ? 0.0f : SA_DEF_ENGAGE);
            if (!best || score < bestScore)
            {
                bestScore = score;
                best = vehicle;
            }
        }

        return best;
    }

    // AttackAction::Attack's hand-over (AttackActions.cpp, AC AttackAction.cpp:150-200), without its chat errors and
    // the "wait for attack" hold (no master holds a bg bot's attack): select, "current target", combat engine, attack.
    bool EngageUnit(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, Unit* target)
    {
        if (!target || !target->IsInWorld() || !target->IsAlive() || !bot->IsValidAttackTarget(target) ||
            !bot->IsWithinLOSInMap(target))
            return false;

        if (bot->GetVictim() == target && context->GetValue<Unit*>("current target")->Get() == target)
            return true;

        Dismount(bot);
        bot->SetSelection(target->GetGUID());
        context->GetValue<Unit*>("old target")->Set(context->GetValue<Unit*>("current target")->Get());
        context->GetValue<Unit*>("current target")->Set(target);
        botAI->ChangeEngine(BOT_STATE_COMBAT);
        bot->Attack(target, bot->IsWithinMeleeRange(target) || PlayerBotSpec::IsMelee(bot));
        return true;
    }

    // #751 (owner 2026-10-08): defender priority. Wowpedia (warcraft.wiki.gg "Strand of the Ancients", revid 6865830):
    // "Demolishers also take priority to kill over players, unless they were hindering your ability to attack the
    // demolisher" / "always attack demolishers as soon as possible, before they are in range of a gate".
    //   1. a player attacking the bot -> fight it first (only switched to here when the bot is on a demolisher of ours;
    //      otherwise the pvp/combat strategies already have it);
    //   2. else an engageable enemy demolisher -> "prioritized targets" = it (attackers -> dps target keep the combat
    //      engine on it, TargetValue.cpp IsHighPriority) and attack it now;
    //   3. else drop the priority.
    // A gunner with a front cannon to man leaves the demolishers to that cannon (AimCannon).
    void DefenderEngage(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundSA* bg, TeamId team,
                        uint32 role, SaState const& s)
    {
        if (!BotDue(bot, 46, SA_DEF_ENGAGE_MS))
            return;

        // Owner 2026-10-08: no vehicle hunting before the round starts (warcraft.wiki.gg SotA: the defence moves freely
        // during the countdown; attackers may only use the demolishers 10-15 s into the round).
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
        {
            DropVehiclePriority(bot, context);
            return;
        }

        ObjectGuid const vehicleGuid = DefVehicleOf(bot);
        if (Unit* threat = PlayerThreat(bot, botAI, context))
        {
            if (!vehicleGuid)
                return;

            Unit* current = context->GetValue<Unit*>("current target")->Get();
            DropVehiclePriority(bot, context);
            if ((!current || current->GetGUID() == vehicleGuid) && EngageUnit(bot, botAI, context, threat))
                TC_LOG_DEBUG("playerbots", "[SOTA] def-target bot=%s vehicle=0 defend=%s dist=%.1f at=%.1f,%.1f,%.1f",
                             bot->GetName().c_str(), threat->GetName().c_str(), bot->GetExactDist(threat),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            return;
        }

        if (role <= SA_ROLE_CANNON_LAST && !s.relic && PreferredCannon(bot, bg, team, s, role, 400.0f))
        {
            DropVehiclePriority(bot, context);
            return;
        }

        Creature* demolisher = EngageableDemolisher(bot, bg, team, s);
        if (!demolisher)
        {
            DropVehiclePriority(bot, context);
            return;
        }

        bool const fresh = vehicleGuid != demolisher->GetGUID();
        if (fresh)
            DropVehiclePriority(bot, context);

        context->GetValue<GuidVector>("prioritized targets")->Set({demolisher->GetGUID()});
        {
            SaLock lock(g_sotaState);
            s_defVehicle[bot->GetGUID()] = demolisher->GetGUID();
        }
        if (!EngageUnit(bot, botAI, context, demolisher))
            return;

        if (fresh || BotDue(bot, 47, 10000))
        {
            SaGate const* gate = NearestGate(NextGates(s), demolisher);
            Player* driver = DriverOf(demolisher);
            TC_LOG_DEBUG("playerbots", "[SOTA] def-target bot=%s role=%u vehicle=%u vguid=%u driver=%s hp=%.0f dist=%.1f gate=%s gatedist=%.1f",
                         bot->GetName().c_str(), role, demolisher->GetEntry(), demolisher->GetGUID().GetCounter(),
                         driver ? driver->GetName().c_str() : "-", demolisher->GetHealthPct(), bot->GetExactDist(demolisher),
                         gate ? gate->name : "-",
                         gate ? demolisher->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY()) : 0.0f);
        }
    }

    // #737 defender detours. The Defender's Portal teleports on this core (owner 2026-10-08; the earlier "portal-dead"
    // match-wide fallback assumed it did not and is gone): GO 192819 is a SPELLCASTER (type 22) casting 54640 on the
    // user (GameObject::Use, GameObject.cpp SPELLCASTER case); 54640 is SCRIPT_EFFECT (SpellEffect.dbc effect 77)
    // run by world.spell_scripts (54640, command 15 CAST_SPELL 54643, datalong2 2 = target casts on itself);
    // 54643 effect 0 DUMMY at TARGET_UNIT_NEARBY_ENTRY limited by world.conditions (13/1/54643, type 31) to creature
    // 23472, whose position spell_wintergrasp_defender_teleport_trigger (zone_wintergrasp.cpp, "SotA uses same
    // teleport spells") makes the destination of effect 1 TELEPORT_UNITS; effect 2 is a 10 s marker aura (SpellDuration
    // index 1) that spell_wintergrasp_defender_teleport's CheckCast refuses a new use under. The 23472 triggers stand
    // behind each gate (world.creature map 607 guids 100518/100528/100530/100531/100579 = AC's SOTADefPortalDest, and
    // BG_SA_NpcSpawnlocs' "Triggers"), each ~30 yd from its portal, so the nearest one is behind the portal's gate.
    constexpr uint32 SA_SPELL_DEFENDER_PORTAL_LOCK = 54643;

    std::mutex g_sotaPortalMutex;
    std::map<ObjectGuid, std::pair<uint32, Position>> g_sotaPortalUse;  // bot -> (getMSTime, where it used one)

    void NotePortalUse(Player* bot)
    {
        std::lock_guard<std::mutex> lock(g_sotaPortalMutex);
        g_sotaPortalUse[bot->GetGUID()] = std::make_pair(getMSTime(), Position(bot->GetPositionX(), bot->GetPositionY(),
                                                                               bot->GetPositionZ()));
    }

    // Evidence only (per bot, no fallback): how far a portal use moved the bot. A "portal-stay" is a single failed use
    // (e.g. still under the 54643 lock); the bot keeps the portal objective and AtObjective retries it.
    void CheckPortalUse(Player* bot)
    {
        std::lock_guard<std::mutex> lock(g_sotaPortalMutex);
        auto itr = g_sotaPortalUse.find(bot->GetGUID());
        if (itr == g_sotaPortalUse.end() || getMSTimeDiff(itr->second.first, getMSTime()) < SA_PORTAL_SETTLE_MS)
            return;

        float const moved = bot->GetExactDist2d(itr->second.second.GetPositionX(), itr->second.second.GetPositionY());
        TC_LOG_DEBUG("playerbots", "[SOTA] %s bot=%s moved=%.1f from=%.1f,%.1f,%.1f at=%.1f,%.1f,%.1f",
                     moved < 8.0f ? "portal-stay" : "portal-tp", bot->GetName().c_str(), moved,
                     itr->second.second.GetPositionX(), itr->second.second.GetPositionY(),
                     itr->second.second.GetPositionZ(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        g_sotaPortalUse.erase(itr);
    }

    // Owner 2026-10-08: defenders went through closed gates; they must take the stairs to the turrets and jump down,
    // and come back through the teleporter by the gate. If the navmesh path to `dest` crosses an intact gate:
    // from inside -> the wall platform beside that gate (AtObjective jumps from there), from outside -> that gate's
    // Defender's Portal (AtObjective uses it). No route for that gate (Yellow out, the Chamber) -> unchanged.
    // bot -> DefenderDetour's last answer; the path test runs at most every SA_DETOUR_RECHECK_MS per bot and destination.
    // #750: namespace scope so a wall jump can drop it (else the landed bot is sent back up to the platform for up to
    // SA_DETOUR_RECHECK_MS - through the gate, the landing is outside).
    struct SaDetour
    {
        uint32 instance = 0;
        Position dest;
        bool set = false;
        Position via;
    };

    std::map<ObjectGuid, SaDetour> s_detour;  // guarded by g_sotaState

    void ForgetDetour(Player* bot)
    {
        SaLock lock(g_sotaState);
        s_detour.erase(bot->GetGUID());
    }

    bool DefenderDetour(Player* bot, BattlegroundSA* bg, SaState const& s, Position const& dest, Position& via)
    {
        SaDetour cached;
        {
            SaLock lock(g_sotaState);
            cached = s_detour[bot->GetGUID()];
        }

        if (cached.instance == bg->GetInstanceID() && cached.dest.GetExactDist2d(&dest) < 10.0f &&
            !BotDue(bot, 41, SA_DETOUR_RECHECK_MS))
        {
            via = cached.via;
            return cached.set;
        }

        cached.instance = bg->GetInstanceID();
        cached.dest = dest;
        cached.set = false;

        SaCrossing const crossing = FirstClosedCrossing(bot, bg, s, dest);
        if (!crossing.gate)
        {
            SaLock lock(g_sotaState);
            s_detour[bot->GetGUID()] = cached;
            return false;
        }

        if (!crossing.outside)
        {
            if (SaWallRoute const* route = WallRouteOf(crossing.gate->slot))
            {
                via = route->platform;
                cached.set = true;
            }
        }
        else
        {
            if (GameObject* portal = SlotObject(bg, PortalSlotOf(crossing.gate->slot)))
            {
                via = Position(portal->GetPositionX(), portal->GetPositionY(), portal->GetPositionZ());
                cached.set = true;
            }
        }

        cached.via = via;
        {
            SaLock lock(g_sotaState);
            s_detour[bot->GetGUID()] = cached;
        }

        if (cached.set && BotDue(bot, 43, 15000))
            TC_LOG_DEBUG("playerbots", "[SOTA] detour bot=%s gate=%s via=%s to=%.1f,%.1f,%.1f from=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), crossing.gate->name, crossing.outside ? "portal" : "wall-jump",
                         via.GetPositionX(), via.GetPositionY(), via.GetPositionZ(), bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ());
        return cached.set;
    }

    // #737: on the wall platform the bot was sent to (`ox`,`oy` = its objective / start spot) - jump down in front of
    // the cannon, as AC's wsJumpDown/eyJumpDown do (MovementAction::JumpTo: MoveJump at run speed; a free function here,
    // as BgTacticsIC's DropFromGunship). #750: also during WAIT_JOIN (StartSpot), `phase` says which. Wave 20: no
    // dismount any more, and not while a mount is about to cut the jump in the air (MountWontCut).
    bool WallJump(Player* bot, BattlegroundSA* bg, SaState const& s, float ox, float oy, char const* phase)
    {
        for (SaWallRoute const& route : SA_WALL_ROUTES)
        {
            if (s.down[route.gateSlot] || route.platform.GetExactDist2d(ox, oy) > 1.0f ||
                bot->GetExactDist2d(route.platform.GetPositionX(), route.platform.GetPositionY()) > SA_PLATFORM_REACH ||
                std::fabs(bot->GetPositionZ() - route.platform.GetPositionZ()) > 4.0f || !MountWontCut(bot, bg) ||
                !BotDue(bot, 44, 3000))
                continue;

            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            float const speed = bot->GetSpeed(MOVE_RUN);
            bot->GetMotionMaster()->MoveJump(route.landing.GetPositionX(), route.landing.GetPositionY(),
                                             route.landing.GetPositionZ(), speed, speed);
            ForgetDetour(bot);
            TC_LOG_DEBUG("playerbots", "[SOTA] wall-jump bot=%s gate=%s phase=%s from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f mounted=%u",
                         bot->GetName().c_str(), SA_GATES[route.gateSlot].name, phase, bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ(), route.landing.GetPositionX(),
                         route.landing.GetPositionY(), route.landing.GetPositionZ(), uint32(bot->IsMounted()));
            return true;
        }

        return false;
    }

    // #750 (wave 20): a jump cut in the air (by a mount, or anything that stops the bot) ends in the #747 fall, onto
    // whatever is under it - Emerita: jump from the Red platform cut 3 yd out, fell to 1233.2,-187.0,52.8, a ledge
    // the navmesh does not connect to the courtyard ("detour via=portal" every few seconds, static for the whole
    // round). A bot standing still below a platform on the landing's side of it (outward, more than 5 yd lower), within
    // SA_LANDING_RESCUE of the landing, hops onto the landing.
    bool LandingHop(Player* bot, BattlegroundSA* bg, char const* phase)
    {
        if (bot->isMoving() || bot->IsInCombat() || bot->GetTransport() || bot->GetVehicle() ||
            bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
            return false;

        for (SaWallRoute const& route : SA_WALL_ROUTES)
        {
            float const dist = bot->GetExactDist2d(route.landing.GetPositionX(), route.landing.GetPositionY());
            if (dist <= 3.0f || dist > SA_LANDING_RESCUE ||
                bot->GetPositionZ() > route.platform.GetPositionZ() - 5.0f ||
                bot->GetPositionZ() < route.landing.GetPositionZ() - 4.0f)
                continue;

            // outward: past the platform towards the landing
            float const ox = route.landing.GetPositionX() - route.platform.GetPositionX();
            float const oy = route.landing.GetPositionY() - route.platform.GetPositionY();
            float const along = ((bot->GetPositionX() - route.platform.GetPositionX()) * ox +
                                 (bot->GetPositionY() - route.platform.GetPositionY()) * oy) / (ox * ox + oy * oy);
            if (along < 0.1f || !MountWontCut(bot, bg) || !BotDue(bot, 52, 10000))
                continue;

            float const speed = bot->GetSpeed(MOVE_RUN);
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            bot->GetMotionMaster()->MoveJump(route.landing.GetPositionX(), route.landing.GetPositionY(),
                                             route.landing.GetPositionZ(), speed, 4.0f);
            ForgetDetour(bot);
            TC_LOG_DEBUG("playerbots", "[SOTA] landing-hop bot=%s gate=%s phase=%s from=%.1f,%.1f,%.1f dist=%.1f",
                         bot->GetName().c_str(), SA_GATES[route.gateSlot].name, phase, bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ(), dist);
            return true;
        }

        return false;
    }

    // #750 (wave 20) ROOT CAUSE: StartSpot runs only from "bg move to start", whose trigger "bg waiting" is checked
    // every 30 s (PvpTriggers.h BgWaitingTrigger, Trigger(..., 30) - as AC), and the first warm-up on this core is
    // 60 s (BattlegroundSA.h BG_SA_WARMUPLENGTH). The step-by-step warm-up (walk to the platform / jump / walk on /
    // board) needed a call at every step, so a defender got two: walk to the platform, then jump - and stood at the
    // landing (Arcel, Jesto, ...) or, arriving after its second call, on the platform (Emerita, Estevin, ...) when the
    // round began: 0 of 20 posts. Fix: StrandStrategy also runs "bg move to start" on "often" (coordinator's hook), so
    // StartSpot comes every few seconds, and the warm-up goes in legs, each started from StartSpot:
    //   1. ground leg to the wall platform (take-off): a navmesh path as a spline chain (one link);
    //   2. the REAL jump off it (WallJump: MoveJump, the parabolic jump the client draws; never an air-walking spline,
    //      owner #747 "bots floating through the air"), when no mount is about to cut it (MountWontCut);
    //   3. after landing (no detour any more), the ground leg from the landing to the post;
    //   then board the cannon / log the gate post on arrival. A bot on a leg is left alone (OnPrepLeg); one stopped
    //   off its leg (a mount, a fight) gets its next leg re-planned from where it stands.
    struct SaPrepLeg
    {
        SaRound round;
        Position post;     // where the bot is going in the end
        Position target;   // this leg's end (the platform, or the post)
        uint32 startMs = 0;
        uint32 etaMs = 0;
        char const* postKind = "none";
        char const* gate = "-";
    };

    std::map<ObjectGuid, SaPrepLeg> s_prepLeg;  // guarded by g_sotaState

    bool PrepLegOf(Player* bot, BattlegroundSA* bg, SaPrepLeg& out)
    {
        SaLock lock(g_sotaState);
        auto itr = s_prepLeg.find(bot->GetGUID());
        if (itr == s_prepLeg.end() || !SameRound(itr->second.round, RoundOf(bg)))
            return false;

        out = itr->second;
        return true;
    }

    // Still on the leg this file sent it on, towards (about) this post.
    bool OnPrepLeg(Player* bot, BattlegroundSA* bg, Position const& post)
    {
        SaPrepLeg leg;
        return bot->isMoving() && bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == SPLINE_CHAIN_MOTION_TYPE &&
               PrepLegOf(bot, bg, leg) && leg.post.GetExactDist2d(&post) <= SA_PREP_POST_REACH &&
               getMSTimeDiff(leg.startMs, getMSTime()) < leg.etaMs + 10000;
    }

    // Start the next ground leg towards `post`: to `target` (the platform when `detour`, else the post itself) along
    // the navmesh, as a one-link spline chain at run speed. False = no clean path (the caller falls back to MoveTo).
    bool LaunchPrepLeg(Player* bot, BattlegroundSA* bg, SaState const& s, Position const& post, Position const& target,
                       bool toPlatform, char const* postKind, SaGate const* postGate, uint32 role)
    {
        PathGenerator path(bot);
        path.CalculatePath(target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(), false);
        uint32 const type = uint32(path.GetPathType());
        if (!(type & PATHFIND_NORMAL) || (type & (PATHFIND_NOPATH | PATHFIND_INCOMPLETE | PATHFIND_SHORT)))
            return false;

        Movement::PointsArray points;
        points.push_back(G3D::Vector3(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ()));
        for (G3D::Vector3 const& point : path.GetPath())
            if ((point - points.back()).length() > 0.1f)
                points.push_back(point);

        if (points.size() < 2)
            return false;

        // the post leg must not cross an intact gate (DefenderDetour would have sent it over the wall)
        if (!toPlatform)
            for (size_t i = 1; i < points.size(); ++i)
                if (SegmentClosedCrossing(bg, s, points[i - 1], points[i]).gate)
                    return false;

        G3D::Vector3 const middle = (points.front() + points.back()) / 2.0f;
        float length = 0.0f;
        for (size_t i = 0; i < points.size(); ++i)
        {
            G3D::Vector3 const offset = points[i] - middle;
            if (std::fabs(offset.x) >= SA_SPLINE_HALF_SPAN || std::fabs(offset.y) >= SA_SPLINE_HALF_SPAN ||
                std::fabs(offset.z) >= SA_SPLINE_HALF_SPAN)
                return false;

            if (i)
                length += (points[i] - points[i - 1]).length();
        }

        float const speed = bot->GetSpeed(MOVE_RUN);
        uint32 const etaMs = uint32(length / speed * 1000.0f);
        std::vector<SplineChainLink> chain;
        chain.emplace_back(points, etaMs, 0u, speed);
        bot->GetMotionMaster()->MoveAlongSplineChain(0, chain, false);

        SaPrepLeg leg;
        leg.round = RoundOf(bg);
        leg.post = post;
        leg.target = target;
        leg.startMs = getMSTime();
        leg.etaMs = etaMs;
        leg.postKind = postKind;
        leg.gate = postGate ? postGate->name : "-";
        {
            SaLock lock(g_sotaState);
            s_prepLeg[bot->GetGUID()] = leg;
        }

        TC_LOG_DEBUG("playerbots", "[SOTA] prep-leg bot=%s role=%u leg=%s post=%s gate=%s points=%u len=%.1f eta=%u speed=%.1f start-delay=%u from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f",
                     bot->GetName().c_str(), role, toPlatform ? "platform" : "post", postKind, leg.gate,
                     uint32(points.size()), length, etaMs, speed, bg->GetStartDelayTime(), bot->GetPositionX(),
                     bot->GetPositionY(), bot->GetPositionZ(), target.GetPositionX(), target.GetPositionY(),
                     target.GetPositionZ());
        return true;
    }

    bool SelectAttacker(Player* bot, PlayerbotAI* botAI, BattlegroundSA* bg, TeamId team, uint32 role,
                        PositionMap& posMap, SaState const& s)
    {
        uint32 const mapId = bot->GetMapId();
        std::vector<SaGate const*> const gates = NextGates(s);

        // #736: riding a boat - hold until it is docked, then off onto the dock (BoatStep)
        bool acted = false;
        if (BoatStep(bot, bg, "match", acted) || acted)
            return true;

        // Out at sea and not on a boat (fell off; the core rescues it at 59 s / 53 s): hold, never walk into the sea.
        if (Offshore(bot))
        {
            if (BotDue(bot, 36, 60000))
                TC_LOG_DEBUG("playerbots", "[SOTA] boat bot=%s at=%.1f,%.1f,%.1f transport=%u", bot->GetName().c_str(),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetTransport() ? 1u : 0u);
            return true;
        }

        WorldObject const* from = bot->GetVehicleBase() ? static_cast<WorldObject const*>(bot->GetVehicleBase()) : bot;
        int8 const lane = AttackerLane(from, role, s);
        SaGate const* laneGate = LaneGate(s, lane);  // #752: EffectiveGate below, per mover
        if (laneGate && BotDue(bot, 42, 30000))
            TC_LOG_DEBUG("playerbots", "[SOTA] lane bot=%s role=%u lane=%s gate=%s vehicle=%u", bot->GetName().c_str(),
                         role, lane == 0 ? "west" : lane == 1 ? "east" : "inner", laneGate->name,
                         bot->GetVehicleBase() ? bot->GetVehicleBase()->GetEntry() : 0u);

        if (Unit* base = bot->GetVehicleBase())
        {
            if (base->GetEntry() != NPC_DEMOLISHER_SA || !botAI->IsInVehicle(true))
                return true;  // a passenger: nothing to steer

            // #752: never past an intact gate - the one in the way is the one to break
            SaGate const* gate = EffectiveGate(bot, base, bg, s, laneGate, SA_DRIVE_STAND);
            if (!gate)
            {
                // the Chamber is open: on foot to the relic
                LeaveVehicle(bot, botAI->GetAiObjectContext(), "demolisher", "relic");
                return true;
            }

            // AC PR mod-playerbots#2856's IoC pattern: the gate is the siege position, the objective is where the
            // vehicle stops (here: wherever it is once within SA_SIEGE_RANGE)
            if (GameObject* go = SlotObject(bg, gate->slot))
                SetPos(posMap, "bg siege", *go, mapId);

            float const dist = base->GetExactDist2d(gate->ground.GetPositionX(), gate->ground.GetPositionY());
            // #752: drive to a point in front of the gate on the vehicle's side (dest cut), never the gate centre
            SetPos(posMap, "bg objective",
                   dist <= SA_SIEGE_RANGE ? Position(*base) : GateSide(bg, gate, base, SA_DRIVE_STAND), mapId);
            return true;
        }

        if (s.relic)
        {
            SetPos(posMap, "bg objective", SA_RELIC_FLOOR, mapId);
            return true;
        }

        // #752: on foot too - a carrier / gate holder whose way to its gate runs through another intact gate stops at
        // (and plants on) that one
        laneGate = EffectiveGate(bot, bot, bg, s, laneGate, SA_GATE_HOLD);

        if (CarryingCharge(bot) && laneGate)
        {
            SetPos(posMap, "bg objective", GateSide(bg, laneGate, bot, SA_PLANT_STAND), mapId);  // #736
            return true;
        }

        bool const gyRole = role >= SA_ROLE_GY_FIRST && role <= SA_ROLE_GY_LAST;
        if (SaFlag const* flag = CapturableFlag(bot, bg, s, gyRole ? 1000.0f : SA_NEAR_FLAG))
            if (GameObject* go = SlotObject(bg, flag->slot))
            {
                SetPos(posMap, "bg objective", *go, mapId);
                return true;
            }

        // drivers: any free demolisher of ours, own lane first (#737: the workshop pair too once a side graveyard is
        // ours - Wowpedia: "capture the Eastern and Western Graveyards as soon as possible" for more demolishers)
        if (role <= SA_ROLE_DRIVER_LAST)
            if (Creature* demolisher = FreeDemolisher(bot, bg, team, 1000.0f, lane))
            {
                SetPos(posMap, "bg objective", *demolisher, mapId);
                return true;
            }

        // #737 re-board: cappers with nothing to take and the escort take a free demolisher close by
        if (role >= SA_ROLE_GY_FIRST)
            if (Creature* demolisher = FreeDemolisher(bot, bg, team, SA_REBOARD_RANGE, lane))
            {
                SetPos(posMap, "bg objective", *demolisher, mapId);
                return true;
            }

        if (role >= SA_ROLE_BOMB_FIRST && role <= SA_ROLE_BOMB_LAST && !gates.empty())
            if (GameObject* pile = NearestBombPile(bot, bg, 300.0f))
            {
                SetPos(posMap, "bg objective", *pile, mapId);
                return true;
            }

        // escort: stay with a driven demolisher of ours on our lane (Wowpedia: "Other players need to provide support
        // for the tanks"); the pvp strategy fights whoever attacks it
        if (Creature* demolisher = DrivenDemolisher(bot, bg, team, 300.0f, nullptr, lane))
        {
            SetPos(posMap, "bg objective", Jitter(*demolisher, 4.0f), mapId);
            return true;
        }

        if (laneGate)
            SetPos(posMap, "bg objective", Jitter(GateSide(bg, laneGate, bot, SA_GATE_HOLD), 2.0f), mapId);  // #736

        return true;
    }

    bool SelectDefender(Player* bot, AiObjectContext* context, BattlegroundSA* bg, TeamId team, uint32 role, PositionMap& posMap,
                        SaState const& s)
    {
        uint32 const mapId = bot->GetMapId();
        std::vector<SaGate const*> const gates = NextGates(s);
        int8 const lane = DefenderLane(role, s);

        if (Unit* base = bot->GetVehicleBase())
        {
            if (base->GetEntry() == NPC_ANTI_PERSONNAL_CANNON)
            {
                // AC: a gunner is "not the driver" and gets no objective. Beyond AC: get off once the cannon's gate
                // is down (the wall it guards is breached), so the defender falls back with the line.
                SaGate const* gate = GateOfCannon(bg, base);
                if (s.relic || (gate && s.down[gate->slot]))
                    LeaveVehicle(bot, context, "cannon", "gate-down");
            }
            return true;
        }

        // #737: every defender objective goes through DefenderDetour (wall stairs + jump out, the portal back in)
        auto setObjective = [&](Position const& dest)
        {
            Position via;
            SetPos(posMap, "bg objective", DefenderDetour(bot, bg, s, dest, via) ? via : dest, mapId);
            return true;
        };

        if (s.relic)
            return setObjective(Jitter(SA_RELIC_FLOOR, 3.0f));

        if (role <= SA_ROLE_CANNON_LAST)
            if (Creature* cannon = PreferredCannon(bot, bg, team, s, role, 400.0f))  // #750: one gunner per cannon
                return setObjective(Position(cannon->GetPositionX(), cannon->GetPositionY(), cannon->GetPositionZ()));

        // graveyard guards (Wowpedia: "defend your east and west GY's with a furor"), while the attackers could take it
        for (SaFlag const& flag : SA_FLAGS)
        {
            bool const mine = (role == SA_ROLE_GUARD_WEST && flag.slot == BG_SA_LEFT_FLAG) ||
                              (role == SA_ROLE_GUARD_EAST && flag.slot == BG_SA_RIGHT_FLAG);
            if (!mine || FlagOwner(bg, flag) != team || !FlagOpen(s, flag))
                continue;

            if (GameObject* go = SlotObject(bg, flag.slot))
                return setObjective(Jitter(*go, 4.0f));
        }

        // hunters (and gunners without a cannon): the enemy demolishers closing on our front gates, own lane first
        if (lane >= 0)
        {
            std::vector<SaGate const*> const laneGates = {LaneGate(s, lane)};
            if (Creature* demolisher = DrivenDemolisher(bot, bg, Other(team), 1000.0f, &laneGates))
                return setObjective(Position(demolisher->GetPositionX(), demolisher->GetPositionY(),
                                             demolisher->GetPositionZ()));
        }

        if (Creature* demolisher = DrivenDemolisher(bot, bg, Other(team), 1000.0f, &gates))
            return setObjective(Position(demolisher->GetPositionX(), demolisher->GetPositionY(), demolisher->GetPositionZ()));

        // #736 (owner 2026-10-14; warcraft.wiki.gg SotA ===Defensive===): while the lane's front gate stands, the holders
        // wait on the beach between the landing docks (StartSpot's warm-up post) - DefenderEngage takes any enemy
        // demolisher that comes within SA_DEF_ENGAGE there, the hunt above any driven one on the beach (HuntRange)
        Position beach;
        if (BeachPost(s, lane, role, beach))
            return setObjective(Jitter(beach, 2.0f));

        // gate holders: their lane's front gate, on the side they are on (#736 GateSide; a holder that jumped off the
        // wall to hunt stays in front of it, one still inside holds behind it)
        if (SaGate const* gate = lane >= 0 ? LaneGate(s, lane) : NearestGate(gates, bot))
            return setObjective(Jitter(GateSide(bg, gate, bot, SA_GATE_HOLD), 2.0f));

        return true;
    }
}

namespace SotaBeyondAc
{
    bool Enabled() { return g_sotaEnabled.load(); }

    void SetEnabled(bool enabled) { g_sotaEnabled.store(enabled); }

    void SetDriveChanceOverride(int32 pct)
    {
        g_sotaDriveChanceOverride.store(pct < 0 ? -1 : std::min(pct, 100));

        // the new chance applies to every demolisher from now on: drop the rolls already made (each re-rolls once, with
        // the same deterministic draw)
        SaLock lock(g_sotaState);
        for (auto& itr : s_vehLife)
            itr.second.rolled = false;
    }

    int32 DriveChanceOverride() { return g_sotaDriveChanceOverride.load(); }

    // #765 (beyond AC). A driver never has a "current target": AttackAction refuses a seat without CAN_ATTACK (AC
    // AttackAction.cpp:137, MoP AttackActions.cpp:152; the demolisher driver seat 2021 has none). So on AC's code Ram
    // fires only as a siege shot within 15 yd of the gate (PlayerbotAI.cpp IsSiegeShot, AC PR mod-playerbots#2856),
    // and a SotA driver parks SA_SIEGE_RANGE out, so it never rammed (wave 27: 0 ram in 4737 driver rows). This picks
    // the enemy the Ram hits now: the nearest alive, attackable, visible enemy unit in line of sight within the cone
    // the cast uses. PlayerbotAI::CastVehicleSpell turns a driven vehicle to the "bg siege" point right before it
    // casts (AC code, the facing applies at once - Unit::SetFacingTo updates the spline), so the cone is centred on
    // that bearing, else on the vehicle's facing. "current target" and "bg siege" are not touched: Hurl Boulder (a
    // siege shot, it ignores the target), the siege retarget (#753) and the drive are unchanged, and nothing chases
    // the rammed unit. Ram's own 4 s cooldown paces it.
    Unit* RamTarget(Player* bot, AiObjectContext* context)
    {
        if (!Enabled() || !bot || !context)
            return nullptr;

        BattlegroundSA* bg = dynamic_cast<BattlegroundSA*>(bot->GetBattleground());
        Vehicle* vehicle = bot->GetVehicle();
        if (!bg || bg->GetStatus() != STATUS_IN_PROGRESS || !vehicle)
            return nullptr;

        Unit* base = vehicle->GetBase();
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!base || base->GetEntry() != NPC_DEMOLISHER_SA || !seat || !seat->CanControl())
            return nullptr;

        PositionInfo siege = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
        float const facing = siege.isSet() ? base->GetAngle(siege.x, siege.y) : base->GetOrientation();

        std::list<Unit*> units;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(base, base, SA_RAM_REACH);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(base, units, check);
        Cell::VisitAllObjects(base, searcher, SA_RAM_REACH);

        Unit* best = nullptr;
        float bestDist = SA_RAM_REACH;
        for (Unit* unit : units)
        {
            if (!unit || unit == base || unit == bot || !unit->IsAlive() || !base->IsValidAttackTarget(unit) ||
                !bot->CanSeeOrDetect(unit))
                continue;

            float const dist = base->GetExactDist(unit);
            if (dist > bestDist)
                continue;

            float const rel = std::remainder(base->GetAngle(unit) - facing, 2.0f * static_cast<float>(M_PI));
            if (std::fabs(rel) > SA_RAM_CONE / 2.0f || !base->IsWithinLOSInMap(unit))
                continue;

            best = unit;
            bestDist = dist;
        }

        if (best && BotDue(bot, 54, 4000))
            TC_LOG_DEBUG("playerbots", "[SOTA] ram-target bot=%s target=%s entry=%u player=%u dist=%.1f siege=%u hp=%.0f",
                         bot->GetName().c_str(), best->GetName().c_str(), best->GetEntry(), best->IsPlayer() ? 1u : 0u,
                         bestDist, siege.isSet() ? 1u : 0u, best->GetHealthPct());

        return best;
    }

    uint32 EffectiveDriveChance() { return DriveChance(); }

    bool StartSpot(Player* bot, BattlegroundSA* bg, TeamId team, uint32 role, Position& spot)
    {
        if (!Enabled() || !bot || !bg || team == TEAM_NEUTRAL)
            return false;

        // Attackers stay where the core put them: on a boat. #736: the boats carry them to the dock (round 1 docks at
        // ~60 s of the 2 min warm-up) and the core no longer lands them - off the docked boat onto the dock (BoatStep),
        // then wait there for the start.
        if (team == bg->Attackers)
        {
            bool acted = false;
            BoatStep(bot, bg, "prep", acted);
            if (Offshore(bot) && BotDue(bot, 36, 60000))
                TC_LOG_DEBUG("playerbots", "[SOTA] boat bot=%s at=%.1f,%.1f,%.1f transport=%u", bot->GetName().c_str(),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetTransport() ? 1u : 0u);
            return false;
        }

        // Defenders "move toward defensive positions" (Wowpedia): the gunners to a front-gate cannon of their lane,
        // the rest to their lane's front gate (#737) - over the wall stairs, not through the closed second-wall gate
        // (DefenderDetour).
        // #750 (owner 2026-10-08): "your team is free to move on the map during the initial countdown ... The defense
        // absolutely must use the time to get in to position" (warcraft.wiki.gg, revid 6865830). The jump off the
        // platform used to wait for STATUS_IN_PROGRESS (AtObjective), so every defender stood on the Purple/Red
        // platform all warm-up; it happens here now. Gunners board their own front cannon (PreferredCannon: "people on
        // the turrets for both the blue and the green gates"), the rest hold behind their lane's front gate, spread.
        if (bot->GetVehicle())
            return false;  // manning a cannon already

        if (bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE)
            return false;  // mid-jump: a MoveTo now would cut the jump short on the platform

        SaState const s = ReadState(bg);
        int8 const lane = DefenderLane(role, s);
        Position dest;
        SaGate const* postGate = nullptr;
        bool beach = false;
        Creature* cannon = role <= SA_ROLE_CANNON_LAST ? PreferredCannon(bot, bg, team, s, role, 500.0f, true) : nullptr;
        if (cannon)
        {
            postGate = GateOfCannon(bg, cannon);
            if (bot->GetExactDist(cannon) <= SA_CANNON_REACH)
            {
                if (!BotDue(bot, 40, 2000))
                    return false;

                PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
                bool const boarded = botAI && Board(bot, botAI->GetAiObjectContext(), cannon, "cannon");
                if (PrepPostDue(bot, bg) || boarded)
                    TC_LOG_DEBUG("playerbots", "[SOTA] prep-post bot=%s role=%u post=turret gate=%s cannon=%u boarded=%u at=%.1f,%.1f,%.1f",
                                 bot->GetName().c_str(), role, postGate ? postGate->name : "-",
                                 cannon->GetGUID().GetCounter(), uint32(boarded), bot->GetPositionX(),
                                 bot->GetPositionY(), bot->GetPositionZ());
                return false;
            }

            dest.Relocate(cannon->GetPositionX(), cannon->GetPositionY(), cannon->GetPositionZ());
        }
        else
        {
            postGate = lane >= 0 ? LaneGate(s, lane) : NearestGate(NextGates(s), bot);
            if (!postGate)
                return false;

            // #736 (owner 2026-10-14; warcraft.wiki.gg "Strand of the Ancients" ===Defensive===: turret crews at the
            // Blue/Green gates, the rest on the beach split between the two landing docks): every non-gunner (and a
            // gunner without a cannon of its lane) waits at its lane's beach post while that lane's front gate stands
            beach = BeachPost(s, lane, role, dest);
            if (!beach)
            {
                // -SA_HOLD_SPREAD / 0 / +SA_HOLD_SPREAD by role (west 4/6/8, east 5/7/9 each take a different one)
                float const lateral = (float((role / 2) % 3) - 1.0f) * SA_HOLD_SPREAD;
                Position const side = GateSide(bg, postGate, bot, SA_GATE_HOLD, lateral);  // #736
                dest.Relocate(side.GetPositionX(), side.GetPositionY(), side.GetPositionZ());
            }

            if (bot->GetExactDist2d(dest.GetPositionX(), dest.GetPositionY()) <= SA_PREP_POST_REACH)
            {
                if (PrepPostDue(bot, bg))
                    TC_LOG_DEBUG("playerbots", "[SOTA] prep-post bot=%s role=%u post=%s gate=%s dist=%.1f at=%.1f,%.1f,%.1f",
                                 bot->GetName().c_str(), role, beach ? "beach" : "gate", postGate->name,
                                 beach ? bot->GetExactDist2d(dest.GetPositionX(), dest.GetPositionY())
                                       : bot->GetExactDist2d(postGate->ground.GetPositionX(), postGate->ground.GetPositionY()),
                                 bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
                return false;  // at its post: stand
            }
        }

        // #750 (wave 20): the warm-up in legs (see SaPrepLeg): ground leg -> real jump -> ground leg
        if (OnPrepLeg(bot, bg, dest))
            return false;  // on its way

        if (LandingHop(bot, bg, "prep"))
            return false;

        Position via;
        bool const detour = DefenderDetour(bot, bg, s, dest, via);
        SaWallRoute const* wall = nullptr;
        if (detour)
            for (SaWallRoute const& route : SA_WALL_ROUTES)
                if (!s.down[route.gateSlot] && route.platform.GetExactDist2d(&via) <= 1.0f)
                    wall = &route;

        if (wall)
        {
            if (WallJump(bot, bg, s, via.GetPositionX(), via.GetPositionY(), "prep"))
                return false;

            // at the take-off: stand until the jump is safe (MountWontCut) and due
            if (bot->GetExactDist2d(wall->platform.GetPositionX(), wall->platform.GetPositionY()) <= SA_PLATFORM_REACH &&
                std::fabs(bot->GetPositionZ() - wall->platform.GetPositionZ()) <= 4.0f)
                return false;
        }

        char const* postKind = cannon ? "turret" : beach ? "beach" : "gate";
        if ((wall || !detour) && LaunchPrepLeg(bot, bg, s, dest, wall ? wall->platform : dest, wall != nullptr, postKind,
                                               postGate, role))
            return false;

        // fallback: MoveTo (BGTactics::moveToStart) the platform / portal / post

        if (BotDue(bot, 53, 15000))
            TC_LOG_DEBUG("playerbots", "[SOTA] prep-step bot=%s role=%u detour=%u to=%.1f,%.1f,%.1f at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), role, uint32(detour), (detour ? via : dest).GetPositionX(),
                         (detour ? via : dest).GetPositionY(), (detour ? via : dest).GetPositionZ(), bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ());
        spot = detour ? via : dest;
        return true;
    }

    bool SelectObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundSA* bg, TeamId team,
                         uint32 role, PositionMap& posMap)
    {
        if (!Enabled() || !bot || !botAI || !context || !bg || team == TEAM_NEUTRAL)
            return false;

        CheckRoundSwap(bot, context, bg);
        SaState const s = ReadState(bg);
        if (team == bg->Attackers)
            return SelectAttacker(bot, botAI, bg, team, role, posMap, s);

        return SelectDefender(bot, context, bg, team, role, posMap, s);
    }

    bool AtObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundSA* bg, TeamId team,
                     uint32 role)
    {
        if (!Enabled() || !bot || !botAI || !context || !bg || bg->GetStatus() != STATUS_IN_PROGRESS ||
            team == TEAM_NEUTRAL || !bot->IsAlive())
            return false;

        ObserveMatch(bot, context, bg);
        CheckRoundSwap(bot, context, bg);

        SaState const s = ReadState(bg);
        bool const attacker = team == bg->Attackers;

        // keep a banner capture going (AC atFlag's keepStationaryWhileCapturing)
        for (CurrentSpellTypes type : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
        {
            Spell* spell = bot->GetCurrentSpell(type);
            if (spell && spell->m_spellInfo &&
                (spell->m_spellInfo->Id == SPELL_CAPTURE_BANNER || spell->m_spellInfo->Id == SA_SPELL_PLACE_SEAFORIUM))
            {
                if (bot->isMoving())
                    bot->StopMoving();
                return true;
            }
        }

        if (Unit* base = bot->GetVehicleBase())
        {
            if (attacker && base->GetEntry() == NPC_DEMOLISHER_SA && botAI->IsInVehicle(true))
                DriveDemolisher(bot, context, bg, base, s, role);
            else if (!attacker && base->GetEntry() == NPC_ANTI_PERSONNAL_CANNON)
                AimCannon(bot, context, bg, base);

            return false;  // the vehicle weapons are their own triggers ("in vehicle")
        }

        if (bot->IsNonMeleeSpellCasted(false))
            return false;

        if (attacker)
        {
            // #736: round 2's boats dock about when the round starts - hold aboard, then off onto the dock
            bool acted = false;
            if (BoatStep(bot, bg, "match", acted))
                return false;
            if (acted)
                return true;

            if (Offshore(bot))
                return false;

            if (s.relic)
            {
                GameObject* relic = SlotObject(bg, BG_SA_TITAN_RELIC);
                if (relic && relic->isSpawned() &&
                    bot->GetExactDist2d(SA_RELIC_FLOOR.GetPositionX(), SA_RELIC_FLOOR.GetPositionY()) <= SA_RELIC_REACH &&
                    BotDue(bot, 37, 3000))
                    return UseObject(bot, relic, "relic");

                return false;
            }

            std::vector<SaGate const*> const gates = NextGates(s);
            // #752: any intact gate in reach - a carrier stopped at a gate in its way (EffectiveGate) plants there
            if (CarryingCharge(bot))
                for (SaGate const& gate : SA_GATES)
                    if (!s.down[gate.slot])
                        if (GameObject* go = SlotObject(bg, gate.slot))
                            if (bot->GetExactDist2d(go) <= SA_PLANT_RANGE)
                                return PlantCharge(bot, go, gate);

            if (SaFlag const* flag = CapturableFlag(bot, bg, s, INTERACTION_DISTANCE))
                if (GameObject* go = SlotObject(bg, flag->slot))
                    return Capture(bot, botAI, context, go, flag->name);

            // drivers, and (#737) a capper with no banner left to take or the escort next to a free demolisher
            bool const mayDrive = role <= SA_ROLE_DRIVER_LAST || role == SA_ROLE_GY_LAST + 1 ||
                                  (role >= SA_ROLE_GY_FIRST && !CapturableFlag(bot, bg, s, 1000.0f));
            if (mayDrive)
                if (Creature* demolisher = FreeDemolisher(bot, bg, team, INTERACTION_DISTANCE + 2.0f))
                    if (BotDue(bot, 38, 2000))
                        return Board(bot, context, demolisher, "demolisher");

            if (role >= SA_ROLE_BOMB_FIRST && role <= SA_ROLE_BOMB_LAST && !CarryingCharge(bot) && !gates.empty())
                if (GameObject* pile = NearestBombPile(bot, bg, INTERACTION_DISTANCE))
                    if (BotDue(bot, 39, 3000))
                        return UseObject(bot, pile, "bomb-pickup");

            return false;
        }

        // #750: a defender whose post was not logged in the warm-up says once at round start where it is: at the post
        // of its warm-up route (it arrived after the last 30 s call), or post=none (with how far it still had to go)
        if (PrepPostDue(bot, bg))
        {
            SaPrepLeg leg;
            bool const routed = PrepLegOf(bot, bg, leg);
            float const left = routed ? bot->GetExactDist2d(leg.post.GetPositionX(), leg.post.GetPositionY()) : -1.0f;
            bool const there = routed && left <= (std::strcmp(leg.postKind, "turret") ? SA_PREP_POST_REACH : SA_CANNON_REACH);
            TC_LOG_DEBUG("playerbots", "[SOTA] prep-post bot=%s role=%u post=%s gate=%s at-start=1 route=%u left=%.1f moving=%u at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), role, there ? leg.postKind : "none", routed ? leg.gate : "-",
                         uint32(routed), left, uint32(bot->isMoving()), bot->GetPositionX(), bot->GetPositionY(),
                         bot->GetPositionZ());
        }

        // #750 (wave 20): off a ledge a cut jump left it on
        if (LandingHop(bot, bg, "match"))
            return true;

        // #750: a gunner next to its front cannon boards it before anything else - BG_SA_GUN_2 stands 0.7 yd from the
        // Green wall-route platform, so the wall jump below would otherwise throw its gunner off the wall
        if (role <= SA_ROLE_CANNON_LAST && !s.relic)
            if (Creature* cannon = FreeCannon(bot, bg, team, s, SA_CANNON_REACH))
                if (BotDue(bot, 40, 2000))
                    return Board(bot, context, cannon, "cannon");

        // #737: on a wall platform the objective was set to (DefenderDetour) - jump down in front of the cannon (WallJump)
        CheckPortalUse(bot);
        PositionInfo objective = context->GetValue<PositionMap&>("position")->Get()["bg objective"];  // isSet() is non-const
        if (objective.isSet())
        {
            if (WallJump(bot, bg, s, objective.x, objective.y, "match"))
            {
                ResetPos(context, "bg objective");
                return true;
            }

            // ... or at the Defender's Portal it was sent to: use it (54640 -> 54643 teleports to the 23472 trigger
            // behind the gate, see SA_SPELL_DEFENDER_PORTAL_LOCK). Under the 10 s 54643 lock the cast is refused
            // (spell_wintergrasp_defender_teleport CheckCast), so wait at the portal until it expires.
            for (SaGate const& info : SA_GATES)
            {
                GameObject* portal = SlotObject(bg, PortalSlotOf(info.slot));
                if (!portal || s.down[info.slot] || portal->GetExactDist2d(objective.x, objective.y) > 1.0f ||
                    bot->GetExactDist2d(portal) > INTERACTION_DISTANCE ||
                    bot->HasAura(SA_SPELL_DEFENDER_PORTAL_LOCK) || !BotDue(bot, 45, 5000))
                    continue;

                NotePortalUse(bot);
                ResetPos(context, "bg objective");
                return UseObject(bot, portal, "portal");
            }
        }

        // #751: enemy demolishers near the gates before players, unless a player is attacking the bot (DefenderEngage;
        // the old "current target" hand-off here was read by nothing)
        DefenderEngage(bot, botAI, context, bg, team, role, s);
        return false;
    }

    void ObserveEnd(Player* bot, BattlegroundSA* bg)
    {
        if (!Enabled() || !bot || !bg || !LogDue(bg->GetInstanceID(), 99, 0))
            return;

        TC_LOG_DEBUG("playerbots", "[SOTA] result instance=%u winner=%u last-attackers=%s", bg->GetInstanceID(),
                     uint32(bg->GetWinner()), TeamName(bg->Attackers));
    }
}
