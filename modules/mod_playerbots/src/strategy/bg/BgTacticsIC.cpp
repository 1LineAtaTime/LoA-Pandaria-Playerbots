/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Isle of Conquest (map 628) play beyond AC mod-playerbots - issue #675 (epic #205), build 14.
//
// AC's IoC tactics (BattleGroundTactics.cpp BATTLEGROUND_IC cases, AC Ai/Base/Actions/BattleGroundTactics.cpp
// 2843-3168, ported 1:1 in #393 plus AC PR mod-playerbots#2856) capture the nodes, drive the Demolishers and
// Glaive Throwers to the enemy front gate and fire at it, and go for the keep banner and the general once a gate
// is down. AC leaves out, on purpose (AC VehicleActions.cpp:51-55: "dont let them get in the cannons ... dont
// let them in catapult"), and by accident (AC EnterVehicleAction skips any vehicle with a passenger, and the
// Siege Engine always carries its three turret accessories, vehicle_template_accessory 34776/35069):
//   - the keep cannons,                        -> defenders man them while enemy siege vehicles close in;
//   - the catapults,                           -> an attacker launches itself over the enemy wall;
//   - the seaforium bombs,                     -> attackers carry the Workshop bombs to the enemy front gate,
//                                                 and a bot inside the enemy keep plants a Huge Seaforium Bomb
//                                                 on the gate from the inside;
//   - the Siege Engine driver seat,            -> one attacker drives it (AC's own siege-engine branch then
//                                                 rams the gate);
//   - the hangar gunship and its parachutes.   -> gated off, see IOC_GUNSHIP_ROUTE.
// Each feature follows AC's own pattern: SelectObjective sets the "bg objective" (and, for a launch, the
// "bg siege") position the way AC selectObjective does, BGTactics walks/drives there, and AtObjective acts once
// the bot is there, the way AC atFlag does. Every hook returns false when AC's play applies.
//
// Mechanics and ids (sources, 2026-10-06):
//   - Wowpedia "Isle of Conquest" (MediaWiki API wikitext): keep cannons on the keep walls for the defenders,
//     catapults "launch your invading party over the walls", Workshop Seaforium Bombs are planted (10 s fuse)
//     at a keep gate, "Attackers may use their enemy's own bombs [Huge Seaforium Bombs] to destroy Keep Gates
//     from within", the Siege Engine is repaired 3 minutes after the Workshop is taken, gunship parachutes.
//   - Wowhead mop-classic tooltips: 66218 Launch ("Launches the driver towards the designated location"),
//     66268 Place Seaforium Bomb / 66674 Place Huge Seaforium Bomb (detonate after 10 s), 66265 / 66669 (the
//     bomb piles' create-item spells), 68077 Repair Cannon (8 s channel), 67452 Rocket Blast, 66541 Incendiary
//     Rocket, 66629/66638 Gunship Portal Click, 66656/66657 Parachute.
//   - MoP DBC (testserver data-mop-extract): 66218 targets DEST (0x40), E3 dummy at DEST_TRAJ + E140 force-cast
//     66224 (aura 236 control vehicle, amount 2 = seat 1); 66268/66674 summon GO 195235/195331 in front of the
//     caster; blasts 66676/66672 GAMEOBJECT_DAMAGE 4712/9424 in 10 yd. VehicleSeat: keep cannon 510 is
//     FIXED_POSITION with a CAN_CONTROL|CAN_CAST seat (so AC treats a gunner as "not the driver"); catapult 438
//     seat 0 drives, seat 1 is the launch seat.
//   - Core: BattlegroundIC.cpp (slots, node states, DestroyGate -> portcullis open), isle_of_conquest.cpp
//     (spell_ioc_launch: 1.5 s after Launch the seat-1 rider is ejected and MoveJump'ed to the spell's
//     destination; npc_ioc_keep_cannon: a "dead" cannon becomes 35819 with feign death and is repaired through
//     its spellclick 68077), world DB npc_spellclick_spells (68458 board cannon, 66245 ride, 46598 siege engine),
//     item_template 46847 Seaforium Bombs / 47030 Huge Seaforium Bombs.
//   - Every position below is on the MoP map-628 navmesh (mmtile_probe.py, tol 4 yd); the keep-cannon slots
//     are all reachable except cannon 10 (an isolated 2-poly island, excluded).
//
// Roles: the "bg role" value (0-9, pinned to i % 10 by the 1:1 harness). AC's IoC split is role < 3 docks,
// 3-5 hangar, 9 side base, 6-8 workshop then guard (BattleGroundTactics.cpp IC case). Beyond AC:
//   role 1 (a docks role)      -> catapult launcher, while our team holds the Docks;
//   role 3 (a hangar role)     -> Siege Engine driver, while our team holds the Workshop;
//   role 4 (a hangar role)     -> gunship paratrooper, while our team holds the Hangar (IOC_GUNSHIP_ROUTE);
//   roles 6, 7 (workshop)      -> Workshop seaforium bombs on the enemy front gate;
//   role 8 (workshop/guard)    -> keep cannon, while enemy siege vehicles are within 170 yd of our front gate;
//   #675 (2026-10-21) siege drivers once their node is ours: roles 0/2 a Glaive Thrower (Docks), role 8 (when no
//                                 cannon is needed) / 5 (Hangar ours) / 9 (side base ours) a Demolisher (Workshop);
//                                 each vehicle is sent to a gate of the enemy keep (front / west / east, PickGate);
//   roles 0, 5 / role 2        -> (#738) once OUR keep is breached: hold the command room by our general / hold or
//                                 retake our Keep Graveyard banner; any bot inside our breached keep while the
//                                 general is attacked or an enemy is inside falls back to the general (KeepDefence).
// Any bot inside the enemy keep while its gates stand (after a launch or a drop) goes for a Huge Seaforium Bomb.
// Once the enemy keep is breached (AC's portcullis test) the bomb and catapult objectives stop and AC's
// boss/keep-banner play takes over.
//
// Evidence lines (TC_LOG_DEBUG "playerbots", MoP-only - AC has none of this): "[IOC] <event> ..." with event
// board, launch, bomb-pickup, bomb-plant, repair, leave-cannon, portal, drop, gate-hp, gate-down,
// keep-breached, boss-hp, keep-defend (#738); #675: gate-pick, drive, siege-park, vehicle-target, siege-retarget, gates,
// ram-target.

#include "BattleGroundTactics.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundIC.h"
#include "BgTacticsIC.h"
#include "CellImpl.h"
#include "Creature.h"
#include "GameObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Item.h"
#include "Map.h"
#include "MotionMaster.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellHistory.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Transport.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <atomic>
#include <cmath>
#include <map>
#include <mutex>
#include <utility>

namespace
{
    std::atomic<bool> g_iocEnabled{true};

    // The hangar gunship + parachute route. OFF until the PlayerbotAI.cpp transport-loop guard staged in
    // ioc-reg.md is applied: PlayerbotAI::UpdateAI re-probes every bot for a transport once a second
    // (Map::GetTransportForPos casts a 30 yd ray down from the bot) and AddPassenger + StopOnCurrentPos a bot
    // whose ray hits a hull - which is exactly where a bot is during the first second of a drop, so without the
    // guard the dropping bot is re-attached to the gunship and snapped back onto the deck.
    constexpr bool IOC_GUNSHIP_ROUTE = false;

    // Ids the core does not export in a header (isle_of_conquest.cpp, item_template).
    constexpr uint32 IOC_ITEM_SEAFORIUM_BOMBS = 46847;       // on use 66268 Place Seaforium Bomb
    constexpr uint32 IOC_ITEM_HUGE_SEAFORIUM_BOMBS = 47030;  // on use 66674 Place Huge Seaforium Bomb
    constexpr uint32 IOC_SPELL_LAUNCH = 66218;               // catapult seat 0 (spell_ioc_launch)
    constexpr uint32 IOC_NPC_BROKEN_KEEP_CANNON = 35819;     // npc_ioc_keep_cannon, KeepCannonEnums

    constexpr uint32 IOC_ROLE_CATAPULT = 1;
    constexpr uint32 IOC_ROLE_SIEGE_ENGINE = 3;
    constexpr uint32 IOC_ROLE_GUNSHIP = 4;
    constexpr uint32 IOC_ROLE_BOMB_A = 6;
    constexpr uint32 IOC_ROLE_BOMB_B = 7;
    constexpr uint32 IOC_ROLE_CANNON = 8;

    // Enemy siege vehicles this close to our front gate put the keep cannons to use. AC PR #2856 parks the
    // Demolishers 59-65 yd from a front gate; AC's original Horde spot was 94 yd; the Glaive Throwers fire from
    // the peninsulas (Wowpedia) - 170 yd covers all of them.
    constexpr float IOC_THREAT_RANGE = 170.0f;
    // INTERACTION_DISTANCE plus the 2 yd between the wall-top navmesh and the cannons standing on the
    // battlements (keep cannons 5/6 are 4.5-5.4 yd from the nearest walkable poly).
    constexpr float IOC_CANNON_REACH = INTERACTION_DISTANCE + 2.0f;
    constexpr float IOC_PLANT_RADIUS = 4.0f;    // bomb plant spot reached (BGTactics stops within 4 yd)
    constexpr float IOC_LAUNCH_RADIUS = 10.0f;  // catapult launch spot reached
    constexpr float IOC_LAUNCH_PITCH = 1.0f;    // 57 deg: the arc clears the 39 yd keep wall (apex ~48 yd)
    constexpr float IOC_GRAVITY = 19.29f;       // Movement::gravity (MoveSplineInitArgs/MovementTypedefs.h)
    constexpr float IOC_GUNSHIP_DROP_RANGE = 125.0f;  // the gunship paths pass 90-120 yd from the drop points
    constexpr float IOC_GLIDE_SPEED = 8.0f;           // horizontal yd/s of the parachute glide

