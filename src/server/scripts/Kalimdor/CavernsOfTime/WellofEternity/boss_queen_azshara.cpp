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
#include "well_of_eternity.h"

enum ScriptedTexts
{
    SAY_AGGRO       = 0,
    SAY_INTERRUPT   = 1,
    SAY_END_1       = 2,
    SAY_END_2       = 3,
    SAY_END_3       = 4,
    SAY_WIPE        = 5,
    SAY_KILL        = 6,
    SAY_PUPPET      = 7,
    SAY_ALL         = 8,
    SAY_ADDS_1      = 9,
    SAY_ADDS_2      = 10,
    SAY_ADDS_3      = 11,
};

enum Spells
{
    SPELL_SERVANT_OF_THE_QUEEN      = 102334,
    SPELL_TOTAL_OBEDIENCE           = 103241,
    SPELL_TOTAL_OBEDIENCE_1         = 110096, // ?

    // Enchanted Magus
    SPELL_ARCANE_SHOCK              = 102463,
    SPELL_HAMMER_OF_DIVINITY_1      = 102454,
    SPELL_HAMMER_OF_DIVINITY_2      = 102454,
    SPELL_HAMMER_OF_DIVINITY_DUMMY  = 102460,
    SPELL_ARCANE_BOMB               = 102455,
    SPELL_ARCANE_BOMB_DUMMY         = 109122,
    SPELL_FIREBALL                  = 102265,
    SPELL_FIREBOMB                  = 102482,
    SPELL_BLASTWAVE                 = 102483,
    SPELL_BLADES_OF_ICE             = 102467,
    SPELL_BLADES_OF_ICE_DMG         = 102468,
    SPELL_ICE_FLING                 = 102478,
    SPELL_COLDFLAME                 = 102465,
    SPELL_COLDFLAME_DMG             = 102466,

    SPELL_PUPPET_CROSS              = 102310,
    SPELL_PUPPET_STRING_HOVER       = 102312,
    SPELL_PUPPET_STRING_DUMMY_1     = 102315,
    SPELL_PUPPET_STRING_DUMMY_2     = 102318,
    SPELL_PUPPET_STRING_DUMMY_3     = 102319,
    SPELL_PUPPET_STRING_SCRIPT_1    = 102333,
    SPELL_PUPPET_STRING_SCRIPT_2    = 102345,
};

enum Adds
{
    NPC_HAND_OF_QUEEN           = 54728,
    NPC_ENCHANTED_MAGUS_ARCANE  = 54884,
    NPC_ENCHANTED_MAGUS_FIRE    = 54882,
    NPC_ENCHANTED_MAGUS_FROST   = 54883,
    NPC_HAMMER_OF_DIVINITY_1    = 54864,
    NPC_HAMMER_OF_DIVINITY_2    = 54865,
};

enum Events
{   
    EVENT_ADDS_1                = 1,
    EVENT_ADDS_2                = 2,
    EVENT_ADDS_3                = 3,
    EVENT_SERVANT_OF_THE_QUEEN  = 4,
    EVENT_TOTAL_OBEDIENCE       = 5,
    EVENT_END                   = 6,
    EVENT_ARCANE_SHOCK          = 7,
    EVENT_ARCANE_BOMB           = 8,
    EVENT_FIREBALL              = 9,
    EVENT_FIREBOMB              = 10,
    EVENT_BLASTWAVE             = 11,
    EVENT_BLADES_OF_ICE         = 12,
    EVENT_COLDFLAME             = 13,
    EVENT_ICE_FLING             = 14,
};

enum Actions
{
    ACTION_ATTACK   = 1,
};

