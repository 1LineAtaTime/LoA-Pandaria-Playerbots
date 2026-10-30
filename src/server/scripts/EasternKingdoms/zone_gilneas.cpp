/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedEscortAI.h"
#include "PassiveAI.h"
#include "Vehicle.h"
#include "GameObjectAI.h"
#include "TaskScheduler.h"
#include "ScriptedGossip.h"
#include "TemporarySummon.h"
#include "SmartScriptMgr.h"
#include "GameObject.h"
#include "CreatureTextMgr.h"

enum Gilneas
{
    SPELL_PING_GILNEAN_CROW                 = 93275,
    SPELL_SUMMON_RAVENOUS_WORGEN_1          = 66836,
    SPELL_SUMMON_RAVENOUS_WORGEN_2          = 66925,
    SPELL_SHOOT_INSTAKILL                   = 67593, // Visual problem
    SPELL_COSMETIC_ATTACK                   = 69873,
    SPELL_PULL_TO                           = 67357,
    SPELL_GET_SHOT                          = 67349,

    EVENT_START_TALK_WITH_CITIZEN           = 1,
    EVENT_TALK_WITH_CITIZEN_1               = 2,
    EVENT_TALK_WITH_CITIZEN_2               = 3,
    EVENT_TALK_WITH_CITIZEN_3               = 4,

    EVENT_START_DIALOG                      = 1,
    EVENT_START_TALK_TO_GUARD               = 2,
    EVENT_TALK_TO_GUARD_1                   = 3,
    EVENT_TALK_TO_GUARD_2                   = 4,
    EVENT_RESET_DIALOG                      = 5,

    PRINCE_LIAM_GREYMANE_TEXT_00            = 0,
    PRINCE_LIAM_GREYMANE_TEXT_01            = 1,
    PRINCE_LIAM_GREYMANE_TEXT_02            = 2,

    EVENT_JUMP_TO_PRISON                    = 1,
    EVENT_AGGRO_PLAYER                      = 2,
    EVENT_FORCE_DESPAWN                     = 3,

    EVENT_COSMETIC_ATTACK                   = 1,
    EVENT_JUMP_TO_PLAYER                    = 2,
    EVENT_SHOOT_JOSIAH                      = 3,

    PHASE_ROOF                              = 0,
    PHASE_COMBAT                            = 1,

    WORGEN_ID_ROOF_1                        = 0,
    WORGEN_ID_ROOF_2                        = 1,
    WORGEN_ID_ROOF_3                        = 2,
    WORGEN_ID_ROOF_4                        = 3,
    WORGEN_ID_ROOF_5                        = 4,
    WORGEN_ID_ROOF_6                        = 5,
    WORGEN_ID_ROOF_7                        = 6,

    WORGEN_ID_CATHEDRAL_1                   = 7,
    WORGEN_ID_CATHEDRAL_2                   = 8,
    WORGEN_ID_CATHEDRAL_3                   = 9,
    WORGEN_ID_CATHEDRAL_4                   = 10,
    WORGEN_ID_CATHEDRAL_5                   = 11,
    WORGEN_ID_CATHEDRAL_6                   = 12,
    WORGEN_ID_CATHEDRAL_7                   = 13,
    WORGEN_ID_CATHEDRAL_8                   = 14,
    WORGEN_ID_CATHEDRAL_9                   = 15,

    NPC_PANICKED_CITIZEN_GATE               = 44086,
    NPC_WORGEN_RUNT                         = 35456,
    NPC_WORGEN_RUNT_2                       = 35188,
    NPC_WORGEN_ALPHA                        = 35170,
    NPC_WORGEN_ALPHA_2                      = 35167,
    NPC_LORNA_CROWLEY                       = 35378,
    NPC_GENERIC_TRIGGER_LAB                 = 35374,

    SAY_JOSIAH_AVERY_1                      = 0,
    SAY_JOSIAH_AVERY_2                      = 1,
    SAY_JOSIAH_AVERY_3                      = 2,
    SAY_JOSIAH_AVERY_4                      = 3,
    SAY_JOSIAH_AVERY_5                      = 4,
    SAY_JOSIAH_AVERY_6                      = 5,

    QUEST_FROM_THE_SHADOWS                  = 14204,

    NPC_GILNEAS_MASTIFF                     = 35631,
    NPC_BLOODFANG_LURKER                    = 35463,

    SPELL_SUMMON_MASTIFF                    = 67807,
    SPELL_ATTACK_LURKER                     = 67805,
    SPELL_SHADOWSTALKER_STEALTH             = 5916,
    SPELL_UNDYING_FRENZY                    = 80515,
    SPELL_ENRAGE                            = 8599,

    WORGEN_ENRAGE_EMOTE                     = 0,

    QUEST_SAVE_KRENNAN_ARANAS               = 14293,

    NPC_KRENNAN_ARANAS                      = 35753,
    NPC_KING_GREYMANES_HORSE                = 35905,
    NPC_BLOODFANG_RIPPER                    = 35505,
    NPC_GILNEAS_CITY_GUARD                  = 35509,
    NPC_KRENNAN_ARANAS_KILL_CREDIT          = 35753,

    TEXT_KRENNAN_ARANAS                     = 0,
    EMOTE_SAVE_KRENNAN_ARANAS               = 0,
    TEXT_SAVED_KRENNAN_ARANAS               = 0,

    SPELL_GUARD_SHOOT                       = 48424,

    EVENT_SEARCH_INVADER                    = 1,
    EVENT_SEARCH_FOOTSOLDIER                = 2,
    EVENT_SEARCH_WATCHMAN                   = 3,

    NPC_ENTRY_INVADER                       = 34511,
    NPC_ENTRY_FOOTSOLDIER                   = 36236,
    NPC_ENTRY_WATCHMAN                      = 36211,

    SPELL_SAVE_CYNTHIA                      = 68597,
    SPELL_SAVE_ASHLEY                       = 68598,
    SPELL_SAVE_JAMES                        = 68596,

    PLAYER_SAY_CYNTHIA                      = 0,
    PLAYER_SAY_ASHLEY                       = 1,
    PLAYER_SAY_JAMES                        = 2,

    NPC_JAMES                               = 36289,
    NPC_CYNTHIA                             = 36287,
    NPC_ASHLEY                              = 36288,

    EVENT_TALK_TO_PLAYER                    = 1,
    EVENT_START_RUN                         = 2,
    EVENT_OPEN_DOOR                         = 3,
    EVENT_RESUME_RUN                        = 4,
    EVENT_CRY                               = 5,

    CHILDREN_TEXT_ID                        = 0,

    GO_DOOR_TO_THE_BASEMENT                 = 196411,

    SPELL_CATCH_CAT                         = 68743,
    SPELL_LUCIUS_SHOOT                      = 41440,

    NPC_WAHL                                = 36458,
    NPC_WAHL_WORGEN                         = 36852,
    NPC_LUCIUS_THE_CRUEL                    = 36461,

    SAY_THIS_CAT_IS_MINE                    = 0,
    YELL_DONT_MESS                          = 0,

    ACTION_SUMMON_LUCIUS                    = 1,
    ACTION_CHANCE_DESPAWN                   = 2,
    ACTION_CONTINUE_SCENE                   = 3,

    POINT_CATCH_CHANCE                      = 4,

    SPELL_ROUND_UP_HORSE                    = 68908,
    SPELL_ROPE_CHANNEL                      = 68940,

    EVENT_CHECK_LORNA                       = 1,
    EVENT_CHECK_OWNER                       = 2,

    NPC_LORNA_CROWLEY_2                     = 36457,

    QUEST_THE_HUNGRY_ETTIN                  = 14416,

    QUEST_CREDIT_HORSE                      = 36560,

    MUSIC_ENTRY_TELESCOPE                   = 23539,
    PLAY_CINEMATIC_TELESCOPE                = 167,

    QUEST_ENTRY_EXODUS                      = 24438,

    NPC_STAGECOACH_HARNESS                  = 38755,
    NPC_HARNESS_SUMMONED                    = 43336,

    EVENT_BOARD_HARNESS_OWNER               = 1,

    ACTION_START_WP                         = 1,

    GO_FIRST_GATE                           = 196401,
    GO_KINGS_GATE                           = 196412,

    NPC_KOROTH_THE_HILLBREAKER              = 36294,

    ACTION_START_KOROTH_EVENT               = 1,

    EVENT_KOROTH_THE_HILLBREAKER_2          = 2,
    SAY_KOROTH_THE_HILLBREAKER_1            = 0,
    SAY_KOROTH_THE_HILLBREAKER_2            = 1
};

Position const runt2SummonJumpPos = { -1671.915f, 1446.734f, 52.28712f };
Position const alphaSummonJumpPos = { -1656.723f, 1405.647f, 52.74205f };
Position const alpha2SummonJumpPos = { -1675.44f, 1447.495f, 52.28762f };

Position const josiahJumpPos = { -1796.63f, 1427.73f, 12.4624f };

uint32 const runtHousePathSize1 = 13;

Position const worgenRuntHousePath1[runtHousePathSize1] =
{
    { -1734.77f, 1527.007f, 55.2133f },
    { -1729.345f, 1526.495f, 55.4231f },
    { -1723.921f, 1525.982f, 55.6329f },
    { -1718.885f, 1525.88f, 55.89785f },
    { -1718.002f, 1516.054f, 55.36457f },
    { -1718.162f, 1512.458f, 55.41572f },
    { -1717.852f, 1508.871f, 55.64134f },
    { -1717.868f, 1507.03f, 55.78084f },
    { -1717.939f, 1498.912f, 56.2076f },
    { -1717.975f, 1494.827f, 56.34147f },
    { -1717.666f, 1491.978f, 56.46574f },
    { -1717.708f, 1491.591f, 56.51286f },
    { -1717.708f, 1491.591f, 56.51286f },
};

uint32 const runtHousePathSize2 = 11;

Position const worgenRuntHousePath2[runtHousePathSize2] =
{
    {-1705.29f, 1527.974f, 57.49218f },
    {-1709.63f, 1527.464f, 56.81163f },
    {-1713.971f, 1526.953f, 56.13107f },
    {-1718.249f, 1525.915f, 55.91631f },
    {-1718.002f, 1516.054f, 55.36457f },
    {-1718.162f, 1512.458f, 55.41572f },
    {-1717.852f, 1508.871f, 55.64134f },
    {-1717.866f, 1507.038f, 55.7804f },
    {-1717.928f, 1498.872f, 56.20963f },
    {-1717.947f, 1496.298f, 56.29393f },
    {-1717.947f, 1496.298f, 56.29393f },
};

uint32 const runtHousePathSize3 = 8;

Position const worgenRuntHousePath3[runtHousePathSize3] =
{
    {-1717.74f, 1514.99f, 55.37629f },
    {-1717.75f, 1513.727f, 55.39608f },
    {-1717.76f, 1512.465f, 55.41587f },
    {-1717.787f, 1508.872f, 55.64124f },
    {-1717.799f, 1507.277f, 55.74761f },
    {-1717.864f, 1498.657f, 56.22049f },
    {-1717.887f, 1495.557f, 56.31728f },
    {-1717.887f, 1495.557f, 56.31728f },
};

uint32 const runtHousePathSize4 = 11;

Position const worgenRuntHousePath4[runtHousePathSize4] =
{
    { -1727.101f, 1527.078f, 55.5045f },
    { -1724.719f, 1526.731f, 55.60394f },
    { -1722.337f, 1526.383f, 55.70337f },
    { -1718.885f, 1525.88f, 55.89785f },
    { -1718.002f, 1516.054f, 55.36457f },
    { -1718.162f, 1512.458f, 55.41572f },
    { -1717.852f, 1508.871f, 55.64134f },
    { -1717.845f, 1507.113f, 55.77633f },
    { -1717.81f, 1498.482f, 56.22932f },
    { -1717.793f, 1494.123f, 56.37141f },
    { -1717.793f, 1494.123f, 56.37141f },
};

uint32 const runtHousePathSize5 = 11;

Position const worgenRuntHousePath5[runtHousePathSize5] =
{
    { -1709.699f, 1527.335f, 56.34836f },
    { -1713.974f, 1526.625f, 56.13234f },
    { -1718.249f, 1525.915f, 55.91631f },
    { -1718.002f, 1516.054f, 55.36457f },
    { -1718.162f, 1512.458f, 55.41572f },
    { -1717.852f, 1508.871f, 55.64134f },
    { -1717.891f, 1506.948f, 55.78529f },
    { -1718.047f, 1499.267f, 56.18964f },
    { -1718.125f, 1495.405f, 56.31914f },
    { -1717.693f, 1492.178f, 56.45717f },
    { -1717.693f, 1492.178f, 56.45717f },
};

uint32 const runtHousePathSize6 = 9;

Position const worgenRuntHousePath6[runtHousePathSize6] =
{
    { -1718.083f, 1532.09f, 56.25435f },
    { -1718.104f, 1524.071f, 55.80854f },
    { -1718.125f, 1516.053f, 55.36273f },
    { -1718.134f, 1512.459f, 55.41573f },
    { -1718.143f, 1508.866f, 55.64067f },
    { -1718.151f, 1506.013f, 55.83052f },
    { -1718.167f, 1499.665f, 56.16953f },
    { -1718.172f, 1497.578f, 56.24289f },
    { -1718.172f, 1497.578f, 56.24289f },
};

uint32 const runtHousePathSize7 = 9;

Position const worgenRuntHousePath7[runtHousePathSize7] =
{
    { -1718.083f, 1532.09f, 56.25435f },
    { -1718.104f, 1524.071f, 55.80854f },
    { -1718.125f, 1516.053f, 55.36273f },
    { -1718.134f, 1512.459f, 55.41573f },
    { -1718.143f, 1508.866f, 55.64067f },
    { -1718.151f, 1506.013f, 55.83052f },
    { -1718.167f, 1499.665f, 56.16953f },
    { -1718.172f, 1497.578f, 56.24289f },
    { -1718.172f, 1497.578f, 56.24289f },
};

uint32 const runtCathedralPathSize1 = 10;

Position const worgenRuntCathedralPath1[runtCathedralPathSize1] =
{
    { -1612.885f, 1492.154f, 67.03599f },
    { -1618.054f, 1489.644f, 68.5475f },
    { -1623.222f, 1487.134f, 70.05901f },
    { -1629.727f, 1483.976f, 72.59077f },
    { -1630.914f, 1483.4f, 72.92308f },
    { -1632.244f, 1482.754f, 72.91827f },
    { -1636.865f, 1480.51f, 68.61356f },
    { -1638.087f, 1479.916f, 67.58314f },
    { -1638.852f, 1479.545f, 66.56925f },
    { -1638.852f, 1479.545f, 66.56925f },
};

uint32 const runtCathedralPathSize2 = 7;

Position const worgenRuntCathedralPath2[runtCathedralPathSize2] =
{
    { -1618.982f, 1489.76f, 68.56043f },
    { -1625.62f, 1487.033f, 71.4378f },
    { -1632.258f, 1484.306f, 74.31516f },
    { -1634.6f, 1483.343f, 72.61462f },
    { -1636.067f, 1482.741f, 70.69682f },
    { -1639.282f, 1481.42f, 66.99659f },
    { -1639.282f, 1481.42f, 66.99659f },
};

uint32 const runtCathedralPathSize3 = 6;

Position const worgenRuntCathedralPath3[runtCathedralPathSize3] =
{
    { -1637.957f, 1493.445f, 67.77746f },
    { -1638.569f, 1489.736f, 68.47077f },
    { -1639.182f, 1486.027f, 69.16409f },
    { -1640.578f, 1477.564f, 64.01109f },
    { -1640.676f, 1476.976f, 63.45144f },
    { -1640.676f, 1476.976f, 63.45144f },
};

uint32 const runtCathedralPathSize4 = 5;

Position const worgenRuntCathedralPath4[runtCathedralPathSize4] =
{
    { -1628.66f, 1482.281f, 71.34027f },
    { -1630.399f, 1481.66f, 71.33196f },
    { -1632.139f, 1481.039f, 71.32365f },
    { -1639.006f, 1478.586f, 65.77306f },
    { -1639.006f, 1478.586f, 65.77306f },
};


uint32 const runtCathedralPathSize5 = 9;

Position const worgenRuntCathedralPath5[runtCathedralPathSize5] =
{
    { -1620.279f, 1484.46f, 67.03528f },
    { -1622.424f, 1483.882f, 68.05564f },
    { -1624.568f, 1483.304f, 69.076f },
    { -1628.933f, 1482.127f, 70.91297f },
    { -1632.153f, 1481.259f, 71.52889f },
    { -1637.776f, 1479.743f, 67.45475f },
    { -1638.956f, 1479.425f, 66.41499f },
    { -1639.354f, 1479.318f, 65.98292f },
    { -1639.354f, 1479.318f, 65.98292f },
};

uint32 const runtCathedralPathSize6 = 10;

Position const worgenRuntCathedralPath6[runtCathedralPathSize6] =
{
    { -1633.998f, 1495.233f, 68.24403f },
    { -1634.344f, 1491.3f, 70.41303f },
    { -1634.689f, 1487.368f, 72.58203f },
    { -1634.735f, 1486.842f, 72.91134f },
    { -1634.837f, 1485.684f, 73.20835f },
    { -1634.987f, 1483.981f, 72.29824f },
    { -1635.084f, 1482.868f, 71.9163f },
    { -1635.166f, 1481.939f, 70.77574f },
    { -1635.809f, 1474.621f, 63.4636f },
    { -1635.809f, 1474.621f, 63.4636f },
};

uint32 const runtCathedralPathSize7 = 7;

Position const worgenRuntCathedralPath7[runtCathedralPathSize7] =
{
    { -1629.975f, 1494.066f, 70.64719f },
    { -1631.979f, 1491.585f, 71.31316f },
    { -1633.984f, 1489.104f, 71.97912f },
    { -1634.991f, 1487.856f, 72.09129f },
    { -1636.631f, 1485.826f, 71.53807f },
    { -1640.601f, 1480.912f, 65.49457f },
    { -1640.601f, 1480.912f, 65.49457f },
};

uint32 const runtCathedralPathSize8 = 8;

Position const worgenRuntCathedralPath8[runtCathedralPathSize8] =
{
    { -1620.879f, 1491.133f, 70.67613f },
    { -1622.665f, 1489.818f, 71.04526f },
    { -1624.451f, 1488.503f, 71.41438f },
    { -1630.469f, 1484.073f, 73.09648f },
    { -1631.376f, 1483.405f, 73.20229f },
    { -1632.245f, 1482.765f, 72.92912f },
    { -1635.605f, 1480.292f, 68.99215f },
    { -1635.605f, 1480.292f, 68.99215f },
};

Position const worgenRuntJumpPos[] =
{
    { -1694.03f, 1466.33f, 52.2872f },
    { -1688.92f, 1455.69f, 52.2871f },
    { -1699.46f, 1468.43f, 52.2871f },
    { -1697.73f, 1469.52f, 52.2871f },
    { -1701.65f, 1470.94f, 52.2871f },
    { -1693.91f, 1468.46f, 52.2872f },
    { -1697.30f, 1464.65f, 52.2871f },

    { -1681.57f, 1455.77f, 52.2871f },
    { -1677.47f, 1454.94f, 52.2871f },
    { -1677.71f, 1452.2f,  52.2871f },
    { -1677.66f, 1450.93f, 52.2871f },
    { -1672.56f, 1448.06f, 52.2871f },
    { -1683.13f, 1455.39f, 52.2871f },
    { -1669.81f, 1442.34f, 52.2871f },
    { -1674.15f, 1448.9f,  52.2871f },
};

Position const WorgenRuntHousePos[] =
{
    { -1729.345f, 1526.495f, 55.47962f, 6.188943f },
    { -1709.63f, 1527.464f, 56.86086f, 3.258752f },
    { -1717.75f, 1513.727f, 55.47941f, 4.704845f },
    { -1724.719f, 1526.731f, 55.66177f, 6.138319f },
    { -1713.974f, 1526.625f, 56.21981f, 3.306195f },
    { -1718.104f, 1524.071f, 55.81641f, 4.709816f },
    { -1718.262f, 1518.557f, 55.55954f, 4.726997f },

    { -1618.054f, 1489.644f, 68.45153f, 3.593639f },
    { -1625.62f, 1487.033f, 71.27762f, 3.531424f },
    { -1638.569f, 1489.736f, 68.55273f, 4.548815f },
    { -1630.399f, 1481.66f, 71.41516f, 3.484555f },
    { -1622.424f, 1483.882f, 67.67381f, 3.404875f },
    { -1634.344f, 1491.3f, 70.10101f, 4.6248f },
    { -1631.979f, 1491.585f, 71.11481f, 4.032866f },
    { -1627.273f, 1499.689f, 68.89395f, 4.251452f },
    { -1622.665f, 1489.818f, 71.03797f, 3.776179f },
};

uint8 const JamesPathLenght = 6;
Position const JamesPath[][JamesPathLenght] =
{
    {
        { -1925.925049f, 2539.176514f, 1.392833f, 0.0f },
        { -1913.658203f, 2545.986328f, 1.465530f, 0.0f },
        { -1904.370728f, 2552.793213f, 1.132485f, 0.0f },
        { -1900.970459f, 2550.849365f, 0.714445f, 0.0f },
        { -1886.868774f, 2540.282471f, 1.706371f, 0.0f },
        { -1882.739746f, 2543.941865f, 1.628683f, 0.0f },
    },
};

