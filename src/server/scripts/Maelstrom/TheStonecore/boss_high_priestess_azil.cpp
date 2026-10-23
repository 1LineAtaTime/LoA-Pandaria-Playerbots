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
#include "the_stonecore.h"

enum ScriptTexts
{
    SAY_AGGRO    = 3,
    SAY_DEATH    = 2,
    SAY_SKILL    = 1,
};

// #701 Force Grip and Seismic Shard. Wowpedia MediaWiki API "High Priestess Azil" (read 2026-10-13):
//   "Force Grip: High Priestess Azil traps her current enemy target in a field of energy for 5 sec, repeatedly lifting
//   them up and then slamming them into the ground. Each time the target is slammed into the ground, they are dealt
//   18850 to 21150 Physical damage." (interruptible; strategy: "DPS are encouraged to interrupt Force Grip").
//   "Seismic Shard: High Priestess Azil magically raises up a huge shard of stone, then throws it at the location of a
//   random enemy player. When the shard lands it inflicts 32987 to 37012 damage to all units within 3.5 yards." She casts
//   it three times in the Energy Shield phase, and "a large cloud of dust will appear on the ground below the targeted
//   player who should move away"; heroic "will one shot most players".
// Wowhead Cata tooltip API: 79351 Force Grip "10 yd range, 1.5 sec cast, 6 sec cooldown ... Each time the target is
//   slammed into the ground they take 70687 to 79312 damage" (heroic row); 79002 / 79021 Seismic Shard "hurls it towards
//   a random player's location ... any friendly or enemy units within 3.5 yards will take 942500 to 1057500 damage".
// DBC: 79351 = 1.5 s cast, SCRIPT_EFFECT (79354 vehicle-lift, not driven by the core) + dummy auras; 79358 = the slam
//   damage (SCHOOL_DAMAGE 18850+2301, heroic 70687+8626) with the same script effect; 79021 = the impact (3.5 yd, normal
//   32987+4026, heroic 942500+115001), cast at a destination; NPC 42355 "Seismic Shard" (fly, aura 79009, no script) is the
//   rock. The vehicle lift is done here as a vertical knock-up each second.
enum Spells
{
    SPELL_FORCE_GRIP                = 79351,
    SPELL_FORCE_GRIP_SLAM           = 79358,
    SPELL_SEISMIC_SHARD_IMPACT      = 79021,
    SPELL_CURSE_OF_BLOOD            = 79345,
    SPELL_ARCANE_SHIELD             = 79050,
    SPELL_SUMMON_GRAVITY_WELL       = 79340,
    SPELL_GRAVITY_WELL_AURA_0       = 79245,
    SPELL_GRAVITY_WELL_AURA_1       = 79244,
    SPELL_GRAVITY_WELL_AURA_1_T     = 79251,
    SPELL_GRAVITY_WELL_AURA_MOD     = 92475,
    SPELL_GRAVITY_WELL_AURA_DMG     = 79249,
};

enum Events
{
    EVENT_CURSE_OF_BLOOD    = 1,
    EVENT_SHIELD            = 2,
    EVENT_GRAVITY_WELL      = 3,
    EVENT_GRAVITY_WELL_1    = 4,
    EVENT_SUMMON_ADDS       = 5,
    EVENT_MOVE              = 6,
    EVENT_MOVE_2            = 7,
    EVENT_FORCE_GRIP        = 8,
    EVENT_FORCE_GRIP_SLAM   = 9,
    EVENT_SEISMIC_SHARD     = 10,
};

enum Adds
{
    NPC_ADVOUT_FOLLOWER     = 42428,
    NPC_GRAVITY_WELL        = 42499,
    NPC_SEISMIC_SHARD       = 42355,
};

enum ShardData
{
    SHARD_COUNT             = 3,       // "cast Seismic Shard three times"
    SHARD_INTERVAL          = 3500,
    SHARD_FLIGHT_MS         = 3000,    // the 3 s of the Seismic Shard cast: the dust cloud warning
    FORCE_GRIP_SLAMS        = 5,       // 5 s, one slam per second
};

enum
{
    WORLD_STATE_ROTTEN_TO_THE_CORE = 8336,
};

Position highpriestessazilAddsPos[2] = 
{
    {1263.20f, 966.03f, 205.81f, 0.0f},
    {1278.51f, 1021.72f, 209.08f, 0.0f}
};

