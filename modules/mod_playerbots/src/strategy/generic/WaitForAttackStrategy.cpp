/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "WaitForAttackStrategy.h"

#include "Playerbots.h"

// AC parity (issue #141 item 5, AC Ai/Base/Strategy/WaitForAttackStrategy.cpp:29-58).
bool WaitForAttackStrategy::ShouldWait(PlayerbotAI* botAI)
{
    if (!botAI->HasStrategy("wait for attack", BOT_STATE_COMBAT))
        return false;

    Player* bot = botAI->GetBot();

    // This fork has no ported equivalent of AC's IsSelfBot()/HasGameClientMaster() (see
    // strategy/actions/LootAction.cpp's isUseful() note on the same gap). HasRealPlayerMaster() is
    // the closest existing check: true when the accepted master is not itself another bot AI.
    if (!bot->GetGroup() || !botAI->HasRealPlayerMaster())
        return false;

    // Don't wait if the current target is an enemy player.
    AiObjectContext* context = botAI->GetAiObjectContext();
    Unit* target = context->GetValue<Unit*>("current target")->Get();
    if (target && target->IsPlayer())
        return false;

    if (botAI->GetState() != BOT_STATE_COMBAT)
        return false;

    time_t combatStartTime = context->GetValue<time_t>("combat start time")->Get();
    if (combatStartTime == 0)
    {
        combatStartTime = time(nullptr);
        context->GetValue<time_t>("combat start time")->Set(combatStartTime);
    }

    return time(nullptr) - combatStartTime < GetWaitTime(botAI);
}

uint8 WaitForAttackStrategy::GetWaitTime(PlayerbotAI* botAI)
{
    return botAI->GetAiObjectContext()->GetValue<uint8>("wait for attack time")->Get();
}
