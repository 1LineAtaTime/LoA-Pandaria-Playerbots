/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/BattleGroundJoinAction.cpp)
 * into the Legends of Azeroth Pandaria fork.
 *
 * The one structural difference from upstream: AC queues by hand-building a 3.3.5
 * CMSG_BATTLEMASTER_JOIN and pushing it through the session. On 5.4.8 that packet is bit-packed
 * and carries the battleground type inside the *counter of a packed guid*
 * (see WorldSession::HandleBattlemasterJoinOpcode), so building it from server code would be both
 * fragile and pointless. These actions instead run the same server-side sequence the handler runs
 * once it has finished parsing - the same approach the quest port in #32 takes.
 */

#include "BattleGroundJoinAction.h"
#include "BotScenario.h"

// DatabaseEnv.h must precede BattlegroundQueue.h: the latter pulls in RatedPvp.h, which uses
// CharacterDatabaseTransaction without declaring it.
#include "DatabaseEnv.h"

#include "ArenaTeam.h"
#include "Battleground.h"
#include "BattleGroundTactics.h"
#include "BattlegroundAnyLevel.h"  // #702 P4 (#831): core Battleground.AnyLevel bracket fallback (off by default)
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BgRatedBg.h"            // #897: rated battleground bot groups (AiPlayerbot.RandomBotAutoJoinBGRatedBGCount, off by default)
#include "BgAnyLevelRequests.h"     // #702 P4 (#831): AiPlayerbot.BgAnyLevel level-window fill (off by default)
#include "ArenaAnyLevelRequests.h"  // #702 P5 (#832): AiPlayerbot.ArenaAnyLevel skirmish census + level window (off by default)
#include "Config.h"
#include "DBCStores.h"
#include "Group.h"
#include "GroupMgr.h"
#include "LFGMgr.h"
#include "MotionMaster.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "World.h"
#include "WorldSession.h"

// #632 harness guard (identical on both trees): a bot held in a BotScenario bg/arena roster never leaves
// its arena queue through the waited-too-long path (scenario stages wait up to 600 s for the match).
namespace BotScenario { bool BotScenarioHoldsArenaRoster(Player* bot); }

#include <algorithm>

bool IsSupportedBgQueue(BattlegroundQueueTypeId queueTypeId)
{
    // Upstream has no filter here at all: it walks every queue type and lets canJoinBg() decide.
    // That works on 3.3.5 because every queue in that enum is either a battleground or an arena,
    // and BGArenaType() identifies the arenas. It does not work here - 5.4.8 adds the rated
    // battleground and solo queues, for which BGArenaType() also returns 0, so bots queued for
    // both. So this is a deny-list of exactly those extra queue types, and everything upstream
    // would consider is still allowed through - the 2v2/3v3/5v5 arena queues included since #437
    // (shouldJoinBg's arena branch, AC BattleGroundJoinAction.cpp:239-288).
    //
    // Random Battleground in particular MUST stay allowed. It is a first-class case upstream
    // (BATTLEGROUND_RB in BGJoinAction/BGStatusAction) and it is the queue real players actually
    // use; excluding it means their queue can never fill, because no bot will ever join it.
    switch (queueTypeId)
    {
        case BATTLEGROUND_QUEUE_NONE:
        case BATTLEGORUND_QUEUE_RATED_BG:  // enum spelling is upstream's
        case BATTLEGROUND_QUEUE_SOLO:
            return false;
        default:
            return true;
    }
}

// #693, AC mod-playerbots#2850. 5.4.8 has no Player::GetCurrentBattlegroundQueueSlot(): the slot of the match the
// player is in is the one whose queue type is that battleground's (a Random Battleground keeps BATTLEGROUND_RB as its
// type, BattlegroundMgr.cpp:954, so it maps back to the RB queue, and a rated arena to its 2v2/3v3/5v5 queue).
bool InMatchOfBgQueue(Player* player, BattlegroundQueueTypeId queueTypeId)
{
    if (!player->InBattleground())
        return false;

    Battleground* bg = player->GetBattleground();
    return bg && BattlegroundMgr::BGQueueTypeId(bg->GetTypeID(), bg->GetArenaType()) == queueTypeId;
}

