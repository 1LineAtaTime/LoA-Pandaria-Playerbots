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

#ifndef DEF_DEADMINES_H
#define DEF_DEADMINES_H

enum CannonState
{
    CANNON_NOT_USED,
    CANNON_BLAST_INITIATED,
    PIRATES_ATTACK,
    EVENT_DONE
};

enum NPCs
{
    NPC_GLUBTOK              = 47162,
    NPC_HELIX_GEARBREAKER    = 47296,
    NPC_LUMBERING_OAF        = 47297,
    NPC_FOE_REAPER_5000      = 43778,
    NPC_ADMIRAL_RIPSNARL     = 47626,
    NPC_CAPTAIN_COOKIE       = 47739,
    NPC_VANESSA_VANCLEEF     = 49541,
    NPC_VANESSA_SITTING      = 49429,
    NPC_NOTE_FROM_VANESSA    = 49564,
    // Horde npc
    NPC_KAGTHA               = 46889,
    NPC_SLINKY_SHARPSHIV     = 46906,
    NPC_MISS_MAYHEM          = 46902,
    NPC_MAYHEM_REAPER        = 46903,
    NPC_HAND_ASSASIN         = 46890,
    // Alliance npc
    NPC_HORATIO_LAINE        = 46612,
    NPC_INVESTIGATOR         = 46614,
    NPC_DEFENDER             = 50595,
    NPC_CRIME_SCENE_BOT      = 46613,
    // Trash
    NPC_MINING_POWDER        = 48284,
    NPC_DEFIAS_WATCHER       = 47403,
    NPC_DEFIAS_REAPER        = 47404,

    // Misc
    NPC_FROST_BLOSSOM_BUNNY  = 47284,
    NPC_FIRE_BLOSSOM_BUNNY   = 47282,

    // #826 Vanessa's Nightmare Gauntlet (heroic). Ids: The-Cataclysm-Preservation-Project TrinityCore 4.3.4 deadmines.h:73-129
    // (e503f96, 2025-08-02); every one exists in the MoP test world creature_template (no SmartAI, spawnMask 0 or no spawn).
    NPC_GENERAL_PURPOSE_DUMMY_JMF       = 45979,
    NPC_VANESSAS_TRAP_BUNNY             = 49454,   // VehicleId 1396: five seats hanging under the trap (VehicleSeat.dbc 9282-9286)
    NPC_STEAM_VALVE                     = 49457,   // npcflag SPELLCLICK (npc_spellclick_spells 90102)
    NPC_GLUBTOK_NIGHTMARE               = 49670,
    NPC_VANESSA_VANCLEEF_NIGHTMARE      = 49671,
    NPC_HELIX_GEARBREAKER_NIGHTMARE     = 49674,
    NPC_FOE_REAPER_5000_NIGHTMARE       = 49681,
    NPC_RIPSNARL_NIGHTMARE              = 49682,
    NPC_GLUBTOK_NIGHTMARE_FIRE_BUNNY    = 51594,   // addon aura 92169 Nightmare Flames (5 yd, every 1 s)
    NPC_COLLAPSING_ICICLE               = 49481,
    NPC_NIGHTMARE_SKITTERLING           = 49493,
    NPC_DARKWEB_DEVOURER                = 49494,
    NPC_CHATTERING_HORROR               = 49495,
    NPC_VANESSA_LIGHTNING_PLATTER       = 49520,   // addon aura 95519 Spark
    NPC_VANESSA_LIGHTNING_STALKER       = 49521,   // addon aura 92277 Spark (2 yd, every 0.3 s)
    NPC_ENRAGED_WORGEN                  = 49532,
    NPC_EMME_HARRINGTON                 = 49534,
    NPC_ERIK_HARRINGTON                 = 49535,
    NPC_CALISSA_HARRINGTON              = 49536,   // addon aura 92608 Choking (3% a second)
    NPC_JAMES_HARRINGTON                = 49539,   // VehicleId 1403: Calissa rides him
    NPC_VANESSA_ROPE                    = 49550,   // VehicleId 1399, spellclick 46598 (ride)
    NPC_VANESSAS_ROPE_ANCHOR            = 49552,
    NPC_FIREWALL_PLATTER_2A             = 48976,   // Glubtok heroic Fire Wall dummy (#681)
};

// #826 values of DATA_VANESSA_EVENT past the fork's NOT_STARTED / IN_PROGRESS (the intro) / FAIL / DONE (CPP deadmines.h:174-186)
enum NightmareStages
{
    NIGHTMARE_STAGE_TRAP        = 10,
    NIGHTMARE_STAGE_GLUBTOK     = 11,
    NIGHTMARE_STAGE_HELIX       = 12,
    NIGHTMARE_STAGE_FOE_REAPER  = 13,
    NIGHTMARE_STAGE_RIPSNARL    = 14,
};

