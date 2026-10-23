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

#include "grim_batol.h"
#include "Spell.h"
#include "ScriptPCH.h"
#include "Vehicle.h"

// #688: Flaming Fixate and the Valiona phase follow the real fight (Wowpedia Drahga_Shadowburner, fetched 2026-10-23) and the
// 4.3.4 script of the Cataclysm Preservation Project (TrinityCore fork, GrimBatol/boss_drahga_shadowburner.cpp @0927c5af):
// the Invoked Flaming Spirit fixates one random player (82850) after 2 s and chases only that player; Drahga keeps casting
// Burning Shadowbolt and Invocation of Flame from Valiona's back; Valiona breathes Valiona's Flame at a random player
// (heroic: Devouring Flames at a Devouring Flames stalker on a random player) and leaves at 20% health.

// #688 R15: the groups of creature_text 40319 / 40320 (test world DB) and the 4.3.4 Texts enum. The old values were off by one
// or two: a kill said the spirit emote, every Invocation of Flame said "Dragon, you will do as I command! Catch me!" and
// Valiona leaving announced "begins to cast Devouring Flames".
enum ScriptTexts
{
    // drahga
    SAY_AGGRO                          = 1,   // group 0 holds the same line (4.3.4 SAY_INTRO)
    SAY_ANNOUNCE_ADDS                  = 2,   // raid boss emote: summons an Invoked Flaming Spirit
    SAY_ADDS                           = 3,   // "BY FIRE BE... BURNED!" / "INCINERATE THEM, MINIONS!"
    SAY_VALIONA                        = 4,   // "Dragon, you will do as I command! Catch me!"
    SAY_KILL                           = 5,
    SAY_DEATH                          = 6,

    // valiona
    SAY_ENTER                          = 0,   // "If they do not kill you, I will do it myself!"
    SAY_ANNOUNCE_DEVOURING_FLAMES      = 1,   // raid boss emote (heroic)
    SAY_LEAVE                          = 2,   // "I will not die for you, Drahga."
};

enum Spells
{
    SPELL_BURNING_SHADOWBOLT           = 75245,
    SPELL_INVOCATION_OF_FLAME          = 75222,
    SPELL_FLAMING_FIXATE               = 82850,
    SPELL_INVOKED_FLAME                = 75235,
    SPELL_SUPERNOVA                    = 75238,
    SPELL_TWILIGHT_PROTECTION          = 76303,
    SPELL_TWILIGHT_SHIFT               = 75328,
    SPELL_SHREDDING_SWIPE              = 75271,
    SPELL_SEEPING_TWILIGHT_DUMMY       = 75318,
    SPELL_SEEPING_TWILIGHT             = 75274,
    SPELL_SEEPING_TWILIGHT_DMG         = 75317,
    SPELL_VALIONAS_FLAME               = 75321,
    SPELL_DEVOURING_FLAMES             = 90950,   // 2.5 s cast, 5 s channel on the 48798 stalker -> 90949 every 1 s
    SPELL_DEVOURING_FLAMES_DMG         = 90949,   // 150 degree cone, 40k at Valiona, less with distance (spell script below)
};

enum Adds
{
    NPC_INVOCATION_OF_FLAME_STALKER    = 40355,
    NPC_INVOKED_FLAMING_SPIRIT         = 40357,
    NPC_VALIONA                        = 40320,
    NPC_SEEPING_TWILIGHT               = 40365,
    NPC_DEVOURING_FLAMES               = 48798,
};

enum Events
{
    EVENT_BURNING_SHADOWBOLT           = 1,
    EVENT_INVOCATION_OF_FLAME          = 2,
    EVENT_SELECT_TARGET                = 3,
    EVENT_VALIONAS_FLAME               = 4,
    EVENT_SHREDDING_SWIPE              = 5,
    EVENT_VALIONA_MOVE                 = 6,
};

enum Actions
{
    ACTION_VALIONA_LEAVE               = 1,
    ACTION_CLEAR_FIXATE                = 2,   // Invoked Flaming Spirit: drop Flaming Fixate from its target (Supernova)
};

const Position drahgavalionaPos[3] =
{
    { -373.35f, -659.56f, 253.56f, 3.44f },
    { -373.35f, -659.56f, 282.26f, 3.44f },
    { -414.04f, -681.35f, 267.09f, 3.63f }
};

