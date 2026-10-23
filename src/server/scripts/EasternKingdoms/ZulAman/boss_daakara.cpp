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
#include "zulaman.h"

enum Scripttexts
{
    SAY_BEAR        = 0,
    SAY_DEATH       = 1,
    SAY_DRAGONHAWK  = 2,
    SAY_EAGLE       = 3,
    SAY_FIRE        = 4,
    SAY_KILL        = 5,
    SAY_LYNX        = 6,
    SAY_AGGRO       = 7,
};

enum Spells
{
    SPELL_WHIRLWIND                 = 17207,
    SPELL_GRIEVOUS_THROW            = 43093,

    SPELL_CREEPING_PARALYSIS        = 43095,
    SPELL_OVERPOWER                 = 43456,
    SPELL_SURGE                     = 42402,
    
    SPELL_SUMMON_AMANI_LYNX         = 97686,
    SPELL_LYNX_RUSH                 = 43153,
    SPELL_CLAW_RAGE_CHARGE          = 42583,
    SPELL_CLAW_RAGE                 = 43149,

    SPELL_FLAME_BREATH              = 43140,
    SPELL_FLAME_WHIRL               = 43213,
    SPELL_PILLAR_OF_FLAME_SUMMON    = 43216,
    SPELL_PILLAR_OF_FLAME_VISUAL    = 43541,
    SPELL_PILLAR_OF_FLAME_AURA      = 43218,

    // #678 eagle form (port of AC boss_zuljin.cpp:50-56 + spell_zuljin_zap :438-459): Energy Storm is an enemy area aura
    // (SpellEffect 129, 99 yd) whose proc (SpellAuraOptions procFlags 0x14000 = magic spells done, 500 ms cooldown) makes the
    // caster trigger Zap Inform 42577 on the nearest Feather Vortex (`conditions` 13/1/42577 -> 24136); the vortex answers
    // with Zap 43137 (8000 / heroic 12000 Nature) on that caster. Wowhead 43137 (cata, 2026-10-23): "Unleashes a blast of
    // electricity when a spell is cast nearby, inflicting 12000 Nature damage to the caster of the spell."
    SPELL_ENERGY_STORM              = 43983,
    SPELL_ZAP_INFORM                = 42577,
    SPELL_ZAP_DAMAGE                = 43137,
    //SPELL_SUMMON_VORTEX             = 43112,

    SPELL_SWEEPING_WINDS            = 97647,
    SPELL_FEATHER_VORTEX_VISUAL     = 97835,
    SPELL_FEATHER_VORTEX_PASSIVE    = 43120,
    SPELL_LIGHTNING_TOTEM           = 97930,

    //SPELL_SPIRIT_AURA               = 42466,
    //SPELL_SIPHON_SOUL               = 43501,

    SPELL_SHAPE_OF_THE_BEAR         = 42594,
    SPELL_SHAPE_OF_THE_EAGLE        = 42606,
    SPELL_SHAPE_OF_THE_LYNX         = 42607,
    SPELL_SHAPE_OF_THE_DRAGONHAWK   = 42608,

    SPELL_BERSERK                   = 45078,

    SPELL_FERAL_SWIPE               = 43357,
};

enum Events
{
    EVENT_WHIRLWIND             = 1,
    EVENT_GRIEVOUS_THROW        = 2,
    EVENT_CREEPING_PARALYSIS    = 3,
    EVENT_OVERPOWER             = 4,
    EVENT_SURGE                 = 5,
    EVENT_LYNX_RUSH             = 6,
    EVENT_CLAW_RAGE             = 7,
    EVENT_SWEEPING_WINDS        = 8,
    EVENT_FLAME_BREATH          = 9,
    EVENT_FLAME_WHIRL           = 10,
    EVENT_PILLAR_OF_FLAME       = 11,
    EVENT_BERSERK               = 12,
    EVENT_CONTINUE              = 13,
    EVENT_FERAL_SWIPE           = 14,
    EVENT_LIGHTNING_TOTEM       = 15,
    EVENT_CHECK_PLAYERS         = 16,
};

enum Adds
{
    NPC_PILLAR_OF_FLAME     = 24187,
    NPC_AMANI_LYNX          = 52839,
    NPC_FEATHER_VORTEX      = 24136,
};

enum Actions
{
    ACTION_MOVE_TO_PLAYER   = 1,
};

