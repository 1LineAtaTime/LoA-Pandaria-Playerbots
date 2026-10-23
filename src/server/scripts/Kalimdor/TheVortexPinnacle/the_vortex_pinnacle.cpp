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

#include"ScriptPCH.h"
#include "MoveSplineInit.h"
#include "Vehicle.h"
#include "the_vortex_pinnacle.h"

// �������������� � ������ ���� �� �����������

enum Creatures
{
    NPC_SLIPSTREAM_LANDING_ZONE     = 45504,
    NPC_SKYFALL_LAND                = 45981,

    NPC_CLOUD_PRINCE                = 45917,
    NPC_WIPPING_WIND                = 47238,
    NPC_YOUNG_STORM_DRAGON          = 45919,
    NPC_ARMORED_MISTRAL             = 45915,
    NPC_EMPYREAN_ASSASSIN           = 45922,
    NPC_EXECUTOR_OF_THE_CALIPH      = 45928,
    NPC_GUST_SOLDIER                = 45477,
    NPC_LURKING_TEMPEST             = 45704,
    NPC_MINISTER_OF_AIR             = 45930,
    NPC_SERVANT_OF_ASAAD            = 45926,
    NPC_TEMPEST_ADEPT               = 45935,
    NPC_TURBULENT_SQUALL            = 45924,
    NPC_HOWLING_GALE                = 45572,
    NPC_WILD_VORTEX                 = 45912,
    NPC_SKYFALL_STAR                = 45932,
    NPC_GOLDEN_ORB                  = 51157,
    NPC_ITESH                       = 49943,
    NPC_WORLD_TRIGGER_CATCH_FALL    = 21252,
};

enum Spells
{
    // Slipstream
    SPELL_SLIPSTREAM_AURA           = 85021,
    SPELL_SLIPSTREAM_VEH            = 87742,

    // Cloud Prince
    SPELL_TYPHOON                   = 88074,
    SPELL_STARFALL                  = 88073,

    // Whipping Wind
    SPELL_WHW_LIGHTNING_BOLT        = 88080,

    // Young Storm Dragon
    SPELL_BRUTAL_STRIKES            = 88192,
    SPELL_BRUTAL_STRIKES_DMG        = 88188,
    SPELL_CHILLING_BLAST            = 88194,
    SPELL_HEALING_WELL              = 88201,

    // Armored Mistral
    SPELL_GALE_STRIKE               = 88061,
    SPELL_RISING_WINDS              = 88057,
    SPELL_STORM_SURGE               = 88055,

    // Empyrean Assassin
    SPELL_LETHARGIC_POISON          = 88184,
    SPELL_VAPOR_FORM                = 88186,

    // Executor of the Caliph
    SPELL_DEVASTATE                 = 78660,
    SPELL_RALLY                     = 87761,
    SPELL_SHOCKWAVE                 = 87759,
    
    // Gust Soldier
    SPELL_AIR_NOVA                  = 87933,
    SPELL_CHARGE                    = 87930,
    SPELL_WIND_BLAST                = 87923,

    // Lurking Tempest
    SPELL_LT_LIGHTNING_BOLT         = 89105,
    SPELL_FEIGN_DEATH               = 85267,
    SPELL_LURK                      = 85467,

    // Minister of Air
    SPELL_LIGHTNING_LASH            = 87762,
    SPELL_LIGHTNING_LASH_DUMMY      = 87765,
    SPELL_LIGHTNING_LASH_DMG        = 88963,
    SPELL_LIGHTNING_NOVA            = 87768,

    // Servant of Asaad
    SPELL_CRUSADER_STRIKE           = 87771,
    SPELL_DIVINE_STORM              = 58112,
    SPELL_HAND_OF_PROTECTION        = 87772,

    // Temple Adept
    SPELL_DESPERATE_SPEED           = 87780,
    SPELL_GREATER_HEAL              = 87779,
    SPELL_HOLY_SMITE                = 88959,

    // Turbulent Squall
    SPELL_ASPHYXIATE                = 88175,
    SPELL_CLOUDBURST                = 88170,
    SPELL_HURRICANE                 = 88171,

    // Wild Vortex
    SPELL_WV_LIGHTNING_BOLT         = 88032,
    SPELL_WIND_SHOCK                = 88029,
    SPELL_CYCLONE                   = 88010,

    // Skyfall Star
    SPELL_ARCANE_BARRAGE            = 87854,

    // Howling Gale
    SPELL_HOWLING_GALE              = 85084,
    SPELL_HOWLING_GALE_1            = 85086,
    SPELL_HOWLING_GALE_2            = 85136,
    SPELL_HOWLING_GALE_3            = 85086,
    SPELL_HOWLING_GALE_DMG          = 85159,

    // Fall Catcher
    SPELL_SLIPSTREAM_BUFF           = 87740,
    SPELL_CATCH_FALL_SUMMON_TRIGGER = 89517,
    SPELL_CATCH_FALL_SUMMON_AURA    = 89522,
    SPELL_CATCH_FALL_VEHICLE        = 89524,
    SPELL_CATCH_FALL_JUMP           = 89526,

    SPELL_ACHIEVEMENT               = 94756,
};

