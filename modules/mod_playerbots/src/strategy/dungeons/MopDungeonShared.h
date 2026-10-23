/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Shared building blocks for the MoP 5-man dungeon strategies (issue #781, epic #373). Header-only on purpose:
// the module sources are collected by a configure-time GLOB_RECURSE (modules/CMakeLists.txt:45), so a new .cpp
// would need a CMake re-configure; a header next to DungeonStrategyUtils.h does not.
//
// AC WotLK has no MoP content, so nothing here ports an AC file 1:1. Each piece EXTENDS an AC dungeon pattern:
//   * priority interrupt gate  - AC Ai/Dungeon/MgT/MgTStrategy.cpp:23 ("mgt priority interrupt", ACTION_INTERRUPT + 5)
//                                and MgTShared.cpp:1128-1192 (every bot with an interrupt tries). Extended so the
//                                2-3 interrupters of a 5-man do not all burn their interrupt on the same cast: the
//                                ready interrupters are ranked in group order and each waits rank * 500 ms, so the
//                                second one only fires when the first one did not land (any bot whose interrupt is
//                                off cooldown takes part, the tank's included).
//   * add waves / target switch - AC Ai/Dungeon/AK/AKTriggers.cpp:11-19 (NadoxGuardianTrigger: boss + add present),
//                                AKActions.cpp:10-18 (AttackNadoxGuardianAction: Attack(add) unless already on it),
//                                AKStrategy.cpp:13 (ACTION_RAID + 5), AKMultipliers.cpp:15-29 (ElderNadoxMultiplier:
//                                DpsAssistAction -> 0 while the add lives); the phase switch is the same shape as
//                                AC FoS AttackCorruptedSoulFragmentAction (FoSActions.cpp:24-52) and AN
//                                WatchersTargetAction (ANActions.cpp:36-100). Extended with a tank role (the tank
//                                picks up an add that is not on him) and an optional AoE veto for armoured adds.
//   * safe-spot sampling       - the fork's TJS FindSafePoint (MopTjsShared.cpp) reduced to a scoring callback,
//                                each candidate validated with EncounterHelpers::CanTakeStepTowards.

#ifndef PLAYERBOTS_MOPDUNGEONSHARED_H
#define PLAYERBOTS_MOPDUNGEONSHARED_H

#include "AiObjectContext.h"
#include "AttackActions.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Multiplier.h"
#include "Pet.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellHistory.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Timer.h"
#include "Trigger.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <iterator>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace MopDungeon
{
// ---------------------------------------------------------------------------------------------------------------
// Priority interrupts
// ---------------------------------------------------------------------------------------------------------------

// How long the next ready interrupter waits behind the previous one (a kick lands within one server tick, a ranged
// interrupt within ~200 ms; 500 ms leaves room for a bot that is mid-GCD).
constexpr uint32 INTERRUPT_STAGGER_MS = 500;
// A cast the gate has not looked at for this long counts as a new cast when it shows up again.
constexpr uint32 INTERRUPT_GATE_FORGET_MS = 1500;
// Members further than this from the caster are not counted as interrupters.
constexpr float INTERRUPT_COUNT_RANGE = 45.0f;

// The dedicated interrupts of the 5.4.8 classes that the dungeon strategies cast by name ("kick", "pummel", ...).
// Ids and names checked in the 5.4.8 Spell.dbc on 2026-10-11. The felhunter's Spell Lock is a pet spell: see
// PetInterruptReady() below.
inline std::vector<uint32> const& InterruptSpellIds()
{
    static std::vector<uint32> const ids = {
        1766,    // Kick
        6552,    // Pummel
        57994,   // Wind Shear
        47528,   // Mind Freeze
        2139,    // Counterspell
        96231,   // Rebuke
        116705,  // Spear Hand Strike
        106839,  // Skull Bash
        80964,   // Skull Bash (bear id kept in the 5.4.8 DBC)
        80965,   // Skull Bash (cat id kept in the 5.4.8 DBC)
        147362,  // Counter Shot
        34490,   // Silencing Shot
        15487,   // Silence
        132409,  // Spell Lock (the warlock's own copy after Grimoire of Sacrifice)
    };
    return ids;
}

// The felhunter's Spell Lock (19647). AC counts it as a dedicated interrupt (MgTShared.cpp:546 "spell lock") and the
// "spell id" value resolves it from the pet's spell book (AC and fork SpellIdValue.cpp, "Pet* pet = bot->GetPet()"),
// so a warlock whose felhunter has it ready is an interrupter too.
constexpr uint32 SPELL_FELHUNTER_SPELL_LOCK = 19647;

inline bool PetInterruptReady(Player* player, Unit* caster)
{
    Pet* pet = player ? player->GetPet() : nullptr;
    if (!pet || !pet->IsAlive() || !pet->IsInWorld() || !caster || pet->GetMap() != caster->GetMap() ||
        !pet->HasSpell(SPELL_FELHUNTER_SPELL_LOCK))
        return false;

    SpellInfo const* info = sSpellMgr->GetSpellInfo(SPELL_FELHUNTER_SPELL_LOCK);
    // SpellHistory::HasCooldown is private in the fork (SpellHistory.h:123); IsReady is the public test (as OCActions.cpp:38).
    if (!info || !pet->GetSpellHistory() || !pet->GetSpellHistory()->IsReady(SPELL_FELHUNTER_SPELL_LOCK))
        return false;

    return pet->GetDistance(caster) <= std::max(info->GetMaxRange(false), 5.0f) + 1.0f;
}

// Longest range of an interrupt `player` knows and has off cooldown; 0 when it has none ready.
inline float ReadyInterruptRange(Player* player)
{
    if (!player || !player->IsAlive())
        return 0.0f;

    float best = 0.0f;
    for (uint32 const id : InterruptSpellIds())
    {
        if (!player->HasSpell(id) || player->HasSpellCooldown(id))
            continue;

        float range = 5.0f;
        if (SpellInfo const* info = sSpellMgr->GetSpellInfo(id))
            range = std::max(info->GetMaxRange(false), 5.0f);

        best = std::max(best, range);
    }

    return best;
}

inline bool HasReadyInterrupt(Player* player) { return ReadyInterruptRange(player) > 0.0f; }

// Id of the spell `caster` is casting or channelling right now (0 when none).
inline uint32 CurrentCastSpellId(Unit* caster)
{
    if (!caster)
        return 0;

    for (CurrentSpellTypes const slot : {CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL})
    {
        if (Spell* spell = caster->GetCurrentSpell(slot))
        {
            if (spell->m_spellInfo)
                return spell->m_spellInfo->Id;
        }
    }

    return 0;
}

// Can `member` interrupt `caster` right now: alive, on the bot's map, not channelling, an interrupt off cooldown and
// the caster within its range (+1 yd).
inline bool CanInterruptNow(Player* member, Unit* caster)
{
    if (!member || !caster || !member->IsAlive() || !member->IsInWorld() || member->GetMap() != caster->GetMap())
        return false;

    if (member->GetCurrentSpell(CURRENT_CHANNELED_SPELL))
        return false;

    float const range = ReadyInterruptRange(member);
    if (range > 0.0f && member->GetDistance(caster) <= range + 1.0f)
        return true;

    return PetInterruptReady(member, caster);
}

// Number of bots ahead of `bot` (group order) that can interrupt `caster` right now. Real players are not counted:
// the bots cannot know whether they will kick.
inline uint32 InterrupterRank(Player* bot, Unit* caster)
{
    Group* group = bot->GetGroup();
    if (!group)
        return 0;

    uint32 rank = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member)
            continue;

        if (member == bot)
            return rank;

        if (!GET_PLAYERBOT_AI(member) || member->GetExactDist(caster) > INTERRUPT_COUNT_RANGE)
            continue;

        if (CanInterruptNow(member, caster))
            ++rank;
    }

    return rank;
}

