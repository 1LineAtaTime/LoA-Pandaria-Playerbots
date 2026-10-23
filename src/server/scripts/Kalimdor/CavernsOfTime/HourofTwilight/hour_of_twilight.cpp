/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
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
#include "ScriptMgr.h"
#include "InstanceScript.h"
#include "CreatureTextMgr.h"
#include "Vehicle.h"
#include "VehicleDefines.h"
#include "hour_of_twilight.h"
#include "Player.h"
#include "Log.h"

enum Adds
{
    NPC_TWILIGHT_DRAKE                 = 55636,
    NPC_HARBRINGER_OF_TWILIGHT         = 55969,
    NPC_CHAMPION_OF_LIFE               = 55911,
    NPC_HARBRINGER_OF_DESTRUCTION      = 55967,
    NPC_CHAMPION_OF_TIME               = 55913,
    NPC_DARK_HAZE                      = 54628, // 102287  102255
    NPC_BLOOD                          = 54644,
    NPC_THROW_LOC                      = 54735,
    NPC_SHADOW_BORER                   = 54686,
    NPC_TWILIGHT_PORTAL                = 58233,

    NPC_EARTHEN_SHELL_TARGET           = 55445,
};

enum eEvents
{
    EVENT_INTRO                        = 1,
    EVENT_ESCORT                       = 2,
    EVENT_COMBAT_ARCURION              = 3,
    EVENT_LAVA_BURST                   = 4,
    EVENT_HEALING_WAVE                 = 5,
    EVENT_CHAIN_LIGHTNING              = 6,
    EVENT_ATTACK_FROZEN_SERVITOR       = 7,
    EVENT_ARCURION                     = 8,
    EVENT_BREAK_THE_WALL               = 9,
    EVENT_ESCORT_DIFF                  = 10,
    EVENT_GARROTE                      = 11, // Assassin
    EVENT_BACKSTAB                     = 12,
    EVENT_EVISCREATE                   = 13,
    EVENT_MORTAL_STRIKE                = 14, // Bruiser
    EVENT_STAGGERING_BLOW              = 15,
    EVENT_BEATDOWN                     = 16, // Thug
    EVENT_FREEZING_TRAP                = 17, // Ranger
    EVENT_THUNGERING_SHADOWS           = 18, // Shadow walker
    EVENT_MIND_FLAY                    = 19,
    EVENT_COMBAT                       = 20,
    EVENT_RISING_FIRE_TOTEM            = 21,
    EVENT_SIT_ON_LIFE_WARDEN           = 22,
    EVENT_RESSURECTED                  = 23,
    EVENT_SQUEEZE_LIFELESS             = 24,
    EVENT_TENTACLE_SMASH               = 25,
    EVENT_SHADOW_BORE                  = 26,
    EVENT_SEEKING_SHADOW               = 27,
    EVENT_SHADOW_VALLEY                = 28,
    EVENT_WATER_SHIELD                 = 29,
    EVENT_ESCORT_TO_WALL               = 30,
    EVENT_GALAKROND_REACH              = 31,
    EVENT_CORRUPTED_SLIME              = 32, // Refs #696: Thrall 54634's slime drip during the Benedictus escort
};

enum eSpells
{
    // Thrall
    SPELL_LAVA_BURST                   = 102475,
    SPELL_HEALING_WAVE                 = 103641,
    SPELL_CHAIN_LIGHTNING              = 104130,
    SPELL_GHOST_WOLF                   = 2645,
    SPELL_RISING_FIRE_TOTEM            = 108374, // dummy
    SPELL_RISING_FIRE_TOTEM_APPLY_AURA = 103817,
    SPELL_RISING_FIRE_TOTEM_COSMETIC   = 103813,
    SPELL_RESSURECTION                 = 145728,
    SPELL_WATER_SHIELD                 = 103688,
    SPELL_MOLTEN_FURY                  = 103905,
    SPELL_THRALL_BLOODLUST             = 103834, // Thrall 55779 spell1 (creature_template); party haste +50%, 45 s, no Sated
    SPELL_THRALL_ICY_TOMB              = 103252, // boss_arcurion.cpp SPELL_ICY_TOMB (cast + AddAura on Thrall)
    SPELL_THRALL_ICY_TOMB_EFF          = 103251, // boss_arcurion.cpp SPELL_ICY_TOMB_EFF

    // Arcurion 
    SPELL_APPEARS                      = 104767, // for spawn 
    SPELL_IMPALE                       = 104019,

    // Asira
    SPELL_GARROTE                      = 102925,
    SPELL_STEALTH                      = 102921,
    SPELL_EVISCERATE                   = 102967,
    SPELL_BACKSTAB                     = 102924,

    SPELL_CLEAVE                       = 103001,
    SPELL_MORTAL_STRIKE                = 103002,
    SPELL_STAGGERING_BLOW              = 103000,

    SPELL_LEAP                         = 102975,
    SPELL_FREEZING_TRAP                = 43415,

    SPELL_BEATDOWN                     = 102989,
    SPELL_BASH                         = 102990,

    SPELL_HUNGERING_SHADOWS            = 103021,
    SPELL_MIND_FLAY                    = 103024,
    SPELL_SHADOWFORM_COSMETIC          = 107904,

    // Life Warden
    SPELL_SADDLE                       = 103646,

    // Twilight drake
    SPELL_TWILIGHT_BREATH              = 104230,

    // Benedictus
    SPELL_SQUEEZE_LIFELESS             = 102861,
    SPELL_TENTACLE_SMASH               = 102848,

    SPELL_SHADOW_BORE                  = 102995,

    SPELL_SEEKING_SHADOWS              = 102984,
    SPELL_SHADOW_VOLLEY                = 102992,

    // Custom
    SPELL_TWILIGHT_PUDDLE_COSMETIC     = 102254,
    SPELL_GROW_VISUAL_COSMETIC         = 102255,

    // Teleports
    SPELL_TELEPORT_ARCURION_DEAD       = 108928,
    SPELL_TELEPORT_ASIRA_DEAD          = 108929,
    SPELL_TELEPORT_BENEDICTUS          = 108930,
};

enum eActions
{
    ACTION_ACTIVATE                    = 1,
    ACTION_SPAWN                       = 2,
    ACTION_SPAWN_PH2                   = 3,
    ACTION_SPAWN_PH3                   = 4,
    ACTION_MOVEMENT_RANDOM             = 5,
    ACTION_MOLTEN_FURY                 = 6,
};

enum eYells
{
    // Thrall intro
    TALK_INTRO_1                       = 0,
    TALK_INTRO_2                       = 1,
    TALK_INTRO_3                       = 2,
    TALK_INTRO_4                       = 3,
    TALK_INTRO_5                       = 4,

    // Thrall Arcurion
    TALK_ARCURION_1                    = 0,
    TALK_ARCURION_2                    = 1,
    TALK_ARCURION_3                    = 2,
    TALK_ARCURION_4                    = 3,
    TALK_ARCURION_5                    = 4,

    // Thrall Asira
    TALK_ASIRA_1                       = 0,
    TALK_ASIRA_2                       = 1,
    TALK_ASIRA_3                       = 2,
    TALK_ASIRA_4                       = 3,
    TALK_ASIRA_5                       = 4,
    TALK_ASIRA_6                       = 5,
    TALK_ASIRA_7                       = 6,
    TALK_ASIRA_8                       = 7,
    TALK_ASIRA_9                       = 8,

    // Thrall Benedictus
    TALK_BENEDICTUS_1                  = 0,
    TALK_BENEDICTUS_2                  = 1,

    // Twilight drake
    TALK_TWILIGHT_DRAKE                = 0
};

std::list <Creature*> ActivateNearestStalkers(Creature* me, ObjectGuid ownerGUID, uint32 npc_entry)
{
    std::list<Creature*> StalkersInArea;

    Unit* owner = ObjectAccessor::GetUnit(*me, ownerGUID);

    if (!owner)
        return StalkersInArea;

    GetCreatureListWithEntryInGrid(StalkersInArea, owner, npc_entry, 47.0f);

    return StalkersInArea;
}

Creature* GetFrozenServitorTarget(Creature* me, ObjectGuid ownerGUID)
{
    std::list <Creature*> FrozenServitorsInArea;
    Unit* owner = ObjectAccessor::GetUnit(*me, ownerGUID);

    if (!owner)
        return (Creature*)NULL;

    GetCreatureListWithEntryInGrid(FrozenServitorsInArea, owner, NPC_FROZEN_SERVITOR, 200.0f);

    if (FrozenServitorsInArea.empty())
        return (Creature*)NULL;

    return Trinity::Containers::SelectRandomContainerElement(FrozenServitorsInArea);
}

uint32 GetCurrentPlayersCount(Creature* me, ObjectGuid ownerGUID)
{
    std::list <Player*> PlayersInArea;
    Unit* owner = ObjectAccessor::GetUnit(*me, ownerGUID);

    if (!owner)
        return 0;

    GetPlayerListInGrid(PlayersInArea, owner, 200.0f);
    return PlayersInArea.size();
}

const Position GenerateMovement(Position pos)
{
    return { pos.GetPositionX() + frand(-2.5f, 2.5f), pos.GetPositionY() + frand(-3.0f, 3.0f), pos.GetPositionZ(), pos.GetOrientation() };
}

