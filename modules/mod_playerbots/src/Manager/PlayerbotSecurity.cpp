/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Mgr/Security/PlayerbotSecurity.cpp (#173), adapted to this fork's
 * PlayerbotAI API:
 *  - AC's botAI->GetEquipGearScore(player) -> PlayerbotAI::GetMixedGearScore(player, true, false)
 *    (this fork's gearscore helper is static and takes bags/bank flags explicitly, see
 *    AI/PlayerbotAI.h:110-111 / AI/PlayerbotAI.cpp:2862).
 *  - AC's botAI->GetGroupLeader() -> botAI->GetGroupMaster() (this fork has no GetGroupLeader();
 *    GetGroupMaster() returns the same "group leader, else master" value, see
 *    AI/PlayerbotAI.cpp:1577-1584).
 *  - AC's free function IsSelfBot(player) -> PlayerbotAI::IsRealPlayer() (established fork mapping,
 *    see strategy/actions/LootAction.cpp:121).
 *  - sPlayerbotAIConfig is a pointer on this fork (PlayerbotAIConfig::instance()), so every access
 *    below uses "->" instead of AC's ".".
 */

#include "PlayerbotSecurity.h"
#include "RBAC.h"

#include "Creature.h"

#include "CharacterCache.h"
#include "DBCStores.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "World.h"

PlayerbotSecurity::PlayerbotSecurity(Player* const bot) : bot(bot), account(0)
{
    if (bot)
        account = sCharacterCache->GetCharacterAccountIdByGuid(bot->GetGUID());
}

PlayerbotSecurityLevel PlayerbotSecurity::LevelFor(Player* from, DenyReason* reason, bool ignoreGroup)
{
    if (!bot || !from || !from->GetSession())
    {
        if (reason)
            *reason = PLAYERBOT_DENY_NONE;

        return PLAYERBOT_SECURITY_DENY_ALL;
    }

    // GMs always have full access.
    if (from->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_GM))
        return PLAYERBOT_SECURITY_ALLOW_ALL;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
    {
        if (reason)
            *reason = PLAYERBOT_DENY_NONE;

        return PLAYERBOT_SECURITY_DENY_ALL;
    }

    if (botAI->IsOpposing(from))
    {
        if (reason)
            *reason = PLAYERBOT_DENY_OPPOSING;

        return PLAYERBOT_SECURITY_DENY_ALL;
    }

    if (sPlayerbotAIConfig->IsInRandomAccountList(account))
    {
        // (duplicate check in case of faction change)
        if (botAI->IsOpposing(from))
        {
            if (reason)
                *reason = PLAYERBOT_DENY_OPPOSING;

            return PLAYERBOT_SECURITY_DENY_ALL;
        }

        Group* fromGroup = from->GetGroup();
        Group* botGroup = bot->GetGroup();

        if (fromGroup && botGroup && fromGroup == botGroup && !ignoreGroup)
        {
            if (botAI->GetMaster() == from)
                return PLAYERBOT_SECURITY_ALLOW_ALL;

            if (reason)
                *reason = PLAYERBOT_DENY_NOT_YOURS;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 0)
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NONE;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (sPlayerbotAIConfig->groupInvitationPermission <= 1)
        {
            int32 levelDiff = int32(bot->GetLevel()) - int32(from->GetLevel());
            if (levelDiff > 5)
            {
                if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
                {
                    if (reason)
                        *reason = PLAYERBOT_DENY_LOW_LEVEL;

                    return PLAYERBOT_SECURITY_TALK;
                }
            }
        }

        int32 botGS = static_cast<int32>(PlayerbotAI::GetMixedGearScore(bot, true, false));
        int32 fromGS = static_cast<int32>(PlayerbotAI::GetMixedGearScore(from, true, false));

        if (sPlayerbotAIConfig->gearscorecheck && botGS && bot->GetLevel() > 15 && botGS > fromGS)
        {
            uint32 diffPct = uint32(100 * (botGS - fromGS) / botGS);
            uint32 reqPct = uint32(12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->GetLevel());

            if (diffPct >= reqPct)
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_GEARSCORE;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        if (bot->InBattlegroundQueue())
        {
            if (!bot->GetGuildId() || bot->GetGuildId() != from->GetGuildId())
            {
                if (reason)
                    *reason = PLAYERBOT_DENY_BG;

                return PLAYERBOT_SECURITY_TALK;
            }
        }

        // If the bot is not in a group, offer an invite.
        botGroup = bot->GetGroup();
        if (!botGroup)
        {
            if (reason)
                *reason = PLAYERBOT_DENY_INVITE;

            return PLAYERBOT_SECURITY_INVITE;
        }

        if (!ignoreGroup && botGroup->IsFull())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_FULL_GROUP;

            return PLAYERBOT_SECURITY_TALK;
        }

        if (!ignoreGroup && botGroup->GetLeaderGUID() != bot->GetGUID())
        {
            if (reason)
                *reason = PLAYERBOT_DENY_NOT_LEADER;

            return PLAYERBOT_SECURITY_TALK;
        }

        // The bot is the group leader: the initiator can be invited.
        if (reason)
            *reason = PLAYERBOT_DENY_IS_LEADER;

        return PLAYERBOT_SECURITY_INVITE;
    }

    // Non-random bots: only their master has full access.
    if (botAI->GetMaster() == from)
        return PLAYERBOT_SECURITY_ALLOW_ALL;

    if (reason)
        *reason = PLAYERBOT_DENY_NOT_YOURS;

    return PLAYERBOT_SECURITY_INVITE;
}