// #693, AC getQueueBracket. AC reads GroupQueueInfo::BracketId; 5.4.8's GroupQueueInfo has no bracket field, so the
// waiting case finds the m_QueuedGroups row holding the player's group, the same search BattlegroundQueue::RemovePlayer
// does (src/server/game/Battlegrounds/BattlegroundQueue.cpp:419-440).
PvPDifficultyEntry const* GetAssignedBgBracket(Player* player, BattlegroundQueueTypeId queueTypeId, uint32 mapId)
{
    if (InMatchOfBgQueue(player, queueTypeId))
        return sDBCManager.GetBattlegroundBracketById(mapId, player->GetBattleground()->GetBracketId());

    BattlegroundQueue& queue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
    auto const itr = queue.m_QueuedPlayers.find(player->GetGUID().GetRawValue());
    if (itr == queue.m_QueuedPlayers.end() || !itr->second.GroupInfo)
        return nullptr;

    GroupQueueInfo const* group = itr->second.GroupInfo;
    for (uint32 bracket = 0; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
        for (uint32 type = 0; type < BG_QUEUE_GROUP_TYPES_COUNT; ++type)
            for (GroupQueueInfo const* queued : queue.m_QueuedGroups[bracket][type])
                if (queued == group)
                    return sDBCManager.GetBattlegroundBracketById(mapId, BattlegroundBracketId(bracket));

    return nullptr;
}

// ---------------------------------------------------------------------------------------------
// Rated-arena census (#437) - the arena half of AC RandomPlayerbotMgr::CheckBgQueue.
// ---------------------------------------------------------------------------------------------

namespace
{
// #437 hardening (the w24-hang class, RandomPlayerbotMgr.h BattlegroundDataLock): bots read and book into this map from
// map-update threads (shouldJoinBg, BGStatusAction) while the world thread resets and logs it, so the map's shape must
// never change after construction. Every arena (queue, bracket) key is created once, by the thread-safe function-local
// static initialisation, before the first reader; ResetCensus only assigns to existing entries and Census never inserts.
using RatedArenaMap = std::map<uint32, std::map<uint32, RatedArenaInfo>>;
RatedArenaMap& RatedArenaData()
{
    static RatedArenaMap data = []
    {
        RatedArenaMap m;
        for (uint32 queueType : { BATTLEGROUND_QUEUE_2v2, BATTLEGROUND_QUEUE_3v3, BATTLEGROUND_QUEUE_5v5 })
            for (uint32 bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
                m[queueType][bracket] = RatedArenaInfo();
        return m;
    }();
    return data;
}

char const* ArenaTypeName(uint8 arenaType)
{
    return arenaType == ARENA_TYPE_2v2 ? "2v2" : arenaType == ARENA_TYPE_3v3 ? "3v3" : "5v5";  // AC :1208-1210
}
}  // namespace

RatedArenaInfo& PlayerbotArena::Census(uint32 queueTypeId, uint32 bracketId)
{
    RatedArenaMap& data = RatedArenaData();
    auto const q = data.find(queueTypeId);
    if (q != data.end())
    {
        auto const b = q->second.find(bracketId);
        if (b != q->second.end())
            return b->second;
    }

    // Not an arena (queue, bracket): a per-thread scratch entry, so nothing is inserted into the shared map.
    thread_local RatedArenaInfo scratch;
    scratch = RatedArenaInfo();
    return scratch;
}

void PlayerbotArena::ResetCensus()
{
    // Every arena (queue, bracket) entry already exists (RatedArenaData); this only clears them, as the
    // BattlegroundData reset does, so bots reading and booking into it from shouldJoinBg never insert into the map.
    for (auto& byQueue : RatedArenaData())
        for (auto& byBracket : byQueue.second)
            byBracket.second = RatedArenaInfo();
}

void PlayerbotArena::CountQueued(Player* player, bool isBot, BattlegroundQueueTypeId queueTypeId)
{
    BattlegroundTypeId const bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bgTemplate)
        return;

    // #693, AC mod-playerbots#2850 (RandomPlayerbotMgr.cpp:973, :1063): the assigned bracket, not the current level.
    PvPDifficultyEntry const* pvpDiff = GetAssignedBgBracket(player, queueTypeId, bgTemplate->GetMapId());
    if (!pvpDiff)
        return;

    RatedArenaInfo& info = Census(queueTypeId, pvpDiff->GetBracketId());
    info.minLevel = pvpDiff->MinLevel;  // AC :965-966 / :1054-1055
    info.maxLevel = pvpDiff->MaxLevel;

    // AC :972-982 / :1060-1069. AC also asks BattlegroundQueue::IsPlayerInvitedToRatedArena(), which this core
    // does not have; the GroupQueueInfo stays in the queue until the player enters, so IsRated covers the
    // invited state, and the in-arena check covers the rest.
    bool isRated = false;
    GroupQueueInfo ginfo;
    bool const inQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId)
                             .GetPlayerGroupInfoData(player->GetGUID().GetRawValue(), &ginfo);
    if (inQueue)
        isRated = ginfo.IsRated;

    Battleground* bg = player->GetBattleground();
    if (player->InArena() && bg && bg->IsRated())
        isRated = true;

    // AC counts everything else as skirmish (:986-987, :1073-1074); 5.4.8 has no skirmish arenas of its own. #702 P5
    // (#832): the core's Arena.AnyLevel files a below-max-level arena join as an unrated skirmish; with
    // AiPlayerbot.ArenaAnyLevel.Enable = 1 those slots feed the skirmish census (AC RandomPlayerbotMgr.cpp:1005-1007,
    // :1037-1039, :1092-1094, :1112-1140; ArenaAnyLevelRequests.h). Off: not counted, as before.
    if (!isRated)
    {
        if (ArenaAnyLevelBots::CensusOn())
        {
            bool const waiting = !(inQueue && ginfo.IsInvitedToBGInstanceGUID != 0) && !player->InBattleground();
            uint32 const instanceId = (player->InArena() && bg && InMatchOfBgQueue(player, queueTypeId)) ? bg->GetInstanceID() : 0;
            ArenaAnyLevelBots::CountSkirmish(BattlegroundMgr::BGArenaType(queueTypeId), uint32(pvpDiff->GetBracketId()), isBot,
                waiting, instanceId, uint8(player->GetLevel()));
        }
        return;
    }

    if (isBot)
    {
        info.ratedArenaBotCount++;  // AC :1071-1072

        // AC :1084-1126: a bot inside a rated arena marks that instance as running.
        if (player->InArena() && bg && InMatchOfBgQueue(player, queueTypeId))  // #693, AC #2850 :1098
        {
            uint32 const instanceId = bg->GetInstanceID();
            if (std::find(info.ratedArenaInstances.begin(), info.ratedArenaInstances.end(), instanceId) ==
                info.ratedArenaInstances.end())
                info.ratedArenaInstances.push_back(instanceId);

            info.ratedArenaInstanceCount = info.ratedArenaInstances.size();
        }
        return;  // only a real player opens a bracket - AC's bot pass never sets the active flag
    }

    info.ratedArenaPlayerCount++;  // AC :984-985

    // AC :1012-1017: a real player still waiting (not invited, not inside) opens the bracket for bots.
    bool const invited = inQueue && ginfo.IsInvitedToBGInstanceGUID != 0;
    if (!invited && !player->InBattleground())
        info.activeRatedArenaQueue = 1;
}