enum eSpells
{
    SPELL_EXPLODE_MINE = 89769,
    SPELL_OFF_LINE     = 88348,
};

enum Data
{
    DATA_GLUBTOK          = 0,
    DATA_HELIX            = 1,
    DATA_FOEREAPER        = 2,
    DATA_ADMIRAL          = 3,
    DATA_CAPTAIN          = 4,
    DATA_VANESSA          = 5,
    DATA_CANNON_EVENT     = 6,
    DATA_OAF              = 7,
    DATA_VANESSA_EVENT    = 8,
    DATA_TEAM_IN_INSTANCE = 9,
    DATA_ACTIVATED_VALVE  = 10,  // #826: a Steam Valve was clicked (npc_dm_steam_valve)
    DATA_VANESSA_BOSS     = 11,  // #826: the boss 49541 (GetGuidData)
};

enum GameObjects
{
    GO_FOUNDRY_DOOR      = 16399,
    GO_FACTORY_DOOR      = 13965,
    GO_MAST_ROOM_DOOR    = 16400,
    GO_IRONCLAD_DOOR     = 16397,
    GO_DEFIAS_CANNON     = 16398,
    GO_DOOR_LEVER        = 101833,
    GO_HEAVY_DOOR        = 17153,
    GO_HEAVY_DOOR_2      = 17154,
    GO_GOBLIN_TELEPORTER = 208002,
};

enum Sounds
{
    SOUND_CANNONFIRE  = 1400,
    SOUND_DESTROYDOOR = 3079,
};

enum Worldstates
{
    WORLDSTATE_PROTOTYPE_PRODIGY    = 8938,
    WORLDSTATE_VANCLEEF_VINDICATION = 8993,
};

const Position centershipPos = {-63.167f, -819.315f, 41.27f, 6.25f};
const Position notePos = {-74.3611f, -820.014f, 40.3714f, 4.01426f};
const Position vanessaPos = {-75.5851f, -819.964f, 47.0673f, 6.17846f};

// #681 Glubtok heroic Fire Wall: the Glubtok Firewall Platter's spawn (MoP test world creature 326770, entry 48974) is the wall's
// pivot. CPP boss_glubtok.cpp:107-121 drives a vehicle platter on a 0.8 yd cyclic path at 0.25 yd/s (about 20 s a turn, clockwise),
// its seat offsets (5.4.8 VehicleSeat.dbc: vehicle 1365 seats at 6/18/30/42/-6/-18/-30 yd, each level-1 vehicle 1366/1371/1372 with
// seats at +-1.5/+-4.5 yd) carrying the Fire Wall dummies. This test DB has no vehicle_template_accessory rows for them, so the
// script places the dummies itself on those offsets and turns them around the pivot at the same rate.
const Position fireWallPivotPos = { -193.287f, -441.953f, 54.5703f, 1.833041f };
float const FIRE_WALL_PERIOD = 20.0f;  // seconds a turn; the bots' "dm fire wall" edge (CataDmShared.h) uses the same value

// #826 Nightmare Gauntlet positions: CPP deadmines.h:194-399 and deadmines.cpp:109-120 (e503f96)
const Position trapBunnyPos           = { -205.757f, -579.097f, 42.9862f, 0.0f };   // MoP test DB creature 326857 (spawnMask 0)
const Position trapBunnyMoveDownPos   = { -205.7569f, -579.0972f, 35.98623f, 0.0f };
const Position magmaTrapThrowPos      = { -225.1973f, -563.7677f, 51.23737f, 0.0f }; // CPP deadmines.cpp:617 (sniffed landing)
const Position steamValvePos[4] =
{
    { -199.1944f, -583.1545f, 37.33158f, 2.600541f },
    { -201.809f,  -572.6215f, 37.33158f, 4.171337f },
    { -209.6823f, -585.8924f, 37.33158f, 0.9599311f },
    { -212.3524f, -575.3976f, 37.33158f, 5.707227f },
};

template<class AI>
CreatureAI* GetInstanceAI(Creature* creature)
{
    if (InstanceMap* instance = creature->GetMap()->ToInstanceMap())
        if (instance->GetInstanceScript())
            if (instance->GetScriptId() == sObjectMgr->GetScriptId("instance_deadmines"))
                return new AI(creature);
    return NULL;
}

#endif