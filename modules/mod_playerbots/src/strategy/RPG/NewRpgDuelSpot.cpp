/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #201 (#612 #614 #615): duel spots. New for the Legends of Azeroth Pandaria fork. Pattern: AC NewRpgOutdoorPvP.cpp
 * (status action), the MoP duel-spot objective manager (update, revive, population).
 */

#include "NewRpgDuelSpot.h"

#include <algorithm>
#include <cctype>
#include <cfloat>
#include <cmath>
#include <cstdio>
#include <string>
#include <vector>

#include "BotFactory.h"
#include "BotScenario.h"
#include "Config.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "Group.h"
#include "LFGMgr.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "MapManager.h"
#include "MotionMaster.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "RpgAction.h"
#include "ServerFacade.h"
#include "SocialMgr.h"
#include "Timer.h"
#include "Trigger.h"
#include "World.h"
#include "WorldPacket.h"
#include "WorldSession.h"

namespace
{
// getMSTime() based deadlines: 0 means "unset / due now"; otherwise due once the (wrapping) difference is >= 0.
bool Due(uint32 now, uint32 deadline)
{
    return deadline == 0 || int32(now - deadline) >= 0;
}

uint32 NonZero(uint32 t)
{
    return t ? t : 1;
}

bool IsRealPlayerChar(Player* p)
{
    PlayerbotAI* ai = GET_PLAYERBOT_AI(p);
    return !ai || ai->IsRealPlayer();
}

bool TeamMatches(uint8 spotTeam, TeamId teamId)
{
    return spotTeam == 0 || (spotTeam == 1 && teamId == TEAM_ALLIANCE) || (spotTeam == 2 && teamId == TEAM_HORDE);
}

uint32 EffectiveMinLevel(DuelSpotInfo const& spot)
{
    return std::max(spot.minLevel, sPlayerbotAIConfig->duelSpotMinLevel);
}

bool LevelOk(DuelSpotInfo const& spot, uint32 level)
{
    return level >= EffectiveMinLevel(spot) && level <= spot.maxLevel;
}

// "in a group with a real player" (SPEC 2.1).
bool InGroupWithRealPlayer(Player* bot)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;
    for (GroupReference* itr = group->GetFirstMember(); itr != nullptr; itr = itr->next())
    {
        Player* member = itr->GetSource();
        if (member && member != bot && IsRealPlayerChar(member))
            return true;
    }
    return false;
}

bool InQueueOrMatch(Player* bot)
{
    return bot->InBattleground() || bot->InArena() || bot->InBattlegroundQueue() ||
           sLFGMgr->GetActiveState(bot->GetGUID()) != lfg::LFG_STATE_NONE;
}

// #201 (PM reconciliation, SPEC amendment A7): same as the AC tree - a random point minD-maxD yd around the anchor, ground
// height from the map when it is loaded (|h - anchor z| <= 30), random orientation.
void RandomPointAround(DuelSpotInfo const& spot, float minD, float maxD, Map* map, uint32 phaseMask, float& x, float& y,
                       float& z, float& o)
{
    float const dist = minD + static_cast<float>(rand_norm()) * (maxD - minD);
    float const angle = static_cast<float>(rand_norm()) * 2.0f * static_cast<float>(M_PI);
    x = spot.x + dist * std::cos(angle);
    y = spot.y + dist * std::sin(angle);
    z = spot.z;
    o = static_cast<float>(rand_norm()) * 2.0f * static_cast<float>(M_PI);
    if (map)
    {
        float const h = map->GetHeight(phaseMask, x, y, spot.z + 10.0f);
        if (h > INVALID_HEIGHT && std::fabs(h - spot.z) <= 30.0f)
            z = h + 0.2f;
    }
}

float Frac(double v)
{
    return static_cast<float>(v - std::floor(v));
}

// #201 (#615, wave 32 t8): the seeder holds a seeded bot's registry entry while the bot stands within the spot radius
// + SEED_HOLD_EXTRA (the MoP 40 yd leash) and its own last beat is at most SEED_HOLD_MAX_IDLE_MS old.
// Without it the 30 s prune dropped seeded bots whose status action did not run for 30 s: a bot inside a duel (in
// combat, the non-combat engine does not tick), and a bot waiting for its hand-off (NewRpgStatusUpdateAction runs from
// AC's "random" trigger, and at 1000 bots an idle bot ticked every 10-40 s). The pending hand-off went with the entry,
// the bot stayed IDLE at the spot, and the seeder re-seeded it (Reset, teleport, Randomize) over and over: wave 32 AC
// 74 seed lines in the 480 s pairing window (Seegulo/Emelenn 13 each), MoP Adone re-seeded and then left IDLE, so the
// t8 kill of a seeded bot found it unregistered and it was never revived. Same values on both trees.
// #615 (wave 38): the hold no longer ends after SEED_HOLD_MAX_IDLE_MS. Away from real players a bot runs AC's
// activity rotation (PlayerbotAI::AllowActive: BotActiveAlone 10 % per BotActiveAloneDurationSeconds 30 s window,
// GetFixedBotNumber); a passive tick runs only relevance >= 100 actions (Engine::DoNextAction `minimal`), so neither
// the hand-off (NewRpgStatusUpdateAction, 11) nor the status action (3) runs. P(no active window in 300 s) = 0.9^10
// = 35 %: AC w38 Kaelemar stood IDLE at the spot 375 s with its hand-off pending, was pruned and re-seeded (churn);
// MoP Galanda / Omascon waited 190 / 210 s. Now: a seeded bot inside the spot + SEED_HOLD_EXTRA and not taken by a
// group / BG / arena / queue / LFG is held without a time limit; if its own beat is older than SEED_REARM_IDLE_MS
// the hand-off is re-armed (it re-takes RPG_DUEL_SPOT at its next status update - covers a status lost to a full
// Reset). Outside the zone, alive, not duelling / in combat / teleporting: the seeder leashes it back (the MoP
// 40 yd leash) instead of waiting for the bot's own step 5, which a passive bot does not run (AC w38
// far bot never back, MoP 14 s by luck). Same values on both trees.
constexpr float SEED_HOLD_EXTRA = 40.0f;
constexpr uint32 SEED_REARM_IDLE_MS = 300000;

