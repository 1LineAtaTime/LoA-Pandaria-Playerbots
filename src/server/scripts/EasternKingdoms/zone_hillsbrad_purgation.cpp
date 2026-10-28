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
    ACTION_HEROES_ABORT                         = 2,        // Orkus 48503 / Kasha 48504: end the player's event early
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

    // High Warlord Cromush 48545 (groups from 20; 10 is his Stormpike Apocalypse yell)
    TALK_CROMUSH_WHAT_HAPPENED                  = 20,       // 48797 .. 48803
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

/*######
## Heroes of the Horde! (28400)
######*/

// Orkus leads the player up the hill to the Stormpike camp and stops in the cover of the rock at its eastern edge
Position const HeroesHidingPos = { -1301.5f, 547.0f, 96.0f };

// The camp on the hilltop, around the March of the Stormpike Invisible Stalkers (48506, guids 375400 and 375401) and the
// bonfire. In 5.4.8 Summon Purgation Actors (90213) brings them with Summon Mountaineer 90258, Summon Bloodfang 90209,
// Summon Vanndar 90210 and Summon Balinda 90211, all at spell_target_position spots no sniff has; the spots are ours.
struct PurgationActorSpawn
{
    uint32 Entry;
    Position Pos;
};

PurgationActorSpawn const HeroesActorSpawns[] =
{
    { NPC_STORMPIKE_BATTLE_MASTER,      { -1319.33f, 528.26f, 99.42f, 0.30f } },
    { NPC_VANNDAR_STORMPIKE,            { -1340.0f,  523.0f,  97.0f,  0.20f } },
    { NPC_CAPTAIN_BALINDA_STONEHEARTH,  { -1338.0f,  515.0f,  96.5f,  0.50f } },
    { NPC_PACKLEADER_IVAR_BLOODFANG,    { -1343.0f,  530.0f,  97.5f,  6.00f } }
};

// Static Kingslayer Orkus 48470 on the Purgation Isle beach (guid 3110070): ends The Road to Purgation and They Will Never
// Expect This..., starts Heroes of the Horde!. "I'm ready, Orkus. Let's do this!" (12440) starts the event with the player's
// own Orkus (48503; Summon Orkus 90205 in 5.4.8, from Purgation Isle Summon Controller II 90206). He takes this one's place:
// the player loses the detection of the beach pair (Kasha flies off to wait for his call), and gets it back if the event ends
// early. Talking to him again starts the event anew, ending what is left of an earlier one of the player's (cut short by a
// logout, by leaving the isle or by getting off Kasha) first, so that the player can never be locked out; a player who already
// holds the plans is picked up at once.
struct npc_hillsbrad_orkus_purgation_isle : public ScriptedAI
{
    npc_hillsbrad_orkus_purgation_isle(Creature* creature) : ScriptedAI(creature) { }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_ORKUS_HEROES || gossipListId != GOSSIP_OPTION_ORKUS_READY)
            return false;

        CloseGossipMenuFor(player);

        // The Orkus who rides Kasha in The Road to Purgation is this entry too
        if (me->IsSummon())
            return true;

        if (player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE) != QUEST_STATUS_INCOMPLETE || !player->IsAlive() || player->GetVehicle())
            return true;

        // What is left of an earlier event of the player's goes first: its Orkus with his actors, a Kasha still on her way
        for (uint32 entry : { NPC_KASHA_HEROES, NPC_ORKUS_HEROES })
        {
            std::list<Creature*> creatures;
            player->GetCreatureListWithEntryInGrid(creatures, entry, 300.0f);
            for (Creature* creature : creatures)
                if (creature->GetPrivateObjectOwner() == player->GetGUID() && creature->IsAIEnabled)
                    creature->AI()->DoAction(ACTION_HEROES_ABORT);
        }
        player->RemoveAurasDueToSpell(SPELL_SUMMON_KASHA_HEROES);

        if (me->SummonCreature(NPC_ORKUS_HEROES, me->GetPosition(), TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, player->GetGUID()))
            player->RemoveAurasDueToSpell(SPELL_DETECT_QUEST_INVIS_ZONE_4);
        return true;
    }
};

