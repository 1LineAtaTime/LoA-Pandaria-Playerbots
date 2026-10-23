/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "BattleGroundTactics.h"
#include "BgRealType.h"
#include <limits>

// DatabaseEnv.h must precede the battleground headers: they reach BattlegroundQueue.h, which pulls
// in RatedPvp.h, which uses CharacterDatabaseTransaction without declaring it.
#include "DatabaseEnv.h"

#include "BattleGroundJoinAction.h"
#include "Battleground.h"
#include "BattlegroundAB.h"
#include "BattlegroundAV.h"
#include "BattlegroundEY.h"
#include "BattlegroundIC.h"
#include "BattlegroundMgr.h"
#include "BattlegroundSA.h"
#include "BattlegroundTP.h"
#include "BattlegroundWS.h"
#include "BgTacticsIC.h"  // #675: Isle of Conquest beyond AC
#include "BgTacticsSA.h"  // #717: Strand of the Ancients beyond AC
#include "Event.h"
#include "GameObject.h"
#include "IVMapManager.h"
#include "LastMovementValue.h"  // #756: BGTactics::EngageThreat
#include "ObjectAccessor.h"     // #674: [BGTP] carrier names
#include "PathGenerator.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "AttackersValue.h"   // #852 hunt diagnostic
#include "PositionValue.h"
#include "PvpTriggers.h"
#include "ServerFacade.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <mutex>
#include <string>

// #869 (beyond AC): flag/orb/cart carriers fight back - defined next to BGTactics::FightPathAttacker.
static bool CarrierHoldsToFight(Player* bot, PlayerbotAI* botAI, Battleground* bg, bool carrying);

// 5.4.8 note: upstream hand-builds CMSG_GAMEOBJ_USE / CMSG_AREATRIGGER and pushes them through
// the session. Both opcodes are bit-packed here - HandleGameObjectUseOpcode reads the guid with
// ReadBit/ReadByteSeq, and HandleAreaTriggerOpcode reads two bits after the id - so a 3.3.5-shaped
// buffer is parsed as garbage (the guid resolves to nothing, and the trigger read runs off the end
// of the buffer). That is why bots reached both flag rooms and never picked a flag up. These call
// the same server-side path the handlers reach after parsing, as the #33 queueing actions do.

// common bg positions
Position const WS_WAITING_POS_HORDE_1 = {944.981f, 1423.478f, 345.434f, 6.18f};
Position const WS_WAITING_POS_HORDE_2 = {948.488f, 1459.834f, 343.066f, 6.27f};
Position const WS_WAITING_POS_HORDE_3 = {933.484f, 1433.726f, 345.535f, 0.08f};
Position const WS_WAITING_POS_ALLIANCE_1 = {1510.502f, 1493.385f, 351.995f, 3.1f};
Position const WS_WAITING_POS_ALLIANCE_2 = {1496.578f, 1457.900f, 344.442f, 3.1f};
Position const WS_WAITING_POS_ALLIANCE_3 = {1521.235f, 1480.951f, 352.007f, 3.2f};
Position const WS_FLAG_POS_HORDE = {915.958f, 1433.925f, 346.193f, 0.0f};
Position const WS_FLAG_POS_ALLIANCE = {1539.219f, 1481.747f, 352.458f, 0.0f};
Position const WS_FLAG_HIDE_HORDE_1 = {926.142f, 1460.641f, 346.116f, 4.84f};
Position const WS_FLAG_HIDE_HORDE_2 = {925.166f, 1458.084f, 355.966f, 0.00f};
Position const WS_FLAG_HIDE_HORDE_3 = {924.922f, 1423.672f, 345.524f, 0.82f};
Position const WS_FLAG_HIDE_ALLIANCE_1 = {1529.249f, 1456.470f, 353.04f, 1.25f};
Position const WS_FLAG_HIDE_ALLIANCE_2 = {1540.286f, 1476.026f, 352.692f, 2.91f};
Position const WS_FLAG_HIDE_ALLIANCE_3 = {1495.807f, 1466.774f, 352.350f, 1.50f};
Position const WS_ROAM_POS = {1227.446f, 1476.235f, 307.484f, 1.50f};
Position const WS_GY_CAMPING_HORDE = {1039.819, 1388.759f, 340.703f, 0.0f};
Position const WS_GY_CAMPING_ALLIANCE = {1422.320f, 1551.978f, 342.834f, 0.0f};
std::vector<Position> const WS_FLAG_HIDE_HORDE = {WS_FLAG_HIDE_HORDE_1, WS_FLAG_HIDE_HORDE_2, WS_FLAG_HIDE_HORDE_3};
std::vector<Position> const WS_FLAG_HIDE_ALLIANCE = {WS_FLAG_HIDE_ALLIANCE_1, WS_FLAG_HIDE_ALLIANCE_2,
                                                     WS_FLAG_HIDE_ALLIANCE_3};

// Twin Peaks (map 726) is mechanically Warsong Gulch - same two-flag game, the same flag spells
// (23333 / 23335) and the same world state ids - so it shares all of the Warsong Gulch logic below
// and differs only in this data. Every position was snapped onto this core's own map 726 navmesh,
// anchored on BattlegroundTP::SetupBattleground's flag, gate and buff spawns.
// #674: the WAIT_JOIN guard in BGTactics::Execute pulls a bot back when it strays further than
// sqrt(battleground_template.StartMaxDist) = 50 yd (map 726) from its team start location (WorldSafeLocs 1726
// Alliance 2142.60,175.92,43.65 / 1727 Horde 1549.54,346.95,1.29). The waiting spots below keep that radius for
// every frand() spread: they used to sit beside the doors, up to 64 yd away (Alliance spot 3), so those bots
// were teleported back over and over. The door-side points are kept as TP_GUARD_POSTS_* further down.
Position const TP_WAITING_POS_HORDE_1 = {1560.800f, 316.800f, 1.855f, 3.64f};
Position const TP_WAITING_POS_HORDE_2 = {1570.000f, 323.000f, 1.588f, 4.90f};
Position const TP_WAITING_POS_HORDE_3 = {1569.067f, 368.267f, 1.777f, 3.08f};
Position const TP_WAITING_POS_ALLIANCE_1 = {2116.267f, 158.933f, 43.721f, 5.12f};
Position const TP_WAITING_POS_ALLIANCE_2 = {2152.000f, 206.000f, 43.721f, 0.06f};
Position const TP_WAITING_POS_ALLIANCE_3 = {2140.000f, 194.000f, 43.721f, 0.28f};
Position const TP_FLAG_POS_HORDE = {1578.380f, 344.037f, 2.419f, 0.0f};
Position const TP_FLAG_POS_ALLIANCE = {2118.210f, 191.621f, 44.052f, 0.0f};
Position const TP_FLAG_HIDE_HORDE_1 = {1596.533f, 334.933f, 1.423f, 2.87f};
Position const TP_FLAG_HIDE_HORDE_2 = {1594.667f, 354.133f, 1.588f, 3.70f};
Position const TP_FLAG_HIDE_HORDE_3 = {1576.000f, 362.667f, 1.855f, 4.84f};
Position const TP_FLAG_HIDE_ALLIANCE_1 = {2101.066f, 183.733f, 43.721f, 0.43f};
Position const TP_FLAG_HIDE_ALLIANCE_2 = {2112.000f, 173.067f, 43.721f, 1.25f};
Position const TP_FLAG_HIDE_ALLIANCE_3 = {2114.400f, 213.067f, 43.721f, 4.93f};
// #674: the old point (1834.667, 323.2, -24.279) sits in a hole of the map-726 navmesh at the river centre; this is the
// nearest ground polygon (poly 2564, surf z -24.945, mmtile_probe). SetSafePos() resolves to this exact point.
Position const TP_ROAM_POS = {1840.000f, 320.000f, -24.945f, 1.50f};
// #674: the graveyards a dead player RESPAWNS at while the match runs. BattlegroundTP::GetClosestGraveYard returns
// TP_GRAVEYARD_MIDDLE_HORDE (WorldSafeLocs 1750) / TP_GRAVEYARD_MIDDLE_ALLIANCE (1749) for IN_PROGRESS - the two
// valley graveyards - and the start-room graveyards (1728 / 1729, the old values here) only before the gates open.
// Camping the start graveyards therefore caught nobody. Both points are ON the map-726 navmesh (mmtile_probe).
Position const TP_GY_CAMPING_HORDE = {1816.870f, 160.083f, 1.470f, 0.0f};
Position const TP_GY_CAMPING_ALLIANCE = {1877.150f, 439.111f, -4.090f, 0.0f};
std::vector<Position> const TP_FLAG_HIDE_HORDE = {TP_FLAG_HIDE_HORDE_1, TP_FLAG_HIDE_HORDE_2, TP_FLAG_HIDE_HORDE_3};
std::vector<Position> const TP_FLAG_HIDE_ALLIANCE = {TP_FLAG_HIDE_ALLIANCE_1, TP_FLAG_HIDE_ALLIANCE_2,
                                                     TP_FLAG_HIDE_ALLIANCE_3};

// #674: Twin Peaks flag-room entrance posts. Each base is entered through three doors (BattlegroundTP::
// SetupBattleground BG_TP_OBJECT_DOOR_A_1..3 / DOOR_H_1/2/4, Wowpedia "Twin Peaks": "three entrances to the
// complex"), and these are the in-room points beside those doors (all ON the map-726 navmesh, mmtile_probe).
// Defenders with nothing to chase hold one of them (role % 3) instead of roaming the river valley, which is the
// Warsong Gulch "roam the middle" fallback and leaves the flag room open here.
std::vector<Position> const TP_GUARD_POSTS_ALLIANCE = {{2116.267f, 158.933f, 43.721f, 5.12f},
                                                       {2151.200f, 220.000f, 43.721f, 0.06f},
                                                       {2122.933f, 222.400f, 43.721f, 0.28f}};
std::vector<Position> const TP_GUARD_POSTS_HORDE = {{1560.800f, 316.800f, 1.855f, 3.64f},
                                                    {1586.667f, 322.133f, 1.588f, 4.90f},
                                                    {1569.067f, 368.267f, 1.777f, 3.08f}};


Position const AB_WAITING_POS_HORDE = {702.884f, 703.045f, -16.115f, 0.77f};
Position const AB_WAITING_POS_ALLIANCE = {1286.054f, 1282.500f, -15.697f, 3.95f};
// Arathi Basin lays its objects out in fixed-size groups, one group per dynamic node, so the banner
// for node N is object N * this. The count is implicit in BG_AB_ObjectTypes here rather than named.
uint32 const BG_AB_OBJECTS_PER_NODE = 8;

// Warsong Gulch flag-spawn area triggers. BattlegroundWS::HandleAreaTrigger handles both ids; they
// are simply not given names in this core's header.
uint32 const BG_WS_TRIGGER_ALLIANCE_FLAG_SPAWN = 3646;
uint32 const BG_WS_TRIGGER_HORDE_FLAG_SPAWN = 3647;
// Twin Peaks' equivalents, from BattlegroundTP::HandleAreaTrigger: 5904 is the Alliance flag
// spawn (where an Alliance carrier caps the Horde flag) and 5905 the Horde one.
uint32 const BG_TP_TRIGGER_ALLIANCE_FLAG_SPAWN = 5904;
uint32 const BG_TP_TRIGGER_HORDE_FLAG_SPAWN = 5905;

Position const AB_GY_CAMPING_HORDE = {723.513f, 725.924f, -28.265f, 3.99f};
Position const AB_GY_CAMPING_ALLIANCE = {1262.627f, 1256.341f, -27.289f, 0.64f};

Position const AV_WAITING_POS_ALLIANCE = {793.627f, -493.814f, 99.689f, 3.09f};
Position const AV_WAITING_POS_HORDE = {-1381.865f, -544.872f, 54.773f, 0.76f};
Position const AV_ICEBLOOD_GARRISON_WAITING_ALLIANCE = {-523.105f, -182.178f, 57.956f, 0.0f};
Position const AV_ICEBLOOD_GARRISON_ATTACKING_ALLIANCE = {-545.288f, -167.932f, 57.012f, 0.0f};
Position const AV_STONEHEARTH_WAITING_HORDE = {-36.399f, -306.403f, 15.565f, 0.0f};
Position const AV_STONEHEARTH_ATTACKING_HORDE = {-55.210f, -288.546f, 15.578f, 0.0f};
Position const AV_BOSS_WAIT_H = {689.210f, -20.173f, 50.620f, 0.0f};
Position const AV_BOSS_WAIT_A = {-1361.773f, -248.977f, 99.369f, 0.0f};
Position const AV_MINE_SOUTH_1 = {-860.159f, -12.780f, 70.817f, 0.0f};
Position const AV_MINE_SOUTH_2 = {-827.639f, -147.314f, 62.565f, 0.0f};
Position const AV_MINE_SOUTH_3 = {-920.228f, -134.594f, 61.478f, 0.0f};
Position const AV_MINE_NORTH_1 = {822.477f, -456.782f, 48.569f, 0.0f};
Position const AV_MINE_NORTH_2 = {966.362f, -446.570f, 56.641f, 0.0f};
Position const AV_MINE_NORTH_3 = {952.081f, -335.073f, 63.579f, 0.0f};

// AC's captains are the invisible trigger creatures spawned with (and sharing the death state of)
// the real captain NPC.
uint32 const AV_CREATURE_A_CAPTAIN = AV_CPLACE_TRIGGER16;
uint32 const AV_CREATURE_H_CAPTAIN = AV_CPLACE_TRIGGER18;
// AC's AV_CPLACE_A_BOSS / H_BOSS constants are not in this core's BG_AV_ObjectPlaces enum: the boss
// creatures are spawned into the "static" slot range (AV_CPLACE_MAX + n, BattlegroundAV::AddAVCreature)
// instead. Verified against BG_AV_StaticCreaturePos (BattlegroundAV.h): row 60 is Vanndar Stormpike
// (Alliance), row 122 is Drek'Thar (Horde) - same static slots as AC's AV_CPLACE_MAX+60/+122.
uint32 const AV_CREATURE_A_BOSS = AV_CPLACE_MAX + 60;
uint32 const AV_CREATURE_H_BOSS = AV_CPLACE_MAX + 122;

// Cave spawn points bots respawn at after dying in AV (BG_AV_StartLoc / WorldSafeLocs 610/611);
// used only to detect "just left the cave" so selectObjectiveWp can walk the bot out before scoring
// a path.
Position const AV_CAVE_SPAWN_ALLIANCE = {872.460f, -491.571f, 96.546f, 0.0f};
Position const AV_CAVE_SPAWN_HORDE = {-1437.127f, -608.382f, 51.185f, 0.0f};

Position const EY_WAITING_POS_HORDE = {1809.102f, 1540.854f, 1267.142f, 0.0f};
Position const EY_WAITING_POS_ALLIANCE = {2523.827f, 1596.915f, 1270.204f, 0.0f};
Position const EY_FLAG_RETURN_POS_RETREAT_HORDE = {1885.529f, 1532.157f, 1200.635f, 0.0f};
Position const EY_FLAG_RETURN_POS_RETREAT_ALLIANCE = {2452.253f, 1602.356f, 1203.617f, 0.0f};
Position const EY_GY_CAMPING_HORDE = {1874.854f, 1530.405f, 1207.432f, 0.0f};
Position const EY_GY_CAMPING_ALLIANCE = {2456.887f, 1599.025f, 1206.280f, 0.0f};

Position const IC_WAITING_POS_HORDE = {1166.322f, -762.402f, 48.628f};
Position const IC_WEST_WAITING_POS_HORDE = {1217.666f, -685.449f, 48.915f};
Position const IC_EAST_WAITING_POS_HORDE = {1219.068f, -838.791f, 48.916f};

Position const IC_WAITING_POS_ALLIANCE = {387.893f, -833.384f, 48.714f};
Position const IC_WEST_WAITING_POS_ALLIANCE = {352.129f, -788.029f, 48.916f};
Position const IC_EAST_WAITING_POS_ALLIANCE = {351.517f, -882.477f, 48.916f};

Position const IC_CANNON_POS_HORDE1 = {1140.938f, -838.685f, 88.124f, 2.30f};
Position const IC_CANNON_POS_HORDE2 = {1139.695f, -686.574f, 88.173f, 3.95f};
Position const IC_CANNON_POS_ALLIANCE1 = {424.860f, -855.795f, 87.96f, 0.44f};
Position const IC_CANNON_POS_ALLIANCE2 = {425.525f, -779.538f, 87.717f, 5.88f};

// AC PR mod-playerbots#2856 (merged 2026-10-06, Ai/Base/Actions/BattleGroundTactics.cpp:105): the Horde siege spot
// moves from 94 yd (24 yd below the Alliance front gate) to 65 yd. Its z comes from the map at runtime (icHordePark
// in the selectObjective IC case); the MoP navmesh has the ground at z 33.6 there (mmtile_probe).
Position const IC_GATE_ATTACK_POS_HORDE = {478.3f, -830.2f, 40.0f, 0.0f};  // z from the map at runtime
Position const IC_GATE_ATTACK_POS_ALLIANCE = {1091.273f, -763.619f, 42.352f, 0.0f};

enum BattleBotWsgWaitSpot
{
    BB_WSG_WAIT_SPOT_SPAWN,
    BB_WSG_WAIT_SPOT_LEFT,
    BB_WSG_WAIT_SPOT_RIGHT
};

std::unordered_map<uint32, BGStrategyData> bgStrategies;

std::vector<uint32> const vFlagsAB = {
    BG_AB_OBJECTID_BANNER_A,
    BG_AB_OBJECTID_BANNER_CONT_A,
    BG_AB_OBJECTID_BANNER_H,
    BG_AB_OBJECTID_BANNER_CONT_H,
    BG_AB_OBJECTID_NODE_BANNER_0,
    BG_AB_OBJECTID_NODE_BANNER_1,
    BG_AB_OBJECTID_NODE_BANNER_2,
    BG_AB_OBJECTID_NODE_BANNER_3,
    BG_AB_OBJECTID_NODE_BANNER_4
};

// AC-identical set (Ai/Base/Actions/BattleGroundTactics.cpp:117-120): the 9 capturable AV banners.
std::vector<uint32> const vFlagsAV = {
    BG_AV_OBJECTID_BANNER_H_B,      BG_AV_OBJECTID_BANNER_H,      BG_AV_OBJECTID_BANNER_A_B,
    BG_AV_OBJECTID_BANNER_A,        BG_AV_OBJECTID_BANNER_CONT_A, BG_AV_OBJECTID_BANNER_CONT_A_B,
    BG_AV_OBJECTID_BANNER_CONT_H_B, BG_AV_OBJECTID_BANNER_CONT_H, BG_AV_OBJECTID_BANNER_SNOWFALL_N};

std::vector<uint32> const vFlagsWS = {BG_OBJECT_A_FLAG_WS_ENTRY, BG_OBJECT_H_FLAG_WS_ENTRY,
                                      BG_OBJECT_A_FLAG_GROUND_WS_ENTRY, BG_OBJECT_H_FLAG_GROUND_WS_ENTRY};

std::vector<uint32> const vFlagsEY = {BG_OBJECT_FLAG2_EY_ENTRY, BG_OBJECT_FLAG3_EY_ENTRY};

// AC-identical (Ai/Base/Actions/BattleGroundTactics.cpp:139-173). Every GO_* entry below exists in this
// core's BattlegroundIC.h (GameobjectsIC enum) with the same value as AC.
std::vector<uint32> const vFlagsIC = {GO_HORDE_BANNER,
                                      GO_ALLIANCE_BANNER,
                                      GO_WORKSHOP_BANNER,
                                      GO_DOCKS_BANNER,
                                      GO_HANGAR_BANNER,
                                      GO_QUARRY_BANNER,
                                      GO_REFINERY_BANNER,
                                      GO_ALLIANCE_BANNER_DOCK,
                                      GO_ALLIANCE_BANNER_DOCK_CONT,
                                      GO_HORDE_BANNER_DOCK,
                                      GO_HORDE_BANNER_DOCK_CONT,
                                      GO_HORDE_BANNER_HANGAR,
                                      GO_HORDE_BANNER_HANGAR_CONT,
                                      GO_ALLIANCE_BANNER_HANGAR,
                                      GO_ALLIANCE_BANNER_HANGAR_CONT,
                                      GO_ALLIANCE_BANNER_QUARRY,
                                      GO_ALLIANCE_BANNER_QUARRY_CONT,
                                      GO_HORDE_BANNER_QUARRY,
                                      GO_HORDE_BANNER_QUARRY_CONT,
                                      GO_ALLIANCE_BANNER_REFINERY,
                                      GO_ALLIANCE_BANNER_REFINERY_CONT,
                                      GO_HORDE_BANNER_REFINERY,
                                      GO_HORDE_BANNER_REFINERY_CONT,
                                      GO_ALLIANCE_BANNER_WORKSHOP,
                                      GO_ALLIANCE_BANNER_WORKSHOP_CONT,
                                      GO_HORDE_BANNER_WORKSHOP,
                                      GO_HORDE_BANNER_WORKSHOP_CONT,
                                      GO_ALLIANCE_BANNER_GRAVEYARD_A,
                                      GO_ALLIANCE_BANNER_GRAVEYARD_A_CONT,
                                      GO_HORDE_BANNER_GRAVEYARD_A,
                                      GO_HORDE_BANNER_GRAVEYARD_A_CONT,
                                      GO_ALLIANCE_BANNER_GRAVEYARD_H,
                                      GO_ALLIANCE_BANNER_GRAVEYARD_H_CONT,
                                      GO_HORDE_BANNER_GRAVEYARD_H,
                                      GO_HORDE_BANNER_GRAVEYARD_H_CONT};

// Same order as vFlagsWS - base flags first, then the dropped/ground flags - because atFlag()
// indexes element [BgTeamIdOf(bot)] to decide which flag belongs to the bot's own team.
std::vector<uint32> const vFlagsTP = {BG_OBJECT_A_FLAG_TP_ENTRY, BG_OBJECT_H_FLAG_TP_ENTRY,
                                      BG_OBJECT_A_FLAG_GROUND_TP_ENTRY, BG_OBJECT_H_FLAG_GROUND_TP_ENTRY};

// BG Waypoints (vmangos)

// Horde Flag Room to Horde Graveyard
BattleBotPath vPath_WSG_HordeFlagRoom_to_HordeGraveyard = {
    {933.331f, 1433.72f, 345.536f, nullptr}, {944.859f, 1423.05f, 345.437f, nullptr},
    {966.691f, 1422.53f, 345.223f, nullptr}, {979.588f, 1422.84f, 345.46f, nullptr},
    {997.806f, 1422.52f, 344.623f, nullptr}, {1008.53f, 1417.02f, 343.206f, nullptr},
    {1016.42f, 1402.33f, 341.352f, nullptr}, {1029.14f, 1387.49f, 340.836f, nullptr},
};

// Horde Graveyard to Horde Tunnel
BattleBotPath vPath_WSG_HordeGraveyard_to_HordeTunnel = {
    {1029.14f, 1387.49f, 340.836f, nullptr}, {1034.95f, 1392.62f, 340.856f, nullptr},
    {1038.21f, 1406.43f, 341.562f, nullptr}, {1043.87f, 1426.9f, 339.197f, nullptr},
    {1054.53f, 1441.47f, 339.725f, nullptr}, {1056.33f, 1456.03f, 341.463f, nullptr},
    {1057.39f, 1469.98f, 342.148f, nullptr}, {1057.67f, 1487.55f, 342.537f, nullptr},
    {1048.7f, 1505.37f, 341.117f, nullptr},  {1042.19f, 1521.69f, 338.003f, nullptr},
    {1050.01f, 1538.22f, 332.43f, nullptr},  {1068.15f, 1548.1f, 321.446f, nullptr},
    {1088.14f, 1538.45f, 316.398f, nullptr}, {1101.26f, 1522.79f, 314.918f, nullptr},
    {1114.67f, 1503.18f, 312.947f, nullptr}, {1126.45f, 1487.4f, 314.136f, nullptr},
    {1124.37f, 1462.28f, 315.853f, nullptr},
};

// Horde Tunnel to Horde Flag Room
BattleBotPath vPath_WSG_HordeTunnel_to_HordeFlagRoom = {
    {1124.37f, 1462.28f, 315.853f, nullptr}, {1106.87f, 1462.13f, 316.558f, nullptr},
    {1089.44f, 1461.04f, 316.332f, nullptr}, {1072.07f, 1459.46f, 317.449f, nullptr},
    {1051.09f, 1459.89f, 323.126f, nullptr}, {1030.1f, 1459.58f, 330.204f, nullptr},
    {1010.76f, 1457.49f, 334.896f, nullptr}, {1005.47f, 1448.19f, 335.864f, nullptr},
    {999.974f, 1458.49f, 335.632f, nullptr}, {982.632f, 1459.18f, 336.127f, nullptr},
    {965.049f, 1459.15f, 338.076f, nullptr}, {944.526f, 1459.0f, 344.207f, nullptr},
    {937.479f, 1451.12f, 345.553f, nullptr}, {933.331f, 1433.72f, 345.536f, nullptr},
};

// Alliance Flag Room to Alliance Graveyard
BattleBotPath vPath_WSG_AllianceFlagRoom_to_AllianceGraveyard = {
    {1519.53f, 1481.87f, 352.024f, nullptr}, {1508.27f, 1493.17f, 352.005f, nullptr},
    {1490.78f, 1493.51f, 352.141f, nullptr}, {1469.79f, 1494.13f, 351.774f, nullptr},
    {1453.65f, 1494.39f, 350.614f, nullptr}, {1443.51f, 1501.75f, 348.317f, nullptr},
    {1443.33f, 1517.78f, 345.534f, nullptr}, {1443.55f, 1533.4f, 343.148f, nullptr},
    {1441.47f, 1548.12f, 342.752f, nullptr}, {1433.79f, 1552.67f, 342.763f, nullptr},
    {1422.88f, 1552.37f, 342.751f, nullptr}, {1415.33f, 1554.79f, 343.156f, nullptr},
};

// Alliance Graveyard to Alliance Tunnel
BattleBotPath vPath_WSG_AllianceGraveyard_to_AllianceTunnel = {
    {1415.33f, 1554.79f, 343.156f, nullptr}, {1428.29f, 1551.79f, 342.751f, nullptr},
    {1441.51f, 1545.79f, 342.757f, nullptr}, {1441.15f, 1530.35f, 343.712f, nullptr},
    {1435.53f, 1517.29f, 346.698f, nullptr}, {1424.81f, 1499.24f, 349.486f, nullptr},
    {1416.31f, 1483.94f, 348.536f, nullptr}, {1408.83f, 1468.4f, 347.648f, nullptr},
    {1404.64f, 1449.79f, 347.279f, nullptr}, {1405.34f, 1432.33f, 345.792f, nullptr},
    {1406.38f, 1416.18f, 344.755f, nullptr}, {1400.22f, 1401.87f, 340.496f, nullptr},
    {1385.96f, 1394.15f, 333.829f, nullptr}, {1372.38f, 1390.75f, 328.722f, nullptr},
    {1362.93f, 1390.02f, 327.034f, nullptr}, {1357.91f, 1398.07f, 325.674f, nullptr},
    {1354.17f, 1411.56f, 324.327f, nullptr}, {1351.44f, 1430.38f, 323.506f, nullptr},
    {1350.36f, 1444.43f, 323.388f, nullptr}, {1348.02f, 1461.06f, 323.167f, nullptr},
};

// Alliance Tunnel to Alliance Flag Room
BattleBotPath vPath_WSG_AllianceTunnel_to_AllianceFlagRoom = {
    {1348.02f, 1461.06f, 323.167f, nullptr}, {1359.8f, 1461.49f, 324.527f, nullptr},
    {1372.47f, 1461.61f, 324.354f, nullptr}, {1389.08f, 1461.12f, 325.913f, nullptr},
    {1406.57f, 1460.48f, 330.615f, nullptr}, {1424.04f, 1459.57f, 336.029f, nullptr},
    {1442.5f, 1459.7f, 342.024f, nullptr},   {1449.59f, 1469.14f, 342.65f, nullptr},
    {1458.03f, 1458.43f, 342.746f, nullptr}, {1469.4f, 1458.14f, 342.794f, nullptr},
    {1489.06f, 1457.86f, 342.794f, nullptr}, {1502.27f, 1457.52f, 347.589f, nullptr},
    {1512.87f, 1457.81f, 352.039f, nullptr}, {1517.53f, 1468.79f, 352.033f, nullptr},
    {1519.53f, 1481.87f, 352.024f, nullptr},
};

// Horde Tunnel to Horde Base Roof
BattleBotPath vPath_WSG_HordeTunnel_to_HordeBaseRoof = {
    {1124.37f, 1462.28f, 315.853f, nullptr}, {1106.87f, 1462.13f, 316.558f, nullptr},
    {1089.44f, 1461.04f, 316.332f, nullptr}, {1072.07f, 1459.46f, 317.449f, nullptr},
    {1051.09f, 1459.89f, 323.126f, nullptr}, {1030.1f, 1459.58f, 330.204f, nullptr},
    {1010.76f, 1457.49f, 334.896f, nullptr}, {981.948f, 1459.07f, 336.154f, nullptr},
    {981.768f, 1480.46f, 335.976f, nullptr}, {974.664f, 1495.9f, 340.837f, nullptr},
    {964.661f, 1510.21f, 347.509f, nullptr}, {951.188f, 1520.99f, 356.377f, nullptr},
    {937.37f, 1513.27f, 362.589f, nullptr},  {935.947f, 1499.58f, 364.199f, nullptr},
    {935.9f, 1482.08f, 366.396f, nullptr},   {937.564f, 1462.81f, 367.287f, nullptr},
    {945.871f, 1458.65f, 367.287f, nullptr}, {956.972f, 1459.48f, 367.291f, nullptr},
    {968.317f, 1459.71f, 367.291f, nullptr}, {979.934f, 1454.58f, 367.078f, nullptr},
    {979.99f, 1442.87f, 367.093f, nullptr},  {978.632f, 1430.71f, 367.125f, nullptr},
    {970.395f, 1422.32f, 367.289f, nullptr}, {956.338f, 1425.09f, 367.293f, nullptr},
    {952.778f, 1433.0f, 367.604f, nullptr},  {952.708f, 1445.01f, 367.604f, nullptr},
};

// Alliance Tunnel to Alliance Base Roof
BattleBotPath vPath_WSG_AllianceTunnel_to_AllianceBaseRoof = {
    {1348.02f, 1461.06f, 323.167f, nullptr}, {1359.8f, 1461.49f, 324.527f, nullptr},
    {1372.47f, 1461.61f, 324.354f, nullptr}, {1389.08f, 1461.12f, 325.913f, nullptr},
    {1406.57f, 1460.48f, 330.615f, nullptr}, {1424.04f, 1459.57f, 336.029f, nullptr},
    {1442.5f, 1459.7f, 342.024f, nullptr},   {1471.86f, 1456.65f, 342.794f, nullptr},
    {1470.93f, 1440.5f, 342.794f, nullptr},  {1472.24f, 1427.49f, 342.06f, nullptr},
    {1476.86f, 1412.46f, 341.426f, nullptr}, {1484.42f, 1396.69f, 346.117f, nullptr},
    {1490.7f, 1387.59f, 351.861f, nullptr},  {1500.79f, 1382.98f, 357.784f, nullptr},
    {1511.08f, 1391.29f, 364.444f, nullptr}, {1517.85f, 1403.18f, 370.336f, nullptr},
    {1517.99f, 1417.59f, 371.636f, nullptr}, {1517.07f, 1431.56f, 372.106f, nullptr},
    {1516.66f, 1445.55f, 372.939f, nullptr}, {1514.23f, 1457.37f, 373.689f, nullptr},
    {1503.73f, 1457.67f, 373.684f, nullptr}, {1486.24f, 1457.8f, 373.718f, nullptr},
    {1476.78f, 1460.35f, 373.711f, nullptr}, {1477.37f, 1470.83f, 373.709f, nullptr},
    {1477.5f, 1484.83f, 373.715f, nullptr},  {1480.53f, 1495.26f, 373.721f, nullptr},
    {1492.61f, 1494.72f, 373.721f, nullptr}, {1499.37f, 1489.02f, 373.718f, nullptr},
    {1500.63f, 1472.89f, 373.707f, nullptr},
};

// Alliance Tunnel to Horde Tunnel
BattleBotPath vPath_WSG_AllianceTunnel_to_HordeTunnel = {
    {1129.30f, 1461.03f, 315.206f, nullptr},
    {1149.55f, 1466.57f, 311.031f, nullptr},
    {1196.39f, 1477.25f, 305.697f, nullptr},
    {1227.73f, 1478.38f, 307.418f, nullptr},
    {1267.06f, 1463.40f, 312.227f, nullptr},
    {1303.42f, 1460.18f, 317.257f, nullptr},
};

// Alliance Graveyard (lower) to Horde Flag room
BattleBotPath vPath_WSG_AllianceGraveyardLower_to_HordeFlagRoom = {
    //{1370.71f, 1543.55f, 321.585f, nullptr},
    //{1339.41f, 1533.42f, 313.336f, nullptr},
    {1316.07f, 1533.53f, 315.700f, nullptr},
    {1276.17f, 1533.72f, 311.722f, nullptr},
    {1246.25f, 1533.86f, 307.072f, nullptr},
    {1206.84f, 1528.22f, 307.677f, nullptr},
    {1172.28f, 1523.28f, 301.958f, nullptr},
    {1135.93f, 1505.27f, 308.085f, nullptr},
    {1103.54f, 1521.89f, 314.583f, nullptr},
    {1073.49f, 1551.19f, 319.418f, nullptr},
    {1042.92f, 1530.49f, 336.667f, nullptr},
    {1052.11f, 1493.52f, 342.176f, nullptr},
    {1057.42f, 1452.75f, 341.131f, nullptr},
    {1037.96f, 1422.27f, 339.919f, nullptr},
    {966.01f, 1422.84f, 345.223f, nullptr},
    {942.74f, 1423.10f, 345.467f, nullptr},
    {929.39f, 1434.75f, 345.535f, nullptr},
};

// Horde Graveyard (lower) to Alliance Flag room
BattleBotPath vPath_WSG_HordeGraveyardLower_to_AllianceFlagRoom = {
    //{1096.59f, 1395.07f, 317.016f, nullptr},
    //{1134.38f, 1370.13f, 312.741f, nullptr},
    {1164.96f, 1356.91f, 313.884f, nullptr},
    {1208.32f, 1346.27f, 313.816f, nullptr},
    {1245.06f, 1346.12f, 312.112f, nullptr},
    {1291.43f, 1394.60f, 314.359f, nullptr},
    {1329.33f, 1411.13f, 318.399f, nullptr},
    {1361.57f, 1391.21f, 326.756f, nullptr},
    {1382.19f, 1381.03f, 332.314f, nullptr},
    {1408.06f, 1412.93f, 344.565f, nullptr},
    {1407.88f, 1458.76f, 347.346f, nullptr},
    {1430.40f, 1489.34f, 348.658f, nullptr},
    {1466.64f, 1493.50f, 351.869f, nullptr},
    {1511.51f, 1493.75f, 352.009f, nullptr},
    {1531.44f, 1481.79f, 351.959f, nullptr},
};

// Alliance Graveyard Jump
BattleBotPath vPath_WSG_AllianceGraveyardJump = {
    {1420.08f, 1552.84f, 342.878f, nullptr},
    {1404.06f, 1551.37f, 342.850f, nullptr},
    {1386.24f, 1543.37f, 321.944f, nullptr},
};

// Horde Graveyard Jump
BattleBotPath vPath_WSG_HordeGraveyardJump = {
    {1045.70f, 1389.15f, 340.638f, nullptr},
    {1057.43f, 1390.12f, 339.869f, nullptr},
    {1074.59f, 1400.67f, 323.811f, nullptr},
    {1092.85f, 1399.38f, 317.429f, nullptr},
};

// Alliance Base to Stables
BattleBotPath vPath_AB_AllianceBase_to_Stables = {
    {1285.67f, 1282.14f, -15.8466f, nullptr}, {1272.52f, 1267.83f, -21.7811f, nullptr},
    {1250.44f, 1248.09f, -33.3028f, nullptr}, {1232.56f, 1233.05f, -41.5241f, nullptr},
    {1213.25f, 1224.93f, -47.5513f, nullptr}, {1189.29f, 1219.49f, -53.119f, nullptr},
    {1177.17f, 1210.21f, -56.4593f, nullptr}, {1167.98f, 1202.9f, -56.4743f, nullptr},
};

// Blacksmith to Lumber Mill
BattleBotPath vPath_AB_Blacksmith_to_LumberMill = {
    {967.04f, 1039.03f, -45.091f, nullptr},  {933.67f, 1016.49f, -50.5154f, nullptr},
    {904.02f, 996.63f, -62.3461f, nullptr},  {841.74f, 985.23f, -58.8920f, nullptr},
    {796.25f, 1009.93f, -44.3286f, nullptr}, {781.29f, 1034.49f, -32.887f, nullptr},
    {793.17f, 1107.21f, 5.5663f, nullptr},   {848.98f, 1155.9f, 11.3453f, nullptr},
};

// Blacksmith to GoldMine
BattleBotPath vPath_AB_Blacksmith_to_GoldMine = {
    {1035.98f, 1015.66f, -46.0278f, nullptr}, {1096.86f, 1002.05f, -60.8013f, nullptr},
    {1159.93f, 1003.69f, -63.8378f, nullptr}, {1198.03f, 1064.09f, -65.8385f, nullptr},
    {1218.58f, 1016.96f, -76.9848f, nullptr}, {1192.83f, 956.25f, -93.6974f, nullptr},
    {1162.93f, 908.92f, -108.6703f, nullptr}, {1144.94f, 860.09f, -111.2100f, nullptr},
};

// Farm to Stables
BattleBotPath vPath_AB_Farm_to_Stable = {
    {749.88f, 878.23f, -55.1523f, nullptr},   {819.77f, 931.13f, -57.5882f, nullptr},
    {842.34f, 984.76f, -59.0333f, nullptr},   {863.03f, 1051.47f, -58.0495f, nullptr},
    {899.28f, 1098.27f, -57.4149f, nullptr},  {949.22f, 1153.27f, -54.4464f, nullptr},
    {999.07f, 1189.47f, -49.9125f, nullptr},  {1063.11f, 1211.55f, -53.4164f, nullptr},
    {1098.45f, 1225.47f, -53.1301f, nullptr}, {1146.02f, 1226.34f, -53.8979f, nullptr},
    {1167.10f, 1204.31f, -56.55f, nullptr},
};

