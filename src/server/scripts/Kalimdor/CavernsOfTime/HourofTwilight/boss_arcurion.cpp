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
#include "hour_of_twilight.h"

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_DEATH   = 1,
    SAY_PHASE   = 2,
    SAY_KILL    = 3,
    SAY_TOMB    = 4,
};

enum Spells
{
    SPELL_ARCURION_APPEARS          = 104767,
    SPELL_HAND_OF_FROST             = 102593,
    SPELL_CHAINS_OF_FROST           = 102582,
    SPELL_ICY_TOMB                  = 103252,
    SPELL_ICY_TOMB_EFF              = 103251,
    SPELL_ICY_TOMB_SUMMON_TRIGGER   = 103250,
    SPELL_ICY_TOMB_SUMMON           = 103249,
    SPELL_TORRENT_OF_FROST          = 104050,

    // Frozen Servitor
    SPELL_ICY_BOULDER_AOE           = 102480,
    SPELL_ICY_BOULDER               = 102198,
    SPELL_ICY_BOULDER_DMG           = 102199,
};

enum Events
{
    EVENT_HAND_OF_FROST     = 1,
    EVENT_CHAINS_OF_FROST   = 2,
    EVENT_ICY_TOMB          = 3,
    EVENT_ICY_BOULDER       = 4,
    EVENT_FROZEN_SERVITOR   = 5,
};

enum Adds
{
    NPC_SPAWN_STALKER   = 57197,
};

#define MAX_SERVITOR 6
const Position servitorPos[MAX_SERVITOR] = 
{
    { 4810.14f, 31.5191f, 104.593f, 2.3692f  },
    { 4831.26f, 64.6198f, 108.553f, 3.28342f },
    { 4827.03f, 50.566f,  108.63f,  3.05247f },
    { 4796.06f, 131.432f, 132.468f, 4.60976f },
    { 4788.47f, 125.67f,  129.112f, 4.73712f },
    { 4777.38f, 30.809f,  92.5167f, 1.58226f },
  /*{ 4842.17f, 110.13f,  107.272f, 4.03014f },
    { 4737.55f, 75.5538f, 105.757f, 5.68455f },
    { 4838.94f, 90.1892f, 108.409f, 3.56877f },
    { 4771.36f, 110.743f, 121.498f, 5.00128f },
    { 4818.98f, 44.75f,   106.324f, 3.22131f },
    { 4750.13f, 97.3125f, 112.217f, 5.8437f  },
    { 4756.07f, 103.248f, 114.95f,  5.74322f },
    { 4739.72f, 84.1997f, 107.23f,  5.30915f }*/
};

class boss_arcurion : public CreatureScript
{
    public:
        boss_arcurion() : CreatureScript("boss_arcurion") { }

        struct boss_arcurionAI : public BossAI
        {
            boss_arcurionAI(Creature* creature) : BossAI(creature, DATA_ARCURION)
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
                phase = 0;
                memset(servitors, 0, sizeof(servitors));
            }