void PlayerbotArena::SeedAutoJoin()
{
    // AC PlayerbotAIConfig.cpp:386, 400-405 (defaults 14 / 0 / 0 / 0), read here instead of in PlayerbotAIConfig.h
    // (the same pattern as the AV/SA seeds in RandomPlayerbotMgr.cpp). Bracket 14 is level 80-84 on both cores;
    // the level-90 bracket on a 90-cap realm is 16.
    uint32 const bracket = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinArenaBracket", 14);
    if (bracket >= MAX_BATTLEGROUND_BRACKETS)
        return;

    // AC :1152-1158 updateRatedArenaInstanceCount
    auto seed = [bracket](BattlegroundQueueTypeId queueTypeId, uint32 minCount)
    {
        RatedArenaInfo& info = Census(queueTypeId, bracket);
        if (info.activeRatedArenaQueue == 0 && info.ratedArenaInstanceCount < minCount &&
            info.ratedArenaInstances.size() < minCount)
            info.activeRatedArenaQueue = 1;
    };

    seed(BATTLEGROUND_QUEUE_2v2, sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGRatedArena2v2Count", 0));
    seed(BATTLEGROUND_QUEUE_3v3, sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGRatedArena3v3Count", 0));
    seed(BATTLEGROUND_QUEUE_5v5, sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGRatedArena5v5Count", 0));
}

void PlayerbotArena::LogCensus()
{
    for (auto const& byQueue : RatedArenaData())
    {
        uint8 const arenaType = BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId(byQueue.first));
        for (auto const& byBracket : byQueue.second)
        {
            RatedArenaInfo const& i = byBracket.second;
            if (!i.activeRatedArenaQueue && !i.ratedArenaBotCount && !i.ratedArenaPlayerCount &&
                !i.ratedArenaInstanceCount)
                continue;

            TC_LOG_DEBUG("playerbots",
                         "[ARENAQ] ARENA:%s %u-%u (bracket %u): active=%u Player (Rated:%u) Bots (Rated:%u) "
                         "Total (Rated:%u), Instances (Rated:%u)",
                         ArenaTypeName(arenaType), i.minLevel, i.maxLevel, byBracket.first, i.activeRatedArenaQueue,
                         i.ratedArenaPlayerCount, i.ratedArenaBotCount, i.ratedArenaPlayerCount + i.ratedArenaBotCount,
                         i.ratedArenaInstanceCount);
        }
    }
}

bool BGJoinAction::isUseful()
{
    // .botscenario bg stage isolation (harness hook, no-op unless a bg stage runs).
    if (BotScenario::BgJoinBlocked(bot))
        return false;

    if (!sPlayerbotAIConfig->randomBotJoinBG)
        return false;

    // can't queue while already in a battleground or already queued
    if (bot->InBattleground() || bot->InBattlegroundQueue())
        return false;

    // do not try right after login - the bot is still being set up
    if ((time(nullptr) - bot->GetInGameTime()) < 120)
        return false;

    if (bot->GetLevel() < 10)
        return false;

    // leave bots that belong to a real player alone
    if (botAI->GetMaster() && !GET_PLAYERBOT_AI(botAI->GetMaster()))
        return false;

    // in a group only the leader queues
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    if (bot->IsInCombat() || !bot->IsAlive())
        return false;

    if (bot->InArena())
        return false;

    if (!bot->HasFreeBattlegroundQueueId())
        return false;

    bgList.clear();
    ratedList.clear();  // AC :365

    // #897 (beyond AC, off unless AiPlayerbot.RandomBotAutoJoinBGRatedBGCount > 0): a level-max random bot may lead a
    // rated battleground raid. The rated queue is on the deny list (IsSupportedBgQueue), so it is offered here, outside
    // the queue walk; the per-faction quota is booked by CanLead, as shouldJoinBg books the rated-arena quota.
    if (PlayerbotRatedBg::CanLead(bot))
    {
        bgList.push_back(BATTLEGORUND_QUEUE_RATED_BG);
        ratedList.push_back(BATTLEGORUND_QUEUE_RATED_BG);
    }

    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
    {
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
        {
            BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(queueType);
            BattlegroundBracketId bracketId = BattlegroundBracketId(bracket);

            if (!IsSupportedBgQueue(queueTypeId))
                continue;

            // Only ever noisy for a (queue, bracket) somebody is actually waiting in, which is at
            // most a handful at a time - see the [BGQ] census. Without this there is no way to tell
            // "no bot was eligible" from "no bot wanted to".
            BattlegroundInfo const probe = sRandomPlayerbotMgr->GetBattlegroundInfo(queueTypeId, bracketId);  // w24 hang: no operator[] from a map thread
            bool const liveQueue = probe.activeBgQueue || probe.bgInstanceCount;

            if (!canJoinBg(queueTypeId, bracketId))
            {
                if (liveQueue)
                {
                    BattlegroundTypeId const probeType = BattlegroundMgr::BGTemplateId(queueTypeId);
                    Battleground* const probeBg = sBattlegroundMgr->GetBattlegroundTemplate(probeType);
                    PvPDifficultyEntry const* probeDiff =
                        probeBg ? sDBCManager.GetBattlegroundBracketByLevel(probeBg->GetMapId(), bot->GetLevel())
                                : nullptr;
                    TC_LOG_DEBUG("playerbots",
                                 "[BGJ] %s (lvl %u): queue %u bracket %u -> canJoinBg=FALSE "
                                 "(template=%s access=%u queued=%u botBracket=%d)",
                                 bot->GetName().c_str(), bot->GetLevel(), uint32(queueTypeId), uint32(bracketId),
                                 probeBg ? "yes" : "NO", uint32(bot->GetBGAccessByLevel(probeType)),
                                 uint32(bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId)),
                                 probeDiff ? int32(probeDiff->GetBracketId()) : -1);
                }
                continue;
            }

            if (shouldJoinBg(queueTypeId, bracketId))
                bgList.push_back(queueTypeId);
            else if (liveQueue)
                TC_LOG_DEBUG("playerbots",
                             "[BGJ] %s (lvl %u): queue %u bracket %u -> shouldJoinBg=FALSE "
                             "(active=%u instances=%u A=%u/%u H=%u/%u team=%u)",
                             bot->GetName().c_str(), bot->GetLevel(), uint32(queueTypeId), uint32(bracketId),
                             probe.activeBgQueue, probe.bgInstanceCount,
                             probe.bgAllianceBotCount, probe.bgAlliancePlayerCount,
                             probe.bgHordeBotCount, probe.bgHordePlayerCount, uint32(bot->GetTeamId()));
        }
    }

    return !bgList.empty();
}

namespace
{
// The stock canJoinBg rule after the already-queued check (AC BattleGroundJoinAction.cpp:200-226): access by level, then
// the bracket of the bot's own level must be this bracket.
bool NativeCanJoinBg(Player* bot, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId)
{
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    PvPDifficultyEntry const* pvpDiff = sDBCManager.GetBattlegroundBracketByLevel(bg->GetMapId(), bot->GetLevel());
    if (!pvpDiff)
        return false;

    // only consider the bracket this bot's level actually belongs to
    return pvpDiff->GetBracketId() == bracketId;
}

// #702 P4 (#831): may the core put this bot in this bracket at all? Its access (Player::GetBGAccessByLevel, which the
// core Battleground.AnyLevel switch opens below the range) and the bracket's MaxLevel (HandleBattleFieldPortOpcode and
// BGStatusAction drop an invite above it).
bool AnyLevelWindowOk(Player* bot, BattlegroundTypeId bgTypeId, BattlegroundBracketId bracketId)
{
    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    PvPDifficultyEntry const* entry = sDBCManager.GetBattlegroundBracketById(bg->GetMapId(), bracketId);
    return entry && bot->GetLevel() <= entry->MaxLevel;
}
}  // namespace

bool BGJoinAction::canJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

    if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
        return false;

    // #702 P5 (#832, beyond AC, off unless AiPlayerbot.ArenaAnyLevel.Enable = 1 and a real player is in a skirmish): an
    // arena (queue, bracket) holding a real skirmish player takes the bots within AiPlayerbot.ArenaAnyLevel.LevelWindow
    // levels of that player, up to the bracket's MaxLevel (ArenaAnyLevelRequests.h); every other arena (queue, bracket)
    // keeps the stock rule, and so does the rated fill.
    if (uint8 const arenaType = BattlegroundMgr::BGArenaType(queueTypeId))
        if (ArenaAnyLevelBots::Active())
            return ArenaAnyLevelBots::AcceptBot(arenaType, uint32(bracketId), bot->GetLevel(),
                NativeCanJoinBg(bot, bgTypeId, bracketId), AnyLevelWindowOk(bot, bgTypeId, bracketId),
                PlayerbotArena::Census(uint32(queueTypeId), uint32(bracketId)).ratedArenaPlayerCount > 0);

    // #702 P4 (#831, beyond AC, off unless AiPlayerbot.BgAnyLevel.Enable = 1 and a below-range real player is queued or
    // playing): a (queue, bracket) holding such a player takes bots within AiPlayerbot.BgAnyLevel.LevelWindow levels of
    // that player (BgAnyLevelRequests.h). Arenas keep the stock rule (#832).
    if (BgAnyLevel::Active() && !BattlegroundMgr::BGArenaType(queueTypeId))
        return BgAnyLevel::AcceptBot(uint32(queueTypeId), uint32(bracketId), bot->GetLevel(),
            NativeCanJoinBg(bot, bgTypeId, bracketId), AnyLevelWindowOk(bot, bgTypeId, bracketId));

    return NativeCanJoinBg(bot, bgTypeId, bracketId);
}

