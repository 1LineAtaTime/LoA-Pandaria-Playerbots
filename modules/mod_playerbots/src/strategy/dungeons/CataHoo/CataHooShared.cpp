/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataHooShared.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Unit.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace CataHoo
{
Creature* FindNearestAlive(Player* bot, uint32 entry, float range)
{
    return bot->FindNearestCreature(entry, range, true);
}

void CollectAlive(Player* bot, std::vector<uint32> const& entries, float range, std::vector<Creature*>& out)
{
    // 5.4.8: WorldObject::GetCreatureListWithEntryInGrid takes a single entry (Object.h:574); the Append variant
    // (Object.h:577) is used per entry, the same way MgTShared.cpp:273-288 does.
    std::list<Creature*> found;
    for (uint32 entry : entries)
        bot->GetCreatureListWithEntryInGridAppend(found, entry, range);

    for (Creature* creature : found)
    {
        if (creature && creature->IsAlive())
            out.push_back(creature);
    }
}

Creature* FindEngagedBoss(Player* bot, uint32 entry, float range)
{
    Creature* boss = bot->FindNearestCreature(entry, range, true);
    return boss && boss->IsInCombat() ? boss : nullptr;
}

Creature* LowestGuid(std::vector<Creature*> const& units)
{
    Creature* best = nullptr;
    for (Creature* unit : units)
    {
        if (!best || unit->GetGUID().GetRawValue() < best->GetGUID().GetRawValue())
            best = unit;
    }

    return best;
}

bool IsCasting(Unit* caster, uint32 spellId)
{
    return caster && caster->HasUnitState(UNIT_STATE_CASTING) && caster->FindCurrentSpellBySpellId(spellId) != nullptr;
}

Unit* GetCastTarget(Player* bot, Unit* caster, uint32 spellId)
{
    Spell* spell = caster ? caster->FindCurrentSpellBySpellId(spellId) : nullptr;
    if (!spell)
        return nullptr;

    // GetUnitTarget() can point at a stale unit before the spell has updated (Spell.h:665), so go through the guid.
    ObjectGuid const guid = spell->m_targets.GetUnitTargetGUID();
    if (guid.IsEmpty())
        return nullptr;

    return ObjectAccessor::GetUnit(*bot, guid);
}

void CollectGroupMembers(Player* bot, std::vector<Player*>& out, bool includeSelf)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsInWorld() || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (member == bot && !includeSelf)
            continue;

        out.push_back(member);
    }
}

bool OthersCentroid(Player* bot, float& cx, float& cy)
{
    std::vector<Player*> others;
    CollectGroupMembers(bot, others, false);
    if (others.empty())
        return false;

    float sx = 0.0f;
    float sy = 0.0f;
    for (Player* member : others)
    {
        sx += member->GetPositionX();
        sy += member->GetPositionY();
    }

    cx = sx / static_cast<float>(others.size());
    cy = sy / static_cast<float>(others.size());
    return true;
}

float AngleDiff(float a, float b)
{
    float diff = std::fmod(a - b, 2.0f * PI_F);
    if (diff > PI_F)
        diff -= 2.0f * PI_F;
    else if (diff <= -PI_F)
        diff += 2.0f * PI_F;

    return diff;
}

float GroundZ(Player* bot, float x, float y)
{
    float const z = bot->GetPositionZ();
    float const ground = bot->GetMapWaterOrGroundLevel(x, y, z);
    return ground > INVALID_HEIGHT ? ground : z;
}

// 5.4.8: SpellInfo::Effects[] / SpellEffectInfo::CalcRadius() (no caster: the base radius), the same use as
// CataZaShared.cpp SpellRadius, but for one effect (a dummy effect can carry a much larger radius than the damage).
float EffectRadius(uint32 spellId, uint8 effIndex, float fallback)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo || effIndex >= MAX_SPELL_EFFECTS)
        return fallback;

    float const radius = spellInfo->Effects[effIndex].CalcRadius();
    return radius > 0.0f ? radius : fallback;
}

bool InsideHazard(Player* bot, std::vector<Hazard> const& hazards, float margin)
{
    for (Hazard const& hazard : hazards)
    {
        if (bot->GetExactDist2d(hazard.x, hazard.y) < hazard.radius + margin)
            return true;
    }

    return false;
}

