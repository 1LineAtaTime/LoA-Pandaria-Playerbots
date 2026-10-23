/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "FireMageStrategy.h"

#include "Playerbots.h"

/*
* Rune of power is cast on target not self
*/

class FireMageStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    FireMageStrategyActionNodeFactory()
    {
        creators["fireball"] = &fireball;
        creators["dragon's breath"] = &dragon_breath;
        creators["inferno blast"] = &inferno_blast;
        creators["pyroblast"] = &pyroblast;
        creators["scorch"] = &scorch;
        creators["flamestrike"] = &flamestrike;
        creators["living bomb"] = &living_bomb;
        creators["living bomb on attackers"] = &living_bomb_on_attackers;
    }

private:
    static ActionNode* fireball([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("fireball",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* dragon_breath([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("dragon's breath",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* inferno_blast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("inferno blast",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* pyroblast([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("pyroblast",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    static ActionNode* scorch([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("scorch",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
    // Rotation fix lane B (#865): Living Bomb 44457 and Nether Tempest 114923 are two choices of the same level-75 talent
    // row (see FrostMageStrategy.cpp frost_bomb, #790). A fire bot that took Nether Tempest got 35x "living bomb -
    // IMPOSSIBLE" (w73 fire leg) and never applied a bomb (prio 0/10 in the MoP fire list).
    static ActionNode* living_bomb([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("living bomb",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("nether tempest"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* living_bomb_on_attackers([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("living bomb on attackers",
            /*P*/ nullptr,
            /*A*/ NextAction::array(0, new NextAction("nether tempest on attackers"), nullptr),
            /*C*/ nullptr);
    }
    static ActionNode* flamestrike([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("flamestrike",
            /*P*/ nullptr,
            /*A*/ nullptr,
            /*C*/ nullptr);
    }
};

FireMageStrategy::FireMageStrategy(PlayerbotAI* botAI)
    : GenericMageStrategy(botAI)
{
    actionNodeFactories.Add(new FireMageStrategyActionNodeFactory());
}

NextAction** FireMageStrategy::getDefaultActions()
{
    return NextAction::array(0,
        new NextAction("frostjaw", ACTION_DEFAULT + 0.10f),
        new NextAction("combustion", ACTION_DEFAULT + 0.9f),
        new NextAction("inferno blast", ACTION_DEFAULT + 0.8f), // cast during movement
        new NextAction("living bomb", ACTION_DEFAULT + 0.7f),
        new NextAction("fireball", ACTION_DEFAULT + 0.6f),
        new NextAction("scorch", ACTION_DEFAULT), nullptr);
}

void FireMageStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    GenericMageStrategy::InitTriggers(triggers);

    triggers.push_back(new TriggerNode("medium health", NextAction::array(0, new NextAction("ice barrier", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("heating up", NextAction::array(0, new NextAction("inferno blast", ACTION_INTERRUPT), nullptr)));
    triggers.push_back(new TriggerNode("pyroblast!", NextAction::array(0, new NextAction("pyroblast", ACTION_INTERRUPT + 10), nullptr)));
    triggers.push_back(new TriggerNode("combustion", NextAction::array(0, new NextAction("combustion", 50.0f), nullptr)));
    triggers.push_back(new TriggerNode("living bomb", NextAction::array(0, new NextAction("living bomb", 19.0f), nullptr)));
    // #385 (AC Ai/Class/Mage/Strategy/FireMageStrategy.cpp:60-67): Dragon's Breath when an enemy is too close for a cast
    triggers.push_back(new TriggerNode("enemy too close for spell",
                                       NextAction::array(0, new NextAction("dragon's breath", ACTION_INTERRUPT + 1), nullptr)));
}

void FireMageAoeStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(
        new TriggerNode("medium aoe", NextAction::array(0,
            new NextAction("dragon's breath", 24.0f),
            new NextAction("flamestrike", 23.0f), 
            new NextAction("living bomb on attackers", 21.0f), 
            new NextAction("blizzard", 20.0f), nullptr)));
}

// #385 (AC Ai/Class/Mage/Strategy/FireMageStrategy.cpp:78-95): combat strategy that runs to melee for Blast Wave.
// Disabled by default for the Fire/Frostfire spec: "co +firestarter" / "co -firestarter". While it is on, the
// "... and no firestarter strategy" edges (frost nova / blink back, GenericMageStrategy.cpp) stay quiet.
// 5.4.8: Blast Wave is not a learnable mage spell (cls.py mage -> NOT-LEARNABLE), so "blast wave off cd" is false
// (SpellNoCooldownTrigger returns false for spell id 0, GenericTriggers.cpp:304-311) and the edge is dormant, like AC's
// code for a mage without the talent.
FirestarterStrategy::FirestarterStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}

void FirestarterStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("blast wave off cd and medium aoe",
                                       NextAction::array(0, new NextAction("reach melee", 25.5f), nullptr)));
}
