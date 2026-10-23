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

#include "ScriptPCH.h"
#include "deadmines.h"
#include "SpellAuras.h"
#include "SpellMgr.h"
#include "Vehicle.h"

#define MAX_ENCOUNTER 6

static std::vector<DoorData> const doordata =
{
    {GO_FACTORY_DOOR,   DATA_GLUBTOK,   DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_MAST_ROOM_DOOR, DATA_HELIX,     DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_HEAVY_DOOR,     DATA_HELIX,     DOOR_TYPE_ROOM,       BOUNDARY_NONE},
    {GO_FOUNDRY_DOOR,   DATA_FOEREAPER, DOOR_TYPE_PASSAGE,    BOUNDARY_NONE},
    {GO_HEAVY_DOOR_2,   DATA_FOEREAPER, DOOR_TYPE_ROOM,       BOUNDARY_NONE},
};

// =====================================================================================================================================
// #826 Vanessa's Nightmare Gauntlet (heroic). Port of The-Cataclysm-Preservation-Project TrinityCore 4.3.4 (e503f96, 2025-08-02):
// instance_deadmines.cpp:496-884 (the stage machine), deadmines.cpp:123-523 (trap bunny, steam valves, Vanessa's nightmare talk,
// Helix's nightmare) and the positions of its deadmines.h:194-399. Intended design: Wowpedia "Vanessa VanCleef (tactics)"
// (MediaWiki API, 2026-10-23): trapped over the slag (steam valves) -> Glubtok's nightmare (fire on the floor, falling icicles) ->
// Helix's nightmare (spiders) -> the mechanical nightmare (rotating spark chains) -> Ripsnarl's nightmare (save Emme, Erik and
// Calissa Harrington, kill James) -> Vanessa. The CPP's world DB (SmartAI, vehicle accessories) is not available, so this file
// drives every creature itself:
//   - the Lightning Platters' spark chains are placed on the 5.4.8 VehicleSeat.dbc offsets of vehicle 1397 (seats at +-2/4/6/8 yd,
//     z -1) and turned with the platter (0.4 rad/s, CPP MoveCirclePath radius 1 at 0.4 yd/s);
//   - the nightmare Vanessa / announcer lines are spoken through the creatures' own AI (Talk works without a script);
//   - the Collapsing Icicle (92189 summons it, its 92201 lands Ice Shards 92203 1.7 s later, 7 yd) is summoned at the player and
//     given 92201 directly.
// The trap bunny is a real vehicle (1396, five seats); the players are moved next to it first, then seated with 92379 Ride Magma
// Vehicle (BoardTrap below). Every stage change is logged as "[DM-NIGHTMARE] ..." (Logger.scripts) for the 1:1 driver.
// =====================================================================================================================================
enum NightmareEvents
{
    EVENT_NM_BOARD_TRAP = 1,
    EVENT_NM_SUMMON_VALVES,
    EVENT_NM_TRAP_MOVE_DOWN,
    EVENT_NM_TRAP_MAGMA,
    EVENT_NM_EJECT,
    EVENT_NM_GLUBTOK_AURA,
    EVENT_NM_GLUBTOK_SETUP,
    EVENT_NM_GLUBTOK_SPAWN,
    EVENT_NM_ICICLE,
    EVENT_NM_HELIX_AURA,
    EVENT_NM_HELIX_DESPAWN_FIRE,
    EVENT_NM_HELIX_SETUP,
    EVENT_NM_HELIX_UNFLAG,
    EVENT_NM_HELIX_SPIDERS,
    EVENT_NM_HELIX_SPIRIT_STRIKE,
    EVENT_NM_REAPER_DESPAWN_SPIDERS,
    EVENT_NM_REAPER_SETUP,
    EVENT_NM_REAPER_SPAWN,
    EVENT_NM_RIPSNARL_AURA,
    EVENT_NM_RIPSNARL_SETUP,
    EVENT_NM_EMME,
    EVENT_NM_ERIK,
    EVENT_NM_JAMES,
    EVENT_NM_VANESSA_BOSS,
    EVENT_NM_CHECK_DEAD_PLAYERS,
    EVENT_NM_RESPAWN_NOTE,
    EVENT_NM_TRAP_SEATED,   // #826 w106: count the riders a second after the boarding, seat the missing ones once more
};

enum NightmareSpells
{
    SPELL_ROPE_BEAM                     = 43785,
    SPELL_RIDE_MAGMA_VEHICLE            = 92379,   // CPP 92378 makes every player cast it: CONTROL_VEHICLE, STACK_FOR_DIFF_CASTERS
    SPELL_MOLTEN_MAGMA                  = 92424,   // trap bunny at the bottom: 92425, 10 yd, 18500 Fire a second
    SPELL_NIGHTMARE_SLOW                = 92559,   // -75% speed, 10 s, every player (radius index 28)
    SPELL_NIGHTMARE_AURA                = 92563,
    SPELL_NIGHTMARE_ELIXIR              = 92113,
    SPELL_VANESSAS_BLACKOUT_AURA        = 92120,
    SPELL_NIGHTMARE_AURA_GLUBTOK        = 92565,
    SPELL_NIGHTMARE_AURA_HELIX          = 92566,
    SPELL_NIGHTMARE_AURA_MECHANICAL     = 92567,
    SPELL_NIGHTMARE_AURA_RIPSNARL       = 92568,
    SPELL_ICICLE                        = 92201,   // on the Collapsing Icicle: 1.7 s, then Ice Shards 92203 (7 yd, 47500 Frost)
    SPELL_SPIRIT_STRIKE                 = 59304,   // Helix's nightmare (CPP deadmines.cpp:487-489, every 3.6 s)
    SPELL_ADRENALINE                    = 92604,   // a saved Harrington: +50% speed, 60 s
    SPELL_NIGHTMARES_END                = 92603,
};

enum NightmareTexts
{
    // General Purpose Bunny JMF 45979 (creature_text groups 3-9; CPP instance_deadmines.cpp:66-77)
    SAY_NM_ELIXIR_TAKES_HOLD    = 4,
    SAY_NM_NIGHTMARE_SHIFTS     = 5,
    SAY_NM_SAVE_EMME            = 6,
    SAY_NM_SAVE_ERIK            = 7,
    SAY_NM_SAVE_CALISSA         = 8,
    SAY_NM_ELIXIR_WEARS_OFF     = 9,
    // Vanessa's Trap Bunny 49454
    SAY_NM_ACTIVATE_VALVES      = 0,
    // Vanessa VanCleef (nightmare) 49671 (CPP deadmines.cpp:255-267; all 12 groups exist in the MoP test world creature_text)
    SAY_NM_GLUBTOK_1            = 0,
    SAY_NM_GLUBTOK_2            = 1,
    SAY_NM_ENTERED_GLUBTOK      = 2,
    SAY_NM_BACK_TO_THE_SHIP     = 3,
    SAY_NM_HELIX_1              = 4,
    SAY_NM_HELIX_2              = 5,
    SAY_NM_MECHANICAL_1         = 6,
    SAY_NM_MECHANICAL_2         = 7,
    SAY_NM_ENTERED_MECHANICAL   = 8,
    SAY_NM_RIPSNARL_1           = 9,
    SAY_NM_RIPSNARL_2           = 10,
    SAY_NM_ENTERED_RIPSNARL     = 11,
    // Helix Gearbreaker (nightmare) 49674
    SAY_NM_ENTERED_HELIX        = 0,
    SAY_NM_SPIDERS              = 1,
};

uint32 const NIGHTMARE_ACHIEVEMENT_EVENT = 27527;   // the fork's Vindicator criteria start (boss_vanessa_vancleef.cpp ACHIEVEMENT_EVENT)
uint32 const NM_STALKER_COUNT = 8;
float const NM_STALKER_OFFSETS[NM_STALKER_COUNT] = { 8.0f, 6.0f, 4.0f, 2.0f, -2.0f, -4.0f, -6.0f, -8.0f };
float const NM_PLATTER_OMEGA = 0.4f;                // rad/s

