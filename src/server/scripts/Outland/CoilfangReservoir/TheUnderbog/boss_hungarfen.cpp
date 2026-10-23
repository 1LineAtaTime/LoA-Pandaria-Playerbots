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

/* ScriptData
SDName: Boss_Hungarfen
SD%Complete: 95
SDComment: Need confirmation if spell data are same in both modes. Summons should have faster rate in heroic
SDCategory: Coilfang Resevoir, Underbog
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"

enum Spells
{
    SPELL_FOUL_SPORES           = 31673,
    SPELL_ACID_GEYSER           = 38739,

    SPELL_SPORE_CLOUD           = 34168,
    SPELL_PUTRID_MUSHROOM       = 31690,
    SPELL_GROW                  = 31698
};

enum Events
{
    EVENT_MUSHROOM    = 1,
    EVENT_ACID_GEYSER = 2,
};

class boss_hungarfen : public CreatureScript
{
    public:
        boss_hungarfen() : CreatureScript("boss_hungarfen") { }

        struct boss_hungarfenAI : public ScriptedAI
        {
            boss_hungarfenAI(Creature* creature) : ScriptedAI(creature) { }

            bool Root;
            EventMap events;

            void Reset() override
            {
                events.Reset();
                Root = false;
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_MUSHROOM, 5 * IN_MILLISECONDS);
                events.ScheduleEvent(EVENT_ACID_GEYSER, 10 * IN_MILLISECONDS);
            }

            void DamageTaken(Unit* /*attacker*/, uint32& damage) override
            {
                if (HealthBelowPct(20) && !Root)
                {
                    Root = true;
                    DoCast(me, SPELL_FOUL_SPORES);
                }
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
                        case EVENT_MUSHROOM:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                                me->SummonCreature(17990, target->GetPositionX() + (rand() % 8), target->GetPositionY() + (rand() % 8), target->GetPositionZ(), float(rand() % 5), TEMPSUMMON_TIMED_DESPAWN, 30000ms); // #549: was 22 s; the mushroom now despawns itself at ~26 s (AC lifetime)
                            else
                                me->SummonCreature(17990, me->GetPositionX() + (rand() % 8), me->GetPositionY() + (rand() % 8), me->GetPositionZ(), float(rand() % 5), TEMPSUMMON_TIMED_DESPAWN, 30000ms); // #549: was 22 s; the mushroom now despawns itself at ~26 s (AC lifetime)

                            events.ScheduleEvent(EVENT_MUSHROOM, 10 * IN_MILLISECONDS);
                            break;
                        case EVENT_ACID_GEYSER:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 80.0f, true))
                                DoCast(target, SPELL_ACID_GEYSER);

                            events.ScheduleEvent(EVENT_ACID_GEYSER, 10 * IN_MILLISECONDS + urand(1 * IN_MILLISECONDS, 7.5 * IN_MILLISECONDS));
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_hungarfenAI(creature);
        }
};

class npc_underbog_mushroom : public CreatureScript
{
    public:
        npc_underbog_mushroom() : CreatureScript("npc_underbog_mushroom") { }

        // #549: ported from AC boss_hungarfen.cpp npc_underbog_mushroom (InitializeAI scheduler): Grow every 2 s,
        // Spore Cloud on the 10th Grow (repeat counter 9, ~20 s - Wowpedia "Hungarfen": "explode in a cloud of
        // spores after 20 seconds"), Grow removed 4 s later, despawn 2 s after that. The old script cast Spore Cloud
        // at spawn and grew every 3 s for only 20 s (max 7 Grow stacks).
        struct npc_underbog_mushroomAI : public ScriptedAI
        {
            npc_underbog_mushroomAI(Creature* creature) : ScriptedAI(creature) { }

            static constexpr uint32 MAX_GROW_REPEAT = 9; // AC boss_hungarfen.cpp Misc::MAX_GROW_REPEAT

            bool Stop;
            uint32 Grow_Timer;
            uint32 Grow_Count;
            uint32 Shrink_Timer;

            void Reset() override
            {
                Stop = false;
                Grow_Timer = 2000;
                Grow_Count = 0;
                Shrink_Timer = 0;

                // AC casts SPELL_SHRINK 31691 here; 31691 is not in 5.4.8 Spell.dbc, so the fork keeps its visual.
                DoCast(me, SPELL_PUTRID_MUSHROOM, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (Stop)
                    return;

                if (Shrink_Timer)
                {
                    if (Shrink_Timer <= diff)
                    {
                        me->RemoveAurasDueToSpell(SPELL_GROW);
                        me->DespawnOrUnsummon(2000);
                        Stop = true;
                    } else Shrink_Timer -= diff;
                    return;
                }

                if (Grow_Timer <= diff)
                {
                    DoCast(me, SPELL_GROW, true);
                    if (Grow_Count++ == MAX_GROW_REPEAT)
                    {
                        DoCast(me, SPELL_SPORE_CLOUD, true);
                        Shrink_Timer = 4000;
                    }
                    else
                        Grow_Timer = 2000;
                } else Grow_Timer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_underbog_mushroomAI(creature);
        }
};

class SporeCloudPredicate
{
    public:
        SporeCloudPredicate() {}

        bool operator() (WorldObject* object)
        {
            return object && !object->ToPlayer();
        }
};

// Spore Cloud 31689
class spell_hungarfen_spore_cloud : public SpellScriptLoader
{
    public:
        spell_hungarfen_spore_cloud() : SpellScriptLoader("spell_hungarfen_spore_cloud") { }

        class spell_hungarfen_spore_cloud_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_hungarfen_spore_cloud_SpellScript);

            void CheckTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if(SporeCloudPredicate());
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_hungarfen_spore_cloud_SpellScript::CheckTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_hungarfen_spore_cloud_SpellScript();
        }
};

void AddSC_boss_hungarfen()
{
    new boss_hungarfen();
    new npc_underbog_mushroom();
    new spell_hungarfen_spore_cloud();
}
