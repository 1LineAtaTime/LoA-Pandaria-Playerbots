/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Mgr/Travel/TravelMgr.cpp) into the
 * Legends of Azeroth Pandaria fork. Carries the parts upstream bots use at runtime - flight
 * masters, taxi destinations, travel hubs - plus the cross-map transfer bookkeeping the travel
 * node graph needs. See TravelMgr.h for what is deliberately not ported.
 */

#include "TravelMgr.h"

#include "ChatHelper.h"
#include "Config.h"
#include "Creature.h"
#include "DBCEnums.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Field.h"
#include "GameObjectData.h"
#include "CellImpl.h"
#include "Log.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
// #268 S1: AI_VALUE / AI_VALUE2 / AI_VALUE2_LAZY / GET_PLAYERBOT_AI macros and AiObjectContext.
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
// #268 S1: Quest, QuestObjective, QuestObjectives and the QUEST_INFO_* ids IsInstanceQuest() keys on.
#include "QuestDef.h"
#include "QuestValues.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "SharedValueContext.h"
#include "Timer.h"
#include "TravelNode.h"
#include "World.h"

#include <atomic>

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <numeric>
#include <random>
#include <set>
// #268 S4: std::unordered_set for GetCityLocations' validBankerCities (AC TravelMgr.cpp:4510).
#include <deque>
#include <unordered_map>
#include <unordered_set>
// #268 S5: std::shared_lock/std::shared_mutex for TravelMgr::Clear()'s HashMapHolder guard.
#include <shared_mutex>
#include <sstream>
#include <tuple>

namespace
{
    // taxi-quest-stage-design.md G-T2 / AC parity (AC Mgr/Travel/TravelMgr.cpp:31-50): the fixed
    // capital-city list AC's GetOptimalFlightDestinations draws from for its ProbTeleToBankers
    // branch. Zone/area ids verified byte-identical against this fork's AreaTable.dbc (2026-09-25:
    // all ten carry AREA_FLAG_CAPITAL there too), so the same table is ported unchanged rather than
    // re-derived. `team` is a raw team id (ALLIANCE/HORDE, matching how the rest of this file already
    // takes a team argument), with 0 standing in for AC's TEAM_NEUTRAL (open to both factions).
    struct Capital
    {
        uint32 zoneId;
        uint32 team;
        // #268 S4: AC Mgr/Travel/TravelMgr.cpp:36 `std::vector<uint16> bankers`. AC's third field
        // `char const* name` is dropped: nothing in AC reads it (the only two readers of a Capital,
        // FindCapitalByZone and FindCapitalByBanker, use zoneId and bankers), and the table already
        // names each city in a trailing comment.
        std::vector<uint16> bankers;
    };

    Capital const g_capitals[] = {
        {1519, ALLIANCE, {2455, 2456, 2457}},                     // Stormwind City
        {1537, ALLIANCE, {2460, 2461, 5099}},                     // Ironforge
        {1657, ALLIANCE, {4155, 4208, 4209}},                     // Darnassus
        {3557, ALLIANCE, {17773, 18350, 16710}},                  // The Exodar
        // #944 (MoP-only data): AC's Orgrimmar bankers 3320/3309/3318 are gone since the Cata rebuild (0 spawns), so no Horde
        // bank teleport ever chose Orgrimmar. The Valley of Strength bankers (zone 1637, area 5170) take their place.
        {1637, HORDE, {44851, 44852, 44853, 44854, 44856}},       // Orgrimmar
        {1497, HORDE, {4549, 2459, 2458, 4550}},                  // Undercity
        {1638, HORDE, {2996, 8356, 8357}},                        // Thunder Bluff
        {3487, HORDE, {17631, 17632, 17633, 16615, 16616, 16617}},// Silvermoon City
        {3703, 0, {19246, 19338, 19034, 19318}},                  // Shattrath City (neutral)
        {4395, 0, {30604, 30605, 30607, 28675, 28676, 28677, 29530}}, // Dalaran (neutral)
        // #779 (MoP-only content, extends AC's list): the Pandaria capitals, Shrine of Seven Stars (Alliance, taxi node
        // 1057) and Shrine of Two Moons (Horde, node 1058). Their flight masters sit in parent zone 5840 (Vale of Eternal
        // Blossoms), which is the zone GetFlightNodesInZone matches; the faction caches keep each team on its own shrine.
        // Without it the ProbTeleToBankers roll in GetOptimalFlightDestinations found no capital on map 870 and failed,
        // once in CheckRpgStatusAvailable and again in RandomChangeStatus (w41: Pandaria flight picks 0% to a capital vs
        // 19-25% on the other continents; TRAVEL_FLIGHT entries per availability check 15% vs 26-30%). The banker list
        // stays empty, so the city-banker weighting (GetCityLocations, GetCityWeight) is unchanged.
        {5840, 0, {}},                                            // Vale of Eternal Blossoms shrines (MoP)
        // #947 (MoP-only, extends AC's Dalaran row): the shrine vaultkeepers as weighted bank cities. With
        // EnableWeightTeleToCityBankers = 1 (conf.dist), GetCityLocations only picks a city that owns one of the level's
        // bankers; the 5840 row above has none, so once #947 added the Stormwind/Orgrimmar bankers to 85+, every 85+ bank
        // teleport went to Stormwind/Orgrimmar and none to a shrine (2026-10-28 log: 38 of 38). The zone ids are the
        // vaultkeepers' (areas 6519 Summer's Rest / 6484 The Golden Lantern, whose parents are 6141 / 6553), the zone a bot
        // standing at them reports. No flight master stands in these zones, so the capital-flight roll skips them.
        {6141, HORDE, {63964, 63965, 63966}},                     // Shrine of Two Moons (MoP)
        {6553, ALLIANCE, {63967, 63968, 63969, 63970, 63971, 64023, 64024}}, // Shrine of Seven Stars (MoP)
    };

    // AC TravelMgr.cpp:60-67.
    Capital const* FindCapitalByBanker(uint32 bankerEntry)
    {
        for (Capital const& capital : g_capitals)
            for (uint16 bankerId : capital.bankers)
                if (bankerId == bankerEntry)
                    return &capital;

        return nullptr;
    }

    // AC TravelMgr.cpp:52-58.
    Capital const* FindCapitalByZone(uint32 zoneId)
    {
        for (Capital const& capital : g_capitals)
            if (capital.zoneId == zoneId)
                return &capital;

        return nullptr;
    }

    // #268 S4. AC reads these eleven numbers as sPlayerbotAIConfig members
    // (AC PlayerbotAIConfig.cpp:195-205). They are read file-locally here instead, for the same
    // reason AiPlayerbot.ProbTeleToBankers above is: PlayerbotAIConfig.h is included by most of the
    // module, so adding eleven members there costs a full-module rebuild for what is two config
    // reads. The keys, types and defaults are AC's, byte for byte
    // (AC PlayerbotAIConfig.cpp:195-205); note AC's own defaults disagree with
    // AC conf/playerbots.conf.dist (which ships EnableWeightTeleToCityBankers = 1 while the code
    // default is false), and the code default is what is used when the key is absent.
    struct CityBankerWeights
    {
        bool enabled = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableWeightTeleToCityBankers", false);
        int stormwind = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToStormwindWeight", 2);
        int ironforge = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToIronforgeWeight", 1);
        int darnassus = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToDarnassusWeight", 1);
        int exodar = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToExodarWeight", 1);
        int orgrimmar = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToOrgrimmarWeight", 2);
        int undercity = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToUndercityWeight", 1);
        int thunderBluff = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToThunderBluffWeight", 1);
        int silvermoonCity = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToSilvermoonCityWeight", 1);
        int shattrathCity = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToShattrathCityWeight", 1);
        int dalaran = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToDalaranWeight", 1);
        // #947 (MoP-only keys, same pattern): the shrines are the 85+ bank cities next to Stormwind/Orgrimmar (weight 2).
        // AC sends every 71-80 bank teleport to Dalaran; 6 against 2 sends three of four 85+ bank teleports to the shrine.
        int shrineOfTwoMoons = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToShrineOfTwoMoonsWeight", 6);
        int shrineOfSevenStars = sConfigMgr->GetIntDefault("AiPlayerbot.TeleToShrineOfSevenStarsWeight", 6);
    };

    CityBankerWeights const& CityBankerWeightConfig()
    {
        static CityBankerWeights const cfg;
        return cfg;
    }

    // AC TravelMgr.cpp:69-85. AC switches on its AreaDefines.h AREA_* names; the fork has no
    // AreaDefines.h, so the numeric zone ids already used by g_capitals are switched on instead -
    // the same substitution the rest of this file makes (see PrepareZone2LevelBracket).
    int GetCityWeight(uint32 zoneId)
    {
        CityBankerWeights const& cfg = CityBankerWeightConfig();

        switch (zoneId)
        {
            case 1519: return cfg.stormwind;      // Stormwind City
            case 1537: return cfg.ironforge;      // Ironforge
            case 1657: return cfg.darnassus;      // Darnassus
            case 3557: return cfg.exodar;         // The Exodar
            case 1637: return cfg.orgrimmar;      // Orgrimmar
            case 1497: return cfg.undercity;      // Undercity
            case 1638: return cfg.thunderBluff;   // Thunder Bluff
            case 3487: return cfg.silvermoonCity; // Silvermoon City
            case 3703: return cfg.shattrathCity;  // Shattrath City
            case 4395: return cfg.dalaran;        // Dalaran
            case 6141: return cfg.shrineOfTwoMoons;   // Shrine of Two Moons (#947)
            case 6553: return cfg.shrineOfSevenStars; // Shrine of Seven Stars (#947)
        }

        return 0;
    }

    // Read once, lazily - same pattern BotScenario.cpp uses for its own test-only config. This is the
    // only reader of AiPlayerbot.ProbTeleToBankers on this fork; the key already sits in
    // playerbots.conf.dist (0.25) with nothing consuming it before this fix.
    float ProbTeleToBankers()
    {
        static float const v = sConfigMgr->GetFloatDefault("AiPlayerbot.ProbTeleToBankers", 0.25f);
        return v;
    }
}

// ================================================================================================
// #268 slice S3 - the two helpers LoadQuestTravelTable() and the S1 isActive() bodies need.
// ================================================================================================
namespace
{
    // ALIASING FIX (the trap the #268 plan assigned to S3). TravelDestination::points is a
    // std::vector<WorldPosition*> and AC fills it with `&loopLocal`, so every destination AC builds
    // aliases ONE reused stack slot that dies with the iteration (AC .cpp:2072-2076, :2284-2295,
    // :2334-2363). S1 and S2 created no points at all, so S3 is the slice that owns them. Every
    // point handed to addPoint() goes through here and is heap-allocated, so it outlives the loop;
    // TravelDestination::~TravelDestination() (TravelMgr.cpp) frees them, which makes Clear()'s
    // existing `delete dest` reclaim the points too. See the invariant note on the destructor in
    // TravelMgr.h.
    WorldPosition* OwnedPoint(WorldPosition const& pos)
    {
        return new WorldPosition(pos);
    }

    // #703 (boot travel caches 114 s): MoP-only startup optimisation, documented divergence from AC.
    //
    // AC PrepareDestinationCache (Mgr/Travel/TravelMgr.cpp:4670) calls Map::GetAreaId for every spawn. On a base
    // map with no grids yet that runs EnsureGridCreated -> LoadMapAndVMap, i.e. it reads the .map, the vmap tile
    // (+ its models) and the mmap tile of EVERY grid that holds a spawn: ~1,840 grids over the 10 MoP bot maps
    // (3.7 GB of mmaps on disk). Boot logs show 17.6-118.8 s for the same code, i.e. it is disk-cache bound.
    //
    // In the CREATURES (grind-cell) arm the area only keys tempCreatureCache -> creatureSpawnsByTemplate,
    // which nothing reads in AC or here (#611 note below). So that arm reads only the terrain area grid from
    // the .map file (no NGrid, no vmap, no mmap), keeps 256 area ids per grid (512 bytes) and drops the file.
    // It is Map::GetAreaId minus the WMO (vmap) override: same grid selection (Map::GetGrid), same chunk
    // index (GridMap::getArea), same fallback to MapEntry::AreaTableID. The flight-master / innkeeper arm
    // keeps the exact Map::GetAreaId because its zone id picks the hub bracket (Undercity, Dalaran and
    // other WMO cities would resolve to the zone outside the WMO).
    class TerrainAreaCache
    {
    public:
        uint32 GetAreaId(uint32 mapId, float x, float y)
        {
            uint32 areaId = 0;
            // Map::GetGrid refuses out-of-world coordinates (#105/#435) -> gridAreaId stays 0.
            if (Trinity::IsValidMapCoord(x, y))
            {
                GridCoord const p = Trinity::ComputeGridCoord(x, y);
                uint32 const gx = (MAX_NUMBER_OF_GRIDS - 1) - p.x_coord;   // Map::GetGrid file indices
                uint32 const gy = (MAX_NUMBER_OF_GRIDS - 1) - p.y_coord;
                std::array<uint16, 256> const& areas = Grid(mapId, gx, gy);

                // GridMap::getArea's chunk index, verbatim.
                float const fx = 16 * (32 - x / SIZE_OF_GRIDS);
                float const fy = 16 * (32 - y / SIZE_OF_GRIDS);
                int const lx = int(fx) & 15;
                int const ly = int(fy) & 15;
                areaId = areas[lx * 16 + ly];
            }

            // Map::GetAreaId: `if (!areaId) areaId = i_mapEntry->AreaTableID;`
            if (!areaId)
                if (MapEntry const* mapEntry = sMapStore.LookupEntry(mapId))
                    areaId = mapEntry->AreaTableID;

            return areaId;
        }

    private:
        std::array<uint16, 256> const& Grid(uint32 mapId, uint32 gx, uint32 gy)
        {
            uint64 const key = (uint64(mapId) << 16) | (uint64(gx) << 8) | uint64(gy);
            auto itr = _grids.find(key);
            if (itr != _grids.end())
                return itr->second;

            std::array<uint16, 256>& areas = _grids[key];
            areas.fill(0);

            // Map::LoadMap's file name. A missing file loads as an empty GridMap (getArea -> 0), as in the core.
            char path[1024];
            snprintf(path, sizeof(path), "%smaps/%04u_%02u_%02u.map", sWorld->GetDataPath().c_str(), mapId, gx, gy);
            GridMap gridMap;
            if (gridMap.loadData(path))
            {
                // Sample each 16x16 chunk at its centre through GridMap::getArea, so the stored table is exactly
                // what getArea returns for any point of that chunk: fx = 16 * gx + lx + 0.5 inverts its formula.
                for (uint32 lx = 0; lx < 16; ++lx)
                    for (uint32 ly = 0; ly < 16; ++ly)
                    {
                        float const x = (32.0f - (float(gx) + (float(lx) + 0.5f) / 16.0f)) * SIZE_OF_GRIDS;
                        float const y = (32.0f - (float(gy) + (float(ly) + 0.5f) / 16.0f)) * SIZE_OF_GRIDS;
                        areas[lx * 16 + ly] = gridMap.getArea(x, y);
                    }
            }
            // ~GridMap frees the height/liquid/area buffers here; only the 512-byte table is kept.
            return areas;
        }

        std::unordered_map<uint64, std::array<uint16, 256>> _grids;
    };


}

void SpawnGridIndex::Build()
{
    if (_built)
        return;

    _cells.clear();

    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        CreatureData const& data = itr.second;
        _cells[CellKey(data.mapId, CellOf(data.posX), CellOf(data.posY))].push_back(&data);
    }

    _built = true;

    TC_LOG_INFO("playerbots", ">> Travel spawn index: %u cells over %u creature spawns",
                uint32(_cells.size()), uint32(sObjectMgr->GetAllCreatureData().size()));
}

void SpawnGridIndex::Clear()
{
    _cells.clear();
    _built = false;
}

void SpawnGridIndex::Query(uint32 mapId, float x, float y, float radius, std::vector<CreatureData const*>& out) const
{
    if (!_built)
        return;

    int32 const minX = CellOf(x - radius);
    int32 const maxX = CellOf(x + radius);
    int32 const minY = CellOf(y - radius);
    int32 const maxY = CellOf(y + radius);
    float const radiusSq = radius * radius;

    for (int32 cx = minX; cx <= maxX; ++cx)
    {
        for (int32 cy = minY; cy <= maxY; ++cy)
        {
            auto itr = _cells.find(CellKey(mapId, cx, cy));
            if (itr == _cells.end())
                continue;

            for (CreatureData const* data : itr->second)
            {
                float const dx = data->posX - x;
                float const dy = data->posY - y;
                if (dx * dx + dy * dy <= radiusSq)
                    out.push_back(data);
            }
        }
    }
}

std::vector<CreatureData const*> TravelMgr::GetCreaturesNear(WorldPosition const& pos, float radius) const
{
    std::vector<CreatureData const*> result;
    _spawnIndex.Query(pos.GetMapId(), pos.GetPositionX(), pos.GetPositionY(), radius, result);
    return result;
}

// ---------------------------------------------------------------------------------------------
// Navigation caches (issue #32)
//
// This mirrors the part of upstream AC's TravelMgr that its bots actually use at runtime:
// flight-master lookup, taxi destination selection and innkeeper "travel hubs". Upstream builds
// these in TravelMgr::Init()/PrepareDestinationCache(); the walk-node graph in TravelNode.cpp is
// built alongside but only consumed by debug commands.
//
// One deliberate difference: upstream hard-codes its zone -> level bracket table, and that table
// is 3.3.5 (it stops at Wrath). This fork takes the brackets from `playerbot_farming_zone`
// instead, which is the 5.4.8 data already loaded by RandomPlayerbotMgr and
// covers Cataclysm and Pandaria zones.
// ---------------------------------------------------------------------------------------------

void TravelMgr::Init()
{
    if (_initialized)
        return;

    _initialized = true;

    uint32 const startedAt = getMSTime();

    PrepareZone2LevelBracket();
    PrepareDestinationCache();

    // Taxi reachability (BFS over TaxiPath.dbc). Cheap - DBC only, no map data needed.
    sTravelNodeMap.InitTaxiGraph();

    TC_LOG_INFO("playerbots", ">> Playerbots travel caches built in %u ms: %u zone brackets, %u/%u flight masters "
                              "(alliance/horde)",
                GetMSTimeDiffToNow(startedAt), uint32(zone2LevelBracket.size()),
                uint32(allianceFlightMasterCache.size()), uint32(hordeFlightMasterCache.size()));
}

// #950 (MoP-only): a read-only copy of zone2LevelBracket for RandomPlayerbotMgr's login re-place, published once at
// the end of PrepareZone2LevelBracket so TravelMgr.h does not change. Free function, declared extern at its caller.
namespace
{
std::map<uint32, std::pair<uint32, uint32>> g_publishedBrackets;
std::atomic<bool> g_bracketsPublished{false};
}

bool PlayerbotZoneLevelBracketContains(uint32 zoneId, uint32 level)
{
    if (!g_bracketsPublished.load(std::memory_order_acquire))
        return false;
    auto const itr = g_publishedBrackets.find(zoneId);
    return itr != g_publishedBrackets.end() && level >= itr->second.first && level <= itr->second.second;
}

