/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "RandomPlayerbotMgr.h"

#include "Chat.h"  // #808: ChatHandler for the rndbot console command
#include "ObjectAccessor.h"

#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <chrono>
#include <iomanip>
#include <random>

// DatabaseEnv.h must precede BattlegroundQueue.h: the latter pulls in RatedPvp.h, which uses
// CharacterDatabaseTransaction without declaring it.
#include "DatabaseEnv.h"

#include "AccountMgr.h"
#include "Battleground.h"
#include "BattlegroundMgr.h"
#include "BattlegroundQueue.h"
#include "BattleGroundJoinAction.h"
#include "BgRatedBg.h"  // #897
#include "BotFactory.h"
#include "CharacterCache.h"
#include "CellImpl.h"
#include "ChannelMgr.h"
#include "Config.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Define.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "GuildMgr.h"
#include "Group.h"  // #830: CheckLfgQueue reads a queued premade's members
#include "Helper.h"
#include "LFGMgr.h"
#include "LfgAnyLevelRequests.h"  // #702: any-level LFG requests for LfgJoinAction (off by default)
#include "BgAnyLevelRequests.h"   // #702 P4 (#831): any-level battleground requests for BGJoinAction (off by default)
#include "ArenaAnyLevelRequests.h" // #702 P5 (#832): arena skirmish census + level window for BGJoinAction (off by default)
#include "BattlefieldBots.h"  // #900: random bots in outdoor battlefields (WG / Tol Barad)
#include "MapManager.h"
#include "MotionMaster.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "RandomPlayerbotFactory.h"
#include "PlayerbotRepository.h"
#include "RBAC.h"
#include "PerformanceMonitor.h"
#include "Playerbots.h"
#include "BotScenario.h"
#include "RandomItemManager.h"
#include "RandomPlayerbotBracketMgr.h"
#include "NewRpgDuelSpot.h"  // #201 (#615): sDuelSpotMgr seeder
#include "SharedDefines.h"
#include "TravelMgr.h"
#include "Unit.h"
#include "World.h"

#include <cctype>   // #148: RandomBotLevelMgr (std::isspace)
#include <cmath>    // #148: RandomBotLevelMgr (std::round, std::log)
#include <limits>   // #148: RandomBotLevelMgr (std::numeric_limits)
#include <mutex>
#include <sstream>  // #148: RandomBotLevelMgr (ParseLevelMgrExcludeNames)

namespace
{
// The random-bot event cache (_eventCache) is read and written by bot AI on map-update threads (RandomBotUpdateAction
// -> ProcessBot, TradeStatusAction, SuggestWhatToDoAction, Revive, duel spots) and by the world thread. With
// MapUpdate.Threads > 1 two maps can touch the std::map at the same time. AC (Bot/RandomPlayerbotMgr.cpp:2334
// FindEvent) has no lock here; this follows AC's pattern for shared state (std::mutex + lock_guard, e.g. AC
// WarlockActions.cpp:251). Recursive: GetEventData -> GetEventValue. File scope keeps RandomPlayerbotMgr.h (included
// by every module TU through Playerbots.h) unchanged.
std::recursive_mutex& EventCacheLock()
{
    static std::recursive_mutex lock;
    return lock;
}

// #532 (AC Bot/RandomPlayerbotMgr.cpp:1438,1485 read sPlayerbotAIConfig.min/maxRandomBotReviveTime;
// AC PlayerbotAIConfig.cpp defaults 60/300). Read here instead of adding to PlayerbotAIConfig.h.
uint32 RandomBotReviveTime()
{
    static uint32 const minTime = sConfigMgr->GetIntDefault("AiPlayerbot.MinRandomBotReviveTime", 60);
    static uint32 const maxTime = sConfigMgr->GetIntDefault("AiPlayerbot.MaxRandomBotReviveTime", 300);
    return urand(minTime, std::max(minTime, maxTime));
}
}  // namespace

RandomPlayerbotMgr::RandomPlayerbotMgr()
    : PlayerbotHolder(),
    _processTicks(0)
{
    _playersLevel = 1;// sPlayerbotAIConfig->randombotStartingLevel;
    _playersCheckTimer = 0;
}

RandomPlayerbotMgr::~RandomPlayerbotMgr() {}

void RandomPlayerbotMgr::Reserve(const uint32 size)
{
    _currentBots.reserve(size);
}

uint32 RandomPlayerbotMgr::GetMaxAllowedBotCount()
{
    return GetEventValue(0, "bot_count");
}

void RandomPlayerbotMgr::ScheduleRandomize(uint32 bot, uint32 time)
{
    SetEventValue(bot, "randomize", 1, time);
}

void RandomPlayerbotMgr::ScheduleTeleport(uint32 bot, uint32 time)
{
    if (!time)
        time = 60 + urand(sPlayerbotAIConfig->randomBotUpdateInterval, sPlayerbotAIConfig->randomBotUpdateInterval * 3);

    SetEventValue(bot, "teleport", 1, time);
}

void RandomPlayerbotMgr::UpdateAIInternal(uint32 elapsed, bool /*minimal*/)
{
    if (totalPmo)
        totalPmo->finish();

    totalPmo = sPerformanceMonitor->start(PERF_MON_TOTAL, "RandomPlayerbotMgr::FullTick");

    if (!sPlayerbotAIConfig->randomBotAutologin || !sPlayerbotAIConfig->enabled)
        return;

    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (!maxAllowedBotCount || (maxAllowedBotCount < sPlayerbotAIConfig->minRandomBots ||
        maxAllowedBotCount > sPlayerbotAIConfig->maxRandomBots))
    {
        maxAllowedBotCount = sPlayerbotAIConfig->minRandomBots + (std::rand() % (sPlayerbotAIConfig->maxRandomBots - sPlayerbotAIConfig->minRandomBots + 1));
        auto randomInterval = sPlayerbotAIConfig->randomBotCountChangeMinInterval +
            (std::rand() % (sPlayerbotAIConfig->randomBotCountChangeMaxInterval - sPlayerbotAIConfig->randomBotCountChangeMinInterval + 1));
        SetEventValue(0, "bot_count", maxAllowedBotCount, randomInterval);
    }

    // Refresh the battleground queue snapshot bots consult before queueing (issue #33).
    if (sPlayerbotAIConfig->randomBotJoinBG)
        CheckBgQueue();

    // #900: random bots in outdoor battlefields (accept invites; opt-in census). Self-throttled to 2 s.
    if (BattlefieldBots::Due())
    {
        std::vector<Player*> bfBots;
        for (auto const& itr : playerBots)
            if (itr.second && IsRandomBot(itr.second))
                bfBots.push_back(itr.second);
        BattlefieldBots::Update(bfBots);
    }

    // #142 (AC Bot/RandomPlayerbotMgr.cpp:390-394): refresh the LFG queue snapshot random bots consult (LfgJoinAction).
    if (sPlayerbotAIConfig->randomBotJoinLfg)
    {
        if (time(nullptr) > (_lfgCheckTimer + 30))
            CheckLfgQueue();
    }

    GetBots();
    std::vector<uint32> availableBots = _currentBots;
    uint32 availableBotCount = _currentBots.size();
    uint32 onlineBotCount = playerBots.size();

    uint32 onlineBotFocus = 75;
    if (onlineBotCount < (uint32)(sPlayerbotAIConfig->minRandomBots * 90 / 100))
        onlineBotFocus = 25;

    // only keep updating till initializing time has completed,
    // which prevents unneeded expensive GameTime calls.
    if (_isBotInitializing)
    {
        _isBotInitializing = sWorld->GetUptime() < sPlayerbotAIConfig->maxRandomBots * (0.11 + 0.4);
    }

    uint32 updateIntervalTurboBoost = _isBotInitializing ? 1 : sPlayerbotAIConfig->randomBotUpdateInterval;
    SetNextCheckDelay(updateIntervalTurboBoost * (onlineBotFocus + 25) * 10);

    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(
            PERF_MON_TOTAL,
            onlineBotCount < maxAllowedBotCount ? "RandomPlayerbotMgr::Login" : "RandomPlayerbotMgr::UpdateAIInternal");

    if (availableBotCount < maxAllowedBotCount)
    {
        AddRandomBots();
    }

    uint32 updateBots = sPlayerbotAIConfig->randomBotsPerInterval * onlineBotFocus / 100;
    uint32 maxNewBots = onlineBotCount < maxAllowedBotCount ? maxAllowedBotCount - onlineBotCount : 0;
    uint32 loginBots = std::min(sPlayerbotAIConfig->randomBotsPerInterval - updateBots, maxNewBots);

    if (!availableBots.empty())
    {
        // Update bots
        for (auto bot : availableBots)
        {
            if (!GetPlayerBot(bot))
                continue;

            if (ProcessBot(bot))
            {
                updateBots--;
            }

            if (!updateBots)
                break;
        }

        if (loginBots && botLoading.empty())
        {
            loginBots += updateBots;
            loginBots = std::min(loginBots, maxNewBots);

            TC_LOG_INFO("playerbots", "%d new bots", loginBots);

            // Log in bots
            for (auto bot : availableBots)
            {
                if (GetPlayerBot(bot))
                    continue;

                if (ProcessBot(bot))
                {
                    loginBots--;
                }

                if (!loginBots)
                    break;
            }
        }
    }

    // #201 (#615): duel-spot seeder (population tick). World thread, after the bot loop; no-op unless
    // DuelSpotEnable && DuelSpotSeedEnable.
    sDuelSpotMgr->Update(elapsed);

    if (pmo)
        pmo->finish();
}

// #772: port of AC AddRandomBots (AC Bot/RandomPlayerbotMgr.cpp:659-840). The old loop walked randomBotAccounts in fixed
// order and shuffled only inside one account, so after the #746 boot-time 'add' wipe the same first ~80 accounts were
// always the ones online and a ghost left on a later account was never logged in again. AC instead pools the characters
// of every eligible account, shuffles once and logs in Alliance up to the faction ratio, then Horde, then tops up with
// Alliance (phases 1-3; phase 4 is the shortage error).
// MoP deviations from AC (API differences only):
//  - AC's account list is rndBotTypeAccounts (AssignAccountTypes, :494); this fork has no account types, so the
//    eligible accounts are randomBotAccounts (every prefix account RandomPlayerbotFactory loaded).
//  - AC's CHAR_SEL_CHARS_BY_ACCOUNT_ID returns guid, race, class; MoP's returns only guid (core statement, left
//    unchanged), so one module-local query fetches guid, race, class for every selected account at once.
//  - _currentBots is a std::vector<uint32> here (AC: a set), so membership is std::find.
//  - Phase 4 logs this fork's existing "Not enough random bot accounts" text (AC's CalculateAvailableCharsPerAccount
//    does not exist here), with AC's 10 s anti-spam timer.
uint32 RandomPlayerbotMgr::AddRandomBots()
{
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    static time_t missingBotsTimer = 0;

    if (_currentBots.size() < maxAllowedBotCount)
    {
        // Calculate how many bots to add
        maxAllowedBotCount -= _currentBots.size();
        maxAllowedBotCount = std::min(sPlayerbotAIConfig->randomBotsPerInterval, maxAllowedBotCount);

        // Single RNG instance for all shuffling
        std::mt19937 rng(std::chrono::steady_clock::now().time_since_epoch().count());

        // Only need to track the Alliance count, as it's in Phase 1
        // (AC has no zero guard; both ratios 0 would divide by zero, so fall back to AC's 50/50 default.)
        uint32 allianceRatio = sPlayerbotAIConfig->randomBotAllianceRatio;
        uint32 totalRatio = allianceRatio + sPlayerbotAIConfig->randomBotHordeRatio;
        if (!totalRatio)
        {
            allianceRatio = 50;
            totalRatio = 100;
        }
        uint32 allowedAllianceCount = maxAllowedBotCount * allianceRatio / totalRatio;

        uint32 remainder = maxAllowedBotCount * allianceRatio % totalRatio;

        // Fix #1082: Randomly add one based on reminder
        if (remainder && urand(1, totalRatio) <= remainder)
        {
            allowedAllianceCount++;
        }

        // Determine which accounts to use based on EnablePeriodicOnlineOffline
        std::vector<uint32> accountsToUse;
        if (sPlayerbotAIConfig->enablePeriodicOnlineOffline)
        {
            // Calculate how many accounts can be used
            // With enablePeriodicOnlineOffline, don't use all of the accounts right away. Fraction results are rounded up
            uint32 accountsToUseCount = (rndBotTypeAccounts.size() + sPlayerbotAIConfig->periodicOnlineOfflineRatio - 1)
                                        / sPlayerbotAIConfig->periodicOnlineOfflineRatio;

            // Randomly select accounts
            std::vector<uint32> shuffledAccounts = rndBotTypeAccounts;
            std::shuffle(shuffledAccounts.begin(), shuffledAccounts.end(), rng);

            for (uint32 i = 0; i < accountsToUseCount && i < shuffledAccounts.size(); i++)
            {
                accountsToUse.push_back(shuffledAccounts[i]);
            }
        }
        else
        {
            accountsToUse = rndBotTypeAccounts;   // the random bot accounts (type 1), not the addclass ones
        }

        // Pre-map all characters from selected accounts
        struct CharacterInfo
        {
            uint32 guid;
            uint8 rClass;
            uint8 rRace;
            uint32 accountId;
        };
        std::vector<CharacterInfo> allCharacters;

        if (!accountsToUse.empty())
        {
            std::string accountList;
            for (uint32 accountId : accountsToUse)
            {
                if (!accountList.empty())
                    accountList += ',';
                accountList += std::to_string(accountId);
            }

            QueryResult result = CharacterDatabase.PQuery("SELECT guid, race, class, account FROM characters WHERE account IN (%s)", accountList.c_str());
            if (result)
            {
                do
                {
                    Field* fields = result->Fetch();
                    CharacterInfo info;
                    info.guid = fields[0].GetUInt32();
                    info.rRace = fields[1].GetUInt8();
                    info.rClass = fields[2].GetUInt8();
                    info.accountId = fields[3].GetUInt32();
                    allCharacters.push_back(info);
                } while (result->NextRow());
            }
        }

        // Shuffle for class balance
        std::shuffle(allCharacters.begin(), allCharacters.end(), rng);

        // Separate characters by faction for phased login
        std::vector<CharacterInfo> allianceChars;
        std::vector<CharacterInfo> hordeChars;

        for (auto const& charInfo : allCharacters)
        {
            if (IsAlliance(charInfo.rRace))
                allianceChars.push_back(charInfo);

            else
                hordeChars.push_back(charInfo);
        }

        // Lambda to handle bot login logic
        auto tryLoginBot = [&](CharacterInfo const& charInfo) -> bool
        {
            if (GetEventValue(charInfo.guid, "add") ||
                GetEventValue(charInfo.guid, "logout") ||
                GetPlayerBot(charInfo.guid) ||
                std::find(_currentBots.begin(), _currentBots.end(), charInfo.guid) != _currentBots.end() ||
                (sPlayerbotAIConfig->disableDeathKnightLogin && charInfo.rClass == CLASS_DEATH_KNIGHT))
            {
                return false;
            }

            uint32 add_time = sPlayerbotAIConfig->enablePeriodicOnlineOffline
                                ? urand(sPlayerbotAIConfig->minRandomBotInWorldTime,
                                        sPlayerbotAIConfig->maxRandomBotInWorldTime)
                                : sPlayerbotAIConfig->permanentlyInWorldTime;

            SetEventValue(charInfo.guid, "add", 1, add_time);
            SetEventValue(charInfo.guid, "logout", 0, 0);
            // #714 (AC Bot/RandomPlayerbotMgr.cpp:768-775): AC sets only "add" and "logout" here. "randomize" and
            // "teleport" stay unset, so ProcessBot schedules the first randomize/teleport shortly after login and then
            // every Min..MaxRandomBotRandomizeTime / Min..MaxRandomBotTeleportInterval.
            _currentBots.push_back(charInfo.guid);

            return true;
        };

        // PHASE 1: Log-in Alliance bots up to allowedAllianceCount
        for (auto const& charInfo : allianceChars)
        {
            if (!allowedAllianceCount)
                break;

            if (tryLoginBot(charInfo))
            {
                maxAllowedBotCount--;
                allowedAllianceCount--;
            }
        }

        // PHASE 2: Log-in Horde bots up to maxAllowedBotCount
        for (auto const& charInfo : hordeChars)
        {
            if (!maxAllowedBotCount)
                break;

            if (tryLoginBot(charInfo))
                maxAllowedBotCount--;
        }

        // PHASE 3: If maxAllowedBotCount wasn't reached, log-in more Alliance bots
        for (auto const& charInfo : allianceChars)
        {
            if (!maxAllowedBotCount)
                break;

            if (tryLoginBot(charInfo))
                maxAllowedBotCount--;
        }

        // PHASE 4: An error is given if maxAllowedBotCount is still not reached
        if (maxAllowedBotCount)
        {
            if (missingBotsTimer == 0)
                missingBotsTimer = time(nullptr);

            if (time(nullptr) - missingBotsTimer >= 10)
            {
                TC_LOG_ERROR("playerbots", "Not enough random bot accounts available. Try to increase RandomBotAccountCount in your conf file");
                missingBotsTimer = 0;    // Reset timer so error is not spammed every tick
            }
        }
        else
        {
            missingBotsTimer = 0;       // Reset timer if logins for this interval were successful
        }
    }
    else
    {
        missingBotsTimer = 0;           // Reset timer if there's enough bots
    }

    return _currentBots.size();
}

void RandomPlayerbotMgr::Remove(Player* bot)
{
    ObjectGuid owner = bot->GetGUID();

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER);
    stmt->setUInt32(0, 0);
    stmt->setUInt32(1, owner.GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    {
        std::lock_guard<std::recursive_mutex> guard(EventCacheLock());
        _eventCache[owner.GetCounter()].clear();
    }

    LogoutPlayerBot(owner);
}

void RandomPlayerbotMgr::CheckPlayers()
{
}

