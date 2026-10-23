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
    SAY_AGGRO              = 0,
    SAY_KILL               = 1,
    SAY_SKILL              = 2,
    SAY_DEATH              = 3,
};

// #701 heroic Rupture. Wowpedia MediaWiki API "Ozruk" (read 2026-10-13): "Rupture: When Ozruk performs a Ground Slam in
// Heroic Difficulty, the ground ruptures and causes three lines of rock spikes to erupt in the direction he is facing.
// Any player caught by one of the rock spikes is dealt 141375 to 158625 Physical damage and is knocked upwards."
// Wowhead Cata tooltip API 92381: "Rupture, 0.25 sec cast. Ground Slam causes rock spikes to erupt from the ground in
// front of the caster, dealing 141375 to 158625 damage to all enemies within 3 yards of an erupting spike. This effect
// ignores armor." DBC: Ground Slam 78903 effect 2 (heroic rows) = TRIGGER_SPELL 95669, which summons the Rupture
// Controller 49597 in front of Ozruk (SummonProperties 64); 92381 = damage 141374+17251 within 3 yd of the caster
// (DEST_CASTER + DEST_AREA_ENEMY) and a knock-up (effect 98, 300). The controller had no AI; npc_ozruk_rupture_controller
// lays the three lines of spikes (one 92381 from a trigger at each spike point).
enum Spells
{
    SPELL_RUPTURE          = 92381,
    SPELL_GROUND_SLAM      = 78903,
    SPELL_SHATTER          = 78807,
    SPELL_PARALYZE         = 92426,
    SPELL_PARALYZE_DMG     = 94661,
    SPELL_ENRAGE           = 80467,
    SPELL_SHIELD           = 78835,
    SPELL_BULWARK          = 78939,
};

enum Events
{
    EVENT_GROUND_SLAM      = 1,
    EVENT_SHATTER          = 2,
    EVENT_PARALYZE         = 3,
    EVENT_SHIELD           = 4,
    EVENT_BULWARK          = 5,
};

enum RuptureData
{
    NPC_WORLD_TRIGGER          = 21252,   // "World Trigger (Not Immune PC)": the invisible caster of one spike
    RUPTURE_LINES              = 3,
    RUPTURE_STEPS              = 7,
    RUPTURE_STEP_MS            = 250,
};

class boss_ozruk : public CreatureScript
{
    public:
        boss_ozruk() : CreatureScript("boss_ozruk") { }

        struct boss_ozrukAI : public BossAI
        {
            boss_ozrukAI(Creature* creature) : BossAI(creature, DATA_OZRUK)
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
            bool bEnrage;

            void Reset() override
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                bEnrage = false;
                events.Reset();
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                events.ScheduleEvent(EVENT_SHATTER, 21000);
                events.ScheduleEvent(EVENT_GROUND_SLAM, 7000);
                events.ScheduleEvent(EVENT_BULWARK, 80000);
                events.ScheduleEvent(EVENT_SHIELD, 3000);
                Talk(SAY_AGGRO);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HealthBelowPct(20) && !bEnrage)
                {
                    DoCast(me, SPELL_ENRAGE);
                    bEnrage = true;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_GROUND_SLAM:
                            DoCast(SPELL_GROUND_SLAM);
                            events.ScheduleEvent(EVENT_GROUND_SLAM, 15000);
                            break;
                        case EVENT_SHATTER:
                            events.ScheduleEvent(EVENT_SHATTER, 20000);
                            if (IsHeroic())
                            {
                                // #701: Paralyze (92426) has a 0.6 s cast, so the Shatter DoCast in the same event failed
                                // SPELL_FAILED_SPELL_IN_PROGRESS (Spell.cpp:3463-3466) and heroic never shattered.
                                // Wowpedia "Ozruk" (2026-10-12): "After casting Paralyze, Ozruk will begin to cast
                                // Shatter". EVENT_PARALYZE runs once the Paralyze cast is over (casting blocks the loop).
                                events.RescheduleEvent(EVENT_GROUND_SLAM, 15000);
                                DoCast(SPELL_PARALYZE);
                                events.ScheduleEvent(EVENT_PARALYZE, 100);
                                break;
                            }
                            DoCast(SPELL_SHATTER);
                            break;
                        case EVENT_PARALYZE:
                            DoCast(SPELL_SHATTER);
                            break;
                        case EVENT_SHIELD:
                            DoCast(SPELL_SHIELD);
                            events.ScheduleEvent(EVENT_SHIELD, 15000);
                            break;
                        case EVENT_BULWARK:
                            DoCast(SPELL_BULWARK);
                            events.ScheduleEvent(EVENT_BULWARK, 15000);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_ozrukAI>(creature);
        }
};

// Rupture Controller 49597 (summoned by the heroic Ground Slam, in front of Ozruk): three lines of rock spikes run out
// from it in the direction Ozruk faces, one spike point every 4 yd, 250 ms apart (the 0.25 s cast of 92381).
class npc_ozruk_rupture_controller : public CreatureScript
{
    public:
        npc_ozruk_rupture_controller() : CreatureScript("npc_ozruk_rupture_controller") { }

        struct npc_ozruk_rupture_controllerAI : public ScriptedAI
        {
            npc_ozruk_rupture_controllerAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void IsSummonedBy(Unit* summoner) override
            {
                facing = summoner->GetOrientation();
                step = 0;
                timer = 0;
                running = true;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!running)
                    return;

                if (timer > diff)
                {
                    timer -= diff;
                    return;
                }

                timer = RUPTURE_STEP_MS;

                // the spike points of this step, one per line (-20, 0, +20 degrees)
                float const distance = 4.0f * (step + 1);
                for (int32 line = 0; line < RUPTURE_LINES; ++line)
                {
                    float const angle = facing + (line - 1) * 0.35f;
                    float const x = me->GetPositionX() + distance * std::cos(angle);
                    float const y = me->GetPositionY() + distance * std::sin(angle);
                    float const z = me->GetMap()->GetHeight(x, y, me->GetPositionZ() + 2.0f);

                    if (TempSummon* spike = me->SummonCreature(NPC_WORLD_TRIGGER, x, y, z > INVALID_HEIGHT ? z : me->GetPositionZ(), angle, TEMPSUMMON_TIMED_DESPAWN, 1500ms))
                    {
                        spike->SetFaction(me->GetFaction());
                        spike->CastSpell(spike, SPELL_RUPTURE, true);
                    }
                }

                if (++step >= RUPTURE_STEPS)
                {
                    running = false;
                    me->DespawnOrUnsummon(1000);
                }
            }

        private:
            float facing = 0.0f;
            uint32 step = 0;
            uint32 timer = 0;
            bool running = false;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_ozruk_rupture_controllerAI(creature);
        }
};

void AddSC_boss_ozruk()
{
    new boss_ozruk();
    new npc_ozruk_rupture_controller();
}