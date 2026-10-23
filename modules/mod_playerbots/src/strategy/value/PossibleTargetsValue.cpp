/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PossibleTargetsValue.h"

#include "AttackersValue.h"
#include "CellImpl.h"
#include "DBCStructure.h"
#include "GridObject.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Playerbots.h"
#include "SharedDefines.h"
#include "SpellAuraDefines.h"
#include "SpellAuraEffects.h"
#include "SpellMgr.h"
#include "Unit.h"

#include <cstdlib>
#include <ctime>

namespace
{
// #72 perf + AC parity: AC's searcher check (AC core Grids/Notifiers/GridNotifiers.h:883-899,
// Acore::AnyUnfriendlyUnitInObjectRangeCheck) rejects critters before the distance and IsFriendlyTo tests. This core's
// Trinity::AnyUnfriendlyUnitInObjectRangeCheck (Grids/Notifiers/GridNotifiers.h:779-794) has no critter test, so every
// critter in range was returned and then paid IsFriendlyTo (twice), the PlayerbotAI lookup and an aura scan in
// AttackersValue::IsPossibleTarget before being rejected there. Same checks as AC, module-local so the core check
// (used by spells and creature AI) is unchanged. AC's IsAvoidingAOE() clause only applies to a creature searcher;
// the searcher here is always the bot (a Player), so it is always true.
class BotUnfriendlyUnitInObjectRangeCheck
{
public:
    BotUnfriendlyUnitInObjectRangeCheck(WorldObject const* obj, Unit const* funit, float range)
        : i_obj(obj), i_funit(funit), i_range(range)
    {
    }

    bool operator()(Unit* u)
    {
        return u->IsAlive() && u->GetCreatureType() != CREATURE_TYPE_CRITTER && i_obj->IsWithinDistInMap(u, i_range) &&
               !i_funit->IsFriendlyTo(u);
    }

private:
    WorldObject const* i_obj;
    Unit const* i_funit;
    float i_range;
};
}  // namespace

void PossibleTargetsValue::FindUnits(std::list<Unit*>& targets)
{
    BotUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);
    Trinity::UnitListSearcher<BotUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

// #843 (AC PossibleTargetsValue.cpp:21-32): level-difference thresholds for attacking another player in the open world.
constexpr int32 EXTREME_LEVEL_DIFF = 5;  // Don't attack if enemy is this much higher
constexpr int32 HIGH_LEVEL_DIFF = 4;     // 25% chance at +/- this difference
constexpr int32 MID_LEVEL_DIFF = 3;      // 50% chance at +/- this difference
constexpr int32 LOW_LEVEL_DIFF = 2;      // 75% chance at +/- this difference

// Time window for deterministic attack decisions
constexpr uint32 ATTACK_DECISION_TIME_WINDOW = 2 * MINUTE;

// 64 bit FNV-1a hash constants
constexpr uint64 FNV_OFFSET_BASIS = 14695981039346656037ULL;
constexpr uint64 FNV_PRIME = 1099511628211ULL;

// AC's AREA_* capital ids (AC AreaDefines.h); this core ships no AreaDefines.h. Same eight zones as AC.
constexpr uint32 ZONE_STORMWIND_CITY = 1519;
constexpr uint32 ZONE_IRONFORGE = 1537;
constexpr uint32 ZONE_DARNASSUS = 1657;
constexpr uint32 ZONE_THE_EXODAR = 3557;
constexpr uint32 ZONE_ORGRIMMAR = 1637;
constexpr uint32 ZONE_THUNDER_BLUFF = 1638;
constexpr uint32 ZONE_UNDERCITY = 1497;
constexpr uint32 ZONE_SILVERMOON_CITY = 3487;

