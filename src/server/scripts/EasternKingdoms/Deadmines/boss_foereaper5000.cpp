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

// todo: ����������� ������ � ����� �� �������, ��������� ������ ����� �������� ���� �����

enum ScriptTexts
{
    SAY_AGGRO     = 0,
    SAY_DEATH     = 1,
    SAY_KILL      = 2,
    SAY_SPELL1    = 3,
    SAY_SPELL2    = 4,
    SAY_SPELL3    = 5,
    SAY_SPELL4    = 6,
};

enum Spells
{
    SPELL_REAPER_STRIKE        = 88490,
    SPELL_SAFETY               = 88522,
    SPELL_HARVEST              = 88495,
    SPELL_HARVEST_AURA         = 88497,
    SPELL_HARVEST_DMG          = 88501,
    SPELL_HARVEST_SWEEP        = 88521,
    SPELL_OVERDRIVE            = 88481,
    SPELL_OVERDRIVE_DMG        = 88484,
    SPELL_TARGET_BUNNY         = 71371, // rocket artillery

    // #681 heroic Molten Slag (Wowpedia "Foe Reaper 5000", MediaWiki API 2026-10-23: "Activation of the Foe Reaper 5000 causes an
    // overload in the slag pit, resulting in Molten Slags periodically being spawned"; Fixate 10 s, Molten Shield = 50% less magic
    // damage taken + 18500-21500 Fire to everyone within 10 yd every 2 s). 5.4.8 SpellEffect.dbc: 91815 eff0 aura 87 -50% (school
    // 126), eff1 periodic trigger 2 s of 91819 (radius index 13 = 10 yd); 91829 Fixate = aura 11 MOD_TAUNT, 10 s, cast BY the player
    // ON the slag (CPP boss_foe_reaper_5000.cpp:548-568).
    SPELL_MOLTEN_SHIELD        = 91815,
    SPELL_FIXATE_TAUNT         = 91829,
};

enum Adds
{
    NPC_TARGETING_BUNNY     = 47468,
    NPC_MOLTEN_SLAG         = 49229,
    NPC_PROTOTYPE_REAPER    = 49208, // 87239 91731
};

enum Events
{
    EVENT_OVERDRIVE         = 1,
    EVENT_OVERDRIVE1        = 2,
    EVENT_REAPER_STRIKE     = 3,
    EVENT_HARVEST           = 4,
    EVENT_HARVEST1          = 5,
    EVENT_MOLTEN_SLAG       = 6,   // #681
    EVENT_SLAG_FIXATE       = 7,   // #681
};

const Position defiasreaperPos[3] =
{
    { -182.742f, -565.968f, 19.389f, 3.35f},
    { -228.675f, -565.752f, 19.389f, 5.98f},
    { -201.526f, -548.737f, 51.229f, 4.43f},
};

const Position moltenslagPos[4] =
{
    { -205.582f, -572.034f, 20.97f, 1.59f},
    { -199.143f, -579.843f, 20.97f, 6.16f},
    { -206.385f, -585.898f, 20.97f, 5.17f},
    { -212.704f, -579.072f, 20.97f, 3.09f},
};

class boss_foereaper5000 : public CreatureScript
{
    public:
        boss_foereaper5000() : CreatureScript("boss_foereaper5000") { }

        struct boss_foereaper5000AI : public BossAI
        {
            boss_foereaper5000AI(Creature* creature) : BossAI(creature, DATA_FOEREAPER) { }

            ObjectGuid harvestTargetGuid = ObjectGuid::Empty;
            bool bEnrage;

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
                me->CastSpell(me, SPELL_OFF_LINE, true);
                Reset();
            }

