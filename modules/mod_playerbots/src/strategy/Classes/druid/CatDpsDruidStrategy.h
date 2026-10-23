/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_CATDPSDRUIDSTRATEGY_H
#define _PLAYERBOT_CATDPSDRUIDSTRATEGY_H

#include "FeralDruidStrategy.h"

class PlayerbotAI;

class CatDpsDruidStrategy : public FeralDruidStrategy
{
public:
    CatDpsDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat"; }
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_MELEE; }
};

class CatAoeDruidStrategy : public CombatStrategy
{
public:
    CatAoeDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cat aoe"; }
};

// #408 (AC parity, Ai/Class/Druid/Strategy/CatDruidStrategy.h:26-36). Optional additive strategy
// that layers emergency heals on top of "cat", so the bot stays in cat form for DPS but shifts out
// to heal when the party needs it.
// Enable: co +offheal      Disable: co -offheal
class CatOffhealStrategy : public CombatStrategy
{
public:
    // #408: defined in the .cpp, which registers CatOffhealStrategyActionNodeFactory.
    CatOffhealStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "offheal"; }
};

#endif
