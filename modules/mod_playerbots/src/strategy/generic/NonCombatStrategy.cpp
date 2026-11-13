#include "NonCombatStrategy.h"

#include "Playerbots.h"
#include "Action.h"
#include "Multiplier.h"
#include "NewRpgInfo.h"
#include "PlayerbotAI.h"
#include "Player.h"

namespace
{
// A proactive pull (4.0) otherwise starves quest travel/turn-in (3.0) whenever
// another grind target is nearby. Let the existing quest action reach its POI;
// grinding resumes there for its objectives. Combat defense is unchanged.
class QuestTravelGrindingMultiplier : public Multiplier
{
public:
    explicit QuestTravelGrindingMultiplier(PlayerbotAI* ai) : Multiplier(ai, "quest travel grinding") { }

    float GetValue(Action* action) override
    {
        if (!action || action->getName() != "attack anything" || bot->IsInCombat() ||
            !botAI->HasStrategy("new rpg", BOT_STATE_NON_COMBAT))
            return 1.0f;

        NewRpgInfo const& info = botAI->GetRpgInfo();
        if (info.GetStatus() != RPG_DO_QUEST)
            return 1.0f;

        auto const* quest = std::get_if<NewRpgInfo::DoQuest>(&info.data);
        if (!quest)
            return 1.0f;

        QuestStatus const status = bot->GetQuestStatus(quest->questId);
        if (status == QUEST_STATUS_COMPLETE ||
            (status == QUEST_STATUS_INCOMPLETE && !quest->lastReachPOI))
            return 0.0f;

        return 1.0f;
    }
};
}

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

void GrindingStrategy::InitMultipliers(std::vector<Multiplier*>& multipliers)
{
    multipliers.push_back(new QuestTravelGrindingMultiplier(botAI));
}

void MoveRandomStrategy::InitTriggers(std::vector<TriggerNode*>& triggers)
{
    triggers.push_back(new TriggerNode("often", NextAction::array(0, new NextAction("move random", 1.5f), nullptr)));
}