enum DaakaraForms
{
    DK_FORM_BEAR       = 0,
    DK_FORM_LYNX       = 1,
    DK_FORM_DRAGONHAWK = 2,
    DK_FORM_EAGLE      = 3,
    DK_FORM_NONE       = 0xFF,
};

#define POINT_SWEEP 1

#define CENTER_X 120.148811f
#define CENTER_Y 703.713684f
#define CENTER_Z 45.111477f
#define MAX_X 136.199707f
#define MIN_X 104.217331f
#define MAX_Y 715.623901f
#define MIN_Y 697.285217f

class boss_daakara : public CreatureScript
{
    public:
        boss_daakara() : CreatureScript("boss_daakara") { }

        struct boss_daakaraAI : public BossAI
        {
            boss_daakaraAI(Creature* creature) : BossAI(creature, DATA_DAAKARA)
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
                bAchieve = true;
            }

            void Reset() override
            {
                _Reset();

                phase = 0;
                animalForm = DK_FORM_NONE;
                bEnrage = false;
                bAchieve = true;

                me->SetReactState(REACT_AGGRESSIVE);
                me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEM_ID, 33975);
                //me->SetUInt32Value(UNIT_VIRTUAL_ITEM_INFO, 218172674);
                //me->SetByteValue(UNIT_FIELD_BYTES_2, 0, SHEATH_STATE_MELEE);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                Talk(SAY_AGGRO);

                phase = 0;
                animalForm = DK_FORM_NONE;
                bEnrage = false;
                bAchieve = true;

                me->SetReactState(REACT_AGGRESSIVE);

