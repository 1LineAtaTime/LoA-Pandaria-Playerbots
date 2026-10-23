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
#include "zulgurub.h"

// #679: creature_text 52053 groups (same rows in the MoP data), mapped as The-Cataclysm-Preservation-Project TrinityCore
// boss_zanzil.cpp (sniff based); the old ids yelled the zombie line on death and the Berserker line on a kill.
enum ScriptTexts
{
    SAY_AGGRO                   = 0,
    SAY_ANNOUNCE_ZOMBIES        = 1,
    SAY_TROLLS                  = 2,
    SAY_ANNOUNCE_GAS            = 3,
    SAY_GAS                     = 4,
    SAY_ANNOUNCE_BERSERKER      = 5,
    SAY_BERSERK                 = 6,
    SAY_KILL                    = 7,
    SAY_DEATH                   = 8,
};

enum Spells
{
    SPELL_VOODOO_BOLT   = 96346,
    SPELL_ZANZIL_FIRE   = 96914,
    SPELL_ZANZIL_FIRE1  = 96916,
    SPELL_ZANZIL_RES1   = 96319, // zombie
    SPELL_ZANZIL_RES2   = 96316, // berserker

    SPELL_PURSUIT       = 96342,
    SPELL_THUNDERCLAP   = 96340,
    SPELL_KNOCK_AWAY    = 96341,

    // #679: Zanzil's Graveyard Gas and Terrible Tonic (Wowpedia "Zanzil (tactics)", CPP TrinityCore boss_zanzil.cpp)
    SPELL_TERRIBLE_TONIC            = 96348, // 30000 nature on a random player
    SPELL_GRAVEYARD_GAS             = 96338, // 2 s cast, 7 s self aura pulsing 96339 (script effect on the gas stalkers) every 0.5 s
    SPELL_GRAVEYARD_GAS_STALKER     = 96339,
    SPELL_GRAVEYARD_GAS_DAMAGE      = 96335, // PERIODIC_DAMAGE_PERCENT on every player: 5% / 2 s normal, 20% / 1 s heroic
    SPELL_TOXIC_GAS_SOUND           = 96500,
    SPELL_GREEN_GAS                 = 97180,
};

enum Events
{
    EVENT_ZANZIL_FIRE       = 1,
    EVENT_VOODOO_BOLT       = 2,
    EVENT_SECRET_TECHNIQUE  = 3,
    EVENT_RES_ZOMBIE        = 5,
    EVENT_RES_BERSERK       = 6,
    EVENT_TERRIBLE_TONIC    = 7,
    EVENT_GAS_DAMAGE        = 8,
    EVENT_GAS_END           = 9,
};

// Zanzil's Secret Techniques: one of three, never the same twice in a row (Wowpedia "Zanzil (tactics)").
enum SecretTechniques
{
    TECHNIQUE_NONE      = 0,
    TECHNIQUE_BERSERKER = 1, // Blue Resurrection Elixir
    TECHNIQUE_ZOMBIES   = 2, // Red Resurrection Elixir
    TECHNIQUE_GAS       = 3, // Graveyard Gas
};

const Position greenGasStalkerPos = {-11572.96f, -1272.288f, 77.63614f, 0.0f}; // CPP GreenGasStalkerPos

enum Adds
{
    NPC_ZANZIL_ZOMBIE       = 52055,
    NPC_ZANZIL_BERSERKER    = 52054,
    NPC_ZANZIL_TOXIC_GAS    = 52062,
};

const Position berserkerPos[3] = 
{
    {-11603.59f, -1233.59f, 81.40f, 5.20f},
    {-11545.00f, -1240.56f, 81.55f, 3.92f},
    {-11541.40f, -1298.15f, 85.25f, 2.33f}
};

const Position zombiePos[4] =
{
    {-11590.67f, -1255.56f, 78.15f, 0.0f},
    {-11539.32f, -1259.71f, 78.98f, 0.0f},
    {-11586.93f, -1331.94f, 79.50f, 0.0f},
    {-11617.71f, -1315.54f, 79.40f, 0.0f}
};


class boss_zanzil : public CreatureScript
{
    public:
        boss_zanzil() : CreatureScript("boss_zanzil") { }

        struct boss_zanzilAI : public BossAI
        {
            boss_zanzilAI(Creature* creature) : BossAI(creature, DATA_ZANZIL)
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

            uint8 lastTechnique = TECHNIQUE_NONE;

            void Reset() override
            {
                _Reset();
                lastTechnique = TECHNIQUE_NONE;
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRAVEYARD_GAS_DAMAGE); // an evade mid-gas must not keep killing