const Position addsPos[6] = 
{
    {3453.030029f, -5282.740234f, 230.04f, 4.45f}, // fire 1
    {3443.540039f, -5280.370117f, 230.04f, 4.66f}, // frost 1
    {3461.139893f, -5283.169922f, 230.04f, 4.31f}, // arcane 1
    {3435.590088f, -5278.350098f, 230.04f, 4.50f}, // fire 2
    {3469.729980f, -5282.430176f, 230.04f, 4.53f}, // frost 2
    {3428.43f,     -5274.59f,     230.04f, 4.20f}  // arcane 2
};

const uint32 addsEntry[6] =
{
    NPC_ENCHANTED_MAGUS_FIRE,
    NPC_ENCHANTED_MAGUS_FROST,
    NPC_ENCHANTED_MAGUS_ARCANE,
    NPC_ENCHANTED_MAGUS_FIRE,
    NPC_ENCHANTED_MAGUS_FROST,
    NPC_ENCHANTED_MAGUS_ARCANE
};

class boss_queen_azshara : public CreatureScript
{
    public:
        boss_queen_azshara() : CreatureScript("boss_queen_azshara") { }

        struct boss_queen_azsharaAI : public BossAI
        {
            boss_queen_azsharaAI(Creature* creature) : BossAI(creature, DATA_AZSHARA)
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

            void Reset() override
            {
                _Reset();

                memset(addsGUIDs, 0, sizeof(addsGUIDs));

                for (uint8 i = 0; i < 6; ++i)
                    if (Creature* pAdd = me->SummonCreature(addsEntry[i], addsPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms))
                        addsGUIDs[i] = pAdd->GetGUID();

                addsCount = 0;
                handGUID.Clear();
                ReleasePuppets();
            }

            // #695 "You are mine now!" (Wowpedia "Queen Azshara (tactics)", 2026-10-23): "Periodically, Queen Azshara will charm a
            // random target, causing them to become a puppet. Players will be released from her control when the puppet strings are
            // defeated." Servant of the Queen 102334 (DBC: SET_VEHICLE_ID 1745 + AOE_CHARM + immunity to all damage) goes on one
            // player; the Hand of the Queen (54728) rides the puppet's seat (VehicleSeat 10164) and its death frees the puppet
            // (npc_queen_azshara_hand_of_the_queen). The spell's own area target (SRC_AREA_ENEMY, 100 yd) would puppet the whole
            // group, so the aura is added to the chosen player only. One puppet at a time.
            void ReleasePuppets()
            {
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->GetSource())
                        if (player->HasAura(SPELL_SERVANT_OF_THE_QUEEN))
                            player->RemoveAurasDueToSpell(SPELL_SERVANT_OF_THE_QUEEN);
            }

            void CastServantOfTheQueen()
            {
                if (!handGUID.IsEmpty() && ObjectAccessor::GetCreature(*me, handGUID))
                    return;
                handGUID.Clear();

                uint32 alive = 0;
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->GetSource())
                        if (player->IsAlive() && !player->IsGameMaster() && me->IsWithinDistInMap(player, 100.0f))
                            ++alive;
                if (alive < 2)  // puppeting the last player standing ends the fight on the spot
                    return;

