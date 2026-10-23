/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TalkToQuestGiverAction.cpp for issue #257 slice C (the
 * master-commanded quest layer).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - AC's RewardQuest()/CanRewardQuest() take a reward *choice index*; on 5.4.8 the second argument is the
 *    reward *item id* (Player::RewardQuest(quest, reward, ...) / Player::CanRewardQuest(quest, rewardId, msg),
 *    Player.h:1712/1718) - see the existing fork caller strategy/RPG/NewRpgBaseAction.cpp:567-579. Every
 *    `quest->RewardChoiceItemId[index]` is therefore resolved to its item id at each call site.
 *  - Quest::GetRewChoiceItemsCount() returns uint32 here (QuestDef.h:437; AC returns uint8), so the choice
 *    loops use uint32 instead of AC's uint8. AC's BestRewards() is declared `ItemIds` but defined as
 *    `std::set<uint32>`; the fork header declares std::set<uint32> on both sides (same type, ItemVisitors.h:12).
 *  - AC's pQuest->GetTitle() -> pQuest->GetLogTitle() (QuestDef.h:365); Quest has no GetTitle() on 5.4.8.
 *  - The four sObjectMgr->GetItemTemplate() + FormatItem() pairs AC spreads across the reward helpers go
 *    through the file-local FormatItemOrEmpty() below: ChatHelper::FormatItem() (ChatHelper.cpp:80-97)
 *    dereferences the template, and a reward item missing from the world DB would take the worldserver down
 *    in the middle of a turn-in. Output is byte-identical for every item that has a template.
 *  - AI_VALUE2(ItemUsage, "item usage", id) is passed the item id as std::to_string(id): the fork's
 *    GetValue<T>(name, int32 param) overload (AiObjectContext.h:46) would narrow the uint32 id, and the
 *    string form is what every other ported caller uses (LootRollAction.cpp:269, TradeValues.cpp:81).
 *  - IsSelfBot(master) is not ported on this fork; the AC condition `!masterBotAI || IsSelfBot(master)`
 *    (TalkToQuestGiverAction.cpp:30) becomes the file-local IsRealPlayer() helper below, which is the exact
 *    body of the fork's PlayerbotAI::HasRealPlayerMaster() (PlayerbotAI.cpp:2053-2054).
 *  - botAI->IsAltBot() / HasGameClientMaster() are not ported on this fork (see the established mapping
 *    noted in TaxiAction.cpp:14); the file-local IsAltBot() helper below reconstructs
 *    PlayerbotAI::IsAltBot() (AC PlayerbotAI.cpp:4653) from HasRealPlayerMaster() + sRandomPlayerbotMgr.
 *  - BroadcastHelper (AC Bot/Broadcast/BroadcastHelper.h) is not ported on this fork (issue #322); each
 *    AC broadcast call below is replaced by a single marker comment at the same site.
 *  - PlayerbotTextMgr::instance().GetBotText(): not ported on this fork; the literals below are exactly the
 *    default enUS texts of the three ai_playerbot_texts rows AC would fetch (AC data/sql/playerbots/base/
 *    ai_playerbot_texts.sql:1318-1320, ids 1298-1300), with the %quest / %item placeholders substituted.
 *  - sPlayerbotAIConfig.x -> sPlayerbotAIConfig->x (pointer singleton on this fork).
 *  - TurnInQueryQuestAction (AC .cpp:245-306) is ported at the bottom of this file. AC reads
 *    "ObjectGuid guid; uint32 questId" off SMSG_QUESTGIVER_OFFER_REWARD; 5.4.8's
 *    PlayerMenu::SendQuestGiverOfferReward (GossipDef.cpp:848-1021) writes 72 dwords (questId is the 2nd),
 *    then bit-packed lengths + guid mask bits, then strings and masked guid bytes, so the guid is rebuilt
 *    bit by bit. A quest missing from the template store (AC dereferences it) returns false instead.
 */

#include "TalkToQuestGiverAction.h"

#include "BroadcastHelper.h"
#include "ChatHelper.h"
#include "Event.h"
#include "DBCStructure.h"
#include "ItemPrototype.h"
#include "ItemUsageValue.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "RandomPlayerbotMgr.h"
#include "StatsWeightCalculator.h"
#include <sstream>

namespace
{
// AC Bot/PlayerbotAI.cpp:4653 - `return HasGameClientMaster() && !sRandomPlayerbotMgr.IsRandomBot(bot) &&
// !IsSelfBot(bot);`. The fork has neither HasGameClientMaster() nor IsSelfBot(); HasGameClientMaster() maps
// to HasRealPlayerMaster() (TaxiAction.cpp:14) and IsSelfBot(bot) is always false for the bot's own AI, so
// only the random-bot test survives.
bool IsAltBot(PlayerbotAI* botAI)
{
    return botAI->HasRealPlayerMaster() && !sRandomPlayerbotMgr->IsRandomBot(botAI->GetBot());
}

// AC Bot/PlayerbotAI.cpp:4594 `IsSelfBot(player)`, established fork mapping (`IsSelfBot(bot)` ->
// `botAI->IsRealPlayer()`, see e.g. Manager/PlayerbotSecurity.cpp:13). Same body as the fork's own
// PlayerbotAI::HasRealPlayerMaster() (PlayerbotAI.cpp:2053-2054).
bool IsRealPlayer(Player* master)
{
    PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(master);
    return !masterBotAI || masterBotAI->IsRealPlayer();
}

// AC Bot/Cmd/ChatHelper.cpp:432-457 dereferences the template without a null check; AC's callers below
// fetch the template with sObjectMgr->GetItemTemplate() and can pass null for a reward item that is missing
// from the world DB, which would take the worldserver down mid turn-in. FormatItemOrEmpty() keeps AC's
// output for every item that does have a template and degrades to an empty string for one that does not.
std::string FormatItemOrEmpty(uint32 itemId)
{
    ItemTemplate const* item = sObjectMgr->GetItemTemplate(itemId);
    return item ? ChatHelper::FormatItem(item) : std::string();
}

// AC's GetBotText("quest_status_completed") with args["%quest"] (data/sql/playerbots/base/
// ai_playerbot_texts.sql:1319, id 1299) - see the file header.
std::string QuestStatusCompleted(std::string const quest)
{
    return "I have completed the quest " + quest;
}

// AC's GetBotText("quest_status_unable_to_complete") with args["%quest"] (ai_playerbot_texts.sql:1318,
// id 1298) - see the file header.
std::string QuestStatusUnableToComplete(std::string const quest)
{
    return "I am unable to turn in the quest " + quest;
}

// AC's GetBotText("quest_status_complete_single_reward") with args["%quest"]/args["%item"]
// (ai_playerbot_texts.sql:1320, id 1300) - see the file header.
std::string QuestStatusCompleteSingleReward(std::string const quest, std::string const item)
{
    return "I have completed the quest " + quest + " and received " + item;
}
}

bool TalkToQuestGiverAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    bool isCompleted = false;
    std::ostringstream out;
    out << "Quest ";

    QuestStatus status = bot->GetQuestStatus(quest->GetQuestId());
    Player* master = GetMaster();

    if (sPlayerbotAIConfig->syncQuestForPlayer && master)
    {
        if (IsRealPlayer(master))
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                isCompleted |= CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        if (master && master->GetQuestStatus(quest->GetQuestId()) == QUEST_STATUS_COMPLETE &&
            (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_FAILED))
        {
            isCompleted |= CompleteQuest(bot, quest->GetQuestId());
            status = bot->GetQuestStatus(quest->GetQuestId());
        }
    }

    switch (status)
    {
        case QUEST_STATUS_COMPLETE:
            isCompleted |= TurnInQuest(quest, questGiver, out);
            break;
        case QUEST_STATUS_INCOMPLETE:
            out << "|cffff0000Incompleted|r";
            break;
        case QUEST_STATUS_NONE:
            out << "|cff00ff00Available|r";
            break;
        case QUEST_STATUS_FAILED:
            out << "|cffff0000Failed|r";
            break;
        default:
            break;
    }

    out << ": " << chat->FormatQuest(quest);
    botAI->TellMaster(out);

    return isCompleted;
}