void TravelMgr::PrepareZone2LevelBracket()
{
    // AC TravelMgr::PrepareZone2LevelBracket (Mgr/Travel/TravelMgr.cpp:4558-4632): the hard-coded bracket of
    // every Classic/TBC/WotLK levelling zone (numeric zone ids; AC's AREA_* names in the comments). Without it
    // levels 26-59 (EK), 21-66 (Outland) and 71-84 had no hubs, so the revive teleport (RandomPlayerbotMgr::
    // Revive -> RandomTeleportGrindForLevel) and GO_CAMP found no destination.
    static std::map<uint32, LevelBracket> const acBrackets = {
        // starter zones: Dun Morogh, Elwynn, Durotar, Tirisfal, Teldrassil, Mulgore, Eversong, Azuremyst
        {1, {5, 12}}, {12, {5, 12}}, {14, {5, 12}}, {85, {5, 12}}, {141, {5, 12}}, {215, {5, 12}},
        {3430, {5, 12}}, {3524, {5, 12}},
        // Barrens, Loch Modan, Westfall, Silverpine, Darkshore, Ghostlands, Bloodmyst
        {17, {10, 25}}, {38, {10, 20}}, {40, {10, 21}}, {130, {10, 23}}, {148, {10, 21}}, {3433, {10, 22}},
        {3525, {10, 21}},
        // Duskwood, Wetlands, Redridge, Hillsbrad, Ashenvale, Thousand Needles, Stonetalon
        {10, {19, 33}}, {11, {21, 30}}, {44, {16, 28}}, {267, {20, 34}}, {331, {18, 33}}, {400, {24, 36}},
        {406, {16, 29}},
        // Badlands, Swamp of Sorrows, Dustwallow, Azshara, Stranglethorn, Arathi, Hinterlands, Searing Gorge,
        // Feralas, Desolace, Tanaris
        {3, {36, 46}}, {8, {36, 46}}, {15, {35, 46}}, {16, {45, 52}}, {33, {32, 47}}, {45, {30, 42}},
        {47, {42, 51}}, {51, {45, 51}}, {357, {40, 52}}, {405, {30, 41}}, {440, {41, 52}},
        // Blasted Lands, Western/Eastern Plaguelands, Burning Steppes, Felwood, Un'Goro, Winterspring, Silithus
        {4, {52, 57}}, {28, {50, 60}}, {46, {51, 60}}, {139, {54, 62}}, {361, {47, 57}}, {490, {49, 56}},
        {618, {54, 61}}, {1377, {54, 63}},
        // Hellfire, Nagrand, Terokkar, Shadowmoon, Zangarmarsh, Blade's Edge, Netherstorm, Isle of Quel'Danas
        {3483, {58, 66}}, {3518, {64, 70}}, {3519, {62, 73}}, {3520, {66, 73}}, {3521, {60, 67}},
        {3522, {64, 73}}, {3523, {67, 73}}, {4080, {68, 73}},
        // Dragonblight, Zul'Drak, Storm Peaks, Icecrown, Grizzly Hills, Howling Fjord, Crystalsong, Borean
        // Tundra, Sholazar, Wintergrasp
        {65, {71, 77}}, {66, {74, 80}}, {67, {77, 80}}, {210, {77, 80}}, {394, {72, 78}}, {495, {68, 74}},
        {2817, {77, 80}}, {3537, {68, 75}}, {3711, {75, 80}}, {4197, {79, 80}},
    };
    // Cataclysm zones (MoP-only content, extending AC's pattern): Mount Hyjal, Vashj'ir (its Kelp'thar /
    // Shimmering / Abyssal areas resolve to zone 5146), Deepholm, Uldum, Twilight Highlands, Tol Barad
    // Peninsula; and Stranglethorn Vale, which Cataclysm made zone 5339 (AC's 33 is now its Northern
    // Stranglethorn area, so GetZoneId never returns 33).
    static std::map<uint32, LevelBracket> const cataBrackets = {
        {616, {80, 82}}, {5146, {80, 82}}, {5042, {82, 83}}, {5034, {83, 84}}, {4922, {84, 85}},
        {5389, {85, 85}}, {5339, {25, 35}},
    };
    for (auto const& [zoneId, bracket] : acBrackets)
        zone2LevelBracket[zoneId] = bracket;
    for (auto const& [zoneId, bracket] : cataBrackets)
        zone2LevelBracket[zoneId] = bracket;

    // Then `playerbot_farming_zone` (overrides the table above: zones revamped in Cataclysm keep their MoP
    // levels) carries ZoneId/MinLevel/MaxLevel for the zones this server knows
    // about, including MoP ones. RandomPlayerbotMgr has already loaded it into its farm cache.
    for (RandomPlayerbotMgr::farm_zone const& zone : sRandomPlayerbotMgr->GetFarmCacheData())
    {
        if (!zone.zone_id || zone.max_level == 0)
            continue;

        // AC parity (issue #61): AC's hard-coded bracket table only lists outdoor levelling zones.
        // playerbot_farming_zone also carries capitals (zone_type 1), battlegrounds (2) and dungeons
        // (3); skip those so a capital/BG/dungeon is never picked as a GO_CAMP hub bracket.
        if (zone.zone_type != 0)
            continue;

        zone2LevelBracket[zone.zone_id] = LevelBracket{zone.min_level, zone.max_level};
    }

    if (!g_bracketsPublished.load(std::memory_order_relaxed))
    {
        for (auto const& [zoneId, bracket] : zone2LevelBracket)
            g_publishedBrackets[zoneId] = {bracket.low, bracket.high};
        g_bracketsPublished.store(true, std::memory_order_release);
    }

    if (zone2LevelBracket.empty())
        TC_LOG_WARN("playerbots",
                    "Travel: no zone level brackets - playerbot_farming_zone is empty, so bots will not pick "
                    "flight destinations by zone level.");
}

void TravelMgr::PrepareDestinationCache()
{
    uint32 const maxLevel = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    uint32 flightMasters = 0;
    uint32 innkeepers = 0;
    // #268 S4: count for the PrepareDestinationCache summary line (AC TravelMgr.cpp:4643).
    uint32 bankers = 0;

    // AC tempLocsCache: (map, round(x/50), round(y/50), round(z/50)) -> first spawn's template entry + spawn count.
    std::map<std::tuple<uint16, int32, int32, int32>, std::pair<uint32, uint32>> tempLocsCache;
    // #611 (AC TravelMgr.cpp:4648): per template entry, per zone, the lootable-spawn locations; reduced into
    // creatureSpawnsByTemplate after the loop (AC :4831-4848).
    std::map<uint32, std::map<uint32, std::vector<WorldLocation>>> tempCreatureCache;
    // #703: terrain-only area ids for the CREATURES arm (see TerrainAreaCache); freed when this function returns.
    TerrainAreaCache terrainAreas;

    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        uint32 const spawnGuid = itr.first;
        CreatureData const& creatureData = itr.second;

        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creatureData.id);
        if (!creatureTemplate)
            continue;

        // AC PrepareDestinationCache "CREATURES" branch (issue #61): lootable, non-NPC, normal-rank spawns
        // bucketed into 50-yd cells. Such a spawn can never be a flight master/innkeeper (npcflag == 0), so
        // taking it here and continuing matches AC's if / else-if structure. npcflag2 (MoP-only flags) is
        // required to be 0 as well, which extends AC's "no NPC flags" rule to the 5.4.8 second flag word.
        if (creatureTemplate->npcflag == 0 && creatureTemplate->npcflag2 == 0 && creatureTemplate->lootid != 0 &&
            creatureTemplate->maxlevel - creatureTemplate->minlevel < 3 &&
            creatureTemplate->Entry != 32820 && creatureTemplate->Entry != 24196 &&
            creatureTemplate->Entry != 30627 && creatureTemplate->Entry != 30617 &&
            creatureData.spawntimesecs < 1000 &&
            creatureTemplate->faction != 11 && creatureTemplate->faction != 71 &&
            creatureTemplate->faction != 79 && creatureTemplate->faction != 85 &&
            creatureTemplate->faction != 188 && creatureTemplate->faction != 1575 &&
            (creatureTemplate->unit_flags & 256) == 0 && (creatureTemplate->unit_flags & 4096) == 0 &&
            creatureTemplate->rank == 0)
        {
            std::vector<uint32> const& maps = sPlayerbotAIConfig->randomBotMaps;
            if (std::find(maps.begin(), maps.end(), uint32(creatureData.mapId)) == maps.end())
                continue;

            // AC uses FindMap and skips maps that do not exist; at OnStartup LoA has not created the base
            // maps yet (see the flight-master block below), so CreateBaseMap is the equivalent.
            Map* map = sMapMgr->CreateBaseMap(creatureData.mapId);
            if (!map)
                continue;

            // #703: AC calls map->GetAreaId here (AC :4670), which loads the grid's map+vmap+mmap; see TerrainAreaCache.
            AreaTableEntry const* area = sAreaTableStore.LookupEntry(
                terrainAreas.GetAreaId(creatureData.mapId, creatureData.posX, creatureData.posY));
            if (!area)
                continue;

            // #611 (AC :4663-4666): AC `area->zone ? area->zone : area->ID`; 5.4.8 AreaTableEntry names the parent zone
            // ParentAreaID (DBCStructure.h, same field WorldPosition::getAreaName walks).
            uint32 const areaId = area->ParentAreaID ? area->ParentAreaID : area->ID;

            auto key = std::make_tuple(creatureData.mapId,
                                       int32(std::lround(creatureData.posX / 50.0f)),
                                       int32(std::lround(creatureData.posY / 50.0f)),
                                       int32(std::lround(creatureData.posZ / 50.0f)));
            auto& cell = tempLocsCache[key];
            if (!cell.second)
                cell.first = creatureData.id;   // AC: level comes from creatureDataList[0]
            ++cell.second;
            // #611 (AC :4694)
            tempCreatureCache[creatureData.id][areaId].push_back(
                WorldLocation(creatureData.mapId, creatureData.posX, creatureData.posY, creatureData.posZ));
            continue;
        }

        uint32 const npcFlags = creatureTemplate->npcflag | creatureData.npcflag;
        if (!(npcFlags & (UNIT_NPC_FLAG_FLIGHTMASTER | UNIT_NPC_FLAG_INNKEEPER | UNIT_NPC_FLAG_BANKER)))
            continue;

        uint32 const mapId = creatureData.mapId;

        float const x = creatureData.posX;
        float const y = creatureData.posY;
        float const z = creatureData.posZ;
        float const orient = creatureData.orientation;

        // === BANKERS === (#268 S4; AC TravelMgr.cpp:4776-4808).
        //
        // AC's if / else-if chain in this loop is CREATURES (AC :4677) / FLIGHTMASTER|INNKEEPER
        // (AC :4699) / BANKER (AC :4777), so the banker arm only ever runs for a spawn that is
        // neither of the first two - which is exactly what the gate above plus this `continue`
        // express. It is written as its own block rather than as the chain's third arm so the
        // existing flight-master/innkeeper arm keeps its shape; note the consequence, that the
        // fork's continent filter and the area/faction lookups below stay on the flight-master arm
        // only, which is AC's arrangement too (AC does its map checks before the chain, and has no
        // continent filter on any arm).
        if (creatureTemplate->npcflag & UNIT_NPC_FLAG_BANKER)
        {
            // AC's own arm condition tests the TEMPLATE flag (AC :4777), not the template|spawn
            // union the gate above uses, so a spawn that adds a banker flag to a non-banker template
            // is ignored exactly as upstream ignores it (it falls through to nothing).
            //
            // AC applies randomBotMaps to every arm of the chain (AC :4656-4658), so a banker on a
            // map the bot pool never visits is skipped upstream too.
            std::vector<uint32> const& maps = sPlayerbotAIConfig->randomBotMaps;
            if (std::find(maps.begin(), maps.end(), mapId) == maps.end())
                continue;

            // AC :4778-4784, verbatim: npcflag 135298 is the "banker plus a second flag word" case
            // upstream drops, minlevel 55/65 and factions 35/474/69/57 are the Cata-era / hostile
            // bankers, and 30606/30608/29282 are the three quest-gated bankers upstream excludes.
            if (creatureTemplate->npcflag != 135298 &&
                creatureTemplate->minlevel != 55 && creatureTemplate->minlevel != 65 &&
                creatureTemplate->faction != 35 && creatureTemplate->faction != 474 &&
                creatureTemplate->faction != 69 && creatureTemplate->faction != 57 &&
                creatureTemplate->Entry != 30606 && creatureTemplate->Entry != 30608 &&
                creatureTemplate->Entry != 29282)
            {
                // Stand in front of the banker, facing it - the same offset AC uses, but 6 yd / 2 z
                // instead of the 5 yd / 0.5 z a flight master or innkeeper gets (AC :4787).
                BankerLocation bLoc;
                bLoc.loc = WorldLocation(mapId, x + std::cos(orient) * 6.0f, y + std::sin(orient) * 6.0f,
                                         z + 2.0f, orient + float(M_PI));
                bLoc.entry = creatureData.id;
                uint32 const level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;
                for (uint32 l = 1; l <= maxLevel; ++l)
                {
                    // Bots 1-60 go to base game bankers (all have minlevel 30 or 45)
                    if (l <= 60 && level > 45)
                        continue;

                    // Bots 61-70 go to Shattrath bankers (all have minlevel 60 or 70)
                    if (l >= 61 && l <= 70 && (level < 60 || level > 70))
                        continue;

                    // Bots 71-80 go to Dalaran bankers (all have minlevel 75). AC's arm reads "71+" because
                    // 80 is its cap; #537 (MoP-only, extends AC's bracket pattern): a level 85-90 bot sent
                    // to Dalaran had no grind cell, hub, flight or quest in reach, so it could only draw
                    // WANDER_NPC/REST and spent ~98% of the w31 field window in WANDER_NPC there.
                    if (l >= 71 && l <= 80 && level != 75)
                        continue;

                    // #947 (MoP-only, extends AC's bracket pattern the way Dalaran serves 71-80): Stormwind and
                    // Orgrimmar were the Cataclysm capitals and stay the MoP auction-house/portal hubs (the shrines
                    // have no auction house), but no bot above 60 ever banked there, so 81-90 bots were never seen
                    // in either city. Their bankers now serve 81-84 alone (was: no bracket, #537) and 85+ next to
                    // the vaultkeepers. GetCityLocations keeps each team in its own capital.
                    bool const cataCapitalBanker = bLoc.entry == 2455 || bLoc.entry == 2456 || bLoc.entry == 2457 ||
                                                   (bLoc.entry >= 44851 && bLoc.entry <= 44856);
                    if (l >= 81 && l <= 84 && !cataCapitalBanker)
                        continue;

                    // #537 (MoP-only): bots 85+ go to the Pandaria vaultkeepers (all level 90: Shrine of
                    // Seven Stars / Shrine of Two Moons). GetCityLocations drops the opposite faction's shrine.
                    if (l >= 85 && level < 85 && !cataCapitalBanker)
                        continue;

                    bankerLocsPerLevelCache[uint8(l)].push_back(bLoc);
                    bankerEntryToLocation[bLoc.entry] = bLoc.loc;
                }
                ++bankers;
            }

            continue;
        }

        // AC TravelMgr.cpp:4697-4701: the flight-master/innkeeper arm skips 29480 Grimwing (Storm Peaks) and 3838 Vesprystus
        // (Rut'Theran). #779 (MoP-only content, extends that hand list): four Pandaria flight masters are spawned only in phase 4
        // (creature.phaseMask 4), so a phase-1 bot walks to the spawn, finds no creature and goes idle (NewRpgAction.cpp:915-924;
        // w39: 13 of 16 "no live creature" lines, 0 boardings there): 61744 Tabo the Flyer, 61745 Soaring Paw, 67785 Kromthar,
        // 68226 Daggin Windbeard. #947: the same holds for three phase-4 innkeepers (creature.phaseMask 4): 69088 Keeper Jaril
        // (Krasarang, Lion's Landing / Domination Point), 62882 Kai the Restless and 62883 Mai the Sleepy (Kun-Lai).
        if (creatureTemplate->Entry == 3838 || creatureTemplate->Entry == 29480 || creatureTemplate->Entry == 61744 ||
            creatureTemplate->Entry == 61745 || creatureTemplate->Entry == 67785 || creatureTemplate->Entry == 68226 ||
            creatureTemplate->Entry == 69088 || creatureTemplate->Entry == 62882 || creatureTemplate->Entry == 62883)
            continue;

        // Continents only: instance flight masters/innkeepers are not travel destinations.
        MapEntry const* mapEntry = sMapStore.LookupEntry(mapId);
        if (!mapEntry || mapEntry->IsDungeon() || mapEntry->IsBattlegroundOrArena())
            continue;

        // The zone id comes from the terrain, so the continent map has to exist. Upstream uses
        // FindMap and silently skips maps that are not instantiated yet, which at startup is all
        // of them; CreateBaseMap instantiates the continent instead (it is created anyway as soon
        // as the first player or bot enters it).
        Map* map = sMapMgr->CreateBaseMap(mapId);
        if (!map)
            continue;

        // #703: Map::GetAreaId asks the vmap (GetAreaInfo) BEFORE GetGrid creates the grid and loads its vmap tile,
        // so the first lookup on a fresh grid misses WMO areas (Undercity, Dalaran inns...). Before #703 the grind
        // arm had already created almost every grid; now it no longer does, so warm the grid with one throw-away
        // lookup and take the second, which is what this arm effectively saw before.
        map->GetAreaId(PHASEMASK_NORMAL, x, y, z);
        AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
        if (!area)
            continue;

        uint32 const zoneId = area->ParentAreaID ? area->ParentAreaID : area->ID;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
        if (!factionEntry)
            continue;

        // Same test upstream uses: an NPC hostile to a faction group is not usable by it.
        bool const forHorde = !(factionEntry->hostileMask & FACTION_MASK_HORDE);
        bool const forAlliance = !(factionEntry->hostileMask & FACTION_MASK_ALLIANCE);

        if (npcFlags & UNIT_NPC_FLAG_FLIGHTMASTER)
        {
            WorldPosition pos(mapId, x, y, z, orient);

            // #779 (MoP core): Player::ActivateTaxiPathTo (Player.cpp:23446-23460) refuses a flight-master start with
            // ERR_TAXI_TOO_FAR_AWAY when the player is more than 2 * INTERACTION_DISTANCE from the start node; AC's
            // ActivateTaxiPathTo (AC Player.cpp:10469-10477) has no such check and just flies from the far node. A bot boards
            // from INTERACTION_DISTANCE (+ reach) of the master (NewRpgTravelFlightAction::Execute), so a master whose nearest
            // node for a team is farther than 4 * INTERACTION_DISTANCE can never be boarded by that team on this core; the bot
            // walks there and the activation fails (w41: Alliance bots at 30870 Herzo Safeflight, Warsong Camp, got node 308
            // River's Heart 1293 yd away, 12 failed activations). Such a team does not get the master, as AC leaves out 3838
            // and 29480 above. No node (0) keeps AC's behaviour.
            auto boardable = [&](uint32 nodeId)
            {
                TaxiNodesEntry const* node = nodeId ? sTaxiNodesStore.LookupEntry(nodeId) : nullptr;
                if (!node)
                    return true;
                float const dx = node->x - x;
                float const dy = node->y - y;
                float const dz = node->z - z;
                float const maxDist = 4 * INTERACTION_DISTANCE;
                return dx * dx + dy * dy + dz * dz <= maxDist * maxDist;
            };

            if (forHorde)
            {
                FlightMasterInfo info;
                info.pos = pos;
                info.zoneId = zoneId;
                info.taxiNodeId = sObjectMgr->GetNearestTaxiNode(x, y, z, mapId, HORDE);
                info.templateEntry = creatureData.id;
                info.dbGuid = spawnGuid;
                if (boardable(info.taxiNodeId))
                    hordeFlightMasterCache[spawnGuid] = info;
            }

            if (forAlliance)
            {
                FlightMasterInfo info;
                info.pos = pos;
                info.zoneId = zoneId;
                info.taxiNodeId = sObjectMgr->GetNearestTaxiNode(x, y, z, mapId, ALLIANCE);
                info.templateEntry = creatureData.id;
                info.dbGuid = spawnGuid;
                if (boardable(info.taxiNodeId))
                    allianceFlightMasterCache[spawnGuid] = info;
            }

            ++flightMasters;
        }

        // AC parity (issue #61): AC TravelMgr.cpp:4707-4774. A flight master is a hub only in AC's
        // hand-listed innkeeper-less zones; an innkeeper is always a hub for its zone's bracket. A
        // spawn that is both a flight master and an innkeeper takes only the flight-master branch,
        // matching AC's if/else-if (so it is a hub only when its zone is innkeeper-less).
        // #947 (MoP-only, extends AC's hand list): 6757 Timeless Isle (one innkeeper, so its two flight masters make it
        // three L90 hubs instead of one) and 5146 Vashj'ir (bracket 80-82, no innkeeper on MoP, so no hub at all).
        static std::set<uint32> const zonesWithoutInnkeeper = {4, 16, 28, 46, 51, 361, 490, 2817, 4197, 6757, 5146};

        // Stand in front of the NPC, facing it.
        WorldLocation loc(mapId, x + std::cos(orient) * 5.0f, y + std::sin(orient) * 5.0f, z + 0.5f,
                          orient + float(M_PI));

        if (npcFlags & UNIT_NPC_FLAG_FLIGHTMASTER)
        {
            if (zonesWithoutInnkeeper.count(zoneId))
            {
                auto bracketItr = zone2LevelBracket.find(zoneId);
                if (bracketItr != zone2LevelBracket.end())
                {
                    LevelBracket const& bracket = bracketItr->second;
                    for (uint32 level = bracket.low; level <= bracket.high && level <= maxLevel; ++level)
                    {
                        if (forHorde)
                            hordeHubsPerLevelCache[uint8(level)].push_back(loc);
                        if (forAlliance)
                            allianceHubsPerLevelCache[uint8(level)].push_back(loc);
                    }
                }
            }
        }
        else if (npcFlags & UNIT_NPC_FLAG_INNKEEPER)
        {
            // #947 (MoP-only, extends AC's innkeeper hubs the way MoP's Dalaran inns already are 77-80 hubs): the shrine inns
            // are 85-90 hubs. The shrines are two-level sub-zones of the Vale (5840, bracket 90-90): an inn that resolves to
            // area 6519 Summer's Rest got zone 6141, which has no bracket (capital row, ZoneTyp 1), so the Horde shrine had
            // no hub at all, while the Alliance inn got the Vale's 90-only bracket (2026-10-28 log: 3 hub teleports into
            // the Alliance shrine, 0 into the Horde one). Walk the area chain so both inns get the same bracket whichever
            // shrine area the lookup returns: 6141 / 6554 Shrine of Two Moons, 6553 / 6142 Shrine of Seven Stars.
            static LevelBracket const shrineInnBracket{85, 90};
            bool shrineInn = false;
            AreaTableEntry const* a = area;
            for (uint32 depth = 0; a && !shrineInn && depth < 4; ++depth)
            {
                shrineInn = a->ID == 6141 || a->ID == 6554 || a->ID == 6553 || a->ID == 6142;
                a = a->ParentAreaID ? sAreaTableStore.LookupEntry(a->ParentAreaID) : nullptr;
            }

            auto bracketItr = zone2LevelBracket.find(zoneId);
            if (shrineInn || bracketItr != zone2LevelBracket.end())
            {
                LevelBracket const& bracket = shrineInn ? shrineInnBracket : bracketItr->second;
                for (uint32 level = bracket.low; level <= bracket.high && level <= maxLevel; ++level)
                {
                    if (forHorde)
                        hordeHubsPerLevelCache[uint8(level)].push_back(loc);
                    if (forAlliance)
                        allianceHubsPerLevelCache[uint8(level)].push_back(loc);
                }
                ++innkeepers;
            }
        }
    }

    // AC parity (issue #61): AC TravelMgr.cpp:4812-4829 post-processing. A cell needs >= 2 spawns to
    // qualify; its level is (minlevel + maxlevel + 1) / 2 of the cell's first spawn, and the cell point
    // (snapped to the 50-yd lattice, z included) is offered to every bot level in
    // [level - randomBotTeleLowerLevel, level + randomBotTeleHigherLevel], clamped to [1, maxLevel].
    uint32 cells = 0;
    std::map<uint16, uint32> cellsPerMap;
    std::map<uint16, uint32> level90CellsPerMap;
    for (auto const& [key, cell] : tempLocsCache)
    {
        if (cell.second < 2)
            continue;

        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(cell.first);
        if (!creatureTemplate)
            continue;

        ++cells;
        uint16 const cellMapId = std::get<0>(key);
        ++cellsPerMap[cellMapId];

        int32 const level = (creatureTemplate->minlevel + creatureTemplate->maxlevel + 1) / 2;
        bool countedAtMaxLevel = false;
        for (int32 l = level - int32(sPlayerbotAIConfig->randomBotTeleLowerLevel);
             l <= level + int32(sPlayerbotAIConfig->randomBotTeleHigherLevel); ++l)
        {
            if (l < 1 || l > int32(maxLevel))
                continue;

            if (!countedAtMaxLevel && uint32(l) == maxLevel)
            {
                ++level90CellsPerMap[cellMapId];
                countedAtMaxLevel = true;
            }

            locsPerLevelCache[uint8(l)].push_back(WorldLocation(cellMapId,
                float(std::get<1>(key)) * 50.0f, float(std::get<2>(key)) * 50.0f, float(std::get<3>(key)) * 50.0f));
        }
    }

    // #611 (AC TravelMgr.cpp:4831-4848): templates that spawn in at most three places per zone keep the mean spawn point
    // per zone. Nothing reads creatureSpawnsByTemplate in AC or here.
    for (auto const& [entry, areaMap] : tempCreatureCache)
    {
        for (auto const& [areaIdKey, locList] : areaMap)
        {
            if (locList.size() > 3)
                continue;

            float totalX = 0, totalY = 0, totalZ = 0;
            for (auto const& loc : locList)
            {
                totalX += loc.GetPositionX();
                totalY += loc.GetPositionY();
                totalZ += loc.GetPositionZ();
            }
            float avgX = totalX / locList.size();
            float avgY = totalY / locList.size();
            float avgZ = totalZ / locList.size();
            creatureSpawnsByTemplate[entry].push_back(WorldLocation(locList[0].GetMapId(), avgX, avgY, avgZ, 0));
        }
    }

    // AC parity (issue #61): AC TravelMgr.cpp:4851-4870. Levels 1-5 also get a hub at every
    // race/class starting position, so a freshly created bot has a GO_CAMP destination even before
    // any innkeeper/flight-master hub is in range. No effect at the level-90 bot pool this fork runs.
    for (uint32 race = 1; race < MAX_RACES; ++race)
    {
        for (uint32 cls = 1; cls < MAX_CLASSES; ++cls)
        {
            PlayerInfo const* info = sObjectMgr->GetPlayerInfo(race, cls);
            if (!info)
                continue;

            WorldLocation pos(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

            for (int32 level = 1; level <= 5; ++level)
            {
                if ((1 << (race - 1)) & RACEMASK_ALLIANCE)
                    allianceHubsPerLevelCache[uint8(level)].push_back(pos);
                else
                    hordeHubsPerLevelCache[uint8(level)].push_back(pos);
            }
            break;
        }
    }

    TC_LOG_INFO("playerbots", ">> Travel destination cache: %u flight masters, %u innkeepers, %u bankers, "
                              "%u grind cells across %u maps (%u level %u)",
                flightMasters, innkeepers, bankers, cells, uint32(cellsPerMap.size()),
                uint32(GetLocsPerLevelCache(uint8(maxLevel)).size()), maxLevel);
    for (auto const& [cellMapId, mapCells] : cellsPerMap)
    {
        TC_LOG_INFO("playerbots", ">> Travel destination cache: map %u: %u grind cells (%u valid at level %u)",
                    uint32(cellMapId), mapCells, level90CellsPerMap[cellMapId], maxLevel);
    }
}