bool BGJoinAction::shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId)
{
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;

    // AC BattleGroundJoinAction.cpp:239-288: arenas run on their own quota, the rated-arena census.
    if (uint8 const arenaType = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        uint32 const BracketSize = uint32(arenaType) * 2;
        uint32 const TeamSize = uint32(arenaType);

        RatedArenaInfo& arena = PlayerbotArena::Census(queueTypeId, bracketId);
        bool const isRated = (arena.ratedArenaBotCount + arena.ratedArenaPlayerCount) <
                             (BracketSize * (arena.activeRatedArenaQueue + arena.ratedArenaInstanceCount));

        // AC :258-265: only a captain queues rated, and it books its whole team against the quota at once.
        if (isRated && canLeadArenaParty(arenaType))
        {
            arena.ratedArenaBotCount += TeamSize;
            ratedList.push_back(queueTypeId);
            return true;
        }

        // AC :268-287 then tries a skirmish; 5.4.8 has none of its own. #702 P5 (#832): with AiPlayerbot.ArenaAnyLevel.Enable
        // = 1, a real player's Arena.AnyLevel skirmish in this (queue, bracket) takes lone bots up to AC's skirmish quota
        // (AC :272-291, ArenaAnyLevelBots::Wanted). A bot in a group does not skirmish (AC lets a group leader queue its
        // group; this fork's random bots queue arenas alone or as a gathered rated party). Off: false, as before.
        if (ArenaAnyLevelBots::Enabled() && !bot->GetGroup() && ArenaAnyLevelBots::Wanted(arenaType, uint32(bracketId)))
            return true;

        return false;
    }

    BattlegroundInfo const info = sRandomPlayerbotMgr->GetBattlegroundInfo(queueTypeId, bracketId);  // w24 hang: no operator[] from a map thread

    // Only fill a queue somebody is actually waiting in, and only up to what the match needs -
    // otherwise every eligible bot piles into the same queue.
    uint32 const teamSize = bg->GetMaxPlayersPerTeam();
    uint32 const wanted = teamSize * (info.activeBgQueue + info.bgInstanceCount);

    if (bot->GetTeamId() == TEAM_ALLIANCE)
        return (info.bgAllianceBotCount + info.bgAlliancePlayerCount) < wanted;

    return (info.bgHordeBotCount + info.bgHordePlayerCount) < wanted;
}

bool BGJoinAction::canLeadArenaParty(uint8 arenaType)
{
    // AC :260 asks sArenaTeamMgr->GetArenaTeamByCaptain(bot, type). 5.4.8 has no arena teams (patch 5.4.0: any
    // party of the bracket size queues it), and AC's teams are made of random bots only (RandomPlayerbotFactory),
    // so the leader stand-in is a random bot the core will let lead the queue:
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    // WorldSession::HandleBattlemasterJoinArena refuses 5v5 while solo queue owns that bracket
    // (BattleGroundHandler.cpp:583-587).
    if (arenaType == ARENA_TYPE_5v5 && sWorld->getBoolConfig(CONFIG_SOLO_QUEUE_ENABLED))
        return false;

    // Group::CanJoinBattlegroundQueue refuses the whole party for the leader alone (Group.cpp:2553, 2559).
    Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!bgTemplate || !bot->CanJoinToBattleground(bgTemplate) || bot->IsUsingLfg())
        return false;

    return true;
}

bool BGJoinAction::gatherArenaParty(uint8 arenaType)
{
    uint32 const teamSize = arenaType;

    Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(BATTLEGROUND_AA);
    if (!bgTemplate)
        return false;

    PvPDifficultyEntry const* leaderBracket =
        sDBCManager.GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), bot->GetLevel());
    if (!leaderBracket)
        return false;

    // AC :70-120 walks the captain's arena-team roster. There is no roster on 5.4.8, so walk the random-bot pool
    // (guid order) with AC's member filters, plus what AC's single-faction, level-80 roster already guaranteed and
    // what the core checks for every party member (Group::CanJoinBattlegroundQueue, Group.cpp:2528-2564).
    GuidVector members;
    for (auto const& itr : sRandomPlayerbotMgr->GetAllBots())
    {
        if (members.size() + 1 >= teamSize)
            break;

        Player* member = itr.second;
        if (!member || member == bot || !member->IsInWorld())  // AC :77-81, :103
            continue;

        PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
        if (!memberBotAI)  // AC :90-92
            continue;

        if (member->GetGroup() && memberBotAI->HasActivePlayerMaster())  // AC :94 HasGameClientMaster
            continue;

        if (!sPlayerbotAIConfig->IsInRandomAccountList(member->GetSession()->GetAccountId()))  // AC :97
            continue;

        if (member->IsInCombat() || member->InBattleground() || member->InBattlegroundQueue())  // AC :100-110
            continue;

        // What AC's team roster guaranteed: one faction (Group.cpp:2537) and the leader's bracket (Group.cpp:2540;
        // AC :162 `GetLevel() < 70` is the hard-coded level-80 form of this).
        if (member->GetTeam() != bot->GetTeam())
            continue;

        if (sDBCManager.GetBattlegroundBracketByLevel(bgTemplate->GetMapId(), member->GetLevel()) != leaderBracket)
            continue;

        // The core refuses the party for any of these (Group.cpp:2553-2560); do not pull such a bot in.
        if (!member->CanJoinToBattleground(bgTemplate) || member->IsUsingLfg() || !member->HasFreeBattlegroundQueueId())
            continue;

        // .botscenario: a bg stage limits arena parties to its roster, as it already limits who self-queues.
        if (BotScenario::BgJoinBlocked(member))
            continue;

        members.push_back(member->GetGUID());
    }

    // AC :122-127
    if (members.size() + 1 < teamSize)
    {
        TC_LOG_DEBUG("playerbots", "[ARENA] Bot %u <%s>: %s party has not enough members for match (%u of %u)",
                     bot->GetGUID().GetCounter(), bot->GetName().c_str(), ArenaTypeName(arenaType),
                     uint32(members.size() + 1), teamSize);
        return false;
    }

    // AC :112-115, done only for the members actually picked: with no fixed roster, pulling bots out of their
    // groups before a full party is certain would break parties for nothing.
    for (ObjectGuid const& guid : members)
    {
        Player* member = ObjectAccessor::FindConnectedPlayer(guid);
        if (!member)
            continue;

        if (member->GetGroup())
            member->GetGroup()->RemoveMember(member->GetGUID());

        if (PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member))
            memberBotAI->Reset();
    }

    // AC :129-142: disband the leader's group and found a fresh one.
    if (bot->GetGroup())
        bot->GetGroup()->Disband(true);

    std::string const partyName = bot->GetName() + "'s " + ArenaTypeName(arenaType) + " party";

    Group* group = new Group();
    if (!group->Create(bot))
    {
        TC_LOG_DEBUG("playerbots", "[ARENA] <%s>: Can't create group for arena queue", partyName.c_str());
        delete group;
        return false;
    }
    sGroupMgr->AddGroup(group);

    TC_LOG_DEBUG("playerbots", "[ARENA] Bot %u <%s>: Leader of <%s>", bot->GetGUID().GetCounter(),
                 bot->GetName().c_str(), partyName.c_str());

    // AC :147-178
    for (ObjectGuid const& guid : members)
    {
        if (group->GetMembersCount() >= teamSize)
            break;

        Player* member = ObjectAccessor::FindConnectedPlayer(guid);
        if (!member)
            continue;

        if (!group->AddMember(member))
            continue;

        PlayerbotAI* memberBotAI = GET_PLAYERBOT_AI(member);
        if (!memberBotAI)
            continue;

        memberBotAI->Reset();
        // AC passes AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP; this core's overload builds
        // flag64{flag, 0} and flag64 has no operator| - two calls remove the same auras (#372 note).
        member->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED);
        member->RemoveAurasWithInterruptFlags(AURA_INTERRUPT_FLAG_CHANGE_MAP);
        member->TeleportTo(bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(), 0);

        TC_LOG_DEBUG("playerbots", "[ARENA] Bot %u <%s>: Member of <%s>", member->GetGUID().GetCounter(),
                     member->GetName().c_str(), partyName.c_str());
    }

    // AC :180-193
    if (group->GetMembersCount() >= teamSize)
    {
        TC_LOG_DEBUG("playerbots", "[ARENA] <%s> Group is ready for match", partyName.c_str());
        return true;
    }

    TC_LOG_DEBUG("playerbots", "[ARENA] <%s> Group is not ready for match (not enough members)", partyName.c_str());
    group->Disband();
    return false;
}

