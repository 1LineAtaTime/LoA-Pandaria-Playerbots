/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/ChangeChatAction.cpp. AC ChatHelper::parseChat / FormatChat
 * (Bot/Cmd/ChatHelper.cpp:119-126,472-495) are file-local here: this fork ChatHelper has no chat-channel
 * table.
 */

#include "ChangeChatAction.h"

#include "Event.h"
#include "Playerbots.h"

namespace
{
// AC ChatHelper::parseChat.
ChatMsg ParseChat(std::string const& text)
{
    static std::map<std::string, ChatMsg> chats;
    if (chats.empty())
    {
        chats["party"] = CHAT_MSG_PARTY;
        chats["p"] = CHAT_MSG_PARTY;
        chats["guild"] = CHAT_MSG_GUILD;
        chats["g"] = CHAT_MSG_GUILD;
        chats["raid"] = CHAT_MSG_RAID;
        chats["r"] = CHAT_MSG_RAID;
        chats["whisper"] = CHAT_MSG_WHISPER;
        chats["w"] = CHAT_MSG_WHISPER;
    }

    std::map<std::string, ChatMsg>::const_iterator itr = chats.find(text);
    if (itr != chats.end())
        return itr->second;

    return CHAT_MSG_SYSTEM;
}

// AC ChatHelper::FormatChat.
std::string FormatChat(ChatMsg chat)
{
    switch (chat)
    {
        case CHAT_MSG_GUILD:
            return "guild";
        case CHAT_MSG_PARTY:
            return "party";
        case CHAT_MSG_WHISPER:
            return "whisper";
        case CHAT_MSG_RAID:
            return "raid";
        default:
            break;
    }

    return "unknown";
}

// AC keeps this in the "chat" value (ChatValue, a ManualSetValue defaulting to whisper) that only this command reads
// and writes; this fork has no such value, so the per-bot setting lives here.
std::map<ObjectGuid::LowType, ChatMsg> chatByBot;

ChatMsg GetChat(Player* bot)
{
    std::map<ObjectGuid::LowType, ChatMsg>::const_iterator itr = chatByBot.find(bot->GetGUID().GetCounter());
    return itr != chatByBot.end() ? itr->second : CHAT_MSG_WHISPER;
}
}  // namespace

bool ChangeChatAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ChatMsg parsed = ParseChat(text);
    if (parsed == CHAT_MSG_SYSTEM)
    {
        std::ostringstream out;
        out << "Current chat is " << FormatChat(GetChat(bot));
        botAI->TellMaster(out);
    }
    else
    {
        chatByBot[bot->GetGUID().GetCounter()] = parsed;

        std::ostringstream out;
        out << "Chat set to " << FormatChat(parsed);
        botAI->TellMaster(out);
    }

    return true;
}