TravelMgr::FlightMasterInfo const* TravelMgr::GetNearestFlightMasterInfo(Player* bot) const
{
    if (!bot)
        return nullptr;

    auto const& cache = (bot->GetTeam() == ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    FlightMasterInfo const* nearest = nullptr;
    float nearestDistance = std::numeric_limits<float>::max();

    for (auto const& itr : cache)
    {
        FlightMasterInfo const& info = itr.second;

        if (info.pos.GetMapId() != bot->GetMapId())
            continue;

        float const distance = bot->GetExactDist2dSq(info.pos.GetPositionX(), info.pos.GetPositionY());
        if (distance < nearestDistance)
        {
            nearestDistance = distance;
            nearest = &info;
        }
    }

    return nearest;
}

std::vector<uint32> TravelMgr::GetFlightNodesInZone(uint32 zoneId, uint32 team, uint32 excludeNode) const
{
    auto const& cache = (team == ALLIANCE) ? allianceFlightMasterCache : hordeFlightMasterCache;

    std::vector<uint32> result;
    std::set<uint32> seen;

    for (auto const& itr : cache)
    {
        FlightMasterInfo const& info = itr.second;

        if (info.zoneId != zoneId || info.taxiNodeId == 0 || info.taxiNodeId == excludeNode)
            continue;

        if (seen.insert(info.taxiNodeId).second)
            result.push_back(info.taxiNodeId);
    }

    return result;
}

namespace
{
// #887: a taxi node the bot's team has no flight mount for (Shrine of Two Moons / Serpent's Spine for the Alliance,
// Shrine of Seven Stars for the Horde, ...). Same test the stopover replay uses (PlayerbotAI.cpp ContinueTaxiIfLanded:
// ObjectMgr::GetTaxiMountDisplayId(source, team) == 0 clears the route), MountCreatureID[1] = Alliance, [0] = Horde.
bool TaxiNodeUsableBy(uint32 nodeId, uint32 team)
{
    TaxiNodesEntry const* node = sTaxiNodesStore.LookupEntry(nodeId);
    return node && node->MountCreatureID[team == ALLIANCE ? 1 : 0] != 0;
}

// #887 (MoP-only; AC's TravelNodeMap::FindTaxiPath is faction-blind too, AC TravelNode.cpp:2483-2500, but AC's core
// flies a multi-leg route as ONE spline (AC WaypointMovementGenerator.cpp:579-657, :720-759), so a hostile node in the
// middle is flown over and never landed on). This core lands at every connection point (taxi-stopover-225), and a
// stopover the bot cannot depart from strands it there: the build-87 t847 run had an Alliance paladin land at the Horde
// Shrine of Two Moons (node 1058, route 1017 -> 1058 -> 895 -> 1293) and fight Horde bots. The shared BFS route is kept
// whenever every node on it is usable by the team (so every route AC would take is unchanged); otherwise the shortest
// route over team-usable nodes only is searched, and an empty path (the caller then tries the next zone) if none exists.
std::vector<uint32> TeamTaxiPath(uint32 team, uint32 fromNode, uint32 toNode)
{
    std::vector<uint32> path = sTravelNodeMap.FindTaxiPath(fromNode, toNode);
    if (path.empty())
        return path;

    bool usable = true;
    for (uint32 node : path)
        if (!TaxiNodeUsableBy(node, team))
        {
            usable = false;
            break;
        }
    if (usable)
        return path;

    if (!TaxiNodeUsableBy(fromNode, team) || !TaxiNodeUsableBy(toNode, team))
    {
        TC_LOG_DEBUG("playerbots", "[TAXI] team %u route %u -> %u dropped: endpoint has no mount for the team", team, fromNode, toNode);
        return {};
    }

    // Neighbours from TaxiPath.dbc, built once (the store is read-only after startup; function-local statics are
    // initialised thread-safely, and this runs on map threads).
    static std::unordered_map<uint32, std::vector<uint32>> const adjacency = []()
    {
        std::unordered_map<uint32, std::vector<uint32>> result;
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);
            if (!taxiPath || taxiPath->to == 0 || taxiPath->to == uint32(-1))
                continue;
            result[taxiPath->from].push_back(taxiPath->to);
        }
        return result;
    }();

    std::unordered_map<uint32, uint32> parent;
    std::deque<uint32> queue;
    parent[fromNode] = 0;
    queue.push_back(fromNode);
    while (!queue.empty() && !parent.count(toNode))
    {
        uint32 const current = queue.front();
        queue.pop_front();
        auto itr = adjacency.find(current);
        if (itr == adjacency.end())
            continue;
        for (uint32 next : itr->second)
        {
            if (parent.count(next) || !TaxiNodeUsableBy(next, team))
                continue;
            parent[next] = current;
            queue.push_back(next);
        }
    }
    if (!parent.count(toNode))
    {
        TC_LOG_DEBUG("playerbots", "[TAXI] team %u route %u -> %u dropped: no route over nodes the team can use", team, fromNode, toNode);
        return {};
    }

    std::vector<uint32> result;
    for (uint32 node = toNode; node != 0; node = parent[node])
        result.push_back(node);
    std::reverse(result.begin(), result.end());
    TC_LOG_DEBUG("playerbots", "[TAXI] team %u route %u -> %u rerouted around nodes the team has no mount for (%u hops, was %u)",
                 team, fromNode, toNode, uint32(result.size()), uint32(path.size()));
    return result;
}
}   // namespace

std::vector<std::vector<uint32>> TravelMgr::GetOptimalFlightDestinations(Player* bot)
{
    std::vector<std::vector<uint32>> validDestinations;

    FlightMasterInfo const* nearest = GetNearestFlightMasterInfo(bot);
    if (!nearest || bot->GetDistance(nearest->pos.GetPositionX(), nearest->pos.GetPositionY(),
                                     nearest->pos.GetPositionZ()) > 500.0f)
        return validDestinations;

    uint32 const fromNode = nearest->taxiNodeId;
    if (!fromNode || !sTaxiNodesStore.LookupEntry(fromNode))
        return validDestinations;

    uint32 const botLevel = bot->GetLevel();

    // G-T2 / AC parity (AC TravelMgr.cpp:4421-4440): bots already in a capital should not have
    // another capital picked as a flight destination - that just shuffles them between cities.
    bool botInCapital = false;
    if (AreaTableEntry const* area = sAreaTableStore.LookupEntry(bot->GetZoneId()))
        botInCapital = (area->Flags & AREA_FLAG_CAPITAL) != 0;

    uint32 const botTeam = bot->GetTeam();

    std::vector<uint32> candidateZones;
    // urand(0, 100), not (0, 99): AC TravelMgr.cpp:4431 rolls the same inclusive 0-100 range.
    if (botLevel >= 10 && !botInCapital && urand(0, 100) < uint32(ProbTeleToBankers() * 100))
    {
        for (Capital const& capital : g_capitals)
        {
            if (capital.team != 0 && capital.team != botTeam)
                continue;

            candidateZones.push_back(capital.zoneId);
        }
    }

    if (candidateZones.empty())
    {
        for (auto const& itr : zone2LevelBracket)
        {
            if (!itr.second.InsideBracket(botLevel))
                continue;

            if (GetFlightNodesInZone(itr.first, botTeam, fromNode).empty())
                continue;

            candidateZones.push_back(itr.first);
        }
    }

    while (!candidateZones.empty())
    {
        uint32 const zoneIndex = urand(0, candidateZones.size() - 1);
        uint32 const pickedZone = candidateZones[zoneIndex];

        std::vector<uint32> usableNodes = GetFlightNodesInZone(pickedZone, botTeam, fromNode);

        if (!usableNodes.empty())
        {
            uint32 const pickedNode = usableNodes[urand(0, usableNodes.size() - 1)];
            std::vector<uint32> path = TeamTaxiPath(botTeam, fromNode, pickedNode);   // #887
            if (!path.empty())
            {
                validDestinations.push_back(std::move(path));
                return validDestinations;
            }
        }

        candidateZones.erase(candidateZones.begin() + zoneIndex);
    }

    return validDestinations;
}

std::vector<WorldLocation> const& TravelMgr::GetTravelHubs(Player* bot)
{
    if (!bot)
        return _emptyHubs;

    auto& cache = (bot->GetTeam() == ALLIANCE) ? allianceHubsPerLevelCache : hordeHubsPerLevelCache;

    auto itr = cache.find(uint8(bot->GetLevel()));
    if (itr == cache.end())
        return _emptyHubs;

    return itr->second;
}

std::vector<WorldLocation> const& TravelMgr::GetLocsPerLevelCache(uint8 level) const
{
    // find(), not AC's operator[]: this is read concurrently from map threads and must never insert.
    auto itr = locsPerLevelCache.find(level);
    return itr != locsPerLevelCache.end() ? itr->second : _emptyHubs;
}

// #268 S4. AC TravelMgr.h:879 / TravelMgr.cpp:4480-4488. AC returns by value; the fork returns a
// reference like its two sibling accessors above, which avoids a copy on every teleport roll and is
// safe for the same reason GetLocsPerLevelCache is: nothing mutates these caches after Init().
std::vector<WorldLocation> const& TravelMgr::GetTeleportLocations(Player* bot)
{
    if (!bot)
        return _emptyHubs;

    // With the "new rpg" strategy on, the teleport pool is the faction's innkeeper / flight-master
    // hubs; otherwise it is the 50-yd grind cells.
    if (sPlayerbotAIConfig->enableNewRpgStrategy)
        return GetTravelHubs(bot);

    return GetLocsPerLevelCache(uint8(bot->GetLevel()));
}

// #268 S4. AC TravelMgr.h:881 / TravelMgr.cpp:4498-4556.
std::vector<WorldLocation> TravelMgr::GetCityLocations(Player* bot)
{
    static std::vector<BankerLocation> const noBankers;

    uint32 const level = bot->GetLevel();

    std::vector<BankerLocation> const* levelBankersPtr = nullptr;
    if (auto itr = bankerLocsPerLevelCache.find(uint8(level)); itr != bankerLocsPerLevelCache.end())
        levelBankersPtr = &itr->second;
    std::vector<BankerLocation> const& levelBankers = levelBankersPtr ? *levelBankersPtr : noBankers;

    // #537 (MoP-only): the 85+ arm holds both faction shrines (AC's Dalaran tellers are neutral), so drop a
    // banker whose faction is hostile to the bot's team, the same hostileMask test the hub cache uses.
    uint32 const teamMask = bot->GetTeam() == HORDE ? FACTION_MASK_HORDE : FACTION_MASK_ALLIANCE;
    // #947: from 81 the arm also holds Stormwind and Orgrimmar, so the filter starts there.
    auto usable = [&](BankerLocation const& bLoc)
    {
        if (level < 81)
            return true;
        CreatureTemplate const* ct = sObjectMgr->GetCreatureTemplate(bLoc.entry);
        FactionTemplateEntry const* ft = ct ? sFactionTemplateStore.LookupEntry(ct->faction) : nullptr;
        return ft && !(ft->hostileMask & teamMask);
    };

    std::vector<WorldLocation> fallbackLocations;
    fallbackLocations.reserve(levelBankers.size());
    for (auto const& bLoc : levelBankers)
        if (usable(bLoc))
            fallbackLocations.push_back(bLoc.loc);

    if (!CityBankerWeightConfig().enabled)
        return fallbackLocations;

    uint32 const botTeamId = bot->GetTeam();
    std::unordered_set<uint32> validBankerCities;
    for (auto const& loc : levelBankers)
    {
        Capital const* capital = FindCapitalByBanker(loc.entry);
        if (!capital)
            continue;

        uint32 const cityTeamId = capital->team;

        if (cityTeamId == botTeamId || cityTeamId == 0)
            validBankerCities.insert(capital->zoneId);
    }

    // Fallback if no valid cities
    if (validBankerCities.empty())
        return fallbackLocations;

    // Apply weights to valid cities
    std::vector<uint32> weightedCities;
    for (uint32 zoneId : validBankerCities)
    {
        int const weight = GetCityWeight(zoneId);
        if (weight <= 0)
            continue;

        for (int i = 0; i < weight; ++i)
            weightedCities.push_back(zoneId);
    }

    // Fallback if every city ended up with weight 0
    if (weightedCities.empty())
        return fallbackLocations;

    // Pick a weighted city randomly, then a random banker in that city
    uint32 const selectedCity = weightedCities[urand(0, weightedCities.size() - 1)];
    Capital const* selectedCapital = FindCapitalByZone(selectedCity);
    if (!selectedCapital)
        return fallbackLocations;

    std::vector<uint16> const& bankers = selectedCapital->bankers;
    uint32 const selectedBankerEntry = bankers[urand(0, bankers.size() - 1)];

    auto locIt = bankerEntryToLocation.find(selectedBankerEntry);
    if (locIt != bankerEntryToLocation.end())
        return { locIt->second };

    // Fallback if something went wrong
    return fallbackLocations;
}

void TravelMgr::addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance, bool makeShortcuts)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return;

    // Derive chained transfers so A->C is known when only A->B and B->C were added.
    if (makeShortcuts)
    {
        for (auto& mapTransfers : mapTransfersMap)
        {
            uint32 sMapt = mapTransfers.first.first;
            uint32 eMapt = mapTransfers.first.second;

            for (auto& transfer : mapTransfers.second)
            {
                if (eMapt == sMap && sMapt != eMap)  // [S1 >MT> E1 -> S2] >THIS> E2
                {
                    float newDistToEnd = mapTransDistance(*transfer.getPointFrom(), start) + portalDistance;
                    if (mapTransDistance(*transfer.getPointFrom(), end) > newDistToEnd)
                        addMapTransfer(*transfer.getPointFrom(), end, newDistToEnd, false);
                }

                if (sMapt == eMap && eMapt != sMap)  // S1 >THIS> [E1 -> S2 >MT> E2]
                {
                    float newDistToEnd = portalDistance + mapTransDistance(end, *transfer.getPointTo());
                    if (mapTransDistance(start, *transfer.getPointTo()) > newDistToEnd)
                        addMapTransfer(start, *transfer.getPointTo(), newDistToEnd, false);
                }
            }
        }
    }

    auto itr = mapTransfersMap.find(std::make_pair(sMap, eMap));
    if (itr == mapTransfersMap.end())
        mapTransfersMap.insert({{sMap, eMap}, {mapTransfer(start, end, portalDistance)}});
    else
        itr->second.push_back(mapTransfer(start, end, portalDistance));
}

void TravelMgr::loadMapTransfers()
{
    mapTransfersMap.clear();

    for (auto& node : sTravelNodeMap.getNodes())
        for (auto& link : *node->getLinks())
            addMapTransfer(*node->getPosition(), *link.first->getPosition(), link.second->getDistance());

    // issue #86: the travel graph can only route across maps if these transfers exist; log the
    // count and whether the two most important pairs are present.
    auto const has = [this](uint32 a, uint32 b)
    {
        auto itr = mapTransfersMap.find(std::make_pair(a, b));
        return itr != mapTransfersMap.end() ? uint32(itr->second.size()) : 0u;
    };
    TC_LOG_INFO("playerbots", ">> Map transfers: %u pairs (0->1: %u, 1->0: %u, 1->530: %u, 0->530: %u)",
                uint32(mapTransfersMap.size()), has(0, 1), has(1, 0), has(1, 530), has(0, 530));
}

float TravelMgr::mapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.distance(end);

    float minDist = 200000.f;

    auto itr = mapTransfersMap.find({sMap, eMap});
    if (itr == mapTransfersMap.end())
        return minDist;

    for (auto& transfer : itr->second)
        minDist = std::min(minDist, transfer.distance(start, end));

    return minDist;
}

float TravelMgr::fastMapTransDistance(WorldPosition start, WorldPosition end)
{
    uint32 sMap = start.getMapId();
    uint32 eMap = end.getMapId();

    if (sMap == eMap)
        return start.fDist(end);

    float minDist = 200000.f;

    auto itr = mapTransfersMap.find({sMap, eMap});
    if (itr == mapTransfersMap.end())
        return minDist;

    for (auto& transfer : itr->second)
        minDist = std::min(minDist, transfer.fDist(start, end));

    return minDist;
}


