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
* You should have received a copy of the GNU General Public License
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

// Hillsbrad Foothills: the bosses of the Durnholde Challenge (quests 28643-28647). Their spells come from the 5.4.8 DBC and Wowhead;
// no TDB has a script for them, so the state machines here are ours. Each script only acts for its Durnholde boss: other users of
// the same spells keep their current behaviour.

#include "ScriptMgr.h"
#include "PassiveAI.h"
#include "SpellScript.h"

namespace HillsbradDurnholde
{
enum Durnholde
{
    NPC_ZEPHYRUS        = 49263,
    NPC_TERACULA        = 49265,

    SPELL_TORNADO_AURA  = 80065, // periodic trigger of 80064 every 500 ms; Wowhead lists it as Tornado 42895's ability
    SPELL_RUPTURE_SPIKE = 89933, // trigger missile with a fixed 2 s delay (SpellMisc speed 2, SPECIAL_DELAY_CALCULATION) of 89936
};

// Tornado 42895, summoned for 5 s under Zephyrus's target by 80066.
// The template is level 80 (Howling Riftdweller 39844 summons it too). 80064 has SPELL_ATTR0_LEVEL_DAMAGE_CALCULATION, so at level 80 its
// 7-8 damage would become about 500 a tick (SpellEffectInfo::CalcValue); at Zephyrus's level it is about 10, which fits the Wowhead 28643
// comment "the damage is minimal". With no script its hostile faction would also give it AggressorAI, so a level 80 elite melee.
struct npc_durnholde_tornado : public NullCreatureAI
{
    explicit npc_durnholde_tornado(Creature* creature) : NullCreatureAI(creature) { }

    void IsSummonedBy(Unit* summoner) override
    {
        if (!summoner || summoner->GetEntry() != NPC_ZEPHYRUS)
            return;

        me->SetLevel(summoner->GetLevel());
        me->CastSpell(me, SPELL_TORNADO_AURA, true);
    }
};

// Rupture Line 89932: "Breaks up the ground in front of the caster, causing it to erupt in a spike of earth after 2 sec."
// The DBC has dummies only. As in boss_grilek.cpp (Gri'lek's 96619 has value 45 and his script draws a 45 yard line), the dummy's value
// is taken as the line's length, here 15 yards, towards the target. The 2 s in the tooltip and the comments is the missile's delay.
// Ours: straight line and spacing. 89936 has a 2 yard radius (EffectRadiusMaxIndex 7) and a player's 1.5 yard size adds to it, so a spike reaches 3.5 yards.
// All spikes land on the same tick, and each does about 360 at level 25, so they stand 7.5 yards apart and can never hit a player twice:
// the first at 3.5 yards, the second at 11, together covering 14.5 of the 15 yards but for a half yard gap.
class spell_durnholde_rupture_line : public SpellScript
{
    PrepareSpellScript(spell_durnholde_rupture_line);

    void HandleDummy(SpellEffIndex /*effIndex*/)
    {
        Unit* caster = GetCaster();
        Unit* target = GetHitUnit();
        if (!target || caster->GetEntry() != NPC_TERACULA)
            return;

        float const relativeAngle = caster->GetRelativeAngle(target);
        float const angle = caster->GetOrientation() + relativeAngle;
        Position const end = caster->GetFirstCollisionPosition(float(GetEffectValue()), relativeAngle);
        float const length = caster->GetExactDist2d(end.GetPositionX(), end.GetPositionY());

        for (float distance = std::min(3.5f, length); distance <= length; distance += 7.5f)
        {
            float const x = caster->GetPositionX() + distance * std::cos(angle);
            float const y = caster->GetPositionY() + distance * std::sin(angle);
            float z = caster->GetPositionZ();
            caster->UpdateGroundPositionZ(x, y, z);
            caster->CastSpell(x, y, z, SPELL_RUPTURE_SPIKE, true);
        }
    }

    void Register() override
    {
        OnEffectHitTarget += SpellEffectFn(spell_durnholde_rupture_line::HandleDummy, EFFECT_0, SPELL_EFFECT_DUMMY);
    }
};
}

void AddSC_hillsbrad_durnholde()
{
    using namespace HillsbradDurnholde;
    new creature_script<npc_durnholde_tornado>("npc_durnholde_tornado");
    new spell_script<spell_durnholde_rupture_line>("spell_durnholde_rupture_line");
}
