/*
* This file is part of Project SkyFire https://www.projectskyfire.org. 
* See SKYFIRE27-LICENSE.md for Copyright information
*/

/*
 * Scripts for spells with SPELLFAMILY_DEATHKNIGHT and SPELLFAMILY_GENERIC spells used by deathknight players.
 * Ordered alphabetically using scriptname.
 * Scriptnames of files in this file should be prefixed with "spell_dk_".
 */

#include "ScriptMgr.h"
#include "SpellScript.h"
#include "SpellAuraEffects.h"
#include "Unit.h"
#include "Player.h"
#include "SpellHistory.h"
#include "Pet.h"

namespace SkyFire27_pet
{


enum HunterPetCalculate
{
     SPELL_TAMED_PET_PASSIVE_07         = 20784,
     SPELL_HUNTER_PET_SCALING_01        = 34902,
};

enum WarlockPetCalculate
{
     SPELL_PET_PASSIVE_CRIT             = 35695,
     SPELL_PET_PASSIVE_DAMAGE_TAKEN     = 35697,
     ENTRY_FELGUARD                     = 17252,
     ENTRY_VOIDWALKER                   = 1860,
     ENTRY_FELHUNTER                    = 417,
     ENTRY_SUCCUBUS                     = 1863,
     ENTRY_IMP                          = 416,
};

enum DKPetCalculate
{
    SPELL_DEATH_KNIGHT_RUNE_WEAPON_02   = 51906,
    SPELL_DEATH_KNIGHT_PET_SCALING_01   = 54566,
    SPELL_DEATH_KNIGHT_PET_SCALING_02   = 51996,
    SPELL_DEATH_KNIGHT_PET_SCALING_03   = 61697,
    ENTRY_ARMY_OF_THE_DEAD_GHOUL        = 24207,
};

class spell_gen_pet_calculate : public SpellScriptLoader
{
public:
    spell_gen_pet_calculate() : SpellScriptLoader("spell_gen_pet_calculate") { }

    class spell_gen_pet_calculate_AuraScript : public AuraScript
    {
        PrepareAuraScript(spell_gen_pet_calculate_AuraScript);

        bool Load() override
        {
            if (!GetCaster() || !GetCaster()->GetOwner() || GetCaster()->GetOwner()->GetTypeId() != TypeID::TYPEID_PLAYER)
                return false;
            return true;
        }

        void CalculateAmountCritSpell(AuraEffect const* /* aurEff */, float& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float CritSpell = 0.0f;
                // Crit from Intellect
                CritSpell += owner->GetSpellCritFromIntellect();
                // Increase crit from SPELL_AURA_MOD_SPELL_CRIT_CHANCE
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                // Increase crit from SPELL_AURA_MOD_CRIT_PCT
                CritSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                // Increase crit spell from spell crit ratings
                CritSpell += owner->GetRatingBonusValue(CombatRating::CR_CRIT_SPELL);

                amount += int32(CritSpell);
            }
        }

        void CalculateAmountCritMelee(AuraEffect const* /* aurEff */, float& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float CritMelee = 0.0f;
                // Crit from Agility
                CritMelee += owner->GetMeleeCritFromAgility();
                // Increase crit from SPELL_AURA_MOD_WEAPON_CRIT_PERCENT
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
                // Increase crit from SPELL_AURA_MOD_CRIT_PCT
                CritMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_CRIT_PCT);
                // Increase crit melee from melee crit ratings
                CritMelee += owner->GetRatingBonusValue(CombatRating::CR_CRIT_MELEE);

                amount += int32(CritMelee);
            }
        }

        void CalculateAmountMeleeHit(AuraEffect const* /* aurEff */, float& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitMelee = 0.0f;
                // Increase hit from SPELL_AURA_MOD_HIT_CHANCE
                HitMelee += owner->GetTotalAuraModifier(SPELL_AURA_MOD_HIT_CHANCE);
                // Increase hit melee from meele hit ratings
                HitMelee += owner->GetRatingBonusValue(CombatRating::CR_HIT_MELEE);

                amount += int32(HitMelee);
            }
        }

        void CalculateAmountSpellHit(AuraEffect const* /* aurEff */, float& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float HitSpell = 0.0f;
                // Increase hit from SPELL_AURA_MOD_SPELL_HIT_CHANCE
                HitSpell += owner->GetTotalAuraModifier(SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                // Increase hit spell from spell hit ratings
                HitSpell += owner->GetRatingBonusValue(CombatRating::CR_HIT_SPELL);

                amount += int32(HitSpell);
            }
        }

        void CalculateAmountExpertise(AuraEffect const* /* aurEff */, int32& amount, bool& /*canBeRecalculated*/)
        {
            if (Player* owner = GetCaster()->GetOwner()->ToPlayer())
            {
                // For others recalculate it from:
                float Expertise = 0.0f;
                // Increase hit from SPELL_AURA_MOD_EXPERTISE
                Expertise += owner->GetTotalAuraModifier(SPELL_AURA_MOD_EXPERTISE);
                // Increase Expertise from Expertise ratings
                Expertise += owner->GetRatingBonusValue(CombatRating::CR_EXPERTISE);

                amount += int32(Expertise);
            }
        }

        void Register() override
        {
            switch (m_scriptSpellId)
            {
            case SPELL_PET_PASSIVE_CRIT:
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_pet_calculate_AuraScript::CalculateAmountCritSpell, EFFECT_0, SPELL_AURA_MOD_SPELL_CRIT_CHANCE);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_pet_calculate_AuraScript::CalculateAmountCritMelee, EFFECT_1, SPELL_AURA_MOD_WEAPON_CRIT_PERCENT);
                break;
            case SPELL_DEATH_KNIGHT_PET_SCALING_03:
                //                    case SPELL_SHAMAN_PET_HIT:
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_pet_calculate_AuraScript::CalculateAmountMeleeHit, EFFECT_0, SPELL_AURA_MOD_HIT_CHANCE);
                DoEffectCalcAmount += AuraEffectCalcAmountFn(spell_gen_pet_calculate_AuraScript::CalculateAmountSpellHit, EFFECT_1, SPELL_AURA_MOD_SPELL_HIT_CHANCE);
                break;
            default:
                break;
            }
        }
    };

    AuraScript* GetAuraScript() const override
    {
        return new spell_gen_pet_calculate_AuraScript();
    }
};












// this doesnt actually fit in here
























void AddSC_pet_spell_scripts()
{
    new spell_gen_pet_calculate();
}

}

void AddSC_skyfire27_pet()
{
    SkyFire27_pet::AddSC_pet_spell_scripts();
}