// CPP deadmines.h:194-399 (e503f96), verbatim
Position const vanessaVanCleefNightmareSpawnPos[] =
{
    { -230.717f, -563.0139f, 51.31293f, 1.047198f },
    { -172.809f, -576.9114f, 19.39756f, 4.817109f },
    { -159.3924f, -580.5174f, 19.39817f, 6.195919f },
    { -98.5816f, -662.6632f, 7.498006f,  3.490659f }
};
Position const GlubtokNightmareIntroSpawnPos = { -229.3403f, -560.3629f, 51.31293f, 5.742133f };
Position const GlubtokNightmareSpawnPos = { -174.8212f, -579.7552f, 19.39602f, 3.193953f };
Position const HelixNightmareSpawnPos = { -172.6858f, -580.0469f, 19.39752f, 3.176499f };
Position const FoeReaperNightmareIntroSpawnPos = { -155.4705f, -580.6945f, 19.39779f };
Position const FoeReaperNightmareSpawnPos = { -101.4549f, -663.6493f, 7.505813f, 1.850049f };
Position const RipsnarlNightmareIntroSpawnPos = { -101.2135f, -663.4531f, 7.504823f, 1.867502f };
Position const EmmeHarringtonSpawnPos = { -98.63194f, -721.6268f, 8.547067f, 1.53589f };
Position const EnragedWorgenEmmeSpawnPos[] =
{
    {-97.79166f, -717.8542f, 8.668088f, 4.520403f },
    { -94.40278f, -719.7274f, 8.598646f,  3.560472f },
    { -101.9167f, -718.7552f, 8.726379f,  5.51524f },
};
Position const ErikHarringtonSpawnPos = { 5.239583f, -763.0868f, 9.800426f, 2.007129f };
Position const EnragedWorgenErikSpawnPos[] =
{
    { 3.326038f, -760.3063f, 9.713999f, 5.31516f },
    { 8.798013f, -762.2252f, 9.625132f, 3.379143f },
    { 4.232807f, -766.6125f, 9.804724f, 1.292649f }
};
Position const CalissaHarringtonSpawnPos = { -83.86406f, -775.2837f, 28.37906f, 1.710423f };
Position const JamesHarringtonSpawnPos = { -83.16319f, -774.9636f, 26.90351f, 1.710423f };
Position const glubtokNightmareFireBunnyPos[] =
{
    { -192.2031f,   -595.9636f, 36.37407f,  2.80998f },
    { -220.625f,    -577.9618f, 21.06016f,  2.513274f },
    { -205.3056f,   -563.6285f, 21.06016f,  5.253441f },
    { -214.8958f,   -546.7136f, 19.3898f,   4.712389f },
    { -207.8004f,   -570.6441f, 21.06016f,  1.762783f },
    { -221.4653f,   -549.9358f, 19.3898f,   3.211406f },
    { -229.9635f,   -559.2552f, 19.3898f,   0.0f },
    { -216.8438f,   -568.9011f, 21.06016f,  3.909538f },
    { -235.9045f,   -563.3906f, 19.3898f,   0.0f },
    { -226.6736f,   -580.8316f, 20.43056f,  2.775074f },
    { -227.5226f,   -595.1979f, 20.42358f,  4.206244f },
    { -215.0399f,   -576.3941f, 21.06016f,  3.735005f },
    { -210.592f,    -583.4739f, 21.06016f,  0.0f },
    { -228.592f,    -553.1684f, 19.3898f,   5.550147f },
    { -220.4879f,   -596.382f,  21.95116f,  0.0f },
    { -190.4774f,   -552.2778f, 51.31293f,  5.305801f },
    { -195.6267f,   -550.4393f, 51.31293f,  3.752458f },
    { -209.7257f,   -557.1042f, 51.31293f,  3.525565f },
    { -187.9531f,   -567.0469f, 51.31293f,  5.305801f },
    { -185.2083f,   -562.4844f, 19.3898f,   0.9599311f },
    { -183.4236f,   -577.2674f, 46.87183f,  3.944444f },
    { -184.6528f,   -572.3663f, 49.27317f,  3.159046f },
    { -191.2205f,   -581.4965f, 21.06015f,  2.007129f },
    { -201.224f,    -570.6788f, 21.06016f,  3.577925f },
    { -198.4653f,   -580.757f,  21.06015f,  0.8901179f },
    { -196.8767f,   -574.9688f, 21.06016f,  4.29351f },
    { -194.1302f,   -548.3055f, 19.3898f,   4.153883f },
    { -187.3333f,   -550.8143f, 19.3898f,   3.385939f },
    { -186.9792f,   -556.8472f, 19.3898f,   4.153883f },
    { -196.5504f,   -587.7031f, 21.06015f,  1.27409f },
    { -241.5938f,   -578.6858f, 19.3898f,   2.775074f },
    { -226.1615f,   -573.8021f, 20.40991f,  5.218534f }
};
Position const nightmareSkitterlingPos[] =
{
    { -184.9618f, -552.9132f, 19.3898f },
    { -199.1406f, -573.6893f, 21.06016f },
    { -194.5851f, -578.3386f, 21.06015f },
    { -193.3264f, -569.809f, 21.06016f },
    { -185.7622f, -566.6059f, 19.3898f },
    { -181.401f, -561.684f, 19.3898f },
    { -221.2569f, -549.3316f, 19.3898f },
    { -215.125f, -546.0486f, 19.3898f },
    { -210.3854f, -550.6736f, 19.3898f },
    { -206.8715f, -545.3837f, 19.3898f },
    { -208.3247f, -556.5833f, 19.3898f },
    { -203.1233f, -550.8281f, 19.3898f },
    { -198.2465f, -546.9288f, 19.3898f },
    { -178.6389f, -564.4479f, 19.39f },
    { -193.3698f, -581.8663f, 21.06015f },
    { -188.1788f, -578.3715f, 21.06015f },
    { -185.3385f, -562.592f, 19.3898f },
    { -179.6458f, -557.9063f, 19.3898f },
    { -226.5208f, -587.257f, 19.53274f },
    { -199.3351f, -586.7952f, 21.06015f },
    { -207.6545f, -594.809f, 21.06015f },
    { -201.2587f, -593.309f, 21.06016f },
    { -224.2743f, -592.7917f, 20.29579f },
    { -211.9844f, -569.5417f, 21.06016f },
    { -208.9288f, -563.6094f, 21.06016f },
    { -200.8715f, -557.2518f, 19.3898f },
    { -196.2396f, -553.6927f, 19.3898f },
    { -192.1163f, -550.1024f, 19.3898f },
    { -202.4063f, -563.1337f, 21.06016f },
    { -195.9635f, -560.8889f, 20.58139f },
    { -190.1094f, -557.4722f, 19.3898f },
    { -199.3229f, -550.3785f, 19.3898f },
    { -195.2552f, -557.0087f, 19.3898f },
    { -190.1372f, -553.0625f, 19.3898f },
    { -185.2118f, -557.6406f, 19.3898f },
    { -190.6406f, -561.441f, 19.3898f },
    { -197.7413f, -564.3021f, 21.06016f },
    { -215.5365f, -549.8021f, 19.3898f },
    { -212.1719f, -555.0226f, 19.3898f },
    { -213.2344f, -561.8577f, 21.06016f },
    { -208.5451f, -568.1146f, 21.06016f },
    { -202.7257f, -572.5174f, 21.06016f },
    { -205.6354f, -559.1563f, 21.06016f },
    { -206.1615f, -553.3507f, 19.3898f },
    { -202.5451f, -546.118f, 19.3898f },
    { -204.1024f, -596.7518f, 21.06016f },
    { -204.0087f, -590.8386f, 21.06016f },
    { -198.8316f, -591.7239f, 21.06015f },
    { -195.4375f, -586.2934f, 21.06015f },
    { -197.6493f, -582.4653f, 21.06015f },
    { -219.1337f, -559.7899f, 19.3898f },
    { -221.4063f, -553.1406f, 19.3898f },
    { -216.1215f, -564.882f, 21.06016f },
    { -199.3125f, -578.1996f, 21.06016f },
    { -204.684f, -569.3542f, 21.06016f },
    { -197.8576f, -569.3489f, 21.06016f },
    { -191.7656f, -565.8559f, 21.06016f },
    { -213.4635f, -591.625f, 21.06015f },
    { -206.0781f, -588.0712f, 21.06016f },
    { -213.1997f, -585.125f, 21.06015f },
    { -220.5174f, -581.8143f, 21.06016f },
    { -182.8004f, -569.368f, 19.39167f },
    { -189.0191f, -571.2674f, 21.06016f },
    { -193.7674f, -574.7014f, 21.06016f },
    { -226.1285f, -581.059f, 20.79114f },
    { -218.6076f, -581.0746f, 21.06016f },
    { -216.1094f, -576.8264f, 21.06016f },
    { -222.1545f, -555.1077f, 19.3898f },
    { -212.6059f, -567.2239f, 21.06016f },
    { -203.9184f, -551.0608f, 19.3898f },
    { -202.8663f, -590.1371f, 21.06016f },
    { -196.4757f, -573.0833f, 21.06016f },
    { -185.9288f, -558.125f, 19.3898f },
    { -213.8767f, -559.3125f, 20.02806f },
    { -221.8142f, -556.3976f, 19.3898f },
    { -216.5017f, -553.9722f, 19.3898f },
    { -223.7483f, -574.9896f, 21.06015f },
    { -226.1215f, -563.0382f, 19.3898f },
    { -213.8559f, -574.6094f, 21.06016f },
    { -208.3854f, -571.8004f, 21.06016f },
    { -224.5625f, -568.1476f, 19.92673f },
    { -219.7083f, -588.6094f, 21.06015f },
    { -218.8872f, -570.5643f, 21.06016f },
    { -221.8264f, -562.2795f, 19.3898f },
    { -227.9063f, -559.566f, 19.3898f },
    { -230.4948f, -566.0191f, 19.3898f },
    { -227.0243f, -552.6736f, 19.3898f },
    { -231.566f, -572.1979f, 19.3898f },
    { -231.8802f, -578.5677f, 19.3898f },
    { -232.4462f, -562.8837f, 19.3898f },
    { -232.4514f, -586.2518f, 19.3898f },
    { -230.3767f, -592.8577f, 19.3898f },
    { -232.8004f, -591.1805f, 19.3898f },
    { -234.5208f, -559.4809f, 19.3898f },
    { -236.5712f, -565.5573f, 19.3898f },
    { -237.5799f, -571.5903f, 19.3898f },
    { -238.9063f, -578.1528f, 19.3898f },
    { -234.0608f, -598.0f,    19.95063f },
    { -236.5122f, -591.9913f, 19.3898f },
    { -238.8021f, -584.8594f, 19.3898f },
};
Position const DarkwebDevourerPos[] =
{
    { -215.7064f, -553.3212f, 51.29544f, 0.7243207f },
    { -229.285f, -568.6863f, 51.31313f, 0.8218399f },
    { -231.3453f, -584.1043f, 51.31514f, 1.05134f }
};
Position const ChatteringHorrorPos = { -233.625f, -570.5868f, 19.3898f };
float const LightningPlatterCirclePathHeight[] =
{
    14.28754f,  // -120.9898 Y: -638.3776 Z: 12.34728
    16.38484f,  // -135.3954 Y: -623.0512 Z: 14.44327
    16.18771f,  // -120.1581 Y: -617.6207 Z: 14.24549
    17.47415f,  // -136.7385 Y: -604.6659 Z: 15.53128
    20.52248f,  // -130.4809 Y: -586.5009 Z: 18.57832
    21.08911f   // -143.004  Y: -574.9209 Z: 19.1443
};
Position const LightningPlatterPos[] =
{
    { -120.9898f, -638.3776f, 12.34728f, 0.03183358f },
    { -135.3954f, -623.0512f, 14.44327f, 0.03133297f },
    { -120.1581f, -617.6207f, 14.24549f, 6.252115f   },
    { -136.7385f, -604.6659f, 15.53128f, 0.03079747f },
    { -130.4809f, -586.5009f, 18.57832f, 0.03022512f },
    { -143.004f, -574.9209f, 19.1443f,   6.253215f   }
};