            void Reset() override
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                bEnrage = false;
                harvestTargetGuid = ObjectGuid::Empty;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;
                
                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (HealthBelowPct(30) && !bEnrage)
                {
                    Talk(SAY_SPELL4);
                    bEnrage = true;
                    DoCast(me, SPELL_SAFETY);
                    return;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_OVERDRIVE:
                            DoCast(me, SPELL_OVERDRIVE);
                            Talk(SAY_SPELL3);
                            events.ScheduleEvent(EVENT_OVERDRIVE, urand(25000, 30000));
                            break;
                        case EVENT_REAPER_STRIKE:
                            DoCast(me->GetVictim(), SPELL_REAPER_STRIKE);
                            events.ScheduleEvent(EVENT_REAPER_STRIKE, urand(7000, 10000));
                            break;
                        case EVENT_HARVEST:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                DoCast(target, SPELL_HARVEST);
                                Talk(SAY_SPELL1);
                            }
                            events.ScheduleEvent(EVENT_HARVEST, 30000, 40000);
                            break;
                        case EVENT_MOLTEN_SLAG:
                        {
                            Position const& pos = moltenslagPos[urand(0, 3)];
                            me->SummonCreature(NPC_MOLTEN_SLAG, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms);
                            events.ScheduleEvent(EVENT_MOLTEN_SLAG, urand(21000, 22000));
                            events.RescheduleEvent(EVENT_SLAG_FIXATE, 3000);
                            break;
                        }
                        case EVENT_SLAG_FIXATE:
                            FixateSlags();
                            events.ScheduleEvent(EVENT_SLAG_FIXATE, 12000);
                            break;
                        }
                }

                DoMeleeAttackIfReady();
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    switch (pointId)
                    {
                        case 1001:
                            DoCast(me, SPELL_HARVEST_SWEEP, true);
                            me->RemoveAurasDueToSpell(SPELL_HARVEST_AURA);

                            if (harvestTargetGuid)
                                if (Creature* harvestTarget = ObjectAccessor::GetCreature(*me, harvestTargetGuid))
                                    harvestTarget->DespawnOrUnsummon();

                            if (Unit* vict = me->GetVictim())
                                me->GetMotionMaster()->MoveChase(vict);
                            break;
                    }
                }
            }

            void JustEngagedWith(Unit* /*who*/) override 
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk(SAY_AGGRO);
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_STUNNED);
                events.ScheduleEvent(EVENT_REAPER_STRIKE, urand(5000, 8000));
                events.ScheduleEvent(EVENT_OVERDRIVE, urand(10000, 15000));
                events.ScheduleEvent(EVENT_HARVEST, urand(25000, 30000));
                // #681: CPP boss_foe_reaper_5000.cpp:166-167 / :314-320 - the first slag 17 s into the fight, then one every
                // 21-22 s; each fixates a random player 3 s after it rises and every 12 s after that (:419-443)
                if (IsHeroic())
                    events.ScheduleEvent(EVENT_MOLTEN_SLAG, 17000);
                DoZoneInCombat();
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            // #681: every live slag fixates a random player in range: the player "casts" 91829 (taunt) on it, as CPP
            // spell_foe_reaper_5000_fixate does (boss_foe_reaper_5000.cpp:559-564)
            void FixateSlags()
            {
                std::list<Creature*> slags;
                GetCreatureListWithEntryInGrid(slags, me, NPC_MOLTEN_SLAG, 100.0f);
                for (Creature* slag : slags)
                {
                    if (!slag->IsAlive())
                        continue;

                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                    {
                        target->CastSpell(slag, SPELL_FIXATE_TAUNT, true);
                        slag->AI()->AttackStart(target);
                        TC_LOG_INFO("scripts", "[DM-REAPER] molten slag %u fixates %s", uint32(slag->GetGUID().GetCounter()), target->GetName().c_str());
                    }
                }
            }

            void JustSummoned(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_MOLTEN_SLAG)
                {
                    // #681: Molten Shield from the start (CPP casts it 3 s in, :434-437), then the slag joins the fight
                    BossAI::JustSummoned(summon);
                    summon->CastSpell(summon, SPELL_MOLTEN_SHIELD, true);
                    TC_LOG_INFO("scripts", "[DM-REAPER] molten slag %u rises at (%.1f, %.1f)", uint32(summon->GetGUID().GetCounter()), summon->GetPositionX(), summon->GetPositionY());
                    return;
                }

                BossAI::JustSummoned(summon);
                if (summon->GetEntry()== NPC_TARGETING_BUNNY)
                {
                    harvestTargetGuid = summon->GetGUID();
                    Talk(SAY_SPELL2);
                    me->GetMotionMaster()->MovePoint(1001, summon->GetPositionX(), summon->GetPositionY(), summon->GetPositionZ());
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH); 
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_foereaper5000AI>(creature);
        }
};

class npc_foereaper_targeting_bunny: public CreatureScript
{
    public:
        npc_foereaper_targeting_bunny() : CreatureScript("npc_foereaper_targeting_bunny") { }
     
        struct npc_foereaper_targeting_bunnyAI : public ScriptedAI
        {
            npc_foereaper_targeting_bunnyAI(Creature* creature) : ScriptedAI(creature)
            {
                SetCombatMovement(false);
            }
           
            void Reset() override
            {
                DoCast(SPELL_TARGET_BUNNY);
            }
        };
     
        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_foereaper_targeting_bunnyAI>(creature);
        }
};

void AddSC_boss_foereaper5000()
{
    new boss_foereaper5000();
    new npc_foereaper_targeting_bunny();
}