const Position DrahgaLeavePoint =  { -396.02f, -674.77f, 265.14f, 0.35f };
const Position ValioneLeavePoint = { -373.56f, -613.11f, 299.54f, 0.93f };

class boss_drahga_shadowburner : public CreatureScript
{
    public:
        boss_drahga_shadowburner() : CreatureScript("boss_drahga_shadowburner") { }

        struct boss_drahga_shadowburnerAI : public BossAI
        {
            boss_drahga_shadowburnerAI(Creature* creature) : BossAI(creature, DATA_DRAHGA_SHADOWBURNER)
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
                me->setActive(true);
            }

            uint8 stage;

            void Reset() override
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                me->ExitVehicle();
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                me->SetReactState(REACT_AGGRESSIVE);
                summons.DespawnAll();
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLAMING_FIXATE);
                stage = 0;
            }

            void SpellHit(Unit* /*caster*/, SpellInfo const* spell) override
            {
                if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (me->GetCurrentSpell(CURRENT_GENERIC_SPELL)->m_spellInfo->Id == SPELL_BURNING_SHADOWBOLT)
                        for (uint8 i = 0; i < 3; ++i)
                            if (spell->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
                                me->InterruptSpell(CURRENT_GENERIC_SPELL);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);
            }

            void DamageTaken(Unit* attacker, uint32& damage) override
            {
                if (me->GetVehicle())
                    damage = 0;
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
                events.ScheduleEvent(EVENT_BURNING_SHADOWBOLT, urand(2000, 5000));
                events.ScheduleEvent(EVENT_INVOCATION_OF_FLAME, 10000);
            }

            void DoAction(int32 actionId) override
            {
                events.RescheduleEvent(EVENT_BURNING_SHADOWBOLT, urand(2000, 5000));
                me->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                // Valiona has left: Drahga fights on foot again (he stayed REACT_PASSIVE from the 30% transition before).
                me->SetReactState(REACT_AGGRESSIVE);
                if (Player* itr = me->FindNearestPlayer(80.0f))
                    me->AI()->AttackStart(itr);
            }
            
            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH);
                summons.DespawnAll();
                if (instance)
                    instance->DoRemoveAurasDueToSpellOnPlayers(SPELL_FLAMING_FIXATE);
            }

            void KilledUnit(Unit* /*victim*/) override
            {                
                Talk(SAY_KILL);
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE && pointId == 0)
                {
                    if (Creature* valiona = me->SummonCreature(NPC_VALIONA, drahgavalionaPos[0]))
                    {
                        Talk(SAY_VALIONA);
                        valiona->SetReactState(REACT_PASSIVE);
                        valiona->SetCanFly(true);
                        me->EnterVehicle(valiona, 0);
                        me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                        // Wowpedia: "Drahga will continue casting fireballs at random party members and summoning elemental
                        // adds during this phase" (4.3.4 reschedules both when Valiona lands). Invocation never stopped.
                        events.RescheduleEvent(EVENT_BURNING_SHADOWBOLT, 8000);
                    }
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->GetDistance(me->GetHomePosition()) > 60.0f)
                {
                    EnterEvadeMode();
                    return;
                }

                if (me->HealthBelowPct(30) && stage == 0)
                {
                    stage = 1;
                    DoCast(me, SPELL_TWILIGHT_PROTECTION);
                    me->SetReactState(REACT_PASSIVE);
                    me->GetMotionMaster()->MovePoint(0, DrahgaLeavePoint);
                    events.CancelEvent(EVENT_BURNING_SHADOWBOLT);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_BURNING_SHADOWBOLT:
                            me->CastCustomSpell(SPELL_BURNING_SHADOWBOLT, SPELLVALUE_MAX_TARGETS, 1, 0, false);
                            events.ScheduleEvent(EVENT_BURNING_SHADOWBOLT, urand(8000, 15000));
                            break;
                        case EVENT_INVOCATION_OF_FLAME:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                            {
                                me->SummonCreature(NPC_INVOCATION_OF_FLAME_STALKER,
                                target->GetPositionX(),
                                target->GetPositionY(),
                                target->GetPositionZ(),
                                0.0f, TEMPSUMMON_TIMED_DESPAWN, 6000ms);
                                Talk(SAY_ANNOUNCE_ADDS);
                                Talk(SAY_ADDS);
                            }
                            events.ScheduleEvent(EVENT_INVOCATION_OF_FLAME, 20000);
                            break;
                    }
                }
                if (!me->GetVehicle())
                    DoMeleeAttackIfReady();
            }

        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_drahga_shadowburnerAI>(creature);
        }
};

