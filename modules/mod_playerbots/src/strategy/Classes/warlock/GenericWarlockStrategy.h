/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_GENERICWARLOCKSTRATEGY_H
#define _PLAYERBOT_GENERICWARLOCKSTRATEGY_H

#include "CombatStrategy.h"
#include "RangedCombatStrategy.h"

class PlayerbotAI;

class GenericWarlockStrategy : public RangedCombatStrategy
{
public:
    GenericWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "warlock"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
};

class WarlockBoostStrategy : public Strategy
{
public:
    WarlockBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "boost"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCcStrategy : public Strategy
{
public:
    WarlockCcStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "cc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class AoeWarlockStrategy : public CombatStrategy
{
public:
    AoeWarlockStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "aoe"; }
};

// #408 (AC parity, AC Strategy/TankWarlockStrategy.h:14-21 + .cpp). Kept in this file rather than a
// new TankWarlockStrategy.h/.cpp pair so no CMake source list changes are needed.
class TankWarlockStrategy : public GenericWarlockStrategy
{
public:
    TankWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "tank"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
};

// -- #408 (AC parity, AC Strategy/GenericWarlockStrategy.h:61-113). One opt-in strategy per curse
// so a bot can switch curses with "co +curse of doom" / "co -curse of doom" etc. Plain Strategy
// (no ActionNode factory) - matching this module's other opt-in strategies such as WarlockCcStrategy.
class WarlockCurseOfAgonyStrategy : public Strategy
{
public:
    WarlockCurseOfAgonyStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of agony"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfTheElementsStrategy : public Strategy
{
public:
    WarlockCurseOfTheElementsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of elements"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfDoomStrategy : public Strategy
{
public:
    WarlockCurseOfDoomStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of doom"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfExhaustionStrategy : public Strategy
{
public:
    WarlockCurseOfExhaustionStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of exhaustion"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfTonguesStrategy : public Strategy
{
public:
    WarlockCurseOfTonguesStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of tongues"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class WarlockCurseOfWeaknessStrategy : public Strategy
{
public:
    WarlockCurseOfWeaknessStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "curse of weakness"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
