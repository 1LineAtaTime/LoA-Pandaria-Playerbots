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
#include "zulgurub.h"

// #679: creature_text 52155 groups (same rows in the MoP data), mapped as The-Cataclysm-Preservation-Project TrinityCore
// boss_high_priest_venoxis.cpp (sniff based). The old ids yelled "Let the coils of death unfurl!" on every Whispers event and the
// "High Priest Venoxis is exhausted!" emote on death.
enum ScriptTexts
{
    SAY_AGGRO               = 0,
    SAY_TOXIC_LINK          = 1,
    SAY_TRANSFORM           = 2, // Blessing of the Snake God
    SAY_BLOODVENOM          = 3,
    SAY_ANNOUNCE_BLOODVENOM = 4,
    SAY_ANNOUNCE_EXHAUSTED  = 5,
    SAY_KILL                = 6,
    SAY_DEATH               = 7,
};

enum Spells
{
    SPELL_TOXIC_LINK                = 96475,
    SPELL_TOXIC_LINK_AURA           = 96477, // On players
    SPELL_TOXIC_LINK_DUMMY          = 96476, // Beam on ally
    SPELL_TOXIC_EXPLOSION           = 96489,
    SPELL_WORD_OF_HETHISS           = 96560,
    SPELL_WHISPER_OF_HETHISS        = 96466,
    SPELL_WHISPER_OF_HETHISS_DMG    = 96469,
    SPELL_BREATH_OF_HETHISS         = 96509,
    SPELL_POOL_OF_ACID_TEARS        = 96515,
    SPELL_POOL_OF_ACID_TEARS_AURA   = 96520, // make scale
    SPELL_BLESSING_OF_THE_SNAKE_GOD = 96512,
    SPELL_BLOODVENOM                = 96842,
    SPELL_BLOODVENOM_SUMMON         = 96637,
    SPELL_BLOODVENOM_DUMMY          = 97110,
    SPELL_BLOODVENOM_AURA           = 97099,
    SPELL_VENOM_WITHDRAWAL          = 96653,
    SPELL_VENOMOUS_EFFUSION         = 96678, 
    SPELL_VENOMOUS_EFFUSION_SUMMON  = 96680,
    SPELL_VENOMOUS_EFFUSION_AURA    = 96681,
    SPELL_VENOMOUS_EFFUSION_DUMMY   = 96534,
    SPELL_VENOM_TOTEM_BEAM_RIGHT    = 96937,
    SPELL_VENOM_TOTEM_BEAM_LEFT     = 96936,
    SPELL_VENOM_TOTEM_BEAM_CENTER   = 97098,
};

enum Events
{
    EVENT_TOXIC_LINK            = 1,
    EVENT_WORD_OF_HETISS        = 2,
    EVENT_BREATH_OF_HETHISS     = 3,
    EVENT_POOL_OF_ACID_TEARS    = 4,
    EVENT_MOVE_DOWN             = 5,
    EVENT_VENOM_WITHDRAWAL      = 6,
    EVENT_TRANSFORM             = 7,
    EVENT_WHISPER_OF_HETHISS    = 8,
    EVENT_MOVE_UP               = 9,
    EVENT_BLOODVENOM            = 10,
    EVENT_VENOMOUS_EFFUSION     = 11,
};

enum Adds
{
    NPC_VENOMOUS_EFFUSION           = 52288, // 96681
    NPC_VENOMOUS_EFFUSION_STALKER   = 52302, // 96678
    NPC_POOL_OF_ACID_TEARS          = 52197, // 96520
    NPC_BLOODVENOM                  = 52525, // 97110
};

enum Points
{
    POINT_DOWN          = 1,
    POINT_UP            = 2,
    POINT_WITHDRAWAL    = 3,
};

const Position downPos = {-12000.54f, -1685.88f, 32.28f, 0.68f};
const Position upPos = {-12020.12f, -1699.96f, 39.53f, 0.60f};

