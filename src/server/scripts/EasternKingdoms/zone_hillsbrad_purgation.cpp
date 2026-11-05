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
//  - They Will Never Expect This... (28397): on the isle Orkus goes with the player as a guardian, fights beside them and chats.
//  - Heroes of the Horde! (28400): the event Orkus comes, gives the quest, leads the player up the hill, kills the Stormpike Battle Master and holds off Vanndar, Balinda and Ivar Bloodfang
//    while the player takes the Alliance Battle Plans; Kasha carries the player and the dying Orkus to Tarren Mill.
// Sourced: the spells and their summon properties and vehicle seats (5.4.8 DBC), every line (broadcast_text), the course of
// the events (warcraft.wiki.gg). Ours: the flight paths, heights, speeds, the actors' spots and the timings; nothing was sniffed.
//
// Hillsbrad Foothills: the Ruins of Southshore.
//  - Studies in Lethality (28324): the Blight Slimes' anger meter (its sources are with its scripts below).

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
#include "MoveSpline.h"

enum PurgationIsle
{
    QUEST_THE_ROAD_TO_PURGATION                 = 28375,
    QUEST_THEY_WILL_NEVER_EXPECT_THIS           = 28397,
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

    SPELL_DETECT_QUEST_INVIS_ZONE_4             = 90115,    // shows the landed Kasha on Purgation Isle (spell_area)
    SPELL_RIDE_TO_PURGATION                     = 90122,
    SPELL_SUMMON_KASHA_ROAD_TO_PURGATION        = 90123,
    SPELL_SUMMON_KASHA_HEROES                   = 90278,
    SPELL_PURGATION_ISLE_SUMMON_CONTROLLER_I    = 90130,
    SPELL_PURGATION_ISLE_SUMMON_CONTROLLER_II   = 90206,
    SPELL_SUMMON_ORKUS_PURGATION_ISLE           = 90132,
    SPELL_COMMANDING_SHOUT                      = 82061,    // +2500 health to allies in 20 yards, 2 min

    // Kasha 48467 (Vehicle 1329) and 48504 (Vehicle 1332): seat 0 can control the vehicle (a player there would take her
    // over and her script would stop), seat 1 is a plain passenger seat. Orkus takes seat 0, the player seat 1.
    SEAT_ORKUS                                  = 0,
    SEAT_PLAYER                                 = 1,

    GOSSIP_MENU_ORKUS_HEROES                    = 12440,
    GOSSIP_OPTION_ORKUS_READY                   = 0,

    FACTION_ORGRIMMAR                           = 29,       // Orkus' faction at Southshore (48218), so the Stormpike fight him

    ACTION_ORKUS_CARRIED                        = 1,
    ACTION_HEROES_ABORT                         = 2,        // Orkus 48503 / Kasha 48504: end the player's event early
    ACTION_HEROES_DISMISS                       = 3,        // Orkus 48503: Controller II is gone (the player left the zone)
    ACTION_HEROES_DISCARD                       = 4,        // Orkus 48503: a stale second one of the player's, left behind
    ACTION_KASHA_STAND_DOWN                     = 5,        // Kasha 48467 / 48504: the player's watchdog sets the player down

    AREA_PURGATION_ISLE                         = 896,
};

enum PurgationTexts
{
    // Kingslayer Orkus 48470 (riding Kasha in The Road to Purgation)
    TALK_ORKUS_TO_PURGATION_ISLE                = 0,        // 48671
    TALK_ORKUS_FLEW_OFF                         = 1,        // 48675 (boss whisper)
    // Kingslayer Orkus 48470 (the player's companion on Purgation Isle)
    TALK_ORKUS_CHATTER                          = 2,        // 48698, 48702, 48705, 48706, 48708 .. 48719

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
    TALK_ORKUS_TELL_ME_WHEN_READY               = 16,       // 48757

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

    // Orkus, the player's companion on Purgation Isle
    EVENT_COMPANION_SHOUT                       = 5,

    // Kasha, The Road to Purgation: the flight did not end in time
    EVENT_ROAD_OVERDUE                          = 6,

