/*
 * #702 phase 5 / #832 (AC skirmish port + a level window beyond AC): random bots fill an arena SKIRMISH a real player
 * queued below the maximum level (core Arena.AnyLevel, src/server/game/Battlegrounds/ArenaAnyLevel.h).
 *
 * AC fills skirmishes with its skirmish census (azerothcore-wotlk@7f12e89e modules/mod-playerbots
 * src/Bot/RandomPlayerbotMgr.cpp:1005-1007 / :1037-1039 real players, :1092-1094 / :1112-1140 bots and their instances;
 * src/Ai/Base/Actions/BattleGroundJoinAction.cpp:272-291 shouldJoinBg's skirmish branch, :522-525 the bot's own booking,
 * :955-960 a waiting real player keeps a skirmish bot queued). SPP does the same and never queues a skirmish without a
 * real player (celguar/mangosbot-bots@b9263a55 playerbot/strategy/actions/BattleGroundJoinAction.cpp:430-452,
 * :774-784). Both only let a bot into the bracket of its own level (AC :200-223, SPP :320-351). This fork ported only
 * the rated half (#437, BattleGroundJoinAction.h RatedArenaInfo: "5.4.8 has no skirmish arenas"); the skirmish fields
 * live here so neither BattleGroundJoinAction.h (66 includers through ActionContext.h) nor RandomPlayerbotMgr.h /
 * PlayerbotAIConfig.h change.
 *
 * Beyond AC: a (queue, bracket) holding a real skirmish player takes the bots within AiPlayerbot.ArenaAnyLevel.LevelWindow
 * levels of that player (not capped to the 5-level bracket of the bot's own level; the bot is filed under the
 * requester's bracket), as long as the bot is not above that bracket's MaxLevel.
 *
 * Off unless AiPlayerbot.ArenaAnyLevel.Enable = 1 (read every AiPlayerbot.BgCheckInterval s by CheckBgQueue); off = the
 * census is never filled and every query below answers the stock value (Active() false, Wanted() false).
 * Header-only (RandomPlayerbotMgr.cpp and BattleGroundJoinAction.cpp include it). Thread model as BgAnyLevelRequests.h:
 * the world thread rebuilds the census (CheckBgQueue), bots read it and book themselves from map threads.
 */

#ifndef _PLAYERBOT_ARENAANYLEVELREQUESTS_H
#define _PLAYERBOT_ARENAANYLEVELREQUESTS_H

#include "Define.h"

#include <algorithm>
#include <atomic>
#include <cstdlib>
#include <mutex>
#include <utility>
#include <vector>

namespace ArenaAnyLevelBots
{
    uint32 const BRACKETS = 17;   // MAX_BATTLEGROUND_BRACKETS (DBCEnums.h:60), not included here to keep this header light
    uint32 const ARENA_KINDS = 3; // 2v2, 3v3, 5v5

    // arena type (2, 3, 5) -> 0, 1, 2; anything else -> ARENA_KINDS
    inline uint32 KindIndex(uint32 arenaType)
    {
        return arenaType == 2 ? 0 : arenaType == 3 ? 1 : arenaType == 5 ? 2 : ARENA_KINDS;
    }

    // AC BattlegroundInfo's skirmish fields (AC Bot/RandomPlayerbotMgr.h), one (arena type, bracket) cell.
    struct Cell
    {
        std::atomic<uint32> active{ 0 };         // activeSkirmishArenaQueue: a real player waits (not invited, not inside)
        std::atomic<uint32> playerCount{ 0 };    // skirmishArenaPlayerCount
        std::atomic<uint32> botCount{ 0 };       // skirmishArenaBotCount (bots book themselves here, AC :522-525)
        std::atomic<uint32> instanceCount{ 0 };  // skirmishArenaInstanceCount
    };

    struct Request
    {
        uint32 kind;      // KindIndex(arena type)
        uint32 bracketId; // the bracket the core filed the real player under
        uint8 level;      // the real player's level
    };

