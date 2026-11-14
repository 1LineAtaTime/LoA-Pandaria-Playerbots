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

void AddSC_eastern_plaguelands_finale()
{
    new spell_script<spell_epl_lieutenant_death_grip>("spell_epl_lieutenant_death_grip");
}