// 48503 - Kingslayer Orkus
// Leads the player up the hill, tells them his plan, kills the Stormpike Battle Master and holds the trio while the player loots
// the Alliance Battle Plans from the body; then he calls Kasha (Summon Kasha 90278, cast by the player), again every few seconds
// until she has the player (she gives up on a player she cannot pick up: dead, casting or out of reach). The plans' item spell
// (Send Event 27246) has an empty event in 5.4.8: he watches for the plans instead. If the Battle Master's body is gone before
// the player took the plans, the event ends (the player starts it again at the beach). He cannot fall in the fight; Kasha takes
// him off the hill and he dies on the way to Tarren Mill.
struct npc_hillsbrad_orkus_heroes : public ScriptedAI
{
    npc_hillsbrad_orkus_heroes(Creature* creature) : ScriptedAI(creature), _waitTries(0), _atHiding(false), _fighting(false), _carried(false),
        _kashaCalled(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _playerGUID = me->GetPrivateObjectOwner();

        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        me->SetLevel(80);
        me->SetMaxHealth(600000);
        me->SetFullHealth();
        me->SetFaction(FACTION_ORGRIMMAR);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (player && player->HasItemCount(ITEM_ALLIANCE_BATTLE_PLANS))
            _events.ScheduleEvent(EVENT_HEROES_CALL_KASHA, 1s);
        else
            _events.ScheduleEvent(EVENT_HEROES_WALK, 1s);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_HEROES_ABORT)
        {
            Abort(ObjectAccessor::GetPlayer(*me, _playerGUID));
            return;
        }

        if (action != ACTION_ORKUS_CARRIED)
            return;

