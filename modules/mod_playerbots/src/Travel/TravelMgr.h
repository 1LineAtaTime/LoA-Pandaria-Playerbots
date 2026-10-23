/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Mgr/Travel/TravelMgr.h) into the
 * Legends of Azeroth Pandaria fork.
 *
 * This is deliberately a *lean* TravelMgr: only the cross-map transfer bookkeeping that
 * TravelNode/TravelNodeMap needs to estimate distances between maps. Upstream's TravelMgr also
 * carries the quest/grind/boss/explore destination system (~4.9k lines); the LoA fork selects
 * destinations from the RandomPlayerbotMgr farm/city caches instead (see NewRpgBaseAction), so
 * that half is intentionally not ported. Keeping the upstream class name and method signatures
 * means a later full port can grow this file rather than replace it.
 */

#ifndef _PLAYERBOT_TRAVELMGR_H
#define _PLAYERBOT_TRAVELMGR_H

#include "Define.h"
// #268 S5: getMSTime(), used by TravelTarget's inline constructors and timers.
#include "Timer.h"
#include "WorldPosition.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <map>
#include <random>
#include <string>

#include <unordered_map>
#include <utility>
#include <tuple>
#include <vector>

/// A one-way jump between two maps (portal / transport / flight path), used to estimate the
/// distance between two positions that do not share a map.
class mapTransfer
{
public:
    mapTransfer(WorldPosition pointFrom1, WorldPosition pointTo1, float portalLength1 = 0.1f)
        : pointFrom(pointFrom1), pointTo(pointTo1), portalLength(portalLength1)
    {
    }

    bool isFrom(WorldPosition point) { return point.getMapId() == pointFrom.getMapId(); }
    bool isTo(WorldPosition point) { return point.getMapId() == pointTo.getMapId(); }

    WorldPosition* getPointFrom() { return &pointFrom; }
    WorldPosition* getPointTo() { return &pointTo; }

    bool isUseful(WorldPosition point) { return isFrom(point) || isTo(point); }

    float distance(WorldPosition point)
    {
        return isUseful(point) ? (isFrom(point) ? point.distance(pointFrom) : point.distance(pointTo)) : 200000.f;
    }

    bool isUseful(WorldPosition start, WorldPosition end) { return isFrom(start) && isTo(end); }

    float distance(WorldPosition start, WorldPosition end)
    {
        return isUseful(start, end) ? (start.distance(pointFrom) + portalLength + pointTo.distance(end)) : 200000.f;
    }

    float fDist(WorldPosition start, WorldPosition end)
    {
        return isUseful(start, end) ? (start.fDist(pointFrom) + portalLength + pointTo.fDist(end)) : 200000.f;
    }

private:
    WorldPosition pointFrom;
    WorldPosition pointTo;
    float portalLength = 0.1f;
};
// ================================================================================================
// #268 slice S1 - the TravelDestination hierarchy (base class + all eight subclasses).
//
// Ported from AC mod-playerbots Mgr/Travel/TravelMgr.h:495-742, in AC's declaration order:
//   TravelDestination              (:495), NullTravelDestination        (:574),
//   QuestTravelDestination         (:594), QuestRelationTravelDestination (:613),
//   QuestObjectiveTravelDestination(:636), RpgTravelDestination        (:665),
//   ExploreTravelDestination       (:684), GrindTravelDestination       (:706),
//   BossTravelDestination          (:725).
//
// Scope notes:
//   * WorldPosition here is this fork's module-local Travel/WorldPosition.h (included at the top of
//     this header). AC's own WorldPosition (Mgr/Travel/TravelMgr.h:82-333) is deliberately NOT
//     imported - that class already exists in the fork.
//   * The destination containers and accessors are #268 S2 and are TravelMgr members at the bottom of
//     this header. QuestContainer itself already exists at Travel/WorldPosition.h:75-80 and is NOT
//     redeclared here - see the note above `class TravelMgr`.
//   * getBossTravelDestinations is DECLARATION ONLY (AC TravelMgr.h:919-920): AC defines it in
//     Ai/Base/Actions/ChooseTravelTargetAction.cpp:781, not in TravelMgr.cpp, and #269 lands that
//     file. Defining a second copy here would collide with it at link time.
// ================================================================================================