enum Events
{
    EVENT_ARCANE_BARRAGE            = 1,
    EVENT_TYPHOON                   = 2,
    EVENT_STARFALL                  = 3,
    EVENT_WHW_LIGHTNING_BOLT        = 4,
    EVENT_CHILLING_BLAST            = 5,
    EVENT_GALE_STRIKE               = 6,
    EVENT_STORM_SURGE               = 7,
    EVENT_VAPOR_FORM                = 8,
    EVENT_DEVASTATE                 = 9,
    EVENT_SHOCKWAVE                 = 10,
    EVENT_RALLY                     = 11,
    EVENT_AIR_NOVA                  = 12,
    EVENT_WIND_BLAST                = 13,
    EVENT_LIGHTNING_NOVA            = 14,
    EVENT_LIGHTNING_LASH            = 15,
    EVENT_CRUSADER_STRIKE           = 16,
    EVENT_HAND_OF_PROTECTION        = 17,
    EVENT_DESPERATE_SPEED           = 18,
    EVENT_GREATER_HEAL              = 19,
    EVENT_HOLY_SMITE                = 20,
    EVENT_ASPHYXIATE                = 21,
    EVENT_HURRICANE                 = 22,
    EVENT_CLOUDBURST                = 23,
    EVENT_CYCLONE                   = 24,
    EVENT_WIND_SHOCK                = 25,
    EVENT_WV_LIGHTNING_BOLT         = 26,
    EVENT_HOWLING_GALE              = 27,
    EVENT_HOWLING_GALE_DEACTIVATE   = 28,
};

const Position teleportPos[2] = 
{
    {-906.08f, -176.51f, 664.50f, 2.86f},
    {-1193.67f, 472.83f, 634.86f, 0.50f},
};

enum Other
{
    TYPE_SLIPSTREAM    = 1,
};

// #692 (BEYOND AC - AC WotLK has no Cataclysm dungeon): the Slipstream is a RIDE, not a teleport.
// Cata (The Cataclysm Preservation Project TrinityCore, src/server/scripts/Kalimdor/VortexPinnacle/vortex_pinnacle.cpp
// npc_slipstream / spell_slipstream and instance_vortex_pinnacle.cpp SlipStreamInfo, read 2026-10-23) passes the clicker along
// a chain of slipstream vehicles (45455) to the Slipstream Landing Zone (45504), which ejects it. This core's world DB holds only
// the first slipstream of each chain (spawns 371473 after Ertan, 371471 after Altairus) and the two landing zones, so the ride is
// one carrier per rider: a 45455 (Vehicle 1551: 5 passenger seats 9695-9699, VehicleSeat flags CAN_ENTER_OR_EXIT | UNCONTROLLED)
// summoned at the clicked slipstream flies the Cata chain's nodes to the landing zone and sets the rider down on its navmesh
// surface (mmaps 0657) with Slipstream 87740 (12 s safe fall), the buff npc_fall_catcher_5 gives after a catch.
// A player clicks it (gossip, or the vehicle spellclick); a bot clicks it through Unit::HandleSpellClick -> OnSpellClick
// (mod_playerbots strategy/dungeons/CataVp "vp slipstream"). The two entrance shortcuts the instance summons when Ertan /
// Altairus dies (PastVortexSpawnPos, no spawn id) keep the old teleport.
enum SlipstreamRide
{
    TYPE_SLIPSTREAM_CARRIER     = 2,    // SetData/GetData on a 45455: this one is a ride carrier, value = route index + 1
    POINT_SLIPSTREAM_LANDING    = 1,
    SLIPSTREAM_ROUTE_COUNT      = 2,
    SLIPSTREAM_RIDE_MAX_NODES   = 6,
};

float const SLIPSTREAM_RIDE_SPEED           = 30.0f;    // yd/s: ~11 s after Ertan (~310 yd), ~17 s after Altairus (~500 yd)
float const SLIPSTREAM_CLICK_RANGE          = 10.0f;    // a click from farther away is refused (gossip already checks 5 yd)
uint32 const SLIPSTREAM_BOARD_TIMEOUT       = 5000;     // a carrier nobody boarded within this despawns
uint32 const SLIPSTREAM_RIDE_TIMEOUT        = 60000;    // a ride still flying after this sets its rider down at once
uint32 const SLIPSTREAM_RECLICK_DELAY       = 5000;     // one carrier per rider and click

struct SlipstreamRoute
{
    uint32 gateBoss;                                    // the ride carries once this boss is DONE (as the old teleport did)
    Position source;                                    // the DB slipstream that starts it
    uint32 nodeCount;
    Position nodes[SLIPSTREAM_RIDE_MAX_NODES];          // flown in order; the last one hovers 2 yd above the landing point
    Position landing;                                   // the rider's exit point: landing zone 45504 x/y, navmesh surface z
};

// Nodes: Cata Preservation SlipStreamInfo (DATA_SLIPSTREAM_ERTAN_1..3, DATA_SLIPSTREAM_ALTAIRUS_1..5); landing x/y = this DB's
// 45504 spawns 332211 / 332250, z = mmtile_probe.py surface (0657 tiles: 665.565 / 634.762).
SlipstreamRoute const slipstreamRoutes[SLIPSTREAM_ROUTE_COUNT] =
{
    // after Grand Vizier Ertan -> the Altairus platform
    { DATA_ERTAN, { -765.44f, -41.37f, 639.93f, 0.0f }, 4,
        { { -775.517f, -70.9323f, 640.3123f, 0.0f }, { -848.227f, -68.724f, 654.2203f, 0.0f }, { -844.885f, -205.135f, 660.7083f, 0.0f },
          { -906.08f, -176.514f, 667.6f, 0.0f } },
        { -906.08f, -176.514f, 665.6f, 2.86f } },
    // after Altairus -> the Asaad platform
    { DATA_ALTAIRUS, { -1198.79f, 107.046f, 740.707f, 0.0f }, 6,
        { { -1190.88f, 125.203f, 737.6243f, 0.0f }, { -1138.55f, 178.524f, 711.4943f, 0.0f }, { -1245.21f, 230.986f, 690.6083f, 0.0f },
          { -1282.07f, 344.856f, 660.9873f, 0.0f }, { -1229.64f, 412.26f, 641.2933f, 0.0f }, { -1193.67f, 472.835f, 636.8f, 0.0f } },
        { -1193.67f, 472.835f, 634.8f, 0.5f } },
};