        // Kasha has him: the Stormpike are left behind
        _carried = true;
        _fighting = false;
        _events.Reset();
        me->CombatStop(true);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        DespawnActors(4000);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (damage >= me->GetHealth())
            damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
    }

    void EnterEvadeMode(EvadeReason /*why*/) override { }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type == POINT_MOTION_TYPE && pointId == POINT_HEROES_HIDING)
            ReachHiding();
    }

    void UpdateAI(uint32 diff) override
    {
        if (_carried)
            return;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            Abort(nullptr);
            return;
        }

        QuestStatus status = player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE);
        if ((status != QUEST_STATUS_INCOMPLETE && status != QUEST_STATUS_COMPLETE) || me->GetDistance(player) > 250.0f)
        {
            Abort(player);
            return;
        }

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_HEROES_WALK:
                    SummonActors();
                    me->GetMotionMaster()->MovePoint(POINT_HEROES_HIDING, HeroesHidingPos);
                    _events.ScheduleEvent(EVENT_HEROES_WALK_TIMEOUT, 60s);
                    break;
                case EVENT_HEROES_WALK_TIMEOUT:
                    // No way up was found in time: he is there all the same
                    if (!_atHiding)
                    {
                        me->NearTeleportTo(HeroesHidingPos.GetPositionX(), HeroesHidingPos.GetPositionY(), HeroesHidingPos.GetPositionZ(), me->GetOrientation());
                        ReachHiding();
                    }
                    break;
                case EVENT_HEROES_WAIT_FOR_PLAYER:
                    // He waits up to half a minute for the player to catch up
                    if (me->GetDistance(player) < 20.0f || ++_waitTries > 60)
                    {
                        if (Creature* master = GetActor(NPC_STORMPIKE_BATTLE_MASTER))
                            me->SetFacingToObject(master);
                        Talk(TALK_ORKUS_GET_DOWN, player);
                        me->SetStandState(UNIT_STAND_STATE_KNEEL);
                        _events.ScheduleEvent(EVENT_HEROES_TALK, 4s);
                    }
                    else
                        _events.ScheduleEvent(EVENT_HEROES_WAIT_FOR_PLAYER, 500ms);
                    break;
                case EVENT_HEROES_TALK:
                    Talk(TALK_ORKUS_MAP_TABLE, player);
                    next = 6s;
                    break;
                case EVENT_HEROES_TALK + 1:
                    Talk(TALK_ORKUS_CAREFUL, player);
                    next = 6s;
                    break;
                case EVENT_HEROES_TALK + 2:
                    me->SetStandState(UNIT_STAND_STATE_STAND);
                    me->SetFacingToObject(player);
                    Talk(TALK_ORKUS_PLAN_OF_MY_OWN, player);
                    next = 4s;
                    break;
                case EVENT_HEROES_TALK + 3:
                    Talk(TALK_ORKUS_KILL_THE_GUARD, player);
                    next = 6s;
                    break;
                case EVENT_HEROES_TALK + 4:
                    Talk(TALK_ORKUS_GET_THE_PLANS, player);
                    next = 8s;
                    break;
                case EVENT_HEROES_TALK + 5:
                    Talk(TALK_ORKUS_DO_NOT_WORRY, player);
                    _events.ScheduleEvent(EVENT_HEROES_CHARGE, 5s);
                    break;
                case EVENT_HEROES_CHARGE:
                {
                    Talk(TALK_ORKUS_LOKTAR, player);
                    Creature* master = GetActor(NPC_STORMPIKE_BATTLE_MASTER);
                    if (!master)
                    {
                        // The Battle Master is gone (killed by the player): straight on to holding the trio
                        _events.ScheduleEvent(EVENT_HEROES_KILL_BATTLE_MASTER, 1s);
                        break;
                    }
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    me->SetReactState(REACT_AGGRESSIVE);
                    _fighting = true;
                    AttackStart(master);
                    master->SetReactState(REACT_AGGRESSIVE);
                    master->AI()->AttackStart(me);
                    _events.ScheduleEvent(EVENT_HEROES_KILL_BATTLE_MASTER, 6s);
                    break;
                }
                case EVENT_HEROES_KILL_BATTLE_MASTER:
                {
                    if (Creature* master = GetActor(NPC_STORMPIKE_BATTLE_MASTER))
                    {
                        // The player loots him: the plans are a quest drop of his (creature_loot_template 48515)
                        master->SetLootRecipient(player);
                        master->LowerPlayerDamageReq(master->GetMaxHealth());
                        me->Kill(master);
                    }
                    Talk(TALK_ORKUS_PLANS_AND_GO, player);

                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
                    me->SetReactState(REACT_AGGRESSIVE);
                    _fighting = true;
                    for (uint32 entry : { NPC_VANNDAR_STORMPIKE, NPC_CAPTAIN_BALINDA_STONEHEARTH, NPC_PACKLEADER_IVAR_BLOODFANG })
                    {
                        if (Creature* actor = GetActor(entry))
                        {
                            actor->SetReactState(REACT_AGGRESSIVE);
                            actor->AI()->AttackStart(me);
                        }
                    }
                    if (Creature* vanndar = GetActor(NPC_VANNDAR_STORMPIKE))
                    {
                        vanndar->AI()->Talk(TALK_VANNDAR_LAST_LOKTAR, player);
                        AttackStart(vanndar);
                    }
                    _events.ScheduleEvent(EVENT_HEROES_CHECK_PLANS, 1s);
                    break;
                }
                case EVENT_HEROES_CHECK_PLANS:
                    if (player->HasItemCount(ITEM_ALLIANCE_BATTLE_PLANS))
                        _events.ScheduleEvent(EVENT_HEROES_CALL_KASHA, 1s);
                    else if (!BattleMasterBodyLeft())
                    {
                        // The body decayed (or never came) before the player took the plans: nothing left to wait for
                        Abort(player);
                        return;
                    }
                    else
                        _events.ScheduleEvent(EVENT_HEROES_CHECK_PLANS, 1s);
                    break;
                case EVENT_HEROES_CALL_KASHA:
                    // The plans were lost again: back to waiting for them
                    if (!player->HasItemCount(ITEM_ALLIANCE_BATTLE_PLANS))
                    {
                        _events.ScheduleEvent(EVENT_HEROES_CHECK_PLANS, 1s);
                        break;
                    }
                    // Called again until a Kasha has the player: one that gave up on her pickup is replaced
                    if (player->IsAlive() && !player->GetVehicle() && !FindPrivateCreatureOf(player, NPC_KASHA_HEROES, 300.0f))
                    {
                        if (!_kashaCalled)
                            Talk(TALK_ORKUS_KASHA_GET_OUT, player);
                        _kashaCalled = true;
                        player->RemoveAurasDueToSpell(SPELL_SUMMON_KASHA_HEROES);
                        player->CastSpell(player, SPELL_SUMMON_KASHA_HEROES, true);
                    }
                    _events.ScheduleEvent(EVENT_HEROES_CALL_KASHA, 5s);
                    break;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }

        if (_fighting && UpdateVictim())
            DoMeleeAttackIfReady();
    }