// A destination for a bot to travel to and do something.
class TravelDestination
{
public:
    TravelDestination() {}
    TravelDestination(float radiusMin1, float radiusMax1)
    {
        radiusMin = radiusMin1;
        radiusMax = radiusMax1;
    }
    TravelDestination(std::vector<WorldPosition*> points1, float radiusMin1, float radiusMax1)
    {
        points = points1;
        radiusMin = radiusMin1;
        radiusMax = radiusMax1;
    }
    // #268 S3: the destination now OWNS its points. Upstream AC stores the ADDRESS of a reused
    // loop-local `WorldPosition` (AC .cpp:2072-2076, :2284-2295, :2334-2363), so every point in
    // every destination dangles as soon as the iteration ends - a latent upstream bug that only
    // never fired because nothing in AC ever calls LoadQuestTravelTable(). S3 heap-allocates each
    // point instead, which makes this destructor (defined in TravelMgr.cpp) the owner.
    //
    // INVARIANT for later slices (#268 S4 city/banker + teleport locations): a destination owns
    // exactly the WorldPosition pointers handed to addPoint() / the points ctor, and nothing else
    // deletes them.
    virtual ~TravelDestination();

    void addPoint(WorldPosition* pos) { points.push_back(pos); }

    void setExpireDelay(uint32 delay) { expireDelay = delay; }

    void setCooldownDelay(uint32 delay) { cooldownDelay = delay; }

    void setMaxVisitors(uint32 maxVisitors1 = 0, uint32 maxVisitorsPerPoint1 = 0)
    {
        maxVisitors = maxVisitors1;
        maxVisitorsPerPoint = maxVisitorsPerPoint1;
    }

    std::vector<WorldPosition*> getPoints(bool ignoreFull = false);
    uint32 getExpireDelay() { return expireDelay; }
    uint32 getCooldownDelay() { return cooldownDelay; }

    void addVisitor() { ++visitors; }

    void remVisitor() { --visitors; }

    uint32 getVisitors() { return visitors; }

    virtual Quest const* GetQuestTemplate() { return nullptr; }
    virtual bool isActive([[maybe_unused]] Player* bot) { return false; }

    bool isFull(bool ignoreFull = false);

    virtual std::string const getName() { return "TravelDestination"; }
    virtual int32 getEntry() { return 0; }
    virtual std::string const getTitle() { return "generic travel destination"; }

    WorldPosition* nearestPoint(WorldPosition* pos);

    float distanceTo(WorldPosition* pos) { return nearestPoint(pos)->distance(pos); }
    bool onMap(WorldPosition* pos) { return nearestPoint(pos)->GetMapId() == pos->GetMapId(); }
    virtual bool isIn(WorldPosition* pos, float radius = 0.f)
    {
        return onMap(pos) && distanceTo(pos) <= (radius ? radius : radiusMin);
    }
    virtual bool isOut(WorldPosition* pos, float radius = 0.f)
    {
        return !onMap(pos) || distanceTo(pos) > (radius ? radius : radiusMax);
    }
    float getRadiusMin() { return radiusMin; }

    std::vector<WorldPosition*> touchingPoints(WorldPosition* pos);
    std::vector<WorldPosition*> sortedPoints(WorldPosition* pos);
    std::vector<WorldPosition*> nextPoint(WorldPosition* pos, bool ignoreFull = true);

protected:
    std::vector<WorldPosition*> points;
    float radiusMin = 0;
    float radiusMax = 0;

    uint32 visitors = 0;
    uint32 maxVisitors = 0;
    uint32 maxVisitorsPerPoint = 0;
    uint32 expireDelay = 5 * 1000;
    uint32 cooldownDelay = 60 * 1000;
};

// A travel target that is always inactive and jumps to cooldown.
class NullTravelDestination : public TravelDestination
{
public:
    NullTravelDestination(uint32 cooldownDelay1 = 5 * 60 * 1000) : TravelDestination()
    {
        cooldownDelay = cooldownDelay1;
    };

    Quest const* GetQuestTemplate() override { return nullptr; }

    bool isActive([[maybe_unused]] Player* bot) override { return false; }

    std::string const getName() override { return "NullTravelDestination"; }
    std::string const getTitle() override { return "no destination"; }

    bool isIn([[maybe_unused]] WorldPosition* pos, [[maybe_unused]] float radius = 0.f) override { return true; }
    bool isOut([[maybe_unused]] WorldPosition* pos, [[maybe_unused]] float radius = 0.f) override { return false; }
};

