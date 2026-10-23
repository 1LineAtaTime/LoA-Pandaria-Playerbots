/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FrostDKStrategy.h"

#include "Playerbots.h"

class FrostDKStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FrostDKStrategyActionNodeFactory()
    {
        creators["icy touch"] = &icy_touch;
        creators["obliterate"] = &obliterate;
        creators["howling blast"] = &howling_blast;
        creators["frost strike"] = &frost_strike;
        // creators["chains of ice"] = &chains_of_ice;
        creators["rune strike"] = &rune_strike;
        // creators["icy clutch"] = &icy_clutch;
        // creators["horn of winter"] = &horn_of_winter;
        // creators["killing machine"] = &killing_machine;
        // creators["frost presence"] = &frost_presence;
        // creators["deathchill"] = &deathchill;
        // creators["icebound fortitude"] = &icebound_fortitude;
        // creators["mind freeze"] = &mind_freeze;
    }

private:
    // #601 MoP-only divergence (AC FrostDKStrategy.cpp:28,38,48,58,68 use "blood presence" - WotLK dps used Blood
    // Presence): in 5.4.8 Frost dps uses Frost Presence 48266 (Wowhead mop-classic tooltips 48263/48266/48265 and
    // Wowpedia "Presence", read 2026-10-05), so every Frost prerequisite below is "frost presence".
    static ActionNode* icy_touch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("icy touch",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* obliterate([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("obliterate",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* rune_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("rune strike",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ NextAction::array(0, new NextAction("melee"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* frost_strike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("frost strike",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* howling_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("howling blast",
                              /*P*/ NextAction::array(0, new NextAction("frost presence"), nullptr),
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }
};

FrostDKStrategy::FrostDKStrategy(PlayerbotAI* botAI) : GenericDKStrategy(botAI)
{
    actionNodeFactories.Add(new FrostDKStrategyActionNodeFactory());
}

NextAction** FrostDKStrategy::getDefaultActions()
{
    // #601 (AC FrostDKStrategy.cpp:91-96): AC's defaults. The fork-only "empower rune weapon" (.3) default is
    // dropped; AC casts it from the "no rune" trigger (GenericDKStrategy.cpp:77-82), which the fork has too.
    return NextAction::array(
        // rot-fix-F (wave 74): Obliterate and Death Strike cost the same Frost+Unholy rune pair and DS (DEFAULT+0.71,
        // "always") won every open GCD, so Obliterate (MoP rotation row 25110, the 5.4.8 Frost rune spender, AC FrostDKStrategy.cpp
        // :92) was never cast (17 NOT_READY rejects, 0 casts). Obliterate now sits above Death Strike; Death Strike (25109)
        // is kept as the self-heal edge "dk health 90" below (and as the fallback when Obliterate is not castable).
        0, new NextAction("obliterate", ACTION_DEFAULT + 0.72f),
        // rot-lane-2 (#857): MoP rotation row 25105 Soul Reaper (prio 8, above Plague Strike / Obliterate), 25109 Death
        // Strike (prio 6, above Obliterate prio 5).
        new NextAction("soul reaper", ACTION_DEFAULT + 0.75f),
        new NextAction("death strike", ACTION_DEFAULT + 0.7f),
        new NextAction("frost strike", ACTION_DEFAULT + 0.4f),
        new NextAction("horn of winter", ACTION_DEFAULT + 0.1f), new NextAction("melee", ACTION_DEFAULT), NULL);
}

void FrostDKStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericDKStrategy::InitTriggers(triggers);

    // #601 (AC FrostDKStrategy.cpp:103-110 "unbreakable armor" -> DEFAULT+0.6): Unbreakable Armor does not exist
    // in 5.4.8 (cls.py dk: NO-SUCH-NAME; Wowpedia "Unbreakable Armor" marks it gone since 4.0.1). Its documented
    // 5.x successor is Pillar of Frost 51271 (Frost, L68, 1 min cd, +20% Strength - the same buff; Wowhead
    // mop-classic tooltip 51271, read 2026-10-05), so the edge casts that at AC's number.
    triggers.push_back(new TriggerNode(
        "pillar of frost", NextAction::array(0, new NextAction("pillar of frost", ACTION_DEFAULT + 1.05f), nullptr)));
    // ^ rot-lane-2 (#857): MoP rotation row 25115 Pillar of Frost is a group 5 cooldown, cast on cooldown ahead of the
    // rotation (was DEFAULT+0.6, below Obliterate).

    // 25108 Howling Blast on a Freezing Fog proc (prio 6, above Obliterate prio 5; was DEFAULT+0.5).
    triggers.push_back(new TriggerNode(
        "freezing fog", NextAction::array(0, new NextAction("howling blast", ACTION_DEFAULT + 0.72f), nullptr)));
    // 25101/25102 Frost Strike at runic power >= 89 or with Killing Machine (prio 10).
    triggers.push_back(new TriggerNode(
        "dk frost strike proc", NextAction::array(0, new NextAction("frost strike", ACTION_DEFAULT + 0.95f), nullptr)));
    // 25103/25104 Howling Blast with two frost (or death) runes ready (prio 9).
    triggers.push_back(new TriggerNode(
        "high frost rune", NextAction::array(0, new NextAction("howling blast", ACTION_DEFAULT + 0.9f), nullptr)));
    // rot-fix-F: 25109 Death Strike when hurt (health <= 90, same threshold as MoP rotation row 25016 Vampiric Blood); above
    // Obliterate so the heal is not starved by it. Uses the "dk health 90" trigger registered in DKAiObjectContext.cpp.
    triggers.push_back(new TriggerNode(
        "dk health 90", NextAction::array(0, new NextAction("death strike", ACTION_DEFAULT + 0.8f), nullptr)));
    // 25020-style in-combat Raise Dead (25114, group 5), once per fight (see BloodDKStrategy).
    triggers.push_back(
        new TriggerNode("raise dead", NextAction::array(0, new NextAction("raise dead", ACTION_NORMAL + 1), nullptr)));

    triggers.push_back(new TriggerNode(
        "high blood rune", NextAction::array(0, new NextAction("blood strike", ACTION_DEFAULT + 0.2f), nullptr)));

    triggers.push_back(new TriggerNode(
        "army of the dead", NextAction::array(0, new NextAction("army of the dead", ACTION_HIGH + 6), nullptr)));

    // 25106 Howling Blast when Frost Fever is missing (prio 7); Icy Touch stays as AC's fallback.
    triggers.push_back(new TriggerNode(
        "icy touch", NextAction::array(0, new NextAction("howling blast", ACTION_HIGH + 2),
                                       new NextAction("icy touch", ACTION_HIGH + 2), nullptr)));
    triggers.push_back(new TriggerNode(
        "plague strike", NextAction::array(0, new NextAction("plague strike", ACTION_HIGH + 2), nullptr)));
    // triggers.push_back(new TriggerNode("empower rune weapon", NextAction::array(0, new NextAction("empower rune
    // weapon", ACTION_NORMAL + 4), nullptr)));
}

void FrostDKAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0, new NextAction("howling blast", ACTION_HIGH + 4), nullptr)));
}
