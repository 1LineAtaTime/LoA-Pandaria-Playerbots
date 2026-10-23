#ifndef _PLAYERBOT_FOCUSHEALTARGETSSTRATEGY_H
#define _PLAYERBOT_FOCUSHEALTARGETSSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/FocusTargetStrategy.h): pure marker strategy, toggled by
// SetFocusHealTargetsAction ("focus heal targets" chat command) via
// botAI->ChangeStrategy("+/-focus heal targets", BOT_STATE_COMBAT). PartyMemberToHealValue checks
// botAI->HasStrategy("focus heal targets", BOT_STATE_COMBAT) to restrict healing to the focus list.
class FocusHealTargetsStrategy : public Strategy
{
public:
    FocusHealTargetsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "focus heal targets"; }
};

#endif
