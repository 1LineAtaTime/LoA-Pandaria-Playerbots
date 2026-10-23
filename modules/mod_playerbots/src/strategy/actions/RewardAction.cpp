/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/RewardAction.cpp for #258. 5.4.8 differences:
 *  - AC's `AI_VALUE(Unit*, "group leader")` value does not exist on this fork; botAI->GetGroupMaster() is the
 *    established mapping (the group leader as a Player*).
 *  - Player::RewardQuest / CanRewardQuest take the reward ITEM ID on 5.x (QuestHandler.cpp:463 "reward value is now
 *    an item ID and not slot ID"), not AC's slot index, so itemId is passed through.
 *  - Quest::RewardChoiceItemId is a std::array here, GetRewChoiceItemsCount() is unchanged.
 */

#include "RewardAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "QuestDef.h"   // QuestGiverStatus, used by GossipDef.h
#include "GossipDef.h"  // QuestMenu / QuestMenuItem / PlayerMenu
#include "ObjectMgr.h"
#include "Playerbots.h"

bool RewardAction::Execute(Event event)
{
    std::string const link = event.getParam();

    ItemIds itemIds = chat->parseItems(link);
    if (itemIds.empty())
        return false;

    uint32 itemId = *itemIds.begin();

    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    for (ObjectGuid const guid : npcs)
    {
        if (Unit* npc = botAI->GetUnit(guid))
            if (Reward(itemId, npc))
                return true;
    }

    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (ObjectGuid const guid : gos)
    {
        if (GameObject* go = botAI->GetGameObject(guid))
            if (Reward(itemId, go))
                return true;
    }

    Unit* groupLeaderUnit = botAI->GetGroupMaster();
    if (groupLeaderUnit && Reward(itemId, groupLeaderUnit))
        return true;

    botAI->TellError("Cannot talk to quest giver");
    return false;
}

bool RewardAction::Reward(uint32 itemId, Object* questGiver)
{
    QuestMenu& questMenu = bot->PlayerTalkClass->GetQuestMenu();
    for (uint32 iI = 0; iI < questMenu.GetMenuItemCount(); ++iI)
    {
        QuestMenuItem const& qItem = questMenu.GetItem(iI);

        uint32 questID = qItem.QuestId;
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(questID);
        if (!pQuest)
            continue;

        QuestStatus status = bot->GetQuestStatus(questID);

        // if quest is complete, turn it in
        if (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questID) &&
            pQuest->GetRewChoiceItemsCount() > 1 && bot->CanRewardQuest(pQuest, itemId, false))
        {
            for (uint8 rewardIdx = 0; rewardIdx < pQuest->GetRewChoiceItemsCount(); ++rewardIdx)
            {
                ItemTemplate const* pRewardItem = sObjectMgr->GetItemTemplate(pQuest->RewardChoiceItemId[rewardIdx]);
                if (pRewardItem && itemId == pRewardItem->ItemId)
                {
                    bot->RewardQuest(pQuest, pRewardItem->ItemId, questGiver, false);

                    std::ostringstream out;
                    out << chat->FormatItem(pRewardItem) << " rewarded";
                    botAI->TellMaster(out);

                    return true;
                }
            }
        }
    }

    return false;
}
