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
* You should have received a copy of the GNU General Public License
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Hillsbrad Foothills: Brazie the Botanist's "Plants vs. Ghouls" minigame at Brazie's Farmstead.
//  - Accepting one of Brazie's quests makes the player cast Create Botanist's Vehicle (91683): the player rides Brazie (49176)
//    and plants on the lawn from the vehicle's action bar with Solar Power, while zombies walk down five lanes towards him.
//  - Basic Botany (28733): tutorial, spitters in the center lane. Flower Power (28617): sunflowers, spitters, ghouls.
//  - Ghouls Hate My Grains (28744): rocknuts; each of two massive waves ends with a ghoul dropping Strangler Vine seeds.
//  - Someone Setup the Pumpkin Bomb (28747): pumpkin bombs; each of two massive waves ends with a zombie dropping Freezya seeds.
//  - Lawn of the Dead (28748): freezyas; after two waves Warden Stillwater comes down the lawn himself, raising obelisks.
//  - Tending the Garden (28751, daily): every plant, two massive waves with abominations.
//  - A zombie that reaches the end of its lane eats the player's brains: the quest fails, the player is put off the vehicle.
//    Leaving the seat, dying or moving away fails the quest (abandon it and take it again); after a logout, or a quest a party
//    member shared, Brazie's gossip option starts the game while the quest is in progress.
//
// Levels count as TCPP's: 0 the tutorial, 1 Flower Power, 2 Ghouls Hate My Grains, 3 Pumpkin Bomb, 4 Lawn of the Dead. (The
// kill credit creatures count from 1: "PvG Level 4 - Someone Setup the Pumpkin Bomb".)
//
// Levels 0 and 1 are ported from TCPP (CataclysmPreservationProject, zone_hillsbrad_foothills.cpp and
// custom_2018_06_22_00_world.sql): the lane layout, their timings and wave compositions, the plant, zombie and lawnmower
// behaviour. Differences from TCPP, forced by this core:
//  - no phase 365: every actor is the player's private summon, and private summons of one player see each other here.
//  - Create Empty Spot (91676) and Create Random Sun Power (91741) are server-side spells missing from the 5.4.8 DBC (TCPP
//    adds them to spell_dbc): their summon effect is done in C++ (SummonPrivate), with their summon properties (3077).
//  - the highlight spells (92276, 94032, 94030) have a 0 yard range in the DBC (TCPP widens it in SpellMgr): their aura is
//    added and their "Place it here!" marker summoned in C++.
//  - the vehicle gets no natural energy regeneration (72242 Zero Energy + Zero Regen), as with TCPP's unit_flags2 0: Solar Power
//    only comes from the suns. The vehicle starts with 100 Solar Power, as TCPP's Vehicle::Install gives it.
//  - TCPP's level 1 drops Freezya seeds from the first damaged ghoul. In 5.4.8 that drop spell (92435) gives the kill credit of
//    Someone Setup the Pumpkin Bomb (49561), so the Freezya drop belongs to level 3 here and Flower Power has none.
//  - creature relocation notifications are too rare for a 1 yard/s zombie: plants scan their lane for a target, zombies look
//    for a plant ahead and the lawnmowers for a zombie (starting at 6 yards, TCPP 3) on a timer. Nothing evades back home.
//
// Levels 2-4 and the daily have no script anywhere (no sniff records their waves). Sourced: the plant, seed, boss and summon
// spells with their effects, summon properties and progress bars (5.4.8 DBC), the kill credits (spell effects and quest
// objectives), every text (broadcast_text: the tips, the drops, "Rocknut / Pumpkin Bombs unlocked", Stillwater's lines), the
// creatures (Rocknut and Broken Rocknut, Strangler Vine, Pumpkin Bomb, Abomination, Warden Stillwater and his Obelisk with
// their growl spells). Ours: every wave composition and timing of levels 2-4 and the daily (WaveGroups below), the stage
// breaks, which plants each level has, what the plants do with their spells (Strangle, Pumpkin Explosion), the damage factors
// of abominations and Stillwater, Stillwater's escort and obelisks (his template spells are Place Obelisk; here an obelisk
// raises a zombie into its lane on a timer), and the reuse of "Good job!" for the daily's waves.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "CombatAI.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellInfo.h"
#include "Spell.h"
#include "Vehicle.h"
#include "TemporarySummon.h"
#include "ObjectAccessor.h"
#include "GridNotifiers.h"
#include "DBCStores.h"
#include "WorldSession.h"

namespace HillsbradBrazie
{
enum PlantsVsGhouls
{
    // Quests and their kill credits
    QUEST_BASIC_BOTANY                  = 28733,
    CREDIT_BASIC_BOTANY                 = 49189,
    QUEST_FLOWER_POWER                  = 28617,
    CREDIT_FLOWER_POWER                 = 49542,
    QUEST_GHOULS_HATE_MY_GRAINS         = 28744,    // credit 49553 x2: Create Random Strangler Vine (92340) effect 1
    QUEST_SOMEONE_SETUP_THE_PUMPKIN_BOMB = 28747,   // credit 49561 x2: Create Random Freezya Sack (92435) effect 1
    QUEST_LAWN_OF_THE_DEAD              = 28748,
    CREDIT_LAWN_OF_THE_DEAD             = 49627,
    QUEST_TENDING_THE_GARDEN            = 28751,
    CREDIT_TENDING_THE_GARDEN           = 49626,

    // Progress bars (alternate power)
    SPELL_PROGRESS_BAR_4                = 92259,    // bar 32, "Wave Progress", max 4 (TCPP: tutorial and level 1)
    SPELL_PROGRESS_BAR_5                = 91651,    // bar 30, max 5 (ours: the daily)
    SPELL_PROGRESS_BAR_7                = 92321,    // bar 33, max 7 (ours: level 2)
    SPELL_PROGRESS_BAR_7_PHASE          = 92597,    // bar 35, max 7 (ours: level 3)
    SPELL_PROGRESS_BAR_STILLWATER       = 92598,    // bar 36, "Warden Stillwater", max 35000; triggered by 92767

    // Brazie the Botanist (vehicle 49176)
    SPELL_CREATE_LAWNMOWER              = 91680,
    SPELL_CREATE_SPITTER_TARGET         = 91988,
    SPELL_CREATE_GOAL_STALKER           = 91740,
    SPELL_SPITTER_HIGHLIGHT             = 92276,
    SPELL_SUNFLOWER_HIGHLIGHT           = 94032,
    SPELL_ROCKNUT_HIGHLIGHT             = 94030,
    SPELL_ZERO_ENERGY_REGEN             = 72242,    // Zero Energy + Zero Regen: aura 110 -100% energy regeneration

    SPELL_PLANT_SUNFLOWER               = 91646,
    SPELL_PLANT_SPITTER                 = 91649,
    SPELL_PLANT_ROCKNUT                 = 91704,
    SPELL_PLANT_STRANGLER_VINE          = 91710,
    SPELL_PLANT_PUMPKIN_BOMB            = 92157,
    SPELL_PLANT_FREEZYA                 = 92440,

    // Cast by the player
    SPELL_CREATE_BOTANISTS_VEHICLE      = 91683,
    SPELL_CREATE_ZOMBIE                 = 91739,
    SPELL_CREATE_GHOUL                  = 91834,
    SPELL_CREATE_RANDOM_STRANGLER_VINE  = 92340,
    SPELL_GET_STRANGLER_SEEDS           = 92339,
    SPELL_CREATE_RANDOM_FREEZYA_SACK    = 92435,
    SPELL_GET_FREEZYA_SEEDS             = 92434,
    SPELL_CREATE_WARDEN_STILLWATER      = 92767,
    SPELL_CREATE_OBELISK                = 93253,

    NPC_BRAZIE_VEHICLE                  = 49176,
    NPC_SPITTER_TARGET                  = 49385,
    NPC_EMPTY_SPOT                      = 49190,
    NPC_PLACE_IT_HERE                   = 51098,
    NPC_SOLAR_POWER                     = 49194,
    NPC_SPITTER                         = 49200,
    NPC_SUNFLOWER                       = 49193,
    NPC_ROCKNUT                         = 49205,
    NPC_STRANGLER_VINE                  = 49207,
    NPC_PUMPKIN_BOMB                    = 49474,
    NPC_FREEZYA                         = 49618,
    NPC_STRANGLER_VINE_SEEDS            = 49560,
    NPC_FREEZYA_SEEDS                   = 49604,
    NPC_ZOMBIE                          = 49209,
    NPC_GHOUL                           = 49232,
    NPC_ABOMINATION                     = 49694,
    NPC_WARDEN_STILLWATER               = 49819,
    NPC_OBELISK                         = 49911,
    NPC_GOAL_STALKER                    = 49210,
    NPC_FERTILIZE_O_TRON_2000           = 49192,

    DISPLAY_BROKEN_ROCKNUT              = 36607,    // Broken Rocknut (49761)

    // Server-side spells missing from the 5.4.8 DBC, done in C++ (summon properties of TCPP's spell_dbc rows)
    SPELL_CREATE_EMPTY_SPOT             = 91676,
    SPELL_CREATE_RANDOM_SUN_POWER       = 91741,
    SUMMON_PROPERTIES_PERSONAL          = 3077,     // SUMMON_PROP_FLAG_PERSONAL_SPAWN
    SERVERSIDE_SUMMON_DURATION          = 1800000,  // TCPP's DurationIndex 30

    // Plants
    SPELL_VENOM_SPIT                    = 91748,
    SPELL_SUNLIGHT                      = 93968,
    SPELL_FREEZYA_BLAST                 = 92441,
    SPELL_STRANGLE                      = 91791,    // 10% of the target's health per second
    SPELL_PUMPKIN_EXPLOSION             = 92159,    // 2 s cast, 3500 + 3500 in 13 and 20 yards

    // Solar Power
    SPELL_GAIN_SOLAR_POWER_SPELLCLICK   = 91687,
    SPELL_GAIN_SOLAR_POWER_ENERGIZE     = 94068,

    // Zombies
    SPELL_ZOMBIE_GROWL                  = 93961,
    SPELL_LEAKING_GUTS                  = 93244,
    SPELL_GHOUL_GROWL                   = 93962,
    SPELL_BUCKET_HEAD                   = 93230,
    SPELL_BUCKET                        = 93236,
    SPELL_ABOMINATION_GROWL             = 93963,
    SPELL_BOSS_GROWL                    = 93965,

    // Fertilize-o-tron 2000
    SPELL_LAWNMOWER_STARTUP_SOUND       = 93969,
    SPELL_DAZED                         = 1604,
    SPELL_PLAGUE_GAS                    = 92084,

    // Whispers of the player to themselves (broadcast_text)
    SAY_ANNOUNCE_TUTORIAL_1             = 49559,    // Collect Solar Power and place Spitters in the center lane...
    SAY_ANNOUNCE_TUTORIAL_2             = 49553,    // Good job!
    SAY_ANNOUNCE_TUTORIAL_3             = 49558,    // Keep placing spitters in the center lane...
    SAY_ANNOUNCE_TUTORIAL_4             = 49552,    // Tutorial Completed! You've unlocked the Sunflower.
    SAY_ANNOUNCE_MASSIVE_WAVE           = 49409,    // A Massive Wave of Zombies is Approaching!
    SAY_ANNOUNCE_PLANT_SUNFLOWER        = 49566,
    SAY_ANNOUNCE_PLANT_MORE_SUNFLOWERS  = 49567,
    SAY_ANNOUNCE_LEVEL_1_COMPLETE       = 49551,    // You survived the attack! Rocknut Unlocked!
    SAY_ANNOUNCE_USE_ROCKNUTS           = 49600,    // Use Rocknuts to protect your Spitters and Sunflowers.
    SAY_ANNOUNCE_GHOUL_DROPPED          = 49635,    // A Ghoul dropped something onto the field!
    SAY_ANNOUNCE_LEVEL_2_COMPLETE       = 49678,    // You defended against the attack! Pumpkin Bombs unlocked!
    SAY_ANNOUNCE_USE_PUMPKIN_BOMBS      = 49747,
    SAY_ANNOUNCE_ZOMBIE_DROPPED         = 49685,    // A zombie dropped something onto the field!
    SAY_ANNOUNCE_BUILD_DEFENSES         = 49984,    // Build some defenses before Warden Stillwater arrives!
    SAY_ANNOUNCE_SLAY_STILLWATER        = 49985,    // Slay Warden Stillwater before you're overwhelmed!

