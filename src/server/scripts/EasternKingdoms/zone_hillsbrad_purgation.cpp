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

// Hillsbrad Foothills: Kasha and Kingslayer Orkus, from the Ruins of Southshore to Purgation Isle and on to Tarren Mill.
//  - The Road to Purgation (28375): Kasha flies the player and Orkus to Purgation Isle.
//  - Heroes of the Horde! (28400): Orkus kills the Stormpike Battle Master and holds off Vanndar, Balinda and Ivar Bloodfang
//    while the player takes the Alliance Battle Plans; Kasha carries the player and the dying Orkus to Tarren Mill.
// Sourced: the spells and their summon properties and vehicle seats (5.4.8 DBC), every line (broadcast_text), the course of
// the events (warcraft.wiki.gg). Ours: the flight paths, heights, speeds, the actors' spots and the timings; nothing was sniffed.

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Vehicle.h"
#include "CombatAI.h"
#include "TemporarySummon.h"
#include "ObjectAccessor.h"

enum PurgationIsle
{
    QUEST_THE_ROAD_TO_PURGATION                 = 28375,
    QUEST_HEROES_OF_THE_HORDE                   = 28400,

    NPC_KASHA_ROAD_TO_PURGATION                 = 48467,
    NPC_ORKUS_PURGATION_ISLE                    = 48470,
    NPC_ORKUS_HEROES                            = 48503,
    NPC_KASHA_HEROES                            = 48504,
    NPC_PACKLEADER_IVAR_BLOODFANG               = 48485,
    NPC_CAPTAIN_BALINDA_STONEHEARTH             = 48507,
    NPC_VANNDAR_STORMPIKE                       = 48508,
    NPC_STORMPIKE_BATTLE_MASTER                 = 48515,
    NPC_HIGH_WARLORD_CROMUSH                    = 48545,

    ITEM_ALLIANCE_BATTLE_PLANS                  = 64307,

    SPELL_DETECT_QUEST_INVIS_ZONE_4             = 90115,    // shows the landed Kasha and Orkus on Purgation Isle (spell_area)
    SPELL_RIDE_TO_PURGATION                     = 90122,
    SPELL_SUMMON_KASHA_ROAD_TO_PURGATION        = 90123,
    SPELL_SUMMON_KASHA_HEROES                   = 90278,

    // Kasha 48467 (Vehicle 1329) and 48504 (Vehicle 1332): seat 0 can control the vehicle (a player there would take her
    // over and her script would stop), seat 1 is a plain passenger seat. Orkus takes seat 0, the player seat 1.
    SEAT_ORKUS                                  = 0,
    SEAT_PLAYER                                 = 1,

    GOSSIP_MENU_ORKUS_HEROES                    = 12440,
    GOSSIP_OPTION_ORKUS_READY                   = 0,

    FACTION_ORGRIMMAR                           = 29,       // Orkus' faction at Southshore (48218), so the Stormpike fight him

    ACTION_ORKUS_CARRIED                        = 1,
};

enum PurgationTexts
{
    // Kingslayer Orkus 48470 (riding Kasha in The Road to Purgation)
    TALK_ORKUS_TO_PURGATION_ISLE                = 0,        // 48671
    TALK_ORKUS_FLEW_OFF                         = 1,        // 48675 (boss whisper)

    // Kingslayer Orkus 48503 (Heroes of the Horde!)
    TALK_ORKUS_GET_DOWN                         = 0,        // 48744
    TALK_ORKUS_MAP_TABLE                        = 1,        // 48747
    TALK_ORKUS_CAREFUL                          = 2,        // 48745
    TALK_ORKUS_PLAN_OF_MY_OWN                   = 3,        // 48752
    TALK_ORKUS_KILL_THE_GUARD                   = 4,        // 48753
    TALK_ORKUS_GET_THE_PLANS                    = 5,        // 48754
    TALK_ORKUS_DO_NOT_WORRY                     = 6,        // 48755
    TALK_ORKUS_LOKTAR                           = 7,        // 48764
    TALK_ORKUS_PLANS_AND_GO                     = 8,        // 48765
    TALK_ORKUS_KASHA_GET_OUT                    = 9,        // 48769
    TALK_ORKUS_FORGET_ABOUT_ME                  = 10,       // 48771
    TALK_ORKUS_CAME_BACK                        = 11,       // 48772
    TALK_ORKUS_ASK_OF_YOU                       = 12,       // 48773
    TALK_ORKUS_TAKE_CARE_OF_KASHA               = 13,       // 48779
    TALK_ORKUS_BEST_FRIEND                      = 14,       // 48778
    TALK_ORKUS_SOMEWHERE_SAFE                   = 15,       // 48774

