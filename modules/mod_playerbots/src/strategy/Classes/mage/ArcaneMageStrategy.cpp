/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ArcaneMageStrategy.h"

#include "Playerbots.h"

class ArcaneMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    ArcaneMageStrategyActionNodeFactory()
    {
        creators["arcane blast"] = &arcane_blast;
        creators["arcane barrage"] = &arcane_barrage;
        creators["slow"] = &slow;
        creators["nether tempest"] = &nether_tempest;
    }

private:
    static ActionNode* arcane_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("arcane blast",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("arcane missiles"), nullptr),
                              /*C*/ nullptr);
    }

    static ActionNode* arcane_barrage([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("arcane barrage",
                              /*P*/ nullptr,
                              /*A*/ nullptr,
                              /*C*/ nullptr);
    }

    static ActionNode* slow([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("slow",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }

    static ActionNode* nether_tempest([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("nether tempest",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
};

ArcaneMageStrategy::ArcaneMageStrategy(PlayerbotAI* botAI) : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new ArcaneMageStrategyActionNodeFactory());
}

NextAction** ArcaneMageStrategy::getDefaultActions()
{
    // #602 (AC ArcaneMageStrategy.cpp:37-46): AC defaults, same order and 5.x-band numbers. Fork-only
    // "frostjaw" / "nether tempest" defaults removed (they sat above the band and AC has neither; Nether Tempest
    // keeps its own "nether tempest" DebuffTrigger edge below). 5.4.8 ids (cls.py mage): Arcane Blast 30451,
    // Arcane Missiles 5143 (proc-gated by "Arcane Missiles!" 79683, a failed cast falls through as in AC),
    // Arcane Barrage 44425, Fire Blast 2136, Frostbolt 116, Shoot 5019.
    return NextAction::array(0,
        new NextAction("arcane blast", 5.6f),
        new NextAction("arcane missiles", 5.5f),
        new NextAction("arcane barrage", 5.4f),   // cast while moving
        new NextAction("fire blast", 5.3f),       // cast while moving if arcane barrage isn't available/learned
        new NextAction("frostbolt", 5.2f),        // for arcane immune targets
        new NextAction("shoot", 5.1f), nullptr);
}

void ArcaneMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    // #602 (AC ArcaneMageStrategy.cpp:55-64): the only AC spec edge - 4 charges + missiles proc -> Arcane Missiles.
    // 5.4.8: charges = Arcane Charge 36032, proc = "Arcane Missiles!" 79683 (see MageTriggers.h).
    // Removed fork-only edges (AC has none): "slow not on target" -> slow ACTION_NORMAL+1 (Slow 31589 "succeeded"
    // every tick on a snare-immune target and ended the tick, starving Arcane Blast), the "arcane charge" /
    // "arcane missiles" / "arcane blast stack" -> missiles/barrage 15.0 edges (fired at 1 charge and starved the
    // default Arcane Blast) and the spec-level "arcane power" 19.0 edge (AC casts it from the "boost" strategy).
    triggers.push_back(new TriggerNode("arcane blast 4 stacks and missile barrage", NextAction::array(0, new NextAction("arcane missiles", 15.0f), nullptr)));
    // Lane 10 (#865): Arcane Barrage at >= 4 Arcane Charges, just below Missiles.
    triggers.push_back(new TriggerNode("arcane blast stack", NextAction::array(0, new NextAction("arcane barrage", 14.5f), nullptr)));
    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("ice barrier", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("nether tempest", NextAction::array(0, new NextAction("nether tempest", 19.0f), nullptr)));
}

void ArcaneMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("medium aoe", NextAction::array(0,
        new NextAction("nether tempest on attackers", 24.0f),
        new NextAction("blizzard", 23.0f), nullptr)));
}