    // creature_text of the vehicle
    SAY_GAINED_FREEZYA_SEEDS            = 0,
    SAY_GAINED_STRANGLER_SEEDS          = 1,
    SAY_ZOMBIES_ATE_YOUR_BRAINS         = 2,

    // creature_text of Warden Stillwater
    SAY_STILLWATER_ARRIVES              = 0,        // Enough fooling around. It's time for some hands-on management!
    SAY_STILLWATER_DIES                 = 1,        // What?! No! This lawn... was... mine...

    // Vehicle events
    EVENT_SPITTER_HIGHLIGHT             = 1,
    EVENT_SUMMON_SOLAR_POWER,
    EVENT_CREATE_ZOMBIE,
    EVENT_CREATE_GHOUL,
    EVENT_ANNOUNCE_GOOD_JOB,
    EVENT_ANNOUNCE_PLACING_SPITTERS,
    EVENT_ANNOUNCE_MASSIVE_WAVE,
    EVENT_ANNOUNCE_LEVEL_COMPLETED,
    EVENT_EJECT_PASSENGER,
    EVENT_SUNFLOWER_HIGHLIGHT,
    EVENT_ANNOUNCE_PLANT_MORE_FLOWERS,
    EVENT_CHECK_PASSENGER,
    EVENT_ROCKNUT_HIGHLIGHT,
    EVENT_LEVEL_TIP,
    EVENT_WAVE_ZOMBIE,
    EVENT_WAVE_GHOUL,
    EVENT_WAVE_ABOMINATION,
    EVENT_START_WAVE_STAGE,
    EVENT_SUMMON_STILLWATER,
    EVENT_STILLWATER_ESCORT,
    EVENT_PLACE_OBELISK,

    // Actor events
    EVENT_MOVE_TO_GOAL_POSITION         = 1,
    EVENT_CHECK_VICTIM_STATE,
    EVENT_SUNLIGHT,
    EVENT_KILL_LANE,
    EVENT_PLAGUE_GAS,
    EVENT_CHECK_LANE,
    EVENT_CHECK_VEHICLE,
    EVENT_PUMPKIN_EXPLODED,
    EVENT_RAISE_ZOMBIE,

    // Actions sent to the vehicle
    ACTION_FIRST_SPITTER_PLANTED        = 1,
    ACTION_FIRST_SUNFLOWER_PLANTED,
    ACTION_ZOMBIE_DAMAGED,
    ACTION_ZOMBIE_DIED,
    ACTION_GHOUL_DAMAGED,
    ACTION_QUEST_FAILED,
    ACTION_FIRST_ROCKNUT_PLANTED,
    ACTION_STILLWATER_DIED,

    MAX_LANES                           = 5,
    MAX_EMPTY_SPOT_POSITIONS            = 40,
    CENTER_LANE                         = 2,
    ROCKNUT_HIGHLIGHT_SPOT              = 21,       // ours: fourth column of the center lane, in front of the spitters

    POINT_GOAL_STALKER                  = 1,
    POINT_LANE_END                      = 2,

    LEVEL_NONE                          = 0,
    LEVEL_TUTORIAL,
    LEVEL_1,
    LEVEL_2,
    LEVEL_3,
    LEVEL_4,
    LEVEL_DAILY
};

// The five lanes run from the Spitter Targets (where the zombies appear) west to the Goal Stalkers next to Brazie. (TCPP)
Position const LawnmowerPositions[MAX_LANES] =
{
    { -116.5816f, 237.6823f, 53.2755f },
    { -116.2934f, 228.9896f, 53.2755f },
    { -116.0833f, 220.7691f, 53.2755f },
    { -116.2674f, 213.0243f, 53.2755f },
    { -115.9132f, 204.6198f, 53.2755f }
};

Position const GoalStalkerPositions[MAX_LANES] =
{
    { -116.5816f, 237.6823f, 53.2755f },
    { -116.2934f, 228.9896f, 53.2755f },
    { -116.0833f, 220.7691f, 53.2755f },
    { -116.2674f, 213.0243f, 53.2755f },
    { -115.9132f, 204.6198f, 53.2755f }
};

Position const SpitterTargetPositions[MAX_LANES] =
{
    { -39.83333f, 237.441f,  53.27685f },
    { -40.13715f, 229.6163f, 53.33047f },
    { -40.10764f, 221.4618f, 53.27604f },
    { -40.06424f, 212.9444f, 53.3602f  },
    { -40.14236f, 205.0538f, 53.27604f }
};

// Eight columns of five spots; index 0 is the first spot of the center lane, where the tutorial highlights go. (TCPP)
Position const EmptySpotPositions[MAX_EMPTY_SPOT_POSITIONS] =
{
    { -108.7604f, 221.0226f, 53.35883f },
    { -108.3733f, 212.9219f, 53.35883f },
    { -108.3108f, 229.2309f, 53.35883f },
    { -107.9792f, 203.8629f, 53.35883f },
    { -100.1684f, 220.6615f, 53.35883f },
    { -99.92188f, 212.1406f, 53.35883f },
    { -99.9375f,  204.8385f, 53.35883f },
    { -100.0399f, 229.7882f, 53.35883f },
    { -108.5486f, 237.4844f, 53.35883f },
    { -91.59549f, 220.8715f, 53.35883f },
    { -91.51563f, 212.7379f, 53.35883f },
    { -100.3385f, 237.2656f, 53.35883f },
    { -91.68056f, 229.1997f, 53.35883f },
    { -91.32291f, 204.5434f, 53.35883f },
    { -91.77604f, 237.5816f, 53.35883f },
    { -83.93403f, 213.066f,  53.35883f },
    { -83.21354f, 220.8542f, 53.35883f },
    { -83.61459f, 204.592f,  53.35883f },
    { -83.80903f, 229.6788f, 53.35883f },
    { -83.52257f, 237.217f,  53.35883f },
    { -75.24827f, 213.3108f, 53.35883f },
    { -75.00174f, 220.9861f, 53.35883f },
    { -75.34375f, 204.5382f, 53.35883f },
    { -75.25521f, 229.5278f, 53.35883f },
    { -75.10416f, 238.0816f, 53.35883f },
    { -66.7934f,  213.071f,  53.35883f },
    { -66.5469f,  220.747f,  53.35883f },
    { -66.8889f,  204.299f,  53.35883f },
    { -66.8003f,  229.288f,  53.35883f },
    { -66.6493f,  237.842f,  53.35883f },
    { -58.4965f,  212.372f,  53.35883f },
    { -58.25f,    220.047f,  53.35883f },
    { -58.5035f,  228.589f,  53.35883f },
    { -58.592f,   203.599f,  53.35883f },
    { -58.3524f,  237.142f,  53.35883f },
    { -49.6059f,  212.569f,  53.35883f },
    { -49.3594f,  220.245f,  53.35883f },
    { -49.6128f,  228.786f,  53.35883f },
    { -49.7014f,  203.797f,  53.35883f },
    { -49.4618f,  237.34f,   53.35883f }
};

float const EmptySpotOrientation     = 6.143559f;
float const LawnmowerOrientation     = 6.178465f;
float const SpitterTargetOrientation = 6.232892f;
float const GoalStalkerOrientation   = 6.178465f;

Position const SolarPowerJumpPos     = { -64.24827f, 181.3004f, 53.97961f };
Position const SeedSackJumpPos       = { -116.2847f, 216.7865f, 53.2755f };

float const SolarPowerDropHeight     = 50.0f;   // TCPP: the suns appear 50 yards above the lawn (they fall down here)
uint32 const SolarPowerDuration      = 18000;   // ours: an unclicked sun fades after 18 s (TCPP's 30 minutes let them pile up)
float const MaxVehicleDistance       = 40.0f;   // ours: the vehicle is rooted; a passenger moved further away ends the game
float const MowerTriggerDistance     = 6.0f;
float const ZombieBiteDistance       = 3.0f;    // TCPP: zombies eat plants up to 3 yards (between their bounding radii) ahead
float const TrapTriggerDistance      = 6.0f;    // ours: Strangler Vines and Pumpkin Bombs act on what comes this close
float const ObeliskColumnX           = -44.5f;  // ours: obelisks rise between the last column of spots and the lane ends
uint32 const ObeliskHealth           = 1500;    // ours (the template's 10 x level 20 health would outlast the fight)

// Ours: the waves of levels 2-4 and the daily (no sniff records them). A level is a list of stages; a stage ends when all its
// attackers are dead, the next starts after a short break. Delays and intervals are in ms from the start of the stage.
struct WaveGroup
{
    uint8 Stage;
    uint32 Entry;
    uint8 Count;
    uint32 Delay;
    uint32 Interval;
};

WaveGroup const Level2Waves[] =
{
    { 1, NPC_ZOMBIE, 3, 20000, 9000 },  { 1, NPC_GHOUL, 1, 38000, 0 },
    { 2, NPC_ZOMBIE, 3, 4000, 7000 },   { 2, NPC_GHOUL, 2, 10000, 10000 },
    { 3, NPC_ZOMBIE, 6, 2000, 3000 },   { 3, NPC_GHOUL, 3, 6000, 5000 },        // massive wave 1
    { 4, NPC_ZOMBIE, 3, 6000, 6000 },   { 4, NPC_GHOUL, 2, 12000, 8000 },
    { 5, NPC_ZOMBIE, 3, 4000, 5000 },   { 5, NPC_GHOUL, 2, 8000, 7000 },
    { 6, NPC_ZOMBIE, 7, 2000, 3000 },   { 6, NPC_GHOUL, 4, 5000, 4000 },        // massive wave 2
};

WaveGroup const Level3Waves[] =
{
    { 1, NPC_ZOMBIE, 4, 20000, 7000 },  { 1, NPC_GHOUL, 1, 34000, 0 },
    { 2, NPC_ZOMBIE, 4, 4000, 5000 },   { 2, NPC_GHOUL, 2, 10000, 8000 },
    { 3, NPC_ZOMBIE, 8, 2000, 2500 },   { 3, NPC_GHOUL, 3, 5000, 5000 },        // massive wave 1
    { 4, NPC_ZOMBIE, 4, 6000, 5000 },   { 4, NPC_GHOUL, 3, 10000, 6000 },
    { 5, NPC_ZOMBIE, 9, 2000, 2500 },   { 5, NPC_GHOUL, 4, 4000, 4000 },        // massive wave 2
};

WaveGroup const Level4Waves[] =
{
    { 1, NPC_ZOMBIE, 4, 25000, 6000 },  { 1, NPC_GHOUL, 2, 35000, 8000 },
    { 2, NPC_ZOMBIE, 5, 4000, 4000 },   { 2, NPC_GHOUL, 2, 8000, 8000 },   { 2, NPC_ABOMINATION, 1, 15000, 0 },
};

WaveGroup const DailyWaves[] =
{
    { 1, NPC_ZOMBIE, 4, 20000, 6000 },  { 1, NPC_GHOUL, 2, 30000, 8000 },
    { 2, NPC_ZOMBIE, 8, 2000, 2500 },   { 2, NPC_GHOUL, 4, 5000, 4000 },   { 2, NPC_ABOMINATION, 1, 12000, 0 },     // massive 1
    { 3, NPC_ZOMBIE, 5, 5000, 5000 },   { 3, NPC_GHOUL, 2, 10000, 8000 },
    { 4, NPC_ZOMBIE, 9, 2000, 2000 },   { 4, NPC_GHOUL, 5, 4000, 3000 },   { 4, NPC_ABOMINATION, 2, 10000, 12000 }, // massive 2
};

struct LevelWaves
{
    WaveGroup const* Groups;
    size_t GroupCount;
    uint8 Stages;
    uint32 MassiveStages;   // bit (1 << stage)
};

LevelWaves GetLevelWaves(uint8 level)
{
    switch (level)
    {
        case LEVEL_2:     return { Level2Waves, std::size(Level2Waves), 6, (1 << 3) | (1 << 6) };
        case LEVEL_3:     return { Level3Waves, std::size(Level3Waves), 5, (1 << 3) | (1 << 5) };
        case LEVEL_4:     return { Level4Waves, std::size(Level4Waves), 2, 0 };    // then Warden Stillwater
        case LEVEL_DAILY: return { DailyWaves, std::size(DailyWaves), 4, (1 << 2) | (1 << 4) };
        default:          return { nullptr, 0, 0, 0 };
    }
}

// The lane a position lies in: the one whose Spitter Target is closest in y.
uint8 GetLaneIndex(float y)
{
    uint8 lane = 0;
    for (uint8 i = 1; i < MAX_LANES; ++i)
        if (std::fabs(SpitterTargetPositions[i].GetPositionY() - y) < std::fabs(SpitterTargetPositions[lane].GetPositionY() - y))
            lane = i;
    return lane;
}

bool IsPlantEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_SPITTER:
        case NPC_SUNFLOWER:
        case NPC_FREEZYA:
        case NPC_ROCKNUT:
        case NPC_STRANGLER_VINE:
        case NPC_PUMPKIN_BOMB:
            return true;
        default:
            return false;
    }
}