bool ClearStep(Player* bot, float x, float y)
{
    // Same call UBActions.cpp:34 uses; it can clamp the destination, so a clamped step counts as blocked
    // (EncounterHelpers.cpp CanTakeStepTowards does the same truncation check).
    float dx = x;
    float dy = y;
    float dz = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), dx, dy, dz))
        return false;

    return std::hypot(dx - x, dy - y) <= 1.0f;
}

bool FindSafeSpot(Player* bot, std::vector<Hazard> const& hazards, float margin, Leash const& leash,
                  float maxTravel, float& outX, float& outY)
{
    if (hazards.empty())
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();

    for (float ring = 3.0f; ring <= maxTravel + 0.01f; ring += 3.0f)
    {
        bool found = false;
        float bestClearance = -1.0f;
        for (int k = 0; k < 16; ++k)
        {
            float const angle = static_cast<float>(k) * (PI_F / 8.0f);
            float const x = bx + ring * std::cos(angle);
            float const y = by + ring * std::sin(angle);

            if (leash.radius > 0.0f && std::hypot(x - leash.x, y - leash.y) > leash.radius)
                continue;

            float clearance = 1000.0f;
            bool blocked = false;
            for (Hazard const& hazard : hazards)
            {
                float const edge = std::hypot(x - hazard.x, y - hazard.y) - (hazard.radius + margin);
                if (edge < 0.5f)
                {
                    blocked = true;
                    break;
                }

                clearance = std::min(clearance, edge);
            }

            if (blocked || !ClearStep(bot, x, y))
                continue;

            if (clearance > bestClearance)
            {
                bestClearance = clearance;
                outX = x;
                outY = y;
                found = true;
            }
        }

        if (found)
            return true;
    }

    return false;
}

bool InsideCone(float originX, float originY, float axis, float halfAngle, float length, float x, float y)
{
    float const dist = std::hypot(x - originX, y - originY);
    if (dist > length)
        return false;

    if (dist < 0.5f)
        return true;

    return std::fabs(AngleDiff(std::atan2(y - originY, x - originX), axis)) <= halfAngle;
}

bool FindConeExit(Player* bot, float originX, float originY, float axis, float halfAngle, float length,
                  float angleMargin, float maxMove, float& outX, float& outY)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    if (!InsideCone(originX, originY, axis, halfAngle + angleMargin, length, bx, by))
        return false;

    float const radius = std::max(std::hypot(bx - originX, by - originY), 3.0f);
    float const diff = AngleDiff(std::atan2(by - originY, bx - originX), axis);
    float const preferred = diff >= 0.0f ? 1.0f : -1.0f;

    for (float side : { preferred, -preferred })
    {
        float const bearing = axis + side * (halfAngle + angleMargin + 0.06f);
        float const x = originX + radius * std::cos(bearing);
        float const y = originY + radius * std::sin(bearing);
        if (std::hypot(x - bx, y - by) <= maxMove)
        {
            outX = x;
            outY = y;
            return true;
        }
    }

    return false;
}
}

namespace CataHoo
{
namespace
{
Leash PlatformLeash()
{
    return { PLATFORM_X, PLATFORM_Y, PLATFORM_RADIUS };
}

bool GetSearingLightHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_TEMPLE_GUARDIAN_ANHUUR))
        return false;

    std::vector<Creature*> lights;
    CollectAlive(bot, { NPC_SEARING_LIGHT }, 80.0f, lights);
    for (Creature* light : lights)
        hazards.push_back({ light->GetPositionX(), light->GetPositionY(), SEARING_LIGHT_RADIUS });

    return !hazards.empty();
}

// A bot that carries Divine Reckoning must not stand within 15 yd of anybody else: they would all take the pulse (and
// feed the boss). Tanks keep the boss where it is and are left alone.
bool GetReckoningHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!bot->HasAura(SPELL_DIVINE_RECKONING) || PlayerBotSpec::IsTank(bot) ||
        !FindEngagedBoss(bot, NPC_TEMPLE_GUARDIAN_ANHUUR))
        return false;

    std::vector<Player*> others;
    CollectGroupMembers(bot, others, false);
    for (Player* member : others)
        hazards.push_back({ member->GetPositionX(), member->GetPositionY(), RECKONING_RADIUS });

    return !hazards.empty();
}

