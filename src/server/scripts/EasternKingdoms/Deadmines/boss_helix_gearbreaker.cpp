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

#include "ScriptPCH.h"
#include "deadmines.h"
#include "Vehicle.h"

// todo: ����������� ������ �������, ������� ������� �����

// #681: the group ids of the MoP test world creature_text rows for 47296 (the same as CPP boss_helix_gearbreaker.cpp:35-52). The
// old values said the death line on aggro, "Ready Oafie? Throw!" when the Oaf died, and so on.
enum ScriptTexts
{
    SAY_AGGRO               = 0,  // "The mistress will pay me handsomely for your heads!"
    SAY_STICKY_BOMB         = 1,  // "Bombs away!"
    SAY_THROW_HELIX         = 2,  // "Ready Oafie? Throw!"
    SAY_ANNOUNCE_CHEST_BOMB = 3,  // boss emote "Helix attaches a bomb to $n's chest!"
    SAY_OAF_DEAD            = 4,  // "I didn't need him! Not when I've got YOU oafs!"
    SAY_KILL                = 5,
    SAY_DEATH               = 6,

    SAY_OAF1     = 0,
};

enum Spells
{
    SPELL_CHEST_BOMB            = 88352,
    SPELL_CHEST_BOMB_DMG        = 88250,

    SPELL_OAF_SMASH             = 88300,
    SPELL_CHARGE                = 88288,
    SPELL_FORCE_PLAYER_RIDE_OAF = 88278,
    SPELL_RIDE_OAF              = 88277,
    SPELL_HELIX_RIDE            = 88337,
};

enum Adds
{
    NPC_STICKY_BOMB = 47314,
    NPC_HELIX_CREW  = 49136,
    NPC_MINE_RAT    = 51462,
};

enum Events
{
    EVENT_THROW_HELIX   = 5,
    EVENT_PICKUP_HELIX  = 6,
    EVENT_STICKY_BOMB   = 7,
    EVENT_HELIX_CREW    = 8,
    EVENT_CHARGE_OAF0   = 1,
    EVENT_CHARGE_OAF1   = 2,
    EVENT_CHARGE_OAF2   = 3,
    EVENT_CHARGE_OAF3   = 4,
    EVENT_BOMB_READY    = 9,
    EVENT_BOMB_EXPLODE  = 10,
    EVENT_CHEST_BOMB    = 11,
    EVENT_RIDE_FACE_END = 12,  // #681
    EVENT_LEAP          = 13,  // #681 stage two
    EVENT_HELIX_CREW_2  = 14,  // #681 stage two, heroic
    EVENT_REBOARD_CHECK = 15,  // #681
};

enum Points
{
    POINT_START = 1,
    POINT_END   = 2,
};

enum Actions
{
    ACTION_CHARGE   = 1,
    ACTION_OAF_DOWN = 2,  // #681: the Oaf's fake death -> Helix stage two
};

// #681: CPP boss_helix_gearbreaker.cpp:165-171 - the second crew, heroic stage two
const Position helixSecondCrewPos[4] =
{
    { -282.2639f, -503.8333f, 60.60244f, 5.201081f },
    { -287.474f,  -503.2708f, 60.44886f, 1.675516f },
    { -291.0781f, -504.0938f, 60.25187f, 4.607669f },
    { -294.1632f, -503.2326f, 60.1258f,  1.867502f },
};

const Position lumberingoafPos[3] =
{
    {-301.93f, -516.32f, 51.71f, 2.03f},
    {-289.98f, -528.06f, 49.75f, 1.59f},
    {-289.67f, -488.46f, 49.80f, 1.54f} 
};

const Position helixcrewPos[4] =
{
    {-295.26f,-503.38f,60.16f, 0.0f},
    {-280.85f,-503.98f,60.16f, 0.0f},
    {-291.45f,-503.37f,60.16f, 0.0f},
    {-285.63f,-504.04f,60.16f, 0.0f}
};

const Position oafPos[1] =
{
    {-302.361f, -516.346f, 52.0315f, 0.174533f},
};

class boss_helix_gearbreaker : public CreatureScript
{
    public:
        boss_helix_gearbreaker() : CreatureScript("boss_helix_gearbreaker") { }

        struct boss_helix_gearbreakerAI : public BossAI
        {
            boss_helix_gearbreakerAI(Creature* creature) : BossAI(creature, DATA_HELIX) { }

            bool _stageTwo = false;            // #681
            bool _reboardPending = false;      // #681: back onto the Oaf, the seat is taken a tick later
            ObjectGuid _faceTarget;            // #681: the player Helix rides now