uint32 const NPC_VANESSA_ANCHOR_BUNNY_JMF = 51624;   // MoP test DB creature 326763 (spawnMask 6), above the trap

struct NmTalk
{
    uint32 at;
    ObjectGuid who;
    uint8 group;   // 0xFF = despawn
};

class instance_deadmines : public InstanceMapScript
{
    public:
        instance_deadmines() : InstanceMapScript("instance_deadmines", 36) { }

        struct instance_deadmines_InstanceMapScript : public InstanceScript
        {
            instance_deadmines_InstanceMapScript(Map* map) : InstanceScript(map) 
            {
                SetBossNumber(MAX_ENCOUNTER);
                LoadDoorData(doordata);
                
                uiGlubtokGUID = ObjectGuid::Empty;
                uiHelixGUID = ObjectGuid::Empty;
                uiOafGUID = ObjectGuid::Empty;
                uiFoereaperGUID = ObjectGuid::Empty;
                uiAdmiralGUID = ObjectGuid::Empty;
                uiCaptainGUID = ObjectGuid::Empty;

                GoblinTeleporterGUID = ObjectGuid::Empty;
                HeavyDoorGUID = ObjectGuid::Empty;
                HeavyDoor2GUID       = ObjectGuid::Empty;
                IronCladDoorGUID = ObjectGuid::Empty;
                DefiasCannonGUID = ObjectGuid::Empty;
                DoorLeverGUID = ObjectGuid::Empty;
                TeamInInstance       = 0;
                uiVanessaEvent       = 0;
                OverchargeCount      = 0;
                nmClock              = 0;   // #826
                valvesActivated      = 0;
                deadWorgen           = 0;
                noteSpawnChecked     = false;
                State                = CANNON_NOT_USED;
                instance->SetWorldState(WORLDSTATE_PROTOTYPE_PRODIGY, 1);
            };

            void OnPlayerEnter(Player* player) override
            {
                if (!TeamInInstance)
                    TeamInInstance = player->GetTeam();

                // #826 (CPP instance_deadmines.cpp:158-163): an instance loaded with Cookie dead and Vanessa not gets its Note back
                if (!noteSpawnChecked)
                {
                    noteSpawnChecked = true;
                    if (instance->IsHeroic() && GetBossState(DATA_CAPTAIN) == DONE && GetBossState(DATA_VANESSA) != DONE &&
                        uiVanessaEvent == NOT_STARTED)
                        instance->SummonCreature(NPC_NOTE_FROM_VANESSA, notePos);
                }
            }

            void OnCreatureCreate(Creature* creature) override
            {
                if (!TeamInInstance)
                {
                    Map::PlayerList const &players = instance->GetPlayers();
                    if (!players.isEmpty())
                        if (Player* player = players.begin()->GetSource())
                            TeamInInstance = player->GetTeam();
                }

                switch (creature->GetEntry())
                {
                    case NPC_GLUBTOK:
                        uiGlubtokGUID = creature->GetGUID();
                        break;
                    case NPC_HELIX_GEARBREAKER:
                        uiHelixGUID = creature->GetGUID();
                        break;
                    case NPC_LUMBERING_OAF:
                        uiOafGUID = creature->GetGUID();
                        break;
                    case NPC_FOE_REAPER_5000:
                        uiFoereaperGUID = creature->GetGUID();
                        break;
                    case NPC_ADMIRAL_RIPSNARL:
                        uiAdmiralGUID = creature->GetGUID();
                        break;
                    case NPC_CAPTAIN_COOKIE:
                        uiCaptainGUID = creature->GetGUID();
                        break;
                    case NPC_VANESSA_VANCLEEF:   // #826: the DB spawn, shown at the end of the Nightmare Gauntlet
                        if (!creature->IsSummon())
                            uiVanessaGUID = creature->GetGUID();
                        break;
                    case NPC_VANESSA_ANCHOR_BUNNY_JMF:
                        anchorBunnyGUID = creature->GetGUID();
                        break;
                    case NPC_KAGTHA:
                        if (TeamInInstance == ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_SLINKY_SHARPSHIV:
                        if (TeamInInstance == ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_MISS_MAYHEM:
                        if (TeamInInstance == ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_MAYHEM_REAPER:
                        if (TeamInInstance == ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_HAND_ASSASIN:
                        if (TeamInInstance == ALLIANCE)
                            creature->SetVisible(false);
                        break;
                    case NPC_HORATIO_LAINE:
                        if (TeamInInstance == HORDE)
                            creature->SetVisible(false);
                        break;
                    case NPC_DEFENDER:
                        if (TeamInInstance == HORDE)
                            creature->SetVisible(false);
                        break;
                    case NPC_INVESTIGATOR:
                        if (TeamInInstance == HORDE)
                            creature->SetVisible(false);
                        break;
                    case NPC_CRIME_SCENE_BOT:
                        if (TeamInInstance == HORDE)
                            creature->SetVisible(false);
                        break;
                }
            }

            void OnUnitDeath(Unit* unit) override
            {
                // #826: the gauntlet's stage bosses, the Harringtons and the wipe check
                if (uiVanessaEvent >= NIGHTMARE_STAGE_TRAP)
                    NightmareUnitDeath(unit);

                if (unit->ToCreature())
                {
                    switch (unit->GetEntry())
                    {
                        case NPC_MINING_POWDER:
                            unit->CastSpell(unit, SPELL_EXPLODE_MINE, false);
                            break;
                        case NPC_DEFIAS_WATCHER:
                        case NPC_DEFIAS_REAPER:
                            OverchargeCount++;

                            if (OverchargeCount >= 4)
                                if (Creature* FoeReaper = instance->GetCreature(GetGuidData(DATA_FOEREAPER)))
                                    FoeReaper->RemoveAurasDueToSpell(SPELL_OFF_LINE);
                            break;
                    }
                }
            }

            void OnGameObjectCreate(GameObject* go) override
            {
                switch (go->GetEntry())
                {
                    case GO_HEAVY_DOOR:
                        HeavyDoorGUID = go->GetGUID();
                        AddDoor(go, true);
                        break;
                    case GO_HEAVY_DOOR_2:
                        HeavyDoor2GUID = go->GetGUID();
                        AddDoor(go, true);
                        break;
                    case GO_FOUNDRY_DOOR:
                        FoundryDoorGUID = go->GetGUID();   // #826: closed during the gauntlet, opened for the mechanical nightmare
                        AddDoor(go, true);
                        break;
                    case GO_FACTORY_DOOR:
                    case GO_MAST_ROOM_DOOR:
                        AddDoor(go, true);
                        break;
                    case GO_GOBLIN_TELEPORTER:
                        GoblinTeleporterGUID = go->GetGUID();
                        break;
                    case GO_IRONCLAD_DOOR:
                        IronCladDoorGUID = go->GetGUID();
                        break;
                    case GO_DEFIAS_CANNON:
                        DefiasCannonGUID = go->GetGUID();
                        break;
                    case GO_DOOR_LEVER:
                        DoorLeverGUID = go->GetGUID();
                        break;
                }
            }

            void ShootCannon()
            {
                if (GameObject* pDefiasCannon = instance->GetGameObject(DefiasCannonGUID))
                {
                    pDefiasCannon->SetGoState(GO_STATE_ACTIVE);
                    pDefiasCannon->PlayDirectSound(SOUND_CANNONFIRE);
                }
            }

            void BlastOutDoor()
            {
                if (GameObject* pIronCladDoor = instance->GetGameObject(IronCladDoorGUID))
                {
                    pIronCladDoor->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);
                    pIronCladDoor->PlayDirectSound(SOUND_DESTROYDOOR);
                }
            }

            void SetData(uint32 type, uint32 data) override
            {
                switch (type)
                {
                    case DATA_CANNON_EVENT:
                        State = data;
                        if (data == CANNON_BLAST_INITIATED)
                        {
                            ShootCannon();
                            BlastOutDoor();
                        }    
                        break;
                    case DATA_VANESSA_EVENT:
                        // #826: the end of the intro starts the gauntlet; FAIL = Vanessa's wipe (boss_vanessa_vancleef.cpp EnterEvadeMode)
                        if (data == NIGHTMARE_STAGE_TRAP)
                        {
                            StartNightmare();
                            break;
                        }
                        if (data == FAIL)
                        {
                            ResetNightmare("Vanessa VanCleef wipe");
                            break;
                        }
                        uiVanessaEvent = data;
                        if (data == DONE)
                            SaveToDB();
                        break;
                    case DATA_ACTIVATED_VALVE:
                        ValveActivated();
                        break;
                }
            }

            void Update(uint32 diff) override
            {
                UpdateNightmare(diff);   // #826
            }

            uint32 GetData(uint32 type) const override
            {
                if (type == DATA_VANESSA_EVENT)
                    return uiVanessaEvent;
                return 0;
            }

            ObjectGuid GetGuidData(uint32 type) const override
            {
                switch (type)
                {
                    case DATA_GLUBTOK:
                        return uiGlubtokGUID;
                    case DATA_HELIX:
                        return uiHelixGUID;
                    case DATA_OAF:
                        return uiOafGUID;
                    case DATA_FOEREAPER:
                        return uiFoereaperGUID;
                    case DATA_ADMIRAL:
                        return uiAdmiralGUID;
                    case DATA_TEAM_IN_INSTANCE:     
                        return ObjectGuid(uint64(TeamInInstance));
                    case DATA_VANESSA_BOSS:
                        return uiVanessaGUID;
                }
                return ObjectGuid::Empty;
            }
            
            bool SetBossState(uint32 type, EncounterState state) override
            {
                if (!InstanceScript::SetBossState(type, state))
                    return false;
                    
                switch (type)
                {
                    case DATA_HELIX:
                        if (state == IN_PROGRESS)
                        {
                            if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                                go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        }
                        else if (state == DONE)
                            if (GameObject* go = instance->GetGameObject(HeavyDoorGUID))
                                go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                    case DATA_FOEREAPER:
                        if (state == IN_PROGRESS)
                        {
                            if (GameObject* go = instance->GetGameObject(HeavyDoor2GUID))
                                go->SetFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        }
                        else if (state == DONE)
                            if (GameObject* go = instance->GetGameObject(HeavyDoor2GUID))
                                go->RemoveFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_NOT_SELECTABLE);
                        break;
                }

                return true;
            }


            // ======================================== #826 Nightmare Gauntlet ========================================
            // a live gauntlet creature (not one already queued for removal: a second despawn in the same tick would clean it up twice)
            Creature* GetNmCreature(ObjectGuid guid) const
            {
                Creature* creature = guid ? instance->GetCreature(guid) : nullptr;
                return creature && !creature->IsDestroyedObject() ? creature : nullptr;
            }

            // a gauntlet creature: kept for the cleanup, never despawned by a grid unload, optionally a hazard that must not fight
            Creature* NmSummon(uint32 entry, Position const& pos, bool hazard = false)
            {
                TempSummon* summon = instance->SummonCreature(entry, pos);
                if (!summon)
                    return nullptr;

                summon->setActive(true);
                nightmareGuids.insert(summon->GetGUID());
                if (hazard)
                {
                    // a hazard that entered combat would evade home and stop (or leave) its path. Their area damage is
                    // TARGET_UNIT_SRC_AREA_ENTRY with no conditions (any unit, Spell.cpp WorldObjectSpellTargetCheck): the 2 yd spark
                    // stalkers stand 2 yd apart and the Molten Magma reaches the valves - only the trigger-flagged bunnies
                    // (flags_extra 128) are out of sight for it, so the others take no damage.
                    summon->SetIsCombatDisallowed(true);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->ApplySpellImmune(0, IMMUNITY_DAMAGE, SPELL_SCHOOL_MASK_ALL, true);
                }
                return summon;
            }

            void NmTrack(Creature* creature)
            {
                if (creature)
                    nightmareGuids.insert(creature->GetGUID());
            }

            // talk queue (CPP's per-creature EventMaps): `group` said by `guid` `delayMs` from now
            void QueueTalk(ObjectGuid guid, uint8 group, uint32 delayMs)
            {
                nmTalks.push_back({ nmClock + delayMs, guid, group });
            }

            void QueueDespawn(ObjectGuid guid, uint32 delayMs)
            {
                nmTalks.push_back({ nmClock + delayMs, guid, 0xFF });
            }

            void NmLog(char const* what)
            {
                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] %s (stage %u, t=%u s)", what, uiVanessaEvent, nmClock / 1000);
            }

            template <class F>
            void ForEachAlivePlayer(F f)
            {
                Map::PlayerList const& players = instance->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->GetSource())
                        if (player->IsAlive() && !player->IsGameMaster())
                            f(player);
            }

