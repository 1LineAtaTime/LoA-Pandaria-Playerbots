/*
 * AC parity (#408): ported from AC mod-playerbots Ai/Class/Warrior/Strategy/WarriorPullStrategy.h.
 *
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#ifndef _PLAYERBOT_WARRIORPULLSTRATEGY_H
#define _PLAYERBOT_WARRIORPULLSTRATEGY_H

#include "PullStrategy.h"

class WarriorPullStrategy : public PullStrategy
{
public:
    WarriorPullStrategy(PlayerbotAI* botAI) : PullStrategy(botAI, "shoot") {}

    std::string GetPullActionName() const override;
};

#endif
