/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericPaladinStrategy.h"

#include "GenericPaladinStrategyActionNodeFactory.h"
#include "Playerbots.h"

GenericPaladinStrategy::GenericPaladinStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPaladinStrategyActionNodeFactory());
}

void GenericPaladinStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("divine shield",
        ACTION_HIGH + 5), nullptr)));
    // rotation lane 6 (#861): Rebuke (prio 10/2, current target) and (prio 10/1, any
    // casting enemy in range). Above Hammer of Justice (prio 6/0). The Divine Shield above is guarded
    // against carrying a PvP objective in CastDivineShieldAction::isUseful.
    triggers.push_back(new TriggerNode("rebuke interrupt",
        NextAction::array(0, new NextAction("rebuke", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(new TriggerNode("enemy caster in range",
        NextAction::array(0, new NextAction("rebuke on enemy caster", ACTION_INTERRUPT + 0.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("hammer of justice interrupt",
                        NextAction::array(0, new NextAction("hammer of justice", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "hammer of justice on enemy healer",
        NextAction::array(0, new NextAction("hammer of justice on enemy healer", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "hammer of justice on snare target",
        NextAction::array(0, new NextAction("hammer of justice on snare target", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode(
        "critical health", NextAction::array(0, new NextAction("lay on hands", ACTION_EMERGENCY), nullptr)));
    triggers.push_back(
        new TriggerNode("party member critical health",
                        NextAction::array(0, new NextAction("lay on hands on party", ACTION_EMERGENCY + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "protect party member",
        NextAction::array(0, new NextAction("blessing of protection on party", ACTION_EMERGENCY + 2), nullptr)));
    // #163 S8 (AC parity, Ai/Class/Paladin/Strategy/GenericPaladinStrategy.cpp:34-35).
    triggers.push_back(new TriggerNode(
        "hand of freedom on party",
        NextAction::array(0, new NextAction("hand of freedom on party", ACTION_HIGH + 4), nullptr)));
    // #408 (AC GenericPaladinStrategy.cpp:29-30) - 1:1
    triggers.push_back(new TriggerNode("divine shield low health",
        NextAction::array(0, new NextAction("flash of light", ACTION_EMERGENCY + 3),
                                new NextAction("holy light", ACTION_EMERGENCY + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("high mana", NextAction::array(0, new NextAction("divine plea", ACTION_HIGH), NULL)));
}

void PaladinCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "cleanse cure disease", NextAction::array(0, new NextAction("cleanse disease", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("cleanse party member cure disease",
                        NextAction::array(0, new NextAction("cleanse disease on party", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "cleanse cure poison", NextAction::array(0, new NextAction("cleanse poison", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("cleanse party member cure poison",
                        NextAction::array(0, new NextAction("cleanse poison on party", ACTION_DISPEL + 1), nullptr)));
    triggers.push_back(new TriggerNode(
        "cleanse cure magic", NextAction::array(0, new NextAction("cleanse magic", ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("cleanse party member cure magic",
                        NextAction::array(0, new NextAction("cleanse magic on party", ACTION_DISPEL + 1), nullptr)));
}

void PaladinBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    
    // triggers.push_back(new TriggerNode("divine favor", NextAction::array(0, new NextAction("divine favor",
    // ACTION_HIGH + 1), nullptr)));
}

void PaladinCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("turn undead", NextAction::array(0, new NextAction("turn undead", ACTION_HIGH + 1), nullptr)));
}

void PaladinHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        NextAction::array(0,
                            new NextAction("hammer of wrath", ACTION_DEFAULT + 0.6f),
                            new NextAction("holy shock", ACTION_DEFAULT + 0.5f),
                            new NextAction("shield of righteousness", ACTION_DEFAULT + 0.4f),
                            // 5.4.8: Judgement of Light does not exist; Judgement is a single unified
                            // ability in MoP, so this now casts the generic "judgement" action.
                            new NextAction("judgement", ACTION_DEFAULT + 0.3f),
                            new NextAction("consecration", ACTION_DEFAULT + 0.2f),
                            new NextAction("exorcism", ACTION_DEFAULT+ 0.1f),
                            nullptr)));
}