            void InitializeAI() override
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }
            
            void Reset() override 
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
            {
                if (summon->GetEntry() == NPC_LUMBERING_OAF)
                {
                    Talk(SAY_OAF_DEAD);
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                }
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                events.ScheduleEvent(EVENT_STICKY_BOMB, 8000);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_HELIX_CREW, 5000);
                // #681 Throw Helix (Wowpedia "Helix Gearbreaker", MediaWiki API 2026-10-23: "The Lumbering Oaf will take Helix from
                // his back, and throw the goblin onto a random player's face for ten seconds"; the heroic Chest Bomb is left on that
                // player when he goes back). Timing: CPP boss_helix_gearbreaker.cpp:194 (34 s) and :317-323 (every 52 s). The old
                // random Chest Bomb every 15-20 s (no throw) is gone: on heroic the bomb comes with every face ride.
                events.ScheduleEvent(EVENT_THROW_HELIX, 34000);
                _stageTwo = false;
                _faceTarget = ObjectGuid::Empty;
                Talk(SAY_AGGRO);
                DoZoneInCombat();
            }

            // #681: the Oaf's fake death (npc_lumbering_oafAI::DamageTaken) -> stage two "I didn't need him!": CPP :235-249 -
            // the second crew 3 s later on heroic, the first Leap 2.5 s later (unless Helix already rides a face)
            void DoAction(int32 action) override
            {
                if (action != ACTION_OAF_DOWN || _stageTwo)
                    return;

                _stageTwo = true;
                Talk(SAY_OAF_DEAD);
                events.CancelEvent(EVENT_THROW_HELIX);
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_HELIX_CREW_2, 3000);
                if (!_faceTarget)
                    events.ScheduleEvent(EVENT_LEAP, 2500);
                TC_LOG_INFO("scripts", "[DM-HELIX] stage two (the Oaf is down)");
            }

            // #681 CPP Oafguard (90546, :255-260): Helix cannot die while his Oaf stands
            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (!_stageTwo && damage >= me->GetHealth())
                    damage = me->GetHealth() - 1;
            }

            // #681: Helix on a player's face - Throw Helix (stage one) and Leap (stage two). The CPP rides the player as a vehicle
            // (88360; players get vehicle 1263 from 88337); here he jumps to the player, attacks only him for 10 s
            // (88351 Helix Ride Face Timer Aura, 5.4.8 SpellDuration 10 s) and is attackable meanwhile.
            void RideFace(Unit* target)
            {
                if (!target)
                    return;

                if (me->GetVehicle())
                    me->ExitVehicle();

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                me->GetMotionMaster()->MoveJump(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 20.0f, 10.0f);
                me->GetThreatManager().ResetAllThreat();
                me->GetThreatManager().AddThreat(target, 1000000.0f);
                AttackStart(target);
                _faceTarget = target->GetGUID();
                events.ScheduleEvent(EVENT_RIDE_FACE_END, 10000);
                TC_LOG_INFO("scripts", "[DM-HELIX] %s onto %s's face", _stageTwo ? "leap" : "thrown", target->GetName().c_str());
            }

            Unit* PickFaceTarget(ObjectGuid exclude)
            {
                Unit* pick = nullptr;
                for (uint8 i = 0; i < 6; ++i)
                {
                    pick = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                    if (!pick || pick->GetGUID() != exclude)
                        break;
                }
                return pick;
            }

            void EndRideFace()
            {
                Unit* rider = _faceTarget ? ObjectAccessor::GetUnit(*me, _faceTarget) : nullptr;
                ObjectGuid const last = _faceTarget;
                _faceTarget = ObjectGuid::Empty;
                if (rider && rider->IsAlive() && IsHeroic())
                {
                    DoCast(rider, SPELL_CHEST_BOMB, true);
                    Talk(SAY_ANNOUNCE_CHEST_BOMB, rider);
                }

                if (_stageTwo)
                {
                    // Wowpedia: "Helix then immediately picks a new player and leaps on them"
                    RideFace(PickFaceTarget(last));
                    return;
                }

                // back onto the Oaf (CPP :796-801 casts 52391 on him)
                Creature* oaf = instance ? ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_OAF)) : nullptr;
                if (oaf && oaf->IsAlive())
                {
                    me->AttackStop();
                    _reboardPending = true;
                    me->EnterVehicle(oaf, 0, true);
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                    events.ScheduleEvent(EVENT_REBOARD_CHECK, 2000);
                    TC_LOG_INFO("scripts", "[DM-HELIX] back on the Oaf");
                }
            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                // #667 w68: take his crew and bombs with him (BossAI::_JustDied does the same on death). Before, every wipe left
                // the four Helix' Crew (TEMPSUMMON_CORPSE_DESPAWN) on the log; each re-engage added four more and the w68 pull
                // ran with 10 crews (49136 guids 484, 537, 647, 817, 1797-1799, 1808, 1810 in the helix leg log).
                summons.DespawnAll();
                me->DespawnOrUnsummon();
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }
            
            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH);

                if (Creature* Oaf = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_OAF)))
                    me->Kill(Oaf);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                // #681: not while he is getting back onto the Oaf after a Throw Helix (EndRideFace); EVENT_REBOARD_CHECK ends the wait
                if (me->GetVehicle())
                    _reboardPending = false;
                if (!me->GetVehicle() && !_reboardPending)
                    if (me->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE))
                    {
                        //Talk(SAY_OAF_DEAD);
                        me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                        return;
                    }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_STICKY_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                me->SummonCreature(NPC_STICKY_BOMB, 
                                    target->GetPositionX(),
                                    target->GetPositionY(),
                                    target->GetPositionZ(),
                                    0.0f);
                            }
                            events.ScheduleEvent(EVENT_STICKY_BOMB, urand(8000, 10000));
                            break;
                        case EVENT_HELIX_CREW:
                            for (uint8 i = 0; i < 4; i++)
                                me->SummonCreature(NPC_HELIX_CREW, helixcrewPos[i], TEMPSUMMON_CORPSE_DESPAWN);
                            break;
                        case EVENT_HELIX_CREW_2:
                            for (uint8 i = 0; i < 4; i++)
                                me->SummonCreature(NPC_HELIX_CREW, helixSecondCrewPos[i], TEMPSUMMON_CORPSE_DESPAWN);
                            break;
                        case EVENT_THROW_HELIX:
                            if (!_stageTwo && !_faceTarget)
                            {
                                if (Unit* target = PickFaceTarget(ObjectGuid::Empty))
                                {
                                    Talk(SAY_THROW_HELIX);
                                    RideFace(target);
                                }
                            }
                            events.ScheduleEvent(EVENT_THROW_HELIX, 52000);
                            break;
                        case EVENT_LEAP:
                            if (!_faceTarget)
                                RideFace(PickFaceTarget(ObjectGuid::Empty));
                            break;
                        case EVENT_RIDE_FACE_END:
                            EndRideFace();
                            break;
                        case EVENT_REBOARD_CHECK:
                            if (_reboardPending && !me->GetVehicle())
                                TC_LOG_INFO("scripts", "[DM-HELIX] could not get back onto the Oaf - fights on foot");
                            _reboardPending = false;
                            break;
                    }
                }

                if (!me->GetVehicle())
                    DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_helix_gearbreakerAI>(creature);
        }
};

