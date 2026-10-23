/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/LfgActions.cpp) into the
 * Legends of Azeroth Pandaria fork (#210 S10 G1-G3).
 * Deviations from AC:
 *  - AC bot sessions drain queued packets (PlayerbotHolder::HandleBotPackets), so AC answers the role
 *    check / proposal / teleport by building a CMSG_* packet and calling
 *    bot->GetSession()->QueuePacket(). This fork's bot sessions never drain queued packets
 *    (Manager/PlayerbotMgr.cpp:238-248 is commented out - verified) so these actions call the LFGMgr
 *    entry points the opcode handlers themselves call (UpdateRoleCheck/UpdateProposal/TeleportPlayer)
 *    instead of round-tripping a fake packet (section 2.2 of the doc).
 *  - #142: those LFGMgr calls are NOT made from the action itself. Actions run in Player::Update on the
 *    map-update threads (MapUpdate.Threads 4: mod_playerbots.cpp OnAfterUpdate -> PlayerbotAI::UpdateAI),
 *    LFGMgr has no locking, and its opcode handlers are PROCESS_THREADUNSAFE (Opcodes.cpp:361) - AC says
 *    the same at LfgActions.cpp:154 ("JoinLfg is not threadsafe, so make packet and queue into session").
 *    So each call is handed to the world thread through PlayerbotWorldThreadProcessor (the AC
 *    Script/WorldThr port, whose PlayerbotOperation.h names LFG operations as its use case): the same
 *    world-thread hop AC gets from its session packet queue. QueueLfgCall() below.
 *  - #142: LfgJoinAction / LfgLeaveAction (random-bot self-queue, AC LfgActions.cpp:17-168,222-249,
 *    315-360) are ported at the bottom of this file. Like the three reactions below they call the
 *    LFGMgr entry point the opcode handler calls (JoinLfg / LeaveLfg) instead of queuing a CMSG_*.
 *    MoP differences: LFGMgr::JoinLfg accepts one random dungeon OR several specific dungeons of one
 *    category (LFG_JOIN_DUNGEON_INVALID / MIXED_RAID_DUNGEON otherwise), so the AC "add every queued
 *    dungeon in level range" loop also enforces that; LFG_TYPE_RAID (AC accepts it) is skipped because
 *    JoinLfg logs an error for it on 5.4.8; the queue state is per queue id (GetActiveState).
 *  - The three packet-driven reactions a *grouped, invited* bot needs (role check answer, proposal
 *    accept, teleport copy) run off the always-on "default" strategy, exactly as in AC.
 *  - GetLfgRoles (AC LfgActions.cpp GetRoles, :21-83) keyed off AiFactory::GetPlayerSpecTab(), which
 *    on AC returns a talent-TAB index (0/1/2). This core's GetPlayerSpecTab() returns the real
 *    Specializations enum (Factory/AiFactory.cpp:76-78: `return bot->GetSpecialization();`), so the
 *    switch below matches on spec IDs instead of tab numbers - same AC intent (one tank/heal spec per
 *    class, everything else DPS). Monk (MoP-only) extends the pattern: brewmaster tank, mistweaver
 *    heal, windwalker dps - the spec section 3.5 gap table "Role pick" row.
 *  - SMSG_LFG_PROPOSAL_UPDATE is bit-packed on 5.4.8 (MoPcore Handlers/LFGHandler.cpp:1111-1201,
 *    WorldSession::SendLfgUpdateProposal, verified). The exact ReadBit()/ReadByteSeq() order below
 *    mirrors that write order call-for-call (bit position and byte position both advance in lockstep
 *    between a matching Write/Read pair, regardless of how the two interleave - see the doc for the
 *    full derivation). Only proposal.id is needed, same as AC only reads dungeonId/state/id from its
 *    own (non-bit-packed) WotLK wire format.
 *  - SMSG_LFG_ROLE_CHECK_UPDATE needs no parse at all: like AC, the action never reads the packet body,
 *    it just re-answers with the bot's own role every time the packet arrives.
 *  - CMSG_LFG_TELEPORT (masterIncoming) is a single bit on this core (MoPcore LFGHandler.cpp:229-236,
 *    HandleLfgTeleportOpcode: `bool out = recvData.ReadBit();`), versus AC's bare bool field.
 */

