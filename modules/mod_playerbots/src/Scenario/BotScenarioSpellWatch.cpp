/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #793: spell-hit / aura probe for boss-mechanic evidence. Its own file, like BotScenarioDuel.cpp, so a change costs a
 * few files plus a relink. Written to be identical on the MoP fork and the AC test tree - only the core-API spelling
 * (hook signatures, script base classes, log macro) differs, and every [SPELLWATCH] line is byte-identical on both.
 *
 * Verb (no player needed; the name slot is "-"):
 *   `.botscenario - spellwatch <id,id,...> on`    adds the ids to the watch set
 *   `.botscenario - spellwatch <id,id,...> off`   removes those ids (other runs' ids stay watched)
 *   `.botscenario - spellwatch off`               clears the whole watch set
 *   `.botscenario - spellwatch list`              prints the watch set
 * The watch set is server-global. Issue the verb over SOAP / the console (World::ProcessCliCommands), as every harness
 * driver does: the first `on` registers the hook script (see "Hook registration").
 *
 * While a watched id is in the set, one line per event goes to the "playerbots" debug channel (same channel as [PROBE]):
 *   [SPELLWATCH] cast|hit|aura+|aura- spell=<id> caster=<who> target=<who> amount=<n> map=<id>  iid=<instance id> (w106: map-wide lines mixed two SSB runs)
 *   <who> = the player's name, or <entry>:<guidlow> for a creature (player:<guidlow> / - when the unit is not resolvable).
 *   cast   a successful cast finish (end of Spell::cast / AC Spell::_cast); target = the spell's explicit unit target or -;
 *          amount=0. It is logged after the spell's immediate effects, so an instant spell's hit/aura lines come first.
 *   hit    damage or heal done, amount = the value the core hands its script hook: direct spell damage after class
 *          mitigation, before absorb/resist (Unit::CalculateSpellDamageTaken); a periodic damage / leech tick before
 *          bonuses (AuraEffect::HandlePeriodic{Damage,HealthLeech}AurasTick); a heal (Unit::HealBySpell) or a periodic
 *          heal tick (AuraEffect::HandlePeriodicHealAurasTick) before absorb. A periodic heal tick fires two core hooks
 *          on both cores (the periodic tick, then "heal received"); it is logged once.
 *   aura+  an aura applied to a unit, or a stack added / refreshed on an existing one; amount = stack count.
 *   aura-  an aura application removed from a unit; amount = stack count at removal.
 *
 * Thread safety: the hooks run on the map update threads (MapUpdate.Threads > 1). The watch set is an immutable sorted
 * vector published through one std::atomic pointer (release store by the command, acquire load by the hooks). The
 * command builds a new vector under a mutex and swaps the pointer; an old vector is never freed (kept in a retired list
 * until shutdown) so a map thread still holding it can finish its binary_search. A few bytes per command.
 * Cost: with the set empty the pointer is null and every hook returns after one atomic load. Before the first `on` the
 * hook script is not even registered.
 *
 * Hook registration: the script objects are created lazily, once, by the first `spellwatch ... on` (std::call_once), the
 * same contract as EnsureDuelHarnessScript (BotScenarioDuel.cpp): the command runs on the world thread while the map
 * threads are parked (World::Update has returned from MapManager::Update, which waits for them), so the script
 * registry is not being iterated while it grows.
 *
 * Core hooks used (MoP fork, added for #793 as mirrors of AC's): UnitScript::ModifySpellDamageTaken /
 * ModifyPeriodicDamageAurasTick (SpellInfo argument added), ModifyHealReceived, OnAuraApply, OnAuraRemove,
 * OnUnitSpellCast (AC: AllSpellScript::OnSpellCast).
 */

#include "BotScenario.h"
#include "BotScenarioInternal.h"

#include "Chat.h"
#include "Log.h"
#include "MoveSpline.h"        // mount watch: smode= (status movement mode)
#include "ObjectGuid.h"
#include "Player.h"            // mount watch
#include "Playerbots.h"        // mount watch: PlayerbotAI, "current target"
#include "ScriptMgr.h"
#include "Spell.h"
#include "SpellAuraEffects.h"  // mount watch: AuraEffect::GetId
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "Timer.h"
#include "Unit.h"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <set>
#include <sstream>
#include <string>
#include <unordered_map>
#include <vector>

namespace BotScenario
{
namespace
{
    // One place for the core-specific log macro; every [SPELLWATCH] line is built as a plain string first.
    void SpellWatchLog(std::string const& line)
    {
        TC_LOG_DEBUG("playerbots", "%s", line.c_str());
    }

    // ---- the watch set ------------------------------------------------------------------------
    using WatchSet = std::vector<uint32>;                          // sorted, unique, never modified once published
    std::atomic<WatchSet const*> g_watch{nullptr};                 // null = nothing watched (the fast path)
    std::mutex g_watchMutex;                                       // writers only (the command)
    std::vector<std::unique_ptr<WatchSet const>> g_watchRetired;   // every published set, kept alive (see header)

    bool Watched(uint32 spellId)
    {
        WatchSet const* w = g_watch.load(std::memory_order_acquire);
        return w && std::binary_search(w->begin(), w->end(), spellId);
    }

    std::string SetText(WatchSet const* w)
    {
        if (!w || w->empty())
            return "-";
        std::string out;
        for (uint32 id : *w)
            out += (out.empty() ? "" : ",") + std::to_string(id);
        return out;
    }

    // ---- line formatting ----------------------------------------------------------------------
    std::string GuidText(ObjectGuid guid)
    {
        if (guid.IsEmpty())
            return "-";
        if (guid.IsPlayer())
            return "player:" + std::to_string(guid.GetCounter());
        return std::to_string(guid.GetEntry()) + ":" + std::to_string(guid.GetCounter());
    }

    std::string UnitText(Unit const* u)
    {
        if (!u)
            return "-";
        if (u->GetTypeId() == TYPEID_PLAYER)
            return u->GetName();
        return GuidText(u->GetGUID());
    }

    void Emit(char const* ev, uint32 spellId, std::string const& caster, std::string const& target, int64 amount,
              WorldObject const* where)
    {
        SpellWatchLog(std::string("[SPELLWATCH] ") + ev + " spell=" + std::to_string(spellId) + " caster=" + caster +
                      " target=" + target + " amount=" + std::to_string(amount) + " map=" + std::to_string(where->GetMapId()) + " iid=" + std::to_string(where->GetInstanceId()));
    }

    // A periodic heal tick calls the periodic-tick hook and then the heal-received hook with the same
    // (target, caster, spell, amount) on the same thread; the second call is the duplicate and is skipped.
    struct PeriodicMark
    {
        Unit const* target = nullptr;
        Unit const* caster = nullptr;
        uint32 spellId = 0;
        int64 amount = -1;
    };
    thread_local PeriodicMark t_lastPeriodic;

    // ---- the shared event bodies (called by the core-specific overrides below) ------------------
    void OnDirectDamage(Unit* target, Unit* attacker, int64 damage, SpellInfo const* spellInfo)
    {
        if (!spellInfo || !target || !Watched(spellInfo->Id))
            return;
        Emit("hit", spellInfo->Id, UnitText(attacker), UnitText(target), damage, target);
    }

    void OnPeriodicTick(Unit* target, Unit* attacker, int64 amount, SpellInfo const* spellInfo)
    {
        if (!spellInfo || !target || !Watched(spellInfo->Id))
            return;
        t_lastPeriodic.target = target;
        t_lastPeriodic.caster = attacker;
        t_lastPeriodic.spellId = spellInfo->Id;
        t_lastPeriodic.amount = amount;
        Emit("hit", spellInfo->Id, UnitText(attacker), UnitText(target), amount, target);
    }

    // first/second: the core's argument order. #829: on this core both call sites pass (target, healer) now - Unit::HealBySpell
    // was switched to the hook's declared order (the AC core still passes (healer, target) there; the AC copy of this file keeps
    // that). The periodic heal tick passes (target, healer) right after its periodic-tick hook, which OnPeriodicTick already
    // logged; the mark lives only between those two hooks of one tick, so it is matched without the amount (AutoBalance, when
    // on, scales the heal in between).
    void OnHealReceived(Unit* first, Unit* second, int64 heal, SpellInfo const* spellInfo)
    {
        if (!spellInfo || !Watched(spellInfo->Id))
            return;
        PeriodicMark& m = t_lastPeriodic;
        if (m.spellId == spellInfo->Id && m.target == first && m.caster == second)
        {
            m = PeriodicMark();
            return;
        }
        Unit* healer = second;
        Unit* target = first;
        Unit* where = target ? target : healer;
        if (!where)
            return;
        Emit("hit", spellInfo->Id, UnitText(healer), UnitText(target), heal, where);
    }

    void OnAuraApplied(Unit* unit, Aura* aura)
    {
        if (!unit || !aura || !Watched(aura->GetId()))
            return;
        Unit* caster = aura->GetCaster();
        Emit("aura+", aura->GetId(), caster ? UnitText(caster) : GuidText(aura->GetCasterGUID()), UnitText(unit),
             aura->GetStackAmount(), unit);
    }

    void OnAuraRemoved(Unit* unit, AuraApplication* aurApp)
    {
        if (!unit || !aurApp)
            return;
        Aura* aura = aurApp->GetBase();
        if (!aura || !Watched(aura->GetId()))
            return;
        Unit* caster = aura->GetCaster();
        Emit("aura-", aura->GetId(), caster ? UnitText(caster) : GuidText(aura->GetCasterGUID()), UnitText(unit),
             aura->GetStackAmount(), unit);
    }

    void OnCastFinished(Spell* spell, Unit* caster, SpellInfo const* spellInfo)
    {
        if (!spellInfo || !caster || !Watched(spellInfo->Id))
            return;
        Unit* target = spell ? spell->m_targets.GetUnitTarget() : nullptr;
        Emit("cast", spellInfo->Id, UnitText(caster), UnitText(target), 0, caster);
    }

    // ---- the hook scripts (MoP fork: one UnitScript carries every hook, the cast hook included) ---
    class BotScenarioSpellWatchScript : public UnitScript
    {
    public:
        BotScenarioSpellWatchScript() : UnitScript("BotScenarioSpellWatchScript") {}

        void ModifySpellDamageTaken(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnDirectDamage(target, attacker, damage, spellInfo);
        }

        void ModifyPeriodicDamageAurasTick(Unit* target, Unit* attacker, int32& damage, SpellInfo const* spellInfo) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnPeriodicTick(target, attacker, damage, spellInfo);
        }

        void ModifyHealReceived(Unit* target, Unit* healer, uint32& heal, SpellInfo const* spellInfo) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnHealReceived(target, healer, heal, spellInfo);
        }

        void OnAuraApply(Unit* unit, Aura* aura) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnAuraApplied(unit, aura);
        }

        void OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveMode /*mode*/) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnAuraRemoved(unit, aurApp);
        }

        void OnUnitSpellCast(Spell* spell, Unit* caster, SpellInfo const* spellInfo, bool /*skipCheck*/) override
        {
            if (!g_watch.load(std::memory_order_acquire))
                return;
            OnCastFinished(spell, caster, spellInfo);
        }
    };

    void EnsureSpellWatchScript()
    {
        static std::once_flag once;
        std::call_once(once, [] { new BotScenarioSpellWatchScript(); });
    }

    // "76207,76213" -> ids; false on an empty list or a token that is not a positive number
    bool ParseIds(std::string const& text, std::set<uint32>& ids)
    {
        std::istringstream in(text);
        std::string tok;
        while (std::getline(in, tok, ','))
        {
            if (tok.empty())
                continue;
            char* end = nullptr;
            unsigned long v = std::strtoul(tok.c_str(), &end, 10);
            if (!end || *end != '\0' || v == 0 || v > 0xFFFFFFFFul)
                return false;
            ids.insert(uint32(v));
        }
        return !ids.empty();
    }

    // under g_watchMutex: publish `next` (empty -> null)
    void Publish(std::set<uint32> const& next)
    {
        if (next.empty())
        {
            g_watch.store(nullptr, std::memory_order_release);
            return;
        }
        std::unique_ptr<WatchSet const> snap(new WatchSet(next.begin(), next.end()));
        g_watch.store(snap.get(), std::memory_order_release);
        g_watchRetired.push_back(std::move(snap));
    }
} // anonymous namespace

