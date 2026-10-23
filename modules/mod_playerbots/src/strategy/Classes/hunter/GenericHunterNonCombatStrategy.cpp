/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "GenericHunterNonCombatStrategy.h"

#include "Playerbots.h"

class GenericHunterNonCombatStrategyActionNodeFactory : public NamedObjectFactory<ActionNode>
{
public:
    GenericHunterNonCombatStrategyActionNodeFactory()
    {
        creators["aspect of the pack"] = &aspect_of_the_pack;
    }

private:
    // #521: the rapid fire -> readiness fallback node was removed (Readiness is not learnable in 5.4.8,
    // cls.py hunter 2026-10-04); no non-combat trigger casts rapid fire anyway.
    static ActionNode* aspect_of_the_pack([[maybe_unused]] PlayerbotAI* botAI)
    {
        return new ActionNode("aspect of the pack",
                              /*P*/ nullptr,
                              /*A*/ NextAction::array(0, new NextAction("aspect of the cheetah"), nullptr),
                              /*C*/ nullptr);
    }
};

GenericHunterNonCombatStrategy::GenericHunterNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI)
{
    actionNodeFactories.Add(new GenericHunterNonCombatStrategyActionNodeFactory());
}

void GenericHunterNonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    NonCombatStrategy::InitTriggers(triggers);

    // #163 S8 / #147 (AC parity, GenericHunterNonCombatStrategy.cpp:31): apply the configured stance
    // to a freshly tamed/resummoned hunter pet.
    triggers.push_back(
        new TriggerNode("new pet", NextAction::array(0, new NextAction("set pet stance", 60.0f), nullptr)));
    // Rotation fix lane K (#868, BEYOND AC): "drop target" leaves the combat engine; in a running duel nothing re-targets the
    // opponent there (AC DuelStrategy "no attackers" never fires, the opponent is always in "attackers"), see HunterTriggers.h.
    triggers.push_back(new TriggerNode("hunter duel retarget",
                                       NextAction::array(0, new NextAction("attack duel opponent", 70.0f), nullptr)));

    triggers.push_back(new TriggerNode("trueshot aura", NextAction::array(0, new NextAction("trueshot aura", 2.0f), nullptr)));
    // #434 (AC GenericHunterNonCombatStrategy.cpp:20-21): stones and oils still exist in 5.4.8 (capped by
    // weapon item level), so this edge is ported 1:1 (ImbueAction.cpp).
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("apply stone", 1.0f),
                                                                  new NextAction("apply oil", 1.0f), nullptr)));
    // 5.4.8: ammo removed in Cataclysm, no MoP equivalent (#149).
    // triggers.push_back(new TriggerNode("low ammo", NextAction::array(0, new NextAction("say::low ammo", ACTION_NORMAL), nullptr)));
    triggers.push_back(new TriggerNode("no track", NextAction::array(0, new NextAction("track humanoids", ACTION_NORMAL), nullptr)));
    // triggers.push_back(new TriggerNode("no ammo", NextAction::array(0, new NextAction("switch to melee",
    // ACTION_NORMAL + 1), new NextAction("say::no ammo", ACTION_NORMAL), nullptr))); triggers.push_back(new
    // TriggerNode("has ammo", NextAction::array(0, new NextAction("switch to ranged", ACTION_NORMAL), nullptr)));
}

void HunterPetStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("no pet", NextAction::array(0, new NextAction("call pet 1", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("has pet", NextAction::array(0, new NextAction("toggle pet spell", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet medium health", NextAction::array(0, new NextAction("mend pet", 60.0f), nullptr)));
    triggers.push_back(new TriggerNode("hunters pet dead", NextAction::array(0, new NextAction("revive pet", 60.0f), nullptr)));
}