// Radii include the margin; FindSafeSpot/InsideHazard are then called with margin 0.
bool GetQuicksandHazard(Player* bot, std::vector<Hazard>& hazards)
{
    Creature* ptah = FindNearestAlive(bot, NPC_EARTHRAGER_PTAH, 200.0f);
    if (ptah && ptah->HasAura(SPELL_QUICKSAND))
        hazards.push_back({ ptah->GetPositionX(), ptah->GetPositionY(), QUICKSAND_RADIUS + QUICKSAND_MARGIN });

    // #691: the pools (npc 40503), summoned near the players while he is submerged and left until the encounter ends
    std::vector<Creature*> pools;
    CollectAlive(bot, { NPC_QUICKSAND_POOL }, 100.0f, pools);
    float const poolRadius = std::max(EffectRadius(SPELL_QUICKSAND_DMG, 0, QUICKSAND_POOL_RADIUS),
                                      EffectRadius(SPELL_QUICKSAND_SLOW, 0, QUICKSAND_POOL_RADIUS)) + QUICKSAND_POOL_MARGIN;
    for (Creature* pool : pools)
        hazards.push_back({ pool->GetPositionX(), pool->GetPositionY(), poolRadius });

    return !hazards.empty();
}

// #691 Setesh: the Reign of Chaos bubbles and the Chaos Blasts (before the landing the impact radius, after it the zone's).
void GetSeteshZones(Player* bot, std::vector<Hazard>& bubbles, std::vector<Hazard>& blasts)
{
    if (!FindEngagedBoss(bot, NPC_SETESH))
        return;

    std::vector<Creature*> units;
    CollectAlive(bot, { NPC_REIGN_OF_CHAOS }, 100.0f, units);
    float const bubbleRadius = EffectRadius(SPELL_REIGN_OF_CHAOS_DMG, 1, REIGN_OF_CHAOS_RADIUS) + CHAOS_MARGIN;
    for (Creature* bubble : units)
        bubbles.push_back({ bubble->GetPositionX(), bubble->GetPositionY(), bubbleRadius });

    units.clear();
    CollectAlive(bot, { NPC_CHAOS_BLAST }, 100.0f, units);
    float const impactRadius = EffectRadius(SPELL_CHAOS_BLAST, 0, CHAOS_BLAST_RADIUS) + CHAOS_MARGIN;
    float const zoneRadius = EffectRadius(SPELL_CHAOS_BURN, 0, CHAOS_BURN_RADIUS) + CHAOS_MARGIN;
    for (Creature* blast : units)
        blasts.push_back({ blast->GetPositionX(), blast->GetPositionY(),
                           blast->HasAura(SPELL_CHAOS_BLAST) ? zoneRadius : std::max(impactRadius, zoneRadius) });
}

// #691 Rajh: the Solar Winds, the falling Sun Orbs and the Inferno Leap marker.
void GetRajhZones(Player* bot, std::vector<Hazard>& winds, std::vector<Hazard>& orbs, std::vector<Hazard>& leaps)
{
    if (!FindEngagedBoss(bot, NPC_RAJH))
        return;

    std::vector<Creature*> units;
    CollectAlive(bot, { NPC_SOLAR_WINDS_1, NPC_SOLAR_WINDS_2, NPC_SOLAR_WINDS_3 }, 100.0f, units);
    for (Creature* wind : units)
    {
        // Only the vortex that carries the pulsing aura hurts (the others are summoned shells).
        if (wind->HasAura(SPELL_SOLAR_WINDS_AURA))
            winds.push_back({ wind->GetPositionX(), wind->GetPositionY(), SOLAR_WINDS_RADIUS + SOLAR_WINDS_MARGIN });
    }

    units.clear();
    CollectAlive(bot, { NPC_ORB_OF_SUN }, 100.0f, units);
    float const orbRadius = EffectRadius(SPELL_BLAZING_INFERNO, 1, SUN_ORB_RADIUS) + RAJH_HAZARD_MARGIN;
    for (Creature* orb : units)
        orbs.push_back({ orb->GetPositionX(), orb->GetPositionY(), orbRadius });

    units.clear();
    CollectAlive(bot, { NPC_INFERNO_LEAP }, 100.0f, units);
    float const leapRadius = EffectRadius(SPELL_INFERNO_LEAP, 0, INFERNO_LEAP_RADIUS) + RAJH_HAZARD_MARGIN;
    for (Creature* marker : units)
        leaps.push_back({ marker->GetPositionX(), marker->GetPositionY(), leapRadius });
}

