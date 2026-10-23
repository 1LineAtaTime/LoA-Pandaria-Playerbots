/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "InviteToGroupAction.h"

#include "BroadcastHelper.h"
#include "Event.h"
#include "GuildMgr.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"

namespace
{
    /// True only for a bot with no real human behind it anywhere in its master chain - see the class
    /// comment in InviteToGroupAction.h for why real players are never touched by these actions.
    bool IsUnownedBot(Player* player)
    {
        PlayerbotAI* ai = GET_PLAYERBOT_AI(player);
        return ai && !ai->IsRealPlayer() && !ai->HasRealPlayerMaster();
    }

    /// Generic guild member finder (AC InviteToGroupAction.h FindGuildMembers).
    class FindGuildMembers
    {
    public:
        void operator()(Player* player) { data.push_back(player); }
        std::vector<Player*> const& GetResult() const { return data; }

    private:
        std::vector<Player*> data;
    };
}

bool InviteToGroupAction::Invite(Player* inviter, Player* player)
{
    if (!inviter || !player || inviter == player)
        return false;

    if (player->GetGroup() || player->GetGroupInvite())
        return false;

    Group* group = inviter->GetGroup();
    if (group && group->isBGGroup())
        group = inviter->GetOriginalGroup();

    if (group)
    {
        if (!group->IsLeader(inviter->GetGUID()) && !group->IsAssistant(inviter->GetGUID()))
            return false;

        if (group->IsFull())
            return false;

        if (!group->AddInvite(player))
            return false;
    }
    else
    {
        group = new Group();
        if (!group->AddLeaderInvite(inviter) || !group->AddInvite(player))
        {
            delete group;
            return false;
        }
    }

    player->GetSession()->SendGroupInviteNotification(inviter->GetName(), false);
    return true;
}

bool InviteToGroupAction::Execute(Event event)
{
    Player* target = event.getOwner();
    return Invite(bot, target);
}

// Ported from AC mod-playerbots InviteToGroupAction.cpp:254-282 (#177).
bool JoinGroupAction::Execute(Event event)
{
    if (bot->InBattleground() || bot->InBattlegroundQueue())
        return false;

    Player* master = event.getOwner();
    if (!master)
        return false;

    if (Group* group = master->GetGroup())
    {
        if (group->IsFull())
            return false;

        if (bot->GetGroup() == group)
            return false;
    }

    if (bot->GetGroup())
    {
        if (botAI->HasRealPlayerMaster())
            return false;

        if (!botAI->DoSpecificAction("leave", event, true))
            return false;
    }

    return Invite(master, bot);
}

bool InviteNearbyToGroupAction::isUseful()
{
    // Only a standalone bot (no real player anywhere in its master chain) recruits a pickup group of
    // its own; see the class comment in InviteToGroupAction.h.
    if (!IsUnownedBot(bot))
        return false;

    if (bot->InBattleground() || bot->InBattlegroundQueue())
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (!group->IsLeader(bot->GetGUID()))
            return false;

        if (group->GetMembersCount() >= (group->isRaidGroup() ? 40u : 5u))
            return false;
    }

    return true;
}

bool InviteNearbyToGroupAction::Execute(Event /*event*/)
{
    GuidVector const nearGuids = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest friendly players")->Get();
    for (ObjectGuid const& guid : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player || player == bot)
            continue;

        if (player->GetMapId() != bot->GetMapId())
            continue;

        if (!IsUnownedBot(player))
            continue;

        if (player->GetGroup() || player->GetGroupInvite())
            continue;

        if (player->isDND() || player->IsBeingTeleported())
            continue;

        if (abs(int32(player->GetLevel()) - int32(bot->GetLevel())) > 2)
            continue;

        if (bot->GetDistance2d(player) > sPlayerbotAIConfig->sightDistance)
            continue;

        return Invite(bot, player);
    }

    return false;
}

bool InviteGuildToGroupAction::isUseful()
{
    return bot->GetGuildId() && InviteNearbyToGroupAction::isUseful();
}

bool InviteGuildToGroupAction::Execute(Event /*event*/)
{
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    FindGuildMembers worker;
    guild->BroadcastWorker(worker);

    for (Player* player : worker.GetResult())
    {
        if (!player || player == bot)
            continue;

        if (!IsUnownedBot(player))
            continue;

        if (player->GetGroup() || player->GetGroupInvite())
            continue;

        if (player->isDND() || player->IsBeingTeleported())
            continue;

        if (abs(int32(player->GetLevel()) - int32(bot->GetLevel())) > 5)
            continue;

        if (bot->GetMapId() != player->GetMapId() && bot->GetDistance2d(player) > sPlayerbotAIConfig->sightDistance)
            continue;

        // #322: AC InviteToGroupAction.cpp:242-246 (guild invite chat). AC IsRealPlayer(p) = p && !GET_PLAYERBOT_AI(p).
        if (sPlayerbotAIConfig->inviteChat)
        {
            Player* inviteMaster = botAI->GetMaster();
            if (sRandomPlayerbotMgr->IsRandomBot(bot) || !(inviteMaster && !GET_PLAYERBOT_AI(inviteMaster)))
                BroadcastHelper::BroadcastGuildGroupOrRaidInvite(botAI, bot, player, bot->GetGroup());
        }

        return Invite(bot, player);
    }

    return false;
}
