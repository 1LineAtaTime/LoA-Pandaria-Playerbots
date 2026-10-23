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

#include "siege_of_niuzao_temple.h"
#include "MoveSplineInit.h"

enum
{
    SPELL_GROW                      = 119942,
    SPELL_SAPPLING_SUMMON_VISUAL    = 120071,

    NPC_GLOBULE_SUMMON_DEST         = 61629,
    NPC_SAP_PUDDLE                  = 61613,
    NPC_SAPPLING                    = 61623
};

typedef std::pair<float, ObjectGuid> SapplingTrigger;

const Position centerPos = { 1529.520020f, 5163.240234f, 160.570999f, 0.0f };

class boss_vizier_jinbak : public CreatureScript
{
    enum Yells
    {
        SAY_INTRO,
        SAY_AGGRO,
        SAY_DEATH,
        SAY_SLAY,
        SAY_DETONATE,
        EMOTE_GLOBULE,
    };

    enum Spells
    {
        SPELL_DETONATE                  = 120001,
        SPELL_DETONATE_VISUAL           = 120095,
        SPELL_GROW_STACKER              = 119937,
        SPELL_SUMMON_SAPLING            = 119990
    };

    enum Events
    {
        EVENT_DETONATE                  = 1,
        EVENT_SUMMON_GLOBULES
    };

    public:
        boss_vizier_jinbak() : CreatureScript("boss_vizier_jinbak") { }

        struct boss_vizier_jinbakAI : public BossAI
        {
            boss_vizier_jinbakAI(Creature* creature) : BossAI(creature, BOSS_JINBAK)
            {
                visualTimer = 5000;
                puddleGUID = ObjectGuid::Empty;
                introDone = false;
            }

            void Reset() override
            {
                evadeCheckCooldown = 10000;
                events.Reset();

                if (Creature* trigger = Creature::GetCreature(*me, puddleGUID))
                {
                    // #656 (w44 spellwatch): the passive puddle never enters combat, so its evade did not reset it; the Grow
                    // stacks and the stacker survived Jin'bak's reset and the 1 s Sap Residue kept casting (667 casts in the
                    // next leg) with a radius that reached the stage (w43: 4 bots died out of combat 17 yd from the pool).
                    trigger->RemoveAurasDueToSpell(SPELL_GROW);
                    trigger->RemoveAurasDueToSpell(SPELL_GROW_STACKER);
                    trigger->ToCreature()->AI()->EnterEvadeMode();
                }

                ClearDebuffs();

                summonDestList.clear();

                visualTimer = 5000;
                _Reset();
            }

            void ClearDebuffs()
            {
                std::list<Creature *> stalkers;
                me->GetCreatureListWithEntryInGrid(stalkers, NPC_GLOBULE_SUMMON_DEST, 500.0f);

                for (auto&& itr : stalkers)
                    itr->RemoveAllAuras();

                std::list<Creature *> globules;
                me->GetCreatureListWithEntryInGrid(globules, NPC_SAPPLING, 500.0f);

                for (auto&& itr : globules)
                    itr->DespawnOrUnsummon(500);
            }

            void MoveInLineOfSight(Unit* who) override
            {
                if (!introDone && who->GetTypeId() == TYPEID_PLAYER)
                {
                    if (me->IsWithinDist2d(who, 30.0f))
                    {
                        introDone = true;
                        Talk(SAY_INTRO);
                    }
                }
                BossAI::MoveInLineOfSight(who);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                me->InterruptNonMeleeSpells(true);
                visualTimer = 0;

                // Find nearby globule summon targets and sort by angle from centre -> npc
                std::list<Creature *> destList;
                me->GetCreatureListWithEntryInGrid(destList, NPC_GLOBULE_SUMMON_DEST, 500.0f);
                Creature* sapPuddle = me->FindNearestCreature(NPC_SAP_PUDDLE, 500.0f);

                if (!sapPuddle)
                {
                    EnterEvadeMode();
                    return;
                }

                puddleGUID = sapPuddle->GetGUID();

                sapPuddle->CastSpell(sapPuddle, SPELL_GROW_STACKER, true);

                for (auto&& itr : destList)
                {
                    float angle = sapPuddle->GetAngle(itr);
                    summonDestList.push_back(SapplingTrigger(angle, itr->GetGUID()));
                }

                summonDestList.sort([](SapplingTrigger t1, SapplingTrigger t2) { return t1.first < t2.first; });

                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_SUMMON_GLOBULES, 10000);
                events.ScheduleEvent(EVENT_DETONATE, 30000);
                _JustEngagedWith();
            }

