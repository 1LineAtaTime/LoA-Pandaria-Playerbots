/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/DropQuestAction.cpp for issue #257 slice E (the
 * quest-log pruning layer: "drop quest" and "clean quest log").
 *
 * 5.4.8 / this-fork adaptations vs AC (all of them forced by the core, none cosmetic):
 *  - Quest has no GetTitle() on 5.4.8; the three title sites become GetLogTitle() (QuestDef.h:365) -
 *    the same mapping slice D applied (AcceptQuestAction.cpp:16-17, :98, :168).
 *  - Quest::RequiredItemId[]/RequiredItemCount[], RequiredNpcOrGo[]/RequiredNpcOrGoCount[],
 *    ObjectiveText[] and QuestStatusData::ItemCount[]/CreatureOrGOCount[] - the whole of AC's
 *    HasProgress() - do not exist on 5.4.8. Quest data moved to objectives
 *    (Quest::GetObjectives(), QuestDef.h:376, returning QuestObjective{ID, Type, ObjectID, Amount,
 *    Description}) and per-objective progress moved to Player::GetQuestObjectiveCounter(objectiveId)
 *    (Player.h:3186, the 5.4.8 QuestObjectiveStatusMap). HasProgress() below is rewritten over those
 *    two, exactly as slice B rewrote the same data in QuestAction.cpp (its file header, lines 23-32).
 *  - Quest::GetRequiredClasses() (AC DropQuestAction.cpp:182) does not exist on 5.4.8; the field is
 *    _allowableClasses, read through GetAllowableClasses() (QuestDef.h:346) - same uint32 class bitmask
 *    AC's RequiredClasses is, so the "never drop class quests" guard is unchanged in meaning.
 *  - Player::GetQuestSlotQuestId(uint16) and Player::SetQuestSlot(uint16, uint32, uint32) take a uint16
 *    slot on 5.4.8 (Player.h:1761/1765), so AC's uint8 slot counters become uint16 in both loops.
 *  - PlayerbotChatHandler lives in AI/PlayerbotAI.h on this fork (not Chat/PlayerbotChatHandler.h);
 *    extractQuestId() itself is unchanged (PlayerbotAI.cpp:70) and is the same call slice D makes.
 *  - botAI->rpgStatistic.questDropped++ -> botAI->GetRpgStatistic().questDropped++, the fork's accessor
 *    (PlayerbotAI.h:199; slice B applied the same for questCompleted, QuestAction.cpp:44).
 *  - sPlayerbotAIConfig.dropObsoleteQuests -> sPlayerbotAIConfig->dropObsoleteQuests (pointer singleton on
 *    this fork; the config key itself exists, PlayerbotAIConfig.cpp:353).
 *  - LOG_INFO("playerbots", ...) -> TC_LOG_INFO("playerbots", ...): the fork has no plain LOG_INFO.
 *  - chat->FormatQuest(quest) -> ChatHelper::FormatQuest(quest): FormatQuest is static on this fork
 *    (AI/ChatHelper.h:42), so the AiObject::chat instance member is not used.
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault(...) is not ported on this fork (issue #322, same
 *    as every other chat/tell string ported so far). Each site below is the literal AC fallback string
 *    with %quest/%title already substituted, which is what the enUS fallback would produce.
 *  - BroadcastHelper (AC Bot/Broadcast/BroadcastHelper.h) is not ported on this fork (issue #322); the
 *    one AC broadcast site is replaced by a single marker comment at the same place.
 *  - No IsSelfBot()/IsAltBot() is involved on this path (those divergences from slice C do not apply
 *    here); the only "is this bot really a bot" gate in the file is dropped, see DropQuestAction below.
 */

#include "DropQuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
// #257: for sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF) in DropQuestType; Playerbots.h
// does not reach World.h (same include NewRpgBaseAction.cpp:33 uses for the identical call at :626).
#include "World.h"
#include <algorithm>
#include <random>

namespace
{
// AC's PlayerbotTextMgr::instance().GetBotTextOrDefault("quest_removed_debug", "Quest [%quest] removed",
// {{"%quest", text_quest}}) - PlayerbotTextMgr is not ported on this fork, see the file header.
std::string QuestRemovedDebug(std::string const textQuest)
{
    return "Quest [" + textQuest + "] removed";
}
}

bool DropQuestAction::Execute(Event event)
{
    std::string const link = event.getParam();

    // AC: `Player* master = GetMaster();` - AiObject::GetMaster() is the same accessor on this fork
    // (AiObject.h:24). Note this action needs a master even when it was triggered on the bot itself,
    // because the master is what typed the quest link; the bot-local "is this an alt" question never
    // arises, so slice C's IsAltBot()/HasRealPlayerMaster() divergence is not used here.
    Player* master = GetMaster();
    if (!master)
        return false;

    // #257: PlayerbotChatHandler is declared in AI/PlayerbotAI.h on this fork (PlayerbotAI.h:64).
    PlayerbotChatHandler handler(master);
    uint32 entry = handler.extractQuestId(link);

    // remove all quest entries for 'entry' from quest log
    // 5.4.8: uint8 slot -> uint16, Player::GetQuestSlotQuestId/SetQuestSlot take uint16 (Player.h:1761/1765).
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);

        Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);
        if (!quest)
            continue;

        // 5.4.8: AC's quest->GetTitle() -> quest->GetLogTitle() - see the file header.
        if (logQuest == entry || link.find(quest->GetLogTitle()) != std::string::npos)
        {
            bot->SetQuestSlot(slot, 0);

            // we ignore unequippable quest items in this case, its' still be equipped
            bot->TakeQuestSourceItem(logQuest, false);
            entry = logQuest;
            break;
        }
    }

    if (!entry)
        return false;

    bot->RemoveRewardedQuest(entry);
    bot->RemoveActiveQuest(entry, false);

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) ||
        botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        Quest const* pQuest = sObjectMgr->GetQuestTemplate(entry);
        std::string const text_quest = ChatHelper::FormatQuest(pQuest);
        // 5.4.8: AC's quest->GetTitle() -> quest->GetLogTitle(); LOG_INFO -> TC_LOG_INFO.
        TC_LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), pQuest->GetLogTitle());
        bot->Say(QuestRemovedDebug(text_quest), LANG_UNIVERSAL);
    }

    // PlayerbotTextMgr not ported on this fork; this is AC's own enUS fallback text ("quest_remove").
    botAI->TellMaster("Quest removed");
    return true;
}

