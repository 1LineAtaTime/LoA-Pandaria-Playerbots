/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported from AC mod-playerbots src/Script/WorldThr/PlayerbotOperations.h (MapUpdate.Threads > 1 safety). Every
 * operation AC defines is here; the ones whose AC dependencies this fork does not have are kept as dormant,
 * commented-out code (as AC keeps dormant code), each with the reason:
 *  - ArenaGroupFormationOperation: dormant in AC too - nothing queues it; AC (and this fork,
 *    BattleGroundJoinAction.cpp) form the arena party inline because the queue join follows immediately.
 *  - BotLogoutGroupCleanupOperation: needs PlayerbotRepository::Save (this fork's sPlayerbotDbStore is disabled).
 *  - ArenaTeamAssignOperation: needs RandomPlayerbotFactory::AssignBotToArenaTeamInternal (not on this fork).
 * LOG_* {} -> TC_LOG_* %s; sPlayerbotsMgr / sRandomPlayerbotMgr / sPlayerbotAIConfig are pointers here;
 * sWorldSessionMgr->FindSession -> sWorld->FindSession.
 */

#ifndef PLAYERBOTS_PLAYERBOTOPERATIONS_H
#define PLAYERBOTS_PLAYERBOTOPERATIONS_H

#include "Group.h"
#include "GroupMgr.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotMgr.h"
#include "PlayerbotOperation.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "UseMeetingStoneAction.h"
#include "World.h"
#include "WorldSession.h"

// Group invite operation
class GroupInviteOperation : public PlayerbotOperation
{
public:
    GroupInviteOperation(ObjectGuid botGuid, ObjectGuid targetGuid) : m_botGuid(botGuid), m_targetGuid(targetGuid) {}

    bool Execute() override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        Player* target = ObjectAccessor::FindPlayer(m_targetGuid);

        if (!bot || !target)
        {
            TC_LOG_DEBUG("playerbots", "GroupInviteOperation: Bot or target not found");
            return false;
        }

        // Check if target is already in a group
        if (target->GetGroup())
        {
            TC_LOG_DEBUG("playerbots", "GroupInviteOperation: Target %s is already in a group", target->GetName().c_str());
            return false;
        }

        Group* group = bot->GetGroup();

        // Create group if bot doesn't have one
        if (!group)
        {
            group = new Group;
            if (!group->Create(bot))
            {
                delete group;
                TC_LOG_ERROR("playerbots", "GroupInviteOperation: Failed to create group for bot %s", bot->GetName().c_str());
                return false;
            }
            sGroupMgr->AddGroup(group);
            TC_LOG_DEBUG("playerbots", "GroupInviteOperation: Created new group for bot %s", bot->GetName().c_str());
        }

        // Convert to raid if needed (more than 5 members)
        if (!group->isRaidGroup() && group->GetMembersCount() >= 5)
        {
            group->ConvertToRaid();
            TC_LOG_DEBUG("playerbots", "GroupInviteOperation: Converted group to raid");
        }

        // Add member to group
        if (group->AddMember(target))
        {
            TC_LOG_DEBUG("playerbots", "GroupInviteOperation: Successfully added %s to group", target->GetName().c_str());
            if (sPlayerbotAIConfig->summonWhenGroup && target->GetDistance(bot) > sPlayerbotAIConfig->sightDistance)
            {
                PlayerbotAI* targetAI = GET_PLAYERBOT_AI(target);
                if (targetAI)
                {
                    SummonAction summonAction(targetAI, "group summon");
                    summonAction.Teleport(bot, target, true);
                }
            }
            return true;
        }
        else
        {
            TC_LOG_ERROR("playerbots", "GroupInviteOperation: Failed to add %s to group", target->GetName().c_str());
            return false;
        }
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }

    uint32 GetPriority() const override { return 50; }  // High priority (player-facing)

    std::string GetName() const override { return "GroupInvite"; }

    bool IsValid() const override
    {
        // Check if bot still exists and is online
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        Player* target = ObjectAccessor::FindPlayer(m_targetGuid);
        return bot && target;
    }

private:
    ObjectGuid m_botGuid;
    ObjectGuid m_targetGuid;
};

// Remove member from group
class GroupRemoveMemberOperation : public PlayerbotOperation
{
public:
    GroupRemoveMemberOperation(ObjectGuid botGuid, ObjectGuid targetGuid) : m_botGuid(botGuid), m_targetGuid(targetGuid) {}

    bool Execute() override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        Player* target = ObjectAccessor::FindPlayer(m_targetGuid);

        if (!bot || !target)
            return false;

        Group* group = bot->GetGroup();
        if (!group)
        {
            TC_LOG_DEBUG("playerbots", "GroupRemoveMemberOperation: Bot is not in a group");
            return false;
        }

        if (!group->IsMember(target->GetGUID()))
        {
            TC_LOG_DEBUG("playerbots", "GroupRemoveMemberOperation: Target is not in bot's group");
            return false;
        }

        group->RemoveMember(target->GetGUID());
        TC_LOG_DEBUG("playerbots", "GroupRemoveMemberOperation: Removed %s from group", target->GetName().c_str());
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }

    uint32 GetPriority() const override { return 50; }

    std::string GetName() const override { return "GroupRemoveMember"; }

    bool IsValid() const override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        return bot != nullptr;
    }

private:
    ObjectGuid m_botGuid;
    ObjectGuid m_targetGuid;
};

