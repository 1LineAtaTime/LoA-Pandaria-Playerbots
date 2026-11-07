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

    SPELL_TORNADO_AURA  = 80065, // periodic trigger of 80064 every 500 ms; Wowhead lists it as Tornado 42895's ability
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
}

void AddSC_hillsbrad_durnholde()
{
    using namespace HillsbradDurnholde;
    new creature_script<npc_durnholde_tornado>("npc_durnholde_tornado");
}
