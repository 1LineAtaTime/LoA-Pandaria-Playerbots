/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "LeaveGroupAction.h"

#include "Event.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "WorldSession.h"

bool LeaveGroupAction::Leave(Player* player)
{
    // #177: this used to refuse to run the moment `player` (the chat command's sender, normally the
    // bot's master) was a real player instead of another bot, so a real player whispering "leave" to
    // their own bot did nothing. AC's equivalent (LeaveGroupAction.cpp:81-92, Leave() with no
    // parameter) has no such gate - only Execute() checks that the sender is the bot's own master
    // (see below), which this fork already enforces the same way.
    bool aiMaster = GET_PLAYERBOT_AI(botAI->GetMaster()) != nullptr;

    // AC LeaveGroupAction.cpp:87-92 (Leave()): the bot says goodbye to its master before it leaves
    // (variants "Goodbye!" / "Bye bye!" / "See you later!" from ai_playerbot_texts). AC passes
    // PLAYERBOT_SECURITY_TALK; this fork's TellMaster has no security-level parameter (PlayerbotAI.h).
    if (botAI->GetMaster())
        botAI->TellMaster(PlayerbotTextMgr::instance().GetBotTextOrDefault("goodbye", "Goodbye!", {}));

    if (bot->GetGroup())
    {
        //bot->GetGroup()->Disband();
        bot->RemoveFromGroup();
    }

    bool randomBot = sRandomPlayerbotMgr->IsRandomBot(bot);
    if (randomBot)
    {
        GET_PLAYERBOT_AI(bot)->SetMaster(nullptr);
    }

    if (!aiMaster)
        botAI->ResetStrategies();

    botAI->Reset();

    return true;
}

bool LeaveGroupAction::Execute(Event event)
{
    // AC LeaveGroupAction.cpp:12-18: only the bot's own master (real player or bot) may make it leave.
    Player* player = event.getOwner();
    if (player && player != botAI->GetMaster())
        return false;

    return Leave(player);
}

bool UninviteAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();

    // #178: the master's own CMSG_GROUP_DISBAND (the 5.4.8 "leave party" opcode) used to be mapped to this
    // action (PlayerbotAI.cpp:211), which made every bot leave at once and Reset() their queued actions, so
    // the "party command" action AC reacts with (SMSG_PARTY_COMMAND_RESULT, PartyCommandAction below, AC
    // LeaveGroupAction.cpp:20-45) never ran - the P8 row read 0 on MoP vs 3-4 on AC. AC has no DISBAND
    // handler at all (Bot/PlayerbotAI.cpp:292-293 maps only CMSG_GROUP_UNINVITE / _GUID), so this opcode is
    // ignored here and PartyCommandAction decides (new master or leave).
    if (p.GetOpcode() == CMSG_GROUP_DISBAND)
        return false;

    // AC LeaveGroupAction.cpp:70-77 (CMSG_GROUP_UNINVITE_GUID): leave when the master uninvites THIS bot. 5.4.8
    // packs the guid in bits (WorldSession::HandleGroupUninviteGuidOpcode, GroupHandler.cpp:382-409); the
    // old parse here read a plain uint32 and never matched.
    if (p.GetOpcode() == CMSG_GROUP_UNINVITE_GUID)
    {
        p.rpos(0);
        p.ResetBitPos();
        ObjectGuid guid;
        p.read_skip<uint8>();
        guid[6] = p.ReadBit();
        guid[4] = p.ReadBit();
        guid[3] = p.ReadBit();
        guid[2] = p.ReadBit();
        guid[0] = p.ReadBit();
        guid[1] = p.ReadBit();
        guid[7] = p.ReadBit();
        guid[5] = p.ReadBit();
        uint32 reasonLen = p.ReadBits(8);
        p.ReadString(reasonLen);
        p.ReadByteSeq(guid[5]);
        p.ReadByteSeq(guid[6]);
        p.ReadByteSeq(guid[1]);
        p.ReadByteSeq(guid[4]);
        p.ReadByteSeq(guid[3]);
        p.ReadByteSeq(guid[2]);
        p.ReadByteSeq(guid[7]);
        p.ReadByteSeq(guid[0]);

        if (bot->GetGUID() == guid)
            return Leave(bot);
    }

    return false;
}

