/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "BeastMasteryHunterStrategy.h"

#include "Playerbots.h"

// AC parity (AC Ai/Class/Hunter/Strategy/BeastMasteryHunterStrategy.cpp:14-18): no custom
// ActionNodeFactory is needed, so the ctor only forwards to GenericHunterStrategy.
BeastMasteryHunterStrategy::BeastMasteryHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
}

// ===== Default Actions =====
// AC parity: AC Ai/Class/Hunter/Strategy/BeastMasteryHunterStrategy.cpp:16-28 (same relative order;
// AC's bare 19.0f/5.xf weights are mapped onto this fork's ACTION_HIGH / ACTION_DEFAULT ladder the
// way DpsHunterStrategy.cpp:41-54 does).
NextAction** BeastMasteryHunterStrategy::getDefaultActions()
{
    return NextAction::array(
        0,
        // #380 (AC BeastMasteryHunterStrategy.cpp:21-28): AC literals - Bestial Wrath 19.0 (AC keeps it as a default,
        // below the ACTION_HIGH band; was ACTION_HIGH+1 = 21), then 5.7..5.1. Bestial Wrath 19574 is spec253 L40
        // (cls.py hunter). The 5.x filler spells exist in 5.4.8 (arcane 3044, steady 56641, aimed shot 19434 - 5.4.8
        // Aimed Shot is MM-only so on BM it falls through to the next default).
        new NextAction("bestial wrath", 19.0f),
        // #856 (epic #854) MoP BM order: Stampede, Serpent Sting, Glaive Toss, A Murder of
        // Crows, Dire Beast, Arcane Shot, Cobra Shot (77767). The Chimera Shot row 25307 is a data bug (MM-only spell
        // in the BM list) and is NOT ported. AC's Kill Command / Kill Shot stay on top.
        new NextAction("stampede", 5.95f),
        new NextAction("kill command", 5.9f),
        new NextAction("kill shot", 5.85f),
        new NextAction("serpent sting", 5.8f),
        new NextAction("glaive toss", 5.75f),
        new NextAction("a murder of crows", 5.7f),
        new NextAction("dire beast", 5.65f),
        // Rotation fix lane K (#868): AC's Aimed Shot default dropped - MM-only on 5.4.8 (see the note above; "no-spellid").
        new NextAction("arcane shot", 5.3f),
        new NextAction("cobra shot", 5.25f),
        new NextAction("steady shot", 5.2f),
        new NextAction("auto shot", 5.1f), nullptr);
}

// ===== Trigger Initialization =====
// AC parity: AC .../BeastMasteryHunterStrategy.cpp:31-82 (initializer-list TriggerNode ->
// NextAction::array on this fork, cf. DpsHunterStrategy.cpp:60-69).
void BeastMasteryHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    // AC parity: the "intimidation" action is registered as CastIntimidationAction.
    triggers.push_back(new TriggerNode("intimidation",
                                        NextAction::array(0, new NextAction("intimidation", 40.0f), nullptr)));
    // #856 Binding Shot (target 15-30 yd), between Serpent Sting and Stampede.
    triggers.push_back(new TriggerNode("binding shot range",
                                       NextAction::array(0, new NextAction("binding shot", 5.78f), nullptr)));
    triggers.push_back(new TriggerNode("kill command",
                                       NextAction::array(0, new NextAction("kill command", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("target critical health",
                                       NextAction::array(0, new NextAction("kill shot", 18.0f), nullptr)));
    // #521: "low mana -> viper sting" removed: Viper Sting is not learnable in 5.4.8 (cls.py hunter 2026-10-04).
    triggers.push_back(new TriggerNode("no stings",
                                       NextAction::array(0, new NextAction("serpent sting", 17.0f), nullptr)));
    triggers.push_back(
        new TriggerNode("serpent sting on attacker",
                        NextAction::array(0, new NextAction("serpent sting on attacker", 16.5f), nullptr)));
}
