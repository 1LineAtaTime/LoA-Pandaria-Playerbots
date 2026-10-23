/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ShamanNonCombatStrategy.h"

#include "AiFactory.h"
#include "Playerbots.h"

void ShamanNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #427 (AC parity, AC ShamanNonCombatStrategy.cpp:87-88): the "totemic recall" action/trigger were
    // registered in #408 but nothing consumed the trigger, so the action was inert.
    triggers.push_back(
        new TriggerNode("totemic recall", NextAction::array(0, new NextAction("totemic recall", 60.0f), nullptr)));

    // #163 S8 / #147 (AC parity, ShamanNonCombatStrategy.cpp:139): apply the configured stance to a
    // freshly summoned elemental.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 65.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("party member dead",
                        NextAction::array(0, new NextAction("ancestral spirit", ACTION_CRITICAL_HEAL + 10), nullptr)));
    // #382: the "water breathing" / "water breathing on party" edges are removed - Water Breathing is NOT-LEARNABLE
    // in 5.4.8 (cls.py shaman; removed in 5.0.4, shamans keep only Water Walking 546).
    triggers.push_back(
        new TriggerNode("water walking", NextAction::array(0, new NextAction("water walking", 12.0f), nullptr)));
    triggers.push_back(new TriggerNode("water walking on party",
                                       NextAction::array(0, new NextAction("water walking on party", 11.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member critical health",
                                       NextAction::array(0, new NextAction("riptide on party", 31.0f),
                                                         new NextAction("healing wave on party", 30.0f),
                                                         new NextAction("healing surge on party", 29.5f), NULL)));

    triggers.push_back(new TriggerNode("party member low health",
                                       NextAction::array(0, new NextAction("riptide on party", 29.0f),
                                                         new NextAction("healing wave on party", 28.0f),
                                                         new NextAction("healing surge on party", 27.5f), NULL)));

    triggers.push_back(new TriggerNode("party member medium health",
                                       NextAction::array(0, new NextAction("riptide on party", 27.0f),
                                                         new NextAction("healing wave on party", 26.0f),
                                                         new NextAction("healing surge on party", 25.5f), NULL)));

    triggers.push_back(new TriggerNode("party member almost full health",
                                       NextAction::array(0, new NextAction("riptide on party", 25.0f),
                                                         new NextAction("healing surge on party", 24.0f), NULL)));

    triggers.push_back(
        new TriggerNode("group heal setting", NextAction::array(0, new NextAction("chain heal on party", 27.0f), NULL)));

    // #382 (AC ShamanNonCombatStrategy.cpp:79-84 cure edges): Cure Poison / Cure Disease are NOT-LEARNABLE in 5.4.8
    // (cls.py shaman) and Cleanse Spirit removes only Curse effects there (Wowhead mop-classic tooltip 51886,
    // 2026-10-04), so only AC's curse edges are ported; poison/disease are dropped (HasAuraToDispel does not look at
    // the spell, a poison edge would make the bot spam a cast that cannot remove the aura).
    triggers.push_back(new TriggerNode("cleanse spirit curse",
                                       NextAction::array(0, new NextAction("cleanse spirit", 24.0f), nullptr)));
    triggers.push_back(new TriggerNode("party member cleanse spirit curse",
                                       NextAction::array(0, new NextAction("cleanse spirit curse on party", 23.0f), nullptr)));

    // #427 (AC parity, AC ShamanNonCombatStrategy.cpp:110-129): spec specific out-of-combat weapon imbues
    // and self shields. The spec combat strategies carry the same nodes at combat time; AC additionally
    // re-applies them from the non-combat pass after a fight.
    Player* bot = botAI->GetBot();
    Specializations tab = AiFactory::GetPlayerSpecTab(bot);

    if (tab == Specializations::SPEC_SHAMAN_ELEMENTAL)
    {
        // #382: per-hand imbue actions (AC ShamanNonCombatStrategy.cpp:110-129), see MeleeShamanStrategy.cpp.
        triggers.push_back(new TriggerNode("main hand weapon no imbue",
                                           NextAction::array(0, new NextAction("flametongue weapon main hand", 22.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 21.0f), nullptr)));
    }
    else if (tab == Specializations::SPEC_SHAMAN_ENHANCEMENT)
    {
        triggers.push_back(new TriggerNode("main hand weapon no imbue",
                                           NextAction::array(0, new NextAction("windfury weapon main hand", 22.0f), nullptr)));
        // #382: AC's non-combat node is "flametongue weapon off hand". The hand-less "flametongue weapon" this fork
        // used before could not resolve a weapon at all (ItemForSpellValue stub), the per-hand action can.
        triggers.push_back(new TriggerNode("off hand weapon no imbue",
                                           NextAction::array(0, new NextAction("flametongue weapon off hand", 21.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("lightning shield",
                            NextAction::array(0, new NextAction("lightning shield", 20.0f), nullptr)));
    }
    else if (tab == Specializations::SPEC_SHAMAN_RESTORATION)
    {
        triggers.push_back(new TriggerNode("main hand weapon no imbue",
                                           NextAction::array(0, new NextAction("earthliving weapon main hand", 22.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 20.0f), nullptr)));
        // #382 (MoP-only): Purify Spirit (77130) also removes Magic, see HealShamanStrategy.cpp.
        triggers.push_back(new TriggerNode("party member purify spirit magic",
                                           NextAction::array(0, new NextAction("purify spirit magic on party", 23.0f), nullptr)));
        triggers.push_back(new TriggerNode("purify spirit magic",
                                           NextAction::array(0, new NextAction("purify spirit", 24.0f), nullptr)));
    }

    // #427 (AC parity, AC ShamanNonCombatStrategy.cpp:137-139): keep the configured pet stance/spell active.
    triggers.push_back(
        new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
}

void ShamanNonCombatStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    NonCombatStrategy::InitMultipliers(multipliers);
}