// Refs #673 (w68): the escort Thralls only advance through MovementInform(POINT_MOTION_TYPE) -> EVENT_ESCORT. An ambush
// that engages Thrall while he walks to a waypoint replaces the point generator with MoveChase (both MOTION_SLOT_ACTIVE,
// MotionMaster::MoveChase/MovePoint), and PointMovementGenerator::DoFinalize never informs, so no MovementInform comes.
// When the last ambusher dies, Thrall's combat just ends (Unit::setDeathState -> CombatStop, no evade, no Reset()), and no
// EVENT_ESCORT is left scheduled: he stands there for good (w62-w64, w68: Thrall 54548 at the canyon-entrance ambush for
// the whole 600 s cap; w65/w67 worked when the ambush ended through an evade, whose Reset() reschedules the escort).
// True when an escort that has started and is not finished has nothing driving it: out of combat, not evading (UpdateAI
// does not run then anyway), not walking to a point, and no EVENT_ESCORT pending.
static bool HotEscortNeedsResume(Creature* me, EventMap const& nonCombatEvents, uint32 eventId)
{
    return !me->IsInCombat() && !me->IsInEvadeMode() &&
        me->GetMotionMaster()->GetCurrentMovementGeneratorType() != POINT_MOTION_TYPE &&
        !nonCombatEvents.IsScheduled(eventId);
}

// Refs #673: escort state lines for the stage drivers (Server.log; the playerbots logger is the one the test conf enables).
static void HotEscortLog(Creature* me, char const* what, uint32 wp)
{
    Unit* victim = me->GetVictim();
    TC_LOG_DEBUG("playerbots", "[HOTESCORT] %u %s wp=%u pos=(%.1f, %.1f, %.1f) combat=%u evade=%u victim=%u mgen=%u",
        me->GetEntry(), what, wp, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), uint32(me->IsInCombat()),
        uint32(me->IsInEvadeMode()), victim ? victim->GetEntry() : 0u, uint32(me->GetMotionMaster()->GetCurrentMovementGeneratorType()));
}

// Life warden 55549
class npc_hour_of_twilight_life_warden : public CreatureScript
{
    public:
        npc_hour_of_twilight_life_warden() : CreatureScript("npc_hour_of_twilight_life_warden") { }

        struct npc_hour_of_twilight_life_wardenAI : public ScriptedAI
        {
            npc_hour_of_twilight_life_wardenAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap nonCombatEvents;
            uint32 wp;
            bool SetVehicle;

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                me->SetCanFly(true);
                me->SetSpeed(MOVE_FLIGHT, 2.5f);
                me->SetSpeed(MOVE_RUN, 2.5f);
                me->UpdateSpeed(MOVE_RUN, true);

                Movement::MoveSplineInit init(me);
                for (auto itr : TaxiPathPrepare)
                    init.Path().push_back(G3D::Vector3(itr.GetPositionX(), itr.GetPositionY(), itr.GetPositionZ()));

                init.SetSmooth();
                init.SetUncompressed();
                init.SetFly();
                init.Launch();
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, me->GetSplineDuration());
            }

            void Reset() override { }

            void InitializeAI() override
            {
                me->SetReactState(REACT_PASSIVE);
                wp = 0;
                SetVehicle = false;
                me->SetCanFly(true);
            }

            void MovementInform(uint32 type, uint32 pointId) override 
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (!SetVehicle)
                    return;

                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));
                wp++;

                switch (pointId)
                {
                    case 9:
                        me->SummonCreature(NPC_TWILIGHT_DRAKE, GenerateMovement(TwilightDrakePos), TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    case 11:
                        nonCombatEvents.Reset();
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->Clear();
                        break;
                    default:
                        break;
                }
            }

            void DoAction(int32 action) override 
            {
                if (action == ACTION_REMOVE_ALL_PASSENGERS)
                {
                    if (Vehicle* Passengers = me->GetVehicleKit())
                        Passengers->RemoveAllPassengers(/*true*/);

                    me->SetCanFly(false);
                    me->DealDamage(me, me->GetHealth(), NULL, DIRECT_DAMAGE, SPELL_SCHOOL_MASK_NORMAL, NULL, false);
                }
            }

            void OnSpellClick(Unit* clicker, bool& /*result*/) override
            {
                clicker->ToPlayer()->EnterVehicle(me, 0);
                me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1500);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override { }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESCORT:
                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MovePoint(wp + 6, GenerateMovement(TaxiPath[wp]));
                            break;
                        case EVENT_ESCORT_DIFF:
                            nonCombatEvents.Reset();
                            wp = 0;
                            SetVehicle = true;
                            me->SetFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_SPELLCLICK);
                            break;
                    }
                }

                if (!UpdateVictim())
                    return;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_hour_of_twilight_life_wardenAI(creature);
        }
};

// Life warden Thrall | Asira 55415
class npc_hour_of_twilight_life_warden_thrall : public CreatureScript
{
    public:
        npc_hour_of_twilight_life_warden_thrall() : CreatureScript("npc_hour_of_twilight_life_warden_thrall") { }

        struct npc_hour_of_twilight_life_warden_thrallAI : public ScriptedAI
        {
            npc_hour_of_twilight_life_warden_thrallAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap nonCombatEvents;
            InstanceScript* instance;
            uint32 wp;

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                me->AddAura(SPELL_SADDLE, me);
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
            }

            void InitializeAI() override
            {
                instance = me->GetInstanceScript();
                me->SetReactState(REACT_PASSIVE);
                wp = 0;
                me->SetCanFly(true);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                wp++;
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));

                switch (pointId)
                {
                    case 2:
                        if (Creature* Thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_3, 150.0f))
                            Thrall->AI()->DoAction(ACTION_SUMMONED_ASIRA);
                        break;
                    case 4:
                        me->SummonCreature(NPC_ASIRA, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    case 5:
                        nonCombatEvents.Reset();
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->Clear();

                        if (instance)
                            if (Creature* Asira = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ASIRA)))
                                Asira->AI()->DoAction(ACTION_ASIRA_FALL);

                            me->NearTeleportTo(me->GetPositionX(), me->GetPositionY(), -6.48f, me->GetOrientation());
                            me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                            break;
                    case 6:
                        me->SummonCreature(NPC_THRALL_4, ThrallLastSpawnPoint, TEMPSUMMON_MANUAL_DESPAWN);

                        if (Creature* Thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_3, 10.0f))
                            Thrall->DespawnOrUnsummon();

                        me->DespawnOrUnsummon();
                        break;
                }
            }

            void JustSummoned(Creature* summon) override
            {
                summon->EnterVehicle(me, 0);
            }

            void DoAction(int32 action) override 
            {
                if (action == ACTION_FLY_TO_DRAGON_SOUL)
                {
                    // Refs #696: active for the flight (see npc_thrall_asira ACTION_ASIRA_DEATH); point 6 summons Thrall 54634 and
                    // despawns the warden, which drops the flag.
                    me->setActive(true, ActiveFlags::Script);
                    me->SetCanFly(true);
                    me->GetMotionMaster()->MovementExpired();
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(6, AsiraSpawnPoint);
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                damage = 0;
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    if (eventId == EVENT_ESCORT)
                    {
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MovePoint(wp, GenerateMovement(AsiraTaxiPath[wp]));
                    }
                    break;
                }

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_hour_of_twilight_life_warden_thrallAI(creature);
        }
};

// Twilight drake 55636
class npc_hour_of_twilight_twilight_drake : public CreatureScript
{
    public:
        npc_hour_of_twilight_twilight_drake() : CreatureScript("npc_hour_of_twilight_twilight_drake") { }

        struct npc_hour_of_twilight_twilight_drakeAI : public ScriptedAI
        {
            npc_hour_of_twilight_twilight_drakeAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap nonCombatEvents;
            uint32 wp;
            Creature* LifeWarden;

            void IsSummonedBy(Unit* summoner) override 
            { 
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1500); 
                LifeWarden = summoner->ToCreature();
                me->SetSpeed(MOVE_FLIGHT, 2.0f);
            }

            void Reset() override { }

            void InitializeAI() override
            {
                me->SetReactState(REACT_PASSIVE);
                wp = 0;
                me->SetCanFly(true);
                me->SetDisableGravity(true);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                wp++;
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));

                if (pointId == 2)
                {
                    nonCombatEvents.Reset();
                    me->GetMotionMaster()->Clear();

                    if (LifeWarden)
                    {
                        me->CastSpell(LifeWarden, SPELL_TWILIGHT_BREATH, false);
                        LifeWarden->AI()->DoAction(ACTION_REMOVE_ALL_PASSENGERS);
                    }

                    Talk(TALK_TWILIGHT_DRAKE);
                    nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 2500);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESCORT:
                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MovePoint(wp, GenerateMovement(TwilightDrakeTaxi[wp]));
                            break;
                        case EVENT_ESCORT_DIFF:
                            me->DespawnOrUnsummon();
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_hour_of_twilight_twilight_drakeAI(creature);
        }
};

// Thrall Intro 54548
class npc_thrall_intro : public CreatureScript
{
    public:
        npc_thrall_intro() : CreatureScript("npc_thrall_intro") { }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                creature->AI()->DoAction(ACTION_ACTIVATE);