            void InitializeAI() override
            {
                if (!instance || static_cast<InstanceMap*>(me->GetMap())->GetScriptId() != sObjectMgr->GetScriptId(HoTScriptName))
                    me->IsAIEnabled = false;
                else if (!me->isDead())
                    Reset();
            }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                SetEquipmentSlots(false, MagharStaff, 0, EQUIP_NO_CHANGE);
            }

            void Reset() override
            {
                _Reset();
                phase = 0;
                evadeNoPlayer = false;
                memset(servitors, 0, sizeof(servitors));
            }

            // Refs #824 (w61, EVT 1167696/7: the last player died - SAY_KILL - and in the same ms Arcurion said SAY_AGGRO
            // again and Thrall answered with ACTION_ARCURION's line: the encounter restarted against Thrall alone and
            // stayed IN_PROGRESS with no players). Only players (and their pets/guardians) pull Arcurion; Thrall 55779
            // waits at point 20, ~27 yd away.
            void MoveInLineOfSight(Unit* who) override
            {
                if (!who || !who->GetCharmerOrOwnerPlayerOrPlayerItself())
                    return;

                BossAI::MoveInLineOfSight(who);
            }

            // Refs #824: every evade (wipe) also frees Thrall - out of his Icy Tomb and out of combat, back to his spot.
            void EnterEvadeMode(EvadeReason why = EVADE_REASON_OTHER) override
            {
                evadeNoPlayer = false;
                if (me->IsAlive())
                    if (Creature* thrall = me->FindNearestCreature(NPC_THRALL_2, 200.0f))
                        if (CreatureAI* thrallAI = thrall->AI())
                            thrallAI->DoAction(ACTION_ARCURION_WIPE);

                BossAI::EnterEvadeMode(why);
            }

            // Spirit of Redemption counts as alive but cannot be attacked (Unit::_IsValidAttackTarget), so it never holds the fight.
            bool AnyLivePlayerNear() const
            {
                Map::PlayerList const& players = me->GetMap()->GetPlayers();
                for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
                    if (Player* player = itr->GetSource())
                        if (player->IsAlive() && !player->IsGameMaster() && !player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION)
                            && player->IsWithinDistInMap(me, 100.0f))
                            return true;
                return false;
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                // Refs #824: engaged with no live player around - do not start the encounter. The evade runs on the next
                // UpdateAI, outside the combat-start call chain (CombatManager::SetInCombatWith -> Unit::SetInCombatState ->
                // ... -> EngagementStart -> here). w105: a player-owned `who` is no exemption any more - ~9 s after the forced
                // wipe (all four bots dead, the master ~266 yd away) Arcurion re-engaged three times in 1.4 s (SAY_AGGRO + Thrall's
                // ACTION_ARCURION line each, three "DoZoneInCombat ... empty threat list (54590)" errors) with a source that
                // passed the old `who` test: a dead caster's periodic aura or a dead owner's guardian. Every real pull has a
                // live player within 100 yd.
                if (!AnyLivePlayerNear())
                {
                    evadeNoPlayer = true;
                    return;
                }

                Talk(SAY_AGGRO);
                _JustEngagedWith();
                phase = 0;
                memset(servitors, 0, sizeof(servitors));

                events.ScheduleEvent(EVENT_HAND_OF_FROST, 1000);
                events.ScheduleEvent(EVENT_CHAINS_OF_FROST, 15000);
                events.ScheduleEvent(EVENT_FROZEN_SERVITOR, 1);
                events.ScheduleEvent(EVENT_ICY_TOMB, 25000);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);
                
                SendThrallInCombat();
            }

            void SendThrallInCombat()
            {
                if (Creature* Thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_2, 100.0f))
                {
                    Thrall->AI()->DoAction(ACTION_ARCURION);
                    me->SetInCombatWith(Thrall);
                }
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                DeleteServitor(summon->GetGUID());
                BossAI::SummonedCreatureDespawn(summon);
            }

            void KilledUnit(Unit* victim) override
            {
                if (victim && victim->GetTypeId() == TYPEID_PLAYER)
                    Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                if (Creature* thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_2, 200.0f))
                {
                    thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB);
                    thrall->RemoveAurasDueToSpell(SPELL_ICY_TOMB_EFF);
                    thrall->AI()->DoAction(ACTION_TALK_ARCURION_DEATH);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (evadeNoPlayer)
                {
                    evadeNoPlayer = false;
                    EnterEvadeMode();
                    return;
                }

                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (me->HealthBelowPct(30) && phase == 0)
                {
                    phase = 1;
                    Talk(SAY_PHASE);
                    events.Reset();
                    summons.DespawnEntry(NPC_FROZEN_SERVITOR);

                    if (Creature* Thrall = GetClosestCreatureWithEntry(me, NPC_THRALL_2, 100.0f))
                        Thrall->AI()->DoAction(ACTION_TALK_FINISH_HIM);

                    DoCast(me, SPELL_TORRENT_OF_FROST);
                    return;
                }

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_HAND_OF_FROST:
                            DoCastVictim(SPELL_HAND_OF_FROST);
                            events.ScheduleEvent(EVENT_HAND_OF_FROST, 2000);
                            break;
                        case EVENT_CHAINS_OF_FROST:
                            DoCastAOE(SPELL_CHAINS_OF_FROST);
                            events.ScheduleEvent(EVENT_CHAINS_OF_FROST, urand(15000, 20000));
                            break;
                        case EVENT_ICY_TOMB:
                            Talk(SAY_TOMB);
                            if (Creature* pThrall = me->FindNearestCreature(NPC_THRALL_2, 100.0f))
                            {
                                DoCast(pThrall, SPELL_ICY_TOMB);
                                me->AddAura(SPELL_ICY_TOMB, pThrall);
                                pThrall->SummonCreature(NPC_ICY_TOMB, pThrall->GetPositionX(), pThrall->GetPositionY(), pThrall->GetPositionZ(), pThrall->GetOrientation(), TEMPSUMMON_MANUAL_DESPAWN);
                            }
                            events.ScheduleEvent(EVENT_ICY_TOMB, 50000);
                            break;
                        case EVENT_FROZEN_SERVITOR:
                            SummonNewServitor();
                            events.ScheduleEvent(EVENT_FROZEN_SERVITOR, urand(10000, 15000));
                            break;
                        default:
                            break;
                    }
                }

                //DoMeleeAttackIfReady();
            }
        private:
            uint8 phase;
            bool evadeNoPlayer = false; // Refs #824: set in JustEngagedWith, evade on the next UpdateAI
            uint64 servitors[MAX_SERVITOR];

            void SummonNewServitor()
            {
                for (uint8 i = 0; i < MAX_SERVITOR; ++i)
                {
                    if (servitors[i] == 0)
                    {
                        if (Creature* pServitor = me->SummonCreature(NPC_FROZEN_SERVITOR, servitorPos[i], TEMPSUMMON_CORPSE_TIMED_DESPAWN, 3000ms))
                        {
                            servitors[i] = pServitor->GetGUID();
                            break;
                        }
                    }
                }
            }

            void DeleteServitor(uint64 guid)
            {
                for (uint8 i = 0; i < MAX_SERVITOR; ++i)
                    if (servitors[i] == guid)
                        servitors[i] = 0;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new boss_arcurionAI(creature);
        }
};