// Inside one of `own`: a spot clear of every zone in `all`.
bool EscapeAll(Player* bot, std::vector<Hazard> const& own, std::vector<Hazard> const& all, Leash const& leash,
               float maxTravel, float& x, float& y)
{
    return !own.empty() && InsideHazard(bot, own, 0.0f) && FindSafeSpot(bot, all, 0.0f, leash, maxTravel, x, y);
}

enum class RajhZone { Winds, Orb, Leap };

bool RajhEscape(Player* bot, RajhZone zone, float& x, float& y)
{
    std::vector<Hazard> winds;
    std::vector<Hazard> orbs;
    std::vector<Hazard> leaps;
    GetRajhZones(bot, winds, orbs, leaps);

    std::vector<Hazard> all(winds);
    all.insert(all.end(), orbs.begin(), orbs.end());
    all.insert(all.end(), leaps.begin(), leaps.end());

    std::vector<Hazard> const& own = zone == RajhZone::Winds ? winds : zone == RajhZone::Orb ? orbs : leaps;
    Leash const room = { RAJH_ROOM_X, RAJH_ROOM_Y, RAJH_ROOM_RADIUS };
    return EscapeAll(bot, own, all, room, 24.0f, x, y);
}

bool SeteshEscape(Player* bot, bool bubbles, float& x, float& y)
{
    std::vector<Hazard> reign;
    std::vector<Hazard> blasts;
    GetSeteshZones(bot, reign, blasts);

    std::vector<Hazard> all(reign);
    all.insert(all.end(), blasts.begin(), blasts.end());

    Leash const none = { 0.0f, 0.0f, 0.0f };
    return EscapeAll(bot, bubbles ? reign : blasts, all, none, 18.0f, x, y);
}

bool GetAlphaBeamHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_ANRAPHET))
        return false;

    std::vector<Creature*> beams;
    CollectAlive(bot, { NPC_ALPHA_BEAM }, 100.0f, beams);
    for (Creature* beam : beams)
        hazards.push_back({ beam->GetPositionX(), beam->GetPositionY(), ALPHA_BEAM_RADIUS });

    return !hazards.empty();
}

// A Spore only becomes a cloud once it has been "killed": the script then flags it unattackable and casts the cloud on
// itself (boss_ammunae.cpp:296-306).
bool GetSporeCloudHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_AMMUNAE))
        return false;

    std::vector<Creature*> spores;
    CollectAlive(bot, { NPC_SPORE }, 80.0f, spores);
    // #691: the radius the core applies to 75702 (10 yd at the time of writing, not the 6 yd of Wowhead)
    float const radius = EffectRadius(SPELL_NOXIOUS_SPORES, 0, SPORE_CLOUD_RADIUS);
    for (Creature* spore : spores)
    {
        if (spore->HasAura(SPELL_SPORE_CLOUD) || spore->HasFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE))
            hazards.push_back({ spore->GetPositionX(), spore->GetPositionY(), radius });
    }

    return !hazards.empty();
}

bool EscapeHazards(Player* bot, std::vector<Hazard> const& hazards, float margin, Leash const& leash, float maxTravel,
                   float& x, float& y)
{
    return !hazards.empty() && InsideHazard(bot, hazards, margin) &&
           FindSafeSpot(bot, hazards, margin, leash, maxTravel, x, y);
}

// The duty bots: bots only (a human master in the group cannot be told to jump), no tank, damage dealers first.
bool BeaconDuty(Player* bot, uint32& beaconEntry, bool& soleWorker)
{
    std::vector<Player*> members;
    CollectGroupMembers(bot, members, true);

    std::vector<Player*> workers;
    for (Player* member : members)
    {
        if (GET_PLAYERBOT_AI(member) && !PlayerBotSpec::IsTank(member))
            workers.push_back(member);
    }

    std::sort(workers.begin(), workers.end(), [](Player* a, Player* b)
    {
        bool const healerA = PlayerBotSpec::IsHeal(a);
        bool const healerB = PlayerBotSpec::IsHeal(b);
        if (healerA != healerB)
            return !healerA;

        return a->GetGUID().GetRawValue() < b->GetGUID().GetRawValue();
    });

    soleWorker = workers.size() == 1;
    for (size_t i = 0; i < workers.size() && i < 2; ++i)
    {
        if (workers[i] == bot)
        {
            beaconEntry = i == 0 ? GO_BEACON_LEFT : GO_BEACON_RIGHT;
            return true;
        }
    }

    return false;
}
}