#include "LfgActions.h"

#include "Event.h"
#include "Group.h"
#include "LFGMgr.h"
#include "LfgAnyLevelRequests.h"  // #702: any-level LFG bot fill (off by default)
#include "ObjectAccessor.h"
#include "ObjectGuid.h"
#include "PlayerbotOperation.h"
#include "PlayerbotSpec.h"
#include "PlayerbotWorldThreadProcessor.h"
#include "Playerbots.h"
#include "PlayerbotAI.h"
#include "RandomPlayerbotMgr.h"
#include "WorldPacket.h"

#include <functional>
#include <memory>

using namespace lfg;

namespace
{
// #142: the world-thread stand-in for AC's `bot->GetSession()->QueuePacket(CMSG_LFG_*)` (see the file
// header). Holds only the bot guid and by-value arguments; the bot is re-resolved on the world thread
// and the call is skipped when it has logged out meanwhile (PlayerbotOperation contract).
class LfgWorldThreadOperation : public PlayerbotOperation
{
public:
    LfgWorldThreadOperation(ObjectGuid botGuid, char const* name, std::function<void(Player*)> call)
        : m_botGuid(botGuid), m_name(name), m_call(std::move(call))
    {
    }

    bool Execute() override
    {
        Player* bot = ObjectAccessor::FindPlayer(m_botGuid);
        if (!bot)
            return false;

        m_call(bot);
        return true;
    }

    ObjectGuid GetBotGuid() const override { return m_botGuid; }
    uint32 GetPriority() const override { return 50; }  // player-facing, as GroupSetLeaderOperation
    std::string GetName() const override { return m_name; }
    bool IsValid() const override { return ObjectAccessor::FindPlayer(m_botGuid) != nullptr; }

private:
    ObjectGuid m_botGuid;
    std::string m_name;
    std::function<void(Player*)> m_call;
};

void QueueLfgCall(Player* bot, char const* name, std::function<void(Player*)> call)
{
    PlayerbotWorldThreadProcessor::instance().QueueOperation(
        std::make_unique<LfgWorldThreadOperation>(bot->GetGUID(), name, std::move(call)));
}

uint32 GetLfgRoles(Player* bot)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        if (PlayerBotSpec::IsTank(bot))
            return PLAYER_ROLE_TANK;
        if (PlayerBotSpec::IsHeal(bot))
            return PLAYER_ROLE_HEALER;
        return PLAYER_ROLE_DAMAGE;
    }

    Specializations const spec = bot->GetSpecialization();
    switch (bot->GetClass())
    {
        case CLASS_DRUID:
            if (spec == SPEC_DRUID_RESTORATION)
                return PLAYER_ROLE_HEALER;
            if (spec == SPEC_DRUID_GUARDIAN)
                return PLAYER_ROLE_TANK;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_PALADIN:
            if (spec == SPEC_PALADIN_PROTECTION)
                return PLAYER_ROLE_TANK;
            if (spec == SPEC_PALADIN_HOLY)
                return PLAYER_ROLE_HEALER;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_PRIEST:
            if (spec == SPEC_PRIEST_DISCIPLINE || spec == SPEC_PRIEST_HOLY)
                return PLAYER_ROLE_HEALER;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_SHAMAN:
            if (spec == SPEC_SHAMAN_RESTORATION)
                return PLAYER_ROLE_HEALER;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_WARRIOR:
            if (spec == SPEC_WARRIOR_PROTECTION)
                return PLAYER_ROLE_TANK;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_DEATH_KNIGHT:
            if (spec == SPEC_DEATH_KNIGHT_BLOOD)
                return PLAYER_ROLE_TANK;
            return PLAYER_ROLE_DAMAGE;
        case CLASS_MONK:
            if (spec == SPEC_MONK_BREWMASTER)
                return PLAYER_ROLE_TANK;
            if (spec == SPEC_MONK_MISTWEAVER)
                return PLAYER_ROLE_HEALER;
            return PLAYER_ROLE_DAMAGE;
        default:
            return PLAYER_ROLE_DAMAGE;
    }
}
}  // namespace