    // Orkus, Heroes of the Horde!
    EVENT_HEROES_WALK                           = 10,
    EVENT_HEROES_WALK_TIMEOUT,
    EVENT_HEROES_WAIT_FOR_PLAYER,
    EVENT_HEROES_TALK,                          // .. EVENT_HEROES_TALK + 6
    EVENT_HEROES_CHARGE                         = 20,
    EVENT_HEROES_KILL_BATTLE_MASTER,
    EVENT_HEROES_CHECK_PLANS,
    EVENT_HEROES_CALL_KASHA,
    EVENT_HEROES_FOLLOW                         = 24,

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
    EVENT_KASHA_OVERDUE                         = 61,       // the flight to Tarren Mill did not end in time
    EVENT_KASHA_GOT_OFF                         = 62,       // the player got off on the way: set down, or gone elsewhere
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
    POINT_ROAD_RISE,
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

// What a Kasha whose flight did not end was doing: whether her AI still runs (Creature::Update runs it only when enabled and
// not evading), her motion and her spline
static void LogKashaOverdue(char const* script, Creature* kasha, Player* player)
{
    TC_LOG_ERROR("scripts", "%s: the flight of %s did not land, setting them down (motion %u, spline finalized %u, evading %u, AI enabled %u)",
        script, player->GetName().c_str(), uint32(kasha->GetMotionMaster()->GetCurrentMovementGeneratorType()),
        uint32(kasha->movespline->Finalized()), uint32(kasha->IsInEvadeMode()), uint32(kasha->IsAIEnabled));
}

// Sets a player down at the end of a Kasha flight, in the scene cameras' order (npc_hillsbrad_scene_camera::Finish). A player
// still seated gets off and is then held in the air (gravity off, no root: a root, like a teleport in the same tick, stops the
// exit's jump with a stop spline and leaves the client in the seat's view). A player who already got off (from the vehicle's
// PassengerBoarded, which runs inside Vehicle::RemovePassenger, before _ExitVehicle sends the unroot and the exit's jump) is
// held from their next update. 300 ms later the player is teleported to `dest` and credited, unless they died or were
// teleported away meanwhile (Player::TeleportTo gets them off before it marks the teleport, so only the place they are in by
// then tells), and the area auras are checked again (spell_area does not follow quest changes in this core); gravity comes
// back 100 ms after the teleport, which a teleport cast in the player's own update only sends at the end of it. Without
// `areaAuras` the caller checks them itself, later.
static void SetDownFromKasha(Player* player, Unit* kasha, Position const& dest, uint32 questId, bool areaAuras = true)
{
    if (kasha && player->GetVehicleBase() == kasha)
    {
        player->ExitVehicle();
        player->SetDisableGravity(true);
    }
    else
        player->m_Events.AddLambdaEventAtOffset([player]()
        {
            if (player->IsInWorld() && !player->GetVehicle())
                player->SetDisableGravity(true);
        }, 1);

    WorldLocation const heldAt = player->GetWorldLocation();
    player->m_Events.AddLambdaEventAtOffset([player, heldAt, dest, questId, areaAuras]()
    {
        if (!player->IsInWorld())
            return;

        if (!player->GetVehicle() && !player->IsBeingTeleported() && player->IsAlive() && player->GetMapId() == heldAt.GetMapId()
            && player->GetExactDist(&heldAt) < 30.0f)
        {
            if (player->GetExactDist(&dest) > 2.0f)
                player->NearTeleportTo(dest.GetPositionX(), dest.GetPositionY(), dest.GetPositionZ(), dest.GetOrientation());

            if (player->GetQuestStatus(questId) == QUEST_STATUS_INCOMPLETE)
                player->AreaExploredOrEventHappens(questId);
            if (areaAuras)
                player->UpdateAreaDependentAuras(player->GetAreaId());
        }

        player->m_Events.AddLambdaEventAtOffset([player]()
        {
            if (player->IsInWorld())
                player->SetDisableGravity(false);
        }, 100);
    }, 300);
}

// The player's own check that a Kasha flight ended, in case Kasha's AI no longer runs (then neither her landing nor her own
// overdue timer would): if the player still sits on that Kasha well after the flight's duration, Kasha is told to stand down
// (or sent away, without an AI) and the player is set down at `dest` and credited. `extraAura` is an aura of the flight to take
// off with it.
static void WatchKashaFlight(Player* player, Creature* kasha, Milliseconds after, Position const& dest, uint32 questId, uint32 extraAura,
    char const* script, int32 standDownAction)
{
    ObjectGuid const kashaGUID = kasha->GetGUID();
    player->m_Events.AddLambdaEventAtOffset([player, kashaGUID, dest, questId, extraAura, script, standDownAction]()
    {
        if (!player->IsInWorld())
            return;

        Unit* base = player->GetVehicleBase();
        if (!base || base->GetGUID() != kashaGUID || !base->ToCreature())
            return;

        Creature* kasha = base->ToCreature();
        LogKashaOverdue(script, kasha, player);
        if (kasha->IsAIEnabled)
            kasha->AI()->DoAction(standDownAction);
        else
        {
            // Without her AI nobody else sends away her other riders (Orkus, a manual-despawn summon in Heroes of the Horde!)
            if (Vehicle* vehicle = kasha->GetVehicleKit())
                for (auto const& seat : vehicle->Seats)
                    if (Creature* rider = ObjectAccessor::GetCreature(*kasha, seat.second.Passenger.Guid))
                        rider->DespawnOrUnsummon();
            kasha->DespawnOrUnsummon(500);
        }

        if (extraAura)
            player->RemoveAurasDueToSpell(extraAura);
        SetDownFromKasha(player, kasha, dest, questId);
    }, uint32(after.count()));
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

// The landed Kasha 48471 (guid 312259): the flight ends on her spot, and she takes over from the player's own Kasha when the
// player is credited (and the player's companion Orkus from the one who rode with them)
Position const RoadToPurgationLandingPos = { -1234.18f, 424.32f, 2.93f, 1.73f };
Position const RoadToPurgationPlayerExitPos = { -1231.5f, 420.5f, 3.3f, 1.73f };

float const RoadToPurgationFlightSpeedRate = 2.5f;

// How long past the flight's own duration Kasha waits for her landing before she sets the player down anyway
Milliseconds const RoadToPurgationOverdue = 10s;

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
// She flies them to Purgation Isle and lands where the landed Kasha stands; there the player is credited and set down, and
// the player's own Orkus (Purgation Isle Summon Controller I) takes over from the one who rode with them.
// The credit is given directly: in 5.4.8 Kasha ends the ride with Cancel Cinematic (90128: teleport the summoner to a DB spot,
// Despawn All Summons, QUEST_COMPLETE 28375), whose teleport has no spot here and which would not reach a player who is still
// her passenger. Once she has taken off, the ride ends the same way however it ends: on the landing, when the player gets off
// on the way (the seat lets them) and when the landing does not come (Finish, SetDownFromKasha). Crediting a player who got
// off on the way is ours, after Cancel Cinematic's QUEST_COMPLETE. The landing is watched twice: by her own overdue event, and
// by the player (WatchKashaFlight), since anything that stops her AI (Creature::Update runs it only when enabled and not
// evading) stops her events with it. She is an active object for the flight, so that she never reaches a grid that is not
// loaded: a creature that cannot enter one is removed (Map::MoveAllCreaturesInMoveList), which would drop the player.
struct npc_hillsbrad_kasha_road_to_purgation : public VehicleAI
{
    npc_hillsbrad_kasha_road_to_purgation(Creature* creature) : VehicleAI(creature), _boardTries(0), _started(false), _done(false) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (Player* player = summoner->ToPlayer())
            _playerGUID = player->GetGUID();

        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetCanFly(true);
        me->SetDisableGravity(true);
        me->setActive(true, ActiveFlags::Script);

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

        // Up and out over the water at once, as the player is seated: she is summoned on the beach (Summon Kasha's spot, 0.25 yd
        // over the ground), which rises behind her (6 yd higher 10 yd back, a tree 8-10 yd back), and the view behind her seat
        // started below the ground. 8 yd towards the first point of the flight, over the shallows, and 10 yd up.
        float const angle = me->GetAngle(RoadToPurgationFlightPath[0].GetPositionX(), RoadToPurgationFlightPath[0].GetPositionY());
        Position const rise[] =
        {
            me->GetPosition(),
            { me->GetPositionX() + 8.0f * std::cos(angle), me->GetPositionY() + 8.0f * std::sin(angle), me->GetPositionZ() + 10.0f }
        };
        me->SetSpeed(MOVE_RUN, 1.0f);
        me->SetSpeed(MOVE_FLIGHT, 1.0f);
        me->GetMotionMaster()->MoveSmoothPath(POINT_ROAD_RISE, rise, 2, false, true);

        _events.ScheduleEvent(EVENT_ROAD_WAIT_FOR_RIDER, 1s);
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        // The player is off (getting off also dismisses her, as a vehicle minion). Before the take-off nothing happened yet;
        // after it the player got off on the way and is set down on Purgation Isle.
        if (_flying)
            Finish();
        else
            Despawn();
    }

    void GetPassengerExitPosition(Unit* passenger, int8 /*seatId*/, Position& pos) override
    {
        if (_landed && passenger->GetGUID() == _playerGUID)
            pos.Relocate(RoadToPurgationPlayerExitPos);
    }

    // The player's watchdog takes over: she only goes
    void DoAction(int32 action) override
    {
        if (action != ACTION_KASHA_STAND_DOWN || _done)
            return;

        _done = true;
        _events.Reset();
        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
            orkus->DespawnOrUnsummon();
        me->DespawnOrUnsummon(500);
    }

    void MovementInform(uint32 type, uint32 pointId) override
    {
        if (type != EFFECT_MOTION_TYPE || _done)
            return;

        if (pointId == POINT_ROAD_FLIGHT)
            _events.ScheduleEvent(EVENT_ROAD_LAND, 100ms);
        else if (pointId == POINT_ROAD_LANDING)
            _events.ScheduleEvent(EVENT_ROAD_ARRIVE, 500ms);
    }

    void UpdateAI(uint32 diff) override
    {
        VehicleAI::UpdateAI(diff);

        if (_done)
            return;

        // Logged out: the ride aura is not saved, and the player asks Orkus at Southshore again
        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (!player || !player->IsInWorld())
        {
            Despawn();
            return;
        }

        if (_started && player->GetVehicleBase() != me)
        {
            if (_flying)
                Finish();
            else
                Despawn();
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
                        // Never seated: the ride aura's hiding and phase may still be on the player
                        player->RemoveAurasDueToSpell(SPELL_RIDE_TO_PURGATION);
                        Despawn();
                        return;
                    }
                    break;
                case EVENT_ROAD_TAKE_OFF:
                {
                    TakeOff();
                    _flying = true;
                    Milliseconds const flight = Milliseconds(me->movespline->Duration());
                    _events.ScheduleEvent(EVENT_ROAD_OVERDUE, flight + RoadToPurgationOverdue);
                    WatchKashaFlight(player, me, flight + RoadToPurgationOverdue + 5s, RoadToPurgationPlayerExitPos, QUEST_THE_ROAD_TO_PURGATION,
                        0, "npc_hillsbrad_kasha_road_to_purgation (player watchdog)", ACTION_KASHA_STAND_DOWN);
                    break;
                }
                case EVENT_ROAD_LAND:
                {
                    Position landing = RoadToPurgationLandingPos;
                    landing.m_positionZ = GroundZ(me, landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ());
                    me->GetMotionMaster()->MoveLand(POINT_ROAD_LANDING, landing);
                    break;
                }
                case EVENT_ROAD_ARRIVE:
                    _landed = true;
                    Finish();
                    return;
                case EVENT_ROAD_OVERDUE:
                    // The landing did not come (the end of the flight was lost): set the player down on the isle all the same
                    LogKashaOverdue("npc_hillsbrad_kasha_road_to_purgation", me, player);
                    Finish();
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

    // The end of the ride (Cancel Cinematic): the player gets off, is credited and stands on the Purgation Isle beach
    // (SetDownFromKasha). On the landing the player is already on that spot: the credit and the area auras (the detection of
    // the landed Kasha and Purgation Isle Summon Controller I, whose Orkus comes as the player stands on the beach) come at once,
    // so that the landed Kasha shows as this one goes. On the way, or when the landing never came, the teleport is what brings
    // the player there (and puts the client back in step if it saw the flight end elsewhere), and the credit and the auras come
    // with it. A player who logs out, dies or is teleported away meanwhile is left alone.
    void Finish()
    {
        if (_done)
            return;

        _done = true;
        _events.Reset();

        if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
            orkus->DespawnOrUnsummon();

        Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID);
        if (player && player->IsInWorld() && !player->GetSession()->PlayerLogout() && player->IsAlive())
        {
            SetDownFromKasha(player, me, RoadToPurgationPlayerExitPos, QUEST_THE_ROAD_TO_PURGATION);
            if (_landed)
            {
                if (player->GetQuestStatus(QUEST_THE_ROAD_TO_PURGATION) == QUEST_STATUS_INCOMPLETE)
                    player->AreaExploredOrEventHappens(QUEST_THE_ROAD_TO_PURGATION);
                player->UpdateAreaDependentAuras(player->GetAreaId());
            }
        }

        me->DespawnOrUnsummon(500);
    }

    // Cut short before the take-off or by the player's logout: nothing to set down, the player asks Orkus again
    void Despawn()
    {
        if (_done)
            return;

        _done = true;
        _events.Reset();

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
    bool _started;                              // the player is seated
    bool _flying = false;                       // she has taken off
    bool _landed = false;
    bool _done;
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

// Kingslayer Orkus 48470: the player's companion on Purgation Isle, from The Road to Purgation until They Will Never Expect
// This... is turned in (and, as a plain summon of Kasha's, the Orkus who rides her there). Purgation Isle Summon Controller I
// (90130) brings him with Summon Orkus (90132), which makes him the player's guardian (SummonProperties 3045: ally, defensive,
// without the personal-spawn flag, so other players see him too). He ends The Road to Purgation and starts and ends They Will
// Never Expect This...; at that turn-in he runs off and the event Orkus 48503 takes over. Wowhead comments: "he acts like a pet
// during this section", "Orkus gives you a buff of 2500 health" (Commanding Shout 82061, the +2500 health shout of John J.
// Keeshan, another quest companion; when he casts it is ours: whenever the player is without it), "He hits a bit low for a
// level 80" (he has the player's level here).
struct npc_hillsbrad_orkus_purgation_isle : public ScriptedAI
{
    npc_hillsbrad_orkus_purgation_isle(Creature* creature) : ScriptedAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        // The Orkus who rides Kasha is summoned by her and does nothing of his own
        Player* player = summoner->ToPlayer();
        if (!player || !me->HasUnitTypeMask(UNIT_MASK_GUARDIAN))
            return;

        _ownerGUID = player->GetGUID();
        me->SetReactState(REACT_DEFENSIVE);
        me->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, me->GetFollowAngle());
        _events.ScheduleEvent(EVENT_COMPANION_SHOUT, 2s);
    }

    void Reset() override
    {
        if (_ownerGUID && me->GetUInt32Value(UNIT_FIELD_NPC_FLAGS))
            me->SetReactState(REACT_DEFENSIVE);
    }

    void OwnerAttackedBy(Unit* attacker) override
    {
        HelpOwner(attacker);
    }

    void UpdateAI(uint32 diff) override
    {
        // Not his owner's any more once he runs off (SendPurgationIsleOrkusAway clears his quest flags)
        if (!_ownerGUID || !me->GetUInt32Value(UNIT_FIELD_NPC_FLAGS))
            return;

        Player* owner = ObjectAccessor::GetPlayer(*me, _ownerGUID);
        if (!owner)
            return;

        _events.Update(diff);

        while (uint32 eventId = _events.ExecuteEvent())
        {
            if (eventId == EVENT_COMPANION_SHOUT)
            {
                if (owner->IsAlive() && !owner->HasAura(SPELL_COMMANDING_SHOUT) && me->IsWithinDist(owner, 20.0f))
                    DoCast(me, SPELL_COMMANDING_SHOUT, true);
                _events.ScheduleEvent(EVENT_COMPANION_SHOUT, 10s);
            }
        }

        // Guardians hear OwnerAttackedBy but not OwnerAttacked (only the pet does): join the player's own attacks from here
        if (!me->GetVictim())
        {
            if (Unit* target = owner->GetVictim())
                HelpOwner(target);
            else if (!owner->getAttackers().empty())
                HelpOwner(*owner->getAttackers().begin());
        }

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    void HelpOwner(Unit* target)
    {
        if (target && me->GetUInt32Value(UNIT_FIELD_NPC_FLAGS) && !me->GetVictim() && me->IsValidAttackTarget(target))
            AttackStart(target);
    }

    EventMap _events;
    ObjectGuid _ownerGUID;
};

// 48503 - Kingslayer Orkus
// The player's event Orkus (personal; Purgation Isle Summon Controller II): he runs to the player when They Will Never Expect
// This... is turned in, follows them through Hillsbrad and gives Heroes of the Horde! (retail TDB 1210 has him as its only
// starter). When the player is on the way up to the Stormpike camp with the quest, he leads them to the cover of the rock at its
// edge, stops them ("STOP! GET DOWN, $n!"), shows them the Battle Master and tells his plan, then waits for the player's word
// ("Tell me when you are ready and we will go!", 48757; gossip 12440 "I'm ready, Orkus. Let's do this!"). Wowhead comments on
// 28400: "When you reach the top of the Purgation Isle, it starts", "You have to talk to Orkus again to start the event", "When
// you get near the top you will notice that Orkus slows down starting a scripted event ... After a short while you can talk to
// him", and of the bugged event "Orkus is following me". Where the walk starts is ours. A player who says the word on the way
// up gets the old order: he goes up, tells the plan and attacks.
// Then he kills the Stormpike Battle Master and holds the trio while the player loots the Alliance Battle Plans from the body,
// and calls Kasha (Summon Kasha 90278, cast by the player), again every few seconds until she has the player (she gives up on a
// player she cannot pick up: dead, casting or out of reach). The plans' item spell (Send Event 27246) has an empty event in
// 5.4.8: he watches for the plans instead. A player who already holds the plans when he comes (after an interrupted event) is
// picked up as soon as they are on Purgation Isle. If the Battle Master's body is gone before the player took the plans, or the
// player goes too far, the event ends and he goes with his actors; the controller then brings a new Orkus, so the event can
// always be started again. He cannot fall in the fight; Kasha takes him off the hill and he dies on the way to Tarren Mill.
struct npc_hillsbrad_orkus_heroes : public ScriptedAI
{
    npc_hillsbrad_orkus_heroes(Creature* creature) : ScriptedAI(creature), _waitTries(0), _started(false), _atHiding(false),
        _waitingForWord(false), _wordGiven(false), _fighting(false), _carried(false), _kashaCalled(false) { }

    void IsSummonedBy(Unit* /*summoner*/) override
    {
        _playerGUID = me->GetPrivateObjectOwner();

        me->SetLevel(80);
        me->SetMaxHealth(600000);
        me->SetFullHealth();
        me->SetFaction(FACTION_ORGRIMMAR);
        me->SetReactState(REACT_PASSIVE);
        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP | UNIT_NPC_FLAG_QUESTGIVER);

        if (Player* player = ObjectAccessor::GetPlayer(*me, _playerGUID))
        {
            me->SetWalk(false);
            me->GetMotionMaster()->MoveFollow(player, PET_FOLLOW_DIST, float(M_PI / 2));
        }
        _events.ScheduleEvent(EVENT_HEROES_FOLLOW, 1s);
    }

    void DoAction(int32 action) override
    {
        // One left behind after a teleport while the player got a new one: he and his actors just go, without touching the
        // player's auras (the other Orkus' event may be running)
        if (action == ACTION_HEROES_DISCARD)
        {
            _events.Reset();
            _carried = true;
            DespawnActors(0);
            me->DespawnOrUnsummon();
            return;
        }

        if (action == ACTION_HEROES_ABORT || (action == ACTION_HEROES_DISMISS && !_carried))
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
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        DespawnActors(4000);
    }

    // However he goes (the player's logout or map change unsummons the player's summons, Unit::RemoveFromWorld), his actors go too
    void Unsummoned() override
    {
        DespawnActors(0);
    }

    bool OnGossipSelect(Player* player, uint32 menuId, uint32 gossipListId) override
    {
        if (menuId != GOSSIP_MENU_ORKUS_HEROES || gossipListId != GOSSIP_OPTION_ORKUS_READY)
            return false;

        CloseGossipMenuFor(player);

        if (player->GetGUID() != _playerGUID || _carried || _fighting || _wordGiven
            || player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE) != QUEST_STATUS_INCOMPLETE || !player->IsAlive() || player->GetVehicle())
            return true;

        if (_waitingForWord)
        {
            _wordGiven = true;
            _waitingForWord = false;
            me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
            _events.ScheduleEvent(EVENT_HEROES_TALK + 6, 1s);
        }
        // The word on the way: only on Purgation Isle (the option's condition says the same); elsewhere he asks for it there
        else if (!_started)
        {
            if (player->GetAreaId() != AREA_PURGATION_ISLE)
                return true;

            _wordGiven = true;
            StartWalk(player);
        }
        return true;
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
        if (status == QUEST_STATUS_COMPLETE || status == QUEST_STATUS_REWARDED || me->GetDistance(player) > (_started ? 250.0f : 100.0f)
            || (_started && status != QUEST_STATUS_INCOMPLETE))
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
                case EVENT_HEROES_FOLLOW:
                    if (_started)
                        break;
                    if (status == QUEST_STATUS_INCOMPLETE && !player->GetVehicle() && player->GetAreaId() == AREA_PURGATION_ISLE)
                    {
                        // Back with the plans after an interrupted event: Kasha comes for the player
                        if (player->HasItemCount(ITEM_ALLIANCE_BATTLE_PLANS))
                        {
                            _started = true;
                            me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
                            _events.ScheduleEvent(EVENT_HEROES_CALL_KASHA, 1s);
                            break;
                        }
                        // On the way up to the camp (ours: within 40 yards of his cover, he no more than 60 behind)
                        if (player->GetExactDist2d(HeroesHidingPos.GetPositionX(), HeroesHidingPos.GetPositionY()) < 40.0f
                            && me->IsWithinDist(player, 60.0f))
                        {
                            StartWalk(player);
                            break;
                        }
                    }
                    _events.ScheduleEvent(EVENT_HEROES_FOLLOW, 1s);
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
                    next = 6s;
                    break;
                case EVENT_HEROES_TALK + 5:
                    // The player's word, unless it was given on the way up
                    if (_wordGiven)
                        next = 2s;
                    else
                    {
                        Talk(TALK_ORKUS_TELL_ME_WHEN_READY, player);
                        _waitingForWord = true;
                        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                    }
                    break;
                case EVENT_HEROES_TALK + 6:
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
                    // The plans were lost again: back to waiting for them, or (with no Battle Master to loot) to the start
                    if (!player->HasItemCount(ITEM_ALLIANCE_BATTLE_PLANS))
                    {
                        if (!BattleMasterBodyLeft())
                        {
                            Abort(player);
                            return;
                        }
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
    // Up to his cover with the actors in place; the landed Kasha flies off to wait for his call (the player loses her detection)
    void StartWalk(Player* player)
    {
        _started = true;
        me->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        player->RemoveAurasDueToSpell(SPELL_DETECT_QUEST_INVIS_ZONE_4);
        SummonActors();
        me->GetMotionMaster()->Clear();
        me->GetMotionMaster()->MovePoint(POINT_HEROES_HIDING, HeroesHidingPos);
        _events.CancelEvent(EVENT_HEROES_FOLLOW);
        _events.ScheduleEvent(EVENT_HEROES_WALK_TIMEOUT, 60s);
    }

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
        DespawnActors(0);
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

    // The event ended early: the actors go and so does he; a player still here sees the landed Kasha again, and the controller
    // brings a new Orkus
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
    bool _started;
    bool _atHiding;
    bool _waitingForWord;
    bool _wordGiven;
    bool _fighting;
    bool _carried;
    bool _kashaCalled;
};

// 48515 - Stormpike Battle Master, 48508 - Vanndar Stormpike, 48507 - Captain Balinda Stonehearth, 48485 - Packleader Ivar
// Bloodfang: Orkus' event actors on the hilltop. They wait until Orkus strikes and fight only him (the player is sneaking
// past them). The Battle Master falls to Orkus; the trio cannot be killed, they are the reason Orkus does not come back.
struct npc_hillsbrad_purgation_actor : public ScriptedAI
{
    npc_hillsbrad_purgation_actor(Creature* creature) : ScriptedAI(creature), _summonerCheckTimer(2000) { }

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

    void UpdateAI(uint32 diff) override
    {
        // Their Orkus is gone without despawning them (whatever the way): they go too, so they never stand doubled
        if (_summonerCheckTimer <= diff)
        {
            _summonerCheckTimer = 2000;
            TempSummon* summon = me->ToTempSummon();
            if (!summon || !summon->GetSummoner())
            {
                me->DespawnOrUnsummon();
                return;
            }
        }
        else
            _summonerCheckTimer -= diff;

        if (!UpdateVictim())
            return;

        DoMeleeAttackIfReady();
    }

private:
    uint32 _summonerCheckTimer;
};

/*######
## They Will Never Expect This... (28397) and Heroes of the Horde! (28400): Orkus goes with the player
######*/

// 5.4.8 has two controllers, one per Orkus. Purgation Isle Summon Controller I (90130) keeps the companion 48470 with the
// player on Purgation Isle until They Will Never Expect This... is turned in; then he runs off, and Purgation Isle Summon
// Controller II (90206) brings the event Orkus 48503, who runs to the player, gives Heroes of the Horde! and leads the event
// (Wowhead comments on 48470 and 28400; retail TDB 1210 has only 48503 as the starter of 28400). Neither controller has data
// in either TDB: the quest bounds are spell_area's and ours.

namespace
{

// An unsummoned companion stays in the owner's list until the map removes it (TempSummon::UnSummon only queues it)
bool IsLivePurgationIsleOrkus(Creature* orkus, Player* player)
{
    return orkus->IsInWorld() && !orkus->IsDestroyedObject() && orkus->IsAlive() && orkus->IsInMap(player);
}

// The companion: from the end of the flight (The Road to Purgation complete) until They Will Never Expect This... is turned in.
// He ends the one and gives and ends the other.
bool WantsPurgationIsleOrkus(Player* player)
{
    QuestStatus road = player->GetQuestStatus(QUEST_THE_ROAD_TO_PURGATION);
    if (road != QUEST_STATUS_COMPLETE && road != QUEST_STATUS_REWARDED)
        return false;

    return player->GetQuestStatus(QUEST_THEY_WILL_NEVER_EXPECT_THIS) != QUEST_STATUS_REWARDED;
}

Creature* GetPurgationIsleOrkus(Player* player)
{
    std::list<Creature*> companions;
    player->GetAllMinionsByEntry(companions, NPC_ORKUS_PURGATION_ISLE);
    for (Creature* orkus : companions)
        if (IsLivePurgationIsleOrkus(orkus, player))
            return orkus;
    return nullptr;
}

// One companion while the player's quests call for him: one that is dead or was left far behind is replaced (after a login, a
// death or a teleport the player has none), one that is no longer called for leaves
void SummonPurgationIsleOrkus(Player* player)
{
    if (!player->IsInWorld() || !player->IsAlive() || player->IsInFlight() || player->GetVehicle())
        return;

    bool const wanted = WantsPurgationIsleOrkus(player);

    std::list<Creature*> companions;
    player->GetAllMinionsByEntry(companions, NPC_ORKUS_PURGATION_ISLE);

    bool found = false;
    for (Creature* orkus : companions)
    {
        if (orkus->IsDestroyedObject())
            continue;
        if (wanted && !found && IsLivePurgationIsleOrkus(orkus, player) && orkus->IsWithinDist(player, 100.0f))
            found = true;
        else
            orkus->ToTempSummon()->UnSummon();
    }

    if (!wanted)
        player->RemoveAurasDueToSpell(SPELL_SUMMON_ORKUS_PURGATION_ISLE);

    if (found || !wanted)
        return;

    player->CastSpell(player, SPELL_SUMMON_ORKUS_PURGATION_ISLE, true);
}

// The companion's leave when his controller goes: he turns away from the player and runs off (Wowhead comment on 48470: he
// "runs off and disappears when They Will Never Expect This... is turned in"; the distance and the time are ours)
void SendPurgationIsleOrkusAway(Unit* owner)
{
    std::list<Creature*> companions;
    owner->GetAllMinionsByEntry(companions, NPC_ORKUS_PURGATION_ISLE);
    for (Creature* orkus : companions)
    {
        if (orkus->IsDestroyedObject())
            continue;

        if (!orkus->IsAlive() || !orkus->IsInWorld() || !owner->IsInWorld())
        {
            orkus->ToTempSummon()->UnSummon();
            continue;
        }

        orkus->SetUInt32Value(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_NONE);
        orkus->CombatStop(true);
        orkus->SetReactState(REACT_PASSIVE);
        float x, y, z;
        orkus->GetNearPoint(orkus, x, y, z, 25.0f, owner->GetAngle(orkus));
        orkus->GetMotionMaster()->Clear();
        orkus->GetMotionMaster()->MovePoint(0, x, y, z);
        orkus->ToTempSummon()->UnSummon(4 * IN_MILLISECONDS);
    }
}

// The event Orkus: from They Will Never Expect This... turned in until Heroes of the Horde! is done
bool WantsHeroesOrkus(Player* player)
{
    if (player->GetQuestStatus(QUEST_THEY_WILL_NEVER_EXPECT_THIS) != QUEST_STATUS_REWARDED)
        return false;

    QuestStatus heroes = player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE);
    return heroes == QUEST_STATUS_NONE || heroes == QUEST_STATUS_INCOMPLETE;
}

// The player's event Orkuses anywhere on the map (the player's own summons, Unit::m_summons), unsummoned ones left out
void GetHeroesOrkuses(Player* player, std::list<Creature*>& orkuses)
{
    std::list<TempSummon*> summons;
    player->GetSummons(summons, NPC_ORKUS_HEROES);
    for (TempSummon* orkus : summons)
        if (orkus->IsInWorld() && !orkus->IsDestroyedObject() && orkus->IsAlive() && orkus->IsInMap(player))
            orkus->IsAIEnabled ? orkuses.push_back(orkus) : orkus->UnSummon();
}

// One event Orkus of the player's: he comes running from a little way off (ours). One that has gone far from the player
// unsummons himself (npc_hillsbrad_orkus_heroes), but only while he is updated (creatures update near players only): after a
// teleport on the same map (a hearthstone, a spirit healer) he would stay where he was and come back doubled. So only one
// within his event range (250 yards) is kept, and every other one of the player's goes with his actors.
void SummonHeroesOrkus(Player* player)
{
    if (!player->IsInWorld() || !player->IsAlive() || player->IsInFlight() || player->GetVehicle() || !WantsHeroesOrkus(player))
        return;

    std::list<Creature*> orkuses;
    GetHeroesOrkuses(player, orkuses);

    bool found = false;
    for (Creature* orkus : orkuses)
    {
        if (!found && orkus->IsWithinDist(player, 250.0f))
            found = true;
        else
            orkus->AI()->DoAction(ACTION_HEROES_DISCARD);
    }

    if (found)
        return;

    float x, y, z;
    player->GetNearPoint(player, x, y, z, 15.0f, player->GetOrientation() + float(M_PI));
    player->SummonCreature(NPC_ORKUS_HEROES, { x, y, z, player->GetOrientation() }, TEMPSUMMON_MANUAL_DESPAWN, 0ms, 0, player->GetGUID());
}

} // namespace

// 90130 - Purgation Isle Summon Controller I
// The player's aura on Purgation Isle (its casting requirements: area group 2852 = area 896) while the companion Orkus goes
// with the player; on leaving the isle it goes and Orkus with it (Wowhead comment on 28397: "he evaporated about halfway to
// shore, and didn't show up again till I was back on the island"). Its effect 0 is a dummy (5.4.8): here it brings Orkus
// (Summon Orkus 90132). Its periodic trigger every 30 s, Orkus Broadcast (90129, a dummy on the creatures around), is his
// chatter: one of the sixteen lines Wowhead quotes for 48470 outside the Heroes of the Horde! event (broadcast texts 48698 ..
// 48719; "much more chatty than Dumass and Johnny Awesome, sometimes spamming me with paragraphs at a time"), out of combat
// (ours). The aura also checks every 5 s for a missing Orkus (ours, as the Sludge Field Summon Controller) and says a line
// every sixth check.
class spell_hillsbrad_purgation_isle_summon_controller : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_purgation_isle_summon_controller);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_SUMMON_ORKUS_PURGATION_ISLE, SPELL_COMMANDING_SHOUT });
    }

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        // At login the aura comes back before the player is in the world: the first periodic tick brings him then
        if (Player* player = GetTarget()->ToPlayer())
            SummonPurgationIsleOrkus(player);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        SendPurgationIsleOrkusAway(GetTarget());
        GetTarget()->RemoveAurasDueToSpell(SPELL_SUMMON_ORKUS_PURGATION_ISLE);
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    void OnPeriodic(AuraEffect const* aurEff)
    {
        PreventDefaultAction();

        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        // spell_area takes the aura away when the area or the quests change (the quests only on the next area check here)
        if (!WantsPurgationIsleOrkus(player))
        {
            Remove();
            return;
        }

        SummonPurgationIsleOrkus(player);

        // Orkus Broadcast: every 30 s (the DBC period), a line for his owner
        if (aurEff->GetTickNumber() % 6 == 0)
            if (Creature* orkus = GetPurgationIsleOrkus(player))
                if (!orkus->IsInCombat() && !player->IsInCombat() && orkus->IsWithinDist(player, 30.0f) && orkus->IsAIEnabled)
                    orkus->AI()->Talk(TALK_ORKUS_CHATTER, player);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_hillsbrad_purgation_isle_summon_controller::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_hillsbrad_purgation_isle_summon_controller::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_hillsbrad_purgation_isle_summon_controller::CalcPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hillsbrad_purgation_isle_summon_controller::OnPeriodic, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    }
};

