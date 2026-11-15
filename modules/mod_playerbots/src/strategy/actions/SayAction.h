/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/SayAction.h/.cpp: SayAction ("say::<text>", dormant in AC: only the
 * unregistered SayStrategy names it) and ChatReplyAction (ON in AC: random bots answer say, yell, whisper, party,
 * guild and channel chat, AiPlayerbot.RandomBotTalk = 1).
 *
 * 5.4.8 adaptations, chosen so that PlayerbotAI.h stays unchanged:
 *  - AC keeps the reply queue (PlayerbotAI::chatReplies / QueueChatResponse, Bot/PlayerbotAI.h:398/641) and the
 *    "last said"::"chat" cooldown in PlayerbotAI / its value context. Here both live in a per-bot table in
 *    SayAction.cpp behind one mutex (QueueChatResponse, ProcessChatReplies, GetLastChatTime/SetLastChatTime).
 *    The packet hook that fills the queue runs on whichever thread sends the chat packet - another map's thread for
 *    a guild message - while the bot's own map thread drains it, so the table must be locked; AC has no lock.
 *  - ChatChannelSource / ChatChannelId (AC Bot/PlayerbotAI.h:115-166) and PlayerbotAI::GetChatChannelSource
 *    (Bot/PlayerbotAI.cpp:6632-6730) are declared here; ChatHelper::ExtractAllItemIds / ExtractAllQuestIds
 *    (Bot/Cmd/ChatHelper.cpp:696-733) are ChatReplyAction statics.
 *  - SayToGuild / SayToWorld / SayToChannel and the area/class/race placeholders come from BroadcastHelper
 *    (BotSayToGuild, BotSayToWorld, BotSayToChannel, BotChatPlaceholders), the same 5.4.8 code the broadcasts use.
 */

#ifndef _PLAYERBOT_SAYACTION_H
#define _PLAYERBOT_SAYACTION_H

#include "Action.h"
#include "NamedObjectContext.h"

#include <ctime>
#include <map>
#include <set>
#include <string>
#include <vector>

class Player;
class PlayerbotAI;
struct ChatQueuedReply;

// AC Bot/PlayerbotAI.h:115-136
enum ChatChannelSource
{
    SRC_GUILD,
    SRC_WORLD,
    SRC_GENERAL,
    SRC_TRADE,
    SRC_LOOKING_FOR_GROUP,
    SRC_LOCAL_DEFENSE,
    SRC_WORLD_DEFENSE,
    SRC_GUILD_RECRUITMENT,

    SRC_SAY,
    SRC_WHISPER,
    SRC_EMOTE,
    SRC_TEXT_EMOTE,
    SRC_YELL,

    SRC_PARTY,
    SRC_RAID,

    SRC_UNDEFINED
};

// AC Bot/PlayerbotAI.h:158-166 (ChatChannels.dbc ids, unchanged on 5.4.8)
enum ChatChannelId
{
    GENERAL = 1,
    TRADE = 2,
    LOCAL_DEFENSE = 22,
    WORLD_DEFENSE = 23,
    LOOKING_FOR_GROUP = 26,
    GUILD_RECRUITMENT = 25,
};

class SayAction : public Action, public Qualified
{
public:
    SayAction(PlayerbotAI* botAI);

    bool Execute(Event event) override;
    bool isUseful() override;
    std::string const getName() override { return "say::" + qualifier; }

private:
    static std::map<std::string, std::vector<std::string>> stringTable;
    static std::map<std::string, uint32> probabilityTable;
};

class ChatReplyAction : public Action
{
public:
    ChatReplyAction(PlayerbotAI* ai) : Action(ai, "chat message") {}
    bool Execute(Event /*event*/) override { return true; }
    bool isUseful() override { return true; }

    static void ChatReplyDo(Player* bot, uint32& type, uint32& guid1, std::string& msg, std::string& chanName, std::string& name);
    static bool HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource);
    static bool HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource);
    static bool HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name);
    static bool HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name);
    static bool SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string& responseMessage, std::string& name);
    static std::string GenerateReplyMessage(Player* bot, std::string& incomingMessage, uint32& guid1, std::string& name);

    // AC PlayerbotAI::GetChatChannelSource (Bot/PlayerbotAI.cpp:6632-6730)
    static ChatChannelSource GetChatChannelSource(Player* bot, uint32 type, std::string channelName);

    // AC ChatHelper::ExtractAllQuestIds / ExtractAllItemIds (Bot/Cmd/ChatHelper.cpp:696-733)
    static std::set<uint32> ExtractAllQuestIds(std::string const& text);
    static std::set<uint32> ExtractAllItemIds(std::string const& text);

    // AC PlayerbotAI::QueueChatResponse (Bot/PlayerbotAI.cpp:6288) and the drain at the top of
    // PlayerbotAI::UpdateAIInternal (Bot/PlayerbotAI.cpp:644-656); see the file header for the per-bot table.
    static void QueueChatResponse(Player* bot, ChatQueuedReply const& reply);
    static void ProcessChatReplies(Player* bot);
    // AC "last said"::"chat" value (Ai/Base/Value/LastSaidValue.h, default time(nullptr) - 120)
    static time_t GetLastChatTime(Player* bot);
    static void SetLastChatTime(Player* bot, time_t value);
};

#endif
