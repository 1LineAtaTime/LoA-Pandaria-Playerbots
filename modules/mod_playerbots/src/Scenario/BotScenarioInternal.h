/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Internal header shared only by BotScenario*.cpp implementation files. Declares the side-table
 * state and the scenario-handler registry (HARNESS-SPEC-botscenario-v3-module section 3), so a
 * future scenario kind (taxi, transport, vehicle, quest, gather, fish, loot, follow, duel, bg,
 * dungeon, arena) is one handler added from its own BotScenario<Name>.cpp, without editing
 * BotScenario.cpp itself. Nothing here is part of the public API (see BotScenario.h).
 */

#ifndef _PLAYERBOT_BOTSCENARIO_INTERNAL_H
#define _PLAYERBOT_BOTSCENARIO_INTERNAL_H

#include "Define.h"
#include "ObjectGuid.h"

#include <set>
#include <sstream>
#include <string>
#include <vector>

class ChatHandler;
class Player;
class PlayerbotAI;

namespace BotScenario
{
    // Kind of scenario currently held by a side-table entry. A plain probe (no scenario) leaves
    // this at None; the probe fields further down are independent of it.
    enum class ScenarioKind : uint8
    {
        None = 0,
        Goto,
        WanderNpc,
        WanderRandom,
        WanderFree,
        Attack,
        Die,
        Taxi,
        Quest,
        Transport,
        Bg,
        Drive,   // generic vehicle-driving primitive (issue #207; reused by #204/#205) - see BotScenarioVehicle.cpp
        HotRod,  // quest 14071 "Rolling with my Homies" end-to-end (issue #207) - see BotScenarioVehicle.cpp
        DuelSpot // #201 (#613): duel-spot harness - see BotScenarioDuel.cpp
    };

    // Death stage (research\death-revive-parity.md section 4).
    struct DieTrack
    {
        uint8 phase = 0;             // 0 arm/kill, 1 dead body, 2 ghost, 3 alive (settling)
        uint32 count = 0;            // "dcount=": death count set just before the kill
        uint32 releaseAfterMs = 0;   // "release=": harness runs "auto release" this long after death (0 = natural)
        uint32 mgrAfterMs = 0;       // "mgr=": harness calls RandomPlayerbotMgr::Revive this long after release
        bool releaseForced = false, mgrForced = false, sawReset = false;
        uint32 deathT = 0, releaseT = 0, nearT = 0, reviveT = 0;
        uint32 deathMap = 0;
        float deathX = 0.0f, deathY = 0.0f, deathZ = 0.0f;
        float reviveX = 0.0f, reviveY = 0.0f, reviveZ = 0.0f, reviveHp = 0.0f;
    };

    // Taxi scenario (research\taxi-quest-stage-design.md section A.4).
    struct TaxiTrack
    {
        uint32 from = 0, to = 0;       // taxi node ids
        uint8 phase = 0;                // 0 arm, 1 walk to flight master, 2 in flight, 3 settle
        uint32 fmEntry = 0;             // departure flight master's creature_template entry
        float fmX = 0.0f, fmY = 0.0f, fmZ = 0.0f;
        uint32 fmNode = 0;
        uint32 armT = 0, atFmT = 0, boardT = 0, landT = 0, resolveT = 0;
        // #225 (research\taxi-stopover-225.md section 5b): the route sTravelNodeMap.FindTaxiPath
        // resolves is a node *chain*, so a multi-leg route has one connection point (stopover) per
        // intermediate node. The MoP core flies exactly one leg per FlightPathMovementGenerator and
        // only starts the next one when the client sends CMSG_MOVE_SPLINE_DONE, so a bot without a
        // client stops at the first stopover - which must NOT be read as the landing. legCount is
        // path.size()-1 as resolved in phase 0; legs/stopovers are what this run actually observed.
        uint32 legCount = 0, legs = 0, stopovers = 0;
        uint32 stopoverT = 0;          // t of the first tick the current stopover was seen on
        bool waitReboard = false;      // landed on a connection point, waiting for the next leg
        float departX = 0.0f, departY = 0.0f, departZ = 0.0f; // position when the flight was armed
        float landX = 0.0f, landY = 0.0f, landZ = 0.0f;
        bool mountedAtFm = false;       // G-T4: was the bot mounted on approach to the flight master
        int32 statusAfter = -1;         // NewRpgStatus right after landing
    };