class npc_lumbering_oaf : public CreatureScript
{
    public:
        npc_lumbering_oaf() : CreatureScript("npc_lumbering_oaf") { }

        struct npc_lumbering_oafAI : public ScriptedAI
        {
            npc_lumbering_oafAI(Creature* c) : ScriptedAI(c) { }

            EventMap events;
            InstanceScript* instance;
            bool m_fake;

            void InitializeAI() override
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_CONFUSE, true);
                me->setActive(true);
                instance = me->GetInstanceScript();
                Reset();
            }

            void Reset() override
            {
                events.Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                m_fake = false;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED | UNIT_FLAG_NON_ATTACKABLE);
                me->SetStandState(UNIT_STAND_STATE_STAND);
                me->SetReactState(REACT_AGGRESSIVE);
            }
     
            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_CHARGE:
                        events.ScheduleEvent(EVENT_CHARGE_OAF1, 500);
                        break;
                }
            }

            // #667: Helix (the Oaf's vehicle accessory, vehicle_template_accessory 47297 -> 47296, summoned by the Oaf in
            // Vehicle::InstallAccessory) only engages through his own line-of-sight aggro: nothing hands a vehicle's combat to its
            // passenger (CombatManager.cpp has no vehicle code; ThreatManager.cpp:380 sends a passenger's threat to the vehicle).
            // His stage one (sticky bombs, heroic crew + chest bomb), the encounter state and his unflag in UpdateAI all hang off
            // his JustEngagedWith / victim, and his SummonedCreatureDies(Oaf) never fires (the Oaf is not his summon and never
            // dies, see DamageTaken). Wowpedia "Helix Gearbreaker" (MediaWiki API, 2026-10-14): stage one is Helix on the Oaf
            // throwing bombs, stage two starts when the Oaf dies.
            Creature* GetHelix() const
            {
                return instance ? ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_HELIX)) : nullptr;
            }

            void EngageHelix(Unit* who)
            {
                Creature* helix = GetHelix();
                if (!who || !helix || !helix->IsAlive() || helix->IsInCombat())
                    return;

                helix->GetThreatManager().AddThreat(who, 0.0f, nullptr, true, true);
            }

            void JustEngagedWith(Unit* who) override
            {
                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                EngageHelix(who);

                events.ScheduleEvent(EVENT_CHARGE_OAF0, 10000);
            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                // #667 w68: a wipe resets the whole encounter. Helix is a NON-minion accessory (vehicle_template_accessory 47297 ->
                // 47296, minion 0), so the Oaf's despawn below only drops him off and the respawned Oaf seats a NEW Helix
                // (Vehicle::InstallAllAccessories). A Helix released by the Oaf's fake death (DamageTaken) stayed behind attackable:
                // the w68 pull had two Helixes (47296:769 free, 47296:1752 on the Oaf), double sticky / chest bombs, and the bots
                // spent the pull on the stray one (7% at the wipe). Send the current Helix away with his crew first.
                if (Creature* helix = GetHelix())
                    if (helix->IsAlive() && helix->IsAIEnabled && helix->AI())
                        helix->AI()->EnterEvadeMode(EVADE_REASON_OTHER);

                Reset();
                me->GetMotionMaster()->MoveTargetedHome();
                uint32 corpseDelay = me->GetCorpseDelay();
                uint32 respawnDelay = me->GetRespawnDelay();

                me->SetCorpseDelay(1);
                me->SetRespawnDelay(29);

                me->DespawnOrUnsummon();

                me->SetCorpseDelay(corpseDelay);
                me->SetRespawnDelay(respawnDelay);

                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED | UNIT_FLAG_NON_ATTACKABLE);
            }

            void DamageTaken(Unit* attacker, uint32& damage) override
            {
                if (me->GetHealth() <= damage && !m_fake)
                {
                    damage = 0;

                    if (Vehicle* vehicle = me->GetVehicleKit())
                        vehicle->RemoveAllPassengers();

                    // #667 stage two ("I didn't need him!"): what Helix's SummonedCreatureDies was meant to do - unflag him and
                    // make sure he fights (a heroic Oaf can go down to the first hit, before Helix has aggroed by himself).
                    if (Creature* helix = GetHelix())
                    {
                        if (helix->IsAlive())
                        {
                            helix->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_NON_ATTACKABLE);
                            if (attacker && !helix->IsInCombat())
                                helix->GetThreatManager().AddThreat(attacker, 0.0f, nullptr, true, true);
                            // #681: stage two (Leap, second crew, no more Oafguard)
                            if (helix->IsAIEnabled && helix->AI())
                                helix->AI()->DoAction(ACTION_OAF_DOWN);
                        }
                    }

                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
                    me->SetReactState(REACT_PASSIVE);
                    me->SetStandState(UNIT_STAND_STATE_DEAD);
                    me->AttackStop();

                    if (instance)
                        instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                    events.Reset();
                }
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                        case POINT_START:
                            events.ScheduleEvent(EVENT_CHARGE_OAF2, 500);
                            break;
                        case POINT_END:
                            events.ScheduleEvent(EVENT_CHARGE_OAF3, 500);
                            break;
                    }
                }
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
                        case EVENT_CHARGE_OAF0:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                me->SetReactState(REACT_PASSIVE);
                                DoCast(target, SPELL_CHARGE, true);
                            }
                            break;
                        case EVENT_CHARGE_OAF1:
                            me->GetMotionMaster()->MovePoint(POINT_START, lumberingoafPos[1]);
                            break;
                        case EVENT_CHARGE_OAF2:
                            me->GetMotionMaster()->MoveCharge(
                                lumberingoafPos[2].GetPositionX(),
                                lumberingoafPos[2].GetPositionY(),
                                lumberingoafPos[2].GetPositionZ(),
                                42.0f, POINT_END);
                            break;
                        case EVENT_CHARGE_OAF3:
                            DoCastAOE(SPELL_OAF_SMASH);
                            for (uint32 i = 0; i < 9; ++i)
                            {
                                Position src = { -290.447998f, -484.328003f, 49.887901f };
                                Position dst = { -289.535614f, -493.059448f, 50.117603f };
                                me->MovePosition(src, (float)rand_norm() * 2, (float)rand_norm() * M_PI * 2);
                                me->MovePosition(dst, (float)rand_norm() * 3, (float)rand_norm() * M_PI * 2);
                                if (Creature* rat = me->SummonCreature(NPC_MINE_RAT, src, TEMPSUMMON_TIMED_OR_DEAD_DESPAWN, 60000ms))
                                    rat->GetMotionMaster()->MovePoint(0, dst, false);
                            }
                            me->SetReactState(REACT_AGGRESSIVE);
                            if (me->GetVictim())
                                me->GetMotionMaster()->MoveChase(me->GetVictim());
                            me->RemoveAurasDueToSpell(SPELL_RIDE_OAF);
                            DoCastAOE(SPELL_HELIX_RIDE, true);
                            events.ScheduleEvent(EVENT_CHARGE_OAF0, 30000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_lumbering_oafAI>(creature);
        }
};

