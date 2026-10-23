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
#include "ScriptPCH.h"
#include "Containers.h"

// #688: the weapon phases follow the real fight (Wowpedia Forgemaster_Throngus, fetched 2026-10-23) and the 4.3.4 script of the
// Cataclysm Preservation Project (TrinityCore fork, GrimBatol/boss_forgemaster_throngus.cpp @0927c5af): Mighty Stomp -> Cave In
// on a random player outside the shield phase; Pick Weapon never repeats the previous weapon; the shield phase pacifies him,
// points the shield at a random player every 8.5 s and the Twilight Archers in the windows fire Flaming Arrows at random
// players; Disorienting Roar / Impaling Slam only while their weapon is held. Spell data: MoP 5.4.8 Spell/SpellEffect.dbc.

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_DEATH   = 2,
    SAY_SHIELD  = 3,
    SAY_DUAL    = 4,
    SAY_MACE    = 5,
};

enum Equipment
{
    EQUIPMENT_ID_SHIELD    = 40400,
    EQUIPMENT_ID_SWORD     = 65094,
    EQUIPMENT_ID_MACE      = 65090,
};

enum Spells
{
    SPELL_MIGHTY_STOMP            = 74984,  // 0.85 s cast; force-casts Cave In 74985 on the hit player (spell_throngus_mighty_stomp)
    SPELL_PICK_WEAPON             = 75000,
    SPELL_DUAL_BLADES             = 74981,
    SPELL_ENCUMBERED              = 75007,  // heroic effect 2: periodic 90743 every 2 s -> 90751 summons Fire Patch 48711 (DBC chain)
    SPELL_IMPALING_SLAM_0         = 75057,
    SPELL_IMPALING_SLAM           = 75056,
    SPELL_DISORIENTING_ROAR       = 74976,
    SPELL_BURNING_SHIELD          = 90819,  // heroic Flaming Shield: periodic 90830, 60 yd front cone
    SPELL_BURNING_FLAMES          = 90759,  // heroic, swords: proc 90764 (fire DoT) on the melee target
    SPELL_PERSONAL_PHALANX        = 74908,  // 30 s: pacify + root + periodic 74909 Glancing Blows (front cone, -99% damage done)
    SPELL_FLAMING_ARROW           = 45101,
    SPELL_FLAMING_ARROW_DMG       = 74944,  // Twilight Archer: 4.9-5.1k fire to one random enemy (max targets 1, as 4.3.4)
    SPELL_CAVE_IN_DUMMY           = 74987,  // Cave In Stalker visual (persistent area aura, 6 yd, 240 s)
    SPELL_CAVE_IN                 = 74990,  // Cave In Stalker: periodic 74986 every 1 s for 240 s
    SPELL_CAVE_IN_DMG             = 74986,  // 3.8-4.2k physical within 5 yd
    SPELL_FIRE_PATCH_AURA         = 90752,
    SPELL_LAVA_PATCH_DMG          = 90754,
};

enum Adds
{
    NPC_CAVE_IN_STALKER        = 40228,
    NPC_TWILIGHT_ARCHER        = 40197,
    NPC_FIXATE_STALKER         = 40255,
    NPC_FIRE_PATCH             = 48711,
};

enum Events
{
    EVENT_PICK_WEAPON          = 1,
    EVENT_DISORIENTING_ROAR    = 2,
    EVENT_IMPALING_SLAM        = 3,
    EVENT_FLAMING_ARROW        = 4,
    EVENT_MIGHTY_STOMP         = 5,
    EVENT_FIXATE_PLAYER        = 6,
    EVENT_CAVE_IN_DUMMY        = 8,
    EVENT_CAVE_IN_DMG          = 9,
};

enum Others
{
    ACTION_MACE                = 1,
    ACTION_DUAL_BLADES         = 2,
    ACTION_SHIELD              = 3,
    ACTION_PICK_WEAPON         = 4,
    ACTION_CAVE_IN             = 5,   // SetGUID id: summon a Cave In Stalker on that player
};


