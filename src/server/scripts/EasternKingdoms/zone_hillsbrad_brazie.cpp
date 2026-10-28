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
//  - A zombie that reaches the end of its lane eats the player's brains: the quest fails, the player is put off the vehicle.
//
// Ported from TCPP (CataclysmPreservationProject, zone_hillsbrad_foothills.cpp and custom_2018_06_22_00_world.sql): the lane
// layout, the timings and wave compositions of the tutorial and level 1, the plant, zombie and lawnmower behaviour. That port
// is in turn built on the sniffed spells and texts (5.4.8 DBC, broadcast_text). Differences from TCPP, forced by this core:
//  - no phase 365: every actor is the player's private summon, and private summons of one player see each other here.
//  - Create Empty Spot (91676) and Create Random Sun Power (91741) are server-side spells missing from the 5.4.8 DBC (TCPP
//    adds them to spell_dbc): their summon effect is done in C++ (SummonPrivate), with their summon properties (3077).
//  - the highlight spells (92276, 94032) have a 0 yard range in the DBC (TCPP widens it in SpellMgr): their aura is added and
//    their "Place it here!" marker summoned in C++.
//  - the vehicle gets no natural energy regeneration (72242 Zero Energy + Zero Regen), as with TCPP's unit_flags2 0: Solar Power
//    only comes from the suns. The vehicle starts with 100 Solar Power, as TCPP's Vehicle::Install gives it.
//  - TCPP's level 1 drops Freezya seeds from the first damaged ghoul. In 5.4.8 that drop spell (92435) gives the kill credit of
//    Someone Setup the Pumpkin Bomb (49561, "PvG Level 4" in Blizzard's count, which starts at 1 with the tutorial), so
//    Flower Power has no drop here. Levels here count as TCPP's: 0 the tutorial, 1 Flower Power.
//  - creature relocation notifications are too rare for a 1 yard/s zombie: plants scan their lane once a second for a target,
//    zombies look for a plant ahead twice a second and the lawnmowers for a zombie (starting at 6 yards, TCPP 3) four times a
//    second. Zombies and plants never evade back home.

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

    // Brazie the Botanist (vehicle 49176)
    SPELL_PROGRESS_BAR_4                = 92259,    // alternate power bar 32, "Wave Progress", max 4
    SPELL_CREATE_LAWNMOWER              = 91680,
    SPELL_CREATE_SPITTER_TARGET         = 91988,
    SPELL_CREATE_GOAL_STALKER           = 91740,
    SPELL_SPITTER_HIGHLIGHT             = 92276,
    SPELL_SUNFLOWER_HIGHLIGHT           = 94032,
    SPELL_ZERO_ENERGY_REGEN             = 72242,    // Zero Energy + Zero Regen: aura 110 -100% energy regeneration

    SPELL_PLANT_SPITTER                 = 91649,
    SPELL_PLANT_SUNFLOWER               = 91646,
    SPELL_PLANT_FREEZYA                 = 92440,

    // Cast by the player
    SPELL_CREATE_BOTANISTS_VEHICLE      = 91683,
    SPELL_CREATE_ZOMBIE                 = 91739,
    SPELL_CREATE_GHOUL                  = 91834,

    NPC_BRAZIE_VEHICLE                  = 49176,
    NPC_SPITTER_TARGET                  = 49385,
    NPC_EMPTY_SPOT                      = 49190,
    NPC_PLACE_IT_HERE                   = 51098,
    NPC_SOLAR_POWER                     = 49194,
    NPC_SPITTER                         = 49200,
    NPC_SUNFLOWER                       = 49193,
    NPC_ZOMBIE                          = 49209,
    NPC_GHOUL                           = 49232,
    NPC_FREEZYA                         = 49618,
    NPC_GOAL_STALKER                    = 49210,
    NPC_FERTILIZE_O_TRON_2000           = 49192,

    // Server-side spells missing from the 5.4.8 DBC, done in C++ (summon properties of TCPP's spell_dbc rows)
    SPELL_CREATE_EMPTY_SPOT             = 91676,
    SPELL_CREATE_RANDOM_SUN_POWER       = 91741,
    SUMMON_PROPERTIES_PERSONAL          = 3077,     // SUMMON_PROP_FLAG_PERSONAL_SPAWN
    SERVERSIDE_SUMMON_DURATION          = 1800000,  // TCPP's DurationIndex 30

    // Plants
    SPELL_VENOM_SPIT                    = 91748,
    SPELL_SUNLIGHT                      = 93968,
    SPELL_FREEZYA_BLAST                 = 92441,

    // Solar Power
    SPELL_GAIN_SOLAR_POWER_SPELLCLICK   = 91687,
    SPELL_GAIN_SOLAR_POWER_ENERGIZE     = 94068,

    // Zombies
    SPELL_ZOMBIE_GROWL                  = 93961,
    SPELL_LEAKING_GUTS                  = 93244,
    SPELL_GHOUL_GROWL                   = 93962,
    SPELL_BUCKET_HEAD                   = 93230,
    SPELL_BUCKET                        = 93236,

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

    // creature_text of the vehicle
    SAY_GAINED_FREEZYA_SEEDS            = 0,
    SAY_GAINED_STRANGLER_SEEDS          = 1,
    SAY_ZOMBIES_ATE_YOUR_BRAINS         = 2,

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

    // Actor events
    EVENT_MOVE_TO_GOAL_POSITION         = 1,
    EVENT_CHECK_VICTIM_STATE,
    EVENT_SUNLIGHT,
    EVENT_KILL_LANE,
    EVENT_PLAGUE_GAS,
    EVENT_CHECK_LANE,
    EVENT_CHECK_VEHICLE,

    // Actions sent to the vehicle
    ACTION_FIRST_SPITTER_PLANTED        = 1,
    ACTION_FIRST_SUNFLOWER_PLANTED,
    ACTION_ZOMBIE_DAMAGED,
    ACTION_ZOMBIE_DIED,
    ACTION_GHOUL_DAMAGED,
    ACTION_QUEST_FAILED,

    MAX_LANES                           = 5,
    MAX_EMPTY_SPOT_POSITIONS            = 40,
    CENTER_LANE                         = 2,

    POINT_GOAL_STALKER                  = 1,
    POINT_LANE_END                      = 2,

    LEVEL_NONE                          = 0,
    LEVEL_TUTORIAL,
    LEVEL_1
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
            return true;
        default:
            return false;
    }
}

