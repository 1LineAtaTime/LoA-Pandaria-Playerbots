/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/BattleGroundJoinAction.h)
 * into the Legends of Azeroth Pandaria fork.
 *
 * Rated arenas (#437): 5.4.8 has no arena teams (patch 5.4.0), so AC's team-captain path becomes "a random bot
 * gathers a same-faction party and queues the rated bracket" - see BGJoinAction::gatherArenaParty.
 */

#ifndef _PLAYERBOT_BATTLEGROUNDJOINACTION_H
#define _PLAYERBOT_BATTLEGROUNDJOINACTION_H

#include "Action.h"
#include "DBCEnums.h"
#include "SharedDefines.h"

#include <vector>

class PlayerbotAI;

enum BattlegroundQueueTypeId : uint8;
enum BattlegroundBracketId : uint8;

/// Battleground queues bots are allowed to use.
///
/// Upstream has no filter here at all - it walks every queue type and lets canJoinBg() decide,
/// which works on 3.3.5 because every queue in that enum is either a battleground or an arena and
/// BattlegroundMgr::BGArenaType() identifies the arenas. 5.4.8 adds the rated battleground and
/// solo queues, for which BGArenaType() also reports 0, so this is a deny-list of exactly those
/// extra queue types. Everything upstream would consider - Random Battleground included - is
/// allowed through.
bool IsSupportedBgQueue(BattlegroundQueueTypeId queueTypeId);

class Player;
struct PvPDifficultyEntry;

/// #693, AC mod-playerbots#2850 (Bot/RandomPlayerbotMgr.cpp:938-948, 1012, 1098): is `player` inside the match of
/// this queue slot (AC: InBattleground() && GetCurrentBattlegroundQueueSlot() == slot)?
bool InMatchOfBgQueue(Player* player, BattlegroundQueueTypeId queueTypeId);
/// #693, AC getQueueBracket (same lines): the bracket the instance or the queue assigned to `player` for this queue
/// slot, never the one of their current level (a participant may level past it while waiting or inside the match).
PvPDifficultyEntry const* GetAssignedBgBracket(Player* player, BattlegroundQueueTypeId queueTypeId, uint32 mapId);

/// Rated-arena census for one (arena queue type, bracket): AC BattlegroundInfo's rated-arena fields
/// (AC Bot/RandomPlayerbotMgr.h:20-40), kept out of RandomPlayerbotMgr.h's BattlegroundInfo because that header
/// reaches every module TU through Playerbots.h. AC's skirmish fields are not ported: 5.4.8 has no skirmish
/// arenas (removed in 4.0.1, back in 6.0.2) and every CMSG_BATTLEMASTER_JOIN_ARENA queues the rated bracket.
struct RatedArenaInfo
{
    std::vector<uint32> ratedArenaInstances;
    uint32 ratedArenaInstanceCount = 0;
    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    uint32 activeRatedArenaQueue = 0;  // 0 = Inactive, 1 = Active
    uint32 ratedArenaBotCount = 0;     // Bots (Arena)
    uint32 ratedArenaPlayerCount = 0;  // Players (Arena)
};

/// The rated-arena half of AC RandomPlayerbotMgr::CheckBgQueue (#437), called from the fork's CheckBgQueue.
namespace PlayerbotArena
{
    /// [arena queue type][bracket] entry. Written by the census on the world thread; bots read it, and book their
    /// own party into ratedArenaBotCount, in BGJoinAction::shouldJoinBg - exactly as AC does with BattlegroundData.
    RatedArenaInfo& Census(uint32 queueTypeId, uint32 bracketId);
    /// AC :928-934 - rebuild from scratch on every census pass.
    void ResetCensus();
    /// AC :968-1025 (real players) and :1057-1126 (random bots) for one arena queue slot of `player`.
    void CountQueued(Player* player, bool isBot, BattlegroundQueueTypeId queueTypeId);
    /// AC :1133-1136, :1152-1158, :1172-1177 - keep the configured number of rated matches going.
    void SeedAutoJoin();
    /// AC LogBattlegroundInfo :1198-1217 (arena lines), at DEBUG like the fork's [BGQ] census.
    void LogCensus();
}

/// Decides whether the bot should queue for a battleground, and does it.
class BGJoinAction : public Action
{
public:
    BGJoinAction(PlayerbotAI* botAI, std::string const name = "bg join") : Action(botAI, name) {}

    bool isUseful() override;
    bool Execute(Event event) override;

    /// Is this (queue, bracket) pair one the bot is allowed into at all?
    bool canJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId);
    /// Would queueing here actually help a match form, given who is already waiting?
    virtual bool shouldJoinBg(BattlegroundQueueTypeId queueTypeId, BattlegroundBracketId bracketId);

protected:
    bool JoinQueue(uint32 type);
    /// AC gatherArenaTeam (BattleGroundJoinAction.cpp:60-194), MoP form: builds a same-faction, same-bracket party
    /// of `arenaType` bots led by this bot.
    bool gatherArenaParty(uint8 arenaType);
    /// MoP stand-in for AC's "only captains queue rated" (sArenaTeamMgr->GetArenaTeamByCaptain, :260).
    bool canLeadArenaParty(uint8 arenaType);

    std::vector<uint32> bgList;
    std::vector<uint32> ratedList;  // AC :364-365 - the arena queues this bot may lead a rated party into
};

/// Accepts a battleground invitation (the "enter battle" popup) or drops the queue.
class BGStatusAction : public Action
{
public:
    BGStatusAction(PlayerbotAI* botAI) : Action(botAI, "bg status") {}

    bool isUseful() override;
    bool Execute(Event event) override;

    /// Leave the battleground the bot is currently in, if any.
    static bool LeaveBG(PlayerbotAI* botAI);

private:
    /// #632 AC BattleGroundJoinAction.cpp:925-962: an arena captain queued past TIME_AUTOCLOSE_BATTLEGROUND leaves.
    bool LeaveArenaQueueIfWaitedTooLong(BattlegroundQueueTypeId queueTypeId, struct GroupQueueInfo const& ginfo);
};

/// #396 (AC Ai/Base/Actions/BattleGroundJoinAction.h:64-71): asks the server to resend the bot's battleground
/// status (CMSG_BATTLEFIELD_STATUS), so a pending invite comes back as SMSG_BATTLEFIELD_STATUS_NEEDCONFIRMATION and
/// the "bg status" packet trigger runs BGStatusAction on it. Driven by "bg invite active" in BgStrategy.
class BGStatusCheckAction : public Action
{
public:
    BGStatusCheckAction(PlayerbotAI* botAI, std::string const name = "bg status check") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

/// Leaves the battleground / queue.
class BGLeaveAction : public Action
{
public:
    BGLeaveAction(PlayerbotAI* botAI, std::string const name = "bg leave") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

/// Keeps the bot's strategy set in step with whether it is inside a battleground.
///
/// AiFactory decides which strategies a bot gets, but it only runs when the bot's strategies are
/// (re)built - not when the bot is ported into a match or thrown out at the end of one. Upstream
/// closes that gap by polling this from the queueing strategy: when the "battleground" strategy
/// and the bot's actual location disagree, it calls ResetStrategies() and lets AiFactory rebuild
/// the right set. Without it a bot keeps its out-of-world behaviour for the entire match and
/// keeps its in-match behaviour after being thrown out.
class BGStrategyCheckAction : public Action
{
public:
    BGStrategyCheckAction(PlayerbotAI* botAI, std::string const name = "bg strategy check")
        : Action(botAI, name)
    {
    }

    bool Execute(Event event) override;
};

#endif
