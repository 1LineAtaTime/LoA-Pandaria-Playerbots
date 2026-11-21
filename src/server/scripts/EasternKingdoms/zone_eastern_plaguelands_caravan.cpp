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

// Eastern Plaguelands: Fiona's Caravan. The three rides of the main chain (Onward 27373, The Trek Continues 27448, Nobody to
// Blame but Myself 27489).

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ScriptedGossip.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "SmartScriptMgr.h"
#include "CreatureTextMgr.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "MotionMaster.h"
#include "Group.h"
#include "Pet.h"
#include "ObjectMgr.h"
#include "DBCStores.h"

// ---------------------------------------------------------------------------------------------------------------------------
// The rides
// ---------------------------------------------------------------------------------------------------------------------------

enum CaravanRide
{
    NPC_CARAVAN_GOSSIP_WAGON        = 45400,   // the parked wagon (accessory of harness 45416); also the rides' kill credit
    NPC_PRESTON                     = 45433,
    NPC_ARGYLE                      = 45434,

    NPC_RIDE_1_HARNESS              = 45436,
    NPC_RIDE_1_WAGON                = 45423,
    NPC_RIDE_2_HARNESS              = 45542,
    NPC_RIDE_2_WAGON                = 45437,
    NPC_RIDE_3_HARNESS              = 45782,
    NPC_RIDE_3_WAGON                = 45783,

    NPC_RIDE_GIDWIN                 = 46191,   // rides 1 and 2
    NPC_RIDE_FIONA                  = 46192,
    NPC_RIDE_TARENAR                = 46193,
    NPC_RIDE_ARGUS                  = 46194,
    NPC_RIDE_3_TARENAR              = 46199,
    NPC_RIDE_3_ARGUS                = 46200,
    NPC_RIDE_3_FIONA                = 46201,

    QUEST_ONWARD                    = 27373,
    QUEST_TREK_CONTINUES            = 27448,
    QUEST_NOBODY_TO_BLAME           = 27489,

    SPELL_SUMMON_CARAVAN_1          = 84897,
    SPELL_SUMMON_CARAVAN_2          = 85066,
    SPELL_SUMMON_CARAVAN_3          = 85393,
    SPELL_RIDE_VEHICLE_HARDCODED    = 46598,
    SPELL_FIONAS_LEFT_HAND_ROPE     = 85610,
    SPELL_FIONAS_RIGHT_HAND_ROPE    = 85611,

    // The stops' detections (2026_11_14_00 / _01): taken from the rider while he rides
    SPELL_DETECT_STOP               = 49416,   // every stop (type 7)
    SPELL_SEE_QUEST_INVIS_4         = 85311,   // Gidwin at the river camp (type 12)
    SPELL_SEE_QUEST_INVIS_5         = 85339,   // Tarenar at the river camp (type 13)
    SPELL_DETECT_TYPE_9             = 60922,   // Tarenar at Eastwall (type 9)

    MENU_CARAVAN_WAGON              = 12035,

    WAGON_SEAT                      = 2,       // the wagon's seat on its harness
    RIDER_SEAT                      = 3,       // the first free bench seat of Vehicle 959 on all three rides
    FIONA_SEAT                      = 0,

    POINT_RIDE_STEP                 = 1,

    EVENT_RIDE_BOARD                = 1,
    EVENT_RIDE_START,
    EVENT_RIDE_LINE,
    EVENT_RIDE_LAUGH,
    EVENT_RIDE_FINISH,
    EVENT_RIDE_CHECK,
    EVENT_RIDE_TIMEOUT,
};

struct CaravanRideLine
{
    uint32 Delay;       // ms after the previous line (the first: after the summon)
    uint32 Speaker;     // entry of the wagon passenger
    uint8 Group;        // creature_text group
    bool FionaLaughs;   // Fiona laughs after this line (wiki, ride 1)
};

// Ride 1 and 2: the order and timing of the former SmartAI lists 4543600 / 4554200 (checked against the wiki's lists of
// 27373 and 27448); ride 3: the wiki list of 27489 (broadcast_text 46296-46317), at the same 6 s.
CaravanRideLine const Ride1Lines[] =
{
    {  4000, NPC_RIDE_TARENAR, 0, false }, {  4000, NPC_RIDE_GIDWIN, 0, false },
    {  6000, NPC_RIDE_TARENAR, 1, false }, {  6000, NPC_RIDE_GIDWIN, 1, false },
    {  6000, NPC_RIDE_TARENAR, 2, false }, {  6000, NPC_RIDE_GIDWIN, 2, false },
    {  6000, NPC_RIDE_TARENAR, 3, false }, {  6000, NPC_RIDE_GIDWIN, 3, false },
    {  6000, NPC_RIDE_TARENAR, 4, true  },                                          // "You just told me!" - Fiona laughs
    {  6000, NPC_RIDE_TARENAR, 5, false }, {  6000, NPC_RIDE_TARENAR, 6, false },
    {  6000, NPC_RIDE_TARENAR, 7, false }, {  6000, NPC_RIDE_TARENAR, 8, false },
    {  6000, NPC_RIDE_GIDWIN, 4, false },  {  6000, NPC_RIDE_TARENAR, 9, false },
    {  6000, NPC_RIDE_GIDWIN, 5, false },  {  6000, NPC_RIDE_TARENAR, 10, false },
    {  6000, NPC_RIDE_GIDWIN, 6, true  },                                           // "The Paladin Pals" - Fiona laughs
    {  6000, NPC_RIDE_FIONA, 0, false },   {  6000, NPC_RIDE_GIDWIN, 7, false },
    {  6000, NPC_RIDE_FIONA, 1, false },   {  6000, NPC_RIDE_GIDWIN, 8, false },
    {  6000, NPC_RIDE_FIONA, 2, false },   {  6000, NPC_RIDE_GIDWIN, 9, false },
    {  6000, NPC_RIDE_TARENAR, 11, false }, {  6000, NPC_RIDE_TARENAR, 12, false },
    {  6000, NPC_RIDE_FIONA, 3, false },                                            // "Here we are!"
};

CaravanRideLine const Ride2Lines[] =
{
    {  6000, NPC_RIDE_GIDWIN, 10, false }, {  6000, NPC_RIDE_GIDWIN, 11, false },
    {  6000, NPC_RIDE_FIONA, 4, false },   {     0, NPC_RIDE_TARENAR, 13, false },
    {  6000, NPC_RIDE_GIDWIN, 12, false }, {  6000, NPC_RIDE_FIONA, 5, false },
    {  6000, NPC_RIDE_TARENAR, 14, false }, {  6000, NPC_RIDE_GIDWIN, 13, false },
    {  6000, NPC_RIDE_TARENAR, 15, false }, {  6000, NPC_RIDE_ARGUS, 0, false },
    {  6000, NPC_RIDE_FIONA, 6, false },   {  6000, NPC_RIDE_ARGUS, 1, false },
    {  6000, NPC_RIDE_GIDWIN, 14, false }, {  6000, NPC_RIDE_ARGUS, 2, false },
    {  6000, NPC_RIDE_ARGUS, 3, false },   {  6000, NPC_RIDE_TARENAR, 16, false },
    {  6000, NPC_RIDE_ARGUS, 4, false },   {  6000, NPC_RIDE_ARGUS, 5, false },
    {  6000, NPC_RIDE_GIDWIN, 15, false }, {  6000, NPC_RIDE_GIDWIN, 16, false },
    {  6000, NPC_RIDE_ARGUS, 6, false },   {  6000, NPC_RIDE_TARENAR, 17, false },
    {  6000, NPC_RIDE_ARGUS, 7, false },   {  6000, NPC_RIDE_FIONA, 7, false },
    {  6000, NPC_RIDE_GIDWIN, 17, false }, {  6000, NPC_RIDE_ARGUS, 8, false },
    {  6000, NPC_RIDE_GIDWIN, 18, false },                                          // "Walking, eh...?"
};

CaravanRideLine const Ride3Lines[] =
{
    {  4000, NPC_RIDE_3_TARENAR, 0, false }, {  6000, NPC_RIDE_3_FIONA, 0, false },
    {  6000, NPC_RIDE_3_ARGUS, 0, false },   {  6000, NPC_RIDE_3_TARENAR, 1, false },
    {  6000, NPC_RIDE_3_TARENAR, 2, false }, {  6000, NPC_RIDE_3_ARGUS, 1, false },
    {  6000, NPC_RIDE_3_ARGUS, 2, false },   {  6000, NPC_RIDE_3_ARGUS, 3, false },
    {  6000, NPC_RIDE_3_FIONA, 1, false },   {  6000, NPC_RIDE_3_FIONA, 2, false },
    {  6000, NPC_RIDE_3_ARGUS, 4, false },   {  6000, NPC_RIDE_3_ARGUS, 5, false },
    {  6000, NPC_RIDE_3_TARENAR, 3, false }, {  6000, NPC_RIDE_3_FIONA, 3, false },
    {  6000, NPC_RIDE_3_TARENAR, 4, false }, {  6000, NPC_RIDE_3_FIONA, 4, false },
    {  6000, NPC_RIDE_3_FIONA, 5, false },   {  6000, NPC_RIDE_3_ARGUS, 6, false },
    {  6000, NPC_RIDE_3_TARENAR, 5, false }, {  6000, NPC_RIDE_3_ARGUS, 7, false },
    {  6000, NPC_RIDE_3_TARENAR, 6, false }, {  6000, NPC_RIDE_3_ARGUS, 8, false },
};