                // not the current victim when anyone else is there (position 1 skips the top of the threat list)
                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 100.0f, true, -int32(SPELL_SERVANT_OF_THE_QUEEN));
                if (!target)
                    target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true, -int32(SPELL_SERVANT_OF_THE_QUEEN));
                Player* puppet = target ? target->ToPlayer() : nullptr;
                if (!puppet || puppet->IsCharmed() || puppet->GetVehicle() || puppet->GetVehicleKit())
                    return;

                if (!me->AddAura(SPELL_SERVANT_OF_THE_QUEEN, puppet))
                    return;

                Talk(SAY_PUPPET);
                if (Creature* hand = me->SummonCreature(NPC_HAND_OF_QUEEN, puppet->GetPositionX(), puppet->GetPositionY(), puppet->GetPositionZ(),
                    puppet->GetOrientation(), TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms))
                {
                    handGUID = hand->GetGUID();
                    hand->AI()->SetGUID(puppet->GetGUID());
                    if (puppet->GetVehicleKit())
                        hand->EnterVehicle(puppet, 0);
                }
                else
                    puppet->RemoveAurasDueToSpell(SPELL_SERVANT_OF_THE_QUEEN);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);

                addsCount = 0;

                events.RescheduleEvent(EVENT_ADDS_1, 11000);

                DoZoneInCombat();
                instance->SetBossState(DATA_AZSHARA, IN_PROGRESS);
            }

            void JustReachedHome() override
            {
                Talk(SAY_WIPE);
                addsCount = 0;
                ReleasePuppets();
            }

            void AttackStart(Unit* target) override
            {
                if (target)
                    me->Attack(target, false);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim && victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }
            
            void SpellHit(Unit* /*caster*/, const SpellInfo* spellInfo) override
            {
                if (Spell* spell = me->GetCurrentSpell(CURRENT_GENERIC_SPELL))
                    if (spellInfo->HasEffect(SPELL_EFFECT_INTERRUPT_CAST))
                    {
                        me->InterruptSpell(CURRENT_GENERIC_SPELL);
                        Talk(SAY_INTERRUPT);
                    }
            }

            void DamageTaken(Unit* attacker, uint32& damage) override
            {
                damage = 0;
                me->UpdateUInt32Value(UNIT_FIELD_HEALTH, me->GetMaxHealth());
            }

            void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
            {
                if (!me->IsInCombat())
                    return;

                // #695: the Hand of the Queen is a summon too; only a magus counts towards the waves and the end
                if (summon->GetEntry() == NPC_HAND_OF_QUEEN)
                {
                    handGUID.Clear();
                    return;
                }

                if (summon->GetEntry() == NPC_ENCHANTED_MAGUS_FIRE ||
                    summon->GetEntry() == NPC_ENCHANTED_MAGUS_ARCANE ||
                    summon->GetEntry() == NPC_ENCHANTED_MAGUS_FROST)
                    addsCount++;
                else
                    return;
                
                if ((addsCount == 2) || (addsCount == 4))
                    events.ScheduleEvent(EVENT_ADDS_1, 6000);
                else if (addsCount == 6)
                {
                    events.Reset();
                    me->InterruptNonMeleeSpells(false);
                    Talk(SAY_END_1);
                    events.ScheduleEvent(EVENT_END, 6000);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_TOTAL_OBEDIENCE:
                            Talk(SAY_ALL);
                            DoCastAOE(SPELL_TOTAL_OBEDIENCE);
                            break;
                        case EVENT_ADDS_1:
                            Talk((addsCount / 2) + 9);
                            events.RescheduleEvent(EVENT_ADDS_2, 7000);
                            break;
                        case EVENT_ADDS_2:
                            if (Creature* pAdd = ObjectAccessor::GetCreature(*me, addsGUIDs[addsCount]))
                                pAdd->AI()->Talk(addsCount / 2);
                            events.RescheduleEvent(EVENT_ADDS_3, 6000);
                            break;
                        case EVENT_ADDS_3:
                            for (uint8 i = addsCount; i < (addsCount + 2); ++i)
                            {
                                if (Creature* pAdd = ObjectAccessor::GetCreature(*me, addsGUIDs[i]))
                                {
                                    pAdd->SetReactState(REACT_AGGRESSIVE);
                                    pAdd->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_IMMUNE_TO_PC);
                                    DoZoneInCombat(pAdd);
                                    pAdd->AI()->DoAction(ACTION_ATTACK);
                                }
                            }
                            events.ScheduleEvent(EVENT_TOTAL_OBEDIENCE, urand(10000, 20000));
                            if (addsCount == 0)   // #695: the puppets start with the first pair of magi
                                events.ScheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, urand(15000, 25000));
                            break;
                        case EVENT_SERVANT_OF_THE_QUEEN:
                            CastServantOfTheQueen();
                            events.ScheduleEvent(EVENT_SERVANT_OF_THE_QUEEN, urand(30000, 40000));
                            break;
                        case EVENT_END:
                        {    
                            instance->DoKilledMonsterKredit(QUEST_THE_VAINGLORIOUS, 54853, ObjectGuid::Empty);

                            instance->SetBossState(DATA_AZSHARA, DONE);
                            instance->DoRespawnGameObject(instance->GetGuidData(DATA_ROYAL_CACHE), DAY);
                            instance->DoModifyPlayerCurrencies(CURRENCY_TYPE_JUSTICE_POINTS, 7000);
                            ReleasePuppets();   // #695
                            me->DespawnOrUnsummon();
                            break;
                        }
                        default:
                            break;
                    }
                }
            }

        private:
            EventMap events;
            uint8 phase;
            ObjectGuid addsGUIDs[6];
            uint8 addsCount;
            ObjectGuid handGUID;   // #695: the live Hand of the Queen (one puppet at a time)
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_queen_azsharaAI>(creature);
        }
};