// A travel target specifically related to a quest.
class QuestTravelDestination : public TravelDestination
{
public:
    QuestTravelDestination(uint32 questId1, float radiusMin1, float radiusMax1);

    Quest const* GetQuestTemplate() override { return questTemplate; }

    bool isActive(Player* bot) override;

    std::string const getName() override { return "QuestTravelDestination"; }
    int32 getEntry() override { return 0; }
    std::string const getTitle() override;

protected:
    uint32 questId;
    Quest const* questTemplate;
};

// A quest giver or taker.
class QuestRelationTravelDestination : public QuestTravelDestination
{
public:
    QuestRelationTravelDestination(uint32 quest_id1, uint32 entry1, uint32 relation1, float radiusMin1,
                                   float radiusMax1)
        : QuestTravelDestination(quest_id1, radiusMin1, radiusMax1)
    {
        entry = entry1;
        relation = relation1;
    }

    bool isActive(Player* bot) override;
    std::string const getName() override { return "QuestRelationTravelDestination"; }
    int32 getEntry() override { return entry; }
    std::string const getTitle() override;
    virtual uint32 getRelation() { return relation; }

private:
    uint32 relation;
    int32 entry;
};

// A quest objective (creature/gameobject to grind/loot)
class QuestObjectiveTravelDestination : public QuestTravelDestination
{
public:
    QuestObjectiveTravelDestination(uint32 quest_id1, uint32 entry1, uint32 objective1, float radiusMin1,
                                    float radiusMax1, uint32 itemId1 = 0)
        : QuestTravelDestination(quest_id1, radiusMin1, radiusMax1)
    {
        objective = objective1;
        entry = entry1;
        itemId = itemId1;
    }

    bool isCreature();
    uint32 ReqCreature();
    uint32 ReqGOId();
    uint32 ReqCount();

    bool isActive(Player* bot) override;
    std::string const getName() override { return "QuestObjectiveTravelDestination"; }
    int32 getEntry() override { return entry; }
    std::string const getTitle() override;

private:
    uint32 objective;
    uint32 entry;
    uint32 itemId = 0;
};

// A location with rpg target(s) based on race and level
class RpgTravelDestination : public TravelDestination
{
public:
    RpgTravelDestination(uint32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
    {
        entry = entry1;
    }

    bool isActive(Player* bot) override;
    virtual CreatureTemplate const* GetCreatureTemplate();
    std::string const getName() override { return "RpgTravelDestination"; }
    int32 getEntry() override { return 0; }
    std::string const getTitle() override;

protected:
    uint32 entry;
};

// A location with zone exploration target(s)
class ExploreTravelDestination : public TravelDestination
{
public:
    ExploreTravelDestination(uint32 areaId1, float radiusMin1, float radiusMax1)
        : TravelDestination(radiusMin1, radiusMax1)
    {
        areaId = areaId1;
    }

    bool isActive(Player* bot) override;
    std::string const getName() override { return "ExploreTravelDestination"; }
    int32 getEntry() override { return 0; }
    std::string const getTitle() override { return title; };
    virtual void setTitle(std::string newTitle) { title = newTitle; }
    virtual uint32 getAreaId() { return areaId; }

protected:
    uint32 areaId;
    std::string title = "";
};

// A location with zone exploration target(s)
class GrindTravelDestination : public TravelDestination
{
public:
    GrindTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
    {
        entry = entry1;
    }

    bool isActive(Player* bot) override;
    virtual CreatureTemplate const* GetCreatureTemplate();
    std::string const getName() override { return "GrindTravelDestination"; }
    int32 getEntry() override { return entry; }
    std::string const getTitle() override;

protected:
    int32 entry;
};

// A location with a boss
class BossTravelDestination : public TravelDestination
{
public:
    BossTravelDestination(int32 entry1, float radiusMin1, float radiusMax1) : TravelDestination(radiusMin1, radiusMax1)
    {
        entry = entry1;
        cooldownDelay = 1000;
    }

