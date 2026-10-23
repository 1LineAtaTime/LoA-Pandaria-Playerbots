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

enum ScriptTexts
{
    SAY_AGGRO   = 0,
    SAY_KILL    = 1,
    SAY_SIPHON  = 2,
    SAY_BOLTS   = 3,
    SAY_PET     = 4,
    SAY_DEATH   = 5,
};

enum Spells
{
    SPELL_SPIRIT_BOLTS              = 43383,
    SPELL_DRAIN_POWER               = 44131,
    SPELL_SIPHON_SOUL               = 43501,

    SPELL_FLASH_HEAL                = 43575,
    SPELL_DISPEL_MAGIC              = 43577,

    SPELL_FIREBOLT                  = 43584,

    SPELL_PSYCHIC_WAIL              = 43590,

    SPELL_VENOM_SPIT                = 43579,

    // #678: the Cataclysm 5-man kit, Wowpedia "Hex Lord Malacrass" (MediaWiki API, 2026-10-23) "Siphon Soul" class lists. Ids:
    // the 5.4.8 Spell/SpellEffect.dbc (name + effect + base points against Wowpedia's numbers) and the Wowhead tooltip API
    // (cata / mop-classic, 2026-10-23): 61600 "100% weapon damage plus 500", 61603 "15000 Shadow damage every 2 sec ... 10 sec",
    // 127734 "150% weapon damage ... 25% for 15 sec", 127735 "46250 to 53750 every 3 sec for 15 sec", 127741 "41625 to 48375".

    // Druid
    SPELL_DR_LIFEBLOOM              = 43421,
    SPELL_DR_MOONFIRE               = 43545,
    SPELL_DR_TYPHOON                = 97636, // cone in front (DEST_CASTER_FRONT -> 97637 30 yd cone damage + knockback)

    // Hunter
    SPELL_HU_EXPLOSIVE_TRAP         = 43444,
    SPELL_HU_FREEZING_TRAP          = 43447,
    SPELL_HU_SNAKE_TRAP             = 43449,

    // Mage
    SPELL_MG_FIREBALL               = 41383,
    SPELL_MG_FROST_NOVA             = 43426,
    SPELL_MG_ICE_LANCE              = 43427,
    SPELL_MG_FROSTBOLT              = 43428,

    // Paladin
    SPELL_PA_CONSECRATION           = 43429,
    SPELL_PA_AVENGING_WRATH         = 43430,
    SPELL_PA_HOLY_LIGHT             = 43451,

    // Priest (Cata: Flash Heal 20% of max health on himself, Psychic Scream, Shadow Word: Pain; Leap of Faith 97640 is left out:
    // SCRIPT_EFFECT on TARGET_UNIT_SRC_AREA_ALLY with radius index 0 and no handler, it would do nothing)
    SPELL_PR_FLASH_HEAL             = 43431,
    SPELL_PR_PSYCHIC_SCREAM         = 43432,
    SPELL_PR_SW_PAIN                = 97642,

    // Rogue (43457 was "Ball of Energy", not Slice and Dice; 43547 is the 50% attack speed Slice and Dice)
    SPELL_RO_SMOKE_BOMB             = 97643,
    SPELL_RO_SLICE_DICE             = 43547,
    SPELL_RO_WOUND_POISON           = 43461,

    // Shaman
    SPELL_SH_CHAIN_LIGHT            = 43435,
    SPELL_SH_FIRE_NOVA              = 43436,
    SPELL_SH_HEALING_WAVE           = 43548,

    // Warlock
    SPELL_WL_CURSE_OF_DOOM          = 43439,
    SPELL_WL_RAIN_OF_FIRE           = 43440,
    SPELL_WL_UNSTABLE_AFFL          = 43522,
    SPELL_WL_UNSTABLE_AFFL_DISPEL   = 43523,

    // Warrior
    SPELL_WR_MORTAL_STRIKE          = 43441,
    SPELL_WR_HEROIC_LEAP            = 97521,
    SPELL_WR_SPELL_REFLECT          = 43443,

    // Death Knight
    SPELL_DK_PLAGUE_STRIKE          = 61600,
    SPELL_DK_DEATH_AND_DECAY        = 61603,
    SPELL_DK_BLOOD_WORMS            = 97628, // aura: 4 x 97627 (summon Blood Worm 52827 within 3 yd), 500 ms ticks for 2 s
    SPELL_DK_BLOOD_WORMS_HEAL       = 97630, // "Heals the target for 3% of their life"