struct CaravanRideInfo
{
    uint32 GossipOption;            // menu 12035
    uint32 Quest;
    uint32 SummonSpell;
    uint32 Harness;
    uint32 Wagon;
    uint32 Fiona;                   // the driver (seat 0 of the wagon)
    std::vector<uint32> Hide;       // detections taken from the rider while he rides
    CaravanRideLine const* Lines;
    size_t LineCount;
};

CaravanRideInfo const CaravanRides[] =
{
    { 0, QUEST_ONWARD,          SPELL_SUMMON_CARAVAN_1, NPC_RIDE_1_HARNESS, NPC_RIDE_1_WAGON, NPC_RIDE_FIONA,
        { SPELL_DETECT_STOP, SPELL_SEE_QUEST_INVIS_4, SPELL_SEE_QUEST_INVIS_5 }, Ride1Lines, std::size(Ride1Lines) },
    { 1, QUEST_TREK_CONTINUES,  SPELL_SUMMON_CARAVAN_2, NPC_RIDE_2_HARNESS, NPC_RIDE_2_WAGON, NPC_RIDE_FIONA,
        { SPELL_DETECT_STOP }, Ride2Lines, std::size(Ride2Lines) },
    { 2, QUEST_NOBODY_TO_BLAME, SPELL_SUMMON_CARAVAN_3, NPC_RIDE_3_HARNESS, NPC_RIDE_3_WAGON, NPC_RIDE_3_FIONA,
        { SPELL_DETECT_STOP, SPELL_DETECT_TYPE_9 }, Ride3Lines, std::size(Ride3Lines) },
};

static CaravanRideInfo const* GetCaravanRideByHarness(uint32 harness)
{
    for (CaravanRideInfo const& ride : CaravanRides)
        if (ride.Harness == harness)
            return &ride;
    return nullptr;
}

static CaravanRideInfo const* GetCaravanRideByOption(uint32 option)
{
    for (CaravanRideInfo const& ride : CaravanRides)
        if (ride.GossipOption == option)
            return &ride;
    return nullptr;
}

// Give back what spell_area grants the player where he stands: the stop he left (a ride given up) or the stop he reached.
static void RestoreCaravanSight(Player* player)
{
    if (!player || !player->IsInWorld())
        return;
    player->UpdateZoneDependentAuras(player->GetZoneId());
    player->UpdateAreaDependentAuras(player->GetAreaId());
}

// A ride of this player that is still under way (its harness is his private summon, within sight of him)
static bool HasCaravanRide(Player* player)
{
    for (CaravanRideInfo const& ride : CaravanRides)
    {
        std::list<Creature*> harnesses;
        player->GetCreatureListWithEntryInGrid(harnesses, ride.Harness, 250.0f);
        for (Creature* harness : harnesses)
            if (harness->IsAlive() && !harness->IsDestroyedObject() && harness->IsSummon()
                && harness->ToTempSummon()->GetSummonerGUID() == player->GetGUID())
                return true;
    }
    return false;
}

// 45400 - Fiona's Caravan (the parked wagon, accessory of the harness 45416 at every stop)
// "Ride Fiona's Caravan to its next destination." (menu 12035, options 0-2 with 27373 / 27448 / 27489 in the log). Before, its
// SmartAI had the player cast the ride's summon and nothing else: the ride went off with nobody keeping the player in view of it,
// and a second click while a ride was out summoned a second caravan (Wowhead 205520: "2 caravans on top of each other").
// Now a player already riding, or with a ride of his own still out, gets none. While he rides, the parked caravan and the party
// at the stop are hidden from him: the ride spells' third effect takes the stop's "See Quest Invis" from the caster on retail
// (84892 / 84909 / 85311), which here is the stop's 49416 (and at the river the camp pair's 85311 / 85339, at Eastwall
// Tarenar's 60922); spell_area gives them back when the ride arrives or is given up (RestoreCaravanSight).
struct npc_epl_caravan_wagon : public ScriptedAI
{
    npc_epl_caravan_wagon(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != MENU_CARAVAN_WAGON)
            return false;

        CaravanRideInfo const* ride = GetCaravanRideByOption(gossipListId);
        if (!ride)
            return false;

        CloseGossipMenuFor(player);

        if (player->GetQuestStatus(ride->Quest) != QUEST_STATUS_INCOMPLETE || player->GetVehicle() || HasCaravanRide(player))
            return true;

        for (uint32 spell : ride->Hide)
            player->RemoveAurasDueToSpell(spell);

        player->CastSpell(me, ride->SummonSpell, true);

        if (!HasCaravanRide(player))
            RestoreCaravanSight(player);
        return true;
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }
};

// 45436, 45542, 45782 - Fiona's Caravan Harness (the moving caravan of rides 1, 2 and 3; summoned private to the rider by
// 84897 / 85066 / 85393 at their spell_target_position; horses and the wagon with the party are its accessories)
// What the player sees (sourced: the wiki's notes of 27373, 27448, 27489 and Wowhead): he is seated at the back of the wagon
// (seat 3: Fiona drives, the others sit where the accessories put them), the caravan drives the road to the next tower while
// the party talks (27 / 27 / 22 lines, 6 s apart; Fiona laughs twice on the first ride), and on arrival he is set down with the
// objective "Ride Fiona's Caravan" done (kill credit 45400) beside the caravan parked at that tower.
// Before, this was SmartAI that was never seen working: the passenger-removed handler of the wagon looked the harness up as the
// nearest creature, which skips a private summon, so a player who jumped off was credited on arrival anyway and every caravan
// stayed parked at the end of its road for ever with Fiona and the boys aboard. Now the rider is checked every half second; a
// rider who leaves the wagon, dies or logs out ends the ride without credit (the caravan still stands at the start and the
// option is offered again), and the moving caravan is gone at the end of every ride. The ride waits at the end of the road for
// the last line. Ours: the half-second check, the 2 s pause before setting the player down, the waiting for the last line.
struct npc_epl_caravan_harness : public ScriptedAI
{
    npc_epl_caravan_harness(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _ride = GetCaravanRideByHarness(me->GetEntry());
        if (!_ride || summoner->GetTypeId() != TYPEID_PLAYER)
        {
            me->DespawnOrUnsummon();
            return;
        }

        _playerGUID = summoner->GetGUID();
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        _events.Reset();
        _events.ScheduleEvent(EVENT_RIDE_BOARD, 500ms);
        _events.ScheduleEvent(EVENT_RIDE_START, 2s);
        _events.ScheduleEvent(EVENT_RIDE_LINE, Milliseconds(_ride->Lines[0].Delay));
        _events.ScheduleEvent(EVENT_RIDE_CHECK, 1s);
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }
    void EnterEvadeMode(EvadeReason /*why*/) override { }

    Creature* GetWagon() const
    {
        if (Vehicle* kit = me->GetVehicleKit())
            if (Unit* wagon = kit->GetPassenger(WAGON_SEAT))
                if (wagon->GetEntry() == _ride->Wagon)
                    return wagon->ToCreature();
        return nullptr;
    }

    Creature* GetPassenger(uint32 entry) const
    {
        if (Creature* wagon = GetWagon())
            if (Vehicle* kit = wagon->GetVehicleKit())
                for (int8 seat = 0; seat < 8; ++seat)
                    if (Unit* passenger = kit->GetPassenger(seat))
                        if (passenger->GetEntry() == entry)
                            return passenger->ToCreature();
        return nullptr;
    }

    Player* GetRider() const
    {
        return ObjectAccessor::GetPlayer(*me, _playerGUID);
    }

    bool IsRiding(Player* player) const
    {
        Creature* wagon = GetWagon();
        return player && wagon && player->GetVehicleBase() == wagon;
    }