            player->CLOSE_GOSSIP_MENU();

            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, THRALL_INTRO, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(18287, creature->GetGUID());
            return true;
        }

        struct npc_thrall_introAI : public ScriptedAI
        {
            npc_thrall_introAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

            InstanceScript* instance;
            EventMap events, nonCombatEvents;
            uint32 timer;
            uint32 wp;
            bool escortStarted = false; // Refs #673: the first EVENT_ESCORT move ran (see HotEscortNeedsResume)
            bool escortHeld = false;    // Refs #673: an in-combat hold is being logged

            void Reset() override
            {
                events.Reset();
                nonCombatEvents.Reset();
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1500);
            }

            void DoAction(int32 action) override 
            {
                switch (action)
                {
                    case ACTION_ACTIVATE:
                        Talk(TALK_INTRO_1);
                        SetEquipmentSlots(false, DoomHammer, 0, EQUIP_NO_CHANGE);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        nonCombatEvents.ScheduleEvent(EVENT_INTRO, 6500);
                        break;
                    default:
                        break;
                }
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_LAVA_BURST, 1500);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
            }

            void InitializeAI() override
            {
                wp = 0;
                escortStarted = false;
                escortHeld = false;
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                HotEscortLog(me, "reached", pointId);
                wp++;
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));
                // Refs #673: an evade after an ambush walks Thrall home, and CreatureAI is not updated while evading
                // (Creature::Update skips UpdateAI in evade mode), so with the DB spawn as home every later ambush
                // cost a run back to the canyon entrance first. Home is the last waypoint reached instead.
                me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());

                switch (pointId)
                {
                    case 2:
                        Talk(TALK_INTRO_3);
                        Talk(TALK_INTRO_4);

                        for (auto itr : ActivateNearestStalkers(me, me->GetGUID(), NPC_FROZEN_SERVITOR_SPAWN_STALKER_2))
                            itr->AI()->DoAction(ACTION_SPAWN);

                        for (auto itr : ActivateNearestStalkers(me, me->GetGUID(), NPC_FROZEN_SERVITOR_SPAWN_STALKER))
                            itr->AI()->DoAction(ACTION_SPAWN);
                        break;
                    case 4:
                        me->GetMotionMaster()->Clear();
                        nonCombatEvents.Reset();
                        Talk(TALK_INTRO_5);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
                        me->SummonCreature(NPC_THRALL_2, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        break;
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override { }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_INTRO:
                        {
                            for (auto itr : ActivateNearestStalkers(me, me->GetGUID(), NPC_FROZEN_SERVITOR_STALKER_ENTRANCE))
                                itr->AI()->DoAction(ACTION_SPAWN);
                            me->SetFacingTo(me->GetOrientation() + M_PI);
                            Talk(TALK_INTRO_2);
                            nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 2500);
                            break;
                        }
                        case EVENT_ESCORT:
                        {
                            // Refs #673: hold while an ambush is up, but keep the escort event alive. The old `return`
                            // consumed EVENT_ESCORT; when the last ambusher then died its death ended Thrall's combat
                            // (Unit::setDeathState -> CombatStop -> CombatReference::EndCombat) without an evade
                            // (UnitAI::JustExitedCombat is empty, CreatureAI::UpdateVictim returns early out of combat),
                            // so Reset() never rescheduled the escort and Thrall stood at the ambush for good (w64:
                            // stuck at wp 2 from 610 s to the 1240 s cap after the bots killed the wp 2 ambush).
                            if (me->IsInCombat())
                            {
                                if (!escortHeld)
                                    HotEscortLog(me, "hold (in combat)", wp);
                                escortHeld = true;
                                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                                break;
                            }
                            escortHeld = false;
                            escortStarted = true;
                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();
                            if (me->GetUnitMovementFlags() == MOVEMENTFLAG_WALKING)
                                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
                            me->GetMotionMaster()->MovePoint(wp, ThrallWaypoints_Arcurion[wp]);
                            HotEscortLog(me, "move", wp);
                        }
                            break;
                        default:
                            break;
                    }
                }

                // Refs #673 (w68): an ambush that interrupted the walk to a waypoint and ended without an evade left no
                // EVENT_ESCORT behind (HotEscortNeedsResume). Pick the escort up again at the same waypoint.
                if (escortStarted && wp < MAX_POINT_ACURION && HotEscortNeedsResume(me, nonCombatEvents, EVENT_ESCORT))
                {
                    HotEscortLog(me, "resume (walk interrupted, combat over)", wp);
                    nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LAVA_BURST:
                            if (Unit* itr = me->GetVictim())
                                 me->CastSpell(itr, SPELL_LAVA_BURST, false);  
                            events.ScheduleEvent(EVENT_LAVA_BURST, 10000);
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* itr = me->GetVictim())
                                  me->CastSpell(itr, SPELL_CHAIN_LIGHTNING, false);
                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                            break;
                        case EVENT_HEALING_WAVE:
                            if (HealthBelowPct(50))
                                me->CastSpell(me, SPELL_HEALING_WAVE, false);
                            events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();

            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thrall_introAI(creature);
        }
};

// Thrall Arcurion 55779
class npc_thrall_arcurion : public CreatureScript
{
    public:
        npc_thrall_arcurion() : CreatureScript("npc_thrall_arcurion") { }

        bool activated = false;

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                creature->AI()->DoAction(ACTION_ACTIVATE);

