/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Silvershard Mines (map 727) bot tactics, build 12 / issue #578 (epic #580).
//
// AC has no Silvershard Mines. The pattern is AC's Arathi Basin proximity capture (BattleGroundTactics.cpp
// AB selectObjective 2312-2489, atFlag 3561-3965) applied to three MOVING capture points, plus the WSG
// "follow the carrier" idea (Follow(teamFC), AC BattleGroundTactics.cpp:2222/2266/2287). Core truth is
// BattlegroundSM::CheckPlayerNearMineCart (BattlegroundSM.cpp:362): a living, non-stealthed player of a team
// within 22 yards of a cart (FindNearestCreature(NPC_MINE_CART_n, 22.0f)) pushes that cart's progress bar
// toward its own team; there is no click or spell. So the bot objective is "stand within ~8 yards of a cart
// and keep re-targeting the cart's CURRENT position".
//
// Build 14 (#578), reviewed against the core and Wowpedia "Silvershard Mines":
//  - control is by NUMBERS (every player in range pushes the bar its own way, so the larger side wins it), a held cart
//    pays +1 per second and a cart that reaches its depot while held pays POINTS_PER_MINE_CART = 150 (EventReopenDepot;
//    the 200 in MINE_CART_AT_DEPOT_POINTS is not used by this core);
//  - the two TRACK SWITCHES (NPC 60283) are handled: roles 9 / 8 steer the east / north cart towards their own side
//    before it reaches its fork (see the "Track switches" block below);
//  - cart creatures are read without Battleground::GetBGCreature, which logs an error on every call for the instant
//    between a cart being deleted at its depot and re-summoned.

#include "BattleGroundTactics.h"

// DatabaseEnv.h must precede the battleground headers (see BattleGroundTactics.cpp).
#include "DatabaseEnv.h"

#include "Battleground.h"
#include "BattlegroundSM.h"
#include "Creature.h"
#include "GameObject.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "ServerFacade.h"
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <map>
#include <mutex>
#include <vector>

namespace
{
    // Core proximity radius (BattlegroundSM.cpp:376 FindNearestCreature(..., 22.0f)). Bots stay well inside it.
    constexpr float SM_CONTROL_RADIUS = 22.0f;
    constexpr float SM_ESCORT_NEAR = 8.0f;      // stop closing in inside this
    constexpr float SM_ESCORT_ENGAGE = 16.0f;   // within this, atObjectiveSM takes over from moveToObjective

    constexpr uint32 SM_CART_TYPES[SM_MINE_CART_MAX] = { NPC_SM_MINE_CART_1, NPC_SM_MINE_CART_2, NPC_SM_MINE_CART_3 };

    // Gate-side start spots, on the MoP map-727 navmesh (mmtile_probe --scan, ground polys, z ~329.0).
    // Alliance gate at BG_SM_DoorPos[0..1] (852,158 / 830,144); Horde gate at [2..3] (652,228 / 636,208).
    Position const SM_START_ALLIANCE[3] = { { 846.0f, 144.0f, 329.0f, 0.0f }, { 852.0f, 150.0f, 329.0f, 0.0f }, { 858.0f, 156.0f, 328.9f, 0.0f } };
    Position const SM_START_HORDE[3]    = { { 633.0f, 224.0f, 329.0f, 0.0f }, { 639.0f, 230.0f, 329.0f, 0.0f }, { 645.0f, 236.0f, 329.1f, 0.0f } };
    // Cart spawn pit (BG_SM_StartPos), on the navmesh: where to wait while no cart is spawned yet.
    Position const SM_CART_PIT = { 744.5f, 183.5f, 319.7f, 0.0f };

    // A BgCreatures slot as a creature, or null. Looked up through the map rather than Battleground::GetBGCreature,
    // which logs an error whenever the slot is empty (a cart slot is empty for a moment after the depot capture).
    Creature* SmSlot(Battleground* bg, Player* who, uint32 slot)
    {
        if (slot >= bg->BgCreatures.size() || bg->BgCreatures[slot].IsEmpty())
            return nullptr;
        return who->GetMap()->GetCreature(bg->BgCreatures[slot]);
    }

    Creature* SmCart(Battleground* bg, Player* who, uint32 i)
    {
        Creature* cart = SmSlot(bg, who, SM_CART_TYPES[i]);
        return (cart && cart->IsAlive()) ? cart : nullptr;
    }

