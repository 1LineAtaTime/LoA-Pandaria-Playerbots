/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICDRUIDSTRATEGY_H
#define _PLAYERBOT_GENERICDRUIDSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericDruidStrategy : public CombatStrategy
{
protected:
    GenericDruidStrategy(PlayerbotAI* botAI);

public:
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class DruidCureStrategy : public Strategy
{
public:
    DruidCureStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cure"; }
};

class DruidBoostStrategy : public Strategy
{
public:
    DruidBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "boost"; }
};

class DruidCcStrategy : public Strategy
{
public:
    DruidCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cc"; }
};

class DruidHealerDpsStrategy : public Strategy
{
public:
    DruidHealerDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healer dps"; }
};

// #408 (AC parity, Ai/Class/Druid/Strategy/GenericDruidStrategy.h:59-66). AC exposes one spec-gated
// "aoe" strategy for balance/resto/cat. This fork keeps its own "caster aoe"/"cat aoe", so this is
// the AC-shaped third entry point rather than a replacement for them.
class DruidAoeStrategy : public Strategy
{
public:
    DruidAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

#endif