            player->CLOSE_GOSSIP_MENU();

            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, THRALL_ARCURION, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(18555, creature->GetGUID());
            return true;
        }

        struct npc_thrall_arcurionAI : public ScriptedAI
        {
            npc_thrall_arcurionAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()), summons(me) { }

            InstanceScript* instance;
            EventMap events, nonCombatEvents;
            SummonList summons;
            uint32 timer;
            uint32 wp;
            bool phase;
            bool wallEvent = false;
            bool escortToArcurion = false; // Refs #673: walking to ThrallWaypoints_ArcurionPrepare (point 20), not yet there

            void InitializeAI() override
            {
                SetEquipmentSlots(true, DoomHammer, 0, EQUIP_NO_CHANGE);
                wp           = 0;
                phase        = false;
                wallEvent    = false;
                escortToArcurion = false;
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                DoZoneInCombat();
                me->HandleEmoteStateCommand(EMOTE_STATE_READY1H);
            }

            void Reset() override
            {
                if (instance && instance->GetBossState(DATA_ARCURION) == DONE)
                    StartWallEvent();
            }

            // Refs #673 (w67: Arcurion killed through the real escort, encounter DONE, Thrall said TALK_ARCURION_4, but no
            // Thrall 54972 within 240 s). The wall break + ghost-wolf ride to Galakrond's Rest (~640 yd) is a raw spline and
            // nonCombatEvents timers, both advanced only by Creature::Update. Map::Update (Map.cpp:777-806) updates only the
            // cells within the grid activation range (170 yd in an instance) of a player or of an active object, so a Thrall
            // who rides out of every player's range freezes mid-route and never reaches EVENT_GALAKROND_REACH (the group was
            // still at Arcurion, then teleported ~350 yd past him). Wowpedia "Hour of Twilight" (MediaWiki API, 2026-10-15):
            // "When Arcurion falls, the party heads to Galakrond's Rest" - Thrall goes ahead and waits there, so he is made
            // an active object for the sequence; he despawns at the end (EVENT_GALAKROND_REACH), which drops the flag.
            // Run once: a later evade must not restart it (see EnterEvadeMode).
            void StartWallEvent()
            {
                if (wallEvent)
                    return;

                wallEvent = true;
                me->setActive(true, ActiveFlags::Script);
                MovementFlagWithoutWalking();
                me->GetMotionMaster()->MovePoint(21, BreakTheWall);
                nonCombatEvents.ScheduleEvent(EVENT_ESCORT_TO_WALL, me->GetSplineDuration() + 500);
            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                // Refs #673: once the wall/ride sequence runs, an evade (a straggler dropping Thrall's threat) only leaves
                // combat. The default path (MoveTargetedHome + Reset) would replace the ride spline, and
                // EVENT_GALAKROND_REACH would then summon Thrall 54972 wherever he stood.
                if (wallEvent)
                {
                    _EnterEvadeMode(why);
                    return;
                }

                ScriptedAI::EnterEvadeMode(why);
            }

            void MovementFlagWithoutWalking()
            {
                me->GetMotionMaster()->MovementExpired();
                me->GetMotionMaster()->Clear();

                if (me->GetUnitMovementFlags() == MOVEMENTFLAG_WALKING)
                    me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);

                if (summon->GetEntry() == NPC_RISING_FIRE_TOTEM)
                    summon->CastSpell(summon, SPELL_RISING_FIRE_TOTEM_COSMETIC, false);
            }

            void DoAction(int32 action) override 
            {
                switch (action)
                {
                    case ACTION_ACTIVATE:
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                        nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                        break;
                    case ACTION_ARCURION:
                        Talk(TALK_ARCURION_2);
                        nonCombatEvents.ScheduleEvent(EVENT_ARCURION, 1000);
                        break;
                    case ACTION_ICY_TOMB:
                        me->AddUnitState(UNIT_STATE_STUNNED);
                        events.Reset();
                        break;
                    case ACTION_ICY_TOMB_REMOVE:
                        me->ClearUnitState(UNIT_STATE_STUNNED);
                        events.ScheduleEvent(EVENT_ATTACK_FROZEN_SERVITOR, 1500);
                        break;
                    case ACTION_TALK_FINISH_HIM:
                    {
                        // Refs #696 (w61: the group took Arcurion to 1% and wiped in the Torrent of Frost burn).
                        // Wowpedia "Arcurion" (MediaWiki API, 2026-10-14): reaching 30% "will automatically release Thrall
                        // from the ice tomb", and "at this time Thrall will cast Bloodlust (which does not trigger the sated
                        // debuff ...) making it easier to finish the boss". The tomb is Thrall's summon (boss_arcurion.cpp:223);
                        // despawning it skips npc_icy_tomb::JustDied, so the stun is cleared here.
                        summons.DespawnEntry(NPC_ICY_TOMB);
                        me->RemoveAurasDueToSpell(SPELL_THRALL_ICY_TOMB);
                        me->RemoveAurasDueToSpell(SPELL_THRALL_ICY_TOMB_EFF);
                        me->ClearUnitState(UNIT_STATE_STUNNED);
                        events.CancelEvent(EVENT_ATTACK_FROZEN_SERVITOR);
                        events.ScheduleEvent(EVENT_LAVA_BURST, 2000);
                        Talk(TALK_ARCURION_3);
                        // 103834 targets SRC_AREA_ALLY (SpellEffect.dbc), so the aura is put on each player in range directly.
                        Map::PlayerList const& players = me->GetMap()->GetPlayers();
                        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                            if (Player* player = itr->GetSource())
                                if (player->IsAlive() && player->IsWithinDistInMap(me, 100.0f))
                                    me->AddAura(SPELL_THRALL_BLOODLUST, player);
                        break;
                    }
                    case ACTION_ARCURION_WIPE:
                        // Refs #824 (w61: the last player died, Arcurion evaded, and in the same ms re-engaged with his aggro
                        // line while Thrall, passive and possibly entombed, stayed in combat). boss_arcurion::EnterEvadeMode
                        // sends this: Thrall drops out of his Icy Tomb and out of combat and walks back to his spot (home =
                        // point 20, MovementInform). He stays passive; the next pull re-runs ACTION_ARCURION.
                        if (wallEvent)
                            break;
                        summons.DespawnEntry(NPC_ICY_TOMB);
                        me->RemoveAurasDueToSpell(SPELL_THRALL_ICY_TOMB);
                        me->RemoveAurasDueToSpell(SPELL_THRALL_ICY_TOMB_EFF);
                        me->ClearUnitState(UNIT_STATE_STUNNED);
                        events.Reset();
                        nonCombatEvents.CancelEvent(EVENT_ARCURION);
                        if (!me->IsInEvadeMode())
                            EnterEvadeMode();
                        break;
                    case ACTION_TALK_ARCURION_DEATH:
                        summons.DespawnEntry(NPC_ICY_TOMB);
                        events.Reset();
                        me->GetThreatManager().RemoveMeFromThreatLists();
                        me->GetThreatManager().ClearAllThreat();

                        if (instance)
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                        Talk(TALK_ARCURION_4);
                        // Refs #673: the wall break and the ride to Asira start from Reset() (boss state DONE, set by
                        // boss_arcurion::JustDied -> _JustDied before this action). Reset() only ran through the evade that
                        // UpdateVictim triggers for a passive Thrall in combat; a Thrall that was never pulled into the
                        // fight (Arcurion > 100 yd away at the pull, SendThrallInCombat) never evaded, so no Thrall 54972
                        // was summoned (w62: Arcurion killed, encounter DONE, "asira thrall-3 NOT FOUND after 240s").
                        if (!me->IsInEvadeMode())
                            EnterEvadeMode();
                        // A Thrall already evading skipped the evade above (and so Reset()); start the sequence directly.
                        // No-op when Reset() already started it.
                        StartWallEvent();
                        break;
                    default:
                        break;
                }
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                if (pointId == 20)
                {
                    escortToArcurion = false;
                    HotEscortLog(me, "reached", pointId);
                    me->GetMotionMaster()->Clear();
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                    nonCombatEvents.Reset();
                    Talk(TALK_ARCURION_1);
                    if (Creature* ArcurionStalker = GetClosestCreatureWithEntry(me, NPC_ARCURION_SPAWN_VISIAL_STALKER, 100.0f, true))
                    {
                        ArcurionStalker->CastSpell(ArcurionStalker, SPELL_APPEARS, false);
                        ArcurionStalker->SummonCreature(NPC_ARCURION, ArcurionStalker->GetPositionX(), ArcurionStalker->GetPositionY(), ArcurionStalker->GetPositionZ(), ArcurionStalker->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                    }
                    me->HandleEmoteStateCommand(EMOTE_STATE_READY1H);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESCORT:
                            escortToArcurion = true;
                            MovementFlagWithoutWalking();
                            me->GetMotionMaster()->MovePoint(20, ThrallWaypoints_ArcurionPrepare);
                            HotEscortLog(me, "move", 20);
                            break;
                        case EVENT_ESCORT_DIFF:
                        {
                            DoCast(me, SPELL_GHOST_WOLF);

                            Movement::MoveSplineInit init(me);
                            for (auto itr : ThrallWaypoints_Asira)
                                init.Path().push_back(G3D::Vector3(itr.GetPositionX(), itr.GetPositionY(), itr.GetPositionZ()));

                            init.SetSmooth();
                            init.SetUncompressed();
                            init.Launch();
                            nonCombatEvents.ScheduleEvent(EVENT_GALAKROND_REACH, me->GetSplineDuration());
                            break;
                        }
                        case EVENT_ARCURION:
                            me->SetReactState(REACT_PASSIVE);
                            DoZoneInCombat();
                            events.ScheduleEvent(EVENT_ATTACK_FROZEN_SERVITOR, 1500);
                            break;
                        case EVENT_ESCORT_TO_WALL:
                            me->GetMotionMaster()->Clear();
                            DoCast(me, SPELL_MOLTEN_FURY);
                            phase = true;
                            wp = 0;
                            Talk(TALK_ARCURION_5);
                            nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 11000);
                            break;
                        case EVENT_GALAKROND_REACH:
                            me->SummonCreature(NPC_THRALL_3, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                            me->DespawnOrUnsummon();
                            break;
                    }
                }

                // Refs #673 (w68): same stall as npc_thrall_intro - a fight (or an evade home) that interrupted the walk to
                // point 20 leaves nothing to restart it, and Arcurion is only summoned at point 20.
                if (escortToArcurion && !wallEvent && HotEscortNeedsResume(me, nonCombatEvents, EVENT_ESCORT))
                {
                    HotEscortLog(me, "resume (walk interrupted, combat over)", 20);
                    nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LAVA_BURST:
                            if (Creature* Arc = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_ARCURION)))
                                me->CastSpell(Arc, SPELL_LAVA_BURST, false);

                            events.ScheduleEvent(EVENT_LAVA_BURST, 6000);
                            break;
                        case EVENT_ATTACK_FROZEN_SERVITOR:
                            if (Creature* FrozenServitor = GetFrozenServitorTarget(me, me->GetGUID()))
                                me->CastSpell(FrozenServitor, SPELL_LAVA_BURST, false);

                            events.ScheduleEvent(EVENT_ATTACK_FROZEN_SERVITOR, 5000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thrall_arcurionAI(creature);
        }
};

// Thrall Asira 54972
class npc_thrall_asira : public CreatureScript
{
    public:
        npc_thrall_asira() : CreatureScript("npc_thrall_asira") { }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                creature->AI()->DoAction(ACTION_ACTIVATE);