bool RandomPlayerbotMgr::ProcessBot(uint32 bot)
{
    Player* player = GetPlayerBot(bot);
    PlayerbotAI* botAI = player ? GET_PLAYERBOT_AI(player) : nullptr;
    ObjectGuid botGUID = ObjectGuid::Create<HighGuid::Player>(bot);

    uint32 isValid = GetEventValue(bot, "add");
    if (!isValid)
    {
        if (!player || !player->GetGroup())
        {
            if (player)
            {
                auto side = (player->GetTeamId() == TeamId::TEAM_ALLIANCE ? "A" : "H");
                TC_LOG_INFO("playerbots", "Bot #%u %s:%u <%s>: log out", bot, side, player->GetLevel(), player->GetName().c_str());
            }
            else
                TC_LOG_INFO("playerbots", "Bot #%u: log out", bot);

            SetEventValue(bot, "add", 0, 0);
            _currentBots.erase(std::remove(_currentBots.begin(), _currentBots.end(), bot), _currentBots.end());

            if (player)
                LogoutPlayerBot(botGUID);
        }

        return false;
    }

    uint32 randomTime;
    if (!player)
    {
        AddPlayerBot(botGUID, 0);
        randomTime = 1 + (std::rand() % 2);

        uint32 randomBotUpdateInterval = _isBotInitializing ? 1 : sPlayerbotAIConfig->randomBotUpdateInterval;
        int minValue = std::max(5, static_cast<int>(randomBotUpdateInterval * 0.5));
        int maxValue = std::max(12, static_cast<int>(randomBotUpdateInterval * 2));
        randomTime = minValue + (std::rand() % (maxValue - minValue + 1));
        
        SetEventValue(bot, "update", 1, randomTime);

        // do not randomize or teleport immediately after server start (prevent lagging)
        if (!GetEventValue(bot, "randomize"))
        {
            // #532 (AC Bot/RandomPlayerbotMgr.cpp:1390-1393): the first randomize comes before the first
            // teleport - urand(3, max(4, interval * 0.4)), not the teleport window below.
            randomTime = urand(3, std::max(4, static_cast<int>(randomBotUpdateInterval * 0.4)));

            ScheduleRandomize(bot, randomTime);
        }
        if (!GetEventValue(bot, "teleport"))
        {
            int minValue = std::max(7, static_cast<int>(randomBotUpdateInterval * 0.7));
            int maxValue = std::max(14, static_cast<int>(randomBotUpdateInterval * 1.4));
            randomTime = minValue + (std::rand() % (maxValue - minValue + 1));

            ScheduleTeleport(bot, randomTime);
        }

        return true;
    }

    if (!player->IsInWorld())
        return false;

    if (player->GetGroup() || player->HasUnitState(UNIT_STATE_IN_FLIGHT))
        return false;

    uint32 update = GetEventValue(bot, "update");
    if (!update)
    {
        if (botAI)
            botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);

        bool update = true;
        if (botAI)
        {
            botAI->GetAiObjectContext()->GetValue<bool>("random bot update")->Set(true);
            if (!sRandomPlayerbotMgr->IsRandomBot(player))
                update = false;

            if (player->GetGroup() && botAI->GetGroupMaster())
            {
                PlayerbotAI* groupMasterBotAI = GET_PLAYERBOT_AI(botAI->GetGroupMaster());
                if (!groupMasterBotAI || groupMasterBotAI->IsRealPlayer())
                {
                    update = false;
                }
            }
        }

        if (update)
            ProcessBot(player);

        randomTime = RandomBotReviveTime() /* #532, AC :1438/:1485 */;
        SetEventValue(bot, "update", 1, randomTime);

        return true;
    }

    /*uint32 logout = GetEventValue(bot, "logout");
    if (player && !logout && !isValid)
    {
        auto side = (player->GetTeamId() == TeamId::TEAM_ALLIANCE ? "A" : "H");
        TC_LOG_INFO("playerbots", "Bot #%u %s:%u <%s>: log out", bot, side, player->GetLevel(), player->GetName().c_str());
        LogoutPlayerBot(botGUID);
        for (auto it = _currentBots.begin(); it != _currentBots.end();)
        {
            if (*it == bot)
            {
                it = _currentBots.erase(it);
                break;
            }
            else
                ++it;
        }
        SetEventValue(bot, "logout", 1, urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime));
        return true;
    }*/

    return false;
}

bool RandomPlayerbotMgr::ProcessBot(Player* player)
{
    ObjectGuid bot = player->GetGUID();

    if (player->InBattleground())
        return false;

    if (player->InBattlegroundQueue())
        return false;
    // Test harness: a bot running a .botscenario is left alone by the random-bot manager (no random
    // teleport, randomize or group changes) until the scenario ends, so a 1:1 run is never moved mid-stage.
    // WantsTrail, not IsStatusLocked: free-mode wander does not lock the status.
    if (BotScenario::WantsTrail(player))
        return false;

    

    // if death revive
    if (player->isDead())
    {
        if (!GetEventValue(bot, "dead"))
        {
            uint32 randomTime = RandomBotReviveTime() /* #532, AC :1438/:1485 */;
            TC_LOG_DEBUG("playerbots", "Mark bot %s as dead, will be revived in %us.", player->GetName().c_str(), randomTime);
            SetEventValue(bot, "dead", 1, sPlayerbotAIConfig->maxRandomBotInWorldTime);
            SetEventValue(bot, "revive", 1, randomTime);
            return false;
        }

        if (!GetEventValue(bot, "revive"))
        {
            Revive(player);
            return true;
        }

        return false;
    }

    // only randomize and teleport idle bots
    bool idleBot = false;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (botAI)
    {
        /*if (TravelTarget* target = botAI->GetAiObjectContext()->GetValue<TravelTarget*>("travel target")->Get())
        {
            if (target->getTravelState() == TravelState::TRAVEL_STATE_IDLE)
            {
                idleBot = true;
            }
        }
        else*/
        //{
            idleBot = true;
        //}
    }
    if (idleBot)
    {
        if (player->GetInstanceId() || player->GetGroup() || player->InArena() || player->inRandomLfgDungeon() || player->IsInFlight() || player->IsInCombat() || !player->IsAlive())
            return false;

        // issue #96: a bot can arrive in an enemy capital through paths that never consult the city
        // table - a quest POI, the travel graph / boat, a taxi destination, a corpse run. Send it back
        // instead of letting it settle there. This log line is the regression signal.
        if (IsInEnemyCapital(player))
        {
            char const* capital = CapitalZoneName(player->GetZoneId());
            TC_LOG_DEBUG("playerbots", "[CAPITAL] %s (team %u) is inside enemy capital zone %u (%s) - relocating",
                         player->GetName().c_str(), uint32(player->GetTeam()), player->GetZoneId(),
                         capital ? capital : "?");
            RandomTeleportForLevel(player);
            return false;
        }

        // randomize
        uint32 randomize = GetEventValue(bot, "randomize");
        if (!randomize)
        {
            PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "Randomize");
            Randomize(player);
            TC_LOG_DEBUG("playerbots", "Bot #%u %s:%u <%s>: randomized", bot.GetCounter(), player->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", player->GetLevel(), player->GetName().c_str());
            // #714 (AC Bot/RandomPlayerbotMgr.cpp:1559-1561): the next periodic randomize is scheduled here, after
            // Randomize(), from the AC config window (AiPlayerbot.Min/MaxRandomBotRandomizeTime, 2 h - 14 days).
            uint32 const randomTime =
                urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
            ScheduleRandomize(bot.GetCounter(), randomTime);
            if (pmo)
                pmo->finish();
            return true;
        }

        uint32 teleport = GetEventValue(bot, "teleport");
        if (!teleport)
        {
            PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomTeleportByLocations");
            TC_LOG_DEBUG("playerbots", "Bot #%u <%s>: teleport for level and refresh", bot.GetCounter(), player->GetName().c_str());
            Refresh(player);
            RandomTeleportForLevel(player);
            uint32 time = urand(sPlayerbotAIConfig->minRandomBotTeleportInterval, sPlayerbotAIConfig->maxRandomBotTeleportInterval);
            ScheduleTeleport(bot, time);
            if (pmo)
                pmo->finish();
            return true;
        }
    }

    return false;
}

void RandomPlayerbotMgr::TagForRandomize(Player* bot, uint32 level)
{
    ObjectGuid::LowType guid = bot->GetGUID().GetCounter();

    SetValue(bot, "level", level);
    ScheduleRandomize(guid, 0);
}

namespace
{
// AC PlayerbotAIConfig.cpp:194. Read here rather than added to PlayerbotAIConfig.h, which every
// module TU includes. The default is AC's, so a config without the key keeps the 0.25 this call
// site used to hard-code.
float ProbTeleToBankers()
{
    static float const value = sConfigMgr->GetFloatDefault("AiPlayerbot.ProbTeleToBankers", 0.25f);
    return value;
}

// #219: AC PlayerbotAI::StarterLevelDistanceCheck(player, loc, fromStartUp = true) (Bot/PlayerbotAI.cpp:6728-6755), the
// only form AC's RandomTeleport uses. File-static here so PlayerbotAI.h does not change. A bot above level 16 passes;
// below, the destination must be on the race/class start map and within 500 yd (level <= 4), 2500 yd (<= 10) or
// 10000 yd (11-16) of the start position. AC getRace(true) (the native race) is GetRace() on 5.4.8.
bool StarterLevelDistanceCheck(Player* player, WorldLocation const& loc)
{
    if (player->GetLevel() > 16)
        return true;

    PlayerInfo const* pInfo = sObjectMgr->GetPlayerInfo(player->GetRace(), player->GetClass());
    if (!pInfo)
        return false;   // AC dereferences unchecked; no create info = no start position to measure from
    if (loc.GetMapId() != pInfo->mapId)
        return false;
    float const dis = loc.GetExactDist(pInfo->positionX, pInfo->positionY, pInfo->positionZ);

    float bound = 10000.0f;
    if (player->GetLevel() <= 4)
        bound = 500.0f;
    else if (player->GetLevel() <= 10)
        bound = 2500.0f;

    return dis <= bound;
}

// AC RandomPlayerbotMgr::RandomTeleport(Player*, std::vector<WorldLocation>&, bool) (Bot/RandomPlayerbotMgr.cpp:1596-1738),
// file-static so RandomPlayerbotMgr.h does not change. Filters to randomBotMaps (drops battleground/arena maps -
// Player::TeleportTo silently refuses those outside an active BG, see death-parity-followup.md Gap 2), drops
// enemy-team zones and water/invalid ground, and only Resets/moves the bot once a destination has passed every
// check, so a destination that fails every filter no longer rearms the AI for nothing.
// #268 S9: `hearth` is AC's third parameter (AC Bot/RandomPlayerbotMgr.cpp:1596). It is what
// distinguishes the banker/city roll from the grind roll - the only difference between the two
// calls AC RandomTeleportForLevel makes - and it sets the bot's homebind to where it landed.
bool RandomTeleportToLocs(Player* bot, std::vector<WorldLocation> const& locs, bool hearth = false)
{
    if (bot->IsBeingTeleported() || !bot->IsInWorld())
        return false;
    if (bot->HasUnitState(UNIT_STATE_ROOT))                       // AC: bot->IsRooted()
        return false;
    if (bot->InBattlegroundQueue() || bot->InBattleground() || bot->InArena())
        return false;
    if (bot->GetGroup() && !bot->GetGroup()->IsLeader(bot->GetGUID()))
        return false;
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;
    if (bot->GetTransport() && bot->HasUnitState(UNIT_STATE_IGNORE_PATHFINDING) && botAI->HasPlayerNearby())
        return false;                                              // AC: MOVEMENTFLAG_ONTRANSPORT (no such flag on 5.4.8)

    std::vector<uint32> const& maps = sPlayerbotAIConfig->randomBotMaps;
    std::vector<WorldLocation> tlocs;
    for (WorldLocation const& loc : locs)
        if (std::find(maps.begin(), maps.end(), loc.GetMapId()) != maps.end())
            tlocs.push_back(loc);
    if (tlocs.empty())
    {
        TC_LOG_DEBUG("playerbots", "Cannot teleport bot %s - no locations available", bot->GetName().c_str());
        return false;
    }

    std::mt19937 rnd(urand(0, 0x7FFFFFFF));
    std::shuffle(tlocs.begin(), tlocs.end(), rnd);
    for (uint32 i = 0; i < tlocs.size(); ++i)
    {
        WorldLocation const& loc = tlocs[i];
        float x = loc.GetPositionX(), y = loc.GetPositionY(), z = loc.GetPositionZ();
        Map* map = sMapMgr->FindMap(loc.GetMapId(), 0);           // same call RandomTeleportForLevel used
        if (!map)
            continue;
        AreaTableEntry const* zone = sAreaTableStore.LookupEntry(map->GetZoneId(bot->GetPhaseMask(), x, y, z));
        AreaTableEntry const* area = sAreaTableStore.LookupEntry(map->GetAreaId(bot->GetPhaseMask(), x, y, z));
        if (!zone || !area)
            continue;
        if ((zone->team == 4 && bot->GetTeamId() == TEAM_ALLIANCE) || (zone->team == 2 && bot->GetTeamId() == TEAM_HORDE))
            continue;
        if (map->IsInWater(bot->GetPhaseMask(), x, y, z))
            continue;
        float const ground = map->GetHeight(bot->GetPhaseMask(), x, y, z + 0.5f);
        if (ground <= INVALID_HEIGHT)
            continue;
        z = 0.05f + ground;
        // #219: AC :1716 `if (!botAI->StarterLevelDistanceCheck(bot, loc, true)) continue;`. It was left out while every
        // MoP random bot was level 90; with random levels (owner 2026-10-14) a level 1-16 bot reaches this from the
        // level-up teleport (AutoMaintenanceOnLevelupAction::AutoTeleportForLevel), so it is ported (file-static below).
        if (!StarterLevelDistanceCheck(bot, loc))
            continue;

        TC_LOG_DEBUG("playerbots", "Random teleporting bot %s (level %u) to map %u zone %u area %u %.1f,%.1f,%.1f (%u/%u locations)",
            bot->GetName().c_str(), bot->GetLevel(), loc.GetMapId(), zone->ID, area->ID, x, y, z, i + 1, uint32(tlocs.size()));

        if (hearth)                                                // AC Bot/RandomPlayerbotMgr.cpp:1730
            bot->SetHomebind(loc, zone->ID);

        if (botAI->HasPlayerNearby(150.0f))                        // AC: prevent the blink being seen
            break;

        bot->GetMotionMaster()->Clear();
        botAI->Reset(true);
        bot->RemoveAurasWithInterruptFlags(
            SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP));
        bot->TeleportTo(loc.GetMapId(), x, y, z, 0.0f);
        bot->SendMovementFlagUpdate();
        return true;
    }
    return false;
}

// AC RandomPlayerbotMgr::RandomTeleportGrindForLevel (1900-1910) + TravelMgr::GetTeleportLocations (TravelMgr.cpp:4480-4488).
void RandomTeleportGrindForLevel(Player* bot)
{
    if (bot->InBattleground())
        return;
    // #268 S9: this open-copied the same branch; #268 S4 made it a real TravelMgr entry point,
    // so call it (AC TravelMgr.cpp:4480-4488 via RandomPlayerbotMgr::RandomTeleportGrindForLevel).
    std::vector<WorldLocation> const& locs = sTravelMgr.GetTeleportLocations(bot);
    TC_LOG_DEBUG("playerbots", "Random teleporting bot %s for level %u (%u locations available)",
        bot->GetName().c_str(), bot->GetLevel(), uint32(locs.size()));
    RandomTeleportToLocs(bot, locs);
}
}   // namespace

void RandomPlayerbotMgr::Revive(Player* player)
{
    uint64 bot = player->GetGUID();

    SetEventValue(bot, "dead", 0, 0);
    SetEventValue(bot, "revive", 0, 0);

    Refresh(player);

    // AC RandomPlayerbotMgr::Revive (Bot/RandomPlayerbotMgr.cpp:1584-1594): a revived random bot goes to a
    // level-appropriate grind/hub spot from TravelMgr, never a city or a farm-zone point that can land on a
    // battleground/arena map (death-parity-followup.md Gap 2 - GetFarmZoneForPlayer handed out BG/arena maps
    // that Player::TeleportTo silently refuses, leaving the bot stuck at the graveyard).
    RandomTeleportGrindForLevel(player);   // AC Revive (Bot/RandomPlayerbotMgr.cpp:1593)
}

void RandomPlayerbotMgr::Clear(Player* bot)
{
    //PlayerbotFactory factory(bot, bot->GetLevel());
    //factory.ClearEverything();
}

void RandomPlayerbotMgr::RandomizeFirst(Player* bot)
{
    uint32 maxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    maxLevel = std::min(maxLevel, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL));

    // if lvl sync is enabled, max level is limited by online players lvl
    /*if (sPlayerbotAIConfig->syncLevelWithPlayers)
        maxLevel = std::max(sPlayerbotAIConfig->randomBotMinLevel,
            std::min(playersLevel, sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL)));*/

    //PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "RandomizeFirst");

    // #808: AC Bot/RandomPlayerbotMgr.cpp:2016-2052 - DK bounds, downgrade branch, max/min chance roll.
    // AC randomBotMinLevelChance (PlayerbotAIConfig.cpp:179) has no config member here; read it in place.
    static float const minLevelChance = sConfigMgr->GetFloatDefault("AiPlayerbot.RandomBotMinLevelChance", 0.1f);
    uint32 minLevel = sPlayerbotAIConfig->randomBotMinLevel;
    if (bot->GetClass() == CLASS_DEATH_KNIGHT)
    {
        maxLevel = std::max(maxLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
        minLevel = std::max(minLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    }

    uint32 level;
    if (sPlayerbotAIConfig->downgradeMaxLevelBot && bot->GetLevel() >= sPlayerbotAIConfig->randomBotMaxLevel)
    {
        level = bot->GetClass() == CLASS_DEATH_KNIGHT ? sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)
                                                      : sPlayerbotAIConfig->randomBotMinLevel;
    }
    else
    {
        uint32 roll = urand(1, 100);
        if (roll <= 100 * sPlayerbotAIConfig->randomBotMaxLevelChance)
            level = maxLevel;
        else if (roll <= 100 * (sPlayerbotAIConfig->randomBotMaxLevelChance + minLevelChance))
            level = minLevel;
        else
            level = urand(minLevel, maxLevel);
    }

    if (sPlayerbotAIConfig->disableRandomLevels)
    {
        // issue #136: AC RandomizeFirst uses RandombotStartingLevel here (was a hard-coded 55).
        level = bot->GetClass() == CLASS_DEATH_KNIGHT
            ? std::max(sPlayerbotAIConfig->randombotStartingLevel, sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL))
            : sPlayerbotAIConfig->randombotStartingLevel;
    }

    SetValue(bot, "level", level);
    BotFactory factory(bot, level);
    factory.Randomize(false);

    uint32 randomTime   = urand(sPlayerbotAIConfig->minRandomBotRandomizeTime, sPlayerbotAIConfig->maxRandomBotRandomizeTime);
    uint32 inworldTime  = urand(sPlayerbotAIConfig->minRandomBotInWorldTime, sPlayerbotAIConfig->maxRandomBotInWorldTime);

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->setUInt32(0, randomTime);
    stmt->setString(1, "bot_delete");
    stmt->setUInt32(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_RANDOM_BOTS);
    stmt->setUInt32(0, inworldTime);
    stmt->setString(1, "logout");
    stmt->setUInt32(2, bot->GetGUID().GetCounter());
    PlayerbotsDatabase.Execute(stmt);

    // teleport to a random inn for bot level
    if (GET_PLAYERBOT_AI(bot))
        GET_PLAYERBOT_AI(bot)->Reset(true);

    if (bot->GetGroup())
        bot->RemoveFromGroup();

    //if (pmo)
        //pmo->finish();

    //for (int i = 1; i < MAX_INVTYPE; ++i)
    //{
        //sRandomItemMgr->FindBestItemForLevelAndEquip(bot, InventoryType(i));
    //}

    RandomTeleportForLevel(bot);
}

void RandomPlayerbotMgr::Randomize(Player* bot)
{
    // #808: 1:1 with AC RandomPlayerbotMgr::Randomize (Bot/RandomPlayerbotMgr.cpp:1956-1975) for every random bot,
    // whatever DisableRandomLevels says (the old "else" path below the branch - GetValue(bot, "level") +
    // factory.Randomize(false) + Refresh + teleport - has no AC counterpart).
    if (IsRandomBot(bot))
    {
        if (bot->InBattleground())   // AC :1958
            return;

        if (bot->GetLevel() < 3 || (bot->GetLevel() < 56 && bot->GetClass() == CLASS_DEATH_KNIGHT))
        {
            RandomizeFirst(bot);
        }
        else if (bot->GetLevel() < sPlayerbotAIConfig->randomBotMaxLevel || !sPlayerbotAIConfig->downgradeMaxLevelBot)
        {
            uint8 level = bot->GetLevel();
            BotFactory factory(bot, level);
            factory.Randomize(true);
            // IncreaseLevel(bot);   // commented out in AC (:1967)
        }
        else
        {
            RandomizeFirst(bot);
        }
        // #714: AC RandomPlayerbotMgr::Randomize (Bot/RandomPlayerbotMgr.cpp:1956-1975) schedules nothing; ProcessBot does.
        return;
    }

    // Legacy MoP path, kept for addclass bots only (PlayerbotMgr.cpp:843 - not random bots, so AC's Randomize never
    // sees them): the master-level Randomize(false) + Refresh + teleport.
    if (GET_PLAYERBOT_AI(bot))
        GET_PLAYERBOT_AI(bot)->Reset(true);

    uint8 level = GetValue(bot, "level");
    BotFactory factory(bot, level);
    factory.Randomize(false);

    // rez / clean cd / etc
    Refresh(bot);
    RandomTeleportForLevel(bot);

    // #714: no 62208000 s reschedule here (AC Randomize schedules nothing; ProcessBot does, AC :1559-1561).
}

