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

/*
Name: Boss_Anzu
%Complete: 80%
Comment:
Category: Auchindoun, Sethekk Halls
*/

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "sethekk_halls.h"
#include "SpellMgr.h"
#include <set>

enum Says
{
    SAY_SUMMON_BROOD            = 0,
    SAY_SPELL_BOMB              = 1
};

enum Spells
{
    SPELL_PARALYZING_SCREECH    = 40184,
    SPELL_SPELL_BOMB            = 40303,
    SPELL_CYCLONE_OF_FEATHERS   = 40321,
    SPELL_BANISH_SELF           = 42354,
    SPELL_FLESH_RIP             = 40199,
    SPELL_DIVE                  = 40279,
    SPELL_BROOD_SCREECH          = 31273
};

enum Spirits
{
    // #635: AC boss_anzu.cpp:46-62 (spirit ids and spells), 184-236 (npc_anzu_spirit)
    NPC_HAWK_SPIRIT             = 23134,
    NPC_FALCON_SPIRIT           = 23135,
    NPC_EAGLE_SPIRIT            = 23136,

    SPELL_HAWK                  = 40237,
    SPELL_FALCON                = 40241,
    SPELL_EAGLE                 = 40240,

    SPELL_FREEZE_ANIM           = 16245,
    SPELL_STONEFORM             = 40308,

    SAY_STONED                  = 0
};

// AC boss_anzu.cpp:71-76
Position const AnzuSpiritPos[3] =
{
    { -96.4816f, 304.236f, 26.5135f, 5.23599f },    // Hawk Spirit
    { -72.3434f, 290.861f, 26.4851f, 3.29867f },    // Falcon Spirit
    { -99.5906f, 276.661f, 26.8467f, 0.750492f },   // Eagle Spirit
};

enum Events
{
    EVENT_PARALYZING_SCREECH    = 1,
    EVENT_SPELL_BOMB            = 2,
    EVENT_CYCLONE_OF_FEATHERS   = 3,
    EVENT_FLESH_RIP             = 4,
    EVENT_DIVE                 = 5
};

Position const PosSummonBrood[7] =
{
    { -118.1717f, 284.5299f, 121.2287f, 2.775074f },
    { -98.15528f, 293.4469f, 109.2385f, 0.174533f },
    { -99.70160f, 270.1699f, 98.27389f, 6.178465f },
    { -69.25543f, 303.0768f, 97.84479f, 5.532694f },
    { -87.59662f, 263.5181f, 92.70478f, 1.658063f },
    { -73.54323f, 276.6267f, 94.25807f, 2.802979f },
    { -81.70527f, 280.8776f, 44.58830f, 0.526849f }
};

// SF main607dba2179 / preceding Anzu encounter fixes; retain LoA spirits.
class boss_anzu : public CreatureScript
{
public:
    boss_anzu() : CreatureScript("boss_anzu") { }

    struct boss_anzuAI : public BossAI
    {
        boss_anzuAI(Creature* creature) : BossAI(creature, DATA_ANZU),
            _thresholds(0), _pendingWaves(0), _waveActive(false) { }

        void Reset() override
        {
            _thresholds = 0;
            _pendingWaves = 0;
            _waveActive = false;
            _brood.clear();
            me->RemoveAurasDueToSpell(SPELL_BANISH_SELF);
            _Reset();
        }

        void JustEngagedWith(Unit* /*who*/) override
        {
            _JustEngagedWith();
            SummonSpirits();
            events.ScheduleEvent(EVENT_PARALYZING_SCREECH, 14000);
            events.ScheduleEvent(EVENT_CYCLONE_OF_FEATHERS, 5000);
            // Provisional cadence for the missing abilities; the 18414 journal
            // describes their effects but does not specify encounter cooldowns.
            events.ScheduleEvent(EVENT_FLESH_RIP, 10000);
            events.ScheduleEvent(EVENT_DIVE, 20000);
        }

            void SummonSpirits()
            {
                summons.DespawnEntry(NPC_HAWK_SPIRIT);
                summons.DespawnEntry(NPC_FALCON_SPIRIT);
                summons.DespawnEntry(NPC_EAGLE_SPIRIT);
                me->SummonCreature(NPC_HAWK_SPIRIT, AnzuSpiritPos[0], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_FALCON_SPIRIT, AnzuSpiritPos[1], TEMPSUMMON_MANUAL_DESPAWN);
                me->SummonCreature(NPC_EAGLE_SPIRIT, AnzuSpiritPos[2], TEMPSUMMON_MANUAL_DESPAWN);
            }


        void JustDied(Unit* /*killer*/) override
        {
            _waveActive = false;
            _pendingWaves = 0;
            _brood.clear();
            _JustDied();
        }