                events.ScheduleEvent(EVENT_BERSERK, 660000);
                events.ScheduleEvent(EVENT_GRIEVOUS_THROW, urand(3000, 8000));
                events.ScheduleEvent(EVENT_WHIRLWIND, urand(6000, 8000));
                events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                DoZoneInCombat();
                instance->SetBossState(DATA_DAAKARA, IN_PROGRESS);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
            }

            void SpellHit(Unit* /*caster*/, const SpellInfo* spell) override
            {
                if (spell->HasEffect(SPELL_EFFECT_ATTACK_ME) ||
                    spell->HasAura(SPELL_AURA_MOD_TAUNT))
                    me->RemoveAurasDueToSpell(SPELL_CLAW_RAGE);
            }

            bool AllowAchieve()
            {
                return bAchieve;
            }

            void EnterPhase(uint8 form)
            {
                DoResetThreat();
                me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEM_ID, 0);
                
                switch (form)
                {
                    case 0:
                        Talk(SAY_BEAR);
                        me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
                        DoCast(me, SPELL_SHAPE_OF_THE_BEAR, true);
                        events.ScheduleEvent(EVENT_SURGE, urand(4000, 10000));
                        events.ScheduleEvent(EVENT_OVERPOWER, urand(2000, 9000));
                        events.ScheduleEvent(EVENT_CREEPING_PARALYSIS, urand(10000, 15000));
                        break;
                    case 1:
                        Talk(SAY_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_WHIRLWIND);
                        DoCast(me, SPELL_SHAPE_OF_THE_LYNX, true);
                        DoCast(me, SPELL_SUMMON_AMANI_LYNX, true);
                        events.ScheduleEvent(EVENT_LYNX_RUSH, urand(5000, 12000));
                        events.ScheduleEvent(EVENT_CLAW_RAGE, urand(8000, 15000));
                        break;
                    case 2:
                        Talk(SAY_DRAGONHAWK);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        //me->NearTeleportTo(CENTER_X, CENTER_Y, CENTER_Z, 0.0f);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_BEAR);
                        DoCast(me, SPELL_SHAPE_OF_THE_DRAGONHAWK, true);
                        events.ScheduleEvent(EVENT_FLAME_BREATH, urand(5000, 12000));
                        events.ScheduleEvent(EVENT_FLAME_WHIRL, urand(3000, 5000));
                        events.ScheduleEvent(EVENT_PILLAR_OF_FLAME, urand(12000, 17000));
                        break;
                    case 3:
                        Talk(SAY_EAGLE);
                        me->AttackStop();
                        me->SetReactState(REACT_PASSIVE);
                        //me->NearTeleportTo(CENTER_X, CENTER_Y, CENTER_Z, 0.0f);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_LYNX);
                        me->RemoveAurasDueToSpell(SPELL_SHAPE_OF_THE_BEAR);
                        DoCast(me, SPELL_SHAPE_OF_THE_EAGLE, true);
                        // #678: Zap needs the enemy aura up for the rest of the fight (AC boss_zuljin.cpp:332).
                        DoCast(me, SPELL_ENERGY_STORM, true);
                        // Wowpedia "Daakara": one Feather Cyclone per living player, at that player's feet.
                        std::list<Unit*> targets;
                        SelectTargetList(targets, 5, SELECT_TARGET_RANDOM, 0.0f, true);
                        if (targets.size() > 0)
                        {
                            for (std::list<Unit*>::const_iterator itr = targets.begin(); itr != targets.end(); ++itr)
                            {
                                if (Creature* pCyclone = me->SummonCreature(NPC_FEATHER_VORTEX, (*itr)->GetPositionX(), (*itr)->GetPositionY(), (*itr)->GetPositionZ(), (*itr)->GetOrientation()))
                                    pCyclone->AI()->SetGUID((*itr)->GetGUID());
                            }
                        }
                        events.ScheduleEvent(EVENT_SWEEPING_WINDS, 5000);
                        events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, urand(10000, 13000));
                        break;
                    }
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (me->HealthBelowPct(80) && phase == 0)
                {
                    phase++;
                    events.CancelEvent(EVENT_WHIRLWIND);
                    events.CancelEvent(EVENT_GRIEVOUS_THROW);
                    // #678: random bear or lynx, as before. A form aura already on him is kept (no-op in play; a test can
                    // set it with `npcaura 23863 42594|42607` to make the pair deterministic).
                    animalForm = urand(DK_FORM_BEAR, DK_FORM_LYNX);
                    if (me->HasAura(SPELL_SHAPE_OF_THE_BEAR))
                        animalForm = DK_FORM_BEAR;
                    else if (me->HasAura(SPELL_SHAPE_OF_THE_LYNX))
                        animalForm = DK_FORM_LYNX;
                    EnterPhase(animalForm);
                    return;
                }
                else if (me->HealthBelowPct(40) && phase == 1)
                {
                    phase++;
                    events.CancelEvent(EVENT_SURGE);
                    events.CancelEvent(EVENT_CREEPING_PARALYSIS);
                    events.CancelEvent(EVENT_OVERPOWER);
                    events.CancelEvent(EVENT_LYNX_RUSH);
                    events.CancelEvent(EVENT_CLAW_RAGE);
                    // #678: Wowpedia "Daakara" (MediaWiki API, 2026-10-23): "Daakara shifts to one of two possible pairs of
                    // animal forms (bear/eagle or lynx/dragonhawk)". Eagle restored: Sweeping Winds now finds the vortices
                    // (spell_daakara_sweeping_winds target hook), Zap is scripted (spell_daakara_zap), the bots have a
                    // cyclone edge (cata-za "za feather cyclone").
                    EnterPhase(animalForm == DK_FORM_BEAR ? DK_FORM_EAGLE : DK_FORM_DRAGONHAWK);
                    return;
                }

                events.Update(diff);

                if (me->HasUnitState(UNIT_STATE_CASTING))
                    return;

                if (uint32 eventId = events.ExecuteEvent())
                {
                     switch (eventId)
                     {
                        case EVENT_BERSERK:
                            DoCast(me, SPELL_BERSERK);
                            break;
                        case EVENT_CHECK_PLAYERS:
                        {
                            Map::PlayerList const &plrList = instance->instance->GetPlayers();
                            if (!plrList.isEmpty())
                            {
                                for (Map::PlayerList::const_iterator itr = plrList.begin(); itr != plrList.end(); ++itr)
                                    if (Player* player = itr->GetSource())
                                        if (player->GetPositionX() < MIN_X ||
                                            player->GetPositionX() > MAX_X ||
                                            player->GetPositionY() < MIN_Y ||
                                            player->GetPositionY() > MAX_Y)
                                        {
                                            bAchieve = false;
                                            break;
                                        }
                            }
                            if (bAchieve)
                                events.ScheduleEvent(EVENT_CHECK_PLAYERS, 5000);
                            break;
                        }
                        case EVENT_WHIRLWIND:
                            DoCastAOE(SPELL_WHIRLWIND);
                            events.ScheduleEvent(EVENT_WHIRLWIND, urand(12000, 15000));
                            break;
                        case EVENT_GRIEVOUS_THROW:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                                DoCast(target, SPELL_GRIEVOUS_THROW);
                            events.ScheduleEvent(EVENT_GRIEVOUS_THROW, urand(9000, 11000));
                            break;
                        case EVENT_CREEPING_PARALYSIS:
                            DoCastAOE(SPELL_CREEPING_PARALYSIS);
                            events.ScheduleEvent(EVENT_CREEPING_PARALYSIS, urand(15000, 18000));
                            break;
                        case EVENT_SURGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_FARTHEST, 0, 0.0f, true))
                                DoCast(target, SPELL_SURGE);
                            events.ScheduleEvent(EVENT_SURGE, urand(10000, 15000));
                            break;
                        case EVENT_OVERPOWER:
                            DoCastVictim(SPELL_OVERPOWER);
                            events.ScheduleEvent(EVENT_OVERPOWER, 10000);
                            break;
                        case EVENT_LYNX_RUSH:
                            DoCastVictim(SPELL_LYNX_RUSH);
                            events.ScheduleEvent(EVENT_LYNX_RUSH, 10000);
                            break;
                        case EVENT_CLAW_RAGE:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 1, 0.0f, true))
                                DoCast(target, SPELL_CLAW_RAGE_CHARGE);
                            events.ScheduleEvent(EVENT_CLAW_RAGE, urand(20000, 25000));
                            break;
                        case EVENT_FLAME_WHIRL:
                            DoCastAOE(SPELL_FLAME_WHIRL);
                            events.ScheduleEvent(EVENT_FLAME_WHIRL, 12000);
                            break;
                        case EVENT_FLAME_BREATH:
                            // #669: the dragonhawk form is REACT_PASSIVE after AttackStop() (EnterPhase case 2), so it has no
                            // victim, and 43140 needs a unit target (effect 0 TARGET_DEST_TARGET_ENEMY; Spell::InitExplicitTargets
                            // only falls back to GetVictim()): DoCastAOE never cast it. Breathe at a random player in range, as
                            // Jan'alai does (boss_janalai.cpp:249-253), turned to it first so the 97497 fire line
                            // (spell_janalai_flame_breath, laid along the caster's orientation) follows the cone.
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 50.0f, true))
                            {
                                me->SetFacingTo(me->GetAngle(target));
                                DoCast(target, SPELL_FLAME_BREATH);
                            }
                            events.ScheduleEvent(EVENT_FLAME_BREATH, 15000);
                            break;
                        case EVENT_PILLAR_OF_FLAME:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM,0, 0.0f, true))
                                DoCast(target, SPELL_PILLAR_OF_FLAME_SUMMON);
                            events.ScheduleEvent(EVENT_PILLAR_OF_FLAME, urand(12000, 18000));
                            break;
                        case EVENT_SWEEPING_WINDS:
                            DoCast(me, SPELL_SWEEPING_WINDS);
                            events.ScheduleEvent(EVENT_SWEEPING_WINDS, 8500);
                            break;
                        case EVENT_LIGHTNING_TOTEM:
                            DoCast(me, SPELL_LIGHTNING_TOTEM);
                            events.ScheduleEvent(EVENT_LIGHTNING_TOTEM, 15000);
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }

        private:
            ObjectGuid ClawTargetGUID;
            bool bEnrage;
            uint8 phase;
            uint8 animalForm;
            bool bAchieve;
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_daakaraAI>(creature);
        }
};