bool HandleSpellWatchCommand(ChatHandler* handler, std::istringstream& in)
{
    std::string a, b;
    in >> a >> b;

    if (a == "list" || a.empty())
    {
        std::string const now = SetText(g_watch.load(std::memory_order_acquire));
        handler->SendSysMessage(("botscenario: spellwatch ids=" + now).c_str());
        return true;
    }

    std::lock_guard<std::mutex> lock(g_watchMutex);
    WatchSet const* cur = g_watch.load(std::memory_order_acquire);
    std::set<uint32> next;
    if (cur)
        next.insert(cur->begin(), cur->end());

    std::string verb;
    std::set<uint32> ids;
    if (a == "off" && b.empty())
    {
        verb = "off";
        next.clear();
    }
    else if ((b == "on" || b == "off") && ParseIds(a, ids))
    {
        verb = b;
        if (b == "on")
        {
            EnsureSpellWatchScript();
            next.insert(ids.begin(), ids.end());
        }
        else
            for (uint32 id : ids)
                next.erase(id);
    }
    else
    {
        handler->SendSysMessage("botscenario: usage: botscenario - spellwatch <id,id,...> on|off | off | list");
        return false;
    }

    Publish(next);
    std::string const now = SetText(g_watch.load(std::memory_order_acquire));
    std::string reqText;
    for (uint32 id : ids)
        reqText += (reqText.empty() ? "" : ",") + std::to_string(id);
    if (reqText.empty())
        reqText = "all";
    SpellWatchLog("[SCENARIO] spellwatch " + verb + " ids=" + reqText + " watching=" + now);
    // plain-string reply: PSendSysMessage is printf-style on MoP and fmt-style on AC
    handler->SendSysMessage(("botscenario: spellwatch " + verb + " ids=" + reqText + " watching=" + now).c_str());
    return true;
}

