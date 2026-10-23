/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/QueryQuestAction.cpp for issue #257 slice F.
 *
 * 5.4.8 / this-fork adaptations vs AC (all forced by the core, none cosmetic):
 *  - Quest has no GetTitle() on 5.4.8; the one title site becomes GetLogTitle() (QuestDef.h:365) - the
 *    same mapping slices C/D/E applied (TalkToQuestGiverAction.cpp:16, AcceptQuestAction.cpp:16-17,
 *    DropQuestAction.cpp:11-13).
 *  - PlayerbotChatHandler lives in AI/PlayerbotAI.h on this fork (PlayerbotAI.h:64); extractQuestId()
 *    itself is unchanged (PlayerbotAI.cpp:70).
 *  - chat->FormatQuest / chat->FormatQuestObjective -> ChatHelper::FormatQuest / FormatQuestObjective:
 *    both are static on this fork (AI/ChatHelper.h:42-43), so the AiObject::chat instance member is
 *    not used (same mapping slice D/E applied, AcceptQuestAction.cpp:26-29).
 *  - ChatHelper::eraseAllSubStr does not exist on this fork (ChatHelper.h carries no such member), so
 *    the single " travel" strip in Execute() uses the file-local EraseAllSubStr() helper below, which
 *    removes every occurrence of the substring - exactly what AC's eraseAllSubStr(text, " travel") did.
 *  - Player::GetQuestSlotQuestId(uint8) takes a uint16 slot on 5.4.8 (Player.h:1761), so AC's uint8 slot
 *    counters become uint16 in both loops (same mapping as DropQuestAction.cpp:14-16).
 *  - The whole `if (travel)` block (AC QueryQuestAction.cpp:71-104) is not ported: it is built entirely
 *    on AC's quest travel-destination system (TravelMgr::getQuestTravelDestinations,
 *    TravelDestination, QuestTravelDestination), none of which exists on this fork - its TravelMgr is a
 *    deliberately lean transfer/flightmaster cache with no destination half at all (Travel/TravelMgr.h:9-13,
 *    the class comment). The `travel` keyword is still parsed off the parameter and `travel` is still
 *    set, so the request is accepted exactly as before; only the destination listing is omitted.
 *  - AC's TellObjectives() walks QUEST_OBJECTIVES_COUNT and reads the WotLK-only parallel arrays
 *    Quest::RequiredItemId[] / RequiredItemCount[], Quest::RequiredNpcOrGo[] / RequiredNpcOrGoCount[],
 *    Quest::ObjectiveText[] and QuestStatusData::ItemCount[] / CreatureOrGOCount[] (AC
 *    QueryQuestAction.cpp:125-157). None of those exist on 5.4.8: quest data moved to
 *    Quest::GetObjectives() (QuestDef.h:376, returning QuestObjective{ID, Type, ObjectID, Amount,
 *    Description}, QuestDef.h:280-291) and progress moved to Player::GetQuestObjectiveCounter(objectiveId)
 *    (Player.h:3186). TellObjectives() below is rewritten over those two, exactly as slice B rewrote the
 *    same data in QuestAction.cpp and slice E in DropQuestAction::HasProgress().
 *    The "negative RequiredNpcOrGo means a gameobject" trick becomes the explicit objective Type
 *    (QUEST_OBJECTIVE_MONSTER = creature, QUEST_OBJECTIVE_GAMEOBJECT = gameobject, QuestDef.h:205-208),
 *    which is strictly more precise than AC's sign test.
 *  - AC's questStatus.ItemCount[i] / .CreatureOrGOCount[i] become GetQuestObjectiveCounter(objective.ID).
 */

#include "QueryQuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "Item.h"
#include "DBCStructure.h"
#include "ItemPrototype.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include <sstream>

namespace
{
// ChatHelper::eraseAllSubStr is not ported on this fork - see the file header. Removes every occurrence
// of `sub` from `text`, which is what AC's eraseAllSubStr(text, sub) did.
void EraseAllSubStr(std::string& text, std::string const& sub)
{
    if (sub.empty())
        return;

    size_t pos = text.find(sub);
    while (pos != std::string::npos)
    {
        text.erase(pos, sub.length());
        pos = text.find(sub);
    }
}
}  // namespace

void QueryQuestAction::TellObjective(std::string const name, uint32 available, uint32 required)
{
    botAI->TellMaster(ChatHelper::FormatQuestObjective(name, available, required));
}