// Same sequence as the AC tree's TeleportToSpot (SendMovementFlagUpdate is AC-only).
bool TeleportToSpot(Player* bot, DuelSpotInfo const& spot)
{
    if (!bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    Map* map = bot->GetMapId() == spot.mapId ? bot->GetMap() : sMapMgr->FindBaseNonInstanceMap(spot.mapId);
    float x, y, z, o;
    RandomPointAround(spot, 10.0f, 30.0f, map, bot->GetPhaseMask(), x, y, z, o);
    bot->GetMotionMaster()->Clear();
    bot->RemoveAurasWithInterruptFlags(SpellAuraInterruptFlags(AURA_INTERRUPT_FLAG_TELEPORTED | AURA_INTERRUPT_FLAG_CHANGE_MAP));
    return bot->TeleportTo(spot.mapId, x, y, z, o);
}

// #201 (#612, wave-12 1:1 run): the duel request spell 7266 is instant with a 10 yd range (Wowhead MoP tooltip of spell
// 7266, fetched 2026-10-06; the same on WotLK) and cannot be cast mounted (Spell::CheckCast SPELL_FAILED_NOT_MOUNTED,
// Server.log "[CAST] <bot> spell=7266 ... result=67"). Two players inside a spot can be up to 2 * (radius + 5) yd
// apart, so the challenger first walks to within this distance of its target. Same value on both trees.
constexpr float DUEL_REQUEST_DIST = 8.0f;

// Dismount the way the client does (CMSG_CANCEL_MOUNT_AURA), the call this fork's CheckMountStateAction dismount makes
// (DismountBot, CheckMountStateAction.cpp:53; AC CheckMountStateAction::Dismount, AC :346-357).
void DismountForDuel(Player* bot)
{
    if (bot->isMoving())
        bot->StopMoving();
    if (bot->IsMounted())
    {
        WorldPacket emptyPacket;
        bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    }
}

// #201 (#612): one "[DUELSPOT] scan <bot> spot=<name> <outcome>" debug line per scan of a harness-pinned bot (the
// status is locked by `.botscenario duelspot`), so a 1:1 run shows why a scan did or did not request. Same text on
// both trees. Free-roaming bots never log it.
// #201 (wave 30): also for a SEEDED bot (its RPG_DUEL_SPOT data has seeded=true), otherwise the t8 pairing leg has no
// scan lines at all. Debug level only.
bool ScanLogged(Player* bot)
{
    if (BotScenario::IsStatusLocked(bot))
        return true;
    PlayerbotAI* ai = GET_PLAYERBOT_AI(bot);
    auto const* ds = ai ? std::get_if<NewRpgInfo::DuelSpot>(&ai->GetRpgInfo().data) : nullptr;
    return ds && ds->seeded;
}

void ScanLog(Player* bot, DuelSpotInfo const& spot, std::string const& what)
{
    if (!ScanLogged(bot))
        return;
    TC_LOG_DEBUG("playerbots", "[DUELSPOT] scan %s spot=%s %s", bot->GetName().c_str(), spot.name.c_str(), what.c_str());
}

// #201 (#612, wave-13 t9, wave-30 t45/t8): AC's RpgDuelTrigger refuses to duel while "all targets" (every hostile within
// sight range, LOS ignored) is non-empty. At a duel spot that blocked the Dalaran Circle of Wills (58-62 hostiles 39+ yd
// away), the Orgrimmar gate (Wild Mature Swine at 33 yd: T4 never challenged on either core) and the Crossroads. For a
// duel-spot scan only a hostile within this range counts: the scan gate and the request decision (AC's trigger is still
// tried first; only when it fails is the same list of checks re-run with the near-only targets rule).
constexpr float RELAXED_SPOT_HOSTILE_DIST = 20.0f;

bool RelaxedSpot(Player* bot, DuelSpotInfo const& spot)
{
    return spot.allowPvpProhibitedZone && sPlayerbotAIConfig->IsInPvpProhibitedZone(bot->GetZoneId());
}

bool AnyTargetWithin(PlayerbotAI* botAI, Player* bot, GuidVector const& targets, float dist)
{
    for (ObjectGuid const& g : targets)
        if (Unit* u = botAI->GetUnit(g))
            if (bot->GetExactDist(u) <= dist)
                return true;
    return false;
}

// "n=<count> first=<name>@<yd>" for the "all targets" value (nearest first entry), for ScanLog only.
std::string TargetsText(PlayerbotAI* botAI, Player* bot, GuidVector const& targets)
{
    Unit* nearest = nullptr;
    for (ObjectGuid const& g : targets)
        if (Unit* u = botAI->GetUnit(g))
            if (!nearest || bot->GetExactDist(u) < bot->GetExactDist(nearest))
                nearest = u;
    char buf[160];
    snprintf(buf, sizeof(buf), "n=%u first=%s@%.0f", uint32(targets.size()),
             nearest ? nearest->GetName().c_str() : "-", nearest ? bot->GetExactDist(nearest) : 0.0f);
    return buf;
}

// #201 (owner 2026-10-20: city duels "separated a little bit from each other, as right now they are right next to each
// other"). MoP keeps its duel-spot bots on a ring around the POI: the duel objective update
// sends a bot back to, and wanders it to, a point urand(10, 30) yd from the POI, and pairs a bot with the nearest eligible player within
// 10 yd. It has no explicit separation between pairs. With 6 pairs per city spot (owner) a new duel is
// only placed DUEL_PAIR_MIN_SEP or more from every other duel of the spot (midpoint to midpoint; the duel flag stands
// at the midpoint), and the patrol keeps the idle bots on the MoP ring, away from running duels.
// DUEL_PAIR_MIN_SEP = MoP's 10 yd pairing distance, so two duels never share one pairing circle. Same values on
// both trees.
constexpr float DUEL_PAIR_MIN_SEP = 10.0f;
constexpr float PATROL_RING_MIN = 10.0f;  // small spots use 40 % of their radius

struct DuelMid
{
    float x, y;
};

// Midpoints of the duels (request, countdown or fight) whose players stand within radius + 20 yd of the spot (the area
// of the MaxDuels count), one per pair.
void CollectDuelMids(Map* map, DuelSpotInfo const& spot, std::vector<DuelMid>& out)
{
    out.clear();
    if (!map || map->GetId() != spot.mapId)
        return;
    float const area = spot.radius + 20.0f;
    for (auto const& ref : map->GetPlayers())
    {
        Player* p = ref.GetSource();
        if (!p || !p->IsInWorld() || !p->duel || p->GetExactDist2d(spot.x, spot.y) > area)
            continue;
        Player* o = p->duel->opponent;
        if (!o)
            continue;
        if (o->GetGUID() < p->GetGUID() && o->GetMapId() == spot.mapId && o->GetExactDist2d(spot.x, spot.y) <= area)
            continue;  // the pair is added from the other side
        out.push_back({(p->GetPositionX() + o->GetPositionX()) * 0.5f, (p->GetPositionY() + o->GetPositionY()) * 0.5f});
    }
}

// Distance from (x, y) to the nearest duel midpoint; FLT_MAX when there is none.
float NearestMid(std::vector<DuelMid> const& mids, float x, float y)
{
    float best = FLT_MAX;
    for (DuelMid const& m : mids)
        best = std::min(best, std::hypot(m.x - x, m.y - y));
    return best;
}

// Where a duel between the challenger and its target will stand: the challenger first walks to 4 yd short of a target
// farther than DUEL_REQUEST_DIST (ScanAndChallenge), otherwise both stay where they are.
void ExpectedDuelMid(Player* bot, Player* target, float& x, float& y)
{
    float bx = bot->GetPositionX();
    float by = bot->GetPositionY();
    float const d2 = bot->GetExactDist2d(target);
    if (bot->GetExactDist(target) > DUEL_REQUEST_DIST && d2 > 0.1f)
    {
        float const k = 4.0f / d2;
        bx = target->GetPositionX() + (bx - target->GetPositionX()) * k;
        by = target->GetPositionY() + (by - target->GetPositionY()) * k;
    }
    x = (bx + target->GetPositionX()) * 0.5f;
    y = (by + target->GetPositionY()) * 0.5f;
}

// #201 (owner 2026-10-20: "only a few of them, like maybe 6 pairs"): the living bots of <team> (TEAM_NEUTRAL = any)
// standing within radius + 5 yd of the spot (the candidate area), with their level range, and every visible player.
// MoP keeps a fixed number of bots per POI. Same rule on both trees.
struct SpotCensus
{
    uint32 bots = 0, players = 0, minLevel = 0, maxLevel = 0;
};

SpotCensus TakeCensus(Map* map, DuelSpotInfo const& spot, TeamId team)
{
    SpotCensus c;
    if (!map || map->GetId() != spot.mapId)
        return c;
    for (auto const& ref : map->GetPlayers())
    {
        Player* p = ref.GetSource();
        if (!p || !p->IsInWorld() || !p->IsAlive() || p->GetExactDist2d(spot.x, spot.y) > spot.radius + 5.0f)
            continue;
        if ((team != TEAM_NEUTRAL && p->GetTeamId() != team) || (p->IsGameMaster() && !p->isGMVisible()))
            continue;
        ++c.players;
        if (IsRealPlayerChar(p))
            continue;
        uint32 const lvl = p->GetLevel();
        c.minLevel = c.bots ? std::min(c.minLevel, lvl) : lvl;
        c.maxLevel = std::max(c.maxLevel, lvl);
        ++c.bots;
    }
    return c;
}
}  // namespace