// The undead that walk down the lanes
bool IsZombieEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_ZOMBIE:
        case NPC_GHOUL:
        case NPC_ABOMINATION:
        case NPC_WARDEN_STILLWATER:
            return true;
        default:
            return false;
    }
}

// Everything the plants fight
bool IsAttackerEntry(uint32 entry)
{
    return IsZombieEntry(entry) || entry == NPC_OBELISK;
}

// Summon a minigame actor as the server-side spells would have: the player's private summon with summon properties 3077.
TempSummon* SummonPrivate(Player* player, uint32 entry, Position const& pos, uint32 duration = SERVERSIDE_SUMMON_DURATION)
{
    return player->GetMap()->SummonCreature(entry, pos, sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_PERSONAL),
        Milliseconds(duration), player, 0, 0, player->GetGUID());
}

Player* GetSummoningPlayer(Creature* actor)
{
    if (TempSummon* summon = actor->ToTempSummon())
        return Object::ToPlayer(summon->GetSummoner());
    return nullptr;
}

// The quest whose game Brazie starts, for the vehicle his cast summons (the summon happens within the cast)
struct StartingGame
{
    ObjectGuid PlayerGUID;
    uint32 QuestId = 0;
};
thread_local StartingGame BrazieStartingGame;

// Every actor is summoned by the player while they ride Brazie: the player's vehicle is the game's controller.
Creature* GetBotanistVehicle(Creature* actor)
{
    Player* player = GetSummoningPlayer(actor);
    if (!player)
        return nullptr;

    Creature* vehicle = player->GetVehicleCreatureBase();
    if (!vehicle || vehicle->GetEntry() != NPC_BRAZIE_VEHICLE || !vehicle->IsAIEnabled)
        return nullptr;

    return vehicle;
}

void NotifyVehicle(Creature* actor, int32 action)
{
    if (Creature* vehicle = GetBotanistVehicle(actor))
        vehicle->AI()->DoAction(action);
}

void RegisterWithVehicle(Creature* actor)
{
    if (Creature* vehicle = GetBotanistVehicle(actor))
        vehicle->AI()->JustSummoned(actor);
}

enum BrazieVehicle
{
    EVENT_BOARD_TIMEOUT                 = 100   // ours: a vehicle nobody boarded (the ride failed) despawns
};

// 49176 - Brazie the Botanist (the player's vehicle)
struct npc_brazie_the_botanist_vehicle : public VehicleAI
{
    npc_brazie_the_botanist_vehicle(Creature* creature) : VehicleAI(creature), _summons(creature), _level(LEVEL_NONE),
        _deadZombieCount(0), _damagedZombieCount(0), _damagedGhoulCount(0), _currentStage(0), _timeUntilNextSolarPower(0),
        _pendingSpawns(0), _aliveAttackers(0), _stageRunning(false), _boarded(false), _finished(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _events.ScheduleEvent(EVENT_BOARD_TIMEOUT, 5s);

        Player* player = summoner->ToPlayer();
        if (!player)
            return;

        _playerGUID = player->GetGUID();
        _level = SelectLevel(player);

        // Solar Power is the vehicle's energy (Vehicle 1382, power display 166 POWER_TYPE_SUN_POWER)
        me->SetPowerType(POWER_ENERGY);
        me->SetMaxPower(POWER_ENERGY, 100);
        me->SetPower(POWER_ENERGY, 100);
        me->AddAura(SPELL_ZERO_ENERGY_REGEN, me);

        // The action bar is sent when the player takes the seat
        SetupActionBar();
    }

    void JustSummoned(Creature* summon) override
    {
        _summons.Summon(summon);

        switch (summon->GetEntry())
        {
            case NPC_EMPTY_SPOT:
                _spotGUIDs.push_back(summon->GetGUID());
                break;
            case NPC_ZOMBIE:
            case NPC_GHOUL:
            case NPC_ABOMINATION:
                ++_aliveAttackers;
                break;
            case NPC_WARDEN_STILLWATER:
                _stillwaterGUID = summon->GetGUID();
                Whisper(SAY_ANNOUNCE_SLAY_STILLWATER);
                break;
            default:
                break;
        }
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        Player* player = passenger->ToPlayer();
        if (!player || player->GetGUID() != _playerGUID)
            return;

        if (apply)
        {
            if (_boarded)
                return;

            _boarded = true;
            if (_level == LEVEL_NONE)
            {
                _finished = true;
                _events.ScheduleEvent(EVENT_EJECT_PASSENGER, 500ms);
                return;
            }

            if (uint32 progressBar = GetProgressBarSpell())
                player->CastSpell(player, progressBar, true);
            SetupSpawns(player);
            StartLevel();
            _events.ScheduleEvent(EVENT_CHECK_PASSENGER, 1s);
        }
        else
            EndGame(player);
    }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        Player* player = caster ? caster->ToPlayer() : nullptr;
        if (!player || player->GetGUID() != _playerGUID || _finished)
            return;

