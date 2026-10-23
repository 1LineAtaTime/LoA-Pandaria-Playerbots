/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/TravelStrategy.h)
 * into the Legends of Azeroth Pandaria fork, issue #269.
 */

#ifndef _PLAYERBOT_TRAVELSTRATEGY_H
#define _PLAYERBOT_TRAVELSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class TravelStrategy : public Strategy
{
public:
    TravelStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "travel"; }

    // 5.4.8 divergence: AC (TravelStrategy.h:20-21) returns std::vector<NextAction>; the fork's
    // Strategy uses the old NextAction** API (strategy/Strategy.h), see DuelStrategy.cpp.
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class ExploreStrategy : public Strategy
{
public:
    ExploreStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "explore"; }
};

class MapStrategy : public Strategy
{
public:
    MapStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map"; }
};

class MapFullStrategy : public Strategy
{
public:
    MapFullStrategy(PlayerbotAI* botAI) : Strategy(botAI){};

    std::string const getName() override { return "map full"; }
};

#endif
