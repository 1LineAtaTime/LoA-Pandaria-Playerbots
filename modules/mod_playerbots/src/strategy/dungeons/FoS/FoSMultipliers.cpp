/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "FoSMultipliers.h"
#include "PlayerbotSpec.h"
#include "ChooseTargetActions.h"
#include "FoSActions.h"
#include "FoSTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"

// AC Ai/Dungeon/FoS/FoSMultipliers.cpp:14
float BronjahmMultiplier::GetValue(Action* action) {
    Unit* boss = AI_VALUE2(Unit *, "find target", "bronjahm");
    if (!boss)
        return 1.0f;

    if (dynamic_cast<TankAssistAction*>(action))
        return 0.0f;

    if (bot->HasAura(SPELL_CORRUPT_SOUL))
    {
        if (dynamic_cast<MovementAction*>(action) && !dynamic_cast<MoveFromBronjahmAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/FoS/FoSMultipliers.cpp:32
float AttackFragmentMultiplier::GetValue(Action* action)
{

    Unit* fragment = nullptr;

    GuidVector targets = AI_VALUE(GuidVector, "possible targets no los");

    for (auto& target : targets)
    {
        Unit* unit = botAI->GetUnit(target);
        if (unit && unit->GetEntry() == NPC_CORRUPTED_SOUL_FRAGMENT)
        {
            fragment = unit;
            break;
        }
    }

    // 5.4.8 ADAPTATION: botAI->IsDps is static PlayerBotSpec::IsDps (AI/PlayerbotSpec.h:13)
    if (fragment && PlayerBotSpec::IsDps(bot) && dynamic_cast<BronjahmGroupPositionAction*>(action))
        return 0.0f;

    return 1.0f;
}
