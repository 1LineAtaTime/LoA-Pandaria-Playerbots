/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HealShamanStrategy.h"

#include "Playerbots.h"

class HealShamanStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HealShamanStrategyActionNodeFactory()
    {
        creators["earthliving weapon"] = &earthliving_weapon;
        creators["mana tide totem"] = &mana_tide_totem;
    }

private:
    static ActionNode* earthliving_weapon([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("earthliving weapon",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("flametongue weapon"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* mana_tide_totem([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mana tide totem",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                              /*C*/ nullptr);
    }
};

HealShamanStrategy::HealShamanStrategy(PlayerbotAI* botAI) : GenericShamanStrategy(botAI)
{
    actionNodeFactories.Add(new HealShamanStrategyActionNodeFactory());
}

void HealShamanStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericShamanStrategy::InitTriggers(triggers);

    // #382 (AC RestoShamanStrategy.cpp:20 "call of the elements" 60): DIVERGENCE, edge removed - AC's spell 66842
    // places the Totem Bar totems; the bar and the spell are gone for players in 5.4.8 (wave 9: has=0 why=not-known;
    // Wowhead mop-classic tooltip 66842 / 108285, 2026-10-05). Trigger/action stay registered.
    // #382: Stoneclaw Totem no longer exists in 5.4.8 (cls.py shaman: NO-SUCH-NAME); Stone Bulwark Totem (108270,
    // Wowhead mop-classic tooltip 108270, 2026-10-04) is the earth-slot absorb-shield totem that replaces it.
    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("stone bulwark totem", 40.0f), nullptr)));

    // triggers.push_back(new TriggerNode("enemy out of spell", NextAction::array(0, new NextAction("reach spell",
    // ACTION_NORMAL + 9), nullptr))); triggers.push_back(new TriggerNode("shaman weapon", NextAction::array(0, new
    // NextAction("earthliving weapon", 22.0f), nullptr)));
    // #382 (AC RestoShamanStrategy.cpp:12-61 has no imbue edge): the fork-only combat "main hand weapon no imbue ->
    // earthliving weapon main hand" is removed; AC imbues out of combat only (ShamanNonCombatStrategy.cpp keeps it).
    triggers.push_back(new TriggerNode(
        "group heal setting",
        NextAction::array(0, new NextAction("riptide on party", 27.0f), new NextAction("chain heal on party", 26.0f), NULL)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, new NextAction("riptide on party", 25.0f), new NextAction("healing wave on party", 24.0f),
                          new NextAction("healing surge on party", 23.0f), nullptr)));

    // Rotation lane 3 (#858), Greater Healing Wave (at target <= 60 %, between Chain
    // Heal and Healing Wave), Healing Rain ( top of the heal list, on the AC multi-target heal trigger),
    // 26404 (Unleash Elements on self before healing), 26407 (Ascendance with >= 3 members below 45 %).
    triggers.push_back(new TriggerNode(
        "party member low health", NextAction::array(0, new NextAction("greater healing wave on party", 19.5f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member medium health", NextAction::array(0, new NextAction("greater healing wave on party", 16.5f), nullptr)));
    triggers.push_back(new TriggerNode(
        "medium aoe heal", NextAction::array(0, new NextAction("healing rain", 27.5f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member medium health", NextAction::array(0, new NextAction("unleash elements", 28.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("resto ascendance", NextAction::array(0, new NextAction("ascendance", 28.5f), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("riptide on party", 19.0f), new NextAction("healing wave on party", 18.0f),
                          new NextAction("healing surge on party", 17.0f), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("riptide on party", 16.0f), new NextAction("healing wave on party", 15.0f),
                          new NextAction("healing surge on party", 14.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0, new NextAction("riptide on party", 12.0f),
                                          new NextAction("healing surge on party", 11.0f), nullptr)));

    // #382 (AC RestoShamanStrategy.cpp party-cleanse edges): in 5.4.8 Cleanse Spirit removes only Curse effects and
    // Restoration's Purify Spirit (77130) removes Curse and Magic (Wowhead mop-classic tooltips 51886 / 77130,
    // 2026-10-04); Cure Poison / Cure Disease and Cleansing Totem are gone (cls.py shaman), so AC's poison and
    // disease edges (cleanse spirit ... on party, cleansing totem) are removed. Curse keeps AC's priority with
    // Purify Spirit first (it supersedes Cleanse Spirit for Restoration) and Cleanse Spirit as the fallback;
    // magic is the new Purify Spirit edge, self and party.
    triggers.push_back(new TriggerNode(
        "party member cleanse spirit curse",
        NextAction::array(0, new NextAction("purify spirit curse on party", ACTION_DISPEL + 2),
                          new NextAction("cleanse spirit curse on party", ACTION_DISPEL + 2), NULL)));
    triggers.push_back(new TriggerNode(
        "party member purify spirit magic",
        NextAction::array(0, new NextAction("purify spirit magic on party", ACTION_DISPEL + 2), NULL)));
    triggers.push_back(new TriggerNode(
        "purify spirit magic", NextAction::array(0, new NextAction("purify spirit", ACTION_DISPEL + 3), NULL)));

    // triggers.push_back(new TriggerNode(
    //     "no fire totem",
    //     NextAction::array(0, new NextAction("flametongue totem", 10.0f), NULL)));

    // triggers.push_back(new TriggerNode(
    //     "no water totem",
    //     NextAction::array(0, new NextAction("healing stream totem", 13.0f), NULL)));

    triggers.push_back(
        new TriggerNode("earth shield on main tank",
                        NextAction::array(0, new NextAction("earth shield on main tank", ACTION_HIGH + 7), NULL)));

    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("flee", ACTION_MOVE + 9), nullptr)));

    // #427 (AC parity, AC RestoShamanStrategy.cpp:54).
    triggers.push_back(
        new TriggerNode("water shield", NextAction::array(0, new NextAction("water shield", 19.5f), nullptr)));
    // Rotation fix C (#858, wave 73): Elemental Mastery is a class-wide 5.4.8 haste cooldown, the MoP rotation casts it whenever
    // ready (enhancement and restoration, condition "always"); AC has it only in the elemental strategy.
    triggers.push_back(new TriggerNode("elemental mastery",
                                       NextAction::array(0, new NextAction("elemental mastery", 29.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("mana tide totem", ACTION_HIGH + 5), NULL)));

    // #382 (AC RestoShamanStrategy.cpp has no fire-totem / fire elemental edge): fork-only "no fire totem ->
    // searing totem 7" and "fire elemental totem 32" removed (the "totems" strategy owns the fire slot, "boost" casts
    // Fire Elemental).
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 1), nullptr)));
}
