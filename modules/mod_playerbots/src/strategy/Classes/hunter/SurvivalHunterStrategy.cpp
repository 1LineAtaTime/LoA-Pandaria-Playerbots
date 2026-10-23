/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

#include "SurvivalHunterStrategy.h"

#include "Playerbots.h"

class SurvivalHunterStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    SurvivalHunterStrategyActionNodeFactory()
    {
        creators["explosive shot rank 4"] = &explosive_shot_rank_4;
        creators["explosive shot rank 3"] = &explosive_shot_rank_3;
        creators["explosive shot rank 2"] = &explosive_shot_rank_2;
    }

private:
    static ActionNode* explosive_shot_rank_4([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 4",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("explosive shot rank 3"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* explosive_shot_rank_3([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 3",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("explosive shot rank 2"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* explosive_shot_rank_2([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("explosive shot rank 2",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("explosive shot rank 1"), nullptr),
                              /*C*/ nullptr);
    }
};

SurvivalHunterStrategy::SurvivalHunterStrategy(PlayerbotAI* botAI) : GenericHunterStrategy(botAI)
{
    actionNodeFactories.Add(new SurvivalHunterStrategyActionNodeFactory());
}

// ===== Default Actions =====
// AC parity: AC Ai/Class/Hunter/Strategy/SurvivalHunterStrategy.cpp:51-64 (same relative order; weights
// mapped onto this fork's ACTION_HIGH / ACTION_DEFAULT ladder, cf. DpsHunterStrategy.cpp:41-54).
NextAction** SurvivalHunterStrategy::getDefaultActions()
{
    return NextAction::array(
        0,
        // #380 (AC SurvivalHunterStrategy.cpp:51-64): AC literals 5.9..5.1. Explosive Shot 53301 / Black Arrow 3674 are
        // spec255 (Survival) in 5.4.8 (cls.py hunter).
        // #856 (epic #854) MoP SV order: Stampede, Binding Shot, Glaive Toss, A Murder of
        // Crows, Dire Beast, Serpent Sting, Explosive Shot, Kill Shot, Black Arrow, Arcane/Multi, Cobra Shot (77767).
        // AC's order below is kept; the MoP-only spells sit above it.
        new NextAction("stampede", 5.98f),
        new NextAction("glaive toss", 5.96f),
        new NextAction("a murder of crows", 5.94f),
        new NextAction("dire beast", 5.92f),
        new NextAction("kill command", 5.9f),
        new NextAction("kill shot", 5.8f),
        new NextAction("explosive shot", 5.7f),
        new NextAction("black arrow", 5.6f),
        new NextAction("serpent sting", 5.5f),
        // Rotation fix lane K (#868): AC's Aimed Shot default dropped - MM-only on 5.4.8 (the MoP SV list has none; "no-spellid").
        new NextAction("arcane shot", 5.3f),
        new NextAction("cobra shot", 5.25f),
        new NextAction("steady shot", 5.2f),
        new NextAction("auto shot", 5.1f), nullptr);
}

// ===== Trigger Initialization =====
// AC parity: AC .../SurvivalHunterStrategy.cpp:67-135.
void SurvivalHunterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericHunterStrategy::InitTriggers(triggers);

    // AC parity: the SurvivalHunterStrategyActionNodeFactory above provides the rank fallback chain.
    triggers.push_back(new TriggerNode("lock and load",
                                        NextAction::array(0, new NextAction("explosive shot rank 4", 28.0f), nullptr)));
    // #856 Binding Shot (15-30 yd), just under Stampede.
    triggers.push_back(new TriggerNode("binding shot range",
                                       NextAction::array(0, new NextAction("binding shot", 5.97f), nullptr)));
    triggers.push_back(new TriggerNode("kill command",
                                       NextAction::array(0, new NextAction("kill command", 18.5f), nullptr)));
    triggers.push_back(new TriggerNode("target critical health",
                                       NextAction::array(0, new NextAction("kill shot", 18.0f), nullptr)));
    triggers.push_back(new TriggerNode("explosive shot",
                                       NextAction::array(0, new NextAction("explosive shot", 17.5f), nullptr)));
    triggers.push_back(new TriggerNode("black arrow",
                                       NextAction::array(0, new NextAction("black arrow", 16.5f), nullptr)));
    // #521: "low mana -> viper sting" removed: Viper Sting is not learnable in 5.4.8 (cls.py hunter 2026-10-04).
    triggers.push_back(new TriggerNode("no stings",
                                       NextAction::array(0, new NextAction("serpent sting", 15.5f), nullptr)));
    triggers.push_back(
        new TriggerNode("serpent sting on attacker",
                        NextAction::array(0, new NextAction("serpent sting on attacker", 15.0f), nullptr)));
}