Position highpriestessazilStandPos = {1337.79f, 963.39f, 214.18f, 1.8f};

class boss_high_priestess_azil : public CreatureScript
{
    public:
        boss_high_priestess_azil() : CreatureScript("boss_high_priestess_azil") { }

        struct boss_high_priestess_azilAI : public BossAI
        {
            boss_high_priestess_azilAI(Creature* creature) : BossAI(creature, DATA_HIGH_PRIESTESS_AZIL), summons(me)
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

            EventMap events;
            SummonList summons;
            ObjectGuid gripTarget;
            uint8 gripSlams = 0;
            uint8 shardsThrown = 0;

            void Reset() override
            {
                _Reset();
                gripTarget.Clear();
                gripSlams = 0;
                shardsThrown = 0;

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                summons.DespawnAll();
                events.Reset();

                me->GetMap()->SetWorldState(WORLD_STATE_ROTTEN_TO_THE_CORE, 0);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
                switch (summon->GetEntry())
                {
                    case NPC_ADVOUT_FOLLOWER:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                        {
                            summon->GetThreatManager().AddThreat(target, 10.0f);
                            summon->Attack(target, true);
                            summon->GetMotionMaster()->MoveChase(target);
                        }
                        break;
                }
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_SHIELD, 35000);
                events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 3000);
                events.ScheduleEvent(EVENT_SUMMON_ADDS, urand(5000, 7000));
                events.ScheduleEvent(EVENT_GRAVITY_WELL, 10000);
                events.ScheduleEvent(EVENT_FORCE_GRIP, urand(8000, 12000));

                me->GetMap()->SetWorldState(WORLD_STATE_ROTTEN_TO_THE_CORE, 1);
            }

            // Force Grip landed (the 1.5 s cast was not interrupted): five slams, one a second
            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                if (spell->Id != SPELL_FORCE_GRIP || !target)
                    return;

                gripTarget = target->GetGUID();
                gripSlams = 0;
                events.ScheduleEvent(EVENT_FORCE_GRIP_SLAM, 1000);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                summons.DespawnAll();
                Talk(SAY_DEATH);

                me->GetMap()->SetWorldState(WORLD_STATE_ROTTEN_TO_THE_CORE, 0);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                        case 1:
                            events.ScheduleEvent(EVENT_MOVE_2, 20000);
                            // on the altar: three Seismic Shards
                            shardsThrown = 0;
                            events.ScheduleEvent(EVENT_SEISMIC_SHARD, 1500);
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
                        case EVENT_SUMMON_ADDS:
                            for (uint8 i = 0; i < 3; i++)
                                me->SummonCreature(NPC_ADVOUT_FOLLOWER, highpriestessazilAddsPos[urand(0,1)]);
                            events.ScheduleEvent(EVENT_SUMMON_ADDS, urand(5000, 7000));
                            break;
                        case EVENT_CURSE_OF_BLOOD:
                            DoCast(SPELL_CURSE_OF_BLOOD);
                            events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 15000);
                            break;
                        case EVENT_GRAVITY_WELL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM))
                                DoCast(target, SPELL_SUMMON_GRAVITY_WELL);
                            events.ScheduleEvent(EVENT_GRAVITY_WELL, 15000);
                            break;
                        case EVENT_FORCE_GRIP:
                            // stage one only: her current target, interruptible 1.5 s cast
                            if (Unit* victim = me->GetVictim())
                                DoCast(victim, SPELL_FORCE_GRIP);
                            events.ScheduleEvent(EVENT_FORCE_GRIP, urand(14000, 18000));
                            break;
                        case EVENT_FORCE_GRIP_SLAM:
                        {
                            Unit* victim = ObjectAccessor::GetUnit(*me, gripTarget);
                            if (!victim || !victim->IsAlive())
                                break;

                            // lifted and slammed: the damage of 79358, the lift as a vertical knock-up
                            DoCast(victim, SPELL_FORCE_GRIP_SLAM, true);
                            victim->KnockbackFrom(victim->GetPositionX(), victim->GetPositionY(), 0.0f, 6.0f);
                            if (++gripSlams < FORCE_GRIP_SLAMS)
                                events.ScheduleEvent(EVENT_FORCE_GRIP_SLAM, 1000);
                            break;
                        }
                        case EVENT_SEISMIC_SHARD:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 200.0f, true))
                                ThrowSeismicShard(target);
                            if (++shardsThrown < SHARD_COUNT)
                                events.ScheduleEvent(EVENT_SEISMIC_SHARD, SHARD_INTERVAL);
                            break;
                        case EVENT_SHIELD:
                            Talk(SAY_SKILL);
                            SetCombatMovement(false);
                            events.CancelEvent(EVENT_CURSE_OF_BLOOD);
                            events.CancelEvent(EVENT_FORCE_GRIP);
                            events.CancelEvent(EVENT_FORCE_GRIP_SLAM);
                            DoCast(me, SPELL_ARCANE_SHIELD);
                            events.ScheduleEvent(EVENT_MOVE, 2000);
                            break;
                        case EVENT_MOVE:
                            me->GetMotionMaster()->MovePoint(1, highpriestessazilStandPos);
                            break;
                        case EVENT_MOVE_2:
                            me->RemoveAurasDueToSpell(SPELL_ARCANE_SHIELD);
                            SetCombatMovement(true);
                            if (me->GetVictim())
                                me->GetMotionMaster()->MoveChase(me->GetVictim());
                            events.ScheduleEvent(EVENT_CURSE_OF_BLOOD, 3000);
                            events.ScheduleEvent(EVENT_FORCE_GRIP, urand(6000, 10000));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            // the rock leaves the altar and lands on the player's position of this moment 3 s later (the dust cloud);
            // the impact is 79021 at that point, cast by the shard NPC 42355
            void ThrowSeismicShard(Unit* target)
            {
                Position const aim = target->GetPosition();
                TempSummon* shard = me->SummonCreature(NPC_SEISMIC_SHARD, me->GetPositionX(), me->GetPositionY(), me->GetPositionZ() + 4.0f,
                    0.0f, TEMPSUMMON_TIMED_DESPAWN, 8000ms);
                if (!shard)
                    return;

                float const flat = shard->GetExactDist2d(aim.GetPositionX(), aim.GetPositionY());
                shard->GetMotionMaster()->MoveJump(aim.GetPositionX(), aim.GetPositionY(), aim.GetPositionZ(),
                    std::max(1.0f, flat / (SHARD_FLIGHT_MS / 1000.0f)), 6.0f);

                shard->m_Events.Schedule(uint32(SHARD_FLIGHT_MS), 1, [shard, aim]()
                {
                    shard->CastSpell(aim.GetPositionX(), aim.GetPositionY(), aim.GetPositionZ(), SPELL_SEISMIC_SHARD_IMPACT, true);
                    shard->DespawnOrUnsummon(500);
                });
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_high_priestess_azilAI>(creature);
        }
};

