#ifndef _PLAYERBOT_USEPOTIONSSTRATEGY_H
#define _PLAYERBOT_USEPOTIONSSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/UsePotionsStrategy.cpp): healthstone/healing potion at critical
// health, mana potion at medium mana. The "healthstone" -> "healing potion" fallback and the
// "healing potion"/"mana potion" action nodes are already registered generically in
// Strategy.cpp's ActionNodeFactoryInternal, and "critical health"/"medium mana" are already
// generic triggers (TriggerContext.h), so this strategy only wires the two together.
class UsePotionsStrategy : public Strategy
{
public:
    UsePotionsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "potions"; };
};

#endif
