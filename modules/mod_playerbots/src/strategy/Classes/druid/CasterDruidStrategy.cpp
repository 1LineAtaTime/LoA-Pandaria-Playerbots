/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CasterDruidStrategy.h"

#include "AiObjectContext.h"
#include "FeralDruidStrategy.h"

class CasterDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CasterDruidStrategyActionNodeFactory()
    {
        creators["faerie fire"] = &faerie_fire;
        creators["hibernate"] = &hibernate;
        creators["entangling roots"] = &entangling_roots;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["wrath"] = &wrath;
        creators["starfall"] = &starfall;
        creators["insect swarm"] = &insect_swarm;
        creators["moonfire"] = &moonfire;
        creators["starfire"] = &starfire;
        creators["moonkin form"] = &moonkin_form;
        // #363 (AC parity, Ai/Class/Druid/Strategy/BalanceDruidStrategy.cpp:25,132-140): Typhoon
        // must be cast from Moonkin, so the node carries "moonkin form" as its prerequisite.
        // This fork's CasterDruidStrategy is the counterpart of AC's BalanceDruidStrategy.
        creators["typhoon"] = &typhoon;
        creators["starsurge"] = &starsurge;  // lane 5 (#860)
    }

private:
    static ActionNode* faerie_fire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* hibernate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("hibernate",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ NextAction::array(0, new NextAction("entangling roots"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* entangling_roots([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots on cc",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* wrath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("wrath",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* starfall([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("starfall",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* insect_swarm([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("insect swarm",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* moonfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("moonfire",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* starfire([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("starfire",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* moonkin_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("moonkin form",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    // Rotation lane 5 (#860): Starsurge is cast from Moonkin like the other balance nodes.
    static ActionNode* starsurge([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("starsurge",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* typhoon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("typhoon",
                              /*P*/ NextAction::array(0, new NextAction("moonkin form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

CasterDruidStrategy::CasterDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CasterDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

NextAction** CasterDruidStrategy::getDefaultActions()
{
    // #384/#595 (AC BalanceDruidStrategy.cpp:179-185): AC's whole default is starfire 5.4 / wrath 5.3, nothing else.
    // The fork-only defaults "starfall" (ACTION_HIGH+1) and "force of nature" (ACTION_DEFAULT+1) are gone: they sat
    // above every AC trigger edge. Starfall (48505, spec102 L76) is on its "starfall" trigger in DruidAoeStrategy at
    // 28.5 (AC GenericDruidStrategy.cpp:174) and Force of Nature (106737, talent) on the "force of nature" trigger in
    // DruidBoostStrategy at 29.0 (AC GenericDruidStrategy.cpp:97). Starfire 2912 / Wrath 5176 per cls.py druid.
    return NextAction::array(0,
                             new NextAction("starfire", 5.4f),
                             new NextAction("wrath", 5.3f),
                             nullptr);
}

void CasterDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    // #363 (AC parity, Ai/Class/Druid/Strategy/GenericDruidStrategy.cpp:171). AC guards this with
    // `if (tab == DRUID_TAB_BALANCE)`; the fork has no per-tab branching, and
    // CasterDruidStrategy IS the balance counterpart, so the node lands here unguarded - the same
    // effect. "enemy within melee" exists (strategy/triggers/TriggerContext.h:171) and AC's flat
    // 40.0 outranks the fork's ACTION_MOVE (30) flee node, which is the intended behaviour.
    triggers.push_back(new TriggerNode("enemy within melee",
                                       NextAction::array(0, new NextAction("typhoon", 40.0f), nullptr)));

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell",
    // ACTION_MOVE), nullptr)));
    // #384 (AC BalanceDruidStrategy.cpp:193): Faerie Fire at a flat 29.5 on the balance rotation itself (the fork
    // also offers it through the opt-in "caster debuff"/"dps debuff" strategies). 5.4.8: Faerie Fire 770, level 28.
    triggers.push_back(new TriggerNode("faerie fire", NextAction::array(0, new NextAction("faerie fire", 29.5f), nullptr)));
    // #595 (AC BalanceDruidStrategy.cpp): the fork-only "eclipse (lunar/solar) cooldown" edges (5.2) are removed:
    // with AC's defaults at 5.4/5.3 they could never win, so they were dead. The trigger names stay registered.
    // #595 (AC BalanceDruidStrategy.cpp:193-194): AC numbers 18.0 / 17.5. "insect swarm" is the AC trigger/action
    // name; in 5.4.8 it tracks and casts Sunfire 93402 (spec102 L18; Insect Swarm was removed in 5.0.4) - see
    // DruidTriggers.h:115-126 and DruidActions.h:183.
    triggers.push_back(new TriggerNode(
        "insect swarm", NextAction::array(0, new NextAction("insect swarm", 18.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("moonfire", NextAction::array(0, new NextAction("moonfire", 17.5f), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 10205 (spec 102, prio group 0/4): Starsurge (78674) sits under
    // Moonfire/Sunfire (0/8 -> 18.0 / 17.5) and Starfall (0/7, 28.5 in "aoe") and above the Starfire/Wrath fillers
    // (0/3 -> 5.4 / 5.3), so 17.0.
    triggers.push_back(
        new TriggerNode("starsurge", NextAction::array(0, new NextAction("starsurge", 17.0f), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 10215/10216 (spec 102, prio group 5/1): the in-combat self
    // heals, Rejuvenation at health <= 50% and Healing Touch at <= 30%. Group 5 sits above the damage rotation, so 28.
    triggers.push_back(new TriggerNode("druid self rejuvenation",
                                       NextAction::array(0, new NextAction("rejuvenation", 28.0f), nullptr)));
    triggers.push_back(new TriggerNode("druid self healing touch",
                                       NextAction::array(0, new NextAction("healing touch", 28.1f), nullptr)));
    triggers.push_back(
        new TriggerNode("eclipse (solar)", NextAction::array(0, new NextAction("wrath", 20.0f), nullptr)));
    triggers.push_back(new TriggerNode("eclipse (lunar)",
                                       NextAction::array(0, new NextAction("starfire", 20.0f), nullptr)));
    // #521 w13 MoP-only extension of AC's starfire 5.4 / wrath 5.3 defaults: while the 5.4.8 Balance Energy bar points
    // lunar (marker 67484) Starfire builds no energy, so Wrath (5.5) is the filler until Lunar Eclipse (see DruidTriggers.h).
    triggers.push_back(new TriggerNode("eclipse lunar direction",
                                       NextAction::array(0, new NextAction("wrath", 5.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("innervate", 29.0f), NULL)));
    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("flee", 39.0f), nullptr)));
}

void CasterDruidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("hurricane", ACTION_HIGH + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "light aoe", NextAction::array(0, new NextAction("insect swarm on attacker", ACTION_NORMAL + 3),
                                       new NextAction("moonfire on attacker", ACTION_NORMAL + 3), NULL)));
}

void CasterDruidDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("faerie fire", NextAction::array(0, new NextAction("faerie fire", ACTION_HIGH), nullptr)));
}