            player->CLOSE_GOSSIP_MENU();

            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, THRALL_ASIRA, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(18287, creature->GetGUID());
            return true;
        }

        struct npc_thrall_asiraAI : public ScriptedAI
        {
            npc_thrall_asiraAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) { }

            EventMap events, nonCombatEvents;
            uint32 timer;
            uint32 wp;
            bool escortStarted = false; // Refs #673: see npc_thrall_intro
            bool escortHeld = false;

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                me->HandleEmoteStateCommand(EMOTE_STATE_CANNIBALIZE);
            }

            void Reset() override
            {
                events.Reset();
                nonCombatEvents.Reset();
                instance->GetBossState(DATA_ASIRA) == DONE ? nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 1500) : nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1500);
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_ACTIVATE:
                        SetEquipmentSlots(false, DoomHammer, 0, EQUIP_NO_CHANGE);
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        me->HandleEmoteStateCommand(EMOTE_STATE_NONE);

                        if (instance)
                            instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                        nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                        break;
                    case ACTION_MOVEMENT_RANDOM:
                        Talk(TALK_ASIRA_5);
                        me->GetMotionMaster()->MoveRandom(4.0f);
                        me->SummonCreature(NPC_LIFE_WARDEN_1, AsiraSpawnPoint, TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    case ACTION_SUMMONED_ASIRA:
                        me->GetMotionMaster()->MoveIdle();
                        me->GetMotionMaster()->Clear();

                        if (Creature* LifeWarden = GetClosestCreatureWithEntry(me, NPC_LIFE_WARDEN_1, 150.0f))
                            me->SetFacingTo(me->GetAngle(LifeWarden));

                        Talk(TALK_ASIRA_6);
                        me->HandleEmoteStateCommand(EMOTE_STATE_READY1H);
                        break;
                    case ACTION_ASIRA_DEATH:
                        // Refs #696 (boss_asira_dawnslayer::JustDied): leave the fight; Reset() (DATA_ASIRA DONE) walks him to the
                        // Life Warden (EVENT_ESCORT_DIFF -> point 11 -> resurrection -> ride -> Thrall 54634 at ThrallLastSpawnPoint).
                        // Active for the sequence, like Thrall 55779's ride (#673 w67): the group may leave Galakrond's Rest before
                        // the Life Warden lands, and an object out of every player's grid range is not updated. He despawns at the end.
                        me->setActive(true, ActiveFlags::Script);
                        if (!me->IsInEvadeMode())
                            EnterEvadeMode();
                        if (!nonCombatEvents.IsScheduled(EVENT_ESCORT_DIFF))
                            nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 1500);
                        break;
                    default:
                        break;
                }
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_LAVA_BURST, 1500);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
                events.ScheduleEvent(EVENT_RISING_FIRE_TOTEM, 5500);
            }

            void InitializeAI() override
            {
                wp = 0;
                escortStarted = false;
                escortHeld = false;
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                HotEscortLog(me, "reached", pointId);
                if (instance->GetBossState(DATA_ASIRA) != DONE)
                {
                    wp++;
                    nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));
                    // Refs #673: evade home = the last waypoint reached (see npc_thrall_intro::MovementInform).
                    me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                }

                switch (pointId)
                {
                    case 3:
                        Talk(TALK_ASIRA_1);
                        if (Creature* stalker = GetClosestCreatureWithEntry(me, NPC_ASIRA_TRASH_STALKER_PH1, 50.0f))
                            stalker->AI()->DoAction(ACTION_SPAWN);
                        break;
                    case 4:
                        Talk(TALK_ASIRA_2);
                        break;
                    case 7:
                        Talk(TALK_ASIRA_3);
                        if (Creature* stalker = GetClosestCreatureWithEntry(me, NPC_ASIRA_TRASH_STALKER_PH1, 50.0f))
                            stalker->AI()->DoAction(ACTION_SPAWN_PH2);
                        break;
                    case 9:
                        Talk(TALK_ASIRA_4);
                        if (Creature* stalker = GetClosestCreatureWithEntry(me, NPC_ASIRA_TRASH_STALKER_PH1, 70.0f))
                            stalker->AI()->DoAction(ACTION_SPAWN_PH3);
                        break;
                    case 10:
                        me->GetMotionMaster()->Clear();
                        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        nonCombatEvents.Reset();
                        DoAction(ACTION_MOVEMENT_RANDOM);
                        break;
                    case 11:
                        if (Creature* LifeWarden = GetClosestCreatureWithEntry(me, NPC_LIFE_WARDEN_1, 10.0f))
                            me->CastSpell(LifeWarden, SPELL_RESSURECTION, false);

                        nonCombatEvents.ScheduleEvent(EVENT_RESSURECTED, 10000);
                        break;
                    default:
                        break;
                }
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESCORT:
                            // Refs #673: same hold as npc_thrall_intro (keep the event alive while the trash waves are up).
                            if (me->IsInCombat())
                            {
                                if (!escortHeld)
                                    HotEscortLog(me, "hold (in combat)", wp);
                                escortHeld = true;
                                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                                break;
                            }
                            escortHeld = false;
                            escortStarted = true;
                            // Moving Without Walk...
                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();

                            if (me->GetUnitMovementFlags() == MOVEMENTFLAG_WALKING)
                                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

                            me->GetMotionMaster()->MovePoint(wp, ThrallWaypoints_Asira_Prepare[wp]);
                            HotEscortLog(me, "move", wp);
                            break;
                        case EVENT_ESCORT_DIFF:
                            if (Creature* LifeWarden = GetClosestCreatureWithEntry(me, NPC_LIFE_WARDEN_1, 150.0f))
                            {
                                Position pos = { LifeWarden->GetPositionX(), LifeWarden->GetPositionY(), LifeWarden->GetPositionZ(), LifeWarden->GetOrientation() };
                                me->GetMotionMaster()->MovePoint(11, GenerateMovement(pos));
                            }
                            break;
                        case EVENT_RESSURECTED:
                            if (Creature* LifeWarden = GetClosestCreatureWithEntry(me, NPC_LIFE_WARDEN_1, 20.0f))
                            {
                                LifeWarden->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                                LifeWarden->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                                LifeWarden->SetStandState(UNIT_STAND_STATE_STAND);

                                if (!me->IsOnVehicle())
                                    me->EnterVehicle(LifeWarden, 0);

                                me->HandleEmoteStateCommand(EMOTE_STATE_MOUNT);
                            }
                            nonCombatEvents.ScheduleEvent(EVENT_SIT_ON_LIFE_WARDEN, 1500);
                            break;
                        case EVENT_SIT_ON_LIFE_WARDEN:
                            Talk(TALK_ASIRA_9);
                            if (Creature* LifeWarden = GetClosestCreatureWithEntry(me, NPC_LIFE_WARDEN_1, 20.0f))
                                LifeWarden->AI()->DoAction(ACTION_FLY_TO_DRAGON_SOUL);

                            for (uint32 i = 0; i < GetCurrentPlayersCount(me, me->GetGUID()); i++)
                                me->SummonCreature(NPC_LIFE_WARDEN_2, GenerateMovement(drakePos), TEMPSUMMON_MANUAL_DESPAWN);
                            break;
                        default:
                            break;
                    }
                }

                // Refs #673 (w68): the same interrupted-walk stall as npc_thrall_intro, on the trash-wave route to Galakrond's
                // Rest. Points 0-10 only: point 10 starts the random walk + Life Warden (nonCombatEvents.Reset there), and
                // after Asira (DONE) the route is EVENT_ESCORT_DIFF / point 11.
                if (escortStarted && wp < ASIRA_PREPARE_MAX_POINTS && instance && instance->GetBossState(DATA_ASIRA) != DONE &&
                    HotEscortNeedsResume(me, nonCombatEvents, EVENT_ESCORT))
                {
                    HotEscortLog(me, "resume (walk interrupted, combat over)", wp);
                    nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_LAVA_BURST:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_LAVA_BURST, false);

                            events.ScheduleEvent(EVENT_LAVA_BURST, 10000);
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_CHAIN_LIGHTNING, false);

                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                            break;
                        case EVENT_HEALING_WAVE:
                            if (HealthBelowPct(50))
                                me->CastSpell(me, SPELL_HEALING_WAVE, false);

                            events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
                            break;
                        case EVENT_RISING_FIRE_TOTEM:
                            me->CastSpell(me, SPELL_RISING_FIRE_TOTEM, false);
                            break;
                    }
                }

                DoMeleeAttackIfReady();

            }

        private:
            InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thrall_asiraAI(creature);
        }
};

// Thrall Trash Benedictus 54634
class npc_thrall_trash_benedictus : public CreatureScript
{
    public:
        npc_thrall_trash_benedictus() : CreatureScript("npc_thrall_trash_benedictus") { }

        bool OnGossipSelect(Player* player, Creature* creature, uint32 /*sender*/, uint32 action) override
        {
            player->PlayerTalkClass->ClearMenus();

            if (action == GOSSIP_ACTION_INFO_DEF + 1)
                creature->AI()->DoAction(ACTION_ACTIVATE);

            player->CLOSE_GOSSIP_MENU();

            return true;
        }

        bool OnGossipHello(Player* player, Creature* creature) override
        {
            player->ADD_GOSSIP_ITEM(GOSSIP_ICON_CHAT, THRALL_BEFORE_BENEDICTUS, GOSSIP_SENDER_MAIN, GOSSIP_ACTION_INFO_DEF + 1);
            player->SEND_GOSSIP_MENU(18293, creature->GetGUID());
            return true;
        }

        struct npc_thrall_trash_benedictusAI : public ScriptedAI
        {
            npc_thrall_trash_benedictusAI(Creature* creature) : ScriptedAI(creature), slimes(creature) { }
            EventMap nonCombatEvents;
            SummonList slimes;
            uint32 timer, wp, tentacle;
            bool ToDragonSoul, LastStalker;

            // Refs #696: the Path of the Titans has three faceless waves (Faceless Shadow Weaver + Faceless Brute + Shadow Borer
            // out of a Dark Haze pool, Project-Neltharion-4.3.4 boss_archbishop_benedictus.cpp wavesInfos / ActivateWave, sniffed
            // 4.3.4 positions = our three Dark Haze 54628 DB rows). The old 47 yd stalker search missed the second pool at point 3
            // (3814.1,322.8,-32.8 is 49.1 yd from 3835.7,280.2,-21.0), so wave 2 never came. Each wave point now wakes the nearest
            // pool within 60 yd (one per point: 25 / 49 / 40 yd, the next nearest is 77+ yd); a used pool stays used.
            void ActivateDarkHaze()
            {
                if (Creature* haze = me->FindNearestCreature(NPC_DARK_HAZE_STALKER, 60.0f, true))
                {
                    haze->AI()->DoAction(ACTION_SPAWN);
                    HotEscortLog(me, "wave", wp);
                }
                else
                    HotEscortLog(me, "wave (no dark haze within 60 yd)", wp);
            }

            // Refs #696: Flailing Tentacle 54696 (VehicleId 1743, Health_mod 100 = ~8.3 M hp at level 87 elite) is the scene of a
            // tentacle bursting out of the ground and snatching an Earthen Ring shaman (Neltharion npc_flailing_tentacle:
            // UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NON_ATTACKABLE, despawns 8 s after the grab; Thrall's waves never wait for it).
            // It was summoned as a hostile zone-in-combat add that the escort waited on, so the 4-bot group had to burn a
            // boss-sized, harmless tentacle at every wave point (w106: 330 s at point 2, then stuck at point 3 until the wipe).
            void SummonFlailingTentacle()
            {
                if (tentacle < TAXI_TWILIGHT_DRAKE_POINTS)
                    me->SummonCreature(NPC_FLAILING_TENTACLE, TentacleSpawnPos[tentacle], TEMPSUMMON_TIMED_DESPAWN, 8000ms);
                tentacle++;
            }