    // ---- Track switches ---------------------------------------------------------------------------------
    // Two switch NPCs (BgCreatures SM_TRACK_SWITCH_EAST / _NORTH) steer cart 1 and cart 3 at their fork
    // (BattlegroundSM::MineCartsMoves). Clicking one runs BattlegroundSM::EventPlayerClickedOnFlag(Player*, Unit*)
    // (what the spell click 124492 -> 97372 reaches, SpellEffects.cpp:650-653), which toggles the aura the fork reads:
    //   east  fork (717.2, 114.3): switch OPENED -> path 603780 -> LAVA depot (west, the Horde side),
    //                              otherwise      -> path 6037800 -> DIAMOND depot (east, the Alliance side);
    //   north fork (834.7, 299.8): switch OPENED -> path 6038000 -> DIAMOND depot,
    //                              otherwise      -> path 603800  -> TROLL depot (far north, near neither base).
    // (waypoint_data ids 60378 / 60380 and their x10 / x100 branches; the end points are the depot positions.)
    // Wowpedia: "you can change a cart's route ... ideally towards your teammates", and you need not control the cart.
    // So Alliance wants east=closed (default) and north=opened, Horde wants east=opened and north=closed (default).
    constexpr uint32 SM_SWITCH_EAST = 0;
    constexpr uint32 SM_SWITCH_NORTH = 1;
    constexpr uint32 SM_NO_SWITCH = 2;

    // Where the bot stands to use a switch: on the navmesh (mmtile_probe, map 727) about 8 yd from the NPC, which
    // itself sits just off the mesh on the rail.
    float const SM_SWITCH_STAND[2][3] = { { 715.6f, 93.3f, 320.15f }, { 847.5f, 300.0f, 347.08f } };
    constexpr float SM_SWITCH_CLICK_RANGE = 10.5f;  // 2D, from the NPC (the core does not check a range at all)
    constexpr uint32 SM_SWITCH_CLICK_GAP_MS = 6000; // the core's own prevention aura lasts ~3 s

    // The switch role of a bg role: one bot per team per switch, picked by role like AC AB's defender split.
    uint32 SmSwitchOfRole(uint8 role)
    {
        return role == 9 ? SM_SWITCH_EAST : (role == 8 ? SM_SWITCH_NORTH : SM_NO_SWITCH);
    }

    // True while the cart a switch steers has not yet reached its fork (afterwards the route is fixed). East cart:
    // pit (744, 183) down to (717, 114.5), so y stays above the fork; north cart: pit (760, 199) up to (834.8, 299.6),
    // while both branches start at x > 836 or y > 300.
    bool SmCartBeforeFork(Creature* cart, uint32 which)
    {
        if (which == SM_SWITCH_EAST)
            return cart->GetPositionY() > 116.0f && cart->GetPositionX() < 760.0f;
        return cart->GetPositionX() < 835.5f && cart->GetPositionY() < 299.5f;
    }

    // Does this team want the switch clicked right now? On success npcOut is the switch NPC.
    bool SmSwitchNeedsClick(Battleground* bg, Player* who, uint32 which, TeamId team, Creature*& npcOut)
    {
        Creature* npc = SmSlot(bg, who, which == SM_SWITCH_EAST ? SM_TRACK_SWITCH_EAST : SM_TRACK_SWITCH_NORTH);
        Creature* cart = SmSlot(bg, who, which == SM_SWITCH_EAST ? NPC_SM_MINE_CART_1 : NPC_SM_MINE_CART_3);
        if (!npc || !cart || !cart->IsAlive() || !SmCartBeforeFork(cart, which))
            return false;

        bool const wantsOpened = which == SM_SWITCH_EAST ? team == TEAM_HORDE : team == TEAM_ALLIANCE;
        bool const opened = npc->HasAura(BG_SM_TRACK_SWITCH_OPENED);
        npcOut = npc;
        return wantsOpened != opened;
    }

    // 0 neutral, 1 alliance, 2 horde - from the visual auras the core script puts on the cart.
    uint32 SmCartHeldBy(Creature* cart)
    {
        if (cart->HasAura(BG_SM_CONTROL_VISUAL_ALLIANCE))
            return SM_MINE_CART_CONTROL_ALLIANCE;
        if (cart->HasAura(BG_SM_CONTROL_VISUAL_HORDE))
            return SM_MINE_CART_CONTROL_HORDE;
        return SM_MINE_CART_CONTROL_NEUTRAL;
    }