            void JustSummoned(Creature* summon) override
            {
                if (summon->GetEntry() == NPC_SAPPLING)
                {
                    if (Creature* puddle = Creature::GetCreature(*me, puddleGUID))
                    {
                        summon->SetReactState(REACT_PASSIVE);
                        Movement::MoveSplineInit init(summon);
                        init.MoveTo(puddle->GetPositionX(), puddle->GetPositionY(), puddle->GetPositionZ());

                        init.SetWalk(true);
                        init.SetVelocity(2.0f);
                        init.Launch();
                    }
                }
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_SLAY);
            }

            void JustDied(Unit* killer) override
            {
                Talk(SAY_DEATH);
                if (Creature* puddle = Creature::GetCreature(*me, puddleGUID))
                    puddle->DespawnOrUnsummon();
                ClearDebuffs();
                _JustDied();
            }

            void UpdateAI(uint32 diff) override
            {
                if (visualTimer)
                {
                    if (visualTimer <= diff)
                    {
                        DoCast(me, SPELL_DETONATE_VISUAL, false);
                        visualTimer = 0;
                    } else visualTimer -= diff;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_DETONATE:
                            Talk(SAY_DETONATE);
                            DoCast(me, SPELL_DETONATE, false);
                            events.ScheduleEvent(EVENT_DETONATE, 45500);
                            break;
                        case EVENT_SUMMON_GLOBULES:
                        {
                            // We need to summon them at random 3 dest NPCS
                            // The summon position pattern looks similar to the Mercedes Logo
                            // First, get a random starting point
                            auto itr = summonDestList.begin();
                            std::advance(itr, urand(0, summonDestList.size() / 3));
                            float lastAngle = -10.0f;
                            float minSectorAngle = 100.0f * M_PI / 180.0f;
                            uint8 count = 3;

                            for (int i = 0; i < 13; ++i)
                            {
                                if (itr->first < lastAngle)
                                {
                                    // passed the 2xPI
                                    lastAngle -= M_PI * 2.0f;
                                }

                                if (fabs(itr->first - lastAngle) > minSectorAngle)
                                {
                                    lastAngle = itr->first;
                                    if (Creature* trigger = Creature::GetCreature(*me, itr->second))
                                        DoCast(trigger, SPELL_SUMMON_SAPLING, true);

                                    if (!--count)
                                        break;
                                }

                                if (++itr == summonDestList.end())
                                    itr = summonDestList.begin();

                            }
                            Talk(EMOTE_GLOBULE);
                            events.ScheduleEvent(EVENT_SUMMON_GLOBULES, 45500);

                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();

                // Room check
                if (evadeCheckCooldown <= diff)
                    evadeCheckCooldown = 2500;
                else
                {
                    evadeCheckCooldown -= diff;
                    return;
                }

                if (!me->IsWithinDist2d(1529.520020f, 5163.240234f, 30.0f))
                    EnterEvadeMode();
            }

        private:
            uint32 evadeCheckCooldown;
            uint32 visualTimer;
            std::list<SapplingTrigger > summonDestList;
            ObjectGuid puddleGUID;
            bool introDone;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_vizier_jinbakAI(creature);
        }
};


class npc_sap_globule : public CreatureScript
{
    enum Spells
    {
        SPELL_SAP_RESIDUE           = 120070,
    };

    public:
        npc_sap_globule() : CreatureScript("npc_sap_globule") { }

        struct npc_sap_globuleAI : public ScriptedAI
        {
            npc_sap_globuleAI(Creature* creature) : ScriptedAI(creature) { }

            void InitializeAI() override
            {
                done = false;
            }