const Position mazeleftPos[18] = 
{
    {-12003.12f, -1700.04f, 32.29f, 0.0f},
    {-11999.17f, -1705.85f, 32.29f, 0.0f},
    {-12000.37f, -1714.15f, 32.34f, 0.0f},
    {-11992.49f, -1719.70f, 32.39f, 0.0f},
    {-11985.03f, -1714.68f, 32.50f, 0.0f},
    {-11981.08f, -1718.16f, 32.95f, 0.0f},
    {-11975.54f, -1714.41f, 32.90f, 0.0f},
    {-11978.24f, -1705.43f, 32.40f, 0.0f},
    {-11972.51f, -1702.20f, 32.60f, 0.0f},
    {-11974.63f, -1694.00f, 32.40f, 0.0f},
    {-11970.75f, -1691.41f, 32.70f, 0.0f},
    {-11976.65f, -1683.74f, 32.80f, 0.0f},
    {-11988.19f, -1692.20f, 32.30f, 0.0f},
    {-11983.91f, -1696.76f, 32.30f, 0.0f},
    {-11989.18f, -1701.09f, 32.30f, 0.0f},
    {-11995.53f, -1697.08f, 32.30f, 0.0f},
    {-11992.38f, -1694.31f, 32.30f, 0.0f},
    {-12001.17f, -1688.04f, 32.30f, 0.0f}
};

class boss_venoxis : public CreatureScript
{
    public:
        boss_venoxis() : CreatureScript("boss_venoxis") { }

        struct boss_venoxisAI : public BossAI
        {
            boss_venoxisAI(Creature* creature) : BossAI(creature, DATA_VENOXIS)
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

            uint8 phase;

            void Reset() override
            {
                _Reset();

                phase = 0;
                me->SetReactState(REACT_AGGRESSIVE);
                me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);
                phase = 0;
                me->SetReactState(REACT_AGGRESSIVE);
                // #679: the Venomous Effusion maze (Wowpedia "High Priest Venoxis (tactics)", stages one and two). CPP TrinityCore
                // boss_high_priest_venoxis.cpp summons the stalkers 2.2 s after the pull and again 12 s after Venom Withdrawal.
                events.ScheduleEvent(EVENT_VENOMOUS_EFFUSION, 2200);
                events.ScheduleEvent(EVENT_TOXIC_LINK, 7000);
                events.ScheduleEvent(EVENT_WHISPER_OF_HETHISS, 6000);
                events.ScheduleEvent(EVENT_TRANSFORM, 35000);
                DoCastAOE(SPELL_WORD_OF_HETHISS);
                DoZoneInCombat();
                instance->SetBossState(DATA_VENOXIS, IN_PROGRESS);

                instance->DoStartCriteria(CRITERIA_START_TYPE_EVENT, 28223);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void MovementInform(uint32 type, uint32 id) override
            {
                if (id == POINT_DOWN && phase == 2)
                    events.ScheduleEvent(EVENT_MOVE_UP, 400);
                else if (id == POINT_UP && phase == 2)
                    events.ScheduleEvent(EVENT_BLOODVENOM, 1000);
                else if (id == POINT_WITHDRAWAL)
                {
                    Talk(SAY_ANNOUNCE_EXHAUSTED);
                    me->RemoveAura(SPELL_BLESSING_OF_THE_SNAKE_GOD);
                    me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, false);
                    me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, false);
                    DoCast(me, SPELL_VENOM_WITHDRAWAL);
                    me->SetReactState(REACT_AGGRESSIVE);
                    events.ScheduleEvent(EVENT_TOXIC_LINK, 17000);
                    events.ScheduleEvent(EVENT_WHISPER_OF_HETHISS, 16000);
                    events.ScheduleEvent(EVENT_TRANSFORM, 45000);

