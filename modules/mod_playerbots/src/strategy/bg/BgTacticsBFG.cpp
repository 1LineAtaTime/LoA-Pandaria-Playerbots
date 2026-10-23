/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Battle for Gilneas (map 761, BATTLEGROUND_BFG) bot tactics - issue #576, epic #580.
// AC has no spec for this map; this extends AC's Arathi Basin pattern (selectObjective AB case
// AC BattleGroundTactics.cpp:2312-2489, atFlag AB case :3561-3965, moveToStart AB branch :1719-1840)
// to BfG's three capturable nodes.

#include "BattleGroundTactics.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundBFG.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <map>
#include <mutex>

namespace
{
// Derived node state, same numbering as BattlegroundBFG::m_nodes (BattlegroundBFG.h:241-247,
// enum BG_BFG_NodeStatus :115-123). m_nodes is private with no getter, so the state is read from
// which banner object of the node is currently spawned (CreateBanner/DelBanner,
// BattlegroundBFG.cpp:278-310, uses BgObjects[node * 8 + type]).
constexpr uint8 BFG_STATE_UNKNOWN = 255;  // banner swap in progress (2 s delay, BattlegroundBFG.cpp:284-287)

// Entries a bot may cast SPELL_CAPTURE_BANNER on: the three neutral node banners and the
// contested/occupied variants (BattlegroundBFG.h:48-52, 84-88).
std::vector<uint32> const vFlagsBFG = {
    BG_BFG_OBJECTID_NODE_BANNER_0, BG_BFG_OBJECTID_NODE_BANNER_1, BG_BFG_OBJECTID_NODE_BANNER_2,
    BG_BFG_OBJECTID_BANNER_A,      BG_BFG_OBJECTID_BANNER_CONT_A, BG_BFG_OBJECTID_BANNER_H,
    BG_BFG_OBJECTID_BANNER_CONT_H,
};

uint32 const BFG_AttackObjectives[BG_BFG_DYNAMIC_NODES_COUNT] = {
    BG_BFG_NODE_LIGHTHOUSE, BG_BFG_NODE_WATERWORKS, BG_BFG_NODE_MINE};

// Walkable standing height above the banner object's z: the banner sits in a navmesh hole and the
// surrounding mesh surface is 0.65-0.9 yd above BG_BFG_NodePositions z (mmtile_probe scan, notes file).
float const BFG_NODE_Z_OFFSET = 0.7f;

// Waiting spots in the start rooms: BG_BFG_SpiritGuidePos[3] (Alliance) / [4] (Horde), verified ON NAVMESH.
Position const BFG_WAITING_POS_ALLIANCE = {908.274f, 1338.60f, 27.6449f, 5.95f};
Position const BFG_WAITING_POS_HORDE = {1401.38f, 977.125f, 7.44215f, 3.04f};

// Camp spots for when no node is left to take: the enemy start room (same points as the waiting spots).
Position const BFG_GY_CAMPING_ALLIANCE = {1401.38f, 977.125f, 7.44215f, 3.04f};  // alliance bots camp the horde base
Position const BFG_GY_CAMPING_HORDE = {908.274f, 1338.60f, 27.6449f, 5.95f};

// BgObjects index of a node's banner variant (BattlegroundBFG.cpp:296-301: node * 8 + type).
uint32 BfgObj(uint8 node, uint8 type)
{
    return uint32(node) * 8u + type;
}

// Node state 0 neutral, 1 ally contested, 2 horde contested, 3 ally occupied, 4 horde occupied.
uint8 BfgNodeState(Battleground* bg, uint8 node)
{
    // BG_BFG_OBJECT_BANNER_NEUTRAL..HORDE = 0..4 (BattlegroundBFG.h:56-60)
    for (uint8 type = BG_BFG_OBJECT_BANNER_NEUTRAL; type <= BG_BFG_OBJECT_BANNER_HORDE; ++type)
    {
        GameObject* go = bg->GetBGObject(BfgObj(node, type));
        if (go && go->isSpawned())
            return type;  // type index == state number
    }
    return BFG_STATE_UNKNOWN;
}

bool BfgIsEnemyOccupied(uint8 state, TeamId team)
{
    return (team == TEAM_ALLIANCE && state == BG_BFG_NODE_STATUS_HORDE_OCCUPIED) ||
           (team == TEAM_HORDE && state == BG_BFG_NODE_STATUS_ALLY_OCCUPIED);
}

bool BfgIsEnemyContested(uint8 state, TeamId team)
{
    return (team == TEAM_ALLIANCE && state == BG_BFG_NODE_STATUS_HORDE_CONTESTED) ||
           (team == TEAM_HORDE && state == BG_BFG_NODE_STATUS_ALLY_CONTESTED);
}

bool BfgIsOwned(uint8 state, TeamId team)
{
    return (team == TEAM_ALLIANCE && state == BG_BFG_NODE_STATUS_ALLY_OCCUPIED) ||
           (team == TEAM_HORDE && state == BG_BFG_NODE_STATUS_HORDE_OCCUPIED);
}

bool BfgIsFriendlyContested(uint8 state, TeamId team)
{
    return (team == TEAM_ALLIANCE && state == BG_BFG_NODE_STATUS_ALLY_CONTESTED) ||
           (team == TEAM_HORDE && state == BG_BFG_NODE_STATUS_HORDE_CONTESTED);
}

// Mirror of the click check in BattlegroundBFG::EventPlayerClickedOnFlag (BattlegroundBFG.cpp:386-387):
// a banner may be used when the node is neutral or teamIndex == m_nodes % 2. That means the owner
// of a contested node cannot click its own contested banner again, only the opponent can.
bool BfgCanClick(uint8 state, TeamId team)
{
    if (state == BFG_STATE_UNKNOWN)
        return false;
    return state == BG_BFG_NODE_TYPE_NEUTRAL || uint8(team) == (state % 2);
}

float BfgDistToNode(Player* bot, uint8 node)
{
    float const dx = bot->GetPositionX() - BG_BFG_NodePositions[node][0];
    float const dy = bot->GetPositionY() - BG_BFG_NodePositions[node][1];
    float const dz = bot->GetPositionZ() - BG_BFG_NodePositions[node][2];
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

// Point near a node on the navmesh: banner position + random ring offset, z lifted to the mesh surface.
void BfgPointNearNode(uint8 node, float minR, float maxR, float& x, float& y, float& z)
{
    float const angle = frand(0.0f, 2.0f * 3.14159265f);
    float const r = frand(minR, maxR);
    x = BG_BFG_NodePositions[node][0] + r * std::cos(angle);
    y = BG_BFG_NodePositions[node][1] + r * std::sin(angle);
    z = BG_BFG_NodePositions[node][2] + BFG_NODE_Z_OFFSET;
}

// ---- Graveyard islands (#576) ---------------------------------------------------------------------------------
// Offline Detour check (dtsim.py-style union-find over the 15 map-761 tiles of testserver data-mop-extract mmaps):
// the Horde/Alliance start rooms, all three nodes, the Waterworks graveyard and both base graveyards are in ONE
// connected component (14647 polygons), but the Lighthouse graveyard (WorldSafeLocs 1736, 1034.8 1335.6 z 12) and the
// Mine graveyard (1735, 1252.2 836.5 z 27.8) each sit on a separate navmesh ISLAND (30 / 34 polygons, 684 / 1194 sq yd)
// with no path off it. GetClosestGraveYard hands a dead team the graveyard of every node it owns, so the first bot that
// died after a capture respawned on an island and stood at its rim for the rest of the match: wave 33 (t575 b33-bfg) has
// Rogerus, Inianuel, Giseliane, Rineva, Elinayla around (1258,845) / (1242,832) for 130+ s each, and Zorian, Erinie around
// (1037,1325) / (1009,1337), all "move to objective:FAILED". The rim is only 1-3 yd above the main mesh. Same cure as
// Silvershard Mines (BgTacticsSM.cpp SM_LEDGES) and AC's wsJumpDown/eyJumpDown: walk to a take-off point on the island,
// JumpTo() a landing point on the main mesh. Take-off/landing from the same offline check (nearest island/main vertex
// pairs; the take-off resolves to the island component and the landing to the main one).
struct BfgIslandPoly
{
    uint8 n;
    float v[18];  // x, y, z per vertex, up to 6 vertices (navmesh polygon, WoW coordinates)
};

BfgIslandPoly const BFG_ISLAND_LIGHTHOUSE_GY[] = {
    {4, {1009.1,1337.1,3.7,1009.1,1333.9,4.3,1012.5,1333.9,8.8,1012.8,1337.1,8.8}},
    {5, {1012.5,1333.9,8.8,1016.5,1329.6,9.9,1020.8,1333.9,10.9,1017.1,1341.3,12.0,1012.8,1337.1,8.8}},
    {5, {1024.0,1344.0,12.3,1017.1,1344.0,14.1,1017.1,1341.3,12.0,1020.8,1333.9,10.9,1024.0,1333.9,11.2}},
    {3, {1017.1,1346.1,15.7,1017.1,1344.0,14.1,1024.0,1344.0,12.3}},
    {4, {1024.0,1349.6,16.0,1013.6,1349.6,18.1,1017.1,1346.1,15.7,1024.0,1344.0,12.3}},
    {3, {1035.2,1342.9,11.7,1034.4,1344.0,12.0,1033.1,1344.0,12.0}},
    {3, {1043.2,1327.2,6.7,1045.3,1326.1,4.8,1045.3,1329.3,9.1}},
    {6, {1035.2,1342.9,11.7,1033.1,1344.0,12.0,1030.4,1343.2,12.0,1025.1,1333.9,10.9,1029.1,1329.6,10.4,1033.6,1329.6,10.1}},
    {3, {1037.3,1325.6,4.0,1041.9,1325.6,4.0,1043.2,1327.2,6.7}},
    {5, {1030.4,1343.2,12.0,1029.9,1344.0,12.0,1024.0,1344.0,12.3,1024.0,1333.9,11.2,1025.1,1333.9,10.9}},
    {3, {1045.3,1344.0,11.5,1037.9,1344.0,11.7,1037.1,1342.9,11.7}},
    {6, {1037.3,1325.6,4.0,1043.2,1327.2,6.7,1045.3,1329.3,9.1,1045.3,1344.0,11.5,1037.1,1342.9,11.7,1033.6,1329.6,10.1}},
    {3, {1037.1,1342.9,11.7,1035.2,1342.9,11.7,1033.6,1329.6,10.1}},
    {4, {1033.6,1345.9,12.0,1033.1,1344.0,12.0,1034.4,1344.0,12.0,1034.4,1345.6,12.0}},
    {3, {1024.0,1344.0,12.3,1029.9,1344.0,12.0,1029.3,1346.7,12.3}},
    {5, {1037.3,1346.1,12.0,1037.1,1345.1,13.6,1037.9,1344.0,11.7,1038.7,1345.3,12.0,1039.2,1348.5,12.0}},
    {3, {1033.6,1345.9,12.0,1034.4,1345.6,12.0,1036.3,1346.4,12.0}},
    {5, {1033.9,1349.3,12.3,1024.0,1349.6,16.0,1024.0,1344.0,12.3,1029.3,1346.7,12.3,1031.2,1347.7,12.3}},
    {3, {1031.2,1347.7,12.3,1032.5,1347.7,12.3,1033.9,1349.3,12.3}},
    {4, {1039.2,1348.5,12.0,1040.5,1348.8,12.0,1040.5,1353.6,12.3,1037.9,1353.6,12.3}},
    {3, {1036.3,1346.4,12.0,1037.3,1346.1,12.0,1039.2,1348.5,12.0}},
    {6, {1033.9,1349.3,12.3,1032.5,1347.7,12.3,1033.6,1345.9,12.0,1036.3,1346.4,12.0,1039.2,1348.5,12.0,1037.9,1353.6,12.3}},
    {3, {1042.7,1348.0,11.7,1042.4,1345.3,11.7,1045.3,1345.6,11.5}},
    {3, {1040.5,1353.6,12.3,1040.5,1348.8,12.0,1042.7,1348.0,11.7}},
    {4, {1042.7,1348.0,11.7,1045.3,1345.6,11.5,1045.3,1353.6,11.5,1040.5,1353.6,12.3}},
    {4, {1045.3,1344.0,11.5,1045.3,1345.6,11.5,1042.4,1345.3,11.7,1041.3,1344.5,11.7}},
    {3, {1041.3,1344.5,11.7,1038.7,1345.3,12.0,1037.9,1344.0,11.7}},
    {3, {1045.3,1344.0,11.5,1041.3,1344.5,11.7,1037.9,1344.0,11.7}},
    {4, {1045.3,1344.0,11.5,1045.3,1329.3,9.1,1053.6,1337.3,9.6,1046.9,1344.0,11.5}},
    {3, {1046.9,1344.0,11.5,1045.3,1345.6,11.5,1045.3,1344.0,11.5}},
};

BfgIslandPoly const BFG_ISLAND_MINE_GY[] = {
    {3, {1242.1,810.7,28.0,1242.1,805.3,28.0,1245.6,808.8,28.0}},
    {4, {1242.1,810.7,28.0,1245.6,808.8,28.0,1246.4,808.8,28.0,1258.7,810.7,28.5}},
    {3, {1253.9,804.5,31.5,1258.7,804.5,28.8,1258.7,810.7,28.5}},
    {4, {1246.4,808.8,28.0,1252.0,802.7,31.5,1253.9,804.5,31.5,1258.7,810.7,28.5}},
    {4, {1238.1,812.5,32.3,1239.5,810.9,32.8,1241.9,812.5,28.3,1238.1,820.8,28.5}},
    {6, {1255.7,825.1,28.0,1251.5,826.9,28.0,1248.5,824.5,28.0,1241.9,812.5,28.3,1242.1,810.7,28.0,1258.7,810.7,28.5}},
    {4, {1242.1,824.5,28.0,1245.6,825.9,28.0,1246.7,828.8,28.0,1242.1,832.0,28.0}},
    {3, {1255.7,825.1,28.0,1253.3,826.7,28.0,1251.5,826.9,28.0}},
    {4, {1254.7,828.0,28.0,1254.1,828.3,28.0,1253.3,826.7,28.0,1255.7,825.1,28.0}},
    {4, {1251.5,826.9,28.0,1250.7,829.3,28.0,1249.9,828.0,28.0,1248.5,824.5,28.0}},
    {4, {1253.9,829.6,28.0,1254.1,828.3,28.0,1254.7,828.0,28.0,1257.3,829.3,28.0}},
    {3, {1258.1,829.3,28.0,1258.7,828.3,28.0,1258.7,832.0,28.0}},
    {5, {1248.5,824.5,28.0,1245.6,825.9,28.0,1242.1,824.5,28.0,1238.1,820.8,28.5,1241.9,812.5,28.3}},
    {3, {1258.7,810.7,28.5,1258.7,825.6,28.0,1255.7,825.1,28.0}},
    {3, {1250.7,829.3,28.0,1246.7,828.8,28.0,1249.9,828.0,28.0}},
    {4, {1257.3,829.3,28.0,1258.1,829.3,28.0,1258.7,832.0,28.0,1253.9,829.6,28.0}},
    {4, {1250.7,829.3,28.0,1252.3,829.3,29.6,1253.9,829.6,28.0,1258.7,832.0,28.0}},
    {4, {1242.1,832.0,28.0,1246.7,828.8,28.0,1250.7,829.3,28.0,1258.7,832.0,28.0}},
    {3, {1257.9,841.9,28.5,1257.9,844.8,23.7,1254.7,841.6,28.5}},
    {4, {1258.7,841.6,28.5,1257.9,841.9,28.5,1254.7,841.6,28.5,1258.7,832.0,28.0}},
    {5, {1253.6,841.6,28.3,1247.7,847.5,26.1,1242.1,841.6,28.0,1242.1,832.0,28.0,1258.7,832.0,28.0}},
    {3, {1258.7,832.0,28.0,1254.7,841.6,28.5,1253.6,841.6,28.3}},
    {4, {1266.4,808.8,28.8,1270.9,808.8,28.5,1272.5,810.7,28.0,1258.7,810.7,28.5}},
    {4, {1258.7,810.7,28.5,1258.7,804.5,28.8,1262.4,804.5,29.6,1266.4,808.8,28.8}},
    {4, {1275.2,826.1,28.5,1276.3,826.7,30.1,1276.3,827.5,29.9,1274.7,828.8,27.7}},
    {5, {1274.7,829.6,28.3,1276.5,831.2,32.3,1275.7,832.0,32.0,1258.7,832.0,28.0,1258.7,828.3,28.0}},
    {6, {1272.5,810.7,28.0,1274.7,812.8,28.3,1280.0,819.5,29.9,1274.7,824.5,28.5,1258.7,825.6,28.0,1258.7,810.7,28.5}},
    {5, {1274.7,824.5,28.5,1275.2,826.1,28.5,1274.7,828.8,27.7,1258.7,828.3,28.0,1258.7,825.6,28.0}},
    {4, {1274.7,812.8,28.3,1278.7,812.8,28.5,1280.0,814.4,28.5,1280.0,819.5,29.9}},
    {3, {1274.7,828.8,27.7,1274.7,829.6,28.3,1258.7,828.3,28.0}},
    {3, {1275.7,832.0,32.0,1270.9,837.1,34.4,1268.3,835.2,29.6}},
    {3, {1258.7,832.0,28.0,1275.7,832.0,32.0,1268.3,835.2,29.6}},
    {4, {1260.5,843.2,27.7,1258.7,841.6,28.5,1258.7,832.0,28.0,1268.3,835.2,29.6}},
    {3, {1280.0,819.5,29.9,1280.0,814.4,28.5,1282.7,816.8,29.3}},
};

struct BfgIsland
{
    char const* name;
    BfgIslandPoly const* polys;
    uint32 count;
    float minX, maxX, minY, maxY;
    float takeoff[3];
    float landing[3];
};

BfgIsland const BFG_ISLANDS[2] = {
    {"lighthouse-gy", BFG_ISLAND_LIGHTHOUSE_GY, uint32(sizeof(BFG_ISLAND_LIGHTHOUSE_GY) / sizeof(BfgIslandPoly)),
     1008.0f, 1055.0f, 1324.0f, 1355.0f, {1041.9f, 1325.6f, 4.0f}, {1039.3f, 1323.9f, 2.5f}},
    {"mine-gy", BFG_ISLAND_MINE_GY, uint32(sizeof(BFG_ISLAND_MINE_GY) / sizeof(BfgIslandPoly)),
     1237.0f, 1284.0f, 801.0f, 849.0f, {1257.9f, 844.8f, 23.7f}, {1255.7f, 849.2f, 20.7f}},
};
float const BFG_TAKEOFF_RADIUS = 2.5f;
float const BFG_ISLAND_Z_TOLERANCE = 4.0f;

bool BfgPointInPoly(float x, float y, BfgIslandPoly const& p)
{
    bool inside = false;
    for (uint32 i = 0, j = p.n - 1u; i < p.n; j = i++)
    {
        float const xi = p.v[i * 3], yi = p.v[i * 3 + 1], xj = p.v[j * 3], yj = p.v[j * 3 + 1];
        if (((yi > y) != (yj > y)) && (x < (xj - xi) * (y - yi) / (yj - yi) + xi))
            inside = !inside;
    }
    return inside;
}

// The island the bot is standing on (inside one of its polygons, within a few yards of that polygon's height), or null.
BfgIsland const* BfgIslandOf(Player* bot)
{
    float const x = bot->GetPositionX(), y = bot->GetPositionY(), z = bot->GetPositionZ();
    for (BfgIsland const& isl : BFG_ISLANDS)
    {
        if (x < isl.minX || x > isl.maxX || y < isl.minY || y > isl.maxY)
            continue;
        for (uint32 i = 0; i < isl.count; ++i)
        {
            BfgIslandPoly const& p = isl.polys[i];
            if (!BfgPointInPoly(x, y, p))
                continue;
            float avgZ = 0.0f;
            for (uint32 k = 0; k < p.n; ++k)
                avgZ += p.v[k * 3 + 2];
            avgZ /= p.n;
            if (std::fabs(z - avgZ) <= BFG_ISLAND_Z_TOLERANCE)
                return &isl;
        }
    }
    return nullptr;
}

// ---- Match evidence (#576) ------------------------------------------------------------------------------------
// "[BGBFG] node-change" on every observed node state change and one "[BGBFG] state" line per instance every 30 s,
// written by whichever bot of the instance runs first (the core banner objects are the truth, the bot only reports).
// Process-wide with a mutex, not thread_local: an instance's map update can run on any MapUpdate.Threads worker.
char const* const BFG_NODE_NAMES[3] = {"lighthouse", "waterworks", "mine"};
char const* const BFG_STATE_NAMES[5] = {"neutral", "A-contested", "H-contested", "A-occupied", "H-occupied"};

struct BfgWatch
{
    uint32 lastLogMs = 0;
    uint8 state[3] = {BFG_STATE_UNKNOWN, BFG_STATE_UNKNOWN, BFG_STATE_UNKNOWN};
    uint32 changes = 0;
};
std::mutex s_bfgWatchLock;
std::map<uint32, BfgWatch> s_bfgWatch;

char const* BfgStateName(uint8 s)
{
    return s < 5 ? BFG_STATE_NAMES[s] : "swap";
}

void BfgObserve(Battleground* bg)
{
    if (bg->GetStatus() != STATUS_IN_PROGRESS)
        return;

    uint8 now[3];
    for (uint8 i = 0; i < 3; ++i)
        now[i] = BfgNodeState(bg, i);

    std::lock_guard<std::mutex> guard(s_bfgWatchLock);
    BfgWatch& w = s_bfgWatch[bg->GetInstanceID()];
    for (uint8 i = 0; i < 3; ++i)
    {
        if (now[i] == BFG_STATE_UNKNOWN || now[i] == w.state[i])
            continue;
        if (w.state[i] != BFG_STATE_UNKNOWN)
        {
            ++w.changes;
            TC_LOG_DEBUG("playerbots", "[BGBFG] node-change inst=%u node=%s %s->%s score=%u-%u", bg->GetInstanceID(),
                         BFG_NODE_NAMES[i], BfgStateName(w.state[i]), BfgStateName(now[i]),
                         bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
        }
        w.state[i] = now[i];
    }

    uint32 const nowMs = getMSTime();
    if (w.lastLogMs && getMSTimeDiff(w.lastLogMs, nowMs) < 30000)
        return;
    w.lastLogMs = nowMs;
    TC_LOG_DEBUG("playerbots", "[BGBFG] state inst=%u score=%u-%u changes=%u lighthouse=%s waterworks=%s mine=%s",
                 bg->GetInstanceID(), bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE), w.changes,
                 BfgStateName(now[0]), BfgStateName(now[1]), BfgStateName(now[2]));
}
}  // namespace

bool BGTactics::moveToStartBFG(Battleground* bg)
{
    if (!bg)
        return false;

    // AC moveToStart AB branch (AC BattleGroundTactics.cpp:1760-1767): spread around the waiting spot.
    if (BgTeamIdOf(bot) == TEAM_HORDE)
        return MoveTo(bg->GetMapId(), BFG_WAITING_POS_HORDE.GetPositionX() + frand(-4.0f, 4.0f),
                      BFG_WAITING_POS_HORDE.GetPositionY() + frand(-4.0f, 4.0f), BFG_WAITING_POS_HORDE.GetPositionZ());

    return MoveTo(bg->GetMapId(), BFG_WAITING_POS_ALLIANCE.GetPositionX() + frand(-4.0f, 4.0f),
                  BFG_WAITING_POS_ALLIANCE.GetPositionY() + frand(-4.0f, 4.0f),
                  BFG_WAITING_POS_ALLIANCE.GetPositionZ());
}

bool BGTactics::selectObjectiveBFG(Battleground* bg)
{
    if (!bg)
        return false;

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["bg objective"];

    TeamId const team = BgTeamIdOf(bot);
    uint8 const role = context->GetValue<uint32>("bg role")->Get();

    // Same strategy split as AB (AC :2321-2335). GetBotStrategyForTeam returns 0 (balanced) for a map
    // without a strategy roll, which is the AB default of 3 defenders in 10.
    ABBotStrategy const strategyHorde = static_cast<ABBotStrategy>(GetBotStrategyForTeam(bg, TEAM_HORDE));
    ABBotStrategy const strategyAlliance = static_cast<ABBotStrategy>(GetBotStrategyForTeam(bg, TEAM_ALLIANCE));
    ABBotStrategy const strategy = (team == TEAM_ALLIANCE) ? strategyAlliance : strategyHorde;
    ABBotStrategy const enemyStrategy = (team == TEAM_ALLIANCE) ? strategyHorde : strategyAlliance;

    uint8 defendersProhab = 3;
    if (strategy == AB_STRATEGY_OFFENSIVE)
        defendersProhab = 1;
    else if (strategy == AB_STRATEGY_DEFENSIVE)
        defendersProhab = 6;

    if (enemyStrategy == AB_STRATEGY_DEFENSIVE)
        defendersProhab = 2;

    bool const isDefender = role < defendersProhab;
    bool const isSilly = urand(0, 99) < 20;

    uint8 states[BG_BFG_DYNAMIC_NODES_COUNT];
    for (uint8 i = 0; i < BG_BFG_DYNAMIC_NODES_COUNT; ++i)
        states[i] = BfgNodeState(bg, i);

    // --- PRIORITY 1: nearby enemy (rare aggressive impulse), AC :2347-2360
    if (urand(0, 99) < 5)
    {
        if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
        {
            if (bot->GetDistance(enemy) < 500.0f)
            {
                pos.Set(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), bot->GetMapId());
                posMap["bg objective"] = pos;
                return true;
            }
        }
    }