    struct Store
    {
        Cell cells[ARENA_KINDS][BRACKETS];
        std::atomic<bool> enabled{ false };
        std::atomic<bool> censusOn{ false };     // true while CheckBgQueue counts with the key on
        std::atomic<uint32> window{ 2 };
        std::atomic<uint32> requestCount{ 0 };
        std::atomic<uint32> bracketMask[ARENA_KINDS];         // (static storage: zero-initialised) bit per bracket holding a real skirmish player (lock-free fast path)
        std::mutex lock;                         // guards requests
        std::vector<Request> requests;
        // world thread only (CheckBgQueue -> PlayerbotArena::CountQueued)
        std::vector<Request> pending;
        std::vector<uint32> instances[ARENA_KINDS][BRACKETS];
    };

    inline Store& GetStore()
    {
        static Store store;
        return store;
    }

    inline Cell* GetCell(uint32 arenaType, uint32 bracketId)
    {
        uint32 const kind = KindIndex(arenaType);
        if (kind >= ARENA_KINDS || bracketId >= BRACKETS)
            return nullptr;
        return &GetStore().cells[kind][bracketId];
    }

    inline bool Enabled() { return GetStore().enabled.load(std::memory_order_relaxed); }
    inline bool CensusOn() { return GetStore().censusOn.load(std::memory_order_relaxed); }
    inline uint32 Window() { return GetStore().window.load(std::memory_order_relaxed); }

    // On, and at least one real player is in a skirmish: only then can a canJoinBg verdict differ from the stock one.
    inline bool Active()
    {
        Store& store = GetStore();
        return store.enabled.load(std::memory_order_relaxed) && store.requestCount.load(std::memory_order_relaxed) > 0;
    }

    // CheckBgQueue, world thread, before the census pass (AC :928-934 rebuilds BattlegroundData from scratch).
    inline void BeginCensus(bool enabled)
    {
        Store& store = GetStore();
        for (uint32 kind = 0; kind < ARENA_KINDS; ++kind)
            for (uint32 bracket = 0; bracket < BRACKETS; ++bracket)
            {
                Cell& cell = store.cells[kind][bracket];
                cell.active.store(0, std::memory_order_relaxed);
                cell.playerCount.store(0, std::memory_order_relaxed);
                cell.botCount.store(0, std::memory_order_relaxed);
                cell.instanceCount.store(0, std::memory_order_relaxed);
                store.instances[kind][bracket].clear();
            }
        store.pending.clear();
        store.censusOn.store(enabled, std::memory_order_relaxed);
    }

    // PlayerbotArena::CountQueued for an UNRATED arena queue slot (world thread): AC :1005-1007 / :1037-1039 (real
    // player: count, and a waiting one opens the cell) and :1092-1094 / :1112-1140 (bot: count, and its live instance).
    inline void CountSkirmish(uint32 arenaType, uint32 bracketId, bool isBot, bool waiting, uint32 instanceId, uint8 level)
    {
        Cell* cell = GetCell(arenaType, bracketId);
        if (!cell)
            return;

        Store& store = GetStore();
        if (isBot)
        {
            cell->botCount.fetch_add(1, std::memory_order_relaxed);
            if (instanceId)
            {
                std::vector<uint32>& ids = store.instances[KindIndex(arenaType)][bracketId];
                if (std::find(ids.begin(), ids.end(), instanceId) == ids.end())
                    ids.push_back(instanceId);
                cell->instanceCount.store(uint32(ids.size()), std::memory_order_relaxed);
            }
            return;
        }

        cell->playerCount.fetch_add(1, std::memory_order_relaxed);
        if (waiting)
            cell->active.store(1, std::memory_order_relaxed);
        store.pending.push_back({ KindIndex(arenaType), bracketId, level });
    }

