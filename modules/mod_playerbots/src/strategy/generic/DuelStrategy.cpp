#include "DuelStrategy.h"

#include "Playerbots.h"

void DuelStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    PassTroughStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("duel requested", NextAction::array(0, new NextAction("accept duel", relevance), nullptr)));
    triggers.push_back(new TriggerNode(
        "no attackers", NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));
    // #868 (beyond AC): re-target the duel opponent whenever the bot lost its target mid-duel (any class).
    triggers.push_back(new TriggerNode(
        "duel opponent untargeted", NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));
}