    void Board()
    {
        Player* player = GetRider();
        Creature* wagon = GetWagon();
        if (!player || !wagon)
        {
            End(false);
            return;
        }

        // The reins from Fiona to the two horses
        if (Creature* fiona = GetPassenger(_ride->Fiona))
            if (Vehicle* kit = me->GetVehicleKit())
            {
                if (Unit* horse = kit->GetPassenger(0))
                    fiona->CastSpell(horse, SPELL_FIONAS_LEFT_HAND_ROPE, true);
                if (Unit* horse = kit->GetPassenger(1))
                    fiona->CastSpell(horse, SPELL_FIONAS_RIGHT_HAND_ROPE, true);
            }

        // The aura's amount is the seat + 1 (AuraEffect::HandleAuraControlVehicle)
        player->CastCustomSpell(SPELL_RIDE_VEHICLE_HARDCODED, SPELLVALUE_BASE_POINT0, RIDER_SEAT + 1, wagon, true);
        if (!IsRiding(player))
            player->CastSpell(wagon, SPELL_RIDE_VEHICLE_HARDCODED, true);   // any free seat

        if (!IsRiding(player))
        {
            TC_LOG_ERROR("scripts", "npc_epl_caravan_harness: player %u could not board the wagon of ride %u", player->GetGUID().GetCounter(), me->GetEntry());
            End(false);
            return;
        }
        _boarded = true;

        // The road runs through plaguehounds, bats and the Scourge: nothing on it picks a fight with the rider (ours; a
        // creature cannot attack a unit that is immune to NPCs, Unit::IsValidAttackTarget). Given back in End().
        if (!player->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC))
        {
            player->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
            _shielded = true;
        }
        player->CombatStop(true);
    }

    void Start()
    {
        WPPath const* path = sSmartWaypointMgr->GetPath(me->GetEntry());
        if (!path || path->empty())
        {
            TC_LOG_ERROR("scripts", "npc_epl_caravan_harness: no `waypoints` path %u", me->GetEntry());
            End(false);
            return;
        }

        std::vector<Position> points;
        points.push_back(me->GetPosition());
        for (uint32 id = 1; id <= path->size(); ++id)
        {
            auto itr = path->find(id);
            if (itr == path->end())
                break;
            points.emplace_back(itr->second->x, itr->second->y, itr->second->z);
        }

        // The road is driven point by point, each leg with the server's own path finding, as waypoint movement does. One
        // spline over the whole road cannot be sent to the client: a linear path packs its points as offsets from the
        // path's middle (MovementPacketBuilder WriteLinearPath, about +-256 yd), the roads are 900 to 1100 yd long, and the
        // client then drew the caravan under the ground while the server drove it along the road.
        _road.assign(points.begin() + 1, points.end());
        _next = 0;
        MoveNext();

        // Failsafe: should the spline never report its end, the ride ends without credit 30 s after it should have arrived
        float length = 0.0f;
        for (size_t i = 1; i < points.size(); ++i)
            length += points[i - 1].GetExactDist(&points[i]);
        float speed = std::max(me->GetSpeed(MOVE_RUN), 1.0f);
        _events.ScheduleEvent(EVENT_RIDE_TIMEOUT, Milliseconds(uint32(length / speed * IN_MILLISECONDS) + 30 * IN_MILLISECONDS));
    }

    void MoveNext()
    {
        if (_next >= _road.size())
        {
            _arrived = true;
            TryFinish();
            return;
        }
        me->GetMotionMaster()->MovePoint(POINT_RIDE_STEP, _road[_next], true);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (type == POINT_MOTION_TYPE && id == POINT_RIDE_STEP && !_ended && !_arrived)
        {
            ++_next;
            MoveNext();
        }
    }

    void SayLine()
    {
        CaravanRideLine const& line = _ride->Lines[_line];
        if (Creature* speaker = GetPassenger(line.Speaker))
            sCreatureTextMgr->SendChat(speaker, line.Group, GetRider());
        if (line.FionaLaughs)
            _events.ScheduleEvent(EVENT_RIDE_LAUGH, 1500ms);

        if (++_line < _ride->LineCount)
            _events.ScheduleEvent(EVENT_RIDE_LINE, Milliseconds(_ride->Lines[_line].Delay));
        else
        {
            _linesDone = true;
            TryFinish();
        }
    }

    void TryFinish()
    {
        if (_arrived && _linesDone && !_finishing)
        {
            _finishing = true;
            _events.ScheduleEvent(EVENT_RIDE_FINISH, 2s);
        }
    }

    // "If you're in a group with someone and you can keep up with the wagon you'll also get credit" (Wowhead, Gnomeypriest [+9],
    // 2012): on arrival the rider's group members who hold the ride's quest incomplete and stand within 50 yd of the caravan are
    // credited too (they see the wagon, horses and party drive off with him; only the harness is private). A member who stayed at
    // the start gets nothing and rides himself. (Group bots never choose the ride's gossip, so they would never get it otherwise.)
    void CreditGroup(Player* rider, Position const& arrival)
    {
        Group* group = rider->GetGroup();
        if (!group)
            return;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == rider || !member->IsInMap(rider) || !member->IsAlive() || member->GetExactDist(&arrival) > 50.0f)
                continue;
            if (member->GetQuestStatus(_ride->Quest) == QUEST_STATUS_INCOMPLETE)
                member->KilledMonsterCredit(NPC_CARAVAN_GOSSIP_WAGON);
        }
    }

    // Arrived (credit) or given up (none). The player is set down, his stop detections are given back by spell_area, and the
    // whole moving caravan goes (the accessories leave with their vehicle, Unit::_ExitVehicle).
    void End(bool arrived)
    {
        if (_ended)
            return;
        _ended = true;
        _events.Reset();

        // The moving caravan goes first and at once (its accessories would otherwise linger 2 s after leaving their vehicle,
        // Unit::_ExitVehicle), then the credit opens the next stop's window: the party is never seen twice.
        Player* player = GetRider();
        if (player && _shielded)
            player->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
        _shielded = false;
        bool credit = arrived && player && IsRiding(player);
        Position const arrival = me->GetPosition();
        if (player && IsRiding(player))
            player->ExitVehicle();
        DespawnCaravan();

        if (player)
        {
            if (credit)
            {
                if (player->GetQuestStatus(_ride->Quest) == QUEST_STATUS_INCOMPLETE)
                    player->KilledMonsterCredit(NPC_CARAVAN_GOSSIP_WAGON);
                CreditGroup(player, arrival);
            }
            RestoreCaravanSight(player);
        }
    }

    void DespawnCaravan()
    {
        if (Creature* wagon = GetWagon())
        {
            if (Vehicle* kit = wagon->GetVehicleKit())
                for (int8 seat = 0; seat < 8; ++seat)
                    if (Unit* passenger = kit->GetPassenger(seat))
                        if (passenger->GetTypeId() == TYPEID_UNIT && passenger->IsSummon())
                            passenger->ToTempSummon()->UnSummon();
            wagon->DespawnOrUnsummon();
        }
        if (Vehicle* kit = me->GetVehicleKit())
            for (int8 seat = 0; seat < 8; ++seat)
                if (Unit* passenger = kit->GetPassenger(seat))
                    if (passenger->GetTypeId() == TYPEID_UNIT && passenger->IsSummon())
                        passenger->ToTempSummon()->UnSummon();
        me->DespawnOrUnsummon();
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_ride || _ended)
            return;

        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_RIDE_BOARD:
                    Board();
                    break;
                case EVENT_RIDE_START:
                    Start();
                    break;
                case EVENT_RIDE_LINE:
                    SayLine();
                    break;
                case EVENT_RIDE_LAUGH:
                    if (Creature* fiona = GetPassenger(_ride->Fiona))
                        fiona->HandleEmoteCommand(EMOTE_ONESHOT_LAUGH);
                    break;
                case EVENT_RIDE_FINISH:
                    End(true);
                    return;
                case EVENT_RIDE_TIMEOUT:
                    TC_LOG_ERROR("scripts", "npc_epl_caravan_harness: ride %u did not reach its end in time", me->GetEntry());
                    End(false);
                    return;
                case EVENT_RIDE_CHECK:
                {
                    Player* player = GetRider();
                    if (!player || !player->IsInWorld() || !player->IsAlive() || (_boarded && !IsRiding(player)))
                    {
                        End(false);
                        return;
                    }
                    _events.ScheduleEvent(EVENT_RIDE_CHECK, 500ms);
                    break;
                }
                default:
                    break;
            }
            if (_ended)
                return;
        }
    }

private:
    CaravanRideInfo const* _ride = nullptr;
    ObjectGuid _playerGUID;
    EventMap _events;
    size_t _line = 0;
    std::vector<Position> _road;
    size_t _next = 0;
    bool _boarded = false;
    bool _shielded = false;
    bool _arrived = false;
    bool _linesDone = false;
    bool _finishing = false;
    bool _ended = false;
};

// ---------------------------------------------------------------------------------------------------------------------------
// The party between the rides
// ---------------------------------------------------------------------------------------------------------------------------

enum CaravanParty
{
    SAY_FIONA_BORROW_SUPPLIES       = 0,       // 46090, on accepting Onward

    QUEST_BEEZILS_FLAG              = 27563,   // [FLAG] Beezil's Flag (tracking)
    MENU_BEEZIL                     = 12081,
    GOSSIP_OPTION_BEEZIL_JOIN       = 0,       // "...you're free to join us in our caravan." -> menu 12082 "Sure, I'll join you"

