#ifndef _PLAYERBOT_DUELSTRATEGY_H
#define _PLAYERBOT_DUELSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

// AC parity (Ai/Base/Strategy/DuelStrategy.cpp): accept a duel request, attack the duel opponent
// once accepted.
class DuelStrategy : public PassTroughStrategy
{
public:
    DuelStrategy(PlayerbotAI* botAI) : PassTroughStrategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "duel"; }
};

// #410, AC Ai/Base/Strategy/DuelStrategy.h:23-29 / .cpp:21-23: "start duel" is an EMPTY strategy in AC (no
// triggers). It is only a marker that RpgDuelTrigger::IsActive checks (AC RpgTriggers.cpp:351), and that
// trigger belongs to AC's old "rpg" strategy, which AC does not run when EnableNewRpgStrategy=1 (default).
// #201/#272 (2026-10-05, owner rule "1:1 incl. dormant code"): RpgDuelTrigger (strategy/triggers/RpgTriggers.*)
// and RpgDuelAction (strategy/RPG/RpgSubActions.*) are now ported and registered as "rpg duel" exactly as AC
// registers them (AC TriggerContext.h:239, ActionContext.h:266). As in AC, no TriggerNode anywhere names
// "rpg duel" (AC RpgStrategy::InitTriggers does not list it), so neither runs on either core.
class StartDuelStrategy : public Strategy
{
public:
    StartDuelStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers([[maybe_unused]] std::vector<TriggerNode*>& triggers) override {}
    std::string const getName() override { return "start duel"; }
};

#endif