                    // #679 (CPP EVENT_VENOM_WITHDRAWAL): the old maze fades 3.5 s into Venom Withdrawal, a new one starts 12 s in.
                    std::list<Creature*> clouds;
                    me->GetCreatureListWithEntryInGrid(clouds, NPC_VENOMOUS_EFFUSION, 150.0f);
                    for (Creature* cloud : clouds)
                    {
                        cloud->RemoveAllAuras();
                        cloud->DespawnOrUnsummon(3500);
                    }
                    events.ScheduleEvent(EVENT_VENOMOUS_EFFUSION, 12000);
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
                        case EVENT_TOXIC_LINK:
                            Talk(SAY_TOXIC_LINK);
                            DoCastAOE(SPELL_TOXIC_LINK);
                            events.ScheduleEvent(EVENT_TOXIC_LINK, urand(15000, 19000));
                            break;
                        case EVENT_WHISPER_OF_HETHISS:
                            // #679: Whispers of Hethiss 96466 is an 8 s channel on a random player (SpellMisc AttributesEx 0x4, duration 8 s,
                            // SpellInterrupts 15 / prevention type silence) whose self aura pulses 96469 every 330 ms (heroic 4624 nature).
                            // Wowpedia "High Priest Venoxis (tactics)": interrupt every cast. CPP: random target within 50 yd, 8.5-9 s.
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            {
                                me->StopMoving();
                                DoCast(target, SPELL_WHISPER_OF_HETHISS);
                            }
                            events.ScheduleEvent(EVENT_WHISPER_OF_HETHISS, urand(8500, 9000));
                            break;
                        case EVENT_VENOMOUS_EFFUSION:
                            // #679: the stalker walks the maze path and drops a Venomous Effusion cloud (52288) every 0.5 s.
                            me->SummonCreature(NPC_VENOMOUS_EFFUSION_STALKER, mazeleftPos[0]);
                            break;
                        case EVENT_TRANSFORM:
                            Talk(SAY_TRANSFORM);
                            phase = 1;
                            events.CancelEvent(EVENT_TOXIC_LINK);
                            events.CancelEvent(EVENT_WHISPER_OF_HETHISS);
                            DoCast(me, SPELL_BLESSING_OF_THE_SNAKE_GOD);
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, urand(6000, 9000));
                            events.ScheduleEvent(EVENT_POOL_OF_ACID_TEARS, urand(8000, 10000));
                            events.ScheduleEvent(EVENT_MOVE_DOWN, 40000);
                            break;
                        case EVENT_BREATH_OF_HETHISS:
                            DoCastAOE(SPELL_BREATH_OF_HETHISS);
                            events.ScheduleEvent(EVENT_BREATH_OF_HETHISS, 10000);
                            break;
                        case EVENT_POOL_OF_ACID_TEARS:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_POOL_OF_ACID_TEARS);
                            events.ScheduleEvent(EVENT_POOL_OF_ACID_TEARS, 7000);
                            break;
                        case EVENT_MOVE_DOWN:
                            events.CancelEvent(EVENT_BREATH_OF_HETHISS);
                            events.CancelEvent(EVENT_POOL_OF_ACID_TEARS);
                            phase = 2;
                            me->ApplySpellImmune(0, IMMUNITY_STATE, SPELL_AURA_MOD_TAUNT, true);
                            me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_ATTACK_ME, true);
                            me->AttackStop();
                            me->SetReactState(REACT_PASSIVE);
                            me->GetMotionMaster()->MovePoint(POINT_DOWN, downPos);
                            break;
                        case EVENT_MOVE_UP:
                            me->GetMotionMaster()->Clear();
                            me->GetMotionMaster()->MovePoint(POINT_UP, upPos);
                            break;
                        case EVENT_BLOODVENOM:
                            Talk(SAY_BLOODVENOM);
                            Talk(SAY_ANNOUNCE_BLOODVENOM);
                            DoCastAOE(SPELL_BLOODVENOM);
                            events.ScheduleEvent(EVENT_VENOM_WITHDRAWAL, 15500);
                            break;
                        case EVENT_VENOM_WITHDRAWAL:
                            phase = 0;
                            me->GetMotionMaster()->MovePoint(POINT_WITHDRAWAL, downPos);
                            break;
                    }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_venoxisAI>(creature);
        }
};

class npc_venoxis_bloodvenom : public CreatureScript
{
    public:
        npc_venoxis_bloodvenom() : CreatureScript("npc_venoxis_bloodvenom") { }

        struct npc_venoxis_bloodvenomAI : public ScriptedAI
        {
            npc_venoxis_bloodvenomAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetSpeed(MOVE_RUN, 0.3f);
                me->SetReactState(REACT_PASSIVE);
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            InstanceScript* instance;
            bool bMove;
            uint32 moveTimer;
            ObjectGuid summonerGUID;

            void Reset() override
            {
                bMove = false;
                moveTimer = 1000;
            }