private:
    void ReachHiding()
    {
        if (_atHiding)
            return;

        _atHiding = true;
        _events.CancelEvent(EVENT_HEROES_WALK_TIMEOUT);
        _events.ScheduleEvent(EVENT_HEROES_WAIT_FOR_PLAYER, 500ms);
    }

    void SummonActors()
    {
        for (PurgationActorSpawn const& spawn : HeroesActorSpawns)
        {
            Position pos = spawn.Pos;
            pos.m_positionZ = GroundZ(me, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ());
            if (TempSummon* actor = me->SummonCreature(spawn.Entry, pos, TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, _playerGUID))
            {
                _actorGUIDs.push_back(actor->GetGUID());
                if (spawn.Entry == NPC_STORMPIKE_BATTLE_MASTER)
                    _battleMasterGUID = actor->GetGUID();
            }
        }
    }

    // The Battle Master, alive or as a body that can still be looted (his corpse decays and is unsummoned after a while)
    bool BattleMasterBodyLeft() const
    {
        Creature* master = ObjectAccessor::GetCreature(*me, _battleMasterGUID);
        return master && master->IsInWorld();
    }

    Creature* GetActor(uint32 entry)
    {
        for (ObjectGuid const& guid : _actorGUIDs)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                if (actor->GetEntry() == entry && actor->IsAlive())
                    return actor;
        return nullptr;
    }

    void DespawnActors(uint32 delay)
    {
        for (ObjectGuid const& guid : _actorGUIDs)
            if (Creature* actor = ObjectAccessor::GetCreature(*me, guid))
                actor->DespawnOrUnsummon(delay);
        _actorGUIDs.clear();
    }

    // The event ended early: the actors go, and a player still here sees the beach Orkus and Kasha again
    void Abort(Player* player)
    {
        _events.Reset();
        _carried = true;    // nothing more to do until he is gone
        DespawnActors(0);
        if (player && player->IsInWorld())
        {
            // Summon Kasha's hidden dummy aura (effect 1, no duration) would otherwise stay on the player, saved
            player->RemoveAurasDueToSpell(SPELL_SUMMON_KASHA_HEROES);
            player->UpdateAreaDependentAuras(player->GetAreaId());
        }
        me->DespawnOrUnsummon();
    }

    EventMap _events;
    ObjectGuid _playerGUID;
    ObjectGuid _battleMasterGUID;
    GuidVector _actorGUIDs;
    uint32 _waitTries;
    bool _atHiding;
    bool _fighting;
    bool _carried;
    bool _kashaCalled;
};