class npc_queen_azshara_enchanted_magus : public CreatureScript
{
    public:
        npc_queen_azshara_enchanted_magus() : CreatureScript("npc_queen_azshara_enchanted_magus") { }

        struct npc_queen_azshara_enchanted_magusAI : public ScriptedAI
        {
            npc_queen_azshara_enchanted_magusAI(Creature* creature) : ScriptedAI(creature) { }

            void Reset() override
            {
                events.Reset();
                me->SetReactState(REACT_PASSIVE);
            }

            void DoAction(int32 action) override
            {
                if (action == ACTION_ATTACK)
                {
                    if (me->GetEntry() == NPC_ENCHANTED_MAGUS_FIRE)
                    {
                        events.ScheduleEvent(EVENT_FIREBALL, 1000);
                        events.ScheduleEvent(EVENT_FIREBOMB, urand(10000, 15000));
                        events.ScheduleEvent(EVENT_BLASTWAVE, urand(10000, 20000));
                    }
                    else if (me->GetEntry() == NPC_ENCHANTED_MAGUS_FROST)
                    {
                        events.ScheduleEvent(EVENT_BLADES_OF_ICE, urand(5000, 10000));
                        events.ScheduleEvent(EVENT_COLDFLAME, urand(12000, 20000));
                        events.ScheduleEvent(EVENT_ICE_FLING, urand(2000, 15000));
                    }
                    else if (me->GetEntry() == NPC_ENCHANTED_MAGUS_ARCANE)
                    {
                        events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(10000, 15000));
                        events.ScheduleEvent(EVENT_ARCANE_BOMB, urand(5000, 10000));
                    }
                }
            }
        