// 90206 - Purgation Isle Summon Controller II
// The player's aura in Hillsbrad Foothills (its casting requirements: area group 2822 = zone 267, so the event Orkus also goes
// with Kasha to Tarren Mill) from They Will Never Expect This... turned in until Heroes of the Horde! is done. Its only effect
// is a dummy (5.4.8): here it brings the event Orkus 48503, summoned directly rather than by Summon Orkus 90205 (SummonProperties
// 3081: personal ally, no duration), and checks every 5 s that the player has one (ours). Leaving the zone sends him away,
// unless Kasha already carries him.
class spell_hillsbrad_purgation_isle_summon_controller_heroes : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_purgation_isle_summon_controller_heroes);

    void OnApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (Player* player = GetTarget()->ToPlayer())
            SummonHeroesOrkus(player);
    }

    void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        std::list<Creature*> orkuses;
        GetHeroesOrkuses(player, orkuses);
        for (Creature* orkus : orkuses)
            orkus->AI()->DoAction(ACTION_HEROES_DISMISS);
    }

    void CalcPeriodic(AuraEffect const* /*aurEff*/, bool& isPeriodic, int32& amplitude)
    {
        isPeriodic = true;
        amplitude = 5 * IN_MILLISECONDS;
    }

    void OnPeriodic(AuraEffect const* /*aurEff*/)
    {
        Player* player = GetTarget()->ToPlayer();
        if (!player || !player->IsInWorld())
            return;

        QuestStatus heroes = player->GetQuestStatus(QUEST_HEROES_OF_THE_HORDE);
        if (heroes == QUEST_STATUS_COMPLETE || heroes == QUEST_STATUS_REWARDED)
        {
            Remove();
            return;
        }

        SummonHeroesOrkus(player);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_hillsbrad_purgation_isle_summon_controller_heroes::OnApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_hillsbrad_purgation_isle_summon_controller_heroes::OnRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        DoEffectCalcPeriodic += AuraEffectCalcPeriodicFn(spell_hillsbrad_purgation_isle_summon_controller_heroes::CalcPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_hillsbrad_purgation_isle_summon_controller_heroes::OnPeriodic, EFFECT_0, SPELL_AURA_DUMMY);
    }
};