class npc_drahga_valiona : public CreatureScript
{
    public:
        npc_drahga_valiona() : CreatureScript("npc_drahga_valiona") { }

        struct npc_drahga_valionaAI : public ScriptedAI
        {
            npc_drahga_valionaAI(Creature* creature) : ScriptedAI(creature), summons(me)
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
                instance = creature->GetInstanceScript();
            }

            InstanceScript* instance;
            EventMap events, nonCombatEvents;
            SummonList summons;
            uint8 stage, wp;

            void IsSummonedBy(Unit* summoner) override
            {
                wp = 1;
                nonCombatEvents.ScheduleEvent(EVENT_VALIONA_MOVE, 100);
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                Talk(SAY_ENTER);   // 4.3.4 EVENT_MOVE_INTRO
            }

            void Reset() override
            {
                summons.DespawnAll();
                events.Reset();
                stage = 0;
                wp = 1;
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);

                // Heroic Devouring Flames: turn to the stalker on the random player and channel at it (4.3.4 JustSummoned).
                // 90950 targets the nearest 48798 (TARGET_UNIT_NEARBY_ENTRY, conditions row in the #688 SQL).
                if (summon->GetEntry() == NPC_DEVOURING_FLAMES)
                {
                    summon->SetReactState(REACT_PASSIVE);
                    me->SetFacingToObject(summon);
                    Talk(SAY_ANNOUNCE_DEVOURING_FLAMES);
                    me->CastSpell(summon, SPELL_DEVOURING_FLAMES, false);
                }
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                // 4.3.4: Shredding Swipe 1 s after landing then every 25 s; the breath at 15.5 s then every 25 s.
                events.ScheduleEvent(EVENT_VALIONAS_FLAME, 15500);
                events.ScheduleEvent(EVENT_SHREDDING_SWIPE, urand(8000, 10000));
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (type == POINT_MOTION_TYPE)
                {
                    wp++;

                    switch (pointId)
                    {
                        case 1002:
                            me->DespawnOrUnsummon();
                            break;
                        case 1:
                            nonCombatEvents.ScheduleEvent(EVENT_VALIONA_MOVE, 100);
                            break;
                        case 2:
                            me->SetCanFly(false);
                            me->SetDisableGravity(false);
                            me->SetReactState(REACT_AGGRESSIVE);

                            if (Player* itr = me->FindNearestPlayer(80.0f))
                                me->AI()->AttackStart(itr);
                            break;
                    }
                }
            }

            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                if (me->GetVehicleKit())
                    me->GetVehicleKit()->RemoveAllPassengers();
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override
            {
                nonCombatEvents.Update(diff);

                while (uint32 eventId = nonCombatEvents.ExecuteEvent())
                {
                    if (eventId == EVENT_VALIONA_MOVE)
                    {
                        me->GetMotionMaster()->MovementExpired();
                        me->GetMotionMaster()->MovePoint(wp, drahgavalionaPos[wp]);
                    }
                    break;
                }

                if (!UpdateVictim())
                    return;

                // Wowpedia / 4.3.4: Valiona fights "until she reaches 20% remaining health", then ejects Drahga and leaves.
                if (me->HealthBelowPct(20) && stage == 0)
                {
                    stage = 1;
                    events.Reset();
                    me->InterruptNonMeleeSpells(false);
                    DoCast(me, SPELL_TWILIGHT_SHIFT);
                    me->SetReactState(REACT_PASSIVE);
                    if (me->GetVehicleKit())
                        me->GetVehicleKit()->RemoveAllPassengers();
                    me->SetCanFly(true);
                    me->SetDisableGravity(true);
                    me->GetMotionMaster()->MovePoint(1002, ValioneLeavePoint);
                    if (Creature* drahga = Unit::GetCreature(*me, instance->GetGuidData(DATA_DRAHGA_SHADOWBURNER)))
                    {
                        drahga->RemoveAurasDueToSpell(SPELL_TWILIGHT_PROTECTION);
                        drahga->AI()->DoAction(ACTION_VALIONA_LEAVE);
                    }
                    Talk(SAY_LEAVE);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_VALIONAS_FLAME:
                            // "breathes ... in the direction of a random player" (Wowpedia); it was cast on herself before.
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true))
                            {
                                if (IsHeroic())
                                    me->SummonCreature(NPC_DEVOURING_FLAMES, target->GetPositionX(), target->GetPositionY(),
                                                       target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 8000ms);
                                else
                                {
                                    me->SetFacingToObject(target);
                                    DoCast(target, SPELL_VALIONAS_FLAME);
                                }
                            }
                            events.ScheduleEvent(EVENT_VALIONAS_FLAME, 25000);
                            break;
                        case EVENT_SHREDDING_SWIPE:
                            DoCast(me->GetVictim(), SPELL_SHREDDING_SWIPE);
                            events.ScheduleEvent(EVENT_SHREDDING_SWIPE, 25000);
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_drahga_valionaAI>(creature);
        }
};

