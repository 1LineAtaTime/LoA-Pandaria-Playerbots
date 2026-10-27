#ifndef _PLAYERBOT_RANDOMPLAYERBOTMGR_H
#define _PLAYERBOT_RANDOMPLAYERBOTMGR_H

#include "PlayerbotMgr.h"
#include <shared_mutex>

class CachedEvent
{
public:
    CachedEvent() : value(0), lastChangeTime(0), validIn(0), data("") {}
    CachedEvent(const CachedEvent& other)
        : value(other.value), lastChangeTime(other.lastChangeTime), validIn(other.validIn), data(other.data)
    {
    }
    CachedEvent(uint32 value, uint32 lastChangeTime, uint32 validIn, std::string const data = "")
        : value(value), lastChangeTime(lastChangeTime), validIn(validIn), data(data)
    {
    }

    bool IsEmpty() { return !lastChangeTime; }

public:
    uint32 value;
    uint32 lastChangeTime;
    uint32 validIn;
    std::string data;
};

/// Per (queue type, bracket) snapshot of who is waiting for, or already inside, a battleground.
///
/// Ported from AC mod-playerbots (BattlegroundInfo). Bots use it to decide whether joining a
/// queue would actually help form a match, instead of all piling into the same one. Rebuilt from
/// scratch on every RandomPlayerbotMgr::CheckBgQueue() pass.
struct BattlegroundInfo
{
    std::vector<uint32> bgInstances;

    uint32 bgInstanceCount = 0;
    uint32 minLevel = 0;
    uint32 maxLevel = 0;
    uint32 activeBgQueue = 0;  // 0 = inactive, 1 = someone is waiting, so bots may fill it

    // Bots currently queued, per faction
    uint32 bgHordeBotCount = 0;
    uint32 bgAllianceBotCount = 0;

    // Real players currently queued, per faction
    uint32 bgHordePlayerCount = 0;
    uint32 bgAlliancePlayerCount = 0;
};

class Player;
class ObjectGuid;
class RandomPlayerbotMgr : public PlayerbotHolder
{
public:

    struct farm_spot
    {
        uint32 min_level;
        uint32 max_level;
        uint32 map_id;
        float x;
        float y;
        float z;
    };

    struct farm_zone
    {
        uint32 zone_id;
        uint32 zone_type;
        uint32 min_level;
        uint32 max_level;
        Team team_disabled;
        uint32 map_id;
        uint32 min_player;
        uint32 max_player;

        std::vector<farm_spot> spots;
    };

    struct city
    {
        uint32 zone_id;
        uint32 city_id;
        uint32 map_id;
        uint32 min_level;
        uint32 max_level;
        Team team_disabled;
        float x;
        float y;
        float z;
    };

public:
    RandomPlayerbotMgr();
    virtual ~RandomPlayerbotMgr();
    static RandomPlayerbotMgr* instance()
    {
        static RandomPlayerbotMgr instance;
        return &instance;
    }

    /// <summary>
    /// Allow pre load reserve on internal containers
    /// </summary>
    /// <param name="size"></param>
    void Reserve(const uint32 size);
    void UpdateAIInternal(uint32 elapsed, bool minimal = false) override;

public:
    bool IsRandomBot(Player* bot);
    // Test harness (.botscenario <bot> login): make a bot the harness logged in count as a current random
    // bot, so IsRandomBot() is true and it gets the random-bot strategies, as a manager login would.
    void AddCurrentBotForTest(uint32 guidLow);
    bool IsRandomBot(ObjectGuid::LowType bot);

    void TagForRandomize(Player* bot, uint32 level);
    void RandomizeFirst(Player* bot);
    void Randomize(Player* bot);
    
    void Clear(Player* bot);
    
    void OnPlayerLogout(Player* player);
    void OnPlayerLogin(Player* player);
    void OnPlayerLoginError(uint32 bot);
    
    Player* GetRandomPlayer();
    std::vector<Player*> GetPlayers() { return _players; };
    PlayerBotMap GetAllBots() { return playerBots; };
    void Refresh(Player* bot);
    uint32 GetMaxAllowedBotCount();
    bool ProcessBot(Player* player);
    void Revive(Player* player);
    uint32 GetValue(Player* bot, std::string const type);
    uint32 GetValue(uint32 bot, std::string const type);
    std::string const GetData(uint32 bot, std::string const type);
    void SetValue(uint32 bot, std::string const type, uint32 value, std::string const data = "");
    void SetValue(Player* bot, std::string const type, uint32 value, std::string const data = "");
    void Remove(Player* bot);
    void CheckPlayers();
    float getActivityMod() { return _activityMod; }
    float getActivityPercentage() { return _activityMod * 100.0f; }
    void setActivityPercentage(float percentage) { _activityMod = percentage / 100.0f; }
    static uint8 GetTeamClassIdx(bool isAlliance, uint8 claz) { return isAlliance * 20 + claz; }

    void ScheduleTeleport(uint32 bot, uint32 time = 0);
    void PrepareAddclassCache();
    std::map<uint8, std::vector<ObjectGuid>>& AddclassCache() { return _addclassCache; };

    // AzerothCore's account types (playerbots_account_type): 1 random bot accounts, 2 addclass accounts
    void AssignAccountTypes();
    bool IsAccountType(uint32 accountId, uint8 accountType);
    std::vector<uint32> const& GetRndBotTypeAccounts() const { return rndBotTypeAccounts; }

