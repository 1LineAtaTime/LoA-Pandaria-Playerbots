/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericPaladinNonCombatStrategy.h"

#include "AiFactory.h"
#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"

GenericPaladinNonCombatStrategy::GenericPaladinNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

// #424: AC GenericPaladinNonCombatStrategy::InitTriggers edges that are NOT ported, and why
// (Wowpedia patch notes, read 2026-10-04):
// - "not sensing undead" -> "sense undead": Sense Undead was removed in patch 4.0.1, so the spell
//   does not exist in 5.4.8. The trigger (PaladinTriggers.cpp) and action stay registered 1:1.
// - "greater blessing needed" -> "cast greater blessing assignment": Greater Blessings were removed
//   in patch 4.0.1; the fork has no ai::gbless subsystem (see PaladinAiObjectContext.cpp).
// - "often" -> "apply oil" (holy) / "apply stone" (prot, ret): NOT a divergence. Oils and stones
//   still exist in 5.4.8 (item-level capped). Ported under #434 (ImbueAction.cpp), see below.

void GenericPaladinNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("party member dead", NextAction::array(0, new NextAction("redemption", ACTION_CRITICAL_HEAL + 10), nullptr)));
    triggers.push_back(new TriggerNode("party member almost full health", NextAction::array(0, new NextAction("flash of light on party", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member medium health", NextAction::array(0, new NextAction("flash of light on party", 26.0f), nullptr)));
    // #735: Word of Glory (85673) costs 1-3 Holy Power - gated by the "holy power 1 available" trigger (AC's
    // TwoTriggers pattern, AC TriggerContext.h:125,375) instead of in the action.
    triggers.push_back(new TriggerNode("party member low health and holy power 1 available", NextAction::array(0, new NextAction("word of glory on party", 27.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member critical health and holy power 1 available", NextAction::array(0, new NextAction("word of glory on party", 28.0f), nullptr)));

    // #434 (AC GenericPaladinNonCombatStrategy.cpp:28-32): spec-dependent weapon imbue, same
    // priority (ACTION_IDLE + 1.0f). Holy -> oil; Protection / Retribution -> stone.
    int specTab = int(AiFactory::GetPlayerSpecTab(botAI->GetBot()));
    if (specTab == SPEC_PALADIN_HOLY)
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply oil", ACTION_IDLE + 1.0f), nullptr)));
    if (specTab == SPEC_PALADIN_PROTECTION || specTab == SPEC_PALADIN_RETRIBUTION)
        triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", ACTION_IDLE + 1.0f), nullptr)));
}