uint8 const CynthiaPathLenght = 6;
Position const CynthiaPath[][CynthiaPathLenght] =
{
    {
        { -1947.965088f, 2518.669434f, 1.826697f, 0.0f },
        { -1923.350830f, 2521.841553f, 1.586985f, 0.0f },
        { -1917.197632f, 2520.494385f, 2.297501f, 0.0f },
        { -1890.082031f, 2519.952148f, 1.425827f, 0.0f },
        { -1886.868774f, 2540.282471f, 1.706371f, 0.0f },
        { -1882.739746f, 2543.941865f, 1.628683f, 0.0f },
    },
};

uint8 const AshleyPathLenght = 13;
Position const AshleyPath[][AshleyPathLenght] =
{
    {
        { -1928.023682f, 2558.467285f, 12.733648f, 0.0f },
        { -1928.248901f, 2553.930176f, 12.734390f, 0.0f },
        { -1923.981567f, 2552.113770f, 12.736046f, 0.0f },
        { -1919.301514f, 2563.295166f, 3.579522f, 0.0f },
        { -1930.442017f, 2562.145996f, 3.579824f, 0.0f },
        { -1941.160156f, 2566.118896f, 1.392157f, 0.0f },
        { -1940.852295f, 2543.049072f, 1.392157f, 0.0f },
        { -1919.504517f, 2543.273926f, 1.392157f, 0.0f },
        { -1913.658203f, 2545.986328f, 1.465530f, 0.0f },
        { -1904.370728f, 2552.793213f, 1.132485f, 0.0f },
        { -1900.970459f, 2550.849365f, 0.714445f, 0.0f },
        { -1886.868774f, 2540.282471f, 1.706371f, 0.0f },
        { -1882.739746f, 2543.941865f, 1.628683f, 0.0f },
    },
};

uint8 const childrenBasementPathLenght = 3;
Position const childrenBasementPath[][childrenBasementPathLenght] =
{
    {
        { -1879.062378f, 2546.958984f, -0.130342f, 0.0f },
        { -1873.854980f, 2550.903564f, -5.898719f, 0.0f },
        { -1868.589844f, 2536.521240f, -6.365717f, 0.0f },
    },
};

uint8 const KorothPathLenght = 2;
Position const KorothPath[][KorothPathLenght] =
{
    {
        { -2271.431f, 1963.941f, 99.342613f, 0.0f },
        { -2284.237f, 1963.801f, 95.656654f, 0.0f },
    },
};

const uint32 PlayerText[3] =
{
    36329,   //"It's not safe here.  Go to the Allens' basement.",
    36331,   //"Join the others inside the basement next door. Hurry!",
    36328    //"Your mother's in the basement next door. Get to her now!",
};

struct npc_gilneas_crow : public ScriptedAI
{
    npc_gilneas_crow(Creature* creature) : ScriptedAI(creature) { }

    uint32 spawn;
    bool flying;

    void Reset() override
    {
        flying = false;
        spawn = 0;
        me->SetPosition(me->GetCreatureData()->posX, me->GetCreatureData()->posY, me->GetCreatureData()->posZ, me->GetCreatureData()->orientation);
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_PING_GILNEAN_CROW)
        {
            if (!flying)
            {
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetDisableGravity(true);
                flying = true;
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!flying)
            return;

        if (spawn <= diff)
        {
            me->GetMotionMaster()->MovePoint(0, (me->GetPositionX() + irand(-15, 15)), (me->GetPositionY() + irand(-15, 15)), (me->GetPositionZ() + irand(5, 15)));
            spawn = urand (500, 1000);
        }
        else spawn -= diff;

        if ((me->GetPositionZ() - me->GetCreatureData()->posZ) >= 20.0f)
        {
            me->DisappearAndDie();
            me->RemoveCorpse(true);
            flying = false;
        }
    }
};

struct npc_gilneas_city_guard_gate : public ScriptedAI
{
    npc_gilneas_city_guard_gate(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_events;
    uint8 m_say;
    uint8 m_emote;
    ObjectGuid m_citizenGUID;

    void Reset() override
    {
        if (me->GetDistance2d(-1430.47f, 1345.55f) < 10.0f)
            m_events.ScheduleEvent(EVENT_START_TALK_WITH_CITIZEN, randtime(10s, 30s));
    }

    void UpdateAI(uint32 diff) override
    {
        m_events.Update(diff);

        while (uint32 eventId = m_events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START_TALK_WITH_CITIZEN:
                {
                    m_citizenGUID = GetRandomCitizen();
                    m_emote = RAND(EMOTE_ONESHOT_COWER, EMOTE_STATE_TALK, EMOTE_ONESHOT_CRY, EMOTE_ONESHOT_BEG, EMOTE_ONESHOT_EXCLAMATION, EMOTE_ONESHOT_POINT);
                    m_say = 0;

                    if (Creature* npc = ObjectAccessor::GetCreature(*me, m_citizenGUID))
                        npc->HandleEmoteCommand(m_emote);

                    m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_1, randtime(2s + 200ms, 3s));
                    break;
                }
                case EVENT_TALK_WITH_CITIZEN_1:
                {
                    if (Creature* npc = ObjectAccessor::GetCreature(*me, m_citizenGUID))
                        npc->AI()->Talk(m_say);

                    m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_2, 5s);
                    break;
                }
                case EVENT_TALK_WITH_CITIZEN_2:
                {
                    Talk(m_say);
                    m_events.ScheduleEvent(EVENT_TALK_WITH_CITIZEN_3, 5s);
                    break;
                }
                case EVENT_TALK_WITH_CITIZEN_3:
                {
                    if (Creature* npc = ObjectAccessor::GetCreature(*me, m_citizenGUID))
                        npc->HandleEmoteCommand(EMOTE_STATE_NONE);

                    m_events.ScheduleEvent(EVENT_START_TALK_WITH_CITIZEN, randtime(5s, 30s));
                    break;
                }
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

    void FillCitizenList()
    {
        listOfCitizenGUID.clear();
        std::list<Creature*> listOfCitizen;
        me->GetCreatureListWithEntryInGrid(listOfCitizen, NPC_PANICKED_CITIZEN_GATE, 35.0f);

        for (std::list<Creature*>::iterator itr = listOfCitizen.begin(); itr != listOfCitizen.end(); ++itr)
            listOfCitizenGUID.push_back((*itr)->GetGUID());
    }

    ObjectGuid GetRandomCitizen()
    {
        if (listOfCitizenGUID.empty())
            FillCitizenList();

        uint8 rol = urand(0, listOfCitizenGUID.size() - 1);
        std::list<ObjectGuid>::iterator itr = listOfCitizenGUID.begin();
        std::advance(itr, rol);
        return (*itr);
    }

private:
    std::list<ObjectGuid> listOfCitizenGUID;
};

struct npc_prince_liam_greymane : public ScriptedAI
{
    npc_prince_liam_greymane(Creature *c) : ScriptedAI(c) { }

    EventMap _events;

    void Reset() override
    {
        _events.RescheduleEvent(EVENT_START_DIALOG, 1s);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_START_DIALOG:
                {
                    _events.ScheduleEvent(EVENT_RESET_DIALOG, 2min);
                    _events.ScheduleEvent(EVENT_START_TALK_TO_GUARD, 1s);
                    break;
                }
                case EVENT_START_TALK_TO_GUARD:
                {
                    Talk(PRINCE_LIAM_GREYMANE_TEXT_00);
                    _events.ScheduleEvent(EVENT_TALK_TO_GUARD_1, 15s);
                    break;
                }
                case EVENT_TALK_TO_GUARD_1:
                {
                    Talk(PRINCE_LIAM_GREYMANE_TEXT_01);
                    _events.ScheduleEvent(EVENT_TALK_TO_GUARD_2, 18s);
                    break;
                }
                case EVENT_TALK_TO_GUARD_2:
                {
                    Talk(PRINCE_LIAM_GREYMANE_TEXT_02);
                    break;
                }
                case EVENT_RESET_DIALOG:
                {
                    Reset();
                    break;
                }
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

struct npc_worgen_runt : public ScriptedAI
{
    npc_worgen_runt(Creature* creature) : ScriptedAI(creature)
    {
        Initialize();
    }

    void Initialize()
    {
        _worgenID = 0;
        _wayPointCounter = 0;
        _jumped = false;
        _playerGuid = ObjectGuid::Empty;
    }

    void JustEngagedWith(Unit* /*who*/) override
    {
        _events.SetPhase(PHASE_COMBAT);
    }

    void IsSummonedBy(Unit* summoner) override
    {
        me->setActive(true);
        _events.SetPhase(PHASE_ROOF);
        _events.ScheduleEvent(EVENT_FORCE_DESPAWN, 1min + 10s, 0, PHASE_ROOF);
        _playerGuid = summoner->GetGUID();

        if (me->GetEntry() == NPC_WORGEN_RUNT_2)
            me->GetMotionMaster()->MoveJump(runt2SummonJumpPos, 16.0f, 4.371286f);

        if (me->GetEntry() == NPC_WORGEN_ALPHA)
        {
            me->GetMotionMaster()->MoveJump(alphaSummonJumpPos, 14.0f, 5.395287f);

            me->m_Events.AddLambdaEventAtOffset([this]()
            {
                me->GetMotionMaster()->MovePoint(0, -1676.767944f, 1442.418457f, 52.287319f, true);
            }, 2500);
        }

        if (me->GetEntry() == NPC_WORGEN_ALPHA_2)
            me->GetMotionMaster()->MoveJump(alpha2SummonJumpPos, 17.0f, 4.937166f);
    }

    void DoAction(int32 action) override
    {
        _worgenID = action;

        switch (_worgenID)
        {
            case WORGEN_ID_ROOF_1:
                _wayPointCounter = runtHousePathSize1;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize1, worgenRuntHousePath1, runtHousePathSize1);
                break;
            case WORGEN_ID_ROOF_2:
                _wayPointCounter = runtHousePathSize2;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize2, worgenRuntHousePath2, runtHousePathSize2);
                break;
            case WORGEN_ID_ROOF_3:
                _wayPointCounter = runtHousePathSize3;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize3, worgenRuntHousePath3, runtHousePathSize3);
                break;
            case WORGEN_ID_ROOF_4:
                _wayPointCounter = runtHousePathSize4;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize4, worgenRuntHousePath4, runtHousePathSize4);
                break;
            case WORGEN_ID_ROOF_5:
                _wayPointCounter = runtHousePathSize5;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize5, worgenRuntHousePath5, runtHousePathSize5);
                break;
            case WORGEN_ID_ROOF_6:
                _wayPointCounter = runtHousePathSize6;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize6, worgenRuntHousePath6, runtHousePathSize6);
                break;
            case WORGEN_ID_ROOF_7:
                _wayPointCounter = runtHousePathSize7;
                me->GetMotionMaster()->MoveSmoothPath(runtHousePathSize7, worgenRuntHousePath7, runtHousePathSize7);
                break;
            case WORGEN_ID_CATHEDRAL_1:
                _wayPointCounter = runtCathedralPathSize1;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize1, worgenRuntCathedralPath1, runtCathedralPathSize1);
                break;
            case WORGEN_ID_CATHEDRAL_2:
                _wayPointCounter = runtCathedralPathSize2;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize2, worgenRuntCathedralPath2, runtCathedralPathSize2);
                break;
            case WORGEN_ID_CATHEDRAL_3:
                _wayPointCounter = runtCathedralPathSize3;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize3, worgenRuntCathedralPath3, runtCathedralPathSize3);
                break;
            case WORGEN_ID_CATHEDRAL_4:
                _wayPointCounter = runtCathedralPathSize4;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize4, worgenRuntCathedralPath4, runtCathedralPathSize4);
                break;
            case WORGEN_ID_CATHEDRAL_5:
                _wayPointCounter = runtCathedralPathSize5;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize5, worgenRuntCathedralPath5, runtCathedralPathSize5);
                break;
            case WORGEN_ID_CATHEDRAL_6:
                _wayPointCounter = runtCathedralPathSize6;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize6, worgenRuntCathedralPath6, runtCathedralPathSize6);
                break;
            case WORGEN_ID_CATHEDRAL_7:
                _wayPointCounter = runtCathedralPathSize7;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize7, worgenRuntCathedralPath7, runtCathedralPathSize7);
                break;
            case WORGEN_ID_CATHEDRAL_8:
                _wayPointCounter = runtCathedralPathSize8;
                me->GetMotionMaster()->MoveSmoothPath(runtCathedralPathSize8, worgenRuntCathedralPath8, runtCathedralPathSize8);
                break;
        }
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == EFFECT_MOTION_TYPE && pointId == _wayPointCounter && !_jumped)
        {
            _jumped = true;
            _events.ScheduleEvent(EVENT_JUMP_TO_PRISON, 1s);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim() && !_events.IsInPhase(PHASE_ROOF))
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_FORCE_DESPAWN:
                    me->DespawnOrUnsummon();
                    break;
                case EVENT_JUMP_TO_PRISON:
                    me->GetMotionMaster()->MoveJump(worgenRuntJumpPos[_worgenID], 16.0f, _worgenID < WORGEN_ID_CATHEDRAL_1 ? 19.2911f : frand(3.945607f, 4.852813f));
                    me->SetHomePosition(worgenRuntJumpPos[_worgenID]);
                    _events.ScheduleEvent(EVENT_AGGRO_PLAYER, 2s);
                    break;
                case EVENT_AGGRO_PLAYER:
                    if (Unit* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
                        if (me->IsAIEnabled && me->GetHomePosition().IsInDist(player, 100.0f))
                            me->AI()->AttackStart(player);
                    break;
                default:
                    break;
            }
        }

        DoMeleeAttackIfReady();
    }

private:
    uint32 _worgenID;
    uint32 _wayPointCounter;
    bool _jumped;
    ObjectGuid _playerGuid;
    EventMap _events;
};

class spell_gen_gilneas_prison_periodic_dummy : public SpellScript
{
    PrepareSpellScript(spell_gen_gilneas_prison_periodic_dummy);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        if (!sSpellMgr->GetSpellInfo(SPELL_SUMMON_RAVENOUS_WORGEN_1) ||
            !sSpellMgr->GetSpellInfo(SPELL_SUMMON_RAVENOUS_WORGEN_2))
            return false;
        return true;
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
        {
            switch (RAND(0, 1))
            {
                case 0:
                    caster->CastSpell(caster, SPELL_SUMMON_RAVENOUS_WORGEN_1, true);
                    for (uint8 i = 0; i < 7; i++)
                        if (Creature* runt = caster->SummonCreature(NPC_WORGEN_RUNT, WorgenRuntHousePos[i]))
                            runt->AI()->DoAction(i);
                    break;
                case 1:
                    caster->CastSpell(caster, SPELL_SUMMON_RAVENOUS_WORGEN_2, true);
                    for (uint8 i = 7; i < 16; i++)
                        if (Creature* runt = caster->SummonCreature(NPC_WORGEN_RUNT, WorgenRuntHousePos[i]))
                            runt->AI()->DoAction(i);
                    if (RAND(0, 1) == 1)
                        for (uint8 i = 0; i < RAND(1, 3); i++)
                            if (Creature* runt = caster->SummonCreature(NPC_WORGEN_RUNT, WorgenRuntHousePos[i]))
                                runt->AI()->DoAction(i);
                    break;
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gen_gilneas_prison_periodic_dummy::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

struct npc_josiah_avery : public ScriptedAI
{
    npc_josiah_avery(Creature* creature) : ScriptedAI(creature) { }

    uint32 _text_timer;
    uint32 _current_text;

    void Reset() override
    {
        _text_timer = 20 * IN_MILLISECONDS;
        _current_text = 1;
    }

    void UpdateAI(uint32 diff) override
    {
        if (!me->GetVictim() && me->FindNearestPlayer(20.0f))
        {
            if (_text_timer <= diff)
            {
                switch (_current_text)
                {
                    case 1:
                        Talk(SAY_JOSIAH_AVERY_1);
                        _current_text++;
                        break;
                    case 2:
                        Talk(SAY_JOSIAH_AVERY_2);
                        _current_text++;
                        break;
                    case 3:
                        Talk(SAY_JOSIAH_AVERY_3);
                        _current_text++;
                        break;
                    case 4:
                        Talk(SAY_JOSIAH_AVERY_4);
                        _current_text++;
                        break;
                    case 5:
                        Talk(SAY_JOSIAH_AVERY_5);
                        _current_text++;
                        break;
                    case 6:
                        Talk(SAY_JOSIAH_AVERY_6);
                        _current_text = 1;
                        break;
                }

                _text_timer = 20 * IN_MILLISECONDS;
            }
            else
            {
                _text_timer -= diff;
            }
            return;
        }
    }
};

struct npc_josiah_avery_worgen_form : public PassiveAI
{
    npc_josiah_avery_worgen_form(Creature* creature) : PassiveAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _playerGuid = summoner->GetGUID();

        me->m_Events.AddLambdaEventAtOffset([this, summoner]()
        {
            me->SetFacingToObject(summoner);
            _events.ScheduleEvent(EVENT_COSMETIC_ATTACK, 500ms);
        }, 200);
    }

    void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_SHOOT_INSTAKILL)
            me->CastSpell(me, SPELL_GET_SHOT);
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(5 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch(eventId)
            {
                case EVENT_COSMETIC_ATTACK:
                    if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
                    {
                        DoCast(player, SPELL_COSMETIC_ATTACK);

                        me->m_Events.AddLambdaEventAtOffset([this, player]()
                        {
                            if (Creature* lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY, 30.0f, true))
                                player->GetMotionMaster()->MoveKnockbackFrom(lorna->GetPositionX(), lorna->GetPositionY(), 30, 30);
                        }, 400);

                        if (Creature* lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY, 30.0f, true))
                            if (Creature* labTrigger = lorna->FindNearestCreature(NPC_GENERIC_TRIGGER_LAB, 5.0f, true))
                                labTrigger->CastSpell(player, SPELL_PULL_TO);

                        _events.ScheduleEvent(EVENT_JUMP_TO_PLAYER, 1s);
                    }
                    break;
                case EVENT_JUMP_TO_PLAYER:
                    me->GetMotionMaster()->MoveJump(josiahJumpPos, 10.0f, 14.18636f);
                    _events.ScheduleEvent(EVENT_SHOOT_JOSIAH, 1s + 200ms);
                    break;
                case EVENT_SHOOT_JOSIAH:
                    if (Creature* lorna = me->FindNearestCreature(NPC_LORNA_CROWLEY, 30.0f, true))
                        lorna->CastSpell(me, SPELL_SHOOT_INSTAKILL, true);
                    break;
                default:
                    break;
            }
        }
    }

private:
    ObjectGuid _playerGuid;
    EventMap _events;
};

class spell_gilneas_pull_to : public SpellScript
{
    PrepareSpellScript(spell_gilneas_pull_to);

    void HandPullEffect(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        if (Unit* playerTarget = GetHitPlayer())
        {
            if (Unit* trigger = GetCaster())
            {
                float angle = playerTarget->GetAngle(trigger);
                playerTarget->SendMoveKnockBack(playerTarget->ToPlayer(), 30.0f, -7.361481f, cos(angle), sin(angle));
            }
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gilneas_pull_to::HandPullEffect, EFFECT_0, SPELL_EFFECT_PULL_TOWARDS);
    }
};

class npc_lorna_crowley_basement : public CreatureScript
{
public:
    npc_lorna_crowley_basement(const char* ScriptName) : CreatureScript(ScriptName) { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
        {
            if (player->GetClass() == CLASS_HUNTER)
                player->UnsummonPetTemporaryIfAny();

            player->CastSpell(player, SPELL_SUMMON_MASTIFF, false);
            creature->AI()->Talk(0);
        }
        return true;
    }

    bool OnQuestReward(Player* player, Creature* creature, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() == QUEST_FROM_THE_SHADOWS)
            if (Unit* charm = Unit::GetCreature(*creature, player->GetMinionGUID()))
                if (charm->GetEntry() == NPC_GILNEAS_MASTIFF)
                    if (Creature* mastiff = charm->ToCreature())
                        mastiff->DespawnOrUnsummon();

        if (player->GetClass() == CLASS_HUNTER)
            player->ResummonPetTemporaryUnSummonedIfAny();

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lorna_crowley_basementAI (creature);
    }

    struct npc_lorna_crowley_basementAI : public ScriptedAI
    {
        npc_lorna_crowley_basementAI(Creature* creature) : ScriptedAI(creature)
        {
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        }

        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
        }
    };
};

struct npc_gilnean_mastiff : public ScriptedAI
{
    npc_gilnean_mastiff(Creature* creature) : ScriptedAI(creature) 
    {
        me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_ATTACK_LURKER, true);
    }

    void Reset() override
    {
        me->GetCharmInfo()->InitEmptyActionBar(false);
        me->GetCharmInfo()->SetActionBar(0, SPELL_ATTACK_LURKER, ACT_PASSIVE);
        me->SetReactState(REACT_DEFENSIVE);
        me->GetCharmInfo()->SetIsFollowing(true);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        Player* player = me->GetOwner()->ToPlayer();

        if (player->GetQuestStatus(QUEST_FROM_THE_SHADOWS) == QUEST_STATUS_REWARDED)
            me->DespawnOrUnsummon(1);

        if (!UpdateVictim())
        {
            me->GetCharmInfo()->SetIsFollowing(true);
            me->SetReactState(REACT_DEFENSIVE);
            return;
        }

        DoMeleeAttackIfReady();
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->DespawnOrUnsummon(1);
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        Reset();
    }
};

struct npc_bloodfang_lurker : public ScriptedAI
{
    npc_bloodfang_lurker(Creature* creature) : ScriptedAI(creature) { }