// Alliance Base to Gold Mine
BattleBotPath vPath_AB_AllianceBase_to_GoldMine = {
    {1285.67f, 1282.14f, -15.8466f, nullptr}, {1276.41f, 1267.11f, -20.775f, nullptr},
    {1261.34f, 1241.52f, -31.2971f, nullptr}, {1244.91f, 1219.03f, -41.9658f, nullptr},
    {1232.25f, 1184.41f, -50.3348f, nullptr}, {1226.89f, 1150.82f, -55.7935f, nullptr},
    {1224.09f, 1120.38f, -57.0633f, nullptr}, {1220.03f, 1092.72f, -59.1744f, nullptr},
    {1216.05f, 1060.86f, -67.2771f, nullptr}, {1213.77f, 1027.96f, -74.429f, nullptr},
    {1208.56f, 998.394f, -81.9493f, nullptr}, {1197.42f, 969.73f, -89.9385f, nullptr},
    {1185.23f, 944.531f, -97.2433f, nullptr}, {1166.29f, 913.945f, -107.214f, nullptr},
    {1153.17f, 887.863f, -112.34f, nullptr},  {1148.89f, 871.391f, -111.96f, nullptr},
    {1145.24f, 850.82f, -110.514f, nullptr},
};

// Alliance Base to Lumber Mill
BattleBotPath vPath_AB_AllianceBase_to_LumberMill = {
    {1285.67f, 1282.14f, -15.8466f, nullptr}, {1269.13f, 1267.89f, -22.7764f, nullptr},
    {1247.79f, 1249.77f, -33.2518f, nullptr}, {1226.29f, 1232.02f, -43.9193f, nullptr},
    {1196.68f, 1230.15f, -50.4644f, nullptr}, {1168.72f, 1228.98f, -53.9329f, nullptr},
    {1140.82f, 1226.7f, -53.6318f, nullptr},  {1126.85f, 1225.77f, -47.98f, nullptr},
    {1096.5f, 1226.57f, -53.1769f, nullptr},  {1054.52f, 1226.14f, -49.2011f, nullptr},
    {1033.52f, 1226.08f, -45.5968f, nullptr}, {1005.52f, 1226.08f, -43.2912f, nullptr},
    {977.53f, 1226.68f, -40.16f, nullptr},    {957.242f, 1227.94f, -34.1487f, nullptr},
    {930.689f, 1221.57f, -18.9588f, nullptr}, {918.202f, 1211.98f, -12.2494f, nullptr},
    {880.329f, 1192.63f, 7.61168f, nullptr},  {869.965f, 1178.52f, 10.9678f, nullptr},
    {864.74f, 1163.78f, 12.385f, nullptr},    {859.165f, 1148.84f, 11.5289f, nullptr},
};

// Stables to Blacksmith
BattleBotPath vPath_AB_Stables_to_Blacksmith = {
    {1169.52f, 1198.71f, -56.2742f, nullptr}, {1166.93f, 1185.2f, -56.3634f, nullptr},
    {1173.84f, 1170.6f, -56.4094f, nullptr},  {1186.7f, 1163.92f, -56.3961f, nullptr},
    {1189.7f, 1150.68f, -55.8664f, nullptr},  {1185.18f, 1129.31f, -58.1044f, nullptr},
    {1181.7f, 1108.6f, -62.1797f, nullptr},   {1177.92f, 1087.95f, -63.5768f, nullptr},
    {1174.52f, 1067.23f, -64.402f, nullptr},  {1171.27f, 1051.09f, -65.0833f, nullptr},
    {1163.22f, 1031.7f, -64.954f, nullptr},   {1154.25f, 1010.25f, -63.5299f, nullptr},
    {1141.07f, 999.479f, -63.3713f, nullptr}, {1127.12f, 1000.37f, -60.628f, nullptr},
    {1106.17f, 1001.66f, -61.7457f, nullptr}, {1085.64f, 1005.62f, -58.5932f, nullptr},
    {1064.88f, 1008.65f, -52.3547f, nullptr}, {1044.16f, 1011.96f, -47.2647f, nullptr},
    {1029.72f, 1014.88f, -45.3546f, nullptr}, {1013.94f, 1028.7f, -43.9786f, nullptr},
    {990.89f, 1039.3f, -42.7659f, nullptr},   {978.269f, 1043.84f, -44.4588f, nullptr},
};

// Horde Base to Farm
BattleBotPath vPath_AB_HordeBase_to_Farm = {
    {707.259f, 707.839f, -17.5318f, nullptr}, {712.063f, 712.928f, -20.1802f, nullptr},
    {725.941f, 728.682f, -29.7536f, nullptr}, {734.715f, 739.591f, -35.2144f, nullptr},
    {747.607f, 756.161f, -40.899f, nullptr},  {753.994f, 766.668f, -43.3049f, nullptr},
    {758.715f, 787.106f, -46.7014f, nullptr}, {762.077f, 807.831f, -48.4721f, nullptr},
    {764.132f, 821.68f, -49.656f, nullptr},   {767.947f, 839.274f, -50.8574f, nullptr},
    {773.745f, 852.013f, -52.6226f, nullptr}, {785.123f, 869.103f, -54.2089f, nullptr},
    {804.429f, 874.961f, -55.2691f, nullptr},
};

// Horde Base to Gold Mine
BattleBotPath vPath_AB_HordeBase_to_GoldMine = {
    {707.259f, 707.839f, -17.5318f, nullptr}, {717.935f, 716.874f, -23.3941f, nullptr},
    {739.195f, 732.483f, -34.5791f, nullptr}, {757.087f, 742.008f, -38.1123f, nullptr},
    {776.946f, 748.775f, -42.7346f, nullptr}, {797.138f, 754.539f, -46.3237f, nullptr},
    {817.37f, 760.167f, -48.9235f, nullptr},  {837.638f, 765.664f, -49.7374f, nullptr},
    {865.092f, 774.738f, -51.9831f, nullptr}, {878.86f, 777.149f, -47.2361f, nullptr},
    {903.911f, 780.212f, -53.1424f, nullptr}, {923.454f, 787.888f, -54.7937f, nullptr},
    {946.218f, 798.93f, -59.0904f, nullptr},  {978.1f, 813.321f, -66.7268f, nullptr},
    {1002.94f, 817.895f, -77.3119f, nullptr}, {1030.77f, 820.92f, -88.7717f, nullptr},
    {1058.61f, 823.889f, -94.1623f, nullptr}, {1081.6f, 828.32f, -99.4137f, nullptr},
    {1104.64f, 844.773f, -106.387f, nullptr}, {1117.56f, 853.686f, -110.716f, nullptr},
    {1144.9f, 850.049f, -110.522f, nullptr},
};

// Horde Base to Lumber Mill
BattleBotPath vPath_AB_HordeBase_to_LumberMill = {
    {707.259f, 707.839f, -17.5318f, nullptr}, {721.611f, 726.507f, -27.9646f, nullptr},
    {733.846f, 743.573f, -35.8633f, nullptr}, {746.201f, 760.547f, -40.838f, nullptr},
    {758.937f, 787.565f, -46.741f, nullptr},  {761.289f, 801.357f, -48.0037f, nullptr},
    {764.341f, 822.128f, -49.6908f, nullptr}, {769.766f, 842.244f, -51.1239f, nullptr},
    {775.322f, 855.093f, -53.1161f, nullptr}, {783.995f, 874.216f, -55.0822f, nullptr},
    {789.917f, 886.902f, -56.2935f, nullptr}, {798.03f, 906.259f, -57.1162f, nullptr},
    {803.183f, 919.266f, -57.6692f, nullptr}, {813.248f, 937.688f, -57.7106f, nullptr},
    {820.412f, 958.712f, -56.1492f, nullptr}, {814.247f, 973.692f, -50.4602f, nullptr},
    {807.697f, 985.502f, -47.2383f, nullptr}, {795.672f, 1002.69f, -44.9382f, nullptr},
    {784.653f, 1020.77f, -38.6278f, nullptr}, {784.826f, 1037.34f, -31.5719f, nullptr},
    {786.083f, 1051.28f, -24.0793f, nullptr}, {787.314f, 1065.23f, -16.8918f, nullptr},
    {788.892f, 1086.17f, -6.42608f, nullptr}, {792.077f, 1106.53f, 4.81124f, nullptr},
    {800.398f, 1119.48f, 8.5814f, nullptr},   {812.476f, 1131.1f, 10.439f, nullptr},
    {829.704f, 1142.52f, 10.738f, nullptr},   {842.646f, 1143.51f, 11.9984f, nullptr},
    {857.674f, 1146.16f, 11.529f, nullptr},
};

// Farm to Blacksmith
BattleBotPath vPath_AB_Farm_to_Blacksmith = {
    {803.826f, 874.909f, -55.2547f, nullptr}, {808.763f, 887.991f, -57.4437f, nullptr},
    {818.33f, 906.674f, -59.3554f, nullptr},  {828.634f, 924.972f, -60.5664f, nullptr},
    {835.255f, 937.308f, -60.2915f, nullptr}, {845.244f, 955.78f, -60.4208f, nullptr},
    {852.125f, 967.965f, -61.3135f, nullptr}, {863.232f, 983.109f, -62.6402f, nullptr},
    {875.413f, 989.245f, -61.2916f, nullptr}, {895.765f, 994.41f, -63.6287f, nullptr},
    {914.16f, 1001.09f, -58.37f, nullptr},    {932.418f, 1011.44f, -51.9225f, nullptr},
    {944.244f, 1018.92f, -49.1438f, nullptr}, {961.55f, 1030.81f, -45.814f, nullptr},
    {978.122f, 1043.87f, -44.4682f, nullptr},
};

// Stables to Gold Mine
BattleBotPath vPath_AB_Stables_to_GoldMine = {
    {1169.52f, 1198.71f, -56.2742f, nullptr}, {1166.72f, 1183.58f, -56.3633f, nullptr},
    {1172.14f, 1170.99f, -56.4735f, nullptr}, {1185.18f, 1164.02f, -56.4269f, nullptr},
    {1193.98f, 1155.85f, -55.924f, nullptr},  {1201.51f, 1145.65f, -56.4733f, nullptr},
    {1205.39f, 1134.81f, -56.2366f, nullptr}, {1207.57f, 1106.9f, -58.4748f, nullptr},
    {1209.4f, 1085.98f, -63.4022f, nullptr},  {1212.68f, 1065.25f, -66.514f, nullptr},
    {1216.42f, 1037.52f, -72.0457f, nullptr}, {1215.4f, 1011.56f, -78.3338f, nullptr},
    {1209.8f, 992.293f, -83.2433f, nullptr},  {1201.23f, 973.121f, -88.5661f, nullptr},
    {1192.16f, 954.183f, -94.2209f, nullptr}, {1181.88f, 935.894f, -99.5239f, nullptr},
    {1169.86f, 918.68f, -105.588f, nullptr},  {1159.36f, 900.497f, -110.461f, nullptr},
    {1149.32f, 874.429f, -112.142f, nullptr}, {1145.34f, 849.824f, -110.523f, nullptr},
};

// Stables to Lumber Mill
BattleBotPath vPath_AB_Stables_to_LumberMill = {
    {1169.52f, 1198.71f, -56.2742f, nullptr},  {1169.33f, 1203.43f, -56.5457f, nullptr},
    {1164.77f, 1208.73f, -56.1907f, nullptr},  {1141.52f, 1224.99f, -53.8204f, nullptr},
    {1127.54f, 1224.82f, -48.2081f, nullptr},  {1106.56f, 1225.58f, -50.5154f, nullptr},
    {1085.6f, 1226.54f, -53.1863f, nullptr},   {1064.6f, 1226.82f, -50.4381f, nullptr},
    {1043.6f, 1227.27f, -46.5439f, nullptr},   {1022.61f, 1227.72f, -44.7157f, nullptr},
    {1001.61f, 1227.62f, -42.6876f, nullptr},  {980.623f, 1226.93f, -40.4687f, nullptr},
    {959.628f, 1227.1f, -35.3838f, nullptr},   {938.776f, 1226.34f, -23.5399f, nullptr},
    {926.138f, 1217.21f, -16.2176f, nullptr},  {911.966f, 1205.99f, -9.69655f, nullptr},
    {895.135f, 1198.85f, -0.546275f, nullptr}, {873.419f, 1189.27f, 9.3466f, nullptr},
    {863.821f, 1181.72f, 9.76912f, nullptr},   {851.803f, 1166.3f, 10.4423f, nullptr},
    {853.921f, 1150.92f, 11.543f, nullptr},
};

// Farm to Gold Mine
BattleBotPath vPath_AB_Farm_to_GoldMine = {
    {803.826f, 874.909f, -55.2547f, nullptr}, {801.662f, 865.689f, -56.9445f, nullptr},
    {806.433f, 860.776f, -57.5899f, nullptr}, {816.236f, 857.397f, -57.7029f, nullptr},
    {826.717f, 855.846f, -57.9914f, nullptr}, {836.128f, 851.257f, -57.8321f, nullptr},
    {847.933f, 843.837f, -58.1296f, nullptr}, {855.08f, 832.688f, -57.7373f, nullptr},
    {864.513f, 813.663f, -57.574f, nullptr},  {864.229f, 797.762f, -54.2057f, nullptr},
    {862.967f, 787.372f, -53.0276f, nullptr}, {864.163f, 776.33f, -52.0372f, nullptr},
    {872.583f, 777.391f, -48.5342f, nullptr}, {893.575f, 777.922f, -49.1826f, nullptr},
    {915.941f, 783.534f, -53.6598f, nullptr}, {928.105f, 789.929f, -55.4802f, nullptr},
    {946.263f, 800.46f, -59.166f, nullptr},   {958.715f, 806.845f, -62.1494f, nullptr},
    {975.79f, 811.913f, -65.9648f, nullptr},  {989.468f, 814.883f, -71.3089f, nullptr},
    {1010.13f, 818.643f, -80.0817f, nullptr}, {1023.97f, 820.667f, -86.1114f, nullptr},
    {1044.84f, 823.011f, -92.0583f, nullptr}, {1058.77f, 824.482f, -94.1937f, nullptr},
    {1079.13f, 829.402f, -99.1207f, nullptr}, {1092.85f, 836.986f, -102.755f, nullptr},
    {1114.75f, 851.21f, -109.782f, nullptr},  {1128.22f, 851.928f, -111.078f, nullptr},
    {1145.14f, 849.895f, -110.523f, nullptr},
};

// Farm to Lumber Mill
BattleBotPath vPath_AB_Farm_to_LumberMill = {
    {803.826f, 874.909f, -55.2547f, nullptr}, {802.874f, 894.28f, -56.4661f, nullptr},
    {806.844f, 920.39f, -57.3157f, nullptr},  {814.003f, 934.161f, -57.6065f, nullptr},
    {824.594f, 958.47f, -58.4916f, nullptr},  {820.434f, 971.184f, -53.201f, nullptr},
    {808.339f, 987.79f, -47.5705f, nullptr},  {795.98f, 1004.76f, -44.9189f, nullptr},
    {785.497f, 1019.18f, -39.2806f, nullptr}, {783.94f, 1032.46f, -33.5692f, nullptr},
    {784.956f, 1053.41f, -22.8368f, nullptr}, {787.499f, 1074.25f, -12.4232f, nullptr},
    {789.406f, 1088.11f, -5.28606f, nullptr}, {794.617f, 1109.17f, 6.1966f, nullptr},
    {801.514f, 1120.77f, 8.81455f, nullptr},  {817.3f, 1134.59f, 10.6064f, nullptr},
    {828.961f, 1142.98f, 10.7354f, nullptr},  {841.63f, 1147.75f, 11.6916f, nullptr},
    {854.326f, 1150.55f, 11.537f, nullptr},
};

// AV waypoints - ported unchanged from AC (Ai/Base/Actions/BattleGroundTactics.cpp:541-763). Every
// point snaps onto this core's own map-0030 navmesh; only the
// owner encoding in selectObjective's AV case changes (MoP Team vs AC TeamId), not these coordinates.

//* ALLIANCE SIDE WAYPOINTS *//
BattleBotPath vPath_AV_AllianceSpawn_To_AllianceCrossroad1 = {
    {768.306f, -492.228f, 97.862f, nullptr}, {758.694f, -489.560f, 96.219f, nullptr},
    {718.949f, -473.185f, 75.285f, nullptr}, {701.759f, -457.199f, 64.398f, nullptr},
    {691.888f, -424.726f, 63.641f, nullptr}, {663.458f, -408.930f, 67.525f, nullptr},
    {650.696f, -400.027f, 67.948f, nullptr}, {621.638f, -383.551f, 67.511f, nullptr},
    {584.176f, -395.038f, 65.210f, nullptr}, {554.333f, -392.831f, 55.495f, nullptr},
    {518.020f, -415.663f, 43.470f, nullptr}, {444.974f, -429.032f, 27.276f, nullptr},
    {418.297f, -409.409f, 9.173f, nullptr},  {400.182f, -392.450f, -1.197f, nullptr}
};

BattleBotPath vPath_AV_AllianceFortress_To_AllianceCrossroad1 = {
    {654.750f, -32.779f, 48.611f, nullptr},  {639.731f, -46.509f, 44.072f, nullptr},
    {630.953f, -64.360f, 41.341f, nullptr},  {634.789f, -85.583f, 41.495f, nullptr},
    {628.271f, -100.943f, 40.328f, nullptr}, {621.330f, -129.900f, 33.687f, nullptr},
    {620.615f, -153.156f, 33.606f, nullptr}, {622.186f, -171.443f, 36.848f, nullptr},
    {624.456f, -188.047f, 38.569f, nullptr}, {629.185f, -222.637f, 38.362f, nullptr},
    {633.239f, -252.286f, 34.275f, nullptr}, {635.941f, -272.053f, 30.130f, nullptr},
    {632.884f, -294.946f, 30.267f, nullptr}, {625.348f, -322.407f, 30.133f, nullptr},
    {608.386f, -335.968f, 30.347f, nullptr}, {588.856f, -331.991f, 30.485f, nullptr},
    {560.140f, -327.872f, 17.396f, nullptr}, {530.523f, -324.481f, 1.718f, nullptr},
    {511.340f, -329.767f, -1.084f, nullptr}, {497.221f, -341.494f, -1.184f, nullptr},
    {465.281f, -365.577f, -1.243f, nullptr}, {437.839f, -376.473f, -1.243f, nullptr},
    {414.292f, -384.557f, -1.243f, nullptr}
};

BattleBotPath vPath_AV_AllianceCrossroad1_To_AllianceCrossroad2 = {
    {391.160f, -391.667f, -1.244f, nullptr}, {369.595f, -388.874f, -0.153f, nullptr},
    {334.986f, -384.282f, -0.726f, nullptr}, {308.373f, -383.026f, -0.336f, nullptr},
    {287.175f, -386.773f, 5.531f, nullptr},  {276.595f, -394.487f, 12.126f, nullptr},
    {264.248f, -405.109f, 23.509f, nullptr}, {253.142f, -412.069f, 31.665f, nullptr},
    {237.243f, -416.570f, 37.103f, nullptr}, {216.805f, -416.987f, 40.497f, nullptr},
    {195.369f, -407.750f, 42.876f, nullptr}, {179.619f, -402.642f, 42.793f, nullptr},
    {157.154f, -391.535f, 43.616f, nullptr}, {137.928f, -380.956f, 43.018f, nullptr},
    {117.990f, -380.243f, 43.571f, nullptr}
};

BattleBotPath vPath_AV_StoneheartGrave_To_AllianceCrossroad2 = {
    {73.141f, -481.863f, 48.479f, nullptr}, {72.974f, -461.923f, 48.498f, nullptr},
    {73.498f, -443.377f, 48.989f, nullptr}, {74.061f, -423.435f, 49.235f, nullptr},
    {83.086f, -403.871f, 47.412f, nullptr}, {102.003f, -388.023f, 45.095f, nullptr}
};

BattleBotPath vPath_AV_AllianceCrossroad2_To_StoneheartBunker = {
    {113.412f, -381.154f, 44.324f, nullptr},  {87.868f, -389.039f, 45.025f, nullptr},
    {62.170f, -388.790f, 45.012f, nullptr},   {30.645f, -390.873f, 45.644f, nullptr},
    {6.239f, -410.405f, 45.157f, nullptr},    {-11.185f, -423.994f, 45.222f, nullptr},
    {-19.595f, -437.877f, 46.383f, nullptr},  {-31.747f, -451.316f, 45.500f, nullptr},
    {-49.279f, -466.706f, 41.101f, nullptr},  {-78.427f, -459.899f, 27.648f, nullptr},
    {-103.623f, -467.766f, 24.806f, nullptr}, {-113.718f, -476.527f, 25.802f, nullptr},
    {-129.793f, -481.160f, 27.735f, nullptr}
};

BattleBotPath vPath_AV_AllianceCrossroad2_To_AllianceCaptain = {
    {117.948f, -363.836f, 43.485f, nullptr}, {110.096f, -342.635f, 41.331f, nullptr},
    {92.672f, -320.463f, 35.312f, nullptr},  {75.147f, -303.007f, 29.411f, nullptr},
    {59.694f, -293.137f, 24.670f, nullptr},  {41.342f, -293.374f, 17.193f, nullptr},
    {22.388f, -299.599f, 14.204f, nullptr}
};

BattleBotPath vPath_AV_AllianceCrossroads3_To_SnowfallGraveyard = {
    {-141.090f, -248.599f, 6.746f, nullptr},  {-147.551f, -233.354f, 9.675f, nullptr},
    {-153.353f, -219.665f, 17.267f, nullptr}, {-159.091f, -205.990f, 26.091f, nullptr},
    {-164.041f, -188.716f, 35.813f, nullptr}, {-170.634f, -166.838f, 51.540f, nullptr},
    {-183.338f, -154.159f, 64.252f, nullptr}, {-193.333f, -139.166f, 74.581f, nullptr},
    {-199.853f, -124.194f, 78.247f, nullptr}
};

BattleBotPath vPath_AV_AllianceCaptain_To_AllianceCrossroad3 = {
    {31.023f, -290.783f, 15.966f, nullptr},   {31.857f, -270.165f, 16.040f, nullptr},
    {26.531f, -242.488f, 14.158f, nullptr},   {3.448f, -241.318f, 11.900f, nullptr},
    {-24.158f, -233.744f, 9.802f, nullptr},   {-55.556f, -235.327f, 10.038f, nullptr},
    {-93.670f, -255.273f, 6.264f, nullptr},   {-117.164f, -263.636f, 6.363f, nullptr}
};

BattleBotPath vPath_AV_AllianceCaptain_To_HordeCrossroad3 = {
    {31.023f, -290.783f, 15.966f, nullptr}, {31.857f, -270.165f, 16.040f, nullptr},
    {26.531f, -242.488f, 14.158f, nullptr}, {3.448f, -241.318f, 11.900f, nullptr},
    {-24.158f, -233.744f, 9.802f, nullptr}, {-55.556f, -235.327f, 10.038f, nullptr},
    {-93.670f, -255.273f, 6.264f, nullptr}, {-117.164f, -263.636f, 6.363f, nullptr},
    {-154.433f, -272.428f, 8.016f, nullptr},  {-165.219f, -277.141f, 9.138f, nullptr},
    {-174.154f, -281.561f, 7.062f, nullptr},  {-189.765f, -290.755f, 6.668f, nullptr},
    {-213.121f, -303.227f, 6.668f, nullptr},  {-240.497f, -315.013f, 6.668f, nullptr},
    {-260.829f, -329.400f, 6.677f, nullptr},  {-280.652f, -344.530f, 6.668f, nullptr},
    {-305.708f, -363.655f, 6.668f, nullptr},  {-326.363f, -377.530f, 6.668f, nullptr},
    {-347.706f, -377.546f, 11.493f, nullptr}, {-361.965f, -373.012f, 13.904f, nullptr},
    {-379.646f, -367.389f, 16.907f, nullptr}, {-399.610f, -353.045f, 17.793f, nullptr},
    {-411.324f, -335.019f, 17.564f, nullptr}, {-421.800f, -316.128f, 17.843f, nullptr},
    {-435.855f, -293.863f, 19.553f, nullptr}, {-454.279f, -277.457f, 21.943f, nullptr},
    {-473.868f, -280.536f, 24.837f, nullptr}, {-492.305f, -289.846f, 29.787f, nullptr},
    {-504.724f, -313.745f, 31.938f, nullptr}, {-518.431f, -333.087f, 34.017f, nullptr}
};

BattleBotPath vPath_AV_StoneheartBunker_To_HordeCrossroad3 = {
    {-507.656f, -342.031f, 33.079f, nullptr}, {-490.580f, -348.193f, 29.170f, nullptr},
    {-458.194f, -343.101f, 31.685f, nullptr}, {-441.632f, -329.773f, 19.349f, nullptr},
    {-429.951f, -333.153f, 18.078f, nullptr}, {-415.858f, -341.834f, 17.865f, nullptr},
    {-406.698f, -361.322f, 17.764f, nullptr}, {-394.097f, -376.775f, 16.309f, nullptr},
    {-380.107f, -393.161f, 10.662f, nullptr}, {-359.474f, -405.379f, 10.437f, nullptr},
    {-343.166f, -415.036f, 10.177f, nullptr}, {-328.328f, -423.823f, 11.057f, nullptr},
    {-315.454f, -431.447f, 17.709f, nullptr}, {-296.180f, -449.228f, 22.316f, nullptr},
    {-272.700f, -459.303f, 28.764f, nullptr}, {-262.987f, -457.030f, 30.248f, nullptr},
    {-244.145f, -452.620f, 25.754f, nullptr}, {-221.311f, -448.195f, 25.166f, nullptr},
    {-201.762f, -457.441f, 27.413f, nullptr}, {-172.130f, -474.654f, 28.884f, nullptr},
    {-153.135f, -480.692f, 30.459f, nullptr}, {-137.077f, -478.766f, 28.798f, nullptr}
};

BattleBotPath vPath_AV_AllianceCrossroad1_To_AllianceMine = {
    {414.329f, -386.483f, -1.244f, nullptr}, {428.252f, -380.010f, -1.244f, nullptr},
    {446.203f, -372.263f, -1.244f, nullptr}, {468.352f, -360.936f, -1.244f, nullptr},
    {490.502f, -344.252f, -1.228f, nullptr}, {515.565f, -328.078f, -1.085f, nullptr},
    {531.431f, -323.435f, 1.906f, nullptr},  {548.612f, -324.795f, 10.162f, nullptr},
    {565.412f, -326.485f, 20.217f, nullptr}, {579.313f, -331.417f, 28.435f, nullptr},
    {593.026f, -336.281f, 30.215f, nullptr}, {614.580f, -331.387f, 30.236f, nullptr},
    {626.524f, -309.520f, 30.375f, nullptr}, {642.822f, -290.896f, 30.201f, nullptr},
    {661.428f, -285.810f, 29.862f, nullptr}, {670.488f, -281.637f, 27.951f, nullptr},
    {687.522f, -273.793f, 23.510f, nullptr}, {706.938f, -272.231f, 31.122f, nullptr},
    {725.715f, -281.845f, 40.529f, nullptr}, {733.208f, -296.224f, 47.418f, nullptr},
    {742.182f, -308.196f, 52.947f, nullptr}, {749.742f, -319.208f, 56.275f, nullptr},
    {760.992f, -335.660f, 60.279f, nullptr}, {783.734f, -342.468f, 61.410f, nullptr},
    {791.843f, -330.986f, 63.040f, nullptr}, {800.683f, -338.452f, 63.286f, nullptr},
    {812.383f, -337.218f, 64.698f, nullptr}, {826.326f, -331.789f, 64.492f, nullptr},
    {839.641f, -338.601f, 65.461f, nullptr}, {851.763f, -345.123f, 65.935f, nullptr},
    {871.511f, -345.464f, 64.947f, nullptr}, {882.320f, -340.819f, 66.864f, nullptr},
    {898.911f, -333.166f, 67.532f, nullptr}, {910.825f, -334.185f, 66.889f, nullptr},
    {922.109f, -336.746f, 66.120f, nullptr}, {929.915f, -353.314f, 65.902f, nullptr},
    {918.736f, -367.359f, 66.307f, nullptr}, {920.196f, -380.444f, 62.599f, nullptr},
    {920.625f, -400.310f, 59.454f, nullptr}, {920.521f, -410.085f, 57.002f, nullptr},
    {917.319f, -424.261f, 56.972f, nullptr}, {909.302f, -429.930f, 58.459f, nullptr},
    {893.164f, -430.083f, 55.782f, nullptr}, {873.846f, -425.108f, 51.387f, nullptr},
    {860.920f, -421.705f, 51.032f, nullptr}, {839.651f, -412.104f, 47.572f, nullptr},
    {831.515f, -410.477f, 47.778f, nullptr}
};
//* ALLIANCE SIDE WAYPOINTS *//

//* HORDE SIDE WAYPOINTS *//
BattleBotPath vPath_AV_HordeSpawn_To_MainRoad = {
    {-1366.182f, -532.170f, 53.354f, nullptr}, {-1325.007f, -516.055f, 51.554f, nullptr},
    {-1273.492f, -514.574f, 50.360f, nullptr}, {-1232.875f, -515.097f, 51.085f, nullptr},
    {-1201.299f, -500.807f, 51.665f, nullptr}, {-1161.199f, -476.367f, 54.956f, nullptr},
    {-1135.679f, -469.551f, 56.934f, nullptr}, {-1113.439f, -458.271f, 52.196f, nullptr},
    {-1086.750f, -444.735f, 52.903f, nullptr}, {-1061.950f, -434.380f, 51.396f, nullptr},
    {-1031.777f, -424.596f, 51.262f, nullptr}, {-967.556f, -399.110f, 49.213f, nullptr}
};

BattleBotPath vPath_AV_SnowfallGraveyard_To_HordeCaptain = {
    {-213.992f, -103.451f, 79.389f, nullptr}, {-222.690f, -95.820f, 77.588f, nullptr},
    {-237.377f, -88.173f, 65.871f, nullptr},  {-253.605f, -85.059f, 56.342f, nullptr},
    {-272.886f, -94.676f, 42.306f, nullptr},  {-289.627f, -108.123f, 26.978f, nullptr},
    {-304.265f, -105.120f, 20.341f, nullptr}, {-316.857f, -92.162f, 22.999f, nullptr},
    {-332.066f, -75.537f, 27.062f, nullptr},  {-358.184f, -76.459f, 27.212f, nullptr},
    {-388.166f, -81.693f, 23.836f, nullptr},  {-407.733f, -90.216f, 23.385f, nullptr},
    {-420.646f, -122.109f, 23.955f, nullptr}, {-418.916f, -143.640f, 24.135f, nullptr},
    {-419.211f, -171.836f, 24.088f, nullptr}, {-425.798f, -195.843f, 26.290f, nullptr},
    {-445.352f, -195.483f, 35.300f, nullptr}, {-464.614f, -194.387f, 49.409f, nullptr},
    {-477.910f, -193.219f, 54.985f, nullptr}, {-488.230f, -187.985f, 56.729f, nullptr}
};

BattleBotPath vPath_AV_HordeCrossroad3_To_IcebloodTower = {
    {-533.792f, -341.435f, 35.860f, nullptr}, {-551.527f, -332.298f, 38.432f, nullptr},
    {-574.093f, -312.653f, 44.791f, nullptr}
};

BattleBotPath vPath_AV_IcebloodTower_To_HordeCaptain = {
    {-569.690f, -295.928f, 49.096f, nullptr}, {-559.809f, -282.641f, 52.074f, nullptr},
    {-546.890f, -261.488f, 53.194f, nullptr}, {-529.471f, -236.931f, 56.746f, nullptr},
    {-518.182f, -222.736f, 56.922f, nullptr}, {-500.372f, -205.938f, 57.364f, nullptr},
    {-494.455f, -190.473f, 57.190f, nullptr}
};

BattleBotPath vPath_AV_IcebloodTower_To_IcebloodGrave = {
    {-584.305f, -313.025f, 47.651f, nullptr}, {-600.831f, -327.032f, 51.026f, nullptr},
    {-613.276f, -343.187f, 54.958f, nullptr}, {-625.873f, -364.812f, 56.829f, nullptr},
    {-625.494f, -390.816f, 58.781f, nullptr}
};

BattleBotPath vPath_AV_IcebloodGrave_To_TowerBottom = {
    {-635.524f, -393.738f, 59.527f, nullptr}, {-659.484f, -386.214f, 63.131f, nullptr},
    {-679.221f, -374.851f, 65.710f, nullptr}, {-694.579f, -368.145f, 66.017f, nullptr},
    {-726.698f, -346.235f, 66.804f, nullptr}, {-743.446f, -345.899f, 66.566f, nullptr},
    {-754.564f, -344.804f, 67.422f, nullptr}
};

BattleBotPath vPath_AV_TowerBottom_To_HordeCrossroad1 = {
    {-764.722f, -339.262f, 67.669f, nullptr},  {-777.559f, -338.964f, 66.287f, nullptr},
    {-796.674f, -341.982f, 61.848f, nullptr},  {-812.721f, -346.317f, 53.286f, nullptr},
    {-826.586f, -350.765f, 50.140f, nullptr},  {-842.410f, -355.642f, 49.750f, nullptr},
    {-861.475f, -361.517f, 50.514f, nullptr},  {-878.634f, -366.805f, 49.987f, nullptr},
    {-897.097f, -374.362f, 48.931f, nullptr},  {-920.177f, -383.808f, 49.487f, nullptr},
    {-947.087f, -392.660f, 48.533f, nullptr},  {-977.268f, -395.606f, 49.426f, nullptr},
    {-993.685f, -394.251f, 50.180f, nullptr},  {-1016.760f, -390.774f, 50.955f, nullptr},
    {-1042.994f, -383.854f, 50.904f, nullptr}, {-1066.925f, -377.541f, 52.535f, nullptr},
    {-1103.309f, -365.939f, 51.502f, nullptr}, {-1127.469f, -354.968f, 51.502f, nullptr}
};

BattleBotPath vPath_AV_HordeCrossroad1_To_FrostwolfGrave = {
    {-1127.565f, -340.254f, 51.753f, nullptr}, {-1112.843f, -337.645f, 53.368f, nullptr},
    {-1089.873f, -334.993f, 54.580f, nullptr}
};

BattleBotPath vPath_AV_HordeCrossroad1_To_HordeFortress = {
    {-1140.070f, -349.834f, 51.090f, nullptr}, {-1161.807f, -352.447f, 51.782f, nullptr},
    {-1182.047f, -361.856f, 52.458f, nullptr}, {-1203.318f, -365.951f, 54.427f, nullptr},
    {-1228.105f, -367.462f, 58.155f, nullptr}, {-1243.013f, -357.409f, 59.866f, nullptr},
    {-1245.382f, -337.206f, 59.322f, nullptr}, {-1236.790f, -323.051f, 60.500f, nullptr},
    {-1227.642f, -311.981f, 63.269f, nullptr}, {-1217.120f, -299.546f, 69.058f, nullptr},
    {-1207.536f, -288.218f, 71.742f, nullptr}, {-1198.808f, -270.859f, 72.431f, nullptr},
    {-1203.695f, -255.038f, 72.498f, nullptr}, {-1220.292f, -252.715f, 73.243f, nullptr},
    {-1236.539f, -252.215f, 73.326f, nullptr}, {-1246.512f, -257.577f, 73.326f, nullptr},
    {-1257.051f, -272.847f, 73.018f, nullptr}, {-1265.600f, -284.769f, 77.939f, nullptr},
    {-1282.656f, -290.696f, 88.334f, nullptr}, {-1292.589f, -290.809f, 90.446f, nullptr},
    {-1307.385f, -290.950f, 90.681f, nullptr}, {-1318.955f, -291.061f, 90.451f, nullptr},
    {-1332.717f, -291.117f, 90.806f, nullptr}, {-1346.880f, -287.015f, 91.066f, nullptr}
};

BattleBotPath vPath_AV_FrostwolfGrave_To_HordeMine = {
    {-1075.070f, -332.081f, 55.758f, nullptr}, {-1055.926f, -326.469f, 57.026f, nullptr},
    {-1036.115f, -328.239f, 59.368f, nullptr}, {-1018.217f, -332.306f, 59.335f, nullptr},
    {-990.396f, -337.397f, 58.342f, nullptr},  {-963.987f, -335.529f, 60.945f, nullptr},
    {-954.477f, -321.258f, 63.429f, nullptr},  {-951.052f, -301.476f, 64.761f, nullptr},
    {-955.257f, -282.794f, 63.683f, nullptr},  {-960.355f, -261.040f, 64.498f, nullptr},
    {-967.410f, -230.933f, 67.408f, nullptr},  {-967.187f, -207.444f, 68.924f, nullptr}
};
//* HORDE SIDE WAYPOINTS *//

BattleBotPath vPath_EY_Horde_Spawn_to_Crossroad1Horde = {
    {1809.102f, 1540.854f, 1267.142f, nullptr}, {1832.335f, 1539.495f, 1256.417f, nullptr},
    {1846.995f, 1539.792f, 1243.077f, nullptr}, {1846.243f, 1530.716f, 1238.477f, nullptr},
    {1883.154f, 1532.143f, 1202.143f, nullptr}, {1941.452f, 1549.086f, 1176.700f, nullptr}};

BattleBotPath vPath_EY_Horde_Crossroad1Horde_to_Crossroad2Horde = {{1951.647f, 1545.187f, 1174.831f, nullptr},
                                                                   {1992.266f, 1546.962f, 1169.816f, nullptr},
                                                                   {2045.865f, 1543.925f, 1163.759f, nullptr}};

BattleBotPath vPath_EY_Crossroad1Horde_to_Blood_Elf_Tower = {{1952.907f, 1539.857f, 1174.638f, nullptr},
                                                             {2000.130f, 1508.182f, 1169.778f, nullptr},
                                                             {2044.239f, 1483.860f, 1166.165f, nullptr},
                                                             {2048.773f, 1389.578f, 1193.903f, nullptr}};