bool PlayerbotSecurity::CheckLevelFor(PlayerbotSecurityLevel level, bool silent, Player* from, bool ignoreGroup)
{
    if (!bot || !from || !from->GetSession())
        return false;

    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel realLevel = LevelFor(from, &reason, ignoreGroup);

    if (realLevel >= level || from == bot)
        return true;

    PlayerbotAI* fromBotAI = GET_PLAYERBOT_AI(from);
    if (silent || (fromBotAI && !fromBotAI->IsRealPlayer()))
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    Player* master = botAI->GetMaster();
    if (master && botAI->IsOpposing(master))
        if (master->GetSession() && !master->GetSession()->HasPermission(rbac::RBAC_PERM_COMMAND_GM))
            return false;

    std::ostringstream out;

    switch (realLevel)
    {
        case PLAYERBOT_SECURITY_DENY_ALL:
            out << "I'm kind of busy now";
            break;
        case PLAYERBOT_SECURITY_TALK:
            switch (reason)
            {
                case PLAYERBOT_DENY_NONE:
                    out << "I'll do it later";
                    break;
                case PLAYERBOT_DENY_LOW_LEVEL:
                    out << "You are too low level: |cffff0000" << uint32(from->GetLevel()) << "|cffffffff/|cff00ff00"
                        << uint32(bot->GetLevel());
                    break;
                case PLAYERBOT_DENY_GEARSCORE:
                {
                    int botGS = int(PlayerbotAI::GetMixedGearScore(bot, true, false));
                    int fromGS = int(PlayerbotAI::GetMixedGearScore(from, true, false));
                    int diff = botGS ? (100 * (botGS - fromGS) / botGS) : 0;
                    int req = 12 * sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL) / from->GetLevel();

                    out << "Your gearscore is too low: |cffff0000" << fromGS << "|cffffffff/|cff00ff00" << botGS
                        << " |cffff0000" << diff << "%|cffffffff/|cff00ff00" << req << "%";
                    break;
                }
                case PLAYERBOT_DENY_NOT_YOURS:
                    out << "I have a master already";
                    break;
                case PLAYERBOT_DENY_IS_BOT:
                    out << "You are a bot";
                    break;
                case PLAYERBOT_DENY_OPPOSING:
                    out << "You are the enemy";
                    break;
                case PLAYERBOT_DENY_DEAD:
                    out << "I'm dead. Will do it later";
                    break;
                case PLAYERBOT_DENY_INVITE:
                    out << "Invite me to your group first";
                    break;
                case PLAYERBOT_DENY_FAR:
                {
                    out << "You must be closer to invite me to your group. I am in ";
                    if (AreaTableEntry const* entry = sAreaTableStore.LookupEntry(bot->GetAreaId()))
                        out << " |cffffffff(|cffff0000" << entry->area_name << "|cffffffff)";
                    break;
                }
                case PLAYERBOT_DENY_FULL_GROUP:
                    out << "I am in a full group. Will do it later";
                    break;
                case PLAYERBOT_DENY_IS_LEADER:
                    out << "I am currently leading a group. I can invite you if you want.";
                    break;
                case PLAYERBOT_DENY_NOT_LEADER:
                    if (Player* leader = botAI->GetGroupMaster())
                        out << "I am in a group with " << leader->GetName() << ". You can ask him for invite.";
                    else
                        out << "I am in a group with someone else. You can ask him for invite.";
                    break;
                case PLAYERBOT_DENY_BG:
                    out << "I am in a queue for BG. Will do it later";
                    break;
                case PLAYERBOT_DENY_LFG:
                    out << "I am in a queue for dungeon. Will do it later";
                    break;
                default:
                    out << "I can't do that";
                    break;
            }
            break;
        case PLAYERBOT_SECURITY_INVITE:
            out << "Invite me to your group first";
            break;
        default:
            out << "I can't do that";
            break;
    }

    std::string const text = out.str();
    ObjectGuid guid = from->GetGUID();
    time_t lastSaid = whispers[guid][text];

    if (!lastSaid || (time(nullptr) - lastSaid) >= sPlayerbotAIConfig->repeatDelay / 1000)
    {
        whispers[guid][text] = time(nullptr);

        // Additional protection against crashes during logout.
        if (bot->IsInWorld() && from->IsInWorld())
            bot->Whisper(text, LANG_UNIVERSAL, from);
    }

    return false;
}
