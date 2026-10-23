/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WAITFORATTACKSTRATEGY_H
#define _PLAYERBOT_WAITFORATTACKSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

// AC parity (issue #141 item 5, AC Ai/Base/Strategy/WaitForAttackStrategy.{h,cpp}): lets a real
// player master hold a grouped bot's opening attack for a few seconds after it enters combat
// instead of pulling immediately. Reachable the same way any other named strategy is - through
// ChangeStrategyAction's generic "co +wait for attack" chat toggle - so it has zero effect on any
// bot that never has it turned on (that is every bot today; nothing in this fork or AC itself adds
// it by default). ShouldWait() gates the Attack() call in strategy/actions/AttackActions.cpp (AC
// AttackAction.cpp:194); the combat-clock bookkeeping lives in PlayerbotAI::ChangeEngine's
// BOT_STATE_COMBAT / BOT_STATE_NON_COMBAT cases (AC ChangeEngineOnCombat/OnNonCombat,
// PlayerbotAI.cpp:1474-1496).
//
// Not ported: AC's safe-distance retreat sub-feature (the "wait for attack safe distance" trigger
// and WaitForAttackKeepSafeDistanceAction). That is a separate mechanism from the cited gate and
// bookkeeping and would need its own Trigger + Action port.
class WaitForAttackStrategy : public Strategy
{
public:
    WaitForAttackStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "wait for attack"; }

    static bool ShouldWait(PlayerbotAI* botAI);
    static uint8 GetWaitTime(PlayerbotAI* botAI);
};

#endif