    // Monk
    SPELL_MK_RISING_SUN_KICK        = 127734,
    SPELL_MK_HEX_SPHERE             = 127737, // area trigger 407 within 10 yd of the caster, 30 s
    SPELL_MK_HEX_SPHERE_DAMAGE      = 127741,
    SPELL_MK_RENEWING_MIST          = 127735,
};

enum Events
{
    EVENT_SPIRIT_BOLTS          = 1,
    EVENT_SIPHON_SOUL           = 2,
    EVENT_DRAIN_POWER           = 3,
    EVENT_PLAYER_ABILITY        = 4,
    EVENT_PLAYER_ABILITY_OFF    = 5,
};

enum Adds
{
    NPC_ALYSON_ANTILE   = 24240,
    NPC_SLITHER         = 24242, 
    NPC_GAZAKROTH       = 24244, 
    NPC_DARKHEART       = 24246,
    NPC_BLOOD_WORM      = 52827,
    NPC_HEX_LORD        = 24239,
};

const Position addPos[2] = 
{
    {109.43f, 922.57f, 33.90f, 1.57f},
    {126.40f, 922.78f, 33.90f, 1.57f}
};

enum AbilityTarget
{
    ABILITY_TARGET_SELF    = 0,
    ABILITY_TARGET_VICTIM  = 1,
    ABILITY_TARGET_ENEMY   = 2,
    ABILITY_TARGET_HEAL    = 3,
    ABILITY_TARGET_BUFF    = 4,
    ABILITY_TARGET_SPECIAL = 5,
    ABILITY_TARGET_FACING  = 6, // #678: turn to a random enemy, then cast on self (frontal cone)
    ABILITY_TARGET_WORMS   = 7, // #678: Blood Worms at a random player's feet
};

struct PlayerAbilityStruct
{
    uint32 spell;
    AbilityTarget target;
    uint32 cooldown;
};