        // Seeds picked up from a drop: the plant joins the action bar for the rest of the level
        switch (spell->Id)
        {
            case SPELL_GET_FREEZYA_SEEDS:
                AddPlantToBar(SPELL_PLANT_FREEZYA);
                Talk(SAY_GAINED_FREEZYA_SEEDS, player);
                player->VehicleSpellInitialize();
                break;
            case SPELL_GET_STRANGLER_SEEDS:
                AddPlantToBar(SPELL_PLANT_STRANGLER_VINE);
                Talk(SAY_GAINED_STRANGLER_SEEDS, player);
                player->VehicleSpellInitialize();
                break;
            default:
                break;
        }
    }

    void DoAction(int32 action) override
    {
        if (_finished)
            return;

        switch (action)
        {
            case ACTION_FIRST_SPITTER_PLANTED:
                me->RemoveAurasDueToSpell(SPELL_SPITTER_HIGHLIGHT);
                DespawnHighlight();
                _events.ScheduleEvent(EVENT_CREATE_ZOMBIE, 500ms);
                break;
            case ACTION_FIRST_SUNFLOWER_PLANTED:
                me->RemoveAurasDueToSpell(SPELL_SUNFLOWER_HIGHLIGHT);
                DespawnHighlight();
                break;
            case ACTION_FIRST_ROCKNUT_PLANTED:
                me->RemoveAurasDueToSpell(SPELL_ROCKNUT_HIGHLIGHT);
                DespawnHighlight();
                break;
            case ACTION_ZOMBIE_DAMAGED:
                ++_damagedZombieCount;
                switch (_level)
                {
                    case LEVEL_TUTORIAL:
                        if (_currentStage < 2)
                            EnterNextStage();
                        break;
                    case LEVEL_1:
                        if (_currentStage == 0 && _damagedZombieCount == 1)
                            EnterNextStage();
                        else if (_currentStage == 1 && _damagedZombieCount == 2)
                            EnterNextStage();
                        break;
                    default:
                        break;
                }
                break;
            case ACTION_ZOMBIE_DIED:
                ++_deadZombieCount;
                switch (_level)
                {
                    case LEVEL_TUTORIAL:
                        if (_deadZombieCount == 3)
                            EnterNextStage();
                        else if (_deadZombieCount == 8)
                            EnterNextStage();
                        break;
                    case LEVEL_1:
                        if (_deadZombieCount == 4 && _currentStage == 2)
                            EnterNextStage();
                        else if (_deadZombieCount == 11 && _currentStage == 3)
                            EnterNextStage();
                        else if (_deadZombieCount == 27 && _currentStage == 4)
                            EnterNextStage();
                        break;
                    default:
                        if (_aliveAttackers)
                            --_aliveAttackers;
                        CheckWaveStageCleared();
                        break;
                }
                break;
            case ACTION_GHOUL_DAMAGED:
                ++_damagedGhoulCount;
                break;
            case ACTION_STILLWATER_DIED:
                if (Player* player = GetPlayer())
                    player->KilledMonsterCredit(CREDIT_LAWN_OF_THE_DEAD);
                DespawnAttackers();
                FinishLevel();
                break;
            case ACTION_QUEST_FAILED:
                FailLevel();
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SPITTER_HIGHLIGHT:
                    Highlight(SAY_ANNOUNCE_TUTORIAL_1, SPELL_SPITTER_HIGHLIGHT, 0);
                    break;
                case EVENT_SUNFLOWER_HIGHLIGHT:
                    Highlight(SAY_ANNOUNCE_PLANT_SUNFLOWER, SPELL_SUNFLOWER_HIGHLIGHT, 0);
                    _events.ScheduleEvent(EVENT_ANNOUNCE_PLANT_MORE_FLOWERS, 1s);
                    break;
                case EVENT_ROCKNUT_HIGHLIGHT:
                    Highlight(SAY_ANNOUNCE_USE_ROCKNUTS, SPELL_ROCKNUT_HIGHLIGHT, ROCKNUT_HIGHLIGHT_SPOT);
                    break;
                case EVENT_LEVEL_TIP:
                    switch (_level)
                    {
                        case LEVEL_3: Whisper(SAY_ANNOUNCE_USE_PUMPKIN_BOMBS); break;
                        case LEVEL_4: Whisper(SAY_ANNOUNCE_BUILD_DEFENSES); break;
                        default: break;
                    }
                    break;
                case EVENT_SUMMON_SOLAR_POWER:
                    if (Player* player = GetPlayer())
                        SummonSolarPower(player);

                    if (_level == LEVEL_TUTORIAL)
                        _events.RepeatEvent(uint32(_timeUntilNextSolarPower));
                    else
                    {
                        // TCPP: five and ten seconds in turn
                        _timeUntilNextSolarPower = _timeUntilNextSolarPower == 5000 ? 10000 : 5000;
                        _events.RepeatEvent(uint32(_timeUntilNextSolarPower));
                    }
                    break;
                case EVENT_CREATE_ZOMBIE:
                    CreateAttacker(NPC_ZOMBIE, RandomLane());
                    break;
                case EVENT_CREATE_GHOUL:
                    CreateAttacker(NPC_GHOUL, RandomLane());
                    break;
                case EVENT_WAVE_ZOMBIE:
                case EVENT_WAVE_GHOUL:
                case EVENT_WAVE_ABOMINATION:
                    if (_pendingSpawns)
                        --_pendingSpawns;
                    CreateAttacker(eventId == EVENT_WAVE_ZOMBIE ? NPC_ZOMBIE : eventId == EVENT_WAVE_GHOUL ? NPC_GHOUL : NPC_ABOMINATION,
                        RandomLane());
                    CheckWaveStageCleared();
                    break;
                case EVENT_START_WAVE_STAGE:
                    StartWaveStage();
                    break;
                case EVENT_ANNOUNCE_GOOD_JOB:
                    Whisper(SAY_ANNOUNCE_TUTORIAL_2);
                    _events.ScheduleEvent(EVENT_CREATE_ZOMBIE, 1ms);
                    break;
                case EVENT_ANNOUNCE_PLACING_SPITTERS:
                    Whisper(SAY_ANNOUNCE_TUTORIAL_3);
                    _events.ScheduleEvent(EVENT_CREATE_ZOMBIE, 1ms);
                    break;
                case EVENT_ANNOUNCE_MASSIVE_WAVE:
                    Whisper(SAY_ANNOUNCE_MASSIVE_WAVE);
                    break;
                case EVENT_SUMMON_STILLWATER:
                    // Create Warden Stillwater: at the nearest lane end (conditions), with his health as the progress bar
                    if (Player* player = GetPlayer())
                    {
                        player->CastSpell(player, SPELL_CREATE_WARDEN_STILLWATER, true);
                        player->SetPower(POWER_ALTERNATE_POWER, player->GetMaxPower(POWER_ALTERNATE_POWER));
                    }
                    _events.ScheduleEvent(EVENT_STILLWATER_ESCORT, 9s);
                    _events.ScheduleEvent(EVENT_PLACE_OBELISK, 20s);
                    break;
                case EVENT_STILLWATER_ESCORT:
                {
                    // Ours: while he lives a zombie follows every 9 seconds, every third a ghoul, every sixth an abomination
                    ++_escortCount;
                    uint32 entry = _escortCount % 6 == 0 ? NPC_ABOMINATION : _escortCount % 3 == 0 ? NPC_GHOUL : NPC_ZOMBIE;
                    CreateAttacker(entry, RandomLane());
                    _events.RepeatEvent(9000);
                    break;
                }
                case EVENT_PLACE_OBELISK:
                    PlaceObelisk();
                    _events.RepeatEvent(30000);
                    break;
                case EVENT_ANNOUNCE_LEVEL_COMPLETED:
                    CompleteLevel();
                    break;
                case EVENT_EJECT_PASSENGER:
                    if (Vehicle* vehicle = me->GetVehicleKit())
                        vehicle->RemoveAllPassengers();
                    break;
                case EVENT_ANNOUNCE_PLANT_MORE_FLOWERS:
                    Whisper(SAY_ANNOUNCE_PLANT_MORE_SUNFLOWERS);
                    break;
                case EVENT_CHECK_PASSENGER:
                {
                    // Safety net: the game ends with the ride, also if the passenger got away without PassengerBoarded
                    Player* player = GetPlayer();
                    if (!player || player->GetVehicleBase() != me || player->GetExactDist2d(me) > MaxVehicleDistance)
                    {
                        if (player && player->GetVehicleBase() == me)
                            player->ExitVehicle();
                        else
                            EndGame(player);
                        return;
                    }

                    // The quest was abandoned (or otherwise left the in-progress state) during the game: end it
                    uint32 questId = GetQuestId();
                    if (!_finished && questId && player->GetQuestStatus(questId) != QUEST_STATUS_INCOMPLETE)
                    {
                        _finished = true;
                        _events.Reset();
                        player->ExitVehicle();
                        return;
                    }
                    _events.RepeatEvent(1000);
                    break;
                }
                case EVENT_BOARD_TIMEOUT:
                    if (!_boarded)
                    {
                        if (Player* player = GetPlayer())
                            player->RemoveAurasDueToSpell(SPELL_CREATE_BOTANISTS_VEHICLE);
                        _finished = true;
                        me->DespawnOrUnsummon();
                        return;
                    }
                    break;
                default:
                    break;
            }
        }
    }

    // Called by the plant spells: only an empty spot of this game may be planted on
    bool IsSpotFree(Position const& pos) const
    {
        for (ObjectGuid guid : _spotGUIDs)
            if (Creature* spot = ObjectAccessor::GetCreature(*me, guid))
                if (spot->GetEntry() == NPC_EMPTY_SPOT && spot->GetExactDist2d(pos.GetPositionX(), pos.GetPositionY()) <= 5.0f)
                    return true;

        return false;
    }