bool SearingLightNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SearingLightTarget(bot, x, y);
}

bool SearingLightTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    return GetSearingLightHazards(bot, hazards) &&
           EscapeHazards(bot, hazards, SEARING_LIGHT_MARGIN, PlatformLeash(), 12.0f, x, y);
}

bool ReckoningNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return ReckoningTarget(bot, x, y);
}

bool ReckoningTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    return GetReckoningHazards(bot, hazards) &&
           EscapeHazards(bot, hazards, RECKONING_MARGIN, PlatformLeash(), 24.0f, x, y);
}

bool QuicksandNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return QuicksandTarget(bot, x, y);
}

bool QuicksandTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    Leash const none = { 0.0f, 0.0f, 0.0f };
    return GetQuicksandHazard(bot, hazards) && EscapeHazards(bot, hazards, 0.0f, none, 15.0f, x, y);
}

bool AlphaBeamNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return AlphaBeamTarget(bot, x, y);
}

bool AlphaBeamTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    Leash const none = { 0.0f, 0.0f, 0.0f };
    return GetAlphaBeamHazards(bot, hazards) && EscapeHazards(bot, hazards, ALPHA_BEAM_MARGIN, none, 15.0f, x, y);
}

bool SporeCloudNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SporeCloudTarget(bot, x, y);
}

bool SporeCloudTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    Leash const none = { 0.0f, 0.0f, 0.0f };
    return GetSporeCloudHazards(bot, hazards) && EscapeHazards(bot, hazards, SPORE_CLOUD_MARGIN, none, 15.0f, x, y);
}

bool SolarWindsNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SolarWindsTarget(bot, x, y);
}

bool SolarWindsTarget(Player* bot, float& x, float& y)
{
    // #691: escape spots now also stay clear of the Sun Orbs and the Inferno Leap marker
    return RajhEscape(bot, RajhZone::Winds, x, y);
}

bool ReignOfChaosNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return ReignOfChaosTarget(bot, x, y);
}

bool ReignOfChaosTarget(Player* bot, float& x, float& y)
{
    return SeteshEscape(bot, true, x, y);
}

bool ChaosBlastNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return ChaosBlastTarget(bot, x, y);
}

bool ChaosBlastTarget(Player* bot, float& x, float& y)
{
    return SeteshEscape(bot, false, x, y);
}

bool SunOrbNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SunOrbTarget(bot, x, y);
}

bool SunOrbTarget(Player* bot, float& x, float& y)
{
    return RajhEscape(bot, RajhZone::Orb, x, y);
}

bool InfernoLeapNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return InfernoLeapTarget(bot, x, y);
}

bool InfernoLeapTarget(Player* bot, float& x, float& y)
{
    return RajhEscape(bot, RajhZone::Leap, x, y);
}

bool BeaconNeeded(Player* bot)
{
    GameObject* beacon = nullptr;
    return BeaconPlan(bot, beacon);
}

bool BeaconPlan(Player* bot, GameObject*& beacon)
{
    Creature* boss = FindEngagedBoss(bot, NPC_TEMPLE_GUARDIAN_ANHUUR);
    if (!boss || !boss->HasAura(SPELL_SHIELD_OF_LIGHT))
        return false;

    uint32 entry = 0;
    bool soleWorker = false;
    if (!BeaconDuty(bot, entry, soleWorker))
        return false;

    uint32 const other = entry == GO_BEACON_LEFT ? GO_BEACON_RIGHT : GO_BEACON_LEFT;
    std::vector<uint32> order = { entry };
    if (soleWorker)
        order.push_back(other);

    for (uint32 beaconEntry : order)
    {
        // The script clears GO_FLAG_INTERACT_COND when the shield phase starts and sets it again once a beacon has
        // been used (boss_temple_guardian_anhuur.cpp:259-261, :376).
        GameObject* candidate = bot->FindNearestGameObject(beaconEntry, 200.0f);
        if (candidate && !candidate->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_INTERACT_COND))
        {
            beacon = candidate;
            return true;
        }
    }

    return false;
}

bool PitReturnNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return PitReturnTarget(bot, x, y, z);
}