    // Quest scenario (research\taxi-quest-stage-design.md section B.4).
    struct QuestTrack
    {
        uint32 questId = 0;
        uint8 phase = 0;                 // 0 arm (wait + offer scan), 1 running (DoQuest armed)
        uint32 readyT = 0;               // t when the bot first sat at the start point (phase 0 hold)
        uint32 acceptT = 0, poiT = 0, atPoiT = 0, firstProgT = 0, completeT = 0, rewardT = 0;
        uint32 lastCount[4] = {0, 0, 0, 0}; // last-seen GetQuestObjectiveCounter per objective slot
        uint32 kills = 0;                // sum of objective-counter increases seen this run
        uint32 deaths = 0;               // alive -> dead edges seen this run
        bool wasAlive = true;
        std::set<uint32> logSet;         // quest ids seen in the log as of the previous tick
        uint32 extraAccepts = 0;
        float lastPoiX = 0.0f, lastPoiY = 0.0f;
        uint32 spawnN = 0;                // "spawn=n": creatures to summon at the POI
        bool spawned = false;
        std::vector<ObjectGuid> spawnGuids;
        // #151: quest 8475's kill objective already spawns off the quest's own objective.ObjectID
        // (a creature entry), but a drop/collect-from-GO objective's ObjectID is an *item* id, not a
        // spawnable entry. spawnnpc=<entry>/spawngo=<entry> override the auto-picked entry so those
        // stages can be run deterministically too (loot-stack-port.md section 2 step 7).
        uint32 spawnNpcEntry = 0;         // "spawnnpc=<entry>": override the creature entry to summon
        uint32 spawnGoEntry = 0;          // "spawngo=<entry>": summon this game object entry instead
    };

    // Transport scenario (research\transport-stage-design.md section 3.3). Phase 0 arm, 1 dock A
    // (waiting/placed on the deck), 2 board, 3 ride, 4 offboard/ashore.
    struct TransportTrack
    {
        uint32 entry = 0, dbGuid = 0;       // transport entry, `transports`.guid
        uint8 phase = 0;                    // 0 arm, 1 dock A, 2 board, 3 ride, 4 offboard
        uint8 tcheck = 1;                   // 0 natural, 1 now (default), 2 tick
        bool atWait = false;                // reached the wait point once (phase 0 then only needs the map)
        bool tcheckPending = false;         // "now": consumed once by TransportCheckDue
        uint32 stopA = 0, stopB = 0;        // keyframe vector indices of the two stop frames
        float deckX = 0, deckY = 0, deckZ = 0, destX = 0, destY = 0, destZ = 0;
        float placeX = 0, placeY = 0, placeZ = 0;
        float boardLX = 0, boardLY = 0, boardLZ = 0, maxDrift = 0;
        uint32 lastMap = 0;
        uint32 armT = 0, placeT = 0, boardT = 0, departT = 0, arriveT = 0, offT = 0;
        bool departed = false, dragged = false;
    };

    // Alterac Valley telemetry (research\bg-av-parity.md step 1b / section 3.6), populated through the
    // per-core AV adapter shims in BotScenario.cpp (MoP: the new BattlegroundAV getters, step 2; AC:
    // its own existing public getters plus FillInitialWorldStates for reinforcements). Embedded in
    // BgTrack only for bgTypeId == 1 (Alterac Valley) - left default-constructed and unused otherwise.
    struct AvTrack
    {
        uint32 lastReinfT = 0;                // t of the last reinforcement read (15 s cadence)
        int32 reinf[2] = { -1, -1 };          // 0 = Alliance, 1 = Horde; -1 = not read yet