class npc_vortex_pinnacle_slipsteam : public CreatureScript
{
    public:
        npc_vortex_pinnacle_slipsteam() : CreatureScript("npc_vortex_pinnacle_slipsteam") { }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            // #692: a ride slipstream carries (or refuses); only the entrance shortcuts fall through to the teleport
            if (npc_vortex_pinnacle_slipsteamAI* ai = CAST_AI(npc_vortex_pinnacle_slipsteamAI, creature->AI()))
            {
                int32 const ride = ai->TryRide(player);
                if (ride >= 0)
                {
                    player->PlayerTalkClass->SendCloseGossip();
                    return true;
                }
            }

            if (InstanceScript* instance = creature->GetInstanceScript())
            {
                if (instance->GetBossState(DATA_ALTAIRUS) == DONE)
                {
                    player->NearTeleportTo(
                        teleportPos[1].GetPositionX(),
                        teleportPos[1].GetPositionY(),
                        teleportPos[1].GetPositionZ()+10.0f,
                        teleportPos[1].GetOrientation());
                    return true;
                }
                else if (instance->GetBossState(DATA_ERTAN) == DONE)
                {
                    player->NearTeleportTo(
                        teleportPos[0].GetPositionX(),
                        teleportPos[0].GetPositionY(),
                        teleportPos[0].GetPositionZ()+10.0f,
                        teleportPos[0].GetOrientation());
                    return true;
                }
            }
            return false;
        }

        struct npc_vortex_pinnacle_slipsteamAI : public ScriptedAI
        {
            npc_vortex_pinnacle_slipsteamAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetCanFly(true);
                SetCombatMovement(false);
            }

            void Reset() override
            {
                if (_route)     // #692: a carrier never turns back into a clickable slipstream
                    return;

                me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                DoCast(me, SPELL_SLIPSTREAM_AURA);
            }

            // #692: a bot (Unit::HandleSpellClick) or a client's CMSG_SPELLCLICK - Vehicle 1551 has usable seats, so the
            // vehicle kit flags it UNIT_NPC_FLAG_SPELLCLICK. No npc_spellclick_spells row: this hook is the whole click.
            void OnSpellClick(Unit* clicker, bool& result) override
            {
                if (Player* player = clicker ? clicker->ToPlayer() : nullptr)
                    if (TryRide(player) > 0)
                        result = true;
            }

            // -1: not a ride slipstream (entrance shortcut, or a carrier), 0: refused, 1: the rider is boarding a carrier
            int32 TryRide(Player* player)
            {
                if (_route || !me->GetDBTableGUIDLow())
                    return -1;

                int32 index = -1;
                for (int32 i = 0; i < SLIPSTREAM_ROUTE_COUNT; ++i)
                    if (me->GetExactDist2d(slipstreamRoutes[i].source.GetPositionX(), slipstreamRoutes[i].source.GetPositionY()) < 10.0f)
                        index = i;

                if (index < 0)
                    return -1;

                InstanceScript* instance = me->GetInstanceScript();
                if (!instance || instance->GetBossState(slipstreamRoutes[index].gateBoss) != DONE)
                    return 0;

                if (!player->IsAlive() || player->GetVehicle() || player->IsBeingTeleported() ||
                    !player->IsWithinDistInMap(me, SLIPSTREAM_CLICK_RANGE))
                    return 0;

                uint32 const now = getMSTime();
                auto const last = _lastRide.find(player->GetGUID());
                if (last != _lastRide.end() && getMSTimeDiff(last->second, now) < SLIPSTREAM_RECLICK_DELAY)
                    return 0;

                Creature* carrier = me->SummonCreature(NPC_SLIPSTREAM, *me, TEMPSUMMON_MANUAL_DESPAWN);
                if (!carrier)
                    return 0;

                if (!carrier->GetVehicleKit() || !carrier->AI())
                {
                    carrier->DespawnOrUnsummon();
                    return 0;
                }

                carrier->AI()->SetData(TYPE_SLIPSTREAM_CARRIER, uint32(index + 1));
                _lastRide[player->GetGUID()] = now;
                player->EnterVehicle(carrier, -1, true);
                return 1;
            }

            void SetData(uint32 type, uint32 data) override
            {
                if (type != TYPE_SLIPSTREAM_CARRIER || !data || data > SLIPSTREAM_ROUTE_COUNT)
                    return;

                _route = data;
                _timer = SLIPSTREAM_BOARD_TIMEOUT;
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetDisableGravity(true);
            }

            uint32 GetData(uint32 type) const override
            {
                return type == TYPE_SLIPSTREAM_CARRIER ? _route : 0;
            }

            void PassengerBoarded(Unit* passenger, int8 /*seatId*/, bool apply) override
            {
                if (!_route || !passenger)
                    return;

                if (!apply)
                {
                    if (passenger->GetGUID() == _passenger)
                    {
                        _passenger.Clear();
                        me->DespawnOrUnsummon(1000);
                    }
                    return;
                }

                if (_flying)
                    return;

                _passenger = passenger->GetGUID();
                _flying = true;
                _timer = SLIPSTREAM_RIDE_TIMEOUT;

                SlipstreamRoute const& route = slipstreamRoutes[_route - 1];
                Movement::PointsArray path;
                path.emplace_back(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ());   // MoveSplineInit::Launch puts the real start here
                for (uint32 i = 0; i < route.nodeCount; ++i)
                    path.emplace_back(route.nodes[i].GetPositionX(), route.nodes[i].GetPositionY(), route.nodes[i].GetPositionZ());

                std::function<void(Movement::MoveSplineInit&)> initializer = [path](Movement::MoveSplineInit& init)
                {
                    init.MovebyPath(path);
                    init.SetFly();
                    init.SetSmooth();
                    init.SetUncompressed();
                    init.SetVelocity(SLIPSTREAM_RIDE_SPEED);
                };
                me->GetMotionMaster()->LaunchMoveSpline(std::move(initializer), POINT_SLIPSTREAM_LANDING, MOTION_SLOT_ACTIVE, POINT_MOTION_TYPE);
            }

            // Vehicle::RemovePassenger overwrites any ExitVehicle() position with this hook's: the rider leaves at the landing point
            void GetPassengerExitPosition(Unit* /*passenger*/, int8 /*seatId*/, Position& pos) override
            {
                if (_route)
                    pos.Relocate(slipstreamRoutes[_route - 1].landing);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (_route && type == POINT_MOTION_TYPE && pointId == POINT_SLIPSTREAM_LANDING)
                    Land();
            }

            void Land()
            {
                if (_landed)
                    return;

                _landed = true;
                if (Unit* passenger = ObjectAccessor::GetUnit(*me, _passenger))
                {
                    if (passenger->GetVehicleBase() == me)
                    {
                        passenger->ExitVehicle();
                        passenger->CastSpell(passenger, SPELL_SLIPSTREAM_BUFF, true);
                    }
                }

                me->DespawnOrUnsummon(1000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!_route || _landed)
                    return;

                if (_timer > diff)
                {
                    _timer -= diff;
                    return;
                }

                if (_flying)
                    Land();
                else
                {
                    _landed = true;
                    me->DespawnOrUnsummon();
                }
            }

        private:
            uint32 _route = 0;          // carrier only: route index + 1
            uint32 _timer = 0;
            bool _flying = false;
            bool _landed = false;
            ObjectGuid _passenger;
            std::map<ObjectGuid, uint32> _lastRide;    // source slipstream only: the last click that started a ride, per rider
     };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_vortex_pinnacle_slipsteamAI>(creature);
        }
};

