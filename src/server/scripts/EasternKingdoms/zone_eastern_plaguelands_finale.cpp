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

/*######
## The Corpsebeasts (27530)
######*/

enum TheCorpsebeasts
{
    NPC_ARGENT_LIGHTWELL                        = 45921,

    SPELL_LIGHT_BARRAGE                         = 85593,    // cast every second by each lightwell's glow on the hulks within 15 yd
    SPELL_HOLY_OVERLOAD                         = 85594,    // credits the players within 30 yd, ends the lightwells within 15 yd

    LIGHTWELLS_TO_OVERLOAD                      = 3
};

// A lightwell's barrage counts as long as its last hit is this recent: each lightwell hits once a second (85591's period), and the
// barrage's own aura lasts 1.5 s
uint32 const LightBarrageWindowMs = 1500;
// "Must be 10 yards apart" (the quest objectives)
float const LightwellSpacing = 10.0f;

// 45886, 45893, 45895 - Into the Corpsebeast (the lifeless hulks)
// "Place three or more Argent Lightwells around the corpsebeast ... to destroy it." Three lightwells must hit the same hulk: the
// hulk counts the distinct lightwells whose barrage hit it within the last 1.5 s, and at three it overloads (Holy Overload: the
// credit and the end of the lightwells) and goes 2 s later, as the SmartAI rows it replaces did after ten hits from any number.
// The barrage's own aura cannot be counted: the same spell from another caster replaces the old aura (not periodic, no stacking).
struct npc_epl_corpsebeast : public ScriptedAI
{
    npc_epl_corpsebeast(Creature* creature) : ScriptedAI(creature), _overloaded(false) { }

    void Reset() override
    {
        _barrages.clear();
        _overloaded = false;
    }

    void AttackStart(Unit* /*who*/) override { }
    void MoveInLineOfSight(Unit* /*who*/) override { }
    void UpdateAI(uint32 /*diff*/) override { }

    void SpellHit(Unit* caster, SpellInfo const* spell) override
    {
        if (_overloaded || !caster || spell->Id != SPELL_LIGHT_BARRAGE)
            return;

        uint32 const now = getMSTime();
        _barrages[caster->GetGUID()] = now;
        for (auto itr = _barrages.begin(); itr != _barrages.end();)
        {
            if (getMSTimeDiff(itr->second, now) > LightBarrageWindowMs)
                itr = _barrages.erase(itr);
            else
                ++itr;
        }

        if (_barrages.size() < LIGHTWELLS_TO_OVERLOAD)
            return;

        _overloaded = true;
        _barrages.clear();
        me->CastSpell(me, SPELL_HOLY_OVERLOAD, true);
        me->DespawnOrUnsummon(2 * IN_MILLISECONDS);
    }

private:
    std::map<ObjectGuid, uint32> _barrages;
    bool _overloaded;
};

// 85590 - Summon Argent Lightwell (Argent Lightwell Charm 61375)
// "Must be 10 yards apart": refused within 10 yd of any lightwell, anyone's, with the game's own message. Measured flat from where
// the new lightwell will stand: the summon's TARGET_DEST_CASTER_FRONT without a radius puts it the caster's object size in front
// of the caster (Spell::SelectImplicitCasterDestTargets), the same point taken here. Not by a condition: a range check adds the
// object sizes (the lightwell's combat reach of 15 would turn 10 yd into about 26). The sourced marker of the rule is the
// lightwell's own 10 yd buff (Lightwell's Glow 85591, "the lightwells will give you this buff if you're within 10 yards"), which
// stretches the same way here
class spell_epl_summon_argent_lightwell : public SpellScript
{
    PrepareSpellScript(spell_epl_summon_argent_lightwell);

    SpellCastResult CheckSpacing()
    {
        Unit* caster = GetCaster();
        Position const spot = caster->GetFirstCollisionPosition(caster->GetObjectSize(), 0.0f);
        std::list<Creature*> lightwells;
        caster->GetCreatureListWithEntryInGrid(lightwells, NPC_ARGENT_LIGHTWELL, LightwellSpacing + 10.0f);
        for (Creature* lightwell : lightwells)
        {
            if (lightwell->IsAlive() && lightwell->GetExactDist2d(&spot) < LightwellSpacing)
            {
                SetCustomCastResultMessage(SPELL_CUSTOM_ERROR_TOO_CLOSE_ARGENT_LIGHTWELL);
                return SPELL_FAILED_CUSTOM_ERROR;
            }
        }
        return SPELL_CAST_OK;
    }

    void Register() override
    {
        OnCheckCast += SpellCheckCastFn(spell_epl_summon_argent_lightwell::CheckSpacing);
    }
};

void AddSC_eastern_plaguelands_finale()
{
    new spell_script<spell_epl_lieutenant_death_grip>("spell_epl_lieutenant_death_grip");
    new spell_script<spell_epl_burn_corpse>("spell_epl_burn_corpse");
    new creature_script<npc_epl_corpsebeast>("npc_epl_corpsebeast");
    new spell_script<spell_epl_summon_argent_lightwell>("spell_epl_summon_argent_lightwell");
}
