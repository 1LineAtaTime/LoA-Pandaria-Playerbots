/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "PoSMultipliers.h"
#include "ChooseTargetActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "PoSActions.h"
#include "PoSTriggers.h"

// AC Ai/Dungeon/PoS/PoSMultipliers.cpp:14
float IckAndKrickMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "ick");
    if (!boss)
        return 1.0f;

    // Allow the IckAndKrickAction to run
    if (dynamic_cast<IckAndKrickAction*>(action))
        return 1.0f;

    if (boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS) || boss->FindCurrentSpellBySpellId(SPELL_POISON_NOVA_POS_HC)) && bot->GetExactDist2d(boss) < 20.0f)
        return 0.0f;  // Cancel all other actions when we need to handle Poison Nova

    // 5.4.8 ADAPTATION: botAI->IsTank / IsHeal are static PlayerBotSpec::IsX (AI/PlayerbotSpec.h:11-12)
    if (bot->GetExactDist2d(boss) < 15.0f && bot->HasAura(SPELL_PURSUIT) && !PlayerBotSpec::IsTank(bot))
        return 0.0f;  // Cancel all other actions when we need to handle Pursuit

    if (!PlayerBotSpec::IsHeal(bot) && boss->HasUnitState(UNIT_STATE_CASTING) && (boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_ICK) || boss->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BARRAGE_KRICK)))
        return 0.0f;  // Cancel all other actions when we need to handle Explosive Barrage

    return 1.0f;
}

// AC Ai/Dungeon/PoS/PoSMultipliers.cpp:36
float GarfrostMultiplier::GetValue(Action* /*action*/)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "garfrost");
    if (!boss)
        return 1.0f;

    return 1.0f;
}
