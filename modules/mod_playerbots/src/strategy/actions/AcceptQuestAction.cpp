/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/AcceptQuestAction.cpp for issue #257 slice D (the
 * master-commanded quest layer).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - CMSG_QUESTGIVER_ACCEPT_QUEST is a different wire format on 5.4.8. AC's handler (and AC's
 *    AcceptQuestAction::Execute packet branch) read "ObjectGuid guid, uint32 questId, uint32 unk1"
 *    flat (AC QuestHandler.cpp:118). 5.4.8 reads uint32 questId FIRST, then nine bits - eight
 *    guid bits interleaved with one startCheat bit - then eight masked guid bytes in a fixed order
 *    (WorldSession::HandleQuestgiverAcceptQuestOpcode, src/server/game/Handlers/QuestHandler.cpp:149-170).
 *    The packet branch below is therefore rewritten to consume the packet in exactly the order the
 *    5.4.8 handler reads it, via the file-local ReadAcceptQuestPacket() helper.
 *  - Quest has no GetTitle() on 5.4.8; both AC log/title sites become GetLogTitle()
 *    (QuestDef.h:365), the same mapping slice C applied (TalkToQuestGiverAction.cpp:16).
 *  - PlayerbotTextMgr::instance().GetBotTextOrDefault() is not ported on this fork (same as every
 *    other chat/tell string ported so far) - the literal below is exactly the enUS default AC's own
 *    fallback argument would produce ("Quest [%quest] accepted" with %quest substituted).
 *  - BroadcastHelper (AC Bot/Broadcast/BroadcastHelper.h) is not ported on this fork (issue #322); each
 *    AC broadcast call below is replaced by a single marker comment at the same site.
 *  - bot->SetDivider() takes an ObjectGuid argument on 5.4.8 (Player.h:1810, default ObjectGuid::Empty),
 *    so AC's bare SetDivider() is written SetDivider(ObjectGuid::Empty).
 *  - Player::HasQuest(questId) does not exist on 5.4.8 (AC Player.h:1560 / PlayerQuest.cpp:102-114); it
 *    was exactly "is this quest in the player's quest log". The 5.4.8 equivalent is
 *    FindQuestSlot(questId) < MAX_QUEST_LOG_SIZE (Player.cpp:17593-17600), which is what the
 *    "I have this quest" branch uses below.
 *  - No WotLK-only quest array is touched here (AC's Quest::RequiredItemId / RequiredNpcOrGo families
 *    are slice B's problem, rewritten there over Quest::GetObjectives()); the accept path only needs
 *    CanTakeQuest/CanAddQuest/AddQuest/CompleteQuest/GetSrcSpell, all present in 5.4.8.
 */

#include "AcceptQuestAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "GameObject.h"
#include "ObjectGuid.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "WorldPacket.h"
#include <sstream>

namespace
{
// 5.4.8 CMSG_QUESTGIVER_ACCEPT_QUEST reader, mirroring WorldSession::HandleQuestgiverAcceptQuestOpcode
// (src/server/game/Handlers/QuestHandler.cpp:149-170) field for field. AC's flat
// `p >> guid >> quest` (AC AcceptQuestAction.cpp:79) does not describe the 5.4.8 wire format.
void ReadAcceptQuestPacket(WorldPacket& p, ObjectGuid& guid, uint32& questId)
{
    p.rpos(0);

    questId = p.read<uint32>();

    guid[6] = p.ReadBit();
    guid[0] = p.ReadBit();
    p.ReadBit();  // startCheat - the 5.4.8 handler logs it and otherwise ignores it
    guid[2] = p.ReadBit();
    guid[7] = p.ReadBit();
    guid[5] = p.ReadBit();
    guid[4] = p.ReadBit();
    guid[3] = p.ReadBit();
    guid[1] = p.ReadBit();

    p.ReadByteSeq(guid[5]);
    p.ReadByteSeq(guid[4]);
    p.ReadByteSeq(guid[0]);
    p.ReadByteSeq(guid[1]);
    p.ReadByteSeq(guid[6]);
    p.ReadByteSeq(guid[2]);
    p.ReadByteSeq(guid[3]);
    p.ReadByteSeq(guid[7]);
}

// 5.4.8 SMSG_QUEST_CONFIRM_ACCEPT reader, mirroring Player::SendQuestConfirmAccept
// (src/server/game/Entities/Player/Player.cpp:18245-18286) field for field. AC's SMSG is a flat leading
// uint32 questId; on 5.4.8 the questId is the LAST field, behind nine masked-guid bits, a 10-bit title
// length, the masked guid bytes and the title string, so a flat `p >> questId` reads garbage.
uint32 ReadQuestConfirmAcceptServerPacket(WorldPacket& p)
{
    p.rpos(0);

    ObjectGuid guid;
    guid[0] = p.ReadBit();
    guid[3] = p.ReadBit();
    p.ReadBit();  // "has quest title" - the core always writes 0
    guid[2] = p.ReadBit();
    guid[5] = p.ReadBit();
    guid[6] = p.ReadBit();
    guid[4] = p.ReadBit();
    guid[1] = p.ReadBit();
    uint32 const titleLen = p.ReadBits(10);
    guid[7] = p.ReadBit();

    p.ReadByteSeq(guid[6]);
    p.read_skip(titleLen);  // the quest title string sits between guid byte 6 and guid byte 0
    p.ReadByteSeq(guid[0]);
    p.ReadByteSeq(guid[5]);
    p.ReadByteSeq(guid[3]);
    p.ReadByteSeq(guid[1]);
    p.ReadByteSeq(guid[4]);
    p.ReadByteSeq(guid[2]);
    p.ReadByteSeq(guid[7]);

    return p.read<uint32>();
}

// AC's PlayerbotTextMgr::instance().GetBotTextOrDefault("quest_accept_debug", "Quest [%quest] accepted",
// {{"%quest", text_quest}}) - PlayerbotTextMgr is not ported on this fork, see the file header.
std::string QuestAcceptDebug(std::string const textQuest)
{
    return "Quest [" + textQuest + "] accepted";
}
}

bool AcceptAllQuestsAction::ProcessQuest(Quest const* quest, Object* questGiver)
{
    if (!AcceptQuest(quest, questGiver->GetGUID()))
        return false;

    std::string const text_quest = ChatHelper::FormatQuest(quest);
    bot->PlayDistanceSound(620);

    if (botAI->HasStrategy("debug quest", BotState::BOT_STATE_NON_COMBAT) ||
        botAI->HasStrategy("debug rpg", BotState::BOT_STATE_COMBAT))
    {
        // 5.4.8: AC's quest->GetTitle() -> quest->GetLogTitle() - see the file header.
        TC_LOG_INFO("playerbots", "{} => Quest [{}] accepted", bot->GetName(), quest->GetLogTitle());
        bot->Say(QuestAcceptDebug(text_quest), LANG_UNIVERSAL);
    }

    return true;
}

bool AcceptQuestAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();
    if (!requester)
        return false;

    // AC declares a local `Player* bot = botAI->GetBot();` here; on this fork AiObject::bot already
    // holds exactly that pointer (AiObject.cpp), so the shadowing local is dropped.
    ObjectGuid guid;
    uint32 quest = 0;

    std::string const text = event.getParam();
    PlayerbotChatHandler ch(requester);
    quest = ch.extractQuestId(text);

    bool hasAccept = false;

    if (event.getPacket().empty())
    {
        GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
        for (auto i = npcs.begin(); i != npcs.end(); i++)
        {
            Unit* unit = botAI->GetUnit(*i);
            if (unit && quest && unit->hasQuest(quest))
            {
                guid = unit->GetGUID();
                break;
            }
            if (unit && text == "*" && bot->GetDistance(unit) <= INTERACTION_DISTANCE)
                hasAccept |= QuestAction::ProcessQuests(unit);
        }
        GuidVector gos = AI_VALUE(GuidVector, "nearest game objects no los");
        for (auto i = gos.begin(); i != gos.end(); i++)
        {
            GameObject* go = botAI->GetGameObject(*i);
            if (go && quest && go->hasQuest(quest))
            {
                guid = go->GetGUID();
                break;
            }
            if (go && text == "*" && bot->GetDistance(go) <= INTERACTION_DISTANCE)
                hasAccept |= QuestAction::ProcessQuests(go);
        }
    }
    else
    {
        // 5.4.8: rewritten against the real CMSG_QUESTGIVER_ACCEPT_QUEST layout - see the file header.
        ReadAcceptQuestPacket(event.getPacket(), guid, quest);
    }

    if (!quest || guid.IsEmpty())
        return false;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);
    if (!qInfo)
        return false;

    hasAccept |= AcceptQuest(qInfo, guid);

    if (hasAccept)
    {
        std::stringstream ss;
        // 5.4.8: AC's qInfo->GetTitle() -> qInfo->GetLogTitle() - see the file header.
        ss << "AcceptQuestAction [" << qInfo->GetLogTitle() << "] - [" << std::to_string(qInfo->GetQuestId()) << "]";
        TC_LOG_DEBUG("playerbots", "{}", ss.str().c_str());
        // botAI->TellMaster(ss.str());
    }

    return hasAccept;
}