// #678: indexed by class id (CLASS_WARRIOR 1 .. CLASS_DRUID 11, 5.4.8 SharedDefines.h); row 0 unused. The old TBC table had 10
// rows indexed by class - 1 with a separate Shadow Priest row (a Druid read past the end), and no Death Knight or Monk kit.
// Spell 0 = empty slot.
static PlayerAbilityStruct const PlayerAbility[MAX_CLASSES][3] =
{
    // 0 (no class)
    {
        { 0, ABILITY_TARGET_SELF, 0 }, { 0, ABILITY_TARGET_SELF, 0 }, { 0, ABILITY_TARGET_SELF, 0 }
    },
    // 1 Warrior
    {
        { SPELL_WR_MORTAL_STRIKE,   ABILITY_TARGET_VICTIM,  6000  },
        { SPELL_WR_SPELL_REFLECT,   ABILITY_TARGET_SELF,    10000 },
        { SPELL_WR_HEROIC_LEAP,     ABILITY_TARGET_ENEMY,   10000 }
    },
    // 2 Paladin
    {
        { SPELL_PA_CONSECRATION,    ABILITY_TARGET_SELF,    10000 },
        { SPELL_PA_HOLY_LIGHT,      ABILITY_TARGET_HEAL,    10000 },
        { SPELL_PA_AVENGING_WRATH,  ABILITY_TARGET_SELF,    10000 }
    },
    // 3 Hunter
    {
        { SPELL_HU_EXPLOSIVE_TRAP,  ABILITY_TARGET_SELF,    10000 },
        { SPELL_HU_FREEZING_TRAP,   ABILITY_TARGET_SELF,    10000 },
        { SPELL_HU_SNAKE_TRAP,      ABILITY_TARGET_SELF,    10000 }
    },
    // 4 Rogue
    {
        { SPELL_RO_WOUND_POISON,    ABILITY_TARGET_VICTIM,  3000  },
        { SPELL_RO_SLICE_DICE,      ABILITY_TARGET_SELF,    10000 },
        { SPELL_RO_SMOKE_BOMB,      ABILITY_TARGET_ENEMY,   10000 }
    },
    // 5 Priest
    {
        { SPELL_PR_FLASH_HEAL,      ABILITY_TARGET_SELF,    10000 },
        { SPELL_PR_PSYCHIC_SCREAM,  ABILITY_TARGET_SELF,    10000 },
        { SPELL_PR_SW_PAIN,         ABILITY_TARGET_ENEMY,   10000 }
    },
    // 6 Death Knight
    {
        { SPELL_DK_PLAGUE_STRIKE,   ABILITY_TARGET_VICTIM,  6000  },
        { SPELL_DK_DEATH_AND_DECAY, ABILITY_TARGET_ENEMY,   10000 },
        { SPELL_DK_BLOOD_WORMS,     ABILITY_TARGET_WORMS,   10000 }
    },
    // 7 Shaman
    {
        { SPELL_SH_FIRE_NOVA,       ABILITY_TARGET_SELF,    10000 },
        { SPELL_SH_HEALING_WAVE,    ABILITY_TARGET_SELF,    10000 },
        { SPELL_SH_CHAIN_LIGHT,     ABILITY_TARGET_ENEMY,   8000  }
    },
    // 8 Mage
    {
        { SPELL_MG_FROST_NOVA,      ABILITY_TARGET_SELF,    10000 },
        { SPELL_MG_FROSTBOLT,       ABILITY_TARGET_ENEMY,   5000  },
        { SPELL_MG_ICE_LANCE,       ABILITY_TARGET_ENEMY,   2000  }
    },
    // 9 Warlock
    {
        { SPELL_WL_CURSE_OF_DOOM,   ABILITY_TARGET_ENEMY,   10000 },
        { SPELL_WL_RAIN_OF_FIRE,    ABILITY_TARGET_ENEMY,   10000 },
        { SPELL_WL_UNSTABLE_AFFL,   ABILITY_TARGET_ENEMY,   10000 }
    },
    // 10 Monk
    {
        { SPELL_MK_RISING_SUN_KICK, ABILITY_TARGET_VICTIM,  6000  },
        { SPELL_MK_HEX_SPHERE,      ABILITY_TARGET_SELF,    10000 },
        { SPELL_MK_RENEWING_MIST,   ABILITY_TARGET_SELF,    15000 }
    },
    // 11 Druid
    {
        { SPELL_DR_LIFEBLOOM,       ABILITY_TARGET_SELF,    10000 },
        { SPELL_DR_TYPHOON,         ABILITY_TARGET_FACING,  10000 },
        { SPELL_DR_MOONFIRE,        ABILITY_TARGET_ENEMY,   8000  }
    }
};

class boss_hex_lord_malacrass : public CreatureScript
{
    public:
        boss_hex_lord_malacrass() : CreatureScript("boss_hex_lord_malacrass") { }

        struct boss_hex_lord_malacrassAI : public BossAI
        {
            boss_hex_lord_malacrassAI(Creature* creature) : BossAI(creature, DATA_HEX_LORD_MALACRASS)
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

            uint32 PlayerClass;