// Bots of the group (this one included) that can interrupt `caster` right now.
inline uint32 CountInterrupters(Player* bot, Unit* caster)
{
    Group* group = bot->GetGroup();
    if (!group)
        return CanInterruptNow(bot, caster) ? 1 : 0;

    uint32 count = 0;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (member && GET_PLAYERBOT_AI(member) && CanInterruptNow(member, caster))
            ++count;
    }

    return count;
}

// Per-bot gate (one instance per interrupt trigger): true when this bot may spend its interrupt on `caster`'s current
// cast now. The first ready interrupter fires at once, the second 500 ms later if the cast is still running, ...
class InterruptGate
{
public:
    bool Allow(Player* bot, Unit* caster)
    {
        if (!bot || !caster || !CanInterruptNow(bot, caster))
            return false;

        uint32 const spellId = CurrentCastSpellId(caster);
        if (!spellId)
            return false;

        uint32 const now = getMSTime();
        if (caster->GetGUID() != _caster || spellId != _spellId || getMSTimeDiff(_lastSeen, now) > INTERRUPT_GATE_FORGET_MS)
        {
            _caster = caster->GetGUID();
            _spellId = spellId;
            _firstSeen = now;
        }

        _lastSeen = now;
        return getMSTimeDiff(_firstSeen, now) >= InterrupterRank(bot, caster) * INTERRUPT_STAGGER_MS;
    }

private:
    ObjectGuid _caster;
    uint32 _spellId = 0;
    uint32 _firstSeen = 0;
    uint32 _lastSeen = 0;
};