bool AcceptQuestShareAction::Execute(Event event)
{
    Player* master = GetMaster();

    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 quest;
    p >> quest;

    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);
    if (!qInfo || !bot->GetDivider())
        return false;

    quest = qInfo->GetQuestId();

    // 5.4.8: AC's bot->HasQuest(quest) -> FindQuestSlot() != MAX_QUEST_LOG_SIZE - see the file header.
    if (bot->FindQuestSlot(quest) < MAX_QUEST_LOG_SIZE)
    {
        // 5.4.8: SetDivider takes the guid explicitly - see the file header.
        bot->SetDivider(ObjectGuid::Empty);
        // PlayerbotTextMgr not ported on this fork; this is AC's own enUS fallback text.
        botAI->TellError("I have this quest");
        return false;
    }

    if (!bot->CanTakeQuest(qInfo, false))
    {
        // can't take quest
        bot->SetDivider(ObjectGuid::Empty);
        // PlayerbotTextMgr not ported on this fork; this is AC's own enUS fallback text.
        botAI->TellError("I can't take this quest");

        return false;
    }

    if (!bot->GetDivider().IsEmpty())
    {
        // send msg to quest giving player
        master->SendPushToPartyResponse(bot, QUEST_PARTY_MSG_ACCEPT_QUEST);
        bot->SetDivider(ObjectGuid::Empty);
    }

    if (bot->CanAddQuest(qInfo, false))
    {
        bot->AddQuest(qInfo, master);

        if (bot->CanCompleteQuest(quest))
            bot->CompleteQuest(quest);

        // Runsttren: did not add typeid switch from WorldSession::HandleQuestgiverAcceptQuestOpcode!
        // I think it's not needed, cause typeid should be TYPEID_PLAYER - and this one is not handled
        // there and there is no default case also.

        if (qInfo->GetSrcSpell() > 0)
        {
            bot->CastSpell(bot, qInfo->GetSrcSpell(), true);
        }

        // PlayerbotTextMgr not ported on this fork; this is AC's own enUS fallback text.
        botAI->TellMaster("Quest accepted");
        return true;
    }

    return false;
}