        void DamageTaken(Unit* /*killer*/, uint32& damage) override
        {
            // Do not clamp lethal damage or force unverified phase immunities.
            // Queue both crossed thresholds, but never overlap their waves.
            if (damage >= me->GetHealth())
                return;

            if (!(_thresholds & 1) && me->HealthBelowPctDamaged(75, damage))
            {
                _thresholds |= 1;
                ++_pendingWaves;
            }
            if (!(_thresholds & 2) && me->HealthBelowPctDamaged(35, damage))
            {
                _thresholds |= 2;
                ++_pendingWaves;
            }
        }

        void JustSummoned(Creature* summon) override
        {
            if (summon->GetEntry() == NPC_BROOD_OF_ANZU)
                _brood.insert(summon->GetGUID());
            BossAI::JustSummoned(summon);
        }

        void SummonedCreatureDies(Creature* summon, Unit* /*killer*/) override
        {
            // GUID membership excludes surviving adds from a previous wave.
            // Despawning a live add is not equivalent to killing it.
            if (_brood.erase(summon->GetGUID()) && _brood.empty() && _waveActive)
                me->RemoveAurasDueToSpell(SPELL_BANISH_SELF);
        }

        void StartBroodWave()
        {
            me->InterruptNonMeleeSpells(false);
            DoCast(me, SPELL_BANISH_SELF, true);
            if (!me->HasAura(SPELL_BANISH_SELF))
                return; // Retry without consuming a threshold or creating adds.

            --_pendingWaves;
            _waveActive = true;
            _brood.clear();
            Talk(SAY_SUMMON_BROOD);
            // TODO: Verify the legacy aerial spawn positions and brood pathing in-game.
            for (uint8 i = 0; i < 7; ++i)
                me->SummonCreature(NPC_BROOD_OF_ANZU, PosSummonBrood[i], TempSummonType::TEMPSUMMON_TIMED_DESPAWN_OUT_OF_COMBAT, 46000ms);

            if (_brood.empty())
                me->RemoveAurasDueToSpell(SPELL_BANISH_SELF);
            events.RescheduleEvent(EVENT_SPELL_BOMB, 12000);
        }

        void UpdateAI(uint32 diff) override
        {
            // Keep victim/evade handling active, but do not pause spell timers
            // when banish prevents Anzu from maintaining a melee victim.
            bool const hasVictim = UpdateVictim();
            if (!me->IsInCombat() || (!hasVictim && !me->HasAura(SPELL_BANISH_SELF)))
                return;

            events.Update(diff);
            if (_waveActive && !me->HasAura(SPELL_BANISH_SELF))
            {
                _waveActive = false;
                _brood.clear();
            }
            if (!_waveActive && _pendingWaves)
                StartBroodWave();

            // Banish pacifies and roots Anzu; it must not stop his spell events.
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;

            while (uint32 eventId = events.ExecuteEvent())
            {
                switch (eventId)
                {
                    case EVENT_PARALYZING_SCREECH:
                        DoCastAOE(SPELL_PARALYZING_SCREECH);
                        events.ScheduleEvent(EVENT_PARALYZING_SCREECH, 26000);
                        break;
                    case EVENT_CYCLONE_OF_FEATHERS:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            DoCast(target, SPELL_CYCLONE_OF_FEATHERS);
                        events.ScheduleEvent(EVENT_CYCLONE_OF_FEATHERS, 21000);
                        break;
                    case EVENT_SPELL_BOMB:
                        if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, [](Unit* unit)
                            { return unit && unit->IsAlive() && unit->GetTypeId() == TypeID::TYPEID_PLAYER && unit->GetPowerType() == POWER_MANA; }))
                        {
                            DoCast(target, SPELL_SPELL_BOMB);
                            Talk(SAY_SPELL_BOMB, target);
                        }
                        break;
                    case EVENT_FLESH_RIP:
                    {
                        auto const isMeleePlayer = [this](Unit* unit)
                        {
                            return unit && unit->IsAlive() && unit->GetTypeId() == TypeID::TYPEID_PLAYER &&
                                me->IsWithinMeleeRange(unit);
                        };
                        Unit* target = me->GetVictim();
                        if (!isMeleePlayer(target))
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0, isMeleePlayer);

                        if (!me->HasAura(SPELL_BANISH_SELF) && target)
                        {
                            DoCast(target, SPELL_FLESH_RIP);
                            events.ScheduleEvent(EVENT_FLESH_RIP, 15000);
                        }
                        else
                            events.ScheduleEvent(EVENT_FLESH_RIP, 1000);
                        break;
                    }
                    case EVENT_DIVE:
                    {
                        SpellInfo const* spell = sSpellMgr->GetSpellInfo(SPELL_DIVE);
                        Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, [this, spell](Unit* unit)
                        {
                            return spell && unit && unit->IsAlive() && unit->GetTypeId() == TypeID::TYPEID_PLAYER &&
                                !me->IsWithinMeleeRange(unit) && me->IsWithinDistInMap(unit, spell->GetMaxRange(false, me)) &&
                                me->IsWithinLOSInMap(unit);
                        });
                        if (!me->HasAura(SPELL_BANISH_SELF) && target)
                        {
                            DoCast(target, SPELL_DIVE);
                            events.ScheduleEvent(EVENT_DIVE, 30000);
                        }
                        else
                            events.ScheduleEvent(EVENT_DIVE, 1000);
                        break;
                    }
                    default:
                        break;
                }
                // Keep other due casts queued until the current cast completes.
                if (me->HasUnitState(UNIT_STATE_CASTING))
                    break;
            }
            if (!me->HasAura(SPELL_BANISH_SELF))
                DoMeleeAttackIfReady();
        }

    private:
        uint8 _thresholds;
        uint8 _pendingWaves;
        bool _waveActive;
        std::set<ObjectGuid> _brood;
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetSethekkHallsAI<boss_anzuAI>(creature);
    }
};