    bool isActive(Player* bot) override;
    CreatureTemplate const* getCreatureTemplate();
    std::string const getName() override { return "BossTravelDestination"; }
    int32 getEntry() override { return entry; }
    std::string const getTitle() override;

protected:
    int32 entry;
};

// ================================================================================================
// #268 slice S5 - TravelTarget, the per-bot "where am I travelling right now" state machine.
//
// Ported 1:1 from AC Mgr/Travel/TravelMgr.h:744-843 (declaration) and .cpp:1481-1720 (methods).
// It is declared AFTER the eight destination classes because it holds a TravelDestination*.
// TravelStatus / TravelState are not redeclared here: they already ship with the fork's
// Travel/WorldPosition.h:47-70 (the equivalents of AC TravelMgr.h:44-66). AiObject and the
// `botAI` member come in through WorldPosition.h -> AiObject.h, the fork's counterpart of AC's
// Bot/Engine/AiObject.h.
//
// ALIASING: wPosition is a NON-owning pointer, exactly as in AC. AC's points are owned by their
// destination, and the only WorldPosition TravelTarget itself constructs is a function-local
// `WorldPosition pos(bot)` whose address is passed to distance()/isIn() and never stored or
// returned. So this slice adds no address-of-loop-local point and does NOT reproduce AC's
// aliasing bug (AC .cpp:2072-2076, :2284-2295, :2334-2363 - owning those points is S2/S3's job,
// per the #268 plan correction).
// ================================================================================================

// Current target and location for the bot to travel to.
// The flow is as follows:
// PREPARE   (wait until no loot is near)
// TRAVEL    (move towards target until close enough) (rpg and grind is disabled)
// WORK      (grind/rpg until the target is no longer active) (rpg and grind is enabled on quest mobs)
// COOLDOWN  (wait some time free to do what the bot wants)
// EXPIRE    (if any of the above actions take too long pick a new target)
class TravelTarget : AiObject
{
public:
    TravelTarget(PlayerbotAI* botAI) : AiObject(botAI), m_status(TRAVEL_STATUS_NONE), startTime(getMSTime()){};
    TravelTarget(PlayerbotAI* botAI, TravelDestination* tDestination1, WorldPosition* wPosition1)
        : AiObject(botAI), m_status(TRAVEL_STATUS_NONE), startTime(getMSTime())
    {
        setTarget(tDestination1, wPosition1);
    }

    ~TravelTarget();

    void setTarget(TravelDestination* tDestination1, WorldPosition* wPosition1, bool groupCopy1 = false);
    void setStatus(TravelStatus status);
    void setExpireIn(uint32 expireMs) { statusTime = getExpiredTime() + expireMs; }

    void incRetry(bool isMove)
    {
        if (isMove)
            ++moveRetryCount;
        else
            ++extendRetryCount;
    }

    void setRetry(bool isMove, uint32 newCount = 0)
    {
        if (isMove)
            moveRetryCount = newCount;
        else
            extendRetryCount = newCount;
    }

    void setForced(bool forced1) { forced = forced1; }

    void setRadius(float radius1) { radius = radius1; }

    void copyTarget(TravelTarget* target);
    void addVisitors();
    void releaseVisitors();

    float distance(Player* bot);

    WorldPosition* getPosition();
    TravelDestination* getDestination();

    uint32 getEntry()
    {
        if (!tDestination)
            return 0;

        return tDestination->getEntry();
    }

    PlayerbotAI* getAi() { return botAI; }

    uint32 getExpiredTime() { return getMSTime() - startTime; }

    uint32 getTimeLeft() { return statusTime - getExpiredTime(); }

    uint32 getMaxTravelTime();
    uint32 getRetryCount(bool isMove) { return isMove ? moveRetryCount : extendRetryCount; }

    bool isTraveling();
    bool isActive();
    bool isWorking();
    bool isPreparing();
    bool isMaxRetry(bool isMove) { return isMove ? (moveRetryCount > 5) : (extendRetryCount > 5); }

    TravelStatus getStatus() { return m_status; }

    TravelState getTravelState();

    bool isGroupCopy() { return groupCopy; }

    bool isForced() { return forced; }

protected:
    TravelStatus m_status;

    uint32 startTime;
    uint32 statusTime = 0;

    bool forced = false;
    float radius = 0.f;
    bool groupCopy = false;
    bool visitor = true;

    uint32 extendRetryCount = 0;
    uint32 moveRetryCount = 0;

    TravelDestination* tDestination = nullptr;
    WorldPosition* wPosition = nullptr;
};

struct CreatureData;

/// A coarse grid index over every creature spawn, so "what is spawned near this point" is a
/// bucket lookup instead of a scan.
///
/// Upstream AC answers that question with WorldPosition::getCreaturesNear(), which walks the
/// *entire* spawn store on every call. TravelNodePath::calculateCost calls it once per path
/// point, so node generation there is O(points x spawns) - on 5.4.8 data that is billions of
/// comparisons and boot-time generation would never finish. The index is built once and
/// dropped again when generation is done.
class SpawnGridIndex
{
public:
    static constexpr float CELL_SIZE = 100.0f;