class npc_sticky_bomb : public CreatureScript
{
    public:
        npc_sticky_bomb() : CreatureScript("npc_sticky_bomb") { }

        enum iEvents
        {
            EVENT_ARMING_ARMED  = 1,
            EVENT_ARMING_ORANGE = 2,
            EVENT_ARMING_YELLOW = 3,
            EVENT_ARMING_RED    = 4,
            EVENT_BOOM          = 5,
            EVENT_CHECK_PLAYER  = 6,
        };

        enum iSpells
        {
            SPELL_ARMING_VISUAL_YELLOW         = 88315,
            SPELL_ARMING_VISUAL_RED            = 88317,
            SPELL_ARMING_VISUAL_ORANGE         = 88316,
            SPELL_STICKY_BOMB_ARMED_STATE      = 88319,
            SPELL_STICKY_BOMB_PERIODIC_TRIGGER = 88329,
            SPELL_EXPLODE                      = 88974,
        };

        struct npc_sticky_bombAI : public ScriptedAI
        {
            npc_sticky_bombAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            bool canBoom;

            void InitializeAI() override
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
                me->SetReactState(REACT_PASSIVE);
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                SetCombatMovement(false);
                canBoom = false;
            }
     
            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_ARMING_YELLOW, 1000);
            }

            ObjectGuid GetTarget()
            {
                auto threatlist = me->GetThreatManager().GetUnsortedThreatList();

                for (ThreatReference const* ref : threatlist)
                    if (Unit* target = ObjectAccessor::GetUnit((*me), ref->GetVictim()->GetGUID()))
                        if (target->GetTypeId() == TYPEID_PLAYER && me->GetExactDist2dSq(target) < 4 && !target->GetVehicle())
                            return target->GetGUID();

                return ObjectGuid::Empty;
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
                        case EVENT_ARMING_YELLOW:
                            me->CastSpell(me, canBoom ? SPELL_ARMING_VISUAL_RED : SPELL_ARMING_VISUAL_YELLOW, true);
                            events.ScheduleEvent(EVENT_ARMING_ORANGE, 2000);
                            break;
                        case EVENT_ARMING_ORANGE:
                            me->CastSpell(me, canBoom ? SPELL_ARMING_VISUAL_RED : SPELL_ARMING_VISUAL_ORANGE, true);
                            events.ScheduleEvent(EVENT_ARMING_RED, 2000);
                            break;
                        case EVENT_ARMING_RED:
                            me->CastSpell(me, SPELL_ARMING_VISUAL_RED, true);
                            events.ScheduleEvent(canBoom ? EVENT_BOOM : EVENT_ARMING_ARMED, 1500);
                            break;
                        case EVENT_ARMING_ARMED:
                            canBoom = true;
                            me->CastSpell(me, SPELL_STICKY_BOMB_ARMED_STATE, true);
                            events.ScheduleEvent(EVENT_CHECK_PLAYER, 250);
                            events.ScheduleEvent(EVENT_ARMING_YELLOW, 14000);
                            break;
                        case EVENT_BOOM:
                            // #681: the self-destruct. Wowpedia "Helix Gearbreaker" (MediaWiki API, 2026-10-14): a bomb no player
                            // triggered "will also self destruct and explode" after 18 more seconds. 88329 (Sticky Bomb Periodic
                            // Trigger, 5.4.8 SpellEffect.dbc: aura 23, every 250 ms -> 95500, 1 yd -> 88321 Explode) left the bomb
                            // standing as a permanent proximity mine (Helix's bombs have no despawn timer), so the ground filled up
                            // for the whole fight. Explode once and go, the same way as the player-triggered branch below.
                            me->CastSpell(me, SPELL_EXPLODE, true);
                            me->SetDisplayId(11686);
                            me->DespawnOrUnsummon(2000);
                            events.Reset();
                            break;
                        case EVENT_CHECK_PLAYER:
                        {
                            if (Unit* target = ObjectAccessor::GetUnit(*me, GetTarget()))
                            {
                                me->CastSpell(target, SPELL_EXPLODE, true);
                                me->SetDisplayId(11686);
                                me->DespawnOrUnsummon(2000);
                                events.Reset();
                                break;
                            }

                            events.ScheduleEvent(EVENT_CHECK_PLAYER, 250);
                            break;
                        }
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_sticky_bombAI>(creature);
        }
};