            // #841: Bloodvenom 96842 is a FORCE_CAST (SpellEffect.dbc effect 140, SRC_AREA_ENEMY 150 yd): every player casts 96637, which
            // summons this tendril (52525, SummonProperties 64 = SUMMON_CATEGORY_WILD) next to Venoxis (TARGET_DEST_TARGET_RADIUS, the
            // force-cast target is Venoxis, SpellEffects.cpp EffectForceCast). A wild summon never gets UNIT_FIELD_SUMMONED_BY
            // (SpellEffects.cpp EffectSummonType sets the owner for SUMMON_CATEGORY_ALLY only), so the old `me->GetOwner()` was always
            // null: no tendril ever cast its damage aura 97099 or followed anybody, it sat on the altar and the bots never came within
            // reach ("zg bloodvenom" 0 ticks in w60-w69). The player that summoned it is the TempSummon summoner. Wowpedia
            // "High Priest Venoxis (tactics)": one tendril per living party member, it follows its player and hurts anyone in range.
            void IsSummonedBy(Unit* summoner) override
            {
                summonerGUID = summoner ? summoner->GetGUID() : ObjectGuid::Empty;
                DoCast(me, SPELL_BLOODVENOM_DUMMY, true);
                if (Creature* pVenoxis = me->FindNearestCreature(NPC_VENOXIS, 120.0f))
                    DoCast(pVenoxis, SPELL_VENOM_TOTEM_BEAM_CENTER);
            }

            void UpdateAI(uint32 diff) override
            {
                if (!instance || instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (bMove)
                    return;

                if (moveTimer > diff)
                {
                    moveTimer -= diff;
                    return;
                }

                bMove = true;
                DoCast(me, SPELL_BLOODVENOM_AURA, true); // 97099: pulses 96638 (heroic 18499 nature, 4 yd) every second
                if (Unit* target = ObjectAccessor::GetUnit(*me, summonerGUID))
                    if (target->IsAlive())
                        me->GetMotionMaster()->MoveFollow(target, 0.0f, 0.0f);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_venoxis_bloodvenomAI>(creature);
        }
};

class npc_venoxis_venomous_effusion_stalker : public CreatureScript
{
    public:
        npc_venoxis_venomous_effusion_stalker() : CreatureScript("npc_venoxis_venomous_effusion_stalker") { }

        struct npc_venoxis_venomous_effusion_stalkerAI : public ScriptedAI
        {
            npc_venoxis_venomous_effusion_stalkerAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetSpeed(MOVE_RUN, 0.3f);
                me->SetReactState(REACT_PASSIVE);
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            // #679: 96678 (periodic trigger 96680 every 0.5 s: summons a Venomous Effusion 52288 where the stalker stands, 90 s). Cast once
            // the stalker is in the world (the constructor runs before Map::AddToMap; CPP puts it in creature_template_addon instead).
            void IsSummonedBy(Unit* /*summoner*/) override
            {
                DoCast(me, SPELL_VENOMOUS_EFFUSION, true);
            }
            
            InstanceScript* instance;
            uint8 side;
            uint32 moveTimer;
            uint8 waypoint;

            void Reset() override
            {
                side = (me->GetPositionY() < -1690.0f)? 0: 1;
                waypoint = 1;
                moveTimer = 1;
            }

            void MovementInform(uint32 type, uint32 pointId) override
            {
                if (waypoint == 18)
                    me->DespawnOrUnsummon();
                else
                    moveTimer = 1;
            }

            void JustSummoned(Creature* summon) override
            {
                if (me->IsInCombat())
                    DoZoneInCombat(summon);
            }