BattleBotPath vPath_EY_Crossroad1Horde_to_Fel_Reaver_Ruins = {{1944.301f, 1557.170f, 1176.370f, nullptr},
                                                              {1992.953f, 1625.188f, 1173.616f, nullptr},
                                                              {2040.421f, 1676.989f, 1177.079f, nullptr},
                                                              {2045.527f, 1736.398f, 1189.661f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Horde_to_Blood_Elf_Tower = {{2049.363f, 1532.337f, 1163.178f, nullptr},
                                                             {2050.149f, 1484.721f, 1165.099f, nullptr},
                                                             {2046.865f, 1423.937f, 1188.882f, nullptr},
                                                             {2048.478f, 1389.491f, 1193.878f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Horde_to_Fel_Reaver_Ruins = {{2052.267f, 1555.692f, 1163.147f, nullptr},
                                                              {2047.684f, 1614.272f, 1165.397f, nullptr},
                                                              {2045.993f, 1668.937f, 1174.978f, nullptr},
                                                              {2044.286f, 1733.128f, 1189.739f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Horde_to_Flag = {{2059.276f, 1546.143f, 1162.394f, nullptr},
                                                  {2115.978f, 1559.244f, 1156.362f, nullptr},
                                                  {2149.140f, 1556.570f, 1158.412f, nullptr},
                                                  {2170.601f, 1567.113f, 1159.456f, nullptr}};

BattleBotPath vPath_EY_Alliance_Spawn_to_Crossroad1Alliance = {
    {2502.110f, 1604.330f, 1260.750f, nullptr}, {2497.077f, 1596.198f, 1257.302f, nullptr},
    {2483.930f, 1597.062f, 1244.660f, nullptr}, {2486.549f, 1617.651f, 1225.837f, nullptr},
    {2449.150f, 1601.792f, 1201.552f, nullptr}, {2395.737f, 1588.287f, 1176.570f, nullptr}};

BattleBotPath vPath_EY_Alliance_Crossroad1Alliance_to_Crossroad2Alliance = {
    {2380.262f, 1586.757f, 1173.567f, nullptr},
    {2333.956f, 1586.052f, 1169.873f, nullptr},
    {2291.210f, 1591.435f, 1166.048f, nullptr},
};

BattleBotPath vPath_EY_Crossroad1Alliance_to_Mage_Tower = {{2380.973f, 1593.445f, 1173.189f, nullptr},
                                                           {2335.762f, 1621.922f, 1169.007f, nullptr},
                                                           {2293.526f, 1643.972f, 1166.501f, nullptr},
                                                           {2288.198f, 1688.568f, 1172.790f, nullptr},
                                                           {2284.286f, 1737.889f, 1189.708f, nullptr}};

BattleBotPath vPath_EY_Crossroad1Alliance_to_Draenei_Ruins = {{2388.687f, 1576.089f, 1175.975f, nullptr},
                                                              {2354.921f, 1522.763f, 1176.060f, nullptr},
                                                              {2300.056f, 1459.208f, 1184.181f, nullptr},
                                                              {2289.880f, 1415.640f, 1196.755f, nullptr},
                                                              {2279.870f, 1387.461f, 1195.003f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Alliance_to_Mage_Tower = {{2282.525f, 1597.721f, 1164.553f, nullptr},
                                                           {2281.028f, 1651.310f, 1165.426f, nullptr},
                                                           {2284.633f, 1736.082f, 1189.708f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Alliance_to_Draenei_Ruins = {{2282.487f, 1581.630f, 1165.318f, nullptr},
                                                              {2284.728f, 1525.618f, 1170.812f, nullptr},
                                                              {2287.697f, 1461.228f, 1183.450f, nullptr},
                                                              {2290.861f, 1413.606f, 1197.115f, nullptr}};

BattleBotPath vPath_EY_Crossroad2Alliance_to_Flag = {{2275.622f, 1586.123f, 1164.469f, nullptr},
                                                     {2221.334f, 1575.123f, 1158.277f, nullptr},
                                                     {2178.372f, 1572.144f, 1159.462f, nullptr}};

BattleBotPath vPath_EY_Draenei_Ruins_to_Blood_Elf_Tower = {
    {2287.925f, 1406.976f, 1197.004f, nullptr}, {2283.283f, 1454.769f, 1184.243f, nullptr},
    {2237.519f, 1398.161f, 1178.191f, nullptr}, {2173.150f, 1388.084f, 1170.185f, nullptr},
    {2105.039f, 1381.507f, 1162.911f, nullptr}, {2074.315f, 1404.387f, 1178.141f, nullptr},
    {2047.649f, 1411.681f, 1192.032f, nullptr}, {2049.197f, 1387.392f, 1193.799f, nullptr}};

BattleBotPath vPath_EY_Fel_Reaver_to_Mage_Tower = {
    {2044.519f, 1726.113f, 1189.395f, nullptr}, {2045.408f, 1682.986f, 1177.574f, nullptr},
    {2097.595f, 1736.117f, 1170.419f, nullptr}, {2158.866f, 1746.998f, 1161.184f, nullptr},
    {2220.635f, 1757.837f, 1151.886f, nullptr}, {2249.922f, 1721.807f, 1161.550f, nullptr},
    {2281.021f, 1694.735f, 1174.020f, nullptr}, {2284.522f, 1728.234f, 1189.015f, nullptr}};
// Isle of Conquest (map 628), ported 1:1 from AC (Ai/Base/Actions/BattleGroundTactics.cpp:848-1087).
// The waypoint data is unchanged; see IOC-PORT-NOTES.md for the navmesh check against this core's
// mmaps/0628_*.mmtile.
BattleBotPath vPath_IC_Ally_Keep_to_Ally_Front_Crossroad = {
    //{ 351.652f, -834.837f, 48.916f, nullptr },
    {434.768f, -833.976f, 46.090f, nullptr},
    {506.782f, -828.594f, 24.313f, nullptr},
    {524.955f, -799.002f, 19.498f, nullptr}};

BattleBotPath vPath_IC_Ally_Front_Crossroad_to_Workshop = {
    {524.955f, -799.002f, 19.498f, nullptr}, {573.557f, -804.838f, 9.6291f, nullptr},
    {627.977f, -810.197f, 3.5154f, nullptr}, {681.501f, -805.208f, 3.1464f, nullptr},
    {721.905f, -797.917f, 4.5112f, nullptr}, {774.466f, -801.058f, 6.3428f, nullptr}};

BattleBotPath vPath_IC_Ally_Keep_to_Ally_Dock_Crossroad = {{434.768f, -833.976f, 46.090f, nullptr},
                                                           {446.710f, -776.008f, 48.783f, nullptr},
                                                           {463.745f, -742.368f, 48.584f, nullptr},
                                                           {488.201f, -714.563f, 36.564f, nullptr},
                                                           {525.923f, -666.880f, 25.425f, nullptr}};

BattleBotPath vPath_IC_Ally_Front_Crossroad_to_Ally_Dock_Crossroad = {{524.955f, -799.002f, 19.498f, nullptr},
                                                                      {542.225f, -745.142f, 18.348f, nullptr},
                                                                      {545.309f, -712.497f, 22.005f, nullptr},
                                                                      {538.678f, -748.361f, 18.261f, nullptr},
                                                                      {525.923f, -666.880f, 25.425f, nullptr}};

BattleBotPath vPath_IC_Lower_Graveyard_to_Lower_Graveyard_Crossroad = {{443.095f, -310.797f, 51.749f, nullptr},
                                                                       {462.733f, -323.587f, 48.706f, nullptr},
                                                                       {471.540f, -343.914f, 40.706f, nullptr},
                                                                       {475.622f, -360.728f, 34.384f, nullptr},
                                                                       {484.458f, -379.796f, 33.122f, nullptr}};

BattleBotPath vPath_IC_Lower_Graveyard_Crossroad_to_Ally_Docks_Crossroad = {
    {484.458f, -379.796f, 33.122f, nullptr}, {509.786f, -380.592f, 33.122f, nullptr},
    {532.549f, -381.576f, 33.122f, nullptr}, {553.506f, -386.102f, 33.507f, nullptr},
    {580.533f, -398.536f, 33.416f, nullptr}, {605.112f, -409.843f, 33.121f, nullptr},
    {619.212f, -419.169f, 33.121f, nullptr}, {631.702f, -428.763f, 33.070f, nullptr},
    {648.483f, -444.714f, 28.629f, nullptr}};

BattleBotPath vPath_IC_Lower_Graveyard_Crossroad_to_Ally_Docks_Second_Crossroad = {
    {484.458f, -379.796f, 33.122f, nullptr}, {470.771f, -394.789f, 33.112f, nullptr},
    {461.191f, -409.475f, 33.120f, nullptr}, {452.794f, -431.842f, 33.120f, nullptr},
    {452.794f, -456.896f, 33.658f, nullptr}, {453.279f, -481.742f, 33.052f, nullptr},
    {453.621f, -504.979f, 32.956f, nullptr}, {452.006f, -526.792f, 32.221f, nullptr},
    {453.150f, -548.212f, 29.133f, nullptr}, {455.224f, -571.323f, 26.119f, nullptr},
    {465.486f, -585.424f, 25.756f, nullptr}, {475.366f, -598.414f, 25.784f, nullptr},
    {477.702f, -605.757f, 25.714f, nullptr}};

BattleBotPath vPath_IC_Ally_Dock_Crossroad_to_Ally_Docks_Second_Crossroad = {{525.923f, -666.880f, 25.425f, nullptr},
                                                                             {497.190f, -630.709f, 25.626f, nullptr},
                                                                             {477.702f, -605.757f, 25.714f, nullptr}};

BattleBotPath vPath_IC_Ally_Docks_Second_Crossroad_to_Ally_Docks_Crossroad = {{477.702f, -605.757f, 25.714f, nullptr},
                                                                              {493.697f, -555.838f, 26.014f, nullptr},
                                                                              {522.939f, -525.199f, 26.014f, nullptr},
                                                                              {580.398f, -486.274f, 26.013f, nullptr},
                                                                              {650.132f, -445.811f, 28.503f, nullptr}};

BattleBotPath vPath_IC_Ally_Docks_Crossroad_to_Docks_Flag = {{650.132f, -445.811f, 28.503f, nullptr},
                                                             {690.527f, -452.961f, 18.039f, nullptr},
                                                             {706.813f, -430.003f, 13.797f, nullptr},
                                                             {726.427f, -364.849f, 17.815f, nullptr}};

BattleBotPath vPath_IC_Docks_Graveyard_to_Docks_Flag = {
    {638.142f, -283.782f, 11.512f, nullptr}, {655.760f, -284.433f, 13.220f, nullptr},
    {661.656f, -299.912f, 12.756f, nullptr}, {675.068f, -317.192f, 12.627f, nullptr},
    {692.712f, -323.866f, 12.686f, nullptr}, {712.968f, -341.285f, 13.350f, nullptr},
    {726.427f, -364.849f, 17.815f, nullptr}};

BattleBotPath vPath_IC_Ally_Keep_to_Quarry_Crossroad = {
    {320.547f, -919.896f, 48.481f, nullptr}, {335.384f, -922.371f, 49.518f, nullptr},
    {353.471f, -920.316f, 48.660f, nullptr}, {353.305f, -958.823f, 47.665f, nullptr},
    {369.196f, -989.960f, 37.719f, nullptr}, {380.671f, -1023.51f, 29.369f, nullptr}};

BattleBotPath vPath_IC_Quarry_Crossroad_to_Quarry_Flag = {
    {380.671f, -1023.51f, 29.369f, nullptr}, {361.584f, -1052.89f, 27.445f, nullptr},
    {341.853f, -1070.17f, 24.024f, nullptr}, {295.845f, -1075.22f, 16.164f, nullptr},
    {253.030f, -1094.06f, 4.1517f, nullptr}, {211.391f, -1121.80f, 1.9591f, nullptr},
    {187.836f, -1155.66f, 1.9749f, nullptr}, {221.193f, -1186.87f, 8.0247f, nullptr},
    {249.181f, -1162.09f, 16.687f, nullptr}};

BattleBotPath vPath_IC_Ally_Front_Crossroad_to_Hangar_First_Crossroad = {{524.955f, -799.002f, 19.498f, nullptr},
                                                                         {512.563f, -840.166f, 23.913f, nullptr},
                                                                         {513.418f, -877.726f, 26.333f, nullptr},
                                                                         {512.962f, -945.951f, 39.382f, nullptr}};

BattleBotPath vPath_IC_Ally_Keep_to_Hangar_First_Crossroad = {{434.768f, -833.976f, 46.090f, nullptr},
                                                              {486.355f, -909.736f, 26.112f, nullptr},
                                                              {512.962f, -945.951f, 39.382f, nullptr}};

BattleBotPath vPath_IC_Hangar_First_Crossroad_to_Hangar_Second_Crossroad = {{512.962f, -945.951f, 39.382f, nullptr},
                                                                            {499.525f, -985.850f, 47.659f, nullptr},
                                                                            {492.794f, -1016.36f, 49.834f, nullptr},
                                                                            {481.738f, -1052.67f, 60.190f, nullptr}};

BattleBotPath vPath_IC_Quarry_Crossroad_to_Hangar_Second_Crossroad = {{380.671f, -1023.51f, 29.369f, nullptr},
                                                                      {430.997f, -1021.72f, 31.021f, nullptr},
                                                                      {439.528f, -1044.88f, 41.827f, nullptr},
                                                                      {455.062f, -1060.67f, 67.209f, nullptr},
                                                                      {481.738f, -1052.67f, 60.190f, nullptr}};

BattleBotPath vPath_IC_Hangar_Second_Crossroad_to_Hangar_Flag = {
    {508.945f, -1103.30f, 79.054f, nullptr}, {536.397f, -1145.79f, 95.478f, nullptr},
    {573.242f, -1138.19f, 109.26f, nullptr}, {609.051f, -1112.93f, 128.31f, nullptr},
    {645.569f, -1094.58f, 132.13f, nullptr}, {689.621f, -1068.33f, 132.87f, nullptr},
    {730.045f, -1042.67f, 133.03f, nullptr}, {755.322f, -1030.28f, 133.30f, nullptr},
    {801.685f, -1005.46f, 132.39f, nullptr}, {806.404f, -1001.709f, 132.382f, nullptr}};

BattleBotPath vPath_IC_Horde_Keep_to_Horde_Front_Crossroad = {{1128.646f, -763.221f, 48.385f, nullptr},
                                                              {1091.273f, -763.619f, 42.352f, nullptr},
                                                              {1032.825f, -763.024f, 30.420f, nullptr},
                                                              {991.4235f, -807.672f, 21.788f, nullptr}};

BattleBotPath vPath_IC_Horde_Front_Crossroad_to_Horde_Hangar_Crossroad = {{991.4235f, -807.672f, 21.788f, nullptr},
                                                                          {999.1844f, -855.182f, 21.484f, nullptr},
                                                                          {1012.089f, -923.098f, 19.296f, nullptr}};

BattleBotPath vPath_IC_Horde_Keep_to_Horde_Hangar_Crossroad = {{1128.646f, -763.221f, 48.385f, nullptr},
                                                               {1121.090f, -816.666f, 49.008f, nullptr},
                                                               {1107.106f, -851.459f, 48.804f, nullptr},
                                                               {1072.313f, -888.355f, 30.853f, nullptr},
                                                               {1012.089f, -923.098f, 19.296f, nullptr}};

BattleBotPath vPath_IC_Horde_Hangar_Crossroad_to_Hangar_Flag = {
    {1001.745f, -973.174f, 15.784f, nullptr},  {1015.437f, -1019.47f, 15.578f, nullptr},
    {1009.622f, -1067.78f, 15.777f, nullptr},  {988.0692f, -1113.32f, 18.254f, nullptr},
    {943.7221f, -1134.50f, 32.296f, nullptr},  {892.2205f, -1115.16f, 63.319f, nullptr},
    {849.6576f, -1090.88f, 91.943f, nullptr},  {814.9168f, -1056.42f, 117.275f, nullptr},
    {799.0856f, -1034.62f, 129.000f, nullptr}, {801.685f, -1005.46f, 132.39f, nullptr}};

BattleBotPath vPath_IC_Horde_Keep_to_Horde_Dock_Crossroad = {{1128.646f, -763.221f, 48.385f, nullptr},
                                                             {1116.203f, -723.328f, 48.655f, nullptr},
                                                             {1093.246f, -696.880f, 37.041f, nullptr},
                                                             {1034.226f, -653.581f, 24.432f, nullptr}};

BattleBotPath vPath_IC_Horde_Front_Crossroad_to_Horde_Dock_Crossroad = {{991.4235f, -807.672f, 21.788f, nullptr},
                                                                        {1025.305f, -757.165f, 29.241f, nullptr},
                                                                        {1029.308f, -710.366f, 26.366f, nullptr},
                                                                        {1034.226f, -653.581f, 24.432f, nullptr}};

BattleBotPath vPath_IC_Horde_Dock_Crossroad_to_Refinery_Crossroad = {{1034.226f, -653.581f, 24.432f, nullptr},
                                                                     {1102.358f, -617.505f, 5.4963f, nullptr},
                                                                     {1116.255f, -580.956f, 18.184f, nullptr},
                                                                     {1114.414f, -546.731f, 23.422f, nullptr},
                                                                     {1148.358f, -503.947f, 23.423f, nullptr}};

BattleBotPath vPath_IC_Refinery_Crossroad_to_Refinery_Base = {{1148.358f, -503.947f, 23.423f, nullptr},
                                                              {1201.885f, -500.425f, 4.7262f, nullptr},
                                                              {1240.595f, -471.971f, 0.8933f, nullptr},
                                                              {1265.993f, -435.419f, 10.669f, nullptr}};

BattleBotPath vPath_IC_Horde_Side_Gate_to_Refinery_Base = {
    {1218.676f, -660.487f, 47.870f, nullptr}, {1211.677f, -626.181f, 46.085f, nullptr},
    {1212.720f, -562.300f, 19.514f, nullptr}, {1238.803f, -538.997f, 3.9892f, nullptr},
    {1248.875f, -482.852f, 0.8933f, nullptr}, {1265.993f, -435.419f, 10.669f, nullptr}};

BattleBotPath vPath_IC_Refinery_Crossroad_to_Docks_Crossroad = {
    {1148.358f, -503.947f, 23.423f, nullptr}, {1127.010f, -469.451f, 23.422f, nullptr},
    {1100.976f, -431.146f, 21.312f, nullptr}, {1053.812f, -405.457f, 12.749f, nullptr},
    {1005.570f, -375.439f, 12.695f, nullptr}, {963.4349f, -353.282f, 12.356f, nullptr},
    {907.1394f, -380.470f, 11.912f, nullptr}};

BattleBotPath vPath_IC_Horde_Dock_Crossroad_to_Docks_Crossroad = {
    {1034.226f, -653.581f, 24.432f, nullptr}, {1013.435f, -622.066f, 24.486f, nullptr},
    {988.1990f, -547.937f, 24.424f, nullptr}, {982.4955f, -508.332f, 24.524f, nullptr},
    {982.5065f, -462.920f, 16.833f, nullptr}, {948.8842f, -421.200f, 16.877f, nullptr},
    {907.1394f, -380.470f, 11.912f, nullptr}};

BattleBotPath vPath_IC_Docks_Crossroad_to_Docks_Flag = {{907.1394f, -380.470f, 11.912f, nullptr},
                                                        {851.5726f, -382.503f, 11.906f, nullptr},
                                                        {808.1441f, -381.199f, 11.906f, nullptr},
                                                        {761.1740f, -381.854f, 14.504f, nullptr},
                                                        {726.427f, -364.849f, 17.815f, nullptr}};

BattleBotPath vPath_IC_Horde_Front_Crossroad_to_Workshop = {
    {991.4235f, -807.672f, 21.788f, nullptr}, {944.5518f, -800.344f, 13.155f, nullptr},
    {907.1300f, -798.892f, 8.3237f, nullptr}, {842.9721f, -795.224f, 5.2007f, nullptr},
    {804.5959f, -794.269f, 5.9836f, nullptr}, {774.466f, -801.058f, 6.3428f, nullptr}};

BattleBotPath vPath_IC_Central_Graveyard_to_Workshop = {
    {775.377f, -664.151f, 8.388f, nullptr}, {776.299f, -684.079f, 5.036f, nullptr},
    {777.451f, -707.525f, 0.051f, nullptr}, {779.059f, -734.611f, 1.695f, nullptr},
    {779.643f, -767.010f, 4.843f, nullptr}, {774.466f, -801.058f, 6.3428f, nullptr}};

BattleBotPath vPath_IC_Horde_East_Gate_to_Horde_Keep = {
    {1216.1918f, -864.922f, 48.852f, nullptr}, {1197.3117f, -866.054f, 48.916f, nullptr},
    {1174.195f, -867.931f, 48.621f, nullptr},  {1149.671f, -869.240f, 48.096f, nullptr},
    {1128.257f, -860.087f, 49.562f, nullptr},  {1118.730f, -829.959f, 49.074f, nullptr},
    {1123.201f, -806.498f, 48.896f, nullptr},  {1129.685f, -787.156f, 48.680f, nullptr},
    {1128.646f, -763.221f, 48.385f, nullptr}};

BattleBotPath vPath_IC_Workshop_to_Workshop_Keep = {{773.792f, -825.637f, 8.127f, nullptr},
                                                    {772.706f, -841.881f, 11.622f, nullptr},
                                                    {773.057f, -859.936f, 12.418f, nullptr}};

BattleBotPath vPath_IC_Alliance_Base = {
    {402.020f, -832.289f, 48.627f, nullptr}, {384.784f, -832.551f, 48.830f, nullptr},
    {369.413f, -832.480f, 48.916f, nullptr}, {346.677f, -832.355f, 48.916f, nullptr},
    {326.296f, -832.189f, 48.916f, nullptr}, {311.174f, -832.204f, 48.916f, nullptr},
};

BattleBotPath vPath_IC_Horde_Base = {
    {1158.652f, -762.680f, 48.628f, nullptr}, {1171.598f, -762.628f, 48.649f, nullptr},
    {1189.102f, -763.484f, 48.915f, nullptr}, {1208.599f, -764.332f, 48.915f, nullptr},
    {1227.592f, -764.782f, 48.915f, nullptr}, {1253.676f, -765.441f, 48.915f, nullptr},
};

BattleBotPath vPath_IC_Workshop_to_North_West = {
    {786.051f, -801.798f, 5.968f, nullptr},   {801.767f, -800.845f, 6.201f, nullptr},
    {830.190f, -800.059f, 5.163f, nullptr},   {858.827f, -797.691f, 5.602f, nullptr},
    {881.579f, -795.190f, 6.441f, nullptr},   {905.796f, -792.252f, 8.008f, nullptr},
    {929.874f, -788.912f, 10.674f, nullptr},  {960.001f, -784.233f, 15.521f, nullptr},
    {991.890f, -780.605f, 22.402f, nullptr},  {1014.062f, -761.863f, 28.672f, nullptr},
    {1019.925f, -730.822f, 27.421f, nullptr}, {1022.320f, -698.581f, 25.993f, nullptr},
    {1022.539f, -665.405f, 24.574f, nullptr}, {1016.279f, -632.780f, 24.487f, nullptr},
    {1004.839f, -602.680f, 24.501f, nullptr}, {992.826f, -567.833f, 24.558f, nullptr},
    {984.998f, -535.303f, 24.485f, nullptr},
};

BattleBotPath vPath_IC_South_West_Crossroads = {
    {528.932f, -667.953f, 25.413f, nullptr}, {514.800f, -650.381f, 26.171f, nullptr},
    {488.367f, -621.869f, 25.820f, nullptr}, {479.491f, -594.284f, 26.095f, nullptr},
    {498.094f, -557.031f, 26.015f, nullptr}, {528.272f, -528.761f, 26.015f, nullptr},
    {595.746f, -480.009f, 26.007f, nullptr}, {632.156f, -458.182f, 27.416f, nullptr},
    {656.013f, -446.685f, 28.003f, nullptr},
};

BattleBotPath vPath_IC_Hanger_to_Workshop = {
    {808.923f, -1003.441f, 132.380f, nullptr}, {804.031f, -1002.785f, 132.382f, nullptr},
    {798.466f, -1021.472f, 132.292f, nullptr}, {795.472f, -1031.693f, 130.232f, nullptr},
    {804.631f, -1042.672f, 125.310f, nullptr}, {827.549f, -1061.778f, 111.276f, nullptr},
    {847.424f, -1077.930f, 98.061f, nullptr},  {868.225f, -1091.563f, 83.794f, nullptr},
    {894.100f, -1104.828f, 66.570f, nullptr},  {923.615f, -1117.689f, 47.391f, nullptr},
    {954.614f, -1119.716f, 27.880f, nullptr},  {970.397f, -1116.281f, 22.124f, nullptr},
    {985.906f, -1105.714f, 18.572f, nullptr},  {996.308f, -1090.605f, 17.669f, nullptr},
    {1003.693f, -1072.916f, 16.583f, nullptr}, {1008.660f, -1051.310f, 15.970f, nullptr},
    {1008.437f, -1026.678f, 15.623f, nullptr}, {1000.103f, -999.047f, 16.487f, nullptr},
    {988.412f, -971.096f, 17.796f, nullptr},   {971.503f, -945.742f, 14.438f, nullptr},
    {947.420f, -931.306f, 13.209f, nullptr},   {922.920f, -916.960f, 10.859f, nullptr},
    {901.607f, -902.203f, 9.854f, nullptr},    {881.932f, -882.226f, 7.848f, nullptr},
    {861.795f, -862.477f, 6.731f, nullptr},    {844.186f, -845.952f, 6.192f, nullptr},
    {826.565f, -826.551f, 5.171f, nullptr},    {809.497f, -815.351f, 6.179f, nullptr},
    {790.787f, -809.678f, 6.450f, nullptr},
};

// Twin Peaks (map 726). Cataclysm-era, so there is no AzerothCore path data to port - these
// routes were generated by A* over this core's own map 726 navmesh (dtNavMesh tiles under
// mmaps/0726_*.mmtile), resampled to roughly one point every 16 yards, then anchored on the
// flag stands and the four mid-field buff spawns from BattlegroundTP::SetupBattleground.
// Every point is therefore a real walkable navmesh vertex, not a hand-guessed coordinate.
BattleBotPath vPath_TP_AllianceFlagRoom_to_AllianceGraveyard = {
    {2116.533f, 192.000f, 44.255f, nullptr}, {2129.333f, 202.667f, 43.721f, nullptr},
    {2154.667f, 213.333f, 43.721f, nullptr}, {2171.467f, 217.067f, 44.788f, nullptr},
    {2174.933f, 222.400f, 45.055f, nullptr}, {2167.733f, 241.067f, 43.721f, nullptr},
    {2154.667f, 256.000f, 41.055f, nullptr}, {2154.667f, 277.333f, 33.855f, nullptr},
    {2154.667f, 298.667f, 27.188f, nullptr}, {2154.667f, 320.000f, 23.188f, nullptr},
    {2160.533f, 322.400f, 27.188f, nullptr},
};

BattleBotPath vPath_TP_AllianceGraveyard_to_NorthMid = {
    {2160.533f, 322.400f, 27.188f, nullptr}, {2150.400f, 323.733f, 22.921f, nullptr},
    {2133.333f, 324.000f, 22.921f, nullptr}, {2122.933f, 328.800f, 22.921f, nullptr},
    {2107.733f, 337.333f, 18.388f, nullptr}, {2102.133f, 351.200f, 6.655f, nullptr},
    {2088.800f, 358.667f, 1.588f, nullptr}, {2069.333f, 362.667f, 0.255f, nullptr},
    {2048.000f, 362.667f, 0.255f, nullptr}, {2026.667f, 362.667f, -0.279f, nullptr},
    {2005.333f, 362.667f, -2.945f, nullptr}, {1984.000f, 362.667f, -7.212f, nullptr},
    {1962.667f, 362.667f, -11.479f, nullptr}, {1957.067f, 380.800f, -9.879f, nullptr},
    {1954.133f, 384.800f, -9.879f, nullptr}, {1941.333f, 405.333f, -9.612f, nullptr},
    {1941.333f, 426.667f, -9.612f, nullptr}, {1920.000f, 426.667f, -12.279f, nullptr},
    {1898.667f, 426.667f, -14.945f, nullptr}, {1884.533f, 429.600f, -11.479f, nullptr},
};

BattleBotPath vPath_TP_NorthMid_to_HordeFlagRoom = {
    {1884.533f, 429.600f, -11.479f, nullptr}, {1878.933f, 424.533f, -16.545f, nullptr},
    {1856.000f, 426.667f, -24.545f, nullptr}, {1837.867f, 437.867f, -26.145f, nullptr},
    {1817.333f, 441.600f, -25.079f, nullptr}, {1792.000f, 448.000f, -14.145f, nullptr},
    {1770.667f, 448.000f, -11.745f, nullptr}, {1749.333f, 448.000f, -7.745f, nullptr},
    {1735.733f, 441.333f, -7.745f, nullptr}, {1728.000f, 448.000f, -7.745f, nullptr},
    {1732.267f, 433.067f, -7.745f, nullptr}, {1712.267f, 426.667f, -4.545f, nullptr},
    {1685.333f, 426.667f, -1.612f, nullptr}, {1664.000f, 426.667f, -3.745f, nullptr},
    {1645.867f, 426.667f, -8.279f, nullptr}, {1625.067f, 426.667f, -7.479f, nullptr},
    {1604.800f, 418.400f, -4.812f, nullptr}, {1588.800f, 403.733f, -5.079f, nullptr},
    {1578.667f, 389.067f, -4.812f, nullptr}, {1567.200f, 384.000f, -5.079f, nullptr},
    {1565.067f, 372.267f, 1.855f, nullptr}, {1576.000f, 362.667f, 1.855f, nullptr},
    {1578.667f, 341.333f, 2.655f, nullptr},
};

BattleBotPath vPath_TP_HordeFlagRoom_to_HordeGraveyard = {
    {1578.667f, 341.333f, 2.655f, nullptr}, {1557.333f, 320.000f, 1.588f, nullptr},
    {1544.000f, 305.600f, 0.788f, nullptr}, {1544.267f, 304.533f, 0.788f, nullptr},
    {1557.333f, 298.667f, 0.255f, nullptr}, {1557.333f, 277.333f, 1.588f, nullptr},
    {1562.667f, 262.933f, 3.455f, nullptr}, {1566.400f, 246.667f, 1.855f, nullptr},
    {1572.800f, 220.533f, 1.321f, nullptr}, {1571.200f, 212.533f, 4.788f, nullptr},
};

BattleBotPath vPath_TP_HordeGraveyard_to_SouthMid = {
    {1571.200f, 212.533f, 4.788f, nullptr}, {1600.000f, 213.333f, 1.855f, nullptr},
    {1614.667f, 205.333f, 0.255f, nullptr}, {1642.667f, 192.000f, -2.412f, nullptr},
    {1664.000f, 192.000f, -2.679f, nullptr}, {1685.333f, 192.000f, -3.212f, nullptr},
    {1706.667f, 192.000f, -10.679f, nullptr}, {1728.000f, 187.200f, -16.279f, nullptr},
    {1749.333f, 182.933f, -17.079f, nullptr}, {1770.667f, 192.000f, -14.412f, nullptr},
    {1792.000f, 192.000f, -15.479f, nullptr}, {1808.000f, 192.000f, -16.812f, nullptr},
    {1833.067f, 188.000f, -19.212f, nullptr}, {1834.667f, 178.667f, -9.345f, nullptr},
    {1829.333f, 171.200f, -4.012f, nullptr}, {1837.600f, 178.933f, -12.545f, nullptr},
    {1820.533f, 188.000f, -17.345f, nullptr}, {1804.533f, 192.000f, -15.745f, nullptr},
    {1787.467f, 208.533f, -16.012f, nullptr}, {1773.600f, 225.867f, -13.879f, nullptr},
    {1761.867f, 235.200f, -13.879f, nullptr}, {1750.667f, 237.600f, -13.879f, nullptr},
};

BattleBotPath vPath_TP_SouthMid_to_AllianceFlagRoom = {
    {1750.667f, 237.600f, -13.879f, nullptr}, {1770.667f, 234.667f, -13.879f, nullptr},
    {1792.000f, 234.667f, -13.879f, nullptr}, {1813.333f, 234.667f, -22.945f, nullptr},
    {1834.667f, 234.667f, -26.412f, nullptr}, {1854.400f, 234.667f, -29.879f, nullptr},
    {1877.333f, 234.667f, -29.345f, nullptr}, {1898.667f, 234.667f, -25.879f, nullptr},
    {1920.000f, 234.667f, -20.012f, nullptr}, {1929.600f, 226.933f, -16.812f, nullptr},
    {1930.400f, 225.333f, -17.079f, nullptr}, {1942.400f, 230.133f, -14.145f, nullptr},
    {1962.667f, 234.667f, -5.879f, nullptr}, {1978.667f, 243.200f, 2.388f, nullptr},
    {1989.067f, 247.733f, 6.921f, nullptr}, {2002.400f, 246.400f, 15.188f, nullptr},
    {2026.667f, 234.667f, 28.255f, nullptr}, {2048.000f, 234.667f, 37.321f, nullptr},
    {2069.333f, 234.667f, 43.988f, nullptr}, {2086.133f, 221.067f, 43.721f, nullptr},
    {2112.000f, 213.333f, 43.721f, nullptr}, {2133.333f, 216.000f, 43.721f, nullptr},
    {2129.333f, 202.667f, 43.721f, nullptr}, {2116.533f, 192.000f, 44.255f, nullptr},
};

BattleBotPath vPath_TP_AllianceFlagRoom_to_HordeFlagRoom = {
    {2116.533f, 192.000f, 44.255f, nullptr}, {2129.333f, 202.667f, 43.721f, nullptr},
    {2133.333f, 216.000f, 43.721f, nullptr}, {2118.667f, 217.600f, 43.721f, nullptr},
    {2090.667f, 213.333f, 43.721f, nullptr}, {2076.000f, 227.200f, 43.988f, nullptr},
    {2048.000f, 234.667f, 37.321f, nullptr}, {2026.667f, 234.667f, 28.255f, nullptr},
    {2008.533f, 241.333f, 20.255f, nullptr}, {1997.333f, 249.067f, 13.055f, nullptr},
    {1985.067f, 246.933f, 4.521f, nullptr}, {1962.667f, 234.667f, -5.879f, nullptr},
    {1945.333f, 233.333f, -12.812f, nullptr}, {1930.400f, 225.333f, -17.079f, nullptr},
    {1916.800f, 221.600f, -21.612f, nullptr}, {1901.867f, 213.333f, -27.479f, nullptr},
    {1877.333f, 213.333f, -28.012f, nullptr}, {1868.000f, 198.133f, -25.879f, nullptr},
    {1843.733f, 187.467f, -20.279f, nullptr}, {1837.600f, 175.733f, -10.412f, nullptr},
    {1829.333f, 171.200f, -4.012f, nullptr}, {1837.600f, 178.933f, -12.545f, nullptr},
    {1820.533f, 188.000f, -17.345f, nullptr}, {1804.533f, 192.000f, -15.745f, nullptr},
    {1787.467f, 208.533f, -16.012f, nullptr}, {1770.667f, 213.333f, -13.879f, nullptr},
    {1752.267f, 211.467f, -13.879f, nullptr}, {1739.467f, 221.067f, -13.879f, nullptr},
    {1728.000f, 234.667f, -13.879f, nullptr}, {1706.667f, 234.667f, -8.545f, nullptr},
    {1685.333f, 234.667f, -4.812f, nullptr}, {1674.400f, 247.467f, -4.545f, nullptr},
    {1662.667f, 256.000f, -3.212f, nullptr}, {1649.333f, 265.067f, -5.079f, nullptr},
    {1632.000f, 271.467f, -5.079f, nullptr}, {1606.933f, 276.267f, -5.079f, nullptr},
    {1593.067f, 279.467f, 0.255f, nullptr}, {1578.667f, 298.667f, 0.255f, nullptr},
    {1560.533f, 309.600f, 0.255f, nullptr}, {1571.733f, 329.333f, 1.588f, nullptr},
    {1578.667f, 341.333f, 2.655f, nullptr},
};

BattleBotPath vPath_TP_HordeFlagRoom_to_AllianceFlagRoom = {
    {1578.667f, 341.333f, 2.655f, nullptr}, {1576.267f, 357.867f, 1.855f, nullptr},
    {1570.400f, 373.333f, 1.855f, nullptr}, {1565.867f, 384.000f, -4.545f, nullptr},
    {1578.667f, 387.733f, -5.079f, nullptr}, {1587.200f, 402.133f, -5.079f, nullptr},
    {1601.600f, 417.333f, -5.345f, nullptr}, {1616.800f, 420.800f, -5.879f, nullptr},
    {1642.667f, 426.667f, -8.012f, nullptr}, {1664.000f, 426.667f, -3.745f, nullptr},
    {1685.333f, 426.667f, -1.612f, nullptr}, {1706.667f, 426.667f, -3.212f, nullptr},
    {1728.000f, 426.667f, -7.745f, nullptr}, {1728.000f, 440.000f, -7.745f, nullptr},
    {1728.000f, 448.000f, -7.745f, nullptr}, {1742.667f, 445.867f, -7.745f, nullptr},
    {1757.867f, 438.133f, -7.479f, nullptr}, {1770.667f, 426.667f, -13.079f, nullptr},
    {1792.000f, 426.667f, -15.745f, nullptr}, {1813.333f, 426.667f, -16.012f, nullptr},
    {1825.600f, 416.000f, -17.879f, nullptr}, {1856.000f, 405.333f, -20.545f, nullptr},
    {1875.467f, 402.667f, -19.212f, nullptr}, {1898.667f, 405.333f, -17.612f, nullptr},
    {1914.133f, 410.400f, -15.212f, nullptr}, {1931.467f, 413.867f, -11.212f, nullptr},
    {1945.067f, 394.667f, -8.812f, nullptr}, {1954.133f, 384.800f, -9.879f, nullptr},
    {1961.867f, 370.400f, -11.745f, nullptr}, {1962.667f, 341.333f, -11.745f, nullptr},
    {1962.667f, 320.000f, -10.412f, nullptr}, {1972.000f, 306.133f, -5.879f, nullptr},
    {1984.000f, 298.667f, -1.879f, nullptr}, {1988.000f, 282.933f, 3.455f, nullptr},
    {2005.333f, 277.333f, 7.188f, nullptr}, {2015.200f, 266.933f, 15.455f, nullptr},
    {2026.667f, 256.000f, 22.921f, nullptr}, {2046.667f, 256.000f, 33.055f, nullptr},
    {2060.000f, 248.533f, 41.055f, nullptr}, {2069.333f, 234.667f, 43.988f, nullptr},
    {2086.133f, 221.067f, 43.721f, nullptr}, {2112.000f, 213.333f, 43.721f, nullptr},
    {2133.333f, 216.000f, 43.721f, nullptr}, {2129.333f, 202.667f, 43.721f, nullptr},
    {2116.533f, 192.000f, 44.255f, nullptr},
};

std::vector<BattleBotPath*> const vPaths_TP = {
    &vPath_TP_AllianceFlagRoom_to_AllianceGraveyard,
    &vPath_TP_AllianceGraveyard_to_NorthMid,
    &vPath_TP_NorthMid_to_HordeFlagRoom,
    &vPath_TP_HordeFlagRoom_to_HordeGraveyard,
    &vPath_TP_HordeGraveyard_to_SouthMid,
    &vPath_TP_SouthMid_to_AllianceFlagRoom,
    &vPath_TP_AllianceFlagRoom_to_HordeFlagRoom,
    &vPath_TP_HordeFlagRoom_to_AllianceFlagRoom,
};

// #873 (beyond AC - AC has no Twin Peaks): the run-home paths a Twin Peaks flag carrier may take - its own team's half.
// On Warsong Gulch (AC vPaths_WS, AC BattleGroundTactics.cpp:1090-1104) each graveyard sits at its own base, so every
// way home ends at the carrier's own respawn point. Twin Peaks has mid-field graveyards on opposite halves: the Alliance
// resurrects at WorldSafeLocs 1749 (1877,439, north) and the Horde at 1750 (1817,160, south; tpJumpDown lands it on
// (1797,166)). With the full vPaths_TP the two flag-room-to-flag-room paths tie in selectObjectiveWp (both end on the
// stand, score 0) and the first one wins, so an Alliance carrier always ran the south path, which passes 17 yd from
// the Horde graveyard: w75-w77 carriers died at (1658,266) (1869,211) (1785,189) (1866,197), 0 captures in 3 x 600 s.
// Each side now runs past its own graveyard (+61 yd for the Alliance: 830 vs 769), where its respawns join it.
std::vector<BattleBotPath*> const vPaths_TP_CarrierAlliance = {
    &vPath_TP_AllianceFlagRoom_to_AllianceGraveyard,
    &vPath_TP_AllianceGraveyard_to_NorthMid,
    &vPath_TP_NorthMid_to_HordeFlagRoom,
    &vPath_TP_HordeFlagRoom_to_AllianceFlagRoom,
};
std::vector<BattleBotPath*> const vPaths_TP_CarrierHorde = {
    &vPath_TP_HordeFlagRoom_to_HordeGraveyard,
    &vPath_TP_HordeGraveyard_to_SouthMid,
    &vPath_TP_SouthMid_to_AllianceFlagRoom,
    &vPath_TP_AllianceFlagRoom_to_HordeFlagRoom,
};

// All WSG paths are reachable on the navmesh regenerated from the 5.4.8 client with high-res terrain holes
// (#158); the previous navmesh had the tunnels and roofs as islands.
std::vector<BattleBotPath*> const vPaths_WS = {
    &vPath_WSG_HordeFlagRoom_to_HordeGraveyard,
    &vPath_WSG_HordeGraveyard_to_HordeTunnel,
    &vPath_WSG_HordeTunnel_to_HordeFlagRoom,
    &vPath_WSG_AllianceFlagRoom_to_AllianceGraveyard,
    &vPath_WSG_AllianceGraveyard_to_AllianceTunnel,
    &vPath_WSG_AllianceTunnel_to_AllianceFlagRoom,
    &vPath_WSG_HordeTunnel_to_HordeBaseRoof,
    &vPath_WSG_AllianceTunnel_to_AllianceBaseRoof,
    &vPath_WSG_AllianceTunnel_to_HordeTunnel,
    &vPath_WSG_AllianceGraveyardLower_to_HordeFlagRoom,
    &vPath_WSG_HordeGraveyardLower_to_AllianceFlagRoom,
    &vPath_WSG_AllianceGraveyardJump,
    &vPath_WSG_HordeGraveyardJump
};

std::vector<BattleBotPath*> const vPaths_AB = {
    &vPath_AB_AllianceBase_to_Stables,  &vPath_AB_AllianceBase_to_GoldMine, &vPath_AB_AllianceBase_to_LumberMill,
    &vPath_AB_Stables_to_Blacksmith,    &vPath_AB_HordeBase_to_Farm,        &vPath_AB_HordeBase_to_GoldMine,
    &vPath_AB_HordeBase_to_LumberMill,  &vPath_AB_Farm_to_Blacksmith,       &vPath_AB_Stables_to_GoldMine,
    &vPath_AB_Stables_to_LumberMill,    &vPath_AB_Farm_to_GoldMine,         &vPath_AB_Farm_to_LumberMill,
    &vPath_AB_Blacksmith_to_LumberMill, &vPath_AB_Blacksmith_to_GoldMine,   &vPath_AB_Farm_to_Stable,
};

// AC-identical (Ai/Base/Actions/BattleGroundTactics.cpp:1114-1136): 20 of the 21 authored paths -
// vPath_AV_AllianceCaptain_To_AllianceCrossroad3 stays defined but unreferenced, as on AC.
std::vector<BattleBotPath*> const vPaths_AV = {
    &vPath_AV_AllianceSpawn_To_AllianceCrossroad1,
    &vPath_AV_AllianceFortress_To_AllianceCrossroad1,
    &vPath_AV_AllianceCrossroad1_To_AllianceCrossroad2,
    &vPath_AV_StoneheartGrave_To_AllianceCrossroad2,
    &vPath_AV_AllianceCrossroad2_To_StoneheartBunker,
    &vPath_AV_AllianceCrossroad2_To_AllianceCaptain,
    &vPath_AV_AllianceCaptain_To_HordeCrossroad3,
    &vPath_AV_AllianceCrossroads3_To_SnowfallGraveyard,
    //&vPath_AV_AllianceCaptain_To_AllianceCrossroad3,
    &vPath_AV_StoneheartBunker_To_HordeCrossroad3,
    &vPath_AV_AllianceCrossroad1_To_AllianceMine,
    &vPath_AV_HordeSpawn_To_MainRoad,
    &vPath_AV_SnowfallGraveyard_To_HordeCaptain,
    &vPath_AV_HordeCrossroad3_To_IcebloodTower,
    &vPath_AV_IcebloodTower_To_HordeCaptain,
    &vPath_AV_IcebloodTower_To_IcebloodGrave,
    &vPath_AV_IcebloodGrave_To_TowerBottom,
    &vPath_AV_TowerBottom_To_HordeCrossroad1,
    &vPath_AV_HordeCrossroad1_To_FrostwolfGrave,
    &vPath_AV_HordeCrossroad1_To_HordeFortress,
    &vPath_AV_FrostwolfGrave_To_HordeMine
};

std::vector<BattleBotPath*> const vPaths_EY = {
    &vPath_EY_Horde_Spawn_to_Crossroad1Horde,
    &vPath_EY_Horde_Crossroad1Horde_to_Crossroad2Horde,
    &vPath_EY_Crossroad1Horde_to_Blood_Elf_Tower,
    &vPath_EY_Crossroad1Horde_to_Fel_Reaver_Ruins,
    &vPath_EY_Crossroad2Horde_to_Blood_Elf_Tower,
    &vPath_EY_Crossroad2Horde_to_Fel_Reaver_Ruins,
    &vPath_EY_Crossroad2Horde_to_Flag,
    &vPath_EY_Alliance_Spawn_to_Crossroad1Alliance,
    &vPath_EY_Alliance_Crossroad1Alliance_to_Crossroad2Alliance,
    &vPath_EY_Crossroad1Alliance_to_Mage_Tower,
    &vPath_EY_Crossroad1Alliance_to_Draenei_Ruins,
    &vPath_EY_Crossroad2Alliance_to_Mage_Tower,
    &vPath_EY_Crossroad2Alliance_to_Draenei_Ruins,
    &vPath_EY_Crossroad2Alliance_to_Flag,
    &vPath_EY_Draenei_Ruins_to_Blood_Elf_Tower,
    &vPath_EY_Fel_Reaver_to_Mage_Tower,
};

std::vector<BattleBotPath*> const vPaths_IC = {
    &vPath_IC_Ally_Dock_Crossroad_to_Ally_Docks_Second_Crossroad,
    &vPath_IC_Ally_Docks_Crossroad_to_Docks_Flag,
    &vPath_IC_Ally_Docks_Second_Crossroad_to_Ally_Docks_Crossroad,
    &vPath_IC_Lower_Graveyard_Crossroad_to_Ally_Docks_Crossroad,
    &vPath_IC_Lower_Graveyard_Crossroad_to_Ally_Docks_Second_Crossroad,
    &vPath_IC_Lower_Graveyard_to_Lower_Graveyard_Crossroad,
    &vPath_IC_Ally_Front_Crossroad_to_Ally_Dock_Crossroad,
    &vPath_IC_Ally_Front_Crossroad_to_Hangar_First_Crossroad,
    &vPath_IC_Ally_Front_Crossroad_to_Workshop,
    &vPath_IC_Ally_Keep_to_Ally_Dock_Crossroad,
    &vPath_IC_Ally_Keep_to_Ally_Front_Crossroad,
    &vPath_IC_Ally_Keep_to_Hangar_First_Crossroad,
    &vPath_IC_Ally_Keep_to_Quarry_Crossroad,
    &vPath_IC_Docks_Crossroad_to_Docks_Flag,
    &vPath_IC_Docks_Graveyard_to_Docks_Flag,
    &vPath_IC_Hangar_First_Crossroad_to_Hangar_Second_Crossroad,
    &vPath_IC_Hangar_Second_Crossroad_to_Hangar_Flag,
    &vPath_IC_Horde_Dock_Crossroad_to_Docks_Crossroad,
    &vPath_IC_Horde_Dock_Crossroad_to_Refinery_Crossroad,
    &vPath_IC_Horde_Front_Crossroad_to_Horde_Dock_Crossroad,
    &vPath_IC_Horde_Front_Crossroad_to_Horde_Hangar_Crossroad,
    &vPath_IC_Horde_Front_Crossroad_to_Workshop,
    &vPath_IC_Horde_Hangar_Crossroad_to_Hangar_Flag,
    &vPath_IC_Horde_Keep_to_Horde_Dock_Crossroad,
    &vPath_IC_Horde_Keep_to_Horde_Front_Crossroad,
    &vPath_IC_Horde_Keep_to_Horde_Hangar_Crossroad,
    &vPath_IC_Horde_Side_Gate_to_Refinery_Base,
    &vPath_IC_Quarry_Crossroad_to_Hangar_Second_Crossroad,
    &vPath_IC_Quarry_Crossroad_to_Quarry_Flag,
    &vPath_IC_Refinery_Crossroad_to_Docks_Crossroad,
    &vPath_IC_Refinery_Crossroad_to_Refinery_Base,
    &vPath_IC_Central_Graveyard_to_Workshop,
    &vPath_IC_Horde_East_Gate_to_Horde_Keep,
    &vPath_IC_Workshop_to_Workshop_Keep,
    &vPath_IC_Alliance_Base,
    &vPath_IC_Horde_Base,
    &vPath_IC_Workshop_to_North_West,
    &vPath_IC_South_West_Crossroads,
    &vPath_IC_Hanger_to_Workshop,
};

// Cataclysm/MoP-only battlegrounds (#576-#579): no authored waypoint graphs yet, so far objectives are
// reached over the navmesh (selectObjectiveWp finds no path -> moveToObjective(true) -> MoveNear).
std::vector<BattleBotPath*> const vPaths_NoneMop = {};

std::vector<BattleBotPath*> const vPaths_NoReverseAllowed = {
    &vPath_WSG_AllianceGraveyardJump,
    &vPath_WSG_HordeGraveyardJump,
    &vPath_IC_Central_Graveyard_to_Workshop,
    &vPath_IC_Docks_Graveyard_to_Docks_Flag,
};

static uint32 AB_AttackObjectives[] = {
    BG_AB_NODE_STABLES,
    BG_AB_NODE_BLACKSMITH,
    BG_AB_NODE_FARM, BG_AB_NODE_LUMBER_MILL,
    BG_AB_NODE_GOLD_MINE
};

static std::tuple<uint32, uint32, uint32> EY_AttackObjectives[] = {
    {FEL_REAVER, BG_EY_OBJECT_FLAG_FEL_REAVER, TR_FEL_REAVER_POINT},
    {BLOOD_ELF, BG_EY_OBJECT_FLAG_BLOOD_ELF, TR_BLOOD_ELF_POINT},
    {DRAENEI_RUINS, BG_EY_OBJECT_FLAG_DRAENEI_RUINS, TR_DRAENEI_RUINS_POINT},
    {MAGE_TOWER, BG_EY_OBJECT_FLAG_MAGE_TOWER, TR_MAGE_TOWER_POINT}
};

static std::unordered_map<uint32, Position> EY_NodePositions = {
    {FEL_REAVER, Position(2044.173f, 1727.503f, 1189.505f)},
    {BLOOD_ELF, Position(2048.277f, 1395.093f, 1194.255f)},
    {DRAENEI_RUINS, Position(2286.245f, 1404.683f, 1196.991f)},
    {MAGE_TOWER, Position(2284.720f, 1728.457f, 1189.153f)}
};

// AV attack/defend order tables and node movement targets - AC-identical
// (Ai/Base/Actions/BattleGroundTactics.cpp:1206-1246, BattleGroundTactics.h:73-95 there). Kept as
// file-statics here rather than in BattleGroundTactics.h (harness
// and objective-layer data stay out of the widely-included module headers).
// AC-identical (Ai/Base/Actions/BattleGroundTactics.cpp:1269-1273): the three contestable nodes the
// guard-point fallback in selectObjective cycles through.
static std::pair<uint32, uint32> IC_AttackObjectives[] = {
    {NODE_TYPE_WORKSHOP, BG_IC_GO_WORKSHOP_BANNER},
    {NODE_TYPE_DOCKS, BG_IC_GO_DOCKS_BANNER},
    {NODE_TYPE_HANGAR, BG_IC_GO_HANGAR_BANNER},
};

static std::vector<std::pair<uint8, uint32>> AV_AttackObjectives_Horde = {
    // Attack - these are in order they should be attacked
    {BG_AV_NODES_STONEHEART_GRAVE, BG_AV_OBJECT_FLAG_A_STONEHEART_GRAVE},
    {BG_AV_NODES_STONEHEART_BUNKER, BG_AV_OBJECT_FLAG_A_STONEHEART_BUNKER},
    {BG_AV_NODES_ICEWING_BUNKER, BG_AV_OBJECT_FLAG_A_ICEWING_BUNKER},
    {BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_OBJECT_FLAG_A_STORMPIKE_GRAVE},
    {BG_AV_NODES_DUNBALDAR_SOUTH, BG_AV_OBJECT_FLAG_A_DUNBALDAR_SOUTH},
    {BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_OBJECT_FLAG_A_DUNBALDAR_NORTH},
    {BG_AV_NODES_FIRSTAID_STATION, BG_AV_OBJECT_FLAG_A_FIRSTAID_STATION},
};

static std::vector<std::pair<uint8, uint32>> AV_AttackObjectives_Alliance = {
    // Attack - these are in order they should be attacked
    {BG_AV_NODES_ICEBLOOD_GRAVE, BG_AV_OBJECT_FLAG_H_ICEBLOOD_GRAVE},
    {BG_AV_NODES_ICEBLOOD_TOWER, BG_AV_OBJECT_FLAG_H_ICEBLOOD_TOWER},
    {BG_AV_NODES_TOWER_POINT, BG_AV_OBJECT_FLAG_H_TOWER_POINT},
    {BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_OBJECT_FLAG_H_FROSTWOLF_GRAVE},
    {BG_AV_NODES_FROSTWOLF_ETOWER, BG_AV_OBJECT_FLAG_H_FROSTWOLF_ETOWER},
    {BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_OBJECT_FLAG_H_FROSTWOLF_WTOWER},
    {BG_AV_NODES_FROSTWOLF_HUT, BG_AV_OBJECT_FLAG_H_FROSTWOLF_HUT},
};

static std::vector<std::pair<uint8, uint32>> AV_DefendObjectives_Horde = {
    {BG_AV_NODES_FROSTWOLF_HUT, BG_AV_OBJECT_FLAG_H_FROSTWOLF_HUT},
    {BG_AV_NODES_FROSTWOLF_WTOWER, BG_AV_OBJECT_FLAG_H_FROSTWOLF_WTOWER},
    {BG_AV_NODES_FROSTWOLF_ETOWER, BG_AV_OBJECT_FLAG_H_FROSTWOLF_ETOWER},
    {BG_AV_NODES_FROSTWOLF_GRAVE, BG_AV_OBJECT_FLAG_H_FROSTWOLF_GRAVE},
    {BG_AV_NODES_TOWER_POINT, BG_AV_OBJECT_FLAG_H_TOWER_POINT},
    {BG_AV_NODES_ICEBLOOD_TOWER, BG_AV_OBJECT_FLAG_H_ICEBLOOD_TOWER},
    {BG_AV_NODES_ICEBLOOD_GRAVE, BG_AV_OBJECT_FLAG_H_ICEBLOOD_GRAVE},
};

static std::vector<std::pair<uint8, uint32>> AV_DefendObjectives_Alliance = {
    {BG_AV_NODES_FIRSTAID_STATION, BG_AV_OBJECT_FLAG_A_FIRSTAID_STATION},
    {BG_AV_NODES_DUNBALDAR_SOUTH, BG_AV_OBJECT_FLAG_A_DUNBALDAR_SOUTH},
    {BG_AV_NODES_DUNBALDAR_NORTH, BG_AV_OBJECT_FLAG_A_DUNBALDAR_NORTH},
    {BG_AV_NODES_STORMPIKE_GRAVE, BG_AV_OBJECT_FLAG_A_STORMPIKE_GRAVE},
    {BG_AV_NODES_ICEWING_BUNKER, BG_AV_OBJECT_FLAG_A_ICEWING_BUNKER},
    {BG_AV_NODES_STONEHEART_BUNKER, BG_AV_OBJECT_FLAG_A_STONEHEART_BUNKER},
    {BG_AV_NODES_STONEHEART_GRAVE, BG_AV_OBJECT_FLAG_A_STONEHEART_GRAVE},
};

// "Added to fix bot stuck at objectives" (AC's own comment). File-static rather than in the header
// so a harness/logic change here needs only this .cpp relinked.
static std::unordered_map<uint8, AVNodePositionData> AVNodeMovementTargets = {
    {BG_AV_NODES_FIRSTAID_STATION, {Position(640.364f, -36.535f, 45.625f), 15.0f}},
    {BG_AV_NODES_STORMPIKE_GRAVE, {Position(665.598f, -292.976f, 30.291f), 15.0f}},
    {BG_AV_NODES_STONEHEART_GRAVE, {Position(76.108f, -399.602f, 45.730f), 15.0f}},
    {BG_AV_NODES_SNOWFALL_GRAVE, {Position(-201.298f, -119.661f, 78.291f), 15.0f}},
    {BG_AV_NODES_ICEBLOOD_GRAVE, {Position(-617.858f, -400.654f, 59.692f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_GRAVE, {Position(-1083.803f, -341.520f, 55.304f), 15.0f}},
    {BG_AV_NODES_FROSTWOLF_HUT, {Position(-1405.678f, -309.108f, 89.377f, 0.392f), 10.0f}},
    {BG_AV_NODES_DUNBALDAR_SOUTH, {Position(556.551f, -77.240f, 51.931f), 0.0f}},
    {BG_AV_NODES_DUNBALDAR_NORTH, {Position(670.664f, -142.031f, 63.666f), 0.0f}},
    {BG_AV_NODES_ICEWING_BUNKER, {Position(200.310f, -361.232f, 56.387f), 0.0f}},
    {BG_AV_NODES_STONEHEART_BUNKER, {Position(-156.302f, -440.032f, 40.403f), 0.0f}},
    {BG_AV_NODES_ICEBLOOD_TOWER, {Position(-569.702f, -265.362f, 75.009f), 0.0f}},
    {BG_AV_NODES_TOWER_POINT, {Position(-767.439f, -360.200f, 90.895f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_ETOWER, {Position(-1303.737f, -314.070f, 113.868f), 0.0f}},
    {BG_AV_NODES_FROSTWOLF_WTOWER, {Position(-1300.648f, -267.356f, 114.151f), 0.0f}},
};

// useful commands for fixing BG bugs and checking waypoints/paths
bool BGTactics::HandleConsoleCommand(ChatHandler* handler, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        handler->PSendSysMessage("|cffff0000Playerbot system is currently disabled!");
        return true;
    }
    WorldSession* session = handler->GetSession();
    if (!session)
    {
        handler->PSendSysMessage("Command can only be used from an active session");
        return true;
    }
    std::string const commandOutput = HandleConsoleCommandPrivate(session, args);
    if (!commandOutput.empty())
        handler->PSendSysMessage(commandOutput.c_str());
    return true;
}

// has different name to above as some compilers get confused
std::string const BGTactics::HandleConsoleCommandPrivate(WorldSession* session, char const* args)
{
    Player* player = session->GetPlayer();
    if (!player)
        return "Error - session player not found";
    if (!player->IsGameMaster())
        return "Command can only be used by a GM";
    Battleground* bg = player->GetBattleground();
    if (!bg)
        return "Command can only be used within a battleground";
    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896
    char const* usage = "usage: showpath(=[num]) / showcreature=[num] / showobject=[num]";
    char* cmd = strtok((char*)args, " ");
    // char* charname = strtok(nullptr, " ");
    if (!cmd)
        return usage;

    if (!strncmp(cmd, "showpath", 8))
    {
        int num = -1;
        if (!strncmp(cmd, "showpath=all", 12))
        {
            num = -2;
        }
        else if (!strncmp(cmd, "showpath=", 9))
        {
            if (sscanf(cmd, "showpath=%d", &num) == -1 || num < 0)
                return "Bad showpath parameter";
        }
        std::vector<BattleBotPath*> const* vPaths;
        switch (bgType)
        {
            case BATTLEGROUND_AV:
                vPaths = &vPaths_AV;
                break;
            case BATTLEGROUND_AB:
                vPaths = &vPaths_AB;
                break;
            case BATTLEGROUND_WS:
                vPaths = &vPaths_WS;
                break;
            case BATTLEGROUND_EY:
                vPaths = &vPaths_EY;
                break;
            case BATTLEGROUND_TP:
                vPaths = &vPaths_TP;
                break;
            case BATTLEGROUND_IC:
                vPaths = &vPaths_IC;
                break;
            default:
                vPaths = nullptr;
                break;
        }
        if (!vPaths)
            return "This battleground has no paths and is unsupported";
        if (num == -1)
        {
            float closestPoint = FLT_MAX;
            for (uint32 j = 0; j < vPaths->size(); j++)
            {
                auto const& path = (*vPaths)[j];
                for (uint32 i = 0; i < path->size(); i++)
                {
                    BattleBotWaypoint& waypoint = ((*path)[i]);
                    float dist = player->GetDistance(waypoint.x, waypoint.y, waypoint.z);
                    if (closestPoint > dist)
                    {
                        closestPoint = dist;
                        num = j;
                    }
                }
            }
        }
        uint32 min = 0u;
        uint32 max = vPaths->size() - 1;
        if (num >= 0)  // num specified or found
        {
            if (uint32(num) > max)
                return fmt::format("Path {} of range of 0 - {}", num, max);
            min = num;
            max = num;
        }
        for (uint32 j = min; j <= max; j++)
        {
            auto const& path = (*vPaths)[j];
            for (uint32 i = 0; i < path->size(); i++)
            {
                BattleBotWaypoint& waypoint = ((*path)[i]);
                Creature* wpCreature = player->SummonCreature(15631, waypoint.x, waypoint.y, waypoint.z, 0,
                                                              TEMPSUMMON_TIMED_DESPAWN, Milliseconds(15000));
                wpCreature->SetOwnerGUID(player->GetGUID());
            }
        }
        if (num >= 0)
            return fmt::format("Showing path {}", num);
        return fmt::format("Showing paths 0 - {}", max);
    }

    if (!strncmp(cmd, "showcreature=", 13))
    {
        uint32 num;
        if (sscanf(cmd, "showcreature=%u", &num) == -1)
            return "Bad showcreature parameter";
        if (num >= bg->BgCreatures.size())
            return fmt::format("Creature out of range of 0 - {}", bg->BgCreatures.size() - 1);
        Creature* c = bg->GetBGCreature(num);
        if (!c)
            return "Creature not found";
        Creature* wpCreature = player->SummonCreature(15631, c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), 0,
                                                      TEMPSUMMON_TIMED_DESPAWN, Milliseconds(15000));
        wpCreature->SetOwnerGUID(player->GetGUID());
        float distance = player->GetDistance(c);
        float exactDistance = player->GetExactDist(c);
        return fmt::format("Showing Creature {} location={:.3f},{:.3f},{:.3f} distance={} exactDistance={}",
            num, c->GetPositionX(), c->GetPositionY(), c->GetPositionZ(), distance, exactDistance);
    }

    if (!strncmp(cmd, "showobject=", 11))
    {
        uint32 num;
        if (sscanf(cmd, "showobject=%u", &num) == -1)
            return "Bad showobject parameter";
        if (num >= bg->BgObjects.size())
            return fmt::format("Object out of range of 0 - {}", bg->BgObjects.size() - 1);
        GameObject* o = bg->GetBGObject(num);
        if (!o)
            return "GameObject not found";
        Creature* wpCreature = player->SummonCreature(15631, o->GetPositionX(), o->GetPositionY(), o->GetPositionZ(), 0,
                                                      TEMPSUMMON_TIMED_DESPAWN, Milliseconds(15000));
        wpCreature->SetOwnerGUID(player->GetGUID());
        float distance = player->GetDistance(o);
        float exactDistance = player->GetExactDist(o);
        return fmt::format("Showing GameObject {} location={:.3f},{:.3f},{:.3f} distance={} exactDistance={}",
            num, o->GetPositionX(), o->GetPositionY(), o->GetPositionZ(), distance, exactDistance);
    }

    return usage;
}

// AC rolls this once per match in PlayerBotsBGScript::OnBattlegroundStart (Script/Playerbots.cpp:
// 529-556) because it has a BG-start script hook. MoP has none, so this rolls it lazily instead, the
// first time any bot asks after the match is actually IN_PROGRESS - the map/typeId are only settled
// by then, and gating on it also means every bot in the match still sees the very first roll (they
// all call in on the same tactics tick once the match starts). Before that this returns the same 0
// ("balanced") AC's default-constructed BGStrategyData would give, so nothing behaves differently
// during WAIT_JOIN. bg-parity-stage-design.md fix 3.
//
// The harness pins this map directly (Scenario/BotScenario.cpp writes bgStrategies[instanceId] every
// sample when strat=<a>,<h> is given), and that write always wins on the next sample regardless of
// whatever this function rolled first - the two paths only ever touch the same map entry.
// Temple of Kotmogu (owner 2026-10-08): AC rolls its team strategies uniformly (urand above); ToK draws with weights
// so most matches play the balanced split and the lopsided ones (all on one side, orb rush, centre hold, hunt) stay
// the exception. Index = TOKBotStrategy (BattleGroundTactics.h); the splits are TOK_PLANS in BgTacticsTOK.cpp.
static uint32 const TOK_STRATEGY_WEIGHTS[TOK_STRATEGY_MAX] = {30, 15, 15, 15, 15, 10};

static uint8 RollTokStrategy()
{
    uint32 total = 0;
    for (uint32 w : TOK_STRATEGY_WEIGHTS)
        total += w;

    uint32 roll = urand(0, total - 1);
    for (uint8 i = 0; i < TOK_STRATEGY_MAX; ++i)
    {
        if (roll < TOK_STRATEGY_WEIGHTS[i])
            return i;
        roll -= TOK_STRATEGY_WEIGHTS[i];
    }

    return TOK_STRATEGY_BALANCED;
}

uint8 BGTactics::GetBotStrategyForTeam(Battleground* bg, TeamId teamId)
{
    if (!bg)
        return 0;

    auto itr = bgStrategies.find(bg->GetInstanceID());
    if (itr == bgStrategies.end())
    {
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
            return 0;

        BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

        BGStrategyData data;
        switch (bgType)
        {
            case BATTLEGROUND_WS:
            case BATTLEGROUND_TP:  // Twin Peaks plays the WSG case in selectObjective() too.
                data.allianceStrategy = urand(0, WS_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, WS_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AB:
                data.allianceStrategy = urand(0, AB_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AB_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_AV:
                data.allianceStrategy = urand(0, AV_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, AV_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_EY:
                data.allianceStrategy = urand(0, EY_STRATEGY_MAX - 1);
                data.hordeStrategy = urand(0, EY_STRATEGY_MAX - 1);
                break;
            case BATTLEGROUND_TOK:  // MoP-only (owner 2026-10-08), weighted - see RollTokStrategy
                data.allianceStrategy = RollTokStrategy();
                data.hordeStrategy = RollTokStrategy();
                TC_LOG_DEBUG("playerbots", "[BGTOK] strategy instance=%u alliance=%u horde=%u", bg->GetInstanceID(),
                             uint32(data.allianceStrategy), uint32(data.hordeStrategy));
                break;
            default:
                break;
        }

        itr = bgStrategies.emplace(bg->GetInstanceID(), data).first;
    }

    return teamId == TEAM_ALLIANCE ? itr->second.allianceStrategy : itr->second.hordeStrategy;
}

bool BGTactics::wsJumpDown()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    TeamId team = BgTeamIdOf(bot);
    uint32 mapId = bg->GetMapId();

    if (team == TEAM_HORDE)
    {
        if (bot->GetDistance({1038.220f, 1420.197f, 340.099f}) < 4.0f)
        {
            MoveTo(mapId, 1029.242f, 1387.024f, 340.866f);
            return true;
        }
        if (bot->GetDistance({1029.242f, 1387.024f, 340.866f}) < 4.0f)
        {
            MoveTo(mapId, 1045.764f, 1389.831f, 340.825f);
            return true;
        }
        if (bot->GetDistance({1045.764f, 1389.831f, 340.825f}) < 4.0f)
        {
            MoveTo(mapId, 1057.076f, 1393.081f, 339.505f);
            return true;
        }
        if (bot->GetDistance({1057.076f, 1393.081f, 339.505f}) < 4.0f)
        {
            JumpTo(mapId, 1075.233f, 1398.645f, 323.669f);
            return true;
        }
        if (bot->GetDistance({1075.233f, 1398.645f, 323.669f}) < 4.0f)
        {
            MoveTo(mapId, 1096.590f, 1395.070f, 317.016f);
            return true;
        }
        if (bot->GetDistance({1096.590f, 1395.070f, 317.016f}) < 4.0f)
        {
            MoveTo(mapId, 1134.380f, 1370.130f, 312.741f);
            return true;
        }
    }
    else if (team == TEAM_ALLIANCE)
    {
        if (bot->GetDistance({1415.548f, 1554.538f, 343.164f}) < 4.0f)
        {
            MoveTo(mapId, 1407.234f, 1551.658f, 343.432f);
            return true;
        }
        if (bot->GetDistance({1407.234f, 1551.658f, 343.432f}) < 4.0f)
        {
            JumpTo(mapId, 1385.325f, 1544.592f, 322.047f);
            return true;
        }
        if (bot->GetDistance({1385.325f, 1544.592f, 322.047f}) < 4.0f)
        {
            MoveTo(mapId, 1370.710f, 1543.550f, 321.585f);
            return true;
        }
        if (bot->GetDistance({1370.710f, 1543.550f, 321.585f}) < 4.0f)
        {
            MoveTo(mapId, 1339.410f, 1533.420f, 313.336f);
            return true;
        }
    }

    return false;
}

// #834 (beyond AC: this core's map geometry). AC's eyJumpDown (Ai/Base/Actions/BattleGroundTactics.cpp:1514-1556) is
// kept below unchanged, but its points are WotLK geometry: the 5.4.8 Eye of the Storm start platforms sit ~21 yd lower
// (WorldSafeLocs 1103/1104 here: 2523.8,1596.1,1247.7 / 1803.0,1540.0,1249.2; AC spawns at z 1269.3) and the middle
// ledge AC's first jump lands on does not exist - the platform's low edge (z 1231.4..1233.5) ends 10-12 yd straight
// above the field. AC's 3D GetDistance checks against its z-1270/1254 points never fire here, so the bots walked to
// the edge and every path from there was the platform's one-polygon dead end. Up to build 21 MoveTo turned that into
// a straight glide through the air; since ca272d76 (#747, PathTypeIsWalkable) the move is refused and both teams
// stood at the two edges for the whole match (w30-w63: 0 victims, 0 casts on players). Same cure as AC (walk to the
// edge, jump onto the field), measured on this core's navmesh (data-mop-extract/mmaps/0566_27_28 + 0566_28_28:
// each platform is a 19-polygon component with no link to the 5727-polygon field component).
bool BGTactics::eyJumpDown()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    {
        // [0] platform low edge (platform mesh), [1] landing on the field mesh, 8-10 yd out and 10-13 yd down
        Position const hordeMopPositions[] = {
            {1839.2f, 1538.5f, 1232.1f},  // between platform vertices (1840.0,1540.0,1231.6) and (1838.7,1536.0,1232.1)
            {1845.5f, 1533.5f, 1223.0f},  // field polygon (nav z 1222.6); AC's landing 1846.3,1535.1 is on it too
        };
        Position const allianceMopPositions[] = {
            {2492.3f, 1597.5f, 1232.4f},  // between platform vertices (2490.9,1597.1,1231.4) and (2492.0,1600.0,1231.9)
            {2484.0f, 1593.0f, 1220.1f},  // field polygon (nav z 1219.7); AC's landing 2484.6,1598.2 is on it too
        };
        bool const horde = BgTeamIdOf(bot) == TEAM_HORDE;
        Position const* mop = horde ? hordeMopPositions : allianceMopPositions;
        // platform bounding box (+1 yd). Bots on the platform stand at z >= 1230.8 (A) / 1231.1 (H) (w30/w63 probe
        // rows); the field mesh inside the box reaches 1227.1 (A) / 1229.2 (H), the roofs above start at 1287.
        float const minX = horde ? 1768.0f : 2489.5f, maxX = horde ? 1841.5f : 2564.0f;
        float const minY = horde ? 1508.0f : 1565.0f, maxY = horde ? 1570.5f : 1628.5f;
        float const minZ = horde ? 1230.0f : 1229.0f;
        float const bx = bot->GetPositionX(), by = bot->GetPositionY(), bz = bot->GetPositionZ();
        if (bz >= minZ && bz <= 1265.0f && bx >= minX && bx <= maxX && by >= minY && by <= maxY)
        {
            if (bot->GetExactDist2d(mop[0].GetPositionX(), mop[0].GetPositionY()) < 4.0f)
            {
                JumpTo(bg->GetMapId(), mop[1].GetPositionX(), mop[1].GetPositionY(), mop[1].GetPositionZ());
                return true;
            }
            // a refused walk (not on the platform mesh after all) falls through to AC's checks and path selection
            if (MoveTo(bg->GetMapId(), mop[0].GetPositionX(), mop[0].GetPositionY(), mop[0].GetPositionZ()))
                return true;
        }
    }

    Position const hordeJumpPositions[] = {
        EY_WAITING_POS_HORDE,
        {1838.007f, 1539.856f, 1253.383f},
        {1846.264f, 1535.062f, 1240.796f},
        {1849.813f, 1527.303f, 1237.262f},
        {1849.041f, 1518.884f, 1223.624f},
    };
    Position const allianceJumpPositions[] = {
        EY_WAITING_POS_ALLIANCE,
        {2492.955f, 1597.769f, 1254.828f},
        {2484.601f, 1598.209f, 1244.344f},
        {2478.424f, 1609.539f, 1238.651f},
        {2475.926f, 1619.658f, 1218.706f},
    };
    Position const* positons = BgTeamIdOf(bot) == TEAM_HORDE ? hordeJumpPositions : allianceJumpPositions;
    {
        if (bot->GetDistance(positons[0]) < 16.0f)
        {
            MoveTo(bg->GetMapId(), positons[1].GetPositionX(), positons[1].GetPositionY(), positons[1].GetPositionZ());
            return true;
        }
        if (bot->GetDistance(positons[1]) < 4.0f)
        {
            JumpTo(bg->GetMapId(), positons[2].GetPositionX(), positons[2].GetPositionY(), positons[2].GetPositionZ());
            return true;
        }
        if (bot->GetDistance(positons[2]) < 4.0f)
        {
            MoveTo(bg->GetMapId(), positons[3].GetPositionX(), positons[3].GetPositionY(), positons[3].GetPositionZ());
            return true;
        }
        if (bot->GetDistance(positons[3]) < 4.0f)
        {
            JumpTo(bg->GetMapId(), positons[4].GetPositionX(), positons[4].GetPositionY(), positons[4].GetPositionZ());
            return true;
        }
    }
    return false;
}

/// #747 Twin Peaks: the two mid-field graveyards this core resurrects at during a match
/// (BattlegroundTP::GetClosestGraveYard -> WorldSafeLocs 1749 / 1750) stand on navmesh islands that
/// have no walkable link to the rest of map 726 (7 and 42 polys, no exits, in mmaps/0726_28_31.mmtile).
/// From there every path is partial; once the bot reaches the island edge PathGenerator::BuildPointPath
/// takes its "_polyLength == 1 && pointCount == 1" branch and appends the unreachable destination, so the
/// spline is a 2-point straight line and the bot glides through the air / terrain to its waypoint.
/// Same cure AC uses for the WSG and EotS ledges (wsJumpDown / eyJumpDown): walk to the island edge and
/// jump onto the connected mesh. Points were measured on this core's 0726 navmesh (island poly ->
/// connected poly); the z guards keep bots already on the connected mesh below out of the hand-off.
bool BGTactics::tpJumpDown()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    uint32 const mapId = bg->GetMapId();
    // [0] graveyard (resurrect point), [1] island take-off, [2] landing on the connected mesh
    // Re-measured on the 0726 mesh (wave 20, islands.py: connected components of the detail mesh).
    // Alliance island = 255 triangles, x 1870.9..1887.2 y 433.3..447.9 z -5.1..-1.9; the old landing (1880.0,431.1)
    // and the spot bots then stood on (1875.5,432.8, z -5.4 < the old islandMinZ) are a 3-triangle sliver that
    // connects to nothing, so every path from it was a one-polygon INCOMPLETE path ending in a 300-yd straight
    // line to the Horde flag (MovementActions.cpp PathTypeIsWalkable). The landing is now a vertex of the main
    // mesh (37100 triangles). Horde island = 341 triangles, x 1800.1..1841.5 y 145.9..170.4 z -2.7..4.8: the old
    // pair still joins island and main mesh, but the ledge runs 25 yd east of the graveyard (Aslandel, w19), so the
    // radius covers the whole island instead of 11 yd.
    Position const alliancePositions[] = {
        {1877.20f, 439.10f, -4.00f},  // WorldSafeLocs 1749, TP_GRAVEYARD_MIDDLE_ALLIANCE
        {1877.20f, 437.20f, -3.80f},  // island vertex 1.9 yd from the graveyard
        {1875.20f, 429.10f, -13.30f}, // main-mesh vertex, 8.4 yd out, 9.5 yd down
    };
    Position const hordePositions[] = {
        {1816.90f, 160.10f, 1.80f},   // WorldSafeLocs 1750, TP_GRAVEYARD_MIDDLE_HORDE
        {1802.60f, 160.30f, 1.50f},   // island (nav z 2.0)
        {1797.00f, 165.50f, -5.40f},  // main mesh (nav z -5.3)
    };
    bool const horde = BgTeamIdOf(bot) == TEAM_HORDE;
    Position const* positions = horde ? hordePositions : alliancePositions;
    float const islandMinZ = horde ? -3.2f : -5.8f;  // below the lowest island/sliver polygon; main mesh is lower
    // island bounding box (+1 yd), instead of a radius: the Horde ledge is 41 x 25 yd and a radius that reaches
    // its east end would also take in the higher plateau south-west of the graveyard, a different component
    float const minX = horde ? 1799.0f : 1869.5f, maxX = horde ? 1842.5f : 1888.5f;
    float const minY = horde ? 145.0f : 432.0f, maxY = horde ? 171.5f : 449.0f;

    if (bot->GetPositionZ() < islandMinZ)
        return false;

    if (bot->GetExactDist2d(positions[1].GetPositionX(), positions[1].GetPositionY()) < 4.0f)
    {
        JumpTo(mapId, positions[2].GetPositionX(), positions[2].GetPositionY(), positions[2].GetPositionZ());
        return true;
    }
    if (bot->GetPositionX() >= minX && bot->GetPositionX() <= maxX && bot->GetPositionY() >= minY &&
        bot->GetPositionY() <= maxY)
    {
        MoveTo(mapId, positions[1].GetPositionX(), positions[1].GetPositionY(), positions[1].GetPositionZ());
        return true;
    }
    return false;
}

// ---- Twin Peaks match evidence (#674, beyond AC: AC has no Twin Peaks and logs no WSG flag events) -------------------
// The #579 BgTacticsDG observer pattern on the two Warsong-style flags: "[BGTP] pickup|drop|return|capture|respawn|
// flag-change" on every observed flag-state change, "[BGTP] score" when a team score changes, one "[BGTP] state" line per
// instance every 30 s, and one "[BGTP] result" line when the match ends - written by whichever bot of the instance runs
// first (core state is the truth, the bot only reports). Flag A is the Alliance flag (carried by a Horde player, its
// capture scores for the Horde). State values are BattlegroundTP.h BG_TP_FLAG_STATE_*: the core sets ON_PLAYER on a pick
// up (EventPlayerClickedOnFlag :526-527/548-549, again :583-585/616-618 from the ground), ON_GROUND on a drop
// (EventPlayerDroppedFlag :466/482), WAIT_RESPAWN on a capture (EventPlayerCapturedFlag :334/351) and ON_BASE in
// RespawnFlag (:273/278), which a return by an own-team click or the 10-s BG_TP_FLAG_DROP_TIME both end in - "after="
// on a return line tells them apart (< 10 s = returned by a player).
// Process-wide with a mutex, not thread_local: an instance's map update can run on any MapUpdate.Threads worker.
namespace
{
    char const* const TP_FLAG_STATE_NAMES[4] = {"base", "respawn", "player", "ground"};
    char const* const TP_FLAG_OF[2] = {"A", "H"};

    struct TpWatch
    {
        uint32 lastLogMs = 0;
        uint8 state[2] = {255, 255};
        std::string carrier[2];   // name of the player carrying flag t, empty when nobody does
        uint32 groundSinceMs[2] = {0, 0};
        uint32 score[2] = {0, 0};
        bool scoreSeen = false;
        bool ended = false;
        uint32 pickups = 0, drops = 0, returns = 0, captures = 0;
    };
    std::mutex s_tpWatchLock;
    std::map<uint32, TpWatch> s_tpWatch;

    char const* TpFlagStateName(uint8 s) { return s < 4 ? TP_FLAG_STATE_NAMES[s] : "?"; }

    void TpObserve(Battleground* bg, bool matchEnded)
    {
        BattlegroundTP* tp = dynamic_cast<BattlegroundTP*>(bg);
        if (!tp)
            return;
        if (!matchEnded && bg->GetStatus() != STATUS_IN_PROGRESS)
            return;

        uint8 states[2];
        std::string carriers[2];
        for (uint8 t = 0; t < 2; ++t)
        {
            states[t] = tp->GetFlagState(t == TEAM_ALLIANCE ? ALLIANCE : HORDE);
            ObjectGuid const g = bg->GetFlagPickerGUID(TeamId(t));
            if (!g.IsEmpty())
            {
                Player* c = ObjectAccessor::FindPlayer(g);
                carriers[t] = c ? c->GetName() : "?";
            }
        }
        uint32 const scores[2] = {bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE)};
        uint32 const nowMs = getMSTime();
        uint32 const tMs = bg->GetElapsedTime();

        std::lock_guard<std::mutex> guard(s_tpWatchLock);
        TpWatch& w = s_tpWatch[bg->GetInstanceID()];
        if (w.ended)
            return;

        for (uint8 t = 0; t < 2; ++t)
        {
            uint8 const from = w.state[t], to = states[t];
            if (from != to && from != 255)
            {
                char const* ev = "flag-change";
                std::string who = carriers[t].empty() ? w.carrier[t] : carriers[t];
                char after[24] = "";
                if (to == BG_TP_FLAG_STATE_ON_PLAYER)
                {
                    ev = "pickup";
                    ++w.pickups;
                }
                else if (from == BG_TP_FLAG_STATE_ON_PLAYER && to == BG_TP_FLAG_STATE_ON_GROUND)
                {
                    ev = "drop";
                    ++w.drops;
                }
                else if (from == BG_TP_FLAG_STATE_ON_PLAYER && to == BG_TP_FLAG_STATE_WAIT_RESPAWN)
                {
                    ev = "capture";
                    ++w.captures;
                }
                else if (from == BG_TP_FLAG_STATE_ON_GROUND)
                {
                    ev = "return";
                    ++w.returns;
                    who.clear();
                    if (w.groundSinceMs[t])
                        snprintf(after, sizeof(after), " after=%.1fs",
                                 getMSTimeDiff(w.groundSinceMs[t], nowMs) / 1000.0f);
                }
                else if (from == BG_TP_FLAG_STATE_WAIT_RESPAWN && to == BG_TP_FLAG_STATE_ON_BASE)
                {
                    ev = "respawn";
                    who.clear();
                }
                TC_LOG_DEBUG("playerbots", "[BGTP] %s inst=%u flag=%s %s->%s by=%s%s score=%u-%u t=%u", ev,
                             bg->GetInstanceID(), TP_FLAG_OF[t], TpFlagStateName(from), TpFlagStateName(to),
                             who.empty() ? "-" : who.c_str(), after, scores[0], scores[1], tMs);
            }
            if (to == BG_TP_FLAG_STATE_ON_GROUND && from != BG_TP_FLAG_STATE_ON_GROUND)
                w.groundSinceMs[t] = nowMs;
            w.state[t] = to;
            w.carrier[t] = carriers[t];
        }

        if (w.scoreSeen && (scores[0] != w.score[0] || scores[1] != w.score[1]))
            TC_LOG_DEBUG("playerbots", "[BGTP] score inst=%u %u-%u -> %u-%u t=%u", bg->GetInstanceID(), w.score[0],
                         w.score[1], scores[0], scores[1], tMs);
        w.score[0] = scores[0];
        w.score[1] = scores[1];
        w.scoreSeen = true;

        if (matchEnded)
        {
            w.ended = true;
            uint8 const winner = bg->GetWinner();
            TC_LOG_DEBUG("playerbots",
                         "[BGTP] result inst=%u winner=%s score=%u-%u pickups=%u drops=%u returns=%u captures=%u t=%u",
                         bg->GetInstanceID(),
                         winner == WINNER_ALLIANCE ? "A" : winner == WINNER_HORDE ? "H" : "none", scores[0], scores[1],
                         w.pickups, w.drops, w.returns, w.captures, tMs);
            return;
        }

        if (w.lastLogMs && getMSTimeDiff(w.lastLogMs, nowMs) < 30000)
            return;
        w.lastLogMs = nowMs;
        TC_LOG_DEBUG("playerbots",
                     "[BGTP] state inst=%u score=%u-%u flagA=%s%s%s flagH=%s%s%s pickups=%u drops=%u returns=%u captures=%u t=%u",
                     bg->GetInstanceID(), scores[0], scores[1], TpFlagStateName(states[0]),
                     carriers[0].empty() ? "" : ":", carriers[0].c_str(), TpFlagStateName(states[1]),
                     carriers[1].empty() ? "" : ":", carriers[1].c_str(), w.pickups, w.drops, w.returns, w.captures,
                     tMs);
    }
}  // namespace

//
// actual bg tactics below
//
bool BGTactics::Execute(Event /*event*/)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
    {
        botAI->ResetStrategies();
        return false;
    }

    if (bg->GetStatus() == STATUS_WAIT_LEAVE)
    {
        // #717 (beyond AC): the SotA match-result evidence line (BgTacticsSA.cpp)
        if (bg->GetMapId() == 607)
            SotaBeyondAc::ObserveEnd(bot, static_cast<BattlegroundSA*>(bg));
        // #674 (beyond AC): the Twin Peaks match-end diff (the winning capture ends the match in the same tick) + result
        if (bg->GetMapId() == 726)
            TpObserve(bg, true);

        return BGStatusAction::LeaveBG(botAI);
    }

    if (bg->IsArena())
    {
        // can't use this in arena - no vPaths/vFlagIds (will crash server)
        botAI->ResetStrategies();
        return false;
    }

    // #747 (beyond AC; AC BattleGroundTactics.cpp has no such check): a bot that an earlier straight spline left
    // standing in mid-air never moves again - every path from an off-mesh start is NOPATH, so MoveTo keeps
    // returning false (wave 19: Aslandel, 10 minutes at (1891.5,167.1,9.1) over ground -49 south of the Horde mid
    // graveyard, 2316 of the 3473 gate failures). Drop it with the core's falling spline, the same call AC's
    // CheckMountStateAction.cpp:384 uses for a dismounted flyer; the client shows a fall instead of a hover.
    // Twin Peaks only (wave 20): on Strand of the Ancients the same test dropped bots off the fortress walls and
    // the attackers' boat deck (3-15 yd falls at (1233,-187,63), (1219,56,58), (1407,82,36), (2577,980,15)) and on
    // Battle for Gilneas at (1207,953,18) - gameobject/WMO floors this core's GetWaterOrGroundLevel does not see.
    if (bg->GetMapId() == 726 && bot->IsAlive() && !bot->isMoving() && !bot->IsFlying() && !bot->isSwimming() &&
        !bot->IsFalling() && !bot->GetTransport() && !bot->GetVehicle() &&
        !bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
    {
        float const surface =
            bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        if (surface > INVALID_HEIGHT && bot->GetPositionZ() - surface > 3.0f)
        {
            TC_LOG_DEBUG("playerbots", "[BGF] %s: %.1f yd above the surface at (%.1f,%.1f,%.1f), falling (#747)",
                         bot->GetName().c_str(), bot->GetPositionZ() - surface, bot->GetPositionX(),
                         bot->GetPositionY(), bot->GetPositionZ());
            bot->GetMotionMaster()->MoveFall();
            return true;
        }
    }

    if (bg->GetStatus() == STATUS_IN_PROGRESS)
        botAI->ChangeStrategy("-buff", BOT_STATE_NON_COMBAT);

    // issue #68/#97: while the doors are closed, keep every bot inside the starting area. mmaps do
    // not include the door gameobjects, so a bot whose path crosses one walks straight through it.
    //
    // The core already does this in Battleground::CheckSafePositions(), but only every
    // CHECK_PLAYER_POSITION_INVERVAL and only for the template's StartMaxDist radius - a bot that is
    // outside can therefore be seen running at the gate for that whole interval before it is pulled
    // back. This does the same check every tick and hard-places the bot instead of letting it walk
    // in, so the "runs through the closed gate" window is a single tick.
    {
        static thread_local std::map<ObjectGuid, WorldPosition> s_startArea;

        // #717: not on Strand of the Ancients. Its attackers start on the boats and the sides swap at the round
        // swap (which sets STATUS_WAIT_JOIN again), so the template's fixed per-team start location
        // (battleground_template 9: 1367/1368, StartMaxDist 0 -> the 80 yd fallback) is the wrong place for half
        // the bots; the core places them itself (BattlegroundSA::AddPlayer/TeleportPlayers/PostUpdateImpl).
        if (bg->GetStatus() == STATUS_WAIT_JOIN && bg->GetMapId() != 607)
        {
            float sx = 0.0f, sy = 0.0f, sz = 0.0f, so = 0.0f;
            // NOTE: GetTeamStartLoc takes a *Team* (ALLIANCE = 469 / HORDE = 67), not a TeamId.
            // Battleground::GetTeamIndexByTeamId(uint32 Team) is "Team == ALLIANCE ? TEAM_ALLIANCE :
            // TEAM_HORDE", so handing it BgTeamIdOf() (0/1) returns TEAM_HORDE for TEAM_ALLIANCE and
            // the "start location" came back as the enemy's - which put Alliance bots 592 yd from
            // their own start on WSG and made this guard teleport them to the Horde side.
            bg->GetTeamStartLoc(bot->GetBGTeam(), sx, sy, sz, so);

            // Same radius the core uses (StartMaxDist is stored squared). Fall back to 80 yd when a
            // battleground template has no value, which is what this used to hard-code.
            float const maxDistSq = bg->GetStartMaxDist();
            float const startRadius = maxDistSq > 0.0f ? std::sqrt(maxDistSq) : 80.0f;
            float const distToStart = bot->GetExactDist(sx, sy, sz);

            auto itr = s_startArea.find(bot->GetGUID());
            bool const haveAnchor = itr != s_startArea.end() && itr->second.GetMapId() == bot->GetMapId();

            if (!haveAnchor)
            {
                if (distToStart > startRadius)
                {
                    // Placed outside the pen (late join, a bad teleport, or an earlier run-out):
                    // put it inside rather than let it walk in through a closed door.
                    TC_LOG_DEBUG("playerbots",
                                 "[BG] %s begins WAIT_JOIN %.0f yd outside its start area (map %u, radius %.0f) - hard-placing it",
                                 bot->GetName().c_str(), distToStart, bot->GetMapId(), startRadius);
                    bot->StopMoving();
                    bot->GetMotionMaster()->Clear();
                    bot->TeleportTo(bg->GetMapId(), sx, sy, sz, so);
                    s_startArea[bot->GetGUID()] = WorldPosition(bg->GetMapId(), sx, sy, sz, so);
                    return true;
                }

                s_startArea[bot->GetGUID()] = WorldPosition(bot);
            }
            else
            {
                float const distToAnchor = bot->GetExactDist(itr->second.GetPositionX(),
                                                             itr->second.GetPositionY(),
                                                             itr->second.GetPositionZ());
                if (distToAnchor > startRadius || distToStart > startRadius)
                {
                    TC_LOG_DEBUG("playerbots",
                                 "[BG] %s left the starting area during WAIT_JOIN (%.0f yd from anchor, %.0f yd from start, radius %.0f) - pulling it back",
                                 bot->GetName().c_str(), distToAnchor, distToStart, startRadius);
                    bot->StopMoving();
                    bot->GetMotionMaster()->Clear();
                    bot->TeleportTo(itr->second);
                    return true;
                }
            }
        }
        else
        {
            s_startArea.erase(bot->GetGUID());
        }
    }

    std::vector<BattleBotPath*> const* vPaths;
    std::vector<uint32> const* vFlagIds = nullptr;
    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    switch (bgType)
    {
        case BATTLEGROUND_AV:
        {
            vPaths = &vPaths_AV;
            vFlagIds = &vFlagsAV;
            break;
        }
        case BATTLEGROUND_AB:
        {
            vPaths = &vPaths_AB;
            vFlagIds = &vFlagsAB;
            break;
        }
        case BATTLEGROUND_WS:
        {
            vPaths = &vPaths_WS;
            vFlagIds = &vFlagsWS;
            break;
        }
        case BATTLEGROUND_EY:
        {
            vPaths = &vPaths_EY;
            vFlagIds = &vFlagsEY;
            break;
        }
        case BATTLEGROUND_TP:
        {
            vPaths = &vPaths_TP;
            vFlagIds = &vFlagsTP;
            TpObserve(bg, false);  // #674: [BGTP] evidence (beyond AC)
            break;
        }
        case BATTLEGROUND_IC:
        {
            vPaths = &vPaths_IC;
            vFlagIds = &vFlagsIC;
            break;
        }
        // #576-#579: the MoP-only maps interact through their own at*() hooks (atObjectiveMop below),
        // not through atFlag's entry list, so vFlagIds stays null for them.
        case BATTLEGROUND_BFG:
        case BATTLEGROUND_TOK:
        case BATTLEGROUND_SM:
        case BATTLEGROUND_DG:
        // #717 (beyond AC): Strand of the Ancients plays through BgTacticsSA's hooks the same way.
        case BATTLEGROUND_SA:
        {
            vPaths = &vPaths_NoneMop;
            break;
        }
        default:
            // can't use this in this BG - no vPaths/vFlagIds (will crash server)
            botAI->ResetStrategies();
            return false;
    }

    // #576-#579: AC atFlag's role for the MoP-only maps (see BattleGroundTactics.h).
    auto atObjectiveMop = [&]() -> bool
    {
        switch (bgType)
        {
            case BATTLEGROUND_BFG: return atObjectiveBFG(bg);
            case BATTLEGROUND_TOK: return atObjectiveTOK(bg);
            case BATTLEGROUND_SM:  return atObjectiveSM(bg);
            case BATTLEGROUND_DG:  return atObjectiveDG(bg);
            // #675 (beyond AC): keep cannons, catapult launch, seaforium bombs (BgTacticsIC.cpp)
            case BATTLEGROUND_IC:
                return IocBeyondAc::AtObjective(bot, botAI, context, static_cast<BattlegroundIC*>(bg), BgTeamIdOf(bot),
                                                context->GetValue<uint32>("bg role")->Get());
            // #717 (beyond AC): demolishers, cannons, graveyards, seaforium, the relic (BgTacticsSA.cpp)
            case BATTLEGROUND_SA:
                return SotaBeyondAc::AtObjective(bot, botAI, context, static_cast<BattlegroundSA*>(bg), BgTeamIdOf(bot),
                                                 context->GetValue<uint32>("bg role")->Get());
            default:               return false;
        }
    };

    if (getName() == "move to start")
        return moveToStart();

    if (getName() == "reset objective force")
    {
        // AC's flag-aura test, extended to the Kotmogu orb and the Deepwind cart (#577/#579).
        bool isCarryingFlag = isCarryingObjective(bg);

        if (!isCarryingFlag)
        {
            bot->StopMoving();
            bot->GetMotionMaster()->Clear();
            return resetObjective();  // Reset objective to not use "old" data
        }
    }

    if (getName() == "select objective")
        return selectObjective();

    if (getName() == "protect fc")
    {
        if (protectFC())
            return true;
    }

    if (getName() == "move to objective")
    {
        if (bg->GetStatus() == STATUS_WAIT_JOIN)
            return false;

        // #737/#738 MoP adapter: a vehicle passenger is rooted (Vehicle::AddPassenger SetControlled(true,
        // UNIT_STATE_ROOT)), and this core's MotionMaster::Update returns early for a rooted unit
        // (Movement/MotionMaster.cpp:303), so a point move that was still running when the bot boarded (the walk
        // to the vehicle; the board happens from "check flag" mid-walk) is never updated or finalized and keeps
        // UNIT_STATE_ROAMING_MOVE for the whole ride: IsStopped() below then fails on every tick and the driver
        // never drives (w-SotA: 231/231 "move to objective - FAILED", demolishers parked at their spawn). AC's
        // MotionMaster::UpdateMotion has no such early return and its PointMovementGenerator::DoUpdate clears
        // ROAMING_MOVE while rooted, so AC never sees this. Drop the frozen walk; Clear() finalizes it
        // (PointMovementGenerator::DoFinalize clears the state).
        if (bot->GetVehicle() && bot->HasUnitState(UNIT_STATE_ROOT) && !bot->IsStopped())
        {
            bot->GetMotionMaster()->Clear();
            bot->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        }

        // #756 (beyond AC): a bot fought by something no target value covers (pets, guardians, totems) fights it
        // instead of riding on / standing still - see FightPathAttacker.
        if (FightPathAttacker(bg))
            return true;

        // #869 (beyond AC): a carrier that is being fought holds its run and fights back (combat engine);
        // true = it (re)targeted an attacker or keeps its live target, so this emergency-relevance action must not
        // move it away from the fight. Near its own capture point it keeps running (CarrierNearOwnCapture).
        if (CarrierHoldsToFight(bot, botAI, bg, PlayerHasFlag::IsCapturingFlag(bot) || isCarryingObjective(bg)))
            return false;

        if (bot->isMoving())
            return false;

        if (!bot->IsStopped())
            return false;

        switch (bot->GetMotionMaster()->GetCurrentMovementGeneratorType())
        {
            // TODO: should ESCORT_MOTION_TYPE be here seeing as bots use it by default?
            case IDLE_MOTION_TYPE:
            case CHASE_MOTION_TYPE:
            case POINT_MOTION_TYPE:
                break;
            default:
                return true;
        }

        if (vFlagIds && atFlag(*vPaths, *vFlagIds))
            return true;

        if (atObjectiveMop())
            return true;

        if (useBuff())
            return true;

        // NOTE: can't use IsInCombat() when in vehicle as player is stuck in combat forever while in vehicle (ac bug?)
        bool inCombat = bot->GetVehicle() ? (bool)AI_VALUE(Unit*, "enemy player target") : bot->IsInCombat();
        // AC PR mod-playerbots#2856 (BattleGroundTactics.cpp:1682-1685): a vehicle driver with a siege position
        // keeps driving to it; its weapons still fire on the way
        bool const siegeDrive = botAI->IsInVehicle(true) &&
                                context->GetValue<PositionMap&>("position")->Get()["bg siege"].isSet();
        if (inCombat && !siegeDrive && !PlayerHasFlag::IsCapturingFlag(bot))
        {
            // bot->GetMotionMaster()->MovementExpired();
            return false;
        }

        // #873: a Twin Peaks carrier picks its run-home path from its own team's half (vPaths_TP_Carrier*).
        std::vector<BattleBotPath*> const* wpPaths = vPaths;
        if (bgType == BATTLEGROUND_TP && isCarryingObjective(bg))
            wpPaths = BgTeamIdOf(bot) == TEAM_ALLIANCE ? &vPaths_TP_CarrierAlliance : &vPaths_TP_CarrierHorde;

        if (!moveToObjective(false))
            if (!selectObjectiveWp(*wpPaths))
                return moveToObjective(true);

        // bot with flag (or Kotmogu orb / Deepwind cart, #577/#579) should only move to objective
        if (isCarryingObjective(bg))
            return false;

        if (!startNewPathBegin(*vPaths))
            return moveToObjective(true);

        if (!startNewPathFree(*vPaths))
            return moveToObjective(true);
    }

    if (getName() == "use buff")
        return useBuff();

    if (getName() == "check flag")
    {
        if (vFlagIds && atFlag(*vPaths, *vFlagIds))
            return true;

        if (atObjectiveMop())
            return true;
    }

    if (getName() == "check objective")
    {
        // Issue #53: a bot inside a match must play the objective, not trail a master around.
        // Sampled here rather than in BGStrategyCheckAction, because AiFactory drops the "bg"
        // strategy (and with it that action's "timer" trigger) the moment a bot enters a match, so
        // BGStrategyCheckAction only ever sees the single entry tick. "check objective" runs on
        // "often" for the whole match, and only inside one, so this stays bounded.
        Player* const bgMaster = botAI->GetMaster();
        TC_LOG_DEBUG("playerbots", "[BGF] %s: inBg=1 master=%s follow=%u stay=%u groupSize=%u pos=(%.1f,%.1f,%.1f)",
                     bot->GetName().c_str(), bgMaster ? bgMaster->GetName().c_str() : "none",
                     uint32(botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT)),
                     uint32(botAI->HasStrategy("stay", BOT_STATE_NON_COMBAT)),
                     bot->GetGroup() ? bot->GetGroup()->GetMembersCount() : 0u,
                     bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());

        return resetObjective();
    }

    return false;
}

bool BGTactics::moveToStart(bool force)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    if (!force && bg->GetStatus() != STATUS_WAIT_JOIN)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    if (bgType == BATTLEGROUND_WS || bgType == BATTLEGROUND_TP)
    {
        // Twin Peaks shares every line of the Warsong Gulch start logic and differs only in where
        // the waiting spots are, so bind the map's positions once and use them below.
        bool const isTp = bgType == BATTLEGROUND_TP;
        Position const& waitHorde1 = isTp ? TP_WAITING_POS_HORDE_1 : WS_WAITING_POS_HORDE_1;
        Position const& waitHorde2 = isTp ? TP_WAITING_POS_HORDE_2 : WS_WAITING_POS_HORDE_2;
        Position const& waitHorde3 = isTp ? TP_WAITING_POS_HORDE_3 : WS_WAITING_POS_HORDE_3;
        Position const& waitAlliance1 = isTp ? TP_WAITING_POS_ALLIANCE_1 : WS_WAITING_POS_ALLIANCE_1;
        Position const& waitAlliance2 = isTp ? TP_WAITING_POS_ALLIANCE_2 : WS_WAITING_POS_ALLIANCE_2;
        Position const& waitAlliance3 = isTp ? TP_WAITING_POS_ALLIANCE_3 : WS_WAITING_POS_ALLIANCE_3;

        uint32 role = context->GetValue<uint32>("bg role")->Get();

        int startSpot = role < 4 ? BB_WSG_WAIT_SPOT_LEFT : role > 6 ? BB_WSG_WAIT_SPOT_RIGHT : BB_WSG_WAIT_SPOT_SPAWN;
        if (startSpot == BB_WSG_WAIT_SPOT_RIGHT)
        {
            if (BgTeamIdOf(bot) == TEAM_HORDE)
                MoveTo(bg->GetMapId(), waitHorde1.GetPositionX() + frand(-4.0f, 4.0f),
                       waitHorde1.GetPositionY() + frand(-4.0f, 4.0f),
                       waitHorde1.GetPositionZ());
            else
                MoveTo(bg->GetMapId(), waitAlliance1.GetPositionX() + frand(-4.0f, 4.0f),
                       waitAlliance1.GetPositionY() + frand(-4.0f, 4.0f),
                       waitAlliance1.GetPositionZ());
        }
        else if (startSpot == BB_WSG_WAIT_SPOT_LEFT)
        {
            if (BgTeamIdOf(bot) == TEAM_HORDE)
                MoveTo(bg->GetMapId(), waitHorde2.GetPositionX() + frand(-4.0f, 4.0f),
                       waitHorde2.GetPositionY() + frand(-4.0f, 4.0f),
                       waitHorde2.GetPositionZ());
            else
                MoveTo(bg->GetMapId(), waitAlliance2.GetPositionX() + frand(-4.0f, 4.0f),
                       waitAlliance2.GetPositionY() + frand(-4.0f, 4.0f),
                       waitAlliance2.GetPositionZ());
        }
        else  // BB_WSG_WAIT_SPOT_SPAWN
        {
            if (BgTeamIdOf(bot) == TEAM_HORDE)
                MoveTo(bg->GetMapId(), waitHorde3.GetPositionX() + frand(-10.0f, 10.0f),
                       waitHorde3.GetPositionY() + frand(-10.0f, 10.0f),
                       waitHorde3.GetPositionZ());
            else
                MoveTo(bg->GetMapId(), waitAlliance3.GetPositionX() + frand(-10.0f, 10.0f),
                       waitAlliance3.GetPositionY() + frand(-10.0f, 10.0f),
                       waitAlliance3.GetPositionZ());
        }
    }
    else if (bgType == BATTLEGROUND_AB)
    {
        if (BgTeamIdOf(bot) == TEAM_HORDE)
            MoveTo(bg->GetMapId(), AB_WAITING_POS_HORDE.GetPositionX() + frand(-7.0f, 7.0f),
                   AB_WAITING_POS_HORDE.GetPositionY() + frand(-2.0f, 2.0f), AB_WAITING_POS_HORDE.GetPositionZ());
        else
            MoveTo(bg->GetMapId(), AB_WAITING_POS_ALLIANCE.GetPositionX() + frand(-7.0f, 7.0f),
                   AB_WAITING_POS_ALLIANCE.GetPositionY() + frand(-2.0f, 2.0f), AB_WAITING_POS_ALLIANCE.GetPositionZ());
    }
    else if (bgType == BATTLEGROUND_AV)
    {
        if (BgTeamIdOf(bot) == TEAM_HORDE)
            MoveTo(bg->GetMapId(), AV_WAITING_POS_HORDE.GetPositionX() + frand(-5.0f, 5.0f),
                   AV_WAITING_POS_HORDE.GetPositionY() + frand(-3.0f, 3.0f), AV_WAITING_POS_HORDE.GetPositionZ());
        else
            MoveTo(bg->GetMapId(), AV_WAITING_POS_ALLIANCE.GetPositionX() + frand(-5.0f, 5.0f),
                   AV_WAITING_POS_ALLIANCE.GetPositionY() + frand(-3.0f, 3.0f), AV_WAITING_POS_ALLIANCE.GetPositionZ());
    }
    else if (bgType == BATTLEGROUND_EY)
    {
        /* Disabled: Not needed here and sometimes the bots can go out of map (at least with my map files)
        if (BgTeamIdOf(bot) == TEAM_HORDE)
        {
            MoveTo(bg->GetMapId(), EY_WAITING_POS_HORDE.GetPositionX(), EY_WAITING_POS_HORDE.GetPositionY(), EY_WAITING_POS_HORDE.GetPositionZ());
        }
        else
        {
            MoveTo(bg->GetMapId(), EY_WAITING_POS_ALLIANCE.GetPositionX(), EY_WAITING_POS_ALLIANCE.GetPositionZ(), EY_WAITING_POS_ALLIANCE.GetPositionZ());
        }
        */
    }
    // #576-#579: AC moveToStart's per-map branch for the MoP-only maps.
    else if (bgType == BATTLEGROUND_BFG)
        return moveToStartBFG(bg);
    else if (bgType == BATTLEGROUND_TOK)
        return moveToStartTOK(bg);
    else if (bgType == BATTLEGROUND_SM)
        return moveToStartSM(bg);
    else if (bgType == BATTLEGROUND_DG)
        return moveToStartDG(bg);
    // #717 (beyond AC): both SotA warm-ups - attackers stay on the boats, defenders take their posts (BgTacticsSA.cpp)
    else if (bgType == BATTLEGROUND_SA)
    {
        Position spot;
        if (SotaBeyondAc::StartSpot(bot, static_cast<BattlegroundSA*>(bg), BgTeamIdOf(bot),
                                    context->GetValue<uint32>("bg role")->Get(), spot))
            MoveTo(bg->GetMapId(), spot.GetPositionX(), spot.GetPositionY(), spot.GetPositionZ());
    }
    else if (bgType == BATTLEGROUND_IC)
    {
        uint32 role = context->GetValue<uint32>("bg role")->Get();

        if (BgTeamIdOf(bot) == TEAM_HORDE)
        {
            if (role == 9)  // refinery
                MoveTo(bg->GetMapId(), IC_WEST_WAITING_POS_HORDE.GetPositionX() + frand(-5.0f, 5.0f),
                       IC_WEST_WAITING_POS_HORDE.GetPositionY() + frand(-5.0f, 5.0f),
                       IC_WEST_WAITING_POS_HORDE.GetPositionZ());
            else if (role >= 3 && role < 6)  // hanger
                MoveTo(bg->GetMapId(), IC_EAST_WAITING_POS_HORDE.GetPositionX() + frand(-5.0f, 5.0f),
                       IC_EAST_WAITING_POS_HORDE.GetPositionY() + frand(-5.0f, 5.0f),
                       IC_EAST_WAITING_POS_HORDE.GetPositionZ());
            else  // everything else
                MoveTo(bg->GetMapId(), IC_WAITING_POS_HORDE.GetPositionX() + frand(-5.0f, 5.0f),
                       IC_WAITING_POS_HORDE.GetPositionY() + frand(-5.0f, 5.0f), IC_WAITING_POS_HORDE.GetPositionZ());
        }
        else
        {
            if (role < 3)  // docks
                MoveTo(
                    bg->GetMapId(), IC_WAITING_POS_ALLIANCE.GetPositionX() + frand(-5.0f, 5.0f),
                    IC_WAITING_POS_ALLIANCE.GetPositionY() + frand(-5.0f, 5.0f),
                    IC_WAITING_POS_ALLIANCE.GetPositionZ());  // dont bother using west, there's no paths to use anyway
            else if (role == 9 || (role >= 3 && role < 6))    // quarry and hanger
                MoveTo(bg->GetMapId(), IC_EAST_WAITING_POS_ALLIANCE.GetPositionX() + frand(-5.0f, 5.0f),
                       IC_EAST_WAITING_POS_ALLIANCE.GetPositionY() + frand(-5.0f, 5.0f),
                       IC_EAST_WAITING_POS_ALLIANCE.GetPositionZ());
            else  // everything else
                MoveTo(bg->GetMapId(), IC_WAITING_POS_ALLIANCE.GetPositionX() + frand(-5.0f, 5.0f),
                       IC_WAITING_POS_ALLIANCE.GetPositionY() + frand(-5.0f, 5.0f),
                       IC_WAITING_POS_ALLIANCE.GetPositionZ());
        }
    }

    return true;
}

bool BGTactics::selectObjective(bool reset)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    if (bg->GetStatus() != STATUS_IN_PROGRESS)
        return false;

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    if (pos.isSet() && !reset)
        return false;

    WorldObject* BgObjective = nullptr;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896
    switch (bgType)
    {
        case BATTLEGROUND_WS:
        case BATTLEGROUND_TP:
        {
            Position target;
            TeamId team = BgTeamIdOf(bot);

            // Twin Peaks is the same game as Warsong Gulch, so it runs this whole case unchanged
            // and only swaps the map's positions in. Its flag auras are literally the same spell
            // ids (23333 / 23335), so the hasFlag test below needs nothing added.
            bool const isTp = bgType == BATTLEGROUND_TP;
            Position const& flagPosAlliance = isTp ? TP_FLAG_POS_ALLIANCE : WS_FLAG_POS_ALLIANCE;
            Position const& flagPosHorde = isTp ? TP_FLAG_POS_HORDE : WS_FLAG_POS_HORDE;
            Position const& roamPos = isTp ? TP_ROAM_POS : WS_ROAM_POS;
            Position const& gyCampAlliance = isTp ? TP_GY_CAMPING_ALLIANCE : WS_GY_CAMPING_ALLIANCE;
            Position const& gyCampHorde = isTp ? TP_GY_CAMPING_HORDE : WS_GY_CAMPING_HORDE;
            std::vector<Position> const& flagHideAlliance = isTp ? TP_FLAG_HIDE_ALLIANCE : WS_FLAG_HIDE_ALLIANCE;
            std::vector<Position> const& flagHideHorde = isTp ? TP_FLAG_HIDE_HORDE : WS_FLAG_HIDE_HORDE;
            // #674: Twin Peaks only - the bot's own flag-room entrance posts (null on Warsong Gulch, which keeps AC's code).
            std::vector<Position> const* guardPosts =
                !isTp ? nullptr : (team == TEAM_ALLIANCE ? &TP_GUARD_POSTS_ALLIANCE : &TP_GUARD_POSTS_HORDE);

            // Utility to safely relocate a position with optional random radius
            auto SetSafePos = [&](Position const& origin, float radius = 0.0f) -> void
            {
                float rx, ry, rz;
                if (radius > 0.0f)
                {
                    bot->GetRandomPoint(origin, radius, rx, ry, rz);
                    if (rz == VMAP_INVALID_HEIGHT_VALUE)
                        target.Relocate(rx, ry, rz);
                    else
                        target.Relocate(origin);
                }
                else
                {
                    target.Relocate(origin);
                }
            };

            // Check if the bot is carrying the flag
            bool hasFlag = bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG);

            // Retrieve role
            uint8 role = context->GetValue<uint32>("bg role")->Get();
            WSBotStrategy strategyHorde = static_cast<WSBotStrategy>(GetBotStrategyForTeam(bg, TEAM_HORDE));
            WSBotStrategy strategyAlliance = static_cast<WSBotStrategy>(GetBotStrategyForTeam(bg, TEAM_ALLIANCE));
            WSBotStrategy strategy = (team == TEAM_ALLIANCE) ? strategyAlliance : strategyHorde;
            WSBotStrategy enemyStrategy = (team == TEAM_ALLIANCE) ? strategyHorde : strategyAlliance;

            uint8 defendersProhab = 3;  // Default balanced

            switch (static_cast<uint8>(strategy))
            {
                case 0:
                case 1:
                case 2:
                case 3:  // Balanced
                    defendersProhab = 3;
                    break;
                case 4:
                case 5:
                case 6:
                case 7:  // Heavy Offense
                    defendersProhab = 1;
                    break;
                case 8:
                case 9:  // Heavy Defense
                    defendersProhab = 6;
                    break;
            }

            if (enemyStrategy == WS_STRATEGY_DEFENSIVE)
                defendersProhab = 2;

            // Role check
            bool isDefender = role < defendersProhab;

            // Retrieve flag carriers
            Unit* enemyFC = AI_VALUE(Unit*, "enemy flag carrier");
            Unit* teamFC = AI_VALUE(Unit*, "team flag carrier");

            // Retrieve current score
            uint8 allianceScore = bg->GetTeamScore(TEAM_ALLIANCE);
            uint8 hordeScore = bg->GetTeamScore(TEAM_HORDE);

            // Check if both teams currently have the flag
            bool bothFlagsTaken = enemyFC && teamFC;
            if (!hasFlag && bothFlagsTaken)
            {
                // If both flags taken: Bots have 20% chance to support own flag carrier, otherwise attack enemy FC
                if (urand(0, 99) < 20 && teamFC)
                {
                    target.Relocate(teamFC->GetPositionX(), teamFC->GetPositionY(), teamFC->GetPositionZ());
                    if (sServerFacade->GetDistance2d(bot, teamFC) < 33.0f)
                        Follow(teamFC);
                }
                else
                    target.Relocate(enemyFC->GetPositionX(), enemyFC->GetPositionY(), enemyFC->GetPositionZ());
            }
            // Graveyard Camping if in lead
            else if (!hasFlag && role < 8 &&
                ((team == TEAM_ALLIANCE && allianceScore == 2 && hordeScore == 0) ||
                (team == TEAM_HORDE && hordeScore == 2 && allianceScore == 0)))
            {
                if (team == TEAM_ALLIANCE)
                    SetSafePos(gyCampHorde, 10.0f);
                else
                    SetSafePos(gyCampAlliance, 10.0f);
            }
            else if (hasFlag)
            {
                // AC parity (AC BattleGroundTactics.cpp:2237-2244): hide while the enemy holds our flag,
                // else go home. #57's "always go home" was working around the flag-carrier values'
                // range limit (hunters never saw a hidden carrier), fixed in c08f53a5.
                bool const teamFlagTaken = !bg->GetFlagPickerGUID(team).IsEmpty();
                if (team == TEAM_ALLIANCE)
                    SetSafePos(teamFlagTaken ? flagHideAlliance[urand(0, 2)] : flagPosAlliance);
                else
                    SetSafePos(teamFlagTaken ? flagHideHorde[urand(0, 2)] : flagPosHorde);
            }
            else
            {
                if (isDefender)
                {
                    if (enemyFC)
                    {
                        // Defenders attack enemy FC if found
                        target.Relocate(enemyFC->GetPositionX(), enemyFC->GetPositionY(), enemyFC->GetPositionZ());
                    }
                    else if (urand(0, 99) < 33)
                    {
                        // 33% chance to roam near own base
                        SetSafePos(team == TEAM_ALLIANCE ? flagHideAlliance[urand(0, 2)] : flagHideHorde[urand(0, 2)], 5.0f);
                    }
                    else if (teamFC)
                    {
                        // 70% chance to support own FC
                        if (urand(0, 99) < 70)
                        {
                            target.Relocate(teamFC->GetPositionX(), teamFC->GetPositionY(), teamFC->GetPositionZ());
                            if (sServerFacade->GetDistance2d(bot, teamFC) < 33.0f)
                                Follow(teamFC);
                        }
                    }
                    else if (guardPosts)
                    {
                        // Twin Peaks (#674): no carrier to chase or escort, so hold a flag-room entrance post.
                        SetSafePos((*guardPosts)[role % guardPosts->size()], 3.0f);
                    }
                    else
                    {
                        // Roam around central area
                        SetSafePos(roamPos, 75.0f);
                    }
                }
                else  // attacker logic
                {
                    if (enemyFC && urand(0, 99) < 70)
                    {
                        // 70% chance to pursue enemy FC
                        target.Relocate(enemyFC->GetPositionX(), enemyFC->GetPositionY(), enemyFC->GetPositionZ());
                    }
                    else if (teamFC)
                    {
                        // Assist own FC if not pursuing enemy FC
                        target.Relocate(teamFC->GetPositionX(), teamFC->GetPositionY(), teamFC->GetPositionZ());
                        if (sServerFacade->GetDistance2d(bot, teamFC) < 33.0f)
                            Follow(teamFC);
                    }
                    else if (urand(0, 99) < 5)
                    {
                        // 5% chance to free roam
                        SetSafePos(roamPos, 75.0f);
                    }
                    else
                    {
                        // Push toward enemy flag base
                        SetSafePos(team == TEAM_ALLIANCE ? flagPosHorde : flagPosAlliance);
                    }
                }
            }

            // Save the final target position
            if (target.IsPositionValid())
            {
                pos.Set(target.GetPositionX(), target.GetPositionY(), target.GetPositionZ(), bot->GetMapId());
                posMap["bg objective"] = pos;
                return true;
            }

            break;
        }
        case BATTLEGROUND_AV:
        {
            BattlegroundAV* av = static_cast<BattlegroundAV*>(bg);
            TeamId team = BgTeamIdOf(bot);
            uint8 role = context->GetValue<uint32>("bg role")->Get();
            AVBotStrategy strategyHorde = static_cast<AVBotStrategy>(GetBotStrategyForTeam(bg, TEAM_HORDE));
            AVBotStrategy strategyAlliance = static_cast<AVBotStrategy>(GetBotStrategyForTeam(bg, TEAM_ALLIANCE));
            AVBotStrategy strategy = (team == TEAM_ALLIANCE) ? strategyAlliance : strategyHorde;
            AVBotStrategy enemyStrategy = (team == TEAM_ALLIANCE) ? strategyHorde : strategyAlliance;

            // Owner-encoding adapter: MoP's BG_AV_NodeInfo/mine
            // owners are a Team (ALLIANCE 469 / HORDE 67), not AC's TeamId. This is the only change
            // from AC's logic below - constants, odds and order are otherwise unchanged.
            uint32 const myOwnerTeam = (team == TEAM_ALLIANCE) ? uint32(ALLIANCE) : uint32(HORDE);

            uint8 defendersProhab = 4;
            bool enableMineCapture = true;
            bool enableSnowfall = true;

            switch (strategy)
            {
                case AV_STRATEGY_BALANCED:
                    defendersProhab = 4;
                    break;
                case AV_STRATEGY_OFFENSIVE:
                    defendersProhab = 1;
                    enableMineCapture = false;
                    break;
                case AV_STRATEGY_DEFENSIVE:
                    defendersProhab = 9;
                    enableSnowfall = false;
                    break;
                default:
                    break;
            }

            if (enemyStrategy == AV_STRATEGY_DEFENSIVE)
                defendersProhab = 0;

            bool isDefender = role < defendersProhab;
            bool isAdvanced = !isDefender && role > 8;

            auto const& attackObjectives =
                (team == TEAM_HORDE) ? AV_AttackObjectives_Horde : AV_AttackObjectives_Alliance;
            auto const& defendObjectives =
                (team == TEAM_HORDE) ? AV_DefendObjectives_Horde : AV_DefendObjectives_Alliance;

            uint32 destroyedNodes = 0;
            for (auto const& [nodeId, _] : defendObjectives)
                if (av->GetAVNodeInfo(nodeId).State == POINT_DESTROYED)
                    destroyedNodes++;

            float botX = bot->GetPositionX();
            if (isDefender)
            {
                if ((team == TEAM_HORDE && botX >= -62.0f) || (team == TEAM_ALLIANCE && botX <= -462.0f))
                    isDefender = false;
            }

            if (isDefender && destroyedNodes > 0)
            {
                uint32 switchChance = 20 + (destroyedNodes * 15);
                if (urand(0, 99) < switchChance)
                    isDefender = false;
            }

            // --- Mine Capture (rarely works, needs some improvement) ---
            if (!BgObjective && enableMineCapture && role == 0)
            {
                BG_AV_OTHER_VALUES mineType = (team == TEAM_HORDE) ? AV_SOUTH_MINE : AV_NORTH_MINE;
                if (av->GetMineOwner(mineType) != myOwnerTeam)
                {
                    uint32 bossEntry = (team == TEAM_HORDE) ? AV_CPLACE_MINE_S_3 : AV_CPLACE_MINE_N_3;
                    Creature* mBossNeutral = bg->GetBGCreature(bossEntry);
                    Position const* minePositions[] = {(team == TEAM_HORDE) ? &AV_MINE_SOUTH_1 : &AV_MINE_NORTH_1,
                                                       (team == TEAM_HORDE) ? &AV_MINE_SOUTH_2 : &AV_MINE_NORTH_2,
                                                       (team == TEAM_HORDE) ? &AV_MINE_SOUTH_3 : &AV_MINE_NORTH_3};

                    Position const* chosen = minePositions[urand(0, 2)];
                    pos.Set(chosen->GetPositionX(), chosen->GetPositionY(), chosen->GetPositionZ(), bot->GetMapId());
                    posMap["bg objective"] = pos;
                    BgObjective = mBossNeutral;
                }
            }

            // --- Nearby Enemy ---
            if (!BgObjective && urand(0, 99) < 8)
            {
                if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
                {
                    if (bot->GetDistance(enemy) < 500.0f)
                    {
                        pos.Set(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), bot->GetMapId());
                        posMap["bg objective"] = pos;
                        BgObjective = enemy;
                    }
                }
            }

            // --- Snowfall ---
            bool hasSnowfallRole =
                enableSnowfall && ((team == TEAM_ALLIANCE && role < 6) || (team == TEAM_HORDE && role < 5));

            if (!BgObjective && hasSnowfallRole)
            {
                BG_AV_NodeInfo const& snowfallNode = av->GetAVNodeInfo(BG_AV_NODES_SNOWFALL_GRAVE);
                if (snowfallNode.Owner == AV_NEUTRAL_TEAM)
                {
                    if (GameObject* go = bg->GetBGObject(BG_AV_OBJECT_FLAG_N_SNOWFALL_GRAVE))
                    {
                        Position objPos = go->GetPosition();
                        float rx, ry, rz;
                        bot->GetRandomPoint(objPos, frand(5.0f, 15.0f), rx, ry, rz);

                        pos.Set(rx, ry, rz, go->GetMapId());
                        posMap["bg objective"] = pos;
                        BgObjective = go;
                    }
                }
            }

            // --- Captain ---
            if (!BgObjective && urand(0, 99) < 90)
            {
                if (av->IsCaptainAlive(team == TEAM_HORDE ? TEAM_ALLIANCE : TEAM_HORDE))
                {
                    uint32 creatureId = (team == TEAM_HORDE) ? AV_CREATURE_A_CAPTAIN : AV_CREATURE_H_CAPTAIN;
                    if (Creature* captain = bg->GetBGCreature(creatureId))
                    {
                        if (captain->IsAlive())
                        {
                            BgObjective = captain;
                        }
                    }
                }
            }

            // --- Defender Logic ---
            if (!BgObjective && isDefender)
            {
                std::vector<GameObject*> contestedObjectives;
                std::vector<GameObject*> availableObjectives;

                for (auto const& [nodeId, goId] : defendObjectives)
                {
                    BG_AV_NodeInfo const& node = av->GetAVNodeInfo(nodeId);
                    if (node.State == POINT_DESTROYED)
                        continue;

                    GameObject* go = bg->GetBGObject(goId);
                    if (!go)
                        continue;

                    if (node.State == POINT_ASSAULTED)
                        contestedObjectives.push_back(go);
                    else
                        availableObjectives.push_back(go);
                }

                if (!contestedObjectives.empty())
                    BgObjective = contestedObjectives[urand(0, contestedObjectives.size() - 1)];
                else if (!availableObjectives.empty())
                    BgObjective = availableObjectives[urand(0, availableObjectives.size() - 1)];
            }

            // --- Enemy Boss ---
            if (!BgObjective)
            {
                uint32 towersDown = 0;
                for (auto const& [nodeId, _] : attackObjectives)
                    if (av->GetAVNodeInfo(nodeId).State == POINT_DESTROYED)
                        towersDown++;

                if ((towersDown >= 2) || (strategy == AV_STRATEGY_OFFENSIVE))
                {
                    uint8 lastGY = (team == TEAM_HORDE) ? BG_AV_NODES_FIRSTAID_STATION : BG_AV_NODES_FROSTWOLF_HUT;
                    bool ownsFinalGY = av->GetAVNodeInfo(lastGY).Owner == myOwnerTeam;

                    uint32 bossId = (team == TEAM_HORDE) ? AV_CREATURE_A_BOSS : AV_CREATURE_H_BOSS;
                    if (Creature* boss = bg->GetBGCreature(bossId))
                    {
                        if (boss->IsAlive())
                        {
                            uint32 nearbyCount = getPlayersInArea(team, boss->GetPosition(), 200.0f, false);
                            if (ownsFinalGY || nearbyCount >= 20)
                                BgObjective = boss;
                        }
                    }
                }
            }

            // --- Attacker Logic ---
            if (!BgObjective)
            {
                std::vector<GameObject*> candidates;

                for (auto const& [nodeId, goId] : attackObjectives)
                {
                    BG_AV_NodeInfo const& node = av->GetAVNodeInfo(nodeId);
                    GameObject* go = bg->GetBGObject(goId);
                    if (!go || node.State == POINT_DESTROYED || node.TotalOwner == myOwnerTeam)
                        continue;

                    if (node.State == POINT_ASSAULTED && urand(0, 99) >= 1)
                        continue;

                    candidates.push_back(go);

                    if (((strategy == AV_STRATEGY_BALANCED && candidates.size() >= 2) ||
                         (strategy == AV_STRATEGY_OFFENSIVE && candidates.size() >= 3) ||
                         (strategy == AV_STRATEGY_DEFENSIVE && candidates.size() >= 1)) ||
                        isAdvanced)
                        break;
                }

                if (!candidates.empty())
                    BgObjective = candidates[urand(0, candidates.size() - 1)];
                else
                {
                    // Fallback: move to boss wait position
                    Position const& waitPos = (team == TEAM_HORDE) ? AV_BOSS_WAIT_H : AV_BOSS_WAIT_A;

                    float rx, ry, rz;
                    bot->GetRandomPoint(waitPos, 5.0f, rx, ry, rz);

                    pos.Set(rx, ry, rz, bot->GetMapId());
                    posMap["bg objective"] = pos;

                    uint32 bossId = (team == TEAM_HORDE) ? AV_CREATURE_A_BOSS : AV_CREATURE_H_BOSS;
                    if (Creature* boss = bg->GetBGCreature(bossId))
                        if (boss->IsAlive())
                            BgObjective = boss;
                }
            }

            // --- Movement logic for any valid objective ---
            if (BgObjective)
            {
                Position objPos = BgObjective->GetPosition();

                Optional<uint8> linkedNodeId;
                for (auto const& [nodeId, goId] : attackObjectives)
                    if (bg->GetBGObject(goId) == BgObjective)
                        linkedNodeId = nodeId;

                if (!linkedNodeId)
                {
                    for (auto const& [nodeId, goId] : defendObjectives)
                        if (bg->GetBGObject(goId) == BgObjective)
                            linkedNodeId = nodeId;
                }

                float rx, ry, rz;
                if (linkedNodeId && AVNodeMovementTargets.contains(*linkedNodeId))
                {
                    AVNodePositionData const& data = AVNodeMovementTargets[*linkedNodeId];
                    bot->GetRandomPoint(data.pos, frand(-data.maxRadius, data.maxRadius), rx, ry, rz);
                }
                else
                    bot->GetRandomPoint(objPos, frand(-2.0f, 2.0f), rx, ry, rz);

                pos.Set(rx, ry, rz, BgObjective->GetMapId());
                posMap["bg objective"] = pos;
                return true;
            }

            break;
        }
        case BATTLEGROUND_AB:
        {
            if (!bg)
                break;

            BattlegroundAB* ab = static_cast<BattlegroundAB*>(bg);
            TeamId team = BgTeamIdOf(bot);

            uint8 role = context->GetValue<uint32>("bg role")->Get();
            ABBotStrategy strategyHorde = static_cast<ABBotStrategy>(GetBotStrategyForTeam(bg, TEAM_HORDE));
            ABBotStrategy strategyAlliance = static_cast<ABBotStrategy>(GetBotStrategyForTeam(bg, TEAM_ALLIANCE));
            ABBotStrategy strategy = (team == TEAM_ALLIANCE) ? strategyAlliance : strategyHorde;
            ABBotStrategy enemyStrategy = (team == TEAM_ALLIANCE) ? strategyHorde : strategyAlliance;

            uint8 defendersProhab = 3;
            if (strategy == AB_STRATEGY_OFFENSIVE)
                defendersProhab = 1;
            else if (strategy == AB_STRATEGY_DEFENSIVE)
                defendersProhab = 6;

            if (enemyStrategy == AB_STRATEGY_DEFENSIVE)
                defendersProhab = 2;

            bool isDefender = role < defendersProhab;
            bool isSilly = urand(0, 99) < 20;

            BgObjective = nullptr;

            // --- PRIORITY 1: Nearby enemy (rare aggressive impulse)
            if (urand(0, 99) < 5)
            {
                if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
                {
                    if (bot->GetDistance(enemy) < 500.0f)
                    {
                        pos.Set(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), bot->GetMapId());
                        posMap["bg objective"] = pos;
                        break;
                    }
                }
            }

            // --- PRIORITY 2: No valid nodes? Camp or attack visible enemy
            bool hasValidTarget = false;
            for (uint32 nodeId : AB_AttackObjectives)
            {
                uint8 state = ab->GetNodeStatus(nodeId);
                if (state == BG_AB_NODE_TYPE_NEUTRAL ||
                    (team == TEAM_ALLIANCE &&
                     (state == BG_AB_NODE_STATUS_HORDE_OCCUPIED || state == BG_AB_NODE_STATUS_HORDE_CONTESTED)) ||
                    (team == TEAM_HORDE &&
                     (state == BG_AB_NODE_STATUS_ALLY_OCCUPIED || state == BG_AB_NODE_STATUS_ALLY_CONTESTED)))
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
                        break;
                    }
                }

                // Camp enemy GY fallback
                Position camp = (team == TEAM_ALLIANCE) ? AB_GY_CAMPING_HORDE : AB_GY_CAMPING_ALLIANCE;
                float rx, ry, rz;
                bot->GetRandomPoint(camp, 10.0f, rx, ry, rz);
                pos.Set(rx, ry, rz, bot->GetMapId());
                posMap["bg objective"] = pos;
                break;
            }

            // --- PRIORITY 3: Defender logic ---
            if (isDefender && urand(0, 99) < 85)
            {
                float closestDist = FLT_MAX;
                for (uint32 nodeId : AB_AttackObjectives)
                {
                    uint8 state = ab->GetNodeStatus(nodeId);

                    bool isContested = (team == TEAM_ALLIANCE && state == BG_AB_NODE_STATUS_HORDE_CONTESTED) ||
                                       (team == TEAM_HORDE && state == BG_AB_NODE_STATUS_ALLY_CONTESTED);
                    bool isOwned = (team == TEAM_ALLIANCE && state == BG_AB_NODE_STATUS_ALLY_OCCUPIED) ||
                                   (team == TEAM_HORDE && state == BG_AB_NODE_STATUS_HORDE_OCCUPIED);

                    if (!isContested && !isOwned)
                        continue;

                    GameObject* go = bg->GetBGObject(nodeId * BG_AB_OBJECTS_PER_NODE);
                    if (!go)
                        continue;

                    float dist = bot->GetDistance(go);
                    if (dist < closestDist)
                    {
                        closestDist = dist;
                        BgObjective = go;
                    }
                }
            }

            // --- PRIORITY 4: Attack objectives ---
            if (!BgObjective)
            {
                std::vector<GameObject*> objectivePool;

                for (uint8 i = 0; i < 3; ++i)
                {
                    float bestDist = isSilly ? 0.0f : FLT_MAX;
                    GameObject* chosen = nullptr;

                    for (uint32 nodeId : AB_AttackObjectives)
                    {
                        uint8 state = ab->GetNodeStatus(nodeId);

                        bool isNeutral = state == BG_AB_NODE_TYPE_NEUTRAL;
                        bool isEnemyOccupied = (team == TEAM_ALLIANCE && state == BG_AB_NODE_STATUS_HORDE_OCCUPIED) ||
                                               (team == TEAM_HORDE && state == BG_AB_NODE_STATUS_ALLY_OCCUPIED);
                        bool isFriendlyContested =
                            (team == TEAM_ALLIANCE && state == BG_AB_NODE_STATUS_ALLY_CONTESTED) ||
                            (team == TEAM_HORDE && state == BG_AB_NODE_STATUS_HORDE_CONTESTED);

                        if (!(isNeutral || isEnemyOccupied || isFriendlyContested))
                            continue;

                        GameObject* go = bg->GetBGObject(nodeId * BG_AB_OBJECTS_PER_NODE);
                        if (!go || std::find(objectivePool.begin(), objectivePool.end(), go) != objectivePool.end())
                            continue;

                        float dist = bot->GetDistance(go);
                        if ((isSilly && dist > bestDist) || (!isSilly && dist < bestDist))
                        {
                            bestDist = dist;
                            chosen = go;
                        }
                    }

                    if (chosen)
                        objectivePool.push_back(chosen);
                }

                if (!objectivePool.empty())
                    BgObjective = objectivePool[urand(0, objectivePool.size() - 1)];
            }

            // --- Final move ---
            if (BgObjective)
            {
                float rx, ry, rz;
                Position objPos = BgObjective->GetPosition();
                bot->GetRandomPoint(objPos, frand(5.0f, 15.0f), rx, ry, rz);

                pos.Set(rx, ry, rz, BgObjective->GetMapId());
                posMap["bg objective"] = pos;
            }

            return true;
        }
        case BATTLEGROUND_EY:
        {
            BattlegroundEY* eyeOfTheStormBG = (BattlegroundEY*)bg;
            TeamId team = BgTeamIdOf(bot);
            uint8 role = context->GetValue<uint32>("bg role")->Get();

            EYBotStrategy strategyHorde = static_cast<EYBotStrategy>(GetBotStrategyForTeam(bg, TEAM_HORDE));
            EYBotStrategy strategyAlliance = static_cast<EYBotStrategy>(GetBotStrategyForTeam(bg, TEAM_ALLIANCE));
            EYBotStrategy strategy = (team == TEAM_ALLIANCE) ? strategyAlliance : strategyHorde;

            // GetPointOwnedByTeam returns a team id (ALLIANCE/HORDE), not a TeamId index.
            uint32 const ourTeamId = team == TEAM_ALLIANCE ? uint32(ALLIANCE) : uint32(HORDE);
            auto IsOwned = [&](uint32 nodeId) -> bool
            {
                return eyeOfTheStormBG->GetPointState(nodeId) == EY_POINT_UNDER_CONTROL &&
                       eyeOfTheStormBG->GetPointOwnedByTeam(nodeId) == ourTeamId;
            };

            uint8 defendersProhab = 4;
            switch (strategy)
            {
                case EY_STRATEGY_FLAG_FOCUS:
                    defendersProhab = 2;
                    break;
                case EY_STRATEGY_FRONT_FOCUS:
                case EY_STRATEGY_BACK_FOCUS:
                    defendersProhab = 3;
                    break;
                default:
                    defendersProhab = 4;
                    break;
            }

            bool isDefender = role < defendersProhab;

            std::tuple<uint32, uint32, uint32> front[2];
            std::tuple<uint32, uint32, uint32> back[2];

            if (team == TEAM_HORDE)
            {
                front[0] = EY_AttackObjectives[0];
                front[1] = EY_AttackObjectives[1];
                back[0] = EY_AttackObjectives[2];
                back[1] = EY_AttackObjectives[3];
            }
            else
            {
                front[0] = EY_AttackObjectives[2];
                front[1] = EY_AttackObjectives[3];
                back[0] = EY_AttackObjectives[0];
                back[1] = EY_AttackObjectives[1];
            }

            bool foundObjective = false;

            // --- PRIORITY 1: FLAG CARRIER ---
            if (bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
            {
                uint32 bestNodeId = 0;
                float bestDist = FLT_MAX;
                uint32 bestTrigger = 0;

                for (auto const& [nodeId, _, areaTrigger] : EY_AttackObjectives)
                {
                    if (!IsOwned(nodeId))
                        continue;

                    auto it = EY_NodePositions.find(nodeId);
                    if (it == EY_NodePositions.end())
                        continue;

                    float dist = bot->GetDistance(it->second);
                    if (dist < bestDist)
                    {
                        bestDist = dist;
                        bestNodeId = nodeId;
                        bestTrigger = areaTrigger;
                    }
                }

                if (bestNodeId != 0 && EY_NodePositions.contains(bestNodeId))
                {
                    Position const& targetPos = EY_NodePositions[bestNodeId];
                    float rx, ry, rz;
                    bot->GetRandomPoint(targetPos, 5.0f, rx, ry, rz);

                    pos.Set(rx, ry, rz, bot->GetMapId());

                    // Check AreaTrigger activation range
                    if (bestTrigger && bot->IsWithinDist3d(pos.x, pos.y, pos.z, INTERACTION_DISTANCE))
                    {
                        // See the note at the top of this file: CMSG_AREATRIGGER is bit-packed
                        // here, so go straight to the battleground's handler.
                        bg->HandleAreaTrigger(bot, bestTrigger, true);
                        pos.Reset();
                    }

                    foundObjective = true;
                }
                else
                {
                    Position const& fallback = (team == TEAM_ALLIANCE) ? EY_FLAG_RETURN_POS_RETREAT_ALLIANCE
                                                                       : EY_FLAG_RETURN_POS_RETREAT_HORDE;

                    float rx, ry, rz;
                    bot->GetRandomPoint(fallback, 5.0f, rx, ry, rz);

                    pos.Set(rx, ry, rz, bot->GetMapId());
                    foundObjective = true;
                }
            }

            // --- PRIORITY 2: Nearby unowned contested node ---
            if (!foundObjective)
            {
                for (auto const& [nodeId, _, __] : EY_AttackObjectives)
                {
                    if (IsOwned(nodeId))
                        continue;

                    auto it = EY_NodePositions.find(nodeId);
                    if (it == EY_NodePositions.end())
                        continue;

                    Position const& p = it->second;
                    if (bot->IsWithinDist2d(p.GetPositionX(), p.GetPositionY(), 125.0f))
                    {
                        float rx, ry, rz;
                        bot->GetRandomPoint(p, 5.0f, rx, ry, rz);
                        pos.Set(rx, ry, rz, bot->GetMapId());
                        foundObjective = true;
                    }
                }
            }

            // --- PRIORITY 3: Random nearby enemy (20%) ---
            if (!foundObjective && urand(0, 99) < 20)
            {
                if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
                {
                    if (bot->GetDistance(enemy) < 250.0f)
                    {
                        pos.Set(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), bot->GetMapId());
                        foundObjective = true;
                    }
                }
            }

            // --- PRIORITY 4: Defender Logic ---
            if (!foundObjective && isDefender && urand(0, 99) <= 80)
            {
                // 1. Chase enemy flag carrier
                if (Unit* enemyFC = AI_VALUE(Unit*, "enemy flag carrier"))
                {
                    if (bot->CanSeeOrDetect(enemyFC, false, false, true) && enemyFC->IsAlive())
                    {
                        pos.Set(enemyFC->GetPositionX(), enemyFC->GetPositionY(), enemyFC->GetPositionZ(), bot->GetMapId());
                        foundObjective = true;
                    }
                }

                // 2. Support friendly flag carrier
                if (!foundObjective)
                {
                    if (Unit* friendlyFC = AI_VALUE(Unit*, "team flag carrier"))
                    {
                        if (friendlyFC->IsAlive())
                        {
                            pos.Set(friendlyFC->GetPositionX(), friendlyFC->GetPositionY(), friendlyFC->GetPositionZ(), bot->GetMapId());
                            foundObjective = true;
                        }
                    }
                }

                // 3. Pick up the flag
                if (!foundObjective)
                {
                    if (GameObject* flag = bg->GetBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM); flag && flag->isSpawned())
                    {
                        pos.Set(flag->GetPositionX(), flag->GetPositionY(), flag->GetPositionZ(), flag->GetMapId());
                        foundObjective = true;
                    }
                }

                // 4. Default: defend owned node
                if (!foundObjective && urand(0, 99) < 50)
                {
                    std::vector<uint32> owned;
                    for (auto const& obj : front)
                    {
                        uint32 nodeId = std::get<0>(obj);
                        if (IsOwned(nodeId))
                            owned.push_back(nodeId);
                    }

                    if (!owned.empty())
                    {
                        uint32 chosenId = owned[urand(0, owned.size() - 1)];
                        if (EY_NodePositions.contains(chosenId))
                        {
                            Position const& p = EY_NodePositions[chosenId];
                            float rx, ry, rz;
                            bot->GetRandomPoint(p, 5.0f, rx, ry, rz);
                            pos.Set(rx, ry, rz, bot->GetMapId());
                            foundObjective = true;
                        }
                    }
                }
            }

            // --- PRIORITY 5: Flag Strategy ---
            if (!foundObjective && strategy == EY_STRATEGY_FLAG_FOCUS)
            {
                bool ownsAny = false;
                for (auto const& [nodeId, _, __] : EY_AttackObjectives)
                {
                    if (IsOwned(nodeId))
                    {
                        ownsAny = true;
                        break;
                    }
                }

                if (ownsAny)
                {
                    if (Unit* fc = AI_VALUE(Unit*, "enemy flag carrier"))
                    {
                        pos.Set(fc->GetPositionX(), fc->GetPositionY(), fc->GetPositionZ(), bot->GetMapId());
                        foundObjective = true;
                    }
                    else if (GameObject* flag = bg->GetBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM);
                             flag && flag->isSpawned())
                    {
                        pos.Set(flag->GetPositionX(), flag->GetPositionY(), flag->GetPositionZ(), flag->GetMapId());
                        foundObjective = true;
                    }
                }
                else
                {
                    float bestDist = FLT_MAX;
                    Optional<uint32> bestNode;

                    for (auto const& [nodeId, _, __] : EY_AttackObjectives)
                    {
                        if (IsOwned(nodeId))
                            continue;

                        auto it = EY_NodePositions.find(nodeId);
                        if (it == EY_NodePositions.end())
                            continue;

                        float dist = bot->GetDistance(it->second);
                        if (dist < bestDist)
                        {
                            bestDist = dist;
                            bestNode = nodeId;
                        }
                    }

                    if (bestNode && EY_NodePositions.contains(*bestNode))
                    {
                        Position const& p = EY_NodePositions[*bestNode];
                        float rx, ry, rz;
                        bot->GetRandomPoint(p, 5.0f, rx, ry, rz);
                        pos.Set(rx, ry, rz, bot->GetMapId());
                        foundObjective = true;
                    }
                }
            }

            // --- PRIORITY 6: Strategy Objectives ---
            if (!foundObjective)
            {
                std::vector<std::tuple<uint32, uint32, uint32>> priority, secondary;

                switch (strategy)
                {
                    case EY_STRATEGY_FRONT_FOCUS:
                        priority = {front[0], front[1]};
                        secondary = {back[0], back[1]};
                        break;
                    case EY_STRATEGY_BACK_FOCUS:
                        priority = {back[0], back[1]};
                        secondary = {front[0], front[1]};
                        break;
                    case EY_STRATEGY_BALANCED:
                    default:
                        priority = {front[0], front[1], back[0], back[1]};
                        break;
                }

                std::vector<uint32> candidates;

                for (auto const& obj : priority)
                {
                    uint32 nodeId = std::get<0>(obj);
                    if (!IsOwned(nodeId))
                        candidates.push_back(nodeId);
                }

                if (candidates.empty())
                {
                    for (auto const& obj : secondary)
                    {
                        uint32 nodeId = std::get<0>(obj);
                        if (!IsOwned(nodeId))
                            candidates.push_back(nodeId);
                    }
                }

                if (!candidates.empty())
                {
                    uint32 chosen = candidates[urand(0, candidates.size() - 1)];
                    if (EY_NodePositions.contains(chosen))
                    {
                        Position const& p = EY_NodePositions[chosen];
                        pos.Set(p.GetPositionX(), p.GetPositionY(), p.GetPositionZ(), bot->GetMapId());
                        foundObjective = true;
                    }
                }
            }

            // --- PRIORITY 7: Camp GY if everything is owned ---
            bool ownsAll = IsOwned(std::get<0>(front[0])) && IsOwned(std::get<0>(front[1])) &&
                           IsOwned(std::get<0>(back[0])) && IsOwned(std::get<0>(back[1]));

            if (!foundObjective && ownsAll)
            {
                Position camp = (team == TEAM_HORDE) ? EY_GY_CAMPING_ALLIANCE : EY_GY_CAMPING_HORDE;
                float rx, ry, rz;
                bot->GetRandomPoint(camp, 10.0f, rx, ry, rz);
                pos.Set(rx, ry, rz, bot->GetMapId());
                foundObjective = true;
            }

            if (foundObjective)
                posMap["bg objective"] = pos;

            return true;
        }
        // #576-#579: the MoP-only maps (their own files, see BattleGroundTactics.h).
        case BATTLEGROUND_BFG:
            return selectObjectiveBFG(bg);
        case BATTLEGROUND_TOK:
            return selectObjectiveTOK(bg);
        case BATTLEGROUND_SM:
            return selectObjectiveSM(bg);
        case BATTLEGROUND_DG:
            return selectObjectiveDG(bg);
        // #717 (beyond AC): Strand of the Ancients - AC has no SotA tactics (BgTacticsSA.cpp)
        case BATTLEGROUND_SA:
            return SotaBeyondAc::SelectObjective(bot, botAI, context, static_cast<BattlegroundSA*>(bg), BgTeamIdOf(bot),
                                                 context->GetValue<uint32>("bg role")->Get(), posMap);
        case BATTLEGROUND_IC:
        {
            BattlegroundIC* isleOfConquestBG = static_cast<BattlegroundIC*>(bg);
            // MoP adapter: the battleground team, not the race team (AC used team).
            TeamId const team = BgTeamIdOf(bot);
            // AC PR mod-playerbots#2856 (BattleGroundTactics.cpp:2849-2854): the Horde siege spot's ground height,
            // looked up once.
            static float const icParkZ = bg->GetBgMap()->GetHeight(IC_GATE_ATTACK_POS_HORDE.GetPositionX(),
                                                                    IC_GATE_ATTACK_POS_HORDE.GetPositionY(), 60.0f);
            Position icHordePark = IC_GATE_ATTACK_POS_HORDE;
            if (icParkZ > INVALID_HEIGHT)
                icHordePark.m_positionZ = icParkZ;

            uint32 role = context->GetValue<uint32>("bg role")->Get();
            bool inVehicle = botAI->IsInVehicle();
            bool controlsVehicle = botAI->IsInVehicle(true);
            uint32 vehicleId = inVehicle ? bot->GetVehicleBase()->GetEntry() : 0;

            // #675 (beyond AC, epic #205): keep cannons, catapult launch, the Siege Engine driver seat, seaforium
            // bombs (BgTacticsIC.cpp). Runs ahead of AC's "skip if not the driver" so a cannon gunner or a catapult
            // rider is handled too; returns false wherever AC's tactics below apply.
            if (IocBeyondAc::SelectObjective(bot, botAI, context, isleOfConquestBG, team, role, posMap))
                return true;

            // skip if not the driver
            if (inVehicle && !controlsVehicle)
                return false;

            // #738: the defence of our own breached keep (and of its banner) is IocBeyondAc::SelectObjective's
            // KeepDefence (BgTacticsIC.cpp), which runs above.

            /* TACTICS */
            if (team == TEAM_HORDE)  // HORDE
            {
                bool gateOpen = false;
                if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_DOODAD_PORTCULLISACTIVE02))
                {
                    if (pGO->isSpawned() && pGO->getLootState() == GO_ACTIVATED)
                    {
                        gateOpen = true;
                        PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                        siegePos.Reset();
                        posMap["bg siege"] = siegePos;
                    }
                }
                if (gateOpen && !controlsVehicle &&
                    isleOfConquestBG->GetNodeState(NODE_TYPE_GRAVEYARD_A) ==
                        NODE_STATE_CONTROLLED_H)  // target enemy boss
                {
                    if (Creature* enemyBoss = bg->GetBGCreature(BG_IC_NPC_HIGH_COMMANDER_HALFORD_WYRMBANE))
                    {
                        if (enemyBoss->IsVisible())
                        {
                            BgObjective = enemyBoss;
                            // LOG_INFO("playerbots", "bot={} attack boss", bot->GetName());
                        }
                    }
                }

                if (!BgObjective && gateOpen && !controlsVehicle)
                {
                    if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_ALLIANCE_BANNER))  // capture flag within keep
                    {
                        BgObjective = pGO;
                        // LOG_INFO("playerbots", "bot={} attack keep", bot->GetName());
                    }
                }

                if (!BgObjective && !gateOpen && controlsVehicle)  // attack gates
                {
                    // TODO: check for free vehicles
                    if (GameObject* gate = bg->GetBGObject(BG_IC_GO_ALLIANCE_GATE_3))
                    {
                        if (vehicleId == NPC_SIEGE_ENGINE_H)  // target gate directly if siege engine
                        {
                            BgObjective = gate;
                            // AC PR mod-playerbots#2856: the gate is the siege engine's siege position (Ram, IsSiegeShot)
                            PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                            siegePos.Set(gate->GetPositionX(), gate->GetPositionY(), gate->GetPositionZ(), bot->GetMapId());
                            posMap["bg siege"] = siegePos;
                            // LOG_INFO("playerbots", "bot={} (in siege-engine) attack gate", bot->GetName());
                        }
                        else  // target gate directly at range if other vehicle
                        {
                            // just make bot stay where it is if already close
                            // (stops them shifting around between the random spots)
                            if (bot->GetDistance(icHordePark) < 8.0f)
                                pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
                            else
                                pos.Set(icHordePark.GetPositionX() + frand(-5.0f, +5.0f),
                                        icHordePark.GetPositionY() + frand(-5.0f, +5.0f),
                                        icHordePark.GetPositionZ(), bot->GetMapId());
                            posMap["bg objective"] = pos;
                            // set siege position
                            PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                            siegePos.Set(gate->GetPositionX(), gate->GetPositionY(), gate->GetPositionZ(),
                                         bot->GetMapId());
                            posMap["bg siege"] = siegePos;
                            // LOG_INFO("playerbots", "bot={} (in vehicle={}) attack gate", bot->GetName(), vehicleId);
                            return true;
                        }
                    }
                }

                // If gates arent down and not in vehicle, split tasks
                if (!BgObjective && !controlsVehicle && role == 9)  // Capture Side base
                {
                    // Capture Refinery
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_REFINERY);
                    if (nodeState != NODE_STATE_CONFLICT_H && nodeState != NODE_STATE_CONTROLLED_H)
                    {
                        BgObjective = bg->GetBGObject(BG_IC_GO_REFINERY_BANNER);
                        // LOG_INFO("playerbots", "bot={} attack refinery", bot->GetName());
                    }
                }

                if (!BgObjective && !controlsVehicle && role < 3)  // Capture Docks
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_DOCKS);
                    if (nodeState != NODE_STATE_CONFLICT_H && nodeState != NODE_STATE_CONTROLLED_H)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_DOCKS_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack docks", bot->GetName());
                        }
                    }
                }
                if (!BgObjective && !controlsVehicle && role < 6)  // Capture Hangar
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_HANGAR);
                    if (nodeState != NODE_STATE_CONFLICT_H && nodeState != NODE_STATE_CONTROLLED_H)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_HANGAR_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack hangar", bot->GetName());
                        }
                    }
                }
                if (!BgObjective && !controlsVehicle)  // Capture Workshop
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_WORKSHOP);
                    if (nodeState != NODE_STATE_CONFLICT_H && nodeState != NODE_STATE_CONTROLLED_H)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_WORKSHOP_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack workshop", bot->GetName());
                        }
                    }
                }
                if (!BgObjective)  // Guard point that's not fully capped (also gets them in place to board vehicle)
                {
                    uint32 len = std::end(IC_AttackObjectives) - std::begin(IC_AttackObjectives);
                    for (uint32 i = 0; i < len; i++)
                    {
                        auto const& objective =
                            IC_AttackObjectives[(i + role) %
                                                len];  // use role to determine which objective checked first
                        if (isleOfConquestBG->GetNodeState(objective.first) != NODE_STATE_CONTROLLED_H)
                        {
                            if (GameObject* pGO = bg->GetBGObject(objective.second))
                            {
                                BgObjective = pGO;
                                // LOG_INFO("playerbots", "bot={} guard point while it captures", bot->GetName());
                                break;
                            }
                        }
                    }
                }
                if (!BgObjective)  // guard vehicles as they seige
                {
                    // just make bot stay where it is if already close
                    // (stops them shifting around between the random spots)
                    if (bot->GetDistance(icHordePark) < 8.0f)
                        pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
                    else
                        pos.Set(icHordePark.GetPositionX() + frand(-5.0f, +5.0f),
                                icHordePark.GetPositionY() + frand(-5.0f, +5.0f),
                                icHordePark.GetPositionZ(), bot->GetMapId());
                    posMap["bg objective"] = pos;
                    // LOG_INFO("playerbots", "bot={} guard vehicles as they attack gate", bot->GetName());
                    return true;
                }
            }

            if (team == TEAM_ALLIANCE)  // ALLIANCE
            {
                bool gateOpen = false;
                if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_HORDE_KEEP_PORTCULLIS))
                {
                    if (pGO->isSpawned() && pGO->getLootState() == GO_ACTIVATED)
                    {
                        gateOpen = true;
                        PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                        siegePos.Reset();
                        posMap["bg siege"] = siegePos;
                    }
                }

                if (gateOpen && !controlsVehicle &&
                    isleOfConquestBG->GetNodeState(NODE_TYPE_GRAVEYARD_H) ==
                        NODE_STATE_CONTROLLED_A)  // target enemy boss
                {
                    if (Creature* enemyBoss = bg->GetBGCreature(BG_IC_NPC_OVERLORD_AGMAR))
                    {
                        if (enemyBoss->IsVisible())
                        {
                            BgObjective = enemyBoss;
                            // LOG_INFO("playerbots", "bot={} attack boss", bot->GetName());
                        }
                    }
                }

                if (!BgObjective && gateOpen && !controlsVehicle)
                {
                    if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_HORDE_BANNER))  // capture flag within keep
                    {
                        BgObjective = pGO;
                        // LOG_INFO("playerbots", "bot={} attack keep", bot->GetName());
                    }
                }

                if (!BgObjective && !gateOpen && controlsVehicle)  // attack gates
                {
                    // TODO: check for free vehicles
                    if (GameObject* gate = bg->GetBGObject(BG_IC_GO_HORDE_GATE_1))
                    {
                        if (vehicleId == NPC_SIEGE_ENGINE_A)  // target gate directly if siege engine
                        {
                            BgObjective = gate;
                            // AC PR mod-playerbots#2856: the gate is the siege engine's siege position (Ram, IsSiegeShot)
                            PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                            siegePos.Set(gate->GetPositionX(), gate->GetPositionY(), gate->GetPositionZ(), bot->GetMapId());
                            posMap["bg siege"] = siegePos;
                            // LOG_INFO("playerbots", "bot={} (in siege-engine) attack gate", bot->GetName());
                        }
                        else  // target gate directly at range if other vehicle
                        {
                            // just make bot stay where it is if already close
                            // (stops them shifting around between the random spots)
                            if (bot->GetDistance(IC_GATE_ATTACK_POS_ALLIANCE) < 8.0f)
                                pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
                            else
                                pos.Set(IC_GATE_ATTACK_POS_ALLIANCE.GetPositionX() + frand(-5.0f, +5.0f),
                                        IC_GATE_ATTACK_POS_ALLIANCE.GetPositionY() + frand(-5.0f, +5.0f),
                                        IC_GATE_ATTACK_POS_ALLIANCE.GetPositionZ(), bot->GetMapId());
                            posMap["bg objective"] = pos;
                            // set siege position
                            PositionInfo siegePos = context->GetValue<PositionMap&>("position")->Get()["bg siege"];
                            siegePos.Set(gate->GetPositionX(), gate->GetPositionY(), gate->GetPositionZ(),
                                         bot->GetMapId());
                            posMap["bg siege"] = siegePos;
                            // LOG_INFO("playerbots", "bot={} (in vehicle={}) attack gate", bot->GetName(), vehicleId);
                            return true;
                        }
                    }
                }

                // If gates arent down and not in vehicle, split tasks
                if (!BgObjective && !controlsVehicle && role == 9)  // Capture Side base
                {
                    // Capture Refinery
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_QUARRY);
                    if (nodeState != NODE_STATE_CONFLICT_A && nodeState != NODE_STATE_CONTROLLED_A)
                    {
                        BgObjective = bg->GetBGObject(BG_IC_GO_QUARRY_BANNER);
                        // LOG_INFO("playerbots", "bot={} attack quarry", bot->GetName());
                    }
                }

                if (!BgObjective && !controlsVehicle && role < 3)  // Capture Docks
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_DOCKS);
                    if (nodeState != NODE_STATE_CONFLICT_A && nodeState != NODE_STATE_CONTROLLED_A)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_DOCKS_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack docks", bot->GetName());
                        }
                    }
                }
                if (!BgObjective && !controlsVehicle && role < 6)  // Capture Hangar
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_HANGAR);
                    if (nodeState != NODE_STATE_CONFLICT_A && nodeState != NODE_STATE_CONTROLLED_A)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_HANGAR_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack hangar", bot->GetName());
                        }
                    }
                }
                if (!BgObjective && !controlsVehicle)  // Capture Workshop
                {
                    uint32 const nodeState = isleOfConquestBG->GetNodeState(NODE_TYPE_WORKSHOP);
                    if (nodeState != NODE_STATE_CONFLICT_A && nodeState != NODE_STATE_CONTROLLED_A)
                    {
                        if (GameObject* pGO = bg->GetBGObject(BG_IC_GO_WORKSHOP_BANNER))
                        {
                            BgObjective = pGO;
                            // LOG_INFO("playerbots", "bot={} attack workshop", bot->GetName());
                        }
                    }
                }
                if (!BgObjective)  // Guard point that's not fully capped (also gets them in place to board vehicle)
                {
                    uint32 len = std::end(IC_AttackObjectives) - std::begin(IC_AttackObjectives);
                    for (uint32 i = 0; i < len; i++)
                    {
                        auto const& objective =
                            IC_AttackObjectives[(i + role) %
                                                len];  // use role to determine which objective checked first
                        // AC PR mod-playerbots#2856: the Alliance block compared against _H (copied from the
                        // Horde block), so idle Alliance bots guarded bases they already held
                        if (isleOfConquestBG->GetNodeState(objective.first) != NODE_STATE_CONTROLLED_A)
                        {
                            if (GameObject* pGO = bg->GetBGObject(objective.second))
                            {
                                BgObjective = pGO;
                                // LOG_INFO("playerbots", "bot={} guard point while it captures", bot->GetName());
                                break;
                            }
                        }
                    }
                }
                if (!BgObjective)  // guard vehicles as they seige
                {
                    // just make bot stay where it is if already close
                    // (stops them shifting around between the random spots)
                    if (bot->GetDistance(IC_GATE_ATTACK_POS_ALLIANCE) < 8.0f)
                        pos.Set(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), bot->GetMapId());
                    else
                        pos.Set(IC_GATE_ATTACK_POS_ALLIANCE.GetPositionX() + frand(-5.0f, +5.0f),
                                IC_GATE_ATTACK_POS_ALLIANCE.GetPositionY() + frand(-5.0f, +5.0f),
                                IC_GATE_ATTACK_POS_ALLIANCE.GetPositionZ(), bot->GetMapId());
                    posMap["bg objective"] = pos;
                    // LOG_INFO("playerbots", "bot={} guard vehicles as they attack gate", bot->GetName());
                    return true;
                }
            }

            if (BgObjective)
            {
                pos.Set(BgObjective->GetPositionX(), BgObjective->GetPositionY(), BgObjective->GetPositionZ(),
                        bot->GetMapId());
                posMap["bg objective"] = pos;
                return true;
            }
            break;
        }
        default:
            break;
    }

    return false;
}