class npc_helix_crew : public CreatureScript
{
    public:
        npc_helix_crew() : CreatureScript("npc_helix_crew") { }

        struct npc_helix_crewAI : public ScriptedAI
        {
            npc_helix_crewAI(Creature* creature) : ScriptedAI(creature), summons(me) 
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
                SetCombatMovement(false);
            }

            EventMap events;
            SummonList summons;

            void Reset() override
            {
                events.Reset();  // #667 w68: see JustEngagedWith
                summons.DespawnAll();
                SetCombatMovement(false);
            }
     
            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                // #681 (owner 2026-10-22: the heroic crew joins the fight): a crew bomb arms only from npc_sticky_bombAI's combat
                // (JustEngagedWith / UpdateVictim), and nothing put it in combat - the crew's bombs lay inert for their 30 s.
                // Helix's own bombs are zoned in by BossAI::JustSummoned (ScriptedCreature.cpp:779-784); do the same here.
                if (me->IsInCombat())
                    DoZoneInCombat(summon);
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void JustEngagedWith(Unit* who) override
            {
                // #667 w68: EVENT_STICKY_BOMB re-schedules itself, and nothing reset the map on evade, so every re-engage of the same
                // crew member stacked one more bomb chain.
                events.Reset();
                // #681: CPP npc_helix_crew (boss_helix_gearbreaker.cpp:536-564): the first bomb 1 s after the crew arrives, then
                // one every 5 s per crew member (the fork threw every 15-20 s from 8 s)
                events.ScheduleEvent(EVENT_STICKY_BOMB, 1000);
            }

