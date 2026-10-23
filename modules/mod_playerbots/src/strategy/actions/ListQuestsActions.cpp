/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ListQuestsActions.cpp for issue #257 slice F.
 *
 * 5.4.8 / this-fork adaptations vs AC (all forced by the core, none cosmetic):
 *  - Quest has no GetTitle() on 5.4.8 - no title site is reached in this file (AC's only title read is
 *    inside ChatHelper::FormatQuest, already ported), so the GetLogTitle() mapping (QuestDef.h:365) that
 *    slices C/D/E applied has no site here.
 *  - chat->FormatQuest -> ChatHelper::FormatQuest: FormatQuest is static on this fork
 *    (AI/ChatHelper.h:42), so the AiObject::chat instance member is not used (same mapping as
 *    AcceptQuestAction.cpp:26-29 and DropQuestAction.cpp:21-23).
 *  - Player::GetQuestSlotQuestId(uint8) takes a uint16 slot on 5.4.8 (Player.h:1761), so AC's uint16 loop
 *    variable stays uint16 but is annotated the same way DropQuestAction.cpp:14-16 does.
 *  - AC's `TravelTarget* target = context->GetValue<TravelTarget*>("travel target")` plus the
 *    `target->getDestination()` / `target->getPosition()` / `QuestTravelDestination` block (AC
 *    ListQuestsActions.cpp:66-70, 90-105) is not ported: this fork has no TravelTarget subsystem and
 *    no quest travel destination class. Its "travel target" value is a deliberate permanent-null stub
 *    (strategy/value/TargetValue.h:108-116, "this fork has no travel target subsystem"), and its
 *    TravelMgr carries no destination half at all (Travel/TravelMgr.h:9-13). The getDestination() call
 *    would also have been unguarded against null in AC.
 *  - The QUEST_TRAVEL_DETAIL_SUMMARY and QUEST_TRAVEL_DETAIL_FULL branches (AC
 *    ListQuestsActions.cpp:107-180) are built entirely on TravelMgr::getQuestTravelDestinations() /
 *    TravelDestination, so they are not ported; the parameter is kept, the "travel" / "travel detail"
 *    keywords are still dispatched exactly as in AC so a future slice only has to fill the branches in.
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault(...) is not ported on this fork (#322); none of the
 *    strings below go through it - they are AC literals, unchanged.
 */

#include "ListQuestsActions.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include <sstream>

bool ListQuestsAction::Execute(Event event)
{
    if (event.getParam() == "completed" || event.getParam() == "co")
    {
        ListQuests(QUEST_LIST_FILTER_COMPLETED);
    }
    else if (event.getParam() == "incompleted" || event.getParam() == "in")
    {
        ListQuests(QUEST_LIST_FILTER_INCOMPLETED);
    }
    else if (event.getParam() == "all")
    {
        ListQuests(QUEST_LIST_FILTER_ALL);
    }
    else if (event.getParam() == "travel")
    {
        ListQuests(QUEST_LIST_FILTER_ALL, QUEST_TRAVEL_DETAIL_SUMMARY);
    }
    else if (event.getParam() == "travel detail")
    {
        ListQuests(QUEST_LIST_FILTER_ALL, QUEST_TRAVEL_DETAIL_FULL);
    }
    else
    {
        ListQuests(QUEST_LIST_FILTER_SUMMARY);
    }

    return true;
}

void ListQuestsAction::ListQuests(QuestListFilter filter, QuestTravelDetail travelDetail)
{
    bool showIncompleted = filter & QUEST_LIST_FILTER_INCOMPLETED;
    bool showCompleted = filter & QUEST_LIST_FILTER_COMPLETED;

    if (showIncompleted)
        botAI->TellMaster("--- Incompleted quests ---");

    uint32 incompleteCount = ListQuests(false, !showIncompleted, travelDetail);

    if (showCompleted)
        botAI->TellMaster("--- Completed quests ---");

    uint32 completeCount = ListQuests(true, !showCompleted, travelDetail);

    botAI->TellMaster("--- Summary ---");

    std::ostringstream out;
    out << "Total: " << (completeCount + incompleteCount) << " / 25 (incompleted: " << incompleteCount
        << ", completed: " << completeCount << ")";
    botAI->TellMaster(out);
}

uint32 ListQuestsAction::ListQuests(bool completed, bool silent, QuestTravelDetail travelDetail)
{
    // #257: AC's `TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();`
    // (AC ListQuestsActions.cpp:66-70) is not ported - this fork has no TravelTarget subsystem, its
    // "travel target" value is a permanent-null stub (strategy/value/TargetValue.h:108-116) and its
    // TravelMgr has no destination half (Travel/TravelMgr.h:9-13). See the file header.
    (void)travelDetail;

    uint32 count = 0;
    // 5.4.8: Player::GetQuestSlotQuestId takes a uint16 slot (Player.h:1761).
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* pQuest = sObjectMgr->GetQuestTemplate(questId);
        bool isCompletedQuest = bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE;
        if (completed != isCompletedQuest)
            continue;

        ++count;

        if (silent)
            continue;

        botAI->TellMaster(ChatHelper::FormatQuest(pQuest));

        if (travelDetail != QUEST_TRAVEL_DETAIL_NONE)
        {
            // #257: the "[Active] traveling ..." line (AC ListQuestsActions.cpp:90-105) is not ported -
            // it reads TravelTarget::getDestination() and casts it to QuestTravelDestination, neither of
            // which exists on this fork. See the file header.
        }

        if (travelDetail == QUEST_TRAVEL_DETAIL_SUMMARY)
        {
            // #257: the per-quest "N/M destinations P/Q points." line (AC ListQuestsActions.cpp:107-138)
            // is not ported - it is built on TravelMgr::getQuestTravelDestinations(), absent on 5.4.8.
            // See the file header.
        }
        else if (travelDetail == QUEST_TRAVEL_DETAIL_FULL)
        {
            // #257: the sorted per-destination listing (AC ListQuestsActions.cpp:139-180) is not ported -
            // same reason. See the file header.
        }
    }

    return count;
}