        // Per-node last-seen state, for edge detection (BG_AV_NODES_MAX = 15 on both cores: indices
        // 0-6 are the graveyards/hut plus Snowfall, 7-14 are the 8 towers - same ordering both cores).
        uint8 nodeState[15] = {};             // BG_AV_States (0 neutral/standing, 1 assaulted, 2 destroyed)
        uint8 nodeOwnerLetter[15] = {};       // 0 = neutral, 1 = Alliance, 2 = Horde
        bool nodeSeeded = false;              // first sample only records a baseline - no edges yet

        bool captainAlivePrev[2] = { true, true };
        bool bossAlivePrev[2] = { true, true };
        uint8 mineOwnerLetterPrev[2] = { 0, 0 }; // 0 = north mine, 1 = south mine; letter as above

        // End-line accumulators (section 3.6 "Edges" and the "bgdone" line).
        uint32 assaults[2] = { 0, 0 };
        uint32 defends[2] = { 0, 0 };
        uint32 towersDown[2] = { 0, 0 };      // towers each side LOST (destroyed by the enemy)
        uint32 gyCaps[2] = { 0, 0 };
        uint32 mineCaps[2] = { 0, 0 };
        uint32 captainKillT[2] = { 0, 0 };
        uint32 bossKillT = 0;
        uint32 firstAssaultT = 0;
        uint32 firstTowerDownT = 0;
        uint32 npcDeaths[2] = { 0, 0 };
        std::vector<uint8> npcAttacked;       // per member: a creature was attacking at the last sample (#168)
        bool belowMin = false;
    };

    // Battleground scenario (research\bg-parity-stage-design.md section 3.6). Only the anchor (the
    // first name in "bots=") carries a side-table entry; it reads the whole match - the Battleground
    // object and every member Player - off its own map/AI update thread (section 3.4). Index i in
    // every per-member vector below is the same bot on both cores (select-bg-teams.ps1's class
    // order), and "team" is the member's own faction (TEAM_ALLIANCE/TEAM_HORDE), read once at queue
    // time - not something derived from the bots= list order.
    struct BgTrack
    {
        uint32 bgTypeId = 0;                        // BattlegroundTypeId (2 = WSG)
        uint8 bracketId = 0;                         // BattlegroundBracketId, resolved once at queue time
        uint32 instanceId = 0;                       // the BG instance, from any member inside (anchorout check)
        std::vector<ObjectGuid> members;            // guids, bots= order (index 0 is the anchor)
        std::vector<std::string> memberNames;       // captured at queue time - a member that goes
                                                     // offline mid-run (natural random-bot login churn)
                                                     // still logs a name, not just a raw guid
        std::vector<uint8> memberTeam;               // TeamId per member, same index
        uint8 stratA = 0, stratH = 0;                // strat=<a>,<h>: pinned into bgStrategies every sample
        bool rolesIndex = true;                      // roles=index (default) vs roles=natural (no pin)
        bool naturalQueue = false;                   // queue=natural (#570): the roster queues itself through
                                                     // BGJoinAction (rated arena); the harness only gates + traces
        uint32 fastMs = 0;                           // fast=<ms>: SetStartDelayTime once, first WAIT_JOIN sample
        uint32 sampleMs = 5000;                      // sample=<ms>: periodic [BGSTAGE] sample/pin cadence
        uint32 lastSampleT = 0;                      // last time (t, ms since queued) a sample fired
        bool fastApplied = false;

        // Per-member queue lifecycle (issue: bgnoform root cause - CheckNormalMatch only fills each
        // team to the bracket's *minimum* on the first queue-update pass, so a 10v10 queue only pops
        // 5v5 up front; the rest sit un-invited until another BattlegroundQueue::BattlegroundQueueUpdate
        // pass runs FillPlayersToBG on the now-existing instance). queuedOk records whether
        // QueueMemberForBg's own AddGroup/opcode call succeeded; invited/invitedT track each member's
        // own IsInvitedForBattlegroundQueueType() flip, independent of enterT below.
        std::vector<uint8> queuedOk;
        std::vector<uint8> invited;
        std::vector<uint32> invitedT;