const Position  twilightarcherPos[13] =
{
    { -542.994f, -605.236f, 300.201f, 1.68049f },
    { -543.59f,  -605.413f, 283.784f, 1.50377f },
    { -521.237f, -605.435f, 300.76f,  1.63886f },
    { -483.862f, -588.658f, 297.574f, 2.38106f },
    { -482.655f, -588.461f, 280.966f, 2.34571f },
    { -471.266f, -575.324f, 295.906f, 2.30254f },
    { -525.377f, -455.312f, 285.288f, 4.66187f },
    { -544.49f,  -454.961f, 295.831f, 4.79539f },
    { -522.164f, -455.31f,  299.791f, 4.77575f },
    { -468.703f, -489.004f, 300.462f, 3.78616f },
    { -470.907f, -484.791f, 282.203f, 3.87255f },
    { -485.052f, -474.621f, 296.525f, 3.92361f },
    { -481.352f, -477.21f,  280.714f, 3.72334f }
};

class boss_forgemaster_throngus : public CreatureScript
{
    public:
        boss_forgemaster_throngus() : CreatureScript("boss_forgemaster_throngus") { }

        struct boss_forgemaster_throngusAI : public BossAI
        {
            boss_forgemaster_throngusAI(Creature* creature) : BossAI(creature, DATA_FORGEMASTER_THRONGUS)
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

            int32 lastWeapon = 0;
            int32 weapon = 0;
            bool phalanx = false;
            ObjectGuid fixateStalkerGUID;

            void Reset() override
            {
                _Reset();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                SetEquipmentSlots(false, 0, 0, 0);
                me->SetReactState(REACT_AGGRESSIVE);
                lastWeapon = 0;
                weapon = 0;
                phalanx = false;
                fixateStalkerGUID = ObjectGuid::Empty;
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                _JustEngagedWith();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_ENGAGE, me);

                Talk(SAY_AGGRO);
                events.ScheduleEvent(EVENT_MIGHTY_STOMP, 7000);
                events.ScheduleEvent(EVENT_PICK_WEAPON, 10000);

                // The archers stand in the windows above the forge for the whole fight and only shoot in the shield phase.
                for (Position const& pos : twilightarcherPos)
                    me->SummonCreature(NPC_TWILIGHT_ARCHER, pos, TEMPSUMMON_MANUAL_DESPAWN);
            }

            void JustSummoned(Creature* summon) override
            {
                // No DoZoneInCombat (the BossAI default): archers and stalkers are unselectable scenery for the bots.
                summons.Summon(summon);

                switch (summon->GetEntry())
                {
                    case NPC_TWILIGHT_ARCHER:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->SetFaction(me->GetFaction());
                        break;
                    case NPC_CAVE_IN_STALKER:
                        summon->SetReactState(REACT_PASSIVE);
                        summon->CastSpell(summon, SPELL_CAVE_IN_DUMMY, true);
                        summon->CastSpell(summon, SPELL_CAVE_IN, true);
                        break;
                    case NPC_FIXATE_STALKER:
                        summon->SetReactState(REACT_PASSIVE);
                        break;
                    default:
                        break;
                }
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();

                if (instance)
                    instance->SendEncounterUnit(ENCOUNTER_FRAME_DISENGAGE, me);

                Talk(SAY_DEATH);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void SetGUID(ObjectGuid guid, int32 id) override
            {
                if (id != ACTION_CAVE_IN)
                    return;

                if (Unit* target = ObjectAccessor::GetUnit(*me, guid))
                    me->SummonCreature(NPC_CAVE_IN_STALKER, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), 0.0f,
                                       TEMPSUMMON_TIMED_DESPAWN, 240000ms);
            }

            void EndPhalanx()
            {
                phalanx = false;
                events.CancelEvent(EVENT_FIXATE_PLAYER);
                events.CancelEvent(EVENT_FLAMING_ARROW);
                me->RemoveAurasDueToSpell(SPELL_BURNING_SHIELD);
                if (Creature* stalker = ObjectAccessor::GetCreature(*me, fixateStalkerGUID))
                    stalker->DespawnOrUnsummon();
                fixateStalkerGUID = ObjectGuid::Empty;
                me->SetReactState(REACT_AGGRESSIVE);
            }