class npc_gravity_well : public CreatureScript
{
    public:
        npc_gravity_well() : CreatureScript("npc_gravity_well") { }

        struct npc_gravity_wellAI : public ScriptedAI
        {
            npc_gravity_wellAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint32 uidespawnTimer;

            void Reset() override
            {
                uidespawnTimer = 20000;
                events.ScheduleEvent(EVENT_GRAVITY_WELL_1, 3000);
                DoCast(me, SPELL_GRAVITY_WELL_AURA_0);
            }

            void UpdateAI(uint32 diff) override
            {
                if (uidespawnTimer <= diff)
                    me->DespawnOrUnsummon();
                else
                    uidespawnTimer -= diff;

                events.Update(diff);

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                    case EVENT_GRAVITY_WELL_1:
                        me->RemoveAurasDueToSpell(SPELL_GRAVITY_WELL_AURA_0);
                        DoCast(me, SPELL_GRAVITY_WELL_AURA_1);
                        break;
                    }
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_gravity_wellAI>(creature);
        }
};

class spell_high_priestess_azil_gravity_well_script : public SpellScriptLoader
{
    public:
        spell_high_priestess_azil_gravity_well_script() : SpellScriptLoader("spell_high_priestess_azil_gravity_well_script") { }

        class spell_high_priestess_azil_gravity_well_script_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_high_priestess_azil_gravity_well_script_SpellScript);


            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster() && GetHitUnit())
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_GRAVITY_WELL_AURA_DMG, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_high_priestess_azil_gravity_well_script_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_high_priestess_azil_gravity_well_script_SpellScript();
        }
};

void AddSC_boss_high_priestess_azil()
{
    new boss_high_priestess_azil();
    new npc_gravity_well();
    new spell_high_priestess_azil_gravity_well_script();
}