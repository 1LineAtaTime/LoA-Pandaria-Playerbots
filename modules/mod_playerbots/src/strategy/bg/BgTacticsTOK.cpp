/*
 * This file is part of the mod-playerbots module. See AUTHORS file for Copyright information;
 * released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Temple of Kotmogu (map 998, BATTLEGROUND_TOK) bot tactics - issue #577, epic #580.
 * AC has no spec for this map; this extends AC's WSG/EotS carrier pattern
 * (AC BattleGroundTactics.cpp: WSG selectObjective 2144-2311, atFlag pick-up 3561-3965,
 * moveToStart 1719-1840).
 *
 * Build 14 (#577): the scoring-zone area triggers 7734 / 7735 are now sent for the bot (TokSyncZone below) - they are
 * client-driven, so without it a carrier at the temple centre still scored the OUT rate (3 per tick, not 5).
 *
 * Owner 2026-10-08: divide and conquer like AV, randomized per match and per bot. AC's AV split (AC
 * BattleGroundTactics.cpp:1865-1910: a per-match team strategy from GetBotStrategyForTeam sets how many "bg role"
 * ranks defend, and the per-bot role - urand(0, 9) at join, BattleGroundJoinAction.cpp - picks the bot's job) is
 * extended here: the team's TOKBotStrategy (weighted per-match roll, BattleGroundTactics.cpp RollTokStrategy) picks a
 * TOK_PLANS row - orb takers (split over the west and east orbs), the centre group (with our carriers / the scoring
 * pit), hunters (the enemy carriers) - and the bot's role picks its group. The 1:1 harness pins both (role i % 10,
 * strat=<a>,<h>), so a 1:1 stays deterministic. Evidence: "[BGTOK] strategy ..." (per match) and "[BGTOK] plan ..."
 * (per bot, 30 s).
 */

#include "BattleGroundTactics.h"
#include "BgRealType.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundTOK.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DBCStructure.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include "Timer.h"
#include <cmath>
#include <map>
#include <mutex>

namespace
{
    // Orb stand positions: BattlegroundTOK.h m_orbPositions (index = entry - 212091:
    // 0 blue, 1 purple, 2 green, 3 orange). Ground z is the MoP navmesh surface under the stand
    // (stand z 13.57 is above the floor; the mesh edge is ~1.3 yd from the stand).
    float const TOK_ORB_X[BG_TOK_MAX_ORBS] = {1716.83f, 1850.26f, 1716.78f, 1850.29f};
    float const TOK_ORB_Y[BG_TOK_MAX_ORBS] = {1249.93f, 1416.77f, 1416.64f, 1250.31f};
    float const TOK_ORB_GROUND_Z = 10.774f;

    // Temple centre: midpoint of the four orb stands. BattlegroundTOK::UpdateScore awards 3 / 4 / 5
    // points per 5 s tick for zone OUT / IN / MIDDLE, so the middle is the best place to hold.
    // Navmesh surface at the centre is z=5.708 (the pit).
    float const TOK_CENTRE_X = 1783.5f;
    float const TOK_CENTRE_Y = 1333.0f;
    float const TOK_CENTRE_Z = 5.708f;

    // Start rooms (#577, build 14). The room is the narrow raised ramp BEHIND each door: team start locations are
    // WorldSafeLocs 4059 (Alliance 1784.27,1077.63,28.88) and 4060 (Horde 1780.42,1598.72,33.78), the doors sit at
    // y 1100.66 / 1570.22 (core m_doorPositions) and the ramp floor falls from the start location to the door
    // (mmtile_probe, map 998: Alliance z 27.0 / 26.1 / 24.5 / 22.9 at y 1084 / 1088 / 1092 / 1096; Horde z 30.0 / 31.1 /
    // 32.1 / 33.2 at y 1584 / 1588 / 1592 / 1596), about 12 yd wide. The old spots (y 1120 / 1550) were on the TEMPLE side of
    // the doors and 43 / 49 yd from the start location, while battleground_template.StartMaxDist for map 998 is 30: the
    // WAIT_JOIN guard in BGTactics::Execute therefore pulled every bot back (the #227 "pull-back teleport") each time it
    // walked through the closed door. These are 13 / 11 yd from the start location, 10 / 18 yd short of the door.
    Position const TOK_WAIT_ALLIANCE = {1784.3f, 1090.0f, 25.3f, 0.0f};
    Position const TOK_WAIT_HORDE = {1780.4f, 1588.0f, 31.1f, 0.0f};