    // ---- Start platforms (#578, #748) -------------------------------------------------------------------
    // Wowpedia "Silvershard Mines" (MediaWiki API, read 2026-10-12): "only a single graveyard for each faction, located
    // safely within that faction's raised starting area". Each start area is a platform ~9 yd above the mine floor, and
    // on the map-727 navmesh it is a separate island: Alliance 49 polygons x 820..875 y 114..170 z 328.7..329.5,
    // Horde 50 polygons x 607..661 y 196..253, with the main mesh (1827 polygons: cart pit, every cart waypoint, all four
    // depots, both switches) at z ~320 right below the lip. No path leaves either island, so every bot ran to the
    // nearest lip point ((821.7,144.7), (856.5,170.1), (661.2,230.1)) and stood there - waves 30-33: 0 carts, 0 deaths,
    // "move to objective:FAILED" on 10979 of 24115 probe rows. AC has the same shape on its WSG and EotS ledges and
    // walks to a take-off point, then JumpTo()s down (AC BattleGroundTactics.cpp:1442-1509 wsJumpDown, :1511-1549
    // eyJumpDown, called from selectObjectiveWp :3253/:3268; MoP BattleGroundTactics.cpp:1793 / :1862, and :1916 tpJumpDown
    // for the #747 Twin Peaks islands). Same here, one exit per team on the side facing the cart pit (each door opens
    // toward it: BG_SM_DoorPos[1] (830.1,143.9) o=3.13, [2] (652.2,228.5) o=0.16). Points from the navmesh (offline
    // dtsim over the 0727 tiles): take-off on the island 1.3 yd inside the lip, landing on the main mesh 6 yd out and
    // 8.3 yd down, then a complete path to the pit (89 / 91 yd), the switches, all four depots.
    struct SmLedge
    {
        float minX, maxX, minY, maxY;  // island box (+1 yd)
        float takeoff[3];
        float landing[3];
        char const* name;
    };
    SmLedge const SM_LEDGES[2] = {
        {819.0f, 876.0f, 112.0f, 172.0f, {823.0f, 144.7f, 329.0f}, {817.0f, 145.5f, 320.7f}, "alliance"},
        {605.0f, 663.0f, 194.0f, 254.0f, {659.8f, 229.8f, 329.0f}, {665.5f, 229.0f, 320.7f}, "horde"}};
    // The islands sit at z 328.7..330.4; the main mesh inside the same boxes is at z 319.7..322 (below the overhang) and at
    // z 366..367 (a ledge north of the Alliance box), so a z band picks the island alone.
    constexpr float SM_LEDGE_MIN_Z = 325.5f;
    constexpr float SM_LEDGE_MAX_Z = 334.0f;
    constexpr float SM_TAKEOFF_RADIUS = 3.0f;

    SmLedge const* SmLedgeOf(Player* bot)
    {
        float const z = bot->GetPositionZ();
        if (z < SM_LEDGE_MIN_Z || z > SM_LEDGE_MAX_Z)
            return nullptr;
        for (SmLedge const& l : SM_LEDGES)
            if (bot->GetPositionX() >= l.minX && bot->GetPositionX() <= l.maxX && bot->GetPositionY() >= l.minY &&
                bot->GetPositionY() <= l.maxY)
                return &l;
        return nullptr;
    }

    // ---- Match evidence (#578) --------------------------------------------------------------------------
    // One "[BGSM] carts" line per instance every 30 s and one "[BGSM] capture-seen" line per cart that leaves the map while
    // held next to a depot (BattlegroundSM::EventReopenDepot deletes the cart there and pays POINTS_PER_MINE_CART). Read
    // by whichever bot of the instance runs first; the core state is the truth, the bot only reports it.
    struct SmWatch
    {
        uint32 lastLogMs = 0;
        ObjectGuid cart[SM_MINE_CART_MAX];
        uint32 held[SM_MINE_CART_MAX] = {};
        float x[SM_MINE_CART_MAX] = {};
        float y[SM_MINE_CART_MAX] = {};
        uint32 captures[2] = {};
    };
    // Process-wide, not thread_local: an instance's map update can run on any of the MapUpdate.Threads workers
    // (MapUpdater.cpp:84/111), and a per-thread copy would see a cart "vanish" that another thread already followed.
    std::mutex s_smWatchLock;
    std::map<uint32, SmWatch> s_smWatch;

