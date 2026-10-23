/*
 * #897 (beyond AC, MoP-only): random bots queue the RATED battleground (BATTLEGROUND_RATED_10_VS_10) as a gathered
 * 10-man raid, extending the fork's rated-arena team logic (BattleGroundJoinAction.cpp: PlayerbotArena census,
 * gatherArenaParty). AC has no rated BG. Default OFF: AiPlayerbot.RandomBotAutoJoinBGRatedBGCount = 0.
 *
 * The census is per faction (a rated BG match is one group of 10 per side). Config is cached by SeedAutoJoin on the
 * world thread (CheckBgQueue), so bot threads never touch the ConfigMgr.
 */
#ifndef _PLAYERBOT_BG_RATEDBG_H
#define _PLAYERBOT_BG_RATEDBG_H

#include "Common.h"

class Player;
struct GroupQueueInfo;

namespace PlayerbotRatedBg
{
    /// Rated BG filling is configured on (AiPlayerbot.RandomBotAutoJoinBGRatedBGCount > 0), as of the last census tick.
    bool Enabled();
    /// Group size per side: the template's MaxPlayersPerTeam (10), or 2 while `.debug ratedbg` testing is on.
    uint32 TeamSize();

    /// CheckBgQueue hooks (world thread), same order as PlayerbotArena::ResetCensus / CountQueued / SeedAutoJoin / LogCensus.
    void ResetCensus();
    void CountQueued(Player* player, bool isBot);
    void SeedAutoJoin();
    void LogCensus();

    /// Bot thread: may this bot lead a rated BG group now (eligibility + the per-faction quota)? On true it has
    /// booked TeamSize() bots of its faction against the quota (AC :258-265 pattern).
    bool CanLead(Player* bot);
    /// Bot thread: gather a same-faction raid of TeamSize() level-max random bots around `bot`, then queue it
    /// through WorldSession::HandleBattlemasterJoinRated. True when the leader ends up in the rated BG queue.
    bool GatherAndQueue(Player* bot);
    /// A real player is queued or inside a rated BG (as of the last census tick): bots keep their queue for them.
    bool PlayerWaiting();
    /// A bot still queued (not invited) after the wait limit (5 minutes; AC's arena rule is 120 s, :925-962) leaves the
    /// rated queue and, with the rest of an all-bot group, the group. False while a real player waits.
    bool LeaveQueueIfWaitedTooLong(Player* bot, GroupQueueInfo const& ginfo);
    /// After a rated BG match: a random bot whose group holds only random bots leaves it (the raid dissolves).
    void LeaveGroupAfterMatch(Player* bot);
}

#endif
