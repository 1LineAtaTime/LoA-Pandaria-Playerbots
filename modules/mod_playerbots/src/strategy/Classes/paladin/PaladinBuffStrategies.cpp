/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinBuffStrategies.h"

#include "Playerbots.h"

void PaladinBuffManaStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // 5.4.8: Blessing of Wisdom does not exist; MoP paladins only have Blessing of Kings/Might, neither
    // of which restores mana, so there is no replacement mana blessing to maintain here.
    // triggers.push_back(
    //     new TriggerNode("blessing of wisdom on party",
    //                     NextAction::array(0, new NextAction("blessing of wisdom on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of wisdom",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffHealthStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of kings on party",
                        NextAction::array(0, new NextAction("blessing of kings on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of kings",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of might on party",
                        NextAction::array(0, new NextAction("blessing of might on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of might",
    // ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffArmorStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383 (AC PaladinBuffStrategies.cpp:49-53 devotion aura -> devotion aura, ACTION_NORMAL): edge removed.
    // In 5.4.8 Devotion Aura (31821, L60) is a 3 min raid cooldown (10 s party/raid DR + immunity to
    // silence/interrupt), not a persistent keep-up aura (Wowhead mop-classic tooltip 31821, 2026-10-05:
    // "Instant, 3 min cooldown"); the BuffTrigger fired every tick and wave 9 retried it 29x with
    // why=cooldown. Strategy key "barmor" stays registered for key parity.
}

// #383 divergence (consistent with #424/#426): Retribution Aura (8990/13008/79976) and Concentration Aura
// (79963/81455) are NOT-LEARNABLE in 5.4.8 (cls.py 2026-10-04) - Paladin auras were removed in 5.0 except
// Devotion Aura (31821, L60). There is no 5.x successor for either, so the "baoe"/"bcast" edges below can
// never cast; they stay registered 1:1 with AC for the typed key surface and are intentionally not remapped
// to Devotion Aura ("barmor" already owns that).
void PaladinBuffAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (retribution aura NOT-LEARNABLE in 5.4.8, cls.py 2026-10-05); key stays registered.
}

void PaladinBuffCastStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (concentration aura NOT-LEARNABLE in 5.4.8, cls.py 2026-10-05); key stays registered.
}

void PaladinBuffSpeedStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (crusader aura NO-SUCH-NAME in 5.4.8, cls.py 2026-10-05); key stays registered.
}

void PaladinBuffThreatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "righteous fury", NextAction::array(0, new NextAction("righteous fury", ACTION_HIGH + 8), nullptr)));
}

void PaladinBuffStatsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("blessing of kings on party",
                        NextAction::array(0, new NextAction("blessing of kings on party", 11.0f), nullptr)));
    // triggers.push_back(new TriggerNode("blessing", NextAction::array(0, new NextAction("blessing of kings",
    // ACTION_HIGH + 8), nullptr)));
}

// AC PaladinBuffStrategies.cpp:33-51 (1:1)
void PaladinShadowResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (shadow resistance aura NO-SUCH-NAME in 5.4.8, cls.py 2026-10-05); key stays registered.
}

void PaladinFrostResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (frost resistance aura NO-SUCH-NAME in 5.4.8, cls.py 2026-10-05); key stays registered.
}

void PaladinFireResistanceStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #383: edge removed (fire resistance aura NO-SUCH-NAME in 5.4.8, cls.py 2026-10-05); key stays registered.
}