bool LfgRoleCheckAction::Execute(Event /*event*/)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    uint32 const roles = GetLfgRoles(bot);

    // AC LfgActions.cpp:173-194 queues CMSG_LFG_SET_ROLES on every role-check update; this fork calls
    // the same LFGMgr entry point the opcode handler calls (MoPcore Handlers/LFGHandler.cpp:201-217,
    // HandleLfgSetRolesOpcode -> sLFGMgr->UpdateRoleCheck), on the world thread (#142, file header).
    QueueLfgCall(bot, "LfgSetRoles", [roles](Player* p)
    {
        // HandleLfgSetRolesOpcode reads the group when the packet is handled.
        if (Group* g = p->GetGroup())
            sLFGMgr->UpdateRoleCheck(g->GetGUID(), p->GetGUID(), roles);
    });

    TC_LOG_INFO("playerbots", "Bot %s %s:%u <%s>: LFG roles checked", bot->GetGUID().ToString().c_str(),
                bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str());

    return true;
}

bool LfgAcceptAction::Execute(Event event)
{
    // "lfg proposal" (LfgProposalValue, strategy/value/LfgValues.h) mirrors AC's manual value: a
    // proposal id stashed from an earlier update. Like AC (see LfgActions.cpp:196-208), nothing on
    // this fork ever sets it non-zero yet (dormant, kept for parity/future callers, e.g. a harness
    // verb); the packet branch below is the path that actually fires.
    uint32 id = AI_VALUE(uint32, "lfg proposal");
    if (id)
    {
        bool const accept = !(bot->IsInCombat() || bot->isDead());
        // #142: world thread (file header); AC queues CMSG_LFG_PROPOSAL_RESULT (:200-223).
        QueueLfgCall(bot, "LfgProposalResult",
                     [id, accept](Player* p) { sLFGMgr->UpdateProposal(id, p->GetGUID(), accept); });

        if (accept)
        {
            context->GetValue<uint32>("lfg proposal")->Set(0);
            bot->ClearUnitState(UNIT_STATE_ALL_STATE);
            // #142 (AC LfgActions.cpp:216-220): an ungrouped random bot is refreshed on accept.
            if (sRandomPlayerbotMgr->IsRandomBot(bot) && !bot->GetGroup())
            {
                sRandomPlayerbotMgr->Refresh(bot);
                botAI->ResetStrategies();
            }
            botAI->Reset();
        }

        return true;
    }

    WorldPacket& incoming = event.getPacket();
    if (incoming.empty())
        return false;

    WorldPacket p(incoming);
    p.rpos(0);

    // MoPcore Handlers/LFGHandler.cpp:1111-1201 (SendLfgUpdateProposal) bit-packs two guids (guid1 =
    // the original group or the player, guid2 = the dungeon entry tagged with 0x1F45<<48) around the
    // fixed fields. The order below mirrors that write order call-for-call - see the file header.
    ObjectGuid guid1;
    ObjectGuid guid2;

    guid2[6] = p.ReadBit();
    guid2[0] = p.ReadBit();
    guid1[1] = p.ReadBit();
    guid1[7] = p.ReadBit();
    guid1[5] = p.ReadBit();
    guid2[5] = p.ReadBit();
    guid1[4] = p.ReadBit();
    p.ReadBit();  // silent
    guid2[2] = p.ReadBit();
    guid1[6] = p.ReadBit();
    guid2[3] = p.ReadBit();
    guid2[7] = p.ReadBit();
    guid1[3] = p.ReadBit();

    uint32 const playerCount = p.ReadBits(21);
    for (uint32 i = 0; i < playerCount; ++i)
    {
        p.ReadBit();  // is group in dungeon
        p.ReadBit();  // is this player
        p.ReadBit();  // answered
        p.ReadBit();  // accepted
        p.ReadBit();  // same group as the receiving player
    }

    guid1[2] = p.ReadBit();
    guid2[4] = p.ReadBit();
    p.ReadBit();  // unk
    guid1[0] = p.ReadBit();
    guid2[1] = p.ReadBit();

    p.ReadByteSeq(guid2[1]);
    p.ReadByteSeq(guid1[4]);
    p.ReadByteSeq(guid2[4]);
    p.ReadByteSeq(guid1[7]);
    p.ReadByteSeq(guid1[2]);
    p.ReadByteSeq(guid1[0]);

    p.read_skip<uint32>();  // dungeonEntry
    p.read_skip<uint8>();   // state
    p.read_skip<uint32>();  // clientQueueId (ClientQueueId)

    p.ReadByteSeq(guid2[6]);

    uint32 const proposalId = p.read<uint32>();
    if (!proposalId)
        return false;

    bool const accept = !(bot->IsInCombat() || bot->isDead());
    // #142: world thread (file header); AC queues CMSG_LFG_PROPOSAL_RESULT (:226-256).
    QueueLfgCall(bot, "LfgProposalResult",
                 [proposalId, accept](Player* p) { sLFGMgr->UpdateProposal(proposalId, p->GetGUID(), accept); });

    if (accept)
    {
        context->GetValue<uint32>("lfg proposal")->Set(0);
        bot->ClearUnitState(UNIT_STATE_ALL_STATE);
        // #142 (AC LfgActions.cpp:262-266): an ungrouped random bot is refreshed on accept.
        if (sRandomPlayerbotMgr->IsRandomBot(bot) && !bot->GetGroup())
        {
            sRandomPlayerbotMgr->Refresh(bot);
            botAI->ResetStrategies();
        }
        botAI->Reset();
    }

    return true;
}

