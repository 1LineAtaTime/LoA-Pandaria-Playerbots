/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "CatDpsDruidStrategy.h"

#include "AiObjectContext.h"

class CatDpsDruidStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatDpsDruidStrategyActionNodeFactory()
    {
        creators["faerie fire (feral)"] = &faerie_fire_feral;
        creators["melee"] = &melee;
        creators["feral charge - cat"] = &feral_charge_cat;
        creators["cat form"] = &cat_form;
        creators["claw"] = &claw;
        creators["mangle (cat)"] = &mangle_cat;
        creators["rake"] = &rake;
        creators["ferocious bite"] = &ferocious_bite;
        creators["rip"] = &rip;
        creators["pounce"] = &pounce;
        creators["ravage"] = &ravage;
        // #363 (AC parity, Ai/Class/Druid/Action/DruidCatActions.h:112): Maim needs no
        // prerequisites, but it is registered so the node resolves inside this strategy.
        creators["maim"] = &maim;
    }

private:
    static ActionNode* faerie_fire_feral([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("faerie fire (feral)",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* melee([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("melee",
                              /*P*/ NextAction::array(0, new NextAction("feral charge - cat"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* feral_charge_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("feral charge - cat",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("reach melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* cat_form([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("cat form",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* claw([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("claw",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* mangle_cat([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("mangle (cat)",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* rake([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rake",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* ferocious_bite([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ferocious bite",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("rip"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* rip([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rip",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* pounce([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("pounce",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("ravage"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* ravage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("ravage",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("shred"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* maim([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("maim",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

CatDpsDruidStrategy::CatDpsDruidStrategy(PlayerbotAI* botAI) : FeralDruidStrategy(botAI)
{
    actionNodeFactories.Add(new CatDpsDruidStrategyActionNodeFactory());
}

NextAction** CatDpsDruidStrategy::getDefaultActions()
{
    // #595 (AC CatDruidStrategy.cpp:88-91): AC's whole default is plain "melee" at ACTION_DEFAULT. The fork-only
    // "tiger's fury" default is gone: Tiger's Fury (5217, spec103 L10) is on AC's "low energy" trigger at 27.0 below.
    return NextAction::array(0, new NextAction("melee", ACTION_DEFAULT), nullptr);
}

void CatDpsDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    FeralDruidStrategy::InitTriggers(triggers);

    // #363 (AC parity, Ai/Class/Druid/Strategy/CatDruidStrategy.cpp:100-106). AC hangs
    // "innervate on healer" off the shared "healer low mana" value
    // (strategy/triggers/TriggerContext.h:52) at a flat 35.0.
    triggers.push_back(new TriggerNode("healer low mana",
                                       NextAction::array(0, new NextAction("innervate on healer", 35.0f), nullptr)));

    // #363 (AC parity, Ai/Class/Druid/Strategy/CatDruidStrategy.cpp:151-157). AC drives Maim from
    // "combo points 5 available" at 23.0, just under Rip's 23.5. That trigger name exists in the
    // fork (strategy/triggers/TriggerContext.h:141), so it is used verbatim rather than folded
    // into the fork's own "combo points available" node that already carries Rip.
    triggers.push_back(new TriggerNode("combo points 5 available",
                                       NextAction::array(0, new NextAction("maim", 23.0f), nullptr)));

    // #384 (AC CatDruidStrategy.cpp:108-113): stealth openers. AC runs Ravage then Pounce just above Shred at
    // 5.6/5.5 on "almost full energy available" / "combo points not full". 5.4.8: both need Prowl (Ravage also
    // "behind the target") - Wowhead mop-classic 6785 + 9005, 2026-10-04 - and the existing ActionNode chain
    // pounce -> ravage -> shred means that outside stealth the cast simply falls through to Shred (the line below).
    // Ravage 6785 is spec103/104 level 54, Pounce 9005 level 32 (cls.py druid).
    // Rotation fix lane A (#860), MoP rotation row 10316 (spec 103, prio group 10/0 "prowl pounce"): the stealth opener
    // is Pounce (9005), Ravage is not in MoP's cat list. w73: Ravage (0.6) won the stealth tick every time, so Pounce
    // (0.5) was only ever tried unstealthed (checkcast result=98 ONLY_STEALTHED) or on the GCD (70). Pounce now sits above
    // Ravage; its ActionNode alternative chain (pounce -> ravage -> shred) still falls through outside stealth.
    triggers.push_back(new TriggerNode("almost full energy available",
                                       NextAction::array(0, new NextAction("pounce", ACTION_DEFAULT + 0.6f), nullptr)));
    triggers.push_back(new TriggerNode("combo points not full",
                                       NextAction::array(0, new NextAction("pounce", ACTION_DEFAULT + 0.6f), nullptr)));
    triggers.push_back(new TriggerNode("almost full energy available",
                                       NextAction::array(0, new NextAction("ravage", ACTION_DEFAULT + 0.5f), nullptr)));
    triggers.push_back(new TriggerNode("combo points not full",
                                       NextAction::array(0, new NextAction("ravage", ACTION_DEFAULT + 0.5f), nullptr)));

    // Default priority
    triggers.push_back(new TriggerNode("almost full energy available",
                                       NextAction::array(0, new NextAction("shred", ACTION_DEFAULT + 0.4f), nullptr)));
    triggers.push_back(new TriggerNode("combo points not full",
                                       NextAction::array(0, new NextAction("shred", ACTION_DEFAULT + 0.4f), nullptr)));
    triggers.push_back(new TriggerNode("almost full energy available",
                                       NextAction::array(0, new NextAction("mangle (cat)", ACTION_DEFAULT + 0.3f), nullptr)));
    triggers.push_back(new TriggerNode("combo points not full and high energy",
                                       NextAction::array(0, new NextAction("mangle (cat)", ACTION_DEFAULT + 0.3f), nullptr)));
    // #384: AC's "claw" builder edges (AC CatDruidStrategy.cpp:196-208) are dropped: Claw is NOT-LEARNABLE in 5.4.8
    // (cls.py druid: ids 16827/24187/...). Its 5.x replacement as the basic cat builder is Shred (5221) - already
    // wired at ACTION_DEFAULT + 0.4 above - so nothing is re-mapped here.
    triggers.push_back(
        new TriggerNode("faerie fire (feral)",
                        NextAction::array(0, new NextAction("faerie fire (feral)", ACTION_DEFAULT + 0.0f), nullptr)));

    // #384 (AC CatDruidStrategy.cpp:108-141): AC's own cat edges that the fork was missing. Priorities are AC's
    // literals; each spell is learnable in 5.4.8 (cls.py druid): Prowl 5215 level 6, Tiger's Fury 5217 spec103,
    // Shred 5221 spec103, Ferocious Bite 22568. 5.x Clearcasting is 135700/16870 (Wowhead mop-classic 135700,
    // 2026-10-04: next cost-bearing druid ability costs nothing), same aura name the "clearcasting" trigger tests.
    triggers.push_back(
        new TriggerNode("prowl", NextAction::array(0, new NextAction("prowl", 29.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("low energy", NextAction::array(0, new NextAction("tiger's fury", 27.0f), nullptr)));
    triggers.push_back(new TriggerNode("clearcasting", NextAction::array(0, new NextAction("shred", 24.5f), nullptr)));
    triggers.push_back(new TriggerNode("ferocious bite execute",
                                       NextAction::array(0, new NextAction("ferocious bite", 24.0f), nullptr)));
    // #384 (AC GenericDruidStrategy.cpp:62-66): Predatory Swiftness (69369) makes Rebirth instant and castable in
    // cat form; AC ranks that combat-res above the plain one for non-bear feral druids (plain edge is in
    // GenericDruidStrategy at ACTION_HIGH + 9 = 29, so this one ties it rather than outranking by 0.5).
    triggers.push_back(new TriggerNode("predator's swiftness and combat party member dead",
                                       NextAction::array(0, new NextAction("rebirth", 29.0f), nullptr)));

    // Rotation lane 5 (#860), MoP rotation row 26 (spec 103, prio group 10/2): the cat interrupts the target's
    // cast with Skull Bash (106839; the bear already has the "bash" edge, BearTankDruidStrategy.cpp). In a battleground or
    // arena the shared "enemy caster in range" edge (lane 0 #855) also interrupts any casting enemy (isUseful gate in
    // CastSkullBashOnEnemyCasterAction).
    triggers.push_back(
        new TriggerNode("bash", NextAction::array(0, new NextAction("bash", ACTION_INTERRUPT + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "enemy caster in range", NextAction::array(0, new NextAction("skull bash on enemy caster", ACTION_INTERRUPT + 1), nullptr)));
    // Rotation lane 5 (#860), MoP rotation row 10311 / 10310 (spec 103, prio group 6/2 and 6/0): the free
    // Predatory Swiftness (69369) Healing Touch at health <= 35% (emergency) and <= 90%. Group 6 sits under the group 10
    // interrupt and above the group 5 cooldowns / group 2 finishers: 39.5 / 25.2.
    triggers.push_back(new TriggerNode("predator's swiftness heal emergency",
                                       NextAction::array(0, new NextAction("healing touch", 39.5f), nullptr)));
    triggers.push_back(new TriggerNode("predator's swiftness heal",
                                       NextAction::array(0, new NextAction("healing touch", 25.2f), nullptr)));

    // Main spell
    triggers.push_back(
        new TriggerNode("cat form", NextAction::array(0, new NextAction("cat form", ACTION_HIGH + 8), nullptr)));
    // #595 (AC CatDruidStrategy.cpp:122-176): AC numbers. Savage Roar 26.0; Rip on "combo points 5 available" 23.5
    // (the fork-only bare "combo points available" edge was the same 5-CP trigger at 26, above AC's order); Ferocious
    // Bite on "ferocious bite time" 22.5 (the 24.0 "ferocious bite execute" edge is above); Mangle 22.0; Rake 21.5.
    // The fork-only "target with combo points almost dead" Ferocious Bite edge (24) is dropped: AC's
    // "ferocious bite execute" (24.0) already covers the finisher-on-dying-target case.
    triggers.push_back(
        new TriggerNode("savage roar", NextAction::array(0, new NextAction("savage roar", 26.0f), nullptr)));
    triggers.push_back(new TriggerNode("combo points 5 available",
                                       NextAction::array(0, new NextAction("rip", 23.5f), nullptr)));
    triggers.push_back(new TriggerNode(
        "ferocious bite time", NextAction::array(0, new NextAction("ferocious bite", 22.5f), nullptr)));
    triggers.push_back(new TriggerNode("mangle (cat)",
                                       NextAction::array(0, new NextAction("mangle (cat)", 22.0f), nullptr)));
    // Wave-74 fix G (#860), MoP rotation row 10303 / 10304 / 10305 (group 1: Rake 1/5, Thrash 1/4, Mangle 1/3): in 5.4.8
    // Mangle applies no "mangle" debuff, so the "mangle (cat)" trigger above (a DebuffTrigger on that name) is true on
    // every tick and its 22.0 edge starved Rake (21.5) and Thrash (21.4): w74 cat leg, "PUSH:rake" 40x / "PUSH:thrash"
    // 47x, mangle (cat) OK 11x, rake / thrash never cast. Rake and Thrash now sit just above Mangle (22.0) and below
    // Ferocious Bite time (22.5), the MoP order; both refresh at <= 1.5 s left (RakeTrigger / ThrashCatTrigger).
    triggers.push_back(new TriggerNode("rake", NextAction::array(0, new NextAction("rake", 22.3f), nullptr)));
    // Rotation fix lane A (#860), MoP rotation row 10304 (spec 103, prio group 1/4): Thrash (106832, cat) right under
    // Rake (1/5) and above Mangle (1/3 in MoP 5.4.8; here Mangle keeps AC's 22.0 trigger edge). w73: no cat edge existed
    // (the "thrash" action/trigger were bear-only), so the spell was never pushed. "thrash (cat)" is a plain own-debuff
    // trigger (no Weakened Blows gate, unlike the bear ThrashTrigger); the action name "thrash" resolves to 106832 by name.
    triggers.push_back(new TriggerNode("thrash (cat)", NextAction::array(0, new NextAction("thrash", 22.2f), nullptr)));
    // Wave-74 fix G (#860), MoP rotation row 10316 (group 10/0): Pounce is the first cast out of Prowl. w74: the
    // Pounce edge sat at 5.6, under Thrash (21.4) / Savage Roar (26), so a prowling cat never reached it (A:pounce
    // IMPOSSIBLE = the GCD of the Prowl cast only). Above Prowl's own 29.5 and Feral Charge 29, under the group 10 interrupts.
    triggers.push_back(
        new TriggerNode("prowl active", NextAction::array(0, new NextAction("pounce", 29.8f), nullptr)));
    // Wave-74 fix G (#860), MoP rotation row 10308 (group 5/0, in combat "always"): Tiger's Fury whenever its aura is
    // down (the spell's own 30 s cooldown rate-limits it). Just under Berserk (27.5) like the other group 5 cooldowns.
    triggers.push_back(
        new TriggerNode("tiger's fury", NextAction::array(0, new NextAction("tiger's fury", 27.2f), nullptr)));
    // #384: AC's "medium threat -> cower" edge (AC CatDruidStrategy.cpp:183-187) is dropped: Cower is NOT-LEARNABLE
    // in 5.4.8 (cls.py druid: ids 1742/83175, no class skill-line row) and has no 5.x successor.

    // AOE
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("swipe (cat)", ACTION_HIGH + 3), nullptr)));
    triggers.push_back(new TriggerNode(
        "light aoe", NextAction::array(0, new NextAction("rake on attacker", ACTION_HIGH + 2), nullptr)));
    // Reach target
    triggers.push_back(new TriggerNode(
        "enemy out of melee", NextAction::array(0, new NextAction("feral charge - cat", ACTION_HIGH + 9), nullptr)));
    triggers.push_back(
        new TriggerNode("enemy out of melee", NextAction::array(0, new NextAction("dash", ACTION_HIGH + 8), nullptr)));
}

void CatAoeDruidStrategy::InitTriggers(std::vector<TriggerNode*>& triggers) {}

// #408 (AC parity, Ai/Class/Druid/Strategy/CatDruidStrategy.cpp:280-378). Additive overlay - only
// the healing triggers, designed to be stacked on top of "cat".
class CatOffhealStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    CatOffhealStrategyActionNodeFactory()
    {
        creators["healing touch on party"] = &healing_touch_on_party;
        creators["regrowth on party"] = &regrowth_on_party;
        creators["rejuvenation on party"] = &rejuvenation_on_party;
    }

private:
    // P: shift to caster form before casting   C: shift back to cat form afterwards
    static ActionNode* healing_touch_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("healing touch on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }

    static ActionNode* regrowth_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("regrowth on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }

    static ActionNode* rejuvenation_on_party([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rejuvenation on party",
                              /*P*/ NextAction::array(0, new NextAction("caster form"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ NextAction::array(0, new NextAction("cat form"), nullptr));
    }
};

CatOffhealStrategy::CatOffhealStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI)
{
    actionNodeFactories.Add(new CatOffhealStrategyActionNodeFactory());
}

void CatOffhealStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode(
        "party member critical health",
        NextAction::array(0,
                          new NextAction("regrowth on party", 36.0f),
                          new NextAction("healing touch on party", 35.0f),
                          nullptr)));
    triggers.push_back(new TriggerNode(
        "party member low health",
        NextAction::array(0, new NextAction("healing touch on party", 25.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member medium health",
        NextAction::array(0, new NextAction("rejuvenation on party", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode(
        "party member to heal out of spell range",
        NextAction::array(0, new NextAction("reach party member to heal", 93.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("low mana", NextAction::array(0, new NextAction("innervate", 24.0f), nullptr)));
}