            // CPP: at every stage change the purpose bunny re-casts Nightmare Aura + Nightmare Slow (-75% speed, 10 s) on everyone and
            // the previous stage's screen aura is cancelled (92584-92587 remove 92565-92568)
            void NmStageAura(uint32 previousStageAura)
            {
                if (Creature* bunny = GetNmCreature(announcerGuid))
                {
                    bunny->CastSpell(bunny, SPELL_NIGHTMARE_AURA, true);
                    bunny->CastSpell(bunny, SPELL_NIGHTMARE_SLOW, true);
                }
                if (previousStageAura)
                    DoRemoveAurasDueToSpellOnPlayers(previousStageAura);
            }

            // the intro is over (sitting Vanessa, boss_vanessa_vancleef.cpp): the players hang in Vanessa's trap over the slag
            void StartNightmare()
            {
                uiVanessaEvent = NIGHTMARE_STAGE_TRAP;
                nmClock = 0;
                valvesActivated = 0;
                deadWorgen = 0;
                nmTalks.clear();
                nightmareEvents.Reset();
                NmLog("start: Vanessa's trap");

                if (GameObject* door = instance->GetGameObject(FoundryDoorGUID))
                    door->SetGoState(GO_STATE_READY);
                if (GameObject* door = instance->GetGameObject(IronCladDoorGUID))
                    door->SetGoState(GO_STATE_READY);

                instance->LoadGrid(trapBunnyPos.GetPositionX(), trapBunnyPos.GetPositionY());
                if (Creature* bunny = NmSummon(NPC_GENERAL_PURPOSE_DUMMY_JMF, Position(-205.4688f, -567.2049f, 21.06016f), true))
                    announcerGuid = bunny->GetGUID();   // CPP GeneralPurposeBunnyPos (the nightmare's purpose bunny)

                if (Creature* trap = NmSummon(NPC_VANESSAS_TRAP_BUNNY, trapBunnyPos, true))
                {
                    trapGuid = trap->GetGUID();
                    trap->SetDisableGravity(true);
                    if (Creature* anchor = GetNmCreature(anchorBunnyGUID))
                        anchor->CastSpell(trap, SPELL_ROPE_BEAM, true);
                }

                // next to the trap first (Ride Vehicle 46598: 150 yd range), then seated 2.5 s later
                ForEachAlivePlayer([](Player* player)
                {
                    player->NearTeleportTo(magmaTrapThrowPos.GetPositionX(), magmaTrapThrowPos.GetPositionY(), magmaTrapThrowPos.GetPositionZ(), player->GetOrientation());
                });
                nightmareEvents.ScheduleEvent(EVENT_NM_BOARD_TRAP, 2500);
            }

            // #826 w106 (nm-trap "5 players boarded", then no valve click, triggers 0): every player was put on the trap with
            // EnterVehicle = 46598 Ride Vehicle Hardcoded. Two 46598 auras from different casters only stack on a vehicle with a free
            // "usable" seat (Aura::CanStackWith, SpellAuras.cpp:2063-2075 -> Vehicle::GetAvailableSeatCount, Vehicle.cpp:912-921), and
            // none of the trap's seats is (VehicleSeat.dbc 9282-9286: flags 0x80000007, flagsB 0 - no CAN_ENTER_OR_EXIT, no
            // USABLE_FORCED). So each boarding removed the previous rider's aura (Unit::_RemoveNoStackAurasDueToAura) and only the
            // last player stayed on: the four bots stood at the throw position all gauntlet long ([PROBE] vehicle=-), the seated-bot
            // edge "dm click steam valve" never fired and the trap sat in the slag. CPP seats them with 92379 Ride Magma Vehicle
            // (92378's script effect makes each player cast it; 5.4.8 SpellMisc: AttributesEx3 0x10130200 has
            // SPELL_ATTR3_STACK_FOR_DIFF_CASTERS, so the riders' auras stack). 92379 targets TARGET_UNIT_NEARBY_ENTRY with no
            // conditions row in this DB, so the aura is put on the trap directly, the player as its caster and the seat in its base
            // points - the same call Unit::HandleSpellClick makes for a vehicle-enter aura (Unit.cpp:18232-18237). The aura applies on
            // the trap's next update (HandleAuraControlVehicle -> _EnterVehicle on that seat). Returns the players boarded now.
            uint32 BoardTrap(Creature* trap)
            {
                Vehicle* kit = trap ? trap->GetVehicleKit() : nullptr;
                SpellInfo const* ride = sSpellMgr->GetSpellInfo(SPELL_RIDE_MAGMA_VEHICLE);
                if (!kit || !ride)
                    return 0;

                uint32 boarded = 0;
                int8 seat = 0;
                ForEachAlivePlayer([&](Player* player)
                {
                    if (player->GetVehicleBase() == trap)
                        return;
                    while (seat < 5 && !kit->HasEmptySeat(seat))
                        ++seat;
                    if (seat >= 5)
                        return;

                    int32 bp[MAX_SPELL_EFFECTS];
                    for (uint32 i = 0; i < MAX_SPELL_EFFECTS; ++i)
                        bp[i] = ride->Effects[i].BasePoints;
                    bp[EFFECT_0] = seat + 1;   // HandleAuraControlVehicle seats the caster on m_amount - 1
                    if (Aura::TryRefreshStackOrCreate(ride, MAX_EFFECT_MASK, trap, player, bp))
                        ++boarded;
                    ++seat;
                });
                return boarded;
            }