bool CleanQuestLogAction::Execute(Event event)
{
    // AC's `event.getOwner() ? event.getOwner() : GetMaster()` - Requester is unused afterwards (AC
    // DropQuestAction.cpp:69-71), kept as-is so the master-less case still refuses to run.
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    // 5.4.8: sPlayerbotAIConfig.dropObsoleteQuests -> sPlayerbotAIConfig->dropObsoleteQuests.
    if (!sPlayerbotAIConfig->dropObsoleteQuests)
        return false;

    // Only output this message if "debug rpg" strategy is enabled
    if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        // PlayerbotTextMgr not ported on this fork; AC's enUS fallback ("clean_quest_log_started").
        botAI->TellMaster("Clean Quest Log command received, removing grey/trivial quests...");

    uint8 botLevel = bot->GetLevel();  // Get bot's level

    // 5.4.8: uint8 slot -> uint16 (Player.h:1761).
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        // Determine if quest is trivial by comparing levels
        int32 questLevel = quest->GetQuestLevel();
        if (questLevel == -1) // For scaling quests, default to bot level
            questLevel = botLevel;

        // Set the level difference for when a quest becomes trivial
        // This was determined by using the Lua code the client uses
        int32 trivialLevel = 5;
        if (botLevel >= 40)
            trivialLevel = 8;
        else if (botLevel >= 30)
            trivialLevel = 7;
        else if (botLevel >= 20)
            trivialLevel = 6;

        // Check if the quest is trivial (grey) for the bot
        if ((botLevel - questLevel) > trivialLevel)
        {
            // Output only if "debug rpg" strategy is enabled
            // PlayerbotTextMgr not ported; AC's enUS fallback ("quest_trivial_will_remove"), %title is
            // the raw quest title - 5.4.8: AC's GetTitle() -> GetLogTitle().
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster("Quest [" + quest->GetLogTitle() +
                                 "] will be removed because it is trivial (grey).");

            // Remove quest
            // 5.4.8: botAI->rpgStatistic.questDropped -> GetRpgStatistic().questDropped (PlayerbotAI.h:199).
            botAI->GetRpgStatistic().questDropped++;
            bot->SetQuestSlot(slot, 0);
            bot->TakeQuestSourceItem(questId, false);
            bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
            bot->RemoveRewardedQuest(questId);

            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
            {
                std::string const text_quest = ChatHelper::FormatQuest(quest);
                // 5.4.8: GetTitle() -> GetLogTitle(); LOG_INFO -> TC_LOG_INFO.
                TC_LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), quest->GetLogTitle());
                bot->Say(QuestRemovedDebug(text_quest), LANG_UNIVERSAL);
            }

            // PlayerbotTextMgr not ported; AC's enUS fallback ("quest_has_been_removed"), %title = title.
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster("Quest [" + quest->GetLogTitle() + "] has been removed.");
        }
        else
        {
            // Only output if "debug rpg" strategy is enabled
            // PlayerbotTextMgr not ported; AC's enUS fallback ("quest_not_trivial_kept"), %title = title.
            if (botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
                botAI->TellMaster("Quest [" + quest->GetLogTitle() + "] is not trivial and will be kept.");
        }
    }

    return true;
}