    // Achievement 5442 Full Caravan: Fiona's and Beezil's criteria are "be the target of" these serverside spells, absent from
    // the client data, so they are credited directly
    SPELL_FULL_CARAVAN_FIONA        = 94998,
    SPELL_FULL_CARAVAN_BEEZIL       = 85811,
};

// 45417 - Fiona (every stop of the caravan)
// On accepting Onward 27373 she says "By the way, you're welcome to borrow any supplies you find inside the caravan... as long
// as you bring them back when you're done!" (wiki 27373, broadcast 46090), to the player who took it.
struct npc_epl_fiona : public ScriptedAI
{
    npc_epl_fiona(Creature* creature) : ScriptedAI(creature) { }

    void OnQuestAccept(Player* player, Quest const* quest) override
    {
        if (quest->GetQuestId() == QUEST_ONWARD)
            Talk(SAY_FIONA_BORROW_SUPPLIES, player);
    }

    // Fiona's own Full Caravan criterion: when the first ride is handed in to her, the moment she has taken the player along
    // (ours: the criterion's serverside spell is not in the client data and no source says when retail cast it)
    void OnQuestReward(Player* player, Quest const* quest, uint32 /*opt*/) override
    {
        if (quest->GetQuestId() == QUEST_ONWARD)
            player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_FULL_CARAVAN_FIONA);
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }
};

// 45431 / 45730 - Gidwin Goldbraids, three of his standing copies
// Pacing in front of Crown Guard Tower after the first ride (guid 401103, wiki 27373: "Alright, Gidwin... deep breaths..." and
// three more, 46100-46104), by the statue of Light's Hope nagging Tarenar (373696, wiki 27455, 45814-45816), circling inside the
// Eastwall tower (3128014, wiki 27466, 46108-46111). Each copy is quest-invisible to all but the players at that stage of the
// story, and creature text reaches every player in range whether he can see the speaker or not, so each line is said privately
// (creature_text TextRange 5) to each player within 25 yd who can see him; with nobody to hear he is silent. Every other copy
// of the two entries says nothing. Ours: one line every 40-60 s (the first 15-30 s after he is seen).
struct npc_epl_gidwin_idle : public ScriptedAI
{
    npc_epl_gidwin_idle(Creature* creature) : ScriptedAI(creature) { }

    int8 GetTextGroup() const
    {
        switch (me->GetDBTableGUIDLow())
        {
            case 401103:  return 0;   // 45431, Crown Guard Tower
            case 373696:  return 1;   // 45431, the statue of Light's Hope
            case 3128014: return 0;   // 45730, inside the Eastwall tower
            default:      return -1;
        }
    }

    void Reset() override
    {
        _timer = urand(15, 30) * IN_MILLISECONDS;
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }

    void UpdateAI(uint32 diff) override
    {
        int8 group = GetTextGroup();
        if (group < 0)
            return;

        if (_timer > diff)
        {
            _timer -= diff;
            return;
        }
        _timer = urand(40, 60) * IN_MILLISECONDS;

        std::list<Player*> players;
        me->GetPlayerListInGrid(players, 25.0f);
        for (Player* player : players)
            if (player->IsInWorld() && player->CanSeeOrDetect(me))
                sCreatureTextMgr->SendChat(me, uint8(group), player);
    }

private:
    uint32 _timer = 0;
};

// 46022 - Beezil Linkspanner (Light's Hope)
// Asked to join (menu 12081 option 0, both TDBs) he answers "Sure, I'll join you" (menu 12082, npc_text 16957) and is with the
// caravan from then on: his flag quest 27563 is rewarded, which switches his greeting to "Hey boss" and offers Beezil's Cog
// 27562 at the wagon (2026_11_14_05), and his Full Caravan criterion is credited. The core then shows the reply page (the
// option's ActionMenuID), so the gossip is not closed here. Sourced: the option, the reply, the flag quest, the criterion;
// ours: that choosing the option is what rewards the flag.
struct npc_epl_beezil : public ScriptedAI
{
    npc_epl_beezil(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != MENU_BEEZIL || gossipListId != GOSSIP_OPTION_BEEZIL_JOIN)
            return false;

        if (!player->IsQuestRewarded(QUEST_BEEZILS_FLAG))
            if (Quest const* quest = sObjectMgr->GetQuestTemplate(QUEST_BEEZILS_FLAG))
                player->RewardQuest(quest, 0, player, false);

        player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, SPELL_FULL_CARAVAN_BEEZIL);
        return false;
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }
    void AttackStart(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }
};

// ---------------------------------------------------------------------------------------------------------------------------
// The caravan's buffs
// ---------------------------------------------------------------------------------------------------------------------------

enum CaravanBuffs
{
    SPELL_FIONAS_LUCKY_CHARM        = 85612,   // 85612-85619: the eight buffs, quests 27555-27562 in the same order
    CARAVAN_BUFF_COUNT              = 8,
    QUEST_FIONAS_LUCKY_CHARM        = 27555,
    QUEST_ZONE_BUFF_FLAG_1          = 27552,   // [FLAG] EPL Zone Buff - Flag 1/2/3, 27552-27554
    ZONE_EASTERN_PLAGUELANDS        = 139,
};

// The chosen buff as the three flag quests' bits (0-7 = 85612-85619)
static void StoreCaravanBuff(Player* player, uint32 index)
{
    for (uint32 bit = 0; bit < 3; ++bit)
    {
        uint32 questId = QUEST_ZONE_BUFF_FLAG_1 + bit;
        bool want = (index & (1 << bit)) != 0;
        bool has = player->IsQuestRewarded(questId);
        if (want && !has)
        {
            if (Quest const* quest = sObjectMgr->GetQuestTemplate(questId))
                player->RewardQuest(quest, 0, player, false);
        }
        else if (!want && has)
            player->RemoveRewardedQuest(questId);
    }
}

// 85612-85619 - Fiona's Lucky Charm, Gidwin's Weapon Oil, Tarenar's Talisman, Pamela's Doll, Vex'tul's Armbands, Argus' Journal,
// Rimblat's Stone, Beezil's Cog
// "The last buff you chose will be there when you enter the zone again" (Wowhead, newhon63 [+3]): each buff is limited to the
// zone (AreaGroup 25) and goes when the player leaves it. The DBC's own means of remembering it are the hidden quests
// "[FLAG] EPL Zone Buff - Flag 1/2/3" (27552-27554, tracking): whichever buff the player gets, its number is kept in their three
// bits (set by rewarding, cleared by un-rewarding the flag quest; ours, retail's use of them is not recorded), a moment later
// so that it never runs inside the buff quest's own reward. player_epl_caravan_buff puts it back on entering the zone.
class spell_epl_caravan_buff : public AuraScript
{
    PrepareAuraScript(spell_epl_caravan_buff);

    void AfterApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || GetId() < SPELL_FIONAS_LUCKY_CHARM || GetId() >= SPELL_FIONAS_LUCKY_CHARM + CARAVAN_BUFF_COUNT)
            return;

        uint32 index = GetId() - SPELL_FIONAS_LUCKY_CHARM;
        player->m_Events.AddLambdaEventAtOffset([player, index]()
        {
            if (player->IsInWorld())
                StoreCaravanBuff(player, index);
        }, 1);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_epl_caravan_buff::AfterApply, EFFECT_0, SPELL_AURA_ANY, AURA_EFFECT_HANDLE_REAL);
    }
};

// Entering Eastern Plaguelands (also at login there): the buff the player chose last comes back, unless one is already on.
// "Chose one" is a rewarded buff quest (they are repeatable, and stay rewarded); bits 000 count only if it was the charm.
class player_epl_caravan_buff : public PlayerScript
{
public:
    player_epl_caravan_buff() : PlayerScript("player_epl_caravan_buff") { }

    void OnUpdateZone(Player* player, uint32 newZone, uint32 /*newArea*/) override
    {
        if (newZone != ZONE_EASTERN_PLAGUELANDS)
            return;

        bool chosen = false;
        for (uint32 i = 0; i < CARAVAN_BUFF_COUNT; ++i)
        {
            if (player->HasAura(SPELL_FIONAS_LUCKY_CHARM + i))
                return;
            if (player->IsQuestRewarded(QUEST_FIONAS_LUCKY_CHARM + i))
                chosen = true;
        }
        if (!chosen)
            return;

        uint32 index = 0;
        for (uint32 bit = 0; bit < 3; ++bit)
            if (player->IsQuestRewarded(QUEST_ZONE_BUFF_FLAG_1 + bit))
                index |= 1 << bit;

        if (!index && !player->IsQuestRewarded(QUEST_FIONAS_LUCKY_CHARM))
            return;

        player->CastSpell(player, SPELL_FIONAS_LUCKY_CHARM + index, true);
    }
};

// ---------------------------------------------------------------------------------------------------------------------------
// The followers: every companion of the zone's quests on one script
// ---------------------------------------------------------------------------------------------------------------------------