        // Forming (section 3.2 step 4): each member's own arrival time (t, ms since queued; 0 = not
        // yet entered - distinguished from "entered at t=0" by enteredCount/formed below).
        std::vector<uint32> enterT;
        uint32 enteredCount = 0;
        bool formed = false;                         // all members counted in the match
        uint32 formedT = 0;
        uint32 lastQueueKickT = 0;                    // last time (t) the harness re-issued ScheduleQueueUpdate
        bool outsiderSeen = false;                    // a non-roster player showed up in the instance

        // Gates: the WAIT_JOIN -> IN_PROGRESS transition (section 3.4's t=0). gatesT is an absolute
        // getMSTime() timestamp - a live re-estimate (now + GetStartDelayTime()) while still counting
        // down, frozen at the tick the status actually flips, so [BGSTAGE] lines can show a signed
        // "ms since gates opened" (negative during prep) even before the true instant is known.
        bool gatesSeen = false;
        uint32 gatesT = 0;
        bool mountedLogged = false;

        // Per-member alive/position tracking (parallel to members/memberTeam).
        std::vector<uint8> wasAlive;                 // 1 = was alive as of the previous sample
        std::vector<uint32> deathAtT;                // t of the current death (0 = alive or never died)
        std::vector<float> lastX, lastY;             // position at the previous sample
        std::vector<uint32> staticMs;                // ms accumulated below the 2 yd/sample movement floor
        std::vector<uint8> gateclipLogged;            // dedupe: one "gateclip" line per member per match

        // Flag / score edges (index 0 = Alliance's flag/team, 1 = Horde's - matches TeamId).
        ObjectGuid lastFc[2];
        uint32 score[2] = { 0, 0 };
        uint32 pickups[2] = { 0, 0 }, returns[2] = { 0, 0 }, fckills[2] = { 0, 0 }, deaths[2] = { 0, 0 };
        uint32 firstPickupT = 0, firstCapT = 0;
        uint32 gateclipCount = 0, staticEvents = 0;
        uint32 mountedAtGate = 0;                     // sum of a+h, snapshotted at the "mountedAtGate" line
        uint32 staticSecTotal = 0;                    // running total for the end line's staticSec=

        // Wave-29 arena legs (#391/#709/#437/#632, t703): only used by an arena stage (queue=natural, or bgTypeId 6
        // queued by the harness as a skirmish). Identical on both cores.
        bool arena = false;                           // the stage plays an arena (natural rated or harness skirmish)
        int8 onlyTeam = -1;                           // onlyteam=A|H (queue=natural): only that half may queue (#632)
        bool aqTimeout = false;                       // aqtimeout=1: the roster guard lets the arena queue time out (#632)
        bool arenaLogged = false;                     // the one "[BGSTAGE] arena map=..." line
        std::vector<uint8> leftStart;                 // 1 = was > BG_ARENA_LEFTSTART_YD (3D) from its team start
        std::vector<uint8> vanishedLogged;            // one "vanished" line per member per match
        uint32 leftStartCount = 0;
        int32 engageT[2] = { -1, -1 };                // ms after the gates a member of team i first had an enemy roster
                                                      // member as its victim (-1 = never)
        bool arenaCounted = false;                    // arenaAlive/arenaDead hold a sample
        uint32 arenaAlive[2] = { 0, 0 };              // roster members alive / dead on the arena map at the last sample
        uint32 arenaDead[2] = { 0, 0 };

        AvTrack av;                                    // only populated when bgTypeId == 1 (AV)
    };

    // Generic vehicle-driving scenario ("drive", issue #207; reusable for #204 IoC / #205 SotA vehicle
    // parity tests). Drives a fixed list of waypoints on one map through MovementAction::MoveTo's
    // existing vehicle branch (MovementActions.cpp:365-399 - the same AC-parity path IoC/SotA bots
    // will use) via the BotScenarioVehicle.cpp VehicleDriveAction wrapper. The bot must already be a
    // controlling vehicle passenger (Vehicle::GetSeatForPassenger()->CanControl()) when it starts -
    // this scenario does not itself board anything.
    struct DriveTrack
    {
        std::vector<float> waypoints; // flattened x,y,z triples; leg i = waypoints[3i..3i+2]
        uint32 legIdx = 0;
        float arriveDist = 5.0f;      // "arrive=<yd>": vehicle-base distance that advances to the next leg
        uint32 legStartT = 0;         // t (ms since scenario start) the current leg began
    };