    void PrepareTeleportCache();
    void RandomTeleportForLevel(Player* bot);

    // #268 S9: read-only access to the farm/city DB caches, now BRACKET-ONLY.
    // These rows used to be the destination source for RandomTeleportForLevel; that is
    // TravelMgr's job now (TravelMgr::GetTeleportLocations / GetCityLocations, and the
    // "new rpg" roaming strategy's own TravelMgr route in NewRpgBaseAction.cpp). What is
    // left here is zone -> level bracket data: TravelMgr::PrepareZone2LevelBracket (the
    // fork's MoP bracket extension, which overrides AC's hard-coded table with
    // playerbot_farming_zone) and the issue-#62 login bracket test below.
    // Written once in PrepareTeleportCache() and never mutated afterwards, so concurrent
    // reads from map threads are safe.
    std::vector<farm_zone> const& GetFarmCacheData() const { return _farm_cache_data; }
    std::vector<city> const& GetCityCacheData() const { return _city_cache_data; }

    // -- battleground queue bookkeeping (issue #33) ---------------------------------------
    /// Log every live (queue, bracket) entry of BattlegroundData at DEBUG.
    void LogBgQueueState();

    /// Rebuild BattlegroundData from the players and bots currently queued or in a match.
    void CheckBgQueue();
    /// #142 (AC RandomPlayerbotMgr.h:156): rebuild LfgDungeons from the dungeons real players have queued.
    void CheckLfgQueue();
    /// [team] -> dungeon ids real players are queued for; read by LfgJoinAction::JoinLFG (AC RandomPlayerbotMgr.h:154).
    std::map<TeamId, std::vector<uint32>> LfgDungeons;

    /// Cache the battlemaster spawns per faction and battleground type.
    void PrepareBattleMastersCache();
    /// A battlemaster of `bgTypeId` the bot could plausibly have talked to, or an empty guid.
    ObjectGuid GetBattleMasterGUID(Player* bot, BattlegroundTypeId bgTypeId);

    /// [queue type id][bracket id] -> queue/instance counts. Written only by CheckBgQueue on the
    /// world thread; bots read it while deciding whether to queue.
    std::map<uint32, std::map<uint32, BattlegroundInfo>> BattlegroundData;
    /// w24 hang: map-thread readers used BattlegroundData[q][b] (operator[] INSERTS) while the world thread iterated it
    /// -> corrupted tree, LogBgQueueState spun forever. Writers (CheckBgQueue, world thread) hold this exclusively;
    /// bots read through GetBattlegroundInfo (shared lock, find, copy - never inserts).
    mutable std::shared_mutex BattlegroundDataLock;
    BattlegroundInfo GetBattlegroundInfo(uint32 queueTypeId, uint32 bracketId) const;

protected:
    void OnBotLoginInternal(Player* const bot) override;

private:
    void GetBots();
    uint32 AddRandomBots();
    bool ProcessBot(uint32 bot);
    uint32 GetEventValue(uint32 bot, std::string const event);
    std::string const GetEventData(uint32 bot, std::string const event);
    uint32 SetEventValue(uint32 bot, std::string const event, uint32 value, uint32 validIn, std::string const data = "");

public:
    void ScheduleRandomize(uint32 bot, uint32 time);  // public as in AC (RandomPlayerbotMgr.h:240); harness randomize verb (#592)
    /// True when `zoneId` is a capital city owned by the faction opposite to `team` (issue #96).
    static bool IsEnemyCapitalZone(uint32 zoneId, uint32 team);
    /// True when the bot is standing inside a capital city of the other faction.
    static bool IsInEnemyCapital(Player* bot);
    /// Name of the capital at `zoneId`, or nullptr.
    static char const* CapitalZoneName(uint32 zoneId);

    uint32 activeBots = 0;
private:
    // pid values are set in constructor
    float _activityMod = 0.25;
    bool _isBotInitializing = true;
    time_t _playersCheckTimer;
    typedef void (RandomPlayerbotMgr::* ConsoleCommandHandler)(Player*);
    std::vector<Player*> _players;
    uint32 _processTicks;
    uint32 _playersLevel;

    // -- lists
    std::map<uint32, std::map<std::string, CachedEvent>> _eventCache;
    std::map<uint8, std::vector<ObjectGuid>> _addclassCache;
    std::vector<uint32> rndBotTypeAccounts;
    std::vector<uint32> addClassTypeAccounts;
    std::vector<city> _city_cache_data;
    std::vector<farm_zone> _farm_cache_data;

    std::vector<uint32> _currentBots;

    // [team][bg type] -> battlemaster creature spawn guids
    std::map<TeamId, std::map<BattlegroundTypeId, std::vector<ObjectGuid::LowType>>> _battleMastersCache;
    time_t _bgCheckTimer = 0;
    time_t _lfgCheckTimer = 0;  // #142 (AC LfgCheckTimer)
    time_t _realPlayerLastTimeSeen = 0;  // AC RealPlayerLastTimeSeen (DisabledWithoutRealPlayer)
    time_t _delayLoginBotsTimer = 0;     // AC DelayLoginBotsTimer
};

#define sRandomPlayerbotMgr RandomPlayerbotMgr::instance()

#endif
