/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericDruidStrategy.h"

#include "Playerbots.h"

class GenericDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericDruidStrategyActionNodeFactory()
    {
        creators["melee"] = &melee;
        creators["caster form"] = &caster_form;
        creators["cure poison"] = &cure_poison;
        creators["cure poison on party"] = &cure_poison_on_party;
        creators["abolish poison"] = &abolish_poison;
        creators["abolish poison on party"] = &abolish_poison_on_party;
        creators["rebirth"] = &rebirth;
        creators["entangling roots on cc"] = &entangling_roots_on_cc;
        creators["innervate"] = &innervate;
    }

private:
    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* caster_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("caster form",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* cure_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* cure_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cure poison on party",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* abolish_poison([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* abolish_poison_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("abolish poison on party",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* rebirth([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rebirth",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* entangling_roots_on_cc([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("entangling roots on cc",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* innervate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("innervate",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("mana potion"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericDruidStrategy::GenericDruidStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericDruidStrategyActionNodeFactory());
}

void GenericDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    CombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, GenericDruidStrategy.cpp:103): apply the configured stance to a
    // freshly summoned treant.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));

    triggers.push_back(
        new TriggerNode("low health", NextAction::array(0, new NextAction("barkskin", ACTION_HIGH + 7), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 1100 (class row, every druid spec, prio group 5/0): Might of
    // Ursoc (106922) at health <= 25%. ACTION_EMERGENCY, just under the feral "critical health" Survival Instincts
    // (EMERGENCY + 1); beyond AC (WotLK has no such spell). A non-bear spec leaves the form again through the
    // shapeshift prerequisites of its next spell node (MoP row 1107 "cancel Bear Form").
    triggers.push_back(new TriggerNode("might of ursoc",
                                       NextAction::array(0, new NextAction("might of ursoc", ACTION_EMERGENCY), nullptr)));

    // triggers.push_back(new TriggerNode("low health", NextAction::array(0, new NextAction("regrowth",
    // ACTION_MEDIUM_HEAL + 2), nullptr))); triggers.push_back(new TriggerNode("party member low health",
    // NextAction::array(0, new NextAction("regrowth on party", ACTION_MEDIUM_HEAL + 1), nullptr)));
    // triggers.push_back(new TriggerNode("critical health", NextAction::array(0, new NextAction("regrowth",
    // ACTION_CRITICAL_HEAL + 2), new NextAction("healing touch", ACTION_CRITICAL_HEAL + 2), nullptr)));
    // triggers.push_back(new TriggerNode("party member critical health", NextAction::array(0,  new NextAction("regrowth
    // on party", ACTION_CRITICAL_HEAL + 1), new NextAction("healing touch on party", ACTION_CRITICAL_HEAL + 1),
    // nullptr))); triggers.push_back(new TriggerNode("party member dead", NextAction::array(0, new
    // NextAction("rebirth", ACTION_HIGH + 1), nullptr))); triggers.push_back(new TriggerNode("low mana",
    // NextAction::array(0, new NextAction("innervate", ACTION_EMERGENCY + 5), nullptr)));
    triggers.push_back(new TriggerNode("combat party member dead",
                                       NextAction::array(0, new NextAction("rebirth", ACTION_HIGH + 9), NULL)));

    // #384 (AC GenericDruidStrategy.cpp:58): AC keeps Barkskin on "almost full health" at a flat 40.0 for every
    // druid spec; the fork's own "low health -> barkskin" edge above stays. Barkskin (22812) is learnable at
    // level 44 in 5.4.8 (cls.py druid).
    triggers.push_back(
        new TriggerNode("almost full health", NextAction::array(0, new NextAction("barkskin", 40.0f), nullptr)));

    // #384 (AC GenericDruidStrategy.cpp:79-80): Nature's Grasp (16689, level 52, 1 min cooldown - Wowhead
    // mop-classic 16689, 2026-10-04) when something is hitting the bot. "being attacked" is the shared trigger
    // (TriggerContext.h, #521).
    triggers.push_back(
        new TriggerNode("being attacked", NextAction::array(0, new NextAction("nature's grasp", 39.0f), nullptr)));
}

void DruidCureStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // triggers.push_back(new TriggerNode("cure poison", NextAction::array(0, new NextAction("abolish poison",
    // ACTION_DISPEL + 2), nullptr)));
    triggers.push_back(
        new TriggerNode("party member cure poison",
                        NextAction::array(0, new NextAction("abolish poison on party", ACTION_DISPEL + 1), nullptr)));
    
    triggers.push_back(
        new TriggerNode("party member remove curse",
                        NextAction::array(0, new NextAction("remove curse on party", ACTION_DISPEL + 7), NULL)));
    
}

void DruidBoostStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "nature's swiftness", NextAction::array(0, new NextAction("nature's swiftness", ACTION_HIGH + 9), nullptr)));

    // #384 (AC GenericDruidStrategy.cpp:91-105): spec-gated boosts. AC's tabs: Balance -> Force of Nature + treant
    // stance, Feral (bear + cat) -> Berserk. 5.4.8: Force of Nature is the 106737 talent, Berserk 106952 is
    // spec103/104 (cls.py druid); Guardian is its own spec here and, like AC's Feral tab, gets Berserk.
    Player* bot = botAI->GetBot();
    Specializations const spec = bot->GetSpecialization();

    if (spec == SPEC_DRUID_BALANCE)
    {
        triggers.push_back(
            new TriggerNode("force of nature", NextAction::array(0, new NextAction("force of nature", 29.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
        // Rotation lane 5 (#860), MoP rotation row 10213/10218 (Celestial Alignment) and 10214 (Nature's
        // Vigil), spec 102, prio group 5/1: the Balance cooldowns, on "boost" next to Force of Nature (29.0).
        triggers.push_back(new TriggerNode("celestial alignment",
                                           NextAction::array(0, new NextAction("celestial alignment", 29.0f), nullptr)));
        triggers.push_back(new TriggerNode("nature's vigil",
                                           NextAction::array(0, new NextAction("nature's vigil", 28.9f), nullptr)));
    }

    if (spec == SPEC_DRUID_GUARDIAN)
    {
        // Rotation lane 5 (#860), MoP rotation row 10410 (spec 104, prio group 5/1): Nature's Vigil.
        triggers.push_back(new TriggerNode("nature's vigil",
                                           NextAction::array(0, new NextAction("nature's vigil", 28.9f), nullptr)));
    }

    if (spec == SPEC_DRUID_FERAL || spec == SPEC_DRUID_GUARDIAN)
    {
        // Rotation fix lane A (#860), MoP rotation row 1103 (class row, prio group 4/0): Force of Nature for cat and bear
        // too (AC gates it to the balance tab above). Group 4 sits under the group 5 cooldowns: 27.0 < Berserk 27.5.
        // w73: the cat knew 106737 but no cat/bear edge existed. Talent: bots without it fail "not-known" (situational).
        triggers.push_back(
            new TriggerNode("force of nature", NextAction::array(0, new NextAction("force of nature", 27.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("berserk", NextAction::array(0, new NextAction("berserk", 27.5f), nullptr)));
    }
}

void DruidCcStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // #384 (AC GenericDruidStrategy.cpp:122-134): a cat-form feral druid only crowd-controls while Predatory
    // Swiftness is up, so it does not leave cat form. 5.4.8: Predatory Swiftness (69369) makes the next Entangling
    // Roots / Healing Touch / Hibernate / Rebirth instant, free and castable in all forms (Wowhead mop-classic 69369,
    // 2026-10-04). It does NOT cover Cyclone, so AC's "predator's swiftness and cyclone" edge is deliberately not
    // ported (it would shift the cat out of form for a normal cast with nothing gained).
    Player* bot = botAI->GetBot();
    bool const catFeral =
        bot->GetSpecialization() == SPEC_DRUID_FERAL && bot->HasSpell(768) /*cat form*/ && !bot->HasAura(16931) /*thick hide*/;

    if (catFeral)
    {
        triggers.push_back(new TriggerNode(
            "predator's swiftness and hibernate", NextAction::array(0, new NextAction("hibernate on cc", 41.0f), nullptr)));
        triggers.push_back(new TriggerNode(
            "predator's swiftness and entangling roots",
            NextAction::array(0, new NextAction("entangling roots on cc", 40.0f), nullptr)));
    }
    else
    {
        triggers.push_back(new TriggerNode(
            "entangling roots", NextAction::array(0, new NextAction("entangling roots on cc", ACTION_HIGH + 2), nullptr)));
        triggers.push_back(new TriggerNode(
            "hibernate", NextAction::array(0, new NextAction("hibernate on cc", ACTION_HIGH + 3), nullptr)));
        // #163 S8 (AC parity, GenericDruidStrategy.cpp:120,131,140).
        triggers.push_back(
            new TriggerNode("cyclone", NextAction::array(0, new NextAction("cyclone on cc", ACTION_HIGH + 2), nullptr)));
    }
    // #624 (AC GenericDruidStrategy.cpp:109-146): AC registers the "entangling roots kite" trigger (DruidAiObjectContext.cpp:102)
    // but never attaches an edge to it (grep Ai/Class/Druid: no strategy uses it). The fork-only
    // "entangling roots kite" -> "entangling roots" (HIGH+2) edge fired every tick on any mana-less target (wave 11: T x47
    // on the boss dummy, root cast attempts crowding the rotation), so it is removed.
}

void DruidHealerDpsStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("healer should attack",
                        NextAction::array(0,
                            new NextAction("cancel tree form", ACTION_DEFAULT + 0.3f),
                            new NextAction("moonfire", ACTION_DEFAULT + 0.2f),
                            new NextAction("wrath", ACTION_DEFAULT + 0.1f),
                            new NextAction("starfire", ACTION_DEFAULT),
                            nullptr)));

    // long cast time
    // triggers.push_back(
    //     new TriggerNode("medium aoe and healer should attack",
    //                     NextAction::array(0,
    //                         new NextAction("hurricane", ACTION_DEFAULT + 0.7f),
    //                         nullptr)));
}

// #408 (AC parity, Ai/Class/Druid/Strategy/GenericDruidStrategy.cpp:161-189). AC's single "aoe"
// strategy. On MoP it coexists with "caster aoe"/"cat aoe": Engine::ProcessTriggers() evaluates a
// trigger once and then pushes the handlers of *every* TriggerNode bound to that trigger, so a
// duplicated trigger name here just contributes extra handlers instead of overriding the others.
//
// AC's first line, "hurricane channel check" -> NextAction("cancel channel", 22.0f) (AC :168/:178),
// is NOW ported (see the balance/feral blocks below): #423 added the no-longer-missing
// CancelChannelAction (strategy/actions/CancelChannelAction.h, creators["cancel channel"]).
void DruidAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    Player* bot = botAI->GetBot();
    Specializations const spec = bot->GetSpecialization();

    if (spec == SPEC_DRUID_BALANCE)
    {
        triggers.push_back(
            new TriggerNode("starfall", NextAction::array(0, new NextAction("starfall", 28.5f), nullptr)));
        // #423 (AC parity, AC :168) - "cancel channel" resolves since the action was ported.
        triggers.push_back(new TriggerNode("hurricane channel check",
            NextAction::array(0, new NextAction("cancel channel", 22.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("medium aoe", NextAction::array(0, new NextAction("hurricane", 23.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("enemy within melee", NextAction::array(0, new NextAction("typhoon", 40.0f), nullptr)));
        triggers.push_back(new TriggerNode(
            "insect swarm on attacker",
            NextAction::array(0, new NextAction("insect swarm on attacker", 5.2f), nullptr)));
        triggers.push_back(new TriggerNode(
            "moonfire on attacker", NextAction::array(0, new NextAction("moonfire on attacker", 5.1f), nullptr)));
    }

    if (spec == SPEC_DRUID_RESTORATION)
    {
        // #423 (AC parity, AC :178)
        triggers.push_back(new TriggerNode("hurricane channel check",
            NextAction::array(0, new NextAction("cancel channel", 22.0f), nullptr)));
        triggers.push_back(
            new TriggerNode("medium aoe", NextAction::array(0, new NextAction("hurricane", 23.0f), nullptr)));
        triggers.push_back(new TriggerNode(
            "insect swarm on attacker",
            NextAction::array(0, new NextAction("insect swarm on attacker", 5.2f), nullptr)));
        triggers.push_back(new TriggerNode(
            "moonfire on attacker", NextAction::array(0, new NextAction("moonfire on attacker", 5.1f), nullptr)));
    }

    if (spec == SPEC_DRUID_FERAL && bot->HasSpell(768) /*cat form*/ && !bot->HasAura(16931) /*thick hide*/)
    {
        triggers.push_back(new TriggerNode("clearcasting and medium aoe",
                                           NextAction::array(0, new NextAction("swipe (cat)", 25.5f), nullptr)));
        triggers.push_back(new TriggerNode(
            "medium aoe", NextAction::array(0, new NextAction("swipe (cat)", 25.0f), nullptr)));
    }
}
