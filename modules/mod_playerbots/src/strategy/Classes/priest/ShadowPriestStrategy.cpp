/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShadowPriestStrategy.h"

#include "Playerbots.h"
#include "ShadowPriestStrategyActionNodeFactory.h"

ShadowPriestStrategy::ShadowPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new ShadowPriestStrategyActionNodeFactory());
}

NextAction** ShadowPriestStrategy::getDefaultActions()
{
    // Lane 8 / #863: MoP rotation row 25803 Cascade (group 0/10) sits above Mind Blast (0/3); it falls through when on cooldown.
    return NextAction::array(0, new NextAction("cascade", ACTION_DEFAULT + 0.4f),
                             new NextAction("mind blast", ACTION_DEFAULT + 0.3f),
                             new NextAction("mind flay", ACTION_DEFAULT + 0.2f),
                             new NextAction("shadow word: death", ACTION_DEFAULT + 0.1f), // cast during movement
                             new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void ShadowPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell",
    // ACTION_MOVE + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("shadowform", NextAction::array(0, new NextAction("shadowform", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode("low mana", NextAction::array(0, new NextAction("dispersion", ACTION_HIGH
        + 5), nullptr)));
    triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("dispersion", ACTION_HIGH
        + 5), nullptr)));
    // #524 documented divergence: AC has Vampiric Embrace as a non-combat buff (PriestNonCombatStrategy); in 5.x it is a
    // 3 min cooldown (Wowhead mop-classic spell 15286, 2026-10-04), so MoP keeps it in combat here.
    triggers.push_back(
        new TriggerNode("vampiric embrace", NextAction::array(0, new NextAction("vampiric embrace", 16.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("silence", NextAction::array(0, new NextAction("silence", ACTION_INTERRUPT + 1), nullptr)));
    triggers.push_back(
        new TriggerNode("silence on enemy healer",
                        NextAction::array(0, new NextAction("silence on enemy healer", ACTION_INTERRUPT), nullptr)));
    // Lane 8 / #863 (MoP rotation row 14/15, shadow, group 10/2-10/1): Silence any casting enemy (pvp maps).
    triggers.push_back(new TriggerNode(
        "enemy caster in range",
        NextAction::array(0, new NextAction("silence on enemy caster", ACTION_INTERRUPT + 0.5f), nullptr)));
    // Lane 8 / #863 (MoP rotation row 25802): Void Tendrils on an enemy player within melee range (pvp maps,
    // not already CC'd - guarded in the action). 25803 Cascade is in the default actions.
    triggers.push_back(new TriggerNode(
        "enemy player in melee range", NextAction::array(0, new NextAction("void tendrils", ACTION_HIGH + 3), nullptr)));
    // Lane E (#863, MoP rotation row 25802 prio 0/10, no pvp condition): Void Tendrils on a creature that attacks the bot as well.
    triggers.push_back(new TriggerNode(
        "void tendrils target close", NextAction::array(0, new NextAction("void tendrils", ACTION_HIGH + 3), nullptr)));
    // MoP rotation row 25817/25816/25819 self defensives at their thresholds (PW:S 60%, Vampiric Embrace 40%, Renew 30%).
    triggers.push_back(new TriggerNode(
        "priest self health 60", NextAction::array(0, new NextAction("power word: shield", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode(
        "priest self health 40", NextAction::array(0, new NextAction("vampiric embrace", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode(
        "priest self health 30", NextAction::array(0, new NextAction("renew", ACTION_HIGH), nullptr)));
    // triggers.push_back(new TriggerNode("shadowfiend", NextAction::array(0, new NextAction("shadowfiend",
    // ACTION_HIGH), nullptr))); triggers.push_back(new TriggerNode("medium mana", NextAction::array(0, new
    // NextAction("shadowfiend", ACTION_HIGH), nullptr))); triggers.push_back(new TriggerNode("low mana",
    // NextAction::array(0, new NextAction("mana burn", ACTION_HIGH), nullptr)));
}

void ShadowPriestAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "shadow word: pain on attacker",
        NextAction::array(0, new NextAction("shadow word: pain on attacker", ACTION_NORMAL + 5), nullptr)));
    triggers.push_back(new TriggerNode(
        "vampiric touch on attacker",
        NextAction::array(0, new NextAction("vampiric touch on attacker", ACTION_NORMAL + 4), nullptr)));
    // #524 (AC ShadowPriestStrategy.cpp:68 ShadowPriestAoeStrategy, trigger "mind sear channel check"): cancel a
    // running Mind Sear channel when too few enemies remain (CancelChannelAction, #423).
    triggers.push_back(new TriggerNode(
        "mind sear channel check", NextAction::array(0, new NextAction("cancel channel", ACTION_HIGH + 5), nullptr)));
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("mind sear", ACTION_HIGH + 4), nullptr)));
}

void ShadowPriestDebuffStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #524 (AC ShadowPriestStrategy.cpp:110-132): AC order is vampiric touch HIGH+3, devouring plague HIGH+2,
    // shadow word: pain HIGH+1 (the fork had plague/touch swapped).
    triggers.push_back(new TriggerNode(
        "vampiric touch", NextAction::array(0, new NextAction("vampiric touch", ACTION_HIGH + 3), nullptr)));
    // #735: 5.4.8 Devouring Plague spends 3 Shadow Orbs - fired from "shadow orbs 3 available" (the plain debuff
    // action still skips a target that has our plague), as AC fires Rupture from "combo points 5 available" (AC
    // CombatRogueStrategy.cpp:118-126). The "devouring plague" debuff trigger stays registered (AC surface) but unwired:
    // at 0 orbs it fired a cast that failed SPELL_FAILED_NO_POWER on every try.
    triggers.push_back(new TriggerNode(
        "shadow orbs 3 available", NextAction::array(0, new NextAction("devouring plague", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "shadow word: pain", NextAction::array(0, new NextAction("shadow word: pain", ACTION_HIGH + 1), nullptr)));
    // triggers.push_back(new TriggerNode("feedback", NextAction::array(0, new NextAction("feedback", 80.0f),
    // nullptr))); triggers.push_back(new TriggerNode("hex of weakness", NextAction::array(0, new NextAction("hex of
    // weakness", 10.0f), nullptr)));
}