// #808: port of AC RandomPlayerbotMgr::HandlePlayerbotConsoleCommand (Bot/RandomPlayerbotMgr.cpp:2479-2599), routed from
// cs_playerbots.cpp ".playerbots rndbot <args>" exactly as AC Script/PlayerbotCommandScript.cpp:67 does. A free function
// (declared extern in cs_playerbots.cpp) instead of AC's static member, so RandomPlayerbotMgr.h stays untouched; every
// method it dispatches to is public. Differences from AC, each marked below: "stats", "levelup"/"level", "grind" and
// "change_strategy" are commented out because PrintStats / IncreaseLevel / RandomTeleport(Player*) / ChangeStrategy
// do not exist on this fork; "reset" cannot clear the private _eventCache (AC :2503 does) - it says to restart anyway.
bool HandleRandomPlayerbotConsoleCommand(ChatHandler* /*handler*/, char const* args)
{
    if (!sPlayerbotAIConfig->enabled)
    {
        TC_LOG_ERROR("playerbots", "Playerbots system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        TC_LOG_ERROR("playerbots", "Usage: rndbot stats/update/reset/init/refresh/add/remove");
        return false;
    }

    std::string const cmd = args;

    if (cmd == "reset")
    {
        PlayerbotsDatabase.Execute(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS));
        // AC :2503 sRandomPlayerbotMgr.eventCache.clear(); _eventCache is private here and the restart below reloads it.
        TC_LOG_INFO("playerbots", "Random bots were reset for all players. Please restart the Server.");
        return true;
    }

    if (cmd == "stats")
    {
        // sRandomPlayerbotMgr->PrintStats();   // PrintStats does not exist on this fork
        // activatePrintStatsThread();
        return true;
    }

    if (cmd == "reload")
    {
        sPlayerbotAIConfig->Initialize();
        return true;
    }

    if (cmd == "update")
    {
        sRandomPlayerbotMgr->UpdateAIInternal(0);
        return true;
    }

    typedef void (RandomPlayerbotMgr::* ConsoleCommandHandler)(Player*);   // private in RandomPlayerbotMgr.h
    std::map<std::string, ConsoleCommandHandler> handlers;
    // handlers["initmin"] = &RandomPlayerbotMgr::RandomizeMin;
    handlers["init"] = &RandomPlayerbotMgr::RandomizeFirst;
    handlers["clear"] = &RandomPlayerbotMgr::Clear;
    // handlers["levelup"] = handlers["level"] = &RandomPlayerbotMgr::IncreaseLevel;   // IncreaseLevel does not exist on this fork
    handlers["refresh"] = &RandomPlayerbotMgr::Refresh;
    handlers["teleport"] = &RandomPlayerbotMgr::RandomTeleportForLevel;
    // handlers["rpg"] = &RandomPlayerbotMgr::RandomTeleportForRpg;
    handlers["revive"] = &RandomPlayerbotMgr::Revive;
    // handlers["grind"] = &RandomPlayerbotMgr::RandomTeleport;   // RandomTeleport(Player*) does not exist on this fork
    // handlers["change_strategy"] = &RandomPlayerbotMgr::ChangeStrategy;   // ChangeStrategy does not exist on this fork

    for (std::map<std::string, ConsoleCommandHandler>::iterator j = handlers.begin(); j != handlers.end(); ++j)
    {
        std::string const prefix = j->first;
        if (cmd.find(prefix) != 0)
            continue;

        std::string const name = cmd.size() > prefix.size() + 1 ? cmd.substr(1 + prefix.size()) : "%";

        std::vector<uint32> botIds;
        for (std::vector<uint32>::iterator i = sPlayerbotAIConfig->randomBotAccounts.begin();
             i != sPlayerbotAIConfig->randomBotAccounts.end(); ++i)
        {
            uint32 account = *i;
            if (QueryResult results = CharacterDatabase.PQuery(
                    "SELECT guid FROM characters WHERE account = %u AND name like '%s'", account, name.c_str()))
            {
                do
                {
                    Field* fields = results->Fetch();

                    uint32 botId = fields[0].GetUInt32();
                    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(botId);
                    if (!sRandomPlayerbotMgr->IsRandomBot(guid.GetCounter()))
                    {
                        continue;
                    }
                    Player* bot = ObjectAccessor::FindPlayer(guid);
                    if (!bot)
                        continue;

                    botIds.push_back(botId);
                } while (results->NextRow());
            }
        }

        if (botIds.empty())
        {
            TC_LOG_INFO("playerbots", "Nothing to do");
            return false;
        }

        uint32 processed = 0;
        for (std::vector<uint32>::iterator i = botIds.begin(); i != botIds.end(); ++i)
        {
            ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(*i);
            Player* bot = ObjectAccessor::FindPlayer(guid);
            if (!bot)
                continue;

            TC_LOG_INFO("playerbots", "[%u/%u] Processing command %s for bot %s", processed++, uint32(botIds.size()),
                        cmd.c_str(), bot->GetName().c_str());

            ConsoleCommandHandler handler = j->second;
            (sRandomPlayerbotMgr->*handler)(bot);
        }

        return true;
    }

    // std::vector<std::string> messages = sRandomPlayerbotMgr->HandlePlayerbotCommand(args);
    // for (std::vector<std::string>::iterator i = messages.begin(); i != messages.end(); ++i)
    // {
    //     TC_LOG_INFO("playerbots", "%s", i->c_str());
    // }
    return true;
}

void RandomPlayerbotMgr::Refresh(Player* bot)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return;

    if (bot->isDead())
    {
        bot->ResurrectPlayer(1.0f);
        bot->SpawnCorpseBones();
        botAI->ResetStrategies();
    }

    // if (sPlayerbotAIConfig->disableRandomLevels)
    //     return;

    if (bot->InBattleground())
        return;

    TC_LOG_DEBUG("playerbots", "Refreshing bot #%u <%s>", bot->GetGUID().GetCounter(), bot->GetName().c_str());
    PerformanceMonitorOperation* pmo = sPerformanceMonitor->start(PERF_MON_RNDBOT, "Refresh");

    botAI->Reset();
    bot->DurabilityRepairAll(false, 1.0f, false);
    bot->SetFullHealth();
    bot->Unit::SetPvP(sWorld->IsPvPRealm());  // #532, AC Bot/RandomPlayerbotMgr.cpp:2192
    BotFactory factory(bot, bot->GetLevel());
    factory.Refresh();

    if (bot->GetMaxPower(POWER_MANA) > 0)
        bot->SetPower(POWER_MANA, bot->GetMaxPower(POWER_MANA));

    if (bot->GetMaxPower(POWER_ENERGY) > 0)
        bot->SetPower(POWER_ENERGY, bot->GetMaxPower(POWER_ENERGY));

    uint32 money = bot->GetMoney();
    bot->SetMoney(money + 500 * sqrt(urand(1, bot->GetLevel() * 5)));

    //if (bot->GetGroup())
        //bot->RemoveFromGroup();

    if (pmo)
        pmo->finish();
}

void RandomPlayerbotMgr::AddCurrentBotForTest(uint32 guidLow)
{
    if (std::find(_currentBots.begin(), _currentBots.end(), guidLow) == _currentBots.end())
        _currentBots.push_back(guidLow);
}

bool RandomPlayerbotMgr::IsRandomBot(Player* bot)
{
    if (bot && GET_PLAYERBOT_AI(bot))
    {
        if (GET_PLAYERBOT_AI(bot)->IsRealPlayer())
        {
            return false;
        }
    }
    if (bot)
    {
        return IsRandomBot(bot->GetGUID().GetCounter());
    }

    return false;
}

bool RandomPlayerbotMgr::IsRandomBot(ObjectGuid::LowType bot)
{
    ObjectGuid guid = ObjectGuid::Create<HighGuid::Player>(bot);
    if (!sPlayerbotAIConfig->IsInRandomAccountList(sCharacterCache->GetCharacterAccountIdByGuid(guid)))
        return false;
    if (std::find(_currentBots.begin(), _currentBots.end(), bot) != _currentBots.end())
        return true;
    return false;
}

void RandomPlayerbotMgr::GetBots()
{
    if (!_currentBots.empty())
        return;

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->setUInt32(0, 0);
    stmt->setString(1, "add");
    uint32 maxAllowedBotCount = GetEventValue(0, "bot_count");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 bot = fields[0].GetUInt32();
            if (GetEventValue(bot, "add"))
                _currentBots.push_back(bot);

            if (_currentBots.size() >= maxAllowedBotCount)
                break;
        } while (result->NextRow());
    }
}

uint32 RandomPlayerbotMgr::GetEventValue(uint32 bot, std::string const event)
{
    std::lock_guard<std::recursive_mutex> guard(EventCacheLock());
    // load all events at once on first event load
    if (_eventCache[bot].empty())
    {
        PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_RANDOM_BOTS_BY_OWNER_AND_BOT);
        stmt->setUInt32(0, 0);
        stmt->setUInt64(1, bot);
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();
                std::string const eventName = fields[0].GetString();

                CachedEvent e;
                e.value = fields[1].GetUInt32();
                e.lastChangeTime = fields[2].GetUInt32();
                e.validIn = fields[3].GetUInt32();
                e.data = fields[4].GetString();
                _eventCache[bot][eventName] = std::move(e);
            } while (result->NextRow());
        }
    }

    CachedEvent& e = _eventCache[bot][event];
    if ((time(0) - e.lastChangeTime) >= e.validIn && event != "specNo" && event != "specLink")
        e.value = 0;

    return e.value;
}

std::string const RandomPlayerbotMgr::GetEventData(uint32 bot, std::string const event)
{
    std::string data = "";
    std::lock_guard<std::recursive_mutex> guard(EventCacheLock());
    if (GetEventValue(bot, event))
    {
        CachedEvent e = _eventCache[bot][event];
        data = e.data;
    }

    return data;
}

uint32 RandomPlayerbotMgr::SetEventValue(uint32 bot, std::string const event, uint32 value, uint32 validIn,
    std::string const data)
{
    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_RANDOM_BOTS_BY_OWNER_AND_EVENT);
    stmt->setUInt32(0, 0);
    stmt->setUInt64(1, bot);
    stmt->setString(2, event);
    trans->Append(stmt);

    if (value)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_RANDOM_BOTS);
        stmt->setUInt32(0, 0);
        stmt->setUInt64(1, bot);
        auto time = std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
        stmt->setInt64(2, time);
        stmt->setUInt32(3, validIn);
        stmt->setString(4, event);
        stmt->setUInt32(5, value);
        if (data != "")
        {
            stmt->setString(6, data);
        }
        else
        {
            stmt->setString(6, "");
        }
        trans->Append(stmt);
    }

    PlayerbotsDatabase.CommitTransaction(trans);

    CachedEvent e(value, (uint32)time(nullptr), validIn, data);
    std::lock_guard<std::recursive_mutex> guard(EventCacheLock());
    _eventCache[bot][event] = std::move(e);
    return value;
}

uint32 RandomPlayerbotMgr::GetValue(uint32 bot, std::string const type) { return GetEventValue(bot, type); }

uint32 RandomPlayerbotMgr::GetValue(Player* bot, std::string const type)
{
    return GetValue(bot->GetGUID().GetCounter(), type);
}

std::string const RandomPlayerbotMgr::GetData(uint32 bot, std::string const type) { return GetEventData(bot, type); }

void RandomPlayerbotMgr::SetValue(uint32 bot, std::string const type, uint32 value, std::string const data)
{
    SetEventValue(bot, type, value, sPlayerbotAIConfig->maxRandomBotInWorldTime, data);
}

void RandomPlayerbotMgr::SetValue(Player* bot, std::string const type, uint32 value, std::string const data)
{
    SetValue(bot->GetGUID().GetCounter(), type, value, data);
}

// =====================================================================================================================
// #148: random bot level management - port of AC Bot/RandomBotLevelMgr.cpp (the level brackets + level reset
// sub-features, AC :1-1171) with its config loader AC PlayerbotAIConfig::LoadRandomBotLevelConfig
// (PlayerbotAIConfig.cpp:67-79, :947-1098) and AC struct LevelBracketConfig (PlayerbotAIConfig.h:93-99).
// Kept in this TU so no widely included header changes (PlayerbotAIConfig.h / RandomPlayerbotMgr.h are pulled in by
// every module TU through Playerbots.h): the AC config members live in a file-static struct with AC's names, read
// with sConfigMgr like the other file-local keys here. Same AC keys, same AC defaults; both sub-features are OFF by
// default (AiPlayerbot.LevelBrackets.Enabled = 0, AiPlayerbot.ResetBotLevel.Enabled = 0).
// 5.4.8 differences (marked in place):
//  - AC's literal 80 (the WotLK level cap) in the ResetBotLevel validation and default is 90, the 5.4.8 cap.
//  - 5.4.8 has no arena teams (patch 5.4.0; see BattleGroundJoinAction.cpp canLeadArenaParty), so BotInArenaTeam is
//    always false and AiPlayerbot.LevelBrackets.IgnoreArenaTeamBots has nothing to exclude.
//  - AC's equipAndSpecPersistence branch in AdjustBotToRange is kept commented out: the fork has no equip/spec
//    persistence feature (BotFactory.cpp:837 note), which is what that branch works around.
//  - Hooks (AC registers a WorldScript and a PlayerScript, :1097-1171): login and logout are called from
//    RandomPlayerbotMgr::OnPlayerLogin / OnPlayerLogout below (both run for every player, the login one before the bot
//    AI exists, like AC's PLAYERHOOK_ON_LOGIN); the world update and the level-changed hook are the exported
//    RandomBotLevelMgrUpdate / RandomBotLevelMgrOnLevelChanged, called from mod_playerbots.cpp (PlayerbotsWorldScript::
//    OnUpdate, PlayerbotsPlayerScript::OnLevelChanged). AC's OnStartup load is done on the first call of any hook;
//    RandomBotLevelMgrReloadConfig is AC's OnAfterConfigLoad(reload = true).
// =====================================================================================================================
namespace
{
uint8 const RESET_BOT_LEVEL_CAP = 90;   // AC PlayerbotAIConfig.cpp:1032-1069 literal 80 = the WotLK cap; 5.4.8 = 90

// AC PlayerbotAIConfig.h:93-99
struct LevelBracketConfig
{
    uint8 lower = 1;
    uint8 upper = 80;
    uint8 pct = 0;
};

// AC PlayerbotAIConfig.h:514-545 (the level brackets + level reset members).
struct RandomBotLevelConfig
{
    bool levelBracketsEnabled = false;
    uint32 levelBracketsCheckFrequency = 300;
    uint32 levelBracketsFlaggedCheckFrequency = 15;
    uint32 levelBracketsFlaggedProcessLimit = 5;
    bool levelBracketsIgnoreGuildWithRealPlayers = true;
    bool levelBracketsIgnoreArenaTeamBots = true;
    bool levelBracketsIgnoreFriendListed = true;
    std::vector<std::string> levelBracketsExcludeNames;
    uint8 levelBracketsNumRanges = 9;
    std::vector<LevelBracketConfig> levelBracketsAlliance;
    std::vector<LevelBracketConfig> levelBracketsHorde;
    bool levelBracketsDynamicDistribution = false;
    float levelBracketsRealPlayerWeight = 1.0f;
    bool levelBracketsSyncFactions = false;

    bool resetBotLevelEnabled = false;
    uint8 resetBotLevelMaxLevel = RESET_BOT_LEVEL_CAP;
    uint8 resetBotLevelResetTo = 1;
    uint8 resetBotLevelSkipFrom = 0;
    uint8 resetBotLevelSkipTo = 1;
    uint8 resetBotLevelChance = 100;
    bool resetBotLevelScaledChance = false;
    bool resetBotLevelRestrictTimePlayed = false;
    uint32 resetBotLevelMinTimePlayed = 86400;
    uint32 resetBotLevelPlayedTimeCheckFrequency = 864;
    bool resetBotLevelIgnoreGuildWithRealPlayers = false;
    std::vector<std::string> resetBotLevelExcludeNames;
};

RandomBotLevelConfig& LevelCfg()
{
    static RandomBotLevelConfig cfg;
    return cfg;
}

// AC PlayerbotAIConfig.cpp:67-79 ParseLevelMgrExcludeNames.
void ParseLevelMgrExcludeNames(std::string const& csv, std::vector<std::string>& out)
{
    out.clear();
    std::istringstream f(csv);
    std::string s;
    while (getline(f, s, ','))
    {
        s.erase(std::remove_if(s.begin(), s.end(), [](unsigned char c) { return std::isspace(c); }), s.end());
        if (!s.empty())
            out.push_back(s);
    }
}

// AC PlayerbotAIConfig::LoadRandomBotLevelConfig (PlayerbotAIConfig.cpp:952-1098). AC GetOption<T> = the fork's
// Get*Default; uint32 keys read as int.
void LoadRandomBotLevelConfig()
{
    RandomBotLevelConfig& c = LevelCfg();

    // ---- Level brackets ----
    c.levelBracketsEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.Enabled", false);
    c.levelBracketsIgnoreGuildWithRealPlayers =
        sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.IgnoreGuildBotsWithRealPlayers", true);
    c.levelBracketsIgnoreArenaTeamBots = sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.IgnoreArenaTeamBots", true);

    c.levelBracketsCheckFrequency = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.CheckFrequency", 300));
    c.levelBracketsFlaggedCheckFrequency =
        uint32(sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.CheckFlaggedFrequency", 15));
    c.levelBracketsDynamicDistribution =
        sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.Dynamic.UseDynamicDistribution", false);
    c.levelBracketsRealPlayerWeight =
        sConfigMgr->GetFloatDefault("AiPlayerbot.LevelBrackets.Dynamic.RealPlayerWeight", 1.0f);
    c.levelBracketsSyncFactions = sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.Dynamic.SyncFactions", false);
    c.levelBracketsIgnoreFriendListed = sConfigMgr->GetBoolDefault("AiPlayerbot.LevelBrackets.IgnoreFriendListed", true);
    c.levelBracketsFlaggedProcessLimit =
        uint32(sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.FlaggedProcessLimit", 5));

    ParseLevelMgrExcludeNames(sConfigMgr->GetStringDefault("AiPlayerbot.LevelBrackets.ExcludeNames", ""),
        c.levelBracketsExcludeNames);

    c.levelBracketsNumRanges = static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.NumRanges", 9));
    c.levelBracketsAlliance.assign(c.levelBracketsNumRanges, LevelBracketConfig());
    c.levelBracketsHorde.assign(c.levelBracketsNumRanges, LevelBracketConfig());

    uint32 const randomBotMaxLevel = sPlayerbotAIConfig->randomBotMaxLevel;
    for (uint8 i = 0; i < c.levelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < c.levelBracketsNumRanges - 1 ? i * 10 + 9 : randomBotMaxLevel);
        c.levelBracketsAlliance[i].lower = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Lower", defaultLower));
        c.levelBracketsAlliance[i].upper = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Upper", defaultUpper));
        c.levelBracketsAlliance[i].pct = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Alliance.Range" + idx + ".Pct", 11));
    }

    for (uint8 i = 0; i < c.levelBracketsNumRanges; ++i)
    {
        std::string idx = std::to_string(i + 1);
        uint32 defaultLower = (i == 0 ? 1 : i * 10);
        uint32 defaultUpper = (i < c.levelBracketsNumRanges - 1 ? i * 10 + 9 : randomBotMaxLevel);
        c.levelBracketsHorde[i].lower = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Lower", defaultLower));
        c.levelBracketsHorde[i].upper = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Upper", defaultUpper));
        c.levelBracketsHorde[i].pct = static_cast<uint8>(
            sConfigMgr->GetIntDefault("AiPlayerbot.LevelBrackets.Horde.Range" + idx + ".Pct", 11));
    }

    // A mismatch forcibly disables SyncFactions and logs an error; it never brings the server down.
    if (c.levelBracketsSyncFactions)
    {
        for (uint8 i = 0; i < c.levelBracketsNumRanges; ++i)
        {
            if (c.levelBracketsAlliance[i].lower != c.levelBracketsHorde[i].lower ||
                c.levelBracketsAlliance[i].upper != c.levelBracketsHorde[i].upper)
            {
                TC_LOG_ERROR("server.loading",
                    "[RandomBotLevelMgr] Bracket mismatch detected between factions at index %u. Alliance: %u-%u, "
                    "Horde: %u-%u. SyncFactions requires both bracket count and min/max levels to match exactly; "
                    "forcibly disabling SyncFactions for this session. Check your configuration.",
                    uint32(i), uint32(c.levelBracketsAlliance[i].lower), uint32(c.levelBracketsAlliance[i].upper),
                    uint32(c.levelBracketsHorde[i].lower), uint32(c.levelBracketsHorde[i].upper));
                c.levelBracketsSyncFactions = false;
                break;
            }
        }
    }

    // ---- Level reset ----
    c.resetBotLevelEnabled = sConfigMgr->GetBoolDefault("AiPlayerbot.ResetBotLevel.Enabled", false);

    c.resetBotLevelMaxLevel =
        static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.MaxLevel", RESET_BOT_LEVEL_CAP));
    if ((c.resetBotLevelMaxLevel < 2 || c.resetBotLevelMaxLevel > RESET_BOT_LEVEL_CAP) && c.resetBotLevelMaxLevel != 0)
    {
        TC_LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.MaxLevel value: %u. Using default value %u.",
            uint32(c.resetBotLevelMaxLevel), uint32(RESET_BOT_LEVEL_CAP));
        c.resetBotLevelMaxLevel = RESET_BOT_LEVEL_CAP;
    }

    c.resetBotLevelResetTo = static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.ResetToLevel", 1));
    if (c.resetBotLevelResetTo < 1 || (c.resetBotLevelMaxLevel > 0 && c.resetBotLevelResetTo >= c.resetBotLevelMaxLevel))
    {
        TC_LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetToLevel value: %u. Using default value 1.",
            uint32(c.resetBotLevelResetTo));
        c.resetBotLevelResetTo = 1;
    }