    bool _enrage;
    bool _frenzy;

    void Reset() override
    {
        _enrage = false;
        _frenzy = false;
        DoCastSelf(SPELL_SHADOWSTALKER_STEALTH);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        me->SetReactState(REACT_PASSIVE);
    }

    void StartAttack(Unit* who)
    {
        me->SetReactState(REACT_AGGRESSIVE);
        me->SetInCombatWith(who);
        who->SetInCombatWith(me);
    }

    void DamageTaken(Unit* attacker, uint32 &damage) override
    {
        if (me->HasReactState(REACT_PASSIVE))
            StartAttack(attacker);
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_ATTACK_LURKER)
            StartAttack(caster);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        if (!_frenzy && me->HealthBelowPct(45))
        {
            _frenzy = true;
            DoCast(SPELL_UNDYING_FRENZY);
        }

        if (!_enrage && me->HealthBelowPct(30))
        {
            _enrage = true;
            DoCast(SPELL_ENRAGE);
            Talk(WORGEN_ENRAGE_EMOTE);
        }

        DoMeleeAttackIfReady();
    }
};

class spell_attack_lurker : public SpellScript
{
    PrepareSpellScript(spell_attack_lurker);

    void HandleDummy()
    {
        Unit* caster = GetCaster();

        if (Creature* target = caster->FindNearestCreature(NPC_BLOODFANG_LURKER, 30.0f))
        {
            float x,y,z,o;
            target->GetContactPoint(caster, x, y, z, CONTACT_DISTANCE);
            o = caster->GetOrientation();
            float speedXY, speedZ;
            speedZ = 10.0f;
            speedXY = caster->GetExactDist2d(x, y) * 30.0f / speedZ;
            caster->GetMotionMaster()->MoveCharge(x, y, z, speedXY, speedZ);
            target->RemoveAura(SPELL_SHADOWSTALKER_STEALTH);
            target->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            target->SetReactState(REACT_AGGRESSIVE);
            target->AI()->AttackStart(caster);
        }
    }

    void Register() override
    {
        AfterCast += SpellCastFn(spell_attack_lurker::HandleDummy);
    }
};

class npc_king_genn_greymane : public CreatureScript
{
public:
    npc_king_genn_greymane() : CreatureScript("npc_king_genn_greymane") { }

    bool OnQuestAccept(Player* player, Creature* creature, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_SAVE_KRENNAN_ARANAS)
        {
            float x, y;
            creature->GetNearPoint2D(nullptr, x, y, 2.0f, player->GetOrientation() + M_PI / 2);

            if (Creature* horse = player->SummonCreature(NPC_KING_GREYMANES_HORSE, x, y, creature->GetPositionZ(), creature->GetOrientation()))
            {
                if (npc_escortAI* escort = CAST_AI(npc_escortAI, horse->AI()))
                {
                    escort->AddWaypoint(1, -1799.37f, 1400.21f, 19.8951f);
                    escort->AddWaypoint(2, -1798.23f, 1396.9f, 19.8993f);
                    escort->AddWaypoint(3, -1795.03f, 1388.01f, 19.8898f);
                    escort->AddWaypoint(4, -1790.16f, 1378.7f, 19.8016f);
                    escort->AddWaypoint(5, -1786.41f, 1372.97f, 19.8406f);
                    escort->AddWaypoint(6, -1779.72f, 1364.88f, 19.8131f);
                    escort->AddWaypoint(7, -1774.43f, 1359.87f, 19.7021f);
                    escort->AddWaypoint(8, -1769.0f, 1356.76f, 19.7439f);
                    escort->AddWaypoint(9, -1762.64f, 1356.02f, 19.7979f);
                    escort->AddWaypoint(10, -1758.91f, 1356.08f, 19.8635f);
                    escort->AddWaypoint(11, -1751.95f, 1356.8f, 19.8273f);
                    escort->AddWaypoint(12, -1745.66f, 1357.21f, 19.7993f);
                    escort->AddWaypoint(13, -1738.7f, 1356.64f, 19.7822f);
                    escort->AddWaypoint(14, -1731.79f, 1355.51f, 19.7149f);
                    escort->AddWaypoint(15, -1724.89f, 1354.29f, 19.8661f);
                    escort->AddWaypoint(16, -1718.03f, 1352.93f, 19.7824f);
                    escort->AddWaypoint(17, -1707.68f, 1351.16f, 19.7811f, 0, true); // Jump
                    escort->AddWaypoint(18, -1673.04f, 1344.91f, 15.1353f, 2000);
                    escort->AddWaypoint(19, -1673.04f, 1344.91f, 15.1353f);
                    escort->AddWaypoint(20, -1669.32f, 1346.55f, 15.1353f);
                    escort->AddWaypoint(21, -1666.45f, 1349.89f, 15.1353f);
                    escort->AddWaypoint(22, -1665.61f, 1353.85f, 15.1353f);
                    escort->AddWaypoint(23, -1666.04f, 1358.01f, 15.1353f);
                    escort->AddWaypoint(24, -1669.79f, 1360.71f, 15.1353f);
                    escort->AddWaypoint(25, -1673.1f, 1362.11f, 15.1353f);
                    escort->AddWaypoint(26, -1677.12f, 1361.57f, 15.1353f);
                    escort->AddWaypoint(27, -1679.9f, 1360.1f, 15.1353f);
                    escort->AddWaypoint(28, -1682.79f, 1357.56f, 15.1353f);
                    escort->AddWaypoint(29, -1682.79f, 1357.56f, 15.1353f);
                    escort->AddWaypoint(30, -1689.07f, 1352.39f, 15.1353f);
                    escort->AddWaypoint(31, -1691.91f, 1351.83f, 15.1353f);
                    escort->AddWaypoint(32, -1703.81f, 1351.82f, 19.7604f);
                    escort->AddWaypoint(33, -1707.26f, 1352.38f, 19.7826f);
                    escort->AddWaypoint(34, -1712.25f, 1353.55f, 19.7826f);
                    escort->AddWaypoint(35, -1718.2f, 1356.51f, 19.7164f);
                    escort->AddWaypoint(36, -1741.5f, 1372.04f, 19.9569f);
                    escort->AddWaypoint(37, -1746.23f, 1375.8f, 19.9817f);
                    escort->AddWaypoint(38, -1751.06f, 1380.53f, 19.8424f);
                    escort->AddWaypoint(39, -1754.97f, 1386.34f, 19.8474f);
                    escort->AddWaypoint(40, -1760.77f, 1394.37f, 19.9282f);
                    escort->AddWaypoint(41, -1765.11f, 1402.07f, 19.8816f);
                    escort->AddWaypoint(42, -1768.24f, 1410.2f, 19.7833f);
                    escort->AddWaypoint(43, -1772.26f, 1420.48f, 19.9029f);
                    escort->AddWaypoint(44, -1776.98f, 1436.13f, 19.632f);
                    player->EnterVehicle(horse, 0);
                }
            }
        }

        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_king_genn_greymaneAI (creature);
    }

    struct npc_king_genn_greymaneAI : public ScriptedAI
    {
        npc_king_genn_greymaneAI(Creature* creature) : ScriptedAI(creature)
        {
            _scheduler.CancelAll();

            _scheduler.Schedule(randtime(15s, 35s), [this](TaskContext context)
            {
                Talk(0);
                context.Repeat(randtime(15s, 35s));
            });
        }

        void UpdateAI(uint32 diff) override
        {
            _scheduler.Update(diff);
        }

    private:
        TaskScheduler _scheduler;
    };
};

class npc_vehicle_genn_horse : public CreatureScript
{
public:
    npc_vehicle_genn_horse() : CreatureScript("npc_vehicle_genn_horse") { }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_vehicle_genn_horseAI (creature);
    }

    struct npc_vehicle_genn_horseAI : public npc_escortAI
    {
        npc_vehicle_genn_horseAI(Creature* creature) : npc_escortAI(creature)
        {
            _aranasSaved = false;
            _playerSeated = false;
        }

        bool _playerSeated;
        bool _aranasSaved;
        ObjectGuid _riderToEscort;

        void AttackStart(Unit* /*who*/) override { }
        void JustEngagedWith(Unit* /*who*/) override { }
        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override { }

        void Reset() override
        {
            _playerSeated = false;
        }

        void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
        {
            if (who->GetTypeId() == TYPEID_PLAYER)
            {
                // #998: the escort starts on the next UpdateAI, not here. The 14293 accept script summons this horse and
                // seats the player in the same call, so this runs before the horse's first Creature::Update, whose
                // JustAppeared() (npc_escortAI, fired on every creature's first update since upstream c6c7d32e) sets the
                // escort state back to NONE: a Start() made here was lost and the horse never left its spawn.
                if (apply)
                    _riderToEscort = who->GetGUID();
            }
            else 
            {
                if (apply)
                {
                    SetEscortPaused(false);
                    _aranasSaved = true;
                    me->SetSpeed(MOVE_WALK, 1.0f, true);
                    me->SetSpeed(MOVE_RUN, 1.3f, true);
                }
                else
                    me->DespawnOrUnsummon(1);
            }
        }

        void WaypointReached(uint32 i) override
        {
            Player* player = GetPlayerForEscort();

            switch(i)
            {
                case 1:
                {
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        if (Vehicle* vehicle = me->GetVehicleKit())
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                if (Player* player = passenger->ToPlayer())
                                    player->SetClientControl(me, 0);
                    }
                    break;
                }
                case 17:
                {
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        if (Creature* aranas = passenger->FindNearestCreature(NPC_KRENNAN_ARANAS, 50.0f))
                            if (Vehicle* vehicle = me->GetVehicleKit())
                                if (Unit* passenger = vehicle->GetPassenger(0))
                                    if (Player* player = passenger->ToPlayer())
                                        aranas->AI()->Talk(TEXT_KRENNAN_ARANAS);

                        me->GetMotionMaster()->MoveJump(-1673.04f, 1344.91f, 15.1353f, 25.0f, 15.0f);

                        me->m_Events.AddLambdaEventAtOffset([this, passenger]()
                        {
                            Talk(EMOTE_SAVE_KRENNAN_ARANAS, passenger);
                        }, 2000);
                    }
                    break;
                }
                case 40:
                {
                    if (Vehicle* vehicle = me->GetVehicleKit())
                    {
                        if (Unit* passenger = vehicle->GetPassenger(0))
                        {
                            if (Player* player = passenger->ToPlayer())
                            {
                                std::list<Creature*> guards;
                                me->GetCreatureListWithEntryInGrid(guards, NPC_GILNEAS_CITY_GUARD, 90.0f);

                                if (!guards.empty())
                                    for (std::list<Creature*>::const_iterator itr = guards.begin(); itr != guards.end(); ++itr)
                                        if ((*itr)->IsAlive())
                                            if (Creature* worgen = (*itr)->FindNearestCreature(NPC_BLOODFANG_RIPPER, 90.0f))
                                                (*itr)->CastSpell(worgen, SPELL_GUARD_SHOOT, false);
                            }
                        }
                    }
                    break;
                }
                case 41:
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->CombatStop();
                    break;
                }
                case 42:
                {
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        if (Unit* passenger = vehicle->GetPassenger(0))
                            if (Player* player = passenger->ToPlayer())
                            {
                                player->KilledMonsterCredit(NPC_KRENNAN_ARANAS_KILL_CREDIT, ObjectGuid::Empty);

                                if (Unit* passenger_2 = vehicle->GetPassenger(1))
                                    if (Creature* aranas = passenger_2->ToCreature())
                                        aranas->AI()->Talk(TEXT_SAVED_KRENNAN_ARANAS);
                            }
                    break;
                }
                case 44:
                {
                    if (Unit* passenger = me->GetVehicleKit()->GetPassenger(0))
                    {
                        if (Vehicle* vehicle = me->GetVehicleKit())
                        {
                            if (Unit* passenger = vehicle->GetPassenger(0))
                                if (Player* player = passenger->ToPlayer())
                                    player->SetClientControl(me, 1);

                            if (Unit* passenger = vehicle->GetPassenger(1))
                                if (Creature* aranas = passenger->ToCreature())
                                    aranas->DespawnOrUnsummon();

                            vehicle->RemoveAllPassengers();
                        }
                    }
                    me->DespawnOrUnsummon(1);
                    break;
                }
            }
        }

        void OnCharmed(bool /*apply*/) override { }

        void UpdateAI(uint32 diff) override
        {
            // #998: JustAppeared has run by now (Creature::Update calls it before UpdateAI on the first tick)
            if (_riderToEscort)
            {
                ObjectGuid const riderGuid = _riderToEscort;
                _riderToEscort.Clear();
                if (Player* rider = ObjectAccessor::GetPlayer(*me, riderGuid))
                {
                    if (rider->GetVehicleBase() == me)
                    {
                        Start(false, true, riderGuid);
                        me->SetSpeed(MOVE_WALK, 1.0f, true);
                        me->SetSpeed(MOVE_RUN, 1.3f, true);
                        _playerSeated = true;
                    }
                }
            }

            npc_escortAI::UpdateAI(diff);

            if (_playerSeated)
            {
                _playerSeated = false;
                if (Player* player = GetPlayerForEscort())
                    player->SetClientControl(me, 0);
            }
        }
    };
};

struct npc_saved_aranas : public ScriptedAI
{
    npc_saved_aranas(Creature* creature) : ScriptedAI(creature)
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);

        if (me->IsSummon())
        {
            if (Creature* horse = me->FindNearestCreature(NPC_KING_GREYMANES_HORSE, 20.0f))
                DoCast(horse, 84275, true);

            if (Creature* aranas = me->FindNearestCreature(NPC_KRENNAN_ARANAS, 50.0f))
                aranas->DespawnOrUnsummon();
        }
    }
};

// #439 follow-up (t439 cp2/cp2b, builds 6-7): Prince Liam Greymane 34913, the phase-2 Liam of Gilneas City after
// 14078 "Lockdown!". Rampaging Worgen 34884 (40 spawns in phase 2, faction 2174, hostile to Liam's 2173) kill the
// level-5 Liam, who has no AI in this fork, so he is dead (respawn 300 s) most of the time: npcguid alive=true found
// none 57 yd away, also after a relog. The intended script for 34913 is npc_prince_liam_greymane_phase2
// (mopcore gilneas.cpp:88-205): Liam seeks worgen within 16 yd of his post, fights them for show
// (after two hits he only plays the attack animation), yells once, and never loses to them. Ported onto the fork's
// API: worgen cannot take him below half health, his damage to creatures stops at their last 30%, players and pets
// still get the full fight. SQL 2026_10_04_43 binds the ScriptName.
struct npc_prince_liam_greymane_phase2 : public ScriptedAI
{
    npc_prince_liam_greymane_phase2(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _scheduler.CancelAll();
        _yelled = false;

        _scheduler.Schedule(1s, 2s, [this](TaskContext context)
        {
            if (!me->IsInCombat() && me->GetDistance2d(me->GetHomePosition().GetPositionX(), me->GetHomePosition().GetPositionY()) <= 2.0f)
                if (Creature* worgen = me->FindNearestCreature(34884, 16.0f, true))
                    AttackStart(worgen);
            context.Repeat(1s, 2s);
        });

        _scheduler.Schedule(10s, 20s, [this](TaskContext /*context*/)
        {
            if (!_yelled)
            {
                Talk(0);    // "Stand your ground, men!" etc. (creature_text 34913 group 0)
                _yelled = true;
            }
        });
    }

    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (attacker && attacker->GetTypeId() == TYPEID_UNIT && !attacker->IsControlledByPlayer() && me->HealthBelowPctDamaged(50, damage))
            damage = 0;
    }

    void DamageDealt(Unit* victim, uint32& damage, DamageEffectType /*damageType*/) override
    {
        if (victim && victim->GetTypeId() == TYPEID_UNIT && !victim->IsControlledByPlayer() && victim->HealthBelowPctDamaged(30, damage))
            damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    TaskScheduler _scheduler;
    bool _yelled = false;
};