bool LfgTeleportAction::Execute(Event event)
{
    WorldPacket& incoming = event.getPacket();
    if (incoming.empty())
        return false;

    WorldPacket p(incoming);
    p.rpos(0);

    // MoPcore Handlers/LFGHandler.cpp:229-236 (HandleLfgTeleportOpcode): a single bit, versus AC's
    // bare bool field (LfgActions.cpp:294-313).
    bool const out = p.ReadBit();

    bot->ClearUnitState(UNIT_STATE_ALL_STATE);

    // Same entry point the opcode handler calls (LFGHandler.cpp:235: sLFGMgr->TeleportPlayer(player,
    // out, true)) - see the file header for why this fork calls it (on the world thread, #142) instead
    // of queuing a fake CMSG_LFG_TELEPORT like AC does.
    QueueLfgCall(bot, "LfgTeleport", [out](Player* p) { sLFGMgr->TeleportPlayer(p, out, true); });

    return true;
}

// #142 (AC LfgActions.cpp:85-168 JoinLFG): queue the bot for the dungeons real players are queued for.
bool LfgJoinAction::Execute(Event /*event*/) { return JoinLFG(); }

bool LfgJoinAction::JoinLFG()
{
    // check if already in lfg (AC :87-90; 5.4.8 tracks one state per queue id, GetActiveState is the active one)
    if (sLFGMgr->GetActiveState(bot->GetGUID()) != LFG_STATE_NONE)
        return false;

    // AC :92-102 has a commented-out random/heroic/rbotAId gear-score pick here; it stays dropped.

    // AC :104 reads LfgDungeons[team] with operator[]; find() here, because this runs on a map thread and
    // operator[] would insert the key before the first CheckLfgQueue (world thread) has created it.
    auto const teamDungeons = sRandomPlayerbotMgr->LfgDungeons.find(bot->GetTeamId());
    if (teamDungeons == sRandomPlayerbotMgr->LfgDungeons.end())
        return false;

    std::vector<uint32> const dungeons = teamDungeons->second;
    if (dungeons.empty())
        return false;

    LfgDungeonSet list;
    LfgCategory category = LFG_CATEGORY_NONE;
    bool haveRandom = false;

    // #702 (beyond AC, off unless AiPlayerbot.LfgAnyLevel.Enable = 1): dungeons a real player queued outside their
    // level range take bots within AiPlayerbot.LfgAnyLevel.LevelWindow levels of that player (LfgAnyLevelRequests.h).
    bool const anyLevel = LfgAnyLevel::Enabled();
    std::vector<LfgAnyLevel::Request> const anyLevelRequests =
        anyLevel ? LfgAnyLevel::Snapshot(bot->GetTeamId()) : std::vector<LfgAnyLevel::Request>();
    // #830: AiPlayerbot.LfgAnyLevel.ItemLevelWindow compares this with the requester's (unused while that key is 0)
    float const botItemLevel = anyLevel ? bot->GetAverageItemLevel() : 0.0f;

    for (uint32 const dungeonId : dungeons)
    {
        LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(dungeonId);
        // AC :111-114 also lets LFG_TYPE_RAID through; 5.4.8 JoinLfg errors on it, so it is skipped.
        if (!dungeon || (dungeon->type != LFG_TYPE_RANDOM && dungeon->type != LFG_TYPE_DUNGEON &&
                         dungeon->type != LFG_TYPE_HEROIC))
            continue;

        uint32 const botLevel = bot->GetLevel();

        // AC :118-122: in level range, and a specific dungeon only within 10 levels of its minimum.
        bool const nativeOk = !((dungeon->minlevel && (botLevel < dungeon->minlevel || botLevel > dungeon->maxlevel)) ||
            (botLevel > uint32(dungeon->minlevel) + 10 && dungeon->type == LFG_TYPE_DUNGEON));
        if (anyLevel ? !LfgAnyLevel::AcceptBot(anyLevelRequests, dungeon->id, botLevel, nativeOk, botItemLevel) : !nativeOk)
            continue;

        // MoP JoinLfg: one random dungeon alone, or specific dungeons of a single category.
        if (!list.empty() && (dungeon->type == LFG_TYPE_RANDOM || haveRandom || dungeon->category != category))
            continue;

        category = dungeon->category;
        if (dungeon->type == LFG_TYPE_RANDOM)
            haveRandom = true;
        list.insert(dungeon->id);
    }

    if (list.empty())
        return false;

    bool const many = list.size() > 1;
    LFGDungeonData const* first = sLFGMgr->GetLFGDungeon(*list.begin());

    // check role for console msg (AC :133-141)
    std::string _roles = "multiple roles";
    uint32 const roleMask = GetLfgRoles(bot);
    if (roleMask & PLAYER_ROLE_TANK)
        _roles = "TANK";

    if (roleMask & PLAYER_ROLE_HEALER)
        _roles = "HEAL";

    if (roleMask & PLAYER_ROLE_DAMAGE)
        _roles = "DPS";

    TC_LOG_INFO("playerbots", "Bot %s %s:%u <%s>: queues LFG, Dungeon as %s (%s)", bot->GetGUID().ToString().c_str(),
                bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), _roles.c_str(),
                many ? "several dungeons" : (first ? first->name.c_str() : "?"));

    // AC :151-168 puts the gear score in the browser comment, then queues a CMSG_LFG_JOIN into the bot's
    // session ("JoinLfg is not threadsafe"); this fork hands LFGMgr::JoinLfg (what HandleLfgJoinOpcode ends
    // in) to the world thread (#142, file header).
    std::string const gs = std::to_string(PlayerbotAI::GetMixedGearScore(bot, false, false));
    QueueLfgCall(bot, "LfgJoin", [roleMask, list, gs](Player* p)
    {
        // Same guard as :265 at the moment of the real call (the op ran one world tick later).
        if (sLFGMgr->GetActiveState(p->GetGUID()) != LFG_STATE_NONE)
            return;

        LfgDungeonSet dungeonSet = list;  // JoinLfg takes the set by non-const reference
        sLFGMgr->JoinLfg(p, LfgRoles(roleMask), dungeonSet, gs);

        // #142 diagnostic (not in AC): JoinLfg's own result line is TC_LOG_DEBUG("lfg") and the test
        // servers log "lfg" at root level 5 (errors only), so report the outcome here. QUEUED (solo) or
        // ROLECHECK (bot leads a group) = joined; NONE = JoinLfg rejected it (SendLfgJoinResult code).
        LfgState const after = sLFGMgr->GetActiveState(p->GetGUID());
        TC_LOG_INFO("playerbots", "Bot %s <%s>: LFG join result: %s (state %u, %u dungeon(s), roles 0x%X)",
                    p->GetGUID().ToString().c_str(), p->GetName().c_str(),
                    after == LFG_STATE_NONE ? "REJECTED" : "JOINED", uint32(after), uint32(list.size()), roleMask);
    });

    return true;
}