    // Vanndar Stormpike 48508
    TALK_VANNDAR_LAST_LOKTAR                    = 0,        // 48788

    // High Warlord Cromush 48545 (groups from 10, his other quests may use the low ones)
    TALK_CROMUSH_WHAT_HAPPENED                  = 10,       // 48797 .. 48803
};

enum PurgationEvents
{
    // Kasha, The Road to Purgation
    EVENT_ROAD_WAIT_FOR_RIDER                   = 1,
    EVENT_ROAD_TAKE_OFF,
    EVENT_ROAD_LAND,
    EVENT_ROAD_ARRIVE,

    // Orkus, Heroes of the Horde!
    EVENT_HEROES_WALK                           = 10,
    EVENT_HEROES_WALK_TIMEOUT,
    EVENT_HEROES_WAIT_FOR_PLAYER,
    EVENT_HEROES_TALK,                          // .. EVENT_HEROES_TALK + 6
    EVENT_HEROES_CHARGE                         = 20,
    EVENT_HEROES_KILL_BATTLE_MASTER,
    EVENT_HEROES_CHECK_PLANS,
    EVENT_HEROES_CALL_KASHA,

    // Kasha, Heroes of the Horde!
    EVENT_KASHA_APPROACH                        = 30,
    EVENT_KASHA_LAND_BY_PLAYER,
    EVENT_KASHA_PICK_UP_PLAYER,
    EVENT_KASHA_WAIT_FOR_RIDER,
    EVENT_KASHA_FETCH_ORKUS,
    EVENT_KASHA_GRAB_ORKUS,
    EVENT_KASHA_TAKE_OFF,
    EVENT_KASHA_LAND,
    EVENT_KASHA_ARRIVE,
    EVENT_KASHA_PERCH,
    EVENT_KASHA_PERCH_LAND,
    EVENT_KASHA_ORKUS_TALK                      = 42,       // .. + 4
    EVENT_KASHA_ORKUS_DIES                      = 48,
    EVENT_KASHA_CROMUSH_TALK                    = 50,       // .. + 6
    EVENT_KASHA_FINISH                          = 60,
};

enum PurgationPoints
{
    POINT_ROAD_FLIGHT                           = 1,
    POINT_ROAD_LANDING,
    POINT_HEROES_HIDING,
    POINT_KASHA_ABOVE_PLAYER,
    POINT_KASHA_BY_PLAYER,
    POINT_KASHA_ABOVE_ORKUS,
    POINT_KASHA_FLIGHT,
    POINT_KASHA_TARREN_MILL,
    POINT_KASHA_PERCH_APPROACH,
    POINT_KASHA_PERCH,
};

// A creature of the player's own event: a private object of theirs, alive and in reach. Lookups from the player's side find
// only the player's own, as group members see each other's.
static Creature* FindPrivateCreatureOf(Player* player, uint32 entry, float range)
{
    std::list<Creature*> creatures;
    player->GetCreatureListWithEntryInGrid(creatures, entry, range);
    for (Creature* creature : creatures)
        if (creature->IsAlive() && creature->GetPrivateObjectOwner() == player->GetGUID())
            return creature;
    return nullptr;
}

static float GroundZ(WorldObject* object, float x, float y, float z)
{
    float ground = object->GetMap()->GetHeight(x, y, z + 10.0f);
    return ground > INVALID_HEIGHT ? ground : z;
}