private:
    Player* GetPlayer() const
    {
        return ObjectAccessor::GetPlayer(*me, _playerGUID);
    }

    void Whisper(uint32 textId)
    {
        if (Player* player = GetPlayer())
            player->Whisper(textId, player, true);
    }

    // The level of the quest just accepted from Brazie; on a restart from his gossip the one of his quests in progress (the
    // latest of the chain first, the daily last)
    static uint8 SelectLevel(Player* player)
    {
        static std::pair<uint32, uint8> const levels[] =
        {
            { QUEST_LAWN_OF_THE_DEAD, LEVEL_4 },
            { QUEST_SOMEONE_SETUP_THE_PUMPKIN_BOMB, LEVEL_3 },
            { QUEST_GHOULS_HATE_MY_GRAINS, LEVEL_2 },
            { QUEST_FLOWER_POWER, LEVEL_1 },
            { QUEST_BASIC_BOTANY, LEVEL_TUTORIAL },
            { QUEST_TENDING_THE_GARDEN, LEVEL_DAILY },
        };

        uint32 acceptedQuest = BrazieStartingGame.PlayerGUID == player->GetGUID() ? BrazieStartingGame.QuestId : 0;
        if (acceptedQuest && player->GetQuestStatus(acceptedQuest) == QUEST_STATUS_INCOMPLETE)
            for (auto const& level : levels)
                if (level.first == acceptedQuest)
                    return level.second;

        for (auto const& level : levels)
            if (player->GetQuestStatus(level.first) == QUEST_STATUS_INCOMPLETE)
                return level.second;

        return LEVEL_NONE;
    }

    uint32 GetQuestId() const
    {
        switch (_level)
        {
            case LEVEL_TUTORIAL: return QUEST_BASIC_BOTANY;
            case LEVEL_1:        return QUEST_FLOWER_POWER;
            case LEVEL_2:        return QUEST_GHOULS_HATE_MY_GRAINS;
            case LEVEL_3:        return QUEST_SOMEONE_SETUP_THE_PUMPKIN_BOMB;
            case LEVEL_4:        return QUEST_LAWN_OF_THE_DEAD;
            case LEVEL_DAILY:    return QUEST_TENDING_THE_GARDEN;
            default:             return 0;
        }
    }

    // TCPP uses 92259 for levels 0 and 1. Ours: the 7-step bars for levels 2 and 3 (6 and 5 stages), the 5-step one for the
    // daily (4 stages); level 4 shows Warden Stillwater's health once he is there.
    uint32 GetProgressBarSpell() const
    {
        switch (_level)
        {
            case LEVEL_TUTORIAL:
            case LEVEL_1:        return SPELL_PROGRESS_BAR_4;
            case LEVEL_2:        return SPELL_PROGRESS_BAR_7;
            case LEVEL_3:        return SPELL_PROGRESS_BAR_7_PHASE;
            case LEVEL_DAILY:    return SPELL_PROGRESS_BAR_5;
            default:             return 0;
        }
    }

    // Ours from level 2 on: each level adds the plant its quest introduces (Rocknut, Pumpkin Bomb, Freezya); the dropped seeds
    // add Strangler Vine (level 2) and Freezya (level 3) for the rest of the level; the daily has them all.
    void SetupActionBar()
    {
        for (uint8 i = 0; i < CREATURE_MAX_SPELLS; ++i)
            me->m_spells[i] = 0;

        switch (_level)
        {
            case LEVEL_TUTORIAL:
                me->m_spells[0] = SPELL_PLANT_SPITTER;
                break;
            case LEVEL_1:
                me->m_spells[0] = SPELL_PLANT_SUNFLOWER;
                me->m_spells[1] = SPELL_PLANT_SPITTER;
                break;
            case LEVEL_2:
                me->m_spells[0] = SPELL_PLANT_SUNFLOWER;
                me->m_spells[1] = SPELL_PLANT_SPITTER;
                me->m_spells[2] = SPELL_PLANT_ROCKNUT;
                break;
            case LEVEL_3:
                me->m_spells[0] = SPELL_PLANT_SUNFLOWER;
                me->m_spells[1] = SPELL_PLANT_SPITTER;
                me->m_spells[2] = SPELL_PLANT_ROCKNUT;
                me->m_spells[3] = SPELL_PLANT_PUMPKIN_BOMB;
                break;
            case LEVEL_4:
                me->m_spells[0] = SPELL_PLANT_SUNFLOWER;
                me->m_spells[1] = SPELL_PLANT_SPITTER;
                me->m_spells[2] = SPELL_PLANT_ROCKNUT;
                me->m_spells[3] = SPELL_PLANT_PUMPKIN_BOMB;
                me->m_spells[4] = SPELL_PLANT_FREEZYA;
                break;
            case LEVEL_DAILY:
                me->m_spells[0] = SPELL_PLANT_SUNFLOWER;
                me->m_spells[1] = SPELL_PLANT_SPITTER;
                me->m_spells[2] = SPELL_PLANT_ROCKNUT;
                me->m_spells[3] = SPELL_PLANT_STRANGLER_VINE;
                me->m_spells[4] = SPELL_PLANT_PUMPKIN_BOMB;
                me->m_spells[5] = SPELL_PLANT_FREEZYA;
                break;
            default:
                break;
        }
    }

    void AddPlantToBar(uint32 spellId)
    {
        for (uint8 i = 0; i < MAX_SPELL_VEHICLE; ++i)
        {
            if (me->m_spells[i] == spellId)
                return;

            if (!me->m_spells[i])
            {
                me->m_spells[i] = spellId;
                return;
            }
        }
    }

    void SetupSpawns(Player* player)
    {
        for (uint8 i = 0; i < MAX_LANES; ++i)
        {
            // Cast with TriggerCastFlags: that CastSpell(Position) keeps the position's orientation, the bool one turns the
            // summon the way the player faces
            Position pos = LawnmowerPositions[i];
            pos.SetOrientation(LawnmowerOrientation);
            player->CastSpell(pos, SPELL_CREATE_LAWNMOWER, TRIGGERED_FULL_MASK);

            pos = GoalStalkerPositions[i];
            pos.SetOrientation(GoalStalkerOrientation);
            player->CastSpell(pos, SPELL_CREATE_GOAL_STALKER, TRIGGERED_FULL_MASK);

            pos = SpitterTargetPositions[i];
            pos.SetOrientation(SpitterTargetOrientation);
            player->CastSpell(pos, SPELL_CREATE_SPITTER_TARGET, TRIGGERED_FULL_MASK);
        }

        // Create Empty Spot (91676), server-side
        for (uint8 i = 0; i < MAX_EMPTY_SPOT_POSITIONS; ++i)
        {
            Position pos = EmptySpotPositions[i];
            pos.SetOrientation(EmptySpotOrientation);
            SummonPrivate(player, NPC_EMPTY_SPOT, pos);
        }
    }

    void StartLevel()
    {
        switch (_level)
        {
            case LEVEL_TUTORIAL:
                _timeUntilNextSolarPower = 4000;
                _events.ScheduleEvent(EVENT_SPITTER_HIGHLIGHT, 1s);
                _events.ScheduleEvent(EVENT_SUMMON_SOLAR_POWER, 1s);
                break;
            case LEVEL_1:
                _timeUntilNextSolarPower = 5000;
                _events.ScheduleEvent(EVENT_SUNFLOWER_HIGHLIGHT, 1s);
                _events.ScheduleEvent(EVENT_SUMMON_SOLAR_POWER, 5500ms);
                _events.ScheduleEvent(EVENT_CREATE_ZOMBIE, 41s);
                break;
            case LEVEL_2:
            case LEVEL_3:
            case LEVEL_4:
            case LEVEL_DAILY:
                // Ours: Solar Power as in level 1, the stages from the wave tables
                _timeUntilNextSolarPower = 5000;
                _events.ScheduleEvent(EVENT_SUMMON_SOLAR_POWER, 5500ms);
                if (_level == LEVEL_2)
                    _events.ScheduleEvent(EVENT_ROCKNUT_HIGHLIGHT, 1s);
                else
                    _events.ScheduleEvent(EVENT_LEVEL_TIP, 1s);
                _currentStage = 0;
                _events.ScheduleEvent(EVENT_START_WAVE_STAGE, 1s);
                break;
            default:
                break;
        }
    }

    // Create Random Sun Power (91741), server-side: a sun drops onto a random spot of the lawn
    void SummonSolarPower(Player* player)
    {
        Position const& spot = EmptySpotPositions[urand(0, MAX_EMPTY_SPOT_POSITIONS - 1)];
        float angle = frand(0.0f, float(2 * M_PI));
        Position pos(spot.GetPositionX() + std::cos(angle) * 3.0f, spot.GetPositionY() + std::sin(angle) * 3.0f,
            spot.GetPositionZ() + SolarPowerDropHeight);
        SummonPrivate(player, NPC_SOLAR_POWER, pos, SolarPowerDuration);
    }

    void Highlight(uint32 textId, uint32 spellId, uint8 spotIndex)
    {
        Whisper(textId);
        me->AddAura(spellId, me);

        // Effect 1 of the highlight spells: the "Place it here!" marker on a spot
        if (Player* player = GetPlayer())
        {
            Position pos = EmptySpotPositions[spotIndex];
            pos.SetOrientation(EmptySpotOrientation);
            if (TempSummon* marker = SummonPrivate(player, NPC_PLACE_IT_HERE, pos))
                _highlightGUID = marker->GetGUID();
        }
    }

    void DespawnHighlight()
    {
        if (Creature* marker = ObjectAccessor::GetCreature(*me, _highlightGUID))
            marker->DespawnOrUnsummon();
        _highlightGUID.Clear();
    }

    uint8 RandomLane() const
    {
        // The tutorial only sends zombies down the center lane
        return _level == LEVEL_TUTORIAL ? uint8(CENTER_LANE) : uint8(urand(0, MAX_LANES - 1));
    }

    void CreateAttacker(uint32 entry, uint8 lane)
    {
        Player* player = GetPlayer();
        if (!player)
            return;

        Position pos = SpitterTargetPositions[lane];
        pos.SetOrientation(pos.GetAngle(&GoalStalkerPositions[lane]));

        switch (entry)
        {
            case NPC_ZOMBIE:
                player->CastSpell(pos, SPELL_CREATE_ZOMBIE, TRIGGERED_FULL_MASK);
                break;
            case NPC_GHOUL:
                player->CastSpell(pos, SPELL_CREATE_GHOUL, TRIGGERED_FULL_MASK);
                break;
            case NPC_ABOMINATION:
                // No create spell for the abomination in the 5.4.8 DBC: summoned like the zombies' spells do (3077)
                SummonPrivate(player, NPC_ABOMINATION, pos);
                break;
            default:
                break;
        }
    }

    void CreateZombies(uint8 amount, uint32 delay, uint32 repeatTimer)
    {
        for (uint8 i = 0; i < amount; ++i)
            _events.ScheduleEvent(EVENT_CREATE_ZOMBIE, delay + repeatTimer * i);
    }

    void CreateGhouls(uint8 amount, uint32 delay, uint32 repeatTimer)
    {
        for (uint8 i = 0; i < amount; ++i)
            _events.ScheduleEvent(EVENT_CREATE_GHOUL, delay + repeatTimer * i);
    }

    // Levels 0 and 1 (TCPP): stages advance on damaged and dead zombies
    void EnterNextStage()
    {
        ++_currentStage;

        switch (_level)
        {
            case LEVEL_TUTORIAL:
                if (_currentStage == 1)
                    _events.ScheduleEvent(EVENT_ANNOUNCE_GOOD_JOB, 1ms);
                else if (_currentStage == 2)
                    _events.ScheduleEvent(EVENT_ANNOUNCE_PLACING_SPITTERS, 1ms);
                else if (_currentStage == 3)
                    StartMassiveWave();
                else if (_currentStage == 4)
                {
                    FinishLevel();
                    return;
                }
                break;
            case LEVEL_1:
                if (_currentStage == 1)
                    CreateZombies(2, 5000, 7000);
                else if (_currentStage == 2)
                    CreateZombies(3, 8000, 7000);
                else if (_currentStage == 3)
                {
                    CreateZombies(3, 35000, 5000);
                    CreateGhouls(2, 6000, 39000);
                }
                else if (_currentStage == 4)
                    StartMassiveWave();
                else if (_currentStage == 5)
                {
                    FinishLevel();
                    return;
                }
                break;
            default:
                break;
        }

        if (Player* player = GetPlayer())
            player->ModifyPower(POWER_ALTERNATE_POWER, 1);
    }

    void StartMassiveWave()
    {
        switch (_level)
        {
            case LEVEL_TUTORIAL:
                _events.ScheduleEvent(EVENT_ANNOUNCE_MASSIVE_WAVE, 1ms);
                CreateZombies(5, 1800, 3000);
                break;
            case LEVEL_1:
                _events.ScheduleEvent(EVENT_ANNOUNCE_MASSIVE_WAVE, 500ms);
                CreateZombies(2, 2000, 3000);
                CreateZombies(5, 2000, 3000);
                CreateZombies(1, 20000, 0);
                CreateZombies(2, 20000, 2000);
                CreateZombies(1, 29000, 0);
                CreateZombies(1, 29000, 0);

                CreateGhouls(2, 8000, 3000);
                CreateGhouls(1, 26000, 0);
                CreateGhouls(1, 26000, 0);
                break;
            default:
                break;
        }
    }

    // Levels 2-4 and the daily (ours): a stage ends when all its attackers are dead
    void StartWaveStage()
    {
        LevelWaves waves = GetLevelWaves(_level);
        ++_currentStage;
        _stageRunning = true;
        _pendingSpawns = 0;

        if (waves.MassiveStages & (1 << _currentStage))
            _events.ScheduleEvent(EVENT_ANNOUNCE_MASSIVE_WAVE, 500ms);

        for (size_t i = 0; i < waves.GroupCount; ++i)
        {
            WaveGroup const& group = waves.Groups[i];
            if (group.Stage != _currentStage)
                continue;

            uint32 eventId = group.Entry == NPC_ZOMBIE ? EVENT_WAVE_ZOMBIE : group.Entry == NPC_GHOUL ? EVENT_WAVE_GHOUL : EVENT_WAVE_ABOMINATION;
            for (uint8 n = 0; n < group.Count; ++n)
            {
                _events.ScheduleEvent(eventId, group.Delay + group.Interval * n);
                ++_pendingSpawns;
            }
        }
    }

    void CheckWaveStageCleared()
    {
        if (!_stageRunning || _pendingSpawns || _aliveAttackers || _finished)
            return;

        _stageRunning = false;
        LevelWaves waves = GetLevelWaves(_level);
        bool massive = (waves.MassiveStages & (1 << _currentStage)) != 0;

        Player* player = GetPlayer();
        if (!player)
            return;

        if (GetProgressBarSpell())
            player->ModifyPower(POWER_ALTERNATE_POWER, 1);

        if (massive)
        {
            switch (_level)
            {
                case LEVEL_2:
                    // The last ghoul drops Strangler Vine seeds at the far end of the lawn; the drop spell gives the credit
                    Whisper(SAY_ANNOUNCE_GHOUL_DROPPED);
                    player->CastSpell(player, SPELL_CREATE_RANDOM_STRANGLER_VINE, true);
                    break;
                case LEVEL_3:
                    Whisper(SAY_ANNOUNCE_ZOMBIE_DROPPED);
                    player->CastSpell(player, SPELL_CREATE_RANDOM_FREEZYA_SACK, true);
                    break;
                case LEVEL_DAILY:
                    // Ours: "Good job!" (tutorial text) for a massive wave of the daily
                    Whisper(SAY_ANNOUNCE_TUTORIAL_2);
                    player->KilledMonsterCredit(CREDIT_TENDING_THE_GARDEN);
                    break;
                default:
                    break;
            }
        }

        if (_currentStage >= waves.Stages)
        {
            if (_level == LEVEL_4)
                _events.ScheduleEvent(EVENT_SUMMON_STILLWATER, 5s);
            else
                FinishLevel();
            return;
        }

        // Ours: a short break before the next stage
        _events.ScheduleEvent(EVENT_START_WAVE_STAGE, 6s);
    }

    // Ours: Warden Stillwater's obelisks rise at the far end of another lane than his
    void PlaceObelisk()
    {
        Player* player = GetPlayer();
        Creature* stillwater = ObjectAccessor::GetCreature(*me, _stillwaterGUID);
        if (!player || !stillwater || !stillwater->IsAlive())
            return;

        uint8 stillwaterLane = GetLaneIndex(stillwater->GetPositionY());
        uint8 lane = urand(0, MAX_LANES - 2);
        if (lane >= stillwaterLane)
            ++lane;

        Position pos(ObeliskColumnX, SpitterTargetPositions[lane].GetPositionY(), SpitterTargetPositions[lane].GetPositionZ(),
            SpitterTargetOrientation + float(M_PI));
        player->CastSpell(pos, SPELL_CREATE_OBELISK, TRIGGERED_FULL_MASK);
    }

    void FinishLevel()
    {
        _finished = true;
        _events.Reset();
        _events.ScheduleEvent(EVENT_ANNOUNCE_LEVEL_COMPLETED, 1ms);
        _events.ScheduleEvent(EVENT_EJECT_PASSENGER, 1600ms);
    }

    void CompleteLevel()
    {
        Player* player = GetPlayer();
        if (!player)
            return;

        uint32 textId = 0;
        uint32 creditEntry = 0;
        switch (_level)
        {
            case LEVEL_TUTORIAL:
                textId = SAY_ANNOUNCE_TUTORIAL_4;
                creditEntry = CREDIT_BASIC_BOTANY;
                break;
            case LEVEL_1:
                textId = SAY_ANNOUNCE_LEVEL_1_COMPLETE;
                creditEntry = CREDIT_FLOWER_POWER;
                break;
            case LEVEL_2:
                textId = SAY_ANNOUNCE_LEVEL_2_COMPLETE;
                break;
            case LEVEL_3:
                // Ours: no "level complete" text for this level exists; "Good job!" from the tutorial
                textId = SAY_ANNOUNCE_TUTORIAL_2;
                break;
            default:
                break;
        }

        if (textId)
            player->Whisper(textId, player, true);
        RemoveProgressBars(player);
        if (creditEntry)
            player->KilledMonsterCredit(creditEntry);
    }

    // A zombie reached the end of its lane
    void FailLevel()
    {
        _finished = true;
        _events.Reset();

        if (Player* player = GetPlayer())
        {
            if (uint32 questId = GetQuestId())
                player->FailQuest(questId);
            Talk(SAY_ZOMBIES_ATE_YOUR_BRAINS, player);
        }

        DespawnAttackers();
        _events.ScheduleEvent(EVENT_EJECT_PASSENGER, 2300ms);
    }

    void DespawnAttackers()
    {
        _summons.DespawnEntry(NPC_ZOMBIE);
        _summons.DespawnEntry(NPC_GHOUL);
        _summons.DespawnEntry(NPC_ABOMINATION);
        _summons.DespawnEntry(NPC_OBELISK);
        if (Creature* stillwater = ObjectAccessor::GetCreature(*me, _stillwaterGUID))
            if (stillwater->IsAlive())
                stillwater->DespawnOrUnsummon();
    }

    static void RemoveProgressBars(Player* player)
    {
        for (uint32 spellId : { SPELL_PROGRESS_BAR_4, SPELL_PROGRESS_BAR_5, SPELL_PROGRESS_BAR_7, SPELL_PROGRESS_BAR_7_PHASE,
            SPELL_PROGRESS_BAR_STILLWATER })
            player->RemoveAurasDueToSpell(spellId);
    }

    // The ride ended (finished, failed, left the seat, died or logged out): clean up everything of this game
    void EndGame(Player* player)
    {
        if (!_boarded)
            return;

        _boarded = false;
        _events.Reset();

        if (player)
        {
            // Leaving before the end counts as a lost game; the quest can be abandoned and taken again from Brazie
            if (!_finished && !player->GetSession()->PlayerLogout())
                if (uint32 questId = GetQuestId())
                    player->FailQuest(questId);

            RemoveProgressBars(player);
            player->RemoveAurasDueToSpell(SPELL_CREATE_BOTANISTS_VEHICLE);
        }

        _finished = true;
        _summons.DespawnAll();
        _spotGUIDs.clear();
        me->DespawnOrUnsummon(100);
    }

    EventMap _events;
    SummonList _summons;
    GuidVector _spotGUIDs;
    ObjectGuid _playerGUID;
    ObjectGuid _highlightGUID;
    ObjectGuid _stillwaterGUID;
    uint8 _level;
    uint8 _deadZombieCount;
    uint8 _damagedZombieCount;
    uint8 _damagedGhoulCount;
    uint8 _currentStage;
    uint32 _timeUntilNextSolarPower;
    uint32 _pendingSpawns;
    uint32 _aliveAttackers;
    uint32 _escortCount = 0;
    bool _stageRunning;
    bool _boarded;
    bool _finished;
};

