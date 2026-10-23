/*
 * This file is part of the mod-playerbots module. See AUTHORS file for Copyright information;
 * released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Deepwind Gorge (map 1105, BATTLEGROUND_DG, the patch 5.3 version) bot tactics - issue #579,
 * epic #580. AC has no spec for this map; this combines AC's AB node capture
 * (AC BattleGroundTactics.cpp AB selectObjective 2312-2489, atFlag AB branch 3561-3965) with AC's
 * WSG flag steal (WSG selectObjective 2144-2311, atFlag WSG branch, moveToStart 1719-1840): the
 * team carts at each base are flag-like.
 */

#include "BattleGroundTactics.h"
#include "BgRealType.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundDG.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <map>
#include <mutex>
#include <string>

namespace
{
    // Node ground positions: BattlegroundDG.h BG_DG_NodePositions (x, y), z = MoP navmesh surface
    // under the capture-point creature (mmtile_probe, map 1105: all three ON NAVMESH).
    // Index = BG_DG_Nodes (0 goblin, 1 centre, 2 pandaren).
    float const DG_NODE_X[BG_DG_ALL_NODES_COUNT] = {-397.7726f, -167.5035f, 68.39236f};
    float const DG_NODE_Y[BG_DG_ALL_NODES_COUNT] = {574.3680f, 499.0590f, 431.1771f};
    float const DG_NODE_Z[BG_DG_ALL_NODES_COUNT] = {111.333f, 93.629f, 111.588f};

    // Wait spots: the cart stand of each base (BG_DG_CartPositions), inside the start gates
    // (BG_DG_DoorPositions) and ON NAVMESH. Index = TeamId (0 Alliance, 1 Horde).
    float const DG_BASE_X[2] = {-241.7413f, -91.61632f};
    float const DG_BASE_Y[2] = {208.6111f, 791.3611f};
    float const DG_BASE_Z[2] = {133.336f, 133.544f};

    // Area triggers the core uses to score a carried cart: BattlegroundDG::HandleAreaTrigger
    // 9012 = Alliance cart spawn (an Alliance bot carrying the Horde cart), 9013 = Horde cart spawn.
    uint32 const DG_TRIGGER_ALLIANCE_CART_SPAWN = 9012;
    uint32 const DG_TRIGGER_HORDE_CART_SPAWN = 9013;

    // Distance from the own cart stand inside which a carrier fires the capture trigger.
    float const DG_CAPTURE_RANGE = 10.0f;

    // A dropped cart goes back to its base by itself after BG_DG_CART_DROP_TIME (10 s, BattlegroundDG::
    // PostUpdateImplCart), so a bot more than about 10 s of running away from it can never be the one to click it.
    float const DG_GROUND_CART_REACH = 90.0f;

    // The least gold an enemy team must have before a thief walks across the map for its base cart: the core takes
    // min(200, score) from the victim on pick-up (EventPlayerClickedOnFlag) and credits exactly that on the capture.
    uint32 const DG_THEFT_MIN_ENEMY_SCORE = 100;

    bool IsDG(Battleground* bg)
    {
        if (!bg)
            return false;
        BattlegroundTypeId t = RealBgTypeId(bg); // #896
        return t == BATTLEGROUND_DG;
    }