class npc_brood_of_anzu : public CreatureScript
{
public:
    npc_brood_of_anzu() : CreatureScript("npc_brood_of_anzu") { }

    struct npc_brood_of_anzuAI : public ScriptedAI
    {
        npc_brood_of_anzuAI(Creature* creature) : ScriptedAI(creature) { }
        EventMap events;

        void Reset() override { events.Reset(); }
        void JustEngagedWith(Unit* /*who*/) override
        {
            // Provisional cadence; Screech's client duration is eight seconds.
            events.ScheduleEvent(1, 5000);
        }
        void UpdateAI(uint32 diff) override
        {
            if (!UpdateVictim())
                return;
            events.Update(diff);
            if (me->HasUnitState(UNIT_STATE_CASTING))
                return;
            if (events.ExecuteEvent())
            {
                DoCastAOE(SPELL_BROOD_SCREECH);
                events.ScheduleEvent(1, 10000);
            }
            DoMeleeAttackIfReady();
        }
    };

    CreatureAI* GetAI(Creature* creature) const override
    {
        return GetSethekkHallsAI<npc_brood_of_anzuAI>(creature);
    }
};


// AC boss_anzu.cpp:184-236. Every 2.4 s: a Druid HoT on the spirit frees it and it casts its Spite on itself,
// otherwise it stays stoned (Stoneform + Freeze Anim, Say 0).
class npc_anzu_spirit : public CreatureScript
{
    public:
        npc_anzu_spirit() : CreatureScript("npc_anzu_spirit") { }

        struct npc_anzu_spiritAI : public ScriptedAI
        {
            npc_anzu_spiritAI(Creature* creature) : ScriptedAI(creature) { }

            void IsSummonedBy(Unit* /*summoner*/) override
            {
                _scheduler.Schedule(1ms, [this](TaskContext task)
                {
                    // Check for Druid HoTs every 2400ms
                    if (HasDruidHoT())
                    {
                        me->RemoveAurasDueToSpell(SPELL_FREEZE_ANIM);
                        me->RemoveAurasDueToSpell(SPELL_STONEFORM);

                        switch (me->GetEntry())
                        {
                            case NPC_HAWK_SPIRIT:
                                DoCast(me, SPELL_HAWK);
                                break;
                            case NPC_FALCON_SPIRIT:
                                DoCast(me, SPELL_FALCON);
                                break;
                            case NPC_EAGLE_SPIRIT:
                                DoCast(me, SPELL_EAGLE);
                                break;
                            default:
                                break;
                        }
                    }
                    else if (!me->HasAura(SPELL_STONEFORM))
                    {
                        Talk(SAY_STONED);
                        DoCast(me, SPELL_FREEZE_ANIM, true);
                        DoCast(me, SPELL_STONEFORM, true);
                    }

                    task.Repeat(2400ms);
                });
            }

            // #635: AC's GetAuraEffect(type, family, iconId, 0) matches by SpellIconID only; the fork's iconId overload
            // (Unit.cpp:5097) additionally requires !SpellFamilyFlags, which Rejuvenation (icon 64) has in 5.4.8, so it never
            // matched and the spirits never woke. Match family + icon directly (same semantics as AC).
            bool HasDruidHoT() const
            {
                Unit::AuraEffectList const& hots = me->GetAuraEffectsByType(SPELL_AURA_PERIODIC_HEAL);
                for (AuraEffect const* eff : hots)
                {
                    SpellInfo const* info = eff->GetSpellInfo();
                    if (info->SpellFamilyName == SPELLFAMILY_DRUID && info->SpellIconID == 64)
                        return true;
                }
                return false;
            }

            void Reset() override
            {
                _scheduler.CancelAll();
            }

            void UpdateAI(uint32 diff) override
            {
                _scheduler.Update(diff);
            }

        private:
            TaskScheduler _scheduler;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetSethekkHallsAI<npc_anzu_spiritAI>(creature);
        }
};

void AddSC_boss_anzu()
{
    new boss_anzu();
    new npc_brood_of_anzu();
    new npc_anzu_spirit();
}