// Shared by the actors: despawn when the game they belong to is gone (the player left, logged out or the vehicle despawned)
struct BrazieActorAI : public ScriptedAI
{
    BrazieActorAI(Creature* creature) : ScriptedAI(creature) { }

    void ScheduleVehicleCheck()
    {
        _actorEvents.ScheduleEvent(EVENT_CHECK_VEHICLE, 2s);
    }

    bool UpdateVehicleCheck(uint32 diff)
    {
        _actorEvents.Update(diff);
        if (_actorEvents.ExecuteEvent() == EVENT_CHECK_VEHICLE)
        {
            if (!GetBotanistVehicle(me))
            {
                me->DespawnOrUnsummon();
                return false;
            }
            _actorEvents.ScheduleEvent(EVENT_CHECK_VEHICLE, 2s);
        }
        return true;
    }

    EventMap _actorEvents;
};

// 49192 - Fertilize-o-tron 2000: the lawnmower at the end of each lane, once per game it mows its lane clear
struct npc_brazie_fertilize_o_tron_2000 : public BrazieActorAI
{
    npc_brazie_fertilize_o_tron_2000(Creature* creature) : BrazieActorAI(creature), _lane(0), _started(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetDisplayFromModel(1);
        _lane = GetLaneIndex(me->GetPositionY());
        RegisterWithVehicle(me);
        ScheduleVehicleCheck();
        _events.ScheduleEvent(EVENT_CHECK_LANE, 250ms);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void AttackStart(Unit* who) override
    {
        if (_started || !who || !CanStart(who))
            return;

        _started = true;
        _events.CancelEvent(EVENT_CHECK_LANE);
        DoCast(who, SPELL_DAZED, true);
        DoCastSelf(SPELL_LAWNMOWER_STARTUP_SOUND, true);
        me->SetReactState(REACT_PASSIVE);
        _events.ScheduleEvent(EVENT_KILL_LANE, 300ms);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE)
            return;

        if (id == POINT_LANE_END)
            me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVehicleCheck(diff))
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_KILL_LANE:
                    me->GetMotionMaster()->MovePoint(POINT_LANE_END, SpitterTargetPositions[_lane], false);
                    _events.ScheduleEvent(EVENT_PLAGUE_GAS, 200ms);
                    break;
                case EVENT_PLAGUE_GAS:
                    DoCastAOE(SPELL_PLAGUE_GAS, true);
                    _events.RepeatEvent(500);
                    break;
                case EVENT_CHECK_LANE:
                {
                    // Ours: look for a zombie at the mower instead of waiting for a relocation notification
                    std::list<Creature*> zombies;
                    for (uint32 entry : { NPC_ZOMBIE, NPC_GHOUL, NPC_ABOMINATION })
                        me->GetCreatureListWithEntryInGridAppend(zombies, entry, MowerTriggerDistance + 4.0f);
                    for (Creature* zombie : zombies)
                    {
                        if (CanStart(zombie))
                        {
                            AttackStart(zombie);
                            break;
                        }
                    }
                    if (!_started)
                        _events.RepeatEvent(250);
                    break;
                }
                default:
                    break;
            }
        }
    }

private:
    bool CanStart(Unit* who) const
    {
        if (!who->IsAlive() || !IsZombieEntry(who->GetEntry()) || who->GetPrivateObjectOwner() != me->GetPrivateObjectOwner())
            return false;

        // Ours: Plague Gas does not hurt Warden Stillwater, so he does not use up the lane's mower
        if (who->GetEntry() == NPC_WARDEN_STILLWATER)
            return false;

        // TCPP starts at 3 yards; ours 6, so the mower also starts when a zombie stops in front of it to eat it
        if (me->GetExactDist2d(who) > MowerTriggerDistance)
            return false;

        Position pos = me->GetPosition();
        pos.SetOrientation(pos.GetAngle(&SpitterTargetPositions[_lane]));
        return pos.HasInLine(who, 5.0f);
    }

    EventMap _events;
    uint8 _lane;
    bool _started;
};

// 49190 - Empty Spot: one of the 40 spots of the lawn; planting turns it into the plant (UpdateEntry), a plant that is killed
// turns back into an empty spot.
struct npc_brazie_spot : public BrazieActorAI
{
    npc_brazie_spot(Creature* creature) : BrazieActorAI(creature), _lane(0), _brokenRocknut(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->m_SightDistance = 150.0f;
        me->m_CombatDistance = 150.0f;
        me->SetDisplayFromModel(0);
        me->SetControlled(true, UNIT_STATE_ROOT);
        _lane = GetLaneIndex(me->GetPositionY());
        RegisterWithVehicle(me);
        ScheduleVehicleCheck();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        me->AttackStop();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (damage >= me->GetHealth())
        {
            damage = 0;
            ResetToEmptySpot();
            return;
        }

        // Ours: a rocknut below half health looks like the Broken Rocknut (49761)
        if (me->GetEntry() == NPC_ROCKNUT && !_brokenRocknut && me->HealthBelowPctDamaged(50, damage))
        {
            _brokenRocknut = true;
            me->SetDisplayId(DISPLAY_BROKEN_ROCKNUT);
        }
    }

    void AttackStart(Unit* who) override
    {
        if (!who || !IsShooter() || !CanTarget(who, 0.0f))
            return;

        me->SetFacingTo(me->GetAngle(&SpitterTargetPositions[_lane]));
        me->SetNoCallAssistance(true);
        AttackStartNoMove(who);
    }

    void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
    {
        uint32 newEntry = 0;
        switch (spell->Id)
        {
            case SPELL_PLANT_SPITTER:       newEntry = NPC_SPITTER; break;
            case SPELL_PLANT_SUNFLOWER:     newEntry = NPC_SUNFLOWER; break;
            case SPELL_PLANT_FREEZYA:       newEntry = NPC_FREEZYA; break;
            case SPELL_PLANT_ROCKNUT:       newEntry = NPC_ROCKNUT; break;
            case SPELL_PLANT_STRANGLER_VINE: newEntry = NPC_STRANGLER_VINE; break;
            case SPELL_PLANT_PUMPKIN_BOMB:  newEntry = NPC_PUMPKIN_BOMB; break;
            default:
                return;
        }

        _events.Reset();
        _brokenRocknut = false;
        _trapTargetGUID.Clear();
        me->UpdateEntry(newEntry);
        me->SetDisplayFromModel(1);

        if (newEntry == NPC_SUNFLOWER)
            _events.ScheduleEvent(EVENT_SUNLIGHT, 16s);
        else if (newEntry != NPC_ROCKNUT)
            _events.ScheduleEvent(EVENT_CHECK_LANE, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVehicleCheck(diff))
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_SUNLIGHT:
                    if (me->GetEntry() != NPC_SUNFLOWER)
                        break;
                    DoCastSelf(SPELL_SUNLIGHT, true);
                    if (Player* player = GetSummoningPlayer(me))
                        SummonSunlight(player);
                    _events.RepeatEvent(17000);
                    break;
                case EVENT_CHECK_LANE:
                    CheckLane();
                    break;
                case EVENT_PUMPKIN_EXPLODED:
                    ResetToEmptySpot();
                    break;
                default:
                    break;
            }
        }

        if (!me->GetVictim())
            return;

        if (me->GetEntry() == NPC_SPITTER)
            DoSpellAttackIfReady(SPELL_VENOM_SPIT);
        else if (me->GetEntry() == NPC_FREEZYA)
            DoSpellAttackIfReady(SPELL_FREEZYA_BLAST);
    }