            void JustSummoned(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_CORRUPTED_SLIME)
                    slimes.Summon(summon);
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                slimes.Despawn(summon);
            }

            void IsSummonedBy(Unit* summoner) override
            {
                LastStalker = false;
                ToDragonSoul = false;
                me->SetReactState(REACT_PASSIVE);
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_ACTIVATE:
                        me->RemoveFlag(UNIT_FIELD_NPC_FLAGS, UNIT_NPC_FLAG_GOSSIP);
                        SetEquipmentSlots(false, DoomHammer, 0, EQUIP_NO_CHANGE);
                        nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1500);
                        nonCombatEvents.ScheduleEvent(EVENT_CORRUPTED_SLIME, 5000);
                        break;
                    case ACTION_BENEDICTUS:
                        wp = 0;
                        ToDragonSoul = true;
                        nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 3500);
                        break;
                    default:
                        break;
                }
            }

            void JustEngagedWith(Unit* /*who*/) override { }

            void InitializeAI() override
            {
                wp = 0;
                tentacle = 0;
                ToDragonSoul = false;
                me->SetReactState(REACT_PASSIVE);  // temp
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE); // temp
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type != POINT_MOTION_TYPE)
                    return;

                HotEscortLog(me, "reached", pointId); // Refs #696: the driver's benedictus escort-log row
                wp++;

                switch (pointId)
                {
                    case 2:
                    case 3:
                        ActivateDarkHaze();
                        SummonFlailingTentacle();
                        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        nonCombatEvents.ScheduleEvent(EVENT_COMBAT, 1000);
                        break;
                    case 6:
                        ActivateDarkHaze();
                        SummonFlailingTentacle();
                        me->SetHomePosition(me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation());
                        LastStalker = true;
                        nonCombatEvents.ScheduleEvent(EVENT_COMBAT, 1000);
                        break;
                    case 13:
                        me->SummonCreature(NPC_THRALL_5, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->DespawnOrUnsummon();
                        break;
                    default:
                        ToDragonSoul ? nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, urand(100, 200)) : nonCombatEvents.ScheduleEvent(EVENT_ESCORT, urand(100, 200));
                        break;
                }
            }

            // Refs #696: a wave is the three faceless (Neltharion ActivateWave: Thrall moves on after their kills). The cosmetic
            // tentacle and the Corrupted Slimes never hold the escort.
            bool HasInCombat()
            {
                Creature* Brute = GetClosestCreatureWithEntry(me, NPC_FACELESS_BRUTE, 200.0f, true);
                Creature* Borer = GetClosestCreatureWithEntry(me, NPC_FACELESS_BORER, 200.0f, true);
                Creature* ShadowWeaver = GetClosestCreatureWithEntry(me, NPC_FACELESS_SHADOW_WEAVER, 200.0f, true);

                if (Brute || Borer || ShadowWeaver)
                    return true;

                return false;
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_ESCORT:
                        {
                            // Refs #696: the escort was dropped for good when a faceless or a tentacle was still alive within
                            // 200 yd at this moment (the event is consumed and only MovementInform / EVENT_COMBAT schedule it).
                            // Hold and retry, like npc_thrall_intro (#673).
                            if (HasInCombat())
                            {
                                HotEscortLog(me, "hold (faceless alive)", wp);
                                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                                break;
                            }

                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();

                            if (me->GetUnitMovementFlags() == MOVEMENTFLAG_WALKING)
                                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

                            me->GetMotionMaster()->MovePoint(wp, ThrallWaypoints_Benedict_Prepare[wp]);
                            HotEscortLog(me, "move", wp);
                            break;
                        }
                        case EVENT_COMBAT:
                        {
                            if (HasInCombat())
                            {
                                nonCombatEvents.ScheduleEvent(EVENT_COMBAT, urand(100, 200));
                                return;
                            }
                            HotEscortLog(me, "wave-clear", wp);
                            // Refs #696: Reset() here also dropped the slime drip after wave 1; only the escort pair is cleared.
                            nonCombatEvents.CancelEvent(EVENT_ESCORT);
                            nonCombatEvents.CancelEvent(EVENT_COMBAT);

                            if (LastStalker)
                            {
                                // Neltharion WaypointReached 7/8: the slimes are despawned and the drip ends with the last wave.
                                nonCombatEvents.CancelEvent(EVENT_CORRUPTED_SLIME);
                                slimes.DespawnAll();
                                me->SummonCreature(NPC_BENEDICTUS, BenedictusSpawnPoint, TEMPSUMMON_MANUAL_DESPAWN);
                            }
                            else
                                nonCombatEvents.ScheduleEvent(EVENT_ESCORT, 1000);
                            break;
                        }
                        case EVENT_CORRUPTED_SLIME:
                        {
                            // Refs #696: Corrupted Slimes ooze out next to Thrall while he walks the Path of the Titans: two every
                            // 18 s, first 5 s after "Yes Thrall, lets do this!", none once he is past x 3760 (the last wave), all
                            // despawned with the last wave (Project-Neltharion-4.3.4 npc_thrall_hot_3 EVENT_ACTIVATE_BLOOD /
                            // ActivateRandomBlood). They used to come from the three Dark Haze pools every 8-20 s each from the
                            // moment the grid loaded, for the whole life of the instance (the endless ATTACKSTART stream, w106).
                            if (ToDragonSoul || me->GetPositionX() <= 3760.0f)
                                break;

                            for (uint8 i = 0; i < 2; ++i)
                                me->SummonCreature(NPC_CORRUPTED_SLIME, me->GetRandomNearPosition(15.0f), TEMPSUMMON_MANUAL_DESPAWN);
                            nonCombatEvents.ScheduleEvent(EVENT_CORRUPTED_SLIME, 18000);
                            break;
                        }
                        case EVENT_ESCORT_DIFF:
                        {
                            // Refs #673: same hold as npc_thrall_intro (keep the event alive while the faceless are up).
                            if (me->IsInCombat())
                            {
                                nonCombatEvents.ScheduleEvent(EVENT_ESCORT_DIFF, 1000);
                                break;
                            }

                            me->GetMotionMaster()->MovementExpired();
                            me->GetMotionMaster()->Clear();

                            if (me->GetUnitMovementFlags() == MOVEMENTFLAG_WALKING)
                                me->RemoveUnitMovementFlag(MOVEMENTFLAG_WALKING);

                            me->GetMotionMaster()->MovePoint(wp + 7, ThrallWaypoints_To_Dragon_Soul[wp]);
                            HotEscortLog(me, "move", wp + 7);
                            break;
                        }
                        default:
                            break;
                    }
                }

                if (!UpdateVictim())
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thrall_trash_benedictusAI(creature);
        }
};

// Thrall Benedictus 54971
class npc_thrall_benedictus : public CreatureScript
{
    public:
        npc_thrall_benedictus() : CreatureScript("npc_thrall_benedictus") { }

        struct npc_thrall_benedictusAI : public ScriptedAI
        {
            npc_thrall_benedictusAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events, nonCombatEvents;
            uint32 timer;

            void IsSummonedBy(Unit* summoner) override 
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void Reset() override { }

            void InitializeAI() override
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_LAVA_BURST, 1500);
                events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
            }

            void DoAction(int32 action) override 
            {
                switch (action)
                {
                    case ACTION_BENEDICTUS:
                        Talk(TALK_BENEDICTUS_1);
                        break;
                    case ACTION_TALK_FINISH_HIM:
                        Talk(TALK_BENEDICTUS_2);
                        break;
                    case ACTION_COMBAT:
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        me->SetReactState(REACT_AGGRESSIVE);
                        DoZoneInCombat();
                        break;
                    case ACTION_WATER_SHIELD:
                        events.ScheduleEvent(EVENT_WATER_SHIELD, 1000);
                        break;
                    case ACTION_BENEDICTUS_WIPE:
                        // Refs #696 (the #824 pattern): Benedictus evaded with nobody left. Thrall drops the fight (he took no
                        // damage, DamageTaken) and stands passive again; the next pull runs ACTION_COMBAT.
                        events.Reset();
                        me->InterruptNonMeleeSpells(false);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
                        if (!me->IsInEvadeMode())
                            EnterEvadeMode();
                        break;
                    default:
                        break;
                }
            }

            // Refs #696: Wowpedia "Archbishop Benedictus (tactics)" (MediaWiki API, 2026-10-22): "Benedictus will summon three
            // Purifying Light orbs, two of which Thrall will destroy with his chain lightning, leaving only one to avoid", and Thrall
            // "lashes out at the Purifying Light ... destroying them" (Chain Lightning 104130). Phase 1 only: in phase 2 he is held by
            // Engulfing Twilight (103762, conditions -> 54971) and "will be unable to assist".
            void SetGUID(ObjectGuid guid, int32 id) override
            {
                if (id != ACTION_THRALL_ORB_LIGHTNING || me->HasUnitState(UNIT_STATE_STUNNED))
                    return;

                if (Creature* orb = ObjectAccessor::GetCreature(*me, guid))
                {
                    me->CastSpell(orb, SPELL_CHAIN_LIGHTNING, true);
                    orb->DespawnOrUnsummon(1500);
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                damage = 0;
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
                        case EVENT_LAVA_BURST:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_LAVA_BURST, false);
                            events.ScheduleEvent(EVENT_LAVA_BURST, 10000);
                            break;
                        case EVENT_CHAIN_LIGHTNING:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_CHAIN_LIGHTNING, false);
                            events.ScheduleEvent(EVENT_CHAIN_LIGHTNING, 30000);
                            break;
                        case EVENT_HEALING_WAVE:
                            if (HealthBelowPct(50))
                                me->CastSpell(me, SPELL_HEALING_WAVE, false);
                            events.ScheduleEvent(EVENT_HEALING_WAVE, 50000);
                            break;
                        case EVENT_WATER_SHIELD:
                            // Refs #696: 103688 is TARGET_UNIT_TARGET_ANY (SpellEffect.dbc), so DoCastAOE (no target) put the
                            // channel on Thrall's victim - Benedictus - and its 1 s pulse 103744 (4 yd SRC_AREA_ALLY, Water Shell
                            // +100% damage done) went to HIS allies. Thrall shells himself: the players stack on him (Wowpedia
                            // "Archbishop Benedictus (tactics)": "all party members should move to the Water Shell Thrall creates").
                            DoCast(me, SPELL_WATER_SHIELD);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_thrall_benedictusAI(creature);
        }
};

