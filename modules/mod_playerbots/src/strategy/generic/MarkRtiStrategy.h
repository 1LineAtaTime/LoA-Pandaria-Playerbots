#ifndef _PLAYERBOT_MARKRTISTRATEGY_H
#define _PLAYERBOT_MARKRTISTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/MarkRtiStrategy.{h,cpp}): the "mark rti" action already existed on
// this fork (ActionContext.h/RtiAction.h) but had no strategy scheduling it in combat.
class MarkRtiStrategy : public Strategy
{
public:
    MarkRtiStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mark rti"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