    // --- PRIORITY 2: no node left to take? attack a visible enemy or camp their start room (AC :2362-2408)
    bool hasValidTarget = false;
    for (uint32 nodeId : BFG_AttackObjectives)
    {
        uint8 const state = states[nodeId];
        if (state == BG_BFG_NODE_TYPE_NEUTRAL || BfgIsEnemyOccupied(state, team) || BfgIsEnemyContested(state, team))
        {
            hasValidTarget = true;
            break;
        }
    }

    if (!hasValidTarget)
    {
        if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
        {
            if (bot->GetDistance(enemy) < 500.0f)
            {
                pos.Set(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), bot->GetMapId());
                posMap["bg objective"] = pos;
                return true;
            }
        }

        Position const camp = (team == TEAM_ALLIANCE) ? BFG_GY_CAMPING_ALLIANCE : BFG_GY_CAMPING_HORDE;
        // Not WorldObject::GetRandomPoint: the fork's version scales the radius twice and scatters a 6 yd radius
        // up to 24 yd (issue #676), which would put the camper outside the small start room.
        float const campAngle = frand(0.0f, 2.0f * 3.14159265f);
        float const campRadius = frand(0.0f, 5.0f);
        pos.Set(camp.GetPositionX() + campRadius * std::cos(campAngle),
                camp.GetPositionY() + campRadius * std::sin(campAngle), camp.GetPositionZ(), bot->GetMapId());
        posMap["bg objective"] = pos;
        return true;
    }

    // --- PRIORITY 3: defender logic (AC :2410-2434). Stay on nodes we own, or on nodes the enemy is
    // currently contesting (we can click those to defend). #576: also on a node WE are still contesting - it takes
    // BG_BFG_FLAG_CAPTURING_TIME (60 s) to become ours, the enemy can assault it any moment in that window, and a
    // defender standing there is the one who can click it back (the AC AB pattern has no such window).
    int32 chosenNode = -1;
    if (isDefender && urand(0, 99) < 85)
    {
        float closestDist = FLT_MAX;
        for (uint32 nodeId : BFG_AttackObjectives)
        {
            uint8 const state = states[nodeId];
            if (!BfgIsEnemyContested(state, team) && !BfgIsOwned(state, team) && !BfgIsFriendlyContested(state, team))
                continue;

            float const dist = BfgDistToNode(bot, uint8(nodeId));
            if (dist < closestDist)
            {
                closestDist = dist;
                chosenNode = int32(nodeId);
            }
        }
    }

    // --- PRIORITY 4: attack objectives (AC :2436-2478). Neutral, enemy-occupied or enemy-contested
    // nodes only: the owner of a contested node cannot click it (BfgCanClick), so a friendly-contested
    // node is NOT an attack target (AB's isFriendlyContested does not carry over).
    if (chosenNode < 0)
    {
        std::vector<uint32> pool;
        for (uint8 i = 0; i < 3; ++i)
        {
            float bestDist = isSilly ? 0.0f : FLT_MAX;
            int32 chosen = -1;

            for (uint32 nodeId : BFG_AttackObjectives)
            {
                uint8 const state = states[nodeId];
                if (!(state == BG_BFG_NODE_TYPE_NEUTRAL || BfgIsEnemyOccupied(state, team) ||
                      BfgIsEnemyContested(state, team)))
                    continue;

                if (std::find(pool.begin(), pool.end(), nodeId) != pool.end())
                    continue;

                float const dist = BfgDistToNode(bot, uint8(nodeId));
                if ((isSilly && dist > bestDist) || (!isSilly && dist < bestDist))
                {
                    bestDist = dist;
                    chosen = int32(nodeId);
                }
            }

            if (chosen >= 0)
                pool.push_back(uint32(chosen));
        }

        if (!pool.empty())
            chosenNode = int32(pool[urand(0, pool.size() - 1)]);
    }

    if (chosenNode < 0)
        return false;

    // --- Final move: a point 2-6 yd around the banner, on the mesh surface (banner is in a mesh hole).
    float rx, ry, rz;
    BfgPointNearNode(uint8(chosenNode), 2.0f, 6.0f, rx, ry, rz);
    pos.Set(rx, ry, rz, bot->GetMapId());
    posMap["bg objective"] = pos;
    return true;
}

