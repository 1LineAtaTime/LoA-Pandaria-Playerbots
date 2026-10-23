/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "DTKMultipliers.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "DTKActions.h"
#include "DTKTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"

// AC Ai/Dungeon/DTK/DTKMultipliers.cpp:15 (NovosMultiplier::GetValue)
float NovosMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "novos the summoner");
    if (!boss) { return 1.0f; }

    if (boss->FindCurrentSpellBySpellId(SPELL_ARCANE_FIELD) && bot->GetTarget())
    {
        if (dynamic_cast<DpsAssistAction*>(action)
            || dynamic_cast<TankAssistAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}

// AC Ai/Dungeon/DTK/DTKMultipliers.cpp:31 (TharonjaMultiplier::GetValue)
// 5.4.8: AC botAI->IsTank(bot) -> static PlayerBotSpec::IsTank(bot) (AI/PlayerbotSpec.h:11).
float TharonjaMultiplier::GetValue(Action* action)
{
    if (!bot->HasAura(SPELL_GIFT_OF_THARONJA)) { return 1.0f; }

    // Suppress all skills that are not enabled in skeleton form.
    // Still allow non-ability actions such as movement
    if (dynamic_cast<CastSpellAction*>(action)
        && !dynamic_cast<CastSlayingStrikeAction*>(action)
        && !dynamic_cast<CastTauntAction*>(action)
        && !dynamic_cast<CastBoneArmorAction*>(action)
        && !dynamic_cast<CastTouchOfLifeAction*>(action))
    {
        return 0.0f;
    }
    // Also suppress FleeAction to prevent ranged characters from avoiding melee range
    if (dynamic_cast<FleeAction*>(action))
    {
        return 0.0f;
    }

    // Tanks should only taunt, no slaying strike
    if (PlayerBotSpec::IsTank(bot))
    {
        if (dynamic_cast<CastSlayingStrikeAction*>(action))
        {
            return 0.0f;
        }
    }
    // Dps & healer should not taunt
    else
    {
        if (dynamic_cast<CastTauntAction*>(action))
        {
            return 0.0f;
        }
    }
    return 1.0f;
}