    uint32 const TOK_ORB_ENTRY_FIRST = BG_TOK_OBJECT_ORB_1_ENTRY;

    // Per TOKBotStrategy (BattleGroundTactics.h): role ranks [0, takers) take orbs - the first `west` of them the west
    // orbs (blue, green; x 1716), the rest the east orbs (purple, orange; x 1850) - [takers, takers + centre) hold the
    // centre with our carriers, the rest hunt the enemy carriers. BALANCED is the pre-split behaviour (roles 0-4 orbs,
    // 5-6 escort first, 7-9 hunt first) with the takers now spread 3:2 over both sides.
    struct TokPlan
    {
        uint8 takers;
        uint8 west;
        uint8 centre;
        char const* name;
    };

    TokPlan const TOK_PLANS[TOK_STRATEGY_MAX] = {
        {5, 3, 2, "balanced"},  // 3 west, 2 east, 2 centre, 3 hunt
        {5, 4, 2, "west"},      // 4 west, 1 east
        {5, 1, 2, "east"},      // 1 west, 4 east
        {7, 4, 1, "orbs"},      // 4 west, 3 east, 1 centre, 2 hunt
        {3, 2, 4, "centre"},    // 2 west, 1 east, 4 centre, 3 hunt
        {3, 1, 2, "hunt"}};     // 1 west, 2 east, 2 centre, 5 hunt

    enum TokGroup : uint8
    {
        TOK_GROUP_WEST = 0,
        TOK_GROUP_EAST = 1,
        TOK_GROUP_CENTRE = 2,
        TOK_GROUP_HUNT = 3
    };

    char const* const TOK_GROUP_NAMES[4] = {"orb-west", "orb-east", "centre", "hunt"};

    TokPlan const& PlanOf(uint8 strategy)
    {
        return TOK_PLANS[strategy < TOK_STRATEGY_MAX ? strategy : uint8(TOK_STRATEGY_BALANCED)];
    }

    TokGroup GroupOf(TokPlan const& plan, uint8 role)
    {
        if (role < plan.takers)
            return role < plan.west ? TOK_GROUP_WEST : TOK_GROUP_EAST;

        return role < plan.takers + plan.centre ? TOK_GROUP_CENTRE : TOK_GROUP_HUNT;
    }

    BattlegroundTOK* AsTok(Battleground* bg)
    {
        if (!bg)
            return nullptr;
        BattlegroundTypeId t = RealBgTypeId(bg); // #896
        return t == BATTLEGROUND_TOK ? static_cast<BattlegroundTOK*>(bg) : nullptr;
    }

    bool IsOrbEntry(uint32 entry)
    {
        return entry >= TOK_ORB_ENTRY_FIRST && entry < TOK_ORB_ENTRY_FIRST + BG_TOK_MAX_ORBS;
    }