bool BGJoinAction::Execute(Event /*event*/)
{
    if (bgList.empty())
        return false;

    uint32 const queueTypeId = bgList[urand(0, bgList.size() - 1)];

    // #897: a rated battleground is queued by a gathered 10-man raid (HandleBattlemasterJoinRated), not by JoinQueue.
    if (queueTypeId == BATTLEGORUND_QUEUE_RATED_BG)
        return PlayerbotRatedBg::GatherAndQueue(bot);

    // AC :39-49: a rated arena is queued by a party; gather it first.
    if (uint8 const arenaType = BattlegroundMgr::BGArenaType(BattlegroundQueueTypeId(queueTypeId)))
    {
        bool const isRated = std::find(ratedList.begin(), ratedList.end(), queueTypeId) != ratedList.end();
        // AC queues a skirmish when !isRated; 5.4.8 has none, so only the rated path exists - unless #702 P5 (#832)
        // AiPlayerbot.ArenaAnyLevel is on: then a lone bot queues the skirmish itself (JoinQueue), without a party.
        if (!isRated)
        {
            if (!ArenaAnyLevelBots::Enabled() || bot->GetGroup())
                return false;
        }
        else if (!gatherArenaParty(arenaType))
            return false;
    }

    return JoinQueue(queueTypeId);
}

bool BGJoinAction::JoinQueue(uint32 type)
{
    if (!bot || !bot->IsInWorld() || bot->InBattleground())
        return false;

    BattlegroundQueueTypeId queueTypeId = BattlegroundQueueTypeId(type);
    BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
    if (!bg)
        return false;

    PvPDifficultyEntry const* bracketEntry = sDBCManager.GetBattlegroundBracketByLevel(bg->GetMapId(), bot->GetLevel());

    // #702 P4 (#831, off by default): with the any-level verdict in play, queue under the first bracket that both
    // canJoinBg and shouldJoinBg accept (isUseful's own order), which is a below-range requester's bracket for a bot in
    // its level window, else the bot's own bracket.
    if (BgAnyLevel::Active() && !BattlegroundMgr::BGArenaType(queueTypeId))
    {
        bracketEntry = nullptr;
        for (uint32 bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS && !bracketEntry; ++bracket)
            if (canJoinBg(queueTypeId, BattlegroundBracketId(bracket)) && shouldJoinBg(queueTypeId, BattlegroundBracketId(bracket)))
                bracketEntry = sDBCManager.GetBattlegroundBracketById(bg->GetMapId(), BattlegroundBracketId(bracket));
    }

    if (!bracketEntry)
        return false;

    if (bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId))
        return false;

    if (!bot->GetBGAccessByLevel(bgTypeId))
        return false;

    if (!bot->HasFreeBattlegroundQueueId())
        return false;

    // Deserter and friends.
    if (!bot->CanJoinToBattleground(bg))
        return false;

    // #702 P5 (#832, off by default): a skirmish. Execute let a lone bot through for an arena queue it holds no rated
    // booking for. The bot joins the bracket of the real skirmish player canJoinBg accepted it for (the window may pick a
    // bracket other than its own level's) directly in the queue, as the battleground path below does
    // (HandleBattlemasterJoinArena would file it under its own level's bracket, and rated); unrated and not premade,
    // so the queue's unrated pass matches it (BattlegroundQueue::BattlegroundQueueUpdate !isRated). AC JoinQueue :522-525
    // books the bot into the skirmish census.
    if (uint8 const skirmishType = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        if (ArenaAnyLevelBots::Enabled() && !bot->GetGroup() &&
            std::find(ratedList.begin(), ratedList.end(), type) == ratedList.end())
        {
            PvPDifficultyEntry const* skirmishBracket = nullptr;
            for (uint32 bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS && !skirmishBracket; ++bracket)
                if (canJoinBg(queueTypeId, BattlegroundBracketId(bracket)) && ArenaAnyLevelBots::Wanted(skirmishType, bracket))
                    skirmishBracket = sDBCManager.GetBattlegroundBracketById(bg->GetMapId(), BattlegroundBracketId(bracket));
            if (!skirmishBracket)
                return false;

            // the rest of Group::CanJoinBattlegroundQueue's per-member checks (the common ones ran above)
            if (bot->IsUsingLfg(false))
                return false;
            for (BattlegroundQueueTypeId rated : { BATTLEGROUND_QUEUE_2v2, BATTLEGROUND_QUEUE_3v3, BATTLEGROUND_QUEUE_5v5, BATTLEGROUND_QUEUE_SOLO })
                if (bot->InBattlegroundQueueForBattlegroundQueueType(rated))
                    return false;

            BattlegroundQueue& skirmishQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);
            GroupQueueInfo* skirmishInfo = skirmishQueue.AddGroup(bot, nullptr, bgTypeId, skirmishBracket, skirmishType, false, false, 0, 0);
            if (!skirmishInfo)
                return false;

            uint32 const skirmishWait = skirmishQueue.GetAverageQueueWaitTime(skirmishInfo, skirmishBracket->GetBracketId());
            uint32 const skirmishSlot = bot->AddBattlegroundQueueId(queueTypeId);
            bot->AddBattlegroundQueueJoinTime(bgTypeId, skirmishInfo->JoinTime);

            WorldPacket skirmishData;
            sBattlegroundMgr->BuildBattlegroundStatusPacket(&skirmishData, bg, bot, skirmishSlot, STATUS_WAIT_QUEUE, skirmishWait,
                skirmishInfo->JoinTime, skirmishType);
            bot->GetSession()->SendPacket(&skirmishData);

            ArenaAnyLevelBots::Book(skirmishType, uint32(skirmishBracket->GetBracketId()));
            sBattlegroundMgr->ScheduleQueueUpdate(0, skirmishType, queueTypeId, bgTypeId, skirmishBracket->GetBracketId());

            TC_LOG_DEBUG("playerbots", "[ARENA] Bot %u %s:%u <%s> queued %s Skirmish (Arena.AnyLevel; queue %u, bracket %u)",
                         bot->GetGUID().GetCounter(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                         bot->GetName().c_str(), ArenaTypeName(skirmishType), uint32(queueTypeId),
                         uint32(skirmishBracket->GetBracketId()));
            return true;
        }
    }

    // AC :483-553: arenas are joined by the party leader through CMSG_BATTLEMASTER_JOIN_ARENA. On 5.4.8 that
    // packet is a single uint8 slot (BattleGroundHandler.cpp:575-577), and this fork's bot sessions never drain
    // queued packets (LfgActions.cpp header), so call the handler directly. It builds the per-match ArenaTeam from
    // each member's RatedPvpMgr row of THIS slot (GetOrCreateInfo, ArenaTeam.cpp:23-39), runs
    // Group::CanJoinBattlegroundQueue (faction, bracket, exact party size) and queues the party rated. The slot is
    // the core's own type -> slot map (2v2/3v3/5v5 -> 0/1/2, AC :491-507), so a builder's "always
    // slot 0" is not repeated. AC's battlemaster lookup (:437-445) has no 5.4.8 counterpart: the packet carries
    // no battlemaster guid.
    if (uint8 const arenaType = BattlegroundMgr::BGArenaType(queueTypeId))
    {
        Group* group = bot->GetGroup();
        if (!group || !group->IsLeader(bot->GetGUID()))  // the handler's own leader check (:613-618)
            return false;

        uint8 const arenaSlot = ArenaTeam::GetSlotByType(arenaType);
        if (arenaSlot >= PVP_SLOT_RATED_BG)
            return false;

        WorldPacket packet(CMSG_BATTLEMASTER_JOIN_ARENA, 1);
        packet << uint8(arenaSlot);
        bot->GetSession()->HandleBattlemasterJoinArena(packet);

        bool const queued = bot->InBattlegroundQueueForBattlegroundQueueType(queueTypeId);

        std::string party;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            if (Player* member = ref->GetSource())
                party += (party.empty() ? "" : ",") + member->GetName();

        // AC :510-514's line, plus the party and the core's verdict.
        TC_LOG_DEBUG("playerbots",
                     "[ARENA] Bot %u %s:%u <%s> queued %s Rated Arena (slot %u, queue %u, bracket %u, party %u {%s}) ok=%u",
                     bot->GetGUID().GetCounter(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                     bot->GetName().c_str(), ArenaTypeName(arenaType), uint32(arenaSlot), uint32(queueTypeId),
                     uint32(bracketEntry->GetBracketId()), group->GetMembersCount(), party.c_str(), queued ? 1 : 0);

        return queued;
    }

    // Mirrors the tail of WorldSession::HandleBattlemasterJoinOpcode.
    BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);

    GroupQueueInfo* ginfo = bgQueue.AddGroup(bot, nullptr, bgTypeId, bracketEntry, 0, false, false, 0, 0);
    if (!ginfo)
        return false;

    uint32 avgWaitTime = bgQueue.GetAverageQueueWaitTime(ginfo, bracketEntry->GetBracketId());
    uint32 queueSlot = bot->AddBattlegroundQueueId(queueTypeId);

    WorldPacket data;
    sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, bot, queueSlot, STATUS_WAIT_QUEUE, avgWaitTime, 0, 0);
    bot->GetSession()->SendPacket(&data);

    sBattlegroundMgr->ScheduleQueueUpdate(0, 0, queueTypeId, bgTypeId, bracketEntry->GetBracketId());

    TC_LOG_DEBUG("playerbots", "[BG] %s (%s %u) queued for battleground %u (queue %u, bracket %u)",
                 bot->GetName().c_str(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(),
                 uint32(bgTypeId), uint32(queueTypeId), uint32(bracketEntry->GetBracketId()));

    return true;
}

