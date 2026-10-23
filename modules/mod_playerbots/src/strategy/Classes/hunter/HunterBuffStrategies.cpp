/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterBuffStrategies.h"

#include "Playerbots.h"

class BuffHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BuffHunterStrategyActionNodeFactory() { creators["aspect of the hawk"] = &aspect_of_the_hawk; }

private:
    static ActionNode* aspect_of_the_hawk([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the hawk",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the monkey"), nullptr),
                              /*C*/ nullptr);
    }
};

HunterBuffDpsStrategy::HunterBuffDpsStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new BuffHunterStrategyActionNodeFactory());
}

void HunterBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("aspect of the hawk", NextAction::array(0, new NextAction("aspect of the hawk", 20.0f), nullptr)));
    // #521: AC's aspect of the dragonhawk edge (AC HunterBuffStrategies.cpp:45) is not ported: Aspect of the Dragonhawk
    // does not exist in 5.4.8 (cls.py hunter 2026-10-04: NO-SUCH-NAME); Aspect of the Hawk 13165 (L12) is its 5.x form.
}

void HunterNatureResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the wild",
                                       NextAction::array(0, new NextAction("aspect of the wild", 20.0f), nullptr)));
}

void HunterBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("aspect of the pack",
                                       NextAction::array(0, new NextAction("aspect of the pack", 20.0f), nullptr)));
}

void HunterBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #521: the "aspect of the viper" edge (bmana) is removed: Aspect of the Viper does not exist in the 5.4.8 DBC
    // (cls.py hunter 2026-10-04: NO-SUCH-NAME). Strategy name kept so existing `+bmana` commands still resolve.
}
