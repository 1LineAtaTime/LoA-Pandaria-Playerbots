/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #201 (#613): duel-spot harness. Split out into its own file, same reason as BotScenarioQueue.cpp:
 * a harness/probe change should cost a few files plus a relink. Written to be identical on the MoP fork
 * and the AC test tree - only the core-API spelling (the duel pointer, hook names, log macro) differs,
 * and every [DUEL] line is byte-identical on both.
 *
 * Verbs (`.botscenario <name> <verb> [args]`):
 *  - `duelspot spot=<Name|index> [rematch=<s>] [scan=<ms>] [seeded=0|1] [startduel=0|1] [secs=<n>]
 *    [tp=1] [anchor=<map>,<x>,<y>,<z>[,<radius>]|anchor=reset] [pick=nearest] [hold=1]` (a stateful scenario kind,
 *    needs a bot): resolves the spot through sDuelSpotMgr->FindSpot, ChangeToDuelSpot(idx, seeded),
 *    locks the status (IsStatusLocked), pins the scan interval / rematch delay through
 *    SetScanOverride / SetRematchOverride (0 = use the config), and by default keeps the bot's
 *    "start duel" strategy on (AcceptDuelAction::Execute calls ResetStrategies, which re-rolls the
 *    25 % chance, so the harness re-asserts it every trace tick; startduel=0 leaves it to the dice).
 *    tp=1 teleports the bot onto the spot's anchor first. anchor= is a test seam that rewrites the
 *    spot's anchor (and optionally radius) in sPlayerbotAIConfig->duelSpots; anchor=reset restores it.
 *    hold=1 (T0) pins the bot IDLE where it stands (status locked, no duel-spot status, no "start duel") so
 *    `duelinfo` resolves the spot from the position only.
 *  - `duelinfo` (any online player): prints and logs one `[DUEL] INFO <name> state=<none|req|countdown|on>
 *    opp=<name|-> arb=<yd|-> spot=<name|-> idx=<i|->` line.
 *  - `duelhp <pct>` (any online player): sets health to <pct> % (T2 setup; issue it AFTER duelspot,
 *    which starts every run from full health).
 *    #201 (#614, wave 29): `duelhp <pct> [forb=1] [cd=<spellId>[,<spellId>...]]` also primes the reset test: forb=1 adds
 *    Forbearance (25771) and cd= puts a 10 min cooldown on each listed spell, so the SAMPLE rows show the reset
 *    (start: forb=1 cds>0, end after a duel inside a spot with DuelSpotReset=1: forb=0 cds=0). Issue it BEFORE
 *    `duelspot` (which neither clears cooldowns nor auras) when the first duel must see the primed state.
 *    #201 (#613 t2, #398): `duelhp <pct> hold=1` (bot only) keeps it at <pct> % until it answers its next duel
 *    request: "food" off, stood up, health clamped on every trace tick of its duelspot scenario; released (and
 *    "[SCENARIO] <bot> duelhp hold released reason=accepted|declined|timeout|stop hp=<pct>" logged) when the
 *    request is answered, after 120 s, or on stop. The decline test (T2) needs it: the target ate back above 90 %.
 *  - `startduel on|off|?` (bot): adds / removes / reports the "start duel" non-combat strategy (the
 *    existing `strategy` verb cannot carry a name with a space).
 *    #201 (#398): `startduel req <player>` (bot): on its own next tick (its duelspot trace tick or its [PROBE] row) the
 *    bot dismounts, faces <player> and casts 7266 at it (10 yd; waits up to 30 s while either has a duel), and logs
 *    "[SCENARIO] <bot> duelreq <player> cast ok=0|1 | fail=target|busy|range dist=<yd>". A deterministic third-party
 *    request for the #398 boundary (a mastered bot below 90 % declines it).
 *
 * Log lines (all via the "playerbots" debug channel, like [PROBE]; emitted only when either duellist is
 * scenario-tracked: has a scenario, a .botprobe or a masterprobe; t= is that tracked entry's own clock,
 * i.e. the same t= its [PROBE] rows carry):
 *   [DUEL] REQ <challenger> -> <target> t=<ms>
 *   [DUEL] START <a> <b> t=<ms>
 *   [DUEL] END winner=<w> loser=<l> type=WON|FLED|INTERRUPTED|SURRENDER t=<ms>
 *   [DUEL] DECLINE <bot> t=<ms>      request seen, then the duel gone without START
 *   [DUEL] SAMPLE start|end <name> hp=<pct> pwr=<pct|-> cds=<n> forb=<0|1> t=<ms>   400 ms after START / END, both duellists
 *   [DUEL] INFO ...                  duelinfo
 * and in [PROBE]: `duel=<state>:<opp|-> dspot=<idx|->` (appended after spdaura=).
 *
 * Hook registration: the harness PlayerScript is created lazily, once, by the first .botscenario /
 * .botprobe command (EnsureDuelHarnessScript, called from BotScenario.cpp) - this module file set has
 * no loader of its own. AddSC_BotScenarioDuel() below does the same and may be called from the module
 * loader instead (then the lazy path is a no-op).
 *
 * Linkage: BotScenario.cpp's HandlerRegistrar calls RegisterDuelScenarioHandlers(), which forces this
 * .obj to link (see BotScenarioInternal.h's note on RegisterVehicleScenarioHandlers).
 */

#include "BotScenario.h"
#include "BotScenarioInternal.h"

#include "Chat.h"
#include "GameObject.h"
#include "Map.h"
#include "NewRpgDuelSpot.h"
#include "NewRpgInfo.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SpellHistory.h"
#include "ScriptMgr.h"
#include "Timer.h"

#include <algorithm>
#include <atomic>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <variant>
#include <vector>

namespace BotScenario
{
namespace
{
    // One place for the core-specific log macro; every [DUEL] line is built as a plain string first so
    // the text cannot differ between the trees.
    void DuelLog(std::string const& line)
    {
        TC_LOG_DEBUG("playerbots", "%s", line.c_str());
    }

    // none | req (challenge sent, not answered) | countdown (accepted, 3 s) | on (fighting).
    char const* DuelStateName(Player const* p)
    {
        if (!p->duel)
            return "none";
        if (p->duel->startTime)
            return "on";
        if (p->duel->startTimer)
            return "countdown";
        return "req";
    }

    Player* DuelOpponent(Player const* p)
    {
        return p->duel ? p->duel->opponent : nullptr;
    }

    std::string PowerPctText(Player* p)
    {
        Powers const ptype = p->GetPowerType();
        int32 const maxPower = p->GetMaxPower(ptype);
        if (maxPower <= 0)
            return "-";
        char buf[16];
        snprintf(buf, sizeof(buf), "%.0f", 100.0f * float(p->GetPower(ptype)) / float(maxPower));
        return buf;
    }

    // Same definition as BotScenario.cpp's cds= (cooldowns still running now).
    uint32 DuelCooldownCount(Player* p)
    {
        TimeValue const now = TimeValue::Now();
        uint32 n = 0;
        for (auto const& [spellId, entry] : p->GetSpellHistory()->GetCooldowns())
            if (entry.CooldownEnd > now || entry.CategoryEnd > now)
                ++n;
        return n;
    }

    // A tracked duellist's own clock (BotScenario.cpp's ScenarioClock): the t= its [PROBE] rows carry.
    bool EventClock(Player* a, Player* b, uint32& t)
    {
        if (a && ScenarioClock(a->GetGUID(), t))
            return true;
        return b && ScenarioClock(b->GetGUID(), t);
    }

    struct PendingReq
    {
        ObjectGuid challenger;
        ObjectGuid target;
        std::string targetName;
        uint32 reqT = 0; // getMSTime() at the request
    };

    struct DuelSample
    {
        ObjectGuid a, b;
        uint32 startT = 0; // getMSTime() when queued
        char const* tag = "start";
    };

    std::mutex g_duelMutex;
    std::vector<PendingReq> g_pending;
    std::vector<DuelSample> g_samples;

    // #201 (#613 t2, #398): `duelhp <pct> hold=1` keeps the target at <pct> % until it answers its next duel request.
    // Wave 32 t2 FAILed on both cores because the low-hp target ate back above 90 % before the request arrived (AC
    // Morenia 84 -> 98 % in 3 s of "food", then accepted; MoP Hillio was skipped as "busy" while it sat eating and
    // accepted at 99 %). While held: the "food" non-combat strategy is off (restored on release), the bot is stood up,
    // and every trace tick of its own duelspot scenario (its own map thread) clamps its health to <pct>. Released when
    // the request is answered (countdown/on = accepted, gone after a request = declined), after HP_HOLD_MS, or on stop.
    struct HpHold
    {
        uint32 pct = 0;
        uint32 sinceMs = 0;
        bool sawReq = false; // set by OnDuelRequest (any thread), read on the bot's own tick
        bool foodOff = false;
    };
    constexpr uint32 HP_HOLD_MS = 120000;
    std::map<uint32, HpHold> g_hpHold; // guid counter -> hold
    std::mutex g_hpHoldMutex;

    // #201 (#398): `duelreq <target>` - the bot requests a duel from <target> on its own next tick (a third party's
    // request for the #398 decline boundary; the duel-spot scan would also challenge the master standing next to it).
    struct DuelReq
    {
        ObjectGuid target;
        std::string targetName;
        uint32 sinceMs = 0;
    };
    constexpr uint32 DUEL_REQ_WAIT_MS = 30000;
    std::map<uint32, DuelReq> g_duelReq; // challenger guid counter -> request
    std::mutex g_duelReqMutex;

    void MarkHpHoldRequest(Player* target)
    {
        std::lock_guard<std::mutex> lock(g_hpHoldMutex);
        auto const it = g_hpHold.find(target->GetGUID().GetCounter());
        if (it != g_hpHold.end())
            it->second.sawReq = true;
    }

    // Erases the pending request for the pair (either order); true and the copy when there was one.
    bool PopPending(ObjectGuid a, ObjectGuid b, PendingReq* out)
    {
        std::lock_guard<std::mutex> lock(g_duelMutex);
        for (auto it = g_pending.begin(); it != g_pending.end(); ++it)
        {
            if ((it->challenger == a && it->target == b) || (it->challenger == b && it->target == a))
            {
                if (out)
                    *out = *it;
                g_pending.erase(it);
                return true;
            }
        }
        return false;
    }

    void LogDecline(PendingReq const& req, Player* a, Player* b)
    {
        uint32 t = 0;
        EventClock(a, b, t);
        DuelLog("[DUEL] DECLINE " + req.targetName + " t=" + std::to_string(t));
    }

    void QueueSample(char const* tag, Player* a, Player* b)
    {
        DuelSample s;
        s.a = a->GetGUID();
        s.b = b->GetGUID();
        s.startT = getMSTime();
        s.tag = tag;
        std::lock_guard<std::mutex> lock(g_duelMutex);
        g_samples.push_back(s);
    }

    void LogSample(char const* tag, ObjectGuid guid)
    {
        Player* p = ObjectAccessor::FindPlayer(guid);
        if (!p)
            return;
        uint32 t = 0;
        ScenarioClock(guid, t);
        char buf[192];
        snprintf(buf, sizeof(buf), "[DUEL] SAMPLE %s %s hp=%.0f pwr=%s cds=%u forb=%d t=%u", tag, p->GetName().c_str(),
            p->GetHealthPct(), PowerPctText(p).c_str(), DuelCooldownCount(p), p->HasAura(25771) ? 1 : 0, t);
        DuelLog(buf);
    }

    // ---- the harness PlayerScript ---------------------------------------------------------------
    class BotScenarioDuelScript : public PlayerScript
    {
    public:
        BotScenarioDuelScript() : PlayerScript("BotScenarioDuelScript") {}

        void OnDuelRequest(Player* target, Player* challenger) override
        {
            if (!target || !challenger)
                return;
            MarkHpHoldRequest(target); // #201 (#613 t2): before the tracked-only early return
            uint32 t = 0;
            if (!EventClock(challenger, target, t))
                return;
            {
                std::lock_guard<std::mutex> lock(g_duelMutex);
                for (auto it = g_pending.begin(); it != g_pending.end();)
                {
                    if (it->challenger == challenger->GetGUID() && it->target == target->GetGUID())
                        it = g_pending.erase(it);
                    else
                        ++it;
                }
                PendingReq req;
                req.challenger = challenger->GetGUID();
                req.target = target->GetGUID();
                req.targetName = target->GetName();
                req.reqT = getMSTime();
                g_pending.push_back(req);
            }
            DuelLog("[DUEL] REQ " + challenger->GetName() + " -> " + target->GetName() + " t=" + std::to_string(t));
        }

        void OnDuelStart(Player* player1, Player* player2) override
        {
            if (!player1 || !player2)
                return;
            uint32 t = 0;
            if (!EventClock(player1, player2, t))
                return;
            PopPending(player1->GetGUID(), player2->GetGUID(), nullptr);
            DuelLog("[DUEL] START " + player1->GetName() + " " + player2->GetName() + " t=" + std::to_string(t));
            QueueSample("start", player1, player2);
        }

        void OnDuelEnd(Player* winner, Player* loser, DuelCompleteType type) override
        {
            if (!winner || !loser)
                return;
            uint32 t = 0;
            if (!EventClock(winner, loser, t))
                return;
            char const* typeName = "UNKNOWN";
            switch (int(type))
            {
                case 0: typeName = "INTERRUPTED"; break;
                case 1: typeName = "WON"; break;
                case 2: typeName = "FLED"; break;
                case 3: typeName = "SURRENDER"; break;
                default: break;
            }
            DuelLog("[DUEL] END winner=" + winner->GetName() + " loser=" + loser->GetName() + " type=" + typeName +
                " t=" + std::to_string(t));
            PendingReq req;
            // A request that never reached START and ended INTERRUPTED is the decline (the target's
            // /decline or AcceptDuelAction's decline both end in DuelComplete(DUEL_INTERRUPTED)).
            if (PopPending(winner->GetGUID(), loser->GetGUID(), &req) && int(type) == 0)
                LogDecline(req, winner, loser);
            QueueSample("end", winner, loser);
        }
    };

    // ---- scenario kind "duelspot" ---------------------------------------------------------------
    std::map<uint32, DuelSpotInfo> g_spotOrig; // spot index -> config values before the first anchor=
    std::mutex g_spotMutex;

    bool ParseUInt(std::string const& s, uint32& out)
    {
        if (s.empty())
            return false;
        char* end = nullptr;
        unsigned long const v = strtoul(s.c_str(), &end, 10);
        if (!end || *end)
            return false;
        out = uint32(v);
        return true;
    }

    bool ParseFloats(std::string const& s, std::vector<float>& out)
    {
        std::istringstream in(s);
        std::string part;
        while (std::getline(in, part, ','))
        {
            char* end = nullptr;
            float const v = strtof(part.c_str(), &end);
            if (part.empty() || !end || *end)
                return false;
            out.push_back(v);
        }
        return true;
    }

    // Plain-string reply: ChatHandler::PSendSysMessage is printf-style on MoP and fmt-style on AC.
    void Reply(ChatHandler* handler, std::string const& line)
    {
        handler->SendSysMessage(line.c_str());
    }

    char const* const DUELSPOT_USAGE =
        "Usage: .botscenario <botname> duelspot spot=<Name|index> [rematch=<s>] [scan=<ms>] [seeded=0|1] "
        "[startduel=0|1] [secs=<n>] [tp=1] [anchor=<map>,<x>,<y>,<z>[,<radius>]|anchor=reset] [pick=nearest] [hold=1]";

    void ApplyOverrides(Player* bot, ScenarioState const& state)
    {
        sDuelSpotMgr->SetScanOverride(bot->GetGUID(), state.duelspot.scanMs);
        sDuelSpotMgr->SetRematchOverride(bot->GetGUID(), state.duelspot.rematchS);
    }

    void EnsureStartDuel(PlayerbotAI* botAI)
    {
        if (!botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy("+start duel", BOT_STATE_NON_COMBAT);
    }

    // #201 (#612, wave-12 1:1 run): the RPG_DUEL_SPOT action runs from the "new rpg" strategy. AC's `.botscenario login`
    // does not put the bot into RandomPlayerbotMgr's current-bot set (the MoP verb does, AddCurrentBotForTest), so on
    // AC IsRandomBot() is false at login and AiFactory never adds "new rpg" (AC AiFactory.cpp:593-617): AC Server.log
    // has no "S:+new rpg" and no "A:new rpg duel spot" for any pinned bot of that run. A pinned duel-spot bot needs the
    // strategy that runs its status action; a no-op where it is present (every MoP bot). AcceptDuelAction ->
    // ResetStrategies can drop it again on AC, so it is re-asserted every trace tick like "start duel".
    void EnsureNewRpg(PlayerbotAI* botAI)
    {
        if (!botAI->HasStrategy("new rpg", BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy("+new rpg", BOT_STATE_NON_COMBAT);
    }

    // #201 (#617): `duelspot ... hold=1` pins the bot IDLE (status locked, no RPG_DUEL_SPOT) where it stands, for the
    // T0 anchor check: a free random bot otherwise walks away from the anchor before `duelinfo` (MoP t0 wave 12:
    // RazorHill/SenjinVillage/BootyBay/GurubashiArena/Halfhill/OrgrimmarGateMax resolved spot=- because the bot had
    // left on an RPG status). duelinfo then resolves the spot from the position only.
    std::set<uint32> g_hold; // guid counters of held bots
    std::mutex g_holdMutex;

    void SetHold(Player* bot, bool hold)
    {
        std::lock_guard<std::mutex> lock(g_holdMutex);
        if (hold)
            g_hold.insert(bot->GetGUID().GetCounter());
        else
            g_hold.erase(bot->GetGUID().GetCounter());
    }

    bool IsHeld(Player* bot)
    {
        std::lock_guard<std::mutex> lock(g_holdMutex);
        return g_hold.count(bot->GetGUID().GetCounter()) != 0;
    }

    // #201 (#613 t2): the duelhp hold (HpHold above). One "[SCENARIO] <bot> duelhp hold released reason=<r> hp=<pct>"
    // line per hold, same text on both trees.
    void ReleaseHpHold(Player* bot, PlayerbotAI* botAI, char const* reason)
    {
        HpHold h;
        {
            std::lock_guard<std::mutex> lock(g_hpHoldMutex);
            auto const it = g_hpHold.find(bot->GetGUID().GetCounter());
            if (it == g_hpHold.end())
                return;
            h = it->second;
            g_hpHold.erase(it);
        }
        if (h.foodOff && botAI && !botAI->HasStrategy("food", BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy("+food", BOT_STATE_NON_COMBAT);
        char buf[160];
        snprintf(buf, sizeof(buf), "[SCENARIO] %s duelhp hold released reason=%s hp=%.0f", bot->GetName().c_str(), reason,
            bot->GetHealthPct());
        DuelLog(buf);
    }

    // Runs on the held bot's own trace tick (DuelSpotUpdate).
    void ApplyHpHold(Player* bot, PlayerbotAI* botAI, uint32 now)
    {
        HpHold h;
        {
            std::lock_guard<std::mutex> lock(g_hpHoldMutex);
            auto const it = g_hpHold.find(bot->GetGUID().GetCounter());
            if (it == g_hpHold.end())
                return;
            h = it->second;
        }
        std::string const state = DuelStateName(bot);
        if (state == "countdown" || state == "on")
            return ReleaseHpHold(bot, botAI, "accepted");
        if (state == "none" && h.sawReq)
            return ReleaseHpHold(bot, botAI, "declined");
        if (getMSTimeDiff(h.sinceMs, now) > HP_HOLD_MS)
            return ReleaseHpHold(bot, botAI, "timeout");
        if (h.foodOff && botAI->HasStrategy("food", BOT_STATE_NON_COMBAT))
            botAI->ChangeStrategy("-food", BOT_STATE_NON_COMBAT); // ResetStrategies re-adds it
        if (bot->IsSitState())
            bot->SetStandState(UNIT_STAND_STATE_STAND);
        if (bot->GetHealthPct() > float(h.pct) + 0.5f)
            bot->SetHealth(std::max<uint32>(1, uint32(uint64(bot->GetMaxHealth()) * h.pct / 100)));
    }

    // #201 (#398): the queued `duelreq`, on the challenger's own tick. Waits while either side has a duel (up to
    // DUEL_REQ_WAIT_MS), then dismounts, faces and casts 7266 (10 yd). One "[SCENARIO] <bot> duelreq <target> <result>"
    // line, same text on both trees; the request itself shows as the usual [DUEL] REQ line.
    void RunDuelReq(Player* bot, uint32 now)
    {
        DuelReq r;
        {
            std::lock_guard<std::mutex> lock(g_duelReqMutex);
            auto const it = g_duelReq.find(bot->GetGUID().GetCounter());
            if (it == g_duelReq.end())
                return;
            r = it->second;
        }
        auto finish = [&](std::string const& result)
        {
            {
                std::lock_guard<std::mutex> lock(g_duelReqMutex);
                g_duelReq.erase(bot->GetGUID().GetCounter());
            }
            DuelLog("[SCENARIO] " + bot->GetName() + " duelreq " + r.targetName + " " + result);
        };
        Player* target = ObjectAccessor::FindPlayer(r.target);
        if (!target || !target->IsInWorld() || target->GetMapId() != bot->GetMapId())
            return finish("fail=target");
        if (bot->duel || target->duel || !bot->IsAlive() || !target->IsAlive())
        {
            if (getMSTimeDiff(r.sinceMs, now) > DUEL_REQ_WAIT_MS)
                finish("fail=busy");
            return;
        }
        float const dist = bot->GetExactDist(target);
        if (dist > 10.0f)
        {
            char buf[48];
            snprintf(buf, sizeof(buf), "fail=range dist=%.1f", dist);
            return finish(buf);
        }
        bot->RemoveAurasByType(SPELL_AURA_MOUNTED);
        bot->SetFacingToObject(target);
        bot->CastSpell(target, 7266, false);
        finish(bot->duel ? "cast ok=1" : "cast ok=0");
    }

    // Everything the duel harness does on a bot's own tick (its duelspot trace tick, and its [PROBE] row).
    void DuelBotTick(Player* bot, PlayerbotAI* botAI, uint32 now)
    {
        ApplyHpHold(bot, botAI, now);
        RunDuelReq(bot, now);
    }

    bool ParseDuelSpot(ChatHandler* handler, Player* bot, PlayerbotAI* botAI, std::istringstream& in,
                       ScenarioState& state)
    {
        std::string spotTok, anchorTok, tok;
        uint32 scanMs = 0, rematchS = 0, secs = 0, tmp = 0;
        bool seeded = false, startDuel = true, tp = false, pickNearest = false, hold = false;
        while (in >> tok)
        {
            if (tok.rfind("spot=", 0) == 0)
                spotTok = tok.substr(5);
            else if (tok.rfind("rematch=", 0) == 0 && ParseUInt(tok.substr(8), tmp))
                rematchS = tmp;
            else if (tok.rfind("scan=", 0) == 0 && ParseUInt(tok.substr(5), tmp))
                scanMs = tmp;
            else if (tok.rfind("seeded=", 0) == 0 && ParseUInt(tok.substr(7), tmp))
                seeded = tmp != 0;
            else if (tok.rfind("startduel=", 0) == 0 && ParseUInt(tok.substr(10), tmp))
                startDuel = tmp != 0;
            else if (tok.rfind("secs=", 0) == 0 && ParseUInt(tok.substr(5), tmp))
                secs = tmp;
            else if (tok == "tp=1")
                tp = true;
            else if (tok == "pick=nearest")
                pickNearest = true;
            else if (tok == "hold=1")
                hold = true;
            else if (tok.rfind("anchor=", 0) == 0)
                anchorTok = tok.substr(7);
            else
            {
                handler->SendSysMessage(DUELSPOT_USAGE);
                return false;
            }
        }

        if (spotTok.empty())
        {
            handler->SendSysMessage(DUELSPOT_USAGE);
            return false;
        }

        int32 const idx = sDuelSpotMgr->FindSpot(spotTok);
        if (idx < 0 || uint32(idx) >= sPlayerbotAIConfig->duelSpots.size())
        {
            Reply(handler, "botscenario: unknown duel spot '" + spotTok + "' (" +
                std::to_string(sPlayerbotAIConfig->duelSpots.size()) + " enabled spots)");
            return false;
        }

        // Validate the anchor= token fully before any side effect (a failed parse must not change state).
        std::vector<float> anchor;
        bool const anchorReset = anchorTok == "reset";
        if (!anchorTok.empty() && !anchorReset)
        {
            if (!ParseFloats(anchorTok, anchor) || (anchor.size() != 4 && anchor.size() != 5))
            {
                handler->SendSysMessage(DUELSPOT_USAGE);
                return false;
            }
        }

        {
            std::lock_guard<std::mutex> lock(g_spotMutex);
            DuelSpotInfo& spotRef = sPlayerbotAIConfig->duelSpots[idx];
            if (anchorReset)
            {
                auto const it = g_spotOrig.find(uint32(idx));
                if (it != g_spotOrig.end())
                {
                    spotRef = it->second;
                    g_spotOrig.erase(it);
                }
            }
            else if (!anchor.empty())
            {
                g_spotOrig.emplace(uint32(idx), spotRef); // keeps the FIRST original only
                spotRef.mapId = uint32(anchor[0]);
                spotRef.x = anchor[1];
                spotRef.y = anchor[2];
                spotRef.z = anchor[3];
                if (anchor.size() == 5)
                    spotRef.radius = anchor[4];
            }
        }

        DuelSpotInfo const spot = sPlayerbotAIConfig->duelSpots[idx];

        state.kind = ScenarioKind::DuelSpot;
        state.duelspot.spotIdx = uint32(idx);
        state.duelspot.seeded = seeded;
        state.duelspot.startDuel = startDuel;
        state.duelspot.scanMs = scanMs;
        state.duelspot.rematchS = rematchS;
        state.duelspot.secs = secs;
        state.pickNearest = pickNearest;
        state.destMapId = spot.mapId;
        state.destX = spot.x;
        state.destY = spot.y;
        state.destZ = spot.z;
        state.lock = true;
        state.startT = getMSTime();
        state.lastTraceT = 0;

        SetHold(bot, hold);
        ApplyOverrides(bot, state);
        bot->AttackStop();
        botAI->ChangeEngine(BOT_STATE_NON_COMBAT);
        EnsureNewRpg(botAI);
        if (startDuel && !hold)
            EnsureStartDuel(botAI);
        if (tp)
            bot->TeleportTo(spot.mapId, spot.x, spot.y, spot.z, bot->GetOrientation());
        if (hold)
            botAI->GetRpgInfo().ChangeToIdle();
        else
            botAI->GetRpgInfo().ChangeToDuelSpot(uint32(idx), seeded);

        char buf[320];
        snprintf(buf, sizeof(buf),
            "[SCENARIO] %s start duelspot spot=%s idx=%d seeded=%d scan=%u rematch=%u startduel=%d secs=%u tp=%d "
            "at=(%.1f, %.1f, %.1f) map=%u radius=%.0f hold=%d",
            bot->GetName().c_str(), spot.name.c_str(), idx, seeded ? 1 : 0, scanMs, rematchS, startDuel ? 1 : 0, secs,
            tp ? 1 : 0, spot.x, spot.y, spot.z, spot.mapId, spot.radius, hold ? 1 : 0);
        DuelLog(buf);
        return true;
    }

    bool DuelSpotUpdate(Player* bot, PlayerbotAI* botAI, ScenarioState& state, uint32 now, uint32 t)
    {
        DuelBotTick(bot, botAI, now); // #201 (#613 t2, #398)
        if (state.duelspot.secs && t >= state.duelspot.secs * IN_MILLISECONDS)
        {
            sDuelSpotMgr->SetScanOverride(bot->GetGUID(), 0);
            sDuelSpotMgr->SetRematchOverride(bot->GetGUID(), 0);
            char buf[192];
            snprintf(buf, sizeof(buf), "[SCENARIO] %s done t=%u map=%u x=%.1f y=%.1f z=%.1f", bot->GetName().c_str(), t,
                bot->GetMapId(), bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
            DuelLog(buf);
            return true;
        }

        // AcceptDuelAction::Execute -> ResetStrategies re-rolls the 25 % chance after every accepted duel.
        EnsureNewRpg(botAI);
        if (state.duelspot.startDuel && !bot->duel && !IsHeld(bot))
            EnsureStartDuel(botAI);
        return false;
    }

    void RearmDuelSpot(Player* bot, PlayerbotAI* botAI, ScenarioState& state)
    {
        ApplyOverrides(bot, state);
        EnsureNewRpg(botAI);
        bool const held = IsHeld(bot);
        if (state.duelspot.startDuel && !held)
            EnsureStartDuel(botAI);
        if (held)
            botAI->GetRpgInfo().ChangeToIdle();
        else
            botAI->GetRpgInfo().ChangeToDuelSpot(state.duelspot.spotIdx, state.duelspot.seeded);
        state.startT = getMSTime();
        state.lastTraceT = 0;
        state.lock = true;
    }

    std::string SpotNameFor(Player* p, PlayerbotAI* botAI, int32& idxOut)
    {
        idxOut = -1;
        if (botAI)
            if (auto const* ds = std::get_if<NewRpgInfo::DuelSpot>(&botAI->GetRpgInfo().data))
                idxOut = int32(ds->spotIndex);
        if (idxOut < 0)
            for (uint32 i = 0; i < sPlayerbotAIConfig->duelSpots.size(); ++i)
                if (sDuelSpotMgr->InSpot(p, i))
                {
                    idxOut = int32(i);
                    break;
                }
        if (idxOut < 0 || uint32(idxOut) >= sPlayerbotAIConfig->duelSpots.size())
            return "-";
        return sPlayerbotAIConfig->duelSpots[idxOut].name;
    }
} // anonymous namespace

void EnsureDuelHarnessScript()
{
    static std::once_flag once;
    std::call_once(once, [] { new BotScenarioDuelScript(); });
}

void RegisterDuelScenarioHandlers()
{
    RegisterScenarioHandler(ScenarioKind::DuelSpot, {"duelspot", &ParseDuelSpot, &DuelSpotUpdate, &RearmDuelSpot});
}

void DuelProbeFields(Player* p, PlayerbotAI* botAI, std::string& duel, std::string& dspot)
{
    // #201 (#398): a probed bot without a duelspot scenario (the #398 grouped bot) gets the hold / duelreq here. botAI is
    // set only for a bot's own [PROBE] row, which BotScenario::OnUpdate emits on that bot's own tick.
    if (botAI)
        DuelBotTick(p, botAI, getMSTime());
    Player* opp = DuelOpponent(p);
    duel = std::string(DuelStateName(p)) + ":" + (opp ? opp->GetName() : std::string("-"));
    dspot = "-";
    if (botAI)
        if (auto const* ds = std::get_if<NewRpgInfo::DuelSpot>(&botAI->GetRpgInfo().data))
            dspot = std::to_string(ds->spotIndex);
}

void DuelScenarioStopped(Player* bot)
{
    ReleaseHpHold(bot, GET_PLAYERBOT_AI(bot), "stop"); // #201 (#613 t2)
    {
        std::lock_guard<std::mutex> lock(g_duelReqMutex); // #201 (#398): a queued startduel req
        g_duelReq.erase(bot->GetGUID().GetCounter());
    }
    SetHold(bot, false);
    sDuelSpotMgr->SetScanOverride(bot->GetGUID(), 0);
    sDuelSpotMgr->SetRematchOverride(bot->GetGUID(), 0);
}

void DuelSweep(uint32 now)
{
    // Rate-limit (any bot's tick can drive it): 250 ms.
    static std::atomic<uint32> lastSweep{0};
    uint32 prev = lastSweep.load(std::memory_order_relaxed);
    if (prev && getMSTimeDiff(prev, now) < 250)
        return;
    if (!lastSweep.compare_exchange_strong(prev, now))
        return;

    std::vector<DuelSample> dueSamples;
    std::vector<PendingReq> snapshot;
    {
        std::lock_guard<std::mutex> lock(g_duelMutex);
        if (g_pending.empty() && g_samples.empty())
            return;
        for (auto it = g_samples.begin(); it != g_samples.end();)
        {
            if (getMSTimeDiff(it->startT, now) >= 400)
            {
                dueSamples.push_back(*it);
                it = g_samples.erase(it);
            }
            else
                ++it;
        }
        snapshot = g_pending;
    }

    for (DuelSample const& s : dueSamples)
    {
        LogSample(s.tag, s.a);
        LogSample(s.tag, s.b);
    }

    // Decline fallback: "request seen, then the duel pointer gone without START". The END(INTERRUPTED)
    // hook normally catches it first (and erases the request); this covers a request that vanished
    // without that hook. Entries older than 90 s are dropped silently.
    for (PendingReq const& req : snapshot)
    {
        uint32 const age = getMSTimeDiff(req.reqT, now);
        if (age < 1000)
            continue;
        Player* target = ObjectAccessor::FindPlayer(req.target);
        Player* challenger = ObjectAccessor::FindPlayer(req.challenger);
        if (!target)
        {
            PopPending(req.challenger, req.target, nullptr);
            continue;
        }
        bool const alive = target->duel || (challenger && challenger->duel && DuelOpponent(challenger) == target);
        if (!alive)
        {
            if (PopPending(req.challenger, req.target, nullptr))
                LogDecline(req, challenger, target);
        }
        else if (age > 90000)
            PopPending(req.challenger, req.target, nullptr);
    }
}

bool HandleDuelScenarioCommand(ChatHandler* handler, std::string const& name, std::string const& scenario,
                               std::istringstream& in)
{
    Player* target = name.empty() ? nullptr : ObjectAccessor::FindPlayerByName(name);
    if (!target)
    {
        handler->SendSysMessage(("botscenario: no online player named '" + name + "'").c_str());
        return false;
    }
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(target);

    if (scenario == "duelinfo")
    {
        Player* opp = DuelOpponent(target);
        int32 idx = -1;
        std::string const spotName = SpotNameFor(target, botAI, idx);

        std::string arb = "-";
        if (target->duel)
        {
            Map* map = target->GetMap();
            if (GameObject* flag = map ? map->GetGameObject(target->GetGuidValue(PLAYER_FIELD_DUEL_ARBITER)) : nullptr)
            {
                char dbuf[16];
                snprintf(dbuf, sizeof(dbuf), "%.1f", target->GetExactDist(flag));
                arb = dbuf;
            }
        }

        std::string const line = "[DUEL] INFO " + target->GetName() + " state=" + DuelStateName(target) +
            " opp=" + (opp ? opp->GetName() : std::string("-")) + " arb=" + arb + " spot=" + spotName +
            " idx=" + (idx >= 0 ? std::to_string(idx) : std::string("-"));
        DuelLog(line);
        handler->SendSysMessage(line.c_str());
        return true;
    }

    if (scenario == "duelhp")
    {
        uint32 pct = 0;
        if (!(in >> pct) || pct < 1 || pct > 100)
        {
            handler->SendSysMessage("botscenario: usage: botscenario <player> duelhp <1-100>");
            return false;
        }
        // #201 (#614): optional reset-test priming, see the header comment.
        bool forb = false, hold = false;
        std::vector<uint32> primeCds;
        std::string tok;
        while (in >> tok)
        {
            if (tok == "forb=1")
                forb = true;
            else if (tok == "hold=1")
                hold = true; // #201 (#613 t2): HpHold
            else if (tok.rfind("cd=", 0) == 0)
            {
                std::istringstream ids(tok.substr(3));
                std::string part;
                uint32 spellId = 0;
                while (std::getline(ids, part, ','))
                {
                    if (!ParseUInt(part, spellId))
                    {
                        handler->SendSysMessage("botscenario: usage: botscenario <player> duelhp <1-100> [forb=1] [cd=<id>,<id>] [hold=1]");
                        return false;
                    }
                    primeCds.push_back(spellId);
                }
            }
            else
            {
                handler->SendSysMessage("botscenario: usage: botscenario <player> duelhp <1-100> [forb=1] [cd=<id>,<id>] [hold=1]");
                return false;
            }
        }
        if (hold && !botAI)
        {
            handler->SendSysMessage("botscenario: duelhp hold=1 needs a bot");
            return false;
        }
        uint32 const hp = std::max<uint32>(1, uint32(uint64(target->GetMaxHealth()) * pct / 100));
        target->SetHealth(hp);
        for (uint32 spellId : primeCds)
        {
            target->GetSpellHistory()->AddCooldown(spellId, 0, 600000, 0, 0);
        }
        if (forb)
            target->AddAura(25771, target);
        if (hold)
        {
            // #201 (#613 t2): HpHold. A new hold replaces an old one (its food flag carries over).
            HpHold h;
            h.pct = pct;
            h.sinceMs = getMSTime();
            {
                std::lock_guard<std::mutex> lock(g_hpHoldMutex);
                auto const it = g_hpHold.find(target->GetGUID().GetCounter());
                h.foodOff = it != g_hpHold.end() && it->second.foodOff;
            }
            if (botAI->HasStrategy("food", BOT_STATE_NON_COMBAT))
            {
                botAI->ChangeStrategy("-food", BOT_STATE_NON_COMBAT);
                h.foodOff = true;
            }
            if (target->IsSitState())
                target->SetStandState(UNIT_STAND_STATE_STAND);
            std::lock_guard<std::mutex> lock(g_hpHoldMutex);
            g_hpHold[target->GetGUID().GetCounter()] = h;
        }
        char buf[128];
        snprintf(buf, sizeof(buf), "botscenario: %s duelhp hp=%.0f forb=%d cds=%u hold=%d", name.c_str(),
            target->GetHealthPct(), target->HasAura(25771) ? 1 : 0, DuelCooldownCount(target), hold ? 1 : 0);
        Reply(handler, buf);
        return true;
    }

    if (scenario == "startduel")
    {
        if (!botAI)
        {
            handler->SendSysMessage(("botscenario: no online bot named '" + name + "'").c_str());
            return false;
        }
        std::string mode;
        in >> mode;
        if (mode == "req")
        {
            // #201 (#398): `startduel req <target>` queues RunDuelReq (BotScenario.cpp routes only the existing verbs).
            std::string targetName;
            in >> targetName;
            Player* other = targetName.empty() ? nullptr : ObjectAccessor::FindPlayerByName(targetName);
            if (!other || other == target)
            {
                handler->SendSysMessage("botscenario: usage: botscenario <bot> startduel req <online player>");
                return false;
            }
            DuelReq r;
            r.target = other->GetGUID();
            r.targetName = other->GetName();
            r.sinceMs = getMSTime();
            {
                std::lock_guard<std::mutex> lock(g_duelReqMutex);
                g_duelReq[target->GetGUID().GetCounter()] = r;
            }
            Reply(handler, "botscenario: " + name + " startduel req " + r.targetName + " queued");
            return true;
        }
        if (mode == "on")
            botAI->ChangeStrategy("+start duel", BOT_STATE_NON_COMBAT);
        else if (mode == "off")
            botAI->ChangeStrategy("-start duel", BOT_STATE_NON_COMBAT);
        else if (mode != "?")
        {
            handler->SendSysMessage("botscenario: usage: botscenario <bot> startduel on|off|?|req <player>");
            return false;
        }
        Reply(handler, "botscenario: " + name + " startduel " +
            (botAI->HasStrategy("start duel", BOT_STATE_NON_COMBAT) ? "on" : "off"));
        return true;
    }

    handler->SendSysMessage("botscenario: unknown duel verb");
    return false;
}

void AddSC_BotScenarioDuel()
{
    EnsureDuelHarnessScript();
}

} // namespace BotScenario
