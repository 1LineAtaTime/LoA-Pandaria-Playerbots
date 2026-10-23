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
#include "halls_of_origination.h"

enum Texts
{
    SAY_DEATH   = 0,
    SAY_AGGRO   = 1,
    SAY_KILL    = 2,
};

enum Spells
{
    // Setesh
    SPELL_CHAOS_BOLT            = 77069,
    // #691 (Wowpedia Setesh 2026-10-23; TC 4.3.4 TCPP boss_setesh.cpp; 5.4.8 DBC). The DBC spell chains (77023 -> 77024/77061,
    // 76674 -> 76675, 76888) summon through force-casts on players, so the script summons the npcs itself and casts the
    // npc-side spell on them.
    SPELL_REIGN_OF_CHAOS        = 77026, // on a Reign of Chaos bubble (npc 41168): 77030 every 1 s, 6000 within 2 yd
    SPELL_CHAOS_BLAST           = 76681, // on a landed Chaos Blast (npc 41041): impact + 76684 Chaos Burn every 1 s
    SPELL_SEED_OF_CHAOS         = 76870, // a Chaos Seed (npc 41126) bursts: 3.2k within 15 yd + the 15 s chaos infusion
    SPELL_SEED_OF_CHAOS_DUMMY   = 76865, // the seed's visual (creature_template_addon)

    // Void Sentinel
    SPELL_VOID_BARRIER          = 76959,
    SPELL_CHARGED_FISTS         = 77238,

    // Void Seeker
    SPELL_ANTI_MAGIC_PRISON     = 76903,
    SPELL_SHADOW_BOLT_VOLLEY    = 76146, //r
};

enum NPCs
{
    NPC_VOID_SENTINEL       = 41208,
    NPC_VOID_SEEKER         = 41371,
    NPC_VOID_WURM           = 41374,
    NPC_CHAOS_PORTAL        = 41055,
    NPC_REIGN_OF_CHAOS      = 41168, // 77026
    NPC_VOID_LORD           = 41364, // 77458
    NPC_CHAOS_BLAST         = 41041, // 76681
    NPC_CHAOS_SEED          = 41126, // 76870
};

enum SeteshMisc
{
    FACTION_SETESH_HOSTILE  = 14,    // TCPP npc_setesh_chaos_seed: the seed turns hostile to burst
    REIGN_BUBBLES           = 16,    // 77023: 8 s of 77024 every 0.5 s (DBC)
    REIGN_BUBBLE_MS         = 5000,  // Wowpedia: "Each bubble lasts 5 seconds before it pops"
    CHAOS_BLAST_LAND_MS     = 5000,  // Wowpedia: "After 5 seconds the Chaos Blast lands"
    CHAOS_BLAST_MS          = 50000, // 76675 (summons 41041) duration
    CHAOS_SEED_MS           = 120000,
};

// The middle of the ring Setesh walks (movepos below) and the floor height
Position const SeteshRoomCenter = { -517.7f, 10.5f, 343.93f, 0.0f };

// Ground effects summoned by Setesh: held by the encounter's summon list (despawned on reset/death) but never put in combat,
// so their default AI does not chase anybody (BossAI::JustSummoned would DoZoneInCombat them).
static void SeteshPinSummon(Creature* summon)
{
    summon->SetReactState(REACT_PASSIVE);
    summon->AttackStop();
    summon->StopMoving();
    summon->SetControlled(true, UNIT_STATE_ROOT);
}

enum Events
{
    EVENT_CHAOS_BOLT            = 1,
    EVENT_SUMMON_CHAOS_PORTAL   = 2,

    EVENT_SUMMON_6              = 3,
    EVENT_SUMMON_10             = 4,
    EVENT_SUMMON_12             = 5,
    EVENT_SUMMON_15             = 6,

    EVENT_ANTI_MAGIC_PRISON     = 7,
    EVENT_SHADOW_BOLT_VOLLEY    = 8,

    EVENT_VOID_BARRIER          = 9,
    EVENT_CHARGED_FISTS         = 10,

    EVENT_MOVE                  = 11,

    EVENT_CHAOS_BLAST           = 12,
    EVENT_SEED_OF_CHAOS         = 13,
    EVENT_REIGN_OF_CHAOS        = 14,
    EVENT_REIGN_BUBBLE          = 15,
};