            void UpdateAI(uint32 diff) override
            {
                if (instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (moveTimer)
                {
                    me->GetMotionMaster()->Clear();
                    me->GetMotionMaster()->MovePoint(0, mazeleftPos[waypoint]);
                    ++waypoint;
                    moveTimer = 0;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_venoxis_venomous_effusion_stalkerAI>(creature);
        }
};

class npc_venoxis_venomous_effusion : public CreatureScript
{
    public:
        npc_venoxis_venomous_effusion() : CreatureScript("npc_venoxis_venomous_effusion") { }

        struct npc_venoxis_venomous_effusionAI : public ScriptedAI
        {
            npc_venoxis_venomous_effusionAI(Creature* creature) : ScriptedAI(creature)
            {
                me->SetReactState(REACT_PASSIVE);
                instance = creature->GetInstanceScript();
                SetCombatMovement(false);
            }

            InstanceScript* instance;

            // #679: the maze wall does damage now. 96681 pulses 96685 every second: heroic 18849 nature to enemies within 2 yd
            // (SpellEffect.dbc); Wowpedia "High Priest Venoxis (tactics)": 9425-10575 (normal), "slightly larger than the graphic".
            // 96534 is the cloud visual. The bots stay out of it ("zg venomous effusion", CataZg).
            void IsSummonedBy(Unit* /*summoner*/) override
            {
                DoCast(me, SPELL_VENOMOUS_EFFUSION_DUMMY, true);
                DoCast(me, SPELL_VENOMOUS_EFFUSION_AURA, true);
            }

            void UpdateAI(uint32 diff) override
            {
                if (instance->GetBossState(DATA_VENOXIS) != IN_PROGRESS)
                {
                    me->DespawnOrUnsummon();
                    return;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_venoxis_venomous_effusionAI>(creature);
        }
};

class npc_venoxis_venoxis_pool_of_acid_tears : public CreatureScript
{
    public:
        npc_venoxis_venoxis_pool_of_acid_tears() : CreatureScript("npc_venoxis_venoxis_pool_of_acid_tears") { }

        struct npc_venoxis_venoxis_pool_of_acid_tearsAI : public ScriptedAI
        {
            npc_venoxis_venoxis_pool_of_acid_tearsAI(Creature* creature) : ScriptedAI(creature) 
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_POOL_OF_ACID_TEARS_AURA, true);
                SetCombatMovement(false);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_venoxis_venoxis_pool_of_acid_tearsAI>(creature);
        }
};

class spell_venoxis_toxic_link : public SpellScriptLoader
{
    public:
        spell_venoxis_toxic_link() : SpellScriptLoader("spell_venoxis_toxic_link") { }

        class spell_venoxis_toxic_link_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venoxis_toxic_link_SpellScript);
            
            bool Load() override
            {
                bCanLink = false;
                target1 = NULL;
                target2 = NULL;
                return true;
            }

            void HandleDummy()
            {
                if (!GetCaster())
                    return;

                if (bCanLink &&target1 && target1->IsAlive() && target1->IsInWorld() &&
                    target2 && target2->IsAlive() && target2->IsInWorld())
                {
                    GetCaster()->CastSpell(target1, SPELL_TOXIC_LINK_AURA, true);
                    GetCaster()->CastSpell(target2, SPELL_TOXIC_LINK_AURA, true);
                }
            }
            
            void FilterTargets(std::list<WorldObject*>& targets)
            {
                if (targets.size() < 2)
                    bCanLink = false;
                else
                {
                    std::list<WorldObject*>::const_iterator itr = targets.begin();
                    target1 = (*itr)->ToUnit();
                    ++itr;
                    target2 = (*itr)->ToUnit();
                    bCanLink = true;
                }
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_venoxis_toxic_link_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                AfterCast += SpellCastFn(spell_venoxis_toxic_link_SpellScript::HandleDummy);
            }

        private:
            bool bCanLink;
            Unit* target1;
            Unit* target2;
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_venoxis_toxic_link_SpellScript();
        }
};

class spell_venoxis_toxic_link_aura : public SpellScriptLoader
{
    public:
        spell_venoxis_toxic_link_aura() : SpellScriptLoader("spell_venoxis_toxic_link_aura") { }

        class spell_venoxis_toxic_link_aura_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_venoxis_toxic_link_aura_AuraScript);

            void OnRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
            {
                // #679: Toxic Explosion goes off "at the location of each linked player" (Wowpedia "High Priest Venoxis (tactics)").
                // 96489 is TARGET_SRC_CASTER + TARGET_UNIT_SRC_AREA_ALLY (SpellEffect.dbc), so cast by Venoxis on himself it only
                // ever hit his own allies. Each linked player casts it (this hook runs once per link holder), Venoxis stays the
                // original caster; a holder that died is skipped.
                Unit* target = GetTarget();
                if (!target || GetTargetApplication()->GetRemoveMode() == AURA_REMOVE_BY_DEATH)
                    return;

                target->CastSpell(target, SPELL_TOXIC_EXPLOSION, true, nullptr, nullptr, GetCasterGUID());
            }
            
            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (!GetCaster() || !GetTarget())
                    return;

                 UnitList targets;
                 Trinity::AnyUnitHavingBuffInObjectRangeCheck u_check(GetCaster(), GetTarget(), 100, SPELL_TOXIC_LINK_AURA, true);
                 Trinity::UnitListSearcher<Trinity::AnyUnitHavingBuffInObjectRangeCheck> searcher(GetTarget(), targets, u_check);
                 GetTarget()->VisitNearbyObject(80, searcher);

