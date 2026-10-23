#ifndef _PLAYERBOT_USEFOODSTRATEGY_H
#define _PLAYERBOT_USEFOODSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/UseFoodStrategy.{h,cpp}): eat/drink when low, out of combat. AC also
// has a "food cheat" branch (eat/drink proactively at medium health/high mana instead) gated by
// botAI->HasCheat(BotCheatMask::food); this fork has no bot-cheat-mask system (PlayerbotAI.h:319 is
// commented out), so only AC's non-cheat branch is ported.
class UseFoodStrategy : public Strategy
{
public:
    UseFoodStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "food"; }
};

#endif
