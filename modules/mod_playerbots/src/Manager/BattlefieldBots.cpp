/*
 * #900 random bots in outdoor battlefields. See BattlefieldBots.h.
 */
#include "BattlefieldBots.h"

#include "Battlefield.h"
#include "BattlefieldMgr.h"
#include "Config.h"
#include "DBCStructure.h"
#include "Group.h"
#include "Log.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "SharedDefines.h"
#include "World.h"
#include "BgTacticsTB.h"

#include <algorithm>
#include <ctime>
#include <map>
#include <set>

// #903: defined in BgTacticsTB.cpp; declared here, not in BgTacticsTB.h (StrategyContext.h / ActionContext.h include it)
namespace TolBaradBots
{
    void EntryPoint(Player* bot, Position& pos);
}

namespace
{
    // BATTLEFIELD_BATTLEID_WG / _TB (Battlefield.h:38-39)
    uint32 const BattleIds[] = { 1, 21 };
    uint32 const GroupingSeconds = 15 * 60;   // Battlefield::m_StartGroupingTimer (WG: 15 minutes)

    // Bots this module sent in (census), per battle id; "queued" = already called PlayerAcceptInviteToQueue.
    std::map<uint32, std::set<ObjectGuid>> sOurs;
    std::map<uint32, std::set<ObjectGuid>> sQueued;
    // Bots that called PlayerAcceptInviteToWar in the current war. A stale battlefield raid group left over from an earlier war
    // must not count as "accepted": the core then has the bot in no m_PlayersInWar and kicks it (#900 C1 Estevin/Adeleri).
    std::map<uint32, std::set<ObjectGuid>> sAccepted;
    std::map<ObjectGuid, time_t> sTeleportAt;   // census teleport retry throttle (one attempt per 20 s per bot)
    std::map<ObjectGuid, time_t> sSkipLogAt;    // diagnostic throttle

    uint32 MinLevel(uint32 battleId)
    {
        if (battleId == 1)
            return sWorld->getIntConfig(CONFIG_WINTERGRASP_PLR_MIN_LVL);   // Wintergrasp.PlayerMinLvl
        // Battlefield::m_MinLevel is protected; the Tol Barad lane may expose a getter or a World config instead.
        return uint32(sConfigMgr->GetIntDefault("TolBarad.PlayerMinLvl", 85));
    }

    // The bot is free to be pulled into a battlefield: not busy in a BG/arena/queue/LFG dungeon, not flying, and not in a
    // normal party (a party with a real player must never be broken up; a battlefield raid is fine).
    bool Free(Player* bot)
    {
        if (!bot->IsInWorld() || bot->IsBeingTeleported() || bot->IsInFlight())
            return false;
        if (bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue() || bot->inRandomLfgDungeon() || bot->GetInstanceId())
            return false;
        if (Group* group = bot->GetGroup())
            if (!group->isBFGroup())
                return false;
        return true;
    }

    void BfLog(char const* what, Player* bot, uint32 battleId, char const* extra = "")
    {
        TC_LOG_DEBUG("playerbots", "[BF] bot %u %s:%u <%s> %s battlefield %u %s", bot->GetGUID().GetCounter(),
            bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", bot->GetLevel(), bot->GetName().c_str(), what, battleId, extra);
    }
}

namespace BattlefieldBots
{
    bool Due()
    {
        static time_t last = 0;
        time_t const now = time(nullptr);
        if (now < last + 2)
            return false;
        last = now;
        return true;
    }