// ---------------------------------------------------------------------------------------------------------------------
// DuelSpotMgr
// ---------------------------------------------------------------------------------------------------------------------

void DuelSpotMgr::PruneLocked(uint32 nowMs)
{
    // #201 (SPEC amendment A6): a dead seeded bot may not heartbeat while it waits for its revive (30-60 s), so its entry
    // lives 120 s; every other entry 30 s. A pending hand-off lives as long as its registry entry (AC PruneLocked).
    for (auto it = _registry.begin(); it != _registry.end();)
    {
        uint32 const maxAge = it->second.deadSinceMs ? 120000 : 30000;
        if (getMSTimeDiff(it->second.lastBeatMs, nowMs) > maxAge)
            it = _registry.erase(it);
        else
            ++it;
    }
    for (auto it = _pending.begin(); it != _pending.end();)
    {
        if (_registry.find(it->first) == _registry.end())
            it = _pending.erase(it);
        else
            ++it;
    }
    for (auto it = _playerCooldown.begin(); it != _playerCooldown.end();)
    {
        if (Due(nowMs, it->second))
            it = _playerCooldown.erase(it);
        else
            ++it;
    }
}

int32 DuelSpotMgr::SelectSpotFor(Player* bot)
{
    if (!sPlayerbotAIConfig->duelSpotEnable || !bot || !bot->IsInWorld() || !bot->IsAlive())
        return -1;

    Map* map = bot->GetMap();
    if (!map || map->Instanceable())
        return -1;
    if (InQueueOrMatch(bot) || InGroupWithRealPlayer(bot))
        return -1;

    int32 best = -1;
    float bestDist = 0.0f;
    std::vector<DuelSpotInfo> const& spots = sPlayerbotAIConfig->duelSpots;
    for (uint32 i = 0; i < spots.size(); ++i)
    {
        DuelSpotInfo const& spot = spots[i];
        if (spot.mapId != bot->GetMapId() || !TeamMatches(spot.team, bot->GetTeamId()) ||
            !LevelOk(spot, bot->GetLevel()))
            continue;

        float const dist = bot->GetExactDist2d(spot.x, spot.y);
        if (dist > float(sPlayerbotAIConfig->duelSpotSelectDistance))
            continue;
        // #201 (owner 2026-10-20; bots kept at the POI): a spot that already holds its
        // .Bots bots of this team (registered, or standing inside it) takes no more by natural attraction.
        if (spot.bots && (Count(i, bot->GetTeamId()) >= spot.bots ||
                          TakeCensus(map, spot, bot->GetTeamId()).bots >= spot.bots))
            continue;
        if (best < 0 || dist < bestDist)
        {
            best = int32(i);
            bestDist = dist;
        }
    }
    return best;
}

bool DuelSpotMgr::InSpot(WorldObject const* o, uint32 idx, float extra)
{
    std::vector<DuelSpotInfo> const& spots = sPlayerbotAIConfig->duelSpots;
    if (!o || idx >= spots.size())
        return false;
    DuelSpotInfo const& spot = spots[idx];
    // #201 (SPEC amendment A1): 2D everywhere.
    return o->GetMapId() == spot.mapId && o->GetExactDist2d(spot.x, spot.y) <= spot.radius + extra;
}

void DuelSpotMgr::Heartbeat(ObjectGuid bot, uint32 idx, TeamId team, bool seeded, bool dead)
{
    uint32 const now = getMSTime();
    std::lock_guard<std::mutex> lock(_mutex);
    Entry& e = _registry[bot];
    e.idx = idx;
    e.team = team;
    e.seeded = seeded;
    e.lastBeatMs = now;
    e.actionBeatMs = now;
    if (dead)
    {
        if (!e.deadSinceMs)
        {
            e.deadSinceMs = NonZero(now);
            e.reviveAtMs = NonZero(now + urand(30, 60) * 1000);
        }
    }
    else
    {
        e.deadSinceMs = 0;
        e.reviveAtMs = 0;
    }
}

void DuelSpotMgr::Leave(ObjectGuid bot)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _registry.erase(bot);
    _pending.erase(bot);
}

uint32 DuelSpotMgr::Count(uint32 idx, TeamId team)
{
    uint32 const now = getMSTime();
    std::lock_guard<std::mutex> lock(_mutex);
    PruneLocked(now);
    uint32 n = 0;
    for (auto const& kv : _registry)
        if (kv.second.idx == idx && kv.second.team == team)
            ++n;
    return n;
}

bool DuelSpotMgr::IsRegistered(ObjectGuid bot)
{
    std::lock_guard<std::mutex> lock(_mutex);
    return _registry.find(bot) != _registry.end() || _pending.find(bot) != _pending.end();
}

void DuelSpotMgr::SetPending(ObjectGuid bot, uint32 idx, TeamId team)
{
    uint32 const now = getMSTime();
    std::lock_guard<std::mutex> lock(_mutex);
    Pending& p = _pending[bot];
    p.idx = idx;
    p.sinceMs = now;
    // #201 (SPEC amendment A8): as in the AC tree, a placeholder registry entry (seeded) is ALWAYS written, so Count
    // sees the bot at once; the team falls back to the online player's team when the caller passes none.
    if (team == TEAM_NEUTRAL)
    {
        if (Player* online = ObjectAccessor::FindPlayer(bot))
            team = online->GetTeamId();
    }
    Entry& e = _registry[bot];
    e.idx = idx;
    e.team = team;
    e.seeded = true;
    e.lastBeatMs = now;
    e.actionBeatMs = now;
    e.deadSinceMs = 0;
    e.reviveAtMs = 0;
}

bool DuelSpotMgr::TakePending(ObjectGuid bot, uint32& idx)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _pending.find(bot);
    if (it == _pending.end())
        return false;
    idx = it->second.idx;
    _pending.erase(it);
    return true;
}

bool DuelSpotMgr::PlayerOnCooldown(ObjectGuid player, uint32 nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _playerCooldown.find(player);
    return it != _playerCooldown.end() && !Due(nowMs, it->second);
}

void DuelSpotMgr::SetPlayerCooldown(ObjectGuid player, uint32 untilMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    _playerCooldown[player] = NonZero(untilMs);
}