    c.resetBotLevelSkipFrom = static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.SkipFromLevel", 0));
    if (c.resetBotLevelSkipFrom > RESET_BOT_LEVEL_CAP ||
        (c.resetBotLevelMaxLevel > 0 && c.resetBotLevelSkipFrom >= c.resetBotLevelMaxLevel))
    {
        TC_LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipFromLevel value: %u. Using default value 0 "
            "(disabled).",
            uint32(c.resetBotLevelSkipFrom));
        c.resetBotLevelSkipFrom = 0;
    }

    c.resetBotLevelSkipTo = static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.SkipToLevel", 1));
    if (c.resetBotLevelSkipTo < 1 || c.resetBotLevelSkipTo > RESET_BOT_LEVEL_CAP ||
        (c.resetBotLevelMaxLevel > 0 && c.resetBotLevelSkipTo > c.resetBotLevelMaxLevel))
    {
        TC_LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.SkipToLevel value: %u. Using default value 1.",
            uint32(c.resetBotLevelSkipTo));
        c.resetBotLevelSkipTo = 1;
    }

    c.resetBotLevelChance = static_cast<uint8>(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.ResetChance", 100));
    if (c.resetBotLevelChance > 100)
    {
        TC_LOG_ERROR("server.loading",
            "[RandomBotLevelMgr] Invalid AiPlayerbot.ResetBotLevel.ResetChance value: %u. Using default value 100.",
            uint32(c.resetBotLevelChance));
        c.resetBotLevelChance = 100;
    }

    c.resetBotLevelScaledChance = sConfigMgr->GetBoolDefault("AiPlayerbot.ResetBotLevel.ScaledChance", false);

    c.resetBotLevelRestrictTimePlayed = sConfigMgr->GetBoolDefault("AiPlayerbot.ResetBotLevel.RestrictTimePlayed", false);
    c.resetBotLevelMinTimePlayed = uint32(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.MinTimePlayed", 86400));
    c.resetBotLevelPlayedTimeCheckFrequency =
        uint32(sConfigMgr->GetIntDefault("AiPlayerbot.ResetBotLevel.PlayedTimeCheckFrequency", 864));

    c.resetBotLevelIgnoreGuildWithRealPlayers =
        sConfigMgr->GetBoolDefault("AiPlayerbot.ResetBotLevel.IgnoreGuildBotsWithRealPlayers", false);

    ParseLevelMgrExcludeNames(sConfigMgr->GetStringDefault("AiPlayerbot.ResetBotLevel.ExcludeNames", ""),
        c.resetBotLevelExcludeNames);
}

char const* FactionName(TeamId team) { return (team == TEAM_ALLIANCE) ? "Alliance" : "Horde"; }

// AC RandomBotLevelMgr.cpp:31-38
bool IsNameInExcludeList(Player* bot, std::vector<std::string> const& excludeList)
{
    if (!bot)
        return false;

    return std::find(excludeList.begin(), excludeList.end(), bot->GetName()) != excludeList.end();
}

// AC :40-49. AC IsLoggingOut() = isLogingOut() here.
bool BotInFriendList(Player* bot, std::vector<uint32> const& socialFriendsList)
{
    if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->isLogingOut() ||
        bot->IsDuringRemoveFromWorld())
        return false;

    return std::find(socialFriendsList.begin(), socialFriendsList.end(), bot->GetGUID().GetCounter()) !=
        socialFriendsList.end();
}

// AC :51-54
bool IsDisabledBracket(std::vector<LevelBracketConfig> const& configured, uint8 index)
{
    return index < configured.size() && configured[index].pct == 0;
}

// AC :56-67 asks sArenaTeamMgr for the bot's 2v2/3v3/5v5 team. 5.4.8 has no arena teams (patch 5.4.0, any party of the
// bracket size queues; BattleGroundJoinAction.cpp canLeadArenaParty), so no bot is ever in one.
bool BotInArenaTeam(Player* bot)
{
    if (!bot)
        return false;
    return false;
}

// AC :69-110. AC sLFGMgr->GetState(guid) = GetActiveState(guid) on 5.4.8 (queues per id, PlayerbotAI.cpp:1618).
bool IsBotSafeForLevelReset(Player* bot)
{
    if (!bot || !bot->GetSession() || bot->GetSession()->isLogingOut() || bot->IsDuringRemoveFromWorld())
        return false;

    if (!bot->IsInWorld())
        return false;

    if (!bot->IsAlive())
        return false;

    if (bot->IsInCombat())
        return false;

    if (bot->InBattleground() || bot->InArena() || bot->inRandomLfgDungeon() || bot->InBattlegroundQueue())
        return false;

    if (sLFGMgr->GetActiveState(bot->GetGUID()) != lfg::LFG_STATE_NONE)
        return false;

    if (Group* group = bot->GetGroup())
    {
        if (sLFGMgr->GetActiveState(group->GetGUID()) != lfg::LFG_STATE_NONE)
            return false;
    }

    if (bot->IsInFlight())
        return false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                return false;
        }
    }
    return true;
}

// AC RandomBotLevelMgr.h:23-96: owns the two sub-features' runtime working state (working bracket copies, the
// pending-reset queue, the timers). Config lives in LevelCfg() (AC: PlayerbotAIConfig).
class RandomBotLevelMgr
{
public:
    static RandomBotLevelMgr& instance()
    {
        static RandomBotLevelMgr instance;

        return instance;
    }

    void LoadConfig();
    void LogStartupSummary() const;
    void Update(uint32 diff);
    void OnBotLogin(Player* player);
    void OnBotLevelChanged(Player* player, uint8 oldLevel);
    void OnPlayerLogout(Player* player);

    // AC loads the config in PlayerbotAIConfig::Initialize and the working state in RandomBotLevelWorldScript::OnStartup
    // (:1106-1110); here both happen on the first hook call (the first one is the world update after startup).
    void EnsureLoaded()
    {
        std::call_once(_loadOnce, [this]()
        {
            LoadRandomBotLevelConfig();
            LoadConfig();
            LogStartupSummary();
        });
    }

    // Guards _pendingLevelResets: the logout hook can run off the world thread (a bot session's logout).
    std::recursive_mutex& Lock() { return _lock; }

private:
    RandomBotLevelMgr() = default;
    ~RandomBotLevelMgr() = default;

    RandomBotLevelMgr(RandomBotLevelMgr const&) = delete;
    RandomBotLevelMgr& operator=(RandomBotLevelMgr const&) = delete;

    RandomBotLevelMgr(RandomBotLevelMgr&&) = delete;
    RandomBotLevelMgr& operator=(RandomBotLevelMgr&&) = delete;

    // Resolved back to the live _allianceRanges/_hordeRanges vector at process time, since those
    // vectors can be resized on a config reload.
    struct PendingResetEntry
    {
        ObjectGuid botGuid;
        int targetRange;
        TeamId team;
    };

    // ---- Level brackets sub-feature ----
    std::vector<LevelBracketConfig>& GetFactionRanges(TeamId team);
    void ClampAndBalanceBrackets();
    void ApplyBracketWeights(std::vector<LevelBracketConfig>& ranges, std::vector<float> const& weights);
    int GetLevelRangeIndex(uint8 level, TeamId team);
    void AdjustBotToRange(Player* bot, int targetRangeIndex, TeamId team);
    void LoadSocialFriendList();
    int GetOrFlagPlayerBracket(Player* player);
    void RunLevelBracketsDistribution();
    void ProcessFactionDistribution(TeamId team, uint32 totalBots, std::vector<int>& actualCounts,
        std::vector<std::vector<Player*>>& botsByRange);
    void RedistributeSurplusBots(std::vector<Player*>& sourceBots, int fromRange, TeamId team,
        std::vector<int>& actualCounts, std::vector<int> const& desiredCounts, std::vector<int> const& targetRanges);
    void ProcessPendingLevelResets();

    // ---- Level reset sub-feature ----
    uint8 ComputeResetChance(uint8 level) const;
    void ResetBot(Player* player, uint8 currentLevel);
    void SkipBotLevel(Player* player, uint8 currentLevel);
    void RunResetPlayedTimeCheck();

    // Level brackets: working copies, since dynamic distribution and the clamp/rebalance pass
    // mutate percentages at runtime and must never write back into the config.
    std::vector<LevelBracketConfig> _allianceRanges;
    std::vector<LevelBracketConfig> _hordeRanges;
    uint8 _numRanges = 9;
    uint8 _randomBotMinLevel = 1;
    uint8 _randomBotMaxLevel = 80;
    std::vector<PendingResetEntry> _pendingLevelResets;
    std::vector<uint32> _socialFriendsList;

    uint32 _bracketsTimer = 0; // Level brackets: distribution adjustments
    uint32 _flaggedTimer = 0;  // Level brackets: pending reset checks
    uint32 _resetTimer = 0;    // Level reset: played-time based reset checks

    std::once_flag _loadOnce;
    std::recursive_mutex _lock;
};

// =============================================================================
// LEVEL BRACKETS FEATURE
// =============================================================================

std::vector<LevelBracketConfig>& RandomBotLevelMgr::GetFactionRanges(TeamId team)
{
    return (team == TEAM_ALLIANCE) ? _allianceRanges : _hordeRanges;
}

// AC :121-133
void RandomBotLevelMgr::LoadConfig()
{
    _allianceRanges = LevelCfg().levelBracketsAlliance;
    _hordeRanges = LevelCfg().levelBracketsHorde;
    _numRanges = LevelCfg().levelBracketsNumRanges;
    _randomBotMinLevel = static_cast<uint8>(sPlayerbotAIConfig->randomBotMinLevel);
    _randomBotMaxLevel = static_cast<uint8>(sPlayerbotAIConfig->randomBotMaxLevel);

    ClampAndBalanceBrackets();
}

// AC :135-175
void RandomBotLevelMgr::LogStartupSummary() const
{
    RandomBotLevelConfig const& c = LevelCfg();
    if (!c.levelBracketsEnabled)
        TC_LOG_INFO("playerbots", "[RandomBotLevelMgr] Level brackets sub-feature disabled via configuration.");
    else
    {
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] Level brackets loaded. Check frequency: %u seconds, flagged check frequency: %u "
            "seconds.",
            c.levelBracketsCheckFrequency, c.levelBracketsFlaggedCheckFrequency);
        for (uint8 i = 0; i < _numRanges; ++i)
            TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Alliance Range %u: %u-%u, Desired Percentage: %u%%", i + 1,
                uint32(_allianceRanges[i].lower), uint32(_allianceRanges[i].upper), uint32(_allianceRanges[i].pct));
        for (uint8 i = 0; i < _numRanges; ++i)
            TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Horde Range %u: %u-%u, Desired Percentage: %u%%", i + 1,
                uint32(_hordeRanges[i].lower), uint32(_hordeRanges[i].upper), uint32(_hordeRanges[i].pct));
    }

    if (!c.resetBotLevelEnabled)
        TC_LOG_INFO("playerbots", "[RandomBotLevelMgr] Level reset sub-feature disabled via configuration.");
    else
    {
        std::string const excluded = c.resetBotLevelExcludeNames.empty()
            ? std::string("None")
            : std::to_string(c.resetBotLevelExcludeNames.size()) + " names";
        TC_LOG_INFO("playerbots",
            "[RandomBotLevelMgr] Level reset loaded. MaxLevel = %u (%s), ResetToLevel = %u, SkipFromLevel = %u (%s), "
            "SkipToLevel = %u, ResetChance = %u%%, ScaledChance = %s, RestrictTimePlayed = %s, "
            "IgnoreGuildBotsWithRealPlayers = %s, ExcludedNames = %s.",
            uint32(c.resetBotLevelMaxLevel), c.resetBotLevelMaxLevel > 0 ? "Enabled" : "Disabled",
            uint32(c.resetBotLevelResetTo), uint32(c.resetBotLevelSkipFrom),
            c.resetBotLevelSkipFrom > 0 ? "Enabled" : "Disabled", uint32(c.resetBotLevelSkipTo),
            uint32(c.resetBotLevelChance), c.resetBotLevelScaledChance ? "Enabled" : "Disabled",
            c.resetBotLevelRestrictTimePlayed ? "Enabled" : "Disabled",
            c.resetBotLevelIgnoreGuildWithRealPlayers ? "Enabled" : "Disabled", excluded.c_str());
    }
}

// AC :177-242. Clamps bracket bounds to [_randomBotMinLevel, _randomBotMaxLevel] and rebalances the desired
// percentages (per faction) back to summing to 100, if they don't already.
void RandomBotLevelMgr::ClampAndBalanceBrackets()
{
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (_allianceRanges[i].lower < _randomBotMinLevel)
            _allianceRanges[i].lower = _randomBotMinLevel;
        if (_allianceRanges[i].upper > _randomBotMaxLevel)
            _allianceRanges[i].upper = _randomBotMaxLevel;
        if (_allianceRanges[i].lower > _allianceRanges[i].upper)
            _allianceRanges[i].pct = 0;
    }
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (_hordeRanges[i].lower < _randomBotMinLevel)
            _hordeRanges[i].lower = _randomBotMinLevel;
        if (_hordeRanges[i].upper > _randomBotMaxLevel)
            _hordeRanges[i].upper = _randomBotMaxLevel;
        if (_hordeRanges[i].lower > _hordeRanges[i].upper)
            _hordeRanges[i].pct = 0;
    }

    uint32 totalAlliance = 0;
    uint32 totalHorde = 0;
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        totalAlliance += _allianceRanges[i].pct;
        totalHorde += _hordeRanges[i].pct;
    }

    if (totalAlliance != 100 && totalAlliance > 0)
    {
        TC_LOG_TRACE("playerbots",
            "[RandomBotLevelMgr] Alliance: Sum of percentages is %u (expected 100). Auto adjusting.", totalAlliance);
        int missing = 100 - totalAlliance;
        while (missing > 0)
        {
            for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
            {
                if (_allianceRanges[i].lower <= _allianceRanges[i].upper && _allianceRanges[i].pct > 0)
                {
                    _allianceRanges[i].pct++;
                    missing--;
                }
            }
        }
    }
    if (totalHorde != 100 && totalHorde > 0)
    {
        TC_LOG_TRACE("playerbots", "[RandomBotLevelMgr] Horde: Sum of percentages is %u (expected 100). Auto adjusting.",
            totalHorde);
        int missing = 100 - totalHorde;
        while (missing > 0)
        {
            for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
            {
                if (_hordeRanges[i].lower <= _hordeRanges[i].upper && _hordeRanges[i].pct > 0)
                {
                    _hordeRanges[i].pct++;
                    missing--;
                }
            }
        }
    }
}

// AC :244-269
void RandomBotLevelMgr::ApplyBracketWeights(std::vector<LevelBracketConfig>& ranges, std::vector<float> const& weights)
{
    float total = 0.0f;
    for (uint8 i = 0; i < _numRanges; ++i)
        total += weights[i];

    int pctSum = 0;
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        uint8 pct = (total > 0.0f) ? static_cast<uint8>(std::round((weights[i] / total) * 100)) : 0;
        ranges[i].pct = pct;
        pctSum += pct;
    }
    // Fix rounding drift so sum = 100.
    int missing = 100 - pctSum;
    for (uint8 i = 0; i < _numRanges && missing > 0; ++i)
    {
        if (ranges[i].lower <= ranges[i].upper && ranges[i].pct > 0)
        {
            ranges[i].pct++;
            missing--;
        }
    }
}

// AC :271-288
int RandomBotLevelMgr::GetLevelRangeIndex(uint8 level, TeamId team)
{
    if (level < _randomBotMinLevel || level > _randomBotMaxLevel)
        return -1;

    if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
        return -1;

    std::vector<LevelBracketConfig> const& ranges = GetFactionRanges(team);
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        if (level >= ranges[i].lower && level <= ranges[i].upper)
            return i;
    }

    return -1;
}

// AC :290-364. Death Knights are never assigned below CONFIG_START_HEROIC_PLAYER_LEVEL.
void RandomBotLevelMgr::AdjustBotToRange(Player* bot, int targetRangeIndex, TeamId team)
{
    if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->isLogingOut() ||
        bot->IsDuringRemoveFromWorld())
        return;

    if (targetRangeIndex < 0 || targetRangeIndex >= _numRanges)
        return;

    std::vector<LevelBracketConfig> const& factionRanges = GetFactionRanges(team);
    if (static_cast<size_t>(targetRangeIndex) >= factionRanges.size())
        return;

    if (bot->IsMounted())
        bot->Dismount();

    uint8 botOriginalLevel = bot->GetLevel();
    uint8 newLevel = 0;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));

    if (bot->GetClass() == CLASS_DEATH_KNIGHT)
    {
        uint8 lowerBound = factionRanges[targetRangeIndex].lower;
        uint8 upperBound = factionRanges[targetRangeIndex].upper;
        if (upperBound < dkMinLevel)
        {
            TC_LOG_TRACE("playerbots",
                "[RandomBotLevelMgr] AdjustBotToRange: Cannot assign %s Death Knight '%s' (%u) to range %u-%u "
                "(below level %u).",
                FactionName(team), bot->GetName().c_str(), uint32(botOriginalLevel), uint32(lowerBound),
                uint32(upperBound), uint32(dkMinLevel));
            return;
        }
        if (lowerBound < dkMinLevel)
            lowerBound = dkMinLevel;
        if (lowerBound > upperBound)
            return;
        newLevel = urand(lowerBound, upperBound);
    }
    else
    {
        LevelBracketConfig const& range = factionRanges[targetRangeIndex];
        if (range.lower > range.upper)
        {
            TC_LOG_TRACE("playerbots", "[RandomBotLevelMgr] AdjustBotToRange: Invalid range %u-%u for %s bot '%s'.",
                uint32(range.lower), uint32(range.upper), FactionName(team), bot->GetName().c_str());
            return;
        }
        newLevel = urand(range.lower, range.upper);
    }

    BotFactory newFactory(bot, newLevel);
    newFactory.Randomize(false);

    // AC :349-356 force-resets talents when equipAndSpecPersistence is on and the bot rolled to max level. The fork
    // has no equip/spec persistence (BotFactory.cpp:837), the issue this works around, so it stays dormant:
    // if (newLevel == _randomBotMaxLevel && sPlayerbotAIConfig.equipAndSpecPersistence)
    // {
    //     PlayerbotFactory tempFactory(bot, newLevel);
    //     tempFactory.InitTalentsTree(false, true, true);
    // }

    TC_LOG_TRACE("playerbots",
        "[RandomBotLevelMgr] AdjustBotToRange: %s Bot '%s' - %s (%u) adjusted to level %u (target range %u-%u).",
        FactionName(team), bot->GetName().c_str(), ClassToString(Classes(bot->GetClass())).c_str(),
        uint32(botOriginalLevel), uint32(newLevel), uint32(factionRanges[targetRangeIndex].lower),
        uint32(factionRanges[targetRangeIndex].upper));
}

