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
#include "blackrock_caverns.h"

// #699: Lava Spout + Bound Flames (heroic) when Superheated Quicksilver Armor expires, Heat Wave scaled by the armor's
// stacks. Sources (read 2026-10-13): Wowpedia MediaWiki API "Karsh Steelbender" - "Lava Spout: When the duration of
// Superheated Quicksilver Armor expires, Karsh Steelbender's armor cools off and the excess heat causes lava spouts to
// erupt from the circular grate inside the room. Players standing on the grate when the lava spouts erupt take 23125 to
// 26875 Fire damage. Bound Flames (heroic): When the lava spouts erupt from the circular grate, three Bound Flames form
// and begin to attack nearby enemy players." and "Heat Wave ... 1500 Fire damage to all nearby enemies, but the damage
// is further multiplied by the number of stacks of Superheated Quicksilver Armor on Karsh"; Wowhead Cata tooltip API
// (nether.wowhead.com/cata/tooltip/spell/76007) "Lava Spout: Inflicts 57000 to 63000 Fire damage to targets in a small
// area" (heroic row). DBC SpellEffect 76007: SCHOOL_DAMAGE 23125-26875 (heroic 57000-63000), target A DEST_CASTER,
// B DEST_AREA_ENEMY, radius 10 yd; 93539 "Bound Flames" = SUMMON 50417 (3 casts, heroic).

enum ScriptTexts
{
    SAY_AGGRO     = 0,
    SAY_KILL      = 1,
    SAY_BURN      = 2,
    SAY_DEATH     = 3,
};

enum Spells
{
    SPELL_CLEAVE                                = 15284,
    SPELL_QUICKSILVER_ARMOR                     = 75842,
    SPELL_HEAT_WAVE                             = 75851,
    SPELL_SUPERHEATED_QUICKSILVER_ARMOR         = 75846,
    SPELL_SUPERHEATED_QUICKSILVER_ARMOR_0       = 76015,

    SPELL_BURNING                               = 77490,

    SPELL_LAVA_SPOUT                            = 76007,
};

enum Events
{
     EVENT_CLEAVE          = 1,
     EVENT_HEAT_ARMOR      = 2,
};

enum Actions
{
    ACTION_LAVA_SPOUT     = 1,    // from the Superheated Quicksilver Armor aura script when it expires
};

enum Adds
{
    NPC_BOUND_FLAMES    = 50417,
    NPC_WORLD_TRIGGER   = 21252,   // "World Trigger (Not Immune PC)": the invisible caster of the Lava Spout at the grate
};

const Position centerkarshPos = { 237.84f, 784.76f, 95.67f, 1.95f };

class boss_karsh_steelbender : public CreatureScript
{
    public:
        boss_karsh_steelbender() : CreatureScript("boss_karsh_steelbender") { }

        struct boss_karsh_steelbenderAI : public BossAI
        {
            boss_karsh_steelbenderAI(Creature* creature) : BossAI(creature, DATA_KARSH)
            {
               me->ApplySpellImmune(0, IMMUNITY_EFFECT, SPELL_EFFECT_KNOCK_BACK, true);
               me->ApplySpellImmune(0, IMMUNITY_MECHANIC, MECHANIC_GRIP, true);
               me->setActive(true);
            }

            void JustSummoned(Creature* summon) override
            {
                summons.Summon(summon);

                if (summon->GetEntry() == NPC_BOUND_FLAMES)
                    summon->SetInCombatWithZone();
            }

            void SummonedCreatureDespawn(Creature* summon) override
            {
                summons.Despawn(summon);
            }

            void DoAction(int32 action) override
            {
                if (action != ACTION_LAVA_SPOUT || !me->IsAlive() || !me->IsInCombat())
                    return;

                // the eruption: 10 yd around the grate (SPELL_LAVA_SPOUT is cast at the caster's position)
                if (TempSummon* spout = me->SummonCreature(NPC_WORLD_TRIGGER, centerkarshPos, TEMPSUMMON_TIMED_DESPAWN, 3000ms))
                {
                    spout->SetFaction(me->GetFaction());
                    spout->CastSpell(spout, SPELL_LAVA_SPOUT, true);
                }

                // heroic: three Bound Flames form at the grate (spell 93539 summons one each)
                if (IsHeroic())
                    for (uint8 i = 0; i < 3; ++i)
                    {
                        float const angle = centerkarshPos.GetOrientation() + i * (2.0f * float(M_PI) / 3.0f);
                        me->SummonCreature(NPC_BOUND_FLAMES, centerkarshPos.GetPositionX() + 3.0f * std::cos(angle),
                            centerkarshPos.GetPositionY() + 3.0f * std::sin(angle), centerkarshPos.GetPositionZ(), angle,
                            TEMPSUMMON_CORPSE_TIMED_DESPAWN, 15000ms);
                    }
            }