// AC LfgActions.cpp:315-360.
bool LfgJoinAction::isUseful()
{
    if (!sPlayerbotAIConfig->randomBotJoinLfg)
        return false;

    if (bot->GetLevel() < 15)
        return false;

    // Don't use for selfbots (a real player is at the keyboard); AC IsSelfBot = master is the bot itself.
    if (botAI->GetMaster() == bot)
        return false;

    if (bot->GetGroup() && bot->GetGroup()->GetLeaderGUID() != bot->GetGUID())
        return false;

    if (bot->IsBeingTeleported())
        return false;

    if (bot->InBattleground())
        return false;

    if (bot->InBattlegroundQueue())
        return false;

    if (!bot->IsAlive())
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    Map* map = bot->GetMap();
    if (map && map->Instanceable())
        return false;

    LfgState state = sLFGMgr->GetActiveState(bot->GetGUID());
    if (state != LFG_STATE_NONE)
        return false;

    return true;
}

// AC LfgActions.cpp:222-249.
bool LfgLeaveAction::Execute(Event /*event*/)
{
    // Don't leave if already invited / in dungeon
    if (sLFGMgr->GetActiveState(bot->GetGUID()) > LFG_STATE_QUEUED)
        return false;

    // Don't drop a queue we deliberately joined (AC :235-243): the "seldom" tick would otherwise pull
    // random bots straight back out before LFGQueue can form a group from a role-complete pool.
    if (sPlayerbotAIConfig->randomBotJoinLfg && sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    // AC queues CMSG_LFG_LEAVE; HandleLfgLeaveOpcode ends in LeaveLfg(group-or-player guid, queue id) and
    // only the leader may leave for a group (LFGHandler.cpp:136-143).
    Group* group = bot->GetGroup();
    if (group && group->GetLeaderGUID() != bot->GetGUID())
        return false;

    ObjectGuid const gguid = group ? group->GetGUID() : bot->GetGUID();
    if (!sLFGMgr->GetActiveQueueId(gguid))
        return false;

    // #142: world thread (file header); the handler's group / leader / queue id are read when it runs.
    QueueLfgCall(bot, "LfgLeave", [](Player* p)
    {
        Group* g = p->GetGroup();
        if (g && g->GetLeaderGUID() != p->GetGUID())
            return;

        ObjectGuid const leaveGuid = g ? g->GetGUID() : p->GetGUID();
        if (uint32 const queueId = sLFGMgr->GetActiveQueueId(leaveGuid))
            sLFGMgr->LeaveLfg(leaveGuid, queueId);
    });
    return true;
}

bool LfgLeaveAction::isUseful() { return true; }