// 48515 - Stormpike Battle Master, 48508 - Vanndar Stormpike, 48507 - Captain Balinda Stonehearth, 48485 - Packleader Ivar
// Bloodfang: Orkus' event actors on the hilltop. They wait until Orkus strikes and fight only him (the player is sneaking
// past them). The Battle Master falls to Orkus; the trio cannot be killed, they are the reason Orkus does not come back.
struct npc_hillsbrad_purgation_actor : public ScriptedAI
{
    npc_hillsbrad_purgation_actor(Creature* creature) : ScriptedAI(creature) { }

    void Reset() override
    {
        me->SetReactState(REACT_PASSIVE);
    }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_NPC);
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (me->GetEntry() != NPC_STORMPIKE_BATTLE_MASTER && damage >= me->GetHealth())
            damage = me->GetHealth() > 1 ? me->GetHealth() - 1 : 0;
    }

    void JustDied(Unit* /*killer*/) override
    {
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
    }

    void UpdateAI(uint32 /*diff*/) override
    {
        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }
};

// From the hilltop Kasha climbs to 130 yards and crosses the foothills straight to Tarren Mill, 24 yards or more over the
// terrain and the trees (the most at the start, over the rock of the camp), and comes down in front of High Warlord Cromush.
Position const HeroesFlightPath[] =
{
    { -1000.0f,  180.0f, 130.0f },
    {  -500.0f, -380.0f, 130.0f },
    {  -150.0f, -770.0f, 120.0f },
    {   -60.0f, -880.0f, 100.0f },
    {   -25.0f, -905.0f,  75.0f },
    {   -18.0f, -913.0f,  62.0f }
};

Position const HeroesTarrenMillLandingPos = { -18.0f, -913.0f, 54.5f, 4.71f };
Position const HeroesOrkusBodyPos = { -17.5f, -918.5f, 54.4f, 1.6f };
Position const HeroesPlayerExitPos = { -21.5f, -915.0f, 54.5f, 4.71f };
// Where the perched Kasha 48576 (guid 375403) sits on the roof above Cromush; she takes over there at the end
Position const HeroesKashaPerchPos = { -21.1076f, -933.026f, 71.5233f, 0.380397f };

float const HeroesFlightSpeedRate = 3.0f;

// 48504 - Kasha
// Called by Orkus once the player holds the plans (Summon Kasha 90278, personal). She comes down beside the player and takes
// them up (seat 1), snatches Orkus out of the fight (seat 0) and flies to Tarren Mill while he dies; there she sets the player
// and his body down before Cromush, and the player is credited. The credit is given directly: Quest Complete 28400 (90379)
// is aimed at her master, which a summon with no owner does not have here. Cromush's lines follow; she settles on the roof
// where the perched Kasha takes over when the player is given its detection.
struct npc_hillsbrad_kasha_heroes : public VehicleAI
{
    npc_hillsbrad_kasha_heroes(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _arrived(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->SetSpeed(MOVE_RUN, HeroesFlightSpeedRate);
        me->SetSpeed(MOVE_FLIGHT, HeroesFlightSpeedRate);

        _events.ScheduleEvent(EVENT_KASHA_APPROACH, 500ms);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_HEROES_ABORT)
            Leave();
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        if (_started && !_arrived)
            Leave();
    }

    void GetPassengerExitPosition(Unit* passenger, int8 /*seatId*/, Position& pos) override
    {
        if (!_arrived)
            return;

        if (passenger->GetGUID() == _playerGUID)
            pos.Relocate(HeroesPlayerExitPos);
        else if (passenger->GetGUID() == _orkusGUID)
            pos.Relocate(HeroesOrkusBodyPos);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != POINT_MOTION_TYPE && type != EFFECT_MOTION_TYPE)
            return;