    // ---- Scoring zones (the "centre bonus") -------------------------------------------------------------
    // BattlegroundTOK::UpdateScore pays a keeper 3 / 4 / 5 points per 5 s for being OUT of the temple / IN it /
    // in the MIDDLE, and the zone is only ever changed by BattlegroundTOK::HandleAreaTrigger: area trigger 7734
    // (the temple box) toggles OUT <-> IN and 7735 (the pit box) toggles IN <-> MIDDLE. A real client sends those
    // triggers as CMSG_AREATRIGGER when it crosses a box edge; a bot has no client, so (like the Warsong capture
    // trigger and the Deepwind cart trigger, see the note at the top of BattleGroundTactics.cpp) nothing ever called
    // them for a bot and every orb it held scored the OUT rate. TokSyncZone sends them for the bot.
    uint32 const TOK_TRIGGER_TEMPLE = 7734;  // AreaTrigger.dbc: box 145 x 178 x 40 around the four orb stands
    uint32 const TOK_TRIGGER_PIT = 7735;     // AreaTrigger.dbc: box 66 x 91 x 5 round the centre pit floor

    enum TokShadowZone : uint8
    {
        TOK_SHADOW_OUT = 0,
        TOK_SHADOW_IN = 1,
        TOK_SHADOW_MIDDLE = 2
    };

    // The core keeps the zone in a private map, so each bot keeps a mirror of it. It starts OUT, as
    // BattlegroundTOK::AddPlayer sets it, and only changes by the same two toggles the core applies.
    // #577 (2026-10-12): the mirror was thread_local. Bot AI runs inside Player::Update (mod_playerbots.cpp:322-326,
    // OnAfterUpdate), the test server runs MapUpdate.Threads = 4, and MapUpdater hands each map update to whichever worker
    // pops it (MapUpdater.cpp:84/111), so a bot's ticks move between threads and each thread kept its own stale copy.
    // HandleAreaTrigger is a blind toggle (BattlegroundTOK.cpp:265-273: 7734 flips OUT <-> IN, 7735 flips IN <-> MIDDLE),
    // so a stale copy flipped the core zone the WRONG way (a carrier in the pit dropped to OUT: 3 points instead of 5).
    // One process-wide mirror under a lock; the bots of one instance share one map update at a time, so the lock is
    // uncontended in practice. (The other thread_local helpers in these files only cache choices; a stale copy there
    // costs a re-pick, not a wrong core state.)
    struct TokZoneMirror
    {
        uint32 instanceId = 0;
        uint8 zone = TOK_SHADOW_OUT;
    };
    std::mutex s_tokZoneLock;
    std::map<ObjectGuid, TokZoneMirror> s_tokZone;

    char const* const TOK_SHADOW_NAMES[3] = {"out", "in", "middle"};

    // Bring the core's zone for this bot in line with where it actually stands. Two toggles at most.
    void TokSyncZone(Player* bot, Battleground* bg, bool keeper)
    {
        // HandleAreaTrigger ignores everything outside IN_PROGRESS, so the mirror must not move then either.
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
            return;

        AreaTriggerEntry const* temple = sAreaTriggerStore.LookupEntry(TOK_TRIGGER_TEMPLE);
        AreaTriggerEntry const* pit = sAreaTriggerStore.LookupEntry(TOK_TRIGGER_PIT);
        if (!temple || !pit)
            return;

        std::lock_guard<std::mutex> guard(s_tokZoneLock);
        TokZoneMirror& mirror = s_tokZone[bot->GetGUID()];
        if (mirror.instanceId != bg->GetInstanceID())
        {
            mirror.instanceId = bg->GetInstanceID();
            mirror.zone = TOK_SHADOW_OUT;
        }

        // The same box test the core's CMSG_AREATRIGGER handler applies (Player::IsInAreaTrigger).
        uint8 wanted = TOK_SHADOW_OUT;
        if (bot->IsInAreaTrigger(temple))
            wanted = bot->IsInAreaTrigger(pit) ? TOK_SHADOW_MIDDLE : TOK_SHADOW_IN;

        // Evidence (#577): an orb keeper's scoring zone as the core now sees it (UpdateScore pays 3 / 4 / 5 per 5 s).
        if (keeper && mirror.zone != wanted)
            TC_LOG_DEBUG("playerbots", "[BGTOK] keeper-zone bot=%s bgTeam=%u zone=%s->%s at=%.1f,%.1f,%.1f score=%u-%u",
                         bot->GetName().c_str(), uint32(bot->GetBGTeam()), TOK_SHADOW_NAMES[mirror.zone],
                         TOK_SHADOW_NAMES[wanted], bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ(),
                         bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));