                 if (targets.size() < 2)
                     GetTarget()->RemoveAura(SPELL_TOXIC_LINK_AURA);
                 else
                 {
                     for (UnitList::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                         if ((*itr)->GetGUID() != GetTarget()->GetGUID())
                         {
                             if (!GetTarget()->IsWithinDistInMap((*itr), 30.0f))
                             {
                                 GetTarget()->RemoveAurasDueToSpell(SPELL_TOXIC_LINK_AURA);
                                 (*itr)->RemoveAurasDueToSpell(SPELL_TOXIC_LINK_AURA);
                             }
                             break;
                         }
                 }
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_venoxis_toxic_link_aura_AuraScript::PeriodicTick, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
                AfterEffectRemove += AuraEffectRemoveFn(spell_venoxis_toxic_link_aura_AuraScript::OnRemove, EFFECT_0, SPELL_AURA_PERIODIC_TRIGGER_SPELL, AURA_EFFECT_HANDLE_REAL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_venoxis_toxic_link_aura_AuraScript();
        }
};

// #679: 96466 Whispers of Hethiss. EFFECT_1 is a PERIODIC_TRIGGER_SPELL aura on Venoxis himself (TARGET_UNIT_CASTER), so the generic
// tick (SpellAuraEffects.cpp HandlePeriodicTriggerSpellAuraTick) would cast 96469 (TARGET_UNIT_TARGET_ENEMY) back on Venoxis. Every
// 330 ms tick goes to the channel target instead. Port of CPP TrinityCore boss_high_priest_venoxis.cpp
// spell_venoxis_whispers_of_hethiss.
class spell_venoxis_whispers_of_hethiss : public SpellScriptLoader
{
    public:
        spell_venoxis_whispers_of_hethiss() : SpellScriptLoader("spell_venoxis_whispers_of_hethiss") { }

        class spell_venoxis_whispers_of_hethiss_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_venoxis_whispers_of_hethiss_AuraScript);

            void HandleTick(AuraEffect const* /*aurEff*/)
            {
                PreventDefaultAction();
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                if (Unit* target = ObjectAccessor::GetUnit(*caster, caster->GetGuidValue(UNIT_FIELD_CHANNEL_OBJECT)))
                    if (target->IsAlive())
                        caster->CastSpell(target, SPELL_WHISPER_OF_HETHISS_DMG, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_venoxis_whispers_of_hethiss_AuraScript::HandleTick, EFFECT_1, SPELL_AURA_PERIODIC_TRIGGER_SPELL);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_venoxis_whispers_of_hethiss_AuraScript();
        }
};

class ExactDistanceCheck
{
    public:
        ExactDistanceCheck(Unit* source, float dist) : _source(source), _dist(dist) { }

        bool operator()(WorldObject* unit)
        {
            return _source->GetExactDist2d(unit) > _dist;
        }

    private:
        Unit* _source;
        float _dist;
};

class spell_venoxis_pool_of_acid_tears_dmg : public SpellScriptLoader
{
    public:
        spell_venoxis_pool_of_acid_tears_dmg() : SpellScriptLoader("spell_venoxis_pool_of_acid_tears_dmg") { }

        class spell_venoxis_pool_of_acid_tears_dmg_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_venoxis_pool_of_acid_tears_dmg_SpellScript);

            void CorrectRange(std::list<WorldObject*>& targets)
            {
                targets.remove_if(ExactDistanceCheck(GetCaster(), 2.0f * GetCaster()->GetFloatValue(OBJECT_FIELD_SCALE)));
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_venoxis_pool_of_acid_tears_dmg_SpellScript::CorrectRange, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_venoxis_pool_of_acid_tears_dmg_SpellScript();
        }
};

void AddSC_boss_venoxis()
{
    new boss_venoxis();
    new npc_venoxis_venomous_effusion_stalker();
    new npc_venoxis_venomous_effusion();
    new npc_venoxis_bloodvenom();
    new npc_venoxis_venoxis_pool_of_acid_tears();
    new spell_venoxis_toxic_link();
    new spell_venoxis_toxic_link_aura();
    new spell_venoxis_whispers_of_hethiss();
    new spell_venoxis_pool_of_acid_tears_dmg();
}