enum CaravanFollowers
{
    NPC_TARENAR_DEATHS_STEP         = 45445,   // What I Do Best 27371 (84902)
    NPC_GIDWIN_UNTABARDED           = 45677,   // A Boyhood Dream 27463, The Trial of the Crypt 27464 (85218)
    NPC_TARENAR_UNTABARDED          = 45678,   // 27463, 27464 (85217)
    NPC_TARENAR_TABARD              = 45792,   // The Noxious Glade 27465 (85402)
    NPC_GIDWIN_TABARD               = 45793,   // 27465 (85404)
    NPC_TARENAR_IXLAR               = 45794,   // Ix'lar the Underlord 27487 (85405)

    SPELL_SUMMON_TARENAR_DEATHS_STEP = 84902,
    SPELL_SUMMON_GIDWIN_UNTABARDED  = 85218,
    SPELL_SUMMON_TARENAR_UNTABARDED = 85217,
    SPELL_SUMMON_TARENAR_TABARD     = 85402,
    SPELL_SUMMON_GIDWIN_TABARD      = 85404,
    SPELL_SUMMON_TARENAR_IXLAR      = 85405,

    QUEST_WHAT_I_DO_BEST            = 27371,
    QUEST_A_BOYHOOD_DREAM           = 27463,
    QUEST_TRIAL_OF_THE_CRYPT        = 27464,
    QUEST_THE_NOXIOUS_GLADE         = 27465,
    QUEST_IXLAR_THE_UNDERLORD       = 27487,

    AREA_DEATHS_STEP                = 5446,
    AREA_LIGHTS_HOPE_CHAPEL         = 2268,
    AREA_BROWMAN_MILL               = 2269,
    AREA_THE_NOXIOUS_GLADE          = 2270,
    AREA_PESTILENT_SCAR             = 2622,    // both border Light's Hope on the way to the glade
    AREA_DEATHS_BREACH              = 4544,
    AREA_EASTWALL_TOWER             = 2271,
    AREA_IXLARS_DOMAIN              = 5463,
    AREA_ZULMASHAR                  = 2273,

    NPC_VEXTUL_FOLLOWER             = 45741,   // Out of the Ziggurat 27481 / Into the Flames 27482 (85334)
    SPELL_SUMMON_VEXTUL             = 85334,
    QUEST_OUT_OF_THE_ZIGGURAT       = 27481,
    QUEST_INTO_THE_FLAMES           = 27482,

    // The paladins' buffs (the standing Tarenar 45429 / 45729 and Gidwin 45431 / 45730 carry the same spells)
    SPELL_BLESSING_OF_MIGHT         = 79977,
    SPELL_BLESSING_OF_KINGS         = 79968,
    SPELL_RETRIBUTION_AURA          = 79976,
    SPELL_DEVOTION_AURA             = 8258,
    SPELL_SEAL_OF_TRUTH             = 79972,
    SPELL_DIVINE_STORM              = 79970,

    SAY_FOLLOWER_COMBAT             = 0,
    SAY_FOLLOWER_KILL               = 1,       // Tarenar 45445 only

    // George's cue when the Trial of the Crypt begins (zone_eastern_plaguelands_chapel.cpp)
    ACTION_TRIAL_OF_THE_CRYPT_BEGINS = 27464,

    DATA_FOLLOWER_BUSY              = 1,

    EVENT_FOLLOWER_SETUP            = 1,
    EVENT_FOLLOWER_ASSIST,
    EVENT_FOLLOWER_CHECK,
    EVENT_FOLLOWER_BUFF,
    EVENT_FOLLOWER_DIVINE_STORM,
};

enum class FollowerKind : uint8
{
    Tarenar,
    Gidwin,
    Vextul,
};

struct CaravanFollowerInfo
{
    uint32 Entry;
    uint32 Spell;                   // its summon; effect 1 is the dummy aura on the player that stands for "he is out"
    std::vector<uint32> Quests;     // he stays while one of them is in the log (incomplete or complete)
    std::vector<uint32> Areas;      // and the player is in one of these
    FollowerKind Kind;
};

// The areas are each spell's AreaGroup (SpellCastingRequirements), plus Browman Mill 2269, Pestilent Scar 2622 and Death's Breach
// 4544 for the chapel pairs: they lie between the chapel and the Noxious Glade, and "they never left" the player there
// (chapel-argent 27464 claim 13).
std::vector<CaravanFollowerInfo> const CaravanFollowerInfos =
{
    { NPC_TARENAR_DEATHS_STEP, SPELL_SUMMON_TARENAR_DEATHS_STEP, { QUEST_WHAT_I_DO_BEST },
        { AREA_DEATHS_STEP }, FollowerKind::Tarenar },
    { NPC_GIDWIN_UNTABARDED, SPELL_SUMMON_GIDWIN_UNTABARDED, { QUEST_A_BOYHOOD_DREAM, QUEST_TRIAL_OF_THE_CRYPT },
        { AREA_LIGHTS_HOPE_CHAPEL, AREA_BROWMAN_MILL, AREA_THE_NOXIOUS_GLADE, AREA_PESTILENT_SCAR, AREA_DEATHS_BREACH }, FollowerKind::Gidwin },
    { NPC_TARENAR_UNTABARDED, SPELL_SUMMON_TARENAR_UNTABARDED, { QUEST_A_BOYHOOD_DREAM, QUEST_TRIAL_OF_THE_CRYPT },
        { AREA_LIGHTS_HOPE_CHAPEL, AREA_BROWMAN_MILL, AREA_THE_NOXIOUS_GLADE, AREA_PESTILENT_SCAR, AREA_DEATHS_BREACH }, FollowerKind::Tarenar },
    { NPC_TARENAR_TABARD, SPELL_SUMMON_TARENAR_TABARD, { QUEST_THE_NOXIOUS_GLADE },
        { AREA_LIGHTS_HOPE_CHAPEL, AREA_BROWMAN_MILL, AREA_THE_NOXIOUS_GLADE, AREA_PESTILENT_SCAR, AREA_DEATHS_BREACH }, FollowerKind::Tarenar },
    { NPC_GIDWIN_TABARD, SPELL_SUMMON_GIDWIN_TABARD, { QUEST_THE_NOXIOUS_GLADE },
        { AREA_LIGHTS_HOPE_CHAPEL, AREA_BROWMAN_MILL, AREA_THE_NOXIOUS_GLADE, AREA_PESTILENT_SCAR, AREA_DEATHS_BREACH }, FollowerKind::Gidwin },
    { NPC_TARENAR_IXLAR, SPELL_SUMMON_TARENAR_IXLAR, { QUEST_IXLAR_THE_UNDERLORD },
        { AREA_EASTWALL_TOWER, AREA_IXLARS_DOMAIN }, FollowerKind::Tarenar },
    // Its quests are a pair: from Out of the Ziggurat taken until Into the Flames is credited (the existing spell_area row)
    { NPC_VEXTUL_FOLLOWER, SPELL_SUMMON_VEXTUL, { QUEST_OUT_OF_THE_ZIGGURAT, QUEST_INTO_THE_FLAMES },
        { AREA_ZULMASHAR }, FollowerKind::Vextul },
};

static CaravanFollowerInfo const* GetFollowerInfoByEntry(uint32 entry)
{
    for (CaravanFollowerInfo const& info : CaravanFollowerInfos)
        if (info.Entry == entry)
            return &info;
    return nullptr;
}

static CaravanFollowerInfo const* GetFollowerInfoBySpell(uint32 spell)
{
    for (CaravanFollowerInfo const& info : CaravanFollowerInfos)
        if (info.Spell == spell)
            return &info;
    return nullptr;
}

// Is the follower's reason still there: his quest in the player's log and the player where he goes with him
static bool KeepsCaravanFollower(Player* player, CaravanFollowerInfo const* info)
{
    if (!player || !info || !player->IsInWorld() || !player->IsAlive())
        return false;

    bool questOk = false;
    if (info->Kind == FollowerKind::Vextul)
    {
        QuestStatus first = player->GetQuestStatus(QUEST_OUT_OF_THE_ZIGGURAT);
        QuestStatus second = player->GetQuestStatus(QUEST_INTO_THE_FLAMES);
        questOk = first != QUEST_STATUS_NONE && first != QUEST_STATUS_FAILED
            && (second == QUEST_STATUS_NONE || second == QUEST_STATUS_INCOMPLETE);
    }
    else for (uint32 questId : info->Quests)
    {
        QuestStatus status = player->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_COMPLETE)
            questOk = true;
    }
    // A Boyhood Dream is handed in to Tyrosus, who offers The Trial of the Crypt in the same window: the pair stays with the player
    // across that hand-over while he is in the chapel's area, and goes if he leaves it without taking the Trial (ours)
    if (!questOk && (info->Entry == NPC_GIDWIN_UNTABARDED || info->Entry == NPC_TARENAR_UNTABARDED)
        && player->GetQuestStatus(QUEST_A_BOYHOOD_DREAM) == QUEST_STATUS_REWARDED
        && player->GetQuestStatus(QUEST_TRIAL_OF_THE_CRYPT) == QUEST_STATUS_NONE)
        return player->GetAreaId() == AREA_LIGHTS_HOPE_CHAPEL;

    if (!questOk)
        return false;

    return std::find(info->Areas.begin(), info->Areas.end(), player->GetAreaId()) != info->Areas.end();
}