// =====================================================================================================================
// mount watch - the server-side truth for "bots stay visually mounted while fighting in battlegrounds".
// Same file as the spell watch (a new .cpp would need a CMake re-run: the module sources are globbed). Written to be
// identical on the MoP fork and the AC test tree; only the core-API spelling (Mw* helpers below, the UnitScript base
// constructor, the log macro) differs, and every [MNTW] line is byte-identical in layout on both.
//
// Verb (no player needed; the name slot is "-"):
//   `.botscenario - mountwatch <mapId> on [rowMs]`   watch every PLAYERBOT on map <mapId> (every instance); rowMs = the
//                                                    periodic row interval (default 5000, min 250)
//   `.botscenario - mountwatch off`                  stop (and forget the per-bot state)
//   `.botscenario - mountwatch list`                 print the current setting
// Sampling: on each bot's own AI tick (BotScenario::OnUpdate, before the scenario fast path), at most every 250 ms.
// Lines (playerbots debug channel, like [PROBE]):
//   [MNTW] chg|row <name> g=<guidlow> ut=<unix s.ms> iid=<instance> cls=<class> team=A|H alive=0|1 ghost=0|1 ds=<deathState>
//          flag=<UNIT_FLAG_MOUNT> disp=<mount display id> aura=<first SPELL_AURA_MOUNTED spell|0> nau=<count> form=<shapeshift>
//          cmb=<IsInCombat> bst=<bot engine state> tgt=<current target|-> tdist=<yd|-1> cast=<generic spell|0>
//          smode=<status-movement 0|1, '-' on AC> hp=<pct> x= y= z= mcms=<ms mounted+in combat so far> bad=<tags|->
//     chg = the (alive, ghost, flag, disp, aura, cmb, bad) tuple changed since the bot's last line; row = periodic.
//     bad tags (server-side inconsistencies): disp_noaura (alive, display set, no mount aura), aura_noflag, flag_noaura,
//     dead_mounted (dead or ghost with the flag or a display still set).
//   [MNTW] mcend <name> g= ut= dur=<ms> cls= tdist=<at the end>   a mounted+in-combat span ended (any reason)
//   [MNTW] aura+|aura- <name> g= ut= spell= flag= disp= alive= mode=<AuraRemoveMode, 0 on aura+>   a mount aura was applied /
//          removed on a player on the watched map (bots and real players). aura- runs after the effect handlers
//          (Unit::_UnapplyAura), so flag=1 or disp!=0 on an aura- line means the removal did not clear the mount state.
// Thread safety: the setting is two atomics; the per-bot table is guarded by a mutex (a few dozen bots, 4 Hz). The command
// runs on the world thread while the map threads are parked (see "Hook registration" above).
// =====================================================================================================================
namespace
{
    // ---- core-API spelling (MoP fork) -------------------------------------------------------------------------------
    uint8 MwClass(Player const* p) { return p->GetClass(); }
    bool MwGhost(Player const* p) { return p->HasFlag(PLAYER_FIELD_PLAYER_FLAGS, PLAYER_FLAGS_GHOST); }
    uint32 MwDisp(Unit const* u) { return u->GetMountDisplayId(); }
    char const* MwStatusMode(Unit const* u) { return (u->movespline && u->movespline->statusMode) ? "1" : "0"; }
    int MwDeathState(Unit* u) { return int(u->getDeathState()); }

