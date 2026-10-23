/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatFilter.h"

std::string const ChatFilter::Filter(std::string& message) { return message; }

// No concrete filters yet (see the header comment) - the list starts empty, so composing is a no-op
// until a later batch appends real filters here.
CompositeChatFilter::CompositeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

CompositeChatFilter::~CompositeChatFilter()
{
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); ++i)
        delete *i;
}

std::string const CompositeChatFilter::Filter(std::string& message)
{
    std::string current = message;
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); ++i)
        current = (*i)->Filter(current);

    return current;
}