                //for (uint8 i = 0; i < 3; ++i)
                    //me->SummonCreature(NPC_ZANZIL_BERSERKER, berserkerPos[i]);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_VOODOO_BOLT, urand(3000, 5000));
                events.ScheduleEvent(EVENT_ZANZIL_FIRE, 6000);
                // #679: CPP TrinityCore boss_zanzil.cpp - Terrible Tonic every 10-11 s, a Secret Technique 15-16 s in and every 30 s
                events.ScheduleEvent(EVENT_TERRIBLE_TONIC, urand(10000, 11000));
                events.ScheduleEvent(EVENT_SECRET_TECHNIQUE, urand(15000, 16000));
                DoZoneInCombat();
                instance->SetBossState(DATA_ZANZIL, IN_PROGRESS);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRAVEYARD_GAS_DAMAGE);
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustSummoned(Creature* summon) override
            {
                // #679: the gas stalker (faction 14, no script) is scenery: BossAI::JustSummoned would zone it into the fight.
                if (summon->GetEntry() == NPC_ZANZIL_TOXIC_GAS)
                {
                    summons.Summon(summon);
                    summon->SetReactState(REACT_PASSIVE);
                    summon->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC | UNIT_FLAG_NOT_SELECTABLE);
                    return;
                }

                BossAI::JustSummoned(summon);
            }

            // Zanzil's Secret Techniques (Wowpedia "Zanzil (tactics)"): never the same one twice in a row (CPP picks the same way).
            uint8 PickTechnique() const
            {
                uint8 technique = uint8(urand(TECHNIQUE_BERSERKER, TECHNIQUE_GAS));
                if (technique == lastTechnique)
                    technique = technique == TECHNIQUE_GAS ? uint8(urand(TECHNIQUE_BERSERKER, TECHNIQUE_ZOMBIES)) : uint8(technique + 1);
                return technique;
            }
            
            void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
            {
                if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_VOODOO_BOLT)
                        for (uint8 i = 0; i < 3; ++i)
                            if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
                                me->InterruptSpell(CURRENT_GENERIC_SPELL);
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
                        case EVENT_VOODOO_BOLT:
                            DoCastVictim(SPELL_VOODOO_BOLT);
                            events.ScheduleEvent(EVENT_VOODOO_BOLT, urand(7000, 10000));
                            break;
                        case EVENT_ZANZIL_FIRE:
                            DoCastVictim(SPELL_ZANZIL_FIRE);
                            events.ScheduleEvent(EVENT_ZANZIL_FIRE, urand(12000, 15000));
                            break;
                        case EVENT_TERRIBLE_TONIC:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                                DoCast(target, SPELL_TERRIBLE_TONIC);
                            events.ScheduleEvent(EVENT_TERRIBLE_TONIC, urand(10000, 11000));
                            break;
                        case EVENT_SECRET_TECHNIQUE:
                        {
                            // CPP: the technique interrupts his cast and holds the other abilities back for 10-12 s
                            me->InterruptNonMeleeSpells(true);
                            me->StopMoving();
                            events.RescheduleEvent(EVENT_ZANZIL_FIRE, 10000);
                            events.RescheduleEvent(EVENT_TERRIBLE_TONIC, 12000);
                            events.RescheduleEvent(EVENT_VOODOO_BOLT, 12100);

                            uint8 const technique = PickTechnique();
                            switch (technique)
                            {
                                case TECHNIQUE_ZOMBIES:
                                    Talk(SAY_ANNOUNCE_ZOMBIES);
                                    Talk(SAY_TROLLS);
                                    DoCast(me, SPELL_ZANZIL_RES1);
                                    events.ScheduleEvent(EVENT_RES_ZOMBIE, 3000);
                                    break;
                                case TECHNIQUE_BERSERKER:
                                    Talk(SAY_ANNOUNCE_BERSERKER);
                                    Talk(SAY_BERSERK);
                                    DoCast(me, SPELL_ZANZIL_RES2);
                                    events.ScheduleEvent(EVENT_RES_BERSERK, 3000);
                                    break;
                                case TECHNIQUE_GAS:
                                    // #679 Zanzil's Graveyard Gas (Wowpedia "Zanzil (tactics)": the whole area fills with gas, every
                                    // group member needs the green cauldron - Zanzil's Cauldron of Toxic Torment 208315, 96328 = 90% less
                                    // nature damage taken for 30 s). CPP: the cast, damage on every player 9.5 s later, gone at 21 s.
                                    Talk(SAY_ANNOUNCE_GAS);
                                    Talk(SAY_GAS);
                                    // No Zanzil's Toxic Gas (52062) is spawned in the data: one green gas cloud for the visual.
                                    me->SummonCreature(NPC_ZANZIL_TOXIC_GAS, greenGasStalkerPos, TEMPSUMMON_TIMED_DESPAWN, 21500ms);
                                    DoCast(me, SPELL_GRAVEYARD_GAS);
                                    events.ScheduleEvent(EVENT_GAS_DAMAGE, 9500);
                                    events.ScheduleEvent(EVENT_GAS_END, 21000);
                                    break;
                                default:
                                    break;
                            }
                            lastTechnique = technique;
                            events.ScheduleEvent(EVENT_SECRET_TECHNIQUE, 30000);
                            break;
                        }
                        case EVENT_GAS_DAMAGE:
                            instance->DoCastSpellOnPlayers(SPELL_GRAVEYARD_GAS_DAMAGE, true);
                            break;
                        case EVENT_GAS_END:
                            instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_GRAVEYARD_GAS_DAMAGE);
                            break;
                        case EVENT_RES_ZOMBIE:
                        {
                            uint8 pos = urand(0, 3);
                            for (uint8 i = 0; i < 15; ++i)
                                me->SummonCreature(NPC_ZANZIL_ZOMBIE, zombiePos[pos]);
                            break;
                        }
                        case EVENT_RES_BERSERK:
                            me->SummonCreature(NPC_ZANZIL_BERSERKER, berserkerPos[urand(0,2)]);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_zanzilAI>(creature);
        }
};