    std::atomic<uint32> g_mwMap{0};       // 0 = off
    std::atomic<uint32> g_mwRowMs{5000};

    struct MountWatchBot
    {
        uint32 lastSampleMs = 0;
        uint32 lastRowMs = 0;
        uint32 mcStartMs = 0;            // getMSTime() when mounted (flag or aura) + in combat began; 0 = not in that state
        std::string key;
    };
    std::mutex g_mwMutex;
    std::unordered_map<uint32, MountWatchBot> g_mwBots;

    void MountWatchLog(std::string const& line)
    {
        TC_LOG_DEBUG("playerbots", "%s", line.c_str());
    }

    std::string MwUt()
    {
        uint64 const ms = uint64(std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()).count());
        char buf[32];
        std::snprintf(buf, sizeof(buf), "%llu.%03u", (unsigned long long)(ms / 1000), unsigned(ms % 1000));
        return buf;
    }

    uint32 MwMountAura(Unit* u, uint32& count)
    {
        auto const& list = u->GetAuraEffectsByType(SPELL_AURA_MOUNTED);
        count = uint32(list.size());
        return list.empty() ? 0 : list.front()->GetId();
    }

    void MwAuraEvent(char const* ev, Unit* unit, Aura const* aura, int mode)
    {
        uint32 const map = g_mwMap.load(std::memory_order_relaxed);
        if (!map || !unit || !aura || unit->GetTypeId() != TYPEID_PLAYER || unit->GetMapId() != map)
            return;
        SpellInfo const* info = aura->GetSpellInfo();
        if (!info || !info->HasAura(SPELL_AURA_MOUNTED))
            return;
        char buf[256];
        std::snprintf(buf, sizeof(buf), " g=%u ut=%s spell=%u flag=%d disp=%u alive=%d mode=%d", unit->GetGUID().GetCounter(),
                      MwUt().c_str(), info->Id, unit->IsMounted() ? 1 : 0, MwDisp(unit), unit->IsAlive() ? 1 : 0, mode);
        MountWatchLog(std::string("[MNTW] ") + ev + " " + unit->GetName() + buf);
    }

    class BotScenarioMountWatchScript : public UnitScript
    {
    public:
        BotScenarioMountWatchScript() : UnitScript("BotScenarioMountWatchScript") {}

        void OnAuraApply(Unit* unit, Aura* aura) override
        {
            if (!g_mwMap.load(std::memory_order_relaxed))
                return;
            MwAuraEvent("aura+", unit, aura, 0);
        }

        void OnAuraRemove(Unit* unit, AuraApplication* aurApp, AuraRemoveMode mode) override
        {
            if (!g_mwMap.load(std::memory_order_relaxed) || !aurApp)
                return;
            MwAuraEvent("aura-", unit, aurApp->GetBase(), int(mode));
        }
    };

    void EnsureMountWatchScript()
    {
        static std::once_flag once;
        std::call_once(once, [] { new BotScenarioMountWatchScript(); });
    }
} // anonymous namespace

