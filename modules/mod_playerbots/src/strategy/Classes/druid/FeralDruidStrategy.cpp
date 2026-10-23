/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FeralDruidStrategy.h"

#include "Playerbots.h"

class FeralDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FeralDruidStrategyActionNodeFactory()
    {
        creators["survival instincts"] = &survival_instincts;
        creators["thorns"] = &thorns;
        creators["omen of clarity"] = &omen_of_clarity;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["prowl"] = &prowl;
    }

private:
    static ActionNode* survival_instincts([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("survival instincts",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("barkskin"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* thorns([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thorns",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* omen_of_clarity([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("omen of clarity",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* prowl([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("prowl",
                              /*P*/ NextAction::array(0, new NextAction("cat form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

FeralDruidStrategy::FeralDruidStrategy(PlayerbotAI* botAI) : GenericDruidStrategy(botAI)
{
    actionNodeFactories.Add(new FeralDruidStrategyActionNodeFactory());
    actionNodeFactories.Add(new ShapeshiftDruidStrategyActionNodeFactory());
}

void FeralDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDruidStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("not facing target", NextAction::array(0, new NextAction("set facing",
    // ACTION_NORMAL + 7), nullptr)));
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("reach melee", ACTION_HIGH + 1), nullptr)));
    // triggers.push_back(new TriggerNode("enemy too close for melee", NextAction::array(0, new NextAction("move out of
    // enemy contact", ACTION_NORMAL + 8), nullptr)));
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("survival instincts", ACTION_EMERGENCY + 1), nullptr)));
    // #384 (AC FeralDruidStrategy.cpp:86-87): AC fires Survival Instincts from "low health" at 91.0 (the fork's
    // "critical health" edge above stays). Survival Instincts (61336) is spec103/104 level 56 in 5.4.8 (cls.py
    // druid); the ActionNode falls back to barkskin when it is on cooldown.
    triggers.push_back(new TriggerNode(
        "low health", NextAction::array(0, new NextAction("survival instincts", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "omen of clarity", NextAction::array(0, new NextAction("omen of clarity", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(new TriggerNode("player has flag",
                                       NextAction::array(0, new NextAction("dash", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(new TriggerNode("enemy flagcarrier near",
                                       NextAction::array(0, new NextAction("dash", ACTION_EMERGENCY + 2), nullptr)));
    // #595 (AC FeralDruidStrategy.cpp:75-92 has no berserk edge): the fork-only "berserk" edge (ACTION_HIGH+6) that
    // lived here duplicated DruidBoostStrategy's "berserk" -> berserk at 27.5 (AC GenericDruidStrategy.cpp:103) and
    // fired outside the "boost" strategy; removed, Berserk 106952 (spec103/104 L48) now comes only from "boost".
}

// #408 (AC parity, Ai/Class/Druid/Strategy/FeralDruidStrategy.cpp:94-104).
void FeralChargeDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();

    if (bot->HasSpell(768) /*cat form*/ && !bot->HasAura(16931) /*thick hide*/)
        triggers.push_back(new TriggerNode(
            "enemy out of melee", NextAction::array(0, new NextAction("feral charge - cat", 29.0f), nullptr)));
    else
        triggers.push_back(new TriggerNode(
            "enemy out of melee", NextAction::array(0, new NextAction("feral charge - bear", 18.0f), nullptr)));
}