bool IsZombieEntry(uint32 entry)
{
    switch (entry)
    {
        case NPC_ZOMBIE:
        case NPC_GHOUL:
            return true;
        default:
            return false;
    }
}

// Summon a minigame actor as the server-side spells would have: the player's private summon with summon properties 3077.
TempSummon* SummonPrivate(Player* player, uint32 entry, Position const& pos, uint32 duration = SERVERSIDE_SUMMON_DURATION)
{
    return player->GetMap()->SummonCreature(entry, pos, sSummonPropertiesStore.LookupEntry(SUMMON_PROPERTIES_PERSONAL),
        Milliseconds(duration), player, 0, 0, player->GetGUID());
}

// Every actor is summoned by the player while they ride Brazie: the player's vehicle is the game's controller.
Creature* GetBotanistVehicle(Creature* actor)
{
    TempSummon* summon = actor->ToTempSummon();
    if (!summon)
        return nullptr;

    Unit* summoner = summon->GetSummoner();
    if (!summoner)
        return nullptr;

    Creature* vehicle = summoner->GetVehicleCreatureBase();
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
        _boarded(false), _finished(false) { }

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

        if (summon->GetEntry() == NPC_EMPTY_SPOT)
            _spotGUIDs.push_back(summon->GetGUID());
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

            player->CastSpell(player, GetProgressBarSpell(), true);
            SetupSpawns(player);
            StartLevel();
            _events.ScheduleEvent(EVENT_CHECK_PASSENGER, 1s);
        }
        else
            EndGame(player);
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
                        break;
                }
                break;
            case ACTION_GHOUL_DAMAGED:
                ++_damagedGhoulCount;
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
                    Highlight(SAY_ANNOUNCE_TUTORIAL_1, SPELL_SPITTER_HIGHLIGHT);
                    break;
                case EVENT_SUNFLOWER_HIGHLIGHT:
                    Highlight(SAY_ANNOUNCE_PLANT_SUNFLOWER, SPELL_SUNFLOWER_HIGHLIGHT);
                    _events.ScheduleEvent(EVENT_ANNOUNCE_PLANT_MORE_FLOWERS, 1s);
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
                    CreateAttacker(SPELL_CREATE_ZOMBIE);
                    break;
                case EVENT_CREATE_GHOUL:
                    CreateAttacker(SPELL_CREATE_GHOUL);
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

    // The level is the first of Brazie's quests the player has in the log and not yet completed
    static uint8 SelectLevel(Player* player)
    {
        if (player->GetQuestStatus(QUEST_FLOWER_POWER) == QUEST_STATUS_INCOMPLETE)
            return LEVEL_1;
        if (player->GetQuestStatus(QUEST_BASIC_BOTANY) == QUEST_STATUS_INCOMPLETE)
            return LEVEL_TUTORIAL;
        return LEVEL_NONE;
    }

    uint32 GetQuestId() const
    {
        switch (_level)
        {
            case LEVEL_TUTORIAL: return QUEST_BASIC_BOTANY;
            case LEVEL_1:        return QUEST_FLOWER_POWER;
            default:             return 0;
        }
    }

    uint32 GetProgressBarSpell() const
    {
        return SPELL_PROGRESS_BAR_4;
    }

    void SetupActionBar()
    {
        for (uint8 i = 0; i < MAX_SPELL_VEHICLE; ++i)
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
            default:
                break;
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

    void Highlight(uint32 textId, uint32 spellId)
    {
        Whisper(textId);
        me->AddAura(spellId, me);

        // Effect 1 of the highlight spells: the "Place it here!" marker on the first spot of the center lane
        if (Player* player = GetPlayer())
        {
            Position pos = EmptySpotPositions[0];
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

    void CreateAttacker(uint32 spellId)
    {
        // The tutorial only sends zombies down the center lane
        uint8 lane = _level == LEVEL_TUTORIAL ? uint8(CENTER_LANE) : uint8(urand(0, MAX_LANES - 1));
        Position pos = SpitterTargetPositions[lane];
        pos.SetOrientation(pos.GetAngle(&GoalStalkerPositions[lane]));

        if (Player* player = GetPlayer())
            player->CastSpell(pos, spellId, true);
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
    }

    static void RemoveProgressBars(Player* player)
    {
        player->RemoveAurasDueToSpell(SPELL_PROGRESS_BAR_4);
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
    uint8 _level;
    uint8 _deadZombieCount;
    uint8 _damagedZombieCount;
    uint8 _damagedGhoulCount;
    uint8 _currentStage;
    uint32 _timeUntilNextSolarPower;
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
                    GetZombiesInRange(zombies, MowerTriggerDistance + 2.0f);
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

        // TCPP starts at 3 yards; ours 6, so the mower also starts when a zombie stops in front of it to eat it
        if (me->GetExactDist2d(who) > MowerTriggerDistance)
            return false;

        Position pos = me->GetPosition();
        pos.SetOrientation(pos.GetAngle(&SpitterTargetPositions[_lane]));
        return pos.HasInLine(who, 5.0f);
    }

    void GetZombiesInRange(std::list<Creature*>& zombies, float range) const
    {
        me->GetCreatureListWithEntryInGrid(zombies, NPC_ZOMBIE, range);
        me->GetCreatureListWithEntryInGridAppend(zombies, NPC_GHOUL, range);
    }

    EventMap _events;
    uint8 _lane;
    bool _started;
};

// 49190 - Empty Spot: one of the 40 spots of the lawn; planting turns it into the plant (UpdateEntry), a plant that is killed
// turns back into an empty spot.
struct npc_brazie_spot : public BrazieActorAI
{
    npc_brazie_spot(Creature* creature) : BrazieActorAI(creature), _lane(0) { }

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
        }
    }

    void AttackStart(Unit* who) override
    {
        if (!who || !CanTarget(who))
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
            case SPELL_PLANT_SPITTER:
                newEntry = NPC_SPITTER;
                break;
            case SPELL_PLANT_SUNFLOWER:
                newEntry = NPC_SUNFLOWER;
                _events.ScheduleEvent(EVENT_SUNLIGHT, 16s);
                break;
            case SPELL_PLANT_FREEZYA:
                newEntry = NPC_FREEZYA;
                break;
            default:
                return;
        }

        me->UpdateEntry(newEntry);
        me->SetDisplayFromModel(1);
        _events.ScheduleEvent(EVENT_CHECK_LANE, 1s);
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
                    if (TempSummon* summon = me->ToTempSummon())
                        if (Player* player = Object::ToPlayer(summon->GetSummoner()))
                            SummonSunlight(player);
                    _events.RepeatEvent(17000);
                    break;
                case EVENT_CHECK_LANE:
                    if (!IsShooter())
                        break;
                    if (!me->GetVictim() || !me->GetVictim()->IsAlive() || !CanTarget(me->GetVictim()))
                    {
                        me->AttackStop();
                        if (Unit* target = SelectLaneTarget())
                            AttackStart(target);
                    }
                    _events.RepeatEvent(1000);
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

    // Spitters and Freezyas shoot what walks down their own lane towards them
    bool CanTarget(Unit* who) const
    {
        if (!IsShooter() || !who->IsAlive() || !IsZombieEntry(who->GetEntry()))
            return false;

        if (who->GetPrivateObjectOwner() != me->GetPrivateObjectOwner())
            return false;

        Position pos = me->GetPosition();
        pos.SetOrientation(pos.GetAngle(&SpitterTargetPositions[_lane]));
        return pos.HasInLine(who, 5.0f);
    }

    Unit* SelectLaneTarget() const
    {
        std::list<Creature*> candidates;
        me->GetCreatureListWithEntryInGrid(candidates, NPC_ZOMBIE, 90.0f);
        me->GetCreatureListWithEntryInGridAppend(candidates, NPC_GHOUL, 90.0f);

        Unit* target = nullptr;
        for (Creature* candidate : candidates)
            if (const_cast<npc_brazie_spot*>(this)->CanTarget(candidate))
                if (!target || me->GetExactDist2d(candidate) < me->GetExactDist2d(target))
                    target = candidate;
        return target;
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
        me->InterruptNonMeleeSpells(false);
        me->AttackStop();
        me->UpdateEntry(NPC_EMPTY_SPOT);
        me->SetFullHealth();
        me->SetDisplayFromModel(0);
        me->SetFacingTo(me->GetHomePosition().GetOrientation());
    }

    EventMap _events;
    uint8 _lane;
};

