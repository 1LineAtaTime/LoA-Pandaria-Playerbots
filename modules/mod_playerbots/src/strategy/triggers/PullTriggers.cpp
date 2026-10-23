/*
 * AC parity (#408): ported from AC mod-playerbots Ai/Base/Trigger/PullTriggers.cpp.
 *
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the CMaNGOS playerbots project (https://github.com/cmangos/playerbots), GPL v2,
 * with modifications for AzerothCore.
 * Original authors:
 *   David Parra Ausina (davidonete/Flekz) <davidparraausina@gmail.com> - original author
 *   Sebastiaan Keek (mostlikely4r) <sebastiaan.keek@gmail.com>
 *   Cyberium <cyberium@users.noreply.github.com>
 */

#include "PullTriggers.h"

#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "PullStrategy.h"

bool PullStartTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);
    return strategy && strategy->IsPullPendingToStart();
}

bool PullEndTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    if (!strategy || !strategy->HasPullStarted())
        return false;

    Unit* target = strategy->GetTarget();
    if (!target || !target->IsInWorld() || !target->IsAlive())
        return true;

    time_t const secondsSincePullStarted = time(nullptr) - strategy->GetPullStartTime();
    if (secondsSincePullStarted >= PullStrategy::GetMaxPullTime())
        return true;

    float distanceToPullTarget = bot->GetDistance(target);
    // #408: MoP spells this Unit::IsNonMeleeSpellCasted(); the three leading parameters map 1:1.
    // #408: AC's botAI->IsRanged(bot) is the static spec helper here on this fork.
    if (distanceToPullTarget > ATTACK_DISTANCE && !target->IsNonMeleeSpellCasted(false, false, true) &&
        (!PlayerBotSpec::IsRanged(bot) || distanceToPullTarget > botAI->GetRange("spell")))
        return false;

    if (!botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return true;

    PositionInfo pullPosition = context->GetValue<PositionMap&>("position")->Get()["pull"];
    if (!pullPosition.isSet() || pullPosition.mapId != bot->GetMapId())
        return true;

    return bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) <= botAI->GetRange("follow");
}

bool ReturnToPullPositionTrigger::IsActive()
{
    PullStrategy const* strategy = PullStrategy::Get(botAI);

    Unit* target = strategy ? strategy->GetTarget() : nullptr;
    if (!strategy || !strategy->HasPullStarted() || !target || !target->IsInCombat() ||
        !botAI->HasStrategy("pull back", BOT_STATE_COMBAT))
        return false;

    PositionInfo pullPosition = context->GetValue<PositionMap&>("position")->Get()["pull"];
    return pullPosition.isSet() && pullPosition.mapId == bot->GetMapId() &&
           bot->GetDistance(pullPosition.x, pullPosition.y, pullPosition.z) > sPlayerbotAIConfig->followDistance;
}
