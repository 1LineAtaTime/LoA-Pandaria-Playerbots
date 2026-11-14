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

/* ScriptData
SDName: Eastern Plaguelands, the Plaguewood and the finale
SDComment: Quest support: 27525 Guardians of Stratholme, 27530 The Corpsebeasts, 27526 Gidwin's Fate Revealed
SDCategory: Eastern Plaguelands
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "SpellScript.h"
#include "SpellInfo.h"
#include "TemporarySummon.h"
#include "Vehicle.h"
#include "Timer.h"
#include "WorldSession.h"

/*######
## Guardians of Stratholme (27525)
######*/

enum GuardiansOfStratholme
{
    NPC_OMASUM_BLIGHTHOOF                       = 45867,
    SPELL_DEATH_GRIP_JUMP                       = 49575     // the death knights' own pull: the target jumps to a destination
};

// A target this close is already at the caster's feet (melee)
float const DeathGripMinDistance = 5.0f;

// 79894 - Death Grip (Omasum Blighthoof 45867, "Cast Death Grip on Close", 10-30 yd)
// Its only effect is a dummy on the target. As a player death knight's grip (49576 -> 49560 -> 49575), the target is pulled to the
// caster: here it jumps to a spot 2 yd in front of Omasum, who faces his target (the spell needs facing). Only for Omasum: the
// spell is also cast by Mogu Infiltrator 66206 (every 2-4 s in combat, at any range) and Tharil'zun 486, for whom it stays the
// dummy it was. Karthis Darkrune 45868 does not have it. Not for a target already in melee range, on a vehicle or a taxi, or
// deflecting all spells (Deterrence), as the player death knight's grip (spell_dk_death_grip_effect)
class spell_epl_lieutenant_death_grip : public SpellScript
{
    PrepareSpellScript(spell_epl_lieutenant_death_grip);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (caster->GetEntry() != NPC_OMASUM_BLIGHTHOOF || !target || target == caster || target->GetVehicle() || target->IsInFlight()
            || caster->IsWithinMeleeRange(target) || caster->GetExactDist2d(target) < DeathGripMinDistance
            || target->GetTotalAuraModifier(SPELL_AURA_DEFLECT_SPELLS) >= 100)
            return;

        Position const dest = caster->GetNearPosition(2.0f, 0.0f);
        target->CastSpell(dest, SPELL_DEATH_GRIP_JUMP, true);
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_epl_lieutenant_death_grip::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};

/*######
## Scourged Mass (27528)
######*/

enum ScourgedMass
{
    NPC_OVERSTUFFED_GOLEM                       = 45851,
    SPELL_BURN_CORPSE                           = 85555
};

// Burn Corpse's reach (SpellRange 7 yd)
float const BurnCorpseRange = 7.0f;

// 85555 - Burn Corpse (Kevin Frost's Crusader's Torch 61369)
// "Kill Overstuffed Golems, then use the torch on their corpses": effect 0 puts the fire on the nearest Overstuffed Golem within
// 7 yd that is dead and not burning yet (conditions 13/1/85555), effect 1 is the credit (KILL_CREDIT2 to the caster, which this
// core gives to the caster's group as well: Player::RewardPlayerAndGroupAtEvent, the DBC's group form of the credit). When effect
// 0 finds no corpse the cast goes on without a target (Spell::SelectImplicitNearbyTargets returns quietly), so the credit is
// given only when effect 0 has its corpse, and the torch is refused beforehand with no such corpse in reach. Once per corpse: a
// burning corpse is not a target (the burn aura is death-persistent), and it despawns 5 s later (its SmartAI)
class spell_epl_burn_corpse : public SpellScript
{
    PrepareSpellScript(spell_epl_burn_corpse);

    static bool IsBurnable(Unit* caster, Creature* golem)
    {
        return !golem->IsAlive() && !golem->HasAura(SPELL_BURN_CORPSE) && caster->IsWithinDistInMap(golem, BurnCorpseRange);
    }

    SpellCastResult CheckCorpse()
    {
        Unit* caster = GetCaster();
        std::list<Creature*> golems;
        caster->GetCreatureListWithEntryInGrid(golems, NPC_OVERSTUFFED_GOLEM, BurnCorpseRange + 10.0f);
        for (Creature* golem : golems)
            if (IsBurnable(caster, golem))
                return SPELL_CAST_OK;
        return SPELL_FAILED_NO_VALID_TARGETS;
    }

    void NoteCorpse(WorldObject*& target)
    {
        _corpse = target && target->ToCreature() && target->GetEntry() == NPC_OVERSTUFFED_GOLEM;
    }

    void HandleCredit(SpellEffIndex effIndex)
    {
        if (!_corpse)
            PreventHitDefaultEffect(effIndex);
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_burn_corpse::CheckCorpse);
        OnObjectTargetSelect += SpellObjectTargetSelectFn(spell_epl_burn_corpse::NoteCorpse, EFFECT_0, TARGET_UNIT_NEARBY_ENTRY);
        OnEffectHitTarget += SpellEffectFn(spell_epl_burn_corpse::HandleCredit, EFFECT_1, SPELL_EFFECT_KILL_CREDIT2);
    }

    bool _corpse = false;
};

void AddSC_eastern_plaguelands_finale()
{
    new spell_script<spell_epl_lieutenant_death_grip>("spell_epl_lieutenant_death_grip");
    new spell_script<spell_epl_burn_corpse>("spell_epl_burn_corpse");
}