    char const* const SM_HELD_NAMES[3] = {"N", "A", "H"};
    char const* const SM_DEPOT_NAMES[4] = {"waterfall", "lava", "diamond", "troll"};

    void SmObserve(Battleground* bg, Player* who)
    {
        if (bg->GetStatus() != STATUS_IN_PROGRESS)
            return;

        std::lock_guard<std::mutex> guard(s_smWatchLock);
        SmWatch& w = s_smWatch[bg->GetInstanceID()];
        for (uint32 i = 0; i < SM_MINE_CART_MAX; ++i)
        {
            ObjectGuid const now = SM_CART_TYPES[i] < bg->BgCreatures.size() ? bg->BgCreatures[SM_CART_TYPES[i]] : ObjectGuid::Empty;
            Creature* cart = SmCart(bg, who, i);
            if (!w.cart[i].IsEmpty() && (now != w.cart[i] || !cart))
            {
                // The cart we watched is gone. Held and next to a depot = a capture (the core checks 29 yd).
                int depot = -1;
                for (uint32 d = 0; d < 4; ++d)
                    if (std::hypot(w.x[i] - BG_SM_DepotPos[d][0], w.y[i] - BG_SM_DepotPos[d][1]) < 35.0f)
                        depot = int(d);
                if (depot >= 0 && w.held[i] != SM_MINE_CART_CONTROL_NEUTRAL)
                {
                    ++w.captures[w.held[i] == SM_MINE_CART_CONTROL_ALLIANCE ? 0 : 1];
                    TC_LOG_DEBUG("playerbots", "[BGSM] capture-seen inst=%u cart=%u team=%s depot=%s score=%u-%u",
                                 bg->GetInstanceID(), i + 1, SM_HELD_NAMES[w.held[i]], SM_DEPOT_NAMES[depot],
                                 bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE));
                }
                w.cart[i] = ObjectGuid::Empty;
            }
            if (cart)
            {
                w.cart[i] = cart->GetGUID();
                w.held[i] = SmCartHeldBy(cart);
                w.x[i] = cart->GetPositionX();
                w.y[i] = cart->GetPositionY();
            }
        }

        uint32 const nowMs = getMSTime();
        if (w.lastLogMs && getMSTimeDiff(w.lastLogMs, nowMs) < 30000)
            return;
        w.lastLogMs = nowMs;

        char carts[3][40];
        for (uint32 i = 0; i < SM_MINE_CART_MAX; ++i)
        {
            if (w.cart[i].IsEmpty())
                snprintf(carts[i], sizeof(carts[i]), "-");
            else
                snprintf(carts[i], sizeof(carts[i]), "%s@%.0f,%.0f", SM_HELD_NAMES[w.held[i]], w.x[i], w.y[i]);
        }
        TC_LOG_DEBUG("playerbots", "[BGSM] carts inst=%u score=%u-%u captures=%u-%u c1=%s c2=%s c3=%s", bg->GetInstanceID(),
                     bg->GetTeamScore(TEAM_ALLIANCE), bg->GetTeamScore(TEAM_HORDE), w.captures[0], w.captures[1], carts[0],
                     carts[1], carts[2]);
    }
}

bool BGTactics::moveToStartSM(Battleground* bg)
{
    // AC moveToStart per-map idiom: spread near the gate with frand (AC BattleGroundTactics.cpp:1719-1840).
    Position const* spots = BgTeamIdOf(bot) == TEAM_HORDE ? SM_START_HORDE : SM_START_ALLIANCE;
    uint32 role = context->GetValue<uint32>("bg role")->Get();
    Position const& p = spots[role < 4 ? 0 : role > 6 ? 2 : 1];
    return MoveTo(bg->GetMapId(), p.GetPositionX() + frand(-3.0f, 3.0f), p.GetPositionY() + frand(-3.0f, 3.0f),
                  p.GetPositionZ());
}

