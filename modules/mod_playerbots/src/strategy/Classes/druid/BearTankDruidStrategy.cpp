/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "BearTankDruidStrategy.h"

#include "Playerbots.h"

class BearTankDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    BearTankDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["feral charge - bear"] = &feral_charge_bear;
        creators["swipe (bear)"] = &swipe_bear;
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["bear form"] = &bear_form;
        creators["dire bear form"] = &dire_bear_form;
        creators["mangle (bear)"] = &mangle_bear;
        creators["maul"] = &maul;
        creators["bash"] = &bash;
        creators["swipe"] = &swipe;
        creators["lacerate"] = &lacerate;
        creators["demoralizing roar"] = &demoralizing_roar;
        // #384: Thrash replaces Demoralizing Roar (see CastThrashAction, DruidBearActions.h).
        creators["thrash"] = &thrash;
        creators["taunt spell"] = &growl;
        // #363 (AC parity, Ai/Class/Druid/Strategy/BearDruidStrategy.cpp:138): AC fires
        // Challenging Roar off "high aoe" at 26.5. This fork's BearTankDruidStrategy is the
        // counterpart of AC's BearDruidStrategy. AC keeps a raw relevance here rather than an
        // ACTION_* constant, and so does this node, so it lands above the "medium aoe" group
        // (demoralizing roar / swipe, ACTION_HIGH + 6 == 26) exactly as it does in AC.
        creators["challenging roar"] = &challenging_roar;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ NextAction::array(0, new NextAction("feral charge - bear"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* feral_charge_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("feral charge - bear",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("reach melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* swipe_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("swipe (bear)",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire (feral)",
                              /*P*/ NextAction::array(0, new NextAction("feral charge - bear"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* bear_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("bear form",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* dire_bear_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("dire bear form",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ NextAction::array(0, new NextAction("bear form"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* mangle_bear([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mangle (bear)",
                              /*P*/ nullptr,
                              // /*A*/ NextAction::array(0, new NextAction("lacerate"), nullptr),
                              nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* maul([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("maul",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* bash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("bash",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* swipe([[maybe_unused]] PlayerbotAI* botAI)
    {
        // #742: MoP-only merged Swipe (AoE): no alternative, like the warrior AoE nodes (17dee51c); a "melee"
        // alternative at an AoE relevance (HIGH + 5/6 > reach melee 21) would stop a mounted bear in place.
        return new ActionNode("swipe",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* lacerate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("lacerate",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("maul"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* growl([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("growl",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* challenging_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("challenging roar",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* demoralizing_roar([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("demoralizing roar",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* thrash([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("thrash",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

BearTankDruidStrategy::BearTankDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new BearTankDruidStrategyActionNodeFactory());
}

NextAction** BearTankDruidStrategy::getDefaultActions()
{
    return NextAction::array(
        0, new NextAction("mangle (bear)", ACTION_DEFAULT + 0.5f),
        new NextAction("faerie fire (feral)", ACTION_DEFAULT + 0.4f), new NextAction("lacerate", ACTION_DEFAULT + 0.3f),
        new NextAction("maul", ACTION_DEFAULT + 0.2f), new NextAction("enrage", ACTION_DEFAULT + 0.1f),
        new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void BearTankDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("feral charge - bear", ACTION_NORMAL + 8), nullptr)));
    // triggers.push_back(new TriggerNode("thorns", NextAction::array(0, new NextAction("thorns", ACTION_HIGH + 9),
    // nullptr)));
    triggers.push_back(
        new TriggerNode("bear form", NextAction::array(0, new NextAction("dire bear form", ACTION_HIGH + 8), nullptr)));
    // #384 (AC BearDruidStrategy.cpp:115-118): AC fires Frenzied Regeneration from "medium health" at 27.0 (the
    // fork had "low health", which a medium-health trigger already covers). Frenzied Regeneration 22842 is
    // learnable at level 68 in 5.4.8 (cls.py druid).
    triggers.push_back(new TriggerNode(
        "medium health", NextAction::array(0, new NextAction("frenzied regeneration", ACTION_HIGH + 7), nullptr)));
    // #384 (AC BearDruidStrategy.cpp:119-136): the bear rotation edges the fork only had as defaults. Priorities are
    // AC's literals. "mangle (bear)"/"lacerate" (33917 / 33745, level 6 / 38) are learnable in 5.4.8.
    triggers.push_back(new TriggerNode(
        "mangle (bear)", NextAction::array(0, new NextAction("mangle (bear)", 17.5f), nullptr)));
    triggers.push_back(new TriggerNode("lacerate", NextAction::array(0, new NextAction("lacerate", 16.0f), nullptr)));
    // #384 (AC BearDruidStrategy.cpp:130-132 "demoralizing roar" 15.5): Demoralizing Roar is NOT-LEARNABLE in 5.4.8;
    // Thrash (77758, level 28) applies its Weakened Blows debuff (Wowhead mop-classic 77758, 2026-10-04).
    triggers.push_back(new TriggerNode("thrash", NextAction::array(0, new NextAction("thrash", 15.5f), nullptr)));
    // #384 (AC BearDruidStrategy.cpp:140): Berserk (106952) removes Mangle's cooldown for a guardian, so spam it.
    triggers.push_back(new TriggerNode(
        "berserk active", NextAction::array(0, new NextAction("mangle (bear)", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "faerie fire (feral)", NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_HIGH + 7), nullptr)));
    // #384 (AC BearDruidStrategy.cpp:133-139): AC pairs Growl with a Faerie Fire fallback (25.5) on "lose aggro".
    triggers.push_back(
        new TriggerNode("lose aggro", NextAction::array(0, new NextAction("growl", ACTION_HIGH + 8),
                                                        new NextAction("faerie fire (feral)", 25.5f), nullptr)));
    // #384: AC's first medium-aoe action, Demoralizing Roar, is NOT-LEARNABLE in 5.4.8 -> Thrash (see above).
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("thrash", ACTION_HIGH + 6),
                                                        new NextAction("swipe (bear)", ACTION_HIGH + 6), nullptr)));
    triggers.push_back(
        new TriggerNode("light aoe", NextAction::array(0, new NextAction("swipe (bear)", ACTION_HIGH + 5), nullptr)));
    // #363 (AC parity, Ai/Class/Druid/Strategy/BearDruidStrategy.cpp:138). AC puts Challenging
    // Roar on "high aoe" at 26.5 - above the 24.5/24.0 "medium aoe" pair, so the fork keeps the
    // literal (26.5 > ACTION_HIGH + 6 == 26) rather than bending it to an ACTION_* constant.
    triggers.push_back(new TriggerNode("high aoe", NextAction::array(0, new NextAction("challenging roar", 26.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("bash", NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("bash on enemy healer",
                        NextAction::array(0, new NextAction("bash on enemy healer", ACTION_INTERRUPT + 1), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 10421 (spec 104): Skull Bash on any casting enemy in a
    // battleground / arena (shared lane 0 "enemy caster in range"; the action's isUseful gates on "in pvp instance").
    triggers.push_back(new TriggerNode(
        "enemy caster in range",
        NextAction::array(0, new NextAction("skull bash on enemy caster", ACTION_INTERRUPT + 0.5f), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 10417 (spec 104, prio group 5/1): Savage Defense (62606) at
    // rage >= 80, with the other group 5 defensives (Frenzied Regeneration is ACTION_HIGH + 7 above).
    triggers.push_back(new TriggerNode(
        "savage defense", NextAction::array(0, new NextAction("savage defense", ACTION_HIGH + 7.5f), nullptr)));
    // Rotation fix lane A (#860), MoP rotation row 10419 (spec 104, prio group 5/1): Enrage (5229, in combat, "always").
    // w73: Enrage was only a default action at ACTION_DEFAULT + 0.1, behind Mangle/FF/Lacerate/Maul, so it only ever ran on
    // a tick where all of them failed (GCD/casting -> NOT_READY 70, 38x, never OK). Now a group-5 edge just under Savage Defense.
    triggers.push_back(
        new TriggerNode("enrage", NextAction::array(0, new NextAction("enrage", ACTION_HIGH + 7.2f), nullptr)));
}