            void Reset() override
            {
                _Reset();

                PlayerClass = 0; // #678: set before UseAbility can run (it is only scheduled after a Siphon Soul)

                switch (urand(0, 5))
                {
                    case 0:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_SLITHER, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 1:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_GAZAKROTH, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 2:
                        me->SummonCreature(NPC_ALYSON_ANTILE, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 3:
                        me->SummonCreature(NPC_GAZAKROTH, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_SLITHER, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 4:
                        me->SummonCreature(NPC_GAZAKROTH, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                    case 5:
                        me->SummonCreature(NPC_SLITHER, addPos[0], TEMPSUMMON_DEAD_DESPAWN);
                        me->SummonCreature(NPC_DARKHEART, addPos[1], TEMPSUMMON_DEAD_DESPAWN);
                        break;
                }       

                me->SetUInt32Value(UNIT_FIELD_VIRTUAL_ITEM_ID, 46916);
                me->SetSheath(SHEATH_STATE_MELEE);
            }

            void JustEngagedWith(Unit* /*who*/) override
            {
                events.ScheduleEvent(EVENT_SPIRIT_BOLTS, 10000);
                events.ScheduleEvent(EVENT_DRAIN_POWER, 6000);
                Talk(SAY_AGGRO);
                DoZoneInCombat();
                instance->SetBossState(DATA_HEX_LORD_MALACRASS, IN_PROGRESS);
            }

            void KilledUnit(Unit* /*victim*/) override
            {
                Talk(SAY_KILL);
            }

            void SummonedCreatureDies(Creature* /*summon*/, Unit* /*killer*/) override
            {
                Talk(SAY_PET);
            }

            void JustDied(Unit* /*killer*/) override
            {
                _JustDied();
                Talk(SAY_DEATH);
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
                        case EVENT_DRAIN_POWER:
                            DoCast(me, SPELL_DRAIN_POWER, true);
                            events.ScheduleEvent(EVENT_DRAIN_POWER, 40000);
                            break;
                        case EVENT_SPIRIT_BOLTS:
                            Talk(SAY_BOLTS);
                            DoCast(me, SPELL_SPIRIT_BOLTS);
                            events.ScheduleEvent(EVENT_SPIRIT_BOLTS, 45000);
                            events.ScheduleEvent(EVENT_SIPHON_SOUL, 10000);
                            break;
                        case EVENT_SIPHON_SOUL:
                            if (Unit* target = SelectTarget(SELECT_TARGET_RANDOM, 0, 0.0f, true))
                            {
                                // #678: one row per class id (the Cata kit table above, Death Knight and Monk included).
                                PlayerClass = target->GetClass();

                                // #678: never index past the table, whatever class id comes back (row 0 = no ability).
                                if (PlayerClass >= MAX_CLASSES)
                                    PlayerClass = 0;

                                DoCast(target, SPELL_SIPHON_SOUL);
                                me->ClearUnitState(UNIT_STATE_CASTING);
                                events.ScheduleEvent(EVENT_PLAYER_ABILITY, urand(5000, 8000));
                                events.ScheduleEvent(EVENT_PLAYER_ABILITY_OFF, 30000);
                            }
                            break;
                        case EVENT_PLAYER_ABILITY:
                            UseAbility();
                            events.ScheduleEvent(EVENT_PLAYER_ABILITY, urand(8000, 10000));
                            break;
                        case EVENT_PLAYER_ABILITY_OFF:
                            events.CancelEvent(EVENT_PLAYER_ABILITY);
                            break;
                     }
                }

                DoMeleeAttackIfReady();
            }

            void UseAbility()
            {
                uint8 random = urand(0, 2);
                PlayerAbilityStruct const& ability = PlayerAbility[PlayerClass][random];
                if (!ability.spell)
                    return;

                Unit* target = NULL;
                switch (ability.target)
                {
                    case ABILITY_TARGET_SELF:
                        target = me;
                        break;
                    case ABILITY_TARGET_VICTIM:
                        target = me->GetVictim();
                        break;
                    case ABILITY_TARGET_ENEMY:
                    default:
                        target = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true);
                        break;
                    case ABILITY_TARGET_HEAL:
                        target = DoSelectLowestHpFriendly(50, 0);
                        break;
                    case ABILITY_TARGET_BUFF:
                        {
                            std::list<Creature*> templist = DoFindFriendlyMissingBuff(50, ability.spell);
                            if (!templist.empty())
                                target = *(templist.begin());
                        }
                        break;
                    case ABILITY_TARGET_FACING:
                        // #678 Typhoon: the cone goes where he faces (97636 effect 0 TARGET_DEST_CASTER_FRONT).
                        if (Unit* enemy = SelectTarget(SELECT_TARGET_RANDOM, 0, 30.0f, true))
                        {
                            me->SetFacingTo(me->GetAngle(enemy));
                            target = me;
                        }
                        break;
                    case ABILITY_TARGET_WORMS:
                        // #678 Blood Worms (Wowpedia): "infects a random target with Blood Worms, causing worms to emerge from
                        // the infected player's location. These worms move towards Malacrass and heal him for 3% of his maximum
                        // health". 97628 on a player would make the PLAYER cast its 97627 summons (periodic trigger,
                        // SpellInfo::NeedsToBeTriggeredByCaster false -> the aura target casts), so the worms would not be his
                        // summons. Same result as his summons: four worms (97628: 2 s of 500 ms ticks) within 3 yd of the target.
                        if (Unit* infected = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        {
                            for (uint8 i = 0; i < 4; ++i)
                            {
                                Position pos = infected->GetRandomNearPosition(3.0f);
                                me->SummonCreature(NPC_BLOOD_WORM, pos, TEMPSUMMON_CORPSE_TIMED_DESPAWN, 5000ms);
                            }
                        }
                        return;
                }
                if (target)
                    DoCast(target, ability.spell, false);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<boss_hex_lord_malacrassAI>(creature);
        }
};

class npc_alyson_antille : public CreatureScript
{
    public:
        npc_alyson_antille() : CreatureScript("npc_alyson_antille") { }

