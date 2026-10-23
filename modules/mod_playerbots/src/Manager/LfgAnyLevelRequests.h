/*
 * #702 phase 1 (beyond AC): random bots fill an "any level" dungeon-finder queue at the requester's level.
 *
 * AC (Bot/RandomPlayerbotMgr.cpp:1287 CheckLfgQueue + Ai/Base/Actions/LfgActions.cpp:85-168 JoinLFG) and SPP
 * (celguar/mangosbot-bots@b9263a55 playerbot/RandomPlayerbotMgr.cpp:1378-1548 CheckLfgQueue,
 * playerbot/strategy/actions/LfgActions.cpp:830-860) both let a random bot queue only for the dungeons real players
 * are queued for, and only when the bot's level is inside the dungeon's own range. With LFG.AnyLevel (core) a real
 * player can queue a dungeon outside its range; this file carries, from CheckLfgQueue (world thread, every 30 s) to
 * LfgJoinAction (map threads), which dungeons were queued that way and at what level, so a bot within
 * AiPlayerbot.LfgAnyLevel.LevelWindow levels of that player is accepted for it instead.
 *
 * Off unless AiPlayerbot.LfgAnyLevel.Enable = 1; then the native rule is unchanged for every dungeon that has no
 * out-of-range requester. Header-only (included by RandomPlayerbotMgr.cpp and LfgActions.cpp only), so it adds no
 * source file and does not touch RandomPlayerbotMgr.h or PlayerbotAIConfig.h.
 *
 * #830 (#702 phase 3):
 *  - AiPlayerbot.LfgAnyLevel.ItemLevelWindow (0 = off): a bot taken through an out-of-range requester's level window must
 *    also have an average equipped item level (Player::GetAverageItemLevel, the dungeon finder's own item-level measure,
 *    LFGMgr.cpp InitializeLockedDungeons) within that many item levels of the requester's. AC's JoinLFG has only a
 *    commented-out gear pick (AC Ai/Base/Actions/LfgActions.cpp:92-99) and puts the gear score in the comment (:152); the BotFactory
 *    gear-score cap (#629/#698, BotFactory::CalcMixedGearScore) generates gear and is not used to re-gear a random bot
 *    on queue (item churn on a map thread, and AC never does it), so the window selects instead.
 *  - A premade group's request carries its highest-level member's level and item level (RandomPlayerbotMgr
 *    CheckLfgQueue): AutoBalance scales the instance to the highest player level, so that is the level the bots must match.
 */

#ifndef _PLAYERBOT_LFGANYLEVELREQUESTS_H
#define _PLAYERBOT_LFGANYLEVELREQUESTS_H

#include "Define.h"
#include "SharedDefines.h"

#include <atomic>
#include <cmath>
#include <cstdlib>
#include <mutex>
#include <utility>
#include <vector>

namespace LfgAnyLevel
{
    struct Request
    {
        TeamId team;
        uint32 dungeonId;
        uint8 level;          // the real player's level (#830: a premade group's highest member level)
        bool outOfRange;      // the player is outside the dungeon's own level range
        float itemLevel;      // #830: that player's average equipped item level
    };

    struct Store
    {
        std::mutex lock;
        std::vector<Request> requests;
        std::atomic<bool> enabled{ false };
        std::atomic<uint32> window{ 3 };
        std::atomic<uint32> itemLevelWindow{ 0 };   // #830: 0 = off
    };

    inline Store& GetStore()
    {
        static Store store;
        return store;
    }

    inline bool Enabled() { return GetStore().enabled.load(std::memory_order_relaxed); }
    inline uint32 Window() { return GetStore().window.load(std::memory_order_relaxed); }
    inline uint32 ItemLevelWindow() { return GetStore().itemLevelWindow.load(std::memory_order_relaxed); }

    // CheckLfgQueue (world thread)
    inline void Publish(bool enabled, uint32 window, std::vector<Request>&& requests, uint32 itemLevelWindow = 0)
    {
        Store& store = GetStore();
        {
            std::lock_guard<std::mutex> guard(store.lock);
            store.requests.swap(requests);
        }
        store.window.store(window, std::memory_order_relaxed);
        store.itemLevelWindow.store(itemLevelWindow, std::memory_order_relaxed);
        store.enabled.store(enabled, std::memory_order_relaxed);
    }

    // LfgJoinAction (map threads): this team's requests
    inline std::vector<Request> Snapshot(TeamId team)
    {
        Store& store = GetStore();
        std::vector<Request> out;
        std::lock_guard<std::mutex> guard(store.lock);
        for (Request const& request : store.requests)
            if (request.team == team)
                out.push_back(request);
        return out;
    }

    // Verdict for one dungeon: nativeOk is the stock level rule's answer for this bot.
    //  - no out-of-range requester for the dungeon -> the stock rule, unchanged;
    //  - otherwise the bot is accepted when it is within the window of an out-of-range requester, or when the stock rule
    //    accepts it AND an in-range real player also queued the dungeon.
    //  - #830: with AiPlayerbot.LfgAnyLevel.ItemLevelWindow > 0 the window also needs botItemLevel within that many item
    //    levels of the requester's (the in-range / native rule never looks at item level, as before).
    inline bool AcceptBot(std::vector<Request> const& requests, uint32 dungeonId, uint32 botLevel, bool nativeOk,
                          float botItemLevel = 0.0f)
    {
        bool anyOutOfRange = false;
        bool anyInRange = false;
        bool inWindow = false;
        int32 const window = int32(Window());
        float const itemLevelWindow = float(ItemLevelWindow());
        for (Request const& request : requests)
        {
            if (request.dungeonId != dungeonId)
                continue;

            if (!request.outOfRange)
            {
                anyInRange = true;
                continue;
            }

            anyOutOfRange = true;
            if (std::abs(int32(botLevel) - int32(request.level)) <= window &&
                (itemLevelWindow <= 0.0f || std::fabs(botItemLevel - request.itemLevel) <= itemLevelWindow))
                inWindow = true;
        }

        if (!anyOutOfRange)
            return nativeOk;

        return inWindow || (nativeOk && anyInRange);
    }
}

#endif