// AC :366-379. Loads the list of social friend low GUIDs (character_social, flags = 1) into _socialFriendsList.
void RandomBotLevelMgr::LoadSocialFriendList()
{
    _socialFriendsList.clear();
    QueryResult result = CharacterDatabase.Query("SELECT friend FROM character_social WHERE flags = 1");

    if (!result || result->GetRowCount() == 0)
        return;

    do
    {
        _socialFriendsList.push_back(result->Fetch()[0].GetUInt32());
    } while (result->NextRow());
}

// AC :381-476. Returns the bracket index for a player, flagging it for a pending level reset (to the closest
// bracket) if it currently falls outside every defined range for its faction. Only random bots are ever enqueued.
int RandomBotLevelMgr::GetOrFlagPlayerBracket(Player* player)
{
    bool isRandomBot = sRandomPlayerbotMgr->IsRandomBot(player);
    RandomBotLevelConfig const& c = LevelCfg();

    if (isRandomBot && IsNameInExcludeList(player, c.levelBracketsExcludeNames))
        return -1;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (isRandomBot && c.levelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return -1;

    if (isRandomBot && c.levelBracketsIgnoreArenaTeamBots && BotInArenaTeam(player))
        return -1;

    // Exclude bots grouped with a real player from bracket processing.
    if (isRandomBot)
    {
        if (Group* group = player->GetGroup())
        {
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                    return -1;
            }
        }
    }

    TeamId team = player->GetTeamId();
    int rangeIndex = GetLevelRangeIndex(player->GetLevel(), team);
    if (rangeIndex >= 0)
        return rangeIndex;

    if (team != TEAM_ALLIANCE && team != TEAM_HORDE)
        return -1;

    // Only random bots may be queued for a level reset below.
    if (!isRandomBot)
        return -1;

    std::vector<LevelBracketConfig> const& factionRanges = GetFactionRanges(team);

    int targetRange = -1;
    int smallestDiff = std::numeric_limits<int>::max();
    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    for (int i = 0; i < _numRanges; ++i)
    {
        if (factionRanges[i].lower > factionRanges[i].upper)
            continue;

        if (factionRanges[i].pct == 0)
            continue;

        // Skip brackets that Death Knights cannot be assigned to.
        if (player->GetClass() == CLASS_DEATH_KNIGHT && factionRanges[i].upper < dkMinLevel)
            continue;

        int diff = 0;
        if (player->GetLevel() < factionRanges[i].lower)
            diff = factionRanges[i].lower - player->GetLevel();
        else if (player->GetLevel() > factionRanges[i].upper)
            diff = player->GetLevel() - factionRanges[i].upper;
        if (diff < smallestDiff)
        {
            smallestDiff = diff;
            targetRange = i;
        }
    }

    if (targetRange >= 0)
    {
        bool alreadyFlagged = false;
        ObjectGuid guid = player->GetGUID();
        for (auto const& entry : _pendingLevelResets)
        {
            if (entry.botGuid == guid)
            {
                alreadyFlagged = true;
                break;
            }
        }
        if (!alreadyFlagged)
            _pendingLevelResets.push_back({guid, targetRange, team});
    }

    return -1;
}

// AC :478-514. Moves bots from an over-populated range into ranges that still need bots.
void RandomBotLevelMgr::RedistributeSurplusBots(std::vector<Player*>& sourceBots, int fromRange, TeamId team,
    std::vector<int>& actualCounts, std::vector<int> const& desiredCounts, std::vector<int> const& targetRanges)
{
    size_t targetIdx = 0;
    while (actualCounts[fromRange] > desiredCounts[fromRange] && !sourceBots.empty() && targetIdx < targetRanges.size())
    {
        Player* bot = sourceBots.back();
        sourceBots.pop_back();

        int targetRange = targetRanges[targetIdx];
        if (actualCounts[targetRange] >= desiredCounts[targetRange])
        {
            ++targetIdx;
            continue;
        }

        ObjectGuid botGuid = bot->GetGUID();
        bool alreadyFlagged = false;
        for (auto const& entry : _pendingLevelResets)
        {
            if (entry.botGuid == botGuid)
            {
                alreadyFlagged = true;
                break;
            }
        }
        if (!alreadyFlagged)
            _pendingLevelResets.push_back({botGuid, targetRange, team});

        actualCounts[fromRange]--;
        actualCounts[targetRange]++;
        if (actualCounts[targetRange] >= desiredCounts[targetRange])
            ++targetIdx;
    }
}

// AC :516-558. Desired-vs-actual counts for one faction; flags surplus bots (safe first, then flagged ones).
void RandomBotLevelMgr::ProcessFactionDistribution(TeamId team, uint32 totalBots, std::vector<int>& actualCounts,
    std::vector<std::vector<Player*>>& botsByRange)
{
    if (totalBots == 0)
        return;

    std::vector<LevelBracketConfig> const& ranges = GetFactionRanges(team);
    char const* factionName = FactionName(team);

    std::vector<int> desiredCounts(_numRanges, 0);
    for (uint8 i = 0; i < _numRanges; ++i)
    {
        desiredCounts[i] = static_cast<int>(std::round((ranges[i].pct / 100.0) * totalBots));
        TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] %s Range %u (%u-%u): Desired = %d, Actual = %d.",
            factionName, i + 1, uint32(ranges[i].lower), uint32(ranges[i].upper), desiredCounts[i], actualCounts[i]);
    }

    for (uint8 i = 0; i < _numRanges; ++i)
    {
        std::vector<Player*> safeBots;
        std::vector<Player*> flaggedBots;
        for (Player* bot : botsByRange[i])
        {
            if (IsBotSafeForLevelReset(bot))
                safeBots.push_back(bot);
            else
                flaggedBots.push_back(bot);
        }

        std::vector<int> targetRanges;
        for (uint8 j = 0; j < _numRanges; ++j)
        {
            if (actualCounts[j] < desiredCounts[j])
                targetRanges.push_back(j);
        }

        // #148 diagnostic (no AC counterpart): why a surplus range does not drain - safe vs unsafe bots in the range
        // and the pending-reset queue size, for the surplus ranges only.
        if (actualCounts[i] > desiredCounts[i])
            TC_LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] %s Range %u surplus %d: safe=%u unsafe=%u targets=%u pending=%u.",
                factionName, i + 1, actualCounts[i] - desiredCounts[i], uint32(safeBots.size()),
                uint32(flaggedBots.size()), uint32(targetRanges.size()), uint32(_pendingLevelResets.size()));

        RedistributeSurplusBots(safeBots, i, team, actualCounts, desiredCounts, targetRanges);
        RedistributeSurplusBots(flaggedBots, i, team, actualCounts, desiredCounts, targetRanges);
    }
}

// AC :560-708. The periodic distribution pass.
void RandomBotLevelMgr::RunLevelBracketsDistribution()
{
    RandomBotLevelConfig const& c = LevelCfg();
    auto const& allPlayers = ObjectAccessor::GetPlayers();

    LoadSocialFriendList();

    if (c.levelBracketsDynamicDistribution)
    {
        // Calculate real player bracket counts.
        std::vector<int> allianceRealCounts(_numRanges, 0);
        std::vector<int> hordeRealCounts(_numRanges, 0);
        uint32 totalAllianceReal = 0;
        uint32 totalHordeReal = 0;

        for (auto const& itr : allPlayers)
        {
            Player* player = itr.second;
            if (!player || !player->IsInWorld())
                continue;
            if (GET_PLAYERBOT_AI(player))
                continue; // Only count real players.
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex < 0)
                continue;
            if (player->GetTeamId() == TEAM_ALLIANCE)
            {
                allianceRealCounts[rangeIndex]++;
                totalAllianceReal++;
            }
            else if (player->GetTeamId() == TEAM_HORDE)
            {
                hordeRealCounts[rangeIndex]++;
                totalHordeReal++;
            }
        }

        float const baseline = 1.0f;
        std::vector<float> allianceWeights(_numRanges, 0.0f);
        std::vector<float> hordeWeights(_numRanges, 0.0f);

        // SYNCED MODE: real player weighting is combined for both factions, applied to both bracket tables.
        if (c.levelBracketsSyncFactions)
        {
            uint32 totalCombinedReal = totalAllianceReal + totalHordeReal;
            for (uint8 i = 0; i < _numRanges; ++i)
            {
                int combinedReal = allianceRealCounts[i] + hordeRealCounts[i];
                float weight = baseline + c.levelBracketsRealPlayerWeight *
                    (totalCombinedReal > 0 ? (1.0f / float(totalCombinedReal)) : 1.0f) * std::log(1 + combinedReal);

                allianceWeights[i] = IsDisabledBracket(c.levelBracketsAlliance, i) ? 0.0f : weight;
                hordeWeights[i] = IsDisabledBracket(c.levelBracketsHorde, i) ? 0.0f : weight;
            }
        }
        else
        {
            // Separate dynamic weighting for each faction.
            for (uint8 i = 0; i < _numRanges; ++i)
            {
                if (_allianceRanges[i].lower > _allianceRanges[i].upper || IsDisabledBracket(c.levelBracketsAlliance, i))
                    allianceWeights[i] = 0.0f;
                else
                    allianceWeights[i] = baseline + c.levelBracketsRealPlayerWeight *
                        (totalAllianceReal > 0 ? (1.0f / totalAllianceReal) : 1.0f) *
                        std::log(1 + allianceRealCounts[i]);

                if (_hordeRanges[i].lower > _hordeRanges[i].upper || IsDisabledBracket(c.levelBracketsHorde, i))
                    hordeWeights[i] = 0.0f;
                else
                    hordeWeights[i] = baseline + c.levelBracketsRealPlayerWeight *
                        (totalHordeReal > 0 ? (1.0f / totalHordeReal) : 1.0f) * std::log(1 + hordeRealCounts[i]);
            }
        }

        ApplyBracketWeights(_allianceRanges, allianceWeights);
        ApplyBracketWeights(_hordeRanges, hordeWeights);

        // Ensure brackets respect global min/max levels and percentages sum to 100.
        ClampAndBalanceBrackets();

        for (uint8 i = 0; i < _numRanges; ++i)
            TC_LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] Final Range %u: %u-%u, Alliance Desired: %u%%, Horde Desired: %u%%", i + 1,
                uint32(_allianceRanges[i].lower), uint32(_allianceRanges[i].upper), uint32(_allianceRanges[i].pct),
                uint32(_hordeRanges[i].pct));
    }

    uint32 totalAllianceBots = 0;
    std::vector<int> allianceActualCounts(_numRanges, 0);
    std::vector<std::vector<Player*>> allianceBotsByRange(_numRanges);

    uint32 totalHordeBots = 0;
    std::vector<int> hordeActualCounts(_numRanges, 0);
    std::vector<std::vector<Player*>> hordeBotsByRange(_numRanges);

    for (auto const& itr : allPlayers)
    {
        Player* player = itr.second;
        if (!player || !player->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr->IsRandomBot(player))
            continue;
        if (IsNameInExcludeList(player, c.levelBracketsExcludeNames))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (c.levelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
            continue;
        if (c.levelBracketsIgnoreFriendListed && BotInFriendList(player, _socialFriendsList))
            continue;
        if (c.levelBracketsIgnoreArenaTeamBots && BotInArenaTeam(player))
            continue;

        if (player->GetTeamId() == TEAM_ALLIANCE)
        {
            totalAllianceBots++;
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex >= 0)
            {
                allianceActualCounts[rangeIndex]++;
                allianceBotsByRange[rangeIndex].push_back(player);
            }
        }
        else if (player->GetTeamId() == TEAM_HORDE)
        {
            totalHordeBots++;
            int rangeIndex = GetOrFlagPlayerBracket(player);
            if (rangeIndex >= 0)
            {
                hordeActualCounts[rangeIndex]++;
                hordeBotsByRange[rangeIndex].push_back(player);
            }
        }
    }

    TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] Total Alliance Bots: %u. Total Horde Bots: %u.",
        totalAllianceBots, totalHordeBots);

    ProcessFactionDistribution(TEAM_ALLIANCE, totalAllianceBots, allianceActualCounts, allianceBotsByRange);
    ProcessFactionDistribution(TEAM_HORDE, totalHordeBots, hordeActualCounts, hordeBotsByRange);

    TC_LOG_DEBUG("playerbots",
        "[RandomBotLevelMgr] Distribution adjustment complete. Alliance bots: %u, Horde bots: %u.", totalAllianceBots,
        totalHordeBots);
}

// AC :710-800. Applies up to FlaggedProcessLimit pending resets per cycle (0 = unlimited).
void RandomBotLevelMgr::ProcessPendingLevelResets()
{
    if (_pendingLevelResets.empty())
        return;

    RandomBotLevelConfig const& c = LevelCfg();
    uint32 processed = 0;
    for (auto it = _pendingLevelResets.begin(); it != _pendingLevelResets.end();)
    {
        if (c.levelBracketsFlaggedProcessLimit > 0 && processed >= c.levelBracketsFlaggedProcessLimit)
            break;

        Player* bot = ObjectAccessor::FindPlayer(it->botGuid);

        if (!bot || !bot->IsInWorld() || !bot->GetSession() || bot->GetSession()->isLogingOut() ||
            bot->IsDuringRemoveFromWorld())
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        // Defensive: never resolve a queue entry to anything but a random bot.
        if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (IsNameInExcludeList(bot, c.levelBracketsExcludeNames))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        int targetRange = it->targetRange;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (c.levelBracketsIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (c.levelBracketsIgnoreFriendListed && BotInFriendList(bot, _socialFriendsList))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        if (c.levelBracketsIgnoreArenaTeamBots && BotInArenaTeam(bot))
        {
            it = _pendingLevelResets.erase(it);
            continue;
        }

        // Check if the bot is now grouped with a real player.
        if (Group* group = bot->GetGroup())
        {
            bool hasRealPlayer = false;
            for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
            {
                Player* member = ref->GetSource();
                if (member && member->IsInWorld() && !GET_PLAYERBOT_AI(member))
                {
                    hasRealPlayer = true;
                    break;
                }
            }
            if (hasRealPlayer)
            {
                it = _pendingLevelResets.erase(it);
                continue;
            }
        }

        if (IsBotSafeForLevelReset(bot))
        {
            AdjustBotToRange(bot, targetRange, it->team);
            it = _pendingLevelResets.erase(it);
            ++processed;
        }
        else
            ++it;
    }
}

// =============================================================================
// LEVEL RESET FEATURE
// =============================================================================

// AC :806-825
uint8 RandomBotLevelMgr::ComputeResetChance(uint8 level) const
{
    RandomBotLevelConfig const& c = LevelCfg();
    uint8 chance = c.resetBotLevelChance;
    if (c.resetBotLevelScaledChance)
    {
        chance = static_cast<uint8>((static_cast<float>(level) / c.resetBotLevelMaxLevel) * c.resetBotLevelChance);
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] ComputeResetChance: For level %u / %u with scaling, computed chance = %u%%",
            uint32(level), uint32(c.resetBotLevelMaxLevel), uint32(chance));
    }
    else
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] ComputeResetChance: For level %u / %u without scaling, chance = %u%%", uint32(level),
            uint32(c.resetBotLevelMaxLevel), uint32(chance));
    return chance;
}

// AC :827-848. Resets a bot to ResetToLevel (or the Death Knight starting level) via a full factory randomize.
void RandomBotLevelMgr::ResetBot(Player* player, uint8 currentLevel)
{
    uint8 levelToResetTo = LevelCfg().resetBotLevelResetTo;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    if (player->GetClass() == CLASS_DEATH_KNIGHT && levelToResetTo < dkMinLevel)
        levelToResetTo = dkMinLevel;

    // Dismount before randomization to prevent a wrong mount at the new level.
    if (player->IsMounted())
        player->Dismount();

    BotFactory newFactory(player, levelToResetTo);
    newFactory.Randomize(false);

    TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] ResetBot: Bot '%s' - %s at level %u was reset to level %u.",
        player->GetName().c_str(), ClassToString(Classes(player->GetClass())).c_str(), uint32(currentLevel),
        uint32(levelToResetTo));
}

// AC :850-871. Sends a bot straight to SkipToLevel (or the Death Knight starting level).
void RandomBotLevelMgr::SkipBotLevel(Player* player, uint8 currentLevel)
{
    uint8 levelToSkipTo = LevelCfg().resetBotLevelSkipTo;

    uint8 dkMinLevel = static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL));
    if (player->GetClass() == CLASS_DEATH_KNIGHT && levelToSkipTo < dkMinLevel)
        levelToSkipTo = dkMinLevel;

    // Dismount before randomization to prevent a wrong mount at the new level.
    if (player->IsMounted())
        player->Dismount();

    BotFactory newFactory(player, levelToSkipTo);
    newFactory.Randomize(false);

    TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] SkipBotLevel: Bot '%s' - %s at level %u was skipped to level %u.",
        player->GetName().c_str(), ClassToString(Classes(player->GetClass())).c_str(), uint32(currentLevel),
        uint32(levelToSkipTo));
}

// AC :873-923. Played-time based reset check for bots at or above MaxLevel.
void RandomBotLevelMgr::RunResetPlayedTimeCheck()
{
    RandomBotLevelConfig const& c = LevelCfg();
    TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] OnUpdate: Starting time-based reset check...");

    auto const& allPlayers = ObjectAccessor::GetPlayers();
    for (auto const& itr : allPlayers)
    {
        Player* candidate = itr.second;
        if (!candidate || !candidate->IsInWorld())
            continue;
        if (!sRandomPlayerbotMgr->IsRandomBot(candidate))
            continue;

        if (IsNameInExcludeList(candidate, c.resetBotLevelExcludeNames))
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(candidate);
        if (c.resetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
            continue;

        uint8 currentLevel = candidate->GetLevel();
        if (currentLevel < c.resetBotLevelMaxLevel)
            continue;

        // Only reset if the bot has played at least MinTimePlayed seconds at this level.
        if (candidate->GetLevelPlayedTime() < c.resetBotLevelMinTimePlayed)
        {
            TC_LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnUpdate: Bot '%s' at level %u has insufficient played time (%u < %u "
                "seconds).",
                candidate->GetName().c_str(), uint32(currentLevel), candidate->GetLevelPlayedTime(),
                c.resetBotLevelMinTimePlayed);
            continue;
        }

        uint8 resetChance = ComputeResetChance(currentLevel);
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnUpdate: Bot '%s' qualifies for time-based reset. Level: %u, "
            "LevelPlayedTime: %u seconds, computed reset chance: %u%%.",
            candidate->GetName().c_str(), uint32(currentLevel), candidate->GetLevelPlayedTime(), uint32(resetChance));
        if (urand(0, 99) < resetChance)
        {
            TC_LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnUpdate: Reset chance check passed for bot '%s'. Resetting bot.",
                candidate->GetName().c_str());
            ResetBot(candidate, currentLevel);
        }
    }
}

// =============================================================================
// SHARED UPDATE / HOOKS
// =============================================================================