bool BGTactics::moveToObjective(bool ignoreDist)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    if (!pos.isSet())
        return selectObjective();
    else
    {
        // Use portals in Isle of Conquest Base
        if (bgType == BATTLEGROUND_IC)
        {
            if (IsLockedInsideKeep())
                return true;
        }

        if (!ignoreDist && sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, pos.x, pos.y), 100.0f))
        {
            // std::ostringstream out;
            // out << "It is too far away! " << pos.x << ", " << pos.y << ", Distance: " <<
            // sServerFacade->GetDistance2d(bot, pos.x, pos.y); bot->Say(out.str(), LANG_UNIVERSAL);
            return false;
        }

        // don't try to move if already close
        if (bot->GetDistance(pos.x, pos.y, pos.z) < 4.0f)
        {
            resetObjective();
            return true;
        }

        // std::ostringstream out; out << "Moving to objective " << pos.x << ", " << pos.y << ", Distance: " <<
        // sServerFacade->GetDistance2d(bot, pos.x, pos.y); bot->Say(out.str(), LANG_UNIVERSAL);

        // dont increase from 1.5 will cause bugs with horde capping AV towers
        return MoveNear(bot->GetMapId(), pos.x, pos.y, pos.z, 1.5f);
    }
    return false;
}

namespace
{
    // issue #69: selectObjectiveWp scored every path from scratch on each call, so two near-equal
    // paths (and there are many in AB) flipped back and forth every tick and the bot oscillated
    // between waypoints instead of committing to a base. Remember the last choice per bot and keep
    // it unless another path is clearly better.
    struct BgPathChoice
    {
        BattleBotPath* path = nullptr;
        uint32 point = 0;
        bool reverse = false;
        float score = FLT_MAX;
    };