    // Quest 14071 "Rolling with my Homies" (issue #207): a full playerbot-driven run through accept
    // (the bot's own SearchQuestGiverAndAcceptOrReward) -> use the vehicle key (UseItemAction on item
    // 46856) -> drive to Izzy/Ace/Gobber and back (DriveTrack's VehicleDriveAction primitive) -> leave
    // the vehicle (VehicleActions.cpp's LeaveVehicleAction) -> turn in (SearchQuestGiverAndAcceptOrReward
    // again). See BotScenarioVehicle.cpp for the phase machine and the Kezan constants/coordinates.
    struct HotRodTrack
    {
        uint8 phase = 0;                // 0 arm, 1 accept, 2 use key/board, 3 drive to homies, 4 drive
                                         // back to Megs, 5 leave vehicle, 6 turn in
        uint32 phaseT = 0;               // t (ms since scenario start) the current phase began
        uint32 readyT = 0;               // phase 0: settle-hold timer
        uint32 lastActionT = 0;          // throttles UseItemAction/LeaveVehicleAction retries
        uint32 lastKeepaliveT = 0;       // throttles the DoQuest status re-assert (see HotRodUpdate)
        uint8 homieIdx = 0;              // 0 Izzy, 1 Ace, 2 Gobber - current drive-to-homies leg
        uint32 lastLogT = 0;             // throttles the periodic [HOTROD] evidence line
        bool keyUsedOnce = false;        // true once UseItemAction has cast spell 66393 successfully
        uint32 acceptT = 0, keyUsedT = 0, boardT = 0;
        uint32 homieT[3] = { 0, 0, 0 };  // pickup timestamps, indexed like homieIdx
        uint32 returnT = 0, leftVehicleT = 0, rewardT = 0;
    };

    // #201 (#613): the "duelspot" scenario (BotScenarioDuel.cpp). Static per-run settings; nothing here
    // changes after the parse, so OnUpdate needs no per-tick write-back for it.
    struct DuelSpotTrack
    {
        uint32 spotIdx = 0;       // index into sPlayerbotAIConfig->duelSpots
        bool seeded = false;      // "seeded=1": ChangeToDuelSpot(idx, true)
        bool startDuel = true;    // "startduel=0|1": keep the "start duel" strategy on (default 1)
        uint32 scanMs = 0;        // "scan=<ms>": SetScanOverride (0 = config)
        uint32 rematchS = 0;      // "rematch=<s>": SetRematchOverride (0 = config)
        uint32 secs = 0;          // "secs=<n>": end the scenario after n seconds (0 = until stop)
    };

    // Everything a running scenario/probe needs for one bot, keyed by the bot's guid counter in
    // the side table (BotScenario.cpp's g_bots). Replaces the scenario* fields that used to live in
    // NewRpgInfo - moving the table out of NewRpgInfo is what lets it survive
    // PlayerbotAI::Reset(full) (which recreates the whole NewRpgInfo) without a manual save/restore
    // dance, and what keeps NewRpgInfo.h out of the "touch this, rebuild the module" path.
    struct ScenarioState
    {
        std::string botName; // for "botprobe list" and error messages without re-resolving the bot

        // --- .botscenario (HARNESS-SPEC-botscenario-v2 G1-G5) ---
        ScenarioKind kind = ScenarioKind::None;
        uint32 startT = 0;
        uint32 lastTraceT = 0;
        uint32 destMapId = 0;
        float destX = 0.0f;
        float destY = 0.0f;
        float destZ = 0.0f;
        uint32 wanderMs = 0;  // >0: wander/attack scenario, ends after this long instead of on arrival
        bool lock = false;    // NewRpgStatusUpdateAction must not change status while true

