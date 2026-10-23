/*
 * AC parity (#408): ported from AC mod-playerbots Ai/Class/Druid/Strategy/DruidPullStrategy.cpp.
 *
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "DruidPullStrategy.h"

#include "AiObjectContext.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

namespace
{
// #408: AC's PlayerbotAI::HasSpell(std::string const) is not ported on this fork. This is AC's own
// implementation, kept file-local so PlayerbotAI.h does not have to change.
bool HasKnownSpell(PlayerbotAI* botAI, std::string const spellName)
{
    uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", spellName)->Get();
    return spellId && botAI->GetBot()->HasSpell(spellId);
}
} // namespace

std::string DruidPullStrategy::GetPullActionName() const
{
    std::string const pullActionName = PullStrategy::GetPullActionName();
    std::string const actionName =
        HasKnownSpell(botAI, "faerie fire (feral)") &&
            (botAI->HasStrategy("bear", BOT_STATE_COMBAT) || botAI->HasStrategy("cat", BOT_STATE_COMBAT))
        ? "faerie fire (feral)"
        : pullActionName;

    Unit* target = GetTarget();
    if (!target)
        return actionName;

    if (!botAI->CanCastSpell(actionName, target) && botAI->CanCastSpell("growl", target))
        return "growl";

    return actionName;
}

std::string DruidPullStrategy::GetPreActionName() const
{
    if (GetPullActionName() == "faerie fire")
        return "";

    return PullStrategy::GetPreActionName();
}