    struct IocKeep
    {
        TeamId owner;
        uint32 frontGateSlot;      // the gate AC's vehicles attack (BG_IC_GO_ALLIANCE_GATE_3 / BG_IC_GO_HORDE_GATE_1)
        Position outsideBombSpot;  // 4 yd outside the front gate, toward the Workshop
        Position insideBombSpot;   // 5 yd inside the front gate
        Position landing;          // catapult landing point in the courtyard
        Position dropPoint;        // gunship drop point in the courtyard
        Position launchSpot;       // where the ATTACKERS' catapult launches from (AC / PR #2856 gate-attack spot)
        uint32 portcullisSlot;     // AC's "gate open" object (opens in BattlegroundIC::DestroyGate)
        uint32 cannonFirst;        // BG_IC_NPC_KEEP_CANNON_* on this keep's walls
        uint32 cannonLast;
        uint32 hugeBombFirst;      // BG_IC_GO_HUGE_SEAFORIUM_BOMBS_* inside this keep
        uint32 hugeBombLast;
        uint32 bossSlot;           // this keep's general
    };

    IocKeep const IOC_KEEPS[2] = {
        {TEAM_ALLIANCE, BG_IC_GO_ALLIANCE_GATE_3, Position(417.5f, -833.6f, 48.3f), Position(408.5f, -833.9f, 48.8f),
         Position(345.0f, -833.0f, 49.1f), Position(320.0f, -790.0f, 49.1f), Position(478.3f, -830.2f, 33.6f),
         BG_IC_GO_DOODAD_PORTCULLISACTIVE02, BG_IC_NPC_KEEP_CANNON_1, BG_IC_NPC_KEEP_CANNON_12,
         BG_IC_GO_HUGE_SEAFORIUM_BOMBS_A_1, BG_IC_GO_HUGE_SEAFORIUM_BOMBS_A_4, BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE},
        {TEAM_HORDE, BG_IC_GO_HORDE_GATE_1, Position(1146.9f, -763.0f, 48.7f), Position(1155.9f, -762.8f, 48.8f),
         Position(1215.0f, -765.0f, 49.0f), Position(1250.0f, -695.0f, 49.0f), Position(1091.273f, -763.619f, 42.352f),
         BG_IC_GO_HORDE_KEEP_PORTCULLIS, BG_IC_NPC_KEEP_CANNON_13, BG_IC_NPC_KEEP_CANNON_24,
         BG_IC_GO_HUGE_SEAFORIUM_BOMBS_H_1, BG_IC_GO_HUGE_SEAFORIUM_BOMBS_H_4, BG_IC_NPC_OVERLORD_AGMAR}};

    uint32 const IOC_GATE_SLOTS[6] = {BG_IC_GO_ALLIANCE_GATE_1, BG_IC_GO_ALLIANCE_GATE_2, BG_IC_GO_ALLIANCE_GATE_3,
                                      BG_IC_GO_HORDE_GATE_1,    BG_IC_GO_HORDE_GATE_2,    BG_IC_GO_HORDE_GATE_3};

    TeamId Other(TeamId team) { return team == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE; }
    IocKeep const& KeepOf(TeamId owner) { return IOC_KEEPS[owner == TEAM_ALLIANCE ? 0 : 1]; }
    uint32 FactionOf(TeamId team) { return BG_IC_Factions[team == TEAM_ALLIANCE ? 0 : 1]; }
    uint32 Controlled(TeamId team) { return team == TEAM_ALLIANCE ? NODE_STATE_CONTROLLED_A : NODE_STATE_CONTROLLED_H; }
    char const* TeamName(TeamId team) { return team == TEAM_ALLIANCE ? "alliance" : "horde"; }

    // Battleground::GetBGCreature / GetBGObject log an ERROR for every empty slot (Battleground.cpp:2013-2029),
    // and the vehicle, bomb and portal slots stay empty until their node is held - so read the slots quietly.
    Creature* SlotCreature(BattlegroundIC* bg, uint32 slot)
    {
        if (slot >= bg->BgCreatures.size() || bg->BgCreatures[slot].IsEmpty())
            return nullptr;

        BattlegroundMap* map = bg->FindBgMap();
        return map ? map->GetCreature(bg->BgCreatures[slot]) : nullptr;
    }

    GameObject* SlotObject(BattlegroundIC* bg, uint32 slot)
    {
        if (slot >= bg->BgObjects.size() || bg->BgObjects[slot].IsEmpty())
            return nullptr;

        BattlegroundMap* map = bg->FindBgMap();
        return map ? map->GetGameObject(bg->BgObjects[slot]) : nullptr;
    }

    // AC's keep boxes (IsLockedInsideKeep, AC BattleGroundTactics.cpp:4120-4127).
    bool InsideKeep(TeamId keepOwner, WorldObject const* obj)
    {
        float const x = obj->GetPositionX();
        float const y = obj->GetPositionY();
        if (keepOwner == TEAM_ALLIANCE)
            return x < 410.0f && y > -900.0f && y < -765.0f;

        return x > 1153.0f && y > -849.0f && y < -679.0f;
    }

    // AC's "gate open" test (selectObjective IC case): the keep's inner portcullis is activated, which the core
    // does in DestroyGate as soon as any of that keep's three gates falls.
    bool KeepBreached(BattlegroundIC* bg, TeamId keepOwner)
    {
        GameObject* portcullis = SlotObject(bg, KeepOf(keepOwner).portcullisSlot);
        return portcullis && portcullis->isSpawned() && portcullis->getLootState() == GO_ACTIVATED;
    }

    bool GateStands(GameObject* gate) { return gate && gate->GetDestructibleState() != GO_DESTRUCTIBLE_DESTROYED; }

    std::mutex g_iocLogMutex;
    std::map<std::pair<uint32, uint32>, uint32> g_iocLogTimes;  // (bg instance, key) -> getMSTime()

    // true once per everyMs (0 = once per instance) for one (battleground instance, key): evidence lines that
    // every bot of the match could write stay at one line per event.
    bool LogDue(uint32 instanceId, uint32 key, uint32 everyMs)
    {
        std::lock_guard<std::mutex> lock(g_iocLogMutex);
        uint32 const now = getMSTime();
        auto itr = g_iocLogTimes.find(std::make_pair(instanceId, key));
        if (itr == g_iocLogTimes.end())
        {
            g_iocLogTimes[std::make_pair(instanceId, key)] = now;
            return true;
        }

        if (!everyMs || getMSTimeDiff(itr->second, now) < everyMs)
            return false;

        itr->second = now;
        return true;
    }

    // Per-bot retry spacing for actions that fail quietly when the core refuses them (a launch with no energy,
    // a spellclick on a cannon someone else just took). Map-thread local, like BGTactics' s_bgPathChoice.
    bool BotDue(Player* bot, uint32 key, uint32 everyMs)
    {
        static thread_local std::map<std::pair<ObjectGuid, uint32>, uint32> s_last;
        uint32 const now = getMSTime();
        auto itr = s_last.find(std::make_pair(bot->GetGUID(), key));
        if (itr != s_last.end() && getMSTimeDiff(itr->second, now) < everyMs)
            return false;

        s_last[std::make_pair(bot->GetGUID(), key)] = now;
        return true;
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

    // An enemy Demolisher, Siege Engine, Glaive Thrower or Catapult with a driver (seat 0) near our front gate.
    // Seat 0 rather than Vehicle::IsVehicleInUse: the Siege Engine's turret accessories always fill its other
    // seats. Slots BG_IC_NPC_SIEGE_ENGINE_A .. BG_IC_NPC_CATAPULT_4_H are contiguous (BattlegroundIC.h).
    bool OwnKeepThreatened(BattlegroundIC* bg, TeamId team)
    {
        GameObject* gate = SlotObject(bg, KeepOf(team).frontGateSlot);
        if (!gate)
            return false;

        uint32 const enemyFaction = FactionOf(Other(team));
        for (uint32 slot = BG_IC_NPC_SIEGE_ENGINE_A; slot <= BG_IC_NPC_CATAPULT_4_H; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!vehicle || !vehicle->IsAlive() || vehicle->GetFaction() != enemyFaction || !vehicle->GetVehicleKit())
                continue;

            Unit* driver = vehicle->GetVehicleKit()->GetPassenger(0);
            if (driver && driver->GetTypeId() == TYPEID_PLAYER &&
                vehicle->GetExactDist2d(gate->GetPositionX(), gate->GetPositionY()) < IOC_THREAT_RANGE)
                return true;
        }

        return false;
    }