class npc_daakara_vortex : public CreatureScript
{
    public:
        npc_daakara_vortex() : CreatureScript("npc_daakara_vortex") { }

        struct npc_daakara_vortexAI : public ScriptedAI
        {
            npc_daakara_vortexAI(Creature* creature) : ScriptedAI(creature) 
            {
                DoCast(me, SPELL_FEATHER_VORTEX_VISUAL, true);
                me->SetReactState(REACT_PASSIVE);
                // #678: not a target (AC boss_zuljin.cpp npc_zuljin_vortex::Reset, UNIT_FLAG_NOT_SELECTABLE): the cyclones are
                // avoided, not killed; the template unit_flags (0x8000 = CAN_SWIM) leave them attackable.
                me->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NOT_SELECTABLE);
                bStart = false; // #678: was never initialised (read in UpdateAI before any write)
                startTimer = 2000;
                victimGUID = ObjectGuid::Empty;
                SetCombatMovement(false);
            }

            bool bStart;
            uint32 startTimer;
            ObjectGuid victimGUID;

            void Reset() override { }
            
            void SetGUID(ObjectGuid guid, int32 type) override
            {
                victimGUID = guid;
            }

            // #678: Wowpedia "Daakara" (2026-10-23): the cyclones "only move when affected by Sweeping Winds" and then "move in
            // a straight line to the location of a random player. Merely moving away from where you were standing when the
            // whirlwinds begin moving will allow you to avoid them." The old MoveChase on the summon-time player homed in
            // forever. Now: a straight run to where a random player stands at the end of the Sweeping Winds cast, then a stop.
            void DoAction(int32 action) override
            {
                if (action != ACTION_MOVE_TO_PLAYER)
                    return;

                Unit* target = nullptr;
                if (TempSummon* summon = me->ToTempSummon())
                    if (Unit* summoner = summon->GetSummoner())
                        if (summoner->IsAIEnabled && summoner->IsInCombat())
                            target = summoner->GetAI()->SelectTarget(SELECT_TARGET_RANDOM, 0, 100.0f, true);
                if (!target)
                    target = ObjectAccessor::GetUnit(*me, victimGUID);
                if (!target || !target->IsAlive())
                    return;

                me->SetWalk(false);
                me->GetMotionMaster()->Clear();
                me->GetMotionMaster()->MovePoint(POINT_SWEEP, target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
            }

            void UpdateAI(uint32 diff) override
            {
                if (startTimer <= diff && !bStart)
                {
                    bStart = true;
                    DoCast(me, SPELL_FEATHER_VORTEX_PASSIVE, true);
                }
                else
                    startTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_daakara_vortexAI>(creature);
        }
};

class npc_daakara_pillar_of_flame : public CreatureScript
{
    public:
        npc_daakara_pillar_of_flame() : CreatureScript("npc_daakara_pillar_of_flame") { }