bool DuelSpotMgr::IsDuelInSpot(Player* a, Player* b)
{
    if (!sPlayerbotAIConfig->duelSpotEnable)
        return false;
    for (uint32 i = 0; i < sPlayerbotAIConfig->duelSpots.size(); ++i)
        if (InSpot(a, i, 20.0f) || InSpot(b, i, 20.0f))
            return true;
    return false;
}

void DuelSpotMgr::NoteDuelEnd(Player* a, Player* b)
{
    if (!a || !b || !IsDuelInSpot(a, b))
        return;

    uint32 const now = getMSTime();
    Player* const sides[2][2] = {{a, b}, {b, a}};
    for (auto const& side : sides)
    {
        if (!GET_PLAYERBOT_AI(side[0]))
            continue;  // only bots read the window
        uint32 delaySec = GetRematchOverride(side[0]->GetGUID());
        if (!delaySec)
            delaySec = urand(sPlayerbotAIConfig->duelSpotRematchDelayMin, sPlayerbotAIConfig->duelSpotRematchDelayMax);
        std::lock_guard<std::mutex> lock(_mutex);
        _rematchUntil[side[0]->GetGUID()][side[1]->GetGUID()] = NonZero(now + delaySec * 1000);
    }

    std::lock_guard<std::mutex> lock(_mutex);
    for (auto it = _rematchUntil.begin(); it != _rematchUntil.end();)
    {
        for (auto in = it->second.begin(); in != it->second.end();)
            in = Due(now, in->second) ? it->second.erase(in) : std::next(in);
        it = it->second.empty() ? _rematchUntil.erase(it) : std::next(it);
    }
}

bool DuelSpotMgr::IsRematchBlocked(ObjectGuid bot, ObjectGuid other, uint32 nowMs)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _rematchUntil.find(bot);
    if (it == _rematchUntil.end())
        return false;
    auto in = it->second.find(other);
    if (in == it->second.end())
        return false;
    if (Due(nowMs, in->second))
    {
        it->second.erase(in);
        return false;
    }
    return true;
}

void DuelSpotMgr::SetScanOverride(ObjectGuid bot, uint32 ms)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (ms)
        _scanOverride[bot] = ms;
    else
        _scanOverride.erase(bot);
}

void DuelSpotMgr::SetRematchOverride(ObjectGuid bot, uint32 seconds)
{
    std::lock_guard<std::mutex> lock(_mutex);
    if (seconds)
        _rematchOverride[bot] = seconds;
    else
        _rematchOverride.erase(bot);
}

uint32 DuelSpotMgr::GetScanOverride(ObjectGuid bot)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _scanOverride.find(bot);
    return it == _scanOverride.end() ? 0 : it->second;
}

uint32 DuelSpotMgr::GetRematchOverride(ObjectGuid bot)
{
    std::lock_guard<std::mutex> lock(_mutex);
    auto it = _rematchOverride.find(bot);
    return it == _rematchOverride.end() ? 0 : it->second;
}

int32 DuelSpotMgr::FindSpot(std::string const& nameOrIndex)
{
    std::vector<DuelSpotInfo> const& spots = sPlayerbotAIConfig->duelSpots;
    if (nameOrIndex.empty())
        return -1;

    bool numeric = true;
    for (char c : nameOrIndex)
        numeric = numeric && std::isdigit(static_cast<unsigned char>(c));
    if (numeric && nameOrIndex.size() < 9)
    {
        uint32 const idx = uint32(std::stoul(nameOrIndex));
        return idx < spots.size() ? int32(idx) : -1;
    }

    std::string lower = nameOrIndex;
    std::transform(lower.begin(), lower.end(), lower.begin(), [](unsigned char c) { return std::tolower(c); });
    for (uint32 i = 0; i < spots.size(); ++i)
    {
        std::string name = spots[i].name;
        std::transform(name.begin(), name.end(), name.begin(), [](unsigned char c) { return std::tolower(c); });
        if (name == lower)
            return int32(i);
    }
    return -1;
}