        struct npc_alyson_antilleAI : public ScriptedAI
        {
            npc_alyson_antilleAI(Creature* creature) : ScriptedAI(creature) { }

            uint32 flashheal_timer;
            uint32 dispelmagic_timer;

            void Reset() override
            {
                flashheal_timer = 2500;
                dispelmagic_timer = 10000;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (flashheal_timer <= diff)
                {
                    Unit* target = DoSelectLowestHpFriendly(99, 30000);
                    if (target)
                    {
                        if (target->IsWithinDistInMap(me, 50))
                            DoCast(target, SPELL_FLASH_HEAL, false);
                        else
                        {
                            // bugged
                            //me->GetMotionMaster()->Clear();
                            //me->GetMotionMaster()->MoveChase(target, 20);
                        }
                    }
                    else
                    {
                        if (urand(0, 1))
                            target = DoSelectLowestHpFriendly(50, 0);
                        else
                            target = SelectTarget(SELECT_TARGET_RANDOM, 0);
                        if (target)
                            DoCast(target, SPELL_DISPEL_MAGIC, false);
                    }
                    flashheal_timer = 2500;
                } else flashheal_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_alyson_antilleAI>(creature);
        }
};

class npc_gazakroth : public CreatureScript
{
    public:
        npc_gazakroth() : CreatureScript("npc_gazakroth") { }

        struct npc_gazakrothAI : public ScriptedAI
        {
            npc_gazakrothAI(Creature* creature) : ScriptedAI(creature)  { }

            uint32 firebolt_timer;

            void Reset() override
            {
                firebolt_timer = 2000;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (firebolt_timer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_FIREBOLT, false);
                    firebolt_timer = 700;
                } else firebolt_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_gazakrothAI>(creature);
        }
};

class npc_darkheart : public CreatureScript
{
    public:
        npc_darkheart() : CreatureScript("npc_darkheart") { }

        struct npc_darkheartAI : public ScriptedAI
        {
            npc_darkheartAI(Creature* creature) : ScriptedAI(creature)  { }

            uint32 psychicwail_timer;

            void Reset() override
            {
                psychicwail_timer = 8000;
            }
            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (psychicwail_timer <= diff)
                {
                    DoCast(me->GetVictim(), SPELL_PSYCHIC_WAIL, false);
                    psychicwail_timer = 12000;
                } else psychicwail_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_darkheartAI>(creature);
        }
};

class npc_slither : public CreatureScript
{
    public:
        npc_slither() : CreatureScript("npc_slither") { }

        struct npc_slitherAI : public ScriptedAI
        {
            npc_slitherAI(Creature* creature) : ScriptedAI(creature) { }

            uint32 venomspit_timer;

            void Reset() override
            {
                venomspit_timer = 5000;
            }

            void UpdateAI(uint32 diff) override
            {
                if (!UpdateVictim())
                    return;

                if (venomspit_timer <= diff)
                {
                    if (Unit* victim = SelectTarget(SELECT_TARGET_RANDOM, 0, 100, true))
                        DoCast(victim, SPELL_VENOM_SPIT, false);
                    venomspit_timer = 2500;
                } else venomspit_timer -= diff;

                DoMeleeAttackIfReady();
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_slitherAI>(creature);
        }
};

class spell_hexlord_unstable_affliction : public SpellScriptLoader
{
    public:
        spell_hexlord_unstable_affliction() : SpellScriptLoader("spell_hexlord_unstable_affliction") { }

        class spell_hexlord_unstable_affliction_AuraScript : public AuraScript
        {
            PrepareAuraScript(spell_hexlord_unstable_affliction_AuraScript);

            bool Validate(SpellInfo const* /*spell*/) override
            {
                if (!sSpellMgr->GetSpellInfo(SPELL_WL_UNSTABLE_AFFL_DISPEL))
                    return false;
                return true;
            }

            void HandleDispel(DispelInfo* dispelInfo)
            {
                if (Unit* caster = GetCaster())
                    caster->CastSpell(dispelInfo->GetDispeller(), SPELL_WL_UNSTABLE_AFFL_DISPEL, true, NULL, GetEffect(EFFECT_0));
            }