bool BGTactics::selectObjectiveSM(Battleground* bg)
{
    TeamId team = BgTeamIdOf(bot);
    uint32 ownHeld = team == TEAM_ALLIANCE ? SM_MINE_CART_CONTROL_ALLIANCE : SM_MINE_CART_CONTROL_HORDE;
    uint32 role = context->GetValue<uint32>("bg role")->Get();

    struct Cand { Creature* cart; float score; };
    std::vector<Cand> cands;
    for (uint32 i = 0; i < SM_MINE_CART_MAX; ++i)
    {
        Creature* cart = SmCart(bg, bot, i);
        if (!cart)
            continue;

        Position cpos = cart->GetPosition();
        float dist = sServerFacade->GetDistance2d(bot, cart->GetPositionX(), cart->GetPositionY());
        uint32 own = getPlayersInArea(team, cpos, SM_CONTROL_RADIUS);
        uint32 enemy = getPlayersInArea(team == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE, cpos, SM_CONTROL_RADIUS);

        float score = dist + own * 10.0f;       // nearest first, spread away from crowded carts
        if (SmCartHeldBy(cart) == ownHeld)
        {
            score += 60.0f;                     // already ours: low priority ...
            if (enemy > 0 && own <= enemy)
                score -= 80.0f;                 // ... unless it is contested and we are outnumbered
        }
        else if (own >= 4)
            score += 60.0f;                     // enough of us already pushing it
        cands.push_back({cart, score});
    }

    PositionMap& posMap = context->GetValue<PositionMap&>("position")->Get();
    PositionInfo pos = posMap["bg objective"];

    // Track-switch duty (#578): the switch bot walks to its switch while the cart it steers is still before the fork
    // and the switch is not set the way this team wants. Everything else about the bot stays as below.
    uint32 const switchOfRole = SmSwitchOfRole(uint8(role));
    if (switchOfRole != SM_NO_SWITCH)
    {
        Creature* switchNpc = nullptr;
        if (SmSwitchNeedsClick(bg, bot, switchOfRole, team, switchNpc))
        {
            float const* stand = SM_SWITCH_STAND[switchOfRole];
            pos.Set(stand[0] + frand(-1.5f, 1.5f), stand[1] + frand(-1.5f, 1.5f), stand[2], bot->GetMapId());
            posMap["bg objective"] = pos;
            TC_LOG_DEBUG("playerbots", "[BGSM] %s -> %s track switch", bot->GetName().c_str(),
                         switchOfRole == SM_SWITCH_EAST ? "east" : "north");
            return true;
        }
    }

    if (cands.empty())
    {
        // Carts spawn some time after the gates open (BattlegroundSM::FirstMineCartSummon): wait at the pit.
        pos.Set(SM_CART_PIT.GetPositionX() + frand(-6.0f, 6.0f), SM_CART_PIT.GetPositionY() + frand(-6.0f, 6.0f),
                SM_CART_PIT.GetPositionZ(), bot->GetMapId());
        posMap["bg objective"] = pos;
        return true;
    }

    std::sort(cands.begin(), cands.end(), [](Cand const& a, Cand const& b) { return a.score < b.score; });
    // Role spread (AC AB uses bg role to split defenders/attackers): every third role takes the second best.
    size_t idx = (role % 3 == 2 && cands.size() > 1) ? 1 : 0;
    Creature* target = cands[idx].cart;

    pos.Set(target->GetPositionX(), target->GetPositionY(), target->GetPositionZ(), bot->GetMapId());
    posMap["bg objective"] = pos;
    TC_LOG_DEBUG("playerbots", "[BGSM] %s -> cart entry %u held=%u", bot->GetName().c_str(), target->GetEntry(),
                 SmCartHeldBy(target));
    return true;
}