    void Build();
    // Drops the index (defined as SpawnGridIndex::Clear in TravelMgr.cpp). Not AC code - AC has no
    // SpawnGridIndex. (#268 S8: the stray TravelMgr::LoadQuestTravelTable declaration and its comment,
    // misplaced here by S3, were removed; the real declaration is in class TravelMgr below.)
    void Clear();
    bool IsBuilt() const { return _built; }

    /// Spawns within `radius` (2D) of (x, y) on `mapId`.
    void Query(uint32 mapId, float x, float y, float radius, std::vector<CreatureData const*>& out) const;

private:
    static uint64 CellKey(uint32 mapId, int32 cx, int32 cy)
    {
        return (uint64(mapId) << 40) | (uint64(uint16(int16(cx))) << 20) | uint64(uint16(int16(cy)));
    }
    static int32 CellOf(float v) { return int32(std::floor(v / CELL_SIZE)); }

    std::unordered_map<uint64, std::vector<CreatureData const*>> _cells;
    bool _built = false;
};

class Player;
// #268 S8: QuestStatusData is a struct at Quests/QuestDef.h:523; forward-declared for getQuestStatus().
struct QuestStatusData;

// #268 slice S2: AC's `struct QuestContainer` (AC Mgr/Travel/TravelMgr.h:71-77) is NOT redeclared
// here - this fork already carries a byte-identical copy at Travel/WorldPosition.h:75-80, which this
// header includes at line 22. AC declares the type in TravelMgr.h; the fork's earlier TravelMgr port
// put it in WorldPosition.h instead. Declaring it a second time here would be a redefinition at
// TravelMgr.cpp compile time, so the existing WorldPosition.h one is used as-is.

class TravelMgr
{
public:
    // #268 S8. AC TravelMgr.h:849-853. Only used by SelectAuctioneerByMap below.
    struct NpcLocation
    {
        WorldLocation loc;
        uint32 entry;
    };

    /// A flight master spawn, with everything the taxi code needs resolved once at load.
    struct FlightMasterInfo
    {
        WorldPosition pos;
        uint32 zoneId{0};         // resolved once at cache load
        uint32 taxiNodeId{0};     // DBC taxi node nearest to this flight master
        uint32 templateEntry{0};  // creature_template entry
        uint32 dbGuid{0};         // creature spawn guid
    };

    /// Level range a zone is considered appropriate for.
    struct LevelBracket
    {
        uint32 low{0};
        uint32 high{0};

        bool InsideBracket(uint32 level) const { return level >= low && level <= high; }
    };

    /// #268 S4. AC TravelMgr.h:988-992, made public for the same reason LevelBracket above is
    /// (AC keeps both private; nothing outside TravelMgr reads either).
    struct BankerLocation
    {
        WorldLocation loc;
        uint32 entry{0};
    };

    static TravelMgr& instance()
    {
        static TravelMgr instance;
        return instance;
    }

    /// Build the navigation caches (flight masters, travel hubs, zone level brackets) and the
    /// taxi graph. Called once at world startup, after ObjectMgr and MapMgr are up.
    void Init();

    /// Nearest flight master to the bot on its own map, for its faction. Null when there is none.
    FlightMasterInfo const* GetNearestFlightMasterInfo(Player* bot) const;

    /// Taxi node ids reachable in `zoneId` for `team` (Team, i.e. ALLIANCE / HORDE), skipping
    /// `excludeNode`.
    std::vector<uint32> GetFlightNodesInZone(uint32 zoneId, uint32 team, uint32 excludeNode = 0) const;

    /// A taxi route (list of node ids) from the flight master next to the bot to a level
    /// appropriate zone. Empty when the bot is not at a flight master or nothing suitable exists.
    std::vector<std::vector<uint32>> GetOptimalFlightDestinations(Player* bot);

    /// Innkeeper/flight-master "hub" spots appropriate for the bot's level and faction.
    std::vector<WorldLocation> const& GetTravelHubs(Player* bot);

    /// AC TravelMgr::GetLocsPerLevelCache: 50-yd grind cells for a bot of `level` (issue #61).
    std::vector<WorldLocation> const& GetLocsPerLevelCache(uint8 level) const;