static Creature* FindLivingCaravanFollower(Unit* owner, uint32 entry)
{
    std::list<Creature*> minions;
    owner->GetAllMinionsByEntry(minions, entry);
    for (Creature* minion : minions)
        if (minion->IsAlive() && !minion->IsDestroyedObject())
            return minion;
    return nullptr;
}

// What a follower attacks for his master: his melee target, else his selected target if he is fighting it (a caster's spells set
// no melee victim), else whatever attacks him (as npc_wpl_provincial_minuteman in Western Plaguelands)
static Unit* SelectFollowerTarget(Creature* me, Unit* owner)
{
    auto canFight = [me](Unit* target)
    {
        return target && target->IsAlive() && me->IsWithinDistInMap(target, 50.0f) && me->IsValidAttackTarget(target);
    };

    Unit* victim = owner->GetVictim();
    if (canFight(victim))
        return victim;

    if (Player* player = owner->ToPlayer())
    {
        Unit* selected = player->GetSelectedUnit();
        if (selected && player->IsInCombatWith(selected) && canFight(selected))
            return selected;
    }

    for (Unit* attacker : owner->getAttackers())
        if (canFight(attacker))
            return attacker;

    return nullptr;
}

// 84902, 85217, 85218, 85402, 85404, 85405 - Summon Tarenar Sunstrike / Summon Gidwin Goldbraids (effect 0, the summon)
// Each is cast on the player by spell_area while its quest is in his log in the spell's areas (on accept, on entering the area,
// at login and after resurrection there: Player::SendQuestUpdate and UpdateZone run the area auras). Effect 0 summons the
// follower as the player's guardian (SummonProperties 3030 / 713: at the player's level, with his faction), effect 1 puts the
// dummy aura on the player that the staging rows use to hide the standing copy while the follower is out (aura_spell -84902 at
// the Marris Stead, -85405 at Eastwall).
// - Never two: a player who still has a living follower of that entry (the aura was re-cast after he walked through Browman
//   Mill, or after an area change) keeps him; a dead one is cleared first.
// - Slots: 3030 (84902) is the quest summon slot 6, where a new summon unsummons the old one (TempSummon::InitStats); the slot is
//   left as it was, so Tarenar and another companion in that slot never evict each other (the Zen'Kiki and val'kyr fault of
//   Western Plaguelands; Pamela's Doll uses slot 6 every 2 minutes). 713 has no slot.
// - A dead player gets no aura: spell_area casts triggered, which passes a dead caster, but the summon effect does nothing for
//   one (Spell::EffectSummonType), so he would have had the aura and no follower, and the standing copy hidden, until he left
//   the area. Refused here, resurrection casts it.
class spell_epl_follower_summon : public SpellScript
{
    PrepareSpellScript(spell_epl_follower_summon);

    SpellCastResult CheckCast()
    {
        if (!GetCaster()->IsAlive())
            return SPELL_FAILED_CASTER_DEAD;
        return SPELL_CAST_OK;
    }

    void HandleSummon(SpellEffIndex effIndex)
    {
        PreventHitDefaultEffect(effIndex);

        Unit* caster = GetCaster();
        if (!caster)
            return;

        uint32 entry = uint32(GetSpellInfo()->Effects[effIndex].MiscValue);
        if (FindLivingCaravanFollower(caster, entry))
            return;
        caster->RemoveAllMinionsByEntry(entry);

        int32 slot = 0;
        if (SummonPropertiesEntry const* properties = sSummonPropertiesStore.LookupEntry(GetSpellInfo()->Effects[effIndex].MiscValueB))
            slot = properties->Slot;

        if (slot > 0 && slot < SUMMON_SLOT_MAX)
        {
            ObjectGuid const kept = caster->m_SummonSlot[slot];
            caster->m_SummonSlot[slot].Clear();
            GetSpell()->EffectSummonType(effIndex);
            caster->m_SummonSlot[slot] = kept;
        }
        else
            GetSpell()->EffectSummonType(effIndex);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_follower_summon::CheckCast);
        OnEffectHit += SpellEffectFn(spell_epl_follower_summon::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};

// The same spells, effect 1 (the dummy aura on the player)
// The summon is not tied to the aura in this core, so the follower stayed when the aura went. He now goes with it: on reward
// (spell_area no longer fits; for 27487 also its RewardSpell 85807), on abandon, on leaving the spell's areas, and is unsummoned
// with the player at death or logout. Exception: the chapel pairs stay through Browman Mill, where the aura cannot be (the
// spells' AreaGroup is the chapel and the glade only); their own check takes them away anywhere else.
class spell_epl_follower_aura : public AuraScript
{
    PrepareAuraScript(spell_epl_follower_aura);

    void AfterRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
            return;

        CaravanFollowerInfo const* info = GetFollowerInfoBySpell(GetId());
        Player* player = GetTarget()->ToPlayer();
        if (!info || !player)
            return;

        // Still on his way with the player (Browman Mill), or just died (his corpse goes by itself)
        if (KeepsCaravanFollower(player, info))
            return;

        player->RemoveAllMinionsByEntry(info->Entry);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_epl_follower_aura::AfterRemove, EFFECT_1, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

// At login: a summon aura saved under the old data (before these auras were made DONT_SAVE) stands for a follower who is gone,
// blocks spell_area's recast (it casts only when the aura is absent) and keeps the standing copy hidden. It is dropped and the
// area auras run again, which brings the follower back where his quest and area fit.
class player_epl_caravan_follower : public PlayerScript
{
public:
    player_epl_caravan_follower() : PlayerScript("player_epl_caravan_follower") { }

    void OnLogin(Player* player) override
    {
        bool removed = false;
        for (CaravanFollowerInfo const& info : CaravanFollowerInfos)
            if (player->HasAura(info.Spell) && !FindLivingCaravanFollower(player, info.Entry))
            {
                player->RemoveAurasDueToSpell(info.Spell);
                removed = true;
            }

        if (removed)
            RestoreCaravanSight(player);
    }
};

// 45445, 45677, 45678, 45792, 45793, 45794 - Tarenar Sunstrike / Gidwin Goldbraids, following the player
// What the player sees (sourced, Wowhead comments of 27371, 27463-27465, 27487 and the wiki): the follower appears beside him,
// follows him (mounted too), joins what he fights ("Tarenar should engage", "he will attack what you attack"), and buffs him,
// his pet and his party: Tarenar Blessing of Might, Retribution Aura and Seal of Truth, Gidwin Blessing of Kings and Devotion
// Aura ("2 auras and 2 blessings ... devotion, ret, might and kings"), again when a blessing has run out. Tarenar of 27371 opens
// with Divine Storm and says "Take that! Taste the Light!" / "Ahhhhh ha ha haa!" and "Wow. Is it just me, or did that one die
// really fast?"; the Gidwins shout "Ka-boom!" and the three other lines in fights. He can die; when the Trial of the Crypt
// begins they rebuff the party (the scroll's buff, chapel-argent: which buff it was is not recorded).
// How (ours, as Thurman's minutemen and Zen'Kiki in Western Plaguelands):
// - A plain guardian starts aggressive (Guardian::InitStats) and pulls every pack he passes; he is defensive here, starts no fight
//   of his own and follows a mounted master without stopping for what he passes.
// - He is healable: the templates' IMMUNE_TO_PC (45677, 45794) would refuse every heal of the player (Unit::_IsValidAssistTarget).
// - Left more than 60 yd behind (the player rode on while he fought), he is brought to the player.
// - He goes when his quest is no longer in the log or the player has left his areas (also covers 27487's reward, where the
//   cancel spell 85807 removes the aura before the quest is marked rewarded).
// - When he dies, the player's summon aura is removed: the standing copy hidden by it is visible again (sourced for 27371: "the
//   questgiver is back"), and at the latest 30 s later the area auras are run again for the player, which brings a new follower
//   if the quest and area still fit (sooner on any area change or quest status change, e.g. the player's next kill credit;
//   ours; retail needed a relog, a fault players reported). His corpse goes after 5 s.
// - Private actors (the Trial's wardens and Lord Raymond George are private to the reader's group) are seen by him as his master
//   sees them, so he can fight them (CanSeeAlways; WorldObject::CanSeeOrDetect asks it before the private-object check). His
//   spells still skip them (SpellInfo::CheckTarget refuses a private target to anyone outside its owner's group): he fights
//   them in melee.
struct npc_epl_caravan_follower : public ScriptedAI
{
    npc_epl_caravan_follower(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        _info = GetFollowerInfoByEntry(me->GetEntry());
        if (!_info || summoner->GetTypeId() != TYPEID_PLAYER)
            return;

        _ownerGUID = summoner->GetGUID();
        me->SetReactState(REACT_DEFENSIVE);
        // Left behind out of the player's sight (he rode on, or was moved), his grid would stop updating and he would stand
        // there for good while the player kept the summon aura, which blocks a new summon. Active, Check() brings him along.
        me->setActive(true, ActiveFlags::Script);

        _events.Reset();
        // Faction and level are set by Spell::SummonGuardian after this hook
        _events.ScheduleEvent(EVENT_FOLLOWER_SETUP, 500ms);
        _events.ScheduleEvent(EVENT_FOLLOWER_ASSIST, 1s);
        _events.ScheduleEvent(EVENT_FOLLOWER_CHECK, 1s);
        _events.ScheduleEvent(EVENT_FOLLOWER_BUFF, 1500ms);
    }