// ---------------------------------------------------------------------------------------------------------------
// Add waves and phase / target switches (AC AK Nadox guardian pattern, FoS soul fragment, AN watchers)
// ---------------------------------------------------------------------------------------------------------------

enum class WaveRole : uint8
{
    Tank,  // the tank picks up an add that is not attacking him
    Dps,   // the damage dealers (not the tank, not a healer) kill the add first
    All    // every non-healer switches to it (phase switch: the boss is out, this unit is the fight now)
};

struct WaveSpec
{
    uint32 boss;                // the encounter's boss entry (alive and in combat within 100 yd); 0 = no boss check
    std::vector<uint32> adds;   // add entries, in priority order (earlier entries win for the damage dealers)
    WaveRole role;
    float range;                // scan radius around the bot
};

inline bool WaveRoleMatches(Player* bot, WaveRole role)
{
    bool const tank = PlayerBotSpec::IsTank(bot);
    bool const heal = PlayerBotSpec::IsHeal(bot);
    switch (role)
    {
        case WaveRole::Tank:
            return tank;
        case WaveRole::Dps:
            return !tank && !heal && PlayerBotSpec::IsDps(bot);
        case WaveRole::All:
            return !heal || tank;
    }

    return false;
}

// The add this bot should attack under `spec`, or nullptr. Damage dealers take the first entry in `adds` that has a
// live, attackable, engaged unit (lowest health first, so the group focuses one add); the tank takes the nearest add
// that is not attacking him; WaveRole::All takes the nearest unit of the first entry present.
inline Unit* PickWaveTarget(Player* bot, WaveSpec const& spec)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || !WaveRoleMatches(bot, spec.role))
        return nullptr;

    if (spec.boss)
    {
        Creature* boss = bot->FindNearestCreature(spec.boss, 100.0f, true);
        if (!boss || !boss->IsInCombat())
            return nullptr;
    }

    bool const tankPickup = spec.role == WaveRole::Tank;
    for (uint32 const entry : spec.adds)
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, spec.range);

        Unit* best = nullptr;
        float bestKey = FLT_MAX;
        for (Creature* add : found)
        {
            if (!add || !add->IsAlive() || !add->IsInCombat() || !AttackersValue::IsValidTarget(add, bot))
                continue;

            if (tankPickup && add->GetVictim() == bot)
                continue;

            float const key = spec.role == WaveRole::Dps ? add->GetHealthPct() * 1000.0f + bot->GetExactDist2d(add)
                                                         : bot->GetExactDist2d(add);
            if (key < bestKey)
            {
                best = add;
                bestKey = key;
            }
        }

        // the tank picks up whatever is loose; the damage dealers keep the priority order of `adds`
        if (best)
            return best;
    }

    return nullptr;
}

// Active while PickWaveTarget() has a unit and the bot is not already attacking it (AC AKTriggers.cpp:11-19 shape).
class MopWaveTrigger : public Trigger
{
public:
    MopWaveTrigger(PlayerbotAI* botAI, std::string const name, WaveSpec spec)
        : Trigger(botAI, name, 500), _spec(std::move(spec))
    {
    }