// AC :929-959
void RandomBotLevelMgr::Update(uint32 diff)
{
    RandomBotLevelConfig const& c = LevelCfg();
    if (c.levelBracketsEnabled)
    {
        std::lock_guard<std::recursive_mutex> guard(_lock);
        _bracketsTimer += diff;
        _flaggedTimer += diff;

        if (_flaggedTimer >= c.levelBracketsFlaggedCheckFrequency * 1000)
        {
            ProcessPendingLevelResets();
            _flaggedTimer = 0;
        }

        if (_bracketsTimer >= c.levelBracketsCheckFrequency * 1000)
        {
            _bracketsTimer = 0;
            RunLevelBracketsDistribution();
        }
    }

    if (c.resetBotLevelEnabled && c.resetBotLevelRestrictTimePlayed && c.resetBotLevelMaxLevel > 0)
    {
        _resetTimer += diff;
        if (_resetTimer >= c.resetBotLevelPlayedTimeCheckFrequency * 1000)
        {
            _resetTimer = 0;
            RunResetPlayedTimeCheck();
        }
    }
}

// AC :961-1012
void RandomBotLevelMgr::OnBotLogin(Player* player)
{
    RandomBotLevelConfig const& c = LevelCfg();
    if (!sRandomPlayerbotMgr->IsRandomBot(player))
        return;

    if (IsNameInExcludeList(player, c.resetBotLevelExcludeNames))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (c.resetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return;

    uint8 currentLevel = player->GetLevel();

    if (c.resetBotLevelMaxLevel > 0)
    {
        // Bot is above MaxLevel: reset immediately.
        if (currentLevel > c.resetBotLevelMaxLevel)
        {
            TC_LOG_DEBUG("playerbots",
                "[RandomBotLevelMgr] OnPlayerLogin: Bot '%s' above max level %u. Resetting immediately.",
                player->GetName().c_str(), uint32(c.resetBotLevelMaxLevel));
            ResetBot(player, currentLevel);
            return;
        }

        // Bot is exactly at MaxLevel: apply the time-played restriction (if any) and chance.
        if (currentLevel == c.resetBotLevelMaxLevel)
        {
            if (!c.resetBotLevelRestrictTimePlayed || player->GetLevelPlayedTime() >= c.resetBotLevelMinTimePlayed)
            {
                uint8 resetChance = ComputeResetChance(currentLevel);
                if (urand(0, 99) < resetChance)
                {
                    TC_LOG_DEBUG("playerbots",
                        "[RandomBotLevelMgr] OnPlayerLogin: Bot '%s' meets reset criteria. Resetting.",
                        player->GetName().c_str());
                    ResetBot(player, currentLevel);
                }
            }
        }
    }

    if (c.resetBotLevelSkipFrom > 0 && currentLevel == c.resetBotLevelSkipFrom)
    {
        TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] OnPlayerLogin: Bot '%s' at skip level %u. Applying skip.",
            player->GetName().c_str(), uint32(currentLevel));
        SkipBotLevel(player, currentLevel);
    }
}

// AC :1014-1081
void RandomBotLevelMgr::OnBotLevelChanged(Player* player, uint8 oldLevel)
{
    RandomBotLevelConfig const& c = LevelCfg();
    if (!sRandomPlayerbotMgr->IsRandomBot(player))
        return;

    // Only react to a natural level up.
    if (player->GetLevel() != oldLevel + 1)
        return;

    if (IsNameInExcludeList(player, c.resetBotLevelExcludeNames))
        return;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
    if (c.resetBotLevelIgnoreGuildWithRealPlayers && botAI && botAI->IsInRealGuild())
        return;

    uint8 newLevel = player->GetLevel();

    if (newLevel == 1)
        return;

    if (player->GetClass() == CLASS_DEATH_KNIGHT &&
        newLevel == static_cast<uint8>(sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL)))
        return;

    // SkipFromLevel takes priority and is not affected by ScaledChance or RestrictTimePlayed.
    if (c.resetBotLevelSkipFrom > 0 && newLevel == c.resetBotLevelSkipFrom)
    {
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '%s' reached skip level %u. Skipping to level %u.",
            player->GetName().c_str(), uint32(newLevel), uint32(c.resetBotLevelSkipTo));
        SkipBotLevel(player, newLevel);
        return;
    }

    if (c.resetBotLevelMaxLevel == 0)
        return;

    // Strictly above MaxLevel: reset immediately regardless of time played.
    if (newLevel > c.resetBotLevelMaxLevel)
    {
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '%s' exceeded max level %u. Resetting immediately.",
            player->GetName().c_str(), uint32(c.resetBotLevelMaxLevel));
        ResetBot(player, newLevel);
        return;
    }

    // Exactly at MaxLevel with a time-played restriction: defer to the OnUpdate timer.
    if (c.resetBotLevelRestrictTimePlayed && newLevel == c.resetBotLevelMaxLevel)
    {
        TC_LOG_DEBUG("playerbots",
            "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '%s' at level %u deferred to OnUpdate due to "
            "time-played restriction.",
            player->GetName().c_str(), uint32(newLevel));
        return;
    }

    uint8 resetChance = ComputeResetChance(newLevel);
    if (c.resetBotLevelScaledChance || newLevel >= c.resetBotLevelMaxLevel)
    {
        TC_LOG_DEBUG("playerbots", "[RandomBotLevelMgr] OnPlayerLevelChanged: Bot '%s' at level %u has reset chance %u%%.",
            player->GetName().c_str(), uint32(newLevel), uint32(resetChance));
        if (urand(0, 99) < resetChance)
            ResetBot(player, newLevel);
    }
}

// AC :1083-1092. Drops the bot from the pending-reset queue (always empty when the brackets sub-feature is off).
void RandomBotLevelMgr::OnPlayerLogout(Player* player)
{
    std::lock_guard<std::recursive_mutex> guard(_lock);
    ObjectGuid guid = player->GetGUID();
    _pendingLevelResets.erase(
        std::remove_if(_pendingLevelResets.begin(), _pendingLevelResets.end(),
            [guid](PendingResetEntry const& entry) { return entry.botGuid == guid; }),
        _pendingLevelResets.end());
}
}  // namespace

// #148: AC RandomBotLevelWorldScript::OnUpdate (RandomBotLevelMgr.cpp:1126-1129). Exported (not a member) so
// RandomPlayerbotMgr.h does not change; called from mod_playerbots.cpp PlayerbotsWorldScript::OnUpdate (world thread).
void RandomBotLevelMgrUpdate(uint32 diff)
{
    RandomBotLevelMgr::instance().EnsureLoaded();
    RandomBotLevelMgr::instance().Update(diff);
}

// #148: AC RandomBotLevelPlayerScript::OnPlayerLevelChanged (:1151-1156); called from mod_playerbots.cpp
// PlayerbotsPlayerScript::OnLevelChanged (core ScriptMgr::OnPlayerLevelChanged, every player).
void RandomBotLevelMgrOnLevelChanged(Player* player, uint8 oldLevel)
{
    if (!player)
        return;
    RandomBotLevelMgr::instance().EnsureLoaded();
    if (!LevelCfg().resetBotLevelEnabled)
        return;
    RandomBotLevelMgr::instance().OnBotLevelChanged(player, oldLevel);
}

// #148: AC RandomBotLevelWorldScript::OnAfterConfigLoad(reload = true) (:1116-1124) - re-read the keys and the working
// brackets on ".reload config". Optional hook for mod_playerbots.cpp PlayerbotsWorldScript::OnConfigLoad(true).
void RandomBotLevelMgrReloadConfig()
{
    RandomBotLevelMgr::instance().EnsureLoaded();
    std::lock_guard<std::recursive_mutex> guard(RandomBotLevelMgr::instance().Lock());
    LoadRandomBotLevelConfig();
    RandomBotLevelMgr::instance().LoadConfig();
    TC_LOG_INFO("playerbots", "[RandomBotLevelMgr] Level management config reloaded.");
}

void RandomPlayerbotMgr::OnPlayerLogout(Player* player)
{
    if (!player)
        return;

    // #148: AC RandomBotLevelPlayerScript::OnPlayerLogout (RandomBotLevelMgr.cpp:1158-1161), every player.
    RandomBotLevelMgr::instance().OnPlayerLogout(player);

    DisablePlayerBot(player->GetGUID());

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
        if (botAI && player == botAI->GetMaster())
        {
            botAI->SetMaster(nullptr);
            if (!bot->InBattleground())
            {
                // The strategies the player set are not the random bot's own: drop the stored ones too, or its next
                // login without a master would load them
                PlayerbotRepository::instance().Reset(botAI);
                botAI->ResetStrategies(); // #162 AC parity: the master logged out, drop its strategies
            }
        }
    }

    std::vector<Player*>::iterator i = std::find(_players.begin(), _players.end(), player);
    if (i != _players.end())
        _players.erase(i);
}

void RandomPlayerbotMgr::OnBotLoginInternal(Player* const bot)
{
    auto maxAllowed = sRandomPlayerbotMgr->GetMaxAllowedBotCount();
    TC_LOG_INFO("playerbots", "%u/%u Bot %s logged in - Active spec tab: %u Spec: %u", playerBots.size(), maxAllowed, bot->GetName().c_str(), (uint32)bot->GetActiveSpec(), (uint32)bot->GetSpecialization());

    // Phase 1.1 (issue #59): repair bots persisted with an empty class spellbook. InitClassSpells
    // is idempotent and level-filtered; BotFactory logs how much it changed.
    {
        BotFactory factory(bot, bot->GetLevel());
        factory.InitClassSpells();
        // #734 repair, same idea as #59 above: until #734 no bot ever learned a talent or a glyph (empty premade
        // links), and a level-90 bot only re-randomizes every 2 h - 14 days. Fill the missing tiers and the empty
        // unlocked glyph slots now; both are idempotent, so an already talented bot is untouched.
        factory.InitTalentTiers();
        factory.InitGlyphs(true);
    }

    // issue #62 (quests): a bot that logs in somewhere with no level-appropriate data can never
    // take a quest. Every quest giver in range offers quests it has long outlevelled, so
    // IsQuestWorthDoing rejects them and the quest counters stay at zero (measured: level 27-90
    // bots parked in zone 6453, which has no playerbot_farming_zone or playerbot_city row at all).
    // Re-place it into a level-appropriate zone. Bots that are already somewhere sensible are left
    // exactly where they are, so this does not scatter the population on every login.
    {
        uint32 const level = bot->GetLevel();
        uint32 const zoneId = bot->GetZoneId();

        bool known = false;
        bool levelAppropriate = false;

        for (RandomPlayerbotMgr::farm_zone const& zone : sRandomPlayerbotMgr->GetFarmCacheData())
        {
            if (zone.zone_id != zoneId)
                continue;
            known = true;
            levelAppropriate = level >= zone.min_level && level <= zone.max_level;
            break;
        }

        if (!known)
        {
            for (RandomPlayerbotMgr::city const& city : sRandomPlayerbotMgr->GetCityCacheData())
            {
                if (city.zone_id != zoneId)
                    continue;
                known = true;
                levelAppropriate = level >= city.min_level && level <= city.max_level;
                break;
            }
        }

        // issue #96: "known" only means the zone has a data row - it says nothing about whether this
        // bot belongs there. A bot standing in a capital of the other faction is re-placed as well.
        bool const enemyCapital = IsInEnemyCapital(bot);

        if (!known || !levelAppropriate || enemyCapital)
        {
            TC_LOG_DEBUG("playerbots",
                         "[QUEST] %s level %u in zone %u (known=%d appropriate=%d enemyCapital=%d) - re-placing for level",
                         bot->GetName().c_str(), level, zoneId, known ? 1 : 0, levelAppropriate ? 1 : 0,
                         enemyCapital ? 1 : 0);
            RandomTeleportForLevel(bot);
        }
    }

    // If this player has been created recently and is not assign horde / alliance as pandaren
    if (bot->GetRace() == RACE_PANDAREN_NEUTRAL)
    {
        // Allow player to get fully logged. Scheduled on Unit::m_Events, which Unit::Update() pumps every
        // tick. Unit::AddDelayedEvent() pushes into _functionsDelayed, which nothing ever updates, so the
        // pick has never run in this tree (issue #228).
        bot->Schedule(TimeValue(std::chrono::milliseconds(5000)), [bot]()
        {
            static const uint32 JOIN_THE_ALLIANCE = 1;
            static const uint32 JOIN_THE_HORDE = 0;

            WorldPacket packet(CMSG_SELECT_FACTION);
            packet << (std::rand() % 2 ? JOIN_THE_HORDE : JOIN_THE_ALLIANCE);

            if (!bot->IsInWorld())
                return;

            WorldSession* session = bot->GetSession();
            if (!session)
                return;

            session->HandleSelectFactionOpcode(packet);

            TC_LOG_INFO("playerbots", "%s Assigned to faction: %s", bot->GetName().c_str(), (bot->GetTeamId() == TEAM_ALLIANCE ? "Alliance" : "Horde"));
        });        
    }
    if (bot->GetRace() == RACE_GOBLIN && bot->GetLevel() == 1)
    {
        // issue #235: AddDelayedEvent() pushes into Unit::_functionsDelayed, which nothing ever updates
        // (Unit::Update only pumps m_Events), so this body has never run. Moved onto m_Events through
        // Unit::Schedule - the same pattern the pandaren faction pick in #228 uses.
        bot->Schedule(TimeValue(std::chrono::milliseconds(5000)), [bot]()
        {
            if (!bot->IsInWorld())
                return;

            // -- force finish quest
            // 25265/25266/25267 are goblin-only (quest_template.AllowableRaces = 256 = 1 << (RACE_GOBLIN-1),
            // MinLevel 1), so a level 1 goblin is eligible. 25268 "The Voice of Goldrinn" is NOT: it is the
            // Dark Iron Dwarf chain starter, AllowableRaces = 18875469 (alliance + pandaren), MinLevel 80.
            // Force-completing it for a level 1 horde goblin was a data bug, so it is dropped here.
            if (bot->getRewardedQuests().find(25265) == bot->getRewardedQuests().end())
            {
                bot->CompleteQuest(25265, true, true);
                bot->CompleteQuest(25266, true, true);
                bot->CompleteQuest(25267, true, true);
            }
        });
    }
    // issue #235: the worgen force-completion is retired, not revived. It was dead code
    // (Unit::AddDelayedEvent -> Unit::_functionsDelayed, which nothing updates) and it is not eligible for
    // the character it targets: a RACE_WORGEN bot at level 1 is under the minimum level of both quests it
    // names. In the world DB, 26706 "Endgame" is MinLevel 12 / QuestLevel 12 and 14434 "Rut'theran Village"
    // is MinLevel 5 / QuestLevel 12. Reviving the body would hand level 5 and level 12 quests to a level 1
    // character, which is worse than the no-op it replaces. If worgen bots need a real level 1 quest line
    // that is a separate issue with its own quest ids - do not resurrect this block.

    if (bot->GetClass() == CLASS_DEATH_KNIGHT && bot->GetLevel() == 55)
    {
        // issue #235: same dead Unit::AddDelayedEvent path as the goblin block; moved onto the pumped m_Events.
        bot->Schedule(TimeValue(std::chrono::milliseconds(5000)), [bot]()
        {
            if (!bot->IsInWorld())
                return;

            // -- force finish quest
            // 13188 "Where Kings Walk" is MinLevel 55 with the alliance race set, 13189 "Warchief's Blessing"
            // is MinLevel 55 with the horde race set, so the team check below matches eligibility exactly.
            // Both are StartItem quests (item 43440 / 43441), which is why they have to be forced.
            if (bot->getRewardedQuests().find(13188) == bot->getRewardedQuests().end() && bot->GetTeam() == Team::ALLIANCE)
                bot->CompleteQuest(13188, true, true);
            if (bot->getRewardedQuests().find(13189) == bot->getRewardedQuests().end() && bot->GetTeam() == Team::HORDE)
                bot->CompleteQuest(13189, true, true);
        });
    }

    /*if (sPlayerbotAIConfig->randomBotFixedLevel)
    {
        bot->SetPlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }
    else
    {
        bot->RemovePlayerFlag(PLAYER_FLAGS_NO_XP_GAIN);
    }*/
}

void RandomPlayerbotMgr::OnPlayerLogin(Player* player)
{
    // #148: AC RandomBotLevelPlayerScript::OnPlayerLogin (RandomBotLevelMgr.cpp:1144-1149). Runs for every player; for a
    // bot it runs before PlayerbotHolder::OnBotLogin attaches the AI (PlayerbotMgr.cpp HandlePlayerBotLoginCallback), as
    // AC's PLAYERHOOK_ON_LOGIN does (AC HandlePlayerLoginFromDB fires it before PlayerbotHolder::OnBotLogin).
    if (player)
    {
        RandomBotLevelMgr::instance().EnsureLoaded();
        if (LevelCfg().resetBotLevelEnabled)
            RandomBotLevelMgr::instance().OnBotLogin(player);
    }

    uint32 botsNearby = 0;

    for (PlayerBotMap::const_iterator it = GetPlayerBotsBegin(); it != GetPlayerBotsEnd(); ++it)
    {
        Player* const bot = it->second;
        if (player == bot /* || GET_PLAYERBOT_AI(player)*/)  // TEST
            continue;

        Cell playerCell(player->GetPositionX(), player->GetPositionY());
        Cell botCell(bot->GetPositionX(), bot->GetPositionY());

        // if (playerCell == botCell)
        // botsNearby++;

        Group* group = bot->GetGroup();
        if (!group)
            continue;

        for (GroupReference* gref = group->GetFirstMember(); gref; gref = gref->next())
        {
            Player* member = gref->GetSource();
            PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
            // #180: also re-attach when the current master is a stale Player object of the same character (this core
            // parks the old Player for 60 s on a socket drop and creates a new one on relog; AC re-binds the same
            // Player, so its bots never hold a stale master).
            Player* const cur = botAI ? botAI->GetMaster() : nullptr;
            bool const staleSameCharacter = cur && cur != player && cur->GetGUID() == player->GetGUID();
            if (botAI && member == player && (!cur || GET_PLAYERBOT_AI(cur) || staleSameCharacter))
            {
                if (!bot->InBattleground())
                {
                    botAI->SetMaster(player);
                    botAI->ResetStrategies();
                    botAI->TellMaster("Hello");
                }

                break;
            }
        }
    }

    if (botsNearby > 100 && false)
    {
        WorldPosition botPos(player);

        if (!player->GetFactionTemplateEntry())
        {
            botPos.GetReachableRandomPointOnGround(player, sPlayerbotAIConfig->reactDistance * 2, true);
            player->TeleportTo(botPos);
        }
        else
        {
            /*std::vector<TravelDestination*> dests = sTravelMgr->getRpgTravelDestinations(player, true, true, 200000.0f);

            do
            {
                RpgTravelDestination* dest = (RpgTravelDestination*)dests[urand(0, dests.size() - 1)];
                CreatureTemplate const* cInfo = dest->GetCreatureTemplate();
                if (!cInfo)
                    continue;

                FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(cInfo->faction);
                ReputationRank reaction = Unit::GetFactionReactionTo(player->GetFactionTemplateEntry(), factionEntry);

                if (reaction > REP_NEUTRAL && dest->nearestPoint(&botPos)->m_mapId == player->GetMapId())
                {
                    botPos = *dest->nearestPoint(&botPos);
                    break;
                }
            } while (true);*/
        }

        //player->TeleportTo(botPos);
    }

    // #532 (AC Bot/RandomPlayerbotMgr.cpp:2754-2764): a random bot's PvP flag follows the realm type.
    if (IsRandomBot(player))
        player->Unit::SetPvP(sWorld->IsPvPRealm());
    else
    {
        _players.push_back(player);
        TC_LOG_DEBUG("playerbots", "Including non-random bot player %s into random bot update", player->GetName().c_str());
    }
}

void RandomPlayerbotMgr::OnPlayerLoginError(uint32 bot)
{
    SetEventValue(bot, "add", 0, 0);
    _currentBots.erase(std::remove(_currentBots.begin(), _currentBots.end(), bot), _currentBots.end());
}