            void IsSummonedBy(Unit* summoner) override
            {
                if (Creature* jinbak = me->FindNearestCreature(NPC_JINBAK, 500.0f))
                    jinbak->AI()->JustSummoned(me);
            }

            void SpellHit(Unit*  caster, SpellInfo const* spell) override
            {
                if (!done && spell->Id == SPELL_SAP_RESIDUE)
                {
                    // Set hit only if target near with pool, cuz this spell has 100y range lol
                    if (me->GetExactDist2d(caster) < 4.0f * caster->GetObjectScale())
                    {
                        done = true;
                        caster->ToCreature()->AI()->DoAction(1);
                        DoCast(me, SPELL_SAPPLING_SUMMON_VISUAL, true);
                        me->DespawnOrUnsummon(500);
                    }
                }
            }

            void UpdateAI(uint32 diff) override  { }

        private:
            bool done;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_sap_globuleAI(creature);
        }
};

class npc_sap_puddle : public CreatureScript
{
    enum
    {
        SPELL_RESIDUE_DOT           = 119941
    };

    public:
        npc_sap_puddle() : CreatureScript("npc_sap_puddle") { }

        struct npc_sap_puddleAI : public ScriptedAI
        {
            npc_sap_puddleAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                skipTick = 0;
            }

            void DoAction(int32 actionId) override
            {
                if (actionId == 1)
                {
                    skipTick = 3000;
                    Aura * growAura = me->GetAura(SPELL_GROW);
                    if (!growAura)
                    {
                        const SpellInfo* spell = sSpellMgr->GetSpellInfo(SPELL_GROW);
                        int32 tempSolution = 1;
                        growAura = Aura::TryCreate(spell, MAX_EFFECT_MASK, me, me, &tempSolution/*spell->spellPower*/); // TODO: port spell power changes
                    }

                    if (growAura)
                        growAura->ModStackAmount(3); // TODO: find proper value
                }
            }

            void SpellHitTarget(Unit* target, SpellInfo const* spell) override
            {
                if (!skipTick && target->GetTypeId() == TYPEID_PLAYER && spell->Id == SPELL_RESIDUE_DOT)
                    if (Aura * growAura = me->GetAura(SPELL_GROW))
                    {
                        int32 stackAmt = growAura->GetStackAmount();
                        growAura->SetStackAmount(std::max(stackAmt - 3, 1));// 3 Per player each tick
                    }
            }

            void UpdateAI(uint32 diff) override
            {
                if (skipTick)
                {
                    if (skipTick <= diff)
                    {
                        skipTick = 0;
                    } else skipTick -= diff;
                }
            }
        private:
            uint32 skipTick;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_sap_puddleAI(creature);
        }
};


class spell_sap_residue : public SpellScriptLoader
{
    public:
        spell_sap_residue() : SpellScriptLoader("spell_sap_residue") { }