enum SeteshSummonTypes
{
    SETESH_SUMMON_WURM      = 1,
    SETESH_SUMMON_SENTINEL  = 2,
    SETESH_SUMMON_SEEKER    = 3,
};

const Position movepos[9] =
{
    { -481.55f,  14.15f, 343.92f, 2.07f },
    { -490.31f,  25.55f, 343.93f, 2.56f },
    { -508.35f,  30.95f, 343.94f, 3.08f },
    { -524.62f,  30.30f, 343.93f, 3.35f },
    { -534.63f,  22.76f, 343.92f, 4.08f },
    { -539.40f,  9.067f, 343.92f, 4.67f },
    { -537.78f, -3.28f,  343.92f, 5.11f },
    { -528.39f, -16.43f, 343.93f, 5.85f },
    { -513.85f, -19.04f, 343.93f, 6.15f }
};

class boss_setesh : public CreatureScript
{
    public:
        boss_setesh() : CreatureScript("boss_setesh") { }

        struct boss_seteshAI : public BossAI
        {
            boss_seteshAI(Creature* creature) : BossAI(creature, DATA_SETESH)
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
            }

            uint8 reignBubblesLeft = 0;

            void Reset() override
            {
                _Reset();
                reignBubblesLeft = 0;

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk (SAY_AGGRO);
                events.ScheduleEvent(EVENT_CHAOS_BOLT, 10000);
                events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, 20000);
                events.ScheduleEvent(EVENT_MOVE, 2000);
                // #691: TCPP boss_setesh.cpp JustEngagedWith timers (Chaos Blast 15 s / 20 s, Seed of Chaos 20 s / 40 s, Reign of
                // Chaos first at 30 s; TCPP never repeats it - 40-45 s here, the portal cadence).
                events.ScheduleEvent(EVENT_CHAOS_BLAST, 15000);
                events.ScheduleEvent(EVENT_SEED_OF_CHAOS, 20000);
                events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, 30000);

                me->SetReactState(REACT_PASSIVE);

                DoZoneInCombat();
            }

            void JustSummoned(Creature* summon) override
            {
                switch (summon->GetEntry())
                {
                    case NPC_REIGN_OF_CHAOS:
                    case NPC_CHAOS_BLAST:
                    case NPC_CHAOS_SEED:
                        summons.Summon(summon);
                        SeteshPinSummon(summon);
                        break;
                    default:
                        BossAI::JustSummoned(summon);
                        break;
                }
            }

            // Wowpedia Reign of Chaos: "bubbles of void energy every 0.5 second at the location of players and random spots in
            // the room"; 77024 (random spot within 15 yd of Setesh) and 77061 -> 77060 (on the players) alternate here.
            void SummonReignBubble(bool onPlayer)
            {
                Position pos;
                if (Unit* target = onPlayer ? SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true) : nullptr)
                    pos = target->GetPosition();
                else
                    pos = me->GetRandomPoint(me->GetPosition(), 15.0f);

                if (Creature* bubble = me->SummonCreature(NPC_REIGN_OF_CHAOS, pos, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(REIGN_BUBBLE_MS)))
                    bubble->CastSpell(bubble, SPELL_REIGN_OF_CHAOS, true);
            }

            // Wowpedia Chaos Blast: thrown at a random player at least 15 yd away (TCPP spell_setesh_chaos_blast filter), lands
            // 5 s later and leaves the void zone.
            void ThrowChaosBlast()
            {
                Creature* self = me;
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [self](Unit* unit)
                {
                    return unit->GetTypeId() == TYPEID_PLAYER && !self->IsWithinDist(unit, 15.0f);
                });
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                if (!target)
                    return;

                if (Creature* blast = me->SummonCreature(NPC_CHAOS_BLAST, target->GetPosition(), TEMPSUMMON_TIMED_DESPAWN, Milliseconds(CHAOS_BLAST_MS)))
                    blast->m_Events.Schedule(CHAOS_BLAST_LAND_MS, [blast]() { blast->CastSpell(blast, SPELL_CHAOS_BLAST, true); });
            }

            // Wowpedia Seed of Chaos: "a Seed of Chaos at a random location in his room ... Moving close to the seed causes it
            // to explode". 76888 drops it 7-60 yd away (DBC); here within 20 yd of the middle of the room. TCPP gives the seed a
            // 7 yd combat reach and bursts it on aggro: the seed bursts on the first player within 7 yd.
            void PlantChaosSeed()
            {
                Position pos = me->GetRandomPoint(SeteshRoomCenter, 20.0f);
                Creature* seed = me->SummonCreature(NPC_CHAOS_SEED, pos, TEMPSUMMON_TIMED_DESPAWN, Milliseconds(CHAOS_SEED_MS));
                if (!seed)
                    return;

                seed->m_Events.Repeated(1000, 500, [seed]()
                {
                    if (!seed->SelectNearestPlayer(7.0f))
                        return false;

                    seed->SetFaction(FACTION_SETESH_HOSTILE);
                    seed->RemoveAurasDueToSpell(SPELL_SEED_OF_CHAOS_DUMMY);
                    seed->CastSpell(seed, SPELL_SEED_OF_CHAOS, true);
                    seed->DespawnOrUnsummon(2000);
                    return true;
                });
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                        case 1:
                            events.ScheduleEvent(EVENT_MOVE, 1000);
                            break;
                    }
                }
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
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHAOS_BOLT:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_CHAOS_BOLT);
                            events.ScheduleEvent(EVENT_CHAOS_BOLT, urand(5000, 10000));
                            break;
                        case EVENT_SUMMON_CHAOS_PORTAL:
                            me->SummonCreature(NPC_CHAOS_PORTAL,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ(),
                                me->GetOrientation(),
                                IsHeroic()? TEMPSUMMON_DEAD_DESPAWN: TEMPSUMMON_TIMED_DESPAWN,
                                Milliseconds(IsHeroic()? 0: 35000));
                            events.ScheduleEvent(EVENT_SUMMON_CHAOS_PORTAL, urand(40000, 45000));
                            break;
                        case EVENT_MOVE:
                            me->GetMotionMaster()->MovePoint(1, movepos[urand(0, 8)]);
                            break;
                        case EVENT_CHAOS_BLAST:
                            ThrowChaosBlast();
                            events.ScheduleEvent(EVENT_CHAOS_BLAST, 20000);
                            break;
                        case EVENT_SEED_OF_CHAOS:
                            PlantChaosSeed();
                            events.ScheduleEvent(EVENT_SEED_OF_CHAOS, 40000);
                            break;
                        case EVENT_REIGN_OF_CHAOS:
                            reignBubblesLeft = REIGN_BUBBLES;
                            events.ScheduleEvent(EVENT_REIGN_BUBBLE, 500);
                            events.ScheduleEvent(EVENT_REIGN_OF_CHAOS, urand(40000, 45000));
                            break;
                        case EVENT_REIGN_BUBBLE:
                            if (reignBubblesLeft)
                            {
                                SummonReignBubble(reignBubblesLeft % 2 == 0);
                                if (--reignBubblesLeft)
                                    events.ScheduleEvent(EVENT_REIGN_BUBBLE, 500);
                            }
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_seteshAI>(creature);
        }
};