            /*void MovementInform(uint32 type, uint32 pointId) override
            {
                if (me->GetEntry() == NPC_ENCHANTED_MAGUS_FROST && data == EVENT_CHARGE)
                    if (Player* player = me->SelectNearestPlayer(5.0f))
                        DoCast(player, SPELL_BLADES_OF_ICE_DMG, true);
            }*/

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_FIREBALL:
                            DoCastVictim(SPELL_FIREBALL);
                            events.ScheduleEvent(EVENT_FIREBALL, 3000);
                            break;
                        case EVENT_FIREBOMB:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_FIREBOMB);
                            events.ScheduleEvent(EVENT_FIREBOMB, urand(15000, 20000));
                            break;
                        case EVENT_BLASTWAVE:
                            DoCastAOE(SPELL_BLASTWAVE);
                            events.ScheduleEvent(EVENT_BLASTWAVE, urand(15000, 20000));
                            break;
                        case EVENT_ICE_FLING:
                            DoCastAOE(SPELL_ICE_FLING);
                            events.ScheduleEvent(EVENT_ICE_FLING, urand(10000, 20000));
                            break;
                        case EVENT_BLADES_OF_ICE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_BLADES_OF_ICE);
                            events.ScheduleEvent(EVENT_BLADES_OF_ICE, urand(12000, 20000));
                            break;
                        case EVENT_COLDFLAME:
                            // #695: Frost Magus Coldflame (Wowpedia "Queen Azshara (tactics)": "Deals 45000 Frost damage in a line");
                            // 102465 (1.5 s cast) + its periodic aura spell_queen_azshara_coldflame lays 102466 patches in front of
                            // the magus (spell_script_names 102465). The cast faces the target, so the line runs at that player.
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                                DoCast(target, SPELL_COLDFLAME);
                            events.ScheduleEvent(EVENT_COLDFLAME, urand(16000, 25000));
                            break;
                        case EVENT_ARCANE_SHOCK:
                            me->StopMoving();
                            DoCast(me, SPELL_ARCANE_SHOCK);
                            events.ScheduleEvent(EVENT_ARCANE_SHOCK, urand(12000, 20000));
                            break;
                        case EVENT_ARCANE_BOMB:
                        {
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                Creature* pStalker1 = me->SummonCreature(NPC_HAMMER_OF_DIVINITY_2, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000ms);
                                Creature* pStalker2 = me->SummonCreature(NPC_HAMMER_OF_DIVINITY_1, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ() + 30.0f, target->GetOrientation(), TEMPSUMMON_TIMED_DESPAWN, 20000ms);
                                
                                if (pStalker1 && pStalker2)
                                    pStalker2->GetMotionMaster()->MovePoint(0, pStalker1->GetPositionX(), pStalker1->GetPositionY(), pStalker1->GetPositionZ()); 
                            }
                            events.ScheduleEvent(EVENT_ARCANE_BOMB, urand(18000, 25000));
                            break;
                        }
                        default:
                            break;
                    }
                }

                DoMeleeAttackIfReady();
            }

        private:
            EventMap events;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_queen_azshara_enchanted_magusAI>(creature);
        }
};

class npc_queen_azshara_hammer_of_divinity : public CreatureScript
{
    public:
        npc_queen_azshara_hammer_of_divinity() : CreatureScript("npc_queen_azshara_hammer_of_divinity") { }

        struct npc_queen_azshara_hammer_of_divinityAI : public ScriptedAI
        {
            npc_queen_azshara_hammer_of_divinityAI(Creature* creature) : ScriptedAI(creature)
            {
                bDespawn = false;
                me->SetSpeed(MOVE_RUN, 0.1f, true); 
                me->SetCanFly(true);
                me->SetDisableGravity(true);
                SetCombatMovement(false);
            }

            void UpdateAI(uint32 /*diff*/) override
            {
                if (bDespawn)
                    return;

                if (Creature* pStalker = me->FindNearestCreature(NPC_HAMMER_OF_DIVINITY_2, 1.0f))
                {
                    bDespawn = true;
                    DoCastAOE(SPELL_ARCANE_BOMB);
                    pStalker->DespawnOrUnsummon(500);
                    me->DespawnOrUnsummon(500);
                }
            }

        private:
            bool bDespawn;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_queen_azshara_hammer_of_divinityAI>(creature);
        }
};

// #695: the Hand of the Queen (54728) - holds the puppet's strings while it rides the puppet; killing it frees the puppet.
class npc_queen_azshara_hand_of_the_queen : public CreatureScript
{
    public:
        npc_queen_azshara_hand_of_the_queen() : CreatureScript("npc_queen_azshara_hand_of_the_queen") { }

        struct npc_queen_azshara_hand_of_the_queenAI : public ScriptedAI
        {
            npc_queen_azshara_hand_of_the_queenAI(Creature* creature) : ScriptedAI(creature)
            {
                SetCombatMovement(false);
                me->SetReactState(REACT_PASSIVE);
                checkTimer = 2000;
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_PUPPET_CROSS, true);
            }

            void SetGUID(ObjectGuid guid, int32 /*type*/) override
            {
                puppetGUID = guid;
            }