// Duskhaven street fight (Refs #439, #473, audit row WQ003). Ported from alexkulya pandaria_5.4.8 commit 24ff2ce
// ("Undead soldiers vs Watchmans sparring"), src/server/scripts/EasternKingdoms/Zones/zone_gilneas.cpp:1587-1735.
// Watchmen and Forsaken invaders/footsoldiers seek each other out and fight, but never damage each other,
// so the staged fight never ends and never steals a player's kill. Changes against the reference: the search
// is rescheduled every 2-5 s (the reference searched once and stopped when nothing was in range), and the
// footsoldier search is scheduled (the reference declared EVENT_SEARCH_FOOTSOLDIER but never scheduled it).
struct npc_duskhaven_watchman : public ScriptedAI
{
    npc_duskhaven_watchman(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void Reset() override
    {
        events.Reset();
        events.ScheduleEvent(EVENT_SEARCH_INVADER, 2s);
        events.ScheduleEvent(EVENT_SEARCH_FOOTSOLDIER, 3s);
    }

    void DamageTaken(Unit* who, uint32& damage) override
    {
        if (who && who->ToCreature() && (who->GetEntry() == NPC_ENTRY_INVADER || who->GetEntry() == NPC_ENTRY_FOOTSOLDIER))
            damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SEARCH_INVADER:
                    if (!me->IsInCombat())
                        if (Creature* invader = me->FindNearestCreature(NPC_ENTRY_INVADER, 15.5f, true))
                            AttackStart(invader);
                    events.ScheduleEvent(EVENT_SEARCH_INVADER, randtime(2s, 5s));
                    break;
                case EVENT_SEARCH_FOOTSOLDIER:
                    if (!me->IsInCombat())
                        if (Creature* footsoldier = me->FindNearestCreature(NPC_ENTRY_FOOTSOLDIER, 15.5f, true))
                            AttackStart(footsoldier);
                    events.ScheduleEvent(EVENT_SEARCH_FOOTSOLDIER, randtime(2s, 5s));
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

// Shared by Forsaken Invader (34511) and Forsaken Footsoldier (36236): attack the nearest Duskhaven Watchman.
struct npc_forsaken_invader : public ScriptedAI
{
    npc_forsaken_invader(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;

    void Reset() override
    {
        events.Reset();
        events.ScheduleEvent(EVENT_SEARCH_WATCHMAN, 2s);
    }

    void DamageTaken(Unit* who, uint32& damage) override
    {
        if (who && who->ToCreature() && who->GetEntry() == NPC_ENTRY_WATCHMAN)
            damage = 0;
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SEARCH_WATCHMAN:
                    if (!me->IsInCombat())
                        if (Creature* watchman = me->FindNearestCreature(NPC_ENTRY_WATCHMAN, 15.5f, true))
                            AttackStart(watchman);
                    events.ScheduleEvent(EVENT_SEARCH_WATCHMAN, randtime(2s, 5s));
                    break;
                default:
                    break;
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

class npc_gilneas_children : public CreatureScript
{
    public:
        npc_gilneas_children(const char* scriptName, uint32 spellId, uint8 playerSayId) : CreatureScript(scriptName), _spellId(spellId), _playerSayId(playerSayId) { }

    private:
        uint32 _spellId;
        uint8 _playerSayId;

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new creature_script(creature, _spellId, _playerSayId);
        }

        struct creature_script : public ScriptedAI
        {
            creature_script(Creature* creature, uint32 spellId, uint8 playerSayId) : ScriptedAI(creature), _spellId(spellId), _playerSayId(playerSayId) { }

            EventMap events;
            ObjectGuid playerGUID;
            uint32 _spellId;
            uint8 _playerSayId;
            bool activated;

            void Reset() override
            {
                events.Reset();
                playerGUID = ObjectGuid::Empty;
                activated = false;
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

                if (me->GetEntry() == NPC_CYNTHIA)
                    events.ScheduleEvent(EVENT_CRY, 1s);
            }

            void SpellHit(Unit* caster, const SpellInfo* spell) override
            {
                if (!activated && spell->Id == _spellId)
                {
                    if (Player* player = caster->ToPlayer())
                    {
                        activated = true;
                        BroadcastText const* bct = sObjectMgr->GetBroadcastText(PlayerText[_playerSayId]);
                        LocaleConstant loc_idx = player->GetSession()->GetSessionDbLocaleIndex();
                        std::string baseText = "";
                        if (bct)
                            baseText = bct->GetText(loc_idx, player->GetGender());
                            player->Say(baseText, LANG_UNIVERSAL);
                        player->KilledMonsterCredit(me->GetEntry(), ObjectGuid::Empty);
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                        events.ScheduleEvent(EVENT_TALK_TO_PLAYER, 3s + 500ms);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TALK_TO_PLAYER:
                        {
                            events.CancelEvent(EVENT_CRY);

                            if (Player* player = ObjectAccessor::GetPlayer(*me, playerGUID))
                                Talk(CHILDREN_TEXT_ID, player);

                            events.ScheduleEvent(EVENT_START_RUN, 5s);
                        }
                        break;
                        case EVENT_START_RUN:
                        {
                            switch (me->GetEntry())
                            {
                                case NPC_JAMES:
                                    me->GetMotionMaster()->MoveSplinePath(JamesPath[0], JamesPathLenght, false, false, 0.f, false, false);
                                    break;
                                case NPC_CYNTHIA:
                                    me->GetMotionMaster()->MoveSplinePath(CynthiaPath[0], CynthiaPathLenght, false, false, 0.f, false, false);
                                    break;
                                case NPC_ASHLEY:
                                    me->GetMotionMaster()->MoveSplinePath(AshleyPath[0], AshleyPathLenght, false, false, 0.f, false, false);
                                    break;
                                default:
                                    return;
                            }

                            events.ScheduleEvent(EVENT_OPEN_DOOR, me->GetSplineDuration());
                        }
                        break;
                        case EVENT_OPEN_DOOR:
                        {
                            if (GameObject* door = me->FindNearestGameObject(GO_DOOR_TO_THE_BASEMENT, 10.0f))
                            {
                                if (door->GetGoState() == GO_STATE_READY)
                                {
                                    door->UseDoorOrButton();
                                    events.ScheduleEvent(EVENT_RESUME_RUN, 2s);
                                }
                                else
                                    events.ScheduleEvent(EVENT_RESUME_RUN, 0ms);
                            }
                        }
                        break;
                        case EVENT_RESUME_RUN:
                            me->GetMotionMaster()->MoveSplinePath(childrenBasementPath[0], childrenBasementPathLenght, false, false, 0.f, false, false);
                            me->DespawnOrUnsummon(me->GetSplineDuration());
                            break;
                        case EVENT_CRY:
                            me->HandleEmoteCommand(EMOTE_ONESHOT_CRY);
                            events.ScheduleEvent(EVENT_CRY, randtime(1s, 1s + 500ms));
                            break;
                    }
                }
            }
        };
};

class npc_wahl : public CreatureScript
{
public:
    npc_wahl(const char* ScriptName) : CreatureScript(ScriptName) { }

    struct npc_wahlAI : public npc_escortAI
    {
        npc_wahlAI(Creature* creature) : npc_escortAI(creature)
        {
            creature->SetReactState(REACT_PASSIVE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            creature->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        }

        // #477: Creature::Update calls JustAppeared() on the first tick after the summon, and npc_escortAI::JustAppeared()
        // resets the escort state (m_uiEscortState = NONE). Lucius used to AddWaypoint()+Start() right after
        // SummonCreature(), so the reset wiped the escort and Wahl stood still (no yell, no swing). Start it here, after the reset.
        void JustAppeared() override
        {
            npc_escortAI::JustAppeared();

            if (!me->IsSummon())
                return;

            AddWaypoint(0, -2106.54f, 2342.69f, 6.93668f);
            AddWaypoint(1, -2106.12f, 2334.90f, 7.36691f);
            AddWaypoint(2, -2117.80f, 2357.15f, 5.88139f);
            AddWaypoint(3, -2111.46f, 2366.22f, 7.17151f);
            Start(false, true);
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_CONTINUE_SCENE)
            {
                me->m_Events.AddLambdaEventAtOffset([this]()
                {
                    SetEscortPaused(false);
                }, 4000);
            }
        }

        void WaypointReached(uint32 point) override
        {
            if (point == 1)
                if (me->IsSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                    {
                        SetEscortPaused(true);
                        me->SetDisplayId(NPC_WAHL_WORGEN);
                        Talk(YELL_DONT_MESS);
                        me->SetReactState(REACT_AGGRESSIVE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

                        // #477: keep the GUID, not the pointer - the summoner can despawn during the 800 ms delay.
                        ObjectGuid summonerGuid = summoner->GetGUID();
                        me->m_Events.AddLambdaEventAtOffset([this, summonerGuid]()
                        {
                            if (Unit* target = ObjectAccessor::GetUnit(*me, summonerGuid))
                                if (target->IsAlive())
                                    AttackStart(target);
                        }, 800);
                    }
        }

        void UpdateAI(uint32 diff) override
        {
            npc_escortAI::UpdateAI(diff);
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_wahlAI(creature);
    }
};

class npc_lucius_the_cruel : public CreatureScript
{
public:
    npc_lucius_the_cruel(const char* ScriptName) : CreatureScript(ScriptName) { }

    struct npc_lucius_the_cruelAI : public ScriptedAI
    {
        npc_lucius_the_cruelAI(Creature* creature) : ScriptedAI(creature)
        {
            SetCombatMovement(false);
            Catch = false;
            Summon = false;
            uiCatchTimer = 1000;
            uiShootTimer = 500;
            uiPlayerGUID = ObjectGuid::Empty;
            uiSummonTimer = 1500;
            me->SetReactState(REACT_PASSIVE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        }

        ObjectGuid uiPlayerGUID;
        uint32 uiCatchTimer;
        uint32 uiShootTimer;
        uint32 uiSummonTimer;
        bool Catch;
        bool Summon;

        void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
        {
            me->DespawnOrUnsummon();
        }

        void JustDied(Unit* /*killer*/) override
        {
            if (Creature* wahl = me->FindNearestCreature(NPC_WAHL, 30.f))
                wahl->AI()->DoAction(ACTION_CONTINUE_SCENE);
        }

        void MovementInform(uint32 type, uint32 id) override
        {
            if (type != POINT_MOTION_TYPE)
                return;

            if (id == POINT_CATCH_CHANCE)
            {
                me->HandleEmoteCommand(EMOTE_ONESHOT_KNEEL);

                if (me->IsSummon())
                    if (Unit* summoner = me->ToTempSummon()->GetSummoner())
                        if (summoner->ToCreature())
                        {
                            // #477: keep Chance's GUID, not the pointer - the cat can be despawned (or respawn) during
                            // the 4 s delay, which left a dangling Creature* here (same fix as Wahl's summoner above).
                            ObjectGuid chanceGuid = summoner->GetGUID();
                            me->m_Events.AddLambdaEventAtOffset([this, chanceGuid]()
                            {
                                Catch = true;
                                Summon = true;
                                if (Creature* chance = ObjectAccessor::GetCreature(*me, chanceGuid))
                                    if (chance->AI())
                                        chance->AI()->DoAction(ACTION_CHANCE_DESPAWN);
                            }, 4000);
                        }
            }
        }

        void DoAction(int32 const action) override
        {
            if (action == ACTION_SUMMON_LUCIUS)
            {
                me->GetMotionMaster()->MovePoint(POINT_CATCH_CHANCE, -2106.372f, 2331.106f, 7.360674f);

                me->m_Events.AddLambdaEventAtOffset([this]()
                {
                    Talk(SAY_THIS_CAT_IS_MINE);
                }, 1000);
            }
        }

        void UpdateAI(uint32 diff) override
        {
            if (Catch)
            {
                if (uiCatchTimer <= diff)
                {
                    Catch = false;
                    uiCatchTimer = 1000;
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                    me->SetReactState(REACT_AGGRESSIVE);

                    if (Player* player = Unit::GetPlayer(*me, uiPlayerGUID))
                        AttackStart(player);
                }
                else
                    uiCatchTimer -= diff;
            }

            if (Summon)
            {
                if (uiSummonTimer <= diff)
                {
                    Summon = false;
                    uiSummonTimer = 1500;

                    // #477: the escort waypoints are added by npc_wahlAI::JustAppeared (after the escort reset).
                    me->SummonCreature(NPC_WAHL, -2098.366f, 2352.075f, 7.160643f);
                }
                else
                    uiSummonTimer -= diff;
            }

            if (!UpdateVictim())
                return;

            if (uiShootTimer <= diff)
            {
                uiShootTimer = 1000;

                if (me->GetDistance(me->GetVictim()) > 2.0f)
                    DoCastVictim(SPELL_LUCIUS_SHOOT);
            }
            else
                uiShootTimer -= diff;

            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_lucius_the_cruelAI(creature);
    }
};

struct npc_chance_the_cat : public ScriptedAI
{
    npc_chance_the_cat(Creature* creature) : ScriptedAI(creature)
    {
        Despawn = false;
        uiDespawnTimer = 500;
    }

    uint32 uiDespawnTimer;
    bool Despawn;

    void DoAction(int32 const action) override
    {
        if (action == ACTION_CHANCE_DESPAWN)
            Despawn = true;
    }

    void SpellHit(Unit* caster, const SpellInfo* spell) override
    {
        if (spell->Id == SPELL_CATCH_CAT && caster->GetTypeId() == TYPEID_PLAYER)
        {
            me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);

            if (Creature* lucius = me->SummonCreature(NPC_LUCIUS_THE_CRUEL, -2111.533f, 2329.95f, 7.390349f))
            {
                lucius->AI()->DoAction(ACTION_SUMMON_LUCIUS);
                CAST_AI(npc_lucius_the_cruel::npc_lucius_the_cruelAI, lucius->AI())->uiPlayerGUID = caster->GetGUID();
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (Despawn)
        {
            if (uiDespawnTimer <= diff)
            {
                uiDespawnTimer = 500;
                Despawn = false;
                me->DespawnOrUnsummon();
            }
            else
                uiDespawnTimer -= diff;
        }
    }
};

class npc_mountain_horse : public CreatureScript
{
public:
    npc_mountain_horse(const char* ScriptName) : CreatureScript(ScriptName) { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_THE_HUNGRY_ETTIN) == QUEST_STATUS_INCOMPLETE)
        {
            player->EnterVehicle(creature, 0);
            creature->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
            creature->GetMotionMaster()->Clear();
            return true;
        }
        return true;
    }

    struct npc_mountain_horseAI : public ScriptedAI
    {
        npc_mountain_horseAI(Creature* creature) : ScriptedAI(creature) { }

        EventMap events;

        void Reset() override
        {
            if (!me->GetVehicleKit()->GetPassenger(0))
                me->GetMotionMaster()->MoveRandom(8.0f);
        }

        void SpellHit(Unit* caster, SpellInfo const* spell) override
        {
            switch (spell->Id)
            {
                case SPELL_ROUND_UP_HORSE:
                {
                    if (me->GetVehicleKit()->GetPassenger(0))
                        break;

                    me->DespawnOrUnsummon(1);
                    break;
                }
                default:
                    break;
            }
        }

        void PassengerBoarded(Unit* /*passenger*/, int8 /*SeatId*/, bool apply) override
        {
            if (!apply)
            {
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetWalk(true);
                me->AI()->EnterEvadeMode();
            }
        }

        void OnCharmed(bool apply) override { }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_mountain_horseAI (creature);
    }
};

struct npc_mountain_horse_summoned : public ScriptedAI
{
    npc_mountain_horse_summoned(Creature* creature) : ScriptedAI(creature) { }

    EventMap events;
    ObjectGuid followTargetGuid;

    Unit* GetFollowTarget()
    {
        Unit* owner = me->GetCharmerOrOwner();
        if (!owner && me->ToTempSummon())
            owner = me->ToTempSummon()->GetSummoner();

        if (owner)
        {
            if (Unit* vehicle = owner->GetVehicleBase())
            {
                if (vehicle != me && vehicle->IsInWorld())
                    return vehicle;
            }
            return owner;
        }
        return nullptr;
    }

    void IsSummonedBy(Unit* summoner) override
    {
        if (!summoner)
            return;

        Unit* followTarget = GetFollowTarget();
        if (!followTarget)
            followTarget = summoner;

        me->GetMotionMaster()->MoveFollow(followTarget, 6.0f, 0);
        followTargetGuid = followTarget->GetGUID();
        me->CastSpell(summoner, SPELL_ROPE_CHANNEL, true);
        me->ClearUnitState(UNIT_STATE_CASTING);
        events.ScheduleEvent(EVENT_CHECK_LORNA, 2s);
        events.ScheduleEvent(EVENT_CHECK_OWNER, 2s);
        me->SetWalk(false);
        me->SetSpeed(MOVE_RUN, 2.0f, true);
    }

    void JustAppeared() override
    {
        ScriptedAI::JustAppeared();

        if (Unit* followTarget = GetFollowTarget())
        {
            if (followTarget->IsInWorld())
            {
                me->GetMotionMaster()->MoveFollow(followTarget, 6.0f, 0);
                followTargetGuid = followTarget->GetGUID();
            }
        }
    }

    void UpdateAI(uint32 diff) override
    {
        events.Update(diff);

        if (Unit* followTarget = GetFollowTarget())
        {
            if (followTarget->IsInWorld() && (followTargetGuid != followTarget->GetGUID() || me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE))
            {
                me->GetMotionMaster()->MoveFollow(followTarget, 6.0f, 0);
                followTargetGuid = followTarget->GetGUID();
            }
        }

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_CHECK_LORNA:
                {
                    if (Creature* lornaCrowley = me->FindNearestCreature(NPC_LORNA_CROWLEY_2, 8.0f, true))
                    {
                        if (Unit* owner = me->GetCharmerOrOwner())
                        {
                            if (owner->GetTypeId() == TYPEID_PLAYER)
                                owner->ToPlayer()->KilledMonsterCredit(QUEST_CREDIT_HORSE);
                        }
                        events.CancelEvent(EVENT_CHECK_LORNA);
                        me->DespawnOrUnsummon(1);
                    }
                    else
                        events.RescheduleEvent(EVENT_CHECK_LORNA, 2s);
                    break;
                }
                case EVENT_CHECK_OWNER:
                {
                    if (Unit* owner = me->GetCharmerOrOwner())
                    {
                        if (!owner->IsAlive() || !owner->IsInWorld())
                            me->DespawnOrUnsummon(1);

                        events.CancelEvent(EVENT_CHECK_OWNER);
                    }
                    else
                        events.RescheduleEvent(EVENT_CHECK_OWNER, 2s);
                    break;
                }
                default:
                    break;
            }
        }
    }
};

class spell_gilneas_test_telescope : public SpellScript
{
    PrepareSpellScript(spell_gilneas_test_telescope);

    void StartCinematic()
    {
        if (Unit* caster = GetCaster())
        {
            if (caster->GetTypeId() == TYPEID_PLAYER)
            {
                caster->PlayDirectSound(MUSIC_ENTRY_TELESCOPE, caster->ToPlayer());
                caster->ToPlayer()->SendCinematicStart(PLAY_CINEMATIC_TELESCOPE);
            }
        }
    }

    void Register() override
    {
        AfterHit += SpellHitFn(spell_gilneas_test_telescope::StartCinematic);
    }
};

class npc_stagecoach_carriage_exodus : public CreatureScript
{
public:
    npc_stagecoach_carriage_exodus(const char* ScriptName) : CreatureScript(ScriptName) { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        if (player->GetQuestStatus(QUEST_ENTRY_EXODUS) == QUEST_STATUS_COMPLETE)
        {
            if (player->GetVehicleBase())
                return true;

            if (Creature* harness = player->FindNearestCreature(NPC_STAGECOACH_HARNESS, 30.0f, true))
                player->SummonCreature(NPC_HARNESS_SUMMONED, harness->GetPositionX(), harness->GetPositionY(), harness->GetPositionZ(), harness->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN, 600000ms);
            return true;
        }

        return true;
    }

    struct npc_stagecoach_carriage_exodusAI : public ScriptedAI
    {
        npc_stagecoach_carriage_exodusAI(Creature* creature) : ScriptedAI(creature)
        {
            events.ScheduleEvent(EVENT_BOARD_HARNESS_OWNER, 1ms);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
        }

        EventMap events;

        void UpdateAI(uint32 diff)
        {
            events.Update(diff);

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_BOARD_HARNESS_OWNER:
                    {
                        if (Unit* harness = me->GetVehicleCreatureBase())
                        {
                            if (Unit* harnessOwner = harness->ToTempSummon()->GetSummoner())
                            {
                                if (harnessOwner->IsAlive() && harnessOwner->IsInWorld())
                                {
                                    harnessOwner->EnterVehicle(me, 1);
                                    events.CancelEvent(EVENT_BOARD_HARNESS_OWNER);
                                    break;
                                }
                            }
                        }
                        events.CancelEvent(EVENT_BOARD_HARNESS_OWNER);
                        break;
                    }
                    default:
                        break;
                }
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_stagecoach_carriage_exodusAI(creature);
    }
};

class npc_stagecoach_harness : public CreatureScript
{
public:
    npc_stagecoach_harness(const char* ScriptName) : CreatureScript(ScriptName) { }

    struct npc_stagecoach_harnessAI : public npc_escortAI
    {
        npc_stagecoach_harnessAI(Creature* creature) : npc_escortAI(creature) { }

        void OnCharmed(bool apply) override { }

        void IsSummonedBy(Unit* owner) override
        {
            DoAction(ACTION_START_WP);
            me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_IMMUNE_TO_PC);
        }

        void DoAction(int32 action) override
        {
            switch (action)
            {
                case ACTION_START_WP:
                {
                    Start(false, true, ObjectGuid::Empty, NULL, false, false, true);
                    SetDespawnAtEnd(true);

                    if (GameObject* gate = me->FindNearestGameObject(GO_FIRST_GATE, 80.0f))
                        gate->UseDoorOrButton(0, false, me);

                    me->SetWalk(false);
                    me->SetSpeed(MOVE_RUN, 1.34f, true);
                    break;
                }
                default:
                    break;
            }
        }

        void WaypointReached(uint32 point) override
        {
            switch (point)
            {
                case 16:
                {
                    if (GameObject* gate = me->FindNearestGameObject(GO_KINGS_GATE, 80.0f))
                        gate->UseDoorOrButton(0, false, me);
                    break;
                }
                case 24:
                {
                    if (Unit* caravan = me->GetVehicleKit()->GetPassenger(2))
                    {
                        if (Unit* lorna = caravan->GetVehicleKit()->GetPassenger(6))
                        {
                            if (lorna->ToCreature())
                                lorna->ToCreature()->AI()->Talk(0);
                        }
                    }
                    break;
                }
                case 30:
                {
                    if (Unit* caravan = me->GetVehicleKit()->GetPassenger(2))
                    {
                        if (Unit* player = caravan->GetVehicleKit()->GetPassenger(1))
                            player->ExitVehicle();
                    }
                    break;
                }
                default:
                    break;
            }
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_stagecoach_harnessAI(creature);
    }
};

struct npc_koroth_the_hillbreaker : public ScriptedAI
{
    npc_koroth_the_hillbreaker(Creature* creature) : ScriptedAI(creature) { }

    EventMap m_events;
    bool m_sceneRunning = false;

    void DoAction(int32 const action) override
    {
        switch (action)
        {
            case ACTION_START_KOROTH_EVENT:
                // #476: the banner calls this on both open (GO_ACTIVATED) and loot release (GO_JUST_DEACTIVATED).
                if (m_sceneRunning)
                    break;
                m_sceneRunning = true;
                Talk(SAY_KOROTH_THE_HILLBREAKER_1);
				me->GetMotionMaster()->MoveSplinePath(KorothPath[0], KorothPathLenght, false, true, 0.f, false, false);
                //TalkWithDelay(me->GetSplineDuration(), SAY_KOROTH_THE_HILLBREAKER_2);
                {
                    uint32 walk = me->GetSplineDuration() ? me->GetSplineDuration() : 4000;
                    m_events.ScheduleEvent(EVENT_KOROTH_THE_HILLBREAKER_2, walk);

                    me->m_Events.AddLambdaEventAtOffset([this]()
                    {
                        me->GetMotionMaster()->MoveTargetedHome();
                        m_sceneRunning = false;
                    }, walk + 3500);
                }
                break;
        }
    }

    void UpdateAI(uint32 const diff) override
    {
        
       m_events.Update(diff);

        while (uint32 eventId = m_events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_KOROTH_THE_HILLBREAKER_2:
                {
                    Talk(SAY_KOROTH_THE_HILLBREAKER_2);
                    break;
                }

            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

class go_koroth_banner : public GameObjectScript
{
public:
    go_koroth_banner(const char* ScriptName) : GameObjectScript(ScriptName) { }

    struct go_koroth_bannerAI : public GameObjectAI
    {
        go_koroth_bannerAI(GameObject* go) : GameObjectAI(go) { }

        void OnStateChanged(uint32 state, Unit* /*unit*/) override
        {
            // #476: MoP 5.4.8 runs the scene on use; the chest is opened by Opening 3365 (GO_ACTIVATED) and only reaches
            // GO_JUST_DEACTIVATED once its loot is fully released, so react to both (Koroth ignores the second call).
            // 80 yd: Koroth (spawn 25 yd from the banner) can stand further out than the old 30 yd search.
            if (state != GO_ACTIVATED && state != GO_JUST_DEACTIVATED)
                return;

            if (Creature* koroth = me->FindNearestCreature(NPC_KOROTH_THE_HILLBREAKER, 80.0f))
            {
                if (koroth->IsAIEnabled)
                {
                    koroth->GetAI()->DoAction(ACTION_START_KOROTH_EVENT);
                }
            }
        }
    };

    GameObjectAI* GetAI(GameObject* go) const override
    {
        return new go_koroth_bannerAI(go);
    }
};

// ============================================================================================================
// #504 - 14375 "Last Chance at Humanity": the pillory scene in Duskhaven.
// Intended (Wowpedia "Last Chance at Humanity", MediaWiki API 2026-10-09): the player wakes locked in a pillory
// before Krennan Aranas, Lord Godfrey and King Genn Greymane, who argue (four lines); after the quest the three
// disappear and the player is released.
// DBC: 68630 Curse of the Worgen = teleport to the pillory + linked aura 68631 (stun, invisibility) + 69123 (summon
// Krennan 36331 with SummonProperties 3054 = personal spawn, + 94053 Fade to Black). The 14375 reward 68639 =
// forcecast 68643 + SCRIPT_EFFECT (value 68630) + teleport to the release point.
// SQL 2026_10_09_12 casts 68630 through spell_area (area 4786, 14222 rewarded, 14375 not rewarded) and binds the
// two scripts below.
// Krennan: port of mopcore npc_trigger_event_first_c3 (the name bound to 36331;
// mopcore gilneas.cpp:2700-2829), same points and
// timings. Differences: his Godfrey is the player's private summon too; King Genn is the static 14375 giver
// (guid 375757, 2.8 yd from the pillory; mopcore summoned a second Genn onto the same spot); the lines go to the
// player only (TEXT_RANGE_PERSONAL); and he leaves with Godfrey once the curse is gone instead of after 300 s.
// ============================================================================================================
namespace GilneasLastChanceAtHumanity
{
    enum
    {
        NPC_LORD_GODFREY_PILLORY        = 36330,
        NPC_KING_GENN_GREYMANE_PILLORY  = 36332,
        SPELL_CURSE_OF_THE_WORGEN       = 68630,
        // #999: the curse's linked 68631 makes the player invisible (type 9, 100), and an invisible player only sees units that
        // detect his invisibility (WorldObject::CanDetectInvisibilityOf; AC has the same rule). 60922 "Generic Quest
        // Invisibility Detection 3" = detect type 9, 100. The static Genn 375757 gets it from creature_addon (SQL 2026_10_29_999).
        SPELL_SEE_QUEST_INVIS_3         = 60922,

        SAY_KRENNAN_NOT_GIVING_UP       = 0,       // BT 36344
        SAY_GODFREY_PROTOCOL            = 0,       // BT 36336
        SAY_GENN_PROTOCOL               = 0,       // BT 36340
        SAY_GENN_DOUBLE_DOSAGE          = 1,       // BT 36341

        POINT_KRENNAN_WAIT              = 1,
        POINT_KRENNAN_PILLORY           = 2,
        POINT_GODFREY_PILLORY           = 1,
    };

    Position const KrennanWaitPos    = { -1819.72f, 2289.23f, 42.28f, 0.0f };
    float const    KrennanWaitFacing = 1.75f;
    Position const KrennanPilloryPos = { -1819.80f, 2290.40f, 42.22f, 0.0f };
    Position const GodfreySpawnPos   = { -1844.29f, 2290.67f, 42.30f, 0.32f };
    Position const GodfreyPilloryPos = { -1822.14f, 2295.40f, 42.10f, 0.0f };
}

struct npc_krennan_aranas_pillory : public ScriptedAI
{
    npc_krennan_aranas_pillory(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        using namespace GilneasLastChanceAtHumanity;

        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _playerGuid = player->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->AddAura(SPELL_SEE_QUEST_INVIS_3, me);   // #999: else the cursed (invisible) player cannot see him

        if (TempSummon* godfrey = me->SummonCreature(NPC_LORD_GODFREY_PILLORY, GodfreySpawnPos, TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, me->GetPrivateObjectOwner()))
        {
            godfrey->SetReactState(REACT_PASSIVE);
            godfrey->AddAura(SPELL_SEE_QUEST_INVIS_3, godfrey);   // #999
            _godfreyGuid = godfrey->GetGUID();
        }

        me->GetMotionMaster()->MovePoint(POINT_KRENNAN_WAIT, KrennanWaitPos);

        // The scene lasts as long as the curse: the 14375 reward (68639) removes 68630, then Krennan and Godfrey go.
        _scheduler.Schedule(2s, [this](TaskContext context)
        {
            Player* cursed = ObjectAccessor::GetPlayer(*me, _playerGuid);
            if (!cursed || !cursed->HasAura(GilneasLastChanceAtHumanity::SPELL_CURSE_OF_THE_WORGEN))
            {
                Leave();
                return;
            }
            context.Repeat(1s);
        });

        _scheduler.Schedule(12500ms, [this](TaskContext /*context*/)
        {
            me->GetMotionMaster()->MovePoint(GilneasLastChanceAtHumanity::POINT_KRENNAN_PILLORY, GilneasLastChanceAtHumanity::KrennanPilloryPos);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace GilneasLastChanceAtHumanity;

        if (type != POINT_MOTION_TYPE)
            return;

        if (pointId == POINT_KRENNAN_WAIT)
            me->SetFacingTo(KrennanWaitFacing);
        else if (pointId == POINT_KRENNAN_PILLORY && !_sceneStarted)
        {
            _sceneStarted = true;
            StartScene();
        }
    }

    void StartScene()
    {
        using namespace GilneasLastChanceAtHumanity;

        SayToPlayer(me, SAY_KRENNAN_NOT_GIVING_UP);
        if (Creature* godfrey = ObjectAccessor::GetCreature(*me, _godfreyGuid))
            godfrey->GetMotionMaster()->MovePoint(POINT_GODFREY_PILLORY, GodfreyPilloryPos);

        _scheduler.Schedule(4s, [this](TaskContext /*context*/)
        {
            if (Creature* godfrey = ObjectAccessor::GetCreature(*me, _godfreyGuid))
            {
                godfrey->SetFacingToObject(me);
                SayToPlayer(godfrey, GilneasLastChanceAtHumanity::SAY_GODFREY_PROTOCOL);
            }
        });
        _scheduler.Schedule(12s, [this](TaskContext /*context*/)
        {
            if (Creature* genn = me->FindNearestCreature(GilneasLastChanceAtHumanity::NPC_KING_GENN_GREYMANE_PILLORY, 20.0f))
                SayToPlayer(genn, GilneasLastChanceAtHumanity::SAY_GENN_PROTOCOL);
        });
        _scheduler.Schedule(20s, [this](TaskContext /*context*/)
        {
            if (Creature* genn = me->FindNearestCreature(GilneasLastChanceAtHumanity::NPC_KING_GENN_GREYMANE_PILLORY, 20.0f))
                SayToPlayer(genn, GilneasLastChanceAtHumanity::SAY_GENN_DOUBLE_DOSAGE);
        });
    }

    void SayToPlayer(Creature* speaker, uint8 group)
    {
        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGuid))
            sCreatureTextMgr->SendChat(speaker, group, player, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_PERSONAL);
    }

    void Leave()
    {
        _scheduler.CancelAll();
        if (Creature* godfrey = ObjectAccessor::GetCreature(*me, _godfreyGuid))
            godfrey->DespawnOrUnsummon();
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    ObjectGuid _playerGuid;
    ObjectGuid _godfreyGuid;
    bool _sceneStarted = false;
};

// 68639 Forcecast Worgen Intro Completion (14375 reward): effect 1 is a SCRIPT_EFFECT whose value is the curse 68630.
// Removing 68630 also removes its linked stun/invisibility 68631 (SPELL_AURA_LINKED), so the player leaves the
// pillory; effect 2 then teleports him to the release point. Without it the player stayed stunned after the quest.
class spell_gilneas_forcecast_worgen_intro_completion : public SpellScript
{
    PrepareSpellScript(spell_gilneas_forcecast_worgen_intro_completion);

    void HandleScript(SpellEffIndex /*effIndex*/)
    {
        if (Unit* target = GetHitUnit())
            target->RemoveAurasDueToSpell(uint32(GetEffectValue()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_gilneas_forcecast_worgen_intro_completion::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
    }
};

// ============================================================================================================
// #483 item 2 - 14395 "Gasping for Breath": the watchman rides the player and is saved on the shore.
// Intended (Wowpedia "Gasping for Breath", MediaWiki API 2026-10-04): pick up a drowning watchman, swim him to
// shore; he thanks you there. the intended script for 36440 is npc_drowning_watchman (SpellHit 68735 -> remove
// spellclick flag, remove 68730 Drowning, EnterVehicle(clicker)),
// and the 4.3.4 reference (CPP gilneas_chapter_2.cpp at_gasping_for_breath / spell 68737) credits on the shore.
// DBC: 68735 "Rescue Drowning Watchman" gives the clicker vehicle id 520 (aura 296) for 180 s.
// The fork's SAI gave the credit 5 s after the click wherever the player was; this AI replaces it
// (SQL 2026_10_04_36 drops the SAI and binds the ScriptName).
// ============================================================================================================
namespace GilneasGaspingForBreath
{
    enum
    {
        NPC_DROWNING_WATCHMAN_CREDIT    = 36450,
        SPELL_RESCUE_DROWNING_WATCHMAN  = 68735,
        SPELL_DROWNING                  = 68730,
        SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
        SAY_WATCHMAN_SAVED_FIRST        = 0,       // groups 0..2, one line each (BT 36418/36417/36419)
        SAY_WATCHMAN_SAVED_LAST         = 2,
    };
}

struct npc_drowning_watchman : public ScriptedAI
{
    npc_drowning_watchman(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _scheduler.CancelAll();
        _carrierGuid.Clear();
        _carriedMs = 0;
        _saved = false;
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        using namespace GilneasGaspingForBreath;

        if (spell->Id != SPELL_RESCUE_DROWNING_WATCHMAN || !caster || caster->GetTypeId() != TYPEID_PLAYER || !_carrierGuid.IsEmpty())
            return;

        _carrierGuid = caster->GetGUID();
        _carriedMs = 0;
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        me->RemoveAurasDueToSpell(SPELL_DROWNING);

        // The clicker's vehicle kit (aura 296 of the same spell) is applied in the same cast: board a moment later.
        _scheduler.Schedule(300ms, [this](TaskContext /*context*/)
        {
            Player* carrier = ObjectAccessor::GetPlayer(*me, _carrierGuid);
            if (!carrier)
            {
                me->DespawnOrUnsummon(1000);
                return;
            }

            if (carrier->GetVehicleKit())
                me->EnterVehicle(carrier);
            else
                me->CastSpell(carrier, SPELL_RIDE_VEHICLE_HARDCODED, true);
        });

        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            CheckShore();
            if (!_saved && !_carrierGuid.IsEmpty())
                context.Repeat(1s);
        });
    }

    void CheckShore()
    {
        using namespace GilneasGaspingForBreath;

        _carriedMs += 1000;
        Player* carrier = ObjectAccessor::GetPlayer(*me, _carrierGuid);

        // Dropped (carrier gone, dismounted, rescue aura expired): back into the water.
        if (!carrier || !carrier->IsAlive() || me->GetVehicleBase() != carrier || !carrier->HasAura(SPELL_RESCUE_DROWNING_WATCHMAN))
        {
            if (_carriedMs < 2000 && carrier && carrier->HasAura(SPELL_RESCUE_DROWNING_WATCHMAN))
                return; // still boarding
            if (carrier)
                carrier->RemoveAurasDueToSpell(SPELL_RESCUE_DROWNING_WATCHMAN);
            if (me->GetVehicleBase())
                me->ExitVehicle();
            _carrierGuid.Clear();
            me->DespawnOrUnsummon(1000);
            return;
        }

        // On the shore: the carrier stands out of the water.
        if (_carriedMs >= 2000 && !carrier->IsInWater())
        {
            _saved = true;
            carrier->KilledMonsterCredit(NPC_DROWNING_WATCHMAN_CREDIT);
            Talk(uint8(urand(SAY_WATCHMAN_SAVED_FIRST, SAY_WATCHMAN_SAVED_LAST)), carrier);
            me->ExitVehicle();
            carrier->RemoveAurasDueToSpell(SPELL_RESCUE_DROWNING_WATCHMAN);
            _carrierGuid.Clear();
            me->DespawnOrUnsummon(4000);
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    ObjectGuid _carrierGuid;
    uint32 _carriedMs = 0;
    bool _saved = false;
};

// ============================================================================================================
// #483 item 5 - 24468 "Stranded at the Marsh": every Crash Survivor is locked in combat with a Swamp Crocolisk;
// killing the crocolisk saves him and he runs back to the crash site.
// Intended (Wowpedia "Stranded at the Marsh", MediaWiki API 2026-10-04: "The survivors are locked in combat with
// Swamp Crocolisks. Kill the crocolisk to save the survivor, who will quickly run back to the crash site").
// Source: MoP 5.4.8 world data, SAI 37067 (on spawn summon 37078 via 69854, invincible at 50 hp; data set by the crocolisk's
// death -> say, run to -2149.4 1775.73 6.31862, despawn 7 s) and 37078 (attack summoner, on death set data).
// Differences: the survivor deals no damage to creatures and cannot drop below 50% to them (mopcore
// gilneas.cpp:4286-4366 does the same), so only players kill crocolisks - the objective counts 37078 kills.
// SQL 2026_10_04_36 binds the ScriptName, adds the texts, and drops the 11 static crocolisks MoP 5.4.8 does not spawn.
// ============================================================================================================
namespace GilneasStrandedAtTheMarsh
{
    enum
    {
        NPC_SWAMP_CROCOLISK     = 37078,
        SAY_SURVIVOR_SAVED      = 0,
    };

    Position const CrashSitePos = { -2149.4f, 1775.73f, 6.31862f, 0.0f };
}

struct npc_crash_survivor : public ScriptedAI
{
    npc_crash_survivor(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        if (_saved)
            return;

        _scheduler.CancelAll();
        _scheduler.Schedule(1s, [this](TaskContext context)
        {
            using namespace GilneasStrandedAtTheMarsh;

            if (_saved)
                return;

            Creature* croc = ObjectAccessor::GetCreature(*me, _crocGuid);
            if (!croc || !croc->IsAlive())
            {
                float x, y, z;
                me->GetNearPoint(me, x, y, z, 3.0f, me->GetOrientation());
                if (TempSummon* summon = me->SummonCreature(NPC_SWAMP_CROCOLISK, x, y, z, me->GetAngle(x, y) + float(M_PI), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 10s))
                {
                    _crocGuid = summon->GetGUID();
                    summon->AI()->AttackStart(me);
                    AttackStart(summon);
                }
            }
            else if (!me->IsInCombat())
                AttackStart(croc);

            context.Repeat(5s);
        });
    }

    void DamageTaken(Unit* attacker, uint32& damage) override
    {
        if (attacker && attacker->GetTypeId() == TYPEID_UNIT && me->HealthBelowPctDamaged(50, damage))
            damage = 0;
    }

    void DamageDealt(Unit* victim, uint32& damage, DamageEffectType /*damageType*/) override
    {
        if (victim && victim->GetTypeId() == TYPEID_UNIT)
            damage = 0;
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        // Keep the staged fight going while the crocolisk lives; it may be busy with a player.
        if (!_saved)
            if (Creature* croc = ObjectAccessor::GetCreature(*me, _crocGuid))
                if (croc->IsAlive())
                    return;

        ScriptedAI::EnterEvadeMode(why);
    }

    void SummonedCreatureDies(Creature* summon, Unit* killer) override
    {
        using namespace GilneasStrandedAtTheMarsh;

        if (summon->GetGUID() != _crocGuid || _saved)
            return;

        _saved = true;
        _scheduler.CancelAll();
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        Talk(SAY_SURVIVOR_SAVED, killer && killer->GetTypeId() == TYPEID_PLAYER ? killer : nullptr);
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(1, CrashSitePos);

        _scheduler.Schedule(7s, [this](TaskContext /*context*/)
        {
            // Static spawn: the DB respawn timer brings him back with a new crocolisk.
            _saved = false;
            _crocGuid.Clear();
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            me->SetReactState(REACT_AGGRESSIVE);
            me->DespawnOrUnsummon();
        });
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    TaskScheduler _scheduler;
    ObjectGuid _crocGuid;
    bool _saved = false;
};

// ============================================================================================================
// #479 - 26706 "Endgame": Tobias signals, the player jumps on a hippogryph and destroys the Horde gunship.
// Intended (Wowhead tooltip API quest=26706, 2026-10-04: "Jump on a Hippogryph when Tobias signals you and destroy
// the Horde Gunship. Requirements: Gunship destroyed"; Wowpedia "Endgame (quest)": timed event, Tobias' waiting
// and go lines, the hippogryph flies the player to the gunship). Wowhead MoP Classic quest page: "Requires level
// 12" - the fork's MinLevel 12 is right and stays.
// Source: MoP 5.4.8 behaviour (no open-source implementation exists):
//   npc_hippogryph_endgame (43747) OnGossipHello: quest 26706 incomplete -> summon 43751 at the
//   hippogryph and cast 46598 on it; npc_hippogryph (43751, escort AI) PassengerBoarded: eject
//   non-quest players, SetCanFly, Start() over script_waypoint 43751 (6 points: Keel Harbor ->
//   -1465.44 3254.44 190.39, above the gunship-deck spawns); WaypointReached 5 -> despawn. MoP 5.4.8 gives no credit
//   (its quest_template Method 0 auto-completes).
// #506 (owner decision 2026-10-09: follow Wowpedia "Endgame (quest)" Notes, i.e. the ShatterCore static-deck
// sequence): the hippogryph now drops the rider on the top deck of a static gunship (GO 800100, SQL 2026_10_09_14)
// instead of crediting the fly-over. Lorna 43566 runs the boarding (npc_lorna_crowley_endgame), killing Korm
// Bonegrind gives the credit through 81779 (summon + ride the wyvern 43713, KILL_CREDIT 43729), and the wyvern
// (npc_wyvern_endgame) flies the rider back to Keel Harbor. Port source: ShatterCore (Mr-Scooty/ShatterCore)
// b2d634ad6c src/server/scripts/EasternKingdoms/Gilneas/gilneas_chapter_4.cpp, cited per block below.
// ============================================================================================================
namespace GilneasEndgame
{
    enum
    {
        QUEST_ENDGAME                   = 26706,
        NPC_ENDGAME_HIPPOGRYPH_FLIGHT   = 43751,
        NPC_TOBIAS_MISTMANTLE_ENDGAME   = 43749,
        NPC_FINALE_CREDIT               = 43729,

        SAY_TOBIAS_WAIT                 = 0,       // random waiting line
        SAY_TOBIAS_GO                   = 1,       // "Hop on a hippogryph, now's our chance!"
        SAY_TOBIAS_GO_2                 = 2,       // "Time to go!"

        DATA_TOBIAS_SIGNAL              = 1,

        POINT_OVER_GUNSHIP              = 1,
        POINT_HIPPOGRYPH_DEPART         = 2,

        // #506 boarding (ShatterCore gilneas_chapter_4.cpp:1474-1497)
        NPC_KORM_BONEGRIND              = 43567,
        NPC_GUNSHIP_GRUNT               = 42141,
        NPC_WORGEN_WARRIOR_SHIP         = 43651,
        NPC_GILNEAN_SHARPSHOOTER        = 43703,
        NPC_GUNSHIP_NAVIGATOR           = 43767,
        SPELL_SUMMON_WYVERN_RIDE        = 81779,   // DBC: summon 43713 (SummonProperties 3038, vehicle) + KILL_CREDIT 43729

        SAY_LORNA_RAFTERS               = 0,       // "Secure the rafters!"
        SAY_LORNA_RAPPEL                = 1,       // "I want two sharpshooters to stay behind..."
        SAY_LORNA_HANDS_UP              = 2,       // "Hands up, greenskins!..."
        SAY_LORNA_DOWNSTAIRS            = 3,       // "The rest of you, follow me downstairs!"
        SAY_LORNA_FURNACE               = 4,       // "We've got the explosives in the furnace room..."
        SAY_LORNA_BIG_ORC               = 5,       // "That's... one big orc."
        SAY_LORNA_WYVERNS               = 6,       // "It's about to blow, jump on the Wyverns!"
        SAY_LORNA_ATTACK                = 7,       // "Attack!" (SQL 2026_10_09_14)
        SAY_NAVIGATOR_DABU              = 0,       // "Dabu?" (Orcish, SQL 2026_10_09_14)

        POINT_WYVERN_LOOP               = 1,
        POINT_WYVERN_ESCAPE             = 2,
    };

    // script_waypoint 43751; element 0 is replaced by the current position (MoveSplineInit::Launch).
    Position const OutboundPath[] =
    {
        { -1306.0f,  2133.04f,  13.08f  },
        { -1234.43f, 2227.21f,  46.465f },
        { -1153.69f, 2390.74f,  79.506f },
        { -1192.23f, 2675.32f, 121.651f },
        { -1371.0f,  3040.16f, 165.304f },
        { -1465.44f, 3254.44f, 190.387f },
    };

    // ShatterCore gilneas_chapter_4.cpp:1529-1534: drop point over the top deck (~13 yd fall onto z 178.6) and the
    // hippogryph's departure.
    Position const HippogryphEjectPos = { -1465.15f, 3256.89f, 191.68f, 2.3074f };
    Position const HippogryphDepartPath[] =
    {
        { -1465.44f, 3254.44f, 190.387f },
        { -1525.01f, 3322.89f, 105.88f },
        { -1566.10f, 3316.15f, 105.88f },
    };

    // ShatterCore gilneas_chapter_4.cpp:1536-1551: wyvern loop around the static ship, then the escape to Keel Harbor.
    // Element 0 of each path is replaced by the current position (MoveSplineInit::Launch), as for OutboundPath.
    Position const WyvernLoopPath[] =
    {
        { -1508.77f, 3168.55f, 113.94f },
        { -1508.77f, 3168.55f, 113.94f },
        { -1528.32f, 3156.63f, 122.52f },
        { -1544.67f, 3169.84f, 123.02f },
        { -1586.64f, 3222.29f, 141.24f },
    };
    Position const WyvernEscapePath[] =
    {
        { -1586.64f, 3222.29f, 141.24f },
        { -1315.28f, 2030.00f,  63.73f },
        { -1336.09f, 2097.85f,  14.60f },
        { -1336.09f, 2097.85f,   5.63f },
    };
    Position const WyvernEjectPos = { -1336.10f, 2097.88f, 13.87f, 1.8684f };

    // ShatterCore gilneas_chapter_4.cpp:1554-1560: rope-bottom marks on the mid ("rafters") level.
    Position const SquadMidLevelPos[] =
    {
        { -1454.60f, 3268.34f, 128.70f, 2.14f },
        { -1459.38f, 3265.42f, 128.49f, 2.14f },
        { -1483.42f, 3250.66f, 128.58f, 2.14f },
        { -1485.68f, 3251.26f, 128.67f, 2.14f },
    };
}

// Tobias Mistmantle 43749 at Keel Harbor: waits ~60 s (waiting lines), signals, keeps the window open ~60 s.
struct npc_tobias_mistmantle_endgame : public ScriptedAI
{
    npc_tobias_mistmantle_endgame(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _signal = false;
        _scheduler.CancelAll();
        ScheduleWait();
    }

    void ScheduleWait()
    {
        using namespace GilneasEndgame;

        _scheduler.Schedule(20s, [this](TaskContext context)
        {
            if (AnyQuestPlayerNear())
                Talk(SAY_TOBIAS_WAIT);
            if (context.GetRepeatCounter() < 2)
                context.Repeat(20s);
            else
                _scheduler.Schedule(1s, [this](TaskContext /*signal*/) { Signal(); });
        });
    }

    void Signal()
    {
        using namespace GilneasEndgame;

        _signal = true;
        Talk(SAY_TOBIAS_GO);
        _scheduler.Schedule(5s, [this](TaskContext /*context*/) { Talk(SAY_TOBIAS_GO_2); });
        _scheduler.Schedule(60s, [this](TaskContext /*context*/)
        {
            _signal = false;
            ScheduleWait();
        });
    }

    bool AnyQuestPlayerNear() const
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 40.0f);
        for (Player* player : players)
            if (player->GetQuestStatus(GilneasEndgame::QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE)
                return true;
        return false;
    }

    uint32 GetData(uint32 type) const override
    {
        return type == GilneasEndgame::DATA_TOBIAS_SIGNAL ? uint32(_signal) : 0;
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    bool _signal = false;
};

// Hippogryph 43747 (gossip): npc_hippogryph_endgame.
class npc_hippogryph_endgame : public CreatureScript
{
public:
    npc_hippogryph_endgame() : CreatureScript("npc_hippogryph_endgame") { }

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        using namespace GilneasEndgame;

        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();

        if (player->GetQuestStatus(QUEST_ENDGAME) != QUEST_STATUS_INCOMPLETE || player->GetVehicle())
            return true;

        if (Creature* tobias = creature->FindNearestCreature(NPC_TOBIAS_MISTMANTLE_ENDGAME, 60.0f))
        {
            if (tobias->IsAIEnabled && !tobias->AI()->GetData(DATA_TOBIAS_SIGNAL))
            {
                tobias->AI()->Talk(SAY_TOBIAS_WAIT, player);
                return true;
            }
        }

        if (TempSummon* flight = player->SummonCreature(NPC_ENDGAME_HIPPOGRYPH_FLIGHT, creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
            player->EnterVehicle(flight);

        return true;
    }
};

// Hippogryph 43751 (vehicle 641): npc_hippogryph; since #506 it drops the rider on the gunship deck.
struct npc_hippogryph : public ScriptedAI
{
    npc_hippogryph(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
    }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }
    // Vehicle 641 seat 7546 has VEHICLE_SEAT_FLAG_CAN_CONTROL: the rider charms the hippogryph (Vehicle.cpp:604) and the
    // default CreatureAI::OnCharmed swaps this AI for PossessedAI (t439 build 3/4 cp9: rider parked on the vehicle).
    void OnCharmed(bool /*apply*/) override { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        using namespace GilneasEndgame;

        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (!apply)
        {
            if (!_landed)
                me->DespawnOrUnsummon(1000);
            return;
        }

        if (who->ToPlayer()->GetQuestStatus(QUEST_ENDGAME) != QUEST_STATUS_INCOMPLETE)
        {
            _scheduler.Schedule(100ms, [this](TaskContext /*context*/)
            {
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                me->DespawnOrUnsummon(1000);
            });
            return;
        }

        _riderGuid = who->GetGUID();
        who->ToPlayer()->SetClientControl(me, false);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_RUN, 3.0f, true);
        me->SetSpeed(MOVE_FLIGHT, 3.0f, true);

        _scheduler.Schedule(1500ms, [this](TaskContext /*context*/)
        {
            me->GetMotionMaster()->MoveSmoothPath(POINT_OVER_GUNSHIP, OutboundPath, sizeof(OutboundPath) / sizeof(OutboundPath[0]), false, true);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace GilneasEndgame;

        if (type != EFFECT_MOTION_TYPE)
            return;

        // #506: drop the rider on the gunship deck and leave (ShatterCore gilneas_chapter_4.cpp:1601-1615). The credit
        // now comes from Korm Bonegrind's death (npc_lorna_crowley_endgame).
        if (pointId == POINT_OVER_GUNSHIP && !_landed)
        {
            _landed = true;
            // Eject and fly off outside this movement generator's finalize, as the #479 landing did.
            _scheduler.Schedule(500ms, [this](TaskContext /*context*/)
            {
                Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid);
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                if (rider)
                    rider->NearTeleportTo(GilneasEndgame::HippogryphEjectPos.GetPositionX(), GilneasEndgame::HippogryphEjectPos.GetPositionY(),
                        GilneasEndgame::HippogryphEjectPos.GetPositionZ(), GilneasEndgame::HippogryphEjectPos.GetOrientation());
            });
            _scheduler.Schedule(1500ms, [this](TaskContext /*context*/)
            {
                me->GetMotionMaster()->MoveSmoothPath(GilneasEndgame::POINT_HIPPOGRYPH_DEPART, GilneasEndgame::HippogryphDepartPath,
                    sizeof(GilneasEndgame::HippogryphDepartPath) / sizeof(GilneasEndgame::HippogryphDepartPath[0]), false, true);
            });
            me->DespawnOrUnsummon(12000);
        }
        else if (pointId == POINT_HIPPOGRYPH_DEPART)
            me->DespawnOrUnsummon(1000);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    ObjectGuid _riderGuid;
    bool _landed = false;
};

// #506 - Lorna Crowley 43566 on the static gunship deck: the boarding timeline. Port of ShatterCore
// npc_gilneas_endgame_controller (gilneas_chapter_4.cpp:1645-1921); offsets from arming (= a 26706 player on the
// deck), Korm Bonegrind 43567 is released at the furnace beat, his death launches the wyverns (81779 = credit +
// ride), the event re-arms 123 s later. Differences: Lorna herself stays out of the fight (her template is
// IMMUNE_TO_NPC here and the timeline lives in her AI, so she must not die); the Navigator answers "Dabu?" after
// "Hands up, greenskins!" (Wowpedia "Endgame (quest)" Notes, MediaWiki API 2026-10-09; not in ShatterCore).
struct npc_lorna_crowley_endgame : public ScriptedAI
{
    enum
    {
        EVENT_ENDGAME_ATTACK = 1,
        EVENT_ENDGAME_RAFTERS,
        EVENT_ENDGAME_RAPPEL,
        EVENT_ENDGAME_DESCEND,
        EVENT_ENDGAME_HANDS_UP,
        EVENT_ENDGAME_DABU,
        EVENT_ENDGAME_DOWNSTAIRS,
        EVENT_ENDGAME_FURNACE,
        EVENT_ENDGAME_BIG_ORC,
        EVENT_ENDGAME_KORM_POLL,
        EVENT_ENDGAME_WYVERN_YELL,
        EVENT_ENDGAME_LAUNCH,
        EVENT_ENDGAME_RESET,
    };

    npc_lorna_crowley_endgame(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        // An evade must not restart a running boarding.
        if (_running)
            return;
        _events.Reset();
        _armTimer = 2000;
        _kormGuid.Clear();
        _abandonedPolls = 0;
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }

    // ShatterCore gilneas_chapter_4.cpp:1667-1757 (UpdateAI).
    void UpdateAI(uint32 diff) override
    {
        using namespace GilneasEndgame;

        if (_armTimer <= diff)
        {
            _armTimer = 2000;
            if (!_running && HasEligiblePlayerNearby(40.0f))
                Arm();
        }
        else
            _armTimer -= diff;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ENDGAME_ATTACK:
                    Talk(SAY_LORNA_ATTACK);
                    EngageDeckWave(178.0f, 190.0f);
                    break;
                case EVENT_ENDGAME_RAFTERS:
                    Talk(SAY_LORNA_RAFTERS);
                    break;
                case EVENT_ENDGAME_RAPPEL:
                    Talk(SAY_LORNA_RAPPEL);
                    break;
                case EVENT_ENDGAME_DESCEND:
                    SquadDescend();
                    break;
                case EVENT_ENDGAME_HANDS_UP:
                    Talk(SAY_LORNA_HANDS_UP);
                    break;
                case EVENT_ENDGAME_DABU:
                    if (Creature* navigator = me->FindNearestCreature(NPC_GUNSHIP_NAVIGATOR, 100.0f))
                        SayToBoarders(navigator, SAY_NAVIGATOR_DABU);
                    break;
                case EVENT_ENDGAME_DOWNSTAIRS:
                    Talk(SAY_LORNA_DOWNSTAIRS);
                    break;
                case EVENT_ENDGAME_FURNACE:
                    SayToBoarders(me, SAY_LORNA_FURNACE);
                    ActivateKorm();
                    _events.ScheduleEvent(EVENT_ENDGAME_KORM_POLL, 2s);
                    break;
                case EVENT_ENDGAME_BIG_ORC:
                    SayToBoarders(me, SAY_LORNA_BIG_ORC);
                    break;
                case EVENT_ENDGAME_KORM_POLL:
                    if (IsKormDead())
                    {
                        _abandonedPolls = 0;
                        _events.ScheduleEvent(EVENT_ENDGAME_WYVERN_YELL, 750ms);
                        _events.ScheduleEvent(EVENT_ENDGAME_LAUNCH, 3200ms);
                        _events.ScheduleEvent(EVENT_ENDGAME_RESET, 123s);
                    }
                    else
                    {
                        if (++_kormPolls % 10 == 0)
                            if (Creature* korm = ObjectAccessor::GetCreature(*me, _kormGuid))
                                LogKorm(korm, "alive");
                        // Nobody left to kill Korm for ~3 minutes: reset (ShatterCore :1733-1742).
                        if (HasEligiblePlayerNearby(250.0f))
                            _abandonedPolls = 0;
                        else if (++_abandonedPolls >= 90)
                        {
                            ResetEvent();
                            break;
                        }
                        _events.ScheduleEvent(EVENT_ENDGAME_KORM_POLL, 2s);
                    }
                    break;
                case EVENT_ENDGAME_WYVERN_YELL:
                    Talk(SAY_LORNA_WYVERNS);
                    break;
                case EVENT_ENDGAME_LAUNCH:
                    LaunchWyverns();
                    break;
                case EVENT_ENDGAME_RESET:
                    ResetEvent();
                    break;
                default:
                    break;
            }
        }
    }

private:
    // ShatterCore gilneas_chapter_4.cpp:1760-1774 (Arm), timeline unchanged; EVENT_ENDGAME_DABU added.
    void Arm()
    {
        _running = true;
        _kormGuid.Clear();
        _events.Reset();

        _events.ScheduleEvent(EVENT_ENDGAME_ATTACK, 2100ms);
        _events.ScheduleEvent(EVENT_ENDGAME_RAFTERS, 63700ms);
        _events.ScheduleEvent(EVENT_ENDGAME_RAPPEL, 79900ms);
        _events.ScheduleEvent(EVENT_ENDGAME_DESCEND, 94000ms);
        _events.ScheduleEvent(EVENT_ENDGAME_HANDS_UP, 115600ms);
        _events.ScheduleEvent(EVENT_ENDGAME_DABU, 119600ms);
        _events.ScheduleEvent(EVENT_ENDGAME_DOWNSTAIRS, 126900ms);
        _events.ScheduleEvent(EVENT_ENDGAME_FURNACE, 172300ms);
        _events.ScheduleEvent(EVENT_ENDGAME_BIG_ORC, 183600ms);
    }

    // Wave 27 (t439 cp9): the "say" lines (40 yd) never reached the boarding player while the yells (300 yd) did. The
    // Navigator sits 72 yd from the top deck, and a player who follows the party down to Korm is ~85 yd below Lorna,
    // who stays on the top deck in this static version (retail: she leads the party down). So the say-type lines go to
    // every 26706 player of the boarding (same 150 yd as LaunchWyverns), each as a personal message.
    void SayToBoarders(Creature* speaker, uint8 group)
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 150.0f);
        for (Player* player : players)
            if (player->GetQuestStatus(GilneasEndgame::QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE)
                sCreatureTextMgr->SendChat(speaker, group, player, CHAT_MSG_ADDON, LANG_ADDON, TEXT_RANGE_PERSONAL);
    }

    bool HasEligiblePlayerNearby(float range) const
    {
        std::list<Player*> players;
        me->GetPlayerListInGrid(players, range);
        for (Player* player : players)
            if (player->IsAlive() && player->GetQuestStatus(GilneasEndgame::QUEST_ENDGAME) == QUEST_STATUS_INCOMPLETE)
                return true;
        return false;
    }

    // ShatterCore gilneas_chapter_4.cpp:1776-1797 (GetSquad, FindNearestGruntInBand).
    void GetSquad(std::list<Creature*>& squad, float radius) const
    {
        me->GetCreatureListWithEntryInGrid(squad, GilneasEndgame::NPC_WORGEN_WARRIOR_SHIP, radius);
        me->GetCreatureListWithEntryInGridAppend(squad, GilneasEndgame::NPC_GILNEAN_SHARPSHOOTER, radius);
    }

    Creature* FindNearestGruntInBand(WorldObject* from, float minZ, float maxZ) const
    {
        Creature* nearest = nullptr;
        std::list<Creature*> grunts;
        me->GetCreatureListWithEntryInGrid(grunts, GilneasEndgame::NPC_GUNSHIP_GRUNT, 120.0f);
        for (Creature* grunt : grunts)
        {
            if (!grunt->IsAlive() || grunt->GetPositionZ() < minZ || grunt->GetPositionZ() > maxZ)
                continue;
            if (!nearest || from->GetExactDist(grunt) < from->GetExactDist(nearest))
                nearest = grunt;
        }
        return nearest;
    }

    // ShatterCore gilneas_chapter_4.cpp:1799-1814 (EngageDeckWave), without Lorna's own AttackStart.
    void EngageDeckWave(float minZ, float maxZ)
    {
        std::list<Creature*> squad;
        GetSquad(squad, 50.0f);
        for (Creature* member : squad)
        {
            if (!member->IsAlive() || member->IsInCombat() || member->GetPositionZ() < minZ || !member->IsAIEnabled)
                continue;
            if (Creature* grunt = FindNearestGruntInBand(member, minZ, maxZ))
                member->AI()->AttackStart(grunt);
        }
    }

    // ShatterCore gilneas_chapter_4.cpp:1816-1837 (SquadDescend): cosmetic rappel of up to four warriors.
    void SquadDescend()
    {
        using namespace GilneasEndgame;

        std::list<Creature*> warriors;
        me->GetCreatureListWithEntryInGrid(warriors, NPC_WORGEN_WARRIOR_SHIP, 60.0f);
        size_t slot = 0;
        for (Creature* warrior : warriors)
        {
            if (slot >= sizeof(SquadMidLevelPos) / sizeof(SquadMidLevelPos[0]))
                break;
            if (!warrior->IsAlive() || warrior->GetPositionZ() < 170.0f)
                continue;

            warrior->AttackStop();
            warrior->CombatStop(true);
            Position const& pos = SquadMidLevelPos[slot];
            warrior->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
            if (Creature* grunt = FindNearestGruntInBand(warrior, 120.0f, 140.0f))
                if (warrior->IsAIEnabled)
                    warrior->AI()->AttackStart(grunt);
            ++slot;
        }
    }

    // ShatterCore gilneas_chapter_4.cpp:1839-1852 (ActivateKorm). His template (unit_flags 768) keeps him
    // IMMUNE_TO_PC|IMMUNE_TO_NPC until this beat; a respawn restores it.
    // Wave 30 (t439 cp9): Korm stayed in the player's attacker list for 3+ min without landing a hit and 18 15-yd
    // Shock Blasts cast 4.4 yd from his spawn missed him, so he had left the furnace room. ShatterCore also clears
    // IMMUNE_TO_NPC, which lets the boarding squad (now standing on the deck, 2026_10_09_14) pull him away and leaves
    // him somewhere on a deck with no navmesh. Only the player is released here: "Korm Bonegrind appears! Finish him
    // off." (Wowpedia "Endgame (quest)" Notes). He keeps IMMUNE_TO_NPC, so only players engage him, and he
    // waits at the furnace for one.
    void ActivateKorm()
    {
        if (Creature* korm = me->FindNearestCreature(GilneasEndgame::NPC_KORM_BONEGRIND, 250.0f))
        {
            _kormGuid = korm->GetGUID();
            korm->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
            korm->SetReactState(REACT_AGGRESSIVE);
            Player* target = korm->SelectNearestPlayer(60.0f);
            if (target && korm->IsAIEnabled)
                korm->AI()->AttackStart(target);
            LogKorm(korm, target ? "released, attacking the nearest player" : "released, no player within 60 yd");
        }
        else
            TC_LOG_ERROR("scripts", "[#506] Endgame: no live Korm Bonegrind 43567 within 250 yd of Lorna at the furnace beat");
    }

    // Trace of where Korm is and what he does. It was an ERROR-level harness diagnostic for #506 (closed); #540 lowered it to DEBUG.
    void LogKorm(Creature* korm, char const* what) const
    {
        Unit* victim = korm->GetVictim();
        Position const& home = korm->GetHomePosition();
        TC_LOG_DEBUG("scripts", "[#506] Endgame Korm %s: pos=%.1f,%.1f,%.1f home=%.1f yd hp=%u/%u flags=0x%X combat=%u evade=%u victim=%s",
            what, korm->GetPositionX(), korm->GetPositionY(), korm->GetPositionZ(), korm->GetExactDist(&home),
            korm->GetHealth(), korm->GetMaxHealth(), korm->GetUInt32Value(UNIT_FIELD_FLAGS), uint32(korm->IsInCombat()),
            uint32(korm->IsInEvadeMode()), victim ? victim->GetName().c_str() : "-");
    }

    // ShatterCore gilneas_chapter_4.cpp:1865-1879 (IsKormDead).
    bool IsKormDead()
    {
        if (_kormGuid.IsEmpty())
        {
            if (Creature* korm = me->FindNearestCreature(GilneasEndgame::NPC_KORM_BONEGRIND, 250.0f))
            {
                _kormGuid = korm->GetGUID();
                return !korm->IsAlive();
            }
            return true; // already killed and not yet respawned
        }

        Creature* korm = ObjectAccessor::GetCreature(*me, _kormGuid);
        return !korm || !korm->IsAlive();
    }

    // ShatterCore gilneas_chapter_4.cpp:1881-1896 (LaunchWyverns): 81779 gives the credit, summons the wyvern and
    // seats the caster.
    void LaunchWyverns()
    {
        using namespace GilneasEndgame;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 150.0f);
        for (Player* player : players)
        {
            if (!player->IsAlive() || player->GetQuestStatus(QUEST_ENDGAME) != QUEST_STATUS_INCOMPLETE)
                continue;

            if (!player->GetVehicle())
                player->CastSpell(player, SPELL_SUMMON_WYVERN_RIDE, true);
            else
                player->KilledMonsterCredit(NPC_FINALE_CREDIT);
        }
    }

    // ShatterCore gilneas_chapter_4.cpp:1898-1914 (ResetEvent).
    void ResetEvent()
    {
        std::list<Creature*> squad;
        GetSquad(squad, 250.0f);
        for (Creature* member : squad)
        {
            if (!member->IsAlive())
                continue;
            member->AttackStop();
            member->CombatStop(true);
            Position const& home = member->GetHomePosition();
            member->NearTeleportTo(home.GetPositionX(), home.GetPositionY(), home.GetPositionZ(), home.GetOrientation());
        }

        _events.Reset();
        _kormGuid.Clear();
        _abandonedPolls = 0;
        _running = false; // grunts and Korm come back through their respawn timers
    }

    EventMap _events;
    bool _running = false;
    uint32 _armTimer = 2000;
    uint32 _abandonedPolls = 0;
    uint32 _kormPolls = 0;
    ObjectGuid _kormGuid;
};

// #1024 - Rope 204428 (the two ropes on the top deck of the static gunship). This is the way down for the player.
// Lorna yells "Everyone else, use the ropes to rappel down", and Wowpedia "Endgame (quest)" Notes (MediaWiki API,
// 2026-10-30) say the boarding party rappels down and the player follows it to the captain's quarters, then down to
// the engine room. In the ship's collision model (Nd_Hordegunship.wmo) the top deck (z 178.6) has no walkable link to
// the rafters (z ~128.5). The rafters reach the furnace room (z 105.4, Korm Bonegrind) by a ramp. The DBC chain
// 81425 -> 81422 "Jump 2" (control vehicle on a nearby rope 43793) + 81992 "Rope 2" has no target conditions and no
// rope-vehicle movement in any source we have. So a click only cast 81422 and the player stayed on the top deck,
// where Korm (released at the furnace beat) never got a player within reach, and Lorna's timeline waited for his death
// indefinitely. Instead, the click moves the player to the rope-bottom mark under that rope, the same marks
// SquadDescend uses for the squad.
class go_gilneas_gunship_rope : public GameObjectScript
{
public:
    go_gilneas_gunship_rope() : GameObjectScript("go_gilneas_gunship_rope") { }

    bool OnGossipHello(Player* player, GameObject* go) override
    {
        using namespace GilneasEndgame;

        if (go->GetMapId() != 654 || go->GetPositionZ() < 170.0f)
            return false; // not a top-deck rope of the static gunship: default use

        if (!player->IsAlive() || player->GetVehicle())
            return true;

        Position const* bottom = nullptr;
        for (Position const& pos : SquadMidLevelPos)
            if (!bottom || go->GetExactDist2dSq(&pos) < go->GetExactDist2dSq(bottom))
                bottom = &pos;

        player->NearTeleportTo(bottom->GetPositionX(), bottom->GetPositionY(), bottom->GetPositionZ() + 0.5f, bottom->GetOrientation());
        return true;
    }
};

// #506 - Wyvern 43713 (vehicle 983, summoned and ridden through 81779): loop off the ship, then the escape to Keel
// Harbor. Port of ShatterCore npc_gilneas_endgame_wyvern (gilneas_chapter_4.cpp:1923-2014), written like the
// hippogryph above: vehicle 983's seat can control (VEHICLE_SEAT_FLAG_CAN_CONTROL), so OnCharmed is kept and client
// control is dropped. ShatterCore's "explosion" casts at the loop end (81928/81927) are left out: in this DBC they
// are "Force Cast Seat 2"/"Seat 2" (a seat change), not an explosion.
struct npc_wyvern_endgame : public ScriptedAI
{
    npc_wyvern_endgame(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
    }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }
    void OnCharmed(bool /*apply*/) override { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        using namespace GilneasEndgame;

        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (!apply)
        {
            if (!_arrived)
                me->DespawnOrUnsummon(2000);
            return;
        }

        if (!_riderGuid.IsEmpty())
            return;

        _riderGuid = who->GetGUID();
        who->ToPlayer()->SetClientControl(me, false);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_RUN, 16.0f / 7.0f, true);
        me->SetSpeed(MOVE_FLIGHT, 16.0f / 7.0f, true);

        _scheduler.Schedule(2s, [this](TaskContext /*context*/)
        {
            me->GetMotionMaster()->MoveSmoothPath(POINT_WYVERN_LOOP, WyvernLoopPath, sizeof(WyvernLoopPath) / sizeof(WyvernLoopPath[0]), false, true);
        });
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace GilneasEndgame;

        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_WYVERN_LOOP)
        {
            me->SetSpeed(MOVE_RUN, 24.0f / 7.0f, true);
            me->SetSpeed(MOVE_FLIGHT, 24.0f / 7.0f, true);
            // Start the next spline outside this movement generator's finalize (as npc_hippogryph does).
            _scheduler.Schedule(100ms, [this](TaskContext /*context*/)
            {
                me->GetMotionMaster()->MoveSmoothPath(GilneasEndgame::POINT_WYVERN_ESCAPE, GilneasEndgame::WyvernEscapePath,
                    sizeof(GilneasEndgame::WyvernEscapePath) / sizeof(GilneasEndgame::WyvernEscapePath[0]), false, true);
            });
        }
        else if (pointId == POINT_WYVERN_ESCAPE && !_arrived)
        {
            _arrived = true;
            _scheduler.Schedule(500ms, [this](TaskContext /*context*/)
            {
                Player* rider = ObjectAccessor::GetPlayer(*me, _riderGuid);
                if (Vehicle* kit = me->GetVehicleKit())
                    kit->RemoveAllPassengers();
                if (rider)
                    rider->NearTeleportTo(GilneasEndgame::WyvernEjectPos.GetPositionX(), GilneasEndgame::WyvernEjectPos.GetPositionY(),
                        GilneasEndgame::WyvernEjectPos.GetPositionZ(), GilneasEndgame::WyvernEjectPos.GetOrientation());
                me->DespawnOrUnsummon(2000);
            });
        }
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);
    }

private:
    TaskScheduler _scheduler;
    ObjectGuid _riderGuid;
    bool _arrived = false;
};

// ============================================================================================================
// #483 item 3 follow-up - Stout Mountain Horse 38765, the scripted rides of 24677 "Flank the Forsaken" (Lord Hewell
// -> Livery Outpost, path 387650) and 14465 "To Greymane Manor" (Duskhaven, path 38765,
// alexkulya 2d4b1928 SAI). Both are summoned by 72772 (SummonProperties 161, vehicle) with the summoner seated.
// t439 run on build 3/4 (cp11): the rider sat on the horse (PROBE vehicle=38765) and the horse never moved, with
// the SmartAI path rows of 2026_10_04_37 in place. Vehicle 542's seat has VEHICLE_SEAT_FLAG_CAN_CONTROL (0x800,
// VehicleSeat.dbc), so the rider charms the horse (Vehicle.cpp:604-605). This AI drives the ride itself, the way the
// working Genn horse in this file does (OnCharmed kept, client control dropped, point-to-point movement), reading
// the two paths from `waypoints` through SmartWaypointMgr. SQL 2026_10_04_42 binds it and drops the SAI rows.
// ============================================================================================================
namespace GilneasStoutMountainHorse
{
    enum
    {
        PATH_DUSKHAVEN_TO_MANOR     = 38765,    // 14465, gates at points 3 and 13
        PATH_HEWELL_TO_LIVERY       = 387650,   // 24677
        GO_DUSKHAVEN_GATE_1         = 196863,
        GO_DUSKHAVEN_GATE_2         = 196864,
        POINT_RIDE_BASE             = 1000,
    };

    Position const LordHewellPos = { -2041.39f, 975.445f, 70.1487f, 0.0f };
}

struct npc_stout_mountain_horse : public ScriptedAI
{
    npc_stout_mountain_horse(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }
    void OnCharmed(bool /*apply*/) override { }

    void PassengerBoarded(Unit* who, int8 /*seatId*/, bool apply) override
    {
        using namespace GilneasStoutMountainHorse;

        if (!who || who->GetTypeId() != TYPEID_PLAYER)
            return;

        if (!apply)
        {
            if (!_arrived)
                me->DespawnOrUnsummon(1000);
            return;
        }

        if (_started)
            return;

        _started = true;
        who->ToPlayer()->SetClientControl(me, false);
        _pathId = me->GetDistance2d(LordHewellPos.GetPositionX(), LordHewellPos.GetPositionY()) < 60.0f ? PATH_HEWELL_TO_LIVERY : PATH_DUSKHAVEN_TO_MANOR;
        _path = sSmartWaypointMgr->GetPath(_pathId);
        if (!_path || _path->empty())
        {
            Finish();
            return;
        }

        _lastPoint = 0;
        for (auto const& node : *_path)
            _lastPoint = std::max(_lastPoint, node.first);

        _point = 1;
        me->SetWalk(false);
        _scheduler.Schedule(1s, [this](TaskContext /*context*/) { MoveNext(); });
    }

    void MoveNext()
    {
        using namespace GilneasStoutMountainHorse;

        _legMs = 0;
        auto itr = _path->find(_point);
        if (itr == _path->end())
        {
            if (++_point > _lastPoint)
                Finish();
            else
                MoveNext();
            return;
        }

        WayPoint const* wp = itr->second;
        me->GetMotionMaster()->MovePoint(POINT_RIDE_BASE + _point, wp->x, wp->y, wp->z, false);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace GilneasStoutMountainHorse;

        if (type != POINT_MOTION_TYPE || !_started || _arrived || pointId != POINT_RIDE_BASE + _point)
            return;

        uint32 delayMs = 1;
        if (_pathId == PATH_DUSKHAVEN_TO_MANOR && (_point == 3 || _point == 13))
        {
            if (GameObject* gate = me->FindNearestGameObject(_point == 3 ? GO_DUSKHAVEN_GATE_1 : GO_DUSKHAVEN_GATE_2, 30.0f))
            {
                gate->SetLootState(GO_READY);
                gate->UseDoorOrButton();
            }
            delayMs = 1000;
        }

        if (++_point > _lastPoint)
        {
            _scheduler.Schedule(std::chrono::milliseconds(delayMs), [this](TaskContext /*context*/) { Finish(); });
            return;
        }

        _scheduler.Schedule(std::chrono::milliseconds(delayMs), [this](TaskContext /*context*/) { MoveNext(); });
    }

    void Finish()
    {
        _arrived = true;
        if (Vehicle* kit = me->GetVehicleKit())
            kit->RemoveAllPassengers();
        me->DespawnOrUnsummon(1500);
    }

    void UpdateAI(uint32 diff) override
    {
        _scheduler.Update(diff);

        // Stall guard: a leg that does not finish in 20 s is re-issued (blocked spline or lost inform).
        if (_started && !_arrived && _path)
        {
            _legMs += diff;
            if (_legMs >= 20000)
                MoveNext();
        }
    }

private:
    TaskScheduler _scheduler;
    WPPath* _path = nullptr;
    uint32 _pathId = 0;
    uint32 _point = 0;
    uint32 _lastPoint = 0;
    uint32 _legMs = 0;
    bool _started = false;
    bool _arrived = false;
};

// ============================================================================================================
// #478 - 24904 "The Battle for Gilneas City".
// Intended (Wowpedia "The Battle for Gilneas City", MediaWiki API 2026-10-04): talk to Krennan Aranas ("Let us take
// back our city!") to start the phased battle; Liam gives his speech, then leads the militia over the bridge into
// the merchant district, on to the military district (abominations; Lorna brings the Emberstone cannons), to the
// jail where Darius Crowley's worgen fight Gorerot (catapults), then to Greymane Court where Genn leaps at
// Sylvanas; at 25% she wails (stun), shoots at Genn, Liam takes the arrow and dies in his father's arms. Players
// near a leader get "Soldier of the Battle for Gilneas City" (72069). Players who accept after the start can ask
// Krennan for a teleport to the army. Wowhead tooltip API (mop-classic, quest=24904): requirement "Battle for
// Gilneas City Complete" (credit 38854, quest_objective 265467 in this fork).
// Sources: mopcore gilneas.cpp:5051-5870 (ScriptNames npc_krennan_aranas,
// npc_prince_liam_greymane_gilneas, npc_lady_sylvanas_gilneas; speech timings, summon point, finale order, spell
// ids 72069/72113/72116/86141/68442) and the MoP 5.4.8 behaviour (Krennan
// OnGossipSelect: 1001 start = summon Liam 38218 at the gate + SetPhaseMask(262144) + start;
// 1003 = teleport the player to Liam while the battle runs; 1002 GM reset).
// Liam's route exists in no data set (no script_waypoint 38218 in mopcore or this fork; mopcore's escort
// had no waypoints), so the march is authored here: checkpoints on static spawn positions of each district
// (merchant district crossbowmen, military district villagers/abominations, the jail, Greymane Court), walked with
// pathfinding; each district is held until its Forsaken are cleared or a time cap runs out, so the event never
// stalls. Credit: every player with the quest within 150 yd of Liam when he dies.
// The fork's world already shows the battle as a static tableau in phase 262144 (73 militia, 73 crossbowmen,
// 44 worgen, 23 abominations, Gorerot, Crowley, Sylvanas, Genn); the event runs over it.
// ============================================================================================================
namespace GilneasBattle
{
    enum
    {
        QUEST_THE_BATTLE_FOR_GILNEAS_CITY   = 24904,

        NPC_PRINCE_LIAM_BATTLE              = 38218,
        NPC_GILNEAN_MILITIA_BATTLE          = 38221,
        NPC_LORNA_CROWLEY_BATTLE            = 38426,
        NPC_GOREROT                         = 38331,
        NPC_DARIUS_CROWLEY_BATTLE           = 38415,
        NPC_SYLVANAS_BATTLE                 = 38469,
        NPC_GENN_BATTLE                     = 38470,
        NPC_BATTLE_CREDIT                   = 38854,
        NPC_FORSAKEN_CROSSBOWMAN            = 38210,
        NPC_VILE_ABOMINATION                = 38420,

        GOSSIP_MENU_KRENNAN_BATTLE          = 11061,    // option 0 "Let us take back our city!" (BT 38484)
        GOSSIP_ACTION_START_BATTLE          = 1001,
        GOSSIP_ACTION_GM_RESET              = 1002,
        GOSSIP_ACTION_JOIN_BATTLE           = 1003,

        SPELL_SOLDIER_OF_GILNEAS            = 72069,
        SPELL_BANSHEE_QUEENS_WAIL           = 72113,
        SPELL_SHOOT_LIAM                    = 72116,
        SPELL_GREYMANE_TRANSFORM_FX         = 86141,
        SPELL_KNEEL                         = 68442,

        PHASEMASK_BATTLE                    = 262144,
        MILITIA_COUNT                       = 30,

        // creature_text
        SAY_LIAM_SPEECH_FIRST               = 0,        // 0..6 speech, 6 = "FOR GILNEAS!!!"
        SAY_LIAM_SPEECH_LAST                = 6,
        SAY_LIAM_ATTACK                     = 7,
        SAY_LIAM_PUSH_BACK                  = 8,
        SAY_LIAM_ABOMINATIONS               = 9,
        SAY_LIAM_LORNA                      = 10,
        SAY_LIAM_CROWLEY_AHEAD              = 11,
        SAY_LIAM_FATHER                     = 12,       // added by SQL 2026_10_04_38 (BT 38369)
        SAY_LIAM_WE_DID_IT                  = 13,       // BT 38371
        SAY_LIAM_TOOK_BACK                  = 14,       // BT 38372
        SAY_LORNA_CANNONS                   = 0,
        SAY_GOREROT_CRUSH                   = 0,
        SAY_CROWLEY_CATAPULTS               = 0,
        SAY_CROWLEY_JOIN_FATHER             = 1,
        SAY_GENN_BLOCK_RETREAT              = 0,
        SAY_GENN_SYLVANAS                   = 1,
        SAY_GENN_LIAM_NO                    = 2,
        SAY_SYLVANAS_ENOUGH                 = 0,
        SAY_SYLVANAS_STUBBORN               = 1,
        SAY_SYLVANAS_WASTE                  = 2,        // added by SQL 2026_10_04_38 (BT 38370)

        ACTION_START_BATTLE                 = 1,
        ACTION_RESET_BATTLE                 = 2,
        ACTION_START_FINALE                 = 3,

        DATA_BATTLE_RUNNING                 = 1,
        GUID_BATTLE_LIAM                    = 1,

        POINT_CHECKPOINT_BASE               = 100,
        POINT_FINALE_SHIELD                 = 200,
    };

    Position const LiamStartPos = { -1408.661f, 1260.017f, 36.51123f, 1.79f };   // mopcore gilneas.cpp:5126
    Position const LornaCannonPos = { -1549.20f, 1285.96f, 10.48f, 3.47f };      // mopcore gilneas.cpp:5236
    Position const SylvanasFleePos = { -1634.634f, 1631.859f, 21.21159f, 0.0f }; // mopcore gilneas.cpp:5838

    struct BattleCheckpoint
    {
        Position Pos;
        int8 LiamSay;            // on arrival, -1 none
        uint32 HoldEntry;        // stay until no living creature of this entry is within HoldRadius
        float HoldRadius;
        uint32 MaxHoldMs;
    };

    BattleCheckpoint const Checkpoints[] =
    {
        { { -1487.21f, 1338.23f, 35.64f, 0.0f }, SAY_LIAM_ATTACK,        NPC_FORSAKEN_CROSSBOWMAN, 30.0f,  45000 }, // over the bridge
        { { -1556.00f, 1327.00f, 35.64f, 0.0f }, SAY_LIAM_PUSH_BACK,     NPC_FORSAKEN_CROSSBOWMAN, 35.0f,  60000 }, // merchant district
        { { -1606.50f, 1309.89f, 18.15f, 0.0f }, SAY_LIAM_ABOMINATIONS,  NPC_VILE_ABOMINATION,     40.0f,  90000 }, // military district
        { { -1664.23f, 1344.06f, 15.22f, 0.0f }, SAY_LIAM_CROWLEY_AHEAD, NPC_VILE_ABOMINATION,     30.0f,  45000 },
        { { -1710.65f, 1409.97f, 21.75f, 0.0f }, -1,                     NPC_GOREROT,              80.0f, 180000 }, // the jail
        { { -1763.86f, 1510.76f, 26.31f, 0.0f }, -1,                     0,                         0.0f,   5000 },
        { { -1732.65f, 1596.46f, 20.57f, 0.0f }, -1,                     0,                         0.0f,   6000 }, // Greymane Court
        { { -1690.00f, 1615.00f, 20.50f, 0.0f }, -1,                     NPC_SYLVANAS_BATTLE,     100.0f, 120000 }, // Sylvanas
    };

    uint8 const CheckpointCount = uint8(sizeof(Checkpoints) / sizeof(Checkpoints[0]));
    uint8 const CheckpointMilitary = 2;
    uint8 const CheckpointJail = 4;
    uint8 const CheckpointCourt = 6;
    uint8 const CheckpointSylvanas = 7;
    // t439 build 6: the march stalled in combat between checkpoints (the leg timer only ran out of combat) and the
    // 25-min cap fired right after the 4th checkpoint. Liam now marches passive, legs are capped in or out of combat,
    // and the cap runs from the march start: worst case 8 legs x 120 s + holds 551 s = ~25 min < 40 min.
    uint32 const LegTimeoutMs = 120000;     // a leg that takes longer is finished by a short teleport (stall guard)
    uint32 const BattleTimeoutMs = 2400000; // 40 min after the march starts: the battle is abandoned without credit
}

// Prince Liam Greymane 38218: the battle leader when summoned by Krennan; a plain fighter as the static spawn.
struct npc_prince_liam_greymane_gilneas : public ScriptedAI
{
    npc_prince_liam_greymane_gilneas(Creature* creature) : ScriptedAI(creature), _summons(creature) { }

    void Reset() override
    {
        if (_started)
            return;
        _leader = me->IsSummon();
    }

    void DoAction(int32 action) override
    {
        using namespace GilneasBattle;

        if (action == ACTION_START_BATTLE && _leader && !_started)
            StartBattle();
        else if (action == ACTION_START_FINALE && _started && !_finale)
            StartFinale();
        else if (action == ACTION_RESET_BATTLE)
            EndBattle(false);
    }

    uint32 GetData(uint32 type) const override
    {
        return type == GilneasBattle::DATA_BATTLE_RUNNING ? uint32(_started && !_ended) : 0;
    }

    void StartBattle()
    {
        using namespace GilneasBattle;

        _started = true;
        _battleMs = 0;
        me->SetReactState(REACT_AGGRESSIVE);
        DoCast(me, SPELL_SOLDIER_OF_GILNEAS, true);

        // The army gathers around Liam during the speech.
        for (uint8 i = 0; i < MILITIA_COUNT; ++i)
        {
            float x, y, z;
            me->GetNearPoint(me, x, y, z, frand(3.0f, 12.0f), me->GetOrientation() + float(M_PI) + frand(-1.5f, 1.5f));
            if (TempSummon* militia = me->SummonCreature(NPC_GILNEAN_MILITIA_BATTLE, x, y, z, me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN))
                militia->SetHomePosition(militia->GetPosition());
        }

        // Speech (mopcore timings), then the march. Players have about a minute to join.
        uint32 delayMs = 5000;
        uint32 const gaps[] = { 10000, 10000, 12000, 10000, 12000, 5500, 1500 };
        for (uint8 line = SAY_LIAM_SPEECH_FIRST; line <= SAY_LIAM_SPEECH_LAST; ++line)
        {
            _scheduler.Schedule(std::chrono::milliseconds(delayMs), [this, line](TaskContext /*context*/) { Talk(line); });
            delayMs += gaps[line - SAY_LIAM_SPEECH_FIRST];
        }

        _scheduler.Schedule(std::chrono::milliseconds(delayMs), [this](TaskContext /*context*/)
        {
            for (ObjectGuid guid : _summons)
                if (Creature* militia = ObjectAccessor::GetCreature(*me, guid))
                    militia->HandleEmoteCommand(EMOTE_ONESHOT_BATTLE_ROAR);
            _marching = true;
            _checkpoint = 0;
            MoveToCheckpoint();
        });

        // Keep the army together and the morale aura up.
        _scheduler.Schedule(3s, [this](TaskContext context)
        {
            KeepArmyTogether();
            if (!me->HasAura(SPELL_SOLDIER_OF_GILNEAS))
                DoCast(me, SPELL_SOLDIER_OF_GILNEAS, true);
            if (!_ended)
                context.Repeat(3s);
        });
    }

    void KeepArmyTogether()
    {
        uint8 index = 0;
        for (ObjectGuid guid : _summons)
        {
            Creature* militia = ObjectAccessor::GetCreature(*me, guid);
            ++index;
            if (!militia || !militia->IsAlive() || militia->IsInCombat())
                continue;

            militia->SetHomePosition(me->GetPosition());
            if (militia->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE || !militia->IsWithinDistInMap(me, 30.0f))
                militia->GetMotionMaster()->MoveFollow(me, 2.0f + float(index % 6) * 1.5f, float(M_PI) / 2.0f + float(index) * float(M_PI) / float(GilneasBattle::MILITIA_COUNT));
        }
    }

    void MoveToCheckpoint(bool newLeg = true)
    {
        using namespace GilneasBattle;

        if (_checkpoint >= CheckpointCount)
            return;

        if (newLeg)
            _legMs = 0;
        _holding = false;
        // Lead the march: do not stop to duel every Forsaken on the way (the militia fight); fight at the checkpoints.
        me->SetReactState(REACT_PASSIVE);
        if (me->IsInCombat())
            me->CombatStop(true);
        me->SetWalk(false);
        me->GetMotionMaster()->MovePoint(POINT_CHECKPOINT_BASE + _checkpoint, Checkpoints[_checkpoint].Pos);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        using namespace GilneasBattle;

        if (type != POINT_MOTION_TYPE || !_marching || _holding)
            return;

        if (pointId == POINT_CHECKPOINT_BASE + _checkpoint)
            _scheduler.Schedule(1ms, [this](TaskContext /*context*/) { ReachedCheckpoint(); });
    }

    void ReachedCheckpoint()
    {
        using namespace GilneasBattle;

        if (_holding || _checkpoint >= CheckpointCount)
            return;

        _holding = true;
        _holdMs = 0;
        BattleCheckpoint const& cp = Checkpoints[_checkpoint];
        me->SetHomePosition(me->GetPosition());
        me->SetReactState(REACT_AGGRESSIVE);

        if (cp.LiamSay >= 0)
            Talk(uint8(cp.LiamSay));

        switch (_checkpoint)
        {
            case CheckpointMilitary:
                // Lorna and the Emberstone villagers roll in the cannons.
                if (TempSummon* lorna = me->SummonCreature(NPC_LORNA_CROWLEY_BATTLE, LornaCannonPos, TEMPSUMMON_TIMED_DESPAWN, 120s))
                {
                    lorna->SetWalk(false);
                    lorna->GetMotionMaster()->MoveFollow(me, 3.0f, float(M_PI) / 2.0f);
                    ObjectGuid lornaGuid = lorna->GetGUID();
                    _scheduler.Schedule(8s, [this, lornaGuid](TaskContext /*context*/)
                    {
                        if (Creature* lorna = ObjectAccessor::GetCreature(*me, lornaGuid))
                            lorna->AI()->Talk(SAY_LORNA_CANNONS);
                    });
                    _scheduler.Schedule(14s, [this](TaskContext /*context*/) { Talk(SAY_LIAM_LORNA); });
                }
                break;
            case CheckpointJail:
                if (Creature* gorerot = me->FindNearestCreature(NPC_GOREROT, 80.0f))
                    gorerot->AI()->Talk(SAY_GOREROT_CRUSH);
                _scheduler.Schedule(4s, [this](TaskContext /*context*/)
                {
                    if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY_BATTLE, 80.0f))
                        crowley->AI()->Talk(SAY_CROWLEY_CATAPULTS);
                });
                break;
            case CheckpointCourt:
                if (Creature* genn = me->FindNearestCreature(NPC_GENN_BATTLE, 100.0f))
                {
                    genn->AI()->Talk(SAY_GENN_BLOCK_RETREAT);
                    ObjectGuid gennGuid = genn->GetGUID();
                    _scheduler.Schedule(4s, [this, gennGuid](TaskContext /*context*/)
                    {
                        Creature* genn = ObjectAccessor::GetCreature(*me, gennGuid);
                        if (!genn)
                            return;
                        genn->AI()->Talk(SAY_GENN_SYLVANAS);
                        genn->CastSpell(genn, SPELL_GREYMANE_TRANSFORM_FX, true);
                        if (Creature* sylvanas = me->FindNearestCreature(NPC_SYLVANAS_BATTLE, 120.0f))
                            genn->AI()->AttackStart(sylvanas);
                    });
                }
                break;
            case CheckpointSylvanas:
                if (Creature* sylvanas = me->FindNearestCreature(NPC_SYLVANAS_BATTLE, 120.0f))
                {
                    sylvanas->AI()->SetGUID(me->GetGUID(), GUID_BATTLE_LIAM);
                    AttackStart(sylvanas);
                    for (ObjectGuid guid : _summons)
                        if (Creature* militia = ObjectAccessor::GetCreature(*me, guid))
                            if (militia->IsAlive() && militia->GetEntry() == NPC_GILNEAN_MILITIA_BATTLE)
                                militia->AI()->AttackStart(sylvanas);
                }
                break;
            default:
                break;
        }
    }

    bool HoldOver() const
    {
        using namespace GilneasBattle;

        BattleCheckpoint const& cp = Checkpoints[_checkpoint];
        if (_holdMs >= cp.MaxHoldMs)
            return true;
        if (_holdMs < 3000)
            return false;
        if (!cp.HoldEntry)
            return true;
        return me->FindNearestCreature(cp.HoldEntry, cp.HoldRadius, true) == nullptr;
    }

    void AdvanceCheckpoint()
    {
        using namespace GilneasBattle;

        if (_checkpoint == CheckpointJail)
            if (Creature* crowley = me->FindNearestCreature(NPC_DARIUS_CROWLEY_BATTLE, 80.0f))
                crowley->AI()->Talk(SAY_CROWLEY_JOIN_FATHER);

        if (_checkpoint == CheckpointSylvanas)
        {
            // Time cap at Sylvanas: she breaks off the fight herself.
            StartFinale();
            return;
        }

        ++_checkpoint;
        MoveToCheckpoint();
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (!_started || _ended)
        {
            ScriptedAI::EnterEvadeMode(why);
            return;
        }

        // Back to the march instead of the summon point.
        me->CombatStop(true);
        if (_finale)
            return;
        if (_marching && !_holding)
            MoveToCheckpoint(false);  // resume the same leg; its stall timer keeps running
        else
            me->GetMotionMaster()->MoveIdle();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        // The leader cannot fall before the end of the battle.
        if (_leader && !_allowDeath && me->HealthBelowPctDamaged(30, damage))
            damage = 0;
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        if (spell->Id == GilneasBattle::SPELL_SHOOT_LIAM && _finale)
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
    }

    void StartFinale()
    {
        using namespace GilneasBattle;

        if (_finale)
            return;

        _finale = true;
        _marching = false;
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);

        Creature* sylvanas = me->FindNearestCreature(NPC_SYLVANAS_BATTLE, 150.0f);
        Creature* genn = me->FindNearestCreature(NPC_GENN_BATTLE, 150.0f);
        _sylvanasGuid = sylvanas ? sylvanas->GetGUID() : ObjectGuid::Empty;
        _gennGuid = genn ? genn->GetGUID() : ObjectGuid::Empty;

        for (ObjectGuid guid : _summons)
            if (Creature* militia = ObjectAccessor::GetCreature(*me, guid))
            {
                militia->CombatStop(true);
                militia->SetReactState(REACT_PASSIVE);
            }

        if (sylvanas)
        {
            sylvanas->AI()->DoAction(ACTION_START_FINALE);  // "Enough!", Banshee Queen's Wail, untargetable
            if (genn)
                genn->CombatStop(true);
        }

        _scheduler.Schedule(3s, [this](TaskContext /*context*/)
        {
            if (Creature* sylvanas = ObjectAccessor::GetCreature(*me, _sylvanasGuid))
                sylvanas->AI()->Talk(SAY_SYLVANAS_STUBBORN);
        });
        _scheduler.Schedule(5s, [this](TaskContext /*context*/)
        {
            // Liam throws himself in front of his father.
            Talk(SAY_LIAM_FATHER);
            Creature* genn = ObjectAccessor::GetCreature(*me, _gennGuid);
            Creature* sylvanas = ObjectAccessor::GetCreature(*me, _sylvanasGuid);
            if (genn && sylvanas)
            {
                float x = (genn->GetPositionX() + sylvanas->GetPositionX()) / 2.0f;
                float y = (genn->GetPositionY() + sylvanas->GetPositionY()) / 2.0f;
                me->GetMotionMaster()->MovePoint(POINT_FINALE_SHIELD, x, y, genn->GetPositionZ());
            }
        });
        _scheduler.Schedule(8s, [this](TaskContext /*context*/)
        {
            if (Creature* sylvanas = ObjectAccessor::GetCreature(*me, _sylvanasGuid))
            {
                sylvanas->SetFacingToObject(me);
                sylvanas->CastSpell(me, SPELL_SHOOT_LIAM, true);
            }
            me->SetStandState(UNIT_STAND_STATE_KNEEL);
        });
        _scheduler.Schedule(10s, [this](TaskContext /*context*/)
        {
            if (Creature* sylvanas = ObjectAccessor::GetCreature(*me, _sylvanasGuid))
            {
                sylvanas->AI()->Talk(SAY_SYLVANAS_WASTE);
                sylvanas->GetMotionMaster()->MovePoint(0, SylvanasFleePos);
                sylvanas->DespawnOrUnsummon(6000);  // static spawn: back on the DB respawn timer
            }
            if (Creature* genn = ObjectAccessor::GetCreature(*me, _gennGuid))
                genn->AI()->Talk(SAY_GENN_LIAM_NO);
        });
        _scheduler.Schedule(13s, [this](TaskContext /*context*/)
        {
            if (Creature* genn = ObjectAccessor::GetCreature(*me, _gennGuid))
            {
                genn->RemoveAurasDueToSpell(SPELL_GREYMANE_TRANSFORM_FX);
                genn->GetMotionMaster()->MovePoint(0, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());
            }
        });
        _scheduler.Schedule(17s, [this](TaskContext /*context*/)
        {
            if (Creature* genn = ObjectAccessor::GetCreature(*me, _gennGuid))
            {
                genn->SetFacingToObject(me);
                genn->CastSpell(genn, SPELL_KNEEL, true);
            }
            Talk(SAY_LIAM_WE_DID_IT);
        });
        _scheduler.Schedule(22s, [this](TaskContext /*context*/) { Talk(SAY_LIAM_TOOK_BACK); });
        _scheduler.Schedule(27s, [this](TaskContext /*context*/)
        {
            EndBattle(true);
            _allowDeath = true;
            me->Kill(me);
        });
        _scheduler.Schedule(60s, [this](TaskContext /*context*/)
        {
            if (Creature* genn = ObjectAccessor::GetCreature(*me, _gennGuid))
            {
                genn->RemoveAurasDueToSpell(SPELL_KNEEL);
                genn->AI()->EnterEvadeMode();
            }
        });
    }

    void EndBattle(bool victory)
    {
        using namespace GilneasBattle;

        if (_ended)
            return;
        _ended = true;
        _marching = false;

        if (victory)
        {
            std::list<Player*> players;
            me->GetPlayerListInGrid(players, 150.0f);
            for (Player* player : players)
                if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
                    player->KilledMonsterCredit(NPC_BATTLE_CREDIT);
        }

        _summons.DespawnAll(victory ? 5000 : 0);
        me->DespawnOrUnsummon(victory ? 60000 : 0);
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);
    }

    void SummonedCreatureDespawn(Creature* summon) override
    {
        _summons.Despawn(summon);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (_leader && !_ended)
            EndBattle(false);
    }

    void UpdateAI(uint32 diff) override
    {
        using namespace GilneasBattle;

        _scheduler.Update(diff);

        if (_started && !_ended)
        {
            if (_marching || _finale)
                _battleMs += diff;
            if (_battleMs >= BattleTimeoutMs)
            {
                EndBattle(false);
                return;
            }

            if (_marching && !_finale)
            {
                if (_holding)
                {
                    _holdMs += diff;
                    if (HoldOver())
                        AdvanceCheckpoint();
                }
                else
                {
                    _legMs += diff;
                    if (_legMs >= LegTimeoutMs)
                    {
                        // Stall guard: path blocked or kept in combat for too long.
                        me->CombatStop(true);
                        Position const& pos = Checkpoints[_checkpoint].Pos;
                        me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), me->GetOrientation());
                        _scheduler.Schedule(1s, [this](TaskContext /*context*/) { ReachedCheckpoint(); });
                        _legMs = 0;
                    }
                }
            }
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    TaskScheduler _scheduler;
    SummonList _summons;
    ObjectGuid _sylvanasGuid;
    ObjectGuid _gennGuid;
    uint32 _battleMs = 0;
    uint32 _legMs = 0;
    uint32 _holdMs = 0;
    uint8 _checkpoint = 0;
    bool _leader = false;
    bool _started = false;
    bool _marching = false;
    bool _holding = false;
    bool _finale = false;
    bool _ended = false;
    bool _allowDeath = false;
};

// Lady Sylvanas Windrunner 38469 (static, Greymane Court): cannot be killed; at 25% (or on Liam's time cap) she
// wails and the finale runs - only while the battle Liam is near, otherwise she just stays above 25%.
struct npc_lady_sylvanas_gilneas : public ScriptedAI
{
    npc_lady_sylvanas_gilneas(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        _finale = false;
        _requestFinale = false;
        _liamGuid.Clear();
        me->SetReactState(REACT_AGGRESSIVE);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
    }

    void SetGUID(ObjectGuid guid, int32 id) override
    {
        if (id == GilneasBattle::GUID_BATTLE_LIAM)
        {
            _liamGuid = guid;
            // The fork template has UNIT_FLAG_IMMUNE_TO_PC (MoP 5.4.8: 0); players join the fight once Liam arrives.
            // The template flags come back with her respawn after the finale despawn.
            me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        }
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        using namespace GilneasBattle;

        if (_finale)
        {
            damage = 0;
            return;
        }

        if (!me->HealthBelowPctDamaged(25, damage))
            return;

        damage = 0;
        _requestFinale = true;  // handled in UpdateAI, not inside the damage callback
    }

    void DoAction(int32 action) override
    {
        using namespace GilneasBattle;

        if (action != ACTION_START_FINALE || _finale)
            return;

        _finale = true;
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
        Talk(SAY_SYLVANAS_ENOUGH);
        DoCastAOE(SPELL_BANSHEE_QUEENS_WAIL, true);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (_requestFinale && !_finale)
        {
            _requestFinale = false;
            if (Creature* liam = ObjectAccessor::GetCreature(*me, _liamGuid))
                if (liam->IsAlive() && liam->IsAIEnabled)
                    liam->AI()->DoAction(GilneasBattle::ACTION_START_FINALE);
        }

        if (_finale || !UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    ObjectGuid _liamGuid;
    bool _finale = false;
    bool _requestFinale = false;
};

// Krennan Aranas 38553 (Livery Outpost): starts the battle, or sends late players to the army.
class npc_krennan_aranas_battle : public CreatureScript
{
public:
    npc_krennan_aranas_battle() : CreatureScript("npc_krennan_aranas") { }

    struct npc_krennan_aranas_battleAI : public ScriptedAI
    {
        npc_krennan_aranas_battleAI(Creature* creature) : ScriptedAI(creature) { }

        Creature* GetBattleLiam() const
        {
            Creature* liam = ObjectAccessor::GetCreature(*me, _liamGuid);
            if (liam && liam->IsAlive() && liam->IsAIEnabled && liam->AI()->GetData(GilneasBattle::DATA_BATTLE_RUNNING))
                return liam;
            return nullptr;
        }

        uint32 GetData(uint32 type) const override
        {
            return type == GilneasBattle::DATA_BATTLE_RUNNING ? uint32(GetBattleLiam() != nullptr) : 0;
        }

        ObjectGuid GetGUID(int32 id) const override
        {
            if (id == GilneasBattle::GUID_BATTLE_LIAM)
                if (Creature* liam = GetBattleLiam())
                    return liam->GetGUID();
            return ObjectGuid::Empty;
        }

        void DoAction(int32 action) override
        {
            using namespace GilneasBattle;

            if (action == ACTION_START_BATTLE)
            {
                if (GetBattleLiam())
                    return;
                if (TempSummon* liam = me->SummonCreature(NPC_PRINCE_LIAM_BATTLE, LiamStartPos, TEMPSUMMON_MANUAL_DESPAWN))
                {
                    liam->SetPhaseMask(PHASEMASK_BATTLE, true);
                    _liamGuid = liam->GetGUID();
                    liam->AI()->DoAction(ACTION_START_BATTLE);
                }
            }
            else if (action == ACTION_RESET_BATTLE)
            {
                if (Creature* liam = ObjectAccessor::GetCreature(*me, _liamGuid))
                    liam->AI()->DoAction(ACTION_RESET_BATTLE);
                _liamGuid.Clear();
            }
        }

    private:
        ObjectGuid _liamGuid;
    };

    bool OnGossipHello(Player* player, Creature* creature) override
    {
        using namespace GilneasBattle;

        player->PlayerTalkClass->ClearMenus();
        if (creature->IsQuestGiver())
            player->PrepareQuestMenu(creature->GetGUID());

        if (player->GetQuestStatus(QUEST_THE_BATTLE_FOR_GILNEAS_CITY) == QUEST_STATUS_INCOMPLETE)
        {
            bool running = creature->AI()->GetData(DATA_BATTLE_RUNNING) != 0;
            player->ADD_GOSSIP_ITEM_DB(GOSSIP_MENU_KRENNAN_BATTLE, 0, GOSSIP_SENDER_MAIN, running ? GOSSIP_ACTION_JOIN_BATTLE : GOSSIP_ACTION_START_BATTLE);
        }

        if (player->IsGameMaster())
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, "(GM) Reset the Battle for Gilneas City", GOSSIP_SENDER_MAIN, GOSSIP_ACTION_GM_RESET);

        player->SEND_GOSSIP_MENU(player->GetGossipTextId(creature), creature->GetGUID());
        return true;
    }

    bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
    {
        using namespace GilneasBattle;

        player->PlayerTalkClass->ClearMenus();
        player->CLOSE_GOSSIP_MENU();

        switch (action)
        {
            case GOSSIP_ACTION_START_BATTLE:
                creature->AI()->DoAction(ACTION_START_BATTLE);
                break;
            case GOSSIP_ACTION_JOIN_BATTLE:
                if (Creature* liam = ObjectAccessor::GetCreature(*creature, creature->AI()->GetGUID(GUID_BATTLE_LIAM)))
                    player->NearTeleportTo(liam->GetPositionX(), liam->GetPositionY(), liam->GetPositionZ(), liam->GetOrientation());
                break;
            case GOSSIP_ACTION_GM_RESET:
                if (player->IsGameMaster())
                    creature->AI()->DoAction(ACTION_RESET_BATTLE);
                break;
            default:
                break;
        }
        return true;
    }

    CreatureAI* GetAI(Creature* creature) const override
    {
        return new npc_krennan_aranas_battleAI(creature);
    }
};

void AddSC_gilneas()
{
    new creature_script<npc_gilneas_crow>("npc_gilneas_crow");
    new creature_script<npc_gilneas_city_guard_gate>("npc_gilneas_city_guard_gate");
    new creature_script<npc_prince_liam_greymane>("npc_prince_liam_greymane");
    new creature_script<npc_worgen_runt>("npc_worgen_runt");
    new spell_script<spell_gen_gilneas_prison_periodic_dummy>("spell_gen_gilneas_prison_periodic_dummy");
    new creature_script<npc_josiah_avery>("npc_josiah_avery");
    new creature_script<npc_josiah_avery_worgen_form>("npc_josiah_avery_worgen_form");
    new spell_script<spell_gilneas_pull_to>("spell_gilneas_pull_to");
    new npc_lorna_crowley_basement("npc_lorna_crowley_basement");
    new creature_script<npc_gilnean_mastiff>("npc_gilnean_mastiff");
    new creature_script<npc_bloodfang_lurker>("npc_bloodfang_lurker");
    new spell_script<spell_attack_lurker>("spell_attack_lurker");
    new npc_king_genn_greymane();
    new npc_vehicle_genn_horse();
    new creature_script<npc_saved_aranas>("npc_saved_aranas");
    new creature_script<npc_prince_liam_greymane_phase2>("npc_prince_liam_greymane_phase2");
    new creature_script<npc_duskhaven_watchman>("npc_duskhaven_watchman");
    new creature_script<npc_forsaken_invader>("npc_forsaken_invader");
    new creature_script<npc_forsaken_invader>("npc_forsaken_footsoldier");
    new npc_gilneas_children("npc_james", SPELL_SAVE_JAMES, PLAYER_SAY_JAMES);
    new npc_gilneas_children("npc_ashley", SPELL_SAVE_ASHLEY, PLAYER_SAY_ASHLEY);
    new npc_gilneas_children("npc_cynthia", SPELL_SAVE_CYNTHIA, PLAYER_SAY_CYNTHIA);
    new npc_wahl("npc_wahl");
    new npc_lucius_the_cruel("npc_lucius_the_cruel");
    new creature_script<npc_chance_the_cat>("npc_chance_the_cat");
    new npc_mountain_horse("npc_mountain_horse");
    new creature_script<npc_mountain_horse_summoned>("npc_mountain_horse_summoned");
    new spell_script<spell_gilneas_test_telescope>("spell_gilneas_test_telescope");
    new npc_stagecoach_carriage_exodus("npc_stagecoach_carriage_exodus");
    new npc_stagecoach_harness("npc_stagecoach_harness");
    new creature_script<npc_koroth_the_hillbreaker>("npc_koroth_the_hillbreaker");
    new go_koroth_banner("go_koroth_banner");
    new creature_script<npc_krennan_aranas_pillory>("npc_krennan_aranas_pillory");
    new spell_script<spell_gilneas_forcecast_worgen_intro_completion>("spell_gilneas_forcecast_worgen_intro_completion");
    new creature_script<npc_drowning_watchman>("npc_drowning_watchman");
    new creature_script<npc_crash_survivor>("npc_crash_survivor");
    new creature_script<npc_tobias_mistmantle_endgame>("npc_tobias_mistmantle_endgame");
    new npc_hippogryph_endgame();
    new creature_script<npc_hippogryph>("npc_hippogryph");
    new creature_script<npc_lorna_crowley_endgame>("npc_lorna_crowley_endgame");
    new go_gilneas_gunship_rope();
    new creature_script<npc_wyvern_endgame>("npc_wyvern_endgame");
    new creature_script<npc_stout_mountain_horse>("npc_stout_mountain_horse");
    new npc_krennan_aranas_battle();
    new creature_script<npc_prince_liam_greymane_gilneas>("npc_prince_liam_greymane_gilneas");
    new creature_script<npc_lady_sylvanas_gilneas>("npc_lady_sylvanas_gilneas");
}