class npc_skyfall_star : public CreatureScript
{
    public:
        npc_skyfall_star() : CreatureScript("npc_skyfall_star") { }

        struct npc_skyfall_starAI : public ScriptedAI
        {
            npc_skyfall_starAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                SetCombatMovement(false);
            }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustDied(Unit* /*killer*/) override
            {
                me->SetCanFly(false);
                me->SetDisableGravity(false);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(1500, 5000));
                me->GetMotionMaster()->MoveRandom(10.0f);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ARCANE_BARRAGE:
                            DoCast(SPELL_ARCANE_BARRAGE);
                            events.ScheduleEvent(EVENT_ARCANE_BARRAGE, urand(3500, 10500));
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_skyfall_starAI>(creature);
        }
};

class npc_cloud_prince : public CreatureScript
{
    public:
        npc_cloud_prince() : CreatureScript("npc_cloud_prince") { }

        struct npc_cloud_princeAI : public ScriptedAI
        {
            npc_cloud_princeAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_TYPHOON, urand(5000, 7000));
                events.ScheduleEvent(EVENT_STARFALL, urand(7000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TYPHOON:
                            DoCast(SPELL_TYPHOON);
                            events.ScheduleEvent(EVENT_TYPHOON, urand(15000, 20000));
                            break;
                        case EVENT_STARFALL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_STARFALL);
                            events.ScheduleEvent(EVENT_STARFALL, 20000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_cloud_princeAI>(creature);
        }
};

class npc_whipping_wind : public CreatureScript
{
    public:
        npc_whipping_wind() : CreatureScript("npc_whipping_wind") { }

        struct npc_whipping_windAI : public ScriptedAI
        {
            npc_whipping_windAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_WHW_LIGHTNING_BOLT, 2000);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_WHW_LIGHTNING_BOLT:
                            DoCast(me->GetVictim(), SPELL_WHW_LIGHTNING_BOLT);
                            events.ScheduleEvent(EVENT_WHW_LIGHTNING_BOLT, 2000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_whipping_windAI>(creature);
        }
};

class npc_young_storm_dragon : public CreatureScript
{
    public:
        npc_young_storm_dragon() : CreatureScript("npc_young_storm_dragon") { }

        struct npc_young_storm_dragonAI : public ScriptedAI
        {
            npc_young_storm_dragonAI(Creature* creature) : ScriptedAI(creature)
            {
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_STUN, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_ROOT, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FREEZE, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_POLYMORPH, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_HORROR, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_SAPPED, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_CHARM, true);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_DISORIENTED, true);
            }

            EventMap events;

