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
// 27373 and 27448).
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

void AddSC_eastern_plaguelands_caravan()
{
    new creature_script<npc_epl_caravan_wagon>("npc_epl_caravan_wagon");
    new creature_script<npc_epl_caravan_harness>("npc_epl_caravan_harness");
}