            void Reset() override
            {
                _Reset();
                summons.DespawnAll();
                DoCast(me, SPELL_QUICKSILVER_ARMOR);
                bHeat = false;

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                me->GetMap()->SetWorldState(WORLDSTATE_TOO_HOT_TO_HANDLE, 0);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                events.ScheduleEvent(EVENT_CLEAVE, urand(5000, 7000));
                events.ScheduleEvent(EVENT_HEAT_ARMOR, 1000);
                Talk(SAY_AGGRO);
                DoZoneInCombat();
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);
            }
     
            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
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
     
                bHeat = (me->GetDistance(centerkarshPos) <= 6.0f) ? true : false;

                if (!me->HasAura(SPELL_QUICKSILVER_ARMOR) && !me->HasAura(SPELL_SUPERHEATED_QUICKSILVER_ARMOR))
                    DoCast(me, SPELL_QUICKSILVER_ARMOR);
                if (me->HasAura(SPELL_SUPERHEATED_QUICKSILVER_ARMOR))
                    me->RemoveAurasDueToSpell(SPELL_QUICKSILVER_ARMOR);

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_CLEAVE:
                            DoCast(me->GetVictim(), SPELL_CLEAVE);
                            events.ScheduleEvent(EVENT_CLEAVE, urand(5000, 7000));
                            break;
                        case EVENT_HEAT_ARMOR:
                            if (bHeat)
                            {
                                DoCast(me, SPELL_SUPERHEATED_QUICKSILVER_ARMOR);
                                DoCast(me, SPELL_HEAT_WAVE);
                            }
                            events.ScheduleEvent(EVENT_HEAT_ARMOR, 1000);
                            break;
                    }
                    DoMeleeAttackIfReady();
                }
            }

            private:
                bool bHeat;        
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_karsh_steelbenderAI>(creature);
        }
};

// Superheated Quicksilver Armor 75846
class spell_superheated_quicksilver_armor : public AuraScript
{
    PrepareAuraScript(spell_superheated_quicksilver_armor);

    void HandleAuraEffectApply(AuraEffect const* aurEff, AuraEffectHandleModes mode)
    {
        if (GetStackAmount() > 14 && GetOwner() && GetOwner()->ToUnit())
            GetOwner()->ToUnit()->GetMap()->SetWorldState(WORLDSTATE_TOO_HOT_TO_HANDLE, 1);
    }

    // #699: the armor cooling off (the 17 s expire, not a dispel or the boss leaving combat) makes the grate erupt
    void HandleAuraEffectRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        if (GetTargetApplication()->GetRemoveMode() != AURA_REMOVE_BY_EXPIRE)
            return;

        if (Creature* karsh = GetOwner()->ToCreature())
            if (karsh->AI() && karsh->GetEntry() == NPC_KARSH)
                karsh->AI()->DoAction(ACTION_LAVA_SPOUT);
    }

    void Register() override
    {
        OnEffectApply += AuraEffectApplyFn(spell_superheated_quicksilver_armor::HandleAuraEffectApply, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAPPLY);
        AfterEffectRemove += AuraEffectRemoveFn(spell_superheated_quicksilver_armor::HandleAuraEffectRemove, EFFECT_1, SPELL_AURA_MOD_DAMAGE_PERCENT_TAKEN, AURA_EFFECT_HANDLE_REAL);
    }
};

// Heat Wave 75851 - "1500 Fire damage to all nearby enemies, but the damage is further multiplied by the number of stacks
// of Superheated Quicksilver Armor on Karsh" (Wowpedia "Karsh Steelbender"). The spell has no stack term of its own.
class spell_karsh_heat_wave : public SpellScript
{
    PrepareSpellScript(spell_karsh_heat_wave);

    void ScaleByStacks(SpellEffIndex /*effIndex*/)
    {
        if (Unit* caster = GetCaster())
            if (Aura const* armor = caster->GetAura(SPELL_SUPERHEATED_QUICKSILVER_ARMOR))
                SetHitDamage(GetHitDamage() * std::max<int32>(1, armor->GetStackAmount()));
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_karsh_heat_wave::ScaleByStacks, EFFECT_0, SPELL_EFFECT_SCHOOL_DAMAGE);
    }
};
 
void AddSC_boss_karsh_steelbender()
{
    new boss_karsh_steelbender();
    new aura_script<spell_superheated_quicksilver_armor>("spell_superheated_quicksilver_armor");
    new spell_script<spell_karsh_heat_wave>("spell_karsh_heat_wave");
}