            void EjectFromTrap()
            {
                Creature* trap = GetNmCreature(trapGuid);
                if (!trap || !trap->GetVehicleKit())
                    return;

                std::vector<Unit*> riders;
                for (int8 seat = 0; seat < 5; ++seat)
                    if (Unit* passenger = trap->GetVehicleKit()->GetPassenger(seat))
                        riders.push_back(passenger);

                // CPP: ExitVehicle + 92489 Forcecast Magma Trap Throw (jump to the sniffed landing on the Glubtok's-nightmare ledge)
                for (Unit* rider : riders)
                {
                    rider->ExitVehicle();
                    if (Player* player = rider->ToPlayer())
                        player->NearTeleportTo(magmaTrapThrowPos.GetPositionX(), magmaTrapThrowPos.GetPositionY(), magmaTrapThrowPos.GetPositionZ(), player->GetOrientation());
                }
                trap->RemoveAllAuras();
                trap->DespawnOrUnsummon(1000);
            }

            void DespawnNightmareSet(GuidSet& set)
            {
                for (ObjectGuid const& guid : set)
                    if (Creature* creature = GetNmCreature(guid))
                        creature->DespawnOrUnsummon();
                set.clear();
            }

            void CleanupNightmare()
            {
                EjectFromTrap();
                DespawnNightmareSet(spiderGuids);    // Helix's summons; every other gauntlet creature is in nightmareGuids
                fireGuids.clear();
                sparkGuids.clear();
                DespawnNightmareSet(nightmareGuids);
                nightmareEvents.Reset();
                nmTalks.clear();
                trapGuid = announcerGuid = vanessaNmGuid = glubtokNmGuid = helixNmGuid = reaperNmGuid = ObjectGuid::Empty;
                emmeGuid = erikGuid = calissaGuid = jamesGuid = ObjectGuid::Empty;
                static uint32 const playerAuras[] = { SPELL_NIGHTMARE_ELIXIR, SPELL_NIGHTMARE_AURA, SPELL_NIGHTMARE_SLOW, SPELL_VANESSAS_BLACKOUT_AURA,
                    SPELL_NIGHTMARE_AURA_GLUBTOK, SPELL_NIGHTMARE_AURA_HELIX, SPELL_NIGHTMARE_AURA_MECHANICAL, SPELL_NIGHTMARE_AURA_RIPSNARL };
                for (uint32 spell : playerAuras)
                    DoRemoveAurasDueToSpellOnPlayers(spell);
            }

            // CPP ResetVanessasNightmare (instance_deadmines.cpp:397-450): a wipe or a dead Harrington; the Note comes back after 30 s
            void ResetNightmare(char const* why)
            {
                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] FAIL: %s (stage %u, t=%u s) - reset, the Note returns in 30 s", why, uiVanessaEvent, nmClock / 1000);
                CleanupNightmare();
                uiVanessaEvent = NOT_STARTED;
                if (GameObject* door = instance->GetGameObject(FoundryDoorGUID))
                    door->SetGoState(GO_STATE_ACTIVE);
                if (GameObject* door = instance->GetGameObject(IronCladDoorGUID))
                    door->SetGoState(State == CANNON_BLAST_INITIATED ? GO_STATE_ACTIVE_ALTERNATIVE : GO_STATE_ACTIVE);
                nightmareEvents.ScheduleEvent(EVENT_NM_RESPAWN_NOTE, 30000);
                SaveToDB();
            }