bool BGTactics::atObjectiveSM(Battleground* bg)
{
    SmObserve(bg, bot);

    // Start platform (#578, #748): get down to the main mesh first - see SM_LEDGES. Only once the gates are open (before
    // that the bot keeps its moveToStartSM wait spot on the same platform), and also after every resurrection, since the
    // graveyard is on the platform too.
    if (bg->GetStatus() == STATUS_IN_PROGRESS && bot->IsAlive() && !bot->GetVehicle())
    {
        // Mid-jump (MotionMaster::MoveJump runs an EffectMovementGenerator): the arc passes through the island box.
        if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == EFFECT_MOTION_TYPE)
            return true;

        if (SmLedge const* ledge = SmLedgeOf(bot))
        {
            if (bot->GetExactDist2d(ledge->takeoff[0], ledge->takeoff[1]) < SM_TAKEOFF_RADIUS)
            {
                if (JumpTo(bg->GetMapId(), ledge->landing[0], ledge->landing[1], ledge->landing[2]))
                    TC_LOG_DEBUG("playerbots", "[BGSM] jump-down bot=%s ledge=%s from=%.1f,%.1f,%.1f to=%.1f,%.1f,%.1f",
                                 bot->GetName().c_str(), ledge->name, bot->GetPositionX(), bot->GetPositionY(),
                                 bot->GetPositionZ(), ledge->landing[0], ledge->landing[1], ledge->landing[2]);
                return true;
            }

            MoveTo(bg->GetMapId(), ledge->takeoff[0], ledge->takeoff[1], ledge->takeoff[2]);
            return true;
        }
    }

    // Track-switch duty (#578): click the switch when in reach (the walk there is moveToObjective's job).
    uint32 const switchOfRole = SmSwitchOfRole(uint8(context->GetValue<uint32>("bg role")->Get()));
    if (switchOfRole != SM_NO_SWITCH && bot->IsAlive())
    {
        Creature* switchNpc = nullptr;
        if (SmSwitchNeedsClick(bg, bot, switchOfRole, BgTeamIdOf(bot), switchNpc))
        {
            // Engage first, as the cart branch below does.
            if (bot->IsInCombat())
                return false;

            if (bot->GetDistance2d(switchNpc) > SM_SWITCH_CLICK_RANGE)
                return false;

            if (bot->IsMounted())
                bot->RemoveAurasByType(SPELL_AURA_MOUNTED);

            thread_local std::map<ObjectGuid, uint32> s_lastSwitchClick;
            uint32& last = s_lastSwitchClick[bot->GetGUID()];
            uint32 const now = getMSTime();
            if (!switchNpc->HasAura(BG_SM_PREVENTION_AURA) && (!last || getMSTimeDiff(last, now) >= SM_SWITCH_CLICK_GAP_MS))
            {
                TC_LOG_DEBUG("playerbots", "[BGSM] %s (bgTeam %u) clicks the %s track switch", bot->GetName().c_str(),
                             uint32(bot->GetBGTeam()), switchOfRole == SM_SWITCH_EAST ? "east" : "north");
                bot->StopMoving();
                bg->EventPlayerClickedOnFlag(bot, switchNpc);
                last = now;
            }
            return true;  // hold the post until the switch reads the way this team wants it (or the cart is past)
        }
    }

    // Nearest living cart.
    Creature* cart = nullptr;
    float best = SM_ESCORT_ENGAGE;
    for (uint32 i = 0; i < SM_MINE_CART_MAX; ++i)
    {
        Creature* c = SmCart(bg, bot, i);
        if (!c)
            continue;
        float d = sServerFacade->GetDistance2d(bot, c->GetPositionX(), c->GetPositionY());
        if (d < best)
        {
            best = d;
            cart = c;
        }
    }
    if (!cart)
        return false;

    // Engage-before-capture (AC atFlag AB): let combat strategies fight; the cart's 22-yard radius still holds.
    if (bot->IsInCombat())
        return false;

    TeamId team = BgTeamIdOf(bot);
    uint32 ownHeld = team == TEAM_ALLIANCE ? SM_MINE_CART_CONTROL_ALLIANCE : SM_MINE_CART_CONTROL_HORDE;
    Position cpos = cart->GetPosition();
    uint32 own = getPlayersInArea(team, cpos, SM_CONTROL_RADIUS);
    uint32 enemy = getPlayersInArea(team == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE, cpos, SM_CONTROL_RADIUS);

    // "A friend is already capping" (AC atFlag AB): a held, uncontested, over-escorted cart frees this bot.
    if (SmCartHeldBy(cart) == ownHeld && enemy == 0 && own > 3)
    {
        resetObjective();
        return false;
    }

    if (bot->IsMounted())
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);

    // No use-interaction exists: control is pure proximity. Keep up with the moving cart.
    if (best > SM_ESCORT_NEAR)
        MoveNear(cart, 4.0f);
    return true;
}
