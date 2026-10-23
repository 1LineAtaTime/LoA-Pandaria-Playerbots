/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealDruidStrategy.h"

#include "Playerbots.h"

class HealDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealDruidStrategyActionNodeFactory() {
        creators["nourish on party"] = &nourtish_on_party;
        // creators["wild growth on party"] = &wild_growth_on_party;
        // creators["rejuvenation on party"] = &rejuvenation_on_party;
        // creators["regrowth on party"] = &regrowth_on_party;
    }

private:
    static ActionNode* nourtish_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("nourish on party",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("healing touch on party"), nullptr),
                              /*C*/ nullptr);
    }
    // static ActionNode* wild_growth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("wild growth on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
    // static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("rejuvenation on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
    // static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    // {
    //     return new ActionNode("regrowth on party",
    //                           /*P*/ NextAction::array(0, new NextAction("tree form"), nullptr),
    //                           /*A*/ nullptr,
    //                           /*C*/ nullptr);
    // }
};

HealDruidStrategy::HealDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new HealDruidStrategyActionNodeFactory());
}

void HealDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    // triggers.push_back(
    //     new TriggerNode("tree form", NextAction::array(0, new NextAction("tree form", ACTION_HIGH + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 9), nullptr)));

    // Rotation lane 5 (#860), MoP rotation row 10505 (spec 105, prio group 1/13): Ironbark (102342) on the tank
    // at health <= 40%. Group 1 rank 13 sits just below Tranquility (16) and above Wild Growth (12) / Swiftmend (11).
    triggers.push_back(new TriggerNode("ironbark",
                                       NextAction::array(0, new NextAction("ironbark on main tank", ACTION_CRITICAL_HEAL + 4.5f), nullptr)));

    // CRITICAL
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0,
                                          new NextAction("tree form", ACTION_CRITICAL_HEAL + 4.1f),
                                          new NextAction("swiftmend on party", ACTION_CRITICAL_HEAL + 4),
                                          new NextAction("regrowth on party", ACTION_CRITICAL_HEAL + 3),
                                          new NextAction("wild growth on party", ACTION_CRITICAL_HEAL + 2),
                                          new NextAction("nourish on party", ACTION_CRITICAL_HEAL + 1),
                                          // #384 (AC RestoDruidStrategy.cpp:34): Healing Touch (5185, level 26) is
                                          // still a 5.4.8 resto spell; it ends AC's critical chain at +0.
                                          new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 0),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("nature's swiftness", ACTION_CRITICAL_HEAL + 4), nullptr)));

    // #363 (AC parity, Ai/Class/Druid/Strategy/RestoDruidStrategy.cpp:45-46). AC refreshes the
    // tank's Lifebloom whenever Clearcasting is up, at a flat 13.0.
    triggers.push_back(new TriggerNode("clearcasting",
                                       NextAction::array(0, new NextAction("lifebloom on main tank", 13.0f), nullptr)));

    // #384 (AC RestoDruidStrategy.cpp:41-43): with Nature's Swiftness up (132158, spec102/105 level 30 in 5.4.8) AC
    // spends it on a Healing Touch at a flat 55.0.
    triggers.push_back(new TriggerNode("nature's swiftness active",
                                       NextAction::array(0, new NextAction("healing touch on party", 55.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "group heal setting",
        NextAction::array(0, 
                          new NextAction("tree form", ACTION_MEDIUM_HEAL + 2.3f),
                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 2.2f),
                          new NextAction("rejuvenation on not full", ACTION_MEDIUM_HEAL + 2.1f),
                          nullptr)));

    triggers.push_back(
        new TriggerNode("medium group heal setting",
                        NextAction::array(0, 
                        new NextAction("tree form", ACTION_CRITICAL_HEAL + 0.6f),
                        new NextAction("tranquility", ACTION_CRITICAL_HEAL + 0.5f), nullptr)));

    // LOW
    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("tree form", ACTION_MEDIUM_HEAL + 1.5f),
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 1.4f),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1.3f),
                                          new NextAction("swiftmend on party", ACTION_MEDIUM_HEAL + 1.2),
                                          new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 1.1f),
                                          // #384 (AC RestoDruidStrategy.cpp:57): Healing Touch ends the low chain.
                                          new NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 1.0f),
                                          nullptr)));

    // MEDIUM
    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, 
                                          new NextAction("tree form", ACTION_MEDIUM_HEAL + 0.5f),
                                          // #384 (AC RestoDruidStrategy.cpp:64): Swiftmend (18562, spec105 level 10)
                                          // sits between Tree Form and Wild Growth in AC's medium chain (20.4).
                                          new NextAction("swiftmend on party", ACTION_MEDIUM_HEAL + 0.45f),
                                          new NextAction("wild growth on party", ACTION_MEDIUM_HEAL + 0.4f),
                                          new NextAction("rejuvenation on party", ACTION_MEDIUM_HEAL + 0.3f),
                                          new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 0.2f),
                                          new NextAction("nourish on party", ACTION_MEDIUM_HEAL + 0.1f),
                                          // #384 (AC RestoDruidStrategy.cpp:69): Healing Touch ends the medium chain.
                                          new NextAction("healing touch on party", ACTION_MEDIUM_HEAL + 0.0f),
                                          nullptr)));

    // #384 (AC RestoDruidStrategy.cpp:19-20 "no healer dps strategy -> tree form" 5.0) is NOT ported: Tree of Life is
    // the 3-minute, 30-second Incarnation cooldown in 5.4.8 (Wowhead mop-classic 33891/106731, 2026-10-04), not a
    // form to keep up, so a low-priority "keep the form" edge would burn it at the start of every fight. The "tree
    // form" lead-ins on the heal edges above (now mapped to Incarnation) are kept.

    // almost full
    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("wild growth on party", ACTION_LIGHT_HEAL + 0.3f),
                                          new NextAction("rejuvenation on party", ACTION_LIGHT_HEAL + 0.2f),
                                          new NextAction("regrowth on party", ACTION_LIGHT_HEAL + 0.1f), nullptr)));

    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("innervate", ACTION_HIGH + 5), nullptr)));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));
}

// #363 (AC parity, Ai/Class/Druid/Strategy/RestoDruidStrategy.cpp:94-103). AC's blanket node pairs
// the blanket HoT with "tree form" as a prerequisite, because Wild Growth and Rejuvenation can
// only be cast in Tree of Life.
void DruidBlanketStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "wild growth blanket",
        NextAction::array(0, new NextAction("tree form", 8.1f), new NextAction("wild growth blanket", 8.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "rejuvenation blanket",
        NextAction::array(0, new NextAction("tree form", 6.1f), new NextAction("rejuvenation blanket", 6.0f), nullptr)));
}

// #408 (AC parity, Ai/Class/Druid/Strategy/RestoDruidStrategy.cpp:88-92). AC keeps the Tranquility
// burst in its own opt-in strategy. Node priority matches AC's flat 30.6/30.5.
void DruidTranquilityStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        NextAction::array(0, new NextAction("tree form", 30.6f), new NextAction("tranquility", 30.5f), nullptr)));
}
