/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Bot/Cmd/ChatFilter.h (#173). AC's ChatFilter.cpp (611 LOC) adds
 * concrete filters (@tank/@heal/@class/@rti/@level/@group/@spec/@aura/@aggroby placeholders in
 * chat-command text); none of those placeholders are used by anything ported in this pass, so
 * CompositeChatFilter is built with an empty filter list (identity transform) here. Later batches
 * add filters via the constructor without touching PlayerbotAI.h again.
 */

#ifndef _PLAYERBOT_CHATFILTER_H
#define _PLAYERBOT_CHATFILTER_H

#include "PlayerbotAIAware.h"
#include <string>
#include <vector>

class PlayerbotAI;

class ChatFilter : public PlayerbotAIAware
{
public:
    ChatFilter(PlayerbotAI* botAI) : PlayerbotAIAware(botAI) {}
    virtual ~ChatFilter() {}

    virtual std::string const Filter(std::string& message);
};

class CompositeChatFilter : public ChatFilter
{
public:
    CompositeChatFilter(PlayerbotAI* botAI);

    virtual ~CompositeChatFilter();
    std::string const Filter(std::string& message) override;

private:
    std::vector<ChatFilter*> filters;
};

#endif