    // Node status 0 neutral, 1 ally contested, 2 horde contested, 3 ally occupied, 4 horde occupied
    // (BG_DG_NodeStatus). BattlegroundDG::m_nodes is private, so it is read back from the aura the
    // core keeps on the node's capture-point creature (ChangeBanner, BattlegroundDG.cpp:419-465).
    uint8 NodeStatus(Battleground* bg, Player* who, uint8 node)
    {
        uint32 const idx = BG_DG_OBJECT_CAPT_POINT_START + node;
        if (idx >= bg->BgCreatures.size())
            return BG_DG_NODE_TYPE_NEUTRAL;
        Creature* cp = who->GetMap()->GetCreature(bg->BgCreatures[idx]);
        if (!cp)
            return BG_DG_NODE_TYPE_NEUTRAL;
        if (cp->HasAura(BG_DG_CAPT_POINT_ALLIANCE_CONTROL))
            return BG_DG_NODE_STATUS_ALLY_OCCUPIED;
        if (cp->HasAura(BG_DG_CAPT_POINT_HORDE_CONTROL))
            return BG_DG_NODE_STATUS_HORDE_OCCUPIED;
        if (cp->HasAura(BG_DG_CAPT_POINT_ALLIANCE_CONTEST))
            return BG_DG_NODE_STATUS_ALLY_CONTESTED;
        if (cp->HasAura(BG_DG_CAPT_POINT_HORDE_CONTEST))
            return BG_DG_NODE_STATUS_HORDE_CONTESTED;
        return BG_DG_NODE_TYPE_NEUTRAL;
    }

    Creature* NodeCreature(Battleground* bg, Player* who, uint8 node)
    {
        uint32 const idx = BG_DG_OBJECT_CAPT_POINT_START + node;
        if (idx >= bg->BgCreatures.size())
            return nullptr;
        return who->GetMap()->GetCreature(bg->BgCreatures[idx]);
    }

    // BattlegroundDG::CanSeeSpellClick (BattlegroundDG.cpp:495-513): a node cannot be clicked by the
    // team that already holds / is contesting it. Allowed: neutral, or a state whose %2 equals the
    // clicking team index (ally states 1,3 are odd = Horde index; horde states 2,4 are even = Alliance).
    bool CanClickNode(uint8 status, TeamId team)
    {
        return status == BG_DG_NODE_TYPE_NEUTRAL || TeamId(status % 2) == team;
    }

    bool IsCartEntry(uint32 entry)
    {
        return entry == BG_DG_OBJECTID_CART_ALLY || entry == BG_DG_OBJECTID_CART_HORDE ||
               entry == BG_DG_OBJECTID_CART_ALLY_GROUND || entry == BG_DG_OBJECTID_CART_HORDE_GROUND;
    }

    // #795 (beyond AC: AC's AB atFlag assumes the banner cast starts). Spell::prepare can refuse the 7 s capture
    // cast 97388 before it starts (CheckCast, incl. spell_dg_capturing::CheckCast), and the old code then re-tried
    // the same node on every tick: wave 41 had Cornac log 557 captures, 482 of them at the Horde-held pandaren node,
    // without a single cast=97388 probe. A refused node is left alone for a while, like an AB node the team cannot
    // take, so the bot moves on to another node or objective.
    uint32 const DG_CAPTURE_REFUSED_BACKOFF_MS = 30000;

    struct DgRefused
    {
        uint8 node = 255;
        uint32 sinceMs = 0;
    };
    std::mutex s_dgRefusedLock;  // bots of different instances run on different map threads
    std::map<ObjectGuid, DgRefused> s_dgRefused;

    bool CaptureRefused(Player* bot, uint8 node)
    {
        std::lock_guard<std::mutex> guard(s_dgRefusedLock);
        auto it = s_dgRefused.find(bot->GetGUID());
        if (it == s_dgRefused.end() || it->second.node != node)
            return false;
        if (getMSTimeDiff(it->second.sinceMs, getMSTime()) < DG_CAPTURE_REFUSED_BACKOFF_MS)
            return true;
        s_dgRefused.erase(it);
        return false;
    }

    void NoteCaptureRefused(Player* bot, uint8 node)
    {
        std::lock_guard<std::mutex> guard(s_dgRefusedLock);
        DgRefused& r = s_dgRefused[bot->GetGUID()];
        r.node = node;
        r.sinceMs = getMSTime();
    }