    thread_local std::map<ObjectGuid, BgPathChoice> s_bgPathChoice;
}

bool BGTactics::selectObjectiveWp(std::vector<BattleBotPath*> const& vPaths)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    if (!pos.isSet())
        return false;

    if (bgType == BATTLEGROUND_WS)
    {
        if (wsJumpDown())
            return true;
    }
    else if (bgType == BATTLEGROUND_AV)
    {
        // Get bots out of the cave when they respawn there (otherwise path selection happens while
        // they're deep within the cave and the results aren't good). AC-identical
        // (Ai/Base/Actions/BattleGroundTactics.cpp:3241-3249).
        Position const caveSpawn = BgTeamIdOf(bot) == TEAM_ALLIANCE ? AV_CAVE_SPAWN_ALLIANCE : AV_CAVE_SPAWN_HORDE;
        if (bot->GetDistance(caveSpawn) < 16.0f)
            return moveToStart(true);
    }
    else if (bgType == BATTLEGROUND_EY)
    {
        if (eyJumpDown())
            return true;
    }
    else if (bgType == BATTLEGROUND_TP)
    {
        if (tpJumpDown())
            return true;
    }

    float chosenPathScore = FLT_MAX;  // lower score is better
    BattleBotPath* chosenPath = nullptr;
    uint32 chosenPathPoint = 0;
    bool chosenPathReverse = false;

    float botDistanceLimit = 50.0f;         // limit for how far path can be from bot
    float botDistanceScoreSubtract = 8.0f;  // path score modifier - lower = less likely to chose a further path (it's
                                            // basically the distance from bot that's ignored)
    float botDistanceScoreMultiply =
        3.0f;  // path score modifier - higher = less likely to chose a further path (it's basically a multiplier on
               // distance from bot - makes distance from bot more signifcant than distance from destination)

    if (bgType == BATTLEGROUND_IC)
    {
        // botDistanceLimit = 80.0f;
        botDistanceScoreMultiply = 8.0f;
        botDistanceScoreSubtract = 2.0f;
    }
    else if (bgType == BATTLEGROUND_AV)
    {
        botDistanceLimit = 80.0f;
        botDistanceScoreMultiply = 8.0f;
    }
    else if (bgType == BATTLEGROUND_AB || bgType == BATTLEGROUND_EY || bgType == BATTLEGROUND_WS ||
             bgType == BATTLEGROUND_TP)
    {
        botDistanceScoreSubtract = 2.0f;
        botDistanceScoreMultiply = 4.0f;
    }

    // uint32 index = -1;
    // uint32 chosenPathIndex = -1;
    for (auto const& path : vPaths)
    {
        // index++;
        // TODO need to remove sqrt from these two and distToBot but it totally throws path scoring out of
        // whack if you do that without changing how its implemented (I'm amazed it works as well as it does
        // using sqrt'ed distances)
        // In a reworked version maybe compare the differences of path distances to point (ie: against best path)
        // or maybe ratio's (where if a path end is twice the difference in distance from destination we basically
        // use that to multiply the total score?
        BattleBotWaypoint& startPoint = ((*path)[0]);
        float const startPointDistToDestination =
            sqrt(Position(pos.x, pos.y, pos.z, 0.f).GetExactDist(startPoint.x, startPoint.y, startPoint.z));
        BattleBotWaypoint& endPoint = ((*path)[path->size() - 1]);
        float const endPointDistToDestination =
            sqrt(Position(pos.x, pos.y, pos.z, 0.f).GetExactDist(endPoint.x, endPoint.y, endPoint.z));

        bool reverse = startPointDistToDestination < endPointDistToDestination;

        // dont travel reverse if it's a reverse paths
        if (reverse && std::find(vPaths_NoReverseAllowed.begin(), vPaths_NoReverseAllowed.end(), path) !=
                           vPaths_NoReverseAllowed.end())
            continue;

        int closestPointIndex = -1;
        float closestPointDistToBot = FLT_MAX;
        for (uint32 i = 0; i < path->size(); i++)
        {
            BattleBotWaypoint& waypoint = ((*path)[i]);
            float const distToBot = sqrt(bot->GetDistance(waypoint.x, waypoint.y, waypoint.z));
            if (closestPointDistToBot > distToBot)
            {
                closestPointDistToBot = distToBot;
                closestPointIndex = i;
            }
        }

        // don't pick path where bot is already closest to the paths closest point to target (it means path cant lead it
        // anywhere) don't pick path where closest point is too far away
        if (closestPointIndex == int(reverse ? 0 : path->size() - 1) || closestPointDistToBot > botDistanceLimit)
            continue;

        // creates a score based on dist-to-bot and dist-to-destination, where lower is better, and dist-to-bot is more
        // important (when its beyond a certain distance) dist-to-bot is more important because otherwise they cant
        // reach it at all (or will fly through air with MM::MovePoint()), also bot may need to use multiple paths (one
        // after another) anyway
        float distToDestination = reverse ? startPointDistToDestination : endPointDistToDestination;
        float pathScore = (closestPointDistToBot < botDistanceScoreSubtract
                               ? 0.0f
                               : ((closestPointDistToBot - botDistanceScoreSubtract) * botDistanceScoreMultiply)) +
                          distToDestination;

        // LOG_INFO("playerbots", "bot={}\t{:6.1f}\t{:4.1f}\t{:4.1f}\t{}", bot->GetName(), pathScore,
        // closestPointDistToBot, distToDestination, vPaths_AB_name[pathNum]);

        if (chosenPathScore > pathScore)
        {
            chosenPathScore = pathScore;
            chosenPath = path;
            chosenPathPoint = closestPointIndex;
            chosenPathReverse = reverse;
            // chosenPathIndex = index;
        }
    }

    if (!chosenPath)
        return false;

    // Hysteresis (issue #69): stay on the path we already committed to unless the newly best path
    // is meaningfully better. Without this, two paths with near-equal scores alternate every call.
    // #873: only a path of this call's set is kept - a Twin Peaks carrier's set (vPaths_TP_Carrier*) leaves out the
    // enemy half's path it may have walked in on.
    BgPathChoice& last = s_bgPathChoice[bot->GetGUID()];
    if (last.path && last.path != chosenPath && chosenPathScore > last.score * 0.8f &&
        std::find(vPaths.begin(), vPaths.end(), last.path) != vPaths.end())
    {
        chosenPath = last.path;
        chosenPathReverse = last.reverse;
        chosenPathScore = last.score;

        // Recompute the closest point on the kept path, since the bot has moved since last tick.
        uint32 const lastPointInPath = chosenPathReverse ? 0 : (chosenPath->size() - 1);
        float closestPointDistToBot = FLT_MAX;
        chosenPathPoint = lastPointInPath;
        for (uint32 i = 0; i < chosenPath->size(); ++i)
        {
            BattleBotWaypoint& waypoint = (*chosenPath)[i];
            float const distToBot = sqrt(bot->GetDistance(waypoint.x, waypoint.y, waypoint.z));
            if (closestPointDistToBot > distToBot)
            {
                closestPointDistToBot = distToBot;
                chosenPathPoint = i;
            }
        }
    }

    last.path = chosenPath;
    last.point = chosenPathPoint;
    last.reverse = chosenPathReverse;
    last.score = chosenPathScore;

    // LOG_INFO("playerbots", "{} bot={} path={}", (BgTeamIdOf(bot) == TEAM_HORDE ? "HORDE" : "ALLIANCE"),
    // bot->GetName(), chosenPathIndex);

    return moveToObjectiveWp(chosenPath, chosenPathPoint, chosenPathReverse);
}