// ================================================================================================
// #268 slice S1 - the TravelDestination hierarchy (AC Mgr/Travel/TravelMgr.cpp:1075-1479), plus the
// five definitions AC places OUTSIDE that anchor but S1's header block declares: QuestTravelDestination's
// constructor and isActive (AC :4262-4269) and QuestObjectiveTravelDestination's objective accessors
// (AC :4271-4283). Without them QuestTravelDestination - and therefore every subclass - fails to link,
// and declaring members in a header with no definition anywhere is worse than a slightly wider slice.
//
// Core adaptations, all forced by 5.4.8 and all called out inline below:
//   * ChatHelper::FormatWorldEntry (AC Bot/Cmd/ChatHelper.cpp:398-421) does not exist in this fork.
//     AC calls it from exactly five places and all five are the five getTitle() methods ported here,
//     so it is reproduced below as a file-local helper. ChatHelper.h is pulled into most of the module
//     through PlayerbotAI.h, so adding a member there would cost a full-module rebuild for what is
//     only a diagnostic string. The output is byte-identical to AC's. If a later slice needs the
//     ChatHelper member, lift FormatWorldEntry() below verbatim into ChatHelper.{h,cpp} and delete the
//     file-local copy.
//   * Quest::GetType() does not exist on 5.4.8 (it is GetQuestType()) and the 5.4.8 QuestType enum has
//     no QUEST_TYPE_ELITE / QUEST_TYPE_DUNGEON, so AC's "dungeon or elite quest" test becomes
//     IsInstanceQuest() below, keyed on Quest::GetQuestInfoID() (QuestInfo.dbc).
//   * Quest has no RequiredNpcOrGo[]/RequiredNpcOrGoCount[] on 5.4.8 - objectives are
//     Quest::GetObjectives(), a vector<QuestObjective>{Type, ObjectID, Amount}. See ObjectiveAt().
//   * PLAYER_EXPLORED_ZONES_1 -> PLAYER_FIELD_EXPLORED_ZONES.
//   * Unchanged: CreatureTemplate::npcflag/maxlevel/mingold/rank/faction/Name, AreaTableEntry::
//     exploreFlag/area_level, Unit::GetFactionReactionTo, Player::CanTakeQuest/IsActiveQuest.
// ================================================================================================
namespace
{
// AC Bot/Cmd/ChatHelper.cpp:398-421, moved here - see the note above.
std::string const FormatWorldEntry(int32 entry)
{
    CreatureTemplate const* cInfo = nullptr;
    GameObjectTemplate const* gInfo = nullptr;

    if (entry > 0)
        cInfo = sObjectMgr->GetCreatureTemplate(entry);
    else
        gInfo = sObjectMgr->GetGameObjectTemplate(entry * -1);

    std::ostringstream out;
    out << "|cFFFFFF00|Hentry:" << std::abs(entry) << ":"
        << "|h[";

    if (entry < 0 && gInfo)
        out << gInfo->name;
    else if (entry > 0 && cInfo)
        out << cInfo->Name;
    else
        out << "unknown";

    out << "]|h|r";
    return out.str();
}

// AC: (questTemplate->GetType() == QUEST_TYPE_ELITE || questTemplate->GetType() == QUEST_TYPE_DUNGEON).
// 5.4.8: SharedDefines.h's QuestType is only TURNIN / WITH_MAX_LEVEL / NORMAL / TASK, so the dungeon
// half is keyed on QuestInfo.dbc instead (core Quests/QuestDef.h GetQuestInfoID()). MoP has no
// equivalent of the ELITE quest flag at all - dropped rather than guessed at.
bool IsInstanceQuest(Quest const* quest)
{
    switch (quest->GetQuestInfoID())
    {
        case QUEST_INFO_DUNGEON:
        case QUEST_INFO_HEROIC:
        case QUEST_INFO_RAID:
        case QUEST_INFO_RAID_10:
        case QUEST_INFO_RAID_25:
        case QUEST_INFO_SCENARIO:
            return true;
        default:
            return false;
    }
}

// AC indexes Quest::RequiredNpcOrGo[] / RequiredNpcOrGoCount[] raw. 5.4.8's Quest::GetObjectives()
// is a vector, so the index is bounds-checked once here rather than unguarded four times.
QuestObjective const* ObjectiveAt(Quest const* quest, uint32 objective)
{
    QuestObjectives const& objectives = quest->GetObjectives();
    return objective < objectives.size() ? &objectives[objective] : nullptr;
}
}  // namespace

std::vector<WorldPosition*> TravelDestination::getPoints(bool ignoreFull)
{
    if (ignoreFull)
        return points;

    uint32 max = maxVisitorsPerPoint;
    if (!max)
        return points;

    std::vector<WorldPosition*> retVec;
    std::copy_if(points.begin(), points.end(), std::back_inserter(retVec),
                 [max](WorldPosition* p) { return p->getVisitors() < max; });
    return retVec;
}

WorldPosition* TravelDestination::nearestPoint(WorldPosition* pos)
{
    return *std::min_element(points.begin(), points.end(),
                             [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
}

std::vector<WorldPosition*> TravelDestination::touchingPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points;
    for (auto& point : points)
    {
        float dist = pos->distance(point);
        if (!dist)
            continue;

        if (dist > radiusMax * 2)
            continue;

        ret_points.push_back(point);
    }

    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::sortedPoints(WorldPosition* pos)
{
    std::vector<WorldPosition*> ret_points = points;
    std::sort(ret_points.begin(), ret_points.end(),
              [pos](WorldPosition* i, WorldPosition* j) { return i->distance(pos) < j->distance(pos); });
    return ret_points;
};

std::vector<WorldPosition*> TravelDestination::nextPoint(WorldPosition* pos, bool ignoreFull)
{
    return TravelMgr::instance().getNextPoint(pos, ignoreFull ? points : getPoints());
}

bool TravelDestination::isFull(bool ignoreFull)
{
    if (!ignoreFull && maxVisitors > 0 && visitors >= maxVisitors)
        return true;

    if (maxVisitorsPerPoint > 0)
        if (getPoints(ignoreFull).empty())
            return true;

    return false;
}

// AC Mgr/Travel/TravelMgr.cpp:1139. ChatHelper::FormatQuest is already present in this fork
// (AI/ChatHelper.h:60, ported by #322).
std::string const QuestTravelDestination::getTitle() { return ChatHelper::FormatQuest(questTemplate); }

bool QuestRelationTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (botAI && !botAI->HasStrategy("rpg quest", BOT_STATE_NON_COMBAT))
        return false;

    if (relation == 0)
    {
        if ((int32)questTemplate->GetQuestLevel() >= (int32)bot->GetLevel() + (int32)5)
            return false;

        // skip for now this quest
        if (getPoints().front()->GetMapId() != bot->GetMapId())
            return false;

        if (!bot->GetMap()->GetEntry()->IsWorldMap() || !bot->CanTakeQuest(questTemplate, false))
            return false;

        //uint32 dialogStatus = TravelMgr::instance().getDialogStatus(bot, entry, questTemplate); //not used, shadowed by the next declaration, line marked for removal.

        if (AI_VALUE(bool, "can fight equal"))
        {
            if (AI_VALUE(uint8, "free quest log slots") < 5)
                return false;

            //None has yellow exclamation mark.
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest npc::" + std::to_string(entry)))
                if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry) + "need quest objective::" + std::to_string(questId))) //Noone can do this quest for a usefull reward.
                    return false;
        }
        else
        {
            if (!AI_VALUE2(bool, "group or", "following party,near leader,can accept quest low level npc::" + std::to_string(entry))) //Noone can pick up this quest for money.
                return false;

            if (AI_VALUE(uint8, "free quest log slots") < 10)
                return false;
        }

        // Do not try to pick up dungeon/elite quests in instances without a group.
        if (IsInstanceQuest(questTemplate) && !AI_VALUE(bool, "can fight boss"))
            return false;
    }
    else
    {
        if (!AI_VALUE2(bool, "group or", "following party,near leader,can turn in quest npc::" + std::to_string(entry)))
            return false;

        //Do not try to hand-in dungeon/elite quests in instances without a group.
        if (IsInstanceQuest(questTemplate) && !AI_VALUE(bool, "can fight boss"))
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(&pos)->isOverworld())
                return false;
        }
    }

    return true;
}

std::string const QuestRelationTravelDestination::getTitle()
{
    std::ostringstream out;

    if (relation == 0)
        out << "questgiver";
    else
        out << "questtaker";

    out << " " << FormatWorldEntry(entry);
    return out.str();
}

bool QuestObjectiveTravelDestination::isActive(Player* bot)
{
    if (questTemplate->GetQuestLevel() > bot->GetLevel() + 1)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();
    if (questTemplate->GetQuestLevel() + 5 > bot->GetLevel() && !AI_VALUE(bool, "can fight equal"))
        return false;

    // Check mob level
    if (getEntry() > 0)
    {
        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(getEntry());
        if (cInfo && (int)cInfo->maxlevel - (int)bot->GetLevel() > 4)
            return false;

        // Do not try to hand-in dungeon/elite quests in instances without a group.
        if (cInfo->rank > CREATURE_ELITE_NORMAL)
        {
            WorldPosition pos(bot);
            if (!this->nearestPoint(const_cast<WorldPosition*>(&pos))->isOverworld() &&
                !AI_VALUE(bool, "can fight boss"))
                return false;

            if (!AI_VALUE(bool, "can fight elite"))
                return false;
        }
    }

    if (IsInstanceQuest(questTemplate) && !AI_VALUE(bool, "can fight elite"))
        return false;

    if (!TravelMgr::instance().getObjectiveStatus(bot, questTemplate, objective))
        return false;

    WorldPosition botPos(bot);

    if (getEntry() > 0 && !isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == uint32(getEntry()) && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    return true;
}

std::string const QuestObjectiveTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "objective " << objective;

    if (itemId)
        out << " loot " << ChatHelper::FormatItem(sObjectMgr->GetItemTemplate(itemId), 0, 0) << " from";
    else
        out << " to kill";

    out << " " << FormatWorldEntry(entry);
    return out.str();
}

bool RpgTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    CreatureTemplate const* cInfo = GetCreatureTemplate();
    if (!cInfo)
        return false;

    bool isUsefull = false;

    if (cInfo->npcflag & UNIT_NPC_FLAG_VENDOR)
        if (AI_VALUE2_LAZY(bool, "group or", "should sell,can sell,following party,near leader"))
            isUsefull = true;

    if (cInfo->npcflag & UNIT_NPC_FLAG_REPAIR)
        if (AI_VALUE2_LAZY(bool, "group or", "should repair,can repair,following party,near leader"))
            isUsefull = true;

    if (!isUsefull)
        return false;

    // Once the target rpged with it is added to the ignore list. We can now move on.
    GuidSet& ignoreList = GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<GuidSet&>("ignore rpg target")->Get();

    for (ObjectGuid const guid : ignoreList)
    {
        if (guid.GetEntry() == uint32(getEntry()))
            return false;

    }

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction > REP_NEUTRAL;
}

CreatureTemplate const* RpgTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const RpgTravelDestination::getTitle()
{
    std::ostringstream out;

    if (entry > 0)
        out << "rpg npc ";

    out << " " << FormatWorldEntry(entry);

    return out.str();
}

bool ExploreTravelDestination::isActive(Player* bot)
{
    AreaTableEntry const* area = sAreaTableStore.LookupEntry(areaId);

    if (area->area_level && (uint32)area->area_level > bot->GetLevel() && bot->GetLevel() < DEFAULT_MAX_LEVEL)
        return false;

    if (area->exploreFlag == 0xffff)
        return false;

    int offset = area->exploreFlag / 32;

    uint32 val = (uint32)(1 << (area->exploreFlag % 32));
    uint32 currFields = bot->GetUInt32Value(PLAYER_FIELD_EXPLORED_ZONES + offset);

    return !(currFields & val);
}

// std::string const ExploreTravelDestination::getTitle()
//{
//     return points[0]->getAreaName();
// };

bool GrindTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "should get money"))
        return false;

    if (AI_VALUE(bool, "should sell"))
        return false;

    CreatureTemplate const* cInfo = GetCreatureTemplate();

    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f;   //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f;  //(0-2.0f)

    int32 maxLevel = std::max(botLevel * (0.5f + levelMod), botLevel - 5.0f + levelBoost);

    if ((int32)cInfo->maxlevel > maxLevel)  //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = std::max(botLevel * (0.4f + levelMod), botLevel - 12.0f + levelBoost);

    if ((int32)cInfo->maxlevel < minLevel)  //@lvl5 min = 3, @lvl60 max = 50
        return false;

    if (!cInfo->mingold)
        return false;

    if (cInfo->rank > CREATURE_ELITE_NORMAL && !AI_VALUE(bool, "can fight elite"))
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    ReputationRank reaction = bot->GetReputationRank(factionEntry->faction);

    return reaction < REP_NEUTRAL;
}

CreatureTemplate const* GrindTravelDestination::GetCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const GrindTravelDestination::getTitle()
{
    std::ostringstream out;

    out << "grind mob ";

    out << " " << FormatWorldEntry(entry);

    return out.str();
}

bool BossTravelDestination::isActive(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    AiObjectContext* context = botAI->GetAiObjectContext();

    if (!AI_VALUE(bool, "can fight boss"))
        return false;

    CreatureTemplate const* cInfo = getCreatureTemplate();

    /*
    int32 botLevel = bot->GetLevel();

    uint8 botPowerLevel = AI_VALUE(uint8, "durability");
    float levelMod = botPowerLevel / 500.0f; //(0-0.2f)
    float levelBoost = botPowerLevel / 50.0f; //(0-2.0f)

    int32 maxLevel = botLevel + 3.0;

    if ((int32)cInfo->MaxLevel > maxLevel) //@lvl5 max = 3, @lvl60 max = 57
        return false;

    int32 minLevel = botLevel - 10;

    if ((int32)cInfo->MaxLevel < minLevel) //@lvl5 min = 3, @lvl60 max = 50
        return false;
    */

    if ((int32)cInfo->maxlevel > bot->GetLevel() + 3)
        return false;

    FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
    // #268 S1 5.4.8 divergence: AC calls `Unit::GetFactionReactionTo(bot->GetFactionTemplateEntry(),
    // factionEntry)` (AC Mgr/Travel/TravelMgr.cpp:1446), but on this core that helper is STATIC with a
    // `Unit const*` second argument (Entities/Unit/Unit.h:1354) and there is no unit here - only a
    // CreatureTemplate. The same problem was already solved in strategy/value/QuestValues.cpp:135-155 by
    // reproducing the core's faction-template branch (Unit.cpp:8665-8675) over the two template entries.
    // AC's test is `reaction >= REP_NEUTRAL -> return false`, i.e. "refuse anything not HOSTILE", so this
    // is `!IsHostileReactionTo(...)`: not-hostile means REP_NEUTRAL or better.
    bool const hostileToBot = factionEntry && bot->GetFactionTemplateEntry() &&
        bot->GetFactionTemplateEntry()->IsHostileTo(*factionEntry);
    if (!hostileToBot)
        return false;   // AC tests `reaction >= REP_NEUTRAL` and refuses too - that is the hostileToBot test above.

    WorldPosition botPos(bot);
    if (!isOut(&botPos))
    {
        GuidVector targets = AI_VALUE(GuidVector, "possible targets");

        for (auto& target : targets)
            if (target.GetEntry() == uint32(getEntry()) && target.IsCreature() && botAI->GetCreature(target) &&
                botAI->GetCreature(target)->IsAlive())
                return true;

        return false;
    }

    if (!AI_VALUE2(bool, "has upgrade", getEntry()))
        return false;

    return true;
}

CreatureTemplate const* BossTravelDestination::getCreatureTemplate() { return sObjectMgr->GetCreatureTemplate(entry); }

std::string const BossTravelDestination::getTitle()
{
    std::ostringstream out;
    out << "boss mob ";
    out << " " << FormatWorldEntry(entry);

    return out.str();
}

// AC Mgr/Travel/TravelMgr.cpp:4262-4269 - QuestTravelDestination's constructor and isActive. AC files
// them ~3k lines away from the rest of the hierarchy; S1's header block declares both, and every
// subclass's constructor chains into this one.
QuestTravelDestination::QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1)
    : TravelDestination(radiusMin1, radiusMax1)
{
    questId = questId1;
    questTemplate = sObjectMgr->GetQuestTemplate(questId);
}

bool QuestTravelDestination::isActive(Player* bot) { return bot->IsActiveQuest(questId); }

// AC Mgr/Travel/TravelMgr.cpp:4271-4283. AC's RequiredNpcOrGo[i] carries the sign to distinguish
// creature (>0) from gameobject (<0) and RequiredNpcOrGoCount[i] holds the amount. 5.4.8 splits those
// into QuestObjective::Type (QUEST_OBJECTIVE_MONSTER / QUEST_OBJECTIVE_GAMEOBJECT, ObjectID now
// unsigned) and QuestObjective::Amount. Behaviour is the same; only the lookup differs.
bool QuestObjectiveTravelDestination::isCreature()
{
    QuestObjective const* questObjective = ObjectiveAt(GetQuestTemplate(), objective);
    return questObjective && questObjective->Type == QUEST_OBJECTIVE_MONSTER;
}

uint32 QuestObjectiveTravelDestination::ReqCreature()
{
    QuestObjective const* questObjective = ObjectiveAt(GetQuestTemplate(), objective);
    return questObjective && questObjective->Type == QUEST_OBJECTIVE_MONSTER ? questObjective->ObjectID : 0;
}

uint32 QuestObjectiveTravelDestination::ReqGOId()
{
    QuestObjective const* questObjective = ObjectiveAt(GetQuestTemplate(), objective);
    return questObjective && questObjective->Type == QUEST_OBJECTIVE_GAMEOBJECT ? questObjective->ObjectID : 0;
}

uint32 QuestObjectiveTravelDestination::ReqCount()
{
    QuestObjective const* questObjective = ObjectiveAt(GetQuestTemplate(), objective);
    return questObjective ? uint32(questObjective->Amount) : 0;
}


// ================================================================================================
// #268 slice S1 completion - the two TravelMgr members S1's TravelDestination hierarchy calls but
// that AC defines ~2.5k lines away from the hierarchy. S1 declared both (TravelMgr.h:435-437); with a
// declaration and no definition the module compiles and then fails to LINK with exactly two unresolved
// externals (TravelMgr::getNextPoint from TravelDestination::nextPoint, TravelMgr::getObjectiveStatus
// from QuestObjectiveTravelDestination::isActive). The definitions therefore land here, not in S2 as the
// six-slice plan had it.
//
// Core adaptations, all called out at the site:
//   * Acore::Containers::RandomShuffle does not exist in this fork, so the shuffle is std::shuffle over
//     a std::mt19937 seeded from the module RNG (urand, Random.h) - the same shape the rest of the fork
//     already uses for shuffles (RandomPlayerbotMgr.cpp:236-237, RandomItemManager.cpp:705-707,
//     DropQuestAction.cpp:222-224). AC's helper is a plain uniform Fisher-Yates, so the distribution of
//     the shuffle is unchanged.
//   * AC's second getNextPoint overload (by-value WorldPosition, AC :3892-3940) has zero callers anywhere
//     in AC; #268 S8 ported it anyway (see the S8 section below the pointer overload).
//   * AC's distance-weighting block (AC :3869-3887) is commented out upstream and stays commented out
//     here. The weights vector and the unused dists vector that feed it are kept verbatim from AC (both
//     are dead weight in AC too) so the commented block keeps the inputs it was written against.
//   * TravelMgr::getQuestStatus (AC :3942, a one-line &bot->getQuestStatusMap()[questId] accessor) was
//     not ported by S1; #268 S8 ported it (no caller, as in AC; getObjectiveStatus no longer needs it
//     on 5.4.8, see below). AC uses it only from getObjectiveStatus.
//   * No aliasing trap is reproduced: neither function takes the address of a local WorldPosition or
//     stores one. getNextPoint returns the very pointers its input vector already holds, which are the
//     TravelDestination's own (heap) WorldPositions. The pattern the slice plan warns about - AC's
//     addLevel/addQuestData-style helpers taking &loopLocal and handing it to a destination that outlives
//     the loop (AC :2072-2076, :2284-2295, :2334-2363) - lives entirely in the S2/S3 destination-adding
//     code and is not touched here.
// ================================================================================================

// AC Mgr/Travel/TravelMgr.cpp:3838-3890 - the pointer overload, i.e. the one TravelDestination::nextPoint
// calls (AC :1124; this fork TravelMgr.cpp:865). "Selects a random WorldPosition from a list. Use a
// distance weighted distribution." - the weighting is still upstream-disabled, so in practice this
// returns the whole input list in random order.
std::vector<WorldPosition*> TravelMgr::getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points,
                                                    uint32 /*amount*/)
{
    std::vector<WorldPosition*> retVec;

    if (points.size() < 2)
    {
        if (points.size() == 1)
            retVec.push_back(points[0]);

        return retVec;
    }

    retVec = points;

    std::vector<uint32> weights;

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    // std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights), [center](WorldPosition point) { return
    // 1 + 1000 * exp(-1 * pow(point.distance(center) / 400.0, 2)); });

    // List of weights based on distance (Twice the distance = half the weight). Caps out at 200.0000 range.
    std::transform(retVec.begin(), retVec.end(), std::back_inserter(weights),
                   [center](WorldPosition* point)
                   { return static_cast<uint32>(200000.f / (1.f + point->distance(center))); });

    // #268 S1: Acore::Containers::RandomShuffle(retVec) has no fork equivalent - std::shuffle over a
    // std::mt19937 seeded from the module RNG (see the banner above).
    std::mt19937 shuffleGen(urand(0, std::numeric_limits<uint32>::max()));
    std::shuffle(retVec.begin(), retVec.end(), shuffleGen);

    std::vector<float> dists;

    // Total sum of all those weights.
    /*
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    //Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        //Pick a random number in that range.
        uint32 rnd = urand(0, sum);

        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] && (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) ==
    retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }*/

    return retVec;
}

// ================================================================================================
// #268 slice S8 - the remaining TravelMgr class members (AC Mgr/Travel/TravelMgr.cpp:3726-3836,
// :3892-3942, :4285-4358). None of them has a caller in AC; ported 1:1 anyway (owner rule 2026-10-05).
// ================================================================================================