    // A spawned cart object of the given BgObjects slot (base cart or dropped cart), or null.
    // Looked up through the map, not Battleground::GetBGObject, which logs an error for slots the
    // core never filled (the ground carts exist only after a drop).
    GameObject* CartObject(Battleground* bg, Player* who, uint32 slot)
    {
        if (slot >= bg->BgObjects.size() || bg->BgObjects[slot].IsEmpty())
            return nullptr;
        GameObject* go = who->GetMap()->GetGameObject(bg->BgObjects[slot]);
        if (!go || !go->isSpawned())
            return nullptr;
        return go;
    }

    // ---- Match evidence (#579) --------------------------------------------------------------------------
    // "[BGDG] node-change" / "[BGDG] cart-change" on every observed change and one "[BGDG] state" line per instance every
    // 30 s, written by whichever bot of the instance runs first (core state is the truth, the bot only reports).
    // Process-wide with a mutex, not thread_local: an instance's map update can run on any MapUpdate.Threads worker.
    char const* const DG_NODE_NAMES[BG_DG_ALL_NODES_COUNT] = {"goblin", "centre", "pandaren"};
    char const* const DG_STATE_NAMES[5] = {"neutral", "A-contested", "H-contested", "A-occupied", "H-occupied"};
    char const* const DG_CART_OF[2] = {"cartA", "cartH"};

    struct DgWatch
    {
        uint32 lastLogMs = 0;
        uint8 node[BG_DG_ALL_NODES_COUNT] = {255, 255, 255};
        std::string cart[2];  // "base", "ground" or "carried:<name>"; empty until first seen
        uint32 nodeChanges = 0;
        uint32 cartChanges = 0;
    };
    std::mutex s_dgWatchLock;
    std::map<uint32, DgWatch> s_dgWatch;

    char const* DgStateName(uint8 s)
    {
        return s < 5 ? DG_STATE_NAMES[s] : "?";
    }

    // State of the cart of team t: carried by an enemy (the picker table is indexed by the cart's owner team), lying on the
    // ground, or at its base.
    std::string DgCartState(Battleground* bg, Player* who, uint8 t)
    {
        ObjectGuid const g = bg->GetFlagPickerGUID(TeamId(t));
        if (!g.IsEmpty())
        {
            Player* carrier = ObjectAccessor::FindPlayer(g);
            return std::string("carried:") + (carrier ? carrier->GetName() : "?");
        }
        if (CartObject(bg, who, BG_DG_OBJECT_CART_ALLY_GROUND + t))
            return "ground";
        return "base";
    }

    void DgObserve(Battleground* bg, Player* who)
    {
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
            return;

        uint8 nodes[BG_DG_ALL_NODES_COUNT];
        for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
            nodes[i] = NodeStatus(bg, who, i);
        std::string carts[2] = {DgCartState(bg, who, 0), DgCartState(bg, who, 1)};

        std::lock_guard<std::mutex> guard(s_dgWatchLock);
        DgWatch& w = s_dgWatch[bg->GetInstanceID()];
        for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
        {
            if (nodes[i] == w.node[i])
                continue;
            if (w.node[i] != 255)
            {
                ++w.nodeChanges;
                TC_LOG_DEBUG("playerbots", "[BGDG] node-change inst=%u node=%s %s->%s gold=%u-%u", bg->GetInstanceID(),
                             DG_NODE_NAMES[i], DgStateName(w.node[i]), DgStateName(nodes[i]),
                             bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
            }
            w.node[i] = nodes[i];
        }
        for (uint8 t = 0; t < 2; ++t)
        {
            if (carts[t] == w.cart[t])
                continue;
            if (!w.cart[t].empty())
            {
                ++w.cartChanges;
                TC_LOG_DEBUG("playerbots", "[BGDG] cart-change inst=%u %s %s->%s gold=%u-%u", bg->GetInstanceID(),
                             DG_CART_OF[t], w.cart[t].c_str(), carts[t].c_str(), bg->GetTeamScore(TEAM_ALLIANCE),
                             bg->GetTeamScore(TEAM_HORDE));
            }
            w.cart[t] = carts[t];
        }

        uint32 const nowMs = getMSTime();
        if (w.lastLogMs && getMSTimeDiff(w.lastLogMs, nowMs) < 30000)
            return;
        w.lastLogMs = nowMs;
        TC_LOG_DEBUG("playerbots",
                     "[BGDG] state inst=%u gold=%u-%u nodeChanges=%u cartChanges=%u goblin=%s centre=%s pandaren=%s cartA=%s cartH=%s",
                     bg->GetInstanceID(), bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE), w.nodeChanges,
                     w.cartChanges, DgStateName(nodes[0]), DgStateName(nodes[1]), DgStateName(nodes[2]),
                     carts[0].c_str(), carts[1].c_str());
    }
}