            void Reset() override
            {
                DoCast(me, SPELL_BRUTAL_STRIKES);
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                DoCast(me, SPELL_HEALING_WELL);
                events.ScheduleEvent(EVENT_CHILLING_BLAST, urand(12000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHILLING_BLAST:
                            DoCast(me->GetVictim(), SPELL_CHILLING_BLAST);
                            events.ScheduleEvent(EVENT_CHILLING_BLAST, urand(15000, 18000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_young_storm_dragonAI>(creature);
        }
};

class npc_armored_mistral : public CreatureScript
{
    public:
        npc_armored_mistral() : CreatureScript("npc_armored_mistral") { }

        struct npc_armored_mistralAI : public ScriptedAI
        {
            npc_armored_mistralAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                DoCast(me, SPELL_RISING_WINDS);
                events.ScheduleEvent(EVENT_GALE_STRIKE, urand(2000, 4000));
                events.ScheduleEvent(EVENT_STORM_SURGE, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GALE_STRIKE:
                            DoCast(me, SPELL_GALE_STRIKE);
                            events.ScheduleEvent(EVENT_GALE_STRIKE, urand(15000, 20000));
                            break;
                        case EVENT_STORM_SURGE:
                            DoCast(me, SPELL_STORM_SURGE);
                            events.ScheduleEvent(EVENT_STORM_SURGE, urand(15000, 20000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_armored_mistralAI>(creature);
        }
};

class npc_empyrean_assassin : public CreatureScript
{
    public:
        npc_empyrean_assassin() : CreatureScript("npc_empyrean_assassin") { }

        struct npc_empyrean_assassinAI : public ScriptedAI
        {
            npc_empyrean_assassinAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                DoCast(me, SPELL_LETHARGIC_POISON);
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_VAPOR_FORM, urand(15000, 20000));    
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VAPOR_FORM:
                            DoCast(me, SPELL_VAPOR_FORM);
                            events.ScheduleEvent(EVENT_VAPOR_FORM, urand(20000, 25000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_empyrean_assassinAI>(creature);
        }
};

class npc_executor_of_the_caliph : public CreatureScript
{
    public:
        npc_executor_of_the_caliph() : CreatureScript("npc_executor_of_the_caliph") { }

        struct npc_executor_of_the_caliphAI : public ScriptedAI
        {
            npc_executor_of_the_caliphAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_RALLY, urand(5000, 20000));
                events.ScheduleEvent(EVENT_DEVASTATE, urand(2000, 8000));
                events.ScheduleEvent(EVENT_SHOCKWAVE, urand(12000, 20000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_RALLY:
                            DoCast(SPELL_RALLY);
                            events.ScheduleEvent(EVENT_RALLY, urand(20000, 30000));
                            break;
                        case EVENT_SHOCKWAVE:
                            DoCast(SPELL_SHOCKWAVE);
                            events.ScheduleEvent(EVENT_SHOCKWAVE, urand(16000, 30000));
                            break;
                        case EVENT_DEVASTATE:
                            DoCast(me->GetVictim(), SPELL_DEVASTATE);
                            events.ScheduleEvent(EVENT_DEVASTATE, urand(15000, 18000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
     };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_executor_of_the_caliphAI>(creature);
        }
};

class npc_gust_soldier : public CreatureScript
{
    public:
        npc_gust_soldier() : CreatureScript("npc_gust_soldier") { }

        struct npc_gust_soldierAI : public ScriptedAI
        {
            npc_gust_soldierAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* who) override
            {
                DoCast(who, SPELL_CHARGE);
                events.ScheduleEvent(EVENT_AIR_NOVA, urand(5000, 15000));
                events.ScheduleEvent(EVENT_WIND_BLAST, urand(6000, 8000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_AIR_NOVA:
                        DoCast(SPELL_AIR_NOVA);
                        events.ScheduleEvent(EVENT_AIR_NOVA, urand(10000, 15000));
                        break;
                    case EVENT_WIND_BLAST:
                        DoCast(me->GetVictim(), SPELL_WIND_BLAST);
                        events.ScheduleEvent(EVENT_WIND_BLAST, urand(6000, 8000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_gust_soldierAI>(creature);
        }
};

struct npc_lurking_tempest : public ScriptedAI
{
    enum Spells
    {
        SPELL_LIGHTNING_BOLT = 89105,
        SPELL_LURK           = 85467,
        SPELL_FEIGN_DEATH    = 85267,
    };

    enum Events
    {
        EVENT_LIGHTNING_BOLT = 1,
        EVENT_LURK           = 2,
    };

    npc_lurking_tempest(Creature* creature) : ScriptedAI(creature)
    {
        SetCombatMovement(false);
    }

    void Reset() override 
    {
        me->RemoveAurasDueToSpell(SPELL_LURK);
        me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);

        events.Reset();
    }

    void DamageTaken(Unit* /*attacker*/, uint32& damage) override
    {
        if (me->HasAura(SPELL_LURK))
            damage = 0;
    }

    void JustEngagedWith(Unit* /*who*/) override 
    {
        events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 1 * IN_MILLISECONDS);
        events.ScheduleEvent(EVENT_LURK, 3 * IN_MILLISECONDS);
    }

    void UpdateAI(uint32 diff) override
    {
        if (!UpdateVictim())
            return;

        events.Update(diff);

        while (uint32 eventId = events.ExecuteEvent())
        {
            switch (eventId)
            {
                case EVENT_LIGHTNING_BOLT:
                {
                    DoCast(me->GetVictim(), SPELL_LIGHTNING_BOLT);
                    events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 1.6 * IN_MILLISECONDS);
                    break;
                }
                case EVENT_LURK:
                {
                    if (HasAnyPlayerLookAtMe())
                    {
                        me->InterruptNonMeleeSpells(false);
                        events.CancelEvent(EVENT_LIGHTNING_BOLT);
                        DoCast(me, SPELL_LURK);
                        DoCast(me, SPELL_FEIGN_DEATH);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    }
                    else
                    {
                        me->RemoveAurasDueToSpell(SPELL_LURK);
                        me->RemoveAurasDueToSpell(SPELL_FEIGN_DEATH);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                        events.ScheduleEvent(EVENT_LIGHTNING_BOLT, 1 * IN_MILLISECONDS);
                    }
                    events.ScheduleEvent(EVENT_LURK, 3 * IN_MILLISECONDS);
                    break;
                }
            }
        }
    }

private:
    EventMap events;
    bool HasAnyPlayerLookAtMe()
    {
        std::list<Player*> list;
        GetPlayerListInGrid(list, me, 40.0f);

        if (list.empty())
        {
            me->AI()->EnterEvadeMode();
            return false;
        }

        list.remove_if([=](Player* target) { return target->isInFront(me, 2.5f); });
        return list.empty();
    }
};

class npc_howling_gale : public CreatureScript
{
    public:
        npc_howling_gale() : CreatureScript("npc_howling_gale") { }

        struct npc_howling_galeAI : public ScriptedAI
        {
            npc_howling_galeAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                SetCombatMovement(false);
            }

            EventMap events;
            bool bCombat, HasDeactivate;

            void Reset() override
            {
                bCombat = false;
                HasDeactivate = false;
                events.Reset();
                DoCast(me, SPELL_HOWLING_GALE);
                events.ScheduleEvent(EVENT_HOWLING_GALE, 1000);
            }

            void JustReachedHome() override
            {
                bCombat = false;
                HasDeactivate = false;
            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                bCombat = false;
                HasDeactivate = false;
                _EnterEvadeMode();
                Reset();
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override
            {
                if (!HasDeactivate)
                {
                    HasDeactivate = true;
                    events.ScheduleEvent(EVENT_HOWLING_GALE_DEACTIVATE, urand(1500, 2000));
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (bCombat)
                {
                    if (!me->GetMap()->GetPlayers().isEmpty())
                    {
                        uint8 _attackersnum = 0;
                        for (Map::PlayerList::const_iterator itr = me->GetMap()->GetPlayers().begin(); itr != me->GetMap()->GetPlayers().end(); ++itr)
                            if (me->GetDistance2d(itr->GetSource()) < 30.0f)
                                _attackersnum++;

                        if (_attackersnum == 0)
                        {
                            EnterEvadeMode();
                            return;
                        }
                    }
                }

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HOWLING_GALE:
                            DoCast(me, SPELL_HOWLING_GALE_1, true);
                            DoCast(me, SPELL_HOWLING_GALE_2, true);
                            DoCast(me, SPELL_HOWLING_GALE_3, true);
                            events.ScheduleEvent(EVENT_HOWLING_GALE, 2500);
                            break;
                        case EVENT_HOWLING_GALE_DEACTIVATE:
                            bCombat = true;
                            events.CancelEvent(EVENT_HOWLING_GALE);
                            me->RemoveAurasDueToSpell(SPELL_HOWLING_GALE);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_howling_galeAI>(creature);
        }
};

class npc_minister_of_air : public CreatureScript
{
    public:
        npc_minister_of_air() : CreatureScript("npc_minister_of_air") { }

        struct npc_minister_of_airAI : public ScriptedAI
        {
            npc_minister_of_airAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_LIGHTNING_LASH, urand(4000, 8000));
                events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(7000, 10000));

            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LIGHTNING_NOVA:
                            DoCast(SPELL_LIGHTNING_NOVA);
                            events.ScheduleEvent(EVENT_LIGHTNING_NOVA, urand(10000, 20000));
                            break;
                        case EVENT_LIGHTNING_LASH:
                            DoCast(SPELL_LIGHTNING_LASH);
                            events.ScheduleEvent(EVENT_LIGHTNING_LASH, urand(8000, 15000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_minister_of_airAI>(creature);
        }
};

class npc_servant_of_asaad : public CreatureScript
{
    public:
        npc_servant_of_asaad() : CreatureScript("npc_servant_of_asaad") { }

        struct npc_servant_of_asaadAI : public ScriptedAI
        {
            npc_servant_of_asaadAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {

                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_CRUSADER_STRIKE, urand(3000, 6000));
                events.ScheduleEvent(EVENT_HAND_OF_PROTECTION, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CRUSADER_STRIKE:
                            DoCast(me->GetVictim(), SPELL_CRUSADER_STRIKE);
                            events.ScheduleEvent(EVENT_CRUSADER_STRIKE, urand(8000, 12000));
                            break;
                        case EVENT_HAND_OF_PROTECTION:
                            DoCast(me, SPELL_HAND_OF_PROTECTION);
                            events.ScheduleEvent(EVENT_HAND_OF_PROTECTION, urand(20000, 30000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_servant_of_asaadAI>(creature);
        }
};

class npc_temple_adept : public CreatureScript
{
    public:
        npc_temple_adept() : CreatureScript("npc_temple_adept") { }

        struct npc_temple_adeptAI : public ScriptedAI
        {
            npc_temple_adeptAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            Creature* _target;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_HOLY_SMITE, urand(5000, 6000));
                events.ScheduleEvent(EVENT_GREATER_HEAL, urand(5000, 6000));
                events.ScheduleEvent(EVENT_DESPERATE_SPEED, urand(10000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HOLY_SMITE:
                            DoCast(me->GetVictim(), SPELL_HOLY_SMITE);
                            events.ScheduleEvent(EVENT_HOLY_SMITE, urand(5000, 6000));
                            break;
                        case EVENT_DESPERATE_SPEED:
                            DoCast(me, SPELL_DESPERATE_SPEED);
                            events.ScheduleEvent(EVENT_DESPERATE_SPEED, urand(20000, 30000));
                            break;
                        case EVENT_GREATER_HEAL:
                            if (_target = me->FindNearestCreature(NPC_EXECUTOR_OF_THE_CALIPH, 30.0f))
                                if (_target->GetHealthPct() < 50)
                                    DoCast(_target, SPELL_GREATER_HEAL);
                            else if (_target = me->FindNearestCreature(NPC_MINISTER_OF_AIR, 30.0f))
                                if (_target->GetHealthPct() < 50)
                                    DoCast(_target, SPELL_GREATER_HEAL);
                            else if (_target = me->FindNearestCreature(NPC_SERVANT_OF_ASAAD, 30.0f))
                                if (_target->GetHealthPct() < 50)
                                    DoCast(_target, SPELL_GREATER_HEAL);
                            else if (_target = me->FindNearestCreature(NPC_TEMPEST_ADEPT, 30.0f))
                                if (_target->GetHealthPct() < 50)
                                    DoCast(_target, SPELL_GREATER_HEAL);
                            events.ScheduleEvent(EVENT_GREATER_HEAL, urand(5000, 6000));
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_temple_adeptAI>(creature);
        }
};

class npc_turbulent_squall : public CreatureScript
{
    public:
        npc_turbulent_squall() : CreatureScript("npc_turbulent_squall") { }

        struct npc_turbulent_squallAI : public ScriptedAI
        {
            npc_turbulent_squallAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_ASPHYXIATE, urand(3000, 10000));
                events.ScheduleEvent(EVENT_HURRICANE, urand(10000, 20000));
                events.ScheduleEvent(EVENT_CLOUDBURST, urand(5000, 30000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ASPHYXIATE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_ASPHYXIATE);
                            events.ScheduleEvent(EVENT_ASPHYXIATE, urand(10000, 20000));
                            break;
                        case EVENT_HURRICANE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_HURRICANE);
                            events.ScheduleEvent(EVENT_HURRICANE, urand(15000, 30000));
                            break;
                        case EVENT_CLOUDBURST:
                            DoCast(SPELL_CLOUDBURST);
                            events.ScheduleEvent(EVENT_CLOUDBURST, urand(20000, 30000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_turbulent_squallAI>(creature);
        }
};

class npc_wild_vortex : public CreatureScript
{
    public:
        npc_wild_vortex() : CreatureScript("npc_wild_vortex") { }

        struct npc_wild_vortexAI : public ScriptedAI
        {
            npc_wild_vortexAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_WIND_SHOCK, urand(5000, 10000));
                events.ScheduleEvent(EVENT_WV_LIGHTNING_BOLT, 3000);
                events.ScheduleEvent(EVENT_CYCLONE, urand(5000, 15000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CYCLONE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_CYCLONE);
                            events.ScheduleEvent(EVENT_CYCLONE, urand(10000, 20000));
                            break;
                        case EVENT_WIND_SHOCK:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_WIND_SHOCK);
                            events.ScheduleEvent(EVENT_WIND_SHOCK, urand(10000, 20000));
                            break;
                        case EVENT_WV_LIGHTNING_BOLT:
                            DoCast(me->GetVictim(), SPELL_WV_LIGHTNING_BOLT);
                            events.ScheduleEvent(EVENT_WV_LIGHTNING_BOLT, 3000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_wild_vortexAI>(creature);
        }
};

class npc_golden_orb : public CreatureScript
{
    public:
        npc_golden_orb() : CreatureScript("npc_golden_orb") { }

        struct npc_golden_orbAI : public ScriptedAI
        {
            npc_golden_orbAI(Creature* creature) : ScriptedAI(creature) { }

            void OnSpellClick(Unit* /*clicker*/, bool& result) override
            {
                if (!me->IsVisible())
                    return;

                me->GetInstanceScript()->SetData(DATA_GOLDEN_ORB, 1);
                if (me->GetInstanceScript()->GetData(DATA_GOLDEN_ORB) >= 5)
                    DoCastAOE(SPELL_ACHIEVEMENT, true);

                me->SetVisible(false);
                result = true;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_golden_orbAI>(creature);
        }
};

class npc_fall_catcher_5 : public CreatureScript
{
    public:
        npc_fall_catcher_5() : CreatureScript("npc_fall_catcher_5") { }

        struct npc_fall_catcher_5AI : public ScriptedAI
        {
            npc_fall_catcher_5AI(Creature* creature) : ScriptedAI(creature) { }

            void IsSummonedBy(Unit* summoner) override
            {
                summoner->SetDisableGravity(true, true);
                summoner->SendMovementFlagUpdate(true);

                float x, y, z, o;
                summoner->GetPosition(x, y, z, o);
                summoner->NearTeleportTo(x, y, z + 1, o);
                summoner->CastSpell(me, SPELL_CATCH_FALL_VEHICLE, true);
                me->GetPosition(x, y, z);
                me->GetMotionMaster()->MovePoint(1, x, y, z + 50);

                summoner->SetDisableGravity(false, true);
                summoner->SendMovementFlagUpdate(true);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE && pointId == 1)
                {
                    Unit* summoner = me->ToTempSummon()->GetSummoner();
                    summoner->ExitVehicle(me);
                    summoner->CastSpell(summoner, SPELL_SLIPSTREAM_BUFF, true);

                    // Have to do it manually since cell visiting is limited by grid size for some inexplicable reason
                    float x = -356.958008f;
                    float y = 26.140600f;
                    float z = 627.062012f;
                    float dist = summoner->GetExactDist(x, y, z);

                    Map::PlayerList const& players = me->GetMap()->GetPlayers();
                    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    {
                        if (Player* player = itr->GetSource())
                        {
                            if (player != summoner && !player->HasAura(SPELL_CATCH_FALL_SUMMON_AURA) && !player->HasAura(SPELL_SLIPSTREAM_BUFF))
                            {
                                float d = summoner->GetExactDist(player);
                                if (d < dist)
                                {
                                    dist = d;
                                    player->GetPosition(x, y, z);
                                }
                            }
                        }
                    }

                    summoner->RemoveAurasDueToSpell(SPELL_CATCH_FALL_SUMMON_AURA);
                    summoner->CastSpell(x, y, z, SPELL_CATCH_FALL_JUMP, true);
                }
            }

            void UpdateAI(uint32 /*diff*/) override { }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_fall_catcher_5AI>(creature);
        }
};

class spell_minister_of_air_lightning_lash : public SpellScriptLoader
{
    public:
        spell_minister_of_air_lightning_lash() : SpellScriptLoader("spell_minister_of_air_lightning_lash") { }


        class spell_minister_of_air_lightning_lash_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_minister_of_air_lightning_lash_SpellScript);

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_LIGHTNING_LASH_DMG, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_minister_of_air_lightning_lash_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_minister_of_air_lightning_lash_SpellScript();
        }
};

class spell_howling_gale_howling_gale : public SpellScriptLoader
{
    public:
        spell_howling_gale_howling_gale() : SpellScriptLoader("spell_howling_gale_howling_gale") { }
 
        class spell_howling_gale_howling_gale_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_howling_gale_howling_gale_AuraScript)

            void OnPeriodic(AuraEffect const* /*aurEff*/)
            {
                if (!GetCaster())
                    return;

                //GetCaster()->CastSpell(GetCaster(), SPELL_HOWLING_GALE_1, true);
                //GetCaster()->CastSpell(GetCaster(), SPELL_HOWLING_GALE_2, true);
                //GetCaster()->CastSpell(GetCaster(), SPELL_HOWLING_GALE_3, true);
            }
 
            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_howling_gale_howling_gale_AuraScript::OnPeriodic, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };
 
        AuraScript* GetAuraScript() const override
        {
            return new spell_howling_gale_howling_gale_AuraScript();
        }
};

// Lurk 85467
class spell_vp_lurk : public AuraScript
{
    PrepareAuraScript(spell_vp_lurk);

    void OnAuraEffectRemove(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        if (Unit* owner = GetOwner()->ToUnit())
            owner->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }

    void OnAuraEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        if (Unit* owner = GetOwner()->ToUnit())
            owner->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_PACIFIED);
    }

    void Register() override
    {
        OnEffectRemove += AuraEffectRemoveFn(spell_vp_lurk::OnAuraEffectRemove, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectApply += AuraEffectApplyFn(spell_vp_lurk::OnAuraEffectApply, EFFECT_0, SPELL_AURA_DUMMY, AURA_EFFECT_HANDLE_REAL);
    }
};

class at_catch_fall_5 : public AreaTriggerScript
{
    public:
        at_catch_fall_5() : AreaTriggerScript("at_catch_fall_5") { }

        bool OnTrigger(Player* player, const AreaTriggerEntry* /*trigger*/) override
        {
            if (player->HasAura(SPELL_CATCH_FALL_SUMMON_AURA))
                return true;

            player->CastSpell(player, SPELL_CATCH_FALL_SUMMON_TRIGGER, true);

            // Prevent Combat if victim has force down
            for (auto&& itr : bossData)
                if (Creature* encounter = ObjectAccessor::GetCreature(*player, player->GetInstanceScript() ? player->GetInstanceScript()->GetGuidData(itr) : ObjectGuid::Empty))
                    if (encounter->IsInCombat() && encounter->GetVictim() && encounter->GetVictim()->GetGUID() == player->GetGUID())
                        encounter->AI()->EnterEvadeMode();

            return true;
        }
};

void AddSC_the_vortex_pinnacle()
{
    new npc_vortex_pinnacle_slipsteam();
    new npc_skyfall_star();
    new npc_cloud_prince();
    new npc_whipping_wind();
    new npc_young_storm_dragon();
    new npc_armored_mistral();
    new npc_empyrean_assassin();
    new npc_executor_of_the_caliph();
    new npc_gust_soldier();
    new npc_howling_gale();
    new creature_script<npc_lurking_tempest>("npc_lurking_tempest");
    new npc_minister_of_air();
    new npc_servant_of_asaad();
    new npc_temple_adept();
    new npc_turbulent_squall();
    new npc_wild_vortex();
    new npc_golden_orb();
    new npc_fall_catcher_5();
    new spell_minister_of_air_lightning_lash();
    //new spell_howling_gale_howling_gale();
    new aura_script<spell_vp_lurk>("spell_vp_lurk");
    new at_catch_fall_5();
};