void CleanQuestLogAction::DropQuestType(uint8& numQuest, uint8 wantNum, bool isGreen, bool hasProgress,
                                        bool isComplete)
{
    // 5.4.8: uint8 slot -> uint16 (Player.h:1761).
    std::vector<uint16> slots;
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        slots.push_back(slot);

    if (wantNum < 100)
    {
        std::random_device rd;
        std::mt19937 g(rd());
        std::shuffle(slots.begin(), slots.end(), g);
    }

    for (uint16 slot : slots)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
        if (!quest)
            continue;

        // Do not drop class quest, may be not rewarding gold but important spells
        // 5.4.8: AC's quest->GetRequiredClasses() -> quest->GetAllowableClasses() (QuestDef.h:346).
        if (quest->GetAllowableClasses())
            continue;

        if (wantNum == 100)
            numQuest++;

        int32 lowLevelDiff = sWorld->getIntConfig(CONFIG_QUEST_LOW_LEVEL_HIDE_DIFF);
        if (lowLevelDiff < 0 || bot->GetLevel() <= bot->GetQuestLevel(quest) + uint32(lowLevelDiff))
        {                                          // Quest is not gray
            if (bot->GetLevel() + 5 > bot->GetQuestLevel(quest))
            {                                      // Quest is not red
                if (!isGreen)
                    continue;
            }
        }
        else  // Quest is gray
        {
            if (isGreen)
                continue;
        }

        if (HasProgress(bot, quest) && !hasProgress && bot->GetQuestStatus(questId) != QUEST_STATUS_FAILED)
            continue;

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE && !isComplete)
            continue;

        // Always drop failed quests
        if (numQuest <= wantNum && bot->GetQuestStatus(questId) != QUEST_STATUS_FAILED)
            continue;

        // Drop quest.
        bot->SetQuestSlot(slot, 0);

        // We ignore unequippable quest items in this case, its' still be equipped
        bot->TakeQuestSourceItem(questId, false);

        bot->SetQuestStatus(questId, QUEST_STATUS_NONE);
        bot->RemoveRewardedQuest(questId);

        numQuest--;

        if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) ||
            botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
        {
            std::string const text_quest = ChatHelper::FormatQuest(quest);
            // 5.4.8: GetTitle() -> GetLogTitle(); LOG_INFO -> TC_LOG_INFO.
            TC_LOG_INFO("playerbots", "{} => Quest [ {} ] removed", bot->GetName(), quest->GetLogTitle());
            bot->Say(QuestRemovedDebug(text_quest), LANG_UNIVERSAL);
        }

        // 5.4.8: AC's chat->FormatQuest(quest) -> the static ChatHelper::FormatQuest(quest) (AI/ChatHelper.h:42).
        // PlayerbotTextMgr not ported; AC's enUS fallback ("quest_removed_with_name") already wraps
        // %quest in nothing, and FormatQuest itself supplies the "[title]" brackets.
        botAI->TellMaster("Quest removed " + ChatHelper::FormatQuest(quest));

        // #257: BroadcastHelper not ported (see #322) - announcement omitted
    }
}

// 5.4.8 rewrite of AC DropQuestAction.cpp:242-275: the WotLK Quest::ObjectiveText[] / RequiredItemId[] /
// RequiredItemCount[] / RequiredNpcOrGo[] / RequiredNpcOrGoCount[] arrays and the matching
// QuestStatusData::ItemCount[] / CreatureOrGOCount[] counters do not exist here. Per-objective progress
// is Player::GetQuestObjectiveCounter(objective.ID) (Player.h:3186) and the requirement is
// QuestObjective::Amount, the same two values slice B reads in QuestAction.cpp:204-235.
bool CleanQuestLogAction::HasProgress(Player* bot, Quest const* quest)
{
    uint32 questId = quest->GetQuestId();

    if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        return true;

    // 5.4.8: AC's `QuestStatusData questStatus = bot->getQuestStatusMap()[questId];` - getQuestStatusMap()
//    still exists (Player.h:1923) but QuestStatusData on 5.4.8 holds only Status/Timer/Explored
//    (QuestDef.h:523-530), i.e. none of the per-objective counters AC read. Not used.
    for (QuestObjective const& objective : quest->GetObjectives())
    {
        // AC: if (!quest->ObjectiveText[i].empty()) return true; -> 5.4.8 keeps the text on the objective.
        if (!objective.Description.empty())
            return true;

        // AC checked RequiredItemId[i]/RequiredNpcOrGo[i] non-zero and required > 0 before comparing to
        // the counter; on 5.4.8 "there is such an objective" is the type check and the required count is
        // objective.Amount.
        int32 required = objective.Amount;
        if (required <= 0)
            continue;

        // AC: item objectives (ItemCount) and creature/GO objectives (CreatureOrGOCount) both compared
        // "available > 0 && required > 0". On 5.4.8 all three are one counter, keyed by objective id.
        switch (objective.Type)
        {
            case QUEST_OBJECTIVE_ITEM:
            case QUEST_OBJECTIVE_MONSTER:
            case QUEST_OBJECTIVE_GAMEOBJECT:
                if (bot->GetQuestObjectiveCounter(objective.ID) > 0)
                    return true;
                break;
            default:
                break;
        }
    }

    return false;
}
