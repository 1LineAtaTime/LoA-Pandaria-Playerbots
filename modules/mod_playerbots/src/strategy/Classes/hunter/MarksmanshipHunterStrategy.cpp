/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "MarksmanshipHunterStrategy.h"

#include "Playerbots.h"

// AC parity (AC Ai/Class/Hunter/Strategy/MarksmanshipHunterStrategy.cpp:14-18): no custom
// ActionNodeFactory is needed, so the ctor only forwards to GenericHunterStrategy.
MarksmanshipHunterStrategy::MarksmanshipHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
}

// ===== Default Actions =====
// AC parity: AC Ai/Class/Hunter/Strategy/MarksmanshipHunterStrategy.cpp:16-28 (same relative order;
// weights mapped onto this fork's ACTION_HIGH / ACTION_DEFAULT ladder, cf. DpsHunterStrategy.cpp:41-54).
NextAction** MarksmanshipHunterStrategy::getDefaultActions()
{
    return NextAction::array(
        0,
        // #380 (AC MarksmanshipHunterStrategy.cpp:19-26): AC literals 5.8..5.1.
        // #856 (epic #854) MoP MM order: Stampede, Binding Shot, Glaive Toss, A Murder of
        // Crows, Steady Shot (Steady Focus refresh), Chimera, Aimed, Kill Shot. AC's Kill Command / order below is kept.
        new NextAction("stampede", 5.95f),
        new NextAction("glaive toss", 5.9f),
        new NextAction("a murder of crows", 5.85f),
        new NextAction("kill command", 5.8f),
        new NextAction("kill shot", 5.7f),
        new NextAction("serpent sting", 5.6f),
        new NextAction("chimera shot", 5.5f),
        new NextAction("aimed shot", 5.4f),
        new NextAction("arcane shot", 5.3f),
        new NextAction("steady shot", 5.2f),
        new NextAction("auto shot", 5.1f), nullptr);
}

// ===== Trigger Initialization =====
// AC parity: AC .../MarksmanshipHunterStrategy.cpp:31-83.
void MarksmanshipHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("silencing shot",
                                       NextAction::array(0, new NextAction("silencing shot", 40.0f), nullptr)));
    // #856 Binding Shot (15-30 yd) and Steady Shot while Steady Focus <= 3 s.
    triggers.push_back(new TriggerNode("binding shot range",
                                       NextAction::array(0, new NextAction("binding shot", 5.92f), nullptr)));
    // Rotation fix lane A (#856): Steady Shot (prio 0/7) sits BELOW A Murder of Crows (0/8). The edge was
    // 5.875, above Crows' 5.85, so w73 (MM leg) cast Steady Shot on 20 ticks and Crows was only reached 3x (GCD). 5.84 = just under Crows.
    triggers.push_back(new TriggerNode("steady focus low",
                                       NextAction::array(0, new NextAction("steady shot", 5.84f), nullptr)));
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