bool BGTactics::isCarryingDG(Battleground* bg)
{
    if (!IsDG(bg))
        return false;

    // The core's picker table is the truth (BattlegroundDG.h GetFlagPickerGUID :319); the holder
    // auras are a second cue for the frame between the pick-up and a stale read.
    ObjectGuid const myGuid = bot->GetGUID();
    if (bg->GetFlagPickerGUID(TEAM_ALLIANCE) == myGuid || bg->GetFlagPickerGUID(TEAM_HORDE) == myGuid)
        return true;

    return bot->HasAura(BG_DG_ALLIANCE_MINE_CART) || bot->HasAura(BG_DG_HORDE_MINE_CART);
}

bool BGTactics::moveToStartDG(Battleground* bg)
{
    if (!IsDG(bg))
        return false;

    // AC moveToStart (AB/WSG branches): spread around the team's wait spot with frand.
    uint8 const t = BgTeamIdOf(bot) == TEAM_HORDE ? 1 : 0;
    return MoveTo(bg->GetMapId(), DG_BASE_X[t] + frand(-6.0f, 6.0f), DG_BASE_Y[t] + frand(-6.0f, 6.0f),
                  DG_BASE_Z[t]);
}

bool BGTactics::selectObjectiveDG(Battleground* bg)
{
    if (!IsDG(bg))
        return false;

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["bg objective"];

    auto setObjective = [&](float x, float y, float z)
    {
        pos.Set(x, y, z, bot->GetMapId());
        posMap["bg objective"] = pos;
    };

    TeamId const myTeam = BgTeamIdOf(bot);
    TeamId const enemyTeam = myTeam == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE;
    uint8 const me = myTeam;
    uint8 const en = enemyTeam;

    // A carrier brings the stolen cart to its own base (core HandleAreaTrigger 9012/9013 scores it;
    // WSG selectObjective: the flag carrier heads for its base).
    if (isCarryingDG(bg))
    {
        setObjective(DG_BASE_X[me], DG_BASE_Y[me], DG_BASE_Z[me]);
        return true;
    }

    uint8 const role = context->GetValue<uint32>("bg role")->Get();

    // --- Cart situation (core: picker of team T = the enemy carrying T's cart; ground cart object
    // slots BG_DG_OBJECT_CART_ALLY_GROUND + T, base cart slots BG_DG_OBJECT_CART_ALLIANCE + T).
    Player* ownCartThief = nullptr;   // enemy carrying OUR cart
    Player* friendCarrier = nullptr;  // friend carrying THEIR cart
    {
        ObjectGuid const g = bg->GetFlagPickerGUID(me);
        if (!g.IsEmpty())
            ownCartThief = ObjectAccessor::FindPlayer(g);
        ObjectGuid const f = bg->GetFlagPickerGUID(en);
        if (!f.IsEmpty())
            friendCarrier = ObjectAccessor::FindPlayer(f);
    }
    GameObject* ownGround = CartObject(bg, bot, BG_DG_OBJECT_CART_ALLY_GROUND + me);
    GameObject* enemyGround = CartObject(bg, bot, BG_DG_OBJECT_CART_ALLY_GROUND + en);
    GameObject* enemyBaseCart = CartObject(bg, bot, BG_DG_OBJECT_CART_ALLIANCE + en);

    // Own cart dropped on the ground: return it (click by our team = return, core :744-755), if it can be reached
    // before it goes home on its own.
    if (ownGround && role < 4 &&
        sServerFacade->GetDistance2d(bot, ownGround->GetPositionX(), ownGround->GetPositionY()) < DG_GROUND_CART_REACH)
    {
        setObjective(ownGround->GetPositionX(), ownGround->GetPositionY(), ownGround->GetPositionZ());
        return true;
    }

    // Enemy cart dropped on the ground (a friendly carrier died): thieves pick it up again. The gold it carries was
    // taken from the victim at the base pick-up and stays on the cart (#797: a drop or re-pick-up moves no gold; a
    // return or the drop-timer respawn gives it back), so a re-pick-up is worth it at any score.
    if (enemyGround && role >= 8 &&
        sServerFacade->GetDistance2d(bot, enemyGround->GetPositionX(), enemyGround->GetPositionY()) < DG_GROUND_CART_REACH)
    {
        setObjective(enemyGround->GetPositionX(), enemyGround->GetPositionY(), enemyGround->GetPositionZ());
        return true;
    }

    // Roles 5-9: hunt the thief carrying our cart (AC WSG case: hunt the enemy flag carrier).
    if (ownCartThief && ownCartThief->IsAlive() && ownCartThief->GetMapId() == bot->GetMapId() && role >= 5)
    {
        setObjective(ownCartThief->GetPositionX(), ownCartThief->GetPositionY(), ownCartThief->GetPositionZ());
        return true;
    }

    // Roles 8-9: escort our carrier, else go steal the enemy base cart (AC WSG case).
    if (role >= 8)
    {
        if (friendCarrier && friendCarrier->IsAlive() && friendCarrier->GetMapId() == bot->GetMapId() &&
            friendCarrier != bot)
        {
            setObjective(friendCarrier->GetPositionX(), friendCarrier->GetPositionY(),
                         friendCarrier->GetPositionZ());
            return true;
        }
        // Stealing a cart is worth exactly the gold it takes: with the enemy near zero it is a long walk for nothing, so
        // the thief plays the nodes instead until the enemy has something to lose.
        if (enemyBaseCart && bg->GetTeamScore(en) >= DG_THEFT_MIN_ENEMY_SCORE)
        {
            setObjective(DG_BASE_X[en] + frand(-2.0f, 2.0f), DG_BASE_Y[en] + frand(-2.0f, 2.0f), DG_BASE_Z[en]);
            return true;
        }
    }

    // --- Nodes, AB pattern (AC AB selectObjective 2312-2489): roles 0-2 defend, the rest attack.
    bool const isDefender = role < 3;
    bool const isSilly = urand(0, 99) < 20;

    // AB PRIORITY 1: rare aggressive impulse towards a visible enemy.
    if (urand(0, 99) < 5)
    {
        if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
        {
            if (bot->GetDistance(enemy) < 500.0f)
            {
                setObjective(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ());
                return true;
            }
        }
    }

    uint8 status[BG_DG_ALL_NODES_COUNT];
    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
        status[i] = NodeStatus(bg, bot, i);

    // AB PRIORITY 3: defenders stay at nodes the team holds or that the enemy is contesting.
    if (isDefender && urand(0, 99) < 85)
    {
        int best = -1;
        float bestDist = 1.0e9f;
        for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
        {
            bool const owned = status[i] == BG_DG_NODE_STATUS_ALLY_OCCUPIED + me;
            bool const ownContested = status[i] == BG_DG_NODE_STATUS_ALLY_CONTESTED + me;
            bool const enemyContested = status[i] == BG_DG_NODE_STATUS_ALLY_CONTESTED + en;
            if (!owned && !ownContested && !enemyContested)
                continue;
            float const d = sServerFacade->GetDistance2d(bot, DG_NODE_X[i], DG_NODE_Y[i]);
            if (d < bestDist)
            {
                bestDist = d;
                best = i;
            }
        }
        if (best >= 0)
        {
            setObjective(DG_NODE_X[best], DG_NODE_Y[best], DG_NODE_Z[best]);
            return true;
        }
    }

    // AB PRIORITY 4: attack a node the team can click (neutral, enemy occupied, enemy contested).
    // Eligible nodes sorted by distance; role spreads the bots over them, silly bots take the farthest.
    std::vector<std::pair<float, uint8>> eligible;
    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        if (!CanClickNode(status[i], myTeam) || CaptureRefused(bot, i))  // #795
            continue;
        eligible.push_back({sServerFacade->GetDistance2d(bot, DG_NODE_X[i], DG_NODE_Y[i]), i});
    }
    std::sort(eligible.begin(), eligible.end());

    if (!eligible.empty())
    {
        size_t pick = 0;
        if (isSilly)
            pick = eligible.size() - 1;
        else if (urand(0, 99) < 70)
            pick = role % eligible.size();
        uint8 const node = eligible[pick].second;
        setObjective(DG_NODE_X[node], DG_NODE_Y[node], DG_NODE_Z[node]);
        return true;
    }

    // AB PRIORITY 2: no node to take. Guard a node held / contested by our team (a contested one
    // turns occupied after BG_DG_FLAG_CAPTURING_TIME), else attack a visible enemy, else camp the
    // enemy cart stand.
    int hold = -1;
    float holdDist = 1.0e9f;
    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        if (status[i] != BG_DG_NODE_STATUS_ALLY_CONTESTED + me && status[i] != BG_DG_NODE_STATUS_ALLY_OCCUPIED + me)
            continue;
        float const d = sServerFacade->GetDistance2d(bot, DG_NODE_X[i], DG_NODE_Y[i]);
        if (d < holdDist)
        {
            holdDist = d;
            hold = i;
        }
    }
    if (hold >= 0)
    {
        setObjective(DG_NODE_X[hold] + frand(-4.0f, 4.0f), DG_NODE_Y[hold] + frand(-4.0f, 4.0f), DG_NODE_Z[hold]);
        return true;
    }

    if (Unit* enemy = AI_VALUE(Unit*, "enemy player target"))
    {
        if (bot->GetDistance(enemy) < 500.0f)
        {
            setObjective(enemy->GetPositionX(), enemy->GetPositionY(), enemy->GetPositionZ());
            return true;
        }
    }

    setObjective(DG_BASE_X[en] + frand(-8.0f, 8.0f), DG_BASE_Y[en] + frand(-8.0f, 8.0f), DG_BASE_Z[en]);
    return true;
}