bool BGTactics::resetObjective()
{
    // issue #69: drop the sticky path choice so the next selectObjectiveWp starts clean.
    s_bgPathChoice.erase(bot->GetGUID());

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    // Adjust role-change chance based on battleground type
    uint32 oddsToChangeRole = 1;  // default low
    // As everywhere else in this file: a battleground entered through the Random Battleground queue
    // keeps BATTLEGROUND_RB as its type id, so ask for the real one. Upstream misses the unwrap
    // here, which left every Random Battleground on the default odds.
    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    // #576-#579: carrier maps (TOK, DG) take WSG's odds, node/escort maps (BFG, SM) AB's.
    if (bgType == BATTLEGROUND_WS || bgType == BATTLEGROUND_TP || bgType == BATTLEGROUND_TOK ||
        bgType == BATTLEGROUND_DG)
        oddsToChangeRole = 2;
    else if (bgType == BATTLEGROUND_EY || bgType == BATTLEGROUND_IC || bgType == BATTLEGROUND_AB ||
             bgType == BATTLEGROUND_BFG || bgType == BATTLEGROUND_SM)
        oddsToChangeRole = 1;
    else if (bgType == BATTLEGROUND_AV || bgType == BATTLEGROUND_SA)  // #717: SotA roles are posts (driver, gunner)
        oddsToChangeRole = 0;

    bool isCarryingFlag = isCarryingObjective(bg);

    // Change role if allowed by odds and not carrying flag
    if (urand(0, 99) < oddsToChangeRole && !isCarryingFlag)
        context->GetValue<uint32>("bg role")->Set(urand(0, 9));

    // Reset objective position
    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    pos.Reset();
    posMap["bg objective"] = pos;

    return selectObjective(true);
}

