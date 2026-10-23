/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/LootRollAction.{h,cpp} for issue #188 (S6 of epic
 * #163). Same shape as AC: Execute() votes on every pending roll from Group::GetRolls() (the
 * accessor AC's core has, added to this core's Group.h). See LootRollAction.cpp.
 */

#ifndef PLAYERBOTS_LOOTROLLACTION_H
#define PLAYERBOTS_LOOTROLLACTION_H

#include "Action.h"
#include "ItemVisitors.h"

class PlayerbotAI;

struct ItemTemplate;

#include "Group.h"  // RollVote (5.4.8 declares it without an underlying type)

class LootRollAction : public Action
{
public:
    LootRollAction(PlayerbotAI* botAI, std::string const name = "loot roll") : Action(botAI, name) {}

    bool Execute(Event event) override;

protected:
    RollVote CalculateRollVote(ItemTemplate const* proto, ItemUsage usage = ITEM_USAGE_NONE);
};

bool CanBotUseToken(ItemTemplate const* proto, Player* bot);
bool RollUniqueCheck(ItemTemplate const* proto, Player* bot);

class RollAction : public Action
{
public:
    RollAction(PlayerbotAI* botAI) : Action(botAI, "dice roll") {}

    bool Execute(Event event) override;
};

#endif