// Seeder (#615). Population + revive. World thread. Reads only the registry
// (never another bot's rpgInfo); the registry lock is never held across TeleportTo / Randomize.
void DuelSpotMgr::Update(uint32 /*diff*/)
{
    if (!sPlayerbotAIConfig->duelSpotEnable || !sPlayerbotAIConfig->duelSpotSeedEnable)
        return;

    // RandomPlayerbotMgr::UpdateAIInternal runs at the manager's own cadence and passes a single frame's diff, so the
    // seed interval is measured on the clock instead of by summing diffs.
    uint32 const now = getMSTime();
    if (_lastSeedMs && getMSTimeDiff(_lastSeedMs, now) < sPlayerbotAIConfig->duelSpotSeedInterval)
        return;
    _lastSeedMs = NonZero(now);

    std::vector<DuelSpotInfo> const& spots = sPlayerbotAIConfig->duelSpots;
    if (spots.empty())
        return;

    // 1. Revive seeded bots that have been dead for urand(30, 60) s.
    std::vector<ObjectGuid> seededGuids;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        PruneLocked(now);
        for (auto const& kv : _registry)
            if (kv.second.seeded)
                seededGuids.push_back(kv.first);
    }
    for (ObjectGuid const& guid : seededGuids)
    {
        Player* bot = sRandomPlayerbotMgr->GetPlayerBot(guid);
        if (!bot || !bot->IsInWorld())
            continue;

        bool revive = false;
        bool leash = false;
        uint32 idx = 0;
        // #201 (#615): the hold, the re-arm and the leash (see SEED_HOLD_EXTRA). A bot taken by a group / BG / arena /
        // queue / LFG (or pinned by a scenario) is neither held nor leashed: its entry ages out and the seeder refills
        // the spot. Evaluated before the registry lock (LFGMgr / scenario state have their own).
        bool const taken = bot->GetGroup() || InQueueOrMatch(bot) || BotScenario::IsStatusLocked(bot);
        {
            std::lock_guard<std::mutex> lock(_mutex);
            auto it = _registry.find(guid);
            if (it == _registry.end())
                continue;
            if (!taken && InSpot(bot, it->second.idx, SEED_HOLD_EXTRA))
            {
                it->second.lastBeatMs = now;
                if (getMSTimeDiff(it->second.actionBeatMs, now) > SEED_REARM_IDLE_MS &&
                    _pending.find(guid) == _pending.end())
                {
                    Pending& p = _pending[guid];
                    p.idx = it->second.idx;
                    p.sinceMs = now;
                    it->second.actionBeatMs = now;
                }
            }
            else if (!taken && bot->IsAlive() && !bot->duel && !bot->IsInCombat() && !bot->IsBeingTeleported() &&
                     !bot->IsInFlight())
            {
                leash = true;
                idx = it->second.idx;
            }
            // #201 (SPEC amendment A6, AC Update): a dead bot's engine does not run the status action, so the first
            // detection stamps the revive time and refreshes the beat once (the entry then lives up to 120 s).
            if (bot->isDead())
            {
                if (!it->second.deadSinceMs)
                {
                    it->second.deadSinceMs = NonZero(now);
                    it->second.reviveAtMs = NonZero(now + urand(30, 60) * 1000);
                    it->second.lastBeatMs = now;
                }
                else if (Due(now, it->second.reviveAtMs))
                {
                    revive = true;
                    idx = it->second.idx;
                }
            }
        }
        if (leash && idx < spots.size())
        {
            // #201 (#615, wave 38): the seeder's leash; the re-armed hand-off restores RPG_DUEL_SPOT after a full
            // Reset (a cross-map teleport resets the AI on arrival). A refused teleport leaves the entry to age out.
            bool const ok = TeleportToSpot(bot, spots[idx]);
            if (ok)
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _registry.find(guid);
                if (it != _registry.end())
                {
                    uint32 const t = getMSTime();
                    it->second.lastBeatMs = t;
                    it->second.actionBeatMs = t;
                    Pending& p = _pending[guid];
                    p.idx = it->second.idx;
                    p.sinceMs = t;
                }
            }
            TC_LOG_DEBUG("playerbots", "[DUELSPOT] leash %s spot=%s ok=%d", bot->GetName().c_str(), spots[idx].name.c_str(),
                         ok ? 1 : 0);
        }
        if (revive && idx < spots.size())
        {
            bot->ResurrectPlayer(1.0f, false);
            bot->SpawnCorpseBones();
            if (PlayerbotAI* reviveAI = GET_PLAYERBOT_AI(bot))
                reviveAI->ResetStrategies();  // AC: ResetStrategies(false)
            TeleportToSpot(bot, spots[idx]);
            {
                std::lock_guard<std::mutex> lock(_mutex);
                auto it = _registry.find(guid);
                if (it != _registry.end())
                {
                    it->second.deadSinceMs = 0;
                    it->second.reviveAtMs = 0;
                    it->second.lastBeatMs = getMSTime();
                }
            }
            TC_LOG_DEBUG("playerbots", "[DUELSPOT] revive %s spot=%s", bot->GetName().c_str(), spots[idx].name.c_str());
        }
    }

    // #201 (owner 2026-10-20): one census line per occupied spot every 30 s, for the t613 t8 rows (max level only, at
    // most .Bots bots, pair separation). Debug level. minsep = the smallest distance between two running duels' midpoints
    // (fights move, so it is informative; the placement rule is the sep= of the request line).
    static uint32 lastCensusMs = 0;
    if (!lastCensusMs || getMSTimeDiff(lastCensusMs, now) >= 30000)
    {
        lastCensusMs = NonZero(now);
        for (DuelSpotInfo const& spot : spots)
        {
            Map* map = sMapMgr->FindBaseNonInstanceMap(spot.mapId);
            SpotCensus const c = TakeCensus(map, spot, TEAM_NEUTRAL);
            if (!c.players)
                continue;
            std::vector<DuelMid> mids;
            CollectDuelMids(map, spot, mids);
            float minSep = FLT_MAX;
            for (size_t i = 0; i < mids.size(); ++i)
                for (size_t j = i + 1; j < mids.size(); ++j)
                    minSep = std::min(minSep, std::hypot(mids[i].x - mids[j].x, mids[i].y - mids[j].y));
            char sepText[16];
            if (minSep == FLT_MAX)
                snprintf(sepText, sizeof(sepText), "-");
            else
                snprintf(sepText, sizeof(sepText), "%.1f", minSep);
            TC_LOG_DEBUG("playerbots", "[DUELSPOT] census spot=%s players=%u bots=%u cap=%u lvl=%u-%u duels=%u minsep=%s",
                         spot.name.c_str(), c.players, c.bots, spot.bots, c.minLevel, c.maxLevel, uint32(mids.size()),
                         sepText);
        }
    }

    // 2. Budget: online random bots * SeedMaxShare / 100, minus the seeded entries already in the registry.
    std::vector<Player*> randomBots;
    for (Player* p : sRandomPlayerbotMgr->GetPlayerBotsSnapshot())
        if (p && p->IsInWorld() && sRandomPlayerbotMgr->IsRandomBot(p))
            randomBots.push_back(p);

    uint32 seededNow = 0;
    {
        std::lock_guard<std::mutex> lock(_mutex);
        for (auto const& kv : _registry)
            if (kv.second.seeded)
                ++seededNow;
    }
    uint32 const cap = uint32(uint64(randomBots.size()) * sPlayerbotAIConfig->duelSpotSeedMaxShare / 100);
    int64 budget = int64(cap) - int64(seededNow);
    uint32 seedsLeft = sPlayerbotAIConfig->duelSpotSeedPerTick;
    if (budget <= 0 || !seedsLeft)
        return;

    // Real players, once per tick, for the "spot active" test.
    std::vector<Player*> realPlayers;
    for (Player* p : sRandomPlayerbotMgr->GetPlayers())
        if (p && p->IsInWorld() && !(p->IsGameMaster() && !p->isGMVisible()))
            realPlayers.push_back(p);

    uint32 const serverMax = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    uint32 const heroicStart = sWorld->getIntConfig(CONFIG_START_HEROIC_PLAYER_LEVEL);

    // 3. Each enabled spot with bots > 0, in list order.
    for (uint32 idx = 0; idx < spots.size() && budget > 0 && seedsLeft > 0; ++idx)
    {
        DuelSpotInfo const& spot = spots[idx];
        if (!spot.bots)
            continue;
        uint32 const minLevel = EffectiveMinLevel(spot);
        if (minLevel > spot.maxLevel)
            continue;

        bool active = spot.alwaysActive;
        for (size_t i = 0; !active && i < realPlayers.size(); ++i)
            active = realPlayers[i]->GetMapId() == spot.mapId &&
                     realPlayers[i]->GetExactDist2d(spot.x, spot.y) <= float(sPlayerbotAIConfig->duelSpotActiveDistance);
        if (!active)
            continue;

        for (uint8 allowed = 1; allowed <= 2 && budget > 0 && seedsLeft > 0; ++allowed)
        {
            if (spot.team != 0 && spot.team != allowed)
                continue;
            TeamId const teamId = allowed == 1 ? TEAM_ALLIANCE : TEAM_HORDE;

            while (budget > 0 && seedsLeft > 0)
            {
                // #201 (owner 2026-10-20): registered bots, or bots standing inside (a bot in a long duel stops its
                // heartbeat), whichever is larger.
                uint32 const have = std::max(Count(idx, teamId),
                                             TakeCensus(sMapMgr->FindBaseNonInstanceMap(spot.mapId), spot, teamId).bots);
                if (have >= spot.bots)
                    break;

                // Eligible bots of this team: in world, alive, not teleporting, not in combat, not in a BG / arena /
                // queue / LFG, not grouped, on a non-instanceable map, not already registered, not scenario-pinned.
                std::vector<Player*> sameMap, otherMap, anyLevel;
                for (Player* p : randomBots)
                {
                    // #201 (#615, wave 32): never a bot with a duel (request / countdown / fight; not IsInCombat until
                    // the first hit): MoP Zerrin was re-seeded - Reset(true) + teleport - inside its duel at the spot.
                    if (p->GetTeamId() != teamId || !p->IsInWorld() || !p->IsAlive() || p->IsBeingTeleported() ||
                        p->IsInCombat() || p->duel || p->GetGroup() || InQueueOrMatch(p))
                        continue;
                    Map* map = p->GetMap();
                    if (!map || map->Instanceable())
                        continue;
                    if (BotScenario::IsStatusLocked(p) || IsRegistered(p->GetGUID()))
                        continue;

                    if (LevelOk(spot, p->GetLevel()))
                        (p->GetMapId() == spot.mapId ? sameMap : otherMap).push_back(p);
                    else if (sPlayerbotAIConfig->duelSpotSeedRelevel &&
                             (p->GetClass() != CLASS_DEATH_KNIGHT || spot.maxLevel >= heroicStart))
                        anyLevel.push_back(p);
                }

                // Prefer: in range on the same map (nearest), in range on another map, then any (re-level).
                // #201 (SPEC amendment A9): a uniformly random bot of the first non-empty tier (AC), not the nearest.
                std::vector<Player*>& tier = !sameMap.empty() ? sameMap : (!otherMap.empty() ? otherMap : anyLevel);
                if (tier.empty())
                    break;  // nobody left for this spot / team
                Player* pick = tier[urand(0, tier.size() - 1)];

                // 4. Prepare.
                if (!LevelOk(spot, pick->GetLevel()))
                {
                    uint32 newLevel = urand(minLevel, spot.maxLevel);
                    if (pick->GetClass() == CLASS_DEATH_KNIGHT)
                        newLevel = std::min(std::max(newLevel, heroicStart), spot.maxLevel);
                    newLevel = std::min(newLevel, serverMax);
                    sRandomPlayerbotMgr->SetValue(pick, "level", newLevel);
                    BotFactory factory(pick, newLevel);
                    factory.Randomize(false);
                }
                else if (sPlayerbotAIConfig->duelSpotSeedRegear)
                {
                    BotFactory factory(pick, pick->GetLevel());
                    factory.Randomize(true);
                }
                // What RandomPlayerbotMgr::Refresh does for gear/health/mana, without its AI reset.
                pick->DurabilityRepairAll(false, 1.0f, false);
                pick->SetFullHealth();
                if (pick->GetMaxPower(POWER_MANA) > 0)
                    pick->SetPower(POWER_MANA, pick->GetMaxPower(POWER_MANA));
                if (pick->GetMaxPower(POWER_ENERGY) > 0)
                    pick->SetPower(POWER_ENERGY, pick->GetMaxPower(POWER_ENERGY));

                // #201 (SPEC amendment A9): AC sequence - AI reset, teleport, then the pending hand-off.
                if (PlayerbotAI* pickAI = GET_PLAYERBOT_AI(pick))
                    pickAI->Reset(true);
                TeleportToSpot(pick, spot);
                SetPending(pick->GetGUID(), idx, teamId);
                TC_LOG_INFO("playerbots", "[DUELSPOT] seed %s -> %s lvl %u", pick->GetName().c_str(), spot.name.c_str(),
                            pick->GetLevel());

                --budget;
                --seedsLeft;
            }
        }
    }
}