        // Deterministic-mode fields (G1-G4): only meaningful while kind != None.
        uint32 allowMask = 0;     // bit n set = NewRpgStatus n may be chosen by RandomChangeStatus; 0 = no restriction
        bool pickNearest = false; // deterministic target picks (nearest candidate instead of random)
        bool grindSet = false;    // "grind=x,y,z": fixed grind point on the scenario map
        bool noFight = false;     // "nofight": attack anything is not useful for this bot
        bool groundOnly = false;  // "ground": the bot must not use a flying mount while this scenario runs
        float grindX = 0.0f, grindY = 0.0f, grindZ = 0.0f;
        uint32 pickCount = 0;     // deterministic MoveRandomNear golden-angle sequence index
        ObjectGuid lastQg;        // de-dupes [WANDER] kind=qg lines

        // Attack scenario (G5 / issue #129).
        uint32 attackEntry = 0; // creature entry to (re-)acquire
        uint32 attackGuid = 0;  // acquired target creature guid counter (0 = none yet)
        ObjectGuid attackTarget; // full guid of the acquired target: "killed" = this creature dead/gone
        bool attackSpawn = false; // "spawn": summon a fresh target next to the scenario point instead of searching
        bool attackUnflag = false; // "unflag": clear NON_ATTACKABLE/IMMUNE_TO_PC/IMMUNE_TO_NPC on the spawned target (#636)
        float attackSpawnOffset = 8.0f; // "off=<yd>": how far east of the scenario point "spawn" puts the target (#827)

        // Die scenario (death-revive-parity.md section 4).
        DieTrack die;             // Die scenario progress (persisted by OnUpdate)

        // Taxi / quest scenarios (research\taxi-quest-stage-design.md sections A.4/B.4).
        TaxiTrack taxi;
        QuestTrack quest;
        TransportTrack transport;
        BgTrack bg;
        DriveTrack drive;
        HotRodTrack hotrod;
        DuelSpotTrack duelspot; // #201 (#613)

        // --- .botprobe (HARNESS-SPEC-botscenario-v3 section 4) ---
        bool probeEnabled = false;
        uint32 probeIntervalMs = 1000;
        uint32 probeStartT = 0;
        uint32 lastProbeT = 0;
        std::string lastActionName;   // last Engine action name (Engine::LogAction OK/FAILED/IMPOSSIBLE/USELESS)
        std::string lastActionResult;
        bool hasLastCast = false;     // last PlayerbotAI::CastSpell verdict
        uint32 lastCastSpell = 0;
        int32 lastCastResult = 0;
        // #319 (latch, both cores): the transport entry the bot was aboard on ANY UpdateAI tick since the
        // previous [PROBE] row, and how many ticks that was. A 500 ms sample misses a short aboard window;
        // OnUpdate sets these every tick and clears them once a row is emitted; EmitProbe prints tseen= from them.
        uint32 tlatchEntry = 0;
        uint32 tlatchTicks = 0;

        bool IsActive() const { return kind != ScenarioKind::None || probeEnabled; }
    };

    // One scenario kind's behaviour, registered into the table described by
    // HARNESS-SPEC-botscenario-v3-module section 3 ("a small table of {name, parse fn, update fn,
    // reset fn}"). All three take plain function pointers - every handler so far is a free function
    // with no captured state, since all of it lives in the ScenarioState the caller passes in.
    struct ScenarioHandler
    {
        char const* name; // the "botscenario <bot> <name> ..." token ("goto", "wander", "attack", ...)

        // Parses the command tail (everything after the scenario-name token) from `args`, and on
        // success arms `state` (kind, destination, timers, ...) and performs whatever immediate
        // action starts the scenario (teleport, RPG-status change). Returns false - and must not
        // have modified `state` - on a usage error, after sending the usage line to `handler`.
        bool (*parse)(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& args,
                      ScenarioState& state);