bool BGStatusAction::isUseful()
{
    // #210 S10 G6 (AC BattleGroundJoinAction.cpp:744, `InBattlegroundQueue()` only): AC's isUseful is
    // just "queued", because AC's own match-end leave runs off a distinct Time1==TIME_TO_AUTOREMOVE
    // check inside the same handler. This core sends the match-end notice as a fresh
    // SMSG_BATTLEFIELD_STATUS_ACTIVE (STATUS_IN_PROGRESS with GetStatus()==STATUS_WAIT_LEAVE - see
    // AI/PlayerbotAI.cpp's opcode registration), so a bot already inside the battleground must still
    // be allowed to run this action for that one packet.
    return bot && (bot->InBattlegroundQueue() || bot->InBattleground());
}

bool BGStatusAction::Execute(Event /*event*/)
{
    // #210 S10 G6 (AC BattleGroundJoinAction.cpp:870-878): the match ended and the core moved the
    // battleground to STATUS_WAIT_LEAVE. On maps with no BGTactics case (BGTactics::Execute's own
    // STATUS_WAIT_LEAVE handling never runs there - see AiFactory.cpp's map switch) this was the only
    // way such a bot ever left after a match, instead of waiting for the core's own auto-close.
    if (bot->InBattleground())
    {
        if (Battleground* bg = bot->GetBattleground())
            if (bg->GetStatus() == STATUS_WAIT_LEAVE)
                return LeaveBG(botAI);

        return false;
    }

    bool acted = false;

    for (uint8 queueSlot = 0; queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueSlot)
    {
        BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(queueSlot);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;

        BattlegroundQueue& bgQueue = sBattlegroundMgr->GetBattlegroundQueue(queueTypeId);

        GroupQueueInfo ginfo;
        if (!bgQueue.GetPlayerGroupInfoData(bot->GetGUID(), &ginfo))
            continue;

        // Not invited yet - keep waiting.
        if (!ginfo.IsInvitedToBGInstanceGUID)
        {
            // #632 AC BattleGroundJoinAction.cpp:925-962 (STATUS_WAIT_QUEUE; the force-join half, :888-923, is the
            // accept path below). AC reaches this from SMSG_BATTLEFIELD_STATUS (WAIT_QUEUE); 5.4.8 sends
            // SMSG_BATTLEFIELD_STATUS_QUEUED, routed to "bg status" in AI/PlayerbotAI.cpp, and this action reads the
            // queue instead of the packet. Arena only (AC :933 "disabled for BG").
            if (BattlegroundMgr::BGArenaType(queueTypeId) && LeaveArenaQueueIfWaitedTooLong(queueTypeId, ginfo))
            {
                acted = true;
                break;
            }

            // #897: a rated battleground group nobody ever matched leaves (every member, not only the leader).
            if (queueTypeId == BATTLEGORUND_QUEUE_RATED_BG && PlayerbotRatedBg::LeaveQueueIfWaitedTooLong(bot, ginfo))
            {
                botAI->ResetStrategies();
                acted = true;
                break;
            }
            continue;
        }

        if (!bot->IsInvitedForBattlegroundQueueType(queueTypeId))
            continue;

        BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
        Battleground* bg = sBattlegroundMgr->GetBattleground(ginfo.IsInvitedToBGInstanceGUID, bgTypeId);
        if (!bg)
            continue;

        bgTypeId = bg->GetTypeID();

        // #702 P4: the core's own lookup for the port (HandleBattleFieldPortOpcode), incl. the Battleground.AnyLevel
        // fallback for a bot below the battleground's range; with the switch off it is the stock lookup.
        PvPDifficultyEntry const* bracketEntry =
            BattlegroundAnyLevel::GetBracketByLevel(bg, bg->GetMapId(), bot->GetLevel());
        if (!bracketEntry)
            continue;

        // Levelled past the bracket, or picked up a deserter debuff while waiting.
        if (!bot->CanJoinToBattleground(bg) || bot->GetLevel() > bg->GetMaxLevel())
        {
            bot->RemoveBattlegroundQueueId(queueTypeId);
            bgQueue.RemovePlayer(bot->GetGUID(), true, true);
            continue;
        }

        // AC BattleGroundJoinAction.cpp:1027-1028 (#210 S10 G8): dismount before the port, the way
        // the client does - CMSG_CANCEL_MOUNT_AURA clears the mount aura; Unit::Dismount() alone
        // leaves it and the bot can never remount (same primitive as
        // strategy/actions/CheckMountStateAction.cpp's DismountBot()).
        if (bot->IsMounted())
        {
            WorldPacket emptyPacket;
            bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
        }

        // Mirrors the accept branch of WorldSession::HandleBattleFieldPortOpcode.
        if (!bot->InBattleground())
            bot->SetBattlegroundEntryPoint();

        if (!bot->IsAlive())
        {
            bot->ResurrectPlayer(1.0f);
            bot->SpawnCorpseBones();
        }

        if (bot->IsInFlight())
        {
            bot->GetMotionMaster()->MovementExpired();
            bot->CleanupAfterTaxiFlight();
        }

        // Leave combat, pets included, before being ported across maps. A real player is out of
        // combat by the time they click "enter battle"; a bot can be mid-fight with a guardian
        // out, and that guardian is then destroyed by the map change while it still holds combat
        // references - which trips the ASSERT in CombatManager::~CombatManager.
        bot->CombatStopWithPets(true);

        WorldPacket data;
        sBattlegroundMgr->BuildBattlegroundStatusPacket(&data, bg, bot, queueSlot, STATUS_IN_PROGRESS,
                                                        bot->GetBattlegroundQueueJoinTime(bgTypeId),
                                                        bg->GetElapsedTime(), bg->GetArenaType());
        bot->GetSession()->SendPacket(&data);

        bgQueue.RemovePlayer(bot->GetGUID(), false);

        if (Battleground* currentBg = bot->GetBattleground())
            currentBg->RemovePlayerAtLeave(bot->GetGUID(), false, true);

        bot->SetBattlegroundId(bg->GetInstanceID(), bgTypeId);
        bot->SetBGTeam(ginfo.Team);

        sLFGMgr->RemovePlayerQueuesOnPartyFound(bot->GetGUID());

        // AC modules/mod-playerbots/src/Ai/Base/Actions/BattleGroundJoinAction.cpp:1037: rebuild the
        // strategies when the bot accepts, so a bot grouped with a real player GAINS the battleground
        // strategy set and plays the objective instead of trailing its master. That is the whole port.
        //
        // AC's FOLLOWING line is deliberately not ported:
        //     if (!bot->GetBattleground()) botAI->ChangeStrategy("+bg", BOT_STATE_NON_COMBAT);
        // This core calls `bot->SetBattlegroundId(...)` earlier in this same accept branch (above), so
        // `GetBattleground()` is already non-null here and that guard can never be true - porting it
        // would add a line that never executes. AC reaches the opposite state because it queues
        // CMSG_BATTLEFIELD_PORT first and only resets afterwards.
        //
        // Measured on a real WSG accept:
        //     [BGS] <bot>: in bg map 489 -> adding battleground strategies (combat engine)
        //     [BGS] <bot>: in bg map 489 -> adding battleground strategies (non-combat engine)
        //   -> AddBattlegroundStrategies only logs when GetBattleground() is non-null, so this is
        //      direct confirmation that AC's guard would be false at this point.
        //     ... S:+warsong ... S:+follow
        //   -> "+follow" IS still added in the same pass, so `follow` REMAINS in the non-combat set;
        //      it is superseded in play by the battleground strategies, not removed (the grouped bots
        //      were measured running battleground actions, e.g. "move to objective", rather than
        //      following their master).
        botAI->ResetStrategies();

        // Give the bot a job for this match. The tactics split attackers from defenders on this
        // number (BGTactics::selectObjective compares it against a per-strategy threshold), so
        // leaving every bot on the default 0 makes the whole team defend its own base and nobody
        // ever goes for the enemy flag. Upstream rolls it here, on accepting the invitation.
        context->GetValue<uint32>("bg role")->Set(urand(0, 9));

        // AC BattleGroundJoinAction.cpp:1045-1051: throw away any objective position left over from
        // a previous match before the bot starts moving in the new one (bg-parity-stage-design.md
        // fix 4; the bot-side half of issue #125's "references the old BG"). "position" is a plain
        // string-keyed map, so resetting a key that a map doesn't use yet (bg siege - AV/IoC are not
        // ported) is harmless.
        {
            PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
            PositionInfo pos = posMap["bg objective"];
            PositionInfo pos2 = posMap["bg siege"];
            pos.Reset();
            pos2.Reset();
            posMap["bg objective"] = pos;
            posMap["bg siege"] = pos2;
        }

        sBattlegroundMgr->SendToBattleground(bot, ginfo.IsInvitedToBGInstanceGUID, bgTypeId);

        TC_LOG_DEBUG("playerbots", "[BG] %s accepted the invite and is entering battleground %u (instance %u)",
                     bot->GetName().c_str(), uint32(bgTypeId), bg->GetInstanceID());

        acted = true;
        break;
    }

    return acted;
}

