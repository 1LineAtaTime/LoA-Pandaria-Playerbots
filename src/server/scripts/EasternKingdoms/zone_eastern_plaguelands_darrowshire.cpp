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
SDName: Eastern Plaguelands, Darrowshire
SDComment: "Rewriting the Battle of Darrowshire" (quests 27383-27392)
SDCategory: Eastern Plaguelands
EndScriptData */

#include "ScriptMgr.h"
#include "ScriptedCreature.h"
#include "SpellScript.h"

/*######
## Pamela's Doll (27384): one Ghost of the Past per doll part
######*/

enum DarrowshireDoll
{
    NPC_GHOST_OF_THE_PAST           = 10940,
};

// Ghost of the Past cap (ours): a living ghost within 4.5 yd of the trap (3D: two parts lie 4.9 yd apart on two floors of one house,
// 157555 upstairs and 157567 downstairs, and each keeps its own ghost), or a living ghost already fighting the player the trap fired for
static constexpr float DOLL_GHOST_CAP_RANGE     = 4.5f;
static constexpr float DOLL_GHOST_CHASE_RANGE   = 40.0f;

// 17132 - Pamela's Doll Trap (summons a Ghost of the Past 10940 at the trap 176117, 5 min)
// The trap fires on approach (radius 4, its own cooldown and despawn) and through the doll part on every opening of the part's loot
// window (GameObject::TriggeringLinkedGameObject, which applies neither the trap's cooldown nor its despawn), so a player who opens a
// part twice would get two ghosts. Sourced: one ghost per part (OLD comment 2004-12-20 reply [+41], OLD 2010-01-06).
class spell_darrowshire_pamelas_doll_trap : public SpellScript
{
    PrepareSpellScript(spell_darrowshire_pamelas_doll_trap);

    void HandleSummon(SpellEffIndex effIndex)
    {
        Unit* caster = GetCaster();     // the trap's trigger, standing on the trap (GameObject::CastSpell)
        if (!caster)
            return;

        if (caster->FindNearestCreature(NPC_GHOST_OF_THE_PAST, DOLL_GHOST_CAP_RANGE, true))
        {
            PreventHitDefaultEffect(effIndex);
            return;
        }

        // the ghost of this player's first opening may have followed him a few yards off the part
        if (Unit* player = GetExplTargetUnit())
        {
            std::list<Creature*> ghosts;
            caster->GetCreatureListWithEntryInGrid(ghosts, NPC_GHOST_OF_THE_PAST, DOLL_GHOST_CHASE_RANGE);
            for (Creature* ghost : ghosts)
                if (ghost->IsAlive() && ghost->GetVictim() == player)
                {
                    PreventHitDefaultEffect(effIndex);
                    return;
                }
        }
    }

    void Register() override
    {
        OnEffectHit += SpellEffectFn(spell_darrowshire_pamelas_doll_trap::HandleSummon, EFFECT_0, SPELL_EFFECT_SUMMON);
    }
};

void AddSC_eastern_plaguelands_darrowshire()
{
    RegisterSpellScript(spell_darrowshire_pamelas_doll_trap);
}