    // CheckBgQueue, world thread, after the census pass.
    inline void Publish(bool enabled, uint32 window)
    {
        Store& store = GetStore();
        uint32 count = 0;
        uint32 masks[ARENA_KINDS] = {};
        {
            std::lock_guard<std::mutex> guard(store.lock);
            store.requests.swap(store.pending);
            store.pending.clear();
            count = enabled ? uint32(store.requests.size()) : 0;
            for (Request const& request : store.requests)
                if (enabled && request.kind < ARENA_KINDS && request.bracketId < BRACKETS)
                    masks[request.kind] |= (1u << request.bracketId);
        }
        for (uint32 kind = 0; kind < ARENA_KINDS; ++kind)
            store.bracketMask[kind].store(masks[kind], std::memory_order_relaxed);
        store.window.store(window, std::memory_order_relaxed);
        store.requestCount.store(count, std::memory_order_relaxed);
        store.enabled.store(enabled, std::memory_order_relaxed);
    }

    // BGJoinAction::canJoinBg for an arena queue while Active(). nativeOk = the stock rule (own level bracket),
    // windowOk = the core lets this bot into this bracket at all (access, level <= the bracket's MaxLevel),
    // ratedPlayerWaiting = a real player waits RATED in this (queue, bracket) (the #437 census).
    //  - no real skirmish player in this (arena type, bracket) -> the stock rule, unchanged;
    //  - otherwise the bot must be within the window of one of them (and windowOk), or it is a stock-rule bot and a
    //    rated real player waits there too (the rated fill is never narrowed).
    inline bool AcceptBot(uint32 arenaType, uint32 bracketId, uint32 botLevel, bool nativeOk, bool windowOk,
        bool ratedPlayerWaiting)
    {
        uint32 const kind = KindIndex(arenaType);
        if (kind >= ARENA_KINDS || bracketId >= BRACKETS)
            return nativeOk;

        Store& store = GetStore();
        // lock-free fast path: no real skirmish player in this (arena type, bracket) - isUseful asks every queue x bracket
        if (!(store.bracketMask[kind].load(std::memory_order_relaxed) & (1u << bracketId)))
            return nativeOk;

        int32 const window = int32(Window());
        bool any = false;
        bool inWindow = false;
        {
            std::lock_guard<std::mutex> guard(store.lock);
            for (Request const& request : store.requests)
            {
                if (request.kind != kind || request.bracketId != bracketId)
                    continue;
                any = true;
                if (std::abs(int32(botLevel) - int32(request.level)) <= window)
                    inWindow = true;
            }
        }

        if (!any)
            return nativeOk;

        return (inWindow && windowOk) || (nativeOk && ratedPlayerWaiting);
    }

    // AC shouldJoinBg skirmish branch (:272-291): room left for one more bot in this cell's skirmishes.
    inline bool Wanted(uint32 arenaType, uint32 bracketId)
    {
        if (!Enabled())
            return false;

        Cell* cell = GetCell(arenaType, bracketId);
        if (!cell)
            return false;

        uint32 const bracketSize = arenaType * 2;
        uint32 maxRequired = bracketSize * (cell->active.load(std::memory_order_relaxed) +
            cell->instanceCount.load(std::memory_order_relaxed));
        if (maxRequired)
            maxRequired += arenaType;  // AC: an extra team, same-faction skirmishes cannot share a group

        return (cell->botCount.load(std::memory_order_relaxed) + cell->playerCount.load(std::memory_order_relaxed)) <
            maxRequired;
    }

    // AC JoinQueue :522-525 (skirmishArenaBotCount++ when the bot queues).
    inline void Book(uint32 arenaType, uint32 bracketId)
    {
        if (Cell* cell = GetCell(arenaType, bracketId))
            cell->botCount.fetch_add(1, std::memory_order_relaxed);
    }

    // AC :955-960: real players in this cell's skirmishes.
    inline uint32 PlayerCount(uint32 arenaType, uint32 bracketId)
    {
        Cell* cell = GetCell(arenaType, bracketId);
        return cell ? cell->playerCount.load(std::memory_order_relaxed) : 0;
    }
}

#endif
