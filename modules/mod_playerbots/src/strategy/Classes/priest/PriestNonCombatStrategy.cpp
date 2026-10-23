/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PriestNonCombatStrategy.h"

#include "Playerbots.h"
#include "PriestNonCombatStrategyActionNodeFactory.h"

PriestNonCombatStrategy::PriestNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new PriestNonCombatStrategyActionNodeFactory());
}

void PriestNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, PriestNonCombatStrategy.cpp:51): apply the configured stance to a
    // freshly summoned shadowfiend/mindbender.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));

    // triggers.push_back(new TriggerNode("power word: fortitude", NextAction::array(0, new NextAction("power word:
    // fortitude", 12.0f), nullptr))); triggers.push_back(new TriggerNode("divine spirit", NextAction::array(0, new
    // NextAction("divine spirit", 14.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("inner fire", NextAction::array(0, new NextAction("inner fire", 10.0f), nullptr)));
    // Lane E (#863, MoP rotation row 25715 "chakra sanctuary", prio 2/0, mode combat + out of combat): re-enter
    // Chakra: Sanctuary out of combat whenever it is missing (the combat edges switch to Serenity as needed). Holy only:
    // the trigger needs the spell to be known.
    triggers.push_back(new TriggerNode(
        "chakra: sanctuary missing", NextAction::array(0, new NextAction("chakra: sanctuary", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("greater heal", 70.0f),
    // nullptr))); triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0, new
    // NextAction("greater heal on party", 60.0f), nullptr))); triggers.push_back(new TriggerNode("medium health",
    // NextAction::array(0, new NextAction("renew", 41.0f), nullptr))); triggers.push_back(new TriggerNode("party member
    // medium health", NextAction::array(0, new NextAction("renew on party", 40.0f), nullptr))); triggers.push_back(new
    // TriggerNode("medium aoe heal", NextAction::array(0, new NextAction("lightwell", 42.f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member dead", NextAction::array(0, new NextAction("remove shadowform", ACTION_CRITICAL_HEAL + 11),
                                               new NextAction("resurrection", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // triggers.push_back(new TriggerNode("swimming", NextAction::array(0, new NextAction("levitate", 1.0f), nullptr)));
    // #434 (AC PriestNonCombatStrategy.cpp:27): wizard/mana oils still exist in 5.4.8 (capped by weapon
    // item level), so this edge is ported 1:1 (ImbueAction.cpp).
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", 1.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_CRITICAL_HEAL + 3),
                                          new NextAction("penance on party", ACTION_CRITICAL_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_CRITICAL_HEAL + 1), NULL)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("penance on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 1), NULL)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL + 9),
                                          new NextAction("penance on party", ACTION_LIGHT_HEAL + 8), NULL)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("renew on party", ACTION_LIGHT_HEAL + 3), NULL)));

    triggers.push_back(
        new TriggerNode("group heal setting", NextAction::array(0, new NextAction("circle of healing on party", 27.0f), NULL)));
}

void PriestBuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #524: "prayer of fortitude on party" edge removed - Prayer of Fortitude is not learnable in 5.4.8 (cls.py
    // priest NOT-LEARNABLE); 5.x Power Word: Fortitude buffs the group (edge below). AC has no such edge.
    // #524: AC's "prayer of spirit on party" / "divine spirit on party" edges are not ported - Divine Spirit
    // and Prayer of Spirit were removed in 4.0.1 (Wowpedia "Divine Spirit" patch notes, read 2026-10-04;
    // NO-SUCH-NAME in the 5.4.8 DBC per cls.py priest).
    triggers.push_back(
        new TriggerNode("power word: fortitude on party",
                        NextAction::array(0, new NextAction("power word: fortitude on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("fear ward", NextAction::array(0, new NextAction("fear ward", 10.0f),
    // nullptr))); triggers.push_back(new TriggerNode("touch of weakness", NextAction::array(0, new NextAction("touch of
    // weakness", 10.0f), nullptr))); triggers.push_back(new TriggerNode("shadowguard", NextAction::array(0, new
    // NextAction("shadowguard", 10.0f), nullptr)));
}

void PriestShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #524: AC PriestNonCombatStrategy.cpp:62 casts shadow protection (+ on party) here, but Shadow Protection is
    // not learnable in 5.4.8 (cls.py priest NOT-LEARNABLE), so the edges would retry forever; removed. The
    // "rshadow" strategy key stays registered (AiFactory/config use it) and is now only the NC base.
    // triggers.push_back(new TriggerNode("shadow protection on party", NextAction::array(0, new NextAction("shadow
    // protection on party", 10.0f), nullptr)));
}