        switch (pointId)
        {
            case POINT_KASHA_ABOVE_PLAYER:
                _events.ScheduleEvent(EVENT_KASHA_LAND_BY_PLAYER, 100ms);
                break;
            case POINT_KASHA_BY_PLAYER:
                _events.ScheduleEvent(EVENT_KASHA_PICK_UP_PLAYER, 300ms);
                break;
            case POINT_KASHA_ABOVE_ORKUS:
                _events.ScheduleEvent(EVENT_KASHA_GRAB_ORKUS, 300ms);
                break;
            case POINT_KASHA_FLIGHT:
                _events.ScheduleEvent(EVENT_KASHA_LAND, 100ms);
                break;
            case POINT_KASHA_TARREN_MILL:
                _events.ScheduleEvent(EVENT_KASHA_ARRIVE, 500ms);
                break;
            case POINT_KASHA_PERCH_APPROACH:
                _events.ScheduleEvent(EVENT_KASHA_PERCH_LAND, 100ms);
                break;
            case POINT_KASHA_PERCH:
                me->SetFacingTo(HeroesKashaPerchPos.GetOrientation());
                break;
            default:
                break;
        }
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
            Milliseconds next = 0ms;

            switch (eventId)
            {
                case EVENT_KASHA_APPROACH:
                    me->GetMotionMaster()->MovePoint(POINT_KASHA_ABOVE_PLAYER, player->GetPositionX(), player->GetPositionY(), player->GetPositionZ() + 12.0f, false);
                    break;
                case EVENT_KASHA_LAND_BY_PLAYER:
                {
                    Position landing;
                    player->GetNearPoint2D(nullptr, landing.m_positionX, landing.m_positionY, 3.0f, player->GetOrientation() + float(M_PI) / 2.0f);
                    landing.m_positionZ = GroundZ(me, landing.GetPositionX(), landing.GetPositionY(), player->GetPositionZ());
                    landing.SetOrientation(player->GetOrientation());
                    me->GetMotionMaster()->MoveLand(POINT_KASHA_BY_PLAYER, landing);
                    break;
                }
                case EVENT_KASHA_PICK_UP_PLAYER:
                    if (player->IsAlive() && !player->GetVehicle())
                        player->EnterVehicle(me, SEAT_PLAYER);
                    _events.ScheduleEvent(EVENT_KASHA_WAIT_FOR_RIDER, 1s);
                    break;
                case EVENT_KASHA_WAIT_FOR_RIDER:
                    if (player->GetVehicleBase() == me)
                    {
                        _started = true;
                        _events.ScheduleEvent(EVENT_KASHA_FETCH_ORKUS, 1s);
                    }
                    else if (++_boardTries < 30)
                        _events.ScheduleEvent(EVENT_KASHA_PICK_UP_PLAYER, 1s);
                    else
                    {
                        Leave();
                        return;
                    }
                    break;
                case EVENT_KASHA_FETCH_ORKUS:
                    if (Creature* orkus = FindPrivateCreatureOf(player, NPC_ORKUS_HEROES, 150.0f))
                    {
                        _orkusGUID = orkus->GetGUID();
                        me->GetMotionMaster()->MovePoint(POINT_KASHA_ABOVE_ORKUS, orkus->GetPositionX(), orkus->GetPositionY(), orkus->GetPositionZ() + 5.0f, false);
                    }
                    else
                        _events.ScheduleEvent(EVENT_KASHA_TAKE_OFF, 1s);
                    break;
                case EVENT_KASHA_GRAB_ORKUS:
                    if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                    {
                        orkus->AI()->Talk(TALK_ORKUS_FORGET_ABOUT_ME, player);
                        orkus->AI()->DoAction(ACTION_ORKUS_CARRIED);
                        orkus->EnterVehicle(me, SEAT_ORKUS);
                    }
                    _events.ScheduleEvent(EVENT_KASHA_TAKE_OFF, 3s);
                    break;
                case EVENT_KASHA_TAKE_OFF:
                    TakeOff();
                    if (ObjectAccessor::GetCreature(*me, _orkusGUID))
                        _events.ScheduleEvent(EVENT_KASHA_ORKUS_TALK, 6s);
                    break;
                case EVENT_KASHA_ORKUS_TALK:
                    OrkusTalk(player, TALK_ORKUS_CAME_BACK);
                    next = 8s;
                    break;
                case EVENT_KASHA_ORKUS_TALK + 1:
                    OrkusTalk(player, TALK_ORKUS_ASK_OF_YOU);
                    next = 10s;
                    break;
                case EVENT_KASHA_ORKUS_TALK + 2:
                    OrkusTalk(player, TALK_ORKUS_TAKE_CARE_OF_KASHA);
                    next = 9s;
                    break;
                case EVENT_KASHA_ORKUS_TALK + 3:
                    OrkusTalk(player, TALK_ORKUS_BEST_FRIEND);
                    next = 8s;
                    break;
                case EVENT_KASHA_ORKUS_TALK + 4:
                    OrkusTalk(player, TALK_ORKUS_SOMEWHERE_SAFE);
                    _events.ScheduleEvent(EVENT_KASHA_ORKUS_DIES, 4s);
                    break;
                case EVENT_KASHA_ORKUS_DIES:
                    if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                        SetLifeless(orkus);
                    break;
                case EVENT_KASHA_LAND:
                {
                    Position landing = HeroesTarrenMillLandingPos;
                    landing.m_positionZ = GroundZ(me, landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ());
                    me->GetMotionMaster()->MoveLand(POINT_KASHA_TARREN_MILL, landing);
                    break;
                }
                case EVENT_KASHA_ARRIVE:
                    _arrived = true;
                    _events.CancelEvent(EVENT_KASHA_ORKUS_DIES);
                    if (player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE) == QUEST_STATUS_INCOMPLETE)
                        player->AreaExploredOrEventHappens(QUEST_HEROES_OF_THE_HORDE);
                    if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                    {
                        if (orkus->GetVehicleBase() == me)
                            orkus->ExitVehicle();
                        SetLifeless(orkus);
                    }
                    if (player->GetVehicleBase() == me)
                        player->ExitVehicle();
                    _events.ScheduleEvent(EVENT_KASHA_CROMUSH_TALK, 2s);
                    _events.ScheduleEvent(EVENT_KASHA_PERCH, 5s);
                    break;
                case EVENT_KASHA_PERCH:
                {
                    // Up over the eaves, then down onto the roof
                    Position const path[] =
                    {
                        me->GetPosition(),
                        { me->GetPositionX(), me->GetPositionY(), HeroesKashaPerchPos.GetPositionZ() + 8.0f },
                        { HeroesKashaPerchPos.GetPositionX(), HeroesKashaPerchPos.GetPositionY(), HeroesKashaPerchPos.GetPositionZ() + 8.0f }
                    };
                    me->SetSpeed(MOVE_RUN, 1.0f);
                    me->SetSpeed(MOVE_FLIGHT, 1.0f);
                    me->GetMotionMaster()->MoveSmoothPath(POINT_KASHA_PERCH_APPROACH, path, 3, false, true);
                    break;
                }
                case EVENT_KASHA_PERCH_LAND:
                    me->GetMotionMaster()->MoveLand(POINT_KASHA_PERCH, HeroesKashaPerchPos);
                    break;
                case EVENT_KASHA_CROMUSH_TALK:
                case EVENT_KASHA_CROMUSH_TALK + 1:
                case EVENT_KASHA_CROMUSH_TALK + 2:
                case EVENT_KASHA_CROMUSH_TALK + 3:
                case EVENT_KASHA_CROMUSH_TALK + 4:
                case EVENT_KASHA_CROMUSH_TALK + 5:
                case EVENT_KASHA_CROMUSH_TALK + 6:
                {
                    static Milliseconds const pauses[] = { 4s, 4s, 4s, 4s, 7s, 8s, 8s };
                    uint32 line = eventId - EVENT_KASHA_CROMUSH_TALK;
                    if (Creature* cromush = me->FindNearestCreature(NPC_HIGH_WARLORD_CROMUSH, 40.0f))
                        cromush->AI()->Talk(TALK_CROMUSH_WHAT_HAPPENED + line, player);
                    if (line == 6)
                        _events.ScheduleEvent(EVENT_KASHA_FINISH, pauses[line]);
                    else
                        next = pauses[line];
                    break;
                }
                case EVENT_KASHA_FINISH:
                    Finish(player);
                    return;
                default:
                    break;
            }

            if (next > 0ms)
                _events.ScheduleEvent(eventId + 1, next);
        }
    }

