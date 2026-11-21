/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/InviteToGroupAction.h/.cpp: "invite", "join", "invite nearby",
 * "invite guild" and the "lfg" chat command (LfgAction). AC is the spec; the 5.4.8 adaptations are:
 *  - AC's Invite() builds a CMSG_GROUP_INVITE WorldPacket and feeds it to WorldSession::HandleGroupInviteOpcode().
 *    This core's opcode handler (GroupHandler.cpp) reads a bit-packed player name/realm GUID that is fragile to
 *    reconstruct by hand, so Invite() calls the same Group::AddInvite()/AddLeaderInvite() the handler itself calls.
 *  - AC's free functions IsRealPlayer(p) (p && !GET_PLAYERBOT_AI(p)) and IsSelfBot(p) (the bot's master is itself)
 *    are file-local helpers in the .cpp; AC GetGroupLeader() is this fork's GetGroupMaster() (same body).
 *  - LfgAction: AC IsTank/IsHeal(player, bySpec) are PlayerBotSpec::IsTank/IsHeal here, which read the 5.4.8
 *    specialisation (monk brewmaster/mistweaver included). DEFAULT_MAX_LEVEL is the MoP 90 (DBCEnums.h).
 * Default state, as AC: "invite"/"join"/"lfg" are chat commands (ON). "invite nearby"/"invite guild" belong to the
 * "group" strategy, which AiFactory never adds (as AC), and are gated by AiPlayerbot.RandomBotGroupNearby = 0 (dormant).
 */

#ifndef _PLAYERBOT_INVITETOGROUPACTION_H
#define _PLAYERBOT_INVITETOGROUPACTION_H

#include "Action.h"

#include <vector>

class Player;
class PlayerbotAI;

class InviteToGroupAction : public Action
{
public:
    InviteToGroupAction(PlayerbotAI* botAI, std::string const name = "invite") : Action(botAI, name) {}

    bool Execute(Event event) override;

    /// AC InviteToGroupAction.cpp:25-53. Puts `player` on `inviter`'s pending group invite, creating the group if
    /// `inviter` is not already in one (see the file header for why the opcode handler is not used).
    virtual bool Invite(Player* inviter, Player* player);
};

// AC InviteToGroupAction.h:38-43: the "join" chat command - the command's sender invites the bot into the sender's
// own group (Invite() with the arguments reversed from the plain "invite" command).
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

// AC InviteToGroupAction.h:53-65: generic guild member finder
class FindGuildMembers
{
public:
    FindGuildMembers() {}

    void operator()(Player* player) { data.push_back(player); }

    std::vector<Player*> const GetResult() { return data; }

private:
    std::vector<Player*> data;
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

private:
    std::vector<Player*> getGuildMembers();
};

// AC InviteToGroupAction.h:82-87 / .cpp:287-467: the "lfg" chat command. A player whispers "lfg" (optionally
// 5/10/20/25/40) to a random bot, or says it in a chat channel (RandomPlayerbotMgr::HandleCommand routes channel
// chat to every random bot); each bot within 4 levels whose role still has a free slot in the requester's group
// leaves its own group, invites itself into the requester's group and accepts.
class LfgAction : public InviteToGroupAction
{
public:
    LfgAction(PlayerbotAI* botAI, std::string const name = "lfg") : InviteToGroupAction(botAI, name) {}

    bool Execute(Event event) override;
};

#endif