class npc_invocation_of_flame_stalker : public CreatureScript
{
    public:
        npc_invocation_of_flame_stalker() : CreatureScript("npc_invocation_of_flame_stalker") { }

        struct npc_invocation_of_flame_stalkerAI : public ScriptedAI
        {
            npc_invocation_of_flame_stalkerAI(Creature* creature) : ScriptedAI(creature)
            {
                instance = creature->GetInstanceScript();
            }
            
            InstanceScript* instance;
            
            // The Invoked Flaming Spirit picks and fixates its own target 2 s after it appears (npc_invoked_flaming_spirit).
            void JustSummoned(Creature* /*summon*/) override { }

            void Reset() override
            {
                DoCast(me, SPELL_INVOCATION_OF_FLAME);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_invocation_of_flame_stalkerAI>(creature);
        }
};

class npc_invoked_flaming_spirit : public CreatureScript
{
    public:

        npc_invoked_flaming_spirit() : CreatureScript("npc_invoked_flaming_spirit") { }

        struct npc_invoked_flaming_spiritAI : public ScriptedAI
        {
            npc_invoked_flaming_spiritAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetSpeed(MOVE_RUN, 0.8f);
                me->SetSpeed(MOVE_WALK, 0.8f);
                me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_FEAR, true);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                instance = creature->GetInstanceScript();
            }
            
            InstanceScript* instance;
            ObjectGuid fixateGUID;
            uint32 chaseTimer = 2000;

            void Reset() override
            {
                DoCast(me, SPELL_INVOKED_FLAME);
                me->SetReactState(REACT_PASSIVE);
                chaseTimer = 2000;
            }

            void DoAction(int32 action) override
            {
                if (action == ACTION_CLEAR_FIXATE)
                    ClearFixate();
            }

            void ClearFixate()
            {
                if (Unit* target = ObjectAccessor::GetUnit(*me, fixateGUID))
                    target->RemoveAurasDueToSpell(SPELL_FLAMING_FIXATE, me->GetGUID());
                fixateGUID = ObjectGuid::Empty;
                me->GetThreatManager().ClearFixate();
            }

            // Flaming Fixate 82850 (4.3.4 EVENT_CHASE_PLAYER): one random player of Drahga's threat list, chased and nobody else
            // ("ignoring all attempts to forcibly change its target", Wowpedia). A new one when the fixated player dies.
            bool Fixate()
            {
                ClearFixate();

                Creature* drahga = ObjectAccessor::GetCreature(*me, instance->GetGuidData(DATA_DRAHGA_SHADOWBURNER));
                Unit* target = drahga && drahga->AI() ? drahga->AI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true) : nullptr;
                if (!target)
                    return false;