// AC TravelMgr.cpp:3892-3940, verbatim. By-value overload of getNextPoint.
std::vector<WorldPosition> TravelMgr::getNextPoint(WorldPosition center, std::vector<WorldPosition> points,
                                                   uint32 amount)
{
    std::vector<WorldPosition> retVec;

    if (points.size() == 1)
    {
        retVec.push_back(points[0]);
        return retVec;
    }

    // List of weights based on distance (Gausian curve that starts at 100 and lower to 1 at 1000 distance)
    std::vector<uint32> weights;

    std::transform(points.begin(), points.end(), std::back_inserter(weights),
                   [center](WorldPosition point)
                   { return 1 + 1000 * static_cast<uint32>(exp(-1.f * pow(point.distance(center) / 400.f, 2.f))); });

    // Total sum of all those weights.
    uint32 sum = std::accumulate(weights.begin(), weights.end(), 0);

    // Pick a random number in that range.
    uint32 rnd = urand(0, sum);

    // Pick a random point based on weights.
    for (uint32 nr = 0; nr < amount; nr++)
    {
        for (unsigned i = 0; i < points.size(); ++i)
            if (rnd < weights[i] &&
                (retVec.empty() || std::find(retVec.begin(), retVec.end(), points[i]) == retVec.end()))
            {
                retVec.push_back(points[i]);
                break;
            }
            else
                rnd -= weights[i];
    }

    // Peiru: Crash failsafe - if the retVec is still empty but points exist, return first point
    if (retVec.empty() && points.size() > 0)
        retVec.push_back(points[0]);

    if (!retVec.empty())
        return retVec;

    assert(!"No valid point found.");

    return retVec;
}

// AC TravelMgr.cpp:3942, verbatim. Player::getQuestStatusMap() is Entities/Player/Player.h:1923.
// AC bug carried: operator[] inserts a default QuestStatusData for an unknown questId.
QuestStatusData* TravelMgr::getQuestStatus(Player* bot, uint32 questId) { return &bot->getQuestStatusMap()[questId]; }

// AC TravelMgr.cpp:4285-4358 (printGrid, printObj), verbatim apart from the singleton syntax
// (sPlayerbotAIConfig. -> sPlayerbotAIConfig->). Every API exists here: PlayerbotAIConfig::hasLog /
// GetTimestampStr / log (Utils/PlayerbotAIConfig.h:283-294), WorldPosition::printWKT / fromGridCoord /
// getCellCoord / fromCellCoord / gridFromCellCoord (Travel/WorldPosition.h:139,259-263), Cell(float,float)
// + GridX()/GridY() (Grids/Cells/Cell.h, CellImpl.h), ObjectGuid::GetEntry()/GetCounter().
void TravelMgr::printGrid(uint32 mapId, int x, int y, std::string const type)
{
    std::string const fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(mapId, 0, 0, 0, 0);

        std::ostringstream out;
        out << sPlayerbotAIConfig->GetTimestampStr();
        out << "+00, " << 0 << 0 << x << "," << y << ", " << type << ",";
        p.printWKT(p.fromGridCoord(GridCoord(x, y)), out, 1, true);
        sPlayerbotAIConfig->log(fileName, out.str().c_str());
    }
}

void TravelMgr::printObj(WorldObject* obj, std::string const type)
{
    std::string fileName = "unload_grid.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());

        std::vector<WorldPosition> vcell, vgrid;
        vcell = p.fromCellCoord(p.getCellCoord());
        vgrid = p.gridFromCellCoord(p.getCellCoord());

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vcell, out, 1, true);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }

        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT(vgrid, out, 1, true);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }
    }

    fileName = "unload_obj.csv";

    if (sPlayerbotAIConfig->hasLog(fileName))
    {
        WorldPosition p = WorldPosition(obj);

        Cell cell(obj->GetPositionX(), obj->GetPositionY());
        {
            std::ostringstream out;
            out << sPlayerbotAIConfig->GetTimestampStr();
            out << "+00, " << obj->GetGUID().GetEntry() << "," << obj->GetGUID().GetCounter() << "," << cell.GridX()
                << "," << cell.GridY() << ", " << type << ",";

            p.printWKT({p}, out, 0);
            sPlayerbotAIConfig->log(fileName, out.str().c_str());
        }
    }
}

// AC TravelMgr.cpp:3726-3836, NOT ported live (no caller in AC). Reason: it needs WotLK DIALOG_STATUS_*
// constants and AC's `dialogStatusNew > dialogStatus` relies on their ordering; 5.4.8 only has
// `enum class QuestGiverStatus` (Quests/QuestDef.h:111) with different values/ordering. Verbatim AC text:
#if 0
uint32 TravelMgr::getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest)
{
    uint32 dialogStatus = DIALOG_STATUS_NONE;

    QuestRelationBounds rbounds;   // QuestRelations (quest-giver)
    QuestRelationBounds irbounds;  // InvolvedRelations (quest-finisher)

    uint32 questId = pQuest->GetQuestId();

    if (questgiver > 0)
    {
        rbounds = sObjectMgr->GetCreatureQuestRelationBounds(questgiver);
        irbounds = sObjectMgr->GetCreatureQuestInvolvedRelationBounds(questgiver);
    }
    else
    {
        rbounds = sObjectMgr->GetGOQuestRelationBounds(questgiver * -1);
        irbounds = sObjectMgr->GetGOQuestInvolvedRelationBounds(questgiver * -1);
    }

    // Check markings for quest-finisher
    for (QuestRelations::const_iterator itr = irbounds.first; itr != irbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if ((status == QUEST_STATUS_COMPLETE && !pPlayer->GetQuestRewardStatus(questId)) ||
            (pQuest->IsAutoComplete() && pPlayer->CanTakeQuest(pQuest, false)))
        {
            if (pQuest->IsAutoComplete() && pQuest->IsRepeatable())
            {
                dialogStatusNew = DIALOG_STATUS_REWARD_REP;
            }
            else
            {
                dialogStatusNew = DIALOG_STATUS_REWARD2;
            }
        }
        else if (status == QUEST_STATUS_INCOMPLETE)
        {
            dialogStatusNew = DIALOG_STATUS_INCOMPLETE;
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    // check markings for quest-giver
    for (QuestRelations::const_iterator itr = rbounds.first; itr != rbounds.second; ++itr)
    {
        if (itr->second != questId)
            continue;

        uint32 dialogStatusNew = DIALOG_STATUS_NONE;

        if (!pQuest)
        {
            continue;
        }

        QuestStatus status = pPlayer->GetQuestStatus(questId);

        if (status == QUEST_STATUS_NONE)  // For all other cases the mark is handled either at some place else, or with
                                          // involved-relations already
        {
            if (pPlayer->CanSeeStartQuest(pQuest))
            {
                if (pPlayer->SatisfyQuestLevel(pQuest, false))
                {
                    int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
                    if (pQuest->IsAutoComplete() || (pQuest->IsRepeatable() && pPlayer->IsQuestRewarded(questId)))
                    {
                        dialogStatusNew = DIALOG_STATUS_REWARD_REP;
                    }
                    else if (lowLevelDiff < 0 ||
                             pPlayer->GetLevel() <= pPlayer->GetQuestLevel(pQuest) + uint32(lowLevelDiff))
                    {
                        dialogStatusNew = DIALOG_STATUS_AVAILABLE;
                    }
                    else
                    {
                        dialogStatusNew = DIALOG_STATUS_LOW_LEVEL_AVAILABLE;
                    }
                }
                else
                {
                    dialogStatusNew = DIALOG_STATUS_UNAVAILABLE;
                }
            }
        }

        if (dialogStatusNew > dialogStatus)
        {
            dialogStatus = dialogStatusNew;
        }
    }

    return dialogStatus;
}

#endif

// AC Mgr/Travel/TravelMgr.cpp:3944-3968.
//
// 5.4.8 rewrite - none of the four containers AC reads exist on this core. AC compares
//     pQuest->RequiredItemCount[objective]        vs  questStatus->ItemCount[objective]        (items)
//     pQuest->RequiredNpcOrGoCount[objective]     vs  questStatus->CreatureOrGOCount[objective]  (creature/GO)
// Quest::RequiredItemCount/RequiredNpcOrGoCount and QuestStatusData::ItemCount/CreatureOrGOCount are
// WotLK-only. On 5.4.8 both halves come from ONE objective entry (Quest::GetObjectives(),
// QuestDef.h:376; QuestObjective at QuestDef.h:280-291):
//   * the index into that vector takes the place of the old parallel-array index. The file-local
//     ObjectiveAt() helper (declared for S1 above) already does that lookup bounds-checked, and is what
//     QuestObjectiveTravelDestination::isCreature/ReqCreature/ReqGOId/ReqCount use just above.
//   * QuestObjective::Type (QUEST_OBJECTIVE_ITEM / _MONSTER / _GAMEOBJECT, QuestDef.h:203-208) states
//     which of AC's two families the objective belongs to. WotLK had no type here and encoded the same
//     fact as a sign (RequiredNpcOrGo[i] < 0 == gameobject); MoP states it outright.
//   * QuestObjective::Amount (int32) is the required count, and progress is
//     Player::GetQuestObjectiveCounter(objective.ID) (Player.h:3186) - the single replacement for BOTH
//     of AC's parallel status arrays.
// So AC's two sequential guards collapse into one: this objective is of a type AC tracked, it wants a
// count, and the bot has not reached it. That is the same rewrite (and the same reasoning) already
// established in strategy/value/QuestValues.cpp:477-496.
bool TravelMgr::getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective)
{
    uint32 questId = pQuest->GetQuestId();
    if (!bot->IsActiveQuest(questId))
        return false;

    if (bot->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
        return false;

    QuestObjective const* questObjective = ObjectiveAt(pQuest, objective);
    if (!questObjective)
        return false;

    // Only the two families AC guarded: every other 5.4.8 objective type (explore, currency, reputation,
    // event, spell, ...) is a no-count objective, and AC also had both counts at 0 for anything that was
    // not an item or a creature/gameobject, so it returned false for those too.
    if (questObjective->Type != QUEST_OBJECTIVE_ITEM && questObjective->Type != QUEST_OBJECTIVE_MONSTER &&
        questObjective->Type != QUEST_OBJECTIVE_GAMEOBJECT)
        return false;

    // Amount is int32 on 5.4.8 and 0 for "no amount required", which is AC's `reqCount &&` guard.
    if (questObjective->Amount <= 0)
        return false;

    uint32 reqCount = uint32(questObjective->Amount);
    uint32 hasCount = bot->GetQuestObjectiveCounter(questObjective->ID);

    return hasCount < reqCount;
}

// ================================================================================================
// #268 slice S2 - the destination accessors (AC Mgr/Travel/TravelMgr.cpp:3970-4139) and
// TravelMgr::logQuestError() (AC .cpp:1750-1815).
//
// 5.4.8 notes, all mechanical:
//   * AC spells out TravelMgr::instance() nowhere in these four functions, so there is no
//     TravelMgr::instance() -> sTravelMgr rewrite here.
//   * Quest::GetTitle() does not exist on 5.4.8. Quest::GetLogTitle() (QuestDef.h:365, returns
//     `std::string const&`) is the log-facing title and is what AC's GetTitle() returned there.
//   * CreatureTemplate::Name (CreatureData.h:162) and GameObjectTemplate::name (GameObjectData.h:37)
//     are std::string on this core rather than char*, so both go to the logger via .c_str().
//   * LOG_ERROR("playerbots", ...) -> TC_LOG_ERROR("playerbots", ...): the fork's Log.h spelling.
//   * AC's fmt-style "{}" placeholders -> printf-style "%s"/"%u". This is the same substitution the
//     rest of this file already made: TC_LOG_ERROR reaches Trinity::StringFormat (Log.h:77), which is
//     fmt::sprintf (Utilities/StringFormat.h:31), i.e. printf - not fmt::format. Leaving "{}" in would
//     format as literal braces and, on the non-Windows build, would also trip check_args's
//     ATTR_PRINTF(1, 2) (Log.h:151). So the message TEXT is the only thing that changes; every branch,
//     argument and condition is AC's.
//   * ItemTemplate (== ItemPrototype, Entities/Item/ItemPrototype.h:659) is already complete here via
//     ObjectMgr.h:31, and ObjectMgr.h is included at TravelMgr.cpp:25 - after DBCStructure.h
//     (TravelMgr.cpp:18), which is the include-order that header requires. No new include needed.
//   * AC's `bool logQuestErrors = false;  // For debugging.` gate is carried over verbatim, so the
//     string building and every logQuestError branch below it stay dead exactly as they are upstream.
//   * getBossTravelDestinations is NOT defined here: AC defines it in
//     Ai/Base/Actions/ChooseTravelTargetAction.cpp:781, and #269 owns that file.
//
// ALIASING: none of this touches a WorldPosition that outlives a local. The four accessors build one
// `WorldPosition botLocation(bot)` each and pass its address to distanceTo(); it is a stack object
// dropped at return and never stored. S2 allocates NO destination points at all - the addPoint() sites
// live in S3's LoadQuestTravelTable, and S3 is the slice that has to own those points rather than
// repeat AC's &loop-local aliasing.
//
// The two AI-value gaps this block originally warned about are now RESOLVED:
//   * "has upgrade"          - ported (#353, PR #355); the value and its "entry loot usage" /
//                              "entry loot list" chain are registered in strategy/value/ValueContext.h,
//                              so BossTravelDestination::isActive() calls the plain AI_VALUE2 again.
//   * "need quest objective" - was never a value at all: it is qualifier text concatenated into the
//                              "group or" lookup, which resolves fine. AC carries its own string bug
//                              there (the qualifier mangles); reproduced verbatim, not a deref.
// What still matters for #269: the destinations stay empty until LoadQuestTravelTable() is called, and
// that is deliberately deferred because AC does not call it either - see the wiring note there.
// ================================================================================================

std::vector<TravelDestination*> TravelMgr::getQuestTravelDestinations(Player* bot, int32 questId, bool ignoreFull,
                                                                      bool ignoreInactive, float maxDistance,
                                                                      bool ignoreObjectives)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    if (!questId)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
        for (auto& quest : quests)
        {
            for (auto& dest : quest.second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : quest.second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }
    else if (questId == -1)
    {
        for (auto& dest : questGivers)
        {
            if (!ignoreInactive && !dest->isActive(bot))
                continue;

            if (dest->isFull(ignoreFull))
                continue;

            if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                continue;

            retTravelLocations.push_back(dest);
        }
    }
    else
    {
        auto i = quests.find(questId);

        if (i != quests.end())
        {
            for (auto& dest : i->second->questTakers)
            {
                if (!ignoreInactive && !dest->isActive(bot))
                    continue;

                if (dest->isFull(ignoreFull))
                    continue;

                if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                    continue;

                retTravelLocations.push_back(dest);
            }

            if (!ignoreObjectives)
                for (auto& dest : i->second->questObjectives)
                {
                    if (!ignoreInactive && !dest->isActive(bot))
                        continue;

                    if (dest->isFull(ignoreFull))
                        continue;

                    if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
                        continue;

                    retTravelLocations.push_back(dest);
                }
        }
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getRpgTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                    float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : rpgNpcs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getExploreTravelDestinations(Player* bot, bool ignoreFull,
                                                                        bool ignoreInactive)
{
    // botLocation is unused in AC too - kept so the signature and the body stay 1:1.
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : exploreLocs)
    {
        if (!ignoreInactive && !dest.second->isActive(bot))
            continue;

        if (dest.second->isFull(ignoreFull))
            continue;

        retTravelLocations.push_back(dest.second);
    }

    return retTravelLocations;
}

std::vector<TravelDestination*> TravelMgr::getGrindTravelDestinations(Player* bot, bool ignoreFull, bool ignoreInactive,
                                                                      float maxDistance)
{
    WorldPosition botLocation(bot);

    std::vector<TravelDestination*> retTravelLocations;

    for (auto& dest : grindMobs)
    {
        if (!ignoreInactive && !dest->isActive(bot))
            continue;

        if (dest->isFull(ignoreFull))
            continue;

        if (maxDistance > 0 && dest->distanceTo(&botLocation) > maxDistance)
            continue;

        retTravelLocations.push_back(dest);
    }

    return retTravelLocations;
}

void TravelMgr::logQuestError(uint32 errorNr, Quest* quest, uint32 objective, uint32 unitId, uint32 itemId)
{
    bool logQuestErrors = false;  // For debugging.

    if (!logQuestErrors)
        return;

    std::string unitName = "<unknown>";
    CreatureTemplate const* cInfo = nullptr;
    GameObjectTemplate const* gInfo = nullptr;

    if (unitId > 0)
        cInfo = sObjectMgr->GetCreatureTemplate(unitId);
    else
        gInfo = sObjectMgr->GetGameObjectTemplate(unitId * -1);

    if (cInfo)
        unitName = cInfo->Name.c_str();
    else if (gInfo)
        unitName = gInfo->name.c_str();

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);

    if (errorNr == 1)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] has %s %s [%u] but none is found in the world.",
                     quest->GetLogTitle().c_str(), quest->GetQuestId(), objective == 0 ? "quest giver" : "quest taker",
                     unitName.c_str(), unitId);
    }
    else if (errorNr == 2)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] needs %s [%u] for objective %u but none is found in the world.",
                     quest->GetLogTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, objective);
    }
    else if (errorNr == 3)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] needs itemId %u but no such item exists.",
                     quest->GetLogTitle().c_str(), quest->GetQuestId(), itemId);
    }
    else if (errorNr == 4)
    {
        TC_LOG_ERROR(
            "playerbots",
            "Quest %s [%u] needs %s [%u] for loot of item %s [%u] for objective %u but none is found in the world.",
            quest->GetLogTitle().c_str(), quest->GetQuestId(), unitName.c_str(), unitId, proto->Name1.c_str(), itemId,
            objective);
    }
    else if (errorNr == 5)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] needs item %s [%u] for objective %u but none is found in the world.",
                     quest->GetLogTitle().c_str(), quest->GetQuestId(), proto->Name1.c_str(), itemId, objective);
    }
    else if (errorNr == 6)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] has no quest giver.", quest->GetLogTitle().c_str(),
                     quest->GetQuestId());
    }
    else if (errorNr == 7)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] has no quest taker.", quest->GetLogTitle().c_str(),
                     quest->GetQuestId());
    }
    else if (errorNr == 8)
    {
        TC_LOG_ERROR("playerbots", "Quest %s [%u] has no quest viable quest objective.",
                     quest->GetLogTitle().c_str(), quest->GetQuestId());
    }
}

// ================================================================================================
// #268 slice S5 - TravelTarget (AC Mgr/Travel/TravelMgr.cpp:1481-1720), TravelMgr::Clear()'s player
// loop (AC .cpp:1721-1726) and TravelMgr::setNullTravelTarget() (AC .cpp:4141-4154).
//
// 5.4.8 notes, all mechanical:
//   * sPlayerbotAIConfig. -> sPlayerbotAIConfig->  (this fork's PlayerbotAIConfig::instance() returns
//     a pointer, Utils/PlayerbotAIConfig.h:58-59, :297).
//   * TravelMgr::instance() -> sTravelMgr where AC writes TravelMgr::instance() inline (same object).
//   * setStatus(): AC's TRAVEL_STATUS_COOLDOWN case is missing its `break` and falls through
//     into `default: break;` (AC .cpp:1566-1570). Behaviour is identical, so the break is written
//     out explicitly here rather than carrying a -Wimplicit-fallthrough through the fork.
//
// ALIASING: this slice never constructs a WorldPosition that outlives a local. The only ones it
// builds are `WorldPosition pos(bot)` inside distance()/isTraveling()/isWorking(), passed by
// address to distance()/isIn() and dropped at return - AC's aliasing sites are in
// LoadQuestTravelTable (AC .cpp:2072-2076, :2284-2295, :2334-2363), which is S3, and S2/S3 own the
// destination points TravelTarget's wPosition points at.
// ================================================================================================

TravelTarget::~TravelTarget()
{
    if (!tDestination)
        return;

    releaseVisitors();
    // AC Mgr/Travel/TravelMgr.cpp:1487-1488 has the same removal of `this` from
    // TravelMgr::instance().botTargets commented out, so nothing owns/unowns this object either
    // upstream or here; the value in the AI object context owns it. Kept as AC left it.
}

void TravelTarget::setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1)
{
    releaseVisitors();

    wPosition = wPosition1;
    tDestination = tDestination1;
    groupCopy = groupCopy1;
    forced = false;
    radius = 0;

    addVisitors();

    setStatus(TRAVEL_STATUS_TRAVEL);
}

void TravelTarget::copyTarget(TravelTarget* target)
{
    setTarget(target->tDestination, target->wPosition);
    groupCopy = target->isGroupCopy();
    forced = target->forced;
    extendRetryCount = target->extendRetryCount;
}

void TravelTarget::addVisitors()
{
    if (!visitor)
    {
        wPosition->addVisitor();
        tDestination->addVisitor();
    }

    visitor = true;
}

void TravelTarget::releaseVisitors()
{
    if (visitor)
    {
        if (tDestination)
            tDestination->remVisitor();
        if (wPosition)
            wPosition->remVisitor();
    }

    visitor = false;
}

float TravelTarget::distance(Player* bot)
{
    WorldPosition pos(bot);
    return wPosition->distance(&pos);
}

WorldPosition* TravelTarget::getPosition() { return wPosition; }

TravelDestination* TravelTarget::getDestination() { return tDestination; }

void TravelTarget::setStatus(TravelStatus status)
{
    m_status = status;
    startTime = getMSTime();

    switch (m_status)
    {
        case TRAVEL_STATUS_NONE:
        case TRAVEL_STATUS_PREPARE:
        case TRAVEL_STATUS_EXPIRED:
            statusTime = 1;
            break;
        case TRAVEL_STATUS_TRAVEL:
            statusTime = getMaxTravelTime() * 2 + sPlayerbotAIConfig->maxWaitForMove;
            break;
        case TRAVEL_STATUS_WORK:
            statusTime = tDestination->getExpireDelay();
            break;
        case TRAVEL_STATUS_COOLDOWN:
            statusTime = tDestination->getCooldownDelay();
            break;
        default:
            break;
    }
}