// 49209 - Zombie, 49232 - Ghoul: walk down their lane and eat the plants in their way
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
        NotifyVehicle(me, ACTION_ZOMBIE_DIED);
        me->DespawnOrUnsummon(4000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
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
        static uint32 const eatable[] = { NPC_SPITTER, NPC_SUNFLOWER, NPC_FREEZYA, NPC_FERTILIZE_O_TRON_2000 };

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

    EventMap _events;
    uint8 _lane;
    bool _notifiedLowHealth;
};

// 49194 - Solar Power, 49604 - Freezya Seeds, 51098 - Place it here!, 49210 - Goal Marker, 49385 - Spitter Target
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
            default:
                break;
        }
    }

    void UpdateAI(uint32 diff) override
    {
        if (me->IsSummon())
            UpdateVehicleCheck(diff);
    }

protected:
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

    void OnQuestAccept(Player* player, Quest const* /*quest*/) override
    {
        StartGame(player);
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_BRAZIE || gossipListId != GOSSIP_OPTION_TRY_AGAIN)
            return false;

        CloseGossipMenuFor(player);
        StartGame(player);
        return true;
    }

private:
    static void StartGame(Player* player)
    {
        if (!player->GetVehicle() && !player->HasAura(SPELL_CREATE_BOTANISTS_VEHICLE))
            player->CastSpell(player, SPELL_CREATE_BOTANISTS_VEHICLE, true);
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

// 91646 - Plant Sunflower, 91649 - Plant Spitter, 92440 - Plant Freezya
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

        targets.sort(Trinity::ObjectDistanceOrderPred(GetCaster(), true));
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
// Only the closest zombie is hit; Freezya Blast prefers one that is not frozen yet. (TCPP)
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
    new creature_script<npc_brazie_vehicle_notifier>("npc_brazie_vehicle_notifier");
    new creature_script<npc_brazie_the_botanist>("npc_brazie_the_botanist");
    new spell_script<spell_brazie_create_botanists_vehicle>("spell_brazie_create_botanists_vehicle");
    new spell_script<spell_brazie_summon_plant>("spell_brazie_summon_plant");
    new spell_script<spell_brazie_spit>("spell_brazie_spit");
    new spell_script<spell_brazie_pick_up>("spell_brazie_pick_up");
}
