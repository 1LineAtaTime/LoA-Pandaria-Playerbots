/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: The Stonecore (epic #373, issue #662). See CataScShared.h for sources.

#include "CataScShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include <list>
#include <vector>

namespace CataSc
{
namespace
{
struct HazardSpec
{
    uint32 entry;
    float radius;
};

struct DynHazardSpec
{
    uint32 spellId;
    float radius;
};

HazardSpec const& GetSpec(HazardId id)
{
    // Order = enum HazardId. Radii are Wowpedia's (read 2026-10-06).
    static HazardSpec const specs[] = {
        { NPC_LAVA_FISSURE, 5.0f },  // Lava Fissure / Eruption / lava pool: 5 yd
        { NPC_GRAVITY_WELL, 10.0f }, // Gravity Well: pulls everything within 10 yd (Wowhead 79340)
    };

    return specs[static_cast<uint8>(id)];
}

DynHazardSpec const& GetDynSpec(DynHazardId id)
{
    // Order = enum DynHazardId. SpellEffect.dbc 86881: persistent area aura, 5 yd. 80654 (the stalactite warning
    // circle) has no radius of its own; the missile that follows (80647) hits within 2 yd, so 3.5 covers the circle.
    static DynHazardSpec const specs[] = {
        { SPELL_STALACTITE_AREA, 3.5f },
        { SPELL_CRYSTAL_BARRAGE, 5.0f },
    };

    return specs[static_cast<uint8>(id)];
}
}  // namespace

Creature* FindCreature(Player* bot, uint32 entry, float range)
{
    return bot->FindNearestCreature(entry, range, true);
}

bool IsCasting(Unit* unit, uint32 spellId)
{
    return unit && unit->HasUnitState(UNIT_STATE_CASTING) && unit->FindCurrentSpellBySpellId(spellId);
}

float HazardSafeRadius(HazardId id)
{
    return GetSpec(id).radius + HAZARD_SAFE_EXTRA;
}

Creature* GetHazardCreature(Player* bot, HazardId id)
{
    if (!bot->IsInCombat())
        return nullptr;

    HazardSpec const& spec = GetSpec(id);
    float const inside = spec.radius + HAZARD_MARGIN;

    Creature* best = nullptr;
    float bestDist = 0.0f;
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, spec.entry, inside);
    for (Creature* creature : found)
    {
        if (!creature->IsAlive())
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist >= inside)
            continue;

        if (!best || dist < bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

float DynHazardSafeRadius(DynHazardId id)
{
    return GetDynSpec(id).radius + HAZARD_SAFE_EXTRA;
}

// Persistent area auras (Crystal Barrage, the stalactite warning circle) exist as DynamicObjects of the spell
bool GetDynHazard(Player* bot, DynHazardId id, float& cx, float& cy)
{
    if (!bot->IsInCombat())
        return false;

    DynHazardSpec const& spec = GetDynSpec(id);
    float const inside = spec.radius + HAZARD_MARGIN;

    bool found = false;
    float best = 0.0f;
    for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, inside + 5.0f, spec.spellId))
    {
        float const dist = bot->GetExactDist2d(pos);
        if (dist >= inside)
            continue;

        if (!found || dist < best)
        {
            found = true;
            best = dist;
            cx = pos.GetPositionX();
            cy = pos.GetPositionY();
        }
    }

    return found;
}

// Ozruk ------------------------------------------------------------------------------------------------------------
Creature* GetOzruk(Player* bot)
{
    return FindCreature(bot, NPC_OZRUK, 45.0f);
}

// Shatter (3 s cast): everything within 15 yd of Ozruk takes 47k+ (188k heroic). The tank leaves too.
bool ShatterDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = GetOzruk(bot);
    if (!IsCasting(boss, SPELL_SHATTER))
        return false;

    return bot->GetExactDist2d(boss) < SHATTER_SAFE_RADIUS - HAZARD_SAFE_EXTRA;
}

// Ground Slam (3 s cast) lands in front of Ozruk (4 yd at the impact point). The bot he is attacking takes it by
// design; everybody else stays out of the front arc. The heroic Rupture spike lines are not scripted on the fork.
bool GroundSlamDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = GetOzruk(bot);
    if (!IsCasting(boss, SPELL_GROUND_SLAM) || boss->GetVictim() == bot)
        return false;

    return InFrontArc(bot, boss, GROUND_SLAM_ARC, GROUND_SLAM_RANGE);
}

// Slabhide ---------------------------------------------------------------------------------------------------------
Creature* GetSlabhide(Player* bot)
{
    return FindCreature(bot, NPC_SLABHIDE, 40.0f);
}

// Sand Blast is instant (nothing to react to): 15 yd, 60 degree cone in front of Slabhide, "should only hit the tank"
bool SandBlastDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = GetSlabhide(bot);
    if (!boss || boss->GetVictim() == bot || !boss->IsInCombat())
        return false;

    return InFrontArc(bot, boss, SAND_BLAST_ARC, SAND_BLAST_RANGE);
}

bool InFrontArc(Player* bot, Unit* boss, float arc, float range)
{
    if (bot->GetExactDist2d(boss) > range)
        return false;

    return boss->HasInArc(arc, bot);
}

// Focus ------------------------------------------------------------------------------------------------------------
bool IsFocusDps(Player* bot)
{
    return PlayerBotSpec::IsDps(bot) && !PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsHeal(bot);
}

Unit* GetFocusTarget(Player* bot, FocusId id)
{
    if (!bot->IsInCombat())
        return nullptr;

    switch (id)
    {
        case FocusId::CRYSTAL_SHARDS:
            return FindCreature(bot, NPC_CRYSTAL_SHARD, SCAN_RANGE);
        case FocusId::AZIL_FOLLOWERS:
        {
            // Only while Azil has her Energy Shield up (-75% damage taken)
            Creature* azil = FindCreature(bot, NPC_AZIL, SCAN_RANGE);
            if (!azil || !azil->HasAura(SPELL_ARCANE_SHIELD))
                return nullptr;

            return FindCreature(bot, NPC_DEVOUT_FOLLOWER, SCAN_RANGE);
        }
        default:
            return nullptr;
    }
}

Unit* GetAnyFocusTarget(Player* bot)
{
    for (uint8 i = 0; i < static_cast<uint8>(FocusId::COUNT); ++i)
    {
        if (Unit* target = GetFocusTarget(bot, static_cast<FocusId>(i)))
            return target;
    }

    return nullptr;
}

bool InEvadeDanger(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    if (ShatterDanger(bot) || GroundSlamDanger(bot) || SandBlastDanger(bot))
        return true;

    for (uint8 i = 0; i < static_cast<uint8>(HazardId::COUNT); ++i)
    {
        if (GetHazardCreature(bot, static_cast<HazardId>(i)))
            return true;
    }

    float cx = 0.0f;
    float cy = 0.0f;
    for (uint8 i = 0; i < static_cast<uint8>(DynHazardId::COUNT); ++i)
    {
        if (GetDynHazard(bot, static_cast<DynHazardId>(i), cx, cy))
            return true;
    }

    return false;
}
}  // namespace CataSc