bool TravelTarget::isActive()
{
    if (m_status == TRAVEL_STATUS_NONE || m_status == TRAVEL_STATUS_EXPIRED || m_status == TRAVEL_STATUS_PREPARE)
        return false;

    if (forced && isTraveling())
        return true;

    if ((statusTime > 0 && startTime + statusTime < getMSTime()))
    {
        setStatus(TRAVEL_STATUS_EXPIRED);
        return false;
    }

    if (m_status == TRAVEL_STATUS_COOLDOWN)
        return true;

    if (isTraveling())
        return true;

    if (isWorking())
        return true;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return true;
    }

    return true;
}

uint32 TravelTarget::getMaxTravelTime() { return (1000.0 * distance(bot)) / bot->GetSpeed(MOVE_RUN); }

bool TravelTarget::isTraveling()
{
    if (m_status != TRAVEL_STATUS_TRAVEL)
        return false;

    if (!tDestination->isActive(bot) && !forced)  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    bool HasArrived = tDestination->isIn(&pos, radius);

    if (HasArrived)
    {
        setStatus(TRAVEL_STATUS_WORK);
        return false;
    }

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isWorking()
{
    if (m_status != TRAVEL_STATUS_WORK)
        return false;

    if (!tDestination->isActive(bot))  // Target has become invalid. Stop.
    {
        setStatus(TRAVEL_STATUS_COOLDOWN);
        return false;
    }

    WorldPosition pos(bot);

    /*
    bool HasLeft = tDestination->isOut(&pos);

    if (HasLeft)
    {
        setStatus(TRAVEL_STATUS_TRAVEL);
        return false;
    }
    */

    if (!botAI->HasStrategy("travel", BOT_STATE_NON_COMBAT))
    {
        setTarget(sTravelMgr.nullTravelDestination, sTravelMgr.nullWorldPosition, true);
        return false;
    }

    return true;
}

bool TravelTarget::isPreparing()
{
    if (m_status != TRAVEL_STATUS_PREPARE)
        return false;

    return true;
}

TravelState TravelTarget::getTravelState()
{
    if (!tDestination || tDestination->getName() == "NullTravelDestination")
        return TRAVEL_STATE_IDLE;

    if (tDestination->getName() == "QuestRelationTravelDestination")
    {
        if (((QuestRelationTravelDestination*)tDestination)->getRelation() == 0)
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_PICK_UP_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_PICK_UP_QUEST;
        }
        else
        {
            if (isTraveling() || isPreparing())
                return TRAVEL_STATE_TRAVEL_HAND_IN_QUEST;

            if (isWorking())
                return TRAVEL_STATE_WORK_HAND_IN_QUEST;
        }
    }
    else if (tDestination->getName() == "QuestObjectiveTravelDestination")
    {
        if (isTraveling() || isPreparing())
            return TRAVEL_STATE_TRAVEL_DO_QUEST;

        if (isWorking())
            return TRAVEL_STATE_WORK_DO_QUEST;
    }
    else if (tDestination->getName() == "RpgTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_RPG;
    }
    else if (tDestination->getName() == "ExploreTravelDestination")
    {
        return TRAVEL_STATE_TRAVEL_EXPLORE;
    }

    return TRAVEL_STATE_IDLE;
}

// AC Mgr/Travel/TravelMgr.cpp:1721-1747.
//
// #268 S5 brought the player loop (AC .cpp:1721-1726); #268 S2 extended the SAME body with AC's
// destination-container loops (AC .cpp:1728-1747), which read the questGivers / quests containers
// that S2 introduces. One definition only - S2 did not re-declare Clear() in TravelMgr.h.
//
// Ownership notes, all AC's, none invented here:
//   * AC deletes the destinations held by each QuestContainer but never deletes the QuestContainer
//     itself, and `quests.clear()` then drops the containers without freeing them - one leaked struct
//     per quest per Clear(). Kept verbatim rather than silently "fixed", and flagged: S3 is the slice
//     that finally populates `quests`, so S3 is where that leak becomes observable.
//   * AC clears only questGivers and quests. rpgNpcs, grindMobs, bossMobs and exploreLocs are never
//     cleared upstream, so their destinations are neither deleted nor dropped. Also kept verbatim:
//     adding a clear for the four containers that nothing populates yet would change nothing today and
//     would not be a 1:1 port.
void TravelMgr::Clear()
{
    std::shared_lock<std::shared_mutex> lock(*HashMapHolder<Player>::GetLock());
    HashMapHolder<Player>::MapType const& m = ObjectAccessor::GetPlayers();
    for (HashMapHolder<Player>::MapType::const_iterator itr = m.begin(); itr != m.end(); ++itr)
        setNullTravelTarget(itr->second);

    for (auto& quest : quests)
    {
        for (auto& dest : quest.second->questGivers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questTakers)
        {
            delete dest;
        }

        for (auto& dest : quest.second->questObjectives)
        {
            delete dest;
        }
    }

    questGivers.clear();
    quests.clear();
}

// AC Mgr/Travel/TravelMgr.cpp:4141-4154. Unchanged apart from TravelMgr::instance() -> sTravelMgr,
// which is the same object (the macro is defined at the bottom of TravelMgr.h).
//
// This is the one caller of the "travel target" value inside the module outside S6's TargetValue
// payload. It only ever dereferences the value it gets back, so it is safe today while
// TravelTargetValue still hands out nullptr (strategy/value/TargetValue.h:109-113).
void TravelMgr::setNullTravelTarget(Player* player)
{
    if (!player)
        return;

    PlayerbotAI* playerBotAI = GET_PLAYERBOT_AI(player);
    if (!playerBotAI)
        return;

    TravelTarget* target = playerBotAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get();

    if (target)
        target->setTarget(nullTravelDestination, nullWorldPosition, true);
}

// ================================================================================================
// #268 slice S3 - TravelMgr::LoadQuestTravelTable(), the destination-cache population.
//
// Ported from AC Mgr/Travel/TravelMgr.cpp:1817-2383, and STOPPING at AC :2385 exactly as the #268
// plan requires: everything from AC :2385 to the end of AC's function (to ~:3723) is commented-out
// dead code upstream and is not ported.
//
// 5.4.8 divergences applied here, all called out at the site:
//   * sPlayerbotAIConfig. -> sPlayerbotAIConfig-> (Utils/PlayerbotAIConfig.h).
//   * LOG_INFO("playerbots", "{}") -> TC_LOG_INFO("playerbots", "%u"/"%s"): the fork's TC_LOG_* is
//     printf (Log.h:77 -> Trinity::StringFormat), not fmt.
//   * WorldPosition().getCreaturesNear() does not exist on the fork. Substituting
//     sTravelMgr.GetCreaturesNear() + SpawnGridIndex (the fork's own O(1) replacement, TravelMgr.h:474)
//     would be WRONG here rather than merely different: AC calls it on a DEFAULT-CONSTRUCTED
//     WorldPosition (map 0, x 0, y 0, radius 0), so a grid query would return nothing. The correct
//     equivalent is the same whole-store walk AC's helper performed, i.e.
//     sObjectMgr->GetAllCreatureData() - which is exactly what SpawnGridIndex::Build
//     (TravelMgr.cpp:94) already does.
//   * creatureData->mapid -> .mapId.
//   * GameObject spawns: NOT ported, and this is a NO-OP for everything S3 populates. The fork has
//     no whole-gameobject-spawn-store accessor - ObjectMgr keeps _gameObjectDataStore private
//     (Globals/ObjectMgr.h:1952) with only GetGOData(guid) public (ObjectMgr.h:1575), which #257
//     already hit and documented (strategy/value/QuestValues.cpp:43-45, :298-300). So `units` here
//     holds creature spawns only (t_unit.type is always 0), and that is exactly what the three live
//     consumers do with it: the rpg/grind/boss loop skips `u.type != 0` (AC :2261-2262) and the
//     explore loop skips `u.type == 1` (AC :2343-2344). Game-object spawns were only ever read by the
//     commented-out relation/loot block (AC :2091-2248), which S3 does not port. Quest objectives that
//     target gameobjects are unaffected: their points come from the quest guidp map below, not from
//     `units`, and that map already carries gameobject spawns (QuestValues.cpp:279-288).
//   * area->area_name[0] -> area->area_name: on 5.4.8 AreaTableEntry::area_name is a single
//     `char const*` (DataStores/DBCStructure.h:573), not an array, so AC's [0] would yield one char.
//   * area->zone -> not needed here; only getArea()/area->ID/exploreFlag are used, and all three keep
//     AC's names on 5.4.8 (DBCStructure.h:558-574).
//   * Dropped verbatim from AC :2381-2383 (TravelNodeMap::instance().loadNodeStore() / generateAll()):
//     this fork generates its node graph from TravelNodeMap::LoadOrGenerate(), called at
//     mod_playerbots.cpp:105, so calling AC's tail would generate the graph twice.
//
// ALIASING: AC stores `&point` of a loop-local in three places (:2072-2076, :2284-2295, :2334-2363).
// Every one of them goes through OwnedPoint() here, so each destination owns heap copies that outlive
// the loop; the local itself is only read (and, in the rpg/grind/boss loop, only for its isOverworld()
// test). AC's rpg/grind/boss loop additionally reuses ONE `point` across three destinations - the
// aliasing is not merely stale, three live destinations share one address.
//
// WIRING DECISION (#268 trap 4): this function is deliberately NOT called from anywhere in this fork.
//   AC never calls it either - TravelMgr::Init (AC :4361) does not, and a whole-module search plus
//   the public mod-playerbots master confirm no other caller - so upstream these caches are never
//   populated either. Porting it 1:1 and wiring it at boot would therefore be a NEW behaviour, not
//   parity, and it is the wrong slice for it:
//     * The only reader of questGivers/quests/rpgNpcs/grindMobs/bossMobs/exploreLocs in the whole
//       module is ChooseTravelTargetAction (getBossTravelDestinations, AC
//       Ai/Base/Actions/ChooseTravelTargetAction.cpp:781) plus the four accessors S2 added. #269 owns
//       ChooseTravelTargetAction.cpp and has not landed, so nothing in the fork reads these caches
//       yet - today they would be built and never touched.
//     * The cost is not trivial: the population walks every creature spawn on the server, then builds
//       a QuestRelationTravelDestination per questgiver/questtaker spawn, one per objective spawn and
//       one WorldPosition heap object per point. Paid at every boot for data no code reads.
//     * Boot ordering is not free either: the quest half needs sSharedValueContext's "quest guidp map"
//       (SharedValueContext.h:49 builds a throwaway PlayerbotAI per lookup, so it is safe with no bots
//       online), but Clear() takes a shared lock on the player map, and sPlayerbotAIConfig must
//       already be loaded.
//   #269 is the right place to wire it - the exact call is `sTravelMgr.LoadQuestTravelTable();` next to
//   `sTravelMgr.Init()` at mod_playerbots.cpp:97, once the action that consumes it exists.
// ================================================================================================

TravelDestination::~TravelDestination()
{
    for (WorldPosition* point : points)
        delete point;
}

void TravelMgr::LoadQuestTravelTable()
{
    if (!sTravelMgr.quests.empty())
        return;

    // Clearing store (for reloading case)
    Clear();

    struct unit
    {
        uint32 type;
        uint32 entry;
        uint32 map;
        float x;
        float y;
        float z;
        float o;
        uint32 c;
    } t_unit;
    std::vector<unit> units;

    // AC :1874-1881. questIds/questMap only feed the commented-out relation/loot block
    // (AC :2091-2248) and its own ">> Loaded N quest details." log, neither of which S3 ports - they
    // are kept verbatim from AC so a later slice can un-comment that block unchanged.
    ObjectMgr::QuestMap const& questMap = sObjectMgr->GetQuestTemplates();
    std::vector<uint32> questIds;
    std::unordered_map<uint32, uint32> entryCount;

    for (auto& quest : questMap)
        questIds.push_back(quest.first);

    std::sort(questIds.begin(), questIds.end());

    TC_LOG_INFO("playerbots", "Loading units locations.");
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        CreatureData const& creatureData = itr.second;

        t_unit.type = 0;
        t_unit.entry = creatureData.id;
        t_unit.map = creatureData.mapId;
        t_unit.x = creatureData.posX;
        t_unit.y = creatureData.posY;
        t_unit.z = creatureData.posZ;
        t_unit.o = creatureData.orientation;

        entryCount[creatureData.id]++;

        units.push_back(t_unit);
    }

    for (auto& unit : units)
    {
        unit.c = entryCount[unit.entry];
    }

    // AC :1904-1917 ("Loading game object locations.", the getGameObjectsNear() loop) is NOT ported -
    // see the header block above: the fork has no whole-gameobject-spawn-store accessor, and every
    // live consumer of `units` below skips non-creature units anyway.

    TC_LOG_INFO("playerbots", "Loading quest data.");

    bool loadQuestData = true;
    if (loadQuestData)
    {
        questGuidpMap const& questGuidpMapValue =
            sSharedValueContext->getGlobalValue<questGuidpMap>("quest guidp map")->Get();

        for (auto& q : questGuidpMapValue)
        {
            uint32 questId = q.first;

            QuestContainer* container = new QuestContainer;

            for (auto& r : q.second)
            {
                uint32 flag = r.first;

                for (auto& e : r.second)
                {
                    int32 entry = e.first;

                    QuestTravelDestination* loc;
                    std::vector<QuestTravelDestination*> locs;

                    if (flag & (uint32)QuestRelationFlag::questGiver)
                    {
                        loc = new QuestRelationTravelDestination(
                            questId, entry, 0, sPlayerbotAIConfig->tooCloseDistance,
                            sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questGivers.push_back(loc);
                        locs.push_back(loc);
                    }
                    if (flag & (uint32)QuestRelationFlag::questTaker)
                    {
                        loc = new QuestRelationTravelDestination(
                            questId, entry, 1, sPlayerbotAIConfig->tooCloseDistance,
                            sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(5 * 60 * 1000);
                        loc->setMaxVisitors(15, 0);
                        container->questTakers.push_back(loc);
                        locs.push_back(loc);
                    }
                    else
                    {
                        uint32 objective = 0;
                        if (flag & (uint32)QuestRelationFlag::objective1)
                            objective = 0;
                        else if (flag & (uint32)QuestRelationFlag::objective2)
                            objective = 1;
                        else if (flag & (uint32)QuestRelationFlag::objective3)
                            objective = 2;
                        else if (flag & (uint32)QuestRelationFlag::objective4)
                            objective = 3;

                        loc = new QuestObjectiveTravelDestination(questId, entry, objective,
                                                                  sPlayerbotAIConfig->tooCloseDistance,
                                                                  sPlayerbotAIConfig->sightDistance);
                        loc->setExpireDelay(1 * 60 * 1000);
                        loc->setMaxVisitors(100, 1);
                        container->questObjectives.push_back(loc);
                        locs.push_back(loc);
                    }

                    for (auto& guidP : e.second)
                    {
                        WorldPosition point = guidP;
                        for (auto tLoc : locs)
                        {
                            tLoc->addPoint(OwnedPoint(point));
                        }
                    }
                }
            }

            if (!container->questTakers.empty())
            {
                quests.insert(std::make_pair(questId, container));

                for (auto loc : container->questGivers)
                    questGivers.push_back(loc);
            }
        }
    }

    // AC :2091-2248 - the whole `if (loadQuestData && false)` relation/objective/loot block that
    // reads `units`, `relations` and `loots`. It is commented out upstream, depends on
    // `relations`/`loots` (declared only inside that same comment, AC :1857-1872) and on the raw SQL
    // queries in the comment at AC :1919-2005, so it is not ported.

    WorldPosition point;

    TC_LOG_INFO("playerbots", "Loading Rpg, Grind and Boss locations.");

    // Rpg locations
    for (auto& u : units)
    {
        RpgTravelDestination* rLoc;
        GrindTravelDestination* gLoc;
        BossTravelDestination* bLoc;

        if (u.type != 0)
            continue;

        CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
        if (!cInfo)
            continue;

        std::vector<uint32> allowedNpcFlags;

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_GOSSIP);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_BANKER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_AUCTIONEER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_STABLEMASTER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_PETITIONER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TABARDDESIGNER);

        allowedNpcFlags.push_back(UNIT_NPC_FLAG_TRAINER);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_VENDOR);
        allowedNpcFlags.push_back(UNIT_NPC_FLAG_REPAIR);

        point = WorldPosition(u.map, u.x, u.y, u.z, u.o);

        for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
        {
            if ((cInfo->npcflag & *i) != 0)
            {
                rLoc = new RpgTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                                sPlayerbotAIConfig->sightDistance);
                rLoc->setExpireDelay(5 * 60 * 1000);
                rLoc->setMaxVisitors(15, 0);

                rLoc->addPoint(OwnedPoint(point));
                rpgNpcs.push_back(rLoc);
                break;
            }
        }

        if (cInfo->mingold > 0)
        {
            gLoc = new GrindTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                              sPlayerbotAIConfig->sightDistance);
            gLoc->setExpireDelay(5 * 60 * 1000);
            gLoc->setMaxVisitors(100, 0);

            gLoc->addPoint(OwnedPoint(point));
            grindMobs.push_back(gLoc);
        }

        if (cInfo->rank == 3 || (cInfo->rank == 1 && !point.isOverworld() && u.c == 1))
        {
            std::string const nodeName = cInfo->Name;

            bLoc = new BossTravelDestination(u.entry, sPlayerbotAIConfig->tooCloseDistance,
                                             sPlayerbotAIConfig->sightDistance);
            bLoc->setExpireDelay(5 * 60 * 1000);
            bLoc->setMaxVisitors(0, 0);

            bLoc->addPoint(OwnedPoint(point));
            bossMobs.push_back(bLoc);
        }
    }

    TC_LOG_INFO("playerbots", "Loading Explore locations.");

    // Explore points
    for (auto& u : units)
    {
        ExploreTravelDestination* loc;

        WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
        AreaTableEntry const* area = point.getArea();

        if (!area)
            continue;

        if (!area->exploreFlag)
            continue;

        if (u.type == 1)
            continue;

        auto iloc = exploreLocs.find(area->ID);

        if (iloc == exploreLocs.end())
        {
            loc = new ExploreTravelDestination(area->ID, sPlayerbotAIConfig->tooCloseDistance,
                                               sPlayerbotAIConfig->sightDistance);
            loc->setMaxVisitors(1000, 0);
            loc->setCooldownDelay(1000);
            loc->setExpireDelay(1000);
            loc->setTitle(area->area_name);
            exploreLocs.insert_or_assign(area->ID, loc);
        }
        else
        {
            loc = iloc->second;
        }

        loc->addPoint(OwnedPoint(point));
    }

    // Clear these logs files
    sPlayerbotAIConfig->openLog("zones.csv", "w");
    sPlayerbotAIConfig->openLog("creatures.csv", "w");
    sPlayerbotAIConfig->openLog("gos.csv", "w");
    sPlayerbotAIConfig->openLog("bot_movement.csv", "w");
    sPlayerbotAIConfig->openLog("bot_pathfinding.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_attempt.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_attempt_point.csv", "w");
    sPlayerbotAIConfig->openLog("pathfind_result.csv", "w");
    sPlayerbotAIConfig->openLog("load_map_grid.csv", "w");
    sPlayerbotAIConfig->openLog("strategy.csv", "w");

    sPlayerbotAIConfig->openLog("unload_grid.csv", "w");
    sPlayerbotAIConfig->openLog("unload_obj.csv", "w");

    // AC :2381-2383 - commented out, NOT called here (S3 dropped them, S8 restores them as text only).
    // This fork generates its node graph from TravelNodeMap::LoadOrGenerate() (mod_playerbots.cpp:105),
    // so calling AC's tail would build the graph twice.
    // TravelNodeMap::instance().loadNodeStore();
    //
    // TravelNodeMap::instance().generateAll();

    // #268 S8: AC TravelMgr.cpp:2385-3723 - the rest of AC's LoadQuestTravelTable body. NOT ported live.
    // Correction to the S8 brief: that range is NOT one /* */ block. In AC it is three comment blocks
    // (:2385-2727, :2729-2909, :3665-3723) around ~750 LIVE lines (:2911-3663: printMap/printNodeStore/
    // saveNodeStore, the creatures.csv / gos.csv / zones / vmangoslines / strategy.csv exports). The
    // whole range is kept verbatim inside #if 0 (AC's own /* */ pairs are not nestable inside a /* */).
    // The live-in-AC part is disabled here because it needs APIs this fork lacks: WorldPosition::
    // getCreaturesNear()/getGameObjectsNear() with no argument, cData->mapid (5.4.8: mapId), and
    // TravelNodeMap::printMap/printNodeStore/saveNodeStore/calcMapOffset; the function has no caller on
    // either tree, so nothing changes at runtime.