        class spell_sap_residue_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_sap_residue_SpellScript);

            // #656 (w45): the touch radius is 4 yd at base size (LegionCore-7.3.5 value), widened by Grow 119942 only:
            // SPELL_AURA_MOD_SCALE +30 per stack, HandleAuraModScale -> scale = 1 + 0.3 x stacks. It is taken from the Grow
            // stack count and capped by GetObjectScale(), so neither a stray OBJECT_FIELD_SCALE nor a missed filter can
            // stretch the 100 yd SpellRadius search over the room (w45: Grow 0, yet 5 players 17.3 yd away got 1 stack/s).
            static float PuddleRadius(Unit* puddle)
            {
                float growScale = 1.0f + 0.3f * float(puddle->GetAuraCount(SPELL_GROW));
                return 4.0f * std::min(growScale, puddle->GetObjectScale());
            }

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (Unit*  caster = GetCaster())
                {
                    float dist = PuddleRadius(caster);
                    uint32 players = 0;
                    float nearest = 1000.0f;
                    for (WorldObject const* unit : targets)
                        if (unit->GetTypeId() == TYPEID_PLAYER)
                        {
                            ++players;
                            nearest = std::min(nearest, unit->GetExactDist2d(caster));
                        }

                    if (!targets.empty())
                        targets.remove_if ([&caster, &dist](const WorldObject * unit) { return unit->GetExactDist2d(caster) > dist; });

                    // #656 evidence (remove with the issue): one line per tick while a player is inside the 100 yd search.
                    if (players && m_scriptSpellId == 119941)
                        TC_LOG_ERROR("scripts", "[SAPWATCH] select puddle=%.1f,%.1f,%.1f scale=%.2f grow=%u radius=%.1f players=%u nearest=%.1f kept=%u",
                            caster->GetPositionX(), caster->GetPositionY(), caster->GetPositionZ(), caster->GetObjectScale(),
                            caster->GetAuraCount(SPELL_GROW), dist, players, nearest, uint32(targets.size()));
                }
            }

            // #656: hit-time guard. Whatever put a unit on the target list (area filter or not), Sap Residue only lands on
            // a unit that is actually standing in the puddle when the 20 yd/s missile arrives.
            void HandleHit(SpellEffIndex effIndex)
            {
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                float dist = PuddleRadius(caster);
                if (target->GetExactDist2d(caster) <= dist)
                    return;

                TC_LOG_ERROR("scripts", "[SAPWATCH] guard dropped %s dist=%.1f radius=%.1f scale=%.2f puddle=%.1f,%.1f",
                    target->GetName().c_str(), target->GetExactDist2d(caster), dist, caster->GetObjectScale(),
                    caster->GetPositionX(), caster->GetPositionY());
                PreventHitDefaultEffect(effIndex);
                PreventHitAura();
            }

            void Register() override
            {
                // #656: the old EFFECT_1 / TARGET_UNIT_CASTER binding matches neither spell (5.4.8 SpellEffect.dbc: 119941
                // Sap Residue has only EFFECT_0, TARGET_SRC_CASTER + TARGET_UNIT_SRC_AREA_ENEMY, radius index 12 = 100 yd;
                // 120070 has only EFFECT_0, TARGET_SRC_CASTER + TARGET_UNIT_SRC_AREA_ENTRY). Startup logged "Spell `119941`
                // Effect `Index: EFFECT_1 Target: 1` of script `spell_sap_residue` did not match dbc effect data - handler
                // ... won't be executed", so the 4 yd x scale filter never ran and the puddle's 1 s Sap Residue tick
                // (aura 119939) stacked on every player within 100 yd (wave 41: the whole group wiped before the pull).
                // Same binding as LegionCore-7.3.5 boss_jinbak.cpp spell_sap_residue (EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY).
                if (m_scriptSpellId == 120070)
                    OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sap_residue_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                else
                {
                    OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_sap_residue_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                    OnEffectHitTarget += SpellEffectFn(spell_sap_residue_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_APPLY_AURA);
                }
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_sap_residue_SpellScript();
        }
};

// Detonate - 120001
class spell_jinbak_detonate : public SpellScriptLoader
{
    enum
    {
        SPELL_DETONATE_DAMAGE           = 120002
    };

    public:
        spell_jinbak_detonate() : SpellScriptLoader("spell_jinbak_detonate") { }

        class spell_jinbak_detonate_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_jinbak_detonate_AuraScript);

            void OnTick(AuraEffect const* aurEff)
            {
                if (Unit*  target = GetTarget())
                {
                    uint32 stackAmount = std::max<uint32>(1, target->GetAuraCount(SPELL_GROW));
                    bool isHeroic = target->GetMap()->GetDifficulty() != REGULAR_DIFFICULTY;
                    int32 damage = (isHeroic ? 25500 : 8500) * stackAmount;
                    target->CastCustomSpell(target, SPELL_DETONATE_DAMAGE, &damage, NULL, NULL, true);
                    target->RemoveAurasDueToSpell(SPELL_GROW);
                }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_jinbak_detonate_AuraScript::OnTick, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_jinbak_detonate_AuraScript();
        }
};

void AddSC_vizier_jinbak()
{
    new boss_vizier_jinbak();
    new npc_sap_globule();
    new npc_sap_puddle();
    new spell_sap_residue();
    new spell_jinbak_detonate();
}