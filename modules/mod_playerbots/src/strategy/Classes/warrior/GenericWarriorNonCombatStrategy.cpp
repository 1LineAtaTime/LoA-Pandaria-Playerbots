#include "GenericWarriorNonCombatStrategy.h"

#include "Playerbots.h"

class GenericWarriorNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericWarriorNonCombatStrategyActionNodeFactory()
    {
    }

private:
};

GenericWarriorNonCombatStrategy::GenericWarriorNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericWarriorNonCombatStrategyActionNodeFactory());
}

void GenericWarriorNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #522 (AC GenericWarriorNonCombatStrategy.cpp:36-39). "apply stone" exists since #434.
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", 1.0f), nullptr)));
    // Vigilance 114030 (5.4.8 talent, Wowhead tooltip 2026-10-04); target pick/trigger fixed for #196.
    triggers.push_back(new TriggerNode("vigilance", NextAction::array(0, new NextAction("vigilance", 10.0f), nullptr)));
    // AC's berserker rage node (P: "berserker stance", AC :20) is dropped: Berserker Rage needs no stance on 5.4.8.
    triggers.push_back(new TriggerNode("fear sleep sap", NextAction::array(0, new NextAction("berserker rage", ACTION_EMERGENCY + 1), nullptr)));
}