    Player* GetOwnerPlayer() const
    {
        return ObjectAccessor::GetPlayer(*me, _ownerGUID);
    }

    bool CanSeeAlways(WorldObject const* obj) override
    {
        if (!obj->IsPrivateObject())
            return false;
        Player* owner = GetOwnerPlayer();
        return owner && obj->CheckPrivateObjectOwnerVisibility(owner);
    }

    void MoveInLineOfSight(Unit* /*who*/) override { }

    void AttackStart(Unit* who) override
    {
        if (!who || IsBusy())
            return;

        // Riding past: only what has turned on him
        if (Player* owner = GetOwnerPlayer())
            if (owner->IsMounted() && who->GetVictim() != me)
                return;

        ScriptedAI::AttackStart(who);
    }

    void JustEngagedWith(Unit* who) override
    {
        ScriptedAI::JustEngagedWith(who);
        if (!_info)
            return;

        if (me->GetEntry() == NPC_TARENAR_DEATHS_STEP)
        {
            _events.ScheduleEvent(EVENT_FOLLOWER_DIVINE_STORM, 500ms);
            if (roll_chance_i(30))
                Talk(SAY_FOLLOWER_COMBAT);
        }
        else if (_info->Kind == FollowerKind::Gidwin && !_talkCooldown)
        {
            Talk(SAY_FOLLOWER_COMBAT);
            _talkCooldown = urand(20, 40) * IN_MILLISECONDS;
        }
    }

    void KilledUnit(Unit* /*victim*/) override
    {
        if (me->GetEntry() == NPC_TARENAR_DEATHS_STEP && roll_chance_i(30))
            Talk(SAY_FOLLOWER_KILL);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        ScriptedAI::EnterEvadeMode(why);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
    }

    void JustDied(Unit* /*killer*/) override
    {
        _events.Reset();
        me->DespawnOrUnsummon(5000);

        Player* owner = GetOwnerPlayer();
        if (!owner || !_info)
            return;

        owner->RemoveAurasDueToSpell(_info->Spell);
        owner->m_Events.AddLambdaEventAtOffset([owner]()
        {
            if (owner->IsInWorld() && owner->IsAlive())
            {
                owner->UpdateZoneDependentAuras(owner->GetZoneId());
                owner->UpdateAreaDependentAuras(owner->GetAreaId());
            }
        }, 30 * IN_MILLISECONDS);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_TRIAL_OF_THE_CRYPT_BEGINS)
            Buff(true);
    }

    uint32 GetData(uint32 type) const override
    {
        return type == DATA_FOLLOWER_BUSY && IsBusy() ? 1 : 0;
    }

    virtual bool IsBusy() const { return false; }

    void Setup()
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        if (Player* owner = GetOwnerPlayer())
            if (!me->IsInCombat() && !me->GetVictim())
            {
                me->GetMotionMaster()->Clear(false);
                me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
            }
    }

    void BuffUnit(Unit* target, uint32 spell, bool force)
    {
        if (!target || !target->IsAlive() || !target->IsInMap(me) || !me->IsWithinDistInMap(target, 30.0f))
            return;
        if (!force && target->HasAura(spell))
            return;
        me->CastSpell(target, spell, true);
    }

    // His own aura (and seal), and his blessing on the player, his pet and his party members and their pets in reach
    void Buff(bool force)
    {
        Player* owner = GetOwnerPlayer();
        if (!owner || !_info || _info->Kind == FollowerKind::Vextul)
            return;

        uint32 blessing = _info->Kind == FollowerKind::Tarenar ? SPELL_BLESSING_OF_MIGHT : SPELL_BLESSING_OF_KINGS;
        if (_info->Kind == FollowerKind::Tarenar)
        {
            BuffUnit(me, SPELL_RETRIBUTION_AURA, force);
            BuffUnit(me, SPELL_SEAL_OF_TRUTH, force);
        }
        else
            BuffUnit(me, SPELL_DEVOTION_AURA, force);

        std::vector<Player*> players;
        if (Group* group = owner->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                if (Player* member = ref->GetSource())
                    players.push_back(member);
        }
        else
            players.push_back(owner);

        for (Player* player : players)
        {
            BuffUnit(player, blessing, force);
            if (Pet* pet = player->GetPet())
                BuffUnit(pet, blessing, force);
        }
    }

    // His master is gone, or his quest, or the player has left his areas: he goes. Left far behind: brought along.
    bool Check()
    {
        Player* owner = GetOwnerPlayer();
        bool keeps = owner && owner->IsInWorld() && owner->GetMapId() == me->GetMapId();
        if (keeps && !KeepsCaravanFollower(owner, _info))
        {
            // The summon aura decides while the player lives: it goes with the quest and the areas (spell_area) and takes him
            // along (spell_epl_follower_aura). His own look at the player's area can fall on a zone-only cell the player's
            // area update never saw, and he then went while the aura stayed, which blocks the next summon.
            keeps = _info->Kind != FollowerKind::Vextul && owner->IsAlive() && owner->HasAura(_info->Spell);
            // Going with the aura still on (the player died): the aura goes too, so that the area auras can bring him back
            if (!keeps)
                owner->RemoveAurasDueToSpell(_info->Spell);
        }
        if (!keeps)
        {
            me->DespawnOrUnsummon();
            return false;
        }

        if (!IsBusy() && !me->IsWithinDistInMap(owner, 60.0f) && !owner->IsInFlight() && !owner->GetVehicle())
        {
            if (me->IsInCombat() || me->GetVictim())
                EnterEvadeMode(EVADE_REASON_OTHER);

            Position pos = owner->GetNearPosition(owner->GetObjectSize(), 0.0f);
            me->NearTeleportTo(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), owner->GetOrientation());
            me->GetMotionMaster()->Clear(false);
            me->GetMotionMaster()->MoveFollow(owner, PET_FOLLOW_DIST, me->GetFollowAngle());
        }
        return true;
    }

    void Assist()
    {
        Player* owner = GetOwnerPlayer();
        if (!owner || IsBusy())
            return;

        if (owner->IsMounted())
        {
            // A mounted player fights nothing (Unit::Attack); a follower in a fight he was only flagged into follows instead
            if (me->IsInCombat() && me->getAttackers().empty())
                EnterEvadeMode(EVADE_REASON_OTHER);
            return;
        }

        if (!me->GetVictim() && owner->IsInCombat())
            if (Unit* victim = SelectFollowerTarget(me, owner))
                AttackStart(victim);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_info)
            return;

        if (_talkCooldown)
            _talkCooldown = _talkCooldown > diff ? _talkCooldown - diff : 0;

        _events.Update(diff);
        while (uint32 eventId = _events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FOLLOWER_SETUP:
                    Setup();
                    break;
                case EVENT_FOLLOWER_ASSIST:
                    Assist();
                    _events.ScheduleEvent(EVENT_FOLLOWER_ASSIST, 1s);
                    break;
                case EVENT_FOLLOWER_CHECK:
                    if (!Check())
                        return;
                    _events.ScheduleEvent(EVENT_FOLLOWER_CHECK, 1s);
                    break;
                case EVENT_FOLLOWER_BUFF:
                    if (!me->IsInCombat() && !IsBusy())
                        Buff(false);
                    _events.ScheduleEvent(EVENT_FOLLOWER_BUFF, 10s);
                    break;
                case EVENT_FOLLOWER_DIVINE_STORM:
                    if (me->IsInCombat())
                    {
                        if (Unit* victim = me->GetVictim())
                            if (me->IsWithinMeleeRange(victim))
                                me->CastSpell(me, SPELL_DIVINE_STORM, true);
                        _events.ScheduleEvent(EVENT_FOLLOWER_DIVINE_STORM, Milliseconds(urand(8000, 10000)));
                    }
                    break;
                default:
                    break;
            }
        }

        if (IsBusy() || !UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

protected:
    CaravanFollowerInfo const* _info = nullptr;
    ObjectGuid _ownerGUID;
    EventMap _events;
    uint32 _talkCooldown = 0;
};

