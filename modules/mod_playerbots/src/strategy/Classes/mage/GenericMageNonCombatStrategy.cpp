/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericMageNonCombatStrategy.h"

#include "Playerbots.h"

class GenericMageNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericMageNonCombatStrategyActionNodeFactory()
    {
        creators["molten armor"] = &molten_armor;
        creators["mage armor"] = &mage_armor;
        creators["frost armor"] = &frost_armor;
    }

private:
    static ActionNode* molten_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("molten armor",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* mage_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mage armor",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* frost_armor([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost armor",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

GenericMageNonCombatStrategy::GenericMageNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericMageNonCombatStrategyActionNodeFactory());
}

void GenericMageNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("arcane brilliance", NextAction::array(0, new NextAction("arcane brilliance", 21.0f), nullptr)));
    // #385 (AC GenericMageNonCombatStrategy.cpp:41): no mana gem -> conjure mana gem (5.4.8: Conjure
    // Mana Gem 759, single gem item 36799; see MageTriggers.cpp NoManaGemTrigger).
    triggers.push_back(new TriggerNode("no mana gem", NextAction::array(0, new NextAction("conjure mana gem", 20.0f), nullptr)));
    // #434 (AC GenericMageNonCombatStrategy.cpp:40): wizard oil on the main-hand weapon, 1:1.
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
}

void MageBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mage armor", NextAction::array(0, new NextAction("mage armor", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("frost armor", NextAction::array(0, new NextAction("frost armor", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("molten armor", NextAction::array(0, new NextAction("molten armor", 19.0f), nullptr)));
}

void MageBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("mage armor", NextAction::array(0, new NextAction("mage armor", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("frost armor", NextAction::array(0, new NextAction("frost armor", 19.0f), nullptr)));
    triggers.push_back(new TriggerNode("molten armor", NextAction::array(0, new NextAction("molten armor", 19.0f), nullptr)));
}

void MageBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // 5.4.8: MoP has no "Arcane Intellect" separate from the raid buff; both the trigger and action are
    // "arcane brilliance on party" (see MageTriggers.h / MageActions.h / MageAiObjectContext.cpp).
    triggers.push_back(new TriggerNode("arcane brilliance on party", NextAction::array(0, new NextAction("arcane brilliance on party", 20.0f), nullptr)));
}