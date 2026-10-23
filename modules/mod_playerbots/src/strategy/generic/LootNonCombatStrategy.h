/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Strategy/LootNonCombatStrategy.h (issue #151). Replaces
 * strategy/generic/NonCombatStrategy.h's old "LootStrategy" class, which name-clashed with AC's
 * item-loot interface class also named LootStrategy (see strategy/value/LootObjectStack.h). GatherStrategy lives in
 * NonCombatStrategy.h/.cpp; since #156 it queues herb/mining nodes on this loot stack ("timer" ->
 * "add gathering loot" 5.0, AC LootNonCombatStrategy.cpp:19-23), as AC does.
 */

#ifndef _PLAYERBOT_LOOTNONCOMBATSTRATEGY_H
#define _PLAYERBOT_LOOTNONCOMBATSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class LootNonCombatStrategy : public Strategy
{
public:
    LootNonCombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "loot"; }
};

// AC LootNonCombatStrategy.h:25-31 (issue #377): "often" -> "reveal gathering item".
class RevealStrategy : public Strategy
{
public:
    RevealStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "reveal"; }
};

// AC LootNonCombatStrategy.h:42-47 (issue #412): fires "use fishing bobber" once the bobber is ready.
class UseBobberStrategy : public Strategy
{
public:
    UseBobberStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "use bobber"; }
};

// AC NonCombatStrategy.h:62-68 / NonCombatStrategy.cpp:37-67 (issue #412). Declared here, not in
// NonCombatStrategy.h, because that header is included by ~34 TUs; StrategyContext.h already includes this one.
class MasterFishingStrategy : public Strategy
{
public:
    MasterFishingStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "master fishing"; }
};

#endif