class npc_setesh_chaos_portal : public CreatureScript
{
    public:
        npc_setesh_chaos_portal() : CreatureScript("npc_setesh_chaos_portal") { }

        struct npc_setesh_chaos_portalAI : public ScriptedAI
        {
            npc_setesh_chaos_portalAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            InstanceScript* instance;
            EventMap events;

            void SeteshSummon(SeteshSummonTypes type)
            {
                if (Creature* pSetesh = instance->instance->GetCreature(instance->GetGuidData(DATA_SETESH)))
                {    
                    switch (type)
                    {
                        case SETESH_SUMMON_WURM:
                            pSetesh->SummonCreature(NPC_VOID_WURM,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ(),
                                me->GetOrientation());
                            pSetesh->SummonCreature(NPC_VOID_WURM,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ(),
                                me->GetOrientation());
                            break;
                        case SETESH_SUMMON_SENTINEL:
                            pSetesh->SummonCreature(NPC_VOID_SENTINEL,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ(),
                                me->GetOrientation());
                            break;
                        case SETESH_SUMMON_SEEKER:
                            pSetesh->SummonCreature(NPC_VOID_SEEKER,
                                me->GetPositionX(),
                                me->GetPositionY(),
                                me->GetPositionZ(),
                                me->GetOrientation());
                            break;
                    }
                }
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);