                me->SetReactState(REACT_AGGRESSIVE);
                DoCast(target, SPELL_FLAMING_FIXATE, true);
                // 82850 is an endless channel (duration -1): Spell::prepare makes it the current channel, which keeps
                // UNIT_STATE_CASTING on the spirit and UnitAI::DoMeleeAttackIfReady returns early, so it would never swing
                // and never proc Supernova (Invoked Flame 75235). 4.3.4 clears the state right after the cast too.
                me->ClearUnitState(UNIT_STATE_CASTING);
                fixateGUID = target->GetGUID();
                me->GetThreatManager().AddThreat(target, 1500000.0f, nullptr, true, true);
                me->GetThreatManager().FixateTarget(target);
                AttackStart(target);
                return true;
            }

            void JustDied(Unit* /*target*/) override
            {
                ClearFixate();
                me->DespawnOrUnsummon();
            }

            void UpdateAI(uint32 diff) override
            {
                if (!instance)
                    return;

                // #688 R15: GetBossState, not GetData. instance_grim_batol has no GetData override, so GetData(2) is always
                // ZoneScript's 0 (!= IN_PROGRESS) and every spirit despawned on its first update, before the 2 s fixate timer
                // (w106/w107: 75222 x4/x11, 82850 x0, no 40357 in any [PROBE] target/attacker field).
                if (instance->GetBossState(DATA_DRAHGA_SHADOWBURNER) != IN_PROGRESS)
                {
                    ClearFixate();
                    me->DespawnOrUnsummon();
                    return;
                }

                if (chaseTimer)
                {
                    if (chaseTimer <= diff)
                    {
                        chaseTimer = 0;
                        Fixate();
                    }
                    else
                        chaseTimer -= diff;
                    return;
                }

                Unit* target = ObjectAccessor::GetUnit(*me, fixateGUID);
                if (!target || !target->IsAlive())
                {
                    if (!Fixate())
                        return;
                    target = ObjectAccessor::GetUnit(*me, fixateGUID);
                }

                if (target && me->GetVictim() != target)
                    AttackStart(target);

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_invoked_flaming_spiritAI>(creature);
        }
};

class npc_seeping_twilight : public CreatureScript
{
    public:
        npc_seeping_twilight() : CreatureScript("npc_seeping_twilight"){}

        struct npc_seeping_twilightAI : public ScriptedAI
        {
            npc_seeping_twilightAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                DoCast(me, SPELL_SEEPING_TWILIGHT_DUMMY);
                DoCast(me, SPELL_SEEPING_TWILIGHT);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_seeping_twilightAI>(creature);
        }
};

class spell_drahga_supernova : public SpellScriptLoader
{
    public:
        spell_drahga_supernova() : SpellScriptLoader("spell_drahga_supernova") { }

        class spell_drahga_supernova_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_drahga_supernova_SpellScript);

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                Creature* spirit = GetCaster() ? GetCaster()->ToCreature() : nullptr;
                if (!spirit)
                    return;

                if (spirit->AI())
                    spirit->AI()->DoAction(ACTION_CLEAR_FIXATE);
                spirit->DespawnOrUnsummon(500);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_drahga_supernova_SpellScript::HandleScriptEffect, EFFECT_1, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_drahga_supernova_SpellScript();
        }
};

// Devouring Flames 90949 (heroic): "Valiona inflicts less damage to players within the cone of flames the farther away they are
// positioned from her" (Wowpedia); 4.3.4 spell_drahga_devouring_flames takes 3% off per yard. Registered in spell_script_names.
class spell_drahga_devouring_flames : public SpellScriptLoader
{
    public:
        spell_drahga_devouring_flames() : SpellScriptLoader("spell_drahga_devouring_flames") { }

        class spell_drahga_devouring_flames_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_drahga_devouring_flames_SpellScript);

            void ChangeDamage()
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                float const pct = std::min(100.0f, target->GetDistance(caster) * 3.0f);
                SetHitDamage(int32(GetHitDamage() * (100.0f - pct) / 100.0f));
            }

            void Register() override
            {
                OnHit += SpellHitFn(spell_drahga_devouring_flames_SpellScript::ChangeDamage);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_drahga_devouring_flames_SpellScript();
        }
};

void AddSC_boss_drahga_shadowburner()
{
    new spell_drahga_devouring_flames();
    new boss_drahga_shadowburner();
    new npc_invocation_of_flame_stalker();
    new npc_invoked_flaming_spirit();
    new npc_drahga_valiona();
    new npc_seeping_twilight();
    new spell_drahga_supernova();
}