    bool IsActive() override
    {
        Unit* target = PickWaveTarget(bot, _spec);
        if (!target)
            return false;

        return AI_VALUE(Unit*, "current target") != target;
    }

private:
    WaveSpec const _spec;
};

// Attacks the unit PickWaveTarget() names (AC AKActions.cpp:10-18 AttackNadoxGuardianAction shape).
class MopWaveAction : public AttackAction
{
public:
    MopWaveAction(PlayerbotAI* botAI, std::string const name, WaveSpec spec)
        : AttackAction(botAI, name), _spec(std::move(spec))
    {
    }

    bool Execute(Event /*event*/) override
    {
        Unit* target = PickWaveTarget(bot, _spec);
        if (!target || AI_VALUE(Unit*, "current target") == target)
            return false;

        return Attack(target);
    }

private:
    WaveSpec const _spec;
};

// While the wave has a unit for this bot, the generic target pickers must not take it back to the boss
// (AC AKMultipliers.cpp:15-29: DpsAssistAction -> 0; the tank's TankAssistAction for the tank roles). With
// `vetoAoe` the AoE actions are vetoed too (AC ANMultipliers.cpp:54-57), for adds that shrug off area damage.
class MopWaveMultiplier : public Multiplier
{
public:
    MopWaveMultiplier(PlayerbotAI* botAI, std::string const name, WaveSpec spec, bool vetoAoe = false)
        : Multiplier(botAI, name), _spec(std::move(spec)), _vetoAoe(vetoAoe)
    {
    }

    float GetValue(Action* action) override
    {
        bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<TankAssistAction*>(action) ||
                            dynamic_cast<DpsAoeAction*>(action);
        bool const aoe = _vetoAoe && action->getThreatType() == Action::ActionThreatType::Aoe;
        if (!picker && !aoe)
            return 1.0f;

        if (!WaveRoleMatches(bot, _spec.role) || !PickWaveTarget(bot, _spec))
            return 1.0f;

        return 0.0f;
    }

private:
    WaveSpec const _spec;
    bool const _vetoAoe;
};

// ---------------------------------------------------------------------------------------------------------------
// Safe-spot sampling
// ---------------------------------------------------------------------------------------------------------------

// Samples rings around the bot and returns the walkable point with the highest `score(x, y)` that beats the bot's
// own score by at least `minGain`. Candidates are checked with EncounterHelpers::CanTakeStepTowards (no wall, no
// cliff); at most `maxChecks` of them, best first.
template <class Score>
bool FindBestSpot(Player* bot, Score score, float minGain, float& outX, float& outY, float& outZ,
                  uint32 maxChecks = 12)
{
    static constexpr float RADII[] = {4.0f, 7.0f, 10.0f, 14.0f, 19.0f};
    static constexpr int BEARINGS = 16;

    struct Candidate
    {
        float x;
        float y;
        float score;
    };

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const here = score(bx, by);

    std::vector<Candidate> candidates;
    candidates.reserve(std::size(RADII) * BEARINGS);
    for (float const radius : RADII)
    {
        for (int i = 0; i < BEARINGS; ++i)
        {
            float const angle = float(i) * 2.0f * float(M_PI) / float(BEARINGS);
            float const x = bx + std::cos(angle) * radius;
            float const y = by + std::sin(angle) * radius;
            float const s = score(x, y);
            if (s >= here + minGain)
                candidates.push_back({x, y, s - radius * 0.05f});  // the shorter walk wins a tie
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](Candidate const& a, Candidate const& b) { return a.score > b.score; });

    uint32 checks = 0;
    for (Candidate const& candidate : candidates)
    {
        if (++checks > maxChecks)
            break;

        float sx = 0.0f;
        float sy = 0.0f;
        float sz = 0.0f;
        float const dist = std::hypot(candidate.x - bx, candidate.y - by);
        if (!EncounterHelpers::CanTakeStepTowards(bot, candidate.x, candidate.y, dist, sx, sy, sz))
            continue;

        outX = sx;
        outY = sy;
        outZ = sz;
        return true;
    }

    return false;
}
}  // namespace MopDungeon

#endif