class npc_zanzil_berserker : public CreatureScript
{
    public:
        npc_zanzil_berserker() : CreatureScript("npc_zanzil_berserker") { }

        struct npc_zanzil_berserkerAI : public ScriptedAI
        {
            npc_zanzil_berserkerAI(Creature* creature) : ScriptedAI(creature) 
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
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                me->SetSpeed(MOVE_RUN, 0.5f);
            }
            
            bool bSpell;
            uint32 spellTimer;
            uint32 pursuitTimer;

            void Reset() override
            {
                bSpell = true;
                spellTimer = 0;
                pursuitTimer = 3000;
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE | UNIT_FLAG_IMMUNE_TO_PC);
            }
            
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (!bSpell)
                {
                    if (spellTimer <= diff)
                    {
                        bSpell = true;
                        spellTimer = 12000;
                    }
                    else
                        spellTimer -= diff;
                }

                if (bSpell)
                    if (Unit* target = me->SelectNearbyTarget())
                    {
                        bSpell = false;
                        DoCast(target, urand(0, 1)? SPELL_KNOCK_AWAY: SPELL_THUNDERCLAP);
                    }

                if (pursuitTimer <= diff)
                {
                    pursuitTimer = 20000;
                    if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                        DoCast(target, SPELL_PURSUIT);
                }
                else
                    pursuitTimer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_zanzil_berserkerAI>(creature);
        }
};

class spell_zanzil_pursuit : public SpellScriptLoader
{
    public:
        spell_zanzil_pursuit() : SpellScriptLoader("spell_zanzil_pursuit") { }

        class spell_zanzil_pursuit_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_zanzil_pursuit_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                if (GetCaster()->GetEntry() == NPC_ZANZIL_BERSERKER)
                {
                    static_cast<npc_zanzil_berserker::npc_zanzil_berserkerAI*>(GetCaster()->GetAI())->DoResetThreat();
                    GetCaster()->GetThreatManager().AddThreat(GetHitUnit(), 5000000.0f);
                    GetCaster()->GetAI()->AttackStart(GetHitUnit());
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_zanzil_pursuit_SpellScript::HandleScript, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_zanzil_pursuit_SpellScript();
        }
};

class spell_zanzil_fire : public SpellScriptLoader
{
    public:
        spell_zanzil_fire() : SpellScriptLoader("spell_zanzil_fire") { }

        class spell_zanzil_fire_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_zanzil_fire_AuraScript);
            
            bool Load() override
            {
                count = 0;
                return true;
            }

            void PeriodicTick(AuraEffect const* aurEff)
            {
                if (!GetCaster())
                    return;

                // #679 (w61, intended design): the trail runs away from Zanzil in the direction he faced when the cast started
                // (Wowpedia "Zanzil (tactics)"). The patches used to be placed in front of his CURRENT facing, so the trail
                // swung after the tank he keeps turning to and a side-step never got the tank out of it. Fix the origin and the
                // direction on the first tick.
                if (!count)
                {
                    origin = GetCaster()->GetPosition();
                    direction = GetCaster()->GetOrientation();
                }

                count++;

                if (count > 6)
                {
                    GetCaster()->RemoveAurasDueToSpell(SPELL_ZANZIL_FIRE);
                    return;
                }

                Position pos = origin;
                GetCaster()->MovePosition(pos, 4.0f * count, direction - GetCaster()->GetOrientation()); // MovePosition adds the caster's orientation
                GetCaster()->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_ZANZIL_FIRE1, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_zanzil_fire_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }

        private:
            uint8 count;
            Position origin;
            float direction = 0.0f;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_zanzil_fire_AuraScript();
        }
};