enum Texts
{
    EMOTE_DARK_HAZE                     = 0,
};

// Frozen Servitors stalker 55389, 54560, 55384, 55385, 54598
// Asira stalkers 53488, 3570621, 3570622
// Faceless stalkers 54628, 3570623
class npc_frozen_servitor_stalker : public CreatureScript
{
    public:
        npc_frozen_servitor_stalker() : CreatureScript("npc_frozen_servitor_stalker") { }

        struct npc_frozen_servitor_stalkerAI : public ScriptedAI
        {
            npc_frozen_servitor_stalkerAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap nonCombatEvents;
            bool hazeSpent = false; // Refs #696: a Dark Haze pool brings out its faceless wave once

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE);
            }

            // Refs #696: the Dark Haze pools no longer drip Corrupted Slimes on their own (8-20 s each, from grid load, forever,
            // every slime hunting the nearest player within 100 yd); Thrall 54634 drips them during his walk
            // (npc_thrall_trash_benedictus EVENT_CORRUPTED_SLIME).
            void InitializeAI() override
            {
                Reset();
            }

            void DoAction(int32 actionId) override
            {
                switch (actionId)
                {
                    case ACTION_SPAWN:
                    {
                        switch (me->GetEntry())
                        {
                            case NPC_FROZEN_SERVITOR_STALKER_ENTRANCE:
                            case NPC_FROZEN_SERVITOR_SPAWN_STALKER_2:
                                me->CastSpell(me, SPELL_APPEARS, false);
                                me->SummonCreature(NPC_FROZEN_SERVITOR, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                break;
                            case NPC_FROZEN_SERVITOR_SPAWN_STALKER:
                                me->SummonCreature(NPC_CRYSTALLINE_ELEMENTAL, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                break;
                            case NPC_ASIRA_TRASH_STALKER_PH1:
                                me->SummonCreature(NPC_TWILIGHT_ASSASSIN, me->GetPositionX() + frand(-15.0f, 15.0f), me->GetPositionY() + frand(-5.0f, 5.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_TWILIGHT_BRUISER, me->GetPositionX() + frand(-15.0f, 15.0f), me->GetPositionY() + frand(-5.0f, 5.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_TWILIGHT_RANGER, me->GetPositionX() + frand(-15.0f, 15.0f), me->GetPositionY() + frand(-5.0f, 5.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_TWILIGHT_THUG, me->GetPositionX() + frand(-15.0f, 15.0f), me->GetPositionY() + frand(-5.0f, 5.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_TWILIGHT_SHADOW_WALKER, me->GetPositionX() + frand(-15.0f, 15.0f), me->GetPositionY() + frand(-5.0f, 5.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                break;
                            case NPC_DARK_HAZE_STALKER:
                                if (hazeSpent)
                                    break;
                                hazeSpent = true;
                                if (me->HasAura(SPELL_GROW_VISUAL_COSMETIC))
                                    me->RemoveAura(SPELL_GROW_VISUAL_COSMETIC);
                                Talk(EMOTE_DARK_HAZE);
                                me->SummonCreature(NPC_FACELESS_BORER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_FACELESS_SHADOW_WEAVER, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                me->SummonCreature(NPC_FACELESS_BRUTE, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                                break;
                        }
                        break;
                    }
                    case ACTION_SPAWN_PH2:
                    {
                        me->SummonCreature(NPC_TWILIGHT_ASSASSIN, me->GetPositionX() + frand(-3.0f, 3.0f), me->GetPositionY() + frand(-1.5f, 1.5f), me->GetPositionZ() + frand(1.0f, 2.0f), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_TWILIGHT_SHADOW_WALKER, me->GetPositionX() + frand(-3.0f, 3.0f), me->GetPositionY() + frand(-1.5f, 1.5f), me->GetPositionZ() + frand(1.0f, 2.0f), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_TWILIGHT_BRUISER, me->GetPositionX() + frand(-3.0f, 3.0f), me->GetPositionY() + frand(-1.5f, 1.5f), me->GetPositionZ() + frand(1.0f, 2.0f), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    }
                    case ACTION_SPAWN_PH3:
                    {
                        me->SummonCreature(NPC_TWILIGHT_BRUISER, me->GetPositionX() + frand(-1.5f, -1.0f), me->GetPositionY() + frand(-3.0f, 3.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_TWILIGHT_RANGER, me->GetPositionX() + frand(-1.5f, -1.0f), me->GetPositionY() + frand(-3.0f, 3.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_TWILIGHT_SHADOW_WALKER, me->GetPositionX() + frand(-1.5f, -1.0f), me->GetPositionY() + frand(-3.0f, 3.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        me->SummonCreature(NPC_TWILIGHT_SHADOW_WALKER, me->GetPositionX() + frand(-1.5f, -1.0f), me->GetPositionY() + frand(-3.0f, 3.0f), me->GetPositionZ(), me->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                        break;
                    }
                }
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                damage = 0;
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_frozen_servitor_stalkerAI(creature);
        }
};

class npc_twilights_hammer : public CreatureScript
{
    public:
        npc_twilights_hammer() : CreatureScript("npc_twilights_hammer") { }

        struct npc_twilights_hammerAI : public ScriptedAI
        {
            npc_twilights_hammerAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events, nonCombatEvents;
            uint32 timer;

            void IsSummonedBy(Unit* summoner) override 
            {
                if (summoner->GetPositionZ()<12.0f)
                    nonCombatEvents.ScheduleEvent(EVENT_COMBAT, 1000);
                else
                {
                    me->SetHomePosition(me->GetPositionX() + frand(15.0f, 20.0f), me->GetPositionY() + frand(15.0f, 20.0f), -6.36f, me->GetOrientation());
                    me->GetMotionMaster()->MoveJump(me->GetPositionX() + frand(15.0f, 20.0f), me->GetPositionY() + frand(15.0f, 20.0f), -6.36f, 30.0f, 20.0f, EVENT_JUMP);
                    nonCombatEvents.ScheduleEvent(EVENT_COMBAT, 4000);
                    if (me->GetEntry() == NPC_TWILIGHT_RANGER)
                        Talk(0);
                }
            }

            void JustEngagedWith(Unit* who) override
            {
                switch (me->GetEntry())
                {
                    case NPC_TWILIGHT_ASSASSIN:
                        events.ScheduleEvent(EVENT_GARROTE, 1000);
                        events.ScheduleEvent(EVENT_BACKSTAB, 5000);
                        events.ScheduleEvent(EVENT_EVISCREATE, 7000);
                        break;
                    case NPC_TWILIGHT_BRUISER:
                        events.ScheduleEvent(EVENT_MORTAL_STRIKE, 5000);
                        events.ScheduleEvent(EVENT_STAGGERING_BLOW, 10000);
                        break;
                    case NPC_TWILIGHT_RANGER:
                        events.ScheduleEvent(EVENT_FREEZING_TRAP, 1);
                        break;
                    case NPC_TWILIGHT_THUG:
                        events.ScheduleEvent(EVENT_BEATDOWN, urand(7000, 10000));
                        break;
                    case NPC_TWILIGHT_SHADOW_WALKER:
                        events.ScheduleEvent(EVENT_MIND_FLAY, 1000);
                        events.ScheduleEvent(EVENT_THUNGERING_SHADOWS, urand(9000, 13000));
                        break;
                    default:
                        break;
                }
            }

            void Reset() override
            {
                events.Reset();
            }

            void InitializeAI() override
            {
                if (me->GetEntry() == NPC_TWILIGHT_ASSASSIN)
                    SetEquipmentSlots(false, AsiraDaggers, AsiraDaggers, EQUIP_NO_CHANGE);
            }

            void DoAction(int32 /*action*/) override { }

            void DamageTaken(Unit* /*attacker*/, uint32& /*damage*/) override { }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    if (eventId == EVENT_COMBAT)
                    {
                        if (!me->IsInCombat())
                            if (Creature* Thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_3, 80.0f))
                            {
                                me->SetInCombatWith(Thrall);
                                me->Attack(Thrall, true);

                                if (me->GetDistance(Thrall) > 7)
                                    me->GetMotionMaster()->MoveChase(Thrall);
                            }
                    }
                    break;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GARROTE:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_GARROTE, true);
                            break;
                        case EVENT_BACKSTAB:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_BACKSTAB, true);

                            events.ScheduleEvent(EVENT_BACKSTAB, 10000);
                            break;
                        case EVENT_EVISCREATE:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_EVISCERATE, true);

                            events.ScheduleEvent(EVENT_EVISCREATE, urand(6000, 7000));
                            break;
                        case EVENT_MORTAL_STRIKE:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_MORTAL_STRIKE, true);

                            events.ScheduleEvent(EVENT_MORTAL_STRIKE, urand(6000, 7000));
                            break;
                        case EVENT_STAGGERING_BLOW:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_STAGGERING_BLOW, true);

                            events.ScheduleEvent(EVENT_STAGGERING_BLOW, 10000);
                            break;
                        case EVENT_BEATDOWN:
                            if (Unit* itr = me->GetVictim())
                            {
                                me->CastSpell(itr, SPELL_BEATDOWN, true);
                                me->CastSpell(itr, SPELL_BASH, true);
                            }
                            events.ScheduleEvent(EVENT_BEATDOWN, urand(10000, 15000));
                            break;
                        case EVENT_FREEZING_TRAP:
                            break;
                        case EVENT_THUNGERING_SHADOWS:
                            me->CastSpell(me, SPELL_HUNGERING_SHADOWS, false);
                            events.ScheduleEvent(EVENT_THUNGERING_SHADOWS, urand(15000, 25000));
                            break;
                        case EVENT_MIND_FLAY:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_MIND_FLAY, true);

                            events.ScheduleEvent(EVENT_MIND_FLAY, urand(5000, 7000));
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_twilights_hammerAI(creature);
        }
};

class npc_faceless_minions : public CreatureScript
{
    public:
        npc_faceless_minions() : CreatureScript("npc_faceless_minions") { }

        struct npc_faceless_minionsAI : public ScriptedAI
        {
            npc_faceless_minionsAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events, nonCombatEvents;
            uint32 timer;

            void IsSummonedBy(Unit* summoner) override
            {
                switch (me->GetEntry())
                {
                    case NPC_FLAILING_TENTACLE:
                        // Refs #696: scenery, not an add (Project-Neltharion-4.3.4 npc_flailing_tentacle sets
                        // UNIT_FLAG_NOT_ATTACKABLE_1 | UNIT_FLAG_NON_ATTACKABLE; npc_thrall_trash_benedictus summons it for 8 s).
                        SetCombatMovement(false);
                        me->SetReactState(REACT_PASSIVE);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_IMMUNE_TO_NPC);
                        return;
                    case NPC_FACELESS_BORER:
                        SetCombatMovement(false);
                        me->SetInCombatWithZone();
                        [[fallthrough]];
                    default:
                        nonCombatEvents.ScheduleEvent(EVENT_COMBAT, 4000);
                        break;
                }

            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override 
            {
                ScriptedAI::EnterEvadeMode();
            }

            void JustEngagedWith(Unit* who) override
            {
                switch (me->GetEntry())
                {
                    case NPC_FACELESS_BORER:
                        events.ScheduleEvent(EVENT_SHADOW_BORE, urand(1000, 2500));
                        break;
                    case NPC_FACELESS_BRUTE:
                        events.ScheduleEvent(EVENT_TENTACLE_SMASH, 4000);
                        events.ScheduleEvent(EVENT_SQUEEZE_LIFELESS, urand(7000, 10000));
                        break;
                    case NPC_FLAILING_TENTACLE:
                        break;
                    case NPC_FACELESS_SHADOW_WEAVER:
                        events.ScheduleEvent(EVENT_SHADOW_VALLEY, urand(3000, 6000));
                        events.ScheduleEvent(EVENT_SEEKING_SHADOW, urand(9000, 13000));
                        break;
                    default:
                        break;
                }
            }

            void Reset() override
            {
                events.Reset();
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    if (eventId == EVENT_COMBAT)
                    {
                        if (!me->IsInCombat())
                            if (Player* itr = me->FindNearestPlayer(100.0f))
                            {
                                me->SetInCombatWith(itr);
                                me->Attack(itr, true);
                            }
                    }
                    break;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SQUEEZE_LIFELESS:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_SQUEEZE_LIFELESS, false);
                            events.ScheduleEvent(EVENT_SQUEEZE_LIFELESS, 20000);
                            break;
                        case EVENT_TENTACLE_SMASH:
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_TENTACLE_SMASH, false);
                            events.ScheduleEvent(EVENT_TENTACLE_SMASH, urand(8000, 11000));
                            break;
                        case EVENT_SHADOW_BORE:
                            if (Unit* rnd = SelectTarget(SELECT_TARGET_RANDOM))
                                me->CastSpell(rnd, SPELL_SHADOW_BORE, false);
                            events.ScheduleEvent(EVENT_SHADOW_BORE, urand(4000, 5000));
                            break;
                        case EVENT_SEEKING_SHADOW:
                            if (Unit* itr = SelectTarget(SELECT_TARGET_FARTHEST))
                                me->CastSpell(itr, SPELL_SEEKING_SHADOWS, false);
                            events.ScheduleEvent(EVENT_SEEKING_SHADOW, urand(10000, 15000));
                            break;
                        case EVENT_SHADOW_VALLEY:  
                            if (Unit* itr = me->GetVictim())
                                me->CastSpell(itr, SPELL_SHADOW_VOLLEY, false);
                            events.ScheduleEvent(EVENT_SHADOW_VALLEY, urand(6000, 8000));
                            break;
                    }
                }

                if (me->GetEntry() == NPC_FACELESS_BORER || me->GetEntry() == NPC_FLAILING_TENTACLE)
                    return;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_faceless_minionsAI(creature);
        }
};

// Crystalline elemental 55559
class npc_crystalline_elemental : public CreatureScript
{
    public:
        npc_crystalline_elemental() : CreatureScript("npc_crystalline_elemental") { }

        struct npc_crystalline_elementalAI : public ScriptedAI
        {
            npc_crystalline_elementalAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint32 timer;

            void JustEngagedWith(Unit* /*who*/) override
            {
                timer = 3000;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (timer <= diff)
                {
                    if (Unit* itr = me->GetVictim())
                        me->CastSpell(itr, SPELL_IMPALE, false);

                    timer = urand(5000, 8000);
                }
                else timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_crystalline_elementalAI(creature);
        }
};

// Ice wall exit stalker 55728
class npc_ice_wall_exit_stalker : public CreatureScript
{
    public:
        npc_ice_wall_exit_stalker() : CreatureScript("npc_ice_wall_exit_stalker") { }

        struct npc_ice_wall_exit_stalkerAI : public ScriptedAI
        {
            npc_ice_wall_exit_stalkerAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override 
            {
                me->SetDisplayFromModel(1);
                me->SetReactState(REACT_PASSIVE);
            }

            void UpdateAI(uint32 /*diff*/) override { }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_ice_wall_exit_stalkerAI(creature);
        }
};
class MoltenFuryPredicate
{
    public:
        MoltenFuryPredicate() { }

        bool operator()(WorldObject* object)
        {
            return object && object->ToCreature() && object->ToCreature()->GetEntry() != NPC_ICE_WALL_EXIT_STALKER;
        }
};

// Molten Fury 103905
class spell_hour_of_twilight_molten_fury : public SpellScript
{
    PrepareSpellScript(spell_hour_of_twilight_molten_fury);

    void HandleHit(SpellEffIndex effIndex)
    {
        if (Creature* target = GetHitCreature())
        {
            if (GameObject* go = target->FindNearestGameObject(GO_ICEWALL_2, 50.0f))
                go->UseDoorOrButton();

            target->DespawnOrUnsummon();
        }
    }

    void CheckTargets(std::list<WorldObject*>& targets)
    {
        targets.remove_if([=](WorldObject* target) { return target->GetEntry() != NPC_ICE_WALL_EXIT_STALKER; });
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_hour_of_twilight_molten_fury::HandleHit, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
        OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hour_of_twilight_molten_fury::CheckTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
    }
};

/// 210026
class go_hot_time_transit_device : public GameObjectScript
{
public:
    go_hot_time_transit_device() : GameObjectScript("go_hot_time_transit_device") {}

    bool OnReportUse(Player* player, GameObject* go)
    {
        if (player->IsInCombat())
            return false;

        if (InstanceScript* instance = go->GetInstanceScript())
        {
            if (instance->GetBossState(DATA_BENEDICTUS) == DONE)
            {
                player->CastSpell(player, SPELL_TELEPORT_BENEDICTUS, true);
                return true;
            }
            else if (instance->GetBossState(DATA_ASIRA) == DONE)
            {
                player->CastSpell(player, SPELL_TELEPORT_ASIRA_DEAD, true);
                return true;
            }
            else if (instance->GetBossState(DATA_ARCURION) == DONE)
            {
                player->CastSpell(player, SPELL_TELEPORT_ARCURION_DEAD, true);
                return true;
            }
        }

        return false;
    }
};

void AddSC_hour_of_twilight()
{
    new npc_hour_of_twilight_life_warden();
    new npc_hour_of_twilight_life_warden_thrall();
    new npc_hour_of_twilight_twilight_drake();
    new npc_thrall_intro();
    new npc_thrall_arcurion();
    new npc_thrall_asira();
    new npc_thrall_trash_benedictus();
    new npc_thrall_benedictus();
    new npc_frozen_servitor_stalker();
    new npc_twilights_hammer();
    new npc_faceless_minions();
    new npc_crystalline_elemental();
    new npc_ice_wall_exit_stalker();
    new spell_script<spell_hour_of_twilight_molten_fury>("spell_hour_of_twilight_molten_fury");
    new go_hot_time_transit_device();
}