/*######
## The Road to Purgation (28375)
######*/

// From the Southshore beach the flight goes out over the lake, keeps west of the shore hills (45 yards over the water, 37 or
// more over anything under it) and comes down on the Purgation Isle beach where the landed Kasha (48471) stands.
Position const RoadToPurgationFlightPath[] =
{
    {  -980.0f, -520.0f, 45.0f },
    { -1100.0f, -420.0f, 45.0f },
    { -1190.0f, -250.0f, 45.0f },
    { -1215.0f,  -50.0f, 45.0f },
    { -1215.0f,  150.0f, 45.0f },
    { -1205.0f,  330.0f, 40.0f },
    { -1225.0f,  400.0f, 22.0f },
    { -1234.2f,  424.3f, 12.0f }
};

// The landed Kasha 48471 (guid 312259) and Orkus 48470 (guid 3110070): the flight ends on their spots, and they take over from
// the player's own Kasha and Orkus when the player is credited
Position const RoadToPurgationLandingPos = { -1234.18f, 424.32f, 2.93f, 1.73f };
Position const RoadToPurgationPlayerExitPos = { -1231.5f, 420.5f, 3.3f, 1.73f };

float const RoadToPurgationFlightSpeedRate = 2.5f;

// 90123 - Summon Kasha
// One Kasha per player, and only for a player who can ride
class spell_hillsbrad_summon_kasha_road_to_purgation : public SpellScript
{
    PrepareSpellScript(spell_hillsbrad_summon_kasha_road_to_purgation);

    SpellCastResult CheckCast()
    {
        Player* player = GetCaster()->ToPlayer();
        if (!player)
            return SPELL_FAILED_DONT_REPORT;

        if (!player->IsAlive() || player->GetVehicle() || player->IsInFlight()
            || FindPrivateCreatureOf(player, NPC_KASHA_ROAD_TO_PURGATION, 200.0f))
            return SPELL_FAILED_DONT_REPORT;

        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_hillsbrad_summon_kasha_road_to_purgation::CheckCast);
    }
};

// 90122 - Ride to Purgation
// The ride aura names no seat, so the player would take the first free one: seat 0, which controls Kasha and would replace her
// script with the player's control. The player rides behind Orkus in seat 1.
class spell_hillsbrad_ride_to_purgation : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_ride_to_purgation);

    void CalcSeat(AuraEffect const* /*aurEff*/, float& amount, bool& /*canBeRecalculated*/)
    {
        if (GetCaster() && GetCaster()->GetTypeId() == TYPEID_PLAYER)
            amount = float(SEAT_PLAYER + 1);
    }

    void Register() override
    {
        DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_hillsbrad_ride_to_purgation::CalcSeat, EFFECT_0, SPELL_AURA_CONTROL_VEHICLE);
    }
};

