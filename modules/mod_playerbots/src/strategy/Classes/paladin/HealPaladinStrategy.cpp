/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealPaladinStrategy.h"

#include "Playerbots.h"
#include "Strategy.h"

class HealPaladinStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    // HealPaladinStrategyActionNodeFactory()
    // {
    //     creators["concentration aura"] = &concentration_aura;
    // }

private:
    // ACTION_NODE_A(concentration_aura, "concentration aura", "devotion aura");
};

HealPaladinStrategy::HealPaladinStrategy(PlayerbotAI* botAI) : GenericPaladinStrategy(botAI)
{
    actionNodeFactories.Add(new HealPaladinStrategyActionNodeFactory());
}

NextAction** HealPaladinStrategy::getDefaultActions()
{
    // 5.4.8: Judgement of Light does not exist; Judgement is a single unified ability in MoP, so this
    // now casts the generic "judgement" action.
    return NextAction::array(0, new NextAction("judgement", ACTION_DEFAULT), nullptr);
}

void HealPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPaladinStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("seal", NextAction::array(0, new NextAction("seal of insight", ACTION_HIGH), nullptr)));
    // #383 (AC HealPaladinStrategy.cpp "medium mana" -> divine illumination): Divine Illumination was
    // removed in 4.0.1 (71166 NOT-LEARNABLE, cls.py 2026-10-04) and has no direct 5.x successor; the mana
    // tools Holy has (Divine Plea via "high mana"/"low mana" edges, Divine Favor below) already exist, so
    // the edge is removed instead of mapped. The action stays registered.
    // #383 (AC HealPaladinStrategy.cpp "seal" -> seal of wisdom): already mapped to Seal of Insight (20165,
    // the 5.x healer seal: "+5% healing, melee hits heal, Holy restores 6% base mana", Wowhead 2026-10-04).
    // rotation lane 6 (#861): Divine Plea at mana <= 30% (the "low mana" edge)
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("divine plea", ACTION_HIGH + 2),
                                                                      new NextAction("divine favor", ACTION_HIGH + 1), nullptr)));
    // rotation lane 6 (#861): Divine Favor, Guardian of Ancient Kings and Avenging Wrath when
    // >= 3 group members are below 50% (within 40 yd)
    triggers.push_back(new TriggerNode("paladin 3 hurt group", NextAction::array(0,
        new NextAction("guardian of ancient kings", ACTION_HIGH + 4),
        new NextAction("avenging wrath", ACTION_HIGH + 3),
        new NextAction("divine favor", ACTION_HIGH + 2), nullptr)));
    // rotation lane 6 (#861): Holy Radiance, 3+ members < 50% within 10 yd of the target. Above Flash of
    // Light / Divine Light / Holy Light (rows 6511-6513), below Holy Shock / Eternal Flame (rows 6507-6509).
    triggers.push_back(new TriggerNode("paladin holy radiance group",
        NextAction::array(0, new NextAction("holy radiance on party", ACTION_CRITICAL_HEAL + 4.8f), nullptr)));
    triggers.push_back(new TriggerNode("party member to heal out of spell range", NextAction::array(0, new NextAction("reach party member to heal", ACTION_EMERGENCY + 3), nullptr)));

    // rotation lane 6 (#861): "divine sacrifice" removed from both edges below - no such spell in 5.4.8
    // (forkonly_verdicts.txt: NO-SUCH-NAME); the action stays registered.
    triggers.push_back(
        new TriggerNode("medium group heal setting", NextAction::array(0,
            new NextAction("avenging wrath", ACTION_HIGH + 4),
            nullptr)));

    triggers.push_back(
        new TriggerNode("party member critical health", NextAction::array(0,
            new NextAction("holy shock on party", ACTION_CRITICAL_HEAL + 6),
            // Eternal Flame (3 Holy Power), Flash of Light (<= 30%), 6512 Divine Light
            new NextAction("eternal flame on party", ACTION_CRITICAL_HEAL + 5),
            new NextAction("flash of light on party", ACTION_CRITICAL_HEAL + 4.5f),
            new NextAction("divine light on party", ACTION_CRITICAL_HEAL + 4.2f),
            new NextAction("holy light on party", ACTION_CRITICAL_HEAL + 4), nullptr)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("eternal flame on party", ACTION_MEDIUM_HEAL + 6),
                                          new NextAction("divine light on party", ACTION_MEDIUM_HEAL + 5.5f),
                                          new NextAction("holy light on party", ACTION_MEDIUM_HEAL + 5), nullptr)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("execution sentence on party", ACTION_LIGHT_HEAL + 11),
                                          new NextAction("eternal flame on party", ACTION_LIGHT_HEAL + 10),
                                          new NextAction("divine light on party", ACTION_LIGHT_HEAL + 9.5f),
                                          new NextAction("holy light on party", ACTION_LIGHT_HEAL + 9),
                                          new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 8), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member almost full health",
        NextAction::array(0, new NextAction("eternal flame on party", ACTION_LIGHT_HEAL + 4),
                          new NextAction("flash of light on party", ACTION_LIGHT_HEAL + 3), nullptr)));

    triggers.push_back(new TriggerNode(
        "beacon of light on main tank",
        NextAction::array(0, new NextAction("beacon of light on main tank", ACTION_CRITICAL_HEAL + 7), nullptr)));

    triggers.push_back(new TriggerNode(
        "sacred shield on main tank",
        NextAction::array(0, new NextAction("sacred shield on main tank", ACTION_CRITICAL_HEAL + 6), nullptr)));
}
