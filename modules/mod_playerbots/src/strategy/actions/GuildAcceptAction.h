/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * #258 lane Q: AC Ai/Base/Actions/GuildAcceptAction.h. Fired by the bot's own SMSG_GUILD_INVITE
 * (AC WorldPacketHandlerStrategy.cpp:49 "guild invite" -> "guild accept").
 */

#ifndef _PLAYERBOT_GUILDACCEPTACTION_H
#define _PLAYERBOT_GUILDACCEPTACTION_H

#include "Action.h"

class PlayerbotAI;

class GuildAcceptAction : public Action
{
public:
    GuildAcceptAction(PlayerbotAI* botAI) : Action(botAI, "guild accept") {}

    bool Execute(Event event) override;
};

#endif