// #632 AC BattleGroundJoinAction.cpp:925-962. Time2 (AC) is the time the bot has been queued; AC's TIME_TO_AUTOREMOVE
// (Battleground.h:157) is 120000 ms, which this core calls TIME_AUTOCLOSE_BATTLEGROUND (Battleground.h:133, 120000 ms).
bool BGStatusAction::LeaveArenaQueueIfWaitedTooLong(BattlegroundQueueTypeId queueTypeId, GroupQueueInfo const& ginfo)
{
    uint32 const time2 = getMSTimeDiff(ginfo.JoinTime, getMSTime());
    if (time2 <= TIME_AUTOCLOSE_BATTLEGROUND)
        return false;
    if (BotScenario::BotScenarioHoldsArenaRoster(bot))  // #632 harness guard: a scenario roster bot keeps its queue
        return false;

    // AC :950-951: only a group leader, or a lone bot with no master, leaves.
    if (!((bot->GetGroup() && bot->GetGroup()->IsLeader(bot->GetGUID())) || !(bot->GetGroup() || botAI->GetMaster())))
        return false;

    Battleground* bg = sBattlegroundMgr->GetBattlegroundTemplate(BattlegroundMgr::BGTemplateId(queueTypeId));
    if (!bg)
        return false;

    // #702 P5 (#832, off by default): AC :955-960's skirmish half - a real player waiting in this bot's skirmish keeps it
    // queued. Judged on the bracket the queue assigned (a window bot may sit in another level's bracket).
    if (!ginfo.IsRated && ArenaAnyLevelBots::Enabled())
        if (PvPDifficultyEntry const* assigned = GetAssignedBgBracket(bot, queueTypeId, bg->GetMapId()))
            if (ArenaAnyLevelBots::PlayerCount(ginfo.ArenaType, uint32(assigned->GetBracketId())) > 0)
                return false;

    PvPDifficultyEntry const* bracketEntry = sDBCManager.GetBattlegroundBracketByLevel(bg->GetMapId(), bot->GetLevel());
    if (!bracketEntry)
        return false;

    // AC :953-962: a real player waiting in this rated bracket keeps the bot queued (5.4.8 has no skirmish count).
    if (PlayerbotArena::Census(uint32(queueTypeId), bracketEntry->GetBracketId()).ratedArenaPlayerCount > 0)
        return false;

    TC_LOG_DEBUG("playerbots", "[BG] %s %u:%u <%s> waited too long and leaves queue (Arena %ux%u).",
                 bot->GetGUID().ToString().c_str(), uint32(bot->GetTeamId()), bot->GetLevel(), bot->GetName().c_str(),
                 uint32(ginfo.ArenaType), uint32(ginfo.ArenaType));

    // AC sends CMSG_BATTLEFIELD_PORT with action 0 (:966-969); this is the same drop BGLeaveAction does.
    bot->RemoveBattlegroundQueueId(queueTypeId);
    sBattlegroundMgr->GetBattlegroundQueue(queueTypeId).RemovePlayer(bot->GetGUID(), true, true);

    // AC :971-974 (ResetStrategies(!IsRandomBot): the fork's takes no flag; "arena type" value does not exist here).
    botAI->ResetStrategies();
    botAI->GetAiObjectContext()->GetValue<uint32>("bg type")->Set(0);
    botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);
    return true;
}

