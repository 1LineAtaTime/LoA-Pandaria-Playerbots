/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/NewRpgInfo.h) into the
 * Legends of Azeroth Pandaria fork. Only the runtime roaming status store is kept; quest/flight
 * related sub-states are retained for future work.
 */

#ifndef _PLAYERBOT_NEWRPGINFO_H
#define _PLAYERBOT_NEWRPGINFO_H

#include "ObjectGuid.h"
#include "PlayerbotAIConfig.h"
#include "Timer.h"
#include "WorldPosition.h"

#include <unordered_map>
#include <variant>

class Quest;

// NewRpgStatus is defined in PlayerbotAIConfig.h (needed there for RpgStatusProbWeight[]).

struct NewRpgInfo
{
    NewRpgInfo() : data(Idle{}) {}
    ~NewRpgInfo() = default;

    // RPG_GO_GRIND
    struct GoGrind
    {
        WorldPosition pos{};
    };
    // RPG_GO_CAMP
    struct GoCamp
    {
        WorldPosition pos{};
    };
    // RPG_WANDER_NPC
    struct WanderNpc
    {
        ObjectGuid npcOrGo{};
        uint32 lastReach{0};
    };
    // RPG_WANDER_RANDOM
    struct WanderRandom
    {
        WanderRandom() = default;
    };
    // RPG_DO_QUEST
    struct DoQuest
    {
        uint32 questId{0};
        Quest const* quest{nullptr};
        int32 objectiveIdx{0};
        WorldPosition pos{};
        uint32 lastReachPOI{0};
    };
    // RPG_TRAVEL_FLIGHT
    struct TravelFlight
    {
        uint32 flightMasterEntry{0};
        WorldPosition flightMasterPos{};
        std::vector<uint32> path;
        bool inFlight{false};
    };
    // RPG_REST
    struct Rest
    {
        Rest() = default;
    };
    // RPG_OUTDOOR_PVP
    struct OutdoorPvP
    {
        ObjectGuid::LowType capturePointSpawnId{0};
    };
    // RPG_DUEL_SPOT
    // #201 (#612): AC NewRpgInfo.h:68-71 (OutdoorPvP) pattern. spotIndex = index into
    // sPlayerbotAIConfig->duelSpots; seeded = placed there by DuelSpotMgr::Update (#615); nextScanMs = getMSTime() of
    // the next candidate scan (0 = scan now); noRematchUntil = opponent -> getMSTime() until which not to challenge again.
    struct DuelSpot
    {
        uint32 spotIndex = 0;
        bool seeded = false;
        uint32 nextScanMs = 0;
        std::unordered_map<ObjectGuid, uint32> noRematchUntil;
    };
    struct Idle
    {
    };

    uint32 startT{0};  // start timestamp of the current status

    // MOVE_FAR stuck tracking
    float nearestMoveFarDis{FLT_MAX};
    uint32 stuckTs{0};
    uint32 stuckAttempts{0};
    WorldPosition moveFarPos;
    // END MOVE_FAR

    // Debug scenario (.botscenario) / probe (.botprobe) state used to live here (scenarioStartT ...
    // scenarioAttackEntry, ArmWanderScenario) so NewRpgStatusUpdateAction/RandomChangeStatus/etc.
    // could read it inline. HARNESS-SPEC-botscenario-v3-module moved all of it into Scenario/
    // BotScenario.cpp's own side table (keyed by bot guid), so a harness/probe change no longer
    // touches this header - see BotScenario.h for the replacement API.

    using RpgData = std::variant<
        Idle,
        GoGrind,
        GoCamp,
        WanderNpc,
        WanderRandom,
        DoQuest,
        Rest,
        TravelFlight,
        OutdoorPvP,
        DuelSpot  // #201 (#612): AC NewRpgInfo.h:114 pattern
    >;
    RpgData data;

    NewRpgStatus GetStatus() const;
    static NewRpgStatus StatusFromString(std::string const& name);
    bool HasStatusPersisted(uint32 maxDuration) const { return GetMSTimeDiffToNow(startT) > maxDuration; }
    void ChangeToGoGrind(WorldPosition pos);
    void ChangeToGoCamp(WorldPosition pos);
    void ChangeToWanderNpc();
    void ChangeToWanderRandom();
    void ChangeToDoQuest(uint32 questId, Quest const* quest);
    void ChangeToTravelFlight(uint32 flightMasterEntry, WorldPosition flightMasterPos, std::vector<uint32> path);
    void ChangeToOutdoorPvp(ObjectGuid::LowType capturePointSpawnId = 0);
    void ChangeToDuelSpot(uint32 spotIndex, bool seeded = false);  // #201 (#612)
    void ChangeToRest();
    void ChangeToIdle();
    void Reset();
    void SetMoveFarTo(WorldPosition pos);
    std::string ToString() const;
};

struct NewRpgStatistic
{
    uint32 questAccepted{0};
    uint32 questCompleted{0};
    uint32 questAbandoned{0};
    uint32 questRewarded{0};
    uint32 questDropped{0};
};

#endif