class spell_frostburn_formula : public SpellScriptLoader
{
    public:
        spell_frostburn_formula() : SpellScriptLoader("spell_frostburn_formula") { }

        class spell_frostburn_formula_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_frostburn_formula_SpellScript);

            void HandleScript(SpellEffIndex effIndex)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                // #836: no SetHitDamage here. 96331 EFFECT_1 is SPELL_EFFECT_DAMAGE_FROM_MAX_HEALTH_PCT 65 (SpellEffect.dbc), handled after
                // this EFFECT_0 hook (Spell::EffectDamageFromMaxHealthPCT adds to m_damage), so setting 65% here as well made it 130%.
                GetHitUnit()->CastSpell(GetHitUnit(), SPELL_HYPPOTHERMIA, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_frostburn_formula_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_frostburn_formula_SpellScript();
        }
};

// #679: 96339 (Graveyard Gas pulse, SCRIPT_EFFECT, TARGET_UNIT_DEST_AREA_ENTRY 100 yd, no `conditions` row in the data) only
// works on Zanzil's Toxic Gas stalkers (52062): without the filter it hits every unit around him with an empty script effect. The
// stalker at the green-gas spot shows the green cloud, any other plays the gas sound. Port of CPP TrinityCore boss_zanzil.cpp
// spell_zanzil_zanzils_graveyard_gas (without its target-reservation list: this script summons a single stalker).
class spell_zanzil_graveyard_gas : public SpellScriptLoader
{
    public:
        spell_zanzil_graveyard_gas() : SpellScriptLoader("spell_zanzil_graveyard_gas") { }

        class spell_zanzil_graveyard_gas_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_zanzil_graveyard_gas_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if([](WorldObject* target)
                {
                    return target->GetTypeId() != TYPEID_UNIT || target->GetEntry() != NPC_ZANZIL_TOXIC_GAS;
                });
            }

            void HandleScript(SpellEffIndex /*effIndex*/)
            {
                Unit* target = GetHitUnit();
                if (!target)
                    return;

                uint32 const visual = target->GetExactDist(&greenGasStalkerPos) > 1.0f ? SPELL_TOXIC_GAS_SOUND : SPELL_GREEN_GAS;
                if (!target->HasAura(visual))
                    target->CastSpell(target, visual, true);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_zanzil_graveyard_gas_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_DEST_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_zanzil_graveyard_gas_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_zanzil_graveyard_gas_SpellScript();
        }
};

// #679: 96335 is SPELL_AURA_PERIODIC_DAMAGE_PERCENT, and AuraEffect::HandlePeriodicDamageAurasTick applies no damage-taken modifier to
// the percent branch (SpellAuraEffects.cpp, `amount = CountPctFromMaxHealth(amount)` - only SPELL_AURA_PERIODIC_DAMAGE goes through
// SpellDamageBonusTaken). The counter to the gas is Zanzil's Cauldron of Toxic Torment: 96328 SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN -90
// nature (SpellEffect.dbc; Wowpedia "Zanzil (tactics)": "90% less Nature damage for 30 sec", "every group member will need to get the
// debuff from the green cauldron in order to survive"). Without this hook the cauldron changes nothing and the heroic gas (20% of max
// health per second for 11.5 s) kills everybody. Apply the target's nature damage-taken multiplier to each tick.
class spell_zanzil_graveyard_gas_damage : public SpellScriptLoader
{
    public:
        spell_zanzil_graveyard_gas_damage() : SpellScriptLoader("spell_zanzil_graveyard_gas_damage") { }

        class spell_zanzil_graveyard_gas_damage_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_zanzil_graveyard_gas_damage_AuraScript);

            void CalculateTick(AuraEffect const* /*aurEff*/, int32& damage)
            {
                if (Unit* target = GetUnitOwner())
                    damage = int32(damage * target->GetTotalAuraMultiplierByMiscMask(SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, GetSpellInfo()->GetSchoolMask()));
            }

            void Register() override
            {
                DoEffectCalcTickAmount += AuraEffectCalcTickAmountFn(spell_zanzil_graveyard_gas_damage_AuraScript::CalculateTick, EFFECT_0, SPELL_AURA_PERIODIC_DAMAGE_PERCENT);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_zanzil_graveyard_gas_damage_AuraScript();
        }
};

void AddSC_boss_zanzil()
{
    new boss_zanzil();
    new npc_zanzil_berserker();
    new spell_zanzil_pursuit();
    new spell_zanzil_fire();
    new spell_frostburn_formula();
    new spell_zanzil_graveyard_gas();
    new spell_zanzil_graveyard_gas_damage();
}