bool BGTactics::atObjectiveDG(Battleground* bg)
{
    if (!IsDG(bg))
        return false;

    DgObserve(bg, bot);

    if (!bot->IsAlive())
        return false;

    TeamId const myTeam = BgTeamIdOf(bot);
    uint8 const me = myTeam;

    // --- Already channelling the capture: stay still and dismounted, unless the node can no longer
    // be taken (another bot finished it first) - AC atFlag keepStationaryWhileCapturing.
    for (uint8 type : {uint8(CURRENT_GENERIC_SPELL), uint8(CURRENT_CHANNELED_SPELL)})
    {
        Spell* current = bot->GetCurrentSpell(static_cast<CurrentSpellTypes>(type));
        if (!current || !current->m_spellInfo || current->m_spellInfo->Id != BG_DG_CAPT_POINT_START_CAPT)
            continue;

        bool stillValid = false;
        for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
        {
            Creature* cp = NodeCreature(bg, bot, i);
            if (cp && current->m_targets.GetUnitTarget() == cp && CanClickNode(NodeStatus(bg, bot, i), myTeam))
                stillValid = true;
        }
        if (!stillValid)
        {
            bot->CastStop();
            resetObjective();
            return false;
        }

        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        if (bot->IsInDisallowedMountForm())
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);
        if (bot->isMoving())
            bot->StopMoving();
        return true;
    }

    // --- A carrier at its own base scores the cart (WSG atFlag branch: bg->HandleAreaTrigger, not
    // a CMSG packet - the opcode is bit-packed here, see the note at the top of BattleGroundTactics.cpp).
    if (isCarryingDG(bg))
    {
        if (sServerFacade->GetDistance2d(bot, DG_BASE_X[me], DG_BASE_Y[me]) < DG_CAPTURE_RANGE)
        {
            TC_LOG_DEBUG("playerbots", "[BGDG] %s (bgTeam %u) scores the cart at its base", bot->GetName().c_str(),
                         uint32(bot->GetBGTeam()));
            bg->HandleAreaTrigger(
                bot, me == TEAM_HORDE ? DG_TRIGGER_HORDE_CART_SPAWN : DG_TRIGGER_ALLIANCE_CART_SPAWN, true);
            resetObjective();
            return true;
        }
        return false;
    }

    uint8 const role = context->GetValue<uint32>("bg role")->Get();

    // --- Carts (WSG atFlag pick-up): ground carts of either side (own = return, enemy = steal) and
    // the enemy base cart for the thief roles. Our own base cart is never clicked (core ignores it).
    uint32 const enemyBaseEntry = me == TEAM_ALLIANCE ? BG_DG_OBJECTID_CART_HORDE : BG_DG_OBJECTID_CART_ALLY;
    GuidVector closeObjects = *context->GetValue<GuidVector>("nearest game objects no los");
    GameObject* cart = nullptr;
    float cartDist = 1.0e9f;
    for (ObjectGuid const guid : closeObjects)
    {
        GameObject* go = botAI->GetGameObject(guid);
        if (!go || !IsCartEntry(go->GetEntry()) || !go->isSpawned() || go->GetGoState() != GO_STATE_READY)
            continue;

        bool const ground = go->GetEntry() == BG_DG_OBJECTID_CART_ALLY_GROUND ||
                            go->GetEntry() == BG_DG_OBJECTID_CART_HORDE_GROUND;
        if (!ground && (go->GetEntry() != enemyBaseEntry || role < 8))
            continue;

        float const d = bot->GetDistance(go);
        if (d < cartDist)
        {
            cartDist = d;
            cart = go;
        }
    }

    if (cart && cartDist < INTERACTION_DISTANCE)
    {
        if (bot->IsMounted())
            bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        if (bot->IsInDisallowedMountForm())
            bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

        TC_LOG_DEBUG("playerbots", "[BGDG] %s (bgTeam %u) clicks cart %u at %.1f yd", bot->GetName().c_str(),
                     uint32(bot->GetBGTeam()), cart->GetEntry(), cartDist);
        bot->ResetEmoteStateIfNeed();
        cart->Use(bot);
        resetObjective();
        return true;
    }

    // --- Nodes (AB atFlag branch): the nearest clickable node in reach.
    Creature* node = nullptr;
    uint8 nodeIdx = 0;
    float nodeDist = 1.0e9f;
    for (uint8 i = 0; i < BG_DG_ALL_NODES_COUNT; ++i)
    {
        Creature* cp = NodeCreature(bg, bot, i);
        if (!cp || !CanClickNode(NodeStatus(bg, bot, i), myTeam) || CaptureRefused(bot, i))  // #795
            continue;
        float const d = bot->GetDistance(cp);
        if (d < nodeDist)
        {
            nodeDist = d;
            node = cp;
            nodeIdx = i;
        }
    }

    if (!node)
    {
        // A cart in sight but out of reach: walk the last stretch (WSG atFlag MoveTo) for thieves.
        if (cart && cartDist < 25.0f)
            return MoveTo(bot->GetMapId(), cart->GetPositionX(), cart->GetPositionY(), cart->GetPositionZ());
        return false;
    }

    if (nodeDist > 40.0f)
        return false;

    // AC atFlag AB branch: engage a nearby enemy player before the capture.
    Unit* enemyPlayer = AI_VALUE(Unit*, "enemy player target");
    if (enemyPlayer && enemyPlayer->IsAlive() && enemyPlayer->GetDistance(node) < INTERACTION_DISTANCE * 2.0f)
    {
        context->GetValue<Unit*>("current target")->Set(enemyPlayer);
        return false;
    }

    // AC atFlag AB branch: a friend already channelling next to the node - do not stack on it.
    GuidVector closePlayers = *context->GetValue<GuidVector>("closest friendly players");
    for (ObjectGuid const& guid : closePlayers)
    {
        Unit* pFriend = botAI->GetUnit(guid);
        if (!pFriend || pFriend == bot)
            continue;
        Spell* spell = pFriend->GetCurrentSpell(CURRENT_GENERIC_SPELL);
        if (!spell)
            spell = pFriend->GetCurrentSpell(CURRENT_CHANNELED_SPELL);
        if (spell && spell->m_spellInfo && spell->m_spellInfo->Id == BG_DG_CAPT_POINT_START_CAPT &&
            pFriend->GetDistance(node) < 12.0f)
        {
            resetObjective();
            return false;
        }
    }

    if (nodeDist >= INTERACTION_DISTANCE)
        return MoveTo(bot->GetMapId(), node->GetPositionX(), node->GetPositionY(), node->GetPositionZ());

    // #754 (beyond AC): fight whoever is hitting the bot before starting the capture cast (BGTactics::CaptureThreat)
    if (Unit* threat = CaptureThreat(bot))
    {
        // #754 (owner 2026-10-08) keep the current target while it is alive and attacking the bot, else the threat;
        // engage it (#756: setting the value alone left a bot hit by a pet/guardian idle at the flag)
        Unit* const cur = AI_VALUE(Unit*, "current target");
        Unit* const fight = (cur && cur->IsAlive() && cur->GetVictim() == bot) ? cur : threat;
        if (!EngageThreat(bot, fight))
            context->GetValue<Unit*>("current target")->Set(fight);
        return false;
    }

    // Dismount, then channel the capture on the node (core: spell 97388 on the capture-point creature
    // reaches BattlegroundDG::EventPlayerClickedOnFlag(Player*, Unit*), SpellEffects.cpp:650-653).
    if (bot->IsMounted())
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
    if (bot->IsInDisallowedMountForm())
        bot->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(BG_DG_CAPT_POINT_START_CAPT);
    if (!spellInfo)
        return false;

    // #795: all three capture points are creature 53194, so the node name says which one
    TC_LOG_DEBUG("playerbots", "[BGDG] %s (bgTeam %u) captures node creature %u node=%s", bot->GetName().c_str(),
                 uint32(bot->GetBGTeam()), node->GetEntry(), DG_NODE_NAMES[nodeIdx]);
    bot->StopMoving();
    Spell* spell = new Spell(bot, spellInfo, TRIGGERED_NONE);
    spell->m_targets.SetUnitTarget(node);
    spell->prepare(&spell->m_targets);

    // #795: a started capture is the bot's current generic spell (Spell::prepare -> SetCurrentCastedSpell); a refused
    // one was finished inside prepare (the SpellEvent deletes it later, so reading it here is safe, as WaitForSpellCast
    // below already does). Log why and leave the node alone instead of re-trying it every tick.
    if (bot->GetCurrentSpell(CURRENT_GENERIC_SPELL) != spell)
    {
        TC_LOG_DEBUG("playerbots", "[BGDG] %s (bgTeam %u) capture refused node=%s result=%u state=%u moving=%u",
                     bot->GetName().c_str(), uint32(bot->GetBGTeam()), DG_NODE_NAMES[nodeIdx],
                     uint32(spell->GetCastResult()), spell->getState(), uint32(bot->isMoving()));
        NoteCaptureRefused(bot, nodeIdx);
        resetObjective();
        return false;
    }

    botAI->WaitForSpellCast(spell);

    resetObjective();
    return true;
}