class npc_arcurion_frozen_servitor : public CreatureScript
{
    public:
        npc_arcurion_frozen_servitor() : CreatureScript("npc_arcurion_frozen_servitor") { }

        struct npc_arcurion_frozen_servitorAI : public ScriptedAI
        {
            npc_arcurion_frozen_servitorAI(Creature* creature) : ScriptedAI(creature), instance(creature->GetInstanceScript()) {}

                void IsSummonedBy(Unit* /*summoner*/) override
                {
                    events.ScheduleEvent(EVENT_ICY_BOULDER, urand(1000, 3000));
                    if (instance->GetBossState(DATA_ARCURION) == IN_PROGRESS)
                    {
                        me->SetReactState(REACT_PASSIVE);
                        me->AttackStop();
                        me->StopMoving();
                        me->HandleEmoteStateCommand(EMOTE_STATE_READY1H);
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
                        // Refs #696: the glyphs. Wowpedia "Arcurion" (MediaWiki API, 2026-10-22): "all members must move out of the
                        // glyphs that appear on the ground to avoid damage"; Wowhead tooltip 102198 Icy Boulder: "3 sec cast", 102199:
                        // damage "to enemies in the area" (SpellEffect.dbc radius 2.5 yd, TRIGGER_MISSILE 102198 -> 102199). The old
                        // path (102480 AoE dummy -> triggered 102198, spell_arcurion_icy_boulder) skipped the cast time, so the
                        // boulder hit the player it picked at once, with nothing to dodge. Now the servitor marks the spot the
                        // player stands on (an invisible World Trigger that lives past the impact) and casts the boulder at that
                        // marker with its 3 s cast: TARGET_DEST_TARGET_ANY lands it on the marker, not on the player, so moving out
                        // of the spot avoids it. The bots read the marker (CataHot "hot icy boulder").
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 150.0f, true))
                            if (Creature* glyph = me->SummonCreature(NPC_WORLD_TRIGGER_HOT, target->GetPositionX(), target->GetPositionY(),
                                target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 7000ms))
                                me->CastSpell(glyph, SPELL_ICY_BOULDER, false);
                        events.ScheduleEvent(EVENT_ICY_BOULDER, urand(5000, 10000));
                    }

                    DoMeleeAttackIfReady();

                }
            private:
                EventMap events;
                InstanceScript* instance;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_arcurion_frozen_servitorAI(creature);
        }
};

// Icy Tomb 54995
class npc_icy_tomb : public CreatureScript
{
    public:
        npc_icy_tomb() : CreatureScript("npc_icy_tomb") { }

        struct npc_icy_tombAI : public ScriptedAI
        {
            npc_icy_tombAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;
            uint32 timer;
            Unit* Thrall;

            void IsSummonedBy(Unit* summoner) override
            {
                if (summoner)
                {
                    summoner->GetAI()->DoAction(ACTION_ICY_TOMB);
                    Thrall = summoner;
                }
            }

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void InitializeAI() override
            {
                me->SetReactState(REACT_PASSIVE);
            }

            void JustDied(Unit* /*killer*/) override
            {
                if (Thrall)
                    Thrall->GetAI()->DoAction(ACTION_ICY_TOMB_REMOVE);
            }

            void DoAction(int32 /*action*/) override { }

            void DamageTaken(Unit* /*killer*/, uint32& /*damage*/) override { }

            void UpdateAI(uint32 /*diff*/) override { }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return new npc_icy_tombAI(creature);
        }
};

class spell_arcurion_icy_boulder : public SpellScriptLoader
{
    public:
        spell_arcurion_icy_boulder() : SpellScriptLoader("spell_arcurion_icy_boulder") { }

        class spell_arcurion_icy_boulder_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_arcurion_icy_boulder_SpellScript);

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (!GetCaster() || !GetHitUnit())
                    return;

                GetCaster()->CastSpell(GetHitUnit(), SPELL_ICY_BOULDER, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_arcurion_icy_boulder_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_arcurion_icy_boulder_SpellScript();
        }
};


void AddSC_boss_arcurion()
{
    new boss_arcurion();
    new npc_arcurion_frozen_servitor();
    new npc_icy_tomb();
    new spell_arcurion_icy_boulder();
}