bool TalkToQuestGiverAction::TurnInQuest(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    uint32 questID = quest->GetQuestId();

    if (bot->GetQuestRewardStatus(questID))
        return false;

    bot->PlayDistanceSound(621);

    if (quest->GetRewChoiceItemsCount() == 0)
        RewardNoItem(quest, questGiver, out);
    else if (quest->GetRewChoiceItemsCount() == 1)
        RewardSingleItem(quest, questGiver, out);
    else
    {
        RewardMultipleItem(quest, questGiver, out);
    }

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) ||
        botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(questID);
        if (pQuest)
        {
            std::string const text_quest = ChatHelper::FormatQuest(pQuest);
            // 5.4.8: AC's pQuest->GetTitle() -> pQuest->GetLogTitle() (QuestDef.h:365) - see the file header.
            TC_LOG_INFO("playerbots", "{} => Quest [ {} ] completed", bot->GetName(), pQuest->GetLogTitle());
            bot->Say("Quest [ " + text_quest + " ] completed", LANG_UNIVERSAL);
        }
    }

    return true;
}

void TalkToQuestGiverAction::RewardNoItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    if (bot->CanRewardQuest(quest, false))
    {
        out << QuestStatusCompleted(chat->FormatQuest(quest));
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);  // AC TalkToQuestGiverAction.cpp:109/130

        // 5.4.8: reward 0 still means "the fixed (non-choice) rewards only" - Player::RewardQuest(),
        // Player.cpp, guards `reward` against RewChoiceItemId[], same as AC.
        bot->RewardQuest(quest, 0, questGiver, false);
    }
    else
    {
        out << QuestStatusUnableToComplete(chat->FormatQuest(quest));
    }
}

