#ifndef _PLAYERBOT_CASTTIMESTRATEGY_H
#define _PLAYERBOT_CASTTIMESTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/CastTimeStrategy.cpp): de-prioritize a long cast against a target
// that will die from the group's other damage before the cast lands.
class CastTimeMultiplier : public Multiplier
{
public:
    CastTimeMultiplier(PlayerbotAI* botAI) : Multiplier(botAI, "cast time") {}

    float GetValue(Action* action) override;
};

class CastTimeStrategy : public Strategy
{
public:
    CastTimeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
    std::string const getName() override { return "cast time"; }
};

#endif