private:
    void TakeOff()
    {
        std::vector<Position> path;
        path.push_back(me->GetPosition());
        path.push_back({ me->GetPositionX(), me->GetPositionY(), std::max(me->GetPositionZ() + 10.0f, 135.0f) });
        for (Position const& node : HeroesFlightPath)
            path.push_back(node);

        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MoveSmoothPath(POINT_KASHA_FLIGHT, path.data(), path.size(), false, true);
    }

    void OrkusTalk(Player* player, uint8 textId)
    {
        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
            orkus->AI()->Talk(textId, player);
    }

    // Orkus is dead: lying still, as Cromush finds him
    static void SetLifeless(Creature* orkus)
    {
        orkus->SetFlag(UNIT_FIELD_FLAGS_2, UNIT_FLAG2_FEIGN_DEATH);
        orkus->SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, UNIT_DYNFLAG_DEAD);
        orkus->SetStandState(UNIT_STAND_STATE_DEAD);
    }

    // The scene is over: the perched Kasha takes over on the roof, and Orkus' body goes with Cromush's last words
    void Finish(Player* player)
    {
        _events.Reset();
        if (!player)
            player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (player && player->IsInWorld())
        {
            // Summon Kasha's hidden dummy aura (effect 1, no duration) would otherwise stay on the player, saved
            player->RemoveAurasDueToSpell(SPELL_SUMMON_KASHA_HEROES);
            player->UpdateAreaDependentAuras(player->GetAreaId());
        }
        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
            orkus->DespawnOrUnsummon();
        me->DespawnOrUnsummon();
    }

    // Cut short. Before the player was on board (she could not pick them up): only she goes, and Orkus calls another Kasha.
    // Once the player had been on board (they logged out or got off on the way): the player's Orkus ends the event (his actors
    // go, the beach Orkus and Kasha show again); the player keeps the plans and is picked up again by talking to Orkus on the
    // Purgation Isle beach.
    void Leave()
    {
        if (_arrived)
        {
            Finish(nullptr);
            return;
        }

        bool wasStarted = _started;
        _events.Reset();
        _started = false;
        _arrived = true;

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (player && player->GetVehicleBase() == me)
            player->ExitVehicle();

        if (wasStarted)
        {
            Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID);
            if (!orkus && player && player->IsInWorld())
                orkus = FindPrivateCreatureOf(player, NPC_ORKUS_HEROES, 300.0f);
            if (orkus && orkus->IsAIEnabled)
                orkus->AI()->DoAction(ACTION_HEROES_ABORT);
            else if (orkus)
                orkus->DespawnOrUnsummon();
        }

        if (player && player->IsInWorld())
        {
            player->RemoveAurasDueToSpell(SPELL_SUMMON_KASHA_HEROES);
            if (wasStarted)
                player->UpdateAreaDependentAuras(player->GetAreaId());
        }

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
    new creature_script<npc_hillsbrad_orkus_purgation_isle>("npc_hillsbrad_orkus_purgation_isle");
    new creature_script<npc_hillsbrad_orkus_heroes>("npc_hillsbrad_orkus_heroes");
    new creature_script<npc_hillsbrad_purgation_actor>("npc_hillsbrad_purgation_actor");
    new creature_script<npc_hillsbrad_kasha_heroes>("npc_hillsbrad_kasha_heroes");
}