namespace
{
    // .cpp-local reimplementation of AC's PlayerbotAI::FindNewMaster() (AC Bot/PlayerbotAI.cpp:429-488),
    // which this fork does not expose as a public PlayerbotAI.h member (only the inline reassignment
    // pass in AI/PlayerbotAI.cpp has the full logic, including the #53 BG guard and the AV honor
    // exception). This trimmed version only needs to answer "is there an obvious next master right
    // now", because PartyCommandAction's job is to react a tick sooner than that periodic pass - the
    // periodic pass still runs afterwards and is authoritative (it will correct anything this misses,
    // e.g. the AV subgroup/honor exception).
    Player* FindNewMasterForPartyCommand(Player* bot)
    {
        Group* group = bot->GetGroup();
        if (!group)
            return nullptr;

        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsInWorld() || !member->IsInSameRaidWith(bot))
                continue;

            // A fellow bot is not a candidate for this instant pass; AC also prefers a real player
            // (Bot/PlayerbotAI.cpp:429-488 checks !memberBotAI || IsSelfBot(member) first).
            if (GET_PLAYERBOT_AI(member))
                continue;

            // #53 parity: no player master while the bot is inside a battleground match.
            if (bot->InBattleground())
                continue;

            return member;
        }

        return nullptr;
    }
}

// Ported from AC mod-playerbots LeaveGroupAction.cpp:20-45 (PartyCommandAction::Execute, #178/#179).
// Reacts to SMSG_PARTY_COMMAND_RESULT the instant the bot's own master leaves the party: a random bot
// tries to pick a new master immediately (or, in a battleground, accepts having none for now); anyone
// else just leaves. See the class comment (LeaveGroupAction.h) and the FindNewMasterForPartyCommand()
// comment above for what this trims relative to AC's PlayerbotAI::FindNewMaster().
bool PartyCommandAction::Execute(Event event)
{
    WorldPacket& p = event.getPacket();
    p.rpos(0);
    uint32 operation;
    std::string member;
    p >> operation >> member;

    if (operation != PARTY_OP_LEAVE)
        return false;

    Player* master = botAI->GetMaster();
    if (!master || member != master->GetName())
        return false;

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        Player* newMaster = FindNewMasterForPartyCommand(bot);
        if (newMaster || bot->InBattleground())
        {
            botAI->SetMaster(newMaster);
            return false;
        }
    }

    return Leave(bot);
}

// Ported from AC mod-playerbots LeaveGroupAction.cpp:98-102 (LeaveFarAwayAction::Execute).
bool LeaveFarAwayAction::Execute(Event /*event*/)
{
    // Allow the bot to leave the party once it no longer needs to be grouped (see isUseful()).
    return Leave(bot);
}

// Ported from AC mod-playerbots LeaveGroupAction.cpp:104-161 (LeaveFarAwayAction::isUseful), cut
// down to the map/level/distance checks - see the class comment in LeaveGroupAction.h for what was
// dropped and why.
bool LeaveFarAwayAction::isUseful()
{
    // #211: AC LeaveGroupAction.cpp:104-161 1:1. The previous cut-down version had AC's leader check inverted (it left
    // only real-player-led groups). Dormant on both cores: the "group" strategy is commented out in both AiFactory files.
    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* groupLeader = ObjectAccessor::FindPlayer(group->GetLeaderGUID());
    Player* trueMaster = botAI->GetMaster();
    // AC: (bot == groupLeader && !IsSelfBot(bot)); a selfbot is a bot whose master is itself (IsRealPlayer here).
    if (!groupLeader || (bot == groupLeader && !botAI->IsRealPlayer()))
        return false;

    PlayerbotAI* groupLeaderBotAI = GET_PLAYERBOT_AI(groupLeader);
    if (!groupLeaderBotAI)
        return false;

    if (trueMaster && !GET_PLAYERBOT_AI(trueMaster))
        return false;

    // AC's IsAltBot() branch is not ported: this fork has no alt/addclass bot concept (#611).

    if (botAI->GetGrouperType() == GrouperType::SOLO)
        return true;

    uint32 dCount = AI_VALUE(uint32, "death count");

    if (dCount > 9)
        return true;

    if (dCount > 4 && !botAI->HasGameClientMaster())
        return true;

    if (bot->GetGuildId() == groupLeader->GetGuildId())
    {
        if (bot->GetLevel() > groupLeader->GetLevel() + 5)
        {
            if (AI_VALUE(bool, "should get money"))
                return false;
        }
    }

    if (abs(int32(groupLeader->GetLevel() - bot->GetLevel())) > 4)
        return true;

    if (bot->GetMapId() != groupLeader->GetMapId() || bot->GetDistance2d(groupLeader) >= 2 * sPlayerbotAIConfig->rpgDistance)
        return true;

    return false;
}