bool ConfirmQuestAction::Execute(Event event)
{
    Player* requester = event.getOwner() ? event.getOwner() : GetMaster();

    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 quest;
    // 5.4.8: the bot's own SMSG_QUEST_CONFIRM_ACCEPT carries the questId last (see the reader above);
    // the master's CMSG_QUEST_CONFIRM_ACCEPT is the bare uint32 AC reads.
    if (p.GetOpcode() == SMSG_QUEST_CONFIRM_ACCEPT)
        quest = ReadQuestConfirmAcceptServerPacket(p);
    else
        p >> quest;
    Quest const* qInfo = sObjectMgr->GetQuestTemplate(quest);

    if (!qInfo)
        return false;

    quest = qInfo->GetQuestId();
    if (!bot->CanTakeQuest(qInfo, false))
    {
        // can't take quest
        // botAI->TellError("quest_cant_take");
        return false;
    }

    if (bot->CanAddQuest(qInfo, false))
    {
        bot->AddQuest(qInfo, requester);

        if (bot->CanCompleteQuest(quest))
            bot->CompleteQuest(quest);

        if (qInfo->GetSrcSpell() > 0)
        {
            bot->CastSpell(bot, qInfo->GetSrcSpell(), true);
        }

        // botAI->TellMaster("quest_accept");
        return true;
    }

    return false;
}