Player* RandomPlayerbotMgr::GetRandomPlayer()
{
    if (_players.empty())
        return nullptr;

    uint32 index = std::rand() % _players.size();
    return _players[index];
}

// AzerothCore's PrepareAddclassCache: the offline characters of the addclass accounts (type 2), by faction and class.
// Monks are included (AzerothCore skips class 10, which WotLK does not have, and so did this; the addclass command
// takes "monk") and the faction is the core's (Player::TeamForRace, worgen, goblins and pandaren included); a pandaren
// who has not picked a faction yet belongs to neither, but CreateRandomBots gives the addclass accounts faction
// pandaren.
void RandomPlayerbotMgr::PrepareAddclassCache()
{
    if (addClassTypeAccounts.empty())
    {
        TC_LOG_WARN("playerbots", "Unable to prepare add class cache as there are no AddClass accounts");
        return;
    }

    int32 collected = 0;
    for (uint32 accountId : addClassTypeAccounts)
    {
        QueryResult results = CharacterDatabase.PQuery(
            "SELECT guid, race, class FROM characters "
            "WHERE account = %u AND online = 0 AND "
            "guid NOT IN ( SELECT guid FROM guild_member )",
            accountId);
        if (!results)
            continue;

        do
        {
            Field* fields = results->Fetch();
            ObjectGuid guid = ObjectGuid(HighGuid::Player, fields[0].GetUInt32());
            uint32 team = Player::TeamForRace(fields[1].GetUInt8());
            uint8 claz = fields[2].GetUInt8();
            if (team != ALLIANCE && team != HORDE)
                continue;

            AddclassCache()[GetTeamClassIdx(team == ALLIANCE, claz)].push_back(guid);
            collected++;
        } while (results->NextRow());
    }

    TC_LOG_INFO("playerbots", ">> %u characters collected for addclass command from %u AddClass accounts.", uint32(collected),
        uint32(addClassTypeAccounts.size()));
}

// AzerothCore's AssignAccountTypes: every <prefix> account gets a type in playerbots_account_type, the random bot
// accounts (type 1) from the lowest, MaxRandomBots worth, and the addclass accounts (type 2) from the highest,
// AddClassAccountPoolSize of them. Assignments are permanent, unless MaxRandomBots or AddClassAccountPoolSize is 0
// (CalculateTotalAccountCount unassigns them then). The writes are synchronous (AzerothCore queues them), so the
// lists and the table agree when the bots log in.
void RandomPlayerbotMgr::AssignAccountTypes()
{
    TC_LOG_INFO("playerbots", "Assigning account types for random bot accounts...");

    rndBotTypeAccounts.clear();
    addClassTypeAccounts.clear();

    // First, get ALL randombot accounts from the database: the prefix followed by the account number only (AzerothCore
    // matches LIKE '<prefix>%', which also takes a real account whose name starts with the prefix). Names are stored
    // in upper case; each character of the prefix that is not a letter or digit is matched as a bracket expression.
    std::string pattern = "^";
    for (char c : sPlayerbotAIConfig->randomBotAccountPrefix)
    {
        if (std::isalnum(static_cast<unsigned char>(c)))
            pattern += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
        else if (c != '\\' && c != '\'' && c != ']' && c != '^' && c != '%')
            pattern += std::string("[") + c + "]";
    }
    pattern += "[0-9]+$";

    std::vector<uint32> allRandomBotAccounts;
    if (QueryResult allAccounts = LoginDatabase.PQuery("SELECT id FROM account WHERE username REGEXP '%s' ORDER BY id", pattern.c_str()))
    {
        do
        {
            allRandomBotAccounts.push_back(allAccounts->Fetch()[0].GetUInt32());
        } while (allAccounts->NextRow());
    }

    TC_LOG_INFO("playerbots", "Found %u total randombot accounts in database", uint32(allRandomBotAccounts.size()));

    // Check existing assignments
    std::map<uint32, uint8> currentAssignments;
    if (PreparedQueryResult existingAssignments = PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ACCOUNT_TYPE)))
    {
        do
        {
            Field* fields = existingAssignments->Fetch();
            currentAssignments[fields[0].GetUInt32()] = fields[1].GetUInt8();
        } while (existingAssignments->NextRow());
    }

    // Mark ALL randombot accounts as unassigned if not already assigned
    for (uint32 accountId : allRandomBotAccounts)
    {
        if (currentAssignments.find(accountId) == currentAssignments.end())
        {
            PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_ACCOUNT_TYPE);
            stmt->setUInt32(0, accountId);
            stmt->setUInt8(1, 0);
            PlayerbotsDatabase.DirectExecute(stmt);
            currentAssignments[accountId] = 0;
        }
    }

    // Calculate needed RNDbot accounts
    uint32 neededRndBotAccounts = 0;
    if (sPlayerbotAIConfig->maxRandomBots > 0)
    {
        uint32 divisor = RandomPlayerbotFactory::CalculateAvailableCharsPerAccount();
        uint32 maxBots = sPlayerbotAIConfig->maxRandomBots;
        if (sPlayerbotAIConfig->enablePeriodicOnlineOffline)
            maxBots = uint32(maxBots * sPlayerbotAIConfig->periodicOnlineOfflineRatio);
        if (divisor)
            neededRndBotAccounts = (maxBots + divisor - 1) / divisor;
    }

    // Count existing assigned accounts
    uint32 existingRndBotAccounts = 0;
    uint32 existingAddClassAccounts = 0;
    for (auto const& [accountId, accountType] : currentAssignments)
    {
        if (accountType == 1)
            existingRndBotAccounts++;
        else if (accountType == 2)
            existingAddClassAccounts++;
    }

    // Assign RNDbot accounts from lowest position if needed
    if (existingRndBotAccounts < neededRndBotAccounts)
    {
        uint32 toAssign = neededRndBotAccounts - existingRndBotAccounts;
        uint32 assigned = 0;
        for (uint32 i = 0; i < allRandomBotAccounts.size() && assigned < toAssign; i++)
        {
            uint32 accountId = allRandomBotAccounts[i];
            if (currentAssignments[accountId] == 0)
            {
                PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE);
                stmt->setUInt8(0, 1);
                stmt->setUInt32(1, accountId);
                PlayerbotsDatabase.DirectExecute(stmt);
                currentAssignments[accountId] = 1;
                assigned++;
            }
        }

        if (assigned < toAssign)
            TC_LOG_ERROR("playerbots", "Not enough unassigned accounts to fulfill RNDbot requirements. Need %u more accounts.", toAssign - assigned);
    }

    // Assign AddClass accounts from highest position if needed
    uint32 neededAddClassAccounts = sPlayerbotAIConfig->addClassAccountPoolSize;
    if (existingAddClassAccounts < neededAddClassAccounts)
    {
        uint32 toAssign = neededAddClassAccounts - existingAddClassAccounts;
        uint32 assigned = 0;
        for (size_t idx = allRandomBotAccounts.size(); idx-- > 0 && assigned < toAssign;)
        {
            uint32 accountId = allRandomBotAccounts[idx];
            if (currentAssignments[accountId] == 0)
            {
                PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE);
                stmt->setUInt8(0, 2);
                stmt->setUInt32(1, accountId);
                PlayerbotsDatabase.DirectExecute(stmt);
                currentAssignments[accountId] = 2;
                assigned++;
            }
        }

        if (assigned < toAssign)
            TC_LOG_ERROR("playerbots", "Not enough unassigned accounts to fulfill AddClass requirements. Need %u more accounts.", toAssign - assigned);
    }

    // Populate filtered account lists with ALL accounts of each type
    for (auto const& [accountId, accountType] : currentAssignments)
    {
        if (accountType == 1)
            rndBotTypeAccounts.push_back(accountId);
        else if (accountType == 2)
            addClassTypeAccounts.push_back(accountId);
    }

    // Bots must not take realm firsts from players: the core refuses them to accounts with this permission (AzerothCore
    // uses an achievement hook this core lacks). The insert is an upsert, so it is safe on every start.
    for (std::vector<uint32> const* accounts : { &rndBotTypeAccounts, &addClassTypeAccounts })
    {
        for (uint32 accountId : *accounts)
        {
            LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_INS_RBAC_ACCOUNT_PERMISSION);
            stmt->setUInt32(0, accountId);
            stmt->setUInt32(1, rbac::RBAC_PERM_CANNOT_EARN_REALM_FIRST_ACHIEVEMENTS);
            stmt->setBool(2, true);
            stmt->setInt32(3, -1);
            LoginDatabase.Execute(stmt);
        }
    }

    TC_LOG_INFO("playerbots", "Account type assignment complete: %u RNDbot accounts, %u AddClass accounts, %u unassigned",
        uint32(rndBotTypeAccounts.size()), uint32(addClassTypeAccounts.size()),
        uint32(currentAssignments.size() - rndBotTypeAccounts.size() - addClassTypeAccounts.size()));
}

bool RandomPlayerbotMgr::IsAccountType(uint32 accountId, uint8 accountType)
{
    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ACCOUNT_TYPE_BY_ACCOUNT_AND_TYPE);
    stmt->setUInt32(0, accountId);
    stmt->setUInt8(1, accountType);
    return PlayerbotsDatabase.Query(stmt) != nullptr;
}

void RandomPlayerbotMgr::PrepareTeleportCache()
{
    uint32 zone_count = 0, farm_spot_count = 0, city_count = 0;
    QueryResult results = PlayerbotsDatabase.PQuery("SELECT ZoneId, ZoneTyp, MinLevel, MaxLevel, TeamsDisabled, MapId, MinPlayers, MaxPlayers FROM playerbot_farming_zone");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();

            uint32 zoneId = fields[0].GetUInt32();
            uint32 zoneType = fields[1].GetUInt32();
            uint32 minLevel = fields[2].GetUInt32();
            uint32 maxLevel = fields[3].GetUInt32();
            Team disabled = (Team)fields[4].GetUInt32();
            uint32 mapid = fields[5].GetUInt32();
            uint32 minPlayer = fields[6].GetUInt32();
            uint32 maxPlayer = fields[7].GetUInt32();

            farm_zone new_zone
            {
                zoneId,     /* ZoneId */
                zoneType,   /* ZoneType */
                minLevel,   /* MinLevel */
                maxLevel,   /* MaxLevel */
                disabled,   /* TeamDisabled */
                mapid,      /* MapId */
                minPlayer,  /* MinPlayer */
                maxPlayer   /* MaxPlayer */
            };

            _farm_cache_data.emplace_back(std::move(new_zone));
            zone_count++;
        } while (results->NextRow());
    }

    results = PlayerbotsDatabase.PQuery("SELECT ZoneId, Id, MinLevel, MaxLevel, TeamsDisabled, X, Y, Z, Radius FROM playerbot_farming_spot");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();

            uint32 zoneid = fields[0].GetUInt32();
            uint32 id = fields[1].GetUInt32();
            uint32 minLevel = fields[2].GetUInt32();
            uint32 maxLevel = fields[3].GetUInt32();
            Team disabled = (Team)fields[4].GetUInt32();
            float x = fields[5].GetFloat();
            float y = fields[6].GetFloat();
            float z = fields[7].GetFloat();
            uint32 radius = fields[8].GetUInt32();

            farm_spot new_spot
            {
                minLevel,   /* MinLevel */
                maxLevel,   /* MaxLevel */
                0,          /* MapId */
                x,          /* X */
                y,          /* Y */
                z           /* Z */
            };

            for (auto& zone_data : _farm_cache_data)
            {
                if (zone_data.zone_id == zoneid)
                {
                    new_spot.map_id = zone_data.map_id;
                    zone_data.spots.emplace_back(std::move(new_spot));
                    break;
                }
            }

            farm_spot_count++;
        } while (results->NextRow());
    }

    results = PlayerbotsDatabase.PQuery("SELECT ZoneId, CityId, MinLevel, MaxLevel, TeamsDisabled, X, Y, Z, Radius FROM playerbot_city");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();

            uint32 ZoneId = fields[0].GetUInt32();
            uint32 city_id = fields[1].GetUInt32();
            uint32 minLevel = fields[2].GetUInt32();
            uint32 maxLevel = fields[3].GetUInt32();
            Team disabled = (Team)fields[4].GetUInt32();
            float x = fields[5].GetFloat();
            float y = fields[6].GetFloat();
            float z = fields[7].GetFloat();
            uint32 radius = fields[8].GetUInt32();
            uint32 map_id = UINT32_MAX;

            for (auto& zone_data : _farm_cache_data)
            {
                if (zone_data.zone_id == ZoneId)
                {
                    map_id = zone_data.map_id;
                    break;
                }
            }

            if (map_id == UINT32_MAX)
            {
                TC_LOG_ERROR("server.loading", "City {%u} in ZoneId {%u} is not found in zone cache", city_id, ZoneId);
                continue;
            }

            city new_city
            {
                ZoneId,     /* ZoneId */
                city_id,    /* CityId */
                map_id,     /* map_id */
                minLevel,   /* MinLevel */
                maxLevel,   /* MaxLevel */
                disabled,   /* TeamDisabled */
                x,          /* X */
                y,          /* Y */
                z,          /* Z */
            };

            _city_cache_data.emplace_back(std::move(new_city));
            city_count++;
        } while (results->NextRow());
    }
    results = PlayerbotsDatabase.PQuery("SELECT Zone, PathId, PointId, X, Y, Z FROM playerbot_zone_path");
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();

            uint32 ZoneId = fields[0].GetUInt32();
            uint32 PathId = fields[1].GetUInt32();
            uint32 PointId = fields[2].GetUInt32();
            float x = fields[3].GetFloat();
            float y = fields[4].GetFloat();
            float z = fields[5].GetFloat();

            farm_spot new_path
            {
                0,          /* MinLevel */
                0,          /* MaxLevel */
                0,          /* MapId */
                x,          /* X */
                y,          /* Y */
                z           /* Z */
            };

            for (auto& zone_data : _farm_cache_data)
            {
                if (zone_data.zone_id == ZoneId)
                {
                    new_path.map_id = zone_data.map_id;
                    new_path.min_level = zone_data.min_level;
                    new_path.max_level = zone_data.max_level;
                    zone_data.spots.emplace_back(std::move(new_path));
                    break;
                }
            }
        } while (results->NextRow());
    }

    TC_LOG_INFO("server.loading", ">> Loaded %u cache zone for %u farm spot", zone_count, farm_spot_count);
    TC_LOG_INFO("server.loading", ">> Loaded %u city zone", city_count);
}

// ---------------------------------------------------------------------------------------------
// Battleground queue bookkeeping (issue #33)
//
// Ported from AC mod-playerbots (RandomPlayerbotMgr::CheckBgQueue). Bots need to know how many
// players and bots are already waiting for each (queue type, bracket) before deciding to queue,
// otherwise every eligible bot piles into the same queue and the brackets never fill evenly.
//
// Arena queues are counted by the rated-arena census in strategy/bg/BattleGroundJoinAction.cpp
// (#437, PlayerbotArena::*), not into BattlegroundInfo.
// ---------------------------------------------------------------------------------------------

void RandomPlayerbotMgr::PrepareBattleMastersCache()
{
    _battleMastersCache.clear();

    uint32 count = 0;
    for (auto const& itr : sObjectMgr->GetAllCreatureData())
    {
        CreatureData const& creatureData = itr.second;

        CreatureTemplate const* creatureTemplate = sObjectMgr->GetCreatureTemplate(creatureData.id);
        if (!creatureTemplate)
            continue;

        if (!((creatureTemplate->npcflag | creatureData.npcflag) & UNIT_NPC_FLAG_BATTLEMASTER))
            continue;

        BattlegroundTypeId bgTypeId = sBattlegroundMgr->GetBattleMasterBG(creatureData.id);
        if (bgTypeId == BATTLEGROUND_TYPE_NONE)
            continue;

        FactionTemplateEntry const* factionEntry = sFactionTemplateStore.LookupEntry(creatureTemplate->faction);
        if (!factionEntry)
            continue;

        // Same faction-group test the travel caches use: an NPC hostile to a faction group is not
        // usable by it. A neutral battlemaster lands in both lists.
        if (!(factionEntry->hostileMask & FACTION_MASK_ALLIANCE))
            _battleMastersCache[TEAM_ALLIANCE][bgTypeId].push_back(itr.first);

        if (!(factionEntry->hostileMask & FACTION_MASK_HORDE))
            _battleMastersCache[TEAM_HORDE][bgTypeId].push_back(itr.first);

        ++count;
    }

    TC_LOG_INFO("playerbots", ">> Loaded %u battlemasters for bot battleground queueing", count);
}

ObjectGuid RandomPlayerbotMgr::GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId)
{
    if (!bot)
        return ObjectGuid::Empty;

    auto teamItr = _battleMastersCache.find(bot->GetTeamId());
    if (teamItr == _battleMastersCache.end())
        return ObjectGuid::Empty;

    auto bgItr = teamItr->second.find(bgTypeId);
    if (bgItr == teamItr->second.end() || bgItr->second.empty())
        return ObjectGuid::Empty;

    // Prefer one on the bot's own map so the guid resolves to a real creature when needed.
    for (ObjectGuid::LowType spawnId : bgItr->second)
    {
        CreatureData const* data = sObjectMgr->GetCreatureData(spawnId);
        if (!data || data->mapId != bot->GetMapId())
            continue;

        return ObjectGuid(HighGuid::Unit, data->id, spawnId);
    }

    return ObjectGuid::Empty;
}

namespace
{
// AC Bot/RandomPlayerbotMgr.cpp:1182 seeds Alterac Valley through the same updateBGInstanceCount
// helper as the other battlegrounds; Strand of the Ancients needs the same treatment here.
// AC keeps the bracket vector in PlayerbotAIConfig; read it here
// instead so PlayerbotAIConfig.h - which every module TU includes - does not change. The key names are
// AC's own and already sit (unused) in playerbots.conf.dist. An empty list leaves that battleground
// alone, exactly like the other battlegrounds above.
// #304: `def` is the code default when the key is absent - AC's for AV/IC (PlayerbotAIConfig.cpp:391,393: "3", "1",
// level 80) translated to 5.4.8 per-map ids (AV 7, IC 1), and the conf.dist value for the MoP-only maps.
std::vector<uint32> AutoJoinBrackets(char const* key, char const* def = "")
{
    std::vector<uint32> brackets;
    std::string value = sConfigMgr->GetStringDefault(key, def);

    while (!value.empty())
    {
        std::string::size_type comma = value.find(',');
        brackets.push_back(uint32(std::atoi(value.substr(0, comma).c_str())));

        if (comma == std::string::npos)
            break;

        value = value.substr(comma + 1);
    }

    return brackets;
}
}  // namespace