    // ---- #268 slice S4 - city / banker + teleport locations ------------------------------
    //
    // AC TravelMgr.h:879 / .cpp:4480-4488. The teleport pool for a bot of its level: its faction's
    // innkeeper / flight-master hubs when EnableNewRpgStrategy is on, otherwise the grind cells.
    // AC returns by value; the fork returns a reference, like GetTravelHubs / GetLocsPerLevelCache.
    std::vector<WorldLocation> const& GetTeleportLocations(Player* bot);

    // AC TravelMgr.h:881 / .cpp:4498-4556. Banker spots for the bot's level, from
    // bankerLocsPerLevelCache. With AiPlayerbot.EnableWeightTeleToCityBankers on, the result is a
    // single banker picked by the per-city weight config instead of the whole level's list.
    std::vector<WorldLocation> GetCityLocations(Player* bot);

    // #268 S8. AC TravelMgr.h:883. AC declares this and defines it NOWHERE (no definition anywhere in
    // mod-playerbots), so it is declaration-only here too: calling it would not link on either tree.
    bool SelectAuctioneerByMap(Player* bot, NpcLocation& outAuctioneer);

    // #268 S8. AC TravelMgr.h:886-901, verbatim. Weighted Fisher-Yates style shuffle; the RPG actions call
    // `sTravelMgr.weighted_shuffle(actions.begin(), actions.end(), relevances.begin(), relevances.end(), gen)`.
    // Needs <random> (std::discrete_distribution), <iterator> (std::next) and <algorithm> (std::swap).
    template <class D, class W, class URBG>
    void weighted_shuffle(D first, D last, W first_weight, W last_weight, URBG&& g)
    {
        while (first != last && first_weight != last_weight)
        {
            std::discrete_distribution<int> dd(first_weight, last_weight);
            auto i = dd(g);

            if (i)
            {
                std::swap(*first, *std::next(first, i));
                std::swap(*first_weight, *std::next(first_weight, i));
            }
            ++first;
            ++first_weight;
        }
    }

    SpawnGridIndex& GetSpawnIndex() { return _spawnIndex; }

    /// Creature spawns within `radius` of `pos`. Returns nothing while the index is not built.
    std::vector<CreatureData const*> GetCreaturesNear(WorldPosition const& pos, float radius) const;
    // #268 slice S1. AC Mgr/Travel/TravelMgr.h:904-906 / :908 declares both; S1's TravelDestination
    // implementations call them (TravelDestination::nextPoint at AC :1124,
    // QuestObjectiveTravelDestination::isActive at AC :1250). Defined in TravelMgr.cpp (S1 completion).
    std::vector<WorldPosition*> getNextPoint(WorldPosition* center, std::vector<WorldPosition*> points,
                                             uint32 amount = 1);
    // #268 S8. AC TravelMgr.h:906 / .cpp:3892-3940. By-value overload; no caller in AC or here.
    std::vector<WorldPosition> getNextPoint(WorldPosition center, std::vector<WorldPosition> points, uint32 amount = 1);
    // #268 S8. AC TravelMgr.h:907 / .cpp:3942. No caller in AC or here (S1's getObjectiveStatus is
    // rewritten against 5.4.8's Quest::GetObjectives() and does not use it). Player::getQuestStatusMap()
    // exists on 5.4.8 (Entities/Player/Player.h:1923, QuestStatusMap = std::map<uint32, QuestStatusData>).
    QuestStatusData* getQuestStatus(Player* bot, uint32 questId);
    bool getObjectiveStatus(Player* bot, Quest const* pQuest, uint32 objective);
    // #268 S8. AC TravelMgr.h:909 / .cpp:3726-3836. NOT ported live: no caller in AC, and the body needs
    // WotLK DIALOG_STATUS_* values whose ordering AC's `dialogStatusNew > dialogStatus` relies on; 5.4.8
    // has `enum class QuestGiverStatus` (Quests/QuestDef.h:111) with different values/ordering. The body
    // is kept as an #if 0 block in the S8 section of TravelMgr.cpp.
    // uint32 getDialogStatus(Player* pPlayer, int32 questgiver, Quest const* pQuest);