bool QueryQuestAction::Execute(Event event)
{
    Player* bot = botAI->GetBot();
    std::string text = event.getParam();
    bool travel = false;

    if (text.find("travel") != std::string::npos)
    {
        travel = true;
        EraseAllSubStr(text, " travel");
    }

    // #257: PlayerbotChatHandler is declared in AI/PlayerbotAI.h on this fork (PlayerbotAI.h:64).
    PlayerbotChatHandler ch(bot);
    uint32 questId = ch.extractQuestId(text);
    if (!questId)
    {
        // 5.4.8: uint8 slot -> uint16, see the file header.
        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 logQuest = bot->GetQuestSlotQuestId(slot);

            Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);
            if (!quest)
                continue;

            // 5.4.8: AC's quest->GetTitle() -> quest->GetLogTitle() - see the file header.
            if (text.find(quest->GetLogTitle()) != std::string::npos)
            {
                questId = quest->GetQuestId();
                break;
            }
        }
    }

    // 5.4.8: uint8 slot -> uint16, see the file header.
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        if (questId != bot->GetQuestSlotQuestId(slot))
            continue;

        std::ostringstream out;
        out << "--- " << ChatHelper::FormatQuest(sObjectMgr->GetQuestTemplate(questId)) << " ";

        if (bot->GetQuestStatus(questId) == QUEST_STATUS_COMPLETE)
        {
            out << "|c0000FF00completed|r ---";
            botAI->TellMaster(out);
        }
        else
        {
            out << "|c00FF0000not completed|r ---";
            botAI->TellMaster(out);
            TellObjectives(questId);
        }

        if (travel)
        {
            // #257: AC's travel destination listing (AC QueryQuestAction.cpp:71-104) is not ported - this
            // fork's TravelMgr has no quest travel destination system at all (Travel/TravelMgr.h:9-13).
            // The "travel" keyword is still accepted above so the request is not rejected.
            botAI->TellMaster("Travel information for this quest is not available.");
        }

        return true;
    }

    return false;
}

void QueryQuestAction::TellObjectives(uint32 questId)
{
    Quest const* questTemplate = sObjectMgr->GetQuestTemplate(questId);

    // Checks if the questTemplate is valid
    if (!questTemplate)
    {
        botAI->TellMaster("Quest template not found.");
        return;
    }

    // 5.4.8: AC's QUEST_OBJECTIVES_COUNT loop over RequiredItemId/RequiredNpcOrGo/ObjectiveText and
    // QuestStatusData::ItemCount/CreatureOrGOCount is rewritten over Quest::GetObjectives() +
    // Player::GetQuestObjectiveCounter() - see the file header.
    QuestObjectives const& objectives = questTemplate->GetObjectives();

    for (QuestObjective const& objective : objectives)
    {
        // Checks for objective text
        // AC: questTemplate->ObjectiveText[i]; 5.4.8 carries it per objective as Description.
        if (!objective.Description.empty())
            botAI->TellMaster(objective.Description);

        uint32 required = objective.Amount > 0 ? uint32(objective.Amount) : 0;
        uint32 available = bot->GetQuestObjectiveCounter(objective.ID);

        // Checks for required items
        // AC: `if (questTemplate->RequiredItemId[i])`
        if (objective.Type == QUEST_OBJECTIVE_ITEM && objective.ObjectID)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(objective.ObjectID);
            if (proto)
                TellObjective(ChatHelper::FormatItem(proto), available, required);
        }

        // Checks for required NPCs or GOs
        // AC: `if (questTemplate->RequiredNpcOrGo[i])` with a negative entry meaning a gameobject;
        // 5.4.8 states the type explicitly instead of the sign - see the file header.
        if ((objective.Type == QUEST_OBJECTIVE_MONSTER || objective.Type == QUEST_OBJECTIVE_GAMEOBJECT) &&
            objective.ObjectID)
        {
            if (objective.Type == QUEST_OBJECTIVE_GAMEOBJECT)
            {
                if (GameObjectTemplate const* info = sObjectMgr->GetGameObjectTemplate(objective.ObjectID))
                    TellObjective(info->name, available, required);
            }
            else
            {
                if (CreatureTemplate const* info = sObjectMgr->GetCreatureTemplate(objective.ObjectID))
                    TellObjective(info->Name, available, required);
            }
        }
    }
}