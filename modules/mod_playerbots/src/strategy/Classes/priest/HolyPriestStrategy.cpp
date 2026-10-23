/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HolyPriestStrategy.h"

#include "Playerbots.h"

class HolyPriestStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    HolyPriestStrategyActionNodeFactory() { creators["smite"] = &smite; }

private:
    static ActionNode* smite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("smite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("shoot"), nullptr),
                              /*C*/ nullptr);
    }
};

HolyPriestStrategy::HolyPriestStrategy(PlayerbotAI* botAI) : HealPriestStrategy(botAI)
{
    actionNodeFactories.Add(new HolyPriestStrategyActionNodeFactory());
}

NextAction** HolyPriestStrategy::getDefaultActions()
{
    // #524 (AC HolyPriestStrategy.cpp:22-26): Mana Burn and Starshards are not learnable in 5.4.8 (cls.py priest
    // NOT-LEARNABLE), so only smite is kept.
    return NextAction::array(0, new NextAction("smite", ACTION_DEFAULT + 0.2f), nullptr);
}

void HolyPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    HealPriestStrategy::InitTriggers(triggers);

    triggers.push_back(
        new TriggerNode("holy fire", NextAction::array(0, new NextAction("holy fire", ACTION_NORMAL + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("shadowfiend", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));
    triggers.push_back(
        new TriggerNode("medium mana", NextAction::array(0, new NextAction("shadowfiend", ACTION_HIGH), nullptr)));
    // #524: AC "low mana -> mana burn" edge removed - Mana Burn is not learnable in 5.4.8 (cls.py).

    // Lane E (#863, w74: Chakra: Serenity / Sanctuary and Lightwell 0 casts): a solo or non-healer Holy priest runs this
    // "holy dps" strategy, not "holy heal", so the chakra stance and Lightwell edges below never reached it.
    // MoP rotation row 25724 / 25723 (prio 5/1, in combat): Serenity unless >= 3 group members are below 80 %,
    // then Sanctuary. MoP rotation row 25721 (5/1): Lightwell 126135 in combat, always (3 min cooldown).
    triggers.push_back(new TriggerNode(
        "chakra: serenity", NextAction::array(0, new NextAction("chakra: serenity", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode(
        "chakra: sanctuary", NextAction::array(0, new NextAction("chakra: sanctuary", ACTION_HIGH), nullptr)));
    triggers.push_back(new TriggerNode(
        "lightwell can cast", NextAction::array(0, new NextAction("lightwell", ACTION_HIGH - 1), nullptr)));
}

HolyHealPriestStrategy::HolyHealPriestStrategy(PlayerbotAI* botAI) : GenericPriestStrategy(botAI)
{
    actionNodeFactories.Add(new GenericPriestStrategyActionNodeFactory());
}

NextAction** HolyHealPriestStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("shoot", ACTION_DEFAULT), nullptr);
}

void HolyHealPriestStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericPriestStrategy::InitTriggers(triggers);

    // #812 MoP-only (no AC counterpart): keep Chakra: Serenity up, the way AC's shadow strategy keeps Shadowform up
    // (AC ShadowPriestStrategy.cpp:30-36). It replaces, for Holy, AC's "medium mana -> inner focus" edge
    // (AC GenericPriestStrategy.cpp:31-34), dead here: Inner Focus 89485 is Discipline-only in 5.4.8 (spec256).
    // Priority is above every heal so a once-per-fight stance is not starved by healing pressure.
    triggers.push_back(new TriggerNode(
        "chakra: serenity", NextAction::array(0, new NextAction("chakra: serenity", ACTION_CRITICAL_HEAL + 8), nullptr)));
    // Lane E (#863, MoP rotation row 25721 / 25717, prio 5/1): Lightwell 126135 in combat (was never wired).
    triggers.push_back(new TriggerNode(
        "lightwell can cast", NextAction::array(0, new NextAction("lightwell", ACTION_CRITICAL_HEAL + 7), nullptr)));

    triggers.push_back(
        new TriggerNode("group heal setting",
                        NextAction::array(0, 
                            new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 9),
                            new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 8), nullptr)));

    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        NextAction::array(0, new NextAction("divine hymn", ACTION_CRITICAL_HEAL + 7),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("circle of healing on party", ACTION_CRITICAL_HEAL + 5),
                          new NextAction("prayer of healing on party", ACTION_CRITICAL_HEAL + 4), nullptr)));

    // Lane 8 / #863 (MoP rotation row 25715/25723): Chakra: Sanctuary while >= 3 group members are below 80%
    // (ChakraSerenityTrigger stands down meanwhile), and Holy Word: Sanctuary (25716) once it is up.
    triggers.push_back(new TriggerNode(
        "chakra: sanctuary", NextAction::array(0, new NextAction("chakra: sanctuary", ACTION_CRITICAL_HEAL + 8), nullptr)));
    triggers.push_back(new TriggerNode(
        "medium group heal setting",
        NextAction::array(0, new NextAction("holy word: sanctuary on party", ACTION_CRITICAL_HEAL + 8), nullptr)));
    // Lane 8 / #863 (MoP rotation row 25603 disc / 25703 holy, group 1/10-1/12): the Surge of Light proc makes
    // Flash Heal free, so spend it on the hurt party member first.
    triggers.push_back(new TriggerNode(
        "surge of light proc", NextAction::array(0, new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 6), nullptr)));
    // Lane 8 / #863 (MoP rotation row 25607 disc / 25707 holy, group 1/9-1/11): Binding Heal when both the bot
    // and the target are hurt (BindingHealTrigger = party member low health + self below medium).
    triggers.push_back(new TriggerNode(
        "binding heal", NextAction::array(0, new NextAction("binding heal", ACTION_MEDIUM_HEAL + 5), nullptr)));

    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0, 
                          new NextAction("guardian spirit on party", ACTION_CRITICAL_HEAL + 6),
                          new NextAction("power word: shield on party", ACTION_CRITICAL_HEAL + 5),
                          // #812 MoP-only: Holy Word: Serenity (instant, 2% base mana) - AC has no counterpart.
                          new NextAction("holy word: serenity on party", ACTION_CRITICAL_HEAL + 4),
                          new NextAction("prayer of mending on party", ACTION_CRITICAL_HEAL + 3),
                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                          new NextAction("flash heal on party", ACTION_CRITICAL_HEAL + 1),
                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member low health",
                        NextAction::array(0,
                                          // #812 MoP-only: Holy Word: Serenity - AC has no counterpart.
                                          new NextAction("holy word: serenity on party", ACTION_MEDIUM_HEAL + 5),
                                          new NextAction("circle of healing on party", ACTION_MEDIUM_HEAL + 4),
                                          new NextAction("prayer of mending on party", ACTION_MEDIUM_HEAL + 3),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 2),
                                          new NextAction("flash heal on party", ACTION_MEDIUM_HEAL + 1), nullptr)));

    triggers.push_back(
        new TriggerNode("party member medium health",
                        NextAction::array(0,
                                          // #812 MoP-only: Holy Word: Serenity, above AC's greater heal - AC has no counterpart.
                                          new NextAction("holy word: serenity on party", ACTION_MEDIUM_HEAL + 6),
                                          new NextAction("circle of healing on party", ACTION_LIGHT_HEAL + 7),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 6),
                                          new NextAction("greater heal on party", ACTION_MEDIUM_HEAL + 5),
                                          new NextAction("flash heal on party", ACTION_LIGHT_HEAL + 4),
                                          // new NextAction("renew on party", ACTION_LIGHT_HEAL + 8),
                                          nullptr)));

    triggers.push_back(
        new TriggerNode("party member almost full health",
                        NextAction::array(0,
                                          new NextAction("renew on party", ACTION_LIGHT_HEAL + 2),
                                          new NextAction("prayer of mending on party", ACTION_LIGHT_HEAL + 1),
                                          nullptr)));

    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", ACTION_CRITICAL_HEAL + 10), nullptr)));
}