            void JustDied(Unit* /*killer*/) override
            {
                summons.DespawnAll();
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_STICKY_BOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                me->SummonCreature(NPC_STICKY_BOMB, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 30000ms);
                            events.ScheduleEvent(EVENT_STICKY_BOMB, 5000);
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_helix_crewAI>(creature);
        }
};

class spell_helix_chest_bomb : public SpellScriptLoader
{
    public:
        spell_helix_chest_bomb() : SpellScriptLoader("spell_helix_chest_bomb") { }


        class spell_helix_chest_bomb_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_helix_chest_bomb_AuraScript)

            void OnRemove(AuraEffect const* aurEff, AuraEffectHandleModes /*mode*/)
            {
                if (Unit* target = aurEff->GetBase()->GetUnitOwner())
                    target->CastSpell(target, SPELL_CHEST_BOMB_DMG, true);
            }

            void Register() override
            {
                OnEffectRemove += AuraEffectRemoveFn(spell_helix_chest_bomb_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
             }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_helix_chest_bomb_AuraScript();
        }
};

class spell_helix_force_player_to_ride_oaf : public SpellScriptLoader
{
    public:
        spell_helix_force_player_to_ride_oaf() : SpellScriptLoader("spell_helix_force_player_to_ride_oaf") { }


        class spell_helix_force_player_to_ride_oaf_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_helix_force_player_to_ride_oaf_SpellScript);


            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetHitUnit()->CastSpell(GetCaster(), SPELL_RIDE_OAF, true);
                GetCaster()->ToCreature()->AI()->DoAction(ACTION_CHARGE);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_helix_force_player_to_ride_oaf_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_helix_force_player_to_ride_oaf_SpellScript();
        }
};

void AddSC_boss_helix_gearbreaker()
{
    new boss_helix_gearbreaker();
    new npc_lumbering_oaf();
    new npc_helix_crew();
    new npc_sticky_bomb();
    new spell_helix_force_player_to_ride_oaf();
    new spell_helix_chest_bomb();
}