void TalkToQuestGiverAction::RewardSingleItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    uint32 index = 0;
    uint32 itemId = quest->RewardChoiceItemId[index];

    // 5.4.8: CanRewardQuest/RewardQuest take the reward item id, not the choice index - see the file header.
    if (bot->CanRewardQuest(quest, itemId, false))
    {
        out << QuestStatusCompleteSingleReward(chat->FormatQuest(quest), FormatItemOrEmpty(itemId));
        BroadcastHelper::BroadcastQuestTurnedIn(botAI, bot, quest);  // AC TalkToQuestGiverAction.cpp:109/130
        bot->RewardQuest(quest, itemId, questGiver, true);
    }
    else
    {
        out << QuestStatusUnableToComplete(chat->FormatQuest(quest));
    }
}

std::set<uint32> TalkToQuestGiverAction::BestRewards(Quest const* quest)
{
    std::set<uint32> returnIds;
    ItemUsage bestUsage = ITEM_USAGE_NONE;
    if (quest->GetRewChoiceItemsCount() == 0)
        return returnIds;
    else if (quest->GetRewChoiceItemsCount() == 1)
        return {0};
    else
    {
        for (uint32 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", std::to_string(quest->RewardChoiceItemId[i]));
            if (usage == ITEM_USAGE_EQUIP || usage == ITEM_USAGE_REPLACE)
                bestUsage = ITEM_USAGE_EQUIP;
            else if (usage == ITEM_USAGE_BAD_EQUIP && bestUsage != ITEM_USAGE_EQUIP)
                bestUsage = usage;
            else if (usage != ITEM_USAGE_NONE && bestUsage == ITEM_USAGE_NONE)
                bestUsage = usage;
        }
        for (uint32 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
        {
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", std::to_string(quest->RewardChoiceItemId[i]));
            if (usage == bestUsage || usage == ITEM_USAGE_REPLACE)
                returnIds.insert(i);
        }
        return returnIds;
    }
}

void TalkToQuestGiverAction::RewardMultipleItem(Quest const* quest, Object* questGiver, std::ostringstream& out)
{
    std::set<uint32> bestIds;

    if (!IsAltBot(botAI) || sPlayerbotAIConfig->autoPickReward == "yes")
    {
        bestIds = BestRewards(quest);
        if (!bestIds.empty())
        {
            StatsWeightCalculator calc(bot);
            uint32 best = 0;
            float bestScore = 0;
            for (uint32 id : bestIds)
            {
                float score = calc.CalculateItem(quest->RewardChoiceItemId[id]);
                if (score > bestScore)
                {
                    bestScore = score;
                    best = id;
                }
            }
            // 5.4.8: choice index -> reward item id - see the file header.
            bot->RewardQuest(quest, quest->RewardChoiceItemId[best], questGiver, true);
            out << "Rewarded " << FormatItemOrEmpty(quest->RewardChoiceItemId[best]);
        }
        else
        {
            out << "Unable to find suitable reward. Asking for help....";
            AskToSelectReward(quest, out, true);
        }
    }
    else if (sPlayerbotAIConfig->autoPickReward == "no")
    {
        // Old functionality, list rewards.
        AskToSelectReward(quest, out, false);
    }
    else
    {
        // Try to pick the usable item. If multiple, list usable rewards.
        bestIds = BestRewards(quest);

        if (bestIds.size() > 1)
            AskToSelectReward(quest, out, true);

        else if (!bestIds.empty())
        {
            // Pick the first item
            uint32 firstId = *bestIds.begin();
            // 5.4.8: choice index -> reward item id - see the file header.
            bot->RewardQuest(quest, quest->RewardChoiceItemId[firstId], questGiver, true);

            out << "Rewarded " << FormatItemOrEmpty(quest->RewardChoiceItemId[firstId]);
        }
    }
}

void TalkToQuestGiverAction::AskToSelectReward(Quest const* quest, std::ostringstream& out, bool forEquip)
{
    std::ostringstream msg;
    msg << "Choose reward: ";

    for (uint32 i = 0; i < quest->GetRewChoiceItemsCount(); ++i)
    {
        if (!forEquip || BestRewards(quest).count(i) > 0)
        {
            msg << FormatItemOrEmpty(quest->RewardChoiceItemId[i]);
        }
    }

    botAI->TellMaster(msg);
    out << "Reward pending";
}

namespace
{
// 5.4.8 SMSG_QUESTGIVER_OFFER_REWARD (PlayerMenu::SendQuestGiverOfferReward, GossipDef.cpp:913-1021):
// dword 0 = rewInfo[2].second, dword 1 = questId, ... 72 dwords in all (54 `data << uint32` statements, of
// which the reputation loop is 5x3 and the currency loop 4x2), then the bit fields below, then bytes.
constexpr size_t OFFER_REWARD_DWORDS = 72;

bool ReadOfferRewardGuidAndQuest(WorldPacket packet, ObjectGuid& guid, uint32& questId)
{
    try
    {
        packet.rpos(0);
        packet.read_skip<uint32>();
        packet >> questId;
        packet.rpos(OFFER_REWARD_DWORDS * sizeof(uint32));

        // Bit fields in the exact order the core writes them (GossipDef.cpp:979-996).
        uint32 portraitTurnInTextLen = packet.ReadBits(10);
        uint32 portraitGiverNameLen = packet.ReadBits(8);
        guid[6] = packet.ReadBit();
        uint32 rewEmoteCount = packet.ReadBits(21);
        guid[3] = packet.ReadBit();
        guid[7] = packet.ReadBit();
        uint32 logTitleLen = packet.ReadBits(9);
        guid[4] = packet.ReadBit();
        uint32 questTurnTargetNameLen = packet.ReadBits(8);
        uint32 portraitGiverTextLen = packet.ReadBits(10);
        uint32 rewardTextLen = packet.ReadBits(12);
        guid[1] = packet.ReadBit();
        guid[2] = packet.ReadBit();
        guid[0] = packet.ReadBit();
        guid[5] = packet.ReadBit();
        packet.ReadBit();  // enableNext

        // Byte section (GossipDef.cpp:998-1019): strings and masked guid bytes interleaved.
        packet.read_skip(portraitGiverNameLen);
        packet.read_skip(logTitleLen);
        packet.read_skip(size_t(rewEmoteCount) * 2 * sizeof(uint32));
        packet.ReadByteSeq(guid[2]);
        packet.read_skip(rewardTextLen);
        packet.read_skip(portraitTurnInTextLen);
        packet.read_skip(questTurnTargetNameLen);
        packet.ReadByteSeq(guid[5]);
        packet.ReadByteSeq(guid[1]);
        packet.read_skip(portraitGiverTextLen);
        packet.ReadByteSeq(guid[0]);
        packet.ReadByteSeq(guid[7]);
        packet.ReadByteSeq(guid[6]);
        packet.ReadByteSeq(guid[4]);
        packet.ReadByteSeq(guid[3]);
    }
    catch (ByteBufferException const&)
    {
        return false;
    }

    return true;
}
}

// AC TalkToQuestGiverAction.cpp:245-306 (see the file header for the packet-layout adaptation).
bool TurnInQueryQuestAction::Execute(Event event)
{
    ObjectGuid guid;
    uint32 questId = 0;
    if (!ReadOfferRewardGuidAndQuest(event.getPacket(), guid, questId))
        return false;

    Object* object =
        ObjectAccessor::GetObjectByTypeMask(*bot, guid, TYPEMASK_UNIT | TYPEMASK_GAMEOBJECT | TYPEMASK_ITEM);
    if (!object || (!object->hasQuest(questId) && !object->hasInvolvedQuest(questId)))
        return false;

    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest)
        return false;

    QuestStatus status = bot->GetQuestStatus(quest->GetQuestId());
    Player* master = GetMaster();

    if (sPlayerbotAIConfig->syncQuestForPlayer && master)
    {
        if (IsRealPlayer(master))
        {
            QuestStatus masterStatus = master->GetQuestStatus(quest->GetQuestId());
            if (masterStatus == QUEST_STATUS_INCOMPLETE || masterStatus == QUEST_STATUS_FAILED)
                CompleteQuest(master, quest->GetQuestId());
        }
    }

    if (sPlayerbotAIConfig->syncQuestWithPlayer)
    {
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_FAILED)
        {
            CompleteQuest(bot, quest->GetQuestId());
            status = bot->GetQuestStatus(quest->GetQuestId());
        }
    }

    std::ostringstream out;
    out << "Quest ";
    switch (status)
    {
        case QUEST_STATUS_COMPLETE:
            TurnInQuest(quest, object, out);
            break;
        case QUEST_STATUS_INCOMPLETE:
            out << "|cffff0000Incompleted|r";
            break;
        case QUEST_STATUS_NONE:
            out << "|cff00ff00Available|r";
            break;
        case QUEST_STATUS_FAILED:
            out << "|cffff0000Failed|r";
            break;
        case QUEST_STATUS_REWARDED:
            out << "|cffff0000Rewarded|r";
            break;
        default:
            break;
    }

    out << ": " << chat->FormatQuest(quest);
    botAI->TellMaster(out);
    return true;
}