    // ---- #268 slice S2 - destination accessors (AC Mgr/Travel/TravelMgr.h:910-920, defined AC
    // .cpp:3970-4139). ----------------------------------------------------------------------------
    //
    // getQuestStatus() (AC .h:907) and getDialogStatus() (AC .h:909) are declared above next to
    // getNextPoint (#268 S8): getQuestStatus live, getDialogStatus commented out.
    std::vector<TravelDestination*> getQuestTravelDestinations(Player* bot, int32 questId = -1, bool ignoreFull = false,
                                                               bool ignoreInactive = false, float maxDistance = 5000,
                                                               bool ignoreObjectives = false);
    std::vector<TravelDestination*> getRpgTravelDestinations(Player* bot, bool ignoreFull = false,
                                                             bool ignoreInactive = false, float maxDistance = 5000);
    std::vector<TravelDestination*> getExploreTravelDestinations(Player* bot, bool ignoreFull = false,
                                                                 bool ignoreInactive = false);
    std::vector<TravelDestination*> getGrindTravelDestinations(Player* bot, bool ignoreFull = false,
                                                               bool ignoreInactive = false, float maxDistance = 5000);

    // AC declares this at AC TravelMgr.h:919-920 but DEFINES it in
    // Ai/Base/Actions/ChooseTravelTargetAction.cpp:781 - not in TravelMgr.cpp at all. Declaration only:
    // #269 lands ChooseTravelTargetAction.cpp and owns the definition.
    std::vector<TravelDestination*> getBossTravelDestinations(Player* bot, bool ignoreFull = false,
                                                              bool ignoreInactive = false, float maxDistance = 25000);

    // AC TravelMgr.h:949 / .cpp:1750. Diagnostic for a quest objective whose unit/item is not in the
    // world; every error string is behind AC's own `logQuestErrors` guard, so it is silent by default.
    void logQuestError(uint32 errorNr, Quest* quest, uint32 objective = 0, uint32 unitId = 0, uint32 itemId = 0);

    // #268 slice S5. AC TravelMgr.h:922 declares setNullTravelTarget(); AC TravelMgr.h:929-930 holds
    // the two null members. nullTravelDestination is the "no destination" target TravelTarget falls
    // back to when the bot loses the "travel" strategy (AC TravelMgr.cpp:1630, :1662), and
    // nullWorldPosition is the empty point that goes with it so distance() stays well defined.
    // Both stay NSDMIs on the singleton exactly as in AC, so they are constructed with the
    // TravelMgr instance and live for the whole process.
    void setNullTravelTarget(Player* player);

    NullTravelDestination* nullTravelDestination = new NullTravelDestination();
    WorldPosition* nullWorldPosition = new WorldPosition();

    // #268 S8. AC TravelMgr.h:932-944, verbatim (members badVmap / badMmap at AC :961).
    void addBadVmap(uint32 mapId, uint8 x, uint8 y) { badVmap.push_back(std::make_tuple(mapId, x, y)); }

    void addBadMmap(uint32 mapId, uint8 x, uint8 y) { badMmap.push_back(std::make_tuple(mapId, x, y)); }

    bool isBadVmap(uint32 mapId, uint8 x, uint8 y)
    {
        return std::find(badVmap.begin(), badVmap.end(), std::make_tuple(mapId, x, y)) != badVmap.end();
    }

    bool isBadMmap(uint32 mapId, uint8 x, uint8 y)
    {
        return std::find(badMmap.begin(), badMmap.end(), std::make_tuple(mapId, x, y)) != badMmap.end();
    }

    // #268 S8. AC TravelMgr.h:946-947 / .cpp:4285-4358. Debug CSV writers, no caller in AC or here.
    void printGrid(uint32 mapId, int x, int y, std::string const type);
    void printObj(WorldObject* obj, std::string const type);

    // #268 S8. AC TravelMgr.h:951. Never read or written anywhere in AC either.
    std::vector<uint32> avoidLoaded;

    // ---- #268 slice S2 - the destination containers (AC Mgr/Travel/TravelMgr.h:953-959) -----------
    //
    // These are what the accessors above read. S2 only DECLARES them: nothing populates them until
    // #268 S3 (LoadQuestTravelTable, AC .cpp:1817-2383) lands, so until then every accessor returns an
    // empty vector. That is AC's own state too - the #268 plan's trap 1 records that in AC nothing
    // ever calls LoadQuestTravelTable either, so upstream these caches are empty as well.
    //
    // Ownership is AC's: TravelMgr owns every destination in these containers and Clear() deletes them.
    // The WorldPosition points *inside* each destination are a separate problem - AC stores the address
    // of a reused loop-local (AC .cpp:2072-2076, :2284-2295, :2334-2363), which S3 must fix. S2 creates
    // no points at all, so there is nothing for it to own yet.
    std::vector<QuestTravelDestination*> questGivers;
    std::vector<RpgTravelDestination*> rpgNpcs;
    std::vector<GrindTravelDestination*> grindMobs;
    std::vector<BossTravelDestination*> bossMobs;