bool PitReturnTarget(Player* bot, float& x, float& y, float& z)
{
    Creature* boss = FindEngagedBoss(bot, NPC_TEMPLE_GUARDIAN_ANHUUR);
    if (!boss || boss->HasAura(SPELL_SHIELD_OF_LIGHT))
        return false;

    if (bot->GetPositionZ() > boss->GetPositionZ() - PIT_DEPTH || bot->GetExactDist2d(boss) > 50.0f)
        return false;

    // Land a few yards from the boss on the side the bot comes from.
    float const bearing = boss->GetAngle(bot);
    x = boss->GetPositionX() + 4.0f * std::cos(bearing);
    y = boss->GetPositionY() + 4.0f * std::sin(bearing);
    z = boss->GetPositionZ();
    return true;
}

bool SupernovaCasting(Player* bot)
{
    if (PlayerBotSpec::IsTank(bot))
        return false;

    Creature* isiset = FindNearestAlive(bot, NPC_ISISET, 150.0f);
    return isiset && IsCasting(isiset, SPELL_SUPERNOVA);
}

bool SupernovaNeeded(Player* bot)
{
    if (!SupernovaCasting(bot))
        return false;

    Creature* isiset = FindNearestAlive(bot, NPC_ISISET, 150.0f);
    return isiset && bot->isInFront(isiset, SUPERNOVA_ARC);
}

float SupernovaFacing(Player* bot)
{
    Creature* isiset = FindNearestAlive(bot, NPC_ISISET, 150.0f);
    return isiset ? Position::NormalizeOrientation(bot->GetAngle(isiset) + PI_F) : bot->GetOrientation();
}

bool AnyMovementLock(Player* bot)
{
    return SearingLightNeeded(bot) || ReckoningNeeded(bot) || BeaconNeeded(bot) || PitReturnNeeded(bot) ||
           QuicksandNeeded(bot) || AlphaBeamNeeded(bot) || SporeCloudNeeded(bot) || SolarWindsNeeded(bot) ||
           ReignOfChaosNeeded(bot) || ChaosBlastNeeded(bot) || SunOrbNeeded(bot) || InfernoLeapNeeded(bot);
}

// Wowpedia Isiset: Astral Rain is the most dangerous image and goes first, Celestial Call after it (Veil of Sky is easily
// dispelled); the same order serves the second split.
Creature* PriorityImage(Player* bot)
{
    for (uint32 entry : { NPC_ASTRAL_RAIN, NPC_CELESTIAL_CALL, NPC_VEIL_OF_SKY })
    {
        std::vector<Creature*> images;
        CollectAlive(bot, { entry }, 150.0f, images);
        if (Creature* image = LowestGuid(images))
            return image;
    }

    return nullptr;
}

Creature* PriorityPod(Player* bot)
{
    if (!FindEngagedBoss(bot, NPC_AMMUNAE))
        return nullptr;

    std::vector<Creature*> pods;
    CollectAlive(bot, { NPC_SEEDLING_POD, NPC_SEEDLING_POD_2 }, 80.0f, pods);
    return LowestGuid(pods);
}

// Wowpedia Setesh: the Void Seekers (Anti-Magic Prison on the tank or the healer) go first; on heroic the portals have to be
// destroyed. On normal the portal is flagged unattackable (boss_setesh.cpp:272), so it is skipped there. #691: otherwise
// Setesh himself ("Other than that, they must always focus their attacks on Setesh") - not the Void Sentinel behind its
// -90% Void Barrier, nor the wurms the tank holds (w62: 12% of Setesh in 230 s, the dps were on the adds).
Creature* PrioritySetesh(Player* bot)
{
    Creature* setesh = FindEngagedBoss(bot, NPC_SETESH);
    if (!setesh)
        return nullptr;

    std::vector<Creature*> seekers;
    CollectAlive(bot, { NPC_VOID_SEEKER }, 80.0f, seekers);
    if (Creature* seeker = LowestGuid(seekers))
        return seeker;

    std::vector<Creature*> portals;
    CollectAlive(bot, { NPC_CHAOS_PORTAL }, 80.0f, portals);
    std::vector<Creature*> attackable;
    for (Creature* portal : portals)
    {
        if (bot->IsValidAttackTarget(portal))
            attackable.push_back(portal);
    }

    if (Creature* portal = LowestGuid(attackable))
        return portal;

    return bot->IsValidAttackTarget(setesh) ? setesh : nullptr;
}
}
