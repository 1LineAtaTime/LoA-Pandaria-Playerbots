/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICSHAMANSTRATEGY_H
#define _PLAYERBOT_GENERICSHAMANSTRATEGY_H

#include "CombatStrategy.h"

class PlayerbotAI;

class GenericShamanStrategy : public CombatStrategy
{
public:
    GenericShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class ShamanBuffDpsStrategy : public Strategy
{
public:
    ShamanBuffDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bdps"; }
};

class ShamanBuffManaStrategy : public Strategy
{
public:
    ShamanBuffManaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bmana"; }
};

class ShamanCureStrategy : public Strategy
{
public:
    ShamanCureStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cure"; }
};

class ShamanHealerDpsStrategy : public Strategy
{
public:
    ShamanHealerDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healer dps"; }
};

// #408 (AC parity, AC GenericShamanStrategy.h:31-38): Boost strategy (heroism/bloodlust/fire elemental totem).
class ShamanBoostStrategy : public Strategy
{
public:
    ShamanBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "boost"; }
};

// #408 (AC parity, AC GenericShamanStrategy.h:40-47): AoE strategy (fire nova / chain lightning).
class ShamanAoeStrategy : public CombatStrategy
{
public:
    ShamanAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

#endif