        struct npc_daakara_pillar_of_flameAI : public ScriptedAI
        {
            npc_daakara_pillar_of_flameAI(Creature* creature) : ScriptedAI(creature) 
            {
                me->SetReactState(REACT_PASSIVE);
                DoCast(me, SPELL_PILLAR_OF_FLAME_VISUAL, true);
                bExplode = false;
                explodeTimer = 2000;
                SetCombatMovement(false);
            }
            
            bool bExplode;
            uint32 explodeTimer;

            void Reset() override { }

            void UpdateAI(uint32 diff) override
            {
                if (explodeTimer <= diff && !bExplode)
                {
                    bExplode = true;
                    me->RemoveAllAuras();
                    DoCast(me, SPELL_PILLAR_OF_FLAME_AURA, true);
                }
                else
                    explodeTimer -= diff;
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_daakara_pillar_of_flameAI>(creature);
        }
};

class npc_daakara_amani_lynx : public CreatureScript
{
    public:
        npc_daakara_amani_lynx() : CreatureScript("npc_daakara_amani_lynx") { }

        struct npc_daakara_amani_lynxAI : public ScriptedAI
        {
            npc_daakara_amani_lynxAI(Creature* creature) : ScriptedAI(creature) { }

            EventMap events;

            void Reset() override
            {
                events.Reset();
            }
            
            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_FERAL_SWIPE, urand(6000, 9000));
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
                        case EVENT_FERAL_SWIPE:
                            DoCastVictim(SPELL_FERAL_SWIPE);
                            events.ScheduleEvent(EVENT_FERAL_SWIPE, urand(7000, 12000));
                            break;
                     }
                }
                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_daakara_amani_lynxAI>(creature);
        }
};

class spell_daakara_claw_rage_charge : public SpellScriptLoader
{
    public:
        spell_daakara_claw_rage_charge() : SpellScriptLoader("spell_daakara_claw_rage_charge") { }

        class spell_daakara_claw_rage_charge_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_daakara_claw_rage_charge_SpellScript);

            void HandleCharge(SpellEffIndex /*effIndex*/)
            {
                if (GetCaster() && GetHitUnit())
                {
                    if (GetCaster()->IsAIEnabled)
                        GetCaster()->GetAI()->AttackStart(GetHitUnit());
                    GetCaster()->CastSpell(GetHitUnit(), SPELL_CLAW_RAGE, true);
                }
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_daakara_claw_rage_charge_SpellScript::HandleCharge, EFFECT_0, SPELL_EFFECT_CHARGE);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_daakara_claw_rage_charge_SpellScript();
        }
};