            void SetNightmareStage(uint32 stage)
            {
                uiVanessaEvent = stage;
                switch (stage)
                {
                    case NIGHTMARE_STAGE_GLUBTOK:
                        NmLog("stage: Glubtok's nightmare");
                        nightmareEvents.ScheduleEvent(EVENT_NM_EJECT, 0);
                        nightmareEvents.ScheduleEvent(EVENT_NM_GLUBTOK_AURA, 4000);
                        break;
                    case NIGHTMARE_STAGE_HELIX:
                        NmLog("stage: Helix's nightmare");
                        nightmareEvents.CancelEvent(EVENT_NM_ICICLE);
                        nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_AURA, 2900);
                        break;
                    case NIGHTMARE_STAGE_FOE_REAPER:
                        NmLog("stage: the mechanical nightmare");
                        nightmareEvents.CancelEvent(EVENT_NM_HELIX_SPIRIT_STRIKE);
                        nightmareEvents.ScheduleEvent(EVENT_NM_REAPER_DESPAWN_SPIDERS, 2000);
                        break;
                    case NIGHTMARE_STAGE_RIPSNARL:
                        NmLog("stage: Ripsnarl's nightmare");
                        nightmareEvents.ScheduleEvent(EVENT_NM_RIPSNARL_AURA, 2500);
                        break;
                    default:
                        break;
                }
            }

            // a nightmare boss copy that fights (CPP: the intro copy stays IMMUNE_TO_PC / NPC, the fighting one has them removed)
            Creature* NmFighter(uint32 entry, Position const& pos)
            {
                Creature* creature = NmSummon(entry, pos);
                if (creature)
                {
                    creature->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC | UNIT_FLAG_STUNNED);
                    creature->SetReactState(REACT_AGGRESSIVE);
                }
                return creature;
            }

            void SummonWorgen(Position const* positions, Creature* victim)
            {
                for (uint8 i = 0; i < 3; ++i)
                {
                    Creature* worgen = NmSummon(NPC_ENRAGED_WORGEN, positions[i]);
                    if (worgen && victim)
                    {
                        worgen->GetThreatManager().AddThreat(victim, 1000.0f);
                        worgen->AI()->AttackStart(victim);
                    }
                }
            }

            void UpdateNightmare(uint32 diff)
            {
                nmClock += diff;

                // the talk / despawn queue
                for (size_t i = 0; i < nmTalks.size();)
                {
                    if (nmTalks[i].at > nmClock)
                    {
                        ++i;
                        continue;
                    }
                    NmTalk const talk = nmTalks[i];
                    nmTalks.erase(nmTalks.begin() + i);
                    if (Creature* who = GetNmCreature(talk.who))
                    {
                        if (talk.group == 0xFF)
                            who->DespawnOrUnsummon();
                        else if (who->IsAIEnabled && who->AI())
                            who->AI()->Talk(talk.group);
                    }
                }

                nightmareEvents.Update(diff);
                while (uint32 eventId = nightmareEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_NM_BOARD_TRAP:
                        {
                            Creature* trap = GetNmCreature(trapGuid);
                            if (!trap)
                                break;
                            uint32 const boarded = BoardTrap(trap);
                            TC_LOG_INFO("scripts", "[DM-NIGHTMARE] trap: %u players boarded (t=%u s)", boarded, nmClock / 1000);
                            nightmareEvents.ScheduleEvent(EVENT_NM_TRAP_SEATED, 1000);
                            nightmareEvents.ScheduleEvent(EVENT_NM_SUMMON_VALVES, 2900);
                            nightmareEvents.ScheduleEvent(EVENT_NM_TRAP_MOVE_DOWN, 6000);
                            break;
                        }
                        case EVENT_NM_TRAP_SEATED:
                        {
                            // #826 w106: the boarding line counted calls, not riders; this one counts the riders (driver row nm-seated)
                            Creature* trap = GetNmCreature(trapGuid);
                            Vehicle* kit = trap ? trap->GetVehicleKit() : nullptr;
                            if (!kit)
                                break;
                            uint32 alive = 0;
                            uint32 seated = 0;
                            std::string names;
                            ForEachAlivePlayer([&](Player* player)
                            {
                                ++alive;
                                if (player->GetVehicleBase() != trap)
                                    return;
                                ++seated;
                                names += (names.empty() ? "" : ",") + player->GetName();
                            });
                            TC_LOG_INFO("scripts", "[DM-NIGHTMARE] trap: %u/%u players seated (%s) (t=%u s)", seated, alive, names.c_str(), nmClock / 1000);
                            if (seated < alive)
                                if (uint32 again = BoardTrap(trap))
                                    TC_LOG_INFO("scripts", "[DM-NIGHTMARE] trap: %u players boarded again (t=%u s)", again, nmClock / 1000);
                            break;
                        }
                        case EVENT_NM_SUMMON_VALVES:
                            // CPP npc_deadmines_vanessas_trap_bunny (deadmines.cpp:209-216): four Steam Valves around the trap; the
                            // DB-spawned valves of this test DB (spawnMask 6) stay unselectable, the summoned ones are clickable
                            for (uint8 i = 0; i < 4; ++i)
                                if (Creature* valve = NmSummon(NPC_STEAM_VALVE, steamValvePos[i], true))
                                    valve->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                            if (Creature* trap = GetNmCreature(trapGuid))
                                QueueTalk(trap->GetGUID(), SAY_NM_ACTIVATE_VALVES, 3300);
                            NmLog("trap: four steam valves");
                            break;
                        case EVENT_NM_TRAP_MOVE_DOWN:
                            if (Creature* trap = GetNmCreature(trapGuid))
                            {
                                // CPP: run speed = half the walk speed ("moving down very, very slow"), 2.5 * 0.5 = 1.25 yd/s; the
                                // 7 yd to the slag take 5.6 s, then Molten Magma
                                trap->SetSpeed(MOVE_RUN, 1.25f / 7.0f);
                                trap->GetMotionMaster()->MovePoint(1, trapBunnyMoveDownPos.GetPositionX(), trapBunnyMoveDownPos.GetPositionY(), trapBunnyMoveDownPos.GetPositionZ(), false);
                                nightmareEvents.ScheduleEvent(EVENT_NM_TRAP_MAGMA, 5600);
                            }
                            break;
                        case EVENT_NM_TRAP_MAGMA:
                            if (Creature* trap = GetNmCreature(trapGuid))
                            {
                                trap->CastSpell(trap, SPELL_MOLTEN_MAGMA, true);
                                NmLog("trap: Molten Magma (the trap reached the slag)");
                            }
                            break;
                        case EVENT_NM_EJECT:
                            EjectFromTrap();
                            trapGuid = ObjectGuid::Empty;
                            break;
                        case EVENT_NM_GLUBTOK_AURA:
                            NmStageAura(0);
                            QueueTalk(announcerGuid, SAY_NM_ELIXIR_TAKES_HOLD, 200);
                            nightmareEvents.ScheduleEvent(EVENT_NM_GLUBTOK_SETUP, 800);
                            break;
                        case EVENT_NM_GLUBTOK_SETUP:
                        {
                            if (Creature* vanessa = NmSummon(NPC_VANESSA_VANCLEEF_NIGHTMARE, vanessaVanCleefNightmareSpawnPos[0]))
                            {
                                vanessaNmGuid = vanessa->GetGUID();
                                QueueTalk(vanessaNmGuid, SAY_NM_GLUBTOK_1, 1000);
                                QueueTalk(vanessaNmGuid, SAY_NM_GLUBTOK_2, 9400);
                                QueueTalk(vanessaNmGuid, SAY_NM_ENTERED_GLUBTOK, 14100);
                                QueueTalk(vanessaNmGuid, SAY_NM_BACK_TO_THE_SHIP, 18900);
                                QueueDespawn(vanessaNmGuid, 21400);
                            }
                            if (Creature* intro = NmSummon(NPC_GLUBTOK_NIGHTMARE, GlubtokNightmareIntroSpawnPos))
                                QueueDespawn(intro->GetGUID(), 21400);
                            for (Position const& pos : glubtokNightmareFireBunnyPos)
                                if (Creature* fire = NmSummon(NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY, pos, true))
                                    fireGuids.insert(fire->GetGUID());
                            TC_LOG_INFO("scripts", "[DM-NIGHTMARE] glubtok: %u nightmare flames (t=%u s)", uint32(fireGuids.size()), nmClock / 1000);
                            nightmareEvents.ScheduleEvent(EVENT_NM_GLUBTOK_SPAWN, 14400);
                            nightmareEvents.ScheduleEvent(EVENT_NM_ICICLE, 14000);
                            break;
                        }
                        case EVENT_NM_GLUBTOK_SPAWN:
                            if (Creature* glubtok = NmFighter(NPC_GLUBTOK_NIGHTMARE, GlubtokNightmareSpawnPos))
                            {
                                glubtokNmGuid = glubtok->GetGUID();
                                NmLog("glubtok: the nightmare Glubtok (49670) fights");
                            }
                            break;
                        case EVENT_NM_ICICLE:
                        {
                            // CPP: the purpose bunny casts 92189 on a random player within 100 yd every 2.4 s until Glubtok dies
                            std::vector<Player*> targets;
                            ForEachAlivePlayer([&targets](Player* player)
                            {
                                if (player->GetExactDist2d(trapBunnyPos.GetPositionX(), trapBunnyPos.GetPositionY()) < 100.0f)
                                    targets.push_back(player);
                            });
                            if (!targets.empty())
                            {
                                Player* target = targets[urand(0, uint32(targets.size()) - 1)];
                                if (Creature* icicle = NmSummon(NPC_COLLAPSING_ICICLE, target->GetPosition(), true))
                                {
                                    icicle->CastSpell(icicle, SPELL_ICICLE, true);
                                    icicle->DespawnOrUnsummon(4000);
                                }
                            }
                            nightmareEvents.ScheduleEvent(EVENT_NM_ICICLE, 2400);
                            break;
                        }
                        case EVENT_NM_HELIX_AURA:
                            NmStageAura(SPELL_NIGHTMARE_AURA_GLUBTOK);
                            QueueTalk(announcerGuid, SAY_NM_NIGHTMARE_SHIFTS, 200);
                            nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_DESPAWN_FIRE, 400);
                            break;
                        case EVENT_NM_HELIX_DESPAWN_FIRE:
                            DespawnNightmareSet(fireGuids);
                            nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_SETUP, 2800);
                            break;
                        case EVENT_NM_HELIX_SETUP:
                            if (Creature* vanessa = NmSummon(NPC_VANESSA_VANCLEEF_NIGHTMARE, vanessaVanCleefNightmareSpawnPos[1]))
                            {
                                vanessaNmGuid = vanessa->GetGUID();
                                QueueTalk(vanessaNmGuid, SAY_NM_HELIX_1, 1000);
                                QueueTalk(vanessaNmGuid, SAY_NM_HELIX_2, 7000);
                                QueueDespawn(vanessaNmGuid, 15300);
                            }
                            if (Creature* helix = NmSummon(NPC_HELIX_GEARBREAKER_NIGHTMARE, HelixNightmareSpawnPos))
                            {
                                // CPP npc_deadmines_helix_nightmare: attackable 7.7 s in, spiders 18.4 s in (deadmines.cpp:466-517)
                                helixNmGuid = helix->GetGUID();
                                QueueTalk(helixNmGuid, SAY_NM_ENTERED_HELIX, 17800);
                                QueueTalk(helixNmGuid, SAY_NM_SPIDERS, 19200);
                                nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_UNFLAG, 7700);
                                nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_SPIDERS, 18400);
                                nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_SPIRIT_STRIKE, 4400);
                            }
                            break;
                        case EVENT_NM_HELIX_UNFLAG:
                            if (Creature* helix = GetNmCreature(helixNmGuid))
                            {
                                helix->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                                helix->SetReactState(REACT_AGGRESSIVE);
                                NmLog("helix: the nightmare Helix (49674) fights");
                            }
                            break;
                        case EVENT_NM_HELIX_SPIDERS:
                            if (Creature* helix = GetNmCreature(helixNmGuid))
                            {
                                for (Position const& pos : nightmareSkitterlingPos)
                                    if (Creature* spider = helix->SummonCreature(NPC_NIGHTMARE_SKITTERLING, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms))
                                        spiderGuids.insert(spider->GetGUID());
                                for (Position const& pos : DarkwebDevourerPos)
                                    if (Creature* spider = helix->SummonCreature(NPC_DARKWEB_DEVOURER, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms))
                                        spiderGuids.insert(spider->GetGUID());
                                if (Creature* spider = helix->SummonCreature(NPC_CHATTERING_HORROR, ChatteringHorrorPos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms))
                                    spiderGuids.insert(spider->GetGUID());
                                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] helix: %u nightmare spiders (t=%u s)", uint32(spiderGuids.size()), nmClock / 1000);
                            }
                            break;
                        case EVENT_NM_HELIX_SPIRIT_STRIKE:
                            if (Creature* helix = GetNmCreature(helixNmGuid))
                            {
                                if (helix->IsAlive() && helix->IsInCombat() && helix->GetVictim() && !helix->HasUnitState(UNIT_STATE_CASTING))
                                    helix->CastSpell(helix->GetVictim(), SPELL_SPIRIT_STRIKE, false);
                                nightmareEvents.ScheduleEvent(EVENT_NM_HELIX_SPIRIT_STRIKE, 3600);
                            }
                            break;
                        case EVENT_NM_REAPER_DESPAWN_SPIDERS:
                            DespawnNightmareSet(spiderGuids);
                            QueueTalk(announcerGuid, SAY_NM_NIGHTMARE_SHIFTS, 100);
                            NmStageAura(SPELL_NIGHTMARE_AURA_HELIX);
                            nightmareEvents.ScheduleEvent(EVENT_NM_REAPER_SETUP, 2600);
                            break;
                        case EVENT_NM_REAPER_SETUP:
                        {
                            if (GameObject* door = instance->GetGameObject(FoundryDoorGUID))
                                door->SetGoState(GO_STATE_ACTIVE);
                            if (Creature* intro = NmSummon(NPC_FOE_REAPER_5000_NIGHTMARE, FoeReaperNightmareIntroSpawnPos))
                                QueueDespawn(intro->GetGUID(), 10400);
                            if (Creature* vanessa = NmSummon(NPC_VANESSA_VANCLEEF_NIGHTMARE, vanessaVanCleefNightmareSpawnPos[2]))
                            {
                                vanessaNmGuid = vanessa->GetGUID();
                                QueueTalk(vanessaNmGuid, SAY_NM_MECHANICAL_1, 1000);
                                QueueTalk(vanessaNmGuid, SAY_NM_MECHANICAL_2, 4600);
                                QueueTalk(vanessaNmGuid, SAY_NM_ENTERED_MECHANICAL, 8200);
                                QueueDespawn(vanessaNmGuid, 10400);
                            }
                            SummonSparkChains();
                            nightmareEvents.ScheduleEvent(EVENT_NM_REAPER_SPAWN, 9500);
                            break;
                        }
                        case EVENT_NM_REAPER_SPAWN:
                            if (Creature* reaper = NmFighter(NPC_FOE_REAPER_5000_NIGHTMARE, FoeReaperNightmareSpawnPos))
                            {
                                reaper->RemoveAurasDueToSpell(SPELL_OFF_LINE);
                                reaper->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED | UNIT_FLAG_NON_ATTACKABLE);
                                reaperNmGuid = reaper->GetGUID();
                                NmLog("mechanical: the nightmare Foe Reaper (49681) fights");
                            }
                            break;
                        case EVENT_NM_RIPSNARL_AURA:
                            NmStageAura(SPELL_NIGHTMARE_AURA_MECHANICAL);
                            DespawnNightmareSet(sparkGuids);
                            QueueTalk(announcerGuid, SAY_NM_NIGHTMARE_SHIFTS, 150);
                            nightmareEvents.ScheduleEvent(EVENT_NM_RIPSNARL_SETUP, 2600);
                            break;
                        case EVENT_NM_RIPSNARL_SETUP:
                            if (Creature* intro = NmSummon(NPC_RIPSNARL_NIGHTMARE, RipsnarlNightmareIntroSpawnPos))
                                QueueDespawn(intro->GetGUID(), 16500);
                            if (Creature* vanessa = NmSummon(NPC_VANESSA_VANCLEEF_NIGHTMARE, vanessaVanCleefNightmareSpawnPos[3]))
                            {
                                vanessaNmGuid = vanessa->GetGUID();
                                QueueTalk(vanessaNmGuid, SAY_NM_RIPSNARL_1, 2200);
                                QueueTalk(vanessaNmGuid, SAY_NM_RIPSNARL_2, 9350);
                                QueueTalk(vanessaNmGuid, SAY_NM_ENTERED_RIPSNARL, 14150);
                                QueueDespawn(vanessaNmGuid, 16500);
                            }
                            nightmareEvents.ScheduleEvent(EVENT_NM_EMME, 14400);
                            break;
                        case EVENT_NM_EMME:
                            if (GameObject* door = instance->GetGameObject(IronCladDoorGUID))
                                door->SetGoState(GO_STATE_ACTIVE);
                            if (Creature* emme = NmSummon(NPC_EMME_HARRINGTON, EmmeHarringtonSpawnPos))
                            {
                                emmeGuid = emme->GetGUID();
                                SummonWorgen(EnragedWorgenEmmeSpawnPos, emme);
                            }
                            deadWorgen = 0;
                            QueueTalk(announcerGuid, SAY_NM_SAVE_EMME, 3500);
                            NmLog("ripsnarl: save Emme Harrington (3 Enraged Worgen)");
                            break;
                        case EVENT_NM_ERIK:
                            if (Creature* erik = NmSummon(NPC_ERIK_HARRINGTON, ErikHarringtonSpawnPos))
                            {
                                erikGuid = erik->GetGUID();
                                SummonWorgen(EnragedWorgenErikSpawnPos, erik);
                            }
                            deadWorgen = 0;
                            NmLog("ripsnarl: save Erik Harrington (3 Enraged Worgen)");
                            break;
                        case EVENT_NM_JAMES:
                        {
                            Creature* james = NmSummon(NPC_JAMES_HARRINGTON, JamesHarringtonSpawnPos);
                            Creature* calissa = NmSummon(NPC_CALISSA_HARRINGTON, CalissaHarringtonSpawnPos);
                            if (james && calissa)
                            {
                                jamesGuid = james->GetGUID();
                                calissaGuid = calissa->GetGUID();
                                james->SetReactState(REACT_PASSIVE);
                                calissa->EnterVehicle(james, 0, true);   // CPP: VEHICLE_SPELL_RIDE_HARDCODED on James (vehicle 1403)
                                calissa->SetInCombatWith(james);
                            }
                            QueueTalk(announcerGuid, SAY_NM_SAVE_CALISSA, 3400);
                            NmLog("ripsnarl: save Calissa Harrington (kill James before she chokes)");
                            break;
                        }
                        case EVENT_NM_VANESSA_BOSS:
                            if (Creature* vanessa = GetNmCreature(uiVanessaGUID))
                                vanessa->SetVisible(true);
                            DoStartCriteria(CRITERIA_START_TYPE_EVENT, NIGHTMARE_ACHIEVEMENT_EVENT);
                            NmLog("done: Vanessa VanCleef (49541) is visible");
                            break;
                        case EVENT_NM_CHECK_DEAD_PLAYERS:
                        {
                            uint32 alive = 0;
                            ForEachAlivePlayer([&alive](Player*) { ++alive; });
                            if (!alive && uiVanessaEvent >= NIGHTMARE_STAGE_TRAP)
                                ResetNightmare("every player is dead");
                            break;
                        }
                        case EVENT_NM_RESPAWN_NOTE:
                            if (uiVanessaEvent == NOT_STARTED && GetBossState(DATA_CAPTAIN) == DONE)
                            {
                                instance->SummonCreature(NPC_NOTE_FROM_VANESSA, notePos);
                                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] the Note from Vanessa is back");
                            }
                            break;
                        default:
                            break;
                    }
                }
            }

            // the mechanical nightmare: six Lightning Platters (CPP instance_deadmines.cpp:750-757) and their spark chains
            void SummonSparkChains()
            {
                for (uint8 i = 0; i < 6; ++i)
                {
                    Position const& base = LightningPlatterPos[i];
                    float const height = LightningPlatterCirclePathHeight[i];
                    bool const clockwise = urand(0, 1) == 1;
                    Position const platterPos(base.GetPositionX(), base.GetPositionY(), height, base.GetOrientation());
                    if (Creature* platter = NmSummon(NPC_VANESSA_LIGHTNING_PLATTER, platterPos, true))
                    {
                        sparkGuids.insert(platter->GetGUID());
                        platter->SetDisableGravity(true);
                        platter->GetMotionMaster()->MoveCirclePath(base.GetPositionX(), base.GetPositionY(), height, 1.0f, clockwise, 9, 0.4f);
                    }

                    float const lateral = base.GetOrientation() + float(M_PI) / 2.0f;
                    for (uint32 s = 0; s < NM_STALKER_COUNT; ++s)
                    {
                        float const d = NM_STALKER_OFFSETS[s];
                        Position const pos(base.GetPositionX() + d * std::cos(lateral), base.GetPositionY() + d * std::sin(lateral), height - 1.0f, 0.0f);
                        Creature* stalker = NmSummon(NPC_VANESSA_LIGHTNING_STALKER, pos, true);
                        if (!stalker)
                            continue;
                        sparkGuids.insert(stalker->GetGUID());
                        stalker->SetDisableGravity(true);
                        stalker->GetMotionMaster()->MoveCirclePath(base.GetPositionX(), base.GetPositionY(), height - 1.0f, std::fabs(d), clockwise, 24, std::fabs(d) * NM_PLATTER_OMEGA);
                    }
                }
                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] mechanical: %u spark platters + stalkers (t=%u s)", uint32(sparkGuids.size()), nmClock / 1000);
            }

            void NightmareUnitDeath(Unit* unit)
            {
                if (unit->GetTypeId() == TYPEID_PLAYER)
                {
                    nightmareEvents.ScheduleEvent(EVENT_NM_CHECK_DEAD_PLAYERS, 200);
                    return;
                }

                Creature* creature = unit->ToCreature();
                if (!creature)
                    return;

                ObjectGuid const guid = creature->GetGUID();
                switch (creature->GetEntry())
                {
                    case NPC_GLUBTOK_NIGHTMARE:
                        if (guid == glubtokNmGuid && uiVanessaEvent == NIGHTMARE_STAGE_GLUBTOK)
                        {
                            creature->DespawnOrUnsummon(4000);
                            SetNightmareStage(NIGHTMARE_STAGE_HELIX);
                        }
                        break;
                    case NPC_HELIX_GEARBREAKER_NIGHTMARE:
                        if (guid == helixNmGuid && uiVanessaEvent == NIGHTMARE_STAGE_HELIX)
                        {
                            creature->DespawnOrUnsummon(5500);
                            SetNightmareStage(NIGHTMARE_STAGE_FOE_REAPER);
                        }
                        break;
                    case NPC_FOE_REAPER_5000_NIGHTMARE:
                        if (guid == reaperNmGuid && uiVanessaEvent == NIGHTMARE_STAGE_FOE_REAPER)
                        {
                            creature->DespawnOrUnsummon(3600);
                            SetNightmareStage(NIGHTMARE_STAGE_RIPSNARL);
                        }
                        break;
                    case NPC_ENRAGED_WORGEN:
                        if (uiVanessaEvent != NIGHTMARE_STAGE_RIPSNARL)
                            break;
                        creature->DespawnOrUnsummon(5000);
                        if (++deadWorgen < 3)
                            break;
                        deadWorgen = 0;
                        if (Creature* emme = GetNmCreature(emmeGuid))
                        {
                            emme->CastSpell(emme, SPELL_ADRENALINE, true);
                            emme->DespawnOrUnsummon();
                            emmeGuid = ObjectGuid::Empty;
                            NmLog("ripsnarl: Emme Harrington saved");
                            QueueTalk(announcerGuid, SAY_NM_SAVE_ERIK, 4000);
                            nightmareEvents.ScheduleEvent(EVENT_NM_ERIK, 5800);
                        }
                        else if (Creature* erik = GetNmCreature(erikGuid))
                        {
                            erik->CastSpell(erik, SPELL_ADRENALINE, true);
                            erik->DespawnOrUnsummon();
                            erikGuid = ObjectGuid::Empty;
                            NmLog("ripsnarl: Erik Harrington saved");
                            nightmareEvents.ScheduleEvent(EVENT_NM_JAMES, 200);
                        }
                        break;
                    case NPC_JAMES_HARRINGTON:
                        if (guid != jamesGuid || uiVanessaEvent != NIGHTMARE_STAGE_RIPSNARL)
                            break;
                        creature->DespawnOrUnsummon(2300);
                        if (Creature* calissa = GetNmCreature(calissaGuid))
                        {
                            calissa->RemoveAllAuras();
                            calissa->DespawnOrUnsummon(2200);
                        }
                        NmLog("ripsnarl: James Harrington is dead, Calissa saved - the Nightmare Elixir wears off");
                        QueueTalk(announcerGuid, SAY_NM_ELIXIR_WEARS_OFF, 250);
                        CleanupNightmareKeepAnnouncer();
                        uiVanessaEvent = DONE;
                        SaveToDB();
                        nightmareEvents.ScheduleEvent(EVENT_NM_VANESSA_BOSS, 2250);
                        break;
                    case NPC_EMME_HARRINGTON:
                    case NPC_ERIK_HARRINGTON:
                    case NPC_CALISSA_HARRINGTON:
                        if (uiVanessaEvent == NIGHTMARE_STAGE_RIPSNARL)
                        {
                            creature->CastSpell(creature, SPELL_NIGHTMARES_END, true);
                            ResetNightmare(creature->GetEntry() == NPC_EMME_HARRINGTON ? "Emme Harrington died" :
                                           creature->GetEntry() == NPC_ERIK_HARRINGTON ? "Erik Harrington died" : "Calissa Harrington died");
                        }
                        break;
                    default:
                        break;
                }
            }

            // the end of the nightmare (CPP 92609 Cancel Nightmare Elixir): every gauntlet creature goes, except the announcer (it still
            // says "wears off") and James / Calissa (their own short despawn timers)
            void CleanupNightmareKeepAnnouncer()
            {
                static uint32 const playerAuras[] = { SPELL_NIGHTMARE_ELIXIR, SPELL_NIGHTMARE_AURA, SPELL_VANESSAS_BLACKOUT_AURA,
                    SPELL_NIGHTMARE_AURA_GLUBTOK, SPELL_NIGHTMARE_AURA_HELIX, SPELL_NIGHTMARE_AURA_MECHANICAL, SPELL_NIGHTMARE_AURA_RIPSNARL };
                for (uint32 spell : playerAuras)
                    DoRemoveAurasDueToSpellOnPlayers(spell);
                DespawnNightmareSet(spiderGuids);
                fireGuids.clear();
                sparkGuids.clear();
                for (ObjectGuid const& guid : nightmareGuids)
                {
                    if (guid == announcerGuid || guid == jamesGuid || guid == calissaGuid)
                        continue;
                    if (Creature* creature = GetNmCreature(guid))
                        creature->DespawnOrUnsummon();
                }
                nightmareGuids.clear();
                if (Creature* bunny = GetNmCreature(announcerGuid))
                    bunny->DespawnOrUnsummon(3000);
                nightmareEvents.Reset();
            }

            // a Steam Valve was clicked (npc_dm_steam_valve::OnSpellClick)
            void ValveActivated()
            {
                if (uiVanessaEvent != NIGHTMARE_STAGE_TRAP)
                    return;

                ++valvesActivated;
                TC_LOG_INFO("scripts", "[DM-NIGHTMARE] trap: steam valve %u/4 (t=%u s)", uint32(valvesActivated), nmClock / 1000);
                if (valvesActivated < 4)
                    return;

                for (ObjectGuid const& guid : nightmareGuids)
                    if (Creature* creature = GetNmCreature(guid))
                        if (creature->GetEntry() == NPC_STEAM_VALVE)
                            creature->DespawnOrUnsummon(10000);
                SetNightmareStage(NIGHTMARE_STAGE_GLUBTOK);
            }
            // ======================================== end #826 ========================================

            std::string GetSaveData() override
            {
                OUT_SAVE_INST_DATA;

                std::string str_data;
                std::ostringstream saveStream;
                saveStream << "D M " << GetBossSaveData() << State << " " << uiVanessaEvent << " ";
                str_data = saveStream.str();

                OUT_SAVE_INST_DATA_COMPLETE;
                return str_data;
            }

            void Load(const char* in) override
            {
                if (!in)
                {
                    OUT_LOAD_INST_DATA_FAIL;
                    return;
                }

                OUT_LOAD_INST_DATA(in);

                char dataHead1, dataHead2;

                std::istringstream loadStream(in);
                loadStream >> dataHead1 >> dataHead2;

                if (dataHead1 == 'D' && dataHead2 == 'M')
                {

                    for (uint8 i = 0; i < MAX_ENCOUNTER; ++i)
                    {
                        uint32 tmpState;
                        loadStream >> tmpState;
                        if (tmpState == IN_PROGRESS || tmpState > SPECIAL)
                        tmpState = NOT_STARTED;
                        SetBossState(i, EncounterState(tmpState));
                    }

                    loadStream >> State;

                    if (State == CANNON_BLAST_INITIATED)
                        if (GameObject *pIronCladDoor = instance->GetGameObject(IronCladDoorGUID))
                            pIronCladDoor->SetGoState(GO_STATE_ACTIVE_ALTERNATIVE);

                    loadStream >> uiVanessaEvent;
                    if (uiVanessaEvent != DONE)
                        uiVanessaEvent = NOT_STARTED;

                }
                else OUT_LOAD_INST_DATA_FAIL;

                OUT_LOAD_INST_DATA_COMPLETE;
            }

        private:
            ObjectGuid uiGlubtokGUID;
            ObjectGuid uiHelixGUID;
            ObjectGuid uiOafGUID;
            ObjectGuid uiFoereaperGUID;
            ObjectGuid uiAdmiralGUID;
            ObjectGuid uiCaptainGUID;

            ObjectGuid FactoryDoorGUID;
            ObjectGuid FoundryDoorGUID;
            ObjectGuid MastRoomDoorGUID;
            ObjectGuid GoblinTeleporterGUID;
            ObjectGuid HeavyDoorGUID;
            ObjectGuid HeavyDoor2GUID;
            ObjectGuid IronCladDoorGUID;
            ObjectGuid DefiasCannonGUID;
            ObjectGuid DoorLeverGUID;
            ObjectGuid DefiasPirate1GUID;
            ObjectGuid DefiasPirate2GUID;
            ObjectGuid DefiasCompanionGUID;

            uint32 State;
            uint32 uiVanessaEvent;
            uint32 OverchargeCount;

            // #826 Nightmare Gauntlet
            EventMap nightmareEvents;
            std::vector<NmTalk> nmTalks;
            uint32 nmClock;
            uint8 valvesActivated;
            uint8 deadWorgen;
            bool noteSpawnChecked;
            GuidSet nightmareGuids;
            GuidSet fireGuids;
            GuidSet spiderGuids;
            GuidSet sparkGuids;
            ObjectGuid uiVanessaGUID;
            ObjectGuid anchorBunnyGUID;
            ObjectGuid trapGuid;
            ObjectGuid announcerGuid;
            ObjectGuid vanessaNmGuid;
            ObjectGuid glubtokNmGuid;
            ObjectGuid helixNmGuid;
            ObjectGuid reaperNmGuid;
            ObjectGuid emmeGuid;
            ObjectGuid erikGuid;
            ObjectGuid calissaGuid;
            ObjectGuid jamesGuid;

            uint32 TeamInInstance;
        };

        InstanceScript* GetInstanceScript(InstanceMap* map) const override
        {
            return new instance_deadmines_InstanceMapScript(map);
        }
};

void AddSC_instance_deadmines()
{
    new instance_deadmines();
}