private:
    bool IsShooter() const
    {
        return me->GetEntry() == NPC_SPITTER || me->GetEntry() == NPC_FREEZYA;
    }

    // Plants act on what is in their own lane in front of them; maxDistance 0 is the whole lane
    bool CanTarget(Unit* who, float maxDistance) const
    {
        if (!who->IsAlive() || who->GetPrivateObjectOwner() != me->GetPrivateObjectOwner())
            return false;

        switch (me->GetEntry())
        {
            case NPC_SPITTER:
            case NPC_FREEZYA:
                if (!IsAttackerEntry(who->GetEntry()))
                    return false;
                break;
            case NPC_PUMPKIN_BOMB:
                if (!IsZombieEntry(who->GetEntry()))
                    return false;
                break;
            case NPC_STRANGLER_VINE:
                // Ours: Warden Stillwater tears a vine apart, it does not strangle him
                if (!IsZombieEntry(who->GetEntry()) || who->GetEntry() == NPC_WARDEN_STILLWATER)
                    return false;
                break;
            default:
                return false;
        }

        if (maxDistance > 0.0f && me->GetExactDist2d(who) > maxDistance)
            return false;

        Position pos = me->GetPosition();
        pos.SetOrientation(pos.GetAngle(&SpitterTargetPositions[_lane]));
        return pos.HasInLine(who, 5.0f);
    }

    Unit* SelectLaneTarget(float maxDistance) const
    {
        std::list<Creature*> candidates;
        float range = maxDistance > 0.0f ? maxDistance + 2.0f : 90.0f;
        for (uint32 entry : { NPC_ZOMBIE, NPC_GHOUL, NPC_ABOMINATION, NPC_WARDEN_STILLWATER, NPC_OBELISK })
            me->GetCreatureListWithEntryInGridAppend(candidates, entry, range);

        Unit* target = nullptr;
        for (Creature* candidate : candidates)
            if (CanTarget(candidate, maxDistance))
                if (!target || me->GetExactDist2d(candidate) < me->GetExactDist2d(target))
                    target = candidate;
        return target;
    }

    void CheckLane()
    {
        switch (me->GetEntry())
        {
            case NPC_SPITTER:
            case NPC_FREEZYA:
                if (!me->GetVictim() || !CanTarget(me->GetVictim(), 0.0f))
                {
                    me->AttackStop();
                    if (Unit* target = SelectLaneTarget(0.0f))
                        AttackStart(target);
                }
                _events.RepeatEvent(1000);
                break;
            case NPC_STRANGLER_VINE:
            {
                // Ours: the vine strangles the first undead that comes close (Strangle, 10% health a second) and withers
                // when its victim is dead
                if (!_trapTargetGUID.IsEmpty())
                {
                    Unit* victim = ObjectAccessor::GetUnit(*me, _trapTargetGUID);
                    if (!victim || !victim->IsAlive())
                    {
                        ResetToEmptySpot();
                        return;
                    }
                }
                else
                {
                    if (Unit* target = SelectLaneTarget(TrapTriggerDistance))
                    {
                        _trapTargetGUID = target->GetGUID();
                        me->SetFacingToObject(target);
                        DoCast(target, SPELL_STRANGLE, true);
                    }
                }
                _events.RepeatEvent(500);
                break;
            }
            case NPC_PUMPKIN_BOMB:
                // Ours: the pumpkin explodes (Pumpkin Explosion, 2 s cast) when undead come close, and is used up
                if (SelectLaneTarget(TrapTriggerDistance))
                {
                    DoCastAOE(SPELL_PUMPKIN_EXPLOSION);
                    _events.ScheduleEvent(EVENT_PUMPKIN_EXPLODED, 2500ms);
                }
                else
                    _events.RepeatEvent(500);
                break;
            default:
                break;
        }
    }

    void SummonSunlight(Player* player)
    {
        Position const& spot = EmptySpotPositions[urand(0, MAX_EMPTY_SPOT_POSITIONS - 1)];
        float angle = frand(0.0f, float(2 * M_PI));
        Position pos(spot.GetPositionX() + std::cos(angle) * 3.0f, spot.GetPositionY() + std::sin(angle) * 3.0f,
            spot.GetPositionZ() + SolarPowerDropHeight);
        SummonPrivate(player, NPC_SOLAR_POWER, pos, SolarPowerDuration);
    }

    void ResetToEmptySpot()
    {
        _events.Reset();
        _brokenRocknut = false;
        _trapTargetGUID.Clear();
        me->InterruptNonMeleeSpells(false);
        me->AttackStop();
        me->UpdateEntry(NPC_EMPTY_SPOT);
        me->SetFullHealth();
        me->SetDisplayFromModel(0);
        me->SetFacingTo(me->GetHomePosition().GetOrientation());
    }

    EventMap _events;
    ObjectGuid _trapTargetGUID;
    uint8 _lane;
    bool _brokenRocknut;
};

// 49209 - Zombie, 49232 - Ghoul, 49694 - Abomination, 49819 - Warden Stillwater: walk down their lane and eat the plants in
// their way
struct npc_brazie_zombie : public BrazieActorAI
{
    npc_brazie_zombie(Creature* creature) : BrazieActorAI(creature), _lane(0), _notifiedLowHealth(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->m_SightDistance = 3.0f;
        me->m_CombatDistance = 3.0f;
        me->SetWalk(true);
        _lane = GetLaneIndex(me->GetPositionY());

        RegisterWithVehicle(me);
        ScheduleVehicleCheck();

        switch (me->GetEntry())
        {
            case NPC_ZOMBIE:
                DoCastSelf(SPELL_ZOMBIE_GROWL);
                SetDamageMultiplier(3.0f);          // TCPP: DamageModifier 3
                break;
            case NPC_GHOUL:
                me->SetDisplayFromModel(3);
                DoCastSelf(SPELL_GHOUL_GROWL, true);
                DoCastSelf(SPELL_BUCKET_HEAD, true);
                SetDamageMultiplier(6.0f);          // TCPP: DamageModifier 6
                break;
            case NPC_ABOMINATION:
                DoCastSelf(SPELL_ABOMINATION_GROWL, true);
                SetDamageMultiplier(8.0f);          // ours
                break;
            case NPC_WARDEN_STILLWATER:
                DoCastSelf(SPELL_BOSS_GROWL, true);
                SetDamageMultiplier(10.0f);         // ours
                if (Player* player = GetSummoningPlayer(me))
                    Talk(SAY_STILLWATER_ARRIVES, player);
                break;
            default:
                break;
        }

        _events.ScheduleEvent(EVENT_MOVE_TO_GOAL_POSITION, 2400ms);
        _events.ScheduleEvent(EVENT_CHECK_LANE, 3s);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override
    {
        // Never walk back to the spawn point: carry on down the lane
        me->AttackStop();
        MoveToGoal();
    }

    void AttackStart(Unit* who) override
    {
        if (!who || !CanEat(who))
            return;

        me->SetNoCallAssistance(true);
        me->StopMoving();
        ScriptedAI::AttackStart(who);
    }

    void JustDied(Unit* /*killer*/) override
    {
        if (me->GetEntry() == NPC_WARDEN_STILLWATER)
        {
            if (Player* player = GetSummoningPlayer(me))
            {
                Talk(SAY_STILLWATER_DIES, player);
                player->SetPower(POWER_ALTERNATE_POWER, 0);
            }
            NotifyVehicle(me, ACTION_STILLWATER_DIED);
        }
        else
            NotifyVehicle(me, ACTION_ZOMBIE_DIED);

        me->DespawnOrUnsummon(4000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (me->GetEntry() == NPC_WARDEN_STILLWATER)
            UpdateBossBar(damage);

        if (_notifiedLowHealth || !me->HealthBelowPctDamaged(50, damage))
            return;

        _notifiedLowHealth = true;
        switch (me->GetEntry())
        {
            case NPC_ZOMBIE:
                DoCastSelf(SPELL_LEAKING_GUTS, true);
                NotifyVehicle(me, ACTION_ZOMBIE_DAMAGED);
                break;
            case NPC_GHOUL:
                me->RemoveAurasDueToSpell(SPELL_BUCKET_HEAD);
                DoCastSelf(SPELL_BUCKET, true);
                NotifyVehicle(me, ACTION_GHOUL_DAMAGED);
                break;
            default:
                break;
        }
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type != POINT_MOTION_TYPE || id != POINT_GOAL_STALKER)
            return;

        if (me->GetExactDist2d(&GoalStalkerPositions[_lane]) <= 1.0f)
            NotifyVehicle(me, ACTION_QUEST_FAILED);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVehicleCheck(diff))
            return;

        _events.Update(diff);

        if (me->HasUnitState(UNIT_STATE_CASTING))
            return;

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_MOVE_TO_GOAL_POSITION:
                    MoveToGoal();
                    _events.ScheduleEvent(EVENT_CHECK_VICTIM_STATE, 1s);
                    break;
                case EVENT_CHECK_VICTIM_STATE:
                {
                    // The plant we eat was destroyed (turned back into an empty spot) or the lawnmower drove off: walk on
                    Unit* victim = me->GetVictim();
                    if (victim && (!victim->IsAlive() || (victim->GetEntry() != NPC_FERTILIZE_O_TRON_2000 && !IsPlantEntry(victim->GetEntry()))))
                    {
                        me->AttackStop();
                        MoveToGoal();
                    }
                    else if (!victim && !me->isMoving() && !me->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED))
                        MoveToGoal();
                    _events.RepeatEvent(1000);
                    break;
                }
                case EVENT_CHECK_LANE:
                    // Ours: look for a plant ahead twice a second instead of waiting for a relocation notification
                    if (!me->GetVictim())
                        if (Creature* plant = FindPlantAhead())
                            AttackStart(plant);
                    _events.RepeatEvent(500);
                    break;
                default:
                    break;
            }
        }

        if (me->GetVictim())
            DoMeleeAttackIfReady();
    }

private:
    bool CanEat(Unit* who) const
    {
        Creature* target = who->ToCreature();
        if (!target || !target->IsAlive() || target->GetPrivateObjectOwner() != me->GetPrivateObjectOwner())
            return false;

        if (!IsPlantEntry(target->GetEntry()) && target->GetEntry() != NPC_FERTILIZE_O_TRON_2000)
            return false;

        // Just to make sure that a zombie won't call nearby zombies for aid
        if (who->GetDistance(me) > ZombieBiteDistance)
            return false;

        // A plant very close to us has been planted on top of us: walk on (TCPP)
        if (me->GetExactDist2d(who) < ZombieBiteDistance)
            return false;

        // Do not attack plants behind us
        if (!who->isInFront(me, 1.8f))
            return false;

        Position pos = me->GetPosition();
        pos.SetOrientation(pos.GetAngle(&GoalStalkerPositions[_lane]));
        return pos.HasInLine(who, 4.8f);
    }

    Creature* FindPlantAhead() const
    {
        static uint32 const eatable[] = { NPC_SPITTER, NPC_SUNFLOWER, NPC_FREEZYA, NPC_ROCKNUT, NPC_STRANGLER_VINE,
            NPC_PUMPKIN_BOMB, NPC_FERTILIZE_O_TRON_2000 };

        std::list<Creature*> candidates;
        for (uint32 entry : eatable)
            me->GetCreatureListWithEntryInGridAppend(candidates, entry, 8.0f);

        Creature* plant = nullptr;
        for (Creature* candidate : candidates)
            if (CanEat(candidate) && (!plant || me->GetExactDist2d(candidate) < me->GetExactDist2d(plant)))
                plant = candidate;
        return plant;
    }

    void MoveToGoal()
    {
        me->SetWalk(true);
        me->GetMotionMaster()->MovePoint(POINT_GOAL_STALKER, GoalStalkerPositions[_lane], false);
    }

    void SetDamageMultiplier(float multiplier)
    {
        me->SetBaseWeaponDamage(BASE_ATTACK, MINDAMAGE, me->GetWeaponDamageRange(BASE_ATTACK, MINDAMAGE) * multiplier);
        me->SetBaseWeaponDamage(BASE_ATTACK, MAXDAMAGE, me->GetWeaponDamageRange(BASE_ATTACK, MAXDAMAGE) * multiplier);
        me->UpdateDamagePhysical(BASE_ATTACK);
    }

    // The player's "Warden Stillwater" bar (92598, max 35000) shows his health
    void UpdateBossBar(uint32 damage)
    {
        Player* player = GetSummoningPlayer(me);
        if (!player || !me->GetMaxHealth())
            return;

        uint32 health = damage >= me->GetHealth() ? 0 : me->GetHealth() - damage;
        int32 maxPower = player->GetMaxPower(POWER_ALTERNATE_POWER);
        player->SetPower(POWER_ALTERNATE_POWER, int32(uint64(health) * uint64(std::max(maxPower, 0)) / me->GetMaxHealth()));
    }

    EventMap _events;
    uint8 _lane;
    bool _notifiedLowHealth;
};