        for (uint32 step = 0; step < 3 && mirror.zone != wanted; ++step)
        {
            if (mirror.zone == TOK_SHADOW_OUT)  // 7734: OUT -> IN
            {
                bg->HandleAreaTrigger(bot, TOK_TRIGGER_TEMPLE, true);
                mirror.zone = TOK_SHADOW_IN;
            }
            else if (mirror.zone == TOK_SHADOW_IN)
            {
                if (wanted == TOK_SHADOW_MIDDLE)  // 7735: IN -> MIDDLE
                {
                    bg->HandleAreaTrigger(bot, TOK_TRIGGER_PIT, true);
                    mirror.zone = TOK_SHADOW_MIDDLE;
                }
                else  // 7734: IN -> OUT
                {
                    bg->HandleAreaTrigger(bot, TOK_TRIGGER_TEMPLE, false);
                    mirror.zone = TOK_SHADOW_OUT;
                }
            }
            else  // 7735: MIDDLE -> IN
            {
                bg->HandleAreaTrigger(bot, TOK_TRIGGER_PIT, false);
                mirror.zone = TOK_SHADOW_IN;
            }
        }
    }
}

bool BGTactics::isCarryingTOK(Battleground* bg)
{
    BattlegroundTOK* tok = AsTok(bg);
    if (!tok)
        return false;

    // The core's keeper table is the truth (BattlegroundTOK.h GetOrbPickersGUID); the pick-up
    // auras are a second cue for the frame between TakeOrb's aura cast and a stale read.
    ObjectGuid const myGuid = bot->GetGUID();
    for (ObjectGuid const& g : tok->GetOrbPickersGUID())
        if (g == myGuid)
            return true;

    return bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_1) || bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_2) ||
           bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_3) || bot->HasAura(BG_TOK_SPELL_ORB_PICKED_UP_4);
}

bool BGTactics::moveToStartTOK(Battleground* bg)
{
    if (!AsTok(bg))
        return false;

    // AC moveToStart (WSG/AB branches): spread around the team's wait spot with frand - kept inside the ~12 yd wide ramp.
    Position const& wait = BgTeamIdOf(bot) == TEAM_HORDE ? TOK_WAIT_HORDE : TOK_WAIT_ALLIANCE;
    return MoveTo(bg->GetMapId(), wait.GetPositionX() + frand(-2.5f, 2.5f),
                  wait.GetPositionY() + frand(-3.0f, 3.0f), wait.GetPositionZ());
}

