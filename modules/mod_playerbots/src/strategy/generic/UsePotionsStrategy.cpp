#include "UsePotionsStrategy.h"

#include "Playerbots.h"

void UsePotionsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("healthstone", ACTION_MEDIUM_HEAL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "medium mana", NextAction::array(0, new NextAction("mana potion", ACTION_EMERGENCY), nullptr)));
}