#if 0
    /*
    bool fullNavPointReload = false;
    bool storeNavPointReload = true;

   if (!fullNavPointReload && true)
        TravelNodeStore::loadNodes();

    //TravelNodeMap::instance().loadNodeStore();

    for (auto node : TravelNodeMap::instance().getNodes())
    {
        node->setLinked(true);
    }

    bool reloadNavigationPoints = false || fullNavPointReload || storeNavPointReload;

    if (reloadNavigationPoints)
    {
        LOG_INFO("playerbots", "Loading navigation points");

        //Npc nodes

        WorldPosition pos;

        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            std::vector<uint32> allowedNpcFlags;

            allowedNpcFlags.push_back(UNIT_NPC_FLAG_INNKEEPER);
            allowedNpcFlags.push_back(UNIT_NPC_FLAG_FLIGHTMASTER);
            //allowedNpcFlags.push_back(UNIT_NPC_FLAG_QUESTGIVER);

            for (std::vector<uint32>::iterator i = allowedNpcFlags.begin(); i != allowedNpcFlags.end(); ++i)
            {
                if ((cInfo->npcflag & *i) != 0)
                {
                    pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

                    std::string const nodeName = pos.getAreaName(false);
                    if ((cInfo->npcflag & UNIT_NPC_FLAG_INNKEEPER) != 0)
                        nodeName += " innkeeper";
                    else
                        nodeName += " flightMaster";

                    TravelNodeMap::instance().addNode(&pos, nodeName, true, true);

                    break;
                }
            }
        }

        //Build flight paths
        for (uint32 i = 0; i < sTaxiPathStore.GetNumRows(); ++i)
        {
            TaxiPathEntry const* taxiPath = sTaxiPathStore.LookupEntry(i);

            if (!taxiPath)
                continue;

            TaxiNodesEntry const* startTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->from);
            if (!startTaxiNode)
                continue;

            TaxiNodesEntry const* endTaxiNode = sTaxiNodesStore.LookupEntry(taxiPath->to);
            if (!endTaxiNode)
                continue;

            TaxiPathNodeList const& nodes = sTaxiPathNodesByPath[taxiPath->ID];
            if (nodes.empty())
                continue;

            WorldPosition startPos(startTaxiNode->map_id, startTaxiNode->x, startTaxiNode->y, startTaxiNode->z);
            WorldPosition endPos(endTaxiNode->map_id, endTaxiNode->x, endTaxiNode->y, endTaxiNode->z);

            TravelNode* startNode = TravelNodeMap::instance().getNode(&startPos, nullptr, 15.0f);
            TravelNode* endNode = TravelNodeMap::instance().getNode(&endPos, nullptr, 15.0f);

            if (!startNode || !endNode)
                continue;

            std::vector<WorldPosition> ppath;

            for (auto& n : nodes)
                ppath.push_back(WorldPosition(n->mapid, n->x, n->y, n->z, 0.0));

            float totalTime = startPos.getPathLength(ppath) / (450 * 8.0f);

            TravelNodePath travelPath(0.1f, totalTime, (uint8) TravelNodePathType::flightPath, i, true);
            travelPath.setPath(ppath);

            startNode->setPathTo(endNode, travelPath);
        }

        //Unique bosses
        for (auto& u : units)
        {
            if (u.type != 0)
                continue;

            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(u.entry);
            if (!cInfo)
                continue;

            pos = WorldPosition(u.map, u.x, u.y, u.z, u.o);

            if (cInfo->rank == 3 || (cInfo->rank == 1 && !pos.isOverworld() && u.c == 1))
            {
                std::string const nodeName = cInfo->Name;
                TravelNodeMap::instance().addNode(&pos, nodeName, true, true);
            }
        }

        std::map<uint8, std::string> startNames;
        startNames[RACE_HUMAN] = "Human";
        startNames[RACE_ORC] = "Orc and Troll";
        startNames[RACE_DWARF] = "Dwarf and Gnome";
        startNames[RACE_NIGHTELF] = "Night Elf";
        startNames[RACE_UNDEAD_PLAYER] = "Undead";
        startNames[RACE_TAUREN] = "Tauren";
        startNames[RACE_GNOME] = "Dwarf and Gnome";
        startNames[RACE_TROLL] = "Orc and Troll";
        startNames[RACE_DRAENEI] = "Draenei";
        startNames[RACE_BLOODELF] = "Blood Elf";

        for (uint32 i = 0; i < MAX_RACES; i++)
        {
            for (uint32 j = 0; j < MAX_CLASSES; j++)
            {
                PlayerInfo const* info = sObjectMgr->GetPlayerInfo(i, j);
                if (!info)
                    continue;

                pos = WorldPosition(info->mapId, info->positionX, info->positionY, info->positionZ, info->orientation);

                std::string const nodeName = startNames[i] + " start";
                TravelNodeMap::instance().addNode(&pos, nodeName, true, true);
            }
        }

        //Transports
        GameObjectTemplateContainer const* goTemplates = sObjectMgr->GetGameObjectTemplates();
        for (auto const& iter : *goTemplates)
        {
            GameObjectTemplate const* data = &iter.second;
            if (data && (data->type == GAMEOBJECT_TYPE_TRANSPORT || data->type == GAMEOBJECT_TYPE_MO_TRANSPORT))
            {
                TransportAnimation const* animation = sTransportMgr->GetTransportAnimInfo(iter.first);

                uint32 pathId = data->moTransport.taxiPathId;
                float moveSpeed = data->moTransport.moveSpeed;
                if (pathId >= sTaxiPathNodesByPath.size())
                    continue;

                TaxiPathNodeList const& path = sTaxiPathNodesByPath[pathId];

                std::vector<WorldPosition> ppath;
                TravelNode* prevNode = nullptr;

                //Elevators/Trams
                if (path.empty())
                {
                    if (animation)
                    {
                        TransportPathContainer aPath = animation->Path;
                        float timeStart;

                        for (auto& u : units)
                        {
                            if (u.type != 1)
                                continue;

                            if (u.entry != iter.first)
                                continue;

                            prevNode = nullptr;
                            WorldPosition lPos = WorldPosition(u.map, 0, 0, 0, 0);

                            for (auto& p : aPath)
                            {
                                float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z, u.o);

                                if (prevNode)
                                {
                                    ppath.push_back(pos);
                                }

                                if (pos.distance(&lPos) == 0)
                                {
                                    TravelNode* node = TravelNodeMap::instance().addNode(&pos, data->name, true, true, true,
   iter.first);

                                    if (!prevNode)
                                    {
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }
                                    else
                                    {
                                        float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                        TravelNodePath travelPath(0.1f, totalTime, (uint8)
   TravelNodePathType::transport, entry, true); node->setPathTo(prevNode, travelPath); ppath.clear();
                                        ppath.push_back(pos);
                                        timeStart = p.second->TimeSeg;
                                    }

                                    prevNode = node;
                                }

                                lPos = pos;
                            }

                            if (prevNode)
                            {
                                for (auto& p : aPath)
                                {
                                    float dx = cos(u.o) * p.second->X - sin(u.o) * p.second->Y;
                                    float dy = sin(u.o) * p.second->X + cos(u.o) * p.second->Y;
                                    WorldPosition pos = WorldPosition(u.map, u.x + dx, u.y + dy, u.z + p.second->Z,
   u.o);

                                    ppath.push_back(pos);

                                    if (pos.distance(&lPos) == 0)
                                    {
                                        TravelNode* node = TravelNodeMap::instance().addNode(&pos, data->name, true, true, true,
   iter.first); if (node != prevNode)
                                        {
                                            float totalTime = (p.second->TimeSeg - timeStart) / 1000.0f;
                                            TravelNodePath travelPath(0.1f, totalTime, (uint8)
   TravelNodePathType::transport, entry, true); travelPath.setPath(ppath); node->setPathTo(prevNode, travelPath);
                                            ppath.clear();
                                            ppath.push_back(pos);
                                            timeStart = p.second->TimeSeg;
                                        }
                                    }

                                    lPos = pos;
                                }
                            }

                            ppath.clear();
                        }
                    }
                }
                else //Boats/Zepelins
                {
                    //Loop over the path and connect stop locations.
                    for (auto& p : path)
                    {
                        WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                        //if (data->displayId == 3015)
                        //    pos.setZ(pos.getZ() + 6.0f);
                        //else if (data->displayId == 3031)
                       //     pos.setZ(pos.getZ() - 17.0f);

                        if (prevNode)
                        {
                            ppath.push_back(pos);
                        }

                        if (p->delay > 0)
                        {
                            TravelNode* node = TravelNodeMap::instance().addNode(&pos, data->name, true, true, true, iter.first);

                            if (!prevNode)
                            {
                                ppath.push_back(pos);
                            }
                            else
                            {
                                TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry,
   true); travelPath.setPathAndCost(ppath, moveSpeed); node->setPathTo(prevNode, travelPath); ppath.clear();
                                ppath.push_back(pos);
                            }

                            prevNode = node;
                        }
                    }

                    if (prevNode)
                    {
                        //Continue from start until first stop and connect to end.
                        for (auto& p : path)
                        {
                            WorldPosition pos = WorldPosition(p->mapid, p->x, p->y, p->z, 0);

                            //if (data->displayId == 3015)
                            //    pos.setZ(pos.getZ() + 6.0f);
                            //else if (data->displayId == 3031)
                            //    pos.setZ(pos.getZ() - 17.0f);

                            ppath.push_back(pos);

                            if (p->delay > 0)
                            {
                                TravelNode* node = TravelNodeMap::instance().getNode(&pos, nullptr, 5.0f);
                                if (node != prevNode)
                                {
                                    TravelNodePath travelPath(0.1f, 0.0, (uint8) TravelNodePathType::transport, entry,
   true); travelPath.setPathAndCost(ppath, moveSpeed); node->setPathTo(prevNode, travelPath);
                                }
                            }
                        }
                    }

                    ppath.clear();
                }
            }
        }

        //Zone means
        for (auto& loc : exploreLocs)
        {
            std::vector<WorldPosition*> points;

            for (auto p : loc.second->getPoints(true))
                if (!p->isUnderWater())
                    points.push_back(p);

            if (points.empty())
                points = loc.second->getPoints(true);

            WorldPosition  pos = WorldPosition(points, WP_MEAN_CENTROID);

            TravelNode* node = TravelNodeMap::instance().addNode(&pos, pos.getAreaName(), true, true, false);
        }

        LOG_INFO("playerbots", ">> Loaded {} navigation points.", TravelNodeMap::instance().getNodes().size());
    }

    TravelNodeMap::instance().calcMapOffset();
    loadMapTransfers();
    */

    /*
    bool preloadNodePaths = false || fullNavPointReload || storeNavPointReload;             //Calculate paths using
    PathGenerator. bool preloadReLinkFullyLinked = false || fullNavPointReload || storeNavPointReload;      //Retry
    nodes that are fully linked. bool preloadUnlinkedPaths = false || fullNavPointReload;        //Try to connect points
    currently unlinked. bool preloadWorldPaths = true;            //Try to load paths in overworld. bool
    preloadInstancePaths = true;         //Try to load paths in instances. bool preloadSubPrint = false; //Print output
    every 2%.

    if (preloadNodePaths)
    {
        std::unordered_map<uint32, Map*> instances;

        //PathGenerator
        std::vector<WorldPosition> ppath;

        uint32 cur = 0, max = TravelNodeMap::instance().getNodes().size();

        for (auto& startNode : TravelNodeMap::instance().getNodes())
        {
            if (!preloadReLinkFullyLinked && startNode->isLinked())
                continue;

            for (auto& endNode : TravelNodeMap::instance().getNodes())
            {
                if (startNode == endNode)
                    continue;

                if (startNode->getPosition()->isOverworld() && !preloadWorldPaths)
                    continue;

                if (!startNode->getPosition()->isOverworld() && !preloadInstancePaths)
                    continue;

                if (startNode->hasCompletePathTo(endNode))
                    continue;

                if (!preloadUnlinkedPaths && !startNode->hasLinkTo(endNode))
                    continue;

                if (startNode->getMapId() != endNode->getMapId())
                    continue;

                //if (preloadUnlinkedPaths && !startNode->hasLinkTo(endNode) && startNode->isUselessLink(endNode))
                //    continue;

                startNode->buildPath(endNode, nullptr, false);

                //if (startNode->hasLinkTo(endNode) && !startNode->getPathTo(endNode)->getComplete())
                    //startNode->removeLinkTo(endNode);
            }

            startNode->setLinked(true);

            cur++;

            if (preloadSubPrint && (cur * 50) / max > ((cur - 1) * 50) / max)
            {
                TravelNodeMap::instance().printMap();
                TravelNodeMap::instance().printNodeStore();
            }
        }

        if (!preloadSubPrint)
        {
            TravelNodeMap::instance().printNodeStore();
            TravelNodeMap::instance().printMap();
        }

        LOG_INFO("playerbots", ">> Loaded paths for {} nodes.", TravelNodeMap::instance().getNodes().size());
    }

    bool removeLowLinkNodes = false || fullNavPointReload || storeNavPointReload;

    if (removeLowLinkNodes)
    {
        std::vector<TravelNode*> goodNodes;
        std::vector<TravelNode*> remNodes;
        for (auto& node : TravelNodeMap::instance().getNodes())
        {
            if (!node->getPosition()->isOverworld())
                continue;

            if (std::find(goodNodes.begin(), goodNodes.end(), node) != goodNodes.end())
                continue;

            if (std::find(remNodes.begin(), remNodes.end(), node) != remNodes.end())
                continue;

            std::vector<TravelNode*> nodes = node->getNodeMap(true);

            if (nodes.size() < 5)
                remNodes.insert(remNodes.end(), nodes.begin(), nodes.end());
            else
                goodNodes.insert(goodNodes.end(), nodes.begin(), nodes.end());
        }

        for (auto& node : remNodes)
            TravelNodeMap::instance().removeNode(node);

        LOG_INFO("playerbots", ">> Checked {} nodes.", TravelNodeMap::instance().getNodes().size());
    }

    bool cleanUpNodeLinks = false || fullNavPointReload || storeNavPointReload;
    bool cleanUpSubPrint = false;             //Print output every 2%.

    if (cleanUpNodeLinks)
    {
        //Routes
        uint32 cur = 0;
        uint32 max = TravelNodeMap::instance().getNodes().size();

        //Clean up node links
        for (auto& startNode : TravelNodeMap::instance().getNodes())
        {
             startNode->cropUselessLinks();

             cur++;
             if (cleanUpSubPrint && (cur * 10) / max > ((cur - 1) * 10) / max)
             {
                 TravelNodeMap::instance().printMap();
                 TravelNodeMap::instance().printNodeStore();
             }
        }

        LOG_INFO("playerbots", ">> Cleaned paths for {} nodes.", TravelNodeMap::instance().getNodes().size());
    }

    bool reCalculateCost = false || fullNavPointReload || storeNavPointReload;
    bool forceReCalculate = false;

    if (reCalculateCost)
    {
        for (auto& startNode : TravelNodeMap::instance().getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNodePath* nodePath = path.second;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                if (nodePath->getCalculated() && !forceReCalculate)
                    continue;

                nodePath->calculateCost();
            }
        }

        LOG_INFO("playerbots", ">> Calculated pathcost for {} nodes.", TravelNodeMap::instance().getNodes().size());
    }

    bool mirrorMissingPaths = true || fullNavPointReload || storeNavPointReload;

    if (mirrorMissingPaths)
    {
        for (auto& startNode : TravelNodeMap::instance().getNodes())
        {
            for (auto& path : *startNode->getLinks())
            {
                TravelNode* endNode = path.first;

                if (endNode->hasLinkTo(startNode))
                    continue;

                if (path.second->getPathType() != TravelNodePathType::walk)
                    continue;

                TravelNodePath nodePath = *path.second;

                std::vector<WorldPosition> pPath = nodePath.getPath();
                std::reverse(pPath.begin(), pPath.end());

                nodePath.setPath(pPath);

                endNode->setPathTo(startNode, nodePath, true);
            }
        }

        LOG_INFO("playerbots", ">> Reversed missing paths for {} nodes.", TravelNodeMap::instance().getNodes().size());
    }
    */
    // #611: from here AC's LIVE-in-AC code (AC :2911-3663) is compiled again; only the 3.3.5-only strategy.csv matrix below stays in
    // #if 0. Adaptations: sPlayerbotAIConfig. -> sPlayerbotAIConfig->; CreatureData::mapid -> mapId (5.4.8). The members it needed are
    // WorldPosition::getCreaturesNear/getGameObjectsNear (no-arg), WorldPosition::split/IsValid (WorldPosition.h) and
    // TravelNodeMap::printMap/printNodeStore/saveNodeStore/calcMapOffset (TravelNode.h:594-604, ported since). LoadQuestTravelTable has
    // no caller on either tree, so nothing runs; this is parity of the code, kept as AC has it.