    void Update(std::vector<Player*> const& randomBots)
    {
        // Read every pass (quiet: an older playerbots.conf without the keys must not log "Missing name" each time)
        bool const census = sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotJoinBattlefield", 0, true) != 0;
        uint32 const cap = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotJoinBattlefieldCount", 40, true)));

        for (uint32 battleId : BattleIds)
        {
            Battlefield* bf = sBattlefieldMgr->GetBattlefieldByBattleId(battleId);
            if (!bf || !bf->IsEnabled())
                continue;

            bool const war = bf->IsWarTime();
            uint32 const timerSec = bf->GetTimer() / 1000;
            bool const upcoming = !war && timerSec <= GroupingSeconds;
            uint32 const minLevel = MinLevel(battleId);
            std::set<ObjectGuid>& ours = sOurs[battleId];
            std::set<ObjectGuid>& queued = sQueued[battleId];
            std::set<ObjectGuid>& accepted = sAccepted[battleId];
            if (!war)
                accepted.clear();

            // forget census bots that logged out; count the rest per team
            std::set<ObjectGuid> online;
            uint32 count[2] = { 0, 0 };
            // count = random bots of this battlefield per team: census bots + bots already in its war (cap is exact per faction)
            for (Player* bot : randomBots)
            {
                if (!bot)
                    continue;
                bool const isOurs = ours.count(bot->GetGUID()) != 0;
                if (isOurs)
                    online.insert(bot->GetGUID());
                Group* g = bot->GetGroup();
                bool const inThisWar = war && bot->IsInWorld() && bot->GetZoneId() == bf->GetZoneId() && g && g->isBFGroup();
                if (isOurs || inThisWar)
                    ++count[bot->GetTeamId() & 1];
            }
            ours = online;
            if (!war && !upcoming)
                queued.clear();   // new cycle: queue again when the window opens

            uint32 sent[2] = { 0, 0 };   // new census bots this pass (staggers the arrival)
            for (Player* bot : randomBots)
            {
                if (!bot || !bot->IsInWorld())
                    continue;

                if (battleId == 21 || battleId == 1)
                    TolBaradBots::Sync(bot);   // #900/#901: stay-and-fight strategy swap with the war (+ tolbarad on TB)

                // mitigation only: PlayerbotAI.cpp:2308 sets AFK on throttled (minimal) ticks and Battlefield::KickAfkPlayers (1 Hz) then kicks to
                // the KickPosition; a 2 s pass cannot win that race, the real fix is an exemption there (see #900).
                if (bot->isAFK() && TolBaradBots::InAnyWar(bot))
                    bot->ToggleAFK();

                ObjectGuid const guid = bot->GetGUID();
                bool const mine = ours.count(guid) != 0;
                bool const inZone = bot->GetZoneId() == bf->GetZoneId();
                Group* group = bot->GetGroup();
                bool const inWar = group && group->isBFGroup();
                TeamId const team = bot->GetTeamId();

                // stale battlefield raid group (its war is over, bot not census): drop it once, else the bot looks "in the war" forever
                if (battleId == BattleIds[0] && inWar && !mine)
                {
                    Battlefield* zbf = sBattlefieldMgr->GetBattlefieldToZoneId(bot->GetZoneId());
                    if (!zbf || !zbf->IsWarTime())
                    {
                        group->RemoveMember(guid);
                        continue;
                    }
                }

                // (3) battle over / not running: census bots leave the raid and go back to roaming. NOT while the queue window is
                // open ("upcoming"): that made every queued bot "leave" on the next pass and queue again (481 queue lines, 38 bots).
                if (mine && !war && !upcoming)
                {
                    if (inWar)
                        group->RemoveMember(guid);
                    ours.erase(guid);
                    queued.erase(guid);
                    accepted.erase(guid);
                    sTeleportAt.erase(guid);
                    BfLog("leave", bot, battleId, inZone ? "(battle ended, back to roaming)" : "(battle ended, never arrived)");
                    if (inZone)
                        sRandomPlayerbotMgr->ScheduleTeleport(guid.GetCounter(), 1);
                    continue;
                }

                if (!inZone)
                    accepted.erase(guid);   // left (or was kicked from) the zone: accept again on return

                bool free = Free(bot);
                // a census bot we sent in is ours to accept even if Free() trips over its battlefield raid group
                if (!free && mine && inZone && war && !bot->IsBeingTeleported())
                    free = true;
                if (bot->GetLevel() < minLevel || !free)
                {
                    if (mine && war && inZone)
                    {
                        time_t const now = time(nullptr);
                        time_t& t = sSkipLogAt[guid];
                        if (now - t >= 10)
                        {
                            t = now;
                            BfLog("skip", bot, battleId, bot->GetLevel() < minLevel ? "(level)" : "(not free: bg/queue/lfg/instance/teleport/flight)");
                        }
                    }
                    continue;
                }

                if (inZone)
                {
                    // (1) always: accept instead of being kicked; once per war and zone entry, whatever raid group the bot carries
                    if (war && !accepted.count(guid))
                    {
                        accepted.insert(guid);
                        bf->PlayerAcceptInviteToWar(bot);
                        BfLog("accept", bot, battleId, mine ? "(census)" : "(in zone)");
                        // keep the roaming teleport event from pulling the bot out mid-war (census and in-zone bots)
                        sRandomPlayerbotMgr->ScheduleTeleport(guid.GetCounter(), timerSec + 60);
                    }
                    else if (upcoming && !queued.count(guid))
                    {
                        bf->PlayerAcceptInviteToQueue(bot);
                        queued.insert(guid);
                        BfLog("queue", bot, battleId, "(in zone)");
                    }
                    continue;
                }

                // (2) opt-in census: not in the zone yet
                if (!census)
                    continue;

                if (mine)
                {
                    if (war && !bot->IsInCombat() && bot->IsAlive())   // war started: enter through a graveyard our team holds
                    {
                        time_t const now = time(nullptr);
                        time_t& t = sTeleportAt[guid];
                        if (now - t >= 20 && battleId == 21)
                        {
                            // #903: not the WorldSafeLocs spot - in Tol Barad (map 732, BATTLEFIELD_TB_MAPID) those are navmesh islands (BgTacticsTB.cpp EntryPoint)
                            t = now;
                            Position pos;
                            TolBaradBots::EntryPoint(bot, pos);
                            BfLog("teleport", bot, battleId, "(war started, walkable entry)");
                            bot->TeleportTo(732, pos.GetPositionX(), pos.GetPositionY(), pos.GetPositionZ(), pos.GetOrientation());
                        }
                        else if (now - t >= 20)
                            if (WorldSafeLocsEntry const* gy = bf->GetClosestGraveYard(bot))
                            {
                                t = now;
                                BfLog("teleport", bot, battleId, "(war started)");
                                bot->TeleportTo(gy->map_id, gy->x, gy->y, gy->z, gy->o);
                            }
                    }
                    continue;
                }

                if ((!war && !upcoming) || count[team & 1] >= cap || sent[team & 1] >= 5 || bot->IsInCombat() || !bot->IsAlive() || group)
                    continue;

                bf->PlayerAcceptInviteToQueue(bot);
                ours.insert(guid);
                queued.insert(guid);
                ++count[team & 1];
                ++sent[team & 1];
                BfLog("queue", bot, battleId, war ? "(census, war running)" : "(census, upcoming)");
                sRandomPlayerbotMgr->ScheduleTeleport(guid.GetCounter(), timerSec + 3600);   // do not roam away while queued
            }
        }
    }
}
