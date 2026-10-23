/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_FIREMAGESTRATEGY_H
#define _PLAYERBOT_FIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FireMageStrategy : public GenericMageStrategy
{
public:
    FireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire"; }
    NextAction** getDefaultActions() override;
};

class FireMageAoeStrategy : public CombatStrategy
{
public:
    FireMageAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire aoe"; }
};

// #385 (AC Ai/Class/Mage/Strategy/FireMageStrategy.h:24-31): "firestarter", off by default ("co +firestarter").
class FirestarterStrategy : public CombatStrategy
{
public:
    FirestarterStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "firestarter"; }
};

#endif
