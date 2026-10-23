/*
 * #702 phase 4 / #831 (beyond AC): random bots fill an "any level" battleground queue around the requester's level.
 *
 * AC (Bot/RandomPlayerbotMgr.cpp CheckBgQueue + Ai/Base/Actions/BattleGroundJoinAction.cpp:200 canJoinBg / :228
 * shouldJoinBg) and SPP (celguar/mangosbot-bots@b9263a55 playerbot/RandomPlayerbotMgr.cpp:1056-1220 CheckBgQueue,
 * playerbot/strategy/actions/BattleGroundJoinAction.cpp:320-351 canJoinBg) both let a random bot queue only for the
 * bracket of its own level, for the (queue, bracket) pairs a real player is waiting in, both teams. With
 * Battleground.AnyLevel (core, BattlegroundAnyLevel.h) a real player below every bracket of a map is filed under a
 * fallback bracket; this file carries, from CheckBgQueue (world thread, every AiPlayerbot.BgCheckInterval s) to
 * BGJoinAction (map threads), which (queue, bracket) pairs hold such a player and at what level, so
 * a bot within AiPlayerbot.BgAnyLevel.LevelWindow levels of that player is accepted for that bracket instead.
 *
 * Off unless AiPlayerbot.BgAnyLevel.Enable = 1; even then the native rule is unchanged for every (queue, bracket) that
 * has no below-range requester. Header-only (included by RandomPlayerbotMgr.cpp, BattleGroundJoinAction.cpp and
 * PvpTriggers.cpp only), so it adds no source file and touches neither RandomPlayerbotMgr.h nor PlayerbotAIConfig.h.
 * Same shape as LfgAnyLevelRequests.h (#702 P1).
 */

#ifndef _PLAYERBOT_BGANYLEVELREQUESTS_H
#define _PLAYERBOT_BGANYLEVELREQUESTS_H

#include "Define.h"

#include <atomic>
#include <cstdlib>
#include <mutex>
#include <utility>
#include <vector>

namespace BgAnyLevel
{
    struct Request
    {
        uint32 queueTypeId;
        uint32 bracketId;     // the bracket the core filed the real player under (GetAssignedBgBracket)
        uint8 level;          // the real player's level
        bool outOfRange;      // the player is below that bracket's MinLevel (the core's any-level fallback)
    };

    struct Store
    {
        std::mutex lock;
        std::vector<Request> requests;
        std::atomic<bool> enabled{ false };
        std::atomic<uint32> window{ 5 };
        std::atomic<uint32> outOfRangeCount{ 0 };
        std::atomic<uint64> outOfRangeQueues{ 0 };   // bit per queue type id (< 64) holding a below-range requester
    };

    inline Store& GetStore()
    {
        static Store store;
        return store;
    }

    inline bool Enabled() { return GetStore().enabled.load(std::memory_order_relaxed); }
    inline uint32 Window() { return GetStore().window.load(std::memory_order_relaxed); }

    // On, and at least one below-range real player is queued or playing: only then does any bot verdict differ from the
    // native one (the fast path every BGJoinAction::canJoinBg call takes otherwise, without the lock).
    inline bool Active()
    {
        Store& store = GetStore();
        return store.enabled.load(std::memory_order_relaxed) && store.outOfRangeCount.load(std::memory_order_relaxed) > 0;
    }

    // CheckBgQueue (world thread)
    inline void Publish(bool enabled, uint32 window, std::vector<Request>&& requests)
    {
        Store& store = GetStore();
        uint32 outOfRange = 0;
        uint64 queues = 0;
        for (Request const& request : requests)
        {
            if (!request.outOfRange)
                continue;
            ++outOfRange;
            queues |= request.queueTypeId < 64 ? (uint64(1) << request.queueTypeId) : ~uint64(0);
        }
        {
            std::lock_guard<std::mutex> guard(store.lock);
            store.requests.swap(requests);
        }
        store.window.store(window, std::memory_order_relaxed);
        store.outOfRangeQueues.store(enabled ? queues : 0, std::memory_order_relaxed);
        store.outOfRangeCount.store(enabled ? outOfRange : 0, std::memory_order_relaxed);
        store.enabled.store(enabled, std::memory_order_relaxed);
    }

    // Verdict for one (queue, bracket): nativeOk is the stock rule's answer for this bot (its own level bracket is this
    // bracket), windowOk whether the core would let this bot into this bracket at all (access + bracket max level).
    //  - no below-range requester in this (queue, bracket) -> the stock rule, unchanged;
    //  - otherwise the bot is accepted when it is within the window of a below-range requester (and windowOk), or when
    //    the stock rule accepts it AND an in-range real player waits in the same (queue, bracket) too.
    // Both teams fill (SPP RandomPlayerbotMgr.cpp:1208-1212 marks NeedBots for both teams of a battleground).
    inline bool AcceptBot(uint32 queueTypeId, uint32 bracketId, uint32 botLevel, bool nativeOk, bool windowOk)
    {
        Store& store = GetStore();

        // lock-free fast path: no below-range requester in this queue type at all (isUseful asks every queue x bracket)
        uint64 const queues = store.outOfRangeQueues.load(std::memory_order_relaxed);
        if (queueTypeId < 64 && !(queues & (uint64(1) << queueTypeId)))
            return nativeOk;

        bool anyOutOfRange = false;
        bool anyInRange = false;
        bool inWindow = false;
        int32 const window = int32(Window());
        {
            std::lock_guard<std::mutex> guard(store.lock);
            for (Request const& request : store.requests)
            {
                if (request.queueTypeId != queueTypeId || request.bracketId != bracketId)
                    continue;

                if (!request.outOfRange)
                {
                    anyInRange = true;
                    continue;
                }

                anyOutOfRange = true;
                if (std::abs(int32(botLevel) - int32(request.level)) <= window)
                    inWindow = true;
            }
        }

        if (!anyOutOfRange)
            return nativeOk;

        return (inWindow && windowOk) || (nativeOk && anyInRange);
    }
}

#endif