bool BGTactics::atObjectiveBFG(Battleground* bg)
{
    if (!bg)
        return false;

    BfgObserve(bg);

    // Graveyard islands (#576): get down to the main mesh first - see BFG_ISLANDS. Runs after every resurrection too,
    // since the Lighthouse and Mine graveyards are the islands.
    if (bg->GetStatus() == STATUS_IN_PROGRESS && bot->IsAlive() && !bot->GetVehicle())
    {
        // Mid-jump (MotionMaster::MoveJump runs an EffectMovementGenerator): the arc passes through the island.
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
            return true;

        if (BfgIsland const* island = BfgIslandOf(bot))
        {
            if (bot->GetExactDist2d(island->takeoff[0], island->takeoff[1]) < BFG_TAKEOFF_RADIUS)
            {
                if (JumpTo(bg->GetMapId(), island->landing[0], island->landing[1], island->landing[2]))
                    TC_LOG_DEBUG("playerbots", "[BGBFG] jump-down bot=%s island=%s from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f",
                                 bot->GetName().c_str(), island->name, bot->GetPositionX(), bot->GetPositionY(),
                                 bot->GetPositionZ(), island->landing[0], island->landing[1], island->landing[2]);
                return true;
            }

            MoveTo(bg->GetMapId(), island->takeoff[0], island->takeoff[1], island->takeoff[2]);
            return true;
        }
    }

    TeamId const team = BgTeamIdOf(bot);

    GuidVector closeObjects = *context->GetValue<GuidVector>("closest game objects");
    GuidVector closePlayers = *context->GetValue<GuidVector>("closest friendly players");
    float const flagRange = INTERACTION_DISTANCE;

    // Keep the bot still, dismounted and channelling while its own capture cast runs (atFlag).
    auto keepStationaryWhileCapturing = [&](CurrentSpellTypes spellType)
    {
        Spell* currentSpell = bot->GetCurrentSpell(spellType);
        if (!currentSpell || !currentSpell->m_spellInfo || currentSpell->m_spellInfo->Id != SPELL_CAPTURE_BANNER)
            return false;

        GameObject* targetFlag = currentSpell->m_targets.GetGOTarget();
        if (!targetFlag || !targetFlag->isSpawned() || targetFlag->GetGoState() != GO_STATE_READY)
        {
            bot->CastStop();
            resetObjective();
            return false;
        }

        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        if (bot->IsInDisallowedMountForm())
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        if (bot->isMoving())
            bot->StopMoving();
        return true;
    };

    if (keepStationaryWhileCapturing(CURRENT_CHANNELED_SPELL) || keepStationaryWhileCapturing(CURRENT_GENERIC_SPELL))
        return true;

    if (closeObjects.empty())
        return false;

    // Find a banner in reach that this team may actually click (see BfgCanClick).
    GameObject* targetFlag = nullptr;
    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        if (std::find(vFlagsBFG.begin(), vFlagsBFG.end(), go->GetEntry()) == vFlagsBFG.end())
            continue;

        if (!go->isSpawned() || go->GetGoState() != GO_STATE_READY)
            continue;

        if (!bot->CanUseBattlegroundObject(go))
            continue;

        if (bot->GetDistance(go) > flagRange)
            continue;

        // Which node is this banner on? (all banner variants of a node share its position)
        int32 node = -1;
        for (uint8 i = 0; i < BG_BFG_DYNAMIC_NODES_COUNT; ++i)
        {
            float const dx = go->GetPositionX() - BG_BFG_NodePositions[i][0];
            float const dy = go->GetPositionY() - BG_BFG_NodePositions[i][1];
            if (dx * dx + dy * dy < 25.0f)
            {
                node = i;
                break;
            }
        }
        if (node < 0)
            continue;

        if (!BfgCanClick(BfgNodeState(bg, uint8(node)), team))
            continue;

        targetFlag = go;
        break;
    }

    if (!targetFlag)
        return false;

    // Engage-before-capture (AC atFlag AB behaviour): an enemy near the banner is fought first.
    if (Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target"))
    {
        if (enemyPlayer->IsAlive() && enemyPlayer->GetDistance(targetFlag) < flagRange * 2.0f)
        {
            context->GetValue<Unit*>("current target")->Set(enemyPlayer);
            return false;
        }
    }

    // A friend is already capturing: do not stack, pick a different node (AC atFlag AB behaviour).
    for (ObjectGuid const guid : closePlayers)
    {
        Unit* pFriend = botAI->GetUnit(guid);
        if (!pFriend || pFriend->GetGUID() == bot->GetGUID())
            continue;

        Spell* friendSpell = pFriend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!friendSpell)
            friendSpell = pFriend->GetCurrentSpell(CURRENT_CHANNELED_SPELL);

        if (friendSpell && friendSpell->m_spellInfo && friendSpell->m_spellInfo->Id == SPELL_CAPTURE_BANNER)
        {
            if (bot->GetDistance2d(pFriend) < 3.0f)
            {
                float const angle = bot->GetAngle(pFriend);
                MoveTo(bot->GetMapId(), bot->GetPositionX() + 5.0f * std::cos(angle),
                       bot->GetPositionY() + 5.0f * std::sin(angle), bot->GetPositionZ());
            }

            resetObjective();
            if (selectObjectiveBFG(bg))
                moveToObjective(true);
            return true;
        }
    }

    // Prevent capturing from inside the banner model (AC atFlag AB case).
    float const dist = bot->GetDistance(targetFlag);
    if (dist == 0.0f)
    {
        float const moveDist = bot->GetObjectSize() + targetFlag->GetObjectSize() + 0.1f;
        return MoveTo(bot->GetMapId(), targetFlag->GetPositionX() + (urand(0, 1) ? -moveDist : moveDist),
                      targetFlag->GetPositionY() + (urand(0, 1) ? -moveDist : moveDist), targetFlag->GetPositionZ());
    }

    // #754 (beyond AC): fight whoever is hitting the bot before starting the capture cast (BGTactics::CaptureThreat)
    if (Unit* threat = CaptureThreat(bot))
    {
        // #754 (owner 2026-10-08) keep the current target while it is alive and attacking the bot, else the threat;
        // engage it (#756: setting the value alone left a bot hit by a pet/guardian idle at the flag)
        Unit* const cur = AI_VALUE(Unit*, "current target");
        Unit* const fight = (cur && cur->IsAlive() && cur->GetVictim() == bot) ? cur : threat;
        if (!EngageThreat(bot, fight))
            context->GetValue<Unit*>("current target")->Set(fight);
        return false;
    }

    if (bot->IsMounted())
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    if (bot->IsInDisallowedMountForm())
        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_CAPTURE_BANNER);
    if (!spellInfo)
        return false;

    TC_LOG_DEBUG("playerbots", "[BGBFG] %s captures banner %u", bot->GetName().c_str(), targetFlag->GetEntry());

    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
    spell->m_targets.SetGOTarget(targetFlag);
    spell->prepare(&spell->m_targets);

    botAI->WaitForSpellCast(spell);

    resetObjective();
    return true;
}
