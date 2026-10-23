/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/GuildAcceptAction.cpp for #258. 5.4.8 differences:
 *  - AC reads the inviter NAME out of SMSG_GUILD_INVITE (a leading string). On 5.4.8 that packet is bit-packed with
 *    the inviter name behind two guild guids (Guild::HandleInviteMember, Guilds/Guild.cpp:2154-2205), so parsing it
 *    is needless: the same function stores the inviter with SetLastGuildInviterGUID() (Guild.cpp:2146) and the
 *    guild with SetGuildIdInvited() right before sending, which is exactly what is read here.
 *  - CMSG_GUILD_ACCEPT / CMSG_GUILD_DECLINE carry no body on 5.4.8 (WorldSession::HandleGuildAcceptOpcode /
 *    HandleGuildDeclineOpcode, Handlers/GuildHandler.cpp:124,133 ignore the packet), so an empty WorldPacket is
 *    passed.
 *  - PlayerbotTextMgr is not ported on this fork; the three texts are AC's own enUS defaults.
 */

#include "GuildAcceptAction.h"

#include "Event.h"
#include "ObjectAccessor.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

bool GuildAcceptAction::Execute(Event /*event*/)
{
    Player* inviter = ObjectAccessor::FindPlayer(bot->GetLastGuildInviterGUID());
    if (!inviter)
        return false;

    bool accept = true;
    uint32 guildId = inviter->GetGuildId();
    if (!guildId)
    {
        botAI->TellError("You are not in a guild!");
        accept = false;
    }
    else if (bot->GetGuildId())
    {
        botAI->TellError("Sorry, I am in a guild already");
        accept = false;
    }
    else if (!botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_INVITE, false, inviter, true))
    {
        botAI->TellError("Sorry, I don't want to join your guild :(");
        accept = false;
    }

    WorldPacket data;
    if (accept)
        bot->GetSession()->HandleGuildAcceptOpcode(data);
    else
        bot->GetSession()->HandleGuildDeclineOpcode(data);

    return true;
}