void RandomPlayerbotMgr::CheckBgQueue()
{
    if (!_bgCheckTimer)
    {
        _bgCheckTimer = time(nullptr);
        return;
    }

    // A battleground only starts filling on this tick, so it is also the worst-case delay before a
    // waiting player's queue gets any bots. That matters more the bigger the battleground is:
    // Warsong Gulch needs 10 a side and pops in seconds, Arathi Basin needs 15 and was measured
    // taking over three minutes on a 30 s tick. Configurable so a test server can shorten it.
    if (time(nullptr) < _bgCheckTimer + static_cast<time_t>(sPlayerbotAIConfig->bgCheckInterval))
        return;

    _bgCheckTimer = time(nullptr);
    std::unique_lock<std::shared_mutex> bgDataGuard(BattlegroundDataLock);  // w24 hang: see the header

    // Rebuild from scratch: these are live counts, not running totals.
    for (int bracket = BG_BRACKET_ID_FIRST; bracket < MAX_BATTLEGROUND_BRACKETS; ++bracket)
        for (int queueType = BATTLEGROUND_QUEUE_AV; queueType < MAX_BATTLEGROUND_QUEUE_TYPES; ++queueType)
            BattlegroundData[queueType][bracket] = BattlegroundInfo();

    PlayerbotArena::ResetCensus();  // #437: AC :928-934 for the rated-arena fields
    PlayerbotRatedBg::ResetCensus();  // #897: per-faction rated battleground census

    // #702 P5 (#832, AC's skirmish census + a level window, off by default): AiPlayerbot.ArenaAnyLevel.* read every tick
    // like the P4 keys; PlayerbotArena::CountQueued counts unrated arena slots only while the census is on.
    bool const arenaAnyLevel = sConfigMgr->GetBoolDefault("AiPlayerbot.ArenaAnyLevel.Enable", false);
    uint32 const arenaAnyLevelWindow = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.ArenaAnyLevel.LevelWindow", 2)));
    ArenaAnyLevelBots::BeginCensus(arenaAnyLevel);

    // #702 P4 (#831, beyond AC, off by default): also note, per (queue, bracket), the real players the core filed there
    // and whether they are below that bracket (core Battleground.AnyLevel fallback), so BGJoinAction can pick bots around
    // their level (BgAnyLevelRequests.h). Read here, every tick, so a .reload config applies without a restart.
    bool const bgAnyLevel = sConfigMgr->GetBoolDefault("AiPlayerbot.BgAnyLevel.Enable", false);
    uint32 const bgAnyLevelWindow = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.BgAnyLevel.LevelWindow", 5)));
    std::vector<BgAnyLevel::Request> bgAnyLevelRequests;

    auto countQueued = [this, bgAnyLevel, &bgAnyLevelRequests](Player* player, bool isBot)
    {
        if (!player || !player->IsInWorld() || !player->InBattlegroundQueue())
            return;

        Battleground* currentBg = player->GetBattleground();
        if (currentBg && currentBg->GetStatus() == STATUS_WAIT_LEAVE)
            return;

        TeamId teamId = player->GetTeamId();

        for (uint8 slot = 0; slot < PLAYER_MAX_BATTLEGROUND_QUEUES; ++slot)
        {
            BattlegroundQueueTypeId queueTypeId = player->GetBattlegroundQueueTypeId(slot);
            if (queueTypeId == BATTLEGROUND_QUEUE_NONE)
                continue;

            // Only account for the queues bots are allowed to use, so the fill maths below is
            // not skewed by queues they will never join.
            // #437: arena queues feed the rated-arena census (AC RandomPlayerbotMgr.cpp:968-988, 1057-1126),
            // not the battleground counts below.
            // #897: the rated battleground queue feeds its own census (BgRatedBg.cpp)
            if (queueTypeId == BATTLEGORUND_QUEUE_RATED_BG)
            {
                PlayerbotRatedBg::CountQueued(player, isBot);
                continue;
            }

            if (BattlegroundMgr::BGArenaType(queueTypeId))
            {
                PlayerbotArena::CountQueued(player, isBot, queueTypeId);
                continue;
            }

            if (!IsSupportedBgQueue(queueTypeId))
                continue;

            BattlegroundTypeId bgTypeId = BattlegroundMgr::BGTemplateId(queueTypeId);
            Battleground* bgTemplate = sBattlegroundMgr->GetBattlegroundTemplate(bgTypeId);
            if (!bgTemplate)
                continue;

            // #693, AC mod-playerbots#2850 (RandomPlayerbotMgr.cpp:935-948, :973, :1063): a participant may level past
            // the queue's bracket while waiting or inside the match, so use the bracket the instance or the queue
            // assigned, never the current level (GetAssignedBgBracket, BattleGroundJoinAction.cpp).
            PvPDifficultyEntry const* pvpDiff = GetAssignedBgBracket(player, queueTypeId, bgTemplate->GetMapId());
            if (!pvpDiff)
                continue;

            BattlegroundBracketId bracketId = pvpDiff->GetBracketId();
            BattlegroundInfo& info = BattlegroundData[queueTypeId][bracketId];

            info.minLevel = pvpDiff->MinLevel;
            info.maxLevel = pvpDiff->MaxLevel;

            if (isBot)
            {
                if (teamId == TEAM_ALLIANCE)
                    info.bgAllianceBotCount++;
                else
                    info.bgHordeBotCount++;
            }
            else
            {
                if (teamId == TEAM_ALLIANCE)
                    info.bgAlliancePlayerCount++;
                else
                    info.bgHordePlayerCount++;
            }

            if (currentBg && InMatchOfBgQueue(player, queueTypeId))  // #693, AC #2850 (RandomPlayerbotMgr.cpp:1012, :1098)
            {
                uint32 instanceId = currentBg->GetInstanceID();
                if (std::find(info.bgInstances.begin(), info.bgInstances.end(), instanceId) == info.bgInstances.end())
                    info.bgInstances.push_back(instanceId);

                info.bgInstanceCount = info.bgInstances.size();
            }

            // Only a REAL player waiting in a queue opens it for bots. Upstream does this in its
            // real-player pass only; its bot pass contributes counts and instance ids and nothing
            // else. Letting a bot set this flag too (as this originally did) makes every queue a
            // bot touches permanently "active": bots then sustain their own queues forever and
            // spread themselves across every battleground, instead of converging on the one queue
            // somebody is actually waiting in.
            if (isBot)
                continue;

            // #702 P4: a real player's (queue, bracket) and level; below the bracket = filed there by the any-level fallback
            if (bgAnyLevel)
                bgAnyLevelRequests.push_back({ uint32(queueTypeId), uint32(bracketId), uint8(player->GetLevel()),
                    player->GetLevel() < pvpDiff->MinLevel });

            // Upstream calls Player::IsInvitedForBattlegroundInstance() with no argument; LoA's
            // takes an instance id and the backing array is private, so read the invite state off
            // the queue entry instead.
            bool invited = false;
            GroupQueueInfo ginfo;
            if (sBattlegroundMgr->GetBattlegroundQueue(queueTypeId)
                    .GetPlayerGroupInfoData(player->GetGUID().GetRawValue(), &ginfo))
                invited = ginfo.IsInvitedToBGInstanceGUID != 0;

            if (!invited && !player->InBattleground())
                info.activeBgQueue = 1;
        }
    };

    for (Player* player : _players)
        countQueued(player, false);

    for (auto const& itr : playerBots)
    {
        // Upstream's bot pass is restricted to random bots - a bot that belongs to a player is
        // not part of the pool that fills queues, and is already covered by _players.
        if (!IsRandomBot(itr.second))
            continue;

        countQueued(itr.second, true);
    }

    BgAnyLevel::Publish(bgAnyLevel, bgAnyLevelWindow, std::move(bgAnyLevelRequests));  // #702 P4 (empty and off by default)
    ArenaAnyLevelBots::Publish(arenaAnyLevel, arenaAnyLevelWindow);                     // #702 P5 (empty and off by default)

    // Auto-join: mark the configured (battleground, bracket) pairs as active even though nobody
    // is waiting in them, so bots open those queues themselves. This is also what keeps bots
    // concentrated: without it they spread across every bracket of every battleground and no
    // single queue ever reaches the players-per-team a match needs.
    //
    // #582 (AC Bot/RandomPlayerbotMgr.cpp:1130-1131): like upstream, wait until every random bot is logged in
    // before seeding, for the battlegrounds and the rated arenas alike.
    if (!sPlayerbotAIConfig->randomBotAutoJoinBG || playerBots.size() < GetMaxAllowedBotCount())
    {
        LogBgQueueState();
        return;
    }

    auto seedQueue = [this](BattlegroundQueueTypeId queueTypeId, std::vector<uint32> const& brackets, uint32 minCount)
    {
        for (uint32 bracket : brackets)
        {
            if (bracket >= MAX_BATTLEGROUND_BRACKETS)
                continue;

            BattlegroundInfo& info = BattlegroundData[queueTypeId][bracket];

            // Already running (or already being filled) at the requested instance count.
            if (info.activeBgQueue != 0 || info.bgInstanceCount >= minCount || info.bgInstances.size() >= minCount)
                continue;

            info.activeBgQueue = 1;
        }
    };

    seedQueue(BATTLEGROUND_QUEUE_WS, sPlayerbotAIConfig->randomBotAutoJoinWSBrackets,
              sPlayerbotAIConfig->randomBotAutoJoinWSCount);
    seedQueue(BATTLEGROUND_QUEUE_TP, sPlayerbotAIConfig->randomBotAutoJoinTPBrackets,
              sPlayerbotAIConfig->randomBotAutoJoinTPCount);
    seedQueue(BATTLEGROUND_QUEUE_AB, sPlayerbotAIConfig->randomBotAutoJoinABBrackets,
              sPlayerbotAIConfig->randomBotAutoJoinABCount);
    seedQueue(BATTLEGROUND_QUEUE_EY, sPlayerbotAIConfig->randomBotAutoJoinEYBrackets,
              sPlayerbotAIConfig->randomBotAutoJoinEYCount);
    // #304: AC updateBGInstanceCount(BATTLEGROUND_QUEUE_AV/IC, ...) (Bot/RandomPlayerbotMgr.cpp:1185,1187) with
    // AC's keys and code defaults (PlayerbotAIConfig.cpp:391,393 brackets; :397,:399 counts 0 = AV/IoC not
    // seeded unless configured). IoC was never seeded here before #304. Bracket ids are per-map PvPDifficulty
    // RangeIndex (playerbots.conf.dist "Battleground bracket ids on 5.4.8").
    seedQueue(BATTLEGROUND_QUEUE_IC, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinICBrackets", "1"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGICCount", 0));
    seedQueue(BATTLEGROUND_QUEUE_AV, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinAVBrackets", "7"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGAVCount", 0));
    seedQueue(BATTLEGROUND_QUEUE_SA, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinSABrackets", "3"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGSACount", 1));
    // #304: the Cataclysm/Mists battlegrounds AC has no counterpart for, on AC's pattern (updateBGInstanceCount,
    // Bot/RandomPlayerbotMgr.cpp:1161-1170,1181-1185; keys named after the queue enum as AC's are; count default 1
    // like AC's WS/AB/EY at PlayerbotAIConfig.cpp:395-398). Code default brackets are empty: none of these maps has
    // AC's level-80 bracket, so nothing opens unless configured. 5.4.8 PvpDifficulty RangeIndex per map:
    // BFG (761) 0 = 85-89, 1 = 90; TOK (998), SM (727), DG (1105) 0 = 90 only.
    seedQueue(BATTLEGROUND_QUEUE_BFG, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinBFGBrackets"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGBFGCount", 1));
    seedQueue(BATTLEGROUND_QUEUE_TOK, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinTOKBrackets"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGTOKCount", 1));
    seedQueue(BATTLEGROUND_QUEUE_SM, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinSMBrackets"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGSMCount", 1));
    seedQueue(BATTLEGROUND_QUEUE_DG, AutoJoinBrackets("AiPlayerbot.RandomBotAutoJoinDGBrackets"),
              sConfigMgr->GetIntDefault("AiPlayerbot.RandomBotAutoJoinBGDGCount", 1));

    // #437 (AC :1133-1136, :1172-1177): the rated arena seeds, in the same block as upstream.
    PlayerbotArena::SeedAutoJoin();
    PlayerbotRatedBg::SeedAutoJoin();  // #897 (beyond AC): AiPlayerbot.RandomBotAutoJoinBGRatedBGCount

    LogBgQueueState();
}

// #142: port of AC RandomPlayerbotMgr::CheckLfgQueue (Bot/RandomPlayerbotMgr.cpp:1272-1308). Collects the
// dungeons real players currently have queued (per team) so random bots can queue for the same ones
// (LfgJoinAction::JoinLFG). 5.4.8 LFGMgr keeps one queue entry per queue id (multi-queue), so the AC
// GetState(guid)/GetSelectedDungeons(guid) pair becomes a walk over LFGMgr::GetPlayerQueues(): a queue
// counts while it is past NONE and before the dungeon itself (ROLECHECK/QUEUED/PROPOSAL/BOOT), as AC's
// `gState != NONE && gState < DUNGEON`. Runs on the world thread (called from UpdateAIInternal), so the
// AC worker-thread helper (CheckLfgQueueThread) is not ported.
void RandomPlayerbotMgr::CheckLfgQueue()
{
    if (!_lfgCheckTimer || time(nullptr) > (_lfgCheckTimer + 30))
        _lfgCheckTimer = time(nullptr);

    TC_LOG_DEBUG("playerbots", "Checking LFG Queue...");

    // Clear LFG list
    LfgDungeons[TEAM_ALLIANCE].clear();
    LfgDungeons[TEAM_HORDE].clear();

    // #702 (beyond AC, off by default): also note which of those dungeons a real player queued OUTSIDE the dungeon's
    // own level range (core LFG.AnyLevel) and at what level, so LfgJoinAction can pick bots around that level
    // (LfgAnyLevelRequests.h). Read here, every 30 s, so a .reload config applies without a restart.
    bool const anyLevel = sConfigMgr->GetBoolDefault("AiPlayerbot.LfgAnyLevel.Enable", false);
    uint32 const anyLevelWindow = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.LfgAnyLevel.LevelWindow", 3)));
    // #830: 0 = off (the level window alone, as phase 1)
    uint32 const anyLevelItemLevelWindow = uint32(std::max(0, sConfigMgr->GetIntDefault("AiPlayerbot.LfgAnyLevel.ItemLevelWindow", 0)));
    std::vector<LfgAnyLevel::Request> anyLevelRequests;

    for (Player* player : _players)
    {
        if (!player || !player->IsInWorld())
            continue;

        lfg::PlayerQueueDataMap const* queues = sLFGMgr->GetPlayerQueues(player->GetGUID());
        if (!queues)
            continue;

        for (auto const& queue : *queues)
        {
            lfg::LfgState const gState = queue.second.State;
            if (gState == lfg::LFG_STATE_NONE || gState >= lfg::LFG_STATE_DUNGEON)
                continue;

            for (uint32 const dungeonId : queue.second.Dungeons)
            {
                lfg::LFGDungeonData const* dungeon = sLFGMgr->GetLFGDungeon(dungeonId);
                if (!dungeon)
                    continue;

                LfgDungeons[player->GetTeamId()].push_back(dungeon->id);

                if (anyLevel)
                {
                    // #830: a premade group is scaled to its highest player level (AutoBalance), so its bots are picked
                    // around that member's level and item level; a solo requester is its own reference (phase 1).
                    Player* reference = player;
                    if (Group* group = player->GetGroup())
                        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                            if (Player* member = ref->GetSource())
                                if (member->GetLevel() > reference->GetLevel())
                                    reference = member;

                    uint32 const playerLevel = reference->GetLevel();
                    bool const outOfRange = playerLevel < uint32(dungeon->minlevel) ||
                        (dungeon->maxlevel && playerLevel > uint32(dungeon->maxlevel));
                    anyLevelRequests.push_back({ player->GetTeamId(), dungeon->id, uint8(playerLevel), outOfRange,
                                                 reference->GetAverageItemLevel() });
                }
            }
        }
    }

    LfgAnyLevel::Publish(anyLevel, anyLevelWindow, std::move(anyLevelRequests), anyLevelItemLevelWindow);

    TC_LOG_DEBUG("playerbots", "LFG Queue check finished");
}

/// Dump every (queue, bracket) that is live, so it is possible to see from the log whether a real
/// player's queue actually opened for bots and how full each side is.
BattlegroundInfo RandomPlayerbotMgr::GetBattlegroundInfo(uint32 queueTypeId, uint32 bracketId) const
{
    std::shared_lock<std::shared_mutex> guard(BattlegroundDataLock);
    auto const q = BattlegroundData.find(queueTypeId);
    if (q == BattlegroundData.end())
        return BattlegroundInfo();
    auto const b = q->second.find(bracketId);
    return b == q->second.end() ? BattlegroundInfo() : b->second;
}

void RandomPlayerbotMgr::LogBgQueueState()
{
    PlayerbotArena::LogCensus();  // #437: AC LogBattlegroundInfo's ARENA lines
    PlayerbotRatedBg::LogCensus();  // #897

    for (auto const& byQueue : BattlegroundData)
    {
        for (auto const& byBracket : byQueue.second)
        {
            BattlegroundInfo const& i = byBracket.second;
            if (!i.activeBgQueue && !i.bgInstanceCount && !i.bgAllianceBotCount && !i.bgHordeBotCount &&
                !i.bgAlliancePlayerCount && !i.bgHordePlayerCount)
                continue;

            TC_LOG_DEBUG("playerbots",
                         "[BGQ] queue %u bracket %u: active=%u instances=%u A(bots %u/players %u) H(bots %u/players %u)",
                         byQueue.first, byBracket.first, i.activeBgQueue, i.bgInstanceCount,
                         i.bgAllianceBotCount, i.bgAlliancePlayerCount, i.bgHordeBotCount, i.bgHordePlayerCount);
        }
    }
}

void RandomPlayerbotMgr::RandomTeleportForLevel(Player* bot)
{
    if (!bot) return;

    // AC RandomTeleportForLevel (Bot/RandomPlayerbotMgr.cpp:1804-1832): never touch a bot in an active BG/arena.
    if (bot->InBattleground())
        return;

    // #268 S9: the destination half is TravelMgr's again, exactly as upstream (AC :1811 / :1818). The
    // RandomPlayerbotMgr farm/city caches that stood in for it - GetFarmZoneForPlayer and
    // GetCityForPlayer - are deleted; nothing in this file picks a destination from them any more.
    // Note this also drops death-parity-followup.md Gap 2's ad-hoc fix, which existed only because the
    // farm-zone table hands out battleground/arena maps: RandomTeleportToLocs already applies AC's
    // randomBotMaps / team / water / ground filter, which is what that fix reached for.
    if (bot->GetLevel() >= 10 && urand(0, 100) < ProbTeleToBankers() * 100)
    {
        std::vector<WorldLocation> locs = sTravelMgr.GetCityLocations(bot);
        if (!locs.empty())
        {
            RandomTeleportToLocs(bot, locs, true);
            return;
        }
    }

    RandomTeleportToLocs(bot, sTravelMgr.GetTeleportLocations(bot));

    // NOT ported (AC :1820-1825): the sPlayerbotAIConfig.randomBotConcentrateInPlayerZone arm and its
    // GetPlayerZoneTeleportLocations() helper. The key does not exist in the fork's PlayerbotAIConfig
    // and AC's own default is false, so the arm is dead upstream too, and it is a separate feature
    // (gather bots into zones real players occupy) rather than part of this substitution.
}

namespace
{
// Capital cities and the faction that owns them (issue #96). Upstream mod-playerbots keeps the same
// list in TravelMgr::capitals rather than in a DB table; this table is the authority here too,
// because the DB rows proved easy to get wrong (playerbot_city had Stormwind stored with the Horde
// value, which let Horde bots into Stormwind and kept Alliance bots out of it).
struct CapitalZone
{
    uint32 zoneId;
    Team owner;
    char const* name;
};

constexpr CapitalZone CAPITAL_ZONES[] =
{
    {1497, HORDE,    "Undercity"},
    {1637, HORDE,    "Orgrimmar"},
    {1638, HORDE,    "Thunder Bluff"},
    {3487, HORDE,    "Silvermoon"},
    {1519, ALLIANCE, "Stormwind"},
    {1537, ALLIANCE, "Ironforge"},
    {1657, ALLIANCE, "Darnassus"},
    {3557, ALLIANCE, "The Exodar"},
};
}   // namespace

char const* RandomPlayerbotMgr::CapitalZoneName(uint32 zoneId)
{
    for (CapitalZone const& zone : CAPITAL_ZONES)
        if (zone.zoneId == zoneId)
            return zone.name;

    return nullptr;
}

bool RandomPlayerbotMgr::IsEnemyCapitalZone(uint32 zoneId, uint32 team)
{
    // Pandaren-neutral bots (and any other non-faction team) never have an "enemy capital".
    if (team != HORDE && team != ALLIANCE)
        return false;

    for (CapitalZone const& zone : CAPITAL_ZONES)
        if (zone.zoneId == zoneId)
            return zone.owner != team;

    return false;
}

bool RandomPlayerbotMgr::IsInEnemyCapital(Player* bot)
{
    if (!bot || !bot->IsInWorld())
        return false;

    return IsEnemyCapitalZone(bot->GetZoneId(), bot->GetTeam());
}
