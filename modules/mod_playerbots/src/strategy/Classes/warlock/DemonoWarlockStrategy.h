#ifndef _PLAYERBOT_DEMONOWARLOCKSTRATEGY_H
#define _PLAYERBOT_DEMONOWARLOCKSTRATEGY_H

#include "GenericWarlockStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

class DemonoWarlockStrategy : public GenericWarlockStrategy
{
public:
    DemonoWarlockStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "demono"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    NextAction** getDefaultActions() override;
    uint32 GetType() const override { return GenericWarlockStrategy::GetType() | STRATEGY_TYPE_DPS; }
};

// #521 (AC WarlockAiObjectContext.cpp:54-55): AC's spec key "demo" as an alias of "demono".
class DemoAcKeyWarlockStrategy : public DemonoWarlockStrategy
{
public:
    DemoAcKeyWarlockStrategy(PlayerbotAI* botAI) : DemonoWarlockStrategy(botAI) {}
    std::string const getName() override { return "demo"; }
};

// #408 (AC parity, AC Strategy/DemonologyWarlockStrategy.h:25-32 + .cpp:133-146). Opt-in
// ("co +meta melee") - run to melee so Immolation Aura ticks, same as AC.
class MetaMeleeAoeStrategy : public CombatStrategy
{
public:
    MetaMeleeAoeStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "meta melee"; }
};

#endif
