/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "RogueActions.h"
#include "RogueFinishingActions.h"

#include "Event.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "Playerbots.h"

// #742 (AC Ai/Class/Rogue/Action/RogueActions.cpp:21-27): no Stealth when the current target is spell distance or more away.
bool CastStealthAction::isUseful()
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && bot->GetDistance(target) >= sPlayerbotAIConfig->spellDistance)
        return false;
    return true;
}

bool CastStealthAction::isPossible()
{
    // (Stealth, prio group 5): "NOT carrying a PvP objective".
    // do not use with WSG flag or EYE flag
    return !botAI->HasAura(23333, bot) && !botAI->HasAura(23335, bot) && !botAI->HasAura(34976, bot) &&
           !AI_VALUE(bool, "carrying objective");
}

bool UnstealthAction::Execute(Event event)
{
    botAI->RemoveAura("stealth");
    // botAI->ChangeStrategy("+dps,-stealthed", BOT_STATE_COMBAT);

    return true;
}

bool CheckStealthAction::Execute(Event event)
{
    // #742 (AC Ai/Class/Rogue/Action/RogueActions.cpp:43-53): AC's "-combat,+stealthed" / "+combat,-stealthed". Since
    // #386 the combat strategy's name is "combat" (DpsRogueStrategy.h getName), so "-dps" removed nothing and a stealthed
    // combat-spec rogue kept its combat strategy next to "stealthed". "combat" is registered (RogueAiObjectContext.cpp).
    if (botAI->HasAura("stealth", bot))
    {
        botAI->ChangeStrategy("-combat,+stealthed", BOT_STATE_COMBAT);
    }
    else
    {
        botAI->ChangeStrategy("+combat,-stealthed", BOT_STATE_COMBAT);
    }

    return true;
}

bool CastVanishAction::isUseful()
{
    // do not use with WSG flag or EYE flag; MoP rule: no stealth-class spell while carrying a PvP objective.
    return !botAI->HasAura(23333, bot) && !botAI->HasAura(23335, bot) && !botAI->HasAura(34976, bot) &&
           !AI_VALUE(bool, "carrying objective");
}

// PvP stun guard (kidney shot): only stun a target that is not already CC'd, BG/arena only.
bool CastKidneyShotAction::isUseful()
{
    return CastMeleeSpellAction::isUseful() &&
           (!AI_VALUE(bool, "in pvp instance") || !AI_VALUE2(bool, "is cc", "current target"));
}

// Smoke Bomb at <= 20% health. BG/arena only so the open world stays AC.
bool CastSmokeBombAction::isUseful()
{
    return CastBuffSpellAction::isUseful() && AI_VALUE(bool, "in pvp instance");
}

// Kick any casting enemy (target or random valid enemy); BG/arena only.
bool CastKickOnEnemyCasterAction::isUseful()
{
    return CastSpellOnEnemyCasterAction::isUseful() && AI_VALUE(bool, "in pvp instance");
}

bool CastTricksOfTheTradeOnMainTankAction::isUseful()
{
    return CastSpellAction::isUseful() && AI_VALUE2(float, "distance", GetTargetName()) < 20.0f;
}
