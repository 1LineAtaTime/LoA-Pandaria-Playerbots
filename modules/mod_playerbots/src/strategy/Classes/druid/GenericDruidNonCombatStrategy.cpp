/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericDruidNonCombatStrategy.h"

#include "AiFactory.h"
#include "Playerbots.h"

class GenericDruidNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidNonCombatStrategyActionNodeFactory()
    {
        creators["thorns"] = &thorns;
        creators["thorns on party"] = &thorns_on_party;
        creators["mark of the wild"] = &mark_of_the_wild;
        creators["mark of the wild on party"] = &mark_of_the_wild_on_party;
        // creators["innervate"] = &innervate;
        creators["regrowth_on_party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
        creators["remove curse on party"] = &remove_curse_on_party;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["revive"] = &revive;
        // #384 (AC GenericDruidNonCombatStrategy.cpp:25,96-102).
        creators["aquatic form"] = &aquatic_form;
    }

private:
    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* thorns_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* mark_of_the_wild([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mark of the wild",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* mark_of_the_wild_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mark of the wild on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    static ActionNode* regrowth_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* rejuvenation_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* remove_curse_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("remove curse on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* abolish_poison_on_party(PlayerbotAI* ai)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    static ActionNode* revive(PlayerbotAI* ai)
    {
        return new ActionNode("revive",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NULL,
                              /*C*/ NULL);
    }
    // #384 (AC GenericDruidNonCombatStrategy.cpp:96-102): shift out of any other form before Aquatic Form.
    static ActionNode* aquatic_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aquatic form",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
    // static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode ("innervate",
    //         /*P*/ nullptr,
    //         /*A*/ NextAction::array(0, new NextAction("drink"), nullptr),
    //         /*C*/ nullptr);
    // }
};

GenericDruidNonCombatStrategy::GenericDruidNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, GenericDruidNonCombatStrategy.cpp:151): apply the configured stance
    // to a freshly summoned treant.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("mark of the wild", NextAction::array(0, new NextAction("mark of the wild", 14.0f), nullptr)));
    // triggers.push_back(new TriggerNode("thorns", NextAction::array(0, new NextAction("thorns", 12.0f), nullptr)));
    // triggers.push_back(new TriggerNode("cure poison", NextAction::array(0, new NextAction("abolish poison", 21.0f),
    // nullptr)));
    triggers.push_back(new TriggerNode(
        "party member cure poison", NextAction::array(0, new NextAction("abolish poison on party", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member dead", NextAction::array(0, new NextAction("revive", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("innervate", ACTION_EMERGENCY
    // + 5), nullptr))); triggers.push_back(new TriggerNode("swimming", NextAction::array(0, new NextAction("aquatic
    // form", 1.0f), nullptr)));
    // #434 (AC GenericDruidNonCombatStrategy.cpp:118): wizard/mana oils still exist in 5.4.8 (capped by
    // weapon item level), so this edge is ported 1:1 (ImbueAction.cpp).
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0,
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 7),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 6),
                                          new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 5),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, 
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 5),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 4),
                                          new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 3),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 1),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("wild growth on party", ACTION_LIGHT_HEAL + 3), new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 2), NULL)));

    triggers.push_back(
        new TriggerNode("party member remove curse",
                        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL + 7), nullptr)));

    // #384 (AC GenericDruidNonCombatStrategy.cpp:176): Aquatic Form (1066, level 18) is still learnable in 5.4.8
    // (cls.py druid). The "aquatic form" trigger (AquaticFormTrigger) and action are already registered.
    triggers.push_back(new TriggerNode("aquatic form", NextAction::array(0, new NextAction("aquatic form", 10.0f), nullptr)));

    // #434 (AC GenericDruidNonCombatStrategy.cpp:178-182): spec-dependent weapon imbue. AC's tabs
    // Balance/Restoration -> oil, Feral -> stone; on 5.4.8 Guardian is its own spec and, like AC's
    // Feral tab (bear + cat), gets the stone. Evaluated at strategy init, as on AC.
    int specTab = int(AiFactory::GetPlayerSpecTab(botAI->GetBot()));
    if (specTab == SPEC_DRUID_BALANCE || specTab == SPEC_DRUID_RESTORATION)
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    if (specTab == SPEC_DRUID_FERAL || specTab == SPEC_DRUID_GUARDIAN)
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", 1.0f), nullptr)));
}

GenericDruidBuffStrategy::GenericDruidBuffStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidNonCombatStrategyActionNodeFactory());
}

void GenericDruidBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("mark of the wild on party",
                        NextAction::array(0, new NextAction("mark of the wild on party", 13.0f), nullptr)));
    // #384: AC's "thorns on main tank" buff edge (AC GenericDruidNonCombatStrategy.cpp:187-190) is dropped: Thorns is
    // NOT-LEARNABLE in 5.4.8 (cls.py druid: ids 15438/21335/..., no class skill-line row) and has no 5.x successor
    // (the 5.x druid has no thorns-type buff). The thorns actions/triggers stay registered but unused.
}
