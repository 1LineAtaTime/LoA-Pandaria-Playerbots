/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Actions/GuildManagementActions.h. The class layout, names and the opcode
 * member are AC's; SendPacket hands the request to the world thread instead of queuing a packet (see the .cpp).
 */

#ifndef PLAYERBOTS_GUILDMANAGEMENTACTIONS_H
#define PLAYERBOTS_GUILDMANAGEMENTACTIONS_H

#include "Action.h"
#include "Opcodes.h"

class Player;
class PlayerbotAI;

class GuidManageAction : public Action
{
public:
    GuidManageAction(PlayerbotAI* botAI, std::string const name = "guild manage", uint16 opcode = CMSG_GUILD_INVITE)
        : Action(botAI, name), opcode(opcode)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override { return false; }

protected:
    // AC: virtual void SendPacket(WorldPacket const& packet). 5.4.8 bot sessions never drain queued packets, so the
    // target is handed over directly (see GuildManagementActions.cpp).
    virtual void SendPacket(Player* player);
    virtual Player* GetPlayer(Event event);
    virtual bool PlayerIsValid(Player* member);
    virtual uint8 GetRankId(Player* member);

    uint16 opcode;
};

class GuildInviteAction : public GuidManageAction
{
public:
    GuildInviteAction(PlayerbotAI* botAI, std::string const name = "guild invite", uint16 opcode = CMSG_GUILD_INVITE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    bool PlayerIsValid(Player* member) override;
};

class GuildPromoteAction : public GuidManageAction
{
public:
    GuildPromoteAction(PlayerbotAI* botAI, std::string const name = "guild promote", uint16 opcode = CMSG_GUILD_PROMOTE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    bool PlayerIsValid(Player* member) override;
};

class GuildDemoteAction : public GuidManageAction
{
public:
    GuildDemoteAction(PlayerbotAI* botAI, std::string const name = "guild demote", uint16 opcode = CMSG_GUILD_DEMOTE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    bool PlayerIsValid(Player* member) override;
};

class GuildRemoveAction : public GuidManageAction
{
public:
    GuildRemoveAction(PlayerbotAI* botAI, std::string const name = "guild remove", uint16 opcode = CMSG_GUILD_REMOVE)
        : GuidManageAction(botAI, name, opcode)
    {
    }

    bool isUseful() override;

protected:
    bool PlayerIsValid(Player* member) override;
};

class GuildManageNearbyAction : public Action
{
public:
    GuildManageNearbyAction(PlayerbotAI* botAI) : Action(botAI, "guild manage nearby") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class GuildLeaveAction : public Action
{
public:
    GuildLeaveAction(PlayerbotAI* botAI) : Action(botAI, "guild leave") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
