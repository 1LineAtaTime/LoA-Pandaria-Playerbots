/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ExternalEventHelper.h"

//#include "ChatHelper.h"
#include "Playerbots.h"
#include "Trigger.h"

// AC ChatHelper::parseableItem, defined in AI/ChatHelper.cpp (see the note there).
namespace PlayerbotChatParse
{
bool ParseableItem(std::string const& text);
}

bool ExternalEventHelper::ParseChatCommand(std::string const command, Player* owner)
{
    if (HandleCommand(command, "", owner))
        return true;

    size_t i = std::string::npos;
    while (true)
    {
        size_t found = command.rfind(" ", i);
        if (found == std::string::npos || !found)
            break;

        std::string const name = command.substr(0, found);
        std::string const param = command.substr(found + 1);

        i = found - 1;

        if (HandleCommand(name, param, owner))
            return true;
    }

    // AC Bot/Engine/ExternalEventHelper.cpp:33-40: only a command that names an item (link, class, quality, slot,
    // money...) falls through to the item-count / trade commands, and only with AiPlayerbot.EnableAutoTradeOnItemMention.
    // Without this gate every unknown whisper, such as "talents spec list" before "talents" existed, opened a trade
    // window with the master. ("c" has no trigger on this fork yet, so that call finds nothing, as before.)
    if (!PlayerbotChatParse::ParseableItem(command))
        return false;

    if (sPlayerbotAIConfig->enableAutoTradeOnItemMention)
    {
        HandleCommand("c", command, owner);
        HandleCommand("t", command, owner);
    }

    return true;
}

void ExternalEventHelper::HandlePacket(std::map<uint16, std::string>& handlers, WorldPacket const& packet,
                                       Player* owner)
{
    uint16 opcode = packet.GetOpcode();
    std::string const name = handlers[opcode];
    if (name.empty())
        return;

    Trigger* trigger = aiObjectContext->GetTrigger(name);
    if (!trigger)
    {
        TC_LOG_ERROR("playerbots", "No trigger defined for packet: %s", name.c_str());
        return;
    }

    WorldPacket p(packet);
    trigger->ExternalEvent(p, owner);
}

bool ExternalEventHelper::HandleCommand(std::string const name, std::string const param, Player* owner)
{
    Trigger* trigger = aiObjectContext->GetTrigger(name);
    if (!trigger)
        return false;

    trigger->ExternalEvent(param, owner);

    return true;
}