// 49911 - Obelisk (ours): raised by Warden Stillwater's game at the far end of a lane, it raises a zombie into that lane every
// 15 seconds until the plants destroy it
struct npc_brazie_obelisk : public BrazieActorAI
{
    npc_brazie_obelisk(Creature* creature) : BrazieActorAI(creature), _lane(0) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetControlled(true, UNIT_STATE_ROOT);
        me->SetReactState(REACT_PASSIVE);
        me->SetMaxHealth(ObeliskHealth);
        me->SetFullHealth();
        _lane = GetLaneIndex(me->GetPositionY());
        RegisterWithVehicle(me);
        ScheduleVehicleCheck();
        _events.ScheduleEvent(EVENT_RAISE_ZOMBIE, 5s);
    }

    void EnterEvadeMode(EvadeReason /*why*/) override { }
    void AttackStart(Unit* /*who*/) override { }

    void JustDied(Unit* /*killer*/) override
    {
        _events.Reset();
        me->DespawnOrUnsummon(2000);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVehicleCheck(diff))
            return;

        _events.Update(diff);

        if (_events.ExecuteEvent() == EVENT_RAISE_ZOMBIE)
        {
            if (Player* player = GetSummoningPlayer(me))
            {
                Position pos = SpitterTargetPositions[_lane];
                pos.SetOrientation(pos.GetAngle(&GoalStalkerPositions[_lane]));
                player->CastSpell(pos, SPELL_CREATE_ZOMBIE, TRIGGERED_FULL_MASK);
            }
            _events.ScheduleEvent(EVENT_RAISE_ZOMBIE, 15s);
        }
    }

private:
    EventMap _events;
    uint8 _lane;
};

// 49194 - Solar Power, 49560 - Strangler Vine Seeds, 49604 - Freezya Seeds, 51098 - Place it here!, 49210 - Goal Marker,
// 49385 - Spitter Target
struct npc_brazie_vehicle_notifier : public BrazieActorAI
{
    npc_brazie_vehicle_notifier(Creature* creature) : BrazieActorAI(creature) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        switch (me->GetEntry())
        {
            case NPC_SOLAR_POWER:
                me->SetDisplayFromModel(1);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                me->GetMotionMaster()->MoveFall();
                break;
            case NPC_STRANGLER_VINE_SEEDS:
            case NPC_FREEZYA_SEEDS:
                me->SetDisplayFromModel(1);
                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                break;
            case NPC_PLACE_IT_HERE:
                me->SetDisplayFromModel(1);
                break;
            default:
                me->SetDisplayFromModel(0);
                break;
        }

        RegisterWithVehicle(me);
        ScheduleVehicleCheck();
    }

    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        // Only the game's actors: a static spawn of one of these entries belongs to no game
        if (!caster || !me->IsSummon())
            return;

        switch (spell->Id)
        {
            case SPELL_GAIN_SOLAR_POWER_SPELLCLICK:
                if (Unit* vehicle = caster->GetVehicleCreatureBase())
                    DoCast(vehicle, SPELL_GAIN_SOLAR_POWER_ENERGIZE, true);
                Collected(SolarPowerJumpPos);
                break;
            case SPELL_GET_FREEZYA_SEEDS:
            case SPELL_GET_STRANGLER_SEEDS:
                // Effect 0 of the pickup gives the seeds to the vehicle
                Collected(SeedSackJumpPos);
                break;
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->IsSummon())
            UpdateVehicleCheck(diff);
    }

private:
    void Collected(Position const& jumpPos)
    {
        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->GetMotionMaster()->MoveJump(jumpPos, 45.0f, 20.0f);
        me->DespawnOrUnsummon(2000);
    }
};

enum BrazieTheBotanist
{
    GOSSIP_MENU_BRAZIE                  = 12590,
    GOSSIP_OPTION_TRY_AGAIN             = 0,    // "We need to give this another try." (broadcast_text 50035)
};

// 49687 - Brazie the Botanist: accepting any of his quests starts the game (TCPP). Ours: while one of them is in progress
// without a game (the player logged out or left the area, a party member shared it, the vehicle could not be summoned), his
// gossip option starts it again, so the quest need not be abandoned.
struct npc_brazie_the_botanist : public ScriptedAI
{
    npc_brazie_the_botanist(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        StartGame(player, quest->GetQuestId());
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_BRAZIE || gossipListId != GOSSIP_OPTION_TRY_AGAIN)
            return false;

        CloseGossipMenuFor(player);
        StartGame(player, 0);
        return true;
    }

private:
    static void StartGame(Player* player, uint32 questId)
    {
        if (player->GetVehicle() || player->HasAura(SPELL_CREATE_BOTANISTS_VEHICLE))
            return;

        BrazieStartingGame = { player->GetGUID(), questId };
        player->CastSpell(player, SPELL_CREATE_BOTANISTS_VEHICLE, true);
        BrazieStartingGame = { };
    }
};

// 91683 - Create Botanist's Vehicle
// Cast on accepting any of Brazie's quests or from his gossip (npc_brazie_the_botanist): one game at a time, and only for a
// player who can ride. A mounted player may start (the spell is castable while mounted and boarding dismounts), a flying one
// may not.
class spell_brazie_create_botanists_vehicle : public SpellScript
{
    PrepareSpellScript(spell_brazie_create_botanists_vehicle);

    SpellCastResult CheckCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player || !player->IsAlive() || player->GetVehicle() || player->IsInFlight() || player->IsFlying())
            return SPELL_FAILED_DONT_REPORT;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_brazie_create_botanists_vehicle::CheckCast);
    }
};

// 91646 - Plant Sunflower, 91649 - Plant Spitter, 91704 - Plant Rocknut, 91710 - Plant Strangler Vine,
// 92157 - Plant Pumpkin Bomb, 92440 - Plant Freezya
// Cast by the vehicle at a spot of the lawn. Effect 0 would place a gameobject and effect 2 triggers a spell missing from the
// 5.4.8 DBC: the plant is the spot itself, turned into the plant by the dummy (npc_brazie_spot::SpellHit).
class spell_brazie_summon_plant : public SpellScript
{
    PrepareSpellScript(spell_brazie_summon_plant);

    SpellCastResult CheckLocation()
    {
        Creature* caster = GetCaster()->ToCreature();
        WorldLocation const* dest = GetExplTargetDest();
        if (!caster || !dest)
            return SPELL_FAILED_BAD_TARGETS;

        if (npc_brazie_the_botanist_vehicle* ai = CAST_AI(npc_brazie_the_botanist_vehicle, caster->AI()))
            if (ai->IsSpotFree(*dest))
                return SPELL_CAST_OK;

        return SPELL_FAILED_BAD_TARGETS;
    }

    // Only the spot closest to where the player clicked
    void FilterTargets(std::list<WorldObject*>& targets)
    {
        WorldLocation const* dest = GetExplTargetDest();
        if (targets.size() <= 1 || !dest)
            return;

        WorldObject* closest = nullptr;
        for (WorldObject* target : targets)
            if (!closest || target->GetExactDist2d(dest->GetPositionX(), dest->GetPositionY()) < closest->GetExactDist2d(dest->GetPositionX(), dest->GetPositionY()))
                closest = target;

        targets.clear();
        targets.push_back(closest);
    }

    void PreventGameObject(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void PreventTrigger(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);
    }

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Creature* caster = GetCaster()->ToCreature();
        if (!caster || !caster->IsAIEnabled)
            return;

        if (caster->HasAura(SPELL_SUNFLOWER_HIGHLIGHT))
            caster->AI()->DoAction(ACTION_FIRST_SUNFLOWER_PLANTED);

        if (caster->HasAura(SPELL_SPITTER_HIGHLIGHT))
            caster->AI()->DoAction(ACTION_FIRST_SPITTER_PLANTED);

        if (caster->HasAura(SPELL_ROCKNUT_HIGHLIGHT) && m_scriptSpellId == SPELL_PLANT_ROCKNUT)
            caster->AI()->DoAction(ACTION_FIRST_ROCKNUT_PLANTED);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_brazie_summon_plant::CheckLocation);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_brazie_summon_plant::FilterTargets, EFFECT_1, TARGET_UNIT_DEST_AREA_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_brazie_summon_plant::HandleDummy, EFFECT_1, SPELL_EFFECT_DUMMY);
        OnEffectHit += SpellEffectFn(spell_brazie_summon_plant::PreventGameObject, EFFECT_0, SPELL_EFFECT_TRANS_DOOR);
        if (m_scriptSpellId != SPELL_PLANT_SUNFLOWER)
            OnEffectLaunch += SpellEffectFn(spell_brazie_summon_plant::PreventTrigger, EFFECT_2, SPELL_EFFECT_TRIGGER_SPELL);
    }
};

// 91748 - Venom Spit, 92441 - Freezya Blast (a line down the lane, SPELL_ATTR0_CU_CONE_LINE in spell_custom_attribute)
// Only the closest attacker is hit; Freezya Blast prefers one that is not frozen yet. (TCPP)
class spell_brazie_spit : public SpellScript
{
    PrepareSpellScript(spell_brazie_spit);

    void FilterTargets(std::list<WorldObject*>& targets)
    {
        if (targets.empty())
            return;

        if (m_scriptSpellId == SPELL_FREEZYA_BLAST)
        {
            std::list<WorldObject*> unfrozen = targets;
            unfrozen.remove_if(Trinity::UnitAuraCheck(true, SPELL_FREEZYA_BLAST));
            if (!unfrozen.empty())
                targets = unfrozen;
        }

        targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster(), true));
        targets.resize(1);
    }

    void Register() override
    {
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_brazie_spit::FilterTargets, EFFECT_ALL, TARGET_UNIT_CONE_ENEMY_104);
    }
};

// 91687 - Gain Solar Power, 92339 - Get Strangler Seeds, 92434 - Get Freezya Seeds
// The spellclicks of the suns and seed sacks (cast by the clicking player on the clicked unit). Group members see each
// other's private summons: only the game's own player may pick them up.
class spell_brazie_pick_up : public SpellScript
{
    PrepareSpellScript(spell_brazie_pick_up);

    SpellCastResult CheckOwner()
    {
        Unit* target = GetExplTargetUnit();
        if (!target || !target->IsPrivateObject() || target->GetPrivateObjectOwner() != GetCaster()->GetGUID())
            return SPELL_FAILED_DONT_REPORT;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_brazie_pick_up::CheckOwner);
    }
};
}

void AddSC_hillsbrad_brazie()
{
    using namespace HillsbradBrazie;
    new creature_script<npc_brazie_the_botanist_vehicle>("npc_brazie_the_botanist_vehicle");
    new creature_script<npc_brazie_fertilize_o_tron_2000>("npc_brazie_fertilize_o_tron_2000");
    new creature_script<npc_brazie_spot>("npc_brazie_spot");
    new creature_script<npc_brazie_zombie>("npc_brazie_zombie");
    new creature_script<npc_brazie_obelisk>("npc_brazie_obelisk");
    new creature_script<npc_brazie_vehicle_notifier>("npc_brazie_vehicle_notifier");
    new creature_script<npc_brazie_the_botanist>("npc_brazie_the_botanist");
    new spell_script<spell_brazie_create_botanists_vehicle>("spell_brazie_create_botanists_vehicle");
    new spell_script<spell_brazie_summon_plant>("spell_brazie_summon_plant");
    new spell_script<spell_brazie_spit>("spell_brazie_spit");
    new spell_script<spell_brazie_pick_up>("spell_brazie_pick_up");
}