#endif

    TravelNodeMap::instance().printMap();
    TravelNodeMap::instance().printNodeStore();
    TravelNodeMap::instance().saveNodeStore();

    // Creature/gos/zone export.
    if (sPlayerbotAIConfig->hasLog("creatures.csv"))
    {
        for (CreatureData const* cData : WorldPosition().getCreaturesNear())
        {
            CreatureTemplate const* cInfo = sObjectMgr->GetCreatureTemplate(cData->id);
            if (!cInfo)
                continue;

            WorldPosition point =
                WorldPosition(cData->mapId, cData->posX, cData->posY, cData->posZ, cData->orientation);

            std::string name = cInfo->Name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << cInfo->maxlevel << ",";
            out << cInfo->rank << ",";
            out << cInfo->faction << ",";
            out << cInfo->npcflag << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("creatures.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig->hasLog("vmangoslines.csv"))
    {
        uint32 mapId = 0;
        std::vector<WorldPosition> pos;

        static float const topNorthSouthLimit[] = {
            2032.048340f,  -6927.750000f, 1634.863403f,  -6157.505371f, 1109.519775f,  -5181.036133f, 1315.204712f,
            -4096.020508f, 1073.089233f,  -3372.571533f, 825.8331910f,  -3125.778809f, 657.3439940f,  -2314.813232f,
            424.7361450f,  -1888.283691f, 744.3958130f,  -1647.935425f, 1424.160645f,  -654.9481810f, 1447.065308f,
            -169.7513580f, 1208.715454f,  189.74870300f, 1596.240356f,  998.61669900f, 1577.923706f,  1293.4199220f,
            1458.520264f,  1727.3732910f, 1591.916138f,  3728.1394040f};

        pos.clear();

#define my_sizeof(type) ((char*)(&type + 1) - (char*)(&type))

        uint32 size = my_sizeof(topNorthSouthLimit) / my_sizeof(topNorthSouthLimit[0]);
        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (topNorthSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, topNorthSouthLimit[i], topNorthSouthLimit[i + 1], 0));
        }

        std::ostringstream out;
        out << "topNorthSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const ironforgeAreaSouthLimit[] = {
            -7491.33f, 3093.740f, -7472.04f, -391.880f, -6366.68f, -730.100f, -6063.96f, -1411.76f,
            -6087.62f, -2190.21f, -6349.54f, -2533.66f, -6308.63f, -3049.32f, -6107.82f, -3345.30f,
            -6008.49f, -3590.52f, -5989.37f, -4312.29f, -5806.26f, -5864.11f};

        pos.clear();

        size = my_sizeof(ironforgeAreaSouthLimit) / my_sizeof(ironforgeAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (ironforgeAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, ironforgeAreaSouthLimit[i], ironforgeAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "ironforgeAreaSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaNorthLimit[] = {
            -8004.250f, 3714.110f, -8075.000f, -179.000f, -8638.000f, 169.0000f, -9044.000f, 35.00000f,
            -9068.000f, -125.000f, -9094.000f, -147.000f, -9206.000f, -290.000f, -9097.000f, -510.000f,
            -8739.000f, -501.000f, -8725.500f, -1618.45f, -9810.400f, -1698.41f, -10049.60f, -1740.40f,
            -10670.61f, -1692.51f, -10908.48f, -1563.87f, -13006.40f, -1622.80f, -12863.23f, -4798.42f};

        pos.clear();

        size = my_sizeof(stormwindAreaNorthLimit) / my_sizeof(stormwindAreaNorthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaNorthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaNorthLimit[i], stormwindAreaNorthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaNorthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const stormwindAreaSouthLimit[] = {
            -8725.3378910f, 3535.62402300f, -9525.6992190f, 910.13256800f, -9796.9531250f, 839.06958000f,
            -9946.3417970f, 743.10284400f,  -10287.361328f, 760.07647700f, -10083.828125f, 380.38989300f,
            -10148.072266f, 80.056450000f,  -10014.583984f, -161.6385190f, -9978.1464840f, -361.6380310f,
            -9877.4892580f, -563.3048710f,  -9980.9677730f, -1128.510498f, -9991.7177730f, -1428.793213f,
            -9887.5791020f, -1618.514038f,  -10169.600586f, -1801.582031f, -9966.2744140f, -2227.197754f,
            -9861.3095700f, -2989.841064f,  -9944.0263670f, -3205.886963f, -9610.2099610f, -3648.369385f,
            -7949.3295900f, -4081.389404f,  -7910.8593750f, -5855.578125f};

        pos.clear();

        size = my_sizeof(stormwindAreaSouthLimit) / my_sizeof(stormwindAreaSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (stormwindAreaSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, stormwindAreaSouthLimit[i], stormwindAreaSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "stormwindAreaSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        mapId = 1;

        static float const northMiddleLimit[] = {
            -2280.00f, 4054.000f, -2401.00f, 2365.000f, -2432.00f, 1338.000f, -2286.00f, 769.0000f, -2137.00f,
            662.0000f, -2044.54f, 489.8600f, -1808.52f, 436.3900f, -1754.85f, 504.5500f, -1094.55f, 651.7500f,
            -747.460f, 647.7300f, -685.550f, 408.4300f, -311.380f, 114.4300f, -358.400f, -587.420f, -377.920f,
            -748.700f, -512.570f, -919.490f, -280.650f, -1008.87f, -81.2900f, -930.890f, 284.3100f, -1105.39f,
            568.8600f, -892.280f, 1211.090f, -1135.55f, 879.6000f, -2110.18f, 788.9600f, -2276.02f, 899.6800f,
            -2625.56f, 1281.540f, -2689.42f, 1521.820f, -3047.85f, 1424.220f, -3365.69f, 1694.110f, -3615.20f,
            2373.780f, -4019.96f, 2388.130f, -5124.35f, 2193.790f, -5484.38f, 1703.570f, -5510.53f, 1497.590f,
            -6376.56f, 1368.000f, -8530.00f};

        pos.clear();

        size = my_sizeof(northMiddleLimit) / my_sizeof(northMiddleLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (northMiddleLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, northMiddleLimit[i], northMiddleLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "northMiddleLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const durotarSouthLimit[] = {
            2755.0f, -3766.f, 2225.0f, -3596.f, 1762.0f, -3746.f, 1564.0f, -3943.f, 1184.0f, -3915.f, 737.00f,
            -3782.f, -75.00f, -3742.f, -263.0f, -3836.f, -173.0f, -4064.f, -81.00f, -4091.f, -49.00f, -4089.f,
            -16.00f, -4187.f, -5.000f, -4192.f, -14.00f, -4551.f, -397.0f, -4601.f, -522.0f, -4583.f, -668.0f,
            -4539.f, -790.0f, -4502.f, -1176.f, -4213.f, -1387.f, -4674.f, -2243.f, -6046.f};

        pos.clear();

        size = my_sizeof(durotarSouthLimit) / my_sizeof(durotarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (durotarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, durotarSouthLimit[i], durotarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "durotarSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const valleyoftrialsSouthLimit[] = {-324.f,  -3869.f, -774.f,  -3992.f, -965.f,  -4290.f, -932.f,
                                                         -4349.f, -828.f,  -4414.f, -661.f,  -4541.f, -521.f,  -4582.f};

        pos.clear();

        size = my_sizeof(valleyoftrialsSouthLimit) / my_sizeof(valleyoftrialsSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (valleyoftrialsSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, valleyoftrialsSouthLimit[i], valleyoftrialsSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "valleyoftrialsSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const middleToSouthLimit[] = {
            -2402.010000f, 4255.7000000f, -2475.933105f, 3199.5683590f,  // Desolace
            -2344.124023f, 1756.1643070f, -2826.438965f, 403.82473800f,  // Mulgore
            -3472.819580f, 182.52247600f,                                // Feralas
            -4365.006836f, -1602.575439f,                                // the Barrens
            -4515.219727f, -1681.356079f, -4543.093750f, -1882.869385f,  // Thousand Needles
            -4824.160000f, -2310.110000f, -5102.913574f, -2647.062744f, -5248.286621f,
            -3034.536377f, -5246.920898f, -3339.139893f, -5459.449707f, -4920.155273f,  // Tanaris
            -5437.000000f, -5863.000000f};

        pos.clear();

        size = my_sizeof(middleToSouthLimit) / my_sizeof(middleToSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (middleToSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, middleToSouthLimit[i], middleToSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "middleToSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const orgrimmarSouthLimit[] = {
            2132.5076f, -3912.2478f, 1944.4298f, -3855.2583f, 1735.6906f, -3834.2417f, 1654.3671f, -3380.9902f,
            1593.9861f, -3975.5413f, 1439.2548f, -4249.6923f, 1436.3106f, -4007.8950f, 1393.3199f, -4196.0625f,
            1445.2428f, -4373.9052f, 1407.2349f, -4429.4145f, 1464.7142f, -4545.2875f, 1584.1331f, -4596.8764f,
            1716.8065f, -4601.1323f, 1875.8312f, -4788.7187f, 1979.7647f, -4883.4585f, 2219.1562f, -4854.3330f};

        pos.clear();

        size = my_sizeof(orgrimmarSouthLimit) / my_sizeof(orgrimmarSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (orgrimmarSouthLimit[i] == 0)
                break;

            pos.push_back(WorldPosition(mapId, orgrimmarSouthLimit[i], orgrimmarSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "orgrimmarSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());

        static float const feralasThousandNeedlesSouthLimit[] = {
            -6495.4995f, -4711.9810f, -6674.9995f, -4515.0019f, -6769.5717f, -4122.4272f, -6838.2651f, -3874.2792f,
            -6851.1314f, -3659.1179f, -6624.6845f, -3063.3843f, -6416.9067f, -2570.1301f, -5959.8466f, -2287.2634f,
            -5947.9135f, -1866.5028f, -5947.9135f, -820.48810f, -5876.7114f, -3.5138000f, -5876.7114f, 917.640700f,
            -6099.3603f, 1153.28840f, -6021.8989f, 1638.18090f, -6091.6176f, 2335.88920f, -6744.9946f, 2393.48550f,
            -6973.8608f, 3077.02810f, -7068.7241f, 4376.23040f, -7142.1211f, 4808.43310f};

        pos.clear();

        size = my_sizeof(feralasThousandNeedlesSouthLimit) / my_sizeof(feralasThousandNeedlesSouthLimit[0]);

        for (uint32 i = 0; i < size - 1; i = i + 2)
        {
            if (feralasThousandNeedlesSouthLimit[i] == 0)
                break;

            pos.push_back(
                WorldPosition(mapId, feralasThousandNeedlesSouthLimit[i], feralasThousandNeedlesSouthLimit[i + 1], 0));
        }

        out.str("");
        out.clear();

        out << "feralasThousandNeedlesSouthLimit"
            << ",";
        WorldPosition().printWKT(pos, out, 1);
        out << std::fixed;

        sPlayerbotAIConfig->log("vmangoslines.csv", out.str().c_str());
    }

    if (sPlayerbotAIConfig->hasLog("gos.csv"))
    {
        for (GameObjectData const* gData : WorldPosition().getGameObjectsNear())
        {
            GameObjectTemplate const* data = sObjectMgr->GetGameObjectTemplate(gData->id);
            if (!data)
                continue;

            WorldPosition point =
                WorldPosition(gData->mapid, gData->posX, gData->posY, gData->posZ, gData->orientation);

            std::string name = data->name;
            name.erase(remove(name.begin(), name.end(), ','), name.end());
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());

            std::ostringstream out;
            out << name << ",";
            point.printWKT(out);
            out << data->type << ",";
            out << point.getAreaName() << ",";
            out << std::fixed;

            sPlayerbotAIConfig->log("gos.csv", out.str().c_str());
        }
    }

    if (sPlayerbotAIConfig->hasLog("zones.csv"))
    {
        std::unordered_map<std::string, std::vector<WorldPosition>> zoneLocs;

        std::vector<WorldPosition> Locs;

        for (auto& u : units)
        {
            WorldPosition point = WorldPosition(u.map, u.x, u.y, u.z, u.o);
            std::string const name = std::to_string(u.map) + point.getAreaName();

            if (zoneLocs.find(name) == zoneLocs.end())
                zoneLocs.insert_or_assign(name, Locs);

            zoneLocs.find(name)->second.push_back(point);
        }

        for (auto& loc : zoneLocs)
        {
            if (loc.second.empty())
                continue;

            if (!TravelNodeMap::instance().getMapOffset(loc.second.front().GetMapId()) &&
                loc.second.front().GetMapId() != 0)
                continue;

            std::vector<WorldPosition> points = loc.second;
            ;

            std::ostringstream out;

            WorldPosition pos = WorldPosition(points, WP_MEAN_CENTROID);

            out << "\"center\""
                << ",";
            out << points.begin()->GetMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            pos.printWKT(out);

            out << "\n";

            out << "\"area\""
                << ",";
            out << points.begin()->GetMapId() << ",";
            out << points.begin()->getAreaName() << ",";
            out << points.begin()->getAreaName(true, true) << ",";

            point.printWKT(points, out, 0);

            sPlayerbotAIConfig->log("zones.csv", out.str().c_str());
        }
    }

    // #611: AC :3665-3723 onward is the strategy.csv matrix (guarded by `bool printStrategyMap = false`). It builds a Player with the 3.3.5
    // Player::Create(guid, name, race, class, gender, skin, face, hair, hairColor, facialHair, outfitId), walks sRaceMgr and the
    // strategy/trigger tables; none of that exists in this form on 5.4.8 and it can never run (the flag is a constant false), so it
    // stays disabled, verbatim.
#if 0
    bool printStrategyMap = false;

    if (printStrategyMap && sPlayerbotAIConfig.hasLog("strategy.csv"))
    {
        static std::map<uint8, std::string> classes;
        static std::map<uint8, std::map<uint8, std::string>> specs;
        classes[CLASS_DRUID] = "druid";
        specs[CLASS_DRUID][0] = "balance";
        specs[CLASS_DRUID][1] = "feral combat";
        specs[CLASS_DRUID][2] = "restoration";

        classes[CLASS_HUNTER] = "hunter";
        specs[CLASS_HUNTER][0] = "beast mastery";
        specs[CLASS_HUNTER][1] = "marksmanship";
        specs[CLASS_HUNTER][2] = "survival";

        classes[CLASS_MAGE] = "mage";
        specs[CLASS_MAGE][0] = "arcane";
        specs[CLASS_MAGE][1] = "fire";
        specs[CLASS_MAGE][2] = "frost";

        classes[CLASS_PALADIN] = "paladin";
        specs[CLASS_PALADIN][0] = "holy";
        specs[CLASS_PALADIN][1] = "protection";
        specs[CLASS_PALADIN][2] = "retribution";

        classes[CLASS_PRIEST] = "priest";
        specs[CLASS_PRIEST][0] = "discipline";
        specs[CLASS_PRIEST][1] = "holy";
        specs[CLASS_PRIEST][2] = "shadow";

        classes[CLASS_ROGUE] = "rogue";
        specs[CLASS_ROGUE][0] = "assasination";
        specs[CLASS_ROGUE][1] = "combat";
        specs[CLASS_ROGUE][2] = "subtlety";

        classes[CLASS_SHAMAN] = "shaman";
        specs[CLASS_SHAMAN][0] = "elemental";
        specs[CLASS_SHAMAN][1] = "enhancement";
        specs[CLASS_SHAMAN][2] = "restoration";

        classes[CLASS_WARLOCK] = "warlock";
        specs[CLASS_WARLOCK][0] = "affliction";
        specs[CLASS_WARLOCK][1] = "demonology";
        specs[CLASS_WARLOCK][2] = "destruction";

        classes[CLASS_WARRIOR] = "warrior";
        specs[CLASS_WARRIOR][0] = "arms";
        specs[CLASS_WARRIOR][1] = "fury";
        specs[CLASS_WARRIOR][2] = "protection";

        classes[CLASS_DEATH_KNIGHT] = "dk";
        specs[CLASS_DEATH_KNIGHT][0] = "blood";
        specs[CLASS_DEATH_KNIGHT][1] = "frost";
        specs[CLASS_DEATH_KNIGHT][2] = "unholy";

        // Use randombot 0.
        std::ostringstream cout;
        cout << sPlayerbotAIConfig.randomBotAccountPrefix << 0;
        std::string const accountName = cout.str();

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->SetData(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (result)
        {
            Field* fields = result->Fetch();
            uint32 accountId = fields[0].Get<uint32>();

            WorldSession* session =
                new WorldSession(accountId, "", 0x0, nullptr, SEC_PLAYER, EXPANSION_WRATH_OF_THE_LICH_KING, time_t(0),
                                 LOCALE_enUS, 0, false, false, 0, true);

            std::vector<std::pair<std::pair<uint32, uint32>, uint32>> classSpecLevel;

            std::unordered_map<std::string, std::vector<std::pair<std::pair<uint32, uint32>, uint32>>> actions;

            std::ostringstream out;

            for (uint8 race = RACE_HUMAN; race < sRaceMgr->GetMaxRaces(); race++)
            {
                for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                {
                    if (cls != 10)
                    {
                        std::unique_ptr<CharacterCreateInfo> characterInfo =
                            std::make_unique<CharacterCreateInfo>("dummy", race, cls, 1, 1, 1, 1, 1, 1);
                        Player* player = new Player(session);
                        if (player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(),
                                           characterInfo.get()))
                        {
                            for (uint8 tab = 0; tab < 3; tab++)
                            {
                                TalentSpec newSpec;
                                if (tab == 0)
                                    newSpec = TalentSpec(player, "1-0-0");
                                else if (tab == 1)
                                    newSpec = TalentSpec(player, "0-1-0");
                                else
                                    newSpec = TalentSpec(player, "0-0-1");

                                for (uint32 lvl = 1; lvl < MAX_LEVEL; lvl++)
                                {
                                    player->SetLevel(lvl);

                                    std::ostringstream tout;
                                    newSpec.ApplyTalents(player, &tout);

                                    PlayerbotAI* botAI = new PlayerbotAI(player);

                                    botAI->ResetStrategies(false);

                                    AiObjectContext* con = botAI->GetAiObjectContext();

                                    std::vector<std::string> tstrats;
                                    std::set<std::string> strategies;
                                    std::set<std::string> sstrats;

                                    tstrats = botAI->GetStrategies(BOT_STATE_COMBAT);
                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_NON_COMBAT);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    tstrats = botAI->GetStrategies(BOT_STATE_DEAD);
                                    if (!tstrats.empty())
                                        strategies.insert(tstrats.begin(), tstrats.end());

                                    sstrats = con->GetSupportedStrategies();
                                    if (!sstrats.empty())
                                        strategies.insert(sstrats.begin(), sstrats.end());

                                    for (auto& stratName : strategies)
                                    {
                                        Strategy* strat = con->GetStrategy(stratName);

                                        const std::vector<NextAction> defaultActions  = strat->getDefaultActions();

                                        if (defaultActions.size() > 0)
                                        {
                                            for (NextAction nextAction : defaultActions)
                                            {
                                                std::ostringstream aout;
                                                aout << nextAction.getRelevance() << "," << nextAction.getName()
                                                     << ",,S:" << stratName;

                                                if (actions.find(aout.str().c_str()) != actions.end())
                                                    classSpecLevel = actions.find(aout.str().c_str())->second;
                                                else
                                                    classSpecLevel.clear();

                                                classSpecLevel.push_back(std::make_pair(std::make_pair(cls, tab), lvl));

                                                actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                            }
                                        }

                                        std::vector<TriggerNode*> triggers;
                                        strat->InitTriggers(triggers);

                                        for (TriggerNode*& triggerNode : triggers)
                                        {
                                            if (Trigger* trigger = con->GetTrigger(triggerNode->getName()))
                                            {
                                                triggerNode->setTrigger(trigger);

                                                std::vector<NextAction> nextActions = triggerNode->getHandlers();

                                                // for (uint32_t i = 0; i < nextActions.size(); ++i)
                                                for (NextAction nextAction : nextActions)
                                                {
                                                    std::ostringstream aout;
                                                    aout << nextAction.getRelevance() << "," << nextAction.getName()
                                                         << "," << triggerNode->getName() << "," << stratName;

                                                    if (actions.find(aout.str().c_str()) != actions.end())
                                                        classSpecLevel = actions.find(aout.str().c_str())->second;
                                                    else
                                                        classSpecLevel.clear();

                                                    classSpecLevel.push_back(
                                                        std::make_pair(std::make_pair(cls, tab), lvl));

                                                    actions.insert_or_assign(aout.str().c_str(), classSpecLevel);
                                                }
                                            }
                                        }
                                    }

                                    delete botAI;
                                }
                            }
                        }

                        delete player;
                    }
                }
            }

            std::vector<std::string> actionKeys;

            for (auto& action : actions)
                actionKeys.push_back(action.first);

            std::sort(actionKeys.begin(), actionKeys.end(),
                      [](std::string const i, std::string const j)
                      {
                          std::stringstream is(i);
                          std::stringstream js(j);
                          float iref, jref;
                          std::string iact, jact, itrig, jtrig, istrat, jstrat;
                          is >> iref >> iact >> itrig >> istrat;
                          js >> jref >> jact >> jtrig >> jstrat;

                          if (iref > jref)
                              return true;

                          if (iref == jref && istrat < jstrat)
                              return true;

                          if (iref == jref && !(istrat > jstrat) && iact < jact)
                              return true;

                          if (iref == jref && !(istrat > jstrat) && !(iact > jact) && itrig < jtrig)
                              return true;

                          return false;
                      });

            sPlayerbotAIConfig.log("strategy.csv", "relevance, action, trigger, strategy, classes");

            for (auto& actionkey : actionKeys)
            {
                if (actions.find(actionkey)->second.size() != (MAX_LEVEL - 1) * (MAX_CLASSES - 1))
                {
                    classSpecLevel = actions.find(actionkey)->second;

                    std::vector<std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>>> classs;

                    for (auto cl : classSpecLevel)
                    {
                        uint32 minLevel = MAX_LEVEL;
                        uint32 maxLevel = 0;

                        uint32 cls = cl.first.first;
                        uint32 tb = cl.first.second;

                        if (std::find_if(classs.begin(), classs.end(),
                                         [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                         { return i.first.first == cls && i.first.second == tb; }) == classs.end())
                        {
                            for (auto cll : classSpecLevel)
                            {
                                if (cll.first.first == cl.first.first && cll.first.second == cl.first.second)
                                {
                                    minLevel = std::min(minLevel, cll.second);
                                    maxLevel = std::max(maxLevel, cll.second);
                                }
                            }

                            classs.push_back(std::make_pair(cl.first, std::make_pair(minLevel, maxLevel)));
                        }
                    }

                    out << actionkey;

                    if (classs.size() != 9 * 3)
                    {
                        out << ",";

                        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
                        {
                            bool a[3] = {false, false, false};
                            uint32 min[3] = {0, 0, 0};
                            uint32 max[3] = {0, 0, 0};

                            if (std::find_if(classs.begin(), classs.end(),
                                             [cls](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                             { return i.first.first == cls; }) == classs.end())
                                continue;

                            for (uint32 tb = 0; tb < 3; tb++)
                            {
                                auto tcl = std::find_if(
                                    classs.begin(), classs.end(),
                                    [cls, tb](std::pair<std::pair<uint32, uint32>, std::pair<uint32, uint32>> i)
                                    { return i.first.first == cls && i.first.second == tb; });

                                if (tcl == classs.end())
                                    continue;

                                a[tb] = true;
                                min[tb] = tcl->second.first;
                                max[tb] = tcl->second.second;
                            }

                            if (a[0] && a[1] && a[2] && min[0] == min[1] && min[1] == min[2] && max[0] == max[1] &&
                                max[1] == max[2])
                            {
                                if (min[0] != 1 || max[0] != MAX_LEVEL - 1)
                                    out << classes[cls] << "(" << min[0] << "-" << max[0] << ")";
                                else
                                    out << classes[cls];

                                if (cls != classs.back().first.first)
                                    out << ";";
                            }
                            else
                            {
                                for (uint32 tb = 0; tb < 3; tb++)
                                {
                                    if (!a[tb])
                                        continue;

                                    if (min[tb] != 1 || max[tb] != MAX_LEVEL - 1)
                                        out << specs[cls][tb] << " " << classes[cls] << "(" << min[tb] << "-" << max[tb]
                                            << ")";
                                    else
                                        out << specs[cls][tb] << " " << classes[cls];

                                    if (cls != classs.back().first.first || tb != classs.back().first.second)
                                        out << ";";
                                }
                            }
                        }
                    }
                    else
                        out << "all";

                    out << "\n";
                }
                else
                    out << actionkey << "\n";
            }

            sPlayerbotAIConfig.log("strategy.csv", out.str().c_str());
        }
    }

    /*

    sPlayerbotAIConfig.openLog(7, "w");

    //Zone area map REMOVE!
    uint32 k = 0;
    for (auto& node : TravelNodeMap::instance().getNodes())
    {
        WorldPosition* pos = node->getPosition();
        //map area
        for (uint32 x = 0; x < 2000; x++)
        {
            for (uint32 y = 0; y < 2000; y++)
            {
                if (!pos->getMap())
                    continue;

                float nx = pos->GetPositionX() + (x * 5) - 5000.0f;
                float ny = pos->GetPositionY() + (y * 5) - 5000.0f;
                float nz = pos->GetPositionZ() + 100.0f;

                //pos->getMap()->GetHitPosition(nx, ny, nz + 200.0f, nx, ny, nz, -0.5f);

                if (!pos->getMap()->GetHeightInRange(nx, ny, nz, 5000.0f)) // GetHeight can fail
                    continue;

                WorldPosition npos = WorldPosition(pos->GetMapId(), nx, ny, nz, 0.0);
                uint32 area = path.getArea(npos.GetMapId(), npos.GetPositionX(), npos.GetPositionY(),
                                           npos.GetPositionZ());

                std::ostringstream out;
                out << std::fixed << area << "," << npos.getDisplayX() << "," << npos.getDisplayY();
                sPlayerbotAIConfig.log(7, out.str().c_str());
            }
        }
        k++;

        if (k > 0)
            break;
    }

    //Explore map output (REMOVE!)

    sPlayerbotAIConfig.openLog(5, "w");
    for (auto i : exploreLocs)
    {
        for (auto j : i.second->getPoints())
        {
            std::ostringstream out;
            std::string const name = i.second->getTitle();
            name.erase(remove(name.begin(), name.end(), '\"'), name.end());
            out << std::fixed << std::setprecision(2) << name.c_str() << "," << i.first << "," << j->getDisplayX() <<
    "," << j->getDisplayY() << "," << j->GetPositionX() << "," << j->GetPositionY() << "," << j->GetPositionZ();
    sPlayerbotAIConfig.log(5,
    out.str().c_str());
        }
    }

    */
#endif
}