            void Register() override
            {
                AfterDispel += AuraDispelFn(spell_hexlord_unstable_affliction_AuraScript::HandleDispel);
            }
        };

        AuraScript* GetAuraScript() const override
        {
            return new spell_hexlord_unstable_affliction_AuraScript();
        }
};

// #678: Blood Worm 52827 (Death Knight kit). Wowpedia "Hex Lord Malacrass": "These worms move towards Malacrass and heal him for
// 3% of his maximum health when they reach him." Killable on the way (template faction 14, 5.4.8 run speed 0.43).
class npc_hexlord_blood_worm : public CreatureScript
{
    public:
        npc_hexlord_blood_worm() : CreatureScript("npc_hexlord_blood_worm") { }

        struct npc_hexlord_blood_wormAI : public ScriptedAI
        {
            npc_hexlord_blood_wormAI(Creature* creature) : ScriptedAI(creature) { }

            uint32 checkTimer = 0;
            bool fed = false;

            void Reset() override
            {
                me->SetReactState(REACT_PASSIVE);
                checkTimer = 0;
            }

            // It only crawls to Malacrass; it never turns on a player (the zone-in-combat of his summons must not start a chase).
            void AttackStart(Unit* /*who*/) override { }
            void MoveInLineOfSight(Unit* /*who*/) override { }

            Creature* GetHexLord()
            {
                if (TempSummon* summon = me->ToTempSummon())
                    if (Unit* summoner = summon->GetSummoner())
                        if (Creature* hexLord = summoner->ToCreature())
                            return hexLord;
                return me->FindNearestCreature(NPC_HEX_LORD, 100.0f);
            }

            void UpdateAI(uint32 diff) override
            {
                if (fed)
                    return;

                if (checkTimer > diff)
                {
                    checkTimer -= diff;
                    return;
                }
                checkTimer = 500;

                Creature* hexLord = GetHexLord();
                if (!hexLord || !hexLord->IsAlive() || !hexLord->IsInCombat())
                {
                    me->DespawnOrUnsummon();
                    return;
                }

                if (me->IsWithinMeleeRange(hexLord) || me->GetExactDist2d(hexLord) <= 3.0f)
                {
                    fed = true;
                    if (SpellInfo const* heal = sSpellMgr->GetSpellInfo(SPELL_DK_BLOOD_WORMS_HEAL))
                        me->HealBySpell(hexLord, heal, hexLord->CountPctFromMaxHealth(heal->Effects[EFFECT_0].BasePoints > 0 ? heal->Effects[EFFECT_0].BasePoints : 3));
                    me->DespawnOrUnsummon();
                    return;
                }

                if (me->GetMotionMaster()->GetCurrentMovementGeneratorType() != FOLLOW_MOTION_TYPE)
                    me->GetMotionMaster()->MoveFollow(hexLord, 0.0f, 0.0f);
            }
        };

        CreatureAI* GetAI(Creature* creature) const override
        {
            return GetInstanceAI<npc_hexlord_blood_wormAI>(creature);
        }
};

// #678: area trigger 407 from Hex Sphere 127737 (Monk kit). Wowpedia: "a sphere of dark energy on the ground near him that last for
// 30 sec, dealing 41625 to 48375 Shadow damage to enemies that touch it" = 127741. One touch consumes it (IAreaTriggerOnce, the
// sat_monk_healing_sphere pattern, spell_monk.cpp:2295).
class sat_hexlord_hex_sphere : public IAreaTriggerOnce
{
    bool CheckTriggering(WorldObject* object) override
    {
        Player* player = object ? object->ToPlayer() : nullptr;
        return player && player->IsAlive() && GetCaster() && GetCaster()->IsAlive() && player->IsHostileTo(GetCaster());
    }

    void OnTrigger(WorldObject* target) override
    {
        if (Unit* unit = target->ToUnit())
            GetCaster()->CastSpell(unit, SPELL_MK_HEX_SPHERE_DAMAGE, true);
    }
};

void AddSC_boss_hex_lord_malacrass()
{
    new npc_hexlord_blood_worm();
    new atrigger_script<sat_hexlord_hex_sphere>("sat_hexlord_hex_sphere");
    new boss_hex_lord_malacrass();
    new npc_gazakroth();
    new npc_darkheart();
    new npc_slither();
    new npc_alyson_antille();
    new spell_hexlord_unstable_affliction();
}
