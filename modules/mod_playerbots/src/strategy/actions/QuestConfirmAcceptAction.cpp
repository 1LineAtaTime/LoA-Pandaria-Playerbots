/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/QuestConfirmAcceptAction.cpp for issue #257 slice D (the
 * master-commanded quest layer).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - AC's WorldSession::HandleQuestConfirmAccept() takes a WorldPackets::Quest::QuestConfirmAcceptClient
 *    payload wrapper that AC reconstructs with confirmAccept.Read(). This fork has no QuestPackets.h and
 *    the handler takes a plain WorldPacket& that already holds the uint32 questId
 *    (WorldSession::HandleQuestConfirmAccept, src/server/game/Handlers/QuestHandler.cpp:632-635), which is
 *    the exact layout AC's sendPacket writes. So the CMSG_QUEST_CONFIRM_ACCEPT packet built here is handed
 *    to the handler directly, rewound to rpos(0), and the whole wrapper dance is dropped.
 *  - bot->CanAddQuest(quest, true) keeps AC's meaning: the 5.4.8 second parameter is the "show error msg"
 *    flag (Player.h:1708), identical to AC.
 */

#include "QuestConfirmAcceptAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include <sstream>

bool QuestConfirmAcceptAction::Execute(Event event)
{
    WorldPacket packet(event.getPacket());
    uint32 questId;
    packet >> questId;

    WorldPacket sendPacket(CMSG_QUEST_CONFIRM_ACCEPT);
    sendPacket << questId;
    Quest const* quest = sObjectMgr->GetQuestTemplate(questId);
    if (!quest || !bot->CanAddQuest(quest, true))
    {
        return false;
    }
    std::ostringstream out;
    out << "Quest: " << chat->FormatQuest(quest) << " confirm accept";
    botAI->TellMaster(out);
    // 5.4.8: no WorldPackets::Quest::QuestConfirmAcceptClient wrapper on this fork - see the file header.
    sendPacket.rpos(0);
    bot->GetSession()->HandleQuestConfirmAccept(sendPacket);
    return true;
}