// ---------------------------------------------------------------------------------------------------------------------
// NewRpgDuelSpotAction (SPEC 2.2)
// ---------------------------------------------------------------------------------------------------------------------

void NewRpgDuelSpotAction::TeleportAroundSpot(DuelSpotInfo const& spot)
{
    TeleportToSpot(bot, spot);
}

bool NewRpgDuelSpotAction::Patrol(DuelSpotInfo const& spot)
{
    // 1/3 odds: pause (AC NewRpgOutdoorPvpAction::PatrolCapturePoint). Otherwise walk to a random point within
    // radius * 0.8 of the anchor. This fork's MoveRandomNear has no centre argument, so MoveTo is used directly.
    if (urand(0, 2) == 0)
        return ForceToWait(urand(3000, 6000));

    // #201 (owner 2026-10-20, see DUEL_PAIR_MIN_SEP): identical to the AC tree - MoP's ring, PATROL_RING_MIN (small
    // spots: 40 % of the radius) to radius yd from the anchor (MoP wanders urand(10, 30) yd from the POI). Of 6
    // random ring points the ones DUEL_PAIR_MIN_SEP or more from every running duel come first; up to 4 are tried.
    std::vector<DuelMid> mids;
    CollectDuelMids(bot->GetMap(), spot, mids);
    float const ringMin = std::min(PATROL_RING_MIN, spot.radius * 0.4f);
    struct PatrolPoint
    {
        float x, y, z, room;
    };
    std::vector<PatrolPoint> points;
    for (int i = 0; i < 6; ++i)
    {
        PatrolPoint pt;
        float po;
        RandomPointAround(spot, ringMin, spot.radius, bot->GetMap(), bot->GetPhaseMask(), pt.x, pt.y, pt.z, po);
        pt.room = std::min(NearestMid(mids, pt.x, pt.y), DUEL_PAIR_MIN_SEP);
        points.push_back(pt);
    }
    std::stable_sort(points.begin(), points.end(),
                     [](PatrolPoint const& a, PatrolPoint const& b) { return a.room > b.room; });
    for (int attempt = 0; attempt < 4; ++attempt)
    {
        PatrolPoint const& pt = points[attempt];
        if (MoveTo(spot.mapId, pt.x, pt.y, pt.z, false, false, false, true, MovementPriority::MOVEMENT_NORMAL))
            return true;
    }
    return ForceToWait(urand(3000, 6000));
}