    std::unordered_map<uint32, ExploreTravelDestination*> exploreLocs;
    std::unordered_map<uint32, QuestContainer*> quests;

    // #268 S8. AC TravelMgr.h:961.
    std::vector<std::tuple<uint32, uint8, uint8>> badVmap, badMmap;

    // AC TravelMgr.h:872 / .cpp:1817. Populates questGivers / quests / rpgNpcs / grindMobs /
    // bossMobs / exploreLocs from the quest guidp map plus the creature spawn store. #268 S3.
    //
    // WIRING: deliberately NOT called from anywhere in this fork (nor in AC - see the S3 block at
    // the end of TravelMgr.cpp for the full reasoning). The only consumer of these containers is
    // ChooseTravelTargetAction (getBossTravelDestinations plus the "travel target" action), which
    // #269 owns and which has not landed yet.
    void LoadQuestTravelTable();

    // AC TravelMgr.h:871 / .cpp:1721. S5 brought the player loop (AC .cpp:1721-1726); #268 S2 extended
    // the same body with AC's destination-container loops (AC .cpp:1728-1747). Still one definition.
    void Clear();

    /// Record that `start` (on one map) connects to `end` (on another) at `portalDistance` cost.
    /// `makeShortcuts` additionally derives chained transfers (A->B plus B->C gives A->C).
    void addMapTransfer(WorldPosition start, WorldPosition end, float portalDistance = 0.1f, bool makeShortcuts = true);

    /// Rebuild the transfer table from the current TravelNodeMap links.
    void loadMapTransfers();

    void clearMapTransfers() { mapTransfersMap.clear(); }

    /// Distance between two positions, routed through a map transfer when they are on
    /// different maps. Returns 200000 when no transfer is known.
    float mapTransDistance(WorldPosition start, WorldPosition end);
    /// Cheap (squared-ish) variant used inside A* heuristics.
    float fastMapTransDistance(WorldPosition start, WorldPosition end);

private:
    TravelMgr() = default;
    ~TravelMgr() = default;

    TravelMgr(TravelMgr const&) = delete;
    TravelMgr& operator=(TravelMgr const&) = delete;

    struct MapPairHash
    {
        std::size_t operator()(std::pair<uint32, uint32> const& p) const noexcept
        {
            return (static_cast<std::size_t>(p.first) << 32) ^ static_cast<std::size_t>(p.second);
        }
    };

    void PrepareZone2LevelBracket();
    void PrepareDestinationCache();

    std::unordered_map<std::pair<uint32, uint32>, std::vector<mapTransfer>, MapPairHash> mapTransfersMap;
    SpawnGridIndex _spawnIndex;

    // Navigation caches, keyed by creature spawn guid / character level.
    std::map<uint32, FlightMasterInfo> allianceFlightMasterCache;
    std::map<uint32, FlightMasterInfo> hordeFlightMasterCache;
    std::map<uint8, std::vector<WorldLocation>> allianceHubsPerLevelCache;
    std::map<uint8, std::vector<WorldLocation>> hordeHubsPerLevelCache;
    // #268 S4 (AC TravelMgr.h:999-1000). Bankers of each level, and banker entry -> its spot, filled
    // by PrepareDestinationCache and read by GetCityLocations.
    std::map<uint8, std::vector<BankerLocation>> bankerLocsPerLevelCache;
    std::unordered_map<uint32, WorldLocation> bankerEntryToLocation;
    std::map<uint8, std::vector<WorldLocation>> locsPerLevelCache;
    // #268 S8. AC TravelMgr.h:1002. #611: now filled in PrepareDestinationCache from tempCreatureCache (AC .cpp:4648,
    // :4694, :4831-4848). Nothing reads it in AC either.
    std::unordered_map<uint32, std::vector<WorldLocation>> creatureSpawnsByTemplate;
    std::map<uint32, LevelBracket> zone2LevelBracket;

    std::vector<WorldLocation> _emptyHubs;
    bool _initialized = false;
};

#define sTravelMgr TravelMgr::instance()

#endif
