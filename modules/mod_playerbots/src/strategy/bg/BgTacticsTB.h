/*
 * #901 (beyond AC: AC has no battlefield bot play) - Tol Barad (map 732, battle id 21) tactics for bots in the battlefield
 * raid during the war. Self-contained: the "tolbarad" strategy has one trigger ("often") and one action ("tb play"), so no
 * new trigger class and no widely included header is touched. Mechanics and sources: BgTacticsTB.cpp.
 */
#ifndef PLAYERBOTS_BGTACTICSTB_H
#define PLAYERBOTS_BGTACTICSTB_H

#include "MovementActions.h"
#include "Strategy.h"

class Player;

namespace TolBaradBots
{
    /// map 732, Tol Barad war running, the bot is in the battlefield raid.
    bool InWar(Player* bot);
    /// World thread, from BattlefieldBots::Update (every random bot, every pass): adds the "tolbarad" strategy when the
    /// bot enters the war and rebuilds the strategy set (ResetStrategies) when it leaves.
    /// #900: any battlefield (Wintergrasp too), bot in its raid during the war.
    bool InAnyWar(Player* bot);
    void Sync(Player* bot);
}

class TolBaradStrategy : public Strategy
{
public:
    TolBaradStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "tolbarad"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class TolBaradPlayAction : public MovementAction
{
public:
    TolBaradPlayAction(PlayerbotAI* botAI, std::string const name = "tb play") : MovementAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
