/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Strategy/RpgStrategy.h)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 */

#ifndef PLAYERBOTS_RPGSTRATEGY_H
#define PLAYERBOTS_RPGSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class RpgActionMultiplier : public Multiplier
{
public:
    RpgActionMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "rpg action") {}

    float GetValue(Action* action) override;
};

class RpgStrategy : public Strategy
{
public:
    RpgStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "rpg"; }
    // 5.4.8: old NextAction** API (strategy/Strategy.h:59), AC: std::vector<NextAction> getDefaultActions().
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