// Convert group to raid
class GroupConvertToRaidOperation : public PlayerbotOperation
{
public:
    GroupConvertToRaidOperation(ObjectGuid botGuid) : m_botGuid(botGuid) {}

    bool Execute() override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        if (!bot)
            return false;

        Group* group = bot->GetGroup();
        if (!group)
        {
            TC_LOG_DEBUG("playerbots", "GroupConvertToRaidOperation: Bot is not in a group");
            return false;
        }

        if (group->isRaidGroup())
        {
            TC_LOG_DEBUG("playerbots", "GroupConvertToRaidOperation: Group is already a raid");
            return true;  // Success - already in desired state
        }

        group->ConvertToRaid();
        TC_LOG_DEBUG("playerbots", "GroupConvertToRaidOperation: Converted group to raid");
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }

    uint32 GetPriority() const override { return 50; }

    std::string GetName() const override { return "GroupConvertToRaid"; }

    bool IsValid() const override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        return bot != nullptr;
    }

private:
    ObjectGuid m_botGuid;
};

// Set group leader
class GroupSetLeaderOperation : public PlayerbotOperation
{
public:
    GroupSetLeaderOperation(ObjectGuid botGuid, ObjectGuid newLeaderGuid) : m_botGuid(botGuid), m_newLeaderGuid(newLeaderGuid) {}

    bool Execute() override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        Player* newLeader = ObjectAccessor::FindPlayer(m_newLeaderGuid);

        if (!bot || !newLeader)
            return false;

        Group* group = bot->GetGroup();
        if (!group)
        {
            TC_LOG_DEBUG("playerbots", "GroupSetLeaderOperation: Bot is not in a group");
            return false;
        }

        if (!group->IsMember(newLeader->GetGUID()))
        {
            TC_LOG_DEBUG("playerbots", "GroupSetLeaderOperation: New leader is not in the group");
            return false;
        }

        group->ChangeLeader(newLeader->GetGUID());
        group->SendUpdate();
        TC_LOG_DEBUG("playerbots", "GroupSetLeaderOperation: Changed leader to %s", newLeader->GetName().c_str());
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }

    uint32 GetPriority() const override { return 50; }

    std::string GetName() const override { return "GroupSetLeader"; }

    bool IsValid() const override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        Player* newLeader = ObjectAccessor::FindPlayer(m_newLeaderGuid);
        return bot && newLeader;
    }

private:
    ObjectGuid m_botGuid;
    ObjectGuid m_newLeaderGuid;
};

// Form arena group - dormant on this fork (see the file header); AC PlayerbotOperations.h:270-415.
/*
class ArenaGroupFormationOperation : public PlayerbotOperation
{
    ... AC: removes the members from their groups, disbands the leader's group, creates a new group with the
    leader, adds the level >= 70 members, resets their AI and teleports them to the leader
    (RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP); two calls on
    this core, see BattleGroundJoinAction.cpp), then checks the required arena size. Priority 60.
};
*/

// Bot logout group cleanup operation - dormant on this fork (see the file header); AC PlayerbotOperations.h:417-452.
/*
class BotLogoutGroupCleanupOperation : public PlayerbotOperation
{
    ... AC: if the bot is in a group, not in a battleground (queue) and its master is a real player,
    PlayerbotRepository::instance().Save(botAI). Priority 70.
};
*/

// Add player bot operation (for logging in bots from map threads)
class AddPlayerBotOperation : public PlayerbotOperation
{
public:
    AddPlayerBotOperation(ObjectGuid botGuid, uint32 masterAccountId) : m_botGuid(botGuid), m_masterAccountId(masterAccountId) {}

    bool Execute() override
    {
        sRandomPlayerbotMgr->AddPlayerBot(m_botGuid, m_masterAccountId);
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }

    uint32 GetPriority() const override { return 50; }  // High priority

    std::string GetName() const override { return "AddPlayerBot"; }

    bool IsValid() const override { return !ObjectAccessor::FindConnectedPlayer(m_botGuid); }

private:
    ObjectGuid m_botGuid;
    uint32 m_masterAccountId;
};

class OnBotLoginOperation : public PlayerbotOperation
{
public:
    OnBotLoginOperation(ObjectGuid botGuid, uint32 masterAccountId) : m_botGuid(botGuid), m_masterAccountId(masterAccountId) {}

    bool Execute() override
    {
        // find and verify bot still exists
        Player* bot = ObjectAccessor::FindConnectedPlayer(m_botGuid);
        if (!bot)
            return false;

        PlayerbotHolder* holder = sRandomPlayerbotMgr;
        if (m_masterAccountId)
        {
            WorldSession* masterSession = sWorld->FindSession(m_masterAccountId);
            Player* masterPlayer = masterSession ? masterSession->GetPlayer() : nullptr;
            if (masterPlayer)
                holder = GET_PLAYERBOT_MGR(masterPlayer);
        }

        if (!holder)
            return false;

        holder->OnBotLogin(bot);
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }
    uint32 GetPriority() const override { return 100; }
    std::string GetName() const override { return "OnBotLogin"; }

    bool IsValid() const override { return ObjectAccessor::FindConnectedPlayer(m_botGuid) != nullptr; }

private:
    ObjectGuid m_botGuid;
    uint32 m_masterAccountId = 0;
};

// Arena team assignment - dormant on this fork (see the file header); AC PlayerbotOperations.h:527-550.
/*
class ArenaTeamAssignOperation : public PlayerbotOperation
{
    ... AC: RandomPlayerbotFactory::AssignBotToArenaTeamInternal(bot). Default priority.
};
*/

#endif
