#include "NonCombatStrategy.h"

#include "Playerbots.h"

// #529 (AC Ai/Base/Strategy/NonCombatStrategy.cpp:9-13): the "nc" strategy carries both nodes. The
// mount check used to sit on MountStrategy; AC keeps it here, so it also runs where "mount" is removed
// (arena, AC AiFactory.cpp:711) - CheckMountStateAction::isUseful checks the "mount" marker itself.
void NonCombatStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("random", NextAction::array(0, new NextAction("clean quest log", 1.0f), nullptr)));
    triggers.push_back(new TriggerNode("timer", NextAction::array(0, new NextAction("check mount state", 1.0f), nullptr)));
}

// #529 (AC Ai/Base/Strategy/NonCombatStrategy.cpp:21-23): empty on AC - "mount" is only the marker
// strategy CheckMountStateAction::isUseful tests for.
void MountStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/)
{
}

// #156 (AC Ai/Base/Strategy/LootNonCombatStrategy.cpp:19-23): gathering goes through the loot stack like
// any other loot - "add gathering loot" queues a herb/ore node the bot can open, then "move to loot" /
// "open loot" (LootNonCombatStrategy) walk to it and cast Herb Gathering 2366 / Mining 2575 (both
// SPELL_EFFECT_OPEN_LOCK on 5.4.8). The old direct-walk "gather" action (GatherAction) is no longer
// scheduled by any strategy.
void GatherStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("timer", NextAction::array(0, new NextAction("add gathering loot", 5.0f), nullptr)));
}

// #527 (AC Ai/Base/Strategy/GrindingStrategy.cpp:10-16): a grinding bot tops up mana/health before its
// next pull (both are above "attack anything" 4.0).
NextAction** GrindingStrategy::getDefaultActions()
{
    return NextAction::array(0, new NextAction("drink", 4.2f), new NextAction("food", 4.1f), nullptr);
}

void GrindingStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    // AC: "reduce lower than loot"
    triggers.push_back(new TriggerNode("no target", NextAction::array(0, new NextAction("attack anything", 4.0f), nullptr)));
}

void MoveRandomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("move random", 1.5f), nullptr)));
}