bool PossibleTargetsValue::AcceptUnit(Unit* unit)
{
    if (!AttackersValue::IsPossibleTarget(unit, bot, range))
        return false;

    // #843 (AC PossibleTargetsValue.cpp:45-127, 1:1): level-based PvP restrictions. This fork returned
    // IsPossibleTarget alone, so a random bot attacked any flagged enemy player whatever the level gap (w52 field
    // sample: 80 probe samples of MoP bots fighting a player in the open world, AC 0). NearestEnemyPlayersValue
    // and AllTargetsValue inherit this, as on AC.
    if (unit->IsPlayer())
    {
        // Self-defense - always allow fighting back
        if (bot->IsInCombat() && bot->GetVictim() == unit)
            return true;  // Already fighting

        Unit* botAttacker = bot->getAttackerForHelper();
        if (botAttacker)
        {
            if (botAttacker == unit)
                return true;  // Enemy attacking

            if (botAttacker->IsPet())
            {
                Unit* petOwner = botAttacker->GetOwner();
                if (petOwner && petOwner == unit)
                    return true;  // Enemy's pet attacking
            }
        }

        // Skip restrictions in BG/Arena
        if (bot->InBattleground() || bot->InArena())
            return true;

        // Skip restrictions if in duel with this player
        if (bot->duel && bot->duel->opponent == unit)
            return true;

        // Capital cities - no restrictions
        uint32 const zoneId = bot->GetZoneId();
        bool const inCapitalCity = (zoneId == ZONE_STORMWIND_CITY || zoneId == ZONE_IRONFORGE ||
                                    zoneId == ZONE_DARNASSUS || zoneId == ZONE_THE_EXODAR ||
                                    zoneId == ZONE_ORGRIMMAR || zoneId == ZONE_THUNDER_BLUFF ||
                                    zoneId == ZONE_UNDERCITY || zoneId == ZONE_SILVERMOON_CITY);

        if (inCapitalCity)
            return true;

        // Level difference check
        int32 const levelDifference = int32(unit->GetLevel()) - int32(bot->GetLevel());
        int32 const absLevelDifference = std::abs(levelDifference);

        // Extreme difference - do not attack
        if (levelDifference >= EXTREME_LEVEL_DIFF)
            return false;

        // Calculate attack chance based on level difference
        uint32 attackChance = 100;  // Default 100%: Bot and target's levels are very close

        // There's a chance a bot might gank on an extremly low target
        if ((absLevelDifference < EXTREME_LEVEL_DIFF && absLevelDifference >= HIGH_LEVEL_DIFF) ||
            levelDifference <= -EXTREME_LEVEL_DIFF)
            attackChance = 25;
        else if (absLevelDifference < HIGH_LEVEL_DIFF && absLevelDifference >= MID_LEVEL_DIFF)
            attackChance = 50;
        else if (absLevelDifference < MID_LEVEL_DIFF && absLevelDifference >= LOW_LEVEL_DIFF)
            attackChance = 75;

        // If probability check needed, use deterministic hash-based decision
        if (attackChance < 100)
        {
            // Decisions remain stable for ATTACK_DECISION_TIME_WINDOW.
            time_t const timeWindow = time(nullptr) / ATTACK_DECISION_TIME_WINDOW;

            // FNV-1a hash of bot GUID, target GUID and time window -> a stable percentage without a cache.
            uint64 hash = FNV_OFFSET_BASIS;
            hash ^= bot->GetGUID().GetRawValue();
            hash *= FNV_PRIME;
            hash ^= unit->GetGUID().GetRawValue();
            hash *= FNV_PRIME;
            hash ^= static_cast<uint64>(timeWindow);
            hash *= FNV_PRIME;

            // Ex: attackChance=75: hash 0-74 = attack (75%), hash 75-99 = don't attack (25%)
            return (hash % 100) < attackChance;
        }
    }

    return true;
}

void PossibleTriggersValue::FindUnits(std::list<Unit*>& targets)
{
    BotUnfriendlyUnitInObjectRangeCheck u_check(bot, bot, range);  // AC PossibleTargetsValue.cpp:139-144
    Trinity::UnitListSearcher<BotUnfriendlyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, range);
}

bool PossibleTriggersValue::AcceptUnit(Unit* unit)
{
    if (!unit->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE))
    {
        return false;
    }
    Unit::AuraEffectList const& aurasPeriodicTriggerSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL);
    Unit::AuraEffectList const& aurasPeriodicTriggerWithValueSpell =
        unit->GetAuraEffectsByType(SPELL_AURA_PERIODIC_TRIGGER_SPELL_WITH_VALUE);
    for (const Unit::AuraEffectList& list : { aurasPeriodicTriggerSpell, aurasPeriodicTriggerWithValueSpell })
    {
        for (auto i = list.begin(); i != list.end(); ++i)
        {
            AuraEffect* aurEff = *i;
            const SpellInfo* spellInfo = aurEff->GetSpellInfo();
            if (!spellInfo)
                continue;
            const SpellInfo* triggerSpellInfo =
                sSpellMgr->GetSpellInfo(spellInfo->Effects[aurEff->GetEffIndex()].TriggerSpell);
            if (!triggerSpellInfo)
                continue;
            for (int j = 0; j < MAX_SPELL_EFFECTS; j++)
            {
                if (triggerSpellInfo->Effects[j].Effect == SPELL_EFFECT_SCHOOL_DAMAGE)
                {
                    return true;
                }
            }
        }
    }
    return false;
}
