/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_HEALDRUIDSTRATEGY_H
#define _PLAYERBOT_HEALDRUIDSTRATEGY_H

#include "GenericDruidStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class HealDruidStrategy : public GenericDruidStrategy
{
public:
    HealDruidStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    // #593 (AC Ai/Class/*/*AiObjectContext.cpp): stored under AC's spec key so the co list matches AC; the fork key "heal" stays a registered alias.
    std::string const getName() override { return "resto"; }
    uint32 GetType() const override { return STRATEGY_TYPE_RANGED | STRATEGY_TYPE_HEAL; }
};

// #363 (AC parity, Ai/Class/Druid/Strategy/RestoDruidStrategy.h:25-32). AC keeps the blanket
// rotation in its own opt-in strategy rather than folding it into the resto strategy, so the
// same shape is used here: it only runs when a bot's strategy list contains "blanketing"
// (registered at DruidAiObjectContext.cpp, strategy "blanketing"). This fork's HealDruidStrategy
// is the counterpart of AC's RestoDruidStrategy.
class DruidBlanketStrategy : public Strategy
{
public:
    DruidBlanketStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "blanketing"; }
};

// #408 (AC parity, Ai/Class/Druid/Strategy/RestoDruidStrategy.h:34-41). AC keeps the Tranquility
// burst in its own opt-in strategy instead of folding it into the resto strategy. This fork's
// HealDruidStrategy is the counterpart of AC's RestoDruidStrategy and already carries its own
// "medium group heal setting" node, so this is the AC-shaped second entry point.
class DruidTranquilityStrategy : public Strategy
{
public:
    DruidTranquilityStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tranquility"; }
};

#endif