bool NewRpgDuelSpotAction::Execute(Event /*event*/)
{
    NewRpgInfo& info = botAI->GetRpgInfo();
    auto* dataPtr = std::get_if<NewRpgInfo::DuelSpot>(&info.data);
    if (!dataPtr)
        return false;
    NewRpgInfo::DuelSpot& data = *dataPtr;
    ObjectGuid const guid = bot->GetGUID();

    // 1. Invalid index: back to idle.
    if (data.spotIndex >= sPlayerbotAIConfig->duelSpots.size())
    {
        info.ChangeToIdle();
        sDuelSpotMgr->Leave(guid);
        return false;
    }
    DuelSpotInfo const& spot = sPlayerbotAIConfig->duelSpots[data.spotIndex];
    uint32 const now = getMSTime();

    // 2. Registry heartbeat.
    sDuelSpotMgr->Heartbeat(guid, data.spotIndex, bot->GetTeamId(), data.seeded, !bot->IsAlive());

    // 3. AcceptDuelAction -> ResetStrategies() re-rolls AC's 25 % "start duel", so a seeded bot redoes it every tick.
    if (data.seeded && !botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT))
        botAI->ChangeStrategy("+start duel", BOT_STATE_NON_COMBAT);

    // #201 (PM reconciliation): AC returns here when the bot is dead, not in world or being teleported.
    if (!bot->IsAlive() || !bot->IsInWorld() || bot->IsBeingTeleported())
        return false;

    // Expired no-rematch entries.
    for (auto it = data.noRematchUntil.begin(); it != data.noRematchUntil.end();)
    {
        if (Due(now, it->second))
            it = data.noRematchUntil.erase(it);
        else
            ++it;
    }

    // 4. In a duel: remember the opponent, push the next scan out, never move (as in MoP).
    if (bot->duel)
    {
        if (Player* opponent = bot->duel->opponent)
        {
            uint32 const overrideSec = sDuelSpotMgr->GetRematchOverride(guid);
            uint32 const delaySec =
                overrideSec ? overrideSec
                            : urand(sPlayerbotAIConfig->duelSpotRematchDelayMin, sPlayerbotAIConfig->duelSpotRematchDelayMax);
            data.noRematchUntil[opponent->GetGUID()] = NonZero(now + delaySec * 1000);
        }
        data.nextScanMs = NonZero(now + 10000);
        return false;
    }

    // 5. Wrong map, or 250 yd or more away (2D). Seeded: teleport back (as in MoP). Not
    // seeded: another map ends the status; same map and far away: walk (step 6) because the select distance is 1500 yd
    // (SPEC amendment A2, the rule is the same on both trees).
    bool const sameMap = bot->GetMapId() == spot.mapId;
    float const dist = sameMap ? bot->GetExactDist2d(spot.x, spot.y) : FLT_MAX;
    if (!sameMap || (data.seeded && dist >= 250.0f))
    {
        if (data.seeded)
        {
            TeleportToSpot(bot, spot);
            return true;
        }
        info.ChangeToIdle();
        sDuelSpotMgr->Leave(guid);
        return true;
    }

    // 6. Too far from the spot: walk there. A stable per-bot point inside radius * 0.6 (golden-ratio hash of the GUID
    // counter), so MoveFarTo's stuck tracking is not reset every tick (SPEC amendment A7, same as AC).
    if (dist > spot.radius + 15.0f)
    {
        double const g = static_cast<double>(guid.GetCounter());
        float const angle = Frac(g * 0.6180339887) * 2.0f * static_cast<float>(M_PI);
        float const d = spot.radius * 0.6f * std::sqrt(Frac(g * 0.7548776662));
        float const jx = spot.x + d * std::cos(angle);
        float const jy = spot.y + d * std::sin(angle);
        float jz = spot.z;
        float const h = bot->GetMap()->GetHeight(bot->GetPhaseMask(), jx, jy, spot.z + 10.0f);
        if (h > INVALID_HEIGHT && std::fabs(h - spot.z) <= 30.0f)
            jz = h + 0.2f;
        return MoveFarTo(WorldPosition(spot.mapId, jx, jy, jz, 0.0f));
    }

    // 7.
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_NORMAL))
        return false;

    // 8. Before the next scan: patrol.
    if (!Due(now, data.nextScanMs))
        return Patrol(spot);

    // 9. Scan time.
    uint32 const scanOverride = sDuelSpotMgr->GetScanOverride(guid);
    data.nextScanMs = NonZero(
        now + (scanOverride ? scanOverride
                            : urand(sPlayerbotAIConfig->duelSpotScanIntervalMin, sPlayerbotAIConfig->duelSpotScanIntervalMax)));
    // #201 (#612): each gate is named in the scan line (ScanLog).
    if (bot->IsInCombat())
    {
        ScanLog(bot, spot, "gate=combat");
        return false;
    }
    if (bot->HasUnitState(UNIT_STATE_CASTING))
    {
        ScanLog(bot, spot, "gate=casting");
        return false;
    }
    if (bot->GetStandState() == UNIT_STAND_STATE_SIT)
    {
        ScanLog(bot, spot, "gate=sit");
        return false;
    }
    if (!bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND) &&
        !bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_CHEST))
    {
        ScanLog(bot, spot, "gate=nogear");
        return false;
    }
    GuidVector const allTargets = AI_VALUE(GuidVector, "all targets");
    // #201 (waves 13/30): only a hostile within RELAXED_SPOT_HOSTILE_DIST blocks a duel-spot scan (see the constant).
    if (!allTargets.empty() && AnyTargetWithin(botAI, bot, allTargets, RELAXED_SPOT_HOSTILE_DIST))
    {
        ScanLog(bot, spot, "gate=targets " + TargetsText(botAI, bot, allTargets));
        return false;
    }

    // 10 - 12.
    return ScanAndChallenge(data, spot);
}

// AiPlayerbot.DuelSpotDifferentClassOnly: duel-spot challenges skip candidates of the bot's own class, so the duels
// at the city gates are mixed matchups. Read once through sConfigMgr, like AiPlayerbot.RpgTownErrandEnable.
static bool DuelSpotDifferentClassOnly()
{
    static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.DuelSpotDifferentClassOnly", true);
    return v;
}