enum VextulFarewell
{
    NPC_INTO_THE_FLAMES_CREDIT      = 45738,   // the bunny on the bonfire, and the quest's kill credit
    SPELL_DESTROY_THRESHJIN_BODY    = 85327,
    SPELL_VEXTUL_DEATH              = 85337,

    SAY_VEXTUL_GOING                = 0,       // 45810
    SAY_VEXTUL_GREEN_FIELDS         = 1,       // 45811

    ACTION_VEXTUL_FAREWELL          = 1,

    POINT_VEXTUL_FIRE_EDGE          = 1,
    POINT_VEXTUL_FIRE               = 2,

    EVENT_FAREWELL_GOING            = 1,
    EVENT_FAREWELL_GREEN_FIELDS,
    EVENT_FAREWELL_JUMP,
    EVENT_FAREWELL_DIE,
};

// The edge of the bonfire on the ziggurat and the fire itself (the former SmartAI list 4574100: mop's own positions; the fire
// is the spot of the credit bunny 375819)
Position const VextulFireEdge = { 3456.7f, -4998.2f, 198.8f, 0.0f };
Position const VextulFire     = { 3462.1f, -5004.1f, 202.5f, 0.0f };

// 45741 - Vex'tul (Out of the Ziggurat 27481 / Into the Flames 27482, summoned by 85334 in Zul'Mashar)
// The follower above, without buffs or lines in fights ("he could actually fight worth a damn"; no ability is sourced), and his
// end in the fire (wiki notes of 27482): when his master burns Thresh'jin's body at the bonfire he stops fighting, walks to the
// fire, says "I will be goin' now. I have no reason to remain here. I may be undead, but my will is my own." and "I will see you
// in green fields on the other side, <name>.", jumps into the fire and dies there (Vex'tul Death 85337, in the quest's id block),
// and his death gives his master the credit of Into the Flames; his body is gone a moment later (the quest's credit removes
// the summon aura, which takes him away). Before, the bonfire bunny's SmartAI picked the nearest Vex'tul within 20 yd, so a
// player whose Vex'tul had died or lagged behind got nothing, another player's Vex'tul could take the credit, and the dead
// Vex'tul lay on the fire until a hand-in to him (no longer possible: he is neither giver nor ender, 2026_11_14_38).
// Once the farewell has begun the credit is his master's even if Vex'tul is taken away before he dies (the master leaves
// Zul'Mashar or dies meanwhile). Ours: the 4 s / 4 s / 2 s steps (the old list's), the fall-back kill if the death spell fails,
// the credit on an interrupted farewell.
struct npc_epl_vextul_follower : public npc_epl_caravan_follower
{
    npc_epl_vextul_follower(Creature* creature) : npc_epl_caravan_follower(creature) { }

    bool IsBusy() const override { return _farewell; }

    void DoAction(int32 action) override
    {
        if (action != ACTION_VEXTUL_FAREWELL || _farewell || !me->IsAlive())
            return;

        _farewell = true;
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetWalk(false);
        me->GetMotionMaster()->Clear(false);
        me->GetMotionMaster()->MovePoint(POINT_VEXTUL_FIRE_EDGE, VextulFireEdge);
        // Whether or not he gets there (the ziggurat's top is small), he speaks after a few seconds
        _farewellEvents.ScheduleEvent(EVENT_FAREWELL_GOING, 4s);
    }

    void MovementInform(uint32 type, uint32 id) override
    {
        if (_farewell && type == EFFECT_MOTION_TYPE && id == POINT_VEXTUL_FIRE)
            _farewellEvents.ScheduleEvent(EVENT_FAREWELL_DIE, 100ms);
    }

    void EnterEvadeMode(EvadeReason why) override
    {
        if (_farewell)
            return;
        npc_epl_caravan_follower::EnterEvadeMode(why);
    }

    void JustDied(Unit* killer) override
    {
        if (!_farewell)
        {
            npc_epl_caravan_follower::JustDied(killer);
            return;
        }

        _farewellEvents.Reset();
        me->DespawnOrUnsummon(5000);
        CreditOwner();
    }

    // Once the farewell has begun the credit is his master's whatever happens next: the master leaves Zul'Mashar or dies
    // during it (the summon aura goes and takes Vex'tul with it) or he is unsummoned for any other reason
    void Unsummoned() override
    {
        if (_farewell)
            CreditOwner();
    }

    void CreditOwner()
    {
        if (_credited)
            return;
        _credited = true;
        if (Player* owner = GetOwnerPlayer())
            if (owner->GetQuestStatus(QUEST_INTO_THE_FLAMES) == QUEST_STATUS_INCOMPLETE)
                owner->KilledMonsterCredit(NPC_INTO_THE_FLAMES_CREDIT);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!_farewell)
        {
            npc_epl_caravan_follower::UpdateAI(diff);
            return;
        }

        _farewellEvents.Update(diff);
        while (uint32 eventId = _farewellEvents.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_FAREWELL_GOING:
                    me->SetFacingTo(me->GetAngle(&VextulFire));
                    Talk(SAY_VEXTUL_GOING, GetOwnerPlayer());
                    _farewellEvents.ScheduleEvent(EVENT_FAREWELL_GREEN_FIELDS, 4s);
                    break;
                case EVENT_FAREWELL_GREEN_FIELDS:
                    Talk(SAY_VEXTUL_GREEN_FIELDS, GetOwnerPlayer());
                    _farewellEvents.ScheduleEvent(EVENT_FAREWELL_JUMP, 2s);
                    break;
                case EVENT_FAREWELL_JUMP:
                    me->GetMotionMaster()->Clear(false);
                    me->GetMotionMaster()->MoveJump(VextulFire, 10.0f, 8.0f, POINT_VEXTUL_FIRE);
                    // In case the jump never reports its end
                    _farewellEvents.ScheduleEvent(EVENT_FAREWELL_DIE, 4s);
                    break;
                case EVENT_FAREWELL_DIE:
                    if (me->IsAlive())
                    {
                        me->CastSpell(me, SPELL_VEXTUL_DEATH, true);
                        if (me->IsAlive())
                            me->Kill(me, false);
                    }
                    return;
                default:
                    break;
            }
        }
    }

private:
    bool _farewell = false;
    bool _credited = false;
    EventMap _farewellEvents;
};

// 85327 - Destroy Thresh'jin's Body (item 61316, Into the Flames 27482), effect 0 on the bunny 45738 on the bonfire
// The thrower's own Vex'tul (living, within 40 yd, not already saying goodbye) takes his leave and credits him with his death
// (npc_epl_vextul_follower). A thrower without his Vex'tul at hand (he died, the player relogged in the area and has not been
// re-joined yet, he is somewhere below) is credited at once (ours: retail left players stuck there, Wowhead 2013 comments).
// Another player's Vex'tul is never touched.
class spell_epl_destroy_threshjin_body : public SpellScript
{
    PrepareSpellScript(spell_epl_destroy_threshjin_body);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Player* player = GetCaster() ? GetCaster()->ToPlayer() : nullptr;
        if (!player || player->GetQuestStatus(QUEST_INTO_THE_FLAMES) != QUEST_STATUS_INCOMPLETE)
            return;

        if (Creature* vextul = FindLivingCaravanFollower(player, NPC_VEXTUL_FOLLOWER))
        {
            if (vextul->IsAIEnabled && vextul->IsWithinDistInMap(player, 40.0f))
            {
                if (!vextul->AI()->GetData(DATA_FOLLOWER_BUSY))
                    vextul->AI()->DoAction(ACTION_VEXTUL_FAREWELL);
                return;
            }
        }

        player->KilledMonsterCredit(NPC_INTO_THE_FLAMES_CREDIT);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_epl_destroy_threshjin_body::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

void AddSC_eastern_plaguelands_caravan()
{
    new creature_script<npc_epl_caravan_wagon>("npc_epl_caravan_wagon");
    new creature_script<npc_epl_caravan_harness>("npc_epl_caravan_harness");
    new creature_script<npc_epl_fiona>("npc_epl_fiona");
    new creature_script<npc_epl_gidwin_idle>("npc_epl_gidwin_idle");
    new creature_script<npc_epl_beezil>("npc_epl_beezil");
    new aura_script<spell_epl_caravan_buff>("spell_epl_caravan_buff");
    new player_epl_caravan_buff();
    new player_epl_caravan_follower();
    new spell_script<spell_epl_follower_summon>("spell_epl_follower_summon");
    new aura_script<spell_epl_follower_aura>("spell_epl_follower_aura");
    new creature_script<npc_epl_caravan_follower>("npc_epl_caravan_follower");
    new creature_script<npc_epl_vextul_follower>("npc_epl_vextul_follower");
    new spell_script<spell_epl_destroy_threshjin_body>("spell_epl_destroy_threshjin_body");
}
