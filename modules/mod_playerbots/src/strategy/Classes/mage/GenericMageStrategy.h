/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICMAGESTRATEGY_H
#define _PLAYERBOT_GENERICMAGESTRATEGY_H

#include "CombatStrategy.h"
#include "RangedCombatStrategy.h"

class PlayerbotAI;

class GenericMageStrategy : public RangedCombatStrategy
{
public:
    GenericMageStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "mage"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    uint32 GetType() const override
    {
        return RangedCombatStrategy::GetType() | STRATEGY_TYPE_RANGED | STRATEGY_TYPE_DPS;
    }
};

// #408 (AC parity, GenericMageStrategy.h:24-52). AC's MageCureStrategy / MageBoostStrategy /
// MageCcStrategy / MageAoeStrategy were never ported to this fork, which is why the matching
// creators stayed commented out in MageAiObjectContext.cpp. Bodies live in GenericMageStrategy.cpp.
class MageCureStrategy : public Strategy
{
public:
    MageCureStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cure"; }
};

class MageBoostStrategy : public Strategy
{
public:
    MageBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "boost"; }
};

class MageCcStrategy : public Strategy
{
public:
    MageCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cc"; }
};

class MageAoeStrategy : public CombatStrategy
{
public:
    MageAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

#endif
