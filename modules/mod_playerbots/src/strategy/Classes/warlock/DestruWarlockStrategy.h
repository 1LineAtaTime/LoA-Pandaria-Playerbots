#ifndef _PLAYERBOT_DESTRUWARLOCKSTRATEGY_H
#define _PLAYERBOT_DESTRUWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class DestruWarlockStrategy : public GenericWarlockStrategy
{
public:
    DestruWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "destru"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return GenericWarlockStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

// #521 (AC WarlockAiObjectContext.cpp:54-55): AC's spec key "destro" as an alias of "destru".
class DestroAcKeyWarlockStrategy : public DestruWarlockStrategy
{
public:
    DestroAcKeyWarlockStrategy(PlayerbotAI* botAI) : DestruWarlockStrategy(botAI) {}
    std::string const getName() override { return "destro"; }
};

#endif