        // Runs on every trace tick while state.kind matches this handler, after the shared
        // [TRACE] line has been logged. Decides whether the scenario ended this tick
        // (arrived/done/timeout/killed) and if so logs the single "[SCENARIO] <bot> <word> ..."
        // line itself and returns true; OnUpdate clears the scenario fields (and, if no probe is
        // independently enabled, the whole table entry) either way.
        bool (*update)(Player* bot, PlayerbotAI* botAI, ScenarioState& state, uint32 now, uint32 t);

        // Re-arms the RPG status this scenario needs after PlayerbotAI::Reset(full) recreates the
        // RPG info (NewRpgInfo), restarting the scenario's clock. Called from OnReset.
        void (*rearm)(Player* bot, PlayerbotAI* botAI, ScenarioState& state);
    };

    // Registers a scenario handler under both its command name and its kind. Safe to call multiple
    // times for handlers that share a name (the three wander sub-kinds all register "wander").
    // Meyer's-singleton backed, so registration order relative to other static initializers in
    // other translation units does not matter.
    void RegisterScenarioHandler(ScenarioKind kind, ScenarioHandler const& handler);
    // Looks a handler up by its "botscenario <bot> <name> ..." token, for HandleScenarioCommand's
    // dispatch. Returns nullptr if no handler with that name is registered.
    ScenarioHandler const* FindScenarioHandlerByName(std::string const& name);
    // Looks a handler up by kind, for OnUpdate/OnReset's dispatch. Always returns nullptr for
    // ScenarioKind::None (never has a handler).
    ScenarioHandler const* FindScenarioHandler(ScenarioKind kind);

    // Defined in BotScenarioVehicle.cpp (issue #207): registers the "drive"/"hotrod" handlers.
    // Static-initializer self-registration alone is not enough here, because this module links
    // BotScenario*.cpp out of a static library (modules.lib) - a .obj that exports no symbol anyone
    // else references (BotScenarioVehicle.cpp's only effect was a file-static constructor's side
    // effect) is dropped by the linker before that constructor ever runs, regardless of registration
    // order. Called once from BotScenario.cpp's own HandlerRegistrar so the reference forces the
    // linker to pull the .obj in.
    void RegisterVehicleScenarioHandlers();

    // #201 (#613): defined in BotScenarioDuel.cpp. RegisterDuelScenarioHandlers registers the "duelspot"
    // kind and, like RegisterVehicleScenarioHandlers above, exists to force that .obj to link (called
    // from BotScenario.cpp's HandlerRegistrar). The rest are BotScenario.cpp's hooks into the duel harness.
    void RegisterDuelScenarioHandlers();
    // "duelinfo" / "duelhp" / "startduel": verbs that accept any online player (bot or real), dispatched
    // from HandleScenarioCommand ahead of the botAI gate.
    bool HandleDuelScenarioCommand(ChatHandler* handler, std::string const& name, std::string const& scenario,
                                   std::istringstream& in);
    // EmitProbe's `duel=<state>:<opp|-> dspot=<idx|->` fields (botAI may be null for a masterprobe player).
    void DuelProbeFields(Player* p, PlayerbotAI* botAI, std::string& duel, std::string& dspot);
    // OnUpdate: 250 ms-rate-limited sweep (due [DUEL] SAMPLE rows, decline fallback).
    void DuelSweep(uint32 now);
    // Creates the harness PlayerScript once (first .botscenario / .botprobe command).
    void EnsureDuelHarnessScript();
    // Optional module-loader entry; same effect as EnsureDuelHarnessScript().
    void AddSC_BotScenarioDuel();
    // #793: defined in BotScenarioSpellWatch.cpp. `.botscenario - spellwatch <id,id,...> on|off | off | list`, dispatched
    // from HandleScenarioCommand ahead of every player lookup (the watch set is server-global).
    bool HandleSpellWatchCommand(ChatHandler* handler, std::istringstream& in);
    // "stop": clears the bot's scan / rematch overrides.
    void DuelScenarioStopped(Player* bot);
    // Defined in BotScenario.cpp: true when `guid` has a side-table entry (a scenario, .botprobe or
    // masterprobe); `t` is that entry's own clock (ms), the same t= its [PROBE] rows carry.
    bool ScenarioClock(ObjectGuid guid, uint32& t);
}

#endif