bool BGTactics::moveToObjectiveWp(BattleBotPath* const& currentPath, uint32 currentPoint, bool reverse)
{
    if (!currentPath)
        return false;

    uint32 const lastPointInPath = reverse ? 0 : ((*currentPath).size() - 1);

    // NOTE: can't use IsInCombat() when in vehicle as player is stuck in combat forever while in vehicle (ac bug?)
    bool inCombat = bot->GetVehicle() ? (bool)AI_VALUE(Unit*, "enemy player target") : bot->IsInCombat();
    if (currentPoint == lastPointInPath || (inCombat && !PlayerHasFlag::IsCapturingFlag(bot)) || !bot->IsAlive())
    {
        // Path is over.
        // std::ostringstream out;
        // out << "Reached path end!";
        // bot->Say(out.str(), LANG_UNIVERSAL);
        resetObjective();
        return false;
    }

    uint32 currPoint = currentPoint;

    if (reverse)
        currPoint--;
    else
        currPoint++;

    // #747: a swimming bot on Twin Peaks steps one waypoint at a time (water legs, see MovementActions.cpp MoveTo).
    uint32 const step = (bot->GetMapId() == 726 && bot->isSwimming()) ? 1 : urand(1, 5);
    uint32 nPoint = reverse ? std::max((int)(currPoint - step), 0)
                            : std::min((uint32)(currPoint + step), lastPointInPath);
    BattleBotWaypoint& nextPoint = currentPath->at(nPoint);

    // std::ostringstream out;
    // out << "WP: ";
    // reverse ? out << currPoint << " <<< -> " << nPoint : out << currPoint << ">>> ->" << nPoint;
    // out << ", " << nextPoint.x << ", " << nextPoint.y << " Path Size: " << currentPath->size() << ", Dist: " <<
    // sServerFacade->GetDistance2d(bot, nextPoint.x, nextPoint.y); bot->Say(out.str(), LANG_UNIVERSAL);

    return MoveTo(bot->GetMapId(), nextPoint.x + frand(-2, 2), nextPoint.y + frand(-2, 2), nextPoint.z);
}

bool BGTactics::startNewPathBegin(std::vector<BattleBotPath*> const& vPaths)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    if (bgType == BATTLEGROUND_IC)
        return false;

    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    if (!pos.isSet())
        return false;

    // Important for eye of the storm bg, so that bots stay near bg objective (see selectObjective for more Info)
    if (bot->GetDistance(pos.x, pos.y, pos.z) < 25.0f)
        return false;

    struct AvailablePath
    {
        AvailablePath(BattleBotPath* pPath_, bool reverse_) : pPath(pPath_), reverse(reverse_) {}

        BattleBotPath* pPath = nullptr;
        bool reverse = false;
    };

    std::vector<AvailablePath> availablePaths;
    for (auto const& pPath : vPaths)
    {
        // TODO remove sqrt
        BattleBotWaypoint* pStart = &((*pPath)[0]);
        if (sqrt(bot->GetDistance(pStart->x, pStart->y, pStart->z)) < INTERACTION_DISTANCE)
            availablePaths.emplace_back(AvailablePath(pPath, false));

        // Some paths are not allowed backwards.
        if (std::find(vPaths_NoReverseAllowed.begin(), vPaths_NoReverseAllowed.end(), pPath) !=
            vPaths_NoReverseAllowed.end())
            continue;

        // TODO remove sqrt
        BattleBotWaypoint* pEnd = &((*pPath)[(*pPath).size() - 1]);
        if (sqrt(bot->GetDistance(pEnd->x, pEnd->y, pEnd->z)) < INTERACTION_DISTANCE)
            availablePaths.emplace_back(AvailablePath(pPath, true));
    }

    if (availablePaths.empty())
        return false;

    uint32 randomPath = urand(0, availablePaths.size() - 1);
    AvailablePath const* chosenPath = &availablePaths[randomPath];

    BattleBotPath* currentPath = chosenPath->pPath;
    bool reverse = chosenPath->reverse;
    uint32 currentPoint = reverse ? currentPath->size() - 1 : 0;

    return moveToObjectiveWp(currentPath, currentPoint, reverse);
}

bool BGTactics::startNewPathFree(std::vector<BattleBotPath*> const& vPaths)
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    if (bgType == BATTLEGROUND_IC)
        return false;

    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()["bg objective"];
    if (!pos.isSet())
        return false;

    // Important for eye of the storm bg, so that bots stay near bg objective (see selectObjective for more Info)
    if (bot->GetDistance(pos.x, pos.y, pos.z) < 25.0f)
        return false;

    BattleBotPath* pClosestPath = nullptr;
    uint32 closestPoint = 0;
    float closestDistance = FLT_MAX;

    for (auto const& pPath : vPaths)
    {
        for (uint32 i = 0; i < pPath->size(); i++)
        {
            BattleBotWaypoint& waypoint = ((*pPath)[i]);
            // TODO remove sqrt
            float const distanceToPoint = sqrt(bot->GetDistance(waypoint.x, waypoint.y, waypoint.z));
            if (distanceToPoint < closestDistance)
            {
                pClosestPath = pPath;
                closestPoint = i;
                closestDistance = distanceToPoint;
            }
        }
    }

    if (!pClosestPath)
        return false;

    BattleBotPath* currentPath = pClosestPath;
    bool reverse = false;
    uint32 currentPoint = closestPoint - 1;

    return moveToObjectiveWp(currentPath, currentPoint, reverse);
}

/**
 * @brief Handles flag/base capturing gameplay in battlegrounds
 *
 * This function manages the logic for capturing flags and bases in various battlegrounds.
 * It handles:
 * - Enemy detection and combat near objectives
 * - Coordination with friendly players who are capturing
 * - Different capture mechanics for each battleground type
 * - Proper positioning and movement
 *
 * @param vPaths Vector of possible paths the bot can take
 * @param vFlagIds Vector of flag/base GameObjects that can be captured
 * @return true if handling a flag/base action, false otherwise
 */
bool BGTactics::atFlag(std::vector<BattleBotPath*> const& vPaths, std::vector<uint32> const& vFlagIds)
{
    // Basic sanity checks
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    // Get the actual BG type (in case of random BG)
    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    // Initialize vectors for nearby objects and players
    GuidVector closeObjects;
    GuidVector closePlayers;
    float flagRange = 0.0f;

    // Eye of the Storm helpers used later when handling capture positioning
    BattlegroundEY* eyeBg = nullptr;
    GameObject* eyCenterFlag = nullptr;
    if (bgType == BATTLEGROUND_EY)
    {
        eyeBg = static_cast<BattlegroundEY*>(bg);
        if (eyeBg)
            eyCenterFlag = eyeBg->GetBGObject(BG_EY_OBJECT_FLAG_NETHERSTORM);
    }

    // Set up appropriate search ranges and object lists based on BG type
    switch (bgType)
    {
        case BATTLEGROUND_AV:
        case BATTLEGROUND_AB:
        case BATTLEGROUND_IC:
        {
            // For territory control BGs, use standard interaction range
            closeObjects = *context->GetValue<GuidVector>("closest game objects");
            closePlayers = *context->GetValue<GuidVector>("closest friendly players");
            flagRange = INTERACTION_DISTANCE;
            break;
        }
        case BATTLEGROUND_WS:
        case BATTLEGROUND_TP:
        case BATTLEGROUND_EY:
        {
            // For flag carrying BGs, use wider range and ignore LOS
            closeObjects = *context->GetValue<GuidVector>("nearest game objects no los");
            closePlayers = *context->GetValue<GuidVector>("closest friendly players");
            flagRange = 25.0f;
            break;
        }
        default:
            break;
    }

    if (closeObjects.empty())
        return false;

    auto keepStationaryWhileCapturing = [&](CurrentSpellTypes spellType)
    {
        Spell* currentSpell = bot->GetCurrentSpell(spellType);
        if (!currentSpell || !currentSpell->m_spellInfo || currentSpell->m_spellInfo->Id != SPELL_CAPTURE_BANNER)
            return false;

        // If the capture target is no longer available (another bot already captured it), stop channeling
        GameObject* targetFlag = currentSpell->m_targets.GetGOTarget();
        if (!targetFlag || !targetFlag->isSpawned() || targetFlag->GetGoState() != GO_STATE_READY)
        {
            bot->CastStop();
            resetObjective();
            return false;
        }

        if (bot->IsMounted())
        {
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        }

        if (bot->IsInDisallowedMountForm())
        {
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        }

        if (bot->isMoving())
        {
            bot->StopMoving();
        }

        return true;
    };

    // If we are already channeling the capture spell, keep the bot stationary and dismounted
    if (keepStationaryWhileCapturing(CURRENT_CHANNELED_SPELL) || keepStationaryWhileCapturing(CURRENT_GENERIC_SPELL))
        return true;

    // First identify which flag/base we're trying to interact with
    GameObject* targetFlag = nullptr;
    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
        {
            continue;
        }

        bool const isEyCenterFlag = eyeBg && eyCenterFlag && eyCenterFlag->GetGUID() == go->GetGUID();

        // Check if this object is a valid capture target
        std::vector<uint32>::const_iterator f = std::find(vFlagIds.begin(), vFlagIds.end(), go->GetEntry());
        if (f == vFlagIds.end() && !isEyCenterFlag)
        {
            continue;
        }

        // Verify the object is active and ready
        if (!go->isSpawned() || go->GetGoState() != GO_STATE_READY)
        {
            continue;
        }

        // Check if we're in range (using double range for enemy detection)
        float const dist = bot->GetDistance(go);
        if (flagRange && dist > flagRange * 2.0f)
        {
            continue;
        }

        targetFlag = go;
        break;
    }

    // If we found a valid flag/base to interact with
    if (bgType != BATTLEGROUND_WS && bgType != BATTLEGROUND_TP && bgType != BATTLEGROUND_AV)
    {
        if (targetFlag)
        {
            // Check for enemy players near the flag using bot's targeting system
            Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");
            if (enemyPlayer && enemyPlayer->IsAlive())
            {
                // If enemy is near the flag, engage them before attempting capture
                float enemyDist = enemyPlayer->GetDistance(targetFlag);
                if (enemyDist < flagRange * 2.0f)
                {
                    // Set enemy as current target and let combat AI handle it
                    context->GetValue<Unit*>("current target")->Set(enemyPlayer);
                    return false;
                }
            }
        }
    }

    // Check if friendly players are already capturing
    if (!closePlayers.empty() && bgType != BATTLEGROUND_EY)
    {
        // Track number of friendly players capturing and the closest one
        uint32 numCapturing = 0;
        Unit* capturingPlayer = nullptr;
        for (auto& guid : closePlayers)
        {
            if (Unit* pFriend = botAI->GetUnit(guid))
            {
                // Check if they're casting or channeling the capture spell
                Spell* spell = pFriend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
                if (!spell)
                {
                    spell = pFriend->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
                }

                if (spell && spell->m_spellInfo && spell->m_spellInfo->Id == SPELL_CAPTURE_BANNER)
                {
                    numCapturing++;
                    capturingPlayer = pFriend;
                }
            }
        }

        // If friendlies are capturing, stay to defend but don't capture
        if (numCapturing > 0 && capturingPlayer && bot->GetGUID() != capturingPlayer->GetGUID())
        {
            // Move away if too close to avoid crowding
            if (bot->GetDistance2d(capturingPlayer) < 3.0f)
            {
                float angle = bot->GetAngle(capturingPlayer);
                float x = bot->GetPositionX() + 5.0f * cos(angle);
                float y = bot->GetPositionY() + 5.0f * sin(angle);
                MoveTo(bot->GetMapId(), x, y, bot->GetPositionZ());
            }

            // Reset objective and take new path for defending
            resetObjective();
            if (!startNewPathBegin(vPaths))
                moveToObjective(true);
            return true;
        }
    }

    // Area is clear of enemies and no friendlies are capturing
    // Proceed with capture mechanics
    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        bool const isEyCenterFlag = eyeBg && eyCenterFlag && eyCenterFlag->GetGUID() == go->GetGUID();

        // Validate this is a capture target
        std::vector<uint32>::const_iterator f = std::find(vFlagIds.begin(), vFlagIds.end(), go->GetEntry());
        if (f == vFlagIds.end() && !isEyCenterFlag)
            continue;

        // Issue #56: report why a recognised flag was skipped. Gated to objects already matched
        // against vFlagIds, so at most a couple of lines per bot per tick.
        TC_LOG_DEBUG("playerbots", "[BGCAP] %s: flag %u dist=%.1f range=%.1f spawned=%u state=%u canUse=%u",
                     bot->GetName().c_str(), go->GetEntry(), bot->GetDistance(go), flagRange,
                     uint32(go->isSpawned()), uint32(go->GetGoState()),
                     uint32(bot->CanUseBattlegroundObject(go)));

        // Check object is active
        if (!go->isSpawned() || go->GetGoState() != GO_STATE_READY)
            continue;

        // Verify we can interact with it
        if (!bot->CanUseBattlegroundObject(go) && bgType != BATTLEGROUND_WS && bgType != BATTLEGROUND_TP)
            continue;

        float const dist = bot->GetDistance(go);
        if (flagRange && dist > flagRange)
            continue;

        // Special handling for WSG and EY base flags
        // Which flag is "ours" is decided by the BATTLEGROUND team, not the bot's natural faction.
        // This core places players on either side regardless of race (see the cross-faction handling
        // in BattlegroundWS::EventPlayerClickedOnFlag), so for any bot whose BG team differs from its
        // race's team, GetTeamId() names the wrong flag - the guard below then protects the enemy
        // flag and lets the bot click its own, which EventPlayerClickedOnFlag silently ignores.
        // That is why bots stood on the flag clicking it forever without ever picking it up.
        bool isWsBaseFlag = (bgType == BATTLEGROUND_WS || bgType == BATTLEGROUND_TP) &&
                            go->GetEntry() == vFlagIds[BgTeamIdOf(bot)];
        bool isEyBaseFlag = bgType == BATTLEGROUND_EY && go->GetEntry() == vFlagsEY[0];

        // issue #57: log the flag-carrier decision path (all bots, every 10 s, only when the bot is
        // carrying a flag or standing at its own base flag).
        bool const carryingFlag =
            bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG);
        if ((bgType == BATTLEGROUND_WS || bgType == BATTLEGROUND_TP) && (carryingFlag || isWsBaseFlag))
        {
            static thread_local std::map<ObjectGuid, uint32> lastLog;
            uint32 const now = getMSTime();
            uint32& last = lastLog[bot->GetGUID()];
            if (!last || getMSTimeDiff(last, now) >= 10000)
            {
                last = now;
                TC_LOG_DEBUG("playerbots",
                             "[BGF] %s flag%u own=%d carrying=%d dist=%.1f ownFlagTaken=%d teamFlagState=%u",
                             bot->GetName().c_str(), go->GetEntry(), isWsBaseFlag ? 1 : 0, carryingFlag ? 1 : 0,
                             dist, teamFlagTaken() ? 1 : 0,
                             // #674: GetFlagState takes a Team (ALLIANCE/HORDE), not a TeamId, and a Twin Peaks match is a
                             // BattlegroundTP, whose members are not where BattlegroundWS keeps them.
                             uint32(bgType == BATTLEGROUND_TP
                                        ? static_cast<BattlegroundTP*>(bg)->GetFlagState(bot->GetBGTeam())
                                        : static_cast<BattlegroundWS*>(bg)->GetFlagState(bot->GetBGTeam())));
            }
        }

        // Ensure bots are inside the Eye of the Storm capture circle before casting
        if (bgType == BATTLEGROUND_EY)
        {
            GameObject* captureFlag = (isEyBaseFlag && eyCenterFlag) ? eyCenterFlag : go;
            float const requiredRange = 2.5f;
            if (!bot->IsWithinDistInMap(captureFlag, requiredRange))
            {
                // Stay mounted while relocating to avoid mount/dismount loops
                return MoveTo(bot->GetMapId(), captureFlag->GetPositionX(), captureFlag->GetPositionY(),
                              captureFlag->GetPositionZ());
            }

            // Once inside the circle, dismount and stop before starting the channel
            if (bot->IsMounted())
            {
                bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
            }

            if (bot->IsInDisallowedMountForm())
            {
                bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
            }

            if (bot->isMoving())
            {
                bot->StopMoving();
            }
        }

        // Don't capture own flag in a two-flag battleground unless carrying the enemy flag.
        // Twin Peaks uses the same two flag spells (23333 / 23335), so the aura test is unchanged.
        if (isWsBaseFlag && (bgType == BATTLEGROUND_WS || bgType == BATTLEGROUND_TP) &&
            !(bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG)))
            continue;

        // Handle capture mechanics based on BG type
        switch (bgType)
        {
            case BATTLEGROUND_AV:
            case BATTLEGROUND_AB:
            case BATTLEGROUND_IC:
            {
                // #754 (beyond AC): fight whoever is hitting the bot before starting the capture cast (see CaptureThreat)
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

                // Prevent capturing from inside flag pole
                if (dist == 0.0f)
                {
                    float const moveDist = bot->GetObjectSize() + go->GetObjectSize() + 0.1f;
                    return MoveTo(bot->GetMapId(), go->GetPositionX() + (urand(0, 1) ? -moveDist : moveDist),
                                  go->GetPositionY() + (urand(0, 1) ? -moveDist : moveDist), go->GetPositionZ());
                }

                // Dismount before capturing
                if (bot->IsMounted())
                    bot->RemoveAurasByType(SPELL_AURA_MOUNTED);

                if (bot->IsInDisallowedMountForm())
                    bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

                // Cast the capture spell
                SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_CAPTURE_BANNER);
                if (!spellInfo)
                    return false;

                Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
                spell->m_targets.SetGOTarget(go);
                spell->prepare(&spell->m_targets);

                botAI->WaitForSpellCast(spell);

                resetObjective();
                return true;
            }
            case BATTLEGROUND_WS:
            case BATTLEGROUND_TP:
            {
                // Twin Peaks is the same game; only the area trigger ids differ. Without this case
                // a Twin Peaks bot passed every check above and then did nothing at all - including
                // the MoveTo below, which is what closes the last 25 yards to the flag. That is why
                // bots hovered around 15-25 yd from both flag stands and never took one.
                bool const isTpBg = bgType == BATTLEGROUND_TP;
                if (dist < INTERACTION_DISTANCE)
                {
                    // Handle flag capture at base
                    if (isWsBaseFlag)
                    {
                        if (BgTeamIdOf(bot) == TEAM_HORDE)
                        {
                            // See the note at the top of this file: CMSG_AREATRIGGER is
                            // bit-packed here, so go straight to the battleground's handler.
                            bg->HandleAreaTrigger(bot, isTpBg ? BG_TP_TRIGGER_HORDE_FLAG_SPAWN
                                                             : BG_WS_TRIGGER_HORDE_FLAG_SPAWN, true);
                        }
                        else
                        {
                            // See the note at the top of this file: CMSG_AREATRIGGER is
                            // bit-packed here, so go straight to the battleground's handler.
                            bg->HandleAreaTrigger(bot, isTpBg ? BG_TP_TRIGGER_ALLIANCE_FLAG_SPAWN
                                                             : BG_WS_TRIGGER_ALLIANCE_FLAG_SPAWN, true);
                        }
                        return true;
                    }

                    // Dismount before picking up flag
                    if (bot->IsMounted())
                        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);

                    if (bot->IsInDisallowedMountForm())
                        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

                    // Pick up the flag
                    // See the note at the top of this file: CMSG_GAMEOBJ_USE is bit-packed here.
                    TC_LOG_DEBUG("playerbots", "[BGF] %s (bgTeam %u) takes flag %u at %.1f yd",
                                 bot->GetName().c_str(), uint32(bot->GetBGTeam()), go->GetEntry(), dist);
                    bot->ResetEmoteStateIfNeed();
                    go->Use(bot);

                    resetObjective();
                    return true;
                }
                else
                {
                    // Move to flag if not in range
                    return MoveTo(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                }
            }
            case BATTLEGROUND_EY:
            {  // Handle Netherstorm flag capture requiring a channel
                if (dist < INTERACTION_DISTANCE)
                {
                    // Dismount before interacting
                    if (bot->IsMounted())
                    {
                        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
                    }

                    if (bot->IsInDisallowedMountForm())
                    {
                        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
                    }

                    // Handle center flag differently (requires spell cast)
                    if (isEyCenterFlag)
                    {
                        for (uint8 type = CURRENT_MELEE_SPELL; type <= CURRENT_CHANNELED_SPELL; ++type)
                        {
                            if (Spell* currentSpell = bot->GetCurrentSpell(static_cast<CurrentSpellTypes>(type)))
                            {
                                // m_spellInfo may be null in some states: protect access
                                if (currentSpell->m_spellInfo && currentSpell->m_spellInfo->Id == SPELL_CAPTURE_BANNER)
                                {
                                    bot->StopMoving();
                                    botAI->SetNextCheckDelay(500);
                                    return true;
                                }
                            }
                        }

                        // #754 (beyond AC): fight first, as in the AV/AB/IC case above
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

                        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(SPELL_CAPTURE_BANNER);
                        if (!spellInfo)
                            return false;

                        Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
                        spell->m_targets.SetGOTarget(go);

                        bot->StopMoving();
                        spell->prepare(&spell->m_targets);

                        botAI->WaitForSpellCast(spell);
                        resetObjective();
                        return true;
                    }

                    // Pick up dropped flag
                    // See the note at the top of this file: CMSG_GAMEOBJ_USE is bit-packed here.
                    TC_LOG_DEBUG("playerbots",
                                 "[BGF] %s: using flag %u at %.1f yd | canUse=%u alive=%u bgTeam=%u "
                                 "status=%u immune=%u recentlyDropped=%u gotype=%u",
                                 bot->GetName().c_str(), go->GetEntry(), dist,
                                 uint32(bot->CanUseBattlegroundObject(go)), uint32(bot->IsAlive()),
                                 uint32(bot->GetBGTeam()), uint32(bg->GetStatus()),
                                 uint32(bot->isTotalImmune()), uint32(bot->HasAura(SPELL_RECENTLY_DROPPED_FLAG)),
                                 uint32(go->GetGoType()));
                    bot->ResetEmoteStateIfNeed();
                    go->Use(bot);

                    resetObjective();
                    return true;
                }
                else
                {
                    // Move to flag if not in range
                    return MoveTo(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
                }
            }
            default:
                break;
        }
    }

    return false;
}

// #577/#579: AC tests only the WSG/EotS flag auras at each "carrying" site; the Kotmogu orb and the
// Deepwind cart are carried objectives too, so every site asks here instead.
bool BGTactics::isCarryingObjective(Battleground* bg)
{
    if (bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) || bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) ||
        bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL))
        return true;

    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    if (bgType == BATTLEGROUND_TOK)
        return isCarryingTOK(bg);
    if (bgType == BATTLEGROUND_DG)
        return isCarryingDG(bg);

    return false;
}

bool BGTactics::flagTaken()
{
    BattlegroundWS* bg = (BattlegroundWS*)bot->GetBattleground();
    if (!bg)
        return false;

    return !bg->GetFlagPickerGUID(bg->GetOtherTeam(BgTeamIdOf(bot))).IsEmpty();
}