                if (!IsHeroic())
                {
                    me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                    SeteshSummon(SETESH_SUMMON_WURM);
                    events.ScheduleEvent(EVENT_SUMMON_12, 12000);
                }
                else
                {
                    me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_PACIFIED);
                    SeteshSummon(SETESH_SUMMON_SENTINEL);
                    // #691 Wowpedia Setesh (heroic): "a Void Sentinel appears then two Void Wurms appear six seconds later"
                    events.ScheduleEvent(EVENT_SUMMON_6, 6000);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                events.Update(diff);

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_SUMMON_6:
                            SeteshSummon(SETESH_SUMMON_WURM);
                            events.ScheduleEvent(EVENT_SUMMON_10, 10000);
                            break;
                        case EVENT_SUMMON_10:
                            SeteshSummon(SETESH_SUMMON_SEEKER);
                            events.ScheduleEvent(EVENT_SUMMON_15, 15000);
                            break;
                        case EVENT_SUMMON_12:
                            SeteshSummon(SETESH_SUMMON_SEEKER);
                            events.ScheduleEvent(EVENT_SUMMON_15, 15000);   
                            break;
                        case EVENT_SUMMON_15:
                            if (IsHeroic())
                            {
                                // #691 Wowpedia / TCPP EVENT_SUMMON_WAVE_RANDOM: "two Void Wurms or one Void Seeker"
                                if (urand(0, 1))
                                    SeteshSummon(SETESH_SUMMON_WURM);
                                else
                                    SeteshSummon(SETESH_SUMMON_SEEKER);
                                events.ScheduleEvent(EVENT_SUMMON_15, 15000);
                            }
                            else
                            {
                                SeteshSummon(SETESH_SUMMON_SENTINEL);
                                me->DespawnOrUnsummon();
                            }
                            break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_setesh_chaos_portalAI>(creature);
        }
};

class npc_setesh_void_sentinel : public CreatureScript
{
    public:
        npc_setesh_void_sentinel() : CreatureScript("npc_setesh_void_sentinel") { }

        struct npc_setesh_void_sentinelAI : public ScriptedAI
        {
            npc_setesh_void_sentinelAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            // #691: both events were handled but never scheduled. Wowpedia Setesh: Charged Fists (+10% physical damage, +20% attack
            // speed for 15 s); Void Barrier (-90% damage taken) is a channel with no end (Wowhead 76959 "Channeled", DBC duration
            // -1): it holds until interrupted, then comes back.
            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_CHARGED_FISTS, urand(5000, 8000));
                events.ScheduleEvent(EVENT_VOID_BARRIER, urand(10000, 12000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CHARGED_FISTS:
                            DoCast(me, SPELL_CHARGED_FISTS);
                            events.ScheduleEvent(EVENT_CHARGED_FISTS, urand(18000, 22000));
                            break;
                        case EVENT_VOID_BARRIER:
                            DoCast(me, SPELL_VOID_BARRIER);
                            events.ScheduleEvent(EVENT_VOID_BARRIER, urand(20000, 25000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_setesh_void_sentinelAI>(creature);
        }
};

class npc_setesh_void_seeker : public CreatureScript
{
    public:
        npc_setesh_void_seeker() : CreatureScript("npc_setesh_void_seeker") { }

        struct npc_setesh_void_seekerAI : public ScriptedAI
        {
            npc_setesh_void_seekerAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                if (urand(0, 1))
                    events.ScheduleEvent(EVENT_ANTI_MAGIC_PRISON, urand(3000, 5000));
                else
                    events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(3000, 5000));
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                    
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while(uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_ANTI_MAGIC_PRISON:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                            DoCast(target, SPELL_ANTI_MAGIC_PRISON);
                        events.ScheduleEvent(EVENT_ANTI_MAGIC_PRISON, urand(31000, 33000));
                        break;
                    case EVENT_SHADOW_BOLT_VOLLEY:
                        DoCastAOE(SPELL_SHADOW_BOLT_VOLLEY);
                        events.ScheduleEvent(EVENT_SHADOW_BOLT_VOLLEY, urand(9000, 13000));
                        break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_setesh_void_seekerAI>(creature);
        }
};

void AddSC_boss_setesh()
{
    new boss_setesh();
    new npc_setesh_chaos_portal();
    new npc_setesh_void_sentinel();
    new npc_setesh_void_seeker();
}