class spell_daakara_sweeping_winds : public SpellScriptLoader
{
    public:
        spell_daakara_sweeping_winds() : SpellScriptLoader("spell_daakara_sweeping_winds") { }

        class spell_daakara_sweeping_winds_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_daakara_sweeping_winds_SpellScript);

            // #678: 97647 effect 0 is TARGET_UNIT_SRC_AREA_ENTRY with SpellEffect radius index 0 and no `conditions` row
            // (checked 2026-10-23), so the area search found no vortex and nothing ever moved.
            // Every live Feather Vortex in the arena is a target ("causing nearby Feather Vortecies to move", Wowhead 97647).
            void SelectVortices(std::list<WorldObject*>& targets)
            {
                targets.clear();
                Unit* caster = GetCaster();
                if (!caster)
                    return;

                std::list<Creature*> vortices;
                caster->GetCreatureListWithEntryInGrid(vortices, NPC_FEATHER_VORTEX, 100.0f);
                for (Creature* vortex : vortices)
                    if (vortex->IsAlive())
                        targets.push_back(vortex);
            }

            void HandleDummy(SpellEffIndex /*effIndex*/)
            {
                if (GetHitUnit() && GetHitUnit()->GetEntry() == NPC_FEATHER_VORTEX && GetHitUnit()->IsAIEnabled)
                    GetHitUnit()->GetAI()->DoAction(ACTION_MOVE_TO_PLAYER);
            }

            void Register() override
            {
                OnObjectAreaTargetSelect += SpellObjectAreaTargetSelectFn(spell_daakara_sweeping_winds_SpellScript::SelectVortices, EFFECT_0, TARGET_UNIT_SRC_AREA_ENTRY);
                OnEffectHitTarget += SpellEffectFn(spell_daakara_sweeping_winds_SpellScript::HandleDummy, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_daakara_sweeping_winds_SpellScript();
        }
};

// #678: 42577 Zap Inform (Energy Storm proc). Port of AC boss_zuljin.cpp:438-459 spell_zuljin_zap: the hit Feather Vortex
// answers with Zap 43137 on the player whose spell triggered the proc (the 42577 caster).
class spell_daakara_zap : public SpellScriptLoader
{
    public:
        spell_daakara_zap() : SpellScriptLoader("spell_daakara_zap") { }

        class spell_daakara_zap_SpellScript : public SpellScript
        {
            PrepareSpellScript(spell_daakara_zap_SpellScript);

            bool Validate(SpellInfo const* /*spellInfo*/) override
            {
                return sSpellMgr->GetSpellInfo(SPELL_ZAP_DAMAGE) != nullptr;
            }

            void HandleScript(SpellEffIndex effIndex)
            {
                PreventHitDefaultEffect(effIndex);
                Unit* caster = GetCaster();
                Unit* vortex = GetHitUnit();
                if (caster && vortex && vortex->IsAlive() && caster->IsAlive())
                    vortex->CastSpell(caster, SPELL_ZAP_DAMAGE, true);
            }

            void Register() override
            {
                OnEffectHitTarget += SpellEffectFn(spell_daakara_zap_SpellScript::HandleScript, EFFECT_0, SPELL_EFFECT_SCRIPT_EFFECT);
            }
        };

        SpellScript* GetSpellScript() const override
        {
            return new spell_daakara_zap_SpellScript();
        }
};

typedef boss_daakara::boss_daakaraAI DaakaraAI;

class achievement_ring_out : public AchievementCriteriaScript
{
    public:
        achievement_ring_out() : AchievementCriteriaScript("achievement_ring_out") { }

        bool OnCheck(Player* /*source*/, Unit* target) override
        {
            if (!target)
                return false;

            if (DaakaraAI* daakaraAI = CAST_AI(DaakaraAI, target->GetAI()))
                return daakaraAI->AllowAchieve();

            return false;
        }
};

void AddSC_boss_daakara()
{
    new boss_daakara();
    new npc_daakara_vortex();
    new npc_daakara_pillar_of_flame();
    new npc_daakara_amani_lynx();
    new spell_daakara_claw_rage_charge();
    new spell_daakara_sweeping_winds();
    new spell_daakara_zap();
    new achievement_ring_out();
}