void MountWatchTick(PlayerbotAI* botAI)
{
    uint32 const map = g_mwMap.load(std::memory_order_relaxed);
    if (!map || !botAI)
        return;
    Player* bot = botAI->GetBot();
    if (!bot || !bot->IsInWorld() || bot->GetMapId() != map)
        return;

    uint32 const now = getMSTime();
    uint32 const guidLow = bot->GetGUID().GetCounter();

    uint32 nAura = 0;
    uint32 const aura = MwMountAura(bot, nAura);
    bool const flag = bot->IsMounted();
    uint32 const disp = MwDisp(bot);
    bool const alive = bot->IsAlive();
    bool const ghost = MwGhost(bot);
    bool const cmb = bot->IsInCombat();
    bool const mounted = flag || aura;

    std::string bad;
    if (alive && disp && !aura)
        bad += "disp_noaura,";
    if (aura && !flag)
        bad += "aura_noflag,";
    if (flag && !aura)
        bad += "flag_noaura,";
    if ((!alive || ghost) && (flag || disp))
        bad += "dead_mounted,";
    if (bad.empty())
        bad = "-";
    else
        bad.pop_back();

    std::string const key = std::to_string(alive) + std::to_string(ghost) + std::to_string(flag) + ":" +
                            std::to_string(disp) + ":" + std::to_string(aura) + ":" + std::to_string(cmb) + ":" + bad;

    bool emitLine = false;
    char const* ev = "row";
    uint32 mcms = 0;
    uint32 mcEnded = 0;
    {
        std::lock_guard<std::mutex> lock(g_mwMutex);
        MountWatchBot& s = g_mwBots[guidLow];
        if (s.lastSampleMs && getMSTimeDiff(s.lastSampleMs, now) < 250)
            return;
        s.lastSampleMs = now;

        if (mounted && cmb)
        {
            if (!s.mcStartMs)
                s.mcStartMs = now ? now : 1;
            mcms = getMSTimeDiff(s.mcStartMs, now);
        }
        else if (s.mcStartMs)
        {
            mcEnded = getMSTimeDiff(s.mcStartMs, now);
            if (!mcEnded)
                mcEnded = 1;
            s.mcStartMs = 0;
        }

        if (key != s.key)
        {
            emitLine = true;
            ev = "chg";
            s.key = key;
            s.lastRowMs = now;
        }
        else if (getMSTimeDiff(s.lastRowMs, now) >= g_mwRowMs.load(std::memory_order_relaxed))
        {
            emitLine = true;
            s.lastRowMs = now;
        }
    }

    Unit* target = botAI->GetAiObjectContext()->GetValue<Unit*>("current target")->Get();
    float tdist = -1.0f;
    std::string tname = "-";
    if (target && target->IsInWorld() && target->GetMapId() == bot->GetMapId())
    {
        tdist = bot->GetExactDist(target);
        tname = target->GetName();
    }
    std::string const ut = MwUt();

    if (mcEnded)
    {
        char buf[160];
        std::snprintf(buf, sizeof(buf), " g=%u ut=%s dur=%u cls=%u tdist=%.1f", guidLow, ut.c_str(), mcEnded,
                      uint32(MwClass(bot)), tdist);
        MountWatchLog("[MNTW] mcend " + bot->GetName() + buf);
    }

    if (!emitLine)
        return;

    Spell* spell = bot->GetCurrentSpell(CURRENT_GENERIC_SPELL);
    uint32 const castId = (spell && spell->GetSpellInfo()) ? spell->GetSpellInfo()->Id : 0;
    char buf[512];
    std::snprintf(buf, sizeof(buf),
                  " g=%u ut=%s iid=%u cls=%u team=%s alive=%d ghost=%d ds=%d flag=%d disp=%u aura=%u nau=%u form=%u cmb=%d "
                  "bst=%d tgt=%s tdist=%.1f cast=%u smode=%s hp=%.0f x=%.1f y=%.1f z=%.1f mcms=%u bad=%s",
                  guidLow, ut.c_str(), bot->GetInstanceId(), uint32(MwClass(bot)),
                  bot->GetTeamId() == TEAM_ALLIANCE ? "A" : "H", alive ? 1 : 0, ghost ? 1 : 0, MwDeathState(bot),
                  flag ? 1 : 0, disp, aura, nAura, uint32(bot->GetShapeshiftForm()), cmb ? 1 : 0, int(botAI->GetState()),
                  tname.c_str(), tdist, castId, MwStatusMode(bot), bot->GetHealthPct(), bot->GetPositionX(),
                  bot->GetPositionY(), bot->GetPositionZ(), mcms, bad.c_str());
    MountWatchLog(std::string("[MNTW] ") + ev + " " + bot->GetName() + buf);
}