bool BGTactics::teamFlagTaken()
{
    BattlegroundWS* bg = (BattlegroundWS*)bot->GetBattleground();
    if (!bg)
        return false;

    return !bg->GetFlagPickerGUID(BgTeamIdOf(bot)).IsEmpty();
}

bool BGTactics::protectFC()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    Unit* teamFC = AI_VALUE(Unit*, "team flag carrier");

    if (!teamFC || teamFC == bot)
    {
        return false;
    }

    if (!bot->IsInCombat() && !bot->IsWithinDistInMap(teamFC, 20.0f))
    {
        // Get the flag carrier's position
        float fcX = teamFC->GetPositionX();
        float fcY = teamFC->GetPositionY();
        float fcZ = teamFC->GetPositionZ();
        uint32 mapId = bot->GetMapId();

        return MoveNear(mapId, fcX, fcY, fcZ, 5.0f, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}

bool BGTactics::useBuff()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    GuidVector closeObjects = AI_VALUE(GuidVector, "nearest game objects no los");
    if (closeObjects.empty())
        return false;

    bool needRegen = bot->GetHealthPct() < sPlayerbotAIConfig->mediumHealth ||
                     (AI_VALUE2(bool, "has mana", "self target") &&
                      AI_VALUE2(uint8, "mana", "self target") < sPlayerbotAIConfig->mediumMana);
    bool needSpeed = ((bgType != BATTLEGROUND_WS && bgType != BATTLEGROUND_TP) ||
                      bot->HasAura(BG_WS_SPELL_WARSONG_FLAG) ||
                      bot->HasAura(BG_WS_SPELL_SILVERWING_FLAG) || bot->HasAura(BG_EY_NETHERSTORM_FLAG_SPELL)) ||
                     !(teamFlagTaken() || flagTaken());
    bool foundBuff = false;

    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        if (!go->isSpawned())
            continue;

        // use speed buff only if close
        if (sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, go),
                                                 go->GetEntry() == Buff_Entries[0] ? 20.0f : 50.0f))
            continue;

        if (needSpeed && go->GetEntry() == Buff_Entries[0])
            foundBuff = true;

        if (needRegen && go->GetEntry() == Buff_Entries[1])
            foundBuff = true;

        // do not move to Berserk buff if bot is healer or has flag (a Kotmogu orb / Deepwind cart counts, #577/#579)
        if (!isCarryingObjective(bg) && !PlayerBotSpec::IsHeal(bot) && go->GetEntry() == Buff_Entries[2])
            foundBuff = true;

        if (foundBuff)
        {
            // std::ostringstream out;
            // out << "Moving to buff...";
            // bot->Say(out.str(), LANG_UNIVERSAL);
            return MoveTo(go->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ());
        }
    }

    return false;
}

uint32 BGTactics::getPlayersInArea(TeamId teamId, Position point, float range, bool combat)
{
    uint32 defCount = 0;

    if (!bot->InBattleground())
        return false;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return 0;

    for (auto& guid : bg->GetBgMap()->GetPlayers())
    {
        Player* player = guid.GetSource();
        if (!player)
            continue;

        if (player->IsAlive() && (teamId == TEAM_NEUTRAL || teamId == BgTeamIdOf(player)))
        {
            if (!combat && player->IsInCombat())
                continue;

            if (sServerFacade->GetDistance2d(player, point.GetPositionX(), point.GetPositionY()) < range)
                ++defCount;
        }
    }

    return defCount;
}

// #754 (BEYOND AC). The capture cast (21651 banners, 97388 DG nodes) is interrupted by damage, and the AI then sleeps
// out WaitForSpellCast. AC atFlag stands down only for an enemy PLAYER from "enemy player target" within 2 x flagRange
// (10 yd) of the banner, and never in AV/WSG (AC BattleGroundTactics.cpp:3706-3722, MoP atFlag same). That value skips
// the bot's own melee victim (AC EnemyPlayerValue.cpp:61,95), so a bot already swinging at its attacker, or hit by a ranged
// player, a pet or a guard, re-casts into the interrupt (owner report 2026-10-08; IoC probes on both cores).
Unit* BGTactics::CaptureThreat(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInWorld())
        return nullptr;

    // Only units FIGHTING this bot count - an enemy that is merely nearby is ignored (owner 2026-10-08: keep defending
    // while attacked, then go back to the flag). Units in its attackers list count at any distance (they are attacking
    // it); players in PvP combat with it (a caster/hunter hitting it is not in getAttackers(), only Unit::Attack puts
    // a unit there; the reference also lingers a few seconds after the last hostile action) count within 50 yd, which
    // covers every ranged attack, so one who hit it and ran far away does not keep it off the flag.
    float constexpr pvpRange = 50.0f;
    Unit* best = nullptr;
    float bestDist = 0.0f;
    auto const consider = [&](Unit* unit, float maxDist)
    {
        if (!unit || unit == bot || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() ||
            !bot->IsValidAttackTarget(unit))
            return;

        float const dist = bot->GetDistance(unit);
        if (dist <= maxDist && (!best || dist < bestDist))
        {
            best = unit;
            bestDist = dist;
        }
    };

    for (Unit* attacker : bot->getAttackers())
        consider(attacker, std::numeric_limits<float>::max());

    for (auto const& [guid, ref] : bot->GetCombatManager().GetPvPCombatRefs())
        consider(ref->GetOther(bot), pvpRange);

    return best;
}

// #754/#756 (BEYOND AC): engage `threat` the way AttackAction::Attack does (AC AttackAction.cpp:135-196, MoP
// AttackActions.cpp:150-197): select it, make it the "current target", drop a non-combat walk/ride, switch to the combat
// engine and start the auto attack. A mounted bot: Unit::Attack refuses while mounted, and the combat engine's "mounted"
// trigger -> "check mount state" dismounts it once the target is within the dismount distance (AC behaviour).
bool BGTactics::EngageThreat(Player* bot, Unit* threat)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI || !threat || threat == bot || !threat->IsInWorld() || !threat->IsAlive() ||
        threat->GetMapId() != bot->GetMapId() || !bot->IsValidAttackTarget(threat) || !bot->CanSeeOrDetect(threat) ||
        !bot->IsWithinLOSInMap(threat))
        return false;

    // AC AttackAction.cpp:135-137: infantry attacks are not allowed from vehicle drivers.
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    AiObjectContext* const context = botAI->GetAiObjectContext();
    Unit* const oldTarget = context->GetValue<Unit*>("current target")->Get();
    if (oldTarget == threat && botAI->GetState() == BOT_STATE_COMBAT && bot->GetVictim() == threat)
        return true;  // already fighting it

    bot->SetSelection(threat->GetGUID());
    context->GetValue<Unit*>("old target")->Set(oldTarget);
    context->GetValue<Unit*>("current target")->Set(threat);

    // AttackAction::Attack: stop a lower-priority move (the objective ride), never a controlled one (charge, jump, knockback).
    LastMovement& lastMovement = context->GetValue<LastMovement&>("last movement")->Get();
    if (lastMovement.priority < MovementPriority::MOVEMENT_COMBAT && bot->isMoving() &&
        bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) == NULL_MOTION_TYPE)
    {
        lastMovement.clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }

    botAI->ChangeEngine(BOT_STATE_COMBAT);
    bot->Attack(threat, bot->IsWithinMeleeRange(threat) || PlayerBotSpec::IsMelee(bot));
    return true;
}

// #756 (BEYOND AC - AC BattleGroundTactics.cpp:1652-1686 has the same gap; ThreatManager::CanHaveThreatList and the
// target values are identical on both cores). "bg move to objective" is a BG bot's non-combat fallback. When the bot is
// being fought but neither "enemy player target" (players only: PvP combat refs and nearby enemy players) nor "dps
// target" ("attackers" = units with a threat list) found anything to attack, AC keeps riding its path spline (isMoving
// -> return false) and then stands (inCombat -> return false): a bot hit only by pets, guardians or totems - none of them
// has a threat list - never picks a target. Wave 21 SotA: 4 mounted bots went from 100 to 11-37 hp (3 died) in 1-4 s
// with no target and no probed player targeting them; Hailia had 8-9 attackers at once (an Army of the Dead / Stampede
// pattern). MoP has many more of these summons (Stampede, Wild Imps, Force of Nature, Xuen, Grimoire of Service, Mindbender,
// ...), which is why the gap is wider than on AC (#756: EotS 18.5% vs 4.7% hurt before retaliating). Fight the nearest unit
// that is attacking the bot (its attackers list, its PvP combat opponents) within 40 yd (EnemyPlayerValue::
// GetMaxAttackDistance in a BG). Flag/orb/cart carriers fight back too (#869, see CarrierNearOwnCapture),
// except a WSG/TP carrier close to its own capture point, which keeps running to cap.

// #869 (BEYOND AC).
// AC never lets a carrier fight: PlayerHasFlag::IsCapturingFlag makes "attack enemy player" and "dps assist" useless
// (AC ChooseTargetActions.cpp:21,144) and "move to objective" keeps running it home while in combat
// (AC BattleGroundTactics.cpp:1688), so a carrier hunted by a single enemy runs until it dies (w73: Bardon 50->0 hp and
// Boastin 57->0 hp with 1 attacker, no target, "food" as the last action). MoP: the carrier's run-home objective
// (a static areatrigger objective at its capture trigger)
// keeps the base objective's activity flags 10 (enabled | out of combat), and the mode check
// only runs it in combat when bit 1 is set; the bot AI update therefore picks a target and
// runs the combat rotation as soon as UNIT_FLAG_IN_COMBAT is set, whatever the number of attackers. The
// only exception is WSG/TP: within 25 yd of its own capture trigger with its own flag on the stand, the objective gets
// flags 0xb (also in combat, non-blocking), so the carrier keeps running to cap and fights on the way.
// The rotation guards for carriers stay data (condition 46: only Stealth and Divine Shield, which drop the flag).
static bool CarrierNearOwnCapture(Player* bot, Battleground* bg)
{
    if (!bot || !bg)
        return false;

    uint32 const mapId = bg->GetMapId();
    if (mapId != 489 && mapId != 726)  // Warsong Gulch / Twin Peaks: the two-flag maps with the 25 yd exception
        return false;

    bool const isTP = mapId == 726;
    uint32 const allianceFlag = isTP ? uint32(BG_TP_OBJECT_A_FLAG) : uint32(BG_WS_OBJECT_A_FLAG);
    uint32 const hordeFlag = isTP ? uint32(BG_TP_OBJECT_H_FLAG) : uint32(BG_WS_OBJECT_H_FLAG);

    // GetFlagPickerGUID(team) = whoever carries THAT team's flag (PlayerHasFlag::IsCapturingFlag); the bot caps at its
    // own flag, which must be on its stand (spawned) for the capture to count.
    for (uint8 carried = TEAM_ALLIANCE; carried <= TEAM_HORDE; ++carried)
    {
        if (bot->GetGUID() != bg->GetFlagPickerGUID(carried))
            continue;

        GameObject* const ownFlag = bg->GetBGObject(carried == TEAM_ALLIANCE ? hordeFlag : allianceFlag);
        return ownFlag && ownFlag->isSpawned() && bot->GetDistance(ownFlag) <= 25.0f;
    }

    return false;
}

// #869 (PM review): for a carrier, only a unit actually fighting it counts - in its attackers list, meleeing it, or
// targeting it. A PvP combat reference alone (an enemy that hit it once and moved on) does not stop the run.
static bool FightsCarrier(Player* bot, Unit* unit)
{
    return unit && (bot->getAttackers().count(unit) || unit->GetVictim() == bot || unit->GetTarget() == bot->GetGUID());
}

// The nearest unit fighting the carrier (FightsCarrier) within 40 yd that it can attack, see and reach by line of sight.
static Unit* CarrierThreat(Player* bot)
{
    Unit* best = nullptr;
    float bestDist = 0.0f;
    auto const consider = [&](Unit* unit)
    {
        if (!unit || unit == bot || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() ||
            !FightsCarrier(bot, unit))
            return;

        float const dist = bot->GetDistance(unit);
        if (dist > 40.0f || (best && dist >= bestDist) || !bot->IsValidAttackTarget(unit) || !bot->CanSeeOrDetect(unit) ||
            !bot->IsWithinLOSInMap(unit))
            return;

        best = unit;
        bestDist = dist;
    };

    for (Unit* attacker : bot->getAttackers())
        consider(attacker);

    for (auto const& [guid, ref] : bot->GetCombatManager().GetPvPCombatRefs())
        consider(ref->GetOther(bot));

    return best;
}

// #869 (PM review): a carrier holds its run only while hits keep landing on it - at most CARRIER_HOLD_NO_HIT_MS after
// the last one (PM choice, 8 s: neither source has a "no hit on me" limit; MoP resumes the run only when
// UNIT_FLAG_IN_COMBAT clears, which the bot's own attacks keep refreshing - core PVP_COMBAT_TIMEOUT 5 s,
// Combat/CombatManager.h:84). A hit = the carrier's health went down since the previous check (a heal in the same window
// can hide one). State per bot; a carrier not checked for 10 s starts a new hold window (new episode).
static uint32 constexpr CARRIER_HOLD_NO_HIT_MS = 8000;
// w75 (PM choice, AC has no limit): a carrier fights back against 1-2 attackers; with 3 or more on it
// it keeps running (w75 TP: both carries died to 3+ attackers while holding, 0 captures; WSG 1-2 attackers: 7/8 answered).
// w76 TP: 0 captures again (w75 0, w51 4); carrier Jennah held and died to 2 attackers. Narrowed to the owner's case: fight back
// against a single attacker, keep running from 2+.
static size_t constexpr CARRIER_FIGHT_MAX_ATTACKERS = 1;
static bool CarrierHitRecently(Player* bot)
{
    struct HoldState
    {
        uint32 lastSeenMs = 0;
        uint32 lastHitMs = 0;
        uint32 lastHp = 0;
    };
    static std::map<ObjectGuid, HoldState> states;
    static std::mutex lock;

    uint32 const now = getMSTime();
    uint32 const hp = bot->GetHealth();
    std::lock_guard<std::mutex> guard(lock);
    HoldState& s = states[bot->GetGUID()];
    if (!s.lastSeenMs || getMSTimeDiff(s.lastSeenMs, now) > 10000 || hp < s.lastHp)
        s.lastHitMs = now;  // a new episode opens with the attack that brought it here; afterwards only a health drop

    s.lastSeenMs = now;
    s.lastHp = hp;
    return getMSTimeDiff(s.lastHitMs, now) <= CARRIER_HOLD_NO_HIT_MS;
}

bool BGTactics::FightPathAttacker(Battleground* bg)
{
    if (!bot->IsAlive() || bot->GetVehicle() || botAI->GetState() == BOT_STATE_COMBAT)
        return false;

    if (!bot->IsInCombat() && bot->getAttackers().empty())
        return false;

    // #869: a carrier fights units fighting it (FightsCarrier) like any other bot, unless it is about to cap or
    // no hit has landed on it for CARRIER_HOLD_NO_HIT_MS.
    bool const carrier = PlayerHasFlag::IsCapturingFlag(bot) || isCarryingObjective(bg);
    if (carrier && (CarrierNearOwnCapture(bot, bg) || !CarrierHitRecently(bot) ||
                    bot->getAttackers().size() > CARRIER_FIGHT_MAX_ATTACKERS))
        return false;

    float constexpr range = 40.0f;
    Unit* best = nullptr;
    float bestDist = 0.0f;
    auto const consider = [&](Unit* unit)
    {
        if (!unit || unit == bot || !unit->IsInWorld() || !unit->IsAlive() || unit->GetMapId() != bot->GetMapId() ||
            (carrier && !FightsCarrier(bot, unit)))
            return;

        float const dist = bot->GetDistance(unit);
        if (dist > range || (best && dist >= bestDist) || !bot->IsValidAttackTarget(unit) || !bot->CanSeeOrDetect(unit) ||
            !bot->IsWithinLOSInMap(unit))
            return;

        best = unit;
        bestDist = dist;
    };

    for (Unit* attacker : bot->getAttackers())
        consider(attacker);

    for (auto const& [guid, ref] : bot->GetCombatManager().GetPvPCombatRefs())
        consider(ref->GetOther(bot));

    if (!best)
        return false;

    // Owner rule (#754): keep the current target while it is alive and attacking the bot.
    Unit* const cur = AI_VALUE(Unit*, "current target");
    Unit* fight = (cur && cur != best && cur->IsAlive() && cur->GetVictim() == bot && EngageThreat(bot, cur)) ? cur : nullptr;
    if (!fight)
    {
        if (!EngageThreat(bot, best))
            return false;

        fight = best;
    }

    TC_LOG_DEBUG("playerbots",
                 "[BGFIGHT] %s path-attacker %s entry=%u player=%d dist=%.1f mounted=%d attackers=%u carrier=%d",
                 bot->GetName().c_str(), fight->GetName().c_str(), fight->GetEntry(), fight->IsPlayer() ? 1 : 0,
                 bot->GetDistance(fight), bot->IsMounted() ? 1 : 0, uint32(bot->getAttackers().size()), carrier ? 1 : 0);
    return true;
}

// #869 (BEYOND AC - see CarrierNearOwnCapture): the combat-engine half. "player has flag" -> "bg move to
// objective" (ACTION_EMERGENCY) sits on the combat engine too (AiFactory::AddBattlegroundStrategies), and AC's
// IsCapturingFlag bypass of the inCombat stop sends a fighting carrier straight back onto its path, so it would turn its
// back on the attacker it just engaged. "move to objective" yields to the class rotation (true = hold) only while
//   - it is not about to cap (CarrierNearOwnCapture) and a hit landed on it within CARRIER_HOLD_NO_HIT_MS, and
//   - its current target is fighting it (FightsCarrier) or is within its reach (10 yd for a melee bot, the configured
//     spell distance otherwise) - a kiting or distant target does not hold the run;
//   - else it engages the nearest unit fighting it (CarrierThreat) when there is one.
// Otherwise it runs home as on AC. File-static (no header change): `carrying` = PlayerHasFlag::IsCapturingFlag ||
// BGTactics::isCarryingObjective, computed by the caller.
static bool CarrierHoldsToFight(Player* bot, PlayerbotAI* botAI, Battleground* bg, bool carrying)
{
    if (!carrying || !bot || !botAI || !bot->IsAlive() || bot->GetVehicle() || botAI->GetState() != BOT_STATE_COMBAT)
        return false;

    if (CarrierNearOwnCapture(bot, bg) || !CarrierHitRecently(bot) || bot->getAttackers().size() > CARRIER_FIGHT_MAX_ATTACKERS)
        return false;

    Unit* const cur = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    if (cur && cur->IsInWorld() && cur->IsAlive() && cur->GetMapId() == bot->GetMapId() && bot->IsValidAttackTarget(cur))
    {
        float const reach = PlayerBotSpec::IsMelee(bot) ? 10.0f : sPlayerbotAIConfig->spellDistance;
        if (FightsCarrier(bot, cur) || bot->IsWithinDistInMap(cur, reach))
            return true;
    }

    Unit* const threat = CarrierThreat(bot);
    if (!threat || threat == cur || !BGTactics::EngageThreat(bot, threat))
        return false;

    TC_LOG_DEBUG("playerbots", "[BGFIGHT] %s carrier-retarget %s entry=%u player=%d dist=%.1f attackers=%u",
                 bot->GetName().c_str(), threat->GetName().c_str(), threat->GetEntry(), threat->IsPlayer() ? 1 : 0,
                 bot->GetDistance(threat), uint32(bot->getAttackers().size()));
    return true;
}

// check Isle of Conquest Keep position
TeamId BGTactics::BgTeamIdOf(Player* player)
{
    if (!player)
        return TEAM_NEUTRAL;

    // Outside a battleground there is no BG team; fall back to the race's team.
    // NOTE: this used to call BgTeamIdOf(player) - itself - which recursed until the stack blew.
    if (!player->InBattleground())
        return player->GetTeam() == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE;

    return player->GetBGTeam() == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE;
}

// Ported 1:1 from AC (Ai/Base/Actions/BattleGroundTactics.cpp:4105-4237); adapters are commented inline.
bool BGTactics::IsLockedInsideKeep()
{
    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    BattlegroundTypeId bgType = RealBgTypeId(bg); // #896

    if (bgType != BATTLEGROUND_IC)
        return false;

    bool isInside = false;
    if (BgTeamIdOf(bot) == TEAM_ALLIANCE && bot->GetPositionX() < 410.0f && bot->GetPositionY() > -900.0f &&
        bot->GetPositionY() < -765.0f)
        isInside = true;
    if (BgTeamIdOf(bot) == TEAM_HORDE && bot->GetPositionX() > 1153.0f && bot->GetPositionY() > -849.0f &&
        bot->GetPositionY() < -679.0f)
        isInside = true;

    if (!isInside)
        return false;

    // #675 (beyond AC): a keep-cannon defender stays inside to climb to its cannon (BgTacticsIC.cpp).
    if (IocBeyondAc::StaysInOwnKeep(bot, static_cast<BattlegroundIC*>(bg), BgTeamIdOf(bot),
                                    context->GetValue<uint32>("bg role")->Get()))
        return false;

    GuidVector closeObjects;
    closeObjects = *context->GetValue<GuidVector>("nearest game objects no los");
    if (closeObjects.empty())
        return moveToStart(true);

    GameObject* closestPortal = nullptr;
    float closestDistance = 100.0f;
    bool gateLock = false;

    // check inner gates status
    // ALLIANCE
    if (BgTeamIdOf(bot) == TEAM_ALLIANCE)
    {
        if (GameObject* go = bg->GetBGObject(BG_IC_GO_DOODAD_PORTCULLISACTIVE02))
        {
            if (go->isSpawned())
            {
                gateLock = go->getLootState() != GO_ACTIVATED;
            }
            else
            {
                gateLock = false;
            }
        }
    }
    // HORDE
    if (BgTeamIdOf(bot) == TEAM_HORDE)
    {
        if (GameObject* go = bg->GetBGObject(BG_IC_GO_HORDE_KEEP_PORTCULLIS))
        {
            if (go->isSpawned())
            {
                gateLock = go->getLootState() != GO_ACTIVATED;
            }
            else
            {
                gateLock = false;
            }
        }
    }

    for (ObjectGuid const& guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go)
            continue;

        // ALLIANCE
        // get closest portal
        if (BgTeamIdOf(bot) == TEAM_ALLIANCE && go->GetEntry() == GO_TELEPORTER_4)
        {
            float tempDist = sServerFacade->GetDistance2d(bot, go->GetPositionX(), go->GetPositionY());

            if (sServerFacade->IsDistanceLessThan(tempDist, closestDistance))
            {
                closestDistance = tempDist;
                closestPortal = go;
            }
        }

        // HORDE
        // get closest portal
        if (BgTeamIdOf(bot) == TEAM_HORDE && go->GetEntry() == GO_TELEPORTER_2)
        {
            float tempDist = sServerFacade->GetDistance2d(bot, go->GetPositionX(), go->GetPositionY());

            if (sServerFacade->IsDistanceLessThan(tempDist, closestDistance))
            {
                closestDistance = tempDist;
                closestPortal = go;
            }
        }
    }

    // portal not found, move closer
    if (gateLock && !closestPortal)
        return moveToStart(true);

    // #738 (beyond AC, AC BattleGroundTactics.cpp:4222-4224 has the same branch): once a keep gate is down
    // (gateLock false) no portal is needed - the bot walks out through the breach. AC's "!gateLock && !closestPortal
    // -> moveToStart" sent a bot deeper than sightDistance from every in-keep teleporter (the command room, the keep
    // graveyard) back to the waiting spot by the front gate, where the portals come into range again, the objective
    // (or the attackers it fights in the courtyard) pulls it back in, and the loop repeats: the owner's 2026-10-08
    // "Rineva/Solarane run back and forth inside the breached Horde keep".
    if (!gateLock)
    {
        // evidence for the 1:1: the branch that used to send the bot back to the start (10 s per bot)
        static thread_local std::map<ObjectGuid, uint32> s_keepExitLog;
        uint32& last = s_keepExitLog[bot->GetGUID()];
        if (!closestPortal && (!last || getMSTimeDiff(last, getMSTime()) >= 10000))
        {
            last = getMSTime();
            TC_LOG_DEBUG("playerbots", "[IOC] keep-exit bot=%s team=%u at=%.1f,%.1f,%.1f (breached, no portal in range)",
                         bot->GetName().c_str(), uint32(BgTeamIdOf(bot)), bot->GetPositionX(), bot->GetPositionY(),
                         bot->GetPositionZ());
        }
        return false;
    }

    // nothing found, allow move through
    if (!gateLock || !closestPortal)
        return false;

    // portal found
    if (closestPortal)
    {
        // if close
        if (bot->IsWithinDistInMap(closestPortal, INTERACTION_DISTANCE))
        {
            // MoP adapter (AC BattleGroundTactics.cpp:4222-4224): CMSG_GAMEOBJ_USE is bit-packed on this
            // core, so AC's hand-built plain-guid packet would be parsed as garbage (see the note at the
            // top of this file). GameObject::Use is the call HandleGameObjectUseOpcode reaches after
            // parsing (Handlers/SpellHandler.cpp:622-628), and the same call FishingAction.cpp:563 makes.
            closestPortal->Use(bot);
            return true;
        }
        else
        {
            return MoveTo(bot->GetMapId(), closestPortal->GetPositionX(), closestPortal->GetPositionY(),
                          closestPortal->GetPositionZ());
        }
    }

    return moveToStart(true);

    return false;
}

namespace
{
// #794 (MoP-only, see ArenaTactics::Execute): the nearest living arena enemy the bot can detect, wherever it stands -
// no line-of-sight or range filter, because the caller only runs when "possible targets" found nobody in sight.
Player* NearestArenaEnemy(Player* bot, Battleground* bg)
{
    Player* best = nullptr;
    float bestDist = 0.0f;
    for (auto& ref : bg->GetBgMap()->GetPlayers())
    {
        Player* enemy = ref.GetSource();
        if (!enemy || enemy == bot || !enemy->IsInWorld() || !enemy->IsAlive() || enemy->IsGameMaster() ||
            enemy->GetBGTeam() == bot->GetBGTeam())
            continue;

        if (enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE) ||
            !bot->CanSeeOrDetect(enemy))
            continue;

        float const dist = bot->GetExactDist(enemy);
        if (!best || dist < bestDist)
        {
            best = enemy;
            bestDist = dist;
        }
    }

    return best;
}
}  // namespace

/// Arena play, ported 1:1 from AC mod-playerbots Ai/Base/Actions/BattleGroundTactics.cpp:4238-4418
/// (issue #346). The only adaptations are to this core's API, and each one is noted below:
///
///  * AC: botAI->ResetStrategies(!IsRandomBot) -> botAI->ResetStrategies(). AC's own signature is
///    void ResetStrategies(bool /*load*/) (Bot/PlayerbotAI.cpp:2047) and it never reads the argument,
///    so the two calls are identical; AC's local IsRandomBot would be an unused-variable warning.
///  * AC: bg->GetTeamStartPosition(bot->GetBgTeamId()) -> bg->GetTeamStartLoc(bot->GetBGTeam(), ...).
///    5.4.8 has no Battleground::GetTeamStartPosition and no Player::GetBgTeamId; the getter that
///    exists takes a *Team* (ALLIANCE / HORDE), not a TeamId, so passing a TeamId returns the other
///    side's start position (see the same trap documented at BGTactics::Execute).
///  * AC: bg->GetBgTypeID() -> bg->GetTypeID() (same declaration, this core's name).
///  * MoveTo(mapId, x, y, z, false, true) keeps its meaning here: idle = false, react = true.
///  * Inert on this fork, but harmless and left in for parity: the "collision" strategy, which the
///    ChangeStrategy below drops, is not registered in StrategyContext.h yet.
bool ArenaTactics::Execute(Event /*event*/)
{
    if (!bot->InBattleground())
    {
        // Left the arena. Drop "arena" from both engines first, then rebuild - otherwise the reset
        // hands the strategy straight back and the loop repeats every tick.
        botAI->ChangeStrategy("-arena", BOT_STATE_COMBAT);
        botAI->ChangeStrategy("-arena", BOT_STATE_NON_COMBAT);
        botAI->ResetStrategies();
        return false;
    }

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    if (bg->GetStatus() == STATUS_WAIT_LEAVE)
        return BGStatusAction::LeaveBG(botAI);

    if (bg->GetStatus() != STATUS_IN_PROGRESS)
        return false;

    if (bot->isDead())
        return false;

    if (bot->isMoving())
        return false;

    // startup phase
    if (bg->GetStartDelayTime() > 0)
        return false;

    if (botAI->HasStrategy("collision", BOT_STATE_NON_COMBAT))
        botAI->ChangeStrategy("-collision", BOT_STATE_NON_COMBAT);

    if (botAI->HasStrategy("buff", BOT_STATE_NON_COMBAT))
        botAI->ChangeStrategy("-buff", BOT_STATE_NON_COMBAT);

    Unit* target = bot->GetVictim();
    if (target)
    {
        bool losBlocked = !bot->IsWithinLOSInMap(target) || fabs(bot->GetPositionZ() - target->GetPositionZ()) > 5.0f;

        if (losBlocked)
        {
            PathGenerator path(bot);
            path.CalculatePath(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), false);

            if (path.GetPathType() != PATHFIND_NOPATH)
            {
                bot->CastStop();

                float x, y, z;
                target->GetPosition(x, y, z);
                botAI->TellMasterNoFacing("Repositioning to exit the LoS target!");
                return MoveTo(target->GetMapId(), x + frand(-1, +1), y + frand(-1, +1), z, false, true);
            }
        }
    }

    // #794 (MoP divergence; extends AC's LoS reposition above to a bot that has no victim). AC only walks out of a
    // blocked line of sight toward its current victim. A bot whose victim died, or whose combat timed out, and that
    // sees no enemy goes back to moveToCenter for the rest of the match. Wave 39 DS 3v3 rated: the last Alliance
    // healer (Ederini) walked off the raised centre platform to the floor under the Horde pipe (1322.7,809.6,3.2).
    // That spot is ~36 yd from the centre spot moveToCenter keeps (1291.58,790.87,7.8) and has no LoS to it.
    // After their second kill, the Horde warlock and hunter dropped combat. For 18 min they wandered the centre:
    // "no possible targets" -> arena tactics -> moveToCenter. Only the Horde healer kept the survivor in combat
    // (fight 1117843 ms; AC 28 s). AC's survivor stayed in sight, so AC never meets this and has no rule for it.
    // Once the opening is over, a bot with no victim and nobody in sight walks to the nearest living enemy it can
    // detect. It uses AC's path guard and MoveTo from the branch above. When the enemy comes into sight, "enemy
    // player near" takes over. The opening stays AC's: GetElapsedTime counts from the queue pop, and every 5.4.8
    // arena has a 60-s prep, so the hunt starts at most 60 s after the gates.
    if (!target && bg->GetElapsedTime() >= 2 * MINUTE * IN_MILLISECONDS)
    {
        if (Player* enemy = NearestArenaEnemy(bot, bg))
        {
            PathGenerator path(bot);
            path.CalculatePath(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ(), false);

            if (path.GetPathType() != PATHFIND_NOPATH)
            {
                float x, y, z;
                enemy->GetPosition(x, y, z);
                // #852 diagnostic: the hunt picks an enemy the attack filter (AttackersValue::IsPossibleTarget + the LoS
                // of NearestUnitsValue) may reject; then the bot walks up to it and idles. Log which check would fail.
                TC_LOG_DEBUG("playerbots", "[ARENA] hunt bot=%s enemy=%s dist=%.1f from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f "
                             "poss=%d vis=%d friendly=%d pvp=%d ffa=%d immpc=%d nonatk=%d prohib=%d los=%d gm=%d",
                             bot->GetName().c_str(), enemy->GetName().c_str(), bot->GetExactDist(enemy),
                             bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), x, y, z,
                             AttackersValue::IsPossibleTarget(enemy, bot) ? 1 : 0, enemy->IsVisible() ? 1 : 0,
                             enemy->IsFriendlyTo(bot) ? 1 : 0, enemy->IsPvP() ? 1 : 0, enemy->IsFFAPvP() ? 1 : 0,
                             enemy->HasUnitFlag(UNIT_FLAG_IMMUNE_TO_PC) ? 1 : 0,
                             enemy->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NON_ATTACKABLE_2) ? 1 : 0,
                             sPlayerbotAIConfig->IsPvpProhibited(enemy->GetZoneId(), enemy->GetAreaId()) ? 1 : 0,
                             bot->IsWithinLOSInMap(enemy) ? 1 : 0, enemy->IsGameMaster() ? 1 : 0);
                return MoveTo(enemy->GetMapId(), x + frand(-1, +1), y + frand(-1, +1), z, false, true);
            }
        }
    }

    if (!bot->IsInCombat())
        return moveToCenter(bg);

    return true;
}

bool ArenaTactics::moveToCenter(Battleground* bg)
{
    // Sanity check
    if (!bg)
    {
        return true;
    }
    uint32 Preference = 6;

    switch (bot->GetClass())
    {
        case CLASS_PRIEST:
        case CLASS_SHAMAN:
        case CLASS_DRUID:
            Preference = 3;
            break;
        case CLASS_WARRIOR:
        case CLASS_PALADIN:
        case CLASS_ROGUE:
        case CLASS_DEATH_KNIGHT:
            Preference = 6;
            break;
        case CLASS_HUNTER:
        case CLASS_MAGE:
        case CLASS_WARLOCK:
            Preference = 9;
            break;
    }

    // Start position Y, which picks the half of the map this bot walks into. GetTeamStartLoc wants
    // a Team, so bot->GetBGTeam() - never a TeamId (see BGTactics::Execute's note on that).
    float sx = 0.0f, sy = 0.0f, sz = 0.0f, so = 0.0f;
    bg->GetTeamStartLoc(bot->GetBGTeam(), sx, sy, sz, so);

    // #709: AC switches on bg->GetBgTypeID(), which is the real arena because AC's CreateNewBattleground rewrites
    // BATTLEGROUND_AA to the drawn arena (AC BattlegroundMgr.cpp:382-383). The 5.4.8 core keeps the queued type:
    // SetTypeID(originalBgTypeId) = BATTLEGROUND_AA and SetRandomTypeID(drawn arena) (BattlegroundMgr.cpp:936-937),
    // so GetTypeID() matched no case below and no bot ever walked out of its start room in any arena.
    BattlegroundTypeId arenaTypeId = bg->GetTypeID();
    if (arenaTypeId == BATTLEGROUND_AA)
        arenaTypeId = bg->GetTypeID(true);

    switch (arenaTypeId)
    {
        case BATTLEGROUND_BE:
            if (sy < 240)
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 6226.65f + frand(-1, +1), 264.36f + frand(-1, +1), 1.31f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 6239.89f + frand(-1, +1), 261.11f + frand(-1, +1), 0.89f, false, true);
                else
                    MoveTo(bg->GetMapId(), 6235.60f + frand(-1, +1), 258.27f + frand(-1, +1), 0.89f, false, true);
            }
            else
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 6265.72f + frand(-1, +1), 271.92f + frand(-1, +1), 3.65f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 6239.89f + frand(-1, +1), 261.11f + frand(-1, +1), 0.89f, false, true);
                else
                    MoveTo(bg->GetMapId(), 6250.50f + frand(-1, +1), 266.66f + frand(-1, +1), 2.63f, false, true);
            }
            break;
        case BATTLEGROUND_RL:
            if (sy < 1600)
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 1262.14f + frand(-1, +1), 1657.63f + frand(-1, +1), 33.76f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 1266.85f + frand(-1, +1), 1663.52f + frand(-1, +1), 34.04f, false, true);
                else
                    MoveTo(bg->GetMapId(), 1274.07f + frand(-1, +1), 1656.36f + frand(-1, +1), 34.58f, false, true);
            }
            else
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 1261.93f + frand(-1, +1), 1669.27f + frand(-1, +1), 34.25f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 1266.85f + frand(-1, +1), 1663.52f + frand(-1, +1), 34.04f, false, true);
                else
                    MoveTo(bg->GetMapId(), 1266.37f + frand(-1, +1), 1672.40f + frand(-1, +1), 34.21f, false, true);
            }
            break;
        case BATTLEGROUND_NA:
            if (sy < 2870)
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 4068.85f + frand(-1, +1), 2911.98f + frand(-1, +1), 12.99f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 4056.99f + frand(-1, +1), 2919.75f + frand(-1, +1), 13.51f, false, true);
                else
                    MoveTo(bg->GetMapId(), 4056.27f + frand(-1, +1), 2905.33f + frand(-1, +1), 12.90f, false, true);
            }
            else
            {
                if (Preference == 3)
                    MoveTo(bg->GetMapId(), 4043.66f + frand(-1, +1), 2927.93f + frand(-1, +1), 13.17f, false, true);
                else if (Preference == 6)
                    MoveTo(bg->GetMapId(), 4056.99f + frand(-1, +1), 2919.75f + frand(-1, +1), 13.51f, false, true);
                else
                    MoveTo(bg->GetMapId(), 4054.80f + frand(-1, +1), 2934.28f + frand(-1, +1), 13.72f, false, true);
            }
            break;
        case BATTLEGROUND_DS:
            if (!MoveTo(bg->GetMapId(), 1291.58f + frand(-5, +5), 790.87f + frand(-5, +5), 7.8f, false, true))
            {
                // they like to hang around at the tip of the pipes doing nothing, so we just teleport them down
                if (bot->GetDistance(1333.07f, 817.18f, 13.35f) < 4)
                {
                    // AC passes the two flags or'd into one call. 5.4.8's
                    // RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags) builds flag64{flag, 0} and
                    // flag64 has no operator|, so the two calls below are the identical removal -
                    // both values are in the low 32-bit half (SpellDefines.h:81,84).
                    bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED);
                    bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP);
                    bot->TeleportTo(bg->GetMapId(), 1330.96f, 816.75f, 3.2f, bot->GetOrientation());
                }
                if (bot->GetDistance(1250.13f, 764.79f, 13.34f) < 4)
                {
                    bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED);
                    bot->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP);
                    bot->TeleportTo(bg->GetMapId(), 1252.19f, 765.41f, 3.2f, bot->GetOrientation());
                }
            }
            break;
        case BATTLEGROUND_RV:
            MoveTo(bg->GetMapId(), 764.65f + frand(-2, +2), -283.85f + frand(-2, +2), 28.28f, false, true);
            break;
        // #709 (MoP-only arenas, AC RV pattern above): the centre is the floor point each core script uses for its
        // under-map rescue teleport, between the two start rooms.
        case BATTLEGROUND_TV:  // Tol'viron Arena, map 980 (BattlegroundTV.cpp:103)
            MoveTo(bg->GetMapId(), -10716.23f + frand(-2, +2), 429.32f + frand(-2, +2), 24.41f, false, true);
            break;
        case BATTLEGROUND_TTP:  // The Tiger's Peak, map 1134 (BattlegroundTTP.cpp:83)
            MoveTo(bg->GetMapId(), 567.31f + frand(-2, +2), 632.11f + frand(-2, +2), 380.70f, false, true);
            break;
        default:
            break;
    }

    return true;
}