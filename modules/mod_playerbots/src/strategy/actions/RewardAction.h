/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * #258 lane Q: AC Ai/Base/Actions/RewardAction.h. Chat command "r <item link>" (AC ChatCommandHandlerStrategy.cpp:48,
 * ChatActionContext.h:132): picks the quest reward item the master named when a quest offers a choice.
 */

#ifndef _PLAYERBOT_REWARDACTION_H
#define _PLAYERBOT_REWARDACTION_H

#include "InventoryAction.h"

class PlayerbotAI;
class Object;

class RewardAction : public InventoryAction
{
public:
    RewardAction(PlayerbotAI* botAI) : InventoryAction(botAI, "reward") {}

    bool Execute(Event event) override;

private:
    bool Reward(uint32 itemId, Object* pNpc);
};

#endif