    // #738 (owner 2026-10-08): our keep is breached - defend it. Beyond AC: AC's IoC tactics only attack (the enemy
    // keep banner, then the general, BattleGroundTactics.cpp IC case), and AC's IsLockedInsideKeep sends a bot inside
    // a breached keep with no teleporter in sight back to its start ("!gateLock && !closestPortal -> moveToStart",
    // AC BattleGroundTactics.cpp:4222-4224), which made bots run back and forth inside the keep. Intended design
    // (sources, 2026-10-08):
    //   - warcraft.wiki.gg "Horde Keep" / "Alliance Keep" (MediaWiki API wikitext): one destroyed keep gate opens both
    //     the general's command room and the Keep Graveyard banner in the courtyard; "Capturing this graveyard can
    //     delay [the] defenders significantly, while providing [the] attackers with a much closer graveyard"; the keep
    //     cannons cannot fire at "intruders already inside the courtyard"; defenders get Honorable Defender in the
    //     courtyard and the command room.
    //   - warcraft.wiki.gg "Isle of Conquest": "failure to protect your leader will bring shame, dishonor, and defeat";
    //     "Overlord Agmar (tactics)" / "High Commander Halford Wyrmbane (tactics)": the general enrages if pulled out
    //     of his command room (Rage 66776) - he is fought, and so defended, in that room.
    //   - us.forums.blizzard.com "Isle Of Conquest for Dummies" (2019-04-05): "Capturing the enemy keep [graveyard]
    //     allows you to rez at the boss and is vital"; the attackers' sieges, once the gates go down, "clear the
    //     defense, take the gy AND ... hold off anyone trying to mess with the offense attacking the boss".
    // So once our keep is open (AC's own portcullis test, KeepBreached), extending AC's role split:
    //   roles 0 and 5 (one docks, one hangar role) hold the command room by our general;
    //   role 2 (docks) holds our Keep Graveyard banner, and retakes it (AC atFlag captures it, vFlagsIC) if the
    //   attackers have it or are taking it;
    //   any other bot that is inside our keep while the general is under attack or an enemy player is inside it
    //   falls back to the general instead of walking out (the intruders are there).
    // Everyone else keeps AC's play (the attack - the attackers keep AC's banner-then-general order).
    constexpr uint32 IOC_ROLE_KEEP_GUARD_A = 0;
    constexpr uint32 IOC_ROLE_KEEP_GUARD_B = 5;
    constexpr uint32 IOC_ROLE_KEEP_BANNER = 2;
    constexpr float IOC_GENERAL_HOLD = 6.0f;   // jitter around the general (command room)
    constexpr float IOC_BANNER_HOLD = 5.0f;    // jitter around our own keep banner while it is ours