bool HandleMountWatchCommand(ChatHandler* handler, std::istringstream& in)
{
    std::string a, b, c;
    in >> a >> b >> c;

    if (a == "list" || a.empty())
    {
        handler->SendSysMessage(("botscenario: mountwatch map=" + std::to_string(g_mwMap.load()) +
                                 " rowMs=" + std::to_string(g_mwRowMs.load())).c_str());
        return true;
    }

    if (a == "off")
    {
        g_mwMap.store(0, std::memory_order_relaxed);
        {
            std::lock_guard<std::mutex> lock(g_mwMutex);
            g_mwBots.clear();
        }
        MountWatchLog("[SCENARIO] mountwatch off ut=" + MwUt());
        handler->SendSysMessage("botscenario: mountwatch off");
        return true;
    }

    char* end = nullptr;
    unsigned long const map = std::strtoul(a.c_str(), &end, 10);
    if (!end || *end != '\0' || map == 0 || b != "on")
    {
        handler->SendSysMessage("botscenario: usage: botscenario - mountwatch <mapId> on [rowMs] | off | list");
        return false;
    }
    uint32 rowMs = 5000;
    if (!c.empty())
    {
        unsigned long const v = std::strtoul(c.c_str(), &end, 10);
        if (end && *end == '\0' && v >= 250)
            rowMs = uint32(v);
    }

    EnsureMountWatchScript();
    {
        std::lock_guard<std::mutex> lock(g_mwMutex);
        g_mwBots.clear();
    }
    g_mwRowMs.store(rowMs, std::memory_order_relaxed);
    g_mwMap.store(uint32(map), std::memory_order_relaxed);
    MountWatchLog("[SCENARIO] mountwatch on map=" + std::to_string(map) + " rowMs=" + std::to_string(rowMs) + " ut=" + MwUt());
    handler->SendSysMessage(("botscenario: mountwatch on map=" + std::to_string(map) + " rowMs=" + std::to_string(rowMs)).c_str());
    return true;
}

} // namespace BotScenario
