/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ShareQuestAction.cpp for issue #257 slice F.
 *
 * 5.4.8 / this-fork adaptations vs AC (all forced by the core, none cosmetic):
 *  - AC builds a WorldPackets::Quest::PushQuestToParty wrapper (AC ShareQuestAction.cpp:34-38) and calls
 *    WorldSession::HandlePushQuestToParty(PushQuestToParty const&). 5.4.8 has no WorldPackets::Quest
 *    at all - WorldSession::HandlePushQuestToParty takes a plain WorldPacket&
 *    (src/server/game/Server/WorldSession.h:865) and reads exactly one uint32, the quest id
 *    (src/server/game/Handlers/QuestHandler.cpp:758-760). So the AC wrapper is replaced by the raw
 *    `WorldPacket p(CMSG_PUSHQUESTTOPARTY); p << entry;` that AC itself builds just above the wrapper,
 *    handed straight to the handler - the wire format is byte-identical to what AC's Read()/Read() pair
 *    produces, because AC only ever put that same single uint32 in the packet.
 *  - Player::GetQuestSlotQuestId(uint8) takes a uint16 slot on 5.4.8 (Player.h:1761), so AC's uint8 slot
 *    counters become uint16 in both loops - same mapping slice D/E applied (DropQuestAction.cpp:14-16).
 *  - PlayerbotChatHandler lives in AI/PlayerbotAI.h on this fork (not Chat/PlayerbotChatHandler.h);
 *    extractQuestId() itself is unchanged (PlayerbotAI.cpp:70).
 *  - AC's free function IsRealPlayer(Player*) ("is this a human, not a bot") does not exist on this fork;
 *    only the no-argument PlayerbotAI::IsRealPlayer() member ("is *this* bot itself a real player")
 *    (AI/PlayerbotAI.h:212). AutoShareQuestAction::isUseful() therefore uses the file-local
 *    IsRealPlayer() helper below - the exact same mapping slice C applied
 *    (TalkToQuestGiverAction.cpp:65-71) and that ItemUsageValue.cpp:9-13 documents.
 *  - PlayerbotAI::HandleMasterIncomingPacket takes WorldPacket const& on this fork
 *    (AI/PlayerbotAI.h:160); passing a local WorldPacket by value call is unchanged, just const-bound.
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault(...) is not ported on this fork (#322, same as
 *    every other chat/tell string ported so far). Each site below is the literal AC fallback string,
 *    which is what the enUS fallback would produce.
 *  - No IsAltBot() is involved on this path, so slice C's IsAltBot()/HasRealPlayerMaster() divergence
 *    does not apply here.
 */

#include "ShareQuestAction.h"

#include "Event.h"
#include "Group.h"
#include "ObjectMgr.h"
#include "Opcodes.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
// AC's free function IsRealPlayer(Player*) - see the file header. A player with no PlayerbotAI attached
// is a person at a client. Same body as TalkToQuestGiverAction.cpp:67-71.
bool IsRealPlayer(Player* master)
{
    PlayerbotAI* masterBotAI = master ? GET_PLAYERBOT_AI(master) : nullptr;
    return !masterBotAI || masterBotAI->IsRealPlayer();
}

// AC's PlayerbotTextMgr::instance().GetBotTextOrDefault("quest_shared", "Quest shared", {}) - see the
// file header.
std::string QuestSharedText()
{
    return "Quest shared";
}
}  // namespace

bool ShareQuestAction::Execute(Event event)
{
    std::string const link = event.getParam();
    if (!GetMaster())
        return false;

    // #257: PlayerbotChatHandler is declared in AI/PlayerbotAI.h on this fork (PlayerbotAI.h:64).
    PlayerbotChatHandler handler(GetMaster());
    uint32 entry = handler.extractQuestId(link);
    if (!entry)
        return false;

    Quest const* quest = sObjectMgr->GetQuestTemplate(entry);
    if (!quest)
        return false;

    // remove all quest entries for 'entry' from quest log
    // 5.4.8: uint8 slot -> uint16, Player::GetQuestSlotQuestId takes uint16 (Player.h:1761).
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);
        if (logQuest == entry)
        {
            // 5.4.8: no WorldPackets::Quest wrapper; HandlePushQuestToParty reads one uint32 - see the
            // file header.
            WorldPacket p(CMSG_PUSHQUESTTOPARTY);
            p << entry;
            bot->GetSession()->HandlePushQuestToParty(p);
            botAI->TellMaster(QuestSharedText());
            return true;
        }
    }

    return false;
}

bool AutoShareQuestAction::Execute(Event /*event*/)
{
    bool shared = false;

    // 5.4.8: uint8 slot -> uint16, see above.
    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 logQuest = bot->GetQuestSlotQuestId(slot);
        Quest const* quest = sObjectMgr->GetQuestTemplate(logQuest);

        if (!quest)
            continue;

        bool partyNeedsQuest = false;

        for (GroupReference* itr = bot->GetGroup()->GetFirstMember(); itr != nullptr; itr = itr->next())
        {
            Player* player = itr->GetSource();

            if (!player || player == bot || !player->IsInWorld() || !botAI->IsSafe(player))  // skip self
                continue;

            if (bot->GetDistance(player) > 10)
                continue;

            if (!player->SatisfyQuestStatus(quest, false))
                continue;

            if (player->GetQuestStatus(logQuest) == QUEST_STATUS_COMPLETE)
                continue;

            if (!player->CanTakeQuest(quest, false))
                continue;

            if (!player->SatisfyQuestLog(false))
                continue;

            if (player->GetDivider())
                continue;

            if (auto ai = GET_PLAYERBOT_AI(player))
            {
                if (PAI_VALUE(uint8, "free quest log slots") < 15 || !urand(0, 5))
                {
                    WorldPacket packet(CMSG_PUSHQUESTTOPARTY, 20);
                    packet << logQuest;
                    // 5.4.8: HandleMasterIncomingPacket takes WorldPacket const& (PlayerbotAI.h:160).
                    ai->HandleMasterIncomingPacket(packet);
                }
            }
            else
                partyNeedsQuest = true;
        }

        if (!partyNeedsQuest)
            continue;

        // 5.4.8: no WorldPackets::Quest wrapper, see above.
        WorldPacket p(CMSG_PUSHQUESTTOPARTY);
        p << logQuest;
        bot->GetSession()->HandlePushQuestToParty(p);
        botAI->TellMaster(QuestSharedText());
        shared = true;
    }

    return shared;
}

bool AutoShareQuestAction::isUseful()
{
    return bot->GetGroup() && !IsRealPlayer(botAI->GetMaster());
}