bool NewRpgDuelSpotAction::ScanAndChallenge(NewRpgInfo::DuelSpot& data, DuelSpotInfo const& spot)
{
    Map* map = bot->GetMap();
    if (!map)
        return false;

    uint32 const now = getMSTime();
    uint32 const serverMax = sWorld->getIntConfig(CONFIG_MAX_PLAYER_LEVEL);
    struct Candidate
    {
        Player* player;
        float dist;
    };
    std::vector<Candidate> candidates;
    uint32 duelers = 0;
    for (auto const& ref : map->GetPlayers())
    {
        Player* p = ref.GetSource();
        if (!p || !p->IsInWorld())
            continue;
        // #201 (SPEC amendments A1/A4): 2D, InSpot with the same extras as AC (5 yd candidates, 20 yd duel count).
        if (p->duel && sDuelSpotMgr->InSpot(p, data.spotIndex, 20.0f))
            ++duelers;
        if (sDuelSpotMgr->InSpot(p, data.spotIndex, 5.0f))
            candidates.push_back({p, bot->GetExactDist2d(p)});
    }
    // Duels currently inside the spot: players within radius + 20 with a duel, divided by 2.
    if (duelers / 2 >= sPlayerbotAIConfig->duelSpotMaxDuels)
    {
        ScanLog(bot, spot, "full duels=" + std::to_string(duelers / 2));
        return false;
    }
    // #201 (owner 2026-10-20): the running duels of this spot, for the pair separation (DUEL_PAIR_MIN_SEP).
    std::vector<DuelMid> mids;
    CollectDuelMids(map, spot, mids);

    std::sort(candidates.begin(), candidates.end(),
              [](Candidate const& a, Candidate const& b) { return a.dist < b.dist; });

    // #201 (#612): the skipped candidates and why, for the scan line only.
    std::string skipped;
    auto skip = [&skipped](Player* p, std::string const& why)
    {
        skipped += (skipped.empty() ? " skip=" : ",") + p->GetName() + ":" + why;
    };
    // The first check of AC's RpgDuelTrigger::IsActive (this fork RpgTriggers.cpp:359-413, AC RpgTriggers.cpp) that
    // fails for this candidate, in the same order; evaluated only to name the reason in the scan line, never to decide.
    // #201 (wave-13 t9): nearOnly (RelaxedSpot) = the targets check only counts hostiles within RELAXED_SPOT_HOSTILE_DIST;
    // returns "" when every check passes, which is then the decision for a relaxed spot (AC's trigger is not called).
    auto triggerWhy = [&](Player* p, bool nearOnly) -> std::string
    {
        if (!botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT))
            return "nostartduel";
        if (bot->GetLevel() < 3)
            return "lowlevel";
        if (botAI->HasRealPlayerMaster() && botAI->GetMaster() && !botAI->GetMaster()->isAFK())
            return "master";
        uint8 const hp = AI_VALUE2(uint8, "health", "self target");
        if (hp < 90)
            return "hp" + std::to_string(hp);
        if (p->GetLevel() > bot->GetLevel() + 3 || bot->GetLevel() > p->GetLevel() + 10)
            return "level";
        if (bot->duel || p->duel || !p->GetSocial() || p->GetSocial()->HasIgnore(bot->GetGUID()))
            return "social";
        AreaTableEntry const* area = sAreaTableStore.LookupEntry(p->GetAreaId());
        if (area && !(area->Flags & AREA_FLAG_ALLOW_DUELS))
            return "area" + std::to_string(p->GetAreaId());
        GuidVector const targets = AI_VALUE(GuidVector, "all targets");
        if (!targets.empty() && (!nearOnly || AnyTargetWithin(botAI, bot, targets, RELAXED_SPOT_HOSTILE_DIST)))
        {
            std::string t = TargetsText(botAI, bot, targets);
            std::replace(t.begin(), t.end(), ' ', '_'); // one token in the skip list
            return "targets(" + t + ")";
        }
        return "";
    };
    bool const relaxedSpot = RelaxedSpot(bot, spot);
    uint32 others = 0;
    for (Candidate const& c : candidates)
        if (c.player != bot)
            ++others;

    for (Candidate const& c : candidates)
    {
        Player* p = c.player;
        if (p == bot)
            continue;
        if (!p->IsAlive() || p->duel || p->GetStandState() == UNIT_STAND_STATE_SIT)
        {
            skip(p, "busy");
            continue;
        }
        if (sPlayerbotAIConfig->duelSpotSameTeamOnly && p->GetTeamId() != bot->GetTeamId())
        {
            skip(p, "team");
            continue;
        }
        if (DuelSpotDifferentClassOnly() && p->GetClass() == bot->GetClass())
        {
            skip(p, "class");
            continue;
        }

        uint32 const botLevel = bot->GetLevel(), pLevel = p->GetLevel();
        uint32 const diff = botLevel > pLevel ? botLevel - pLevel : pLevel - botLevel;
        if (diff > sPlayerbotAIConfig->duelSpotMaxLevelDiff ||
            (sPlayerbotAIConfig->duelSpotMaxLevelPairsOnly && ((botLevel >= serverMax) != (pLevel >= serverMax))))
        {
            skip(p, "level");
            continue;
        }
        if (data.noRematchUntil.find(p->GetGUID()) != data.noRematchUntil.end() ||
            sDuelSpotMgr->IsRematchBlocked(bot->GetGUID(), p->GetGUID(), now))  // #201: hook-recorded window
        {
            skip(p, "rematch");
            continue;
        }
        // #201 (owner 2026-10-20): the new duel stands DUEL_PAIR_MIN_SEP or more from every running duel of the spot.
        float midX, midY;
        ExpectedDuelMid(bot, p, midX, midY);
        float const sep = NearestMid(mids, midX, midY);
        if (sep < DUEL_PAIR_MIN_SEP)
        {
            skip(p, "crowd");
            continue;
        }

        bool const real = IsRealPlayerChar(p);
        if (real)
        {
            if (!sPlayerbotAIConfig->duelSpotChallengeRealPlayers || sDuelSpotMgr->PlayerOnCooldown(p->GetGUID(), now) ||
                urand(1, 100) > sPlayerbotAIConfig->duelSpotPlayerChallengeChance)
            {
                skip(p, "real");
                continue;
            }
            if (p->isAFK() || p->IsInCombat() || p->IsMounted() || p->HasUnitState(UNIT_STATE_CASTING))
            {
                skip(p, "realbusy");
                continue;
            }
        }
        // #201 (SPEC amendment A9): invisible GMs are never candidates (real or not), on both trees.
        if (p->IsGameMaster() && !p->isGMVisible())
            continue;

        // 11. The request is AC's own code path: "rpg target" + the unmodified "rpg duel" trigger / action.
        SET_AI_VALUE(GuidPosition, "rpg target", GuidPosition(p));
        // AC's trigger first (not in a PvP-prohibited spot such as Dalaran, where it can never be active); when it fails,
        // the same checks with the near-only targets rule decide (#201 waves 13/30).
        bool active = false;
        if (!relaxedSpot)
        {
            Trigger* trigger = context->GetTrigger("rpg duel");
            active = trigger && trigger->IsActive();
        }
        if (!active)
            active = triggerWhy(p, true).empty();
        if (!active)
        {
            if (ScanLogged(bot))
            {
                std::string const why = triggerWhy(p, true);
                skip(p, "trigger-" + (why.empty() ? std::string("?") : why));
            }
            RESET_AI_VALUE(GuidPosition, "rpg target");
            continue;
        }

        // #201 (#612): walk into 7266's range first (DUEL_REQUEST_DIST); the scan after the walk comes 1.5 s later (step
        // 7 waits for the walk). A point 4 yd short of the target, on the line from the target to the bot.
        float const reqDist = bot->GetExactDist(p);
        if (reqDist > DUEL_REQUEST_DIST)
        {
            RESET_AI_VALUE(GuidPosition, "rpg target");
            float const d2 = bot->GetExactDist2d(p);
            float const k = d2 > 0.1f ? 4.0f / d2 : 0.0f;
            float const ax = p->GetPositionX() + (bot->GetPositionX() - p->GetPositionX()) * k;
            float const ay = p->GetPositionY() + (bot->GetPositionY() - p->GetPositionY()) * k;
            data.nextScanMs = NonZero(now + 1500);
            char buf[96];
            snprintf(buf, sizeof(buf), "approach=%s dist=%.1f", p->GetName().c_str(), reqDist);
            ScanLog(bot, spot, buf + skipped);
            MoveTo(spot.mapId, ax, ay, p->GetPositionZ(), false, false, false, true, MovementPriority::MOVEMENT_NORMAL);
            return true;
        }

        // #201 (#612): 7266 cannot be cast mounted (Server.log "[CAST] ... spell=7266 ... result=67"), and AC's
        // CastCustomSpellAction only turns the caster and returns true WITHOUT casting when the target is not in front
        // (this fork CastCustomSpellAction.cpp:158-166, AC :133-141), so dismount and face the target first.
        DismountForDuel(bot);
        sServerFacade->SetFacingTo(bot, p);

        bool requested = false;
        if (relaxedSpot)
        {
            // RpgDuelAction::isUseful would refuse inside a PvP-prohibited zone (Dalaran); do its caster-area check
            // here and then exactly RpgDuelAction::Execute (AC RpgSubActions.cpp:486-495).
            AreaTableEntry const* casterArea = sAreaTableStore.LookupEntry(bot->GetAreaId());
            if (!casterArea || (casterArea->Flags & AREA_FLAG_ALLOW_DUELS))
                requested = botAI->DoSpecificAction("cast custom spell",
                                                    Event("rpg action", RpgFormatWorldobject(p) + " 7266"), true);
        }
        else
            requested = botAI->DoSpecificAction("rpg duel", Event("rpg duel"), true);
        RESET_AI_VALUE(GuidPosition, "rpg target");

        if (requested)
        {
            if (real)
                sDuelSpotMgr->SetPlayerCooldown(
                    p->GetGUID(), now + sPlayerbotAIConfig->duelSpotPlayerCooldown * 1000);
            // #201 (owner 2026-10-20): sep = distance from this duel's midpoint to the nearest other duel ("-" = none).
            char sepText[16];
            if (sep == FLT_MAX)
                snprintf(sepText, sizeof(sepText), "-");
            else
                snprintf(sepText, sizeof(sepText), "%.1f", sep);
            TC_LOG_DEBUG("playerbots", "[DUELSPOT] %s -> %s spot=%s sep=%s", bot->GetName().c_str(), p->GetName().c_str(),
                         spot.name.c_str(), sepText);
        }
        char buf[96];
        snprintf(buf, sizeof(buf), "req=%s dist=%.1f ok=%d", p->GetName().c_str(), reqDist, requested ? 1 : 0);
        ScanLog(bot, spot, buf + skipped);
        // #201 (SPEC amendment A5): return true after the first request (first candidate with an active trigger), as AC.
        return true;
    }
    ScanLog(bot, spot, "nocand n=" + std::to_string(others) + skipped);
    return false;
}