            void JustDied(Unit* /*killer*/) override
            {
                // setDeathState(JUST_DIED) has already taken the Hand off the puppet's seat; dropping the aura now removes the
                // puppet's vehicle kit and the charm (HandleAuraSetVehicle / HandleCharmConvert).
                if (Unit* puppet = ObjectAccessor::GetUnit(*me, puppetGUID))
                    puppet->RemoveAurasDueToSpell(SPELL_SERVANT_OF_THE_QUEEN);
                puppetGUID.Clear();
            }

            void UpdateAI(uint32 diff) override
            {
                if (checkTimer > diff)
                {
                    checkTimer -= diff;
                    return;
                }
                checkTimer = 1000;

                // the puppet died, left or was released some other way (Azshara's reset/end): the strings hold nothing
                Unit* puppet = ObjectAccessor::GetUnit(*me, puppetGUID);
                if (!puppet || !puppet->IsAlive() || !puppet->HasAura(SPELL_SERVANT_OF_THE_QUEEN))
                {
                    puppetGUID.Clear();
                    me->DespawnOrUnsummon();
                }
            }

        private:
            ObjectGuid puppetGUID;
            uint32 checkTimer;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_queen_azshara_hand_of_the_queenAI>(creature);
        }
};

class spell_queen_azshara_coldflame : public SpellScriptLoader
{
    public:
        spell_queen_azshara_coldflame() : SpellScriptLoader("spell_queen_azshara_coldflame") { }

        class spell_queen_azshara_coldflame_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_queen_azshara_coldflame_AuraScript);
            
            bool Load() override
            {
                count = 0;
                return true;
            }

            void PeriodicTick(AuraEffect const* /*aurEff*/)
            {
                if (!GetCaster())
                    return;
                
                count++;

                if (count > 11)
                {
                    GetCaster()->RemoveAura(SPELL_COLDFLAME);
                    return;
                }

                Position pos = GetCaster()->GetNearPosition(3.0f * (count / 2), 0.0f);
                GetCaster()->CastSpell(pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), SPELL_COLDFLAME_DMG, true);
            }

            void Register() override
            {
                OnEffectPeriodic += AuraEffectPeriodicFn(spell_queen_azshara_coldflame_AuraScript::PeriodicTick, EFFECT_1, SPELL_AURA_PERIODIC_DUMMY);
            }

        private:
            uint8 count;
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_queen_azshara_coldflame_AuraScript();
        }
};

class spell_queen_azshara_arcane_bomb : public SpellScriptLoader
{
    public:
        spell_queen_azshara_arcane_bomb() : SpellScriptLoader("spell_queen_azshara_arcane_bomb") { }

        class spell_queen_azshara_arcane_bomb_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_queen_azshara_arcane_bomb_SpellScript);

            void ChangeSummonPos(SpellEffIndex /*effIndex*/)
            {
                WorldLocation summonPos = *GetExplTargetDest();
                Position offset = {0.0f, 0.0f, 15.0f, 0.0f};
                summonPos.RelocateOffset(offset);
                SetExplTargetDest(summonPos);
                GetHitDest()->RelocateOffset(offset);
            }

            void Register() override
            {
                OnEffectHit += SpellEffectFn(spell_queen_azshara_arcane_bomb_SpellScript::ChangeSummonPos, EFFECT_0, SPELL_EFFECT_SUMMON);
            }
        };


        SpellScript* GetSpellScript() const override
        {
            return new spell_queen_azshara_arcane_bomb_SpellScript();
        }
};

void AddSC_boss_queen_azshara()
{
    new boss_queen_azshara();
    new npc_queen_azshara_enchanted_magus();
    new npc_queen_azshara_hammer_of_divinity();
    new npc_queen_azshara_hand_of_the_queen();   // #695
    new spell_queen_azshara_coldflame();
    //new spell_queen_azshara_arcane_bomb();
}
