/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/InviteToGroupAction.h (#149; GroupStrategy.cpp already
 * referenced "invite nearby"/"invite guild" as dead action names). AC's Invite() builds a hand-crafted
 * CMSG_GROUP_INVITE WorldPacket and feeds it to WorldSession::HandleGroupInviteOpcode(); this core's
 * opcode handler (GroupHandler.cpp) reads a bit-packed player name/realm GUID that is fragile to
 * reconstruct by hand, so Invite() below calls the same Group::AddInvite()/AddLeaderInvite() the
 * handler itself calls, skipping packet (de)serialization entirely.
 *
 * AC's isUseful()/Execute() also lean on several concepts this fork does not have - GrouperType,
 * HasGameClientMaster(), PlayerbotOperations/PlayerbotWorldThreadProcessor (raid auto-convert),
 * PlayerbotTextMgr/BroadcastHelper (invite chat) - same gap already called out in LeaveGroupAction.h
 * for LeaveFarAwayAction. Those are dropped rather than guessed at. This is also a
 * server with no config toggle for this behaviour (PlayerbotAIConfig.h is off-limits to add one to),
 * so both actions only ever recruit *other unowned bots* (GET_PLAYERBOT_AI() set, not real-player-
 * controlled) - never a real human player - to keep the blast radius at zero for anyone actually
 * playing on the server.
 */

#ifndef _PLAYERBOT_INVITETOGROUPACTION_H
#define _PLAYERBOT_INVITETOGROUPACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

class InviteToGroupAction : public Action
{
public:
    InviteToGroupAction(PlayerbotAI* botAI, std::string const name = "invite") : Action(botAI, name) {}

    bool Execute(Event event) override;

    /// Shared invite logic: puts `player` on `inviter`'s pending group invite, creating the group if
    /// `inviter` is not already in one. Mirrors WorldSession::HandleGroupInviteOpcode's checks
    /// (GroupHandler.cpp) without going through the opcode/WorldPacket layer.
    virtual bool Invite(Player* inviter, Player* player);
};

// Ported from AC mod-playerbots InviteToGroupAction.h:38-42 (#177): the "join" chat command - the
// bot's master invites the bot into the *master's own* group (Invite() with the arguments reversed
// from the plain "invite" command). Unlike InviteNearbyToGroupAction/InviteGuildToGroupAction this
// always targets the command's sender, so the real-player-safety gate on those two classes does not
// apply here - the bot only ever joins whoever is already allowed to command it.
class JoinGroupAction : public InviteToGroupAction
{
public:
    JoinGroupAction(PlayerbotAI* botAI, std::string const name = "join") : InviteToGroupAction(botAI, name) {}

    bool Execute(Event event) override;
};

class InviteNearbyToGroupAction : public InviteToGroupAction
{
public:
    InviteNearbyToGroupAction(PlayerbotAI* botAI, std::string const name = "invite nearby")
        : InviteToGroupAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

class InviteGuildToGroupAction : public InviteNearbyToGroupAction
{
public:
    InviteGuildToGroupAction(PlayerbotAI* botAI, std::string const name = "invite guild")
        : InviteNearbyToGroupAction(botAI, name)
    {
    }

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
