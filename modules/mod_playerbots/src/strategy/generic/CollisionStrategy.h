/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #271: AC CollisionStrategy (Ai/Base/Strategy/NonCombatStrategy.h:23-31, .cpp:15-19). Header-only here so that
 * NonCombatStrategy.h (included by every class strategy) is not touched; only StrategyContext.h includes it.
 * Dormant exactly like AC: AC registers "collision" (StrategyContext.h:92) but AiFactory.cpp:611,644 keep
 * `nonCombatEngine->addStrategy("collision")` commented out, so it only runs when added by hand ("nc +collision").
 */

#ifndef _PLAYERBOT_COLLISIONSTRATEGY_H
#define _PLAYERBOT_COLLISIONSTRATEGY_H

#include "Action.h"
#include "Strategy.h"
#include "Trigger.h"

class PlayerbotAI;

class CollisionStrategy : public Strategy
{
public:
    CollisionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "collision"; }

    // AC NonCombatStrategy.cpp:15-19
    void InitTriggers(std::vector<TriggerNode*>& triggers) override
    {
        triggers.push_back(
            new TriggerNode("collision", NextAction::array(0, new NextAction("move out of collision", 2.0f), nullptr)));
    }
};

#endif