            // Point the shield at a random player: a Fixate Stalker on its spot and Throngus turned to it (74914 -> 74926 summons
            // 40255, 75071 Fixate Effect in 4.3.4; the DBC chain needs target conditions this DB does not have).
            void FixateRandomPlayer()
            {
                if (Creature* old = ObjectAccessor::GetCreature(*me, fixateStalkerGUID))
                    old->DespawnOrUnsummon();
                fixateStalkerGUID = ObjectGuid::Empty;

                Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                if (!target)
                    return;

                if (Creature* stalker = me->SummonCreature(NPC_FIXATE_STALKER, target->GetPositionX(), target->GetPositionY(),
                                                           target->GetPositionZ(), 0.0f, TEMPSUMMON_TIMED_DESPAWN, 15000ms))
                {
                    fixateStalkerGUID = stalker->GetGUID();
                    me->SetFacingToObject(stalker);
                }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                events.Update(diff);

                if (phalanx && !me->HasAura(SPELL_PERSONAL_PHALANX))
                    EndPhalanx();

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                while (uint32 eventId = events.ExecuteEvent())
                {
                    switch (eventId)
                    {
                        case EVENT_PICK_WEAPON:
                            events.DelayEvents(3000);
                            DoCast(me, SPELL_PICK_WEAPON);
                            events.ScheduleEvent(EVENT_PICK_WEAPON, 35500);
                            break;
                        case EVENT_MIGHTY_STOMP:
                            // "Throngus will not Mighty Stomp if he has his shield equipped" (Wowpedia). One random player.
                            if (!me->HasAura(SPELL_PERSONAL_PHALANX))
                                me->CastCustomSpell(SPELL_MIGHTY_STOMP, SPELLVALUE_MAX_TARGETS, 1, nullptr, false);
                            events.ScheduleEvent(EVENT_MIGHTY_STOMP, 49000);
                            break;
                        case EVENT_IMPALING_SLAM:
                            if (weapon != ACTION_MACE)
                                break;
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, NonTankTargetSelector(me, true)))
                            {
                                DoCast(target, SPELL_IMPALING_SLAM);
                                DoCast(target, SPELL_IMPALING_SLAM_0);
                            }
                            events.ScheduleEvent(EVENT_IMPALING_SLAM, 15500);
                            break;
                        case EVENT_DISORIENTING_ROAR:
                            if (weapon != ACTION_DUAL_BLADES)
                                break;
                            DoCast(me, SPELL_DISORIENTING_ROAR);
                            events.ScheduleEvent(EVENT_DISORIENTING_ROAR, 22000);
                            break;
                        case EVENT_FIXATE_PLAYER:
                            if (!phalanx)
                                break;
                            FixateRandomPlayer();
                            events.ScheduleEvent(EVENT_FIXATE_PLAYER, 8500);
                            break;
                        case EVENT_FLAMING_ARROW:
                        {
                            if (!phalanx)
                                break;
                            // Flame Arrow Barrage (74946 -> 74945 every 5 s in 4.3.4): up to five archers each shoot one random player.
                            std::list<Creature*> archers;
                            for (ObjectGuid guid : summons)
                                if (Creature* archer = ObjectAccessor::GetCreature(*me, guid))
                                    if (archer->GetEntry() == NPC_TWILIGHT_ARCHER && archer->IsAlive())
                                        archers.push_back(archer);
                            Trinity::Containers::RandomResizeList(archers, 5);
                            for (Creature* archer : archers)
                                archer->CastCustomSpell(SPELL_FLAMING_ARROW_DMG, SPELLVALUE_MAX_TARGETS, 1, nullptr, true);
                            events.ScheduleEvent(EVENT_FLAMING_ARROW, 5000);
                            break;
                        }
                    }
                }

                if (!phalanx)
                    DoMeleeAttackIfReady();
            }

            void DoAction(int32 action) override
            {
                switch (action)
                {
                    case ACTION_PICK_WEAPON:
                    {
                        // 4.3.4: never the same weapon twice in a row
                        std::vector<int32> choices;
                        for (int32 w : { int32(ACTION_SHIELD), int32(ACTION_MACE), int32(ACTION_DUAL_BLADES) })
                            if (w != lastWeapon)
                                choices.push_back(w);
                        lastWeapon = Trinity::Containers::SelectRandomContainerElement(choices);
                        DoAction(lastWeapon);
                        break;
                    }
                    case ACTION_MACE:
                        weapon = ACTION_MACE;
                        DoCast(me, SPELL_ENCUMBERED, true);
                        Talk(SAY_MACE);
                        SetEquipmentSlots(false, EQUIPMENT_ID_MACE, 0, 0);
                        events.ScheduleEvent(EVENT_IMPALING_SLAM, 6000);
                        break;
                    case ACTION_DUAL_BLADES:
                        weapon = ACTION_DUAL_BLADES;
                        DoCast(me, SPELL_DUAL_BLADES, true);
                        if (IsHeroic())
                            DoCast(me, SPELL_BURNING_FLAMES, true);
                        Talk(SAY_DUAL);
                        SetEquipmentSlots(false, EQUIPMENT_ID_SWORD, EQUIPMENT_ID_SWORD, 0);
                        events.ScheduleEvent(EVENT_DISORIENTING_ROAR, 5000);
                        break;
                    case ACTION_SHIELD:
                        weapon = ACTION_SHIELD;
                        DoCast(me, SPELL_PERSONAL_PHALANX, true);
                        if (IsHeroic())
                            DoCast(me, SPELL_BURNING_SHIELD, true);
                        Talk(SAY_SHIELD);
                        SetEquipmentSlots(false, 0, EQUIPMENT_ID_SHIELD, 0);
                        // Pacified and rooted by the phalanx: he stops attacking and only turns the shield (4.3.4 ACTION_START_PHALLANX).
                        phalanx = true;
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        me->GetMotionMaster()->Clear();
                        me->GetMotionMaster()->MoveIdle();
                        events.ScheduleEvent(EVENT_FIXATE_PLAYER, 1000);
                        events.ScheduleEvent(EVENT_FLAMING_ARROW, 5000);
                        break;
                }
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_forgemaster_throngusAI>(creature);
        }
};