// 28397 - They Will Never Expect This...
// spell_area does not follow quest changes in this core: at the turn-in the controllers are checked again, so the companion runs
// off and the event Orkus comes at once
class quest_hillsbrad_they_will_never_expect_this : public QuestScript
{
public:
    quest_hillsbrad_they_will_never_expect_this() : QuestScript("quest_hillsbrad_they_will_never_expect_this") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus != QUEST_STATUS_REWARDED || !player->IsInWorld())
            return;

        // Controller II is given zone-wide, Controller I in the area
        player->UpdateZoneDependentAuras(player->GetZoneId());
        player->UpdateAreaDependentAuras(player->GetAreaId());
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

// How long past the flight's own duration Kasha waits for her landing at Tarren Mill before she sets the player down anyway
Milliseconds const HeroesFlightOverdue = 10s;

// 48504 - Kasha
// Called by Orkus once the player holds the plans (Summon Kasha 90278, personal). She comes down beside the player and takes
// them up (seat 1), snatches Orkus out of the fight (seat 0) and flies to Tarren Mill while he dies; there she sets the player
// and his body down before Cromush, and the player is credited. The credit is given directly: Quest Complete 28400 (90379)
// is aimed at her master, which a summon with no owner does not have here. Cromush's lines follow; she settles on the roof
// where the perched Kasha takes over when the player is given its detection.
// As on the Road to Purgation, she is an active object (a creature that cannot enter an unloaded grid is removed, which would
// drop the player), and once she has taken off the player always arrives at Tarren Mill: on her landing; when the player
// gets off on the way (seat 9115 has CAN_ENTER_OR_EXIT), which would otherwise drop them from up to 130 yd; and when the
// landing does not come in time (SetDownAtTarrenMill), watched by her own overdue event and by the player (WatchKashaFlight,
// for an AI that no longer runs). Before the take-off, getting off ends the event as before (the player keeps the plans and
// Kasha is called again on the isle), and so does a player who got off by being teleported elsewhere.
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
        me->setActive(true, ActiveFlags::Script);
        me->SetSpeed(MOVE_RUN, HeroesFlightSpeedRate);
        me->SetSpeed(MOVE_FLIGHT, HeroesFlightSpeedRate);

        _events.ScheduleEvent(EVENT_KASHA_APPROACH, 500ms);
    }

    void DoAction(int32 action) override
    {
        if (action == ACTION_HEROES_ABORT)
            Leave();
        else if (action == ACTION_KASHA_STAND_DOWN && !_arrived)
        {
            // The player's watchdog sets the player down and credits them: she and the dying Orkus only go
            _arrived = true;
            _events.Reset();
            if (Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID))
                orkus->DespawnOrUnsummon();
            me->DespawnOrUnsummon(500);
        }
    }

    void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
    {
        if (apply || passenger->GetGUID() != _playerGUID)
            return;

        if (_started && !_arrived)
            GotOff(passenger->ToPlayer());
    }

    void GetPassengerExitPosition(Unit* passenger, int8 /*seatId*/, Position& pos) override
    {
        if (!_landed)
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
        if (!player || !player->IsInWorld())
        {
            Leave();
            return;
        }

        // Once the player got off on the way, EVENT_KASHA_GOT_OFF decides: the events must go on running
        if (_started && !_arrived && !_gotOff && player->GetVehicleBase() != me)
        {
            GotOff(player);
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
                {
                    TakeOff();
                    _flying = true;
                    Milliseconds const flight = Milliseconds(me->movespline->Duration());
                    _events.ScheduleEvent(EVENT_KASHA_OVERDUE, flight + HeroesFlightOverdue);
                    WatchKashaFlight(player, me, flight + HeroesFlightOverdue + 5s, HeroesPlayerExitPos, QUEST_HEROES_OF_THE_HORDE,
                        SPELL_SUMMON_KASHA_HEROES, "npc_hillsbrad_kasha_heroes (player watchdog)", ACTION_KASHA_STAND_DOWN);
                    if (ObjectAccessor::GetCreature(*me, _orkusGUID))
                        _events.ScheduleEvent(EVENT_KASHA_ORKUS_TALK, 6s);
                    break;
                }
                case EVENT_KASHA_OVERDUE:
                    // The landing did not come (the end of the flight was lost): the player arrives at Tarren Mill all the same
                    LogKashaOverdue("npc_hillsbrad_kasha_heroes", me, player);
                    SetDownAtTarrenMill(player);
                    break;
                case EVENT_KASHA_GOT_OFF:
                    // Still where they got off: set down at Tarren Mill. Gone elsewhere (Player::TeleportTo gets them off before it
                    // marks the teleport, so only where they are by now tells): the event ends early.
                    if (player->IsAlive() && !player->GetVehicle() && !player->IsBeingTeleported() && player->GetMapId() == _gotOffAt.GetMapId()
                        && player->GetExactDist(&_gotOffAt) < 30.0f)
                        SetDownAtTarrenMill(player);
                    else
                    {
                        if (!player->GetVehicle())
                            player->SetDisableGravity(false);
                        Leave();
                        return;
                    }
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
                    _landed = true;
                    _events.CancelEvent(EVENT_KASHA_ORKUS_DIES);
                    _events.CancelEvent(EVENT_KASHA_OVERDUE);
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

    // The player is off before the landing. On the hill, dead or logging out: the event ends early (Leave). In the air, the
    // player is held where they got off (from their next update: PassengerBoarded runs inside Vehicle::RemovePassenger, before
    // the exit's jump is sent) and 300 ms later either set down at Tarren Mill or, if they were teleported away, the event ends.
    void GotOff(Player* player)
    {
        if (_gotOff)
            return;

        if (!_flying || !player || !player->IsInWorld() || player->GetSession()->PlayerLogout() || !player->IsAlive())
        {
            Leave();
            return;
        }

        _gotOff = true;
        _gotOffAt = player->GetWorldLocation();
        player->m_Events.AddLambdaEventAtOffset([player]()
        {
            if (player->IsInWorld() && !player->GetVehicle())
                player->SetDisableGravity(true);
        }, 1);
        // Never left floating, whatever becomes of her
        player->m_Events.AddLambdaEventAtOffset([player]()
        {
            if (player->IsInWorld() && !player->GetVehicle())
                player->SetDisableGravity(false);
        }, 3000);
        _events.Reset();
        _events.ScheduleEvent(EVENT_KASHA_GOT_OFF, 300ms);
    }

    // The landing without the flight: Kasha goes to her spot in front of Cromush first (the grid there loaded for her and for
    // Orkus, who is made active too: a creature moved into an unloaded grid is removed), then Orkus' body, and the player is
    // set down there and credited (SetDownFromKasha: off where Kasha was, held, teleported 300 ms later). The scene goes on as
    // after a landing (Cromush's lines, Kasha on the roof, the area auras checked again at the end).
    void SetDownAtTarrenMill(Player* player)
    {
        _arrived = true;
        _events.Reset();

        Creature* orkus = ObjectAccessor::GetCreature(*me, _orkusGUID);
        if (orkus && orkus->GetVehicleBase() == me)
            orkus->ExitVehicle();

        SetDownFromKasha(player, me, HeroesPlayerExitPos, QUEST_HEROES_OF_THE_HORDE, false);

        me->GetMap()->LoadGrid(HeroesTarrenMillLandingPos.GetPositionX(), HeroesTarrenMillLandingPos.GetPositionY());
        Position landing = HeroesTarrenMillLandingPos;
        landing.m_positionZ = GroundZ(me, landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ());
        me->GetMotionMaster()->Clear();
        me->NearTeleportTo(landing.GetPositionX(), landing.GetPositionY(), landing.GetPositionZ(), landing.GetOrientation());

        if (orkus)
        {
            orkus->setActive(true, ActiveFlags::Script);
            orkus->NearTeleportTo(HeroesOrkusBodyPos.GetPositionX(), HeroesOrkusBodyPos.GetPositionY(), HeroesOrkusBodyPos.GetPositionZ(),
                HeroesOrkusBodyPos.GetOrientation());
            SetLifeless(orkus);
        }

        _events.ScheduleEvent(EVENT_KASHA_CROMUSH_TALK, 2s);
        _events.ScheduleEvent(EVENT_KASHA_PERCH, 5s);
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
    // go, the landed Kasha comes back and Controller II brings a new Orkus); the player keeps the plans, and that Orkus calls
    // Kasha again as soon as the player is back on Purgation Isle.
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
    bool _flying = false;                       // taken off for Tarren Mill
    bool _landed = false;                       // landed there: the passengers get off on their spots
    bool _gotOff = false;                       // the player got off on the way (EVENT_KASHA_GOT_OFF decides)
    WorldLocation _gotOffAt;
};

// Studies in Lethality (28324), Ruins of Southshore: the anger meter.
// Sourced: the bar is Lethality Analyzer 88375 (aura 346, UnitPowerBar 24 "Blight Sample": 0 to 250, starts at 0, no regeneration
// or decay) and fills by one per sample through Specimen Collected 88769 (energize alternate power +1, kill credit 48290); the
// three anger auras Slime Anger 89783/89784/89785 (force reaction of faction 1179 "Slime", the Blight Slimes' faction, to
// Unfriendly, Unfriendly and Hostile; 20 minutes each; "Level I. Moderately peeved", "Level II. Does not like you", "Level III.
// KILL! KILL! KILL!") and their three warnings (broadcast_text 48491-48493). warcraft.wiki.gg: the meter comes with the quest, the
// slimes start neutral, turn unfriendly with those warnings as the player samples them and hostile "near completion". The only
// source for hostile at half the meter is one Wowhead comment (quest 28324, 2010-12-04): "If the "angry-meter" ... goes to half
// full, the Blight Slimes will go hostile. The Slimes will (over time) become less hostile and finally neutral again. That is if
// you don't let the meter become completely full". Later Wowhead comments say a logout resets the meter to 0 and calms the
// slimes while the samples still count, and that the bar "frequently resets"; that is not reproduced: the meter is kept here.
// The wiki's Studies in Lethality page also says the last warning offers Angry Scrubbing Bubbles (28329) and makes the Angry
// Blight Slimes attackable; its 28329 page and the data (48319 faction 14, 28329 pushed on its death) have 28329 start from
// killing one, which is kept.
// Ours: no source applies any of these spells (no sniffed spell_area, SmartAI or script). The bar comes with the quest and goes at
// its completion or abandonment; the anger level follows the meter (Level I from a fifth, Level II from two fifths, Level III
// from half), each sample refreshes it, and when a level runs out it falls to the one below. A full meter keeping the slimes
// hostile is not done: a full meter is the 250th sample, which completes the quest. Sampling does not start combat (no initial
// aggro on 89752/89756, spell_custom_attribute), so a slime only fights the player once Level III makes it hostile: then the
// sampled slime attacks at once, the others as the player comes into their sight.
// Groupmates get 88769's kill credit (RewardPlayerAndGroupAtEvent) but not its energize, so a groupmate's quest count runs ahead
// of their own meter and anger.
enum StudiesInLethality
{
    QUEST_STUDIES_IN_LETHALITY                  = 28324,

    SPELL_LETHALITY_ANALYZER                    = 88375,
    SPELL_SLIME_ANGER_LEVEL_1                   = 89783,
    SPELL_SLIME_ANGER_LEVEL_2                   = 89784,
    SPELL_SLIME_ANGER_LEVEL_3                   = 89785,

    TEXT_SLIMES_REACT_NEGATIVELY                = 48491,    // "The Blight Slimes are beginning to react negatively towards your prodding."
    TEXT_SLIMES_GETTING_ANGRY                   = 48492,    // "The Blight Slimes are getting ANGRY!"
    TEXT_SLIMES_WANT_YOUR_BLOOD                 = 48493,    // "The Blight Slimes want your blood!"
};

static uint32 const SlimeAngerSpells[3] = { SPELL_SLIME_ANGER_LEVEL_1, SPELL_SLIME_ANGER_LEVEL_2, SPELL_SLIME_ANGER_LEVEL_3 };
static uint32 const SlimeAngerTexts[3] = { TEXT_SLIMES_REACT_NEGATIVELY, TEXT_SLIMES_GETTING_ANGRY, TEXT_SLIMES_WANT_YOUR_BLOOD };

// The player's anger level, 0 (none) to 3
static uint8 GetSlimeAngerLevel(Player* player)
{
    for (uint8 level = 3; level > 0; --level)
        if (player->HasAura(SlimeAngerSpells[level - 1]))
            return level;
    return 0;
}

static void SetSlimeAngerLevel(Player* player, uint8 level)
{
    for (uint8 i = 0; i < 3; ++i)
        if (i + 1 != level)
            player->RemoveAurasDueToSpell(SlimeAngerSpells[i]);

    if (level)
        player->CastSpell(player, SlimeAngerSpells[level - 1], true);
}

// 28324 - Studies in Lethality
class quest_hillsbrad_studies_in_lethality : public QuestScript
{
public:
    quest_hillsbrad_studies_in_lethality() : QuestScript("quest_hillsbrad_studies_in_lethality") { }

    void OnQuestStatusChange(Player* player, Quest const* /*quest*/, QuestStatus /*oldStatus*/, QuestStatus newStatus) override
    {
        if (newStatus == QUEST_STATUS_INCOMPLETE)
        {
            if (!player->HasAura(SPELL_LETHALITY_ANALYZER))
                player->CastSpell(player, SPELL_LETHALITY_ANALYZER, true);
            return;
        }

        player->RemoveAurasDueToSpell(SPELL_LETHALITY_ANALYZER);
        if (newStatus == QUEST_STATUS_NONE)
            SetSlimeAngerLevel(player, 0);
    }
};

// 88769 - Specimen Collected (cast by the Blight Slime on the player for every Depleted it takes)
class spell_hillsbrad_specimen_collected : public SpellScript
{
    PrepareSpellScript(spell_hillsbrad_specimen_collected);

    void GiveMeter()
    {
        // The meter of a player who took the quest before it had one, or whose meter lost its maximum
        Player* player = GetHitPlayer();
        if (!player || player->GetQuestStatus(QUEST_STUDIES_IN_LETHALITY) != QUEST_STATUS_INCOMPLETE)
            return;

        if (player->HasAura(SPELL_LETHALITY_ANALYZER))
        {
            if (player->GetMaxPower(POWER_ALTERNATE_POWER) > 0)
                return;
            player->RemoveAurasDueToSpell(SPELL_LETHALITY_ANALYZER);
        }

        player->CastSpell(player, SPELL_LETHALITY_ANALYZER, true);
    }

    void UpdateAnger()
    {
        Player* player = GetHitPlayer();
        if (!player || !player->HasAura(SPELL_LETHALITY_ANALYZER))
            return;

        int32 maxPower = player->GetMaxPower(POWER_ALTERNATE_POWER);
        if (maxPower <= 0)
            return;

        int32 power = player->GetPower(POWER_ALTERNATE_POWER);
        uint8 level = 0;
        if (power * 2 >= maxPower)
            level = 3;
        else if (power * 5 >= maxPower * 2)
            level = 2;
        else if (power * 5 >= maxPower)
            level = 1;

        uint8 oldLevel = GetSlimeAngerLevel(player);
        if (level == oldLevel)
        {
            if (level)
                if (Aura* aura = player->GetAura(SlimeAngerSpells[level - 1]))
                    aura->RefreshDuration();
        }
        else
        {
            // Rising comes with its warning; lower (a meter that was given again) without one
            SetSlimeAngerLevel(player, level);
            if (level > oldLevel)
                if (Unit* slime = GetCaster())
                    slime->Whisper(SlimeAngerTexts[level - 1], player, true);
        }

        // Sampling itself never starts combat (Collect Specimen and Depleted have no initial aggro), so a slime that the
        // anger made hostile turns on the player who is sampling it ("hostile to you and attack you on sight"), who may be
        // standing still and so never come into its sight again.
        if (Creature* slime = GetCaster() ? GetCaster()->ToCreature() : nullptr)
            if (slime->IsAlive() && slime->IsAIEnabled && !slime->IsInCombat() && slime->IsHostileTo(player))
                slime->AI()->AttackStart(player);
    }

    void Register() override
    {
        BeforeHit += SpellHitFn(spell_hillsbrad_specimen_collected::GiveMeter);
        AfterHit += SpellHitFn(spell_hillsbrad_specimen_collected::UpdateAnger);
    }
};

// 89784, 89785 - Slime Anger (Levels II and III)
class spell_hillsbrad_slime_anger : public AuraScript
{
    PrepareAuraScript(spell_hillsbrad_slime_anger);

    void CalmDown(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (Player* player = GetTarget()->ToPlayer())
            player->CastSpell(player, GetId() == SPELL_SLIME_ANGER_LEVEL_3 ? SPELL_SLIME_ANGER_LEVEL_2 : SPELL_SLIME_ANGER_LEVEL_1, true);
    }

    void Register() override
    {
        AfterEffectRemove += AuraEffectRemoveFn(spell_hillsbrad_slime_anger::CalmDown, EFFECT_0, SPELL_AURA_FORCE_REACTION, AURA_EFFECT_HANDLE_REAL);
    }
};

void AddSC_hillsbrad_purgation()
{
    new spell_script<spell_hillsbrad_summon_kasha_road_to_purgation>("spell_hillsbrad_summon_kasha_road_to_purgation");
    new aura_script<spell_hillsbrad_ride_to_purgation>("spell_hillsbrad_ride_to_purgation");
    new creature_script<npc_hillsbrad_kasha_road_to_purgation>("npc_hillsbrad_kasha_road_to_purgation");
    new creature_script<npc_hillsbrad_orkus_purgation_isle>("npc_hillsbrad_orkus_purgation_isle");
    new aura_script<spell_hillsbrad_purgation_isle_summon_controller>("spell_hillsbrad_purgation_isle_summon_controller");
    new aura_script<spell_hillsbrad_purgation_isle_summon_controller_heroes>("spell_hillsbrad_purgation_isle_summon_controller_heroes");
    new quest_hillsbrad_they_will_never_expect_this();
    new creature_script<npc_hillsbrad_orkus_heroes>("npc_hillsbrad_orkus_heroes");
    new creature_script<npc_hillsbrad_purgation_actor>("npc_hillsbrad_purgation_actor");
    new creature_script<npc_hillsbrad_kasha_heroes>("npc_hillsbrad_kasha_heroes");
    new quest_hillsbrad_studies_in_lethality();
    new spell_script<spell_hillsbrad_specimen_collected>("spell_hillsbrad_specimen_collected");
    new aura_script<spell_hillsbrad_slime_anger>("spell_hillsbrad_slime_anger");
}