bool BGStatusAction::LeaveBG(PlayerbotAI* botAI)
{
    if (!botAI)
        return false;

    Player* bot = botAI->GetBot();
    if (!bot)
        return false;

    Battleground* bg = bot->GetBattleground();
    if (!bg)
        return false;

    // AC OnBattlegroundEnd erases the team-strategy roll for the instance (Script/Playerbots.cpp:
    // 558); MoP has no such hook, so do it here, on the one path both the automatic
    // STATUS_WAIT_LEAVE handling (BGTactics::Execute) and the "bg leave" command funnel through.
    // Otherwise the map would grow forever and a later bot could momentarily reuse a stale strategy
    // if the instance id got recycled.
    // Only once the match has ended (AC erases in OnBattlegroundEnd): a bot leaving mid-match via "bg leave"
    // must not wipe the roll the rest of its team is still playing.
    if (bg->GetStatus() == STATUS_WAIT_LEAVE)
        bgStrategies.erase(bg->GetInstanceID());

    // AC BattleGroundJoinAction.cpp:704-705 (#210 S10 G7): drop the master so a random bot
    // re-acquires one through the periodic FindNewMaster-equivalent pass (AI/PlayerbotAI.cpp:
    // ~1486-1501) and re-announces itself ("Hello, I follow you!"), instead of silently keeping a
    // BG-mate as "master" back out in the world.
    if (sRandomPlayerbotMgr->IsRandomBot(bot))
        botAI->SetMaster(nullptr);

    // #897: the rated BG type is the queue type (BgRealType.h), so this is the match of a rated raid
    bool const ratedBgMatch = bg->GetTypeID() == BATTLEGROUND_RATED_10_VS_10;

    bot->LeaveBattleground();
    if (ratedBgMatch)
        PlayerbotRatedBg::LeaveGroupAfterMatch(bot);  // the gathered raid dissolves, like a finished arena party
    botAI->GetAiObjectContext()->GetValue<uint32>("bg role")->Set(0);

    // AC BattleGroundJoinAction.cpp:733-740: drop the stale objective position too (bg-parity-stage-
    // design.md fix 4), so it cannot be read as still-valid the next time this bot enters a match.
    {
        PositionMap& posMap = botAI->GetAiObjectContext()->GetValue<PositionMap&>("position")->Get();
        PositionInfo pos = posMap["bg objective"];
        pos.Reset();
        posMap["bg objective"] = pos;
    }

    botAI->ResetStrategies();

    TC_LOG_DEBUG("playerbots", "[BG] %s left battleground %u", bot->GetName().c_str(), uint32(bg->GetTypeID()));
    return true;
}

// #396 AC Ai/Base/Actions/BattleGroundJoinAction.cpp:1059-1071. 5.4.8's WorldSession::HandleBattlefieldStatusOpcode
// (Handlers/BattleGroundHandler.cpp:506) ignores the packet body like AC's, and answers an invited slot with
// STATUS_WAIT_JOIN = SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION, which AI/PlayerbotAI.cpp routes to the "bg status" trigger.
bool BGStatusCheckAction::Execute(Event /*event*/)
{
    if (bot->IsBeingTeleported())
        return false;

    WorldPacket packet(CMSG_BATTLEFIELD_STATUS);
    bot->GetSession()->HandleBattlefieldStatusOpcode(packet);

    TC_LOG_DEBUG("playerbots", "Bot %s <%s> (%u %s) : Checking BG invite status", bot->GetGUID().ToString().c_str(),
                 bot->GetName().c_str(), bot->GetLevel(), bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H");

    return true;
}

// AC BattleGroundJoinAction.cpp:1073.
bool BGStatusCheckAction::isUseful() { return bot->InBattlegroundQueue(); }

bool BGLeaveAction::Execute(Event /*event*/)
{
    if (bot->InBattleground())
        return BGStatusAction::LeaveBG(botAI);

    // Not in a match: drop any queue the bot is sitting in.
    bool removed = false;
    for (uint8 queueSlot = 0; queueSlot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++queueSlot)
    {
        BattlegroundQueueTypeId queueTypeId = bot->GetBattlegroundQueueTypeId(queueSlot);
        if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
            continue;

        bot->RemoveBattlegroundQueueId(queueTypeId);
        sBattlegroundMgr->GetBattlegroundQueue(queueTypeId).RemovePlayer(bot->GetGUID(), true, true);
        removed = true;

        // Rare enough to be worth a line: this only happens when a queue the bot joined to fill for
        // a real player has gone dead (see BgQueueDeadTrigger and issue #50).
        TC_LOG_DEBUG("playerbots", "[BG] %s left queue %u (nothing left to fill)", bot->GetName().c_str(),
                     uint32(queueTypeId));
    }

    return removed;
}

bool BGStrategyCheckAction::Execute(Event /*event*/)
{
    bool const insideBg = bot->InBattleground() && bot->GetBattleground();
    bool const hasBgStrategy = botAI->HasStrategy("battleground", BOT_STATE_NON_COMBAT);

    if (insideBg == hasBgStrategy)
        return false;

    // Out of step: rebuild from AiFactory, which adds the in-match strategies when the bot is in a
    // battleground and the ordinary world strategies when it is not.
    botAI->ResetStrategies();

    // issue #97: the world-roaming layer must not run inside a battleground. "new rpg" picks
    // grind/camp spots from the farm cache, and inside a BG it selected spots on the battleground
    // map itself (observed: "[New RPG] Bot Catha select random grind pos Map:489 X:949.9 Y:1459.2"),
    // which sent bots wandering the whole map - through the closed gates during WAIT_JOIN. That is
    // what players see as "a few bots run through the gate before the match starts". Gathering is
    // equally meaningless there. Both come back when the bot leaves, because ResetStrategies()
    // rebuilds from AiFactory.
    if (insideBg)
    {
        botAI->ChangeStrategy("-new rpg", BOT_STATE_NON_COMBAT);
        botAI->ChangeStrategy("-gather", BOT_STATE_NON_COMBAT);
    }

    // Upstream reports "acted" only when leaving, so that entering does not consume the action
    // slot the bot needs for its first in-match decision on the same tick.
    return !insideBg;
}