class npc_throngus_fire_patch : public CreatureScript
{
    public:
        npc_throngus_fire_patch() : CreatureScript("npc_throngus_fire_patch"){}

        struct npc_throngus_fire_patchAI : public ScriptedAI
        {
            npc_throngus_fire_patchAI(Creature* creature) : ScriptedAI(creature)
            {
                SetCombatMovement(false);
            }

            void Reset() override
            {
                DoCast(me, SPELL_FIRE_PATCH_AURA);
            }

            void UpdateAI(uint32 /*diff*/) override { }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_throngus_fire_patchAI>(creature);
        }
};

class spell_throngus_pick_weapon : public SpellScriptLoader
{
    public:
        spell_throngus_pick_weapon() : SpellScriptLoader("spell_throngus_pick_weapon") { }

        class spell_throngus_pick_weapon_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_throngus_pick_weapon_SpellScript);

            void HandleScriptEffect(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);

                if (!GetCaster())
                    return;

                if (UnitAI* ai = GetCaster()->GetAI())
                    ai->DoAction(ACTION_PICK_WEAPON);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_throngus_pick_weapon_SpellScript::HandleScriptEffect, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_throngus_pick_weapon_SpellScript();
        }
};

// Mighty Stomp 74984: Spell::EffectForceCast would make the hit player cast Cave In 74985 with the boss as its unit target
// (SpellEffects.cpp EffectForceCast), so the stalker would not land on the player; 4.3.4 casts it on the player itself.
// Here the boss summons the Cave In Stalker on the hit player's spot (registered for 74984 in spell_script_names).
class spell_throngus_mighty_stomp : public SpellScriptLoader
{
    public:
        spell_throngus_mighty_stomp() : SpellScriptLoader("spell_throngus_mighty_stomp") { }

        class spell_throngus_mighty_stomp_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_throngus_mighty_stomp_SpellScript);

            void FilterTargets(std::list<WorldObject*>& targets)
            {
                targets.remove_if([](WorldObject* target) { return target->GetTypeId() != TYPEID_PLAYER; });
                if (targets.size() > 1)
                    Trinity::Containers::RandomResizeList(targets, 1);
            }

            void HandleHit(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* target = GetHitUnit();
                if (!caster || !target)
                    return;

                if (UnitAI* ai = caster->GetAI())
                    ai->SetGUID(target->GetGUID(), ACTION_CAVE_IN);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_throngus_mighty_stomp_SpellScript::FilterTargets, EFFECT_0, TARGET_UNIT_SRC_AREA_ENEMY);
                OnEffectHitTarget += SpellEffectFn(spell_throngus_mighty_stomp_SpellScript::HandleHit, EFFECT_0, SPELL_EFFECT_FORCE_CAST);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_throngus_mighty_stomp_SpellScript();
        }
};

void AddSC_boss_forgemaster_throngus()
{
    new boss_forgemaster_throngus();
    new npc_throngus_fire_patch();
    new spell_throngus_pick_weapon();
    new spell_throngus_mighty_stomp();
}