bool BGTactics::selectObjectiveTOK(Battleground* bg)
{
    BattlegroundTOK* tok = AsTok(bg);
    if (!tok)
        return false;

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["bg objective"];

    auto setObjective = [&](float x, float y, float z)
    {
        pos.Set(x, y, z, bot->GetMapId());
        posMap["bg objective"] = pos;
    };

    // A carrier holds the temple centre: the orb scores every 5 s for as long as it is held and the
    // middle zone pays most (core UpdateScore 3/4/5). A small stable offset per bot (from its guid)
    // keeps two carriers from stacking on one spot without re-rolling every tick.
    if (isCarryingTOK(bg))
    {
        uint32 const n = uint32(bot->GetGUID().GetCounter() % 8);
        float const ang = float(n) * (6.2831853f / 8.0f);
        setObjective(TOK_CENTRE_X + 3.0f * std::cos(ang), TOK_CENTRE_Y + 3.0f * std::sin(ang), TOK_CENTRE_Z);
        return true;
    }

    uint8 const role = context->GetValue<uint32>("bg role")->Get();
    TeamId const myTeam = BgTeamIdOf(bot);
    TokPlan const& plan = PlanOf(GetBotStrategyForTeam(bg, myTeam));
    TokGroup const group = GroupOf(plan, role);
    {
        static thread_local std::map<ObjectGuid, uint32> s_planLog;
        uint32& last = s_planLog[bot->GetGUID()];
        if (!last || getMSTimeDiff(last, getMSTime()) >= 30000)
        {
            last = getMSTime();
            TC_LOG_DEBUG("playerbots", "[BGTOK] plan bot=%s team=%u strat=%s role=%u group=%s instance=%u",
                         bot->GetName().c_str(), uint32(myTeam), plan.name, uint32(role), TOK_GROUP_NAMES[group],
                         bg->GetInstanceID());
        }
    }

    // Orb keepers by side (core keeper table, BattlegroundTOK.h GetOrbPickersGUID).
    std::vector<ObjectGuid> const pickers = tok->GetOrbPickersGUID();
    Player* nearestFriend = nullptr;
    Player* nearestEnemy = nullptr;
    float friendDist = 1.0e9f;
    float enemyDist = 1.0e9f;
    bool orbFree[BG_TOK_MAX_ORBS] = {};
    for (uint8 i = 0; i < BG_TOK_MAX_ORBS && i < pickers.size(); ++i)
    {
        if (pickers[i].IsEmpty())
        {
            orbFree[i] = true;
            continue;
        }
        Player* holder = ObjectAccessor::FindPlayer(pickers[i]);
        if (!holder || holder == bot || !holder->IsAlive() || holder->GetMapId() != bot->GetMapId())
            continue;

        float const d = bot->GetDistance(holder);
        if (BgTeamIdOf(holder) == myTeam)
        {
            if (d < friendDist)
            {
                friendDist = d;
                nearestFriend = holder;
            }
        }
        else if (d < enemyDist)
        {
            enemyDist = d;
            nearestEnemy = holder;
        }
    }

    // Nearest free orb overall, and nearest on the side of the bot's group (TOK_PLANS; west = x below the centre).
    int nearestOrb = -1;
    int sideOrb = -1;
    float orbDist = 1.0e9f;
    float sideDist = 1.0e9f;
    for (uint8 i = 0; i < BG_TOK_MAX_ORBS; ++i)
    {
        if (!orbFree[i])
            continue;
        float const d = sServerFacade->GetDistance2d(bot, TOK_ORB_X[i], TOK_ORB_Y[i]);
        if (d < orbDist)
        {
            orbDist = d;
            nearestOrb = i;
        }
        if ((group == TOK_GROUP_WEST) == (TOK_ORB_X[i] < TOK_CENTRE_X) && d < sideDist)
        {
            sideDist = d;
            sideOrb = i;
        }
    }

    // Orb takers (a holder scores on its own, so every free orb is worth taking): their side's orb first, then any.
    if (group == TOK_GROUP_WEST || group == TOK_GROUP_EAST)
    {
        int const orb = sideOrb >= 0 ? sideOrb : nearestOrb;
        if (orb >= 0)
        {
            setObjective(TOK_ORB_X[orb], TOK_ORB_Y[orb], TOK_ORB_GROUND_Z);
            return true;
        }
    }

    // The centre group escorts an own carrier (carriers hold the centre) and otherwise holds the scoring pit itself;
    // takers without a free orb and hunters go for an enemy carrier first, then escort (AC WSG case: escort team FC /
    // hunt enemy FC, 2300-2311).
    if (group == TOK_GROUP_CENTRE)
    {
        if (nearestFriend)
            setObjective(nearestFriend->GetPositionX(), nearestFriend->GetPositionY(), nearestFriend->GetPositionZ());
        else
            setObjective(TOK_CENTRE_X + frand(-6.0f, 6.0f), TOK_CENTRE_Y + frand(-6.0f, 6.0f), TOK_CENTRE_Z);
        return true;
    }

    if (Player* target = nearestEnemy ? nearestEnemy : nearestFriend)
    {
        setObjective(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
        return true;
    }

    // No carrier on either side: take a free orb if any remain, else contest the centre.
    if (nearestOrb >= 0)
    {
        setObjective(TOK_ORB_X[nearestOrb], TOK_ORB_Y[nearestOrb], TOK_ORB_GROUND_Z);
        return true;
    }

    setObjective(TOK_CENTRE_X + frand(-4.0f, 4.0f), TOK_CENTRE_Y + frand(-4.0f, 4.0f), TOK_CENTRE_Z);
    return true;
}

bool BGTactics::atObjectiveTOK(Battleground* bg)
{
    BattlegroundTOK* tok = AsTok(bg);
    if (!tok)
        return false;

    // The centre bonus: tell the core which scoring zone this bot is in (every bot, every tick - this action
    // runs from "bg check flag" for the whole match, and a zone must be right before the bot ever takes an orb).
    TokSyncZone(bot, bg, isCarryingTOK(bg));

    // One orb per player (core EventPlayerClickedOnFlag HasOrb check).
    if (isCarryingTOK(bg) || !bot->IsAlive())
        return false;

    GuidVector closeObjects = *context->GetValue<GuidVector>("nearest game objects no los");
    if (closeObjects.empty())
        return false;

    GameObject* target = nullptr;
    float best = 1.0e9f;
    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || !IsOrbEntry(go->GetEntry()))
            continue;
        // A taken orb is despawned (TakeOrb: SpawnBGObject RESPAWN_ONE_DAY).
        if (!go->isSpawned() || go->GetGoState() != GO_STATE_READY)
            continue;

        float const d = bot->GetDistance(go);
        if (d < best)
        {
            best = d;
            target = go;
        }
    }
    if (!target)
        return false;

    if (best < INTERACTION_DISTANCE)
    {
        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        if (bot->IsInDisallowedMountForm())
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        // See the note at the top of BattleGroundTactics.cpp: CMSG_GAMEOBJ_USE is bit-packed here.
        TC_LOG_DEBUG("playerbots", "[BGTOK] %s (bgTeam %u) takes orb %u at %.1f yd",
                     bot->GetName().c_str(), uint32(bot->GetBGTeam()), target->GetEntry(), best);
        bot->ResetEmoteStateIfNeed();
        // Lead (b12): the orb GO type was not confirmed (24/36 would route GameObject::Use to the BG; the
        // type-26 switch there does not list 212091-212094). So do exactly what GameObject::Use does for a
        // flag stand (GameObject.cpp:1765-1794) and hand the click straight to the battleground - the same
        // call either way, independent of the template type.
        if (!bot->CanUseBattlegroundObject(target) || bot->GetVehicle())
            return false;
        bot->RemoveAurasByType(SPELL_AURA_MOD_STEALTH);
        bot->RemoveAurasByType(SPELL_AURA_MOD_INVISIBILITY);
        bg->EventPlayerClickedOnFlag(bot, target);
        resetObjective();
        return true;
    }

    // Walk the last stretch to an orb only for the orb takers of this match's plan (TOK_PLANS); the rest keep
    // their centre / hunt objective and pick one up only if they happen to stand next to it.
    uint8 const role = context->GetValue<uint32>("bg role")->Get();
    if (role >= PlanOf(GetBotStrategyForTeam(bg, BgTeamIdOf(bot))).takers || best > 50.0f)
        return false;

    // AC atFlag AB branch: engage a nearby enemy player before going for the objective.
    Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");
    if (enemyPlayer && enemyPlayer->IsAlive() && enemyPlayer->GetDistance(target) < 25.0f)
    {
        context->GetValue<Unit*>("current target")->Set(enemyPlayer);
        return false;
    }

    return MoveTo(bot->GetMapId(), target->GetPositionX(), target->GetPositionY(), target->GetPositionZ());
}