    bool KeepDefence(Player* bot, AiObjectContext* context, BattlegroundIC* bg, TeamId team, uint32 role,
                     PositionMap& posMap)
    {
        if (!KeepBreached(bg, team))
            return false;

        IocKeep const& own = KeepOf(team);
        Creature* general = SlotCreature(bg, own.bossSlot);
        if (!general || !general->IsAlive())
            return false;

        uint32 const mapId = bot->GetMapId();
        char const* why = nullptr;
        if (role == IOC_ROLE_KEEP_BANNER)
        {
            uint32 const node = team == TEAM_ALLIANCE ? NODE_TYPE_GRAVEYARD_A : NODE_TYPE_GRAVEYARD_H;
            if (GameObject* banner = SlotObject(bg, team == TEAM_ALLIANCE ? BG_IC_GO_ALLIANCE_BANNER : BG_IC_GO_HORDE_BANNER))
            {
                if (bg->GetNodeState(node) != Controlled(team))
                {
                    SetPos(posMap, "bg objective", *banner, mapId);
                    why = "banner-retake";
                }
                else
                {
                    SetPos(posMap, "bg objective",
                           Position(banner->GetPositionX() + frand(-IOC_BANNER_HOLD, IOC_BANNER_HOLD),
                                    banner->GetPositionY() + frand(-IOC_BANNER_HOLD, IOC_BANNER_HOLD),
                                    banner->GetPositionZ()),
                           mapId);
                    why = "banner";
                }
            }
        }

        if (!why)
        {
            bool collapse = false;
            if (InsideKeep(team, bot))
            {
                Unit* intruder = context->GetValue<Unit*>("enemy player target")->Get();
                collapse = general->IsInCombat() || (intruder && intruder->IsAlive() && InsideKeep(team, intruder));
            }

            if (role != IOC_ROLE_KEEP_GUARD_A && role != IOC_ROLE_KEEP_GUARD_B && !collapse)
                return false;

            SetPos(posMap, "bg objective",
                   Position(general->GetPositionX() + frand(-IOC_GENERAL_HOLD, IOC_GENERAL_HOLD),
                            general->GetPositionY() + frand(-IOC_GENERAL_HOLD, IOC_GENERAL_HOLD), general->GetPositionZ()),
                   mapId);
            why = collapse && role != IOC_ROLE_KEEP_GUARD_A && role != IOC_ROLE_KEEP_GUARD_B ? "collapse" : "general";
        }

        if (BotDue(bot, 8, 15000))
            TC_LOG_DEBUG("playerbots", "[IOC] keep-defend bot=%s team=%s role=%u as=%s general-hp=%.0f combat=%u at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), TeamName(team), role, why, general->GetHealthPct(),
                         uint32(general->IsInCombat()), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    // Nearest usable keep cannon on our own walls: a working one with a free seat first, then a broken one
    // (35819, feign death) to repair. Cannon 10 is off the navmesh and never picked.
    Creature* FreeKeepCannon(Player* bot, BattlegroundIC* bg, TeamId team, float maxDist)
    {
        IocKeep const& keep = KeepOf(team);
        for (int pass = 0; pass < 2; ++pass)
        {
            Creature* best = nullptr;
            float bestDist = maxDist;
            for (uint32 slot = keep.cannonFirst; slot <= keep.cannonLast; ++slot)
            {
                if (slot == BG_IC_NPC_KEEP_CANNON_10)
                    continue;

                Creature* cannon = SlotCreature(bg, slot);
                if (!cannon || !cannon->IsAlive() || !cannon->GetVehicleKit())
                    continue;

                bool const broken = cannon->GetEntry() == IOC_NPC_BROKEN_KEEP_CANNON;
                if (broken != (pass == 1))
                    continue;

                if (!broken && (cannon->GetVehicleKit()->IsVehicleInUse() || !cannon->GetVehicleKit()->GetAvailableSeatCount()))
                    continue;

                float const dist = bot->GetExactDist(cannon);
                if (dist < bestDist)
                {
                    bestDist = dist;
                    best = cannon;
                }
            }

            if (best)
                return best;
        }

        return nullptr;
    }

    // Nearest of our team's vehicles in [first, last] whose driver seat (0) is free and that can be clicked.
    Creature* FreeOwnVehicle(Player* bot, BattlegroundIC* bg, TeamId team, uint32 first, uint32 last, float maxDist)
    {
        Creature* best = nullptr;
        float bestDist = maxDist;
        uint32 const ownFaction = FactionOf(team);
        for (uint32 slot = first; slot <= last; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!vehicle || !vehicle->IsAlive() || vehicle->GetFaction() != ownFaction || !vehicle->GetVehicleKit())
                continue;

            // the Siege Engine stays NOT_SELECTABLE through its 3 minute assembly (BattlegroundIC.cpp
            // HandleCapturedNodes / PostUpdateImpl)
            if (vehicle->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || !vehicle->GetVehicleKit()->HasEmptySeat(0))
                continue;

            float const dist = bot->GetExactDist(vehicle);
            if (dist < bestDist)
            {
                bestDist = dist;
                best = vehicle;
            }
        }

        return best;
    }

    Creature* FreeCatapult(Player* bot, BattlegroundIC* bg, TeamId team, float maxDist)
    {
        return team == TEAM_ALLIANCE
                   ? FreeOwnVehicle(bot, bg, team, BG_IC_NPC_CATAPULT_1_A, BG_IC_NPC_CATAPULT_4_A, maxDist)
                   : FreeOwnVehicle(bot, bg, team, BG_IC_NPC_CATAPULT_1_H, BG_IC_NPC_CATAPULT_4_H, maxDist);
    }

    Creature* FreeSiegeEngine(Player* bot, BattlegroundIC* bg, TeamId team, float maxDist)
    {
        uint32 const slot = team == TEAM_ALLIANCE ? BG_IC_NPC_SIEGE_ENGINE_A : BG_IC_NPC_SIEGE_ENGINE_H;
        return FreeOwnVehicle(bot, bg, team, slot, slot, maxDist);
    }

    // #675 (owner 2026-10-21: the SotA vehicle play "works on SotA. You may just need to update/reuse it for IoC maps/
    // gates/mechanics/strategy"): the Demolishers (Workshop) and the Glaive Throwers (Docks). AC boards them only when a
    // bot passes within 40 yd of a free one (AC EnterVehicleAction, VehicleActions.cpp:43-71) and sends every vehicle
    // to the front gate (AC BattleGroundTactics.cpp IC case, "attack gates", PR #2856 parks). After the SotA demolisher
    // pattern (BgTacticsSA.cpp FreeDemolisher / SelectAttacker / DriveDemolisher, #737 lanes, #753 retarget):
    //   - drivers by role, once their node is ours and AC's capture order is done with them: roles 0 and 2 (AC's docks
    //     roles) take a Glaive Thrower; role 8 (workshop) a Demolisher while the keep cannons do not need it; role 5
    //     (hangar) once the Hangar is ours; role 9 once its side base (Quarry / Refinery, AC's role-9 node) is ours;
    //   - a gate per vehicle (PickGate, sticky per vehicle): the first IOC_FRONT_DEMOLISHERS driven Demolishers go to the
    //     front gate (AC's park), the next ones to the side gate with the fewest of ours (split, as SotA's lanes); a Glaive
    //     Thrower to the Docks-side (west) gate; every vehicle switches to an enemy gate at half health or less (focus -
    //     one fallen gate opens the whole keep, BattlegroundIC::DestroyGate -> the portcullis AC tests);
    //   - each gate has a park where the vehicle stops and fires (the siege shot needs <= 120 yd 2D and a reachable arc,
    //     PlayerbotAI::CanCastVehicleSpell / SiegeShotArc). The front parks are AC's IC_GATE_ATTACK_POS_*; the side parks
    //     are on the map-628 navmesh (mmtile_probe.py, testserver data-mop-extract mmaps) and dtsim.py find_path reaches
    //     them from the Workshop and Docks vehicle spawns (BattlegroundIC.h BG_IC_WorkshopVehicles /
    //     BG_IC_DocksVehiclesGlaives) without crossing a keep box (2026-10-21);
    //   - an enemy driven siege vehicle within IOC_VEHICLE_ENGAGE of ours takes the "bg siege" point, then the gate again
    //     (AimSiege, every tick, the #753 pattern): the lobbed Hurl Boulder (67440 -> 67439) and Glaive Throw (66456 /
    //     67034 -> 66289) also deal SCHOOL_DAMAGE to the enemies at the impact (5.4.8 SpellEffect.dbc: effect 2 at
    //     TARGET_UNIT_DEST_AREA_ENEMY), and IsSiegeShot aims them at "bg siege".
    // Left to AC: the Siege Engine's front-gate ram (#2856; role 3 boards it above), every vehicle once the enemy keep is
    // breached (a driver that got out there would be put back in by AC's "enter vehicle" timer within 40 yd), and every
    // vehicle with no free driver role. Evidence: board what=demolisher|glaive, gate-pick, drive, siege-park,
    // vehicle-target, siege-retarget, gates.
    constexpr uint32 IOC_ROLE_GLAIVE_A = 0;
    constexpr uint32 IOC_ROLE_GLAIVE_B = 2;
    constexpr uint32 IOC_ROLE_DEMOLISHER_HANGAR = 5;
    constexpr uint32 IOC_ROLE_DEMOLISHER_WORKSHOP = 8;
    constexpr uint32 IOC_ROLE_DEMOLISHER_SIDE = 9;
    constexpr uint32 IOC_FRONT_DEMOLISHERS = 2;     // driven vehicles of ours on the front gate before the split starts
    constexpr float IOC_PARK_HOLD = 8.0f;           // AC: "just make bot stay where it is if already close"
    constexpr float IOC_PARK_JITTER_FRONT = 5.0f;   // AC's frand(-5, 5) around the front park
    constexpr float IOC_PARK_JITTER_SIDE = 3.0f;    // the side parks stand on narrower ground (Horde east: a ledge)
    constexpr float IOC_VEHICLE_ENGAGE = 50.0f;     // an enemy driven siege vehicle this close is shot first
    constexpr float IOC_FOCUS_HP = 0.5f;            // a gate at or below this health draws every vehicle ...
    constexpr float IOC_FOCUS_MARGIN = 0.25f;       // ... unless the vehicle's own gate is within this much of it
    constexpr float IOC_SIEGE_GATE_MATCH = 20.0f;   // 2D, the "bg siege" point belongs to the gate this close to it
    // Ram (67441, 5.4.8 DBC): knockback + SCHOOL_DAMAGE on the enemies (TARGET_UNIT_CONE_ENEMY_104) and GAMEOBJECT_DAMAGE
    // (cone 108) within SpellRadius 37 = 7 yd, SpellRange 141 = 7 yd, 4 s cooldown (SpellCooldowns 5108). No
    // SpellTargetRestrictions row, so a creature caster's cone is M_PI / 2 wide (as SotA's Ram 60206, BgTacticsSA.cpp).
    constexpr float IOC_RAM_REACH = 7.0f;
    constexpr float IOC_RAM_CONE = static_cast<float>(M_PI) / 2.0f;

    constexpr uint8 IOC_GATE_FRONT = 0;
    constexpr uint8 IOC_GATE_WEST = 1;   // WoW +y is west: the Docks side of both keeps
    constexpr uint8 IOC_GATE_EAST = 2;   // the Hangar side

    struct IocGate
    {
        uint32 slot;       // BG_IC_GO_*_GATE_* (BattlegroundIC.h BG_IC_ObjSpawnlocs)
        TeamId owner;      // the keep the gate closes
        uint8 side;        // IOC_GATE_FRONT / WEST / EAST
        char const* name;
        Position park;     // where an attacking Demolisher / Glaive Thrower stops and fires
    };

    // Order: the Alliance keep's front, west, east, then the Horde keep's (the "[IOC] gates" line keeps it).
    IocGate const IOC_GATES[6] = {
        {BG_IC_GO_ALLIANCE_GATE_3, TEAM_ALLIANCE, IOC_GATE_FRONT, "a-front", Position(478.3f, -830.2f, 33.6f)},     // AC IC_GATE_ATTACK_POS_HORDE (z: navmesh)
        {BG_IC_GO_ALLIANCE_GATE_1, TEAM_ALLIANCE, IOC_GATE_WEST, "a-west", Position(373.0f, -716.0f, 51.0f)},       // 51 yd N of the gate
        {BG_IC_GO_ALLIANCE_GATE_2, TEAM_ALLIANCE, IOC_GATE_EAST, "a-east", Position(351.0f, -963.3f, 48.2f)},       // 60 yd S
        {BG_IC_GO_HORDE_GATE_1, TEAM_HORDE, IOC_GATE_FRONT, "h-front", Position(1091.273f, -763.619f, 42.352f)},   // AC IC_GATE_ATTACK_POS_ALLIANCE
        {BG_IC_GO_HORDE_GATE_3, TEAM_HORDE, IOC_GATE_WEST, "h-west", Position(1218.0f, -627.0f, 45.4f)},           // 50 yd N
        {BG_IC_GO_HORDE_GATE_2, TEAM_HORDE, IOC_GATE_EAST, "h-east", Position(1218.0f, -880.0f, 46.8f)}};          // 29 yd S (cliff below)

    bool IsGlaiveThrower(uint32 entry) { return entry == NPC_GLAIVE_THROWER_A || entry == NPC_GLAIVE_THROWER_H; }
    bool IsLobbingVehicle(uint32 entry) { return entry == NPC_DEMOLISHER || IsGlaiveThrower(entry); }

    int32 GateIndex(TeamId owner, uint8 side)
    {
        for (int32 i = 0; i < 6; ++i)
            if (IOC_GATES[i].owner == owner && IOC_GATES[i].side == side)
                return i;

        return -1;
    }

    float GateHealth(GameObject* gate)
    {
        uint32 const maxHp = gate->GetGOValue()->Building.MaxHealth;
        return maxHp ? float(gate->GetGOValue()->Building.Health) / float(maxHp) : 1.0f;
    }

    std::mutex g_iocVehMutex;
    std::map<std::pair<uint32, ObjectGuid>, int32> g_iocVehGate;  // (bg instance, vehicle) -> IOC_GATES index
    std::map<ObjectGuid, Position> g_iocBoardPos;                  // driver -> where it boarded (the "drive" moved=)

    int32 AssignedGate(uint32 instanceId, ObjectGuid vehicle)
    {
        std::lock_guard<std::mutex> lock(g_iocVehMutex);
        auto itr = g_iocVehGate.find(std::make_pair(instanceId, vehicle));
        return itr != g_iocVehGate.end() ? itr->second : -1;
    }

    // The gate the vehicle `base` (driven by `bot`) attacks; nullptr when every enemy gate is down.
    IocGate const* PickGate(Player* bot, BattlegroundIC* bg, TeamId team, Unit* base)
    {
        BattlegroundMap* map = bg->FindBgMap();
        if (!map)
            return nullptr;

        uint32 const instanceId = bg->GetInstanceID();
        TeamId const enemy = Other(team);
        uint32 counts[6] = {};
        int32 current = -1;
        {
            std::lock_guard<std::mutex> lock(g_iocVehMutex);
            for (auto itr = g_iocVehGate.begin(); itr != g_iocVehGate.end();)
            {
                if (itr->first.first != instanceId)
                {
                    ++itr;
                    continue;
                }

                Creature* vehicle = map->GetCreature(itr->first.second);
                if (!vehicle || !vehicle->IsAlive() || !vehicle->GetVehicleKit())
                {
                    itr = g_iocVehGate.erase(itr);
                    continue;
                }

                if (vehicle == base)
                    current = itr->second;
                else if (Unit* driver = vehicle->GetVehicleKit()->GetPassenger(0))
                    if (driver->GetTypeId() == TYPEID_PLAYER)
                        ++counts[itr->second];

                ++itr;
            }
        }

        // the weakest standing gate of the enemy keep
        int32 weakest = -1;
        float weakHp = 2.0f;
        for (int32 i = 0; i < 6; ++i)
        {
            if (IOC_GATES[i].owner != enemy)
                continue;

            GameObject* gate = SlotObject(bg, IOC_GATES[i].slot);
            if (!GateStands(gate))
                continue;

            float const hp = GateHealth(gate);
            if (hp < weakHp)
            {
                weakHp = hp;
                weakest = i;
            }
        }

        if (weakest < 0)
            return nullptr;

        GameObject* currentGate = current >= 0 ? SlotObject(bg, IOC_GATES[current].slot) : nullptr;
        bool const currentStands = currentGate && GateStands(currentGate);
        float const currentHp = currentStands ? GateHealth(currentGate) : 2.0f;

        int32 pick = -1;
        char const* why = nullptr;
        if (weakHp <= IOC_FOCUS_HP && current != weakest && currentHp - weakHp >= IOC_FOCUS_MARGIN)
        {
            pick = weakest;
            why = "focus";
        }
        else if (currentStands)
            pick = current;
        else if (IsGlaiveThrower(base->GetEntry()))
        {
            pick = GateIndex(enemy, IOC_GATE_WEST);
            why = "docks-side";
        }
        else
        {
            int32 const front = GateIndex(enemy, IOC_GATE_FRONT);
            if (counts[front] < IOC_FRONT_DEMOLISHERS)
            {
                pick = front;
                why = "front";
            }
            else
            {
                int32 const east = GateIndex(enemy, IOC_GATE_EAST);
                int32 const west = GateIndex(enemy, IOC_GATE_WEST);
                pick = counts[west] < counts[east] ? west : east;
                why = "split";
            }
        }

        // a picked gate that is down (only before the portcullis state catches up): the nearest standing park
        if (pick < 0 || !GateStands(SlotObject(bg, IOC_GATES[pick].slot)))
        {
            pick = weakest;
            float best = base->GetExactDist2d(IOC_GATES[weakest].park);
            for (int32 i = 0; i < 6; ++i)
                if (IOC_GATES[i].owner == enemy && GateStands(SlotObject(bg, IOC_GATES[i].slot)) &&
                    base->GetExactDist2d(IOC_GATES[i].park) < best)
                {
                    best = base->GetExactDist2d(IOC_GATES[i].park);
                    pick = i;
                }
            why = "nearest";
        }

        if (pick != current)
        {
            {
                std::lock_guard<std::mutex> lock(g_iocVehMutex);
                g_iocVehGate[std::make_pair(instanceId, base->GetGUID())] = pick;
            }

            GameObject* gate = SlotObject(bg, IOC_GATES[pick].slot);
            TC_LOG_DEBUG("playerbots", "[IOC] gate-pick bot=%s team=%s vehicle=%u gate=%s why=%s hp=%.2f front=%u west=%u east=%u at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), TeamName(team), base->GetEntry(), IOC_GATES[pick].name, why ? why : "-",
                         gate ? GateHealth(gate) : 0.0f, counts[GateIndex(enemy, IOC_GATE_FRONT)],
                         counts[GateIndex(enemy, IOC_GATE_WEST)], counts[GateIndex(enemy, IOC_GATE_EAST)],
                         base->GetPositionX(), base->GetPositionY(), base->GetPositionZ());
        }

        return &IOC_GATES[pick];
    }

    // The nearest enemy Siege Engine / Demolisher / Glaive Thrower / Catapult with a player driving it, in line of sight
    // of `base` within `range` (slots BG_IC_NPC_SIEGE_ENGINE_A .. BG_IC_NPC_CATAPULT_4_H, as OwnKeepThreatened).
    Creature* EnemyVehicleNear(BattlegroundIC* bg, TeamId team, Unit* base, float range)
    {
        Creature* best = nullptr;
        float bestDist = range;
        uint32 const enemyFaction = FactionOf(Other(team));
        for (uint32 slot = BG_IC_NPC_SIEGE_ENGINE_A; slot <= BG_IC_NPC_CATAPULT_4_H; ++slot)
        {
            Creature* vehicle = SlotCreature(bg, slot);
            if (!vehicle || !vehicle->IsAlive() || vehicle->GetFaction() != enemyFaction || !vehicle->GetVehicleKit())
                continue;

            Unit* driver = vehicle->GetVehicleKit()->GetPassenger(0);
            if (!driver || driver->GetTypeId() != TYPEID_PLAYER)
                continue;

            float const dist = base->GetExactDist(vehicle);
            if (dist < bestDist && base->IsWithinLOSInMap(vehicle))
            {
                bestDist = dist;
                best = vehicle;
            }
        }

        return best;
    }

    // 0 = no siege vehicle for this role now, 1 = a Glaive Thrower, 2 = a Demolisher (see the role list above).
    uint32 SiegeDriverKind(BattlegroundIC* bg, TeamId team, uint32 role)
    {
        if ((role == IOC_ROLE_GLAIVE_A || role == IOC_ROLE_GLAIVE_B) && bg->GetNodeState(NODE_TYPE_DOCKS) == Controlled(team))
            return 1;

        if (bg->GetNodeState(NODE_TYPE_WORKSHOP) != Controlled(team))
            return 0;

        if (role == IOC_ROLE_DEMOLISHER_WORKSHOP && !OwnKeepThreatened(bg, team))
            return 2;

        if (role == IOC_ROLE_DEMOLISHER_HANGAR && bg->GetNodeState(NODE_TYPE_HANGAR) == Controlled(team))
            return 2;

        uint32 const sideBase = team == TEAM_ALLIANCE ? NODE_TYPE_QUARRY : NODE_TYPE_REFINERY;
        if (role == IOC_ROLE_DEMOLISHER_SIDE && bg->GetNodeState(sideBase) == Controlled(team))
            return 2;

        return 0;
    }

    Creature* FreeSiegeVehicle(Player* bot, BattlegroundIC* bg, TeamId team, uint32 kind, float maxDist)
    {
        bool const alliance = team == TEAM_ALLIANCE;
        if (kind == 1)
            return FreeOwnVehicle(bot, bg, team, alliance ? BG_IC_NPC_GLAIVE_THROWER_1_A : BG_IC_NPC_GLAIVE_THROWER_1_H,
                                  alliance ? BG_IC_NPC_GLAIVE_THROWER_2_A : BG_IC_NPC_GLAIVE_THROWER_2_H, maxDist);

        if (kind == 2)
            return FreeOwnVehicle(bot, bg, team, alliance ? BG_IC_NPC_DEMOLISHER_1_A : BG_IC_NPC_DEMOLISHER_1_H,
                                  alliance ? BG_IC_NPC_DEMOLISHER_4_A : BG_IC_NPC_DEMOLISHER_4_H, maxDist);

        return nullptr;
    }

    // Driver of a Demolisher / Glaive Thrower, every tick (AtObjective): aim "bg siege" at an enemy driven siege vehicle
    // close by, else back at the vehicle's gate (#753 pattern: before this tick's shot fires at a stale point). Never
    // ends the tick: BGTactics keeps driving to the park, the "in vehicle" triggers fire.
    void AimSiege(Player* bot, AiObjectContext* context, BattlegroundIC* bg, TeamId team, Unit* base)
    {
        int32 const idx = AssignedGate(bg->GetInstanceID(), base->GetGUID());
        if (idx < 0)
            return;  // SelectObjective has not picked a gate for this vehicle yet

        GameObject* gate = SlotObject(bg, IOC_GATES[idx].slot);
        if (!GateStands(gate))
            return;

        PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
        uint32 const mapId = bot->GetMapId();
        PositionInfo siege = posMap["bg siege"];  // by value: PositionInfo::isSet is not const here
        if (Creature* foe = EnemyVehicleNear(bg, team, base, IOC_VEHICLE_ENGAGE))
        {
            SetPos(posMap, "bg siege", *foe, mapId);
            if (BotDue(bot, 12, 10000))
                TC_LOG_DEBUG("playerbots", "[IOC] vehicle-target bot=%s vehicle=%u target=%u dist=%.1f target-hp=%.0f gate=%s",
                             bot->GetName().c_str(), base->GetEntry(), foe->GetEntry(), base->GetExactDist(foe),
                             foe->GetHealthPct(), IOC_GATES[idx].name);
        }
        else if (!siege.isSet() || gate->GetExactDist2d(siege.x, siege.y) > IOC_SIEGE_GATE_MATCH)
        {
            SetPos(posMap, "bg siege", *gate, mapId);
            if (siege.isSet() && BotDue(bot, 14, 10000))
                TC_LOG_DEBUG("playerbots", "[IOC] siege-retarget bot=%s vehicle=%u gate=%s", bot->GetName().c_str(),
                             base->GetEntry(), IOC_GATES[idx].name);
        }

        Position const& park = IOC_GATES[idx].park;
        if (BotDue(bot, 11, 10000))
        {
            float moved = -1.0f;
            {
                std::lock_guard<std::mutex> lock(g_iocVehMutex);
                auto itr = g_iocBoardPos.find(bot->GetGUID());
                if (itr != g_iocBoardPos.end())
                    moved = base->GetExactDist2d(itr->second);
            }
            TC_LOG_DEBUG("playerbots", "[IOC] drive bot=%s team=%s vehicle=%u gate=%s gatedist=%.1f parkdist=%.1f moved=%.1f hp=%.0f gatehp=%.2f at=%.1f,%.1f,%.1f",
                         bot->GetName().c_str(), TeamName(team), base->GetEntry(), IOC_GATES[idx].name,
                         base->GetExactDist2d(gate->GetPositionX(), gate->GetPositionY()), base->GetExactDist2d(park), moved,
                         base->GetHealthPct(), GateHealth(gate), base->GetPositionX(), base->GetPositionY(), base->GetPositionZ());
        }

        if (base->GetExactDist2d(park) < IOC_PARK_HOLD && BotDue(bot, 13, 30000))
            TC_LOG_DEBUG("playerbots", "[IOC] siege-park bot=%s vehicle=%u gate=%s gatedist=%.1f", bot->GetName().c_str(),
                         base->GetEntry(), IOC_GATES[idx].name, base->GetExactDist2d(gate->GetPositionX(), gate->GetPositionY()));
    }

    GameObject* NearestSpawnedObject(Player* bot, BattlegroundIC* bg, uint32 first, uint32 last, float maxDist)
    {
        GameObject* best = nullptr;
        float bestDist = maxDist;
        for (uint32 slot = first; slot <= last; ++slot)
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

    // The gunship of the bot's own team that the bot is riding (the hangar portal puts it on board,
    // BattlegroundIC::DoAction).
    Transport* OwnGunship(Player* bot, TeamId team)
    {
        Transport* ship = bot->GetTransport();
        uint32 const entry = team == TEAM_ALLIANCE ? GO_ALLIANCE_GUNSHIP : GO_HORDE_GUNSHIP;
        return ship && ship->GetEntry() == entry ? ship : nullptr;
    }

    void Dismount(Player* bot)
    {
        // as AC's atFlag before a capture
        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    }

    // AC EnterVehicleAction::EnterVehicle (AC VehicleActions.cpp:78-98): spellclick, then cancel the mount the
    // bot may have entered on. The siege position belongs to the vehicle it was set for (AC PR #2856).
    // #675: as SotA's Board (BgTacticsSA.cpp, #737 MoP adapter) the bot boards standing still - a point move still
    // running at the board would be frozen on the rooted passenger (MotionMaster.cpp:303).
    bool Board(Player* bot, AiObjectContext* context, Creature* vehicle, int8 seat, char const* what)
    {
        bot->StopMoving();
        bot->GetMotionMaster()->Clear();
        vehicle->HandleSpellClick(bot, seat);
        if (!bot->IsOnVehicle(vehicle))
            return false;

        {
            std::lock_guard<std::mutex> lock(g_iocVehMutex);
            g_iocBoardPos[bot->GetGUID()] = Position(vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ());
        }
        ResetPos(context, "bg siege");
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        TC_LOG_DEBUG("playerbots", "[IOC] board bot=%s what=%s vehicle=%u at=%.1f,%.1f,%.1f", bot->GetName().c_str(), what,
                     vehicle->GetEntry(), vehicle->GetPositionX(), vehicle->GetPositionY(), vehicle->GetPositionZ());
        return true;
    }

    bool UseObject(Player* bot, GameObject* go, char const* what)
    {
        Dismount(bot);
        bot->StopMoving();
        // MoP adapter, as BGTactics::IsLockedInsideKeep: CMSG_GAMEOBJ_USE is bit-packed on this core, so call
        // what HandleGameObjectUseOpcode calls after parsing it (Handlers/SpellHandler.cpp).
        go->Use(bot);
        TC_LOG_DEBUG("playerbots", "[IOC] %s bot=%s go=%u at=%.1f,%.1f,%.1f", what, bot->GetName().c_str(),
                     go->GetEntry(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        return true;
    }

    // Plant a carried (Huge) Seaforium Bomb: its on-use spell summons the bomb in front of the bot, so face the
    // gate. Same use path as UseItemAction::UseItem (OnItemUse, then CastItemUseSpell).
    bool PlantBomb(Player* bot, Item* item, GameObject* gate)
    {
        if (bot->IsNonMeleeSpellCasted(false) || bot->CanUseItem(item) != EQUIP_ERR_OK || !BotDue(bot, 1, 2000))
            return false;

        Dismount(bot);
        bot->StopMoving();
        bot->SetFacingToObject(gate);

        // #749: read the entry before the cast - the on-use spell consumes the bomb and destroys the Item, so any
        // item-> access after CastItemUseSpell is a use-after-free (w19 crash, Object::GetUInt32Value).
        uint32 const itemEntry = item->GetEntry();
        SpellCastTargets targets;
        targets.SetUnitTarget(bot);
        if (!sScriptMgr->OnItemUse(bot, item, targets))
            bot->CastItemUseSpell(item, targets, 1, 0);

        TC_LOG_DEBUG("playerbots", "[IOC] bomb-plant bot=%s item=%u gate=%u gatehp=%u/%u at=%.1f,%.1f,%.1f",
                     bot->GetName().c_str(), itemEntry, gate->GetEntry(), gate->GetGOValue()->Building.Health,
                     gate->GetGOValue()->Building.MaxHealth, bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        return true;
    }

    // Launch (66218) from the catapult driver seat at a courtyard point. Cast the way PlayerbotAI::CastVehicleSpell
    // casts a vehicle spell (the vehicle base casts it with a DEST target, speed and elevation), but with a pitch
    // and speed that put the arc over the keep wall: spell_ioc_launch hands the spell's speed and elevation to
    // MoveJump, and MoveJump ends at the destination whatever the arc.
    bool LaunchCatapult(Player* bot, Unit* base, Position const& landing)
    {
        SpellInfo const* info = sSpellMgr->GetSpellInfo(IOC_SPELL_LAUNCH);
        if (!info || !BotDue(bot, 2, 3000))
            return false;

        if (base->GetSpellHistory() && !base->GetSpellHistory()->IsReady(info, 0, false))
            return false;

        float const dist = base->GetExactDist2d(landing.GetPositionX(), landing.GetPositionY());
        if (dist < 1.0f)
            return false;

        float const speed = std::sqrt(dist * IOC_GRAVITY / std::sin(2.0f * IOC_LAUNCH_PITCH));

        base->StopMoving();
        base->SetFacingTo(base->GetAngle(landing.GetPositionX(), landing.GetPositionY()));

        SpellCastTargets targets;
        targets.SetDst(landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ(), 0.0f, bot->GetMapId());
        targets.SetSpeed(speed);
        targets.SetElevation(IOC_LAUNCH_PITCH);

        Spell* spell = new Spell(base, info, TRIGGERED_NONE);
        spell->prepare(&targets);

        TC_LOG_DEBUG("playerbots", "[IOC] launch bot=%s from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f dist=%.1f speed=%.1f",
                     bot->GetName().c_str(), base->GetPositionX(), base->GetPositionY(), base->GetPositionZ(),
                     landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ(), dist, speed);
        return true;
    }

    // Leave our gunship over the enemy keep: off the transport, the parachute (66657: slow fall, the aura
    // spell_ioc_parachute_ic gives a falling player), and a glide that ends in the courtyard.
    bool DropFromGunship(Player* bot, Transport* ship, Position const& drop)
    {
        if (bot->GetExactDist2d(drop.GetPositionX(), drop.GetPositionY()) > IOC_GUNSHIP_DROP_RANGE || !BotDue(bot, 3, 3000))
            return false;

        ship->RemovePassenger(bot, true);
        // MoP's RemovePassenger keeps IGNORE_PATHFINDING for an MO transport rider (see PlayerbotAI.cpp #141 F1)
        bot->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        bot->StopMoving();
        bot->CastSpell(bot, SPELL_PARACHUTE_IC, true);
        bot->GetMotionMaster()->MoveJump(drop.GetPositionX(), drop.GetPositionY(), drop.GetPositionZ(), IOC_GLIDE_SPEED,
                                         0.5f);
        TC_LOG_DEBUG("playerbots", "[IOC] drop bot=%s from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f", bot->GetName().c_str(),
                     bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), drop.GetPositionX(),
                     drop.GetPositionY(), drop.GetPositionZ());
        return true;
    }

    // Evidence for the 1:1 (MoP-only): gate health while vehicles have a siege position, gate destruction,
    // keep breach and the general's health - one line per event per match, whichever bot sees it first.
    void ObserveMatch(Player* bot, AiObjectContext* context, BattlegroundIC* bg)
    {
        uint32 const instanceId = bg->GetInstanceID();

        if (bot->GetVehicle())
        {
            PositionInfo siege = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
            if (siege.isSet())
            {
                for (uint32 slot : IOC_GATE_SLOTS)
                {
                    GameObject* gate = SlotObject(bg, slot);
                    if (!gate || gate->GetExactDist2d(siege.x, siege.y) > 20.0f)
                        continue;

                    if (LogDue(instanceId, gate->GetEntry(), 15000))
                        TC_LOG_DEBUG("playerbots", "[IOC] gate-hp gate=%u hp=%u/%u state=%u by=%s vehicle=%u dist=%.1f",
                                     gate->GetEntry(), gate->GetGOValue()->Building.Health,
                                     gate->GetGOValue()->Building.MaxHealth, uint32(gate->GetDestructibleState()),
                                     bot->GetName().c_str(), bot->GetVehicleBase() ? bot->GetVehicleBase()->GetEntry() : 0,
                                     bot->GetExactDist2d(gate->GetPositionX(), gate->GetPositionY()));
                    break;
                }
            }
        }

        // #675: every gate's health / max : destructible state (1 intact, 2 damaged, 3 destroyed - the gate world states
        // BG_IC_GATE_*_WS read the same GO state, BattlegroundIC.cpp)
        if (LogDue(instanceId, 2, 30000))
        {
            uint32 hp[6] = {};
            uint32 maxHp[6] = {};
            uint32 state[6] = {};
            for (uint32 i = 0; i < 6; ++i)
                if (GameObject* gate = SlotObject(bg, IOC_GATES[i].slot))
                {
                    hp[i] = gate->GetGOValue()->Building.Health;
                    maxHp[i] = gate->GetGOValue()->Building.MaxHealth;
                    state[i] = uint32(gate->GetDestructibleState()) + 1;
                }

            TC_LOG_DEBUG("playerbots", "[IOC] gates a-front=%u/%u:%u a-west=%u/%u:%u a-east=%u/%u:%u h-front=%u/%u:%u h-west=%u/%u:%u h-east=%u/%u:%u t=%u instance=%u",
                         hp[0], maxHp[0], state[0], hp[1], maxHp[1], state[1], hp[2], maxHp[2], state[2], hp[3], maxHp[3],
                         state[3], hp[4], maxHp[4], state[4], hp[5], maxHp[5], state[5], bg->GetElapsedTime(), instanceId);
        }

        if (!LogDue(instanceId, 1, 5000))
            return;

        for (uint32 slot : IOC_GATE_SLOTS)
            if (GameObject* gate = SlotObject(bg, slot))
                if (!GateStands(gate) && LogDue(instanceId, 1000000 + gate->GetEntry(), 0))
                    TC_LOG_DEBUG("playerbots", "[IOC] gate-down gate=%u instance=%u", gate->GetEntry(), instanceId);

        for (TeamId owner : {TEAM_ALLIANCE, TEAM_HORDE})
        {
            if (!KeepBreached(bg, owner))
                continue;

            if (LogDue(instanceId, 10 + uint32(owner), 0))
                TC_LOG_DEBUG("playerbots", "[IOC] keep-breached keep=%s instance=%u", TeamName(owner), instanceId);

            if (Creature* boss = SlotCreature(bg, KeepOf(owner).bossSlot))
                if (LogDue(instanceId, 20 + uint32(owner), 10000))
                    TC_LOG_DEBUG("playerbots", "[IOC] boss-hp keep=%s boss=%u hp=%.0f alive=%u visible=%u instance=%u",
                                 TeamName(owner), boss->GetEntry(), boss->GetHealthPct(), boss->IsAlive() ? 1u : 0u,
                                 boss->IsVisible() ? 1u : 0u, instanceId);
        }
    }
}

namespace IocBeyondAc
{
    bool Enabled() { return g_iocEnabled.load(); }

    void SetEnabled(bool enabled) { g_iocEnabled.store(enabled); }

    bool SelectObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundIC* bg, TeamId team,
                         uint32 role, PositionMap& posMap)
    {
        if (!Enabled() || !bot || !botAI || !context || !bg || team == TEAM_NEUTRAL)
            return false;

        IocKeep const& enemy = KeepOf(Other(team));
        uint32 const mapId = bot->GetMapId();
        bool const enemyClosed = !KeepBreached(bg, Other(team));

        // Riding our gunship: stay on the deck, AtObjective drops over the enemy keep.
        if (IOC_GUNSHIP_ROUTE && OwnGunship(bot, team))
            return true;

        if (Unit* base = bot->GetVehicleBase())
        {
            uint32 const entry = base->GetEntry();
            if (entry == NPC_KEEP_CANNON || entry == IOC_NPC_BROKEN_KEEP_CANNON)
            {
                // AC: a gunner is "not the driver" and gets no objective. Beyond AC: get out once no enemy
                // siege vehicle threatens the keep any more, so the defender rejoins the fight.
                if (!OwnKeepThreatened(bg, team))
                {
                    WorldPacket packet;
                    bot->GetSession()->HandleRequestVehicleExit(packet);
                    TC_LOG_DEBUG("playerbots", "[IOC] leave-cannon bot=%s team=%s", bot->GetName().c_str(), TeamName(team));
                }
                return true;
            }

            if (entry == NPC_CATAPULT)
            {
                // seat 1 is the launch seat: spell_ioc_launch throws its rider 1.5 s after Launch
                if (!botAI->IsInVehicle(true))
                    return true;

                SetPos(posMap, "bg siege", enemy.landing, mapId);
                SetPos(posMap, "bg objective", enemy.launchSpot, mapId);
                return true;
            }

            // #675: a Demolisher / Glaive Thrower driver drives to its gate's park and fires at the gate (PickGate). Once
            // the enemy keep is breached, the Siege Engine, the turrets and passengers: AC's tactics.
            if (IsLobbingVehicle(entry) && enemyClosed && botAI->IsInVehicle(true))
            {
                IocGate const* gate = PickGate(bot, bg, team, base);
                GameObject* go = gate ? SlotObject(bg, gate->slot) : nullptr;
                if (!go)
                    return false;

                // AC PR mod-playerbots#2856's IoC pattern: the gate is the siege position, the park the objective
                // (AimSiege may point the siege position at an enemy vehicle in between)
                SetPos(posMap, "bg siege", *go, mapId);
                if (base->GetExactDist2d(gate->park) < IOC_PARK_HOLD)
                    SetPos(posMap, "bg objective", *base, mapId);
                else
                {
                    float const jitter = gate->side == IOC_GATE_FRONT ? IOC_PARK_JITTER_FRONT : IOC_PARK_JITTER_SIDE;
                    SetPos(posMap, "bg objective",
                           Position(gate->park.GetPositionX() + frand(-jitter, jitter),
                                    gate->park.GetPositionY() + frand(-jitter, jitter), gate->park.GetPositionZ()),
                           mapId);
                }

                return true;
            }

            // Siege Engines, turrets, passengers, and every vehicle once the enemy keep is open: AC's tactics.
            return false;
        }

        // #738: our keep is open - its guards, its banner, and whoever is inside with the intruders defend it.
        if (KeepDefence(bot, context, bg, team, role, posMap))
            return true;

        // Carrying a bomb: walk it to its spot at the enemy front gate.
        if (enemyClosed)
        {
            GameObject* gate = SlotObject(bg, enemy.frontGateSlot);
            if (GateStands(gate))
            {
                if (bot->HasItemCount(IOC_ITEM_HUGE_SEAFORIUM_BOMBS, 1) && InsideKeep(Other(team), bot))
                {
                    SetPos(posMap, "bg objective", enemy.insideBombSpot, mapId);
                    return true;
                }

                if (bot->HasItemCount(IOC_ITEM_SEAFORIUM_BOMBS, 1))
                {
                    SetPos(posMap, "bg objective", enemy.outsideBombSpot, mapId);
                    return true;
                }
            }

            // Inside the enemy keep (a catapult launch or a gunship drop) while its gates stand: fetch one of the
            // keep's own Huge Seaforium Bombs; wait at the gate while the pile respawns (10 s).
            if (InsideKeep(Other(team), bot))
            {
                if (GameObject* bomb = NearestSpawnedObject(bot, bg, enemy.hugeBombFirst, enemy.hugeBombLast, 300.0f))
                    SetPos(posMap, "bg objective", *bomb, mapId);
                else
                    SetPos(posMap, "bg objective", enemy.insideBombSpot, mapId);

                return true;
            }
        }

        // Defenders: man a keep cannon (or repair a broken one) while enemy siege vehicles close in.
        if (role == IOC_ROLE_CANNON && OwnKeepThreatened(bg, team))
        {
            if (Creature* cannon = FreeKeepCannon(bot, bg, team, 1000.0f))
            {
                SetPos(posMap, "bg objective", *cannon, mapId);
                return true;
            }
        }

        if (!enemyClosed)
            return false;

        // Attackers: a catapult from the Docks, launched over the enemy wall.
        if (role == IOC_ROLE_CATAPULT && bg->GetNodeState(NODE_TYPE_DOCKS) == Controlled(team))
        {
            if (Creature* catapult = FreeCatapult(bot, bg, team, 1000.0f))
            {
                SetPos(posMap, "bg objective", *catapult, mapId);
                return true;
            }
        }

        // Attackers: drive the Siege Engine (AC's siege-engine branch then rams the gate with it).
        if (role == IOC_ROLE_SIEGE_ENGINE && bg->GetNodeState(NODE_TYPE_WORKSHOP) == Controlled(team))
        {
            if (Creature* siegeEngine = FreeSiegeEngine(bot, bg, team, 1000.0f))
            {
                SetPos(posMap, "bg objective", *siegeEngine, mapId);
                return true;
            }
        }

        // #675 attackers: a Glaive Thrower from the Docks / a Demolisher from the Workshop (SiegeDriverKind).
        if (uint32 const kind = SiegeDriverKind(bg, team, role))
        {
            if (Creature* vehicle = FreeSiegeVehicle(bot, bg, team, kind, 1000.0f))
            {
                SetPos(posMap, "bg objective", *vehicle, mapId);
                return true;
            }
        }

        // Attackers: a Workshop Seaforium Bomb for the enemy front gate.
        if ((role == IOC_ROLE_BOMB_A || role == IOC_ROLE_BOMB_B) &&
            bg->GetNodeState(NODE_TYPE_WORKSHOP) == Controlled(team) && GateStands(SlotObject(bg, enemy.frontGateSlot)))
        {
            if (GameObject* bombs = NearestSpawnedObject(bot, bg, BG_IC_GO_SEAFORIUM_BOMBS_1, BG_IC_GO_SEAFORIUM_BOMBS_2, 1000.0f))
            {
                SetPos(posMap, "bg objective", *bombs, mapId);
                return true;
            }
        }

        // Paratroopers: our gunship's portal at the Hangar.
        if (IOC_GUNSHIP_ROUTE && role == IOC_ROLE_GUNSHIP && bg->GetNodeState(NODE_TYPE_HANGAR) == Controlled(team))
        {
            if (GameObject* portal = NearestSpawnedObject(bot, bg, BG_IC_GO_HANGAR_TELEPORTER_1, BG_IC_GO_HANGAR_TELEPORTER_3, 1000.0f))
            {
                SetPos(posMap, "bg objective", *portal, mapId);
                return true;
            }
        }

        return false;
    }

    bool AtObjective(Player* bot, PlayerbotAI* botAI, AiObjectContext* context, BattlegroundIC* bg, TeamId team,
                     uint32 role)
    {
        if (!Enabled() || !bot || !botAI || !context || !bg || bg->GetStatus() != STATUS_IN_PROGRESS ||
            team == TEAM_NEUTRAL || !bot->IsAlive())
            return false;

        ObserveMatch(bot, context, bg);

        IocKeep const& enemy = KeepOf(Other(team));
        bool const enemyClosed = !KeepBreached(bg, Other(team));

        if (IOC_GUNSHIP_ROUTE)
            if (Transport* ship = OwnGunship(bot, team))
                return DropFromGunship(bot, ship, enemy.dropPoint);

        if (Unit* base = bot->GetVehicleBase())
        {
            if (base->GetEntry() == NPC_CATAPULT && botAI->IsInVehicle(true) &&
                base->GetExactDist2d(enemy.launchSpot.GetPositionX(), enemy.launchSpot.GetPositionY()) <= IOC_LAUNCH_RADIUS)
                return LaunchCatapult(bot, base, enemy.landing);

            // #675: a Demolisher / Glaive Thrower driver shoots an enemy siege vehicle close by, else its gate
            if (enemyClosed && IsLobbingVehicle(base->GetEntry()) && botAI->IsInVehicle(true))
                AimSiege(bot, context, bg, team, base);

            return false;
        }

        if (bot->IsNonMeleeSpellCasted(false))  // e.g. channelling Repair Cannon
            return false;

        if (enemyClosed)
        {
            GameObject* gate = SlotObject(bg, enemy.frontGateSlot);
            if (GateStands(gate))
            {
                if (Item* huge = bot->GetItemByEntry(IOC_ITEM_HUGE_SEAFORIUM_BOMBS))
                    if (bot->GetExactDist2d(enemy.insideBombSpot.GetPositionX(), enemy.insideBombSpot.GetPositionY()) <= IOC_PLANT_RADIUS)
                        return PlantBomb(bot, huge, gate);

                if (Item* bomb = bot->GetItemByEntry(IOC_ITEM_SEAFORIUM_BOMBS))
                    if (bot->GetExactDist2d(enemy.outsideBombSpot.GetPositionX(), enemy.outsideBombSpot.GetPositionY()) <= IOC_PLANT_RADIUS)
                        return PlantBomb(bot, bomb, gate);
            }

            if (InsideKeep(Other(team), bot) && !bot->HasItemCount(IOC_ITEM_HUGE_SEAFORIUM_BOMBS, 1))
                if (GameObject* pile = NearestSpawnedObject(bot, bg, enemy.hugeBombFirst, enemy.hugeBombLast, INTERACTION_DISTANCE))
                    return UseObject(bot, pile, "bomb-pickup");

            if ((role == IOC_ROLE_BOMB_A || role == IOC_ROLE_BOMB_B) &&
                bg->GetNodeState(NODE_TYPE_WORKSHOP) == Controlled(team) && !bot->HasItemCount(IOC_ITEM_SEAFORIUM_BOMBS, 1))
                if (GameObject* pile = NearestSpawnedObject(bot, bg, BG_IC_GO_SEAFORIUM_BOMBS_1, BG_IC_GO_SEAFORIUM_BOMBS_2, INTERACTION_DISTANCE))
                    return UseObject(bot, pile, "bomb-pickup");
        }

        if (role == IOC_ROLE_CANNON && OwnKeepThreatened(bg, team))
        {
            if (Creature* cannon = FreeKeepCannon(bot, bg, team, IOC_CANNON_REACH))
            {
                if (!BotDue(bot, 4, 2000))
                    return false;

                if (cannon->GetEntry() == IOC_NPC_BROKEN_KEEP_CANNON)
                {
                    // the cannon's spellclick casts Repair Cannon (68077, 8 s channel) while it is broken
                    Dismount(bot);
                    bot->StopMoving();
                    cannon->HandleSpellClick(bot);
                    TC_LOG_DEBUG("playerbots", "[IOC] repair bot=%s team=%s cannon=%.1f,%.1f,%.1f", bot->GetName().c_str(),
                                 TeamName(team), cannon->GetPositionX(), cannon->GetPositionY(), cannon->GetPositionZ());
                    return true;
                }

                return Board(bot, context, cannon, -1, "cannon");
            }
        }

        if (!enemyClosed)
            return false;

        if (role == IOC_ROLE_CATAPULT && bg->GetNodeState(NODE_TYPE_DOCKS) == Controlled(team))
            if (Creature* catapult = FreeCatapult(bot, bg, team, INTERACTION_DISTANCE))
                if (BotDue(bot, 5, 2000))
                    return Board(bot, context, catapult, 0, "catapult");

        if (role == IOC_ROLE_SIEGE_ENGINE && bg->GetNodeState(NODE_TYPE_WORKSHOP) == Controlled(team))
            if (Creature* siegeEngine = FreeSiegeEngine(bot, bg, team, INTERACTION_DISTANCE + 2.0f))
                if (BotDue(bot, 6, 2000))
                    return Board(bot, context, siegeEngine, 0, "siege-engine");

        // #675: a Glaive Thrower / Demolisher driver (SiegeDriverKind)
        if (uint32 const kind = SiegeDriverKind(bg, team, role))
            if (Creature* vehicle = FreeSiegeVehicle(bot, bg, team, kind, INTERACTION_DISTANCE + 2.0f))
                if (BotDue(bot, 9, 2000))
                    return Board(bot, context, vehicle, 0, kind == 1 ? "glaive" : "demolisher");

        if (IOC_GUNSHIP_ROUTE && role == IOC_ROLE_GUNSHIP && bg->GetNodeState(NODE_TYPE_HANGAR) == Controlled(team))
            if (GameObject* portal = NearestSpawnedObject(bot, bg, BG_IC_GO_HANGAR_TELEPORTER_1, BG_IC_GO_HANGAR_TELEPORTER_3, INTERACTION_DISTANCE))
                if (BotDue(bot, 7, 5000))
                    return UseObject(bot, portal, "portal");

        return false;
    }

    bool StaysInOwnKeep(Player* bot, BattlegroundIC* bg, TeamId team, uint32 role)
    {
        if (!Enabled() || !bot || !bg || team == TEAM_NEUTRAL || role != IOC_ROLE_CANNON)
            return false;

        return OwnKeepThreatened(bg, team) && FreeKeepCannon(bot, bg, team, 1000.0f);
    }

    // #675 (beyond AC, after SotA's #765 SotaBeyondAc::RamTarget): a Demolisher driver has no "current target" (AC
    // AttackAction refuses a seat without CAN_ATTACK), so on AC's code Ram fires only as a siege shot within 15 yd of
    // the gate (PlayerbotAI.cpp IsSiegeShot, AC PR mod-playerbots#2856) - never from a park 50-60 yd out. This picks the
    // enemy the Ram cone hits now: the nearest alive, attackable, visible enemy unit in line of sight within
    // IOC_RAM_REACH and the cone centred on the "bg siege" bearing (PlayerbotAI::CastVehicleSpell turns a driven vehicle
    // to that point right before it casts), else on the vehicle's facing. "current target" and "bg siege" are not
    // touched, nothing chases the rammed unit; Ram's own 4 s cooldown paces it. Map 628 and the Demolisher only.
    Unit* RamTarget(Player* bot, AiObjectContext* context)
    {
        if (!Enabled() || !bot || !context || bot->GetMapId() != 628)
            return nullptr;

        BattlegroundIC* bg = dynamic_cast<BattlegroundIC*>(bot->GetBattleground());
        Vehicle* vehicle = bot->GetVehicle();
        if (!bg || bg->GetStatus() != STATUS_IN_PROGRESS || !vehicle)
            return nullptr;

        Unit* base = vehicle->GetBase();
        VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
        if (!base || base->GetEntry() != NPC_DEMOLISHER || !seat || !seat->CanControl())
            return nullptr;

        PositionInfo siege = context->GetValue<PositionMap&>("position")->Get()["bg siege"];  // by value (isSet)
        float const facing = siege.isSet() ? base->GetAngle(siege.x, siege.y) : base->GetOrientation();

        std::list<Unit*> units;
        Trinity::AnyUnfriendlyUnitInObjectRangeCheck check(base, base, IOC_RAM_REACH);
        Trinity::UnitListSearcher<Trinity::AnyUnfriendlyUnitInObjectRangeCheck> searcher(base, units, check);
        Cell::VisitAllObjects(base, searcher, IOC_RAM_REACH);

        Unit* best = nullptr;
        float bestDist = IOC_RAM_REACH;
        for (Unit* unit : units)
        {
            if (!unit || unit == base || unit == bot || !unit->IsAlive() || !base->IsValidAttackTarget(unit) ||
                !bot->CanSeeOrDetect(unit))
                continue;

            float const dist = base->GetExactDist(unit);
            if (dist > bestDist)
                continue;

            float const rel = std::remainder(base->GetAngle(unit) - facing, 2.0f * static_cast<float>(M_PI));
            if (std::fabs(rel) > IOC_RAM_CONE / 2.0f || !base->IsWithinLOSInMap(unit))
                continue;

            best = unit;
            bestDist = dist;
        }

        if (best && BotDue(bot, 15, 4000))
            TC_LOG_DEBUG("playerbots", "[IOC] ram-target bot=%s target=%s entry=%u player=%u dist=%.1f siege=%u hp=%.0f",
                         bot->GetName().c_str(), best->GetName().c_str(), best->GetEntry(), best->IsPlayer() ? 1u : 0u,
                         bestDist, siege.isSet() ? 1u : 0u, best->GetHealthPct());

        return best;
    }
}