// 48467 - Kasha
// Summoned beside Orkus at Southshore by Summon Kasha (90123, from his gossip option), with the player on her through its ride
// aura (Ride to Purgation 90122, which also gives the Road to Purgation phase 348 and hides the rider). Orkus rides in front.
// She flies them to Purgation Isle and lands where the landed Kasha stands; there the player is credited and set down.
// The credit is given directly: in 5.4.8 Kasha ends the ride with Cancel Cinematic (90128: teleport the summoner to a DB spot,
// Despawn All Summons, QUEST_COMPLETE 28375), whose teleport has no spot here and which would not reach a player who is still
// her passenger.
struct npc_hillsbrad_kasha_road_to_purgation : public VehicleAI
{
    npc_hillsbrad_kasha_road_to_purgation(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _arrived(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);

        // Orkus rides in front, in the seat that steers. He leaves with her; the timed despawn is for a Kasha that goes with
        // her owner's logout.
        if (TempSummon* orkus = me->SummonCreature(NPC_ORKUS_PURGATION_ISLE, me->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, 180s, 0, _playerGUID))
        {
            orkus->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            orkus->SetReactState(REACT_PASSIVE);
            orkus->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
            orkus->EnterVehicle(me, SEAT_ORKUS);
            _orkusGUID = orkus->GetGUID();
        }

        _events.ScheduleEvent(EVENT_ROAD_WAIT_FOR_RIDER, 1s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player is off: Kasha leaves (getting off also dismisses her, as a vehicle minion)
        if (!_arrived)
            Leave();
    }

    void GetPassengerExitPosition(Unit* passenger, int8 /*seatId*/, Position& pos) override
    {
        if (_arrived && passenger->GetGUID() == _playerGUID)
            pos.Relocate(RoadToPurgationPlayerExitPos);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE)
            return;

        if (pointId == POINT_ROAD_FLIGHT)
            _events.ScheduleEvent(EVENT_ROAD_LAND, 100ms);
        else if (pointId == POINT_ROAD_LANDING)
            _events.ScheduleEvent(EVENT_ROAD_ARRIVE, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld() || (_started && !_arrived && player->GetVehicleBase() != me))
        {
            Leave();
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_ROAD_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                    {
                        _started = true;
                        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                        {
                            orkus->AI()->Talk(TALK_ORKUS_TO_PURGATION_ISLE, player);
                            orkus->AI()->Talk(TALK_ORKUS_FLEW_OFF, player);
                        }
                        _events.ScheduleEvent(EVENT_ROAD_TAKE_OFF, 2s);
                    }
                    else if (++_boardTries < 10)
                        _events.ScheduleEvent(EVENT_ROAD_WAIT_FOR_RIDER, 500ms);
                    else
                    {
                        Leave();
                        return;
                    }
                    break;
                case EVENT_ROAD_TAKE_OFF:
                    TakeOff();
                    break;
                case EVENT_ROAD_LAND:
                {
                    Position landing = RoadToPurgationLandingPos;
                    landing.m_positionZ = GroundZ(me, landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ());
                    me->GetMotionMaster()->MoveLand(POINT_ROAD_LANDING, landing);
                    break;
                }
                case EVENT_ROAD_ARRIVE:
                    _arrived = true;
                    if (player->GetQuestStatus(QUEST_THE_ROAD_TO_PURGATION) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(QUEST_THE_ROAD_TO_PURGATION);
                    // spell_area does not follow quest changes in this core: give the detection of the landed Kasha and Orkus
                    // now, so they stand where the player's own pair leaves them
                    player->UpdateAreaDependentAuras(player->GetAreaId());
                    if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                        orkus->DespawnOrUnsummon();
                    if (player->GetVehicleBase() == me)
                        player->ExitVehicle();
                    Leave();
                    return;
                default:
                    break;
            }
        }
    }

private:
    void TakeOff()
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());
        path.push_back({ me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 20.0f });
        for (Position const& node : RoadToPurgationFlightPath)
            path.push_back(node);

        me->SetSpeed(MOVE_RUN, RoadToPurgationFlightSpeedRate);
        me->SetSpeed(MOVE_FLIGHT, RoadToPurgationFlightSpeedRate);
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_ROAD_FLIGHT, path.data(), path.size(), false, true);
    }

    // A player still on board is set down first; Orkus leaves with her
    void Leave()
    {
        _events.Reset();
        _started = false;
        _arrived = true;

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
            if (player->GetVehicleBase() == me)
                player->ExitVehicle();

        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
            orkus->DespawnOrUnsummon();
        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _orkusGUID;
    uint32 _boardTries;
    bool _started;
    bool _arrived;
};

void AddSC_hillsbrad_purgation()
{
    new spell_script<spell_hillsbrad_summon_kasha_road_to_purgation>("spell_hillsbrad_summon_kasha_road_to_purgation");
    new aura_script<spell_hillsbrad_ride_to_purgation>("spell_hillsbrad_ride_to_purgation");
    new creature_script<npc_hillsbrad_kasha_road_to_purgation>("npc_hillsbrad_kasha_road_to_purgation");
}
