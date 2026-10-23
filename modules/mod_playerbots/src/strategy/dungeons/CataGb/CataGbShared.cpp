/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataGbShared.h"
#include "Creature.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Spell.h"
#include "Unit.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace CataGb
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

namespace CataGb
{
namespace
{
// The cone of an Umbriss Ground Siege that is being cast: it points from the boss to the stalker on the target.
bool GetGroundSiegeCone(Player* bot, Creature*& boss, float& axis)
{
    boss = FindEngagedBoss(bot, NPC_GENERAL_UMBRISS);
    if (!boss || !IsCasting(boss, SPELL_GROUND_SIEGE))
        return false;

    Creature* stalker = FindNearestAlive(bot, NPC_GROUND_SIEGE_STALKER, 60.0f);
    axis = stalker ? boss->GetAngle(stalker) : boss->GetOrientation();
    return true;
}

bool GetBlitzHazard(Player* bot, std::vector<Hazard>& hazards)
{
    Creature* boss = FindEngagedBoss(bot, NPC_GENERAL_UMBRISS);
    if (!boss || !IsCasting(boss, SPELL_BLITZ))
        return false;

    Creature* stalker = FindNearestAlive(bot, NPC_BLITZ_STALKER, 80.0f);
    if (!stalker)
        return false;

    hazards.push_back({ stalker->GetPositionX(), stalker->GetPositionY(), BLITZ_RADIUS });
    return true;
}

bool GetFlamePortalHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_DRAHGA_SHADOWBURNER))
        return false;

    std::vector<Creature*> portals;
    CollectAlive(bot, { NPC_INVOCATION_OF_FLAME_STALKER }, 80.0f, portals);
    for (Creature* portal : portals)
        hazards.push_back({ portal->GetPositionX(), portal->GetPositionY(), FLAME_PORTAL_RADIUS });

    return !hazards.empty();
}

// Only the spirits that are chasing this bot: it is the one that has to keep running. #688: the spirit marks its target with
// Flaming Fixate 82850 (caster = the spirit), so the aura names the chaser even before its first chase step.
bool GetSpiritHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_DRAHGA_SHADOWBURNER))
        return false;

    std::vector<Creature*> spirits;
    CollectAlive(bot, { NPC_INVOKED_FLAMING_SPIRIT }, 80.0f, spirits);
    for (Creature* spirit : spirits)
    {
        if (spirit->GetVictim() == bot || bot->HasAura(SPELL_FLAMING_FIXATE, spirit->GetGUID()))
            hazards.push_back({ spirit->GetPositionX(), spirit->GetPositionY(), SPIRIT_KITE_RADIUS });
    }

    return !hazards.empty();
}

// Throngus ground circles (#688): every alive Cave In Stalker / heroic Fire Patch while he is engaged.
bool GetThrongusCircles(Player* bot, uint32 entry, float radius, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_FORGEMASTER_THRONGUS))
        return false;

    std::vector<Creature*> circles;
    CollectAlive(bot, { entry }, 80.0f, circles);
    for (Creature* circle : circles)
        hazards.push_back({ circle->GetPositionX(), circle->GetPositionY(), radius });

    return !hazards.empty();
}

// Valiona's breath (#688): Valiona's Flame while she casts it (cone towards the cast target), heroic Devouring Flames while the
// 48798 stalker lives (cone towards the stalker).
bool GetValionaBreathCone(Player* bot, Creature*& valiona, float& axis, float& halfAngle, float& length)
{
    valiona = FindEngagedBoss(bot, NPC_VALIONA);
    if (!valiona)
        return false;

    if (IsCasting(valiona, SPELL_VALIONAS_FLAME))
    {
        Unit* target = GetCastTarget(bot, valiona, SPELL_VALIONAS_FLAME);
        axis = target ? valiona->GetAngle(target) : valiona->GetOrientation();
        halfAngle = VALIONA_FLAME_HALF_ANGLE;
        length = VALIONA_FLAME_LENGTH;
        return true;
    }

    if (Creature* flames = FindNearestAlive(bot, NPC_DEVOURING_FLAMES, 100.0f))
    {
        axis = valiona->GetAngle(flames);
        halfAngle = DEVOURING_HALF_ANGLE;
        length = DEVOURING_LENGTH;
        return true;
    }

    return false;
}

bool GetSeepingTwilightHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_DRAHGA_SHADOWBURNER))
        return false;

    std::vector<Creature*> pools;
    CollectAlive(bot, { NPC_SEEPING_TWILIGHT }, 80.0f, pools);
    for (Creature* pool : pools)
        hazards.push_back({ pool->GetPositionX(), pool->GetPositionY(), SEEPING_TWILIGHT_RADIUS });

    return !hazards.empty();
}

// Binding Shadows in flight: the circle around its target. The target itself cannot step out of a circle that is
// centred on it, so it is told to run away from the rest of the group instead (see BindingShadowsTarget).
bool GetBindingShadows(Player* bot, Unit*& target)
{
    Creature* boss = FindEngagedBoss(bot, NPC_ERUDAX);
    if (!boss || !IsCasting(boss, SPELL_BINDING_SHADOWS))
        return false;

    target = GetCastTarget(bot, boss, SPELL_BINDING_SHADOWS);
    return target != nullptr;
}
}

bool GroundSiegeNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return GroundSiegeTarget(bot, x, y);
}

bool GroundSiegeTarget(Player* bot, float& x, float& y)
{
    Creature* boss = nullptr;
    float axis = 0.0f;
    if (!GetGroundSiegeCone(bot, boss, axis))
        return false;

    return FindConeExit(bot, boss->GetPositionX(), boss->GetPositionY(), axis, GROUND_SIEGE_HALF_ANGLE,
                        GROUND_SIEGE_LENGTH, CONE_ANGLE_MARGIN, CONE_MAX_MOVE, x, y);
}

bool BlitzNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return BlitzTarget(bot, x, y);
}

bool BlitzTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetBlitzHazard(bot, hazards) || !InsideHazard(bot, hazards, BLITZ_MARGIN))
        return false;

    Leash const none = { 0.0f, 0.0f, 0.0f };
    return FindSafeSpot(bot, hazards, BLITZ_MARGIN, none, 18.0f, x, y);
}

bool PhalanxNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return PhalanxTarget(bot, x, y);
}

bool PhalanxTarget(Player* bot, float& x, float& y)
{
    // #688: in the shield phase Throngus is pacified and points the shield at a random player (Fixate Stalker 40255), not at
    // the tank, so the tank leaves the front cone too ("Everyone ... should be standing right behind him", Wowpedia).
    Creature* boss = FindEngagedBoss(bot, NPC_FORGEMASTER_THRONGUS);
    if (!boss || !boss->HasAura(SPELL_PERSONAL_PHALANX) || boss->GetVictim() == bot)
        return false;

    float const dist = bot->GetExactDist2d(boss);
    if (dist > PHALANX_RANGE)
        return false;

    float const bearing = std::atan2(bot->GetPositionY() - boss->GetPositionY(), bot->GetPositionX() - boss->GetPositionX());
    float const diff = AngleDiff(bearing, boss->GetOrientation());
    if (std::fabs(diff) >= PHALANX_HALF_ANGLE)
        return false;

    // Round to the side the bot is already on, at the distance it already keeps (melee stay in melee range).
    // #688 w106: not into a Cave In - the mage walked round the shield into one (3 hits); try the other side, then 6 yd farther out.
    float const radius = std::min(std::max(dist, 3.5f), 30.0f);
    float const side = diff >= 0.0f ? 1.0f : -1.0f;
    for (float const extra : { 0.0f, 6.0f })
    {
        for (float const sign : { side, -side })
        {
            float const target = boss->GetOrientation() + sign * PHALANX_AIM;
            float const px = boss->GetPositionX() + (radius + extra) * std::cos(target);
            float const py = boss->GetPositionY() + (radius + extra) * std::sin(target);
            if (InCaveInCircle(bot, px, py))
                continue;

            x = px;
            y = py;
            return true;
        }
    }

    float const target = boss->GetOrientation() + side * PHALANX_AIM;
    x = boss->GetPositionX() + radius * std::cos(target);
    y = boss->GetPositionY() + radius * std::sin(target);
    return true;
}

bool CaveInNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return CaveInTarget(bot, x, y);
}

bool CaveInTarget(Player* bot, float& x, float& y)
{
    // Everyone leaves, the tank too: "Tanks should move Throngus should these cave-ins appear on the melee" (Wowpedia).
    std::vector<Hazard> hazards;
    if (!GetThrongusCircles(bot, NPC_CAVE_IN_STALKER, CAVE_IN_RADIUS, hazards))
        return false;

    Leash const none = { 0.0f, 0.0f, 0.0f };

    // #688 w106: the tank Throngus is chasing drags him clear - it walks his melee gap (+ CAVE_IN_BOSS_SLACK) beyond the circle so
    // that he, following to melee range, ends outside it too. It stops once it is that far out (he is on his way).
    Creature* boss = nullptr;
    if (ThrongusAtCaveIn(bot, boss) && boss->GetVictim() == bot)
    {
        float const drag = CAVE_IN_MARGIN + CAVE_IN_BOSS_SLACK + boss->GetCombatReach() + bot->GetCombatReach();
        if (InsideHazard(bot, hazards, drag))
        {
            GetThrongusCircles(bot, NPC_FIRE_PATCH, FIRE_PATCH_RADIUS, hazards);
            if (FindSafeSpot(bot, hazards, drag, none, CAVE_IN_DRAG_TRAVEL, x, y))
                return true;
        }
    }

    if (!InsideHazard(bot, hazards, CAVE_IN_MARGIN))
        return false;

    return FindSafeSpot(bot, hazards, CAVE_IN_MARGIN, none, 15.0f, x, y);
}

bool ThrongusAtCaveIn(Player* bot, Creature*& boss, float extra)
{
    boss = FindEngagedBoss(bot, NPC_FORGEMASTER_THRONGUS);
    if (!boss || boss->HasAura(SPELL_PERSONAL_PHALANX))
        return false;

    // Cave Ins only: the heroic Fire Patches trail right behind him as he walks, so he always stands next to the newest one.
    std::vector<Hazard> hazards;
    GetThrongusCircles(bot, NPC_CAVE_IN_STALKER, CAVE_IN_RADIUS, hazards);
    for (Hazard const& hazard : hazards)
    {
        if (boss->GetExactDist2d(hazard.x, hazard.y) < hazard.radius + CAVE_IN_MARGIN + extra)
            return true;
    }

    return false;
}

bool CaveInApproachBlocked(Player* bot, Unit* boss, bool behind)
{
    // Cave Ins only (240 s, where the melee stand); a heroic Fire Patch trail behind him would idle the melee for the whole mace phase.
    std::vector<Hazard> hazards;
    GetThrongusCircles(bot, NPC_CAVE_IN_STALKER, CAVE_IN_RADIUS, hazards);
    if (hazards.empty())
        return false;

    // Where a straight approach stops: on the line bot -> boss, his melee gap from his center ("behind": that far behind his back).
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const dist = bot->GetExactDist2d(boss);
    float const reachGap = boss->GetCombatReach() + bot->GetCombatReach();
    float px = 0.0f;
    float py = 0.0f;
    if (behind)
    {
        float const back = boss->GetOrientation() + PI_F;
        px = boss->GetPositionX() + reachGap * std::cos(back);
        py = boss->GetPositionY() + reachGap * std::sin(back);
    }
    else
    {
        float const gap = std::min(dist, reachGap);
        float const t = dist > 0.1f ? (dist - gap) / dist : 0.0f;
        px = bx + (boss->GetPositionX() - bx) * t;
        py = by + (boss->GetPositionY() - by) * t;
    }

    int const steps = std::max(1, static_cast<int>(std::ceil(std::hypot(px - bx, py - by))));
    for (Hazard const& hazard : hazards)
    {
        float const reach = hazard.radius + CAVE_IN_MARGIN;
        if (std::hypot(bx - hazard.x, by - hazard.y) < reach)
            continue; // already inside: "gb throngus cave in" and the movement lock handle it

        for (int i = 1; i <= steps; ++i)
        {
            float const f = static_cast<float>(i) / static_cast<float>(steps);
            if (std::hypot(bx + (px - bx) * f - hazard.x, by + (py - by) * f - hazard.y) < reach)
                return true;
        }
    }

    return false;
}

bool InCaveInCircle(Player* bot, float x, float y)
{
    std::vector<Hazard> hazards;
    GetThrongusCircles(bot, NPC_CAVE_IN_STALKER, CAVE_IN_RADIUS, hazards);
    GetThrongusCircles(bot, NPC_FIRE_PATCH, FIRE_PATCH_RADIUS, hazards);
    for (Hazard const& hazard : hazards)
    {
        if (std::hypot(x - hazard.x, y - hazard.y) < hazard.radius + CAVE_IN_MARGIN)
            return true;
    }

    return false;
}

bool FirePatchNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return FirePatchTarget(bot, x, y);
}

bool FirePatchTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetThrongusCircles(bot, NPC_FIRE_PATCH, FIRE_PATCH_RADIUS, hazards) || !InsideHazard(bot, hazards, FIRE_PATCH_MARGIN))
        return false;

    // The patches trail behind him, so the cave-ins count as hazards for the escape spot too.
    GetThrongusCircles(bot, NPC_CAVE_IN_STALKER, CAVE_IN_RADIUS, hazards);
    Leash const none = { 0.0f, 0.0f, 0.0f };
    return FindSafeSpot(bot, hazards, FIRE_PATCH_MARGIN, none, 15.0f, x, y);
}

bool ValionaBreathNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return ValionaBreathTarget(bot, x, y);
}

bool ValionaBreathTarget(Player* bot, float& x, float& y)
{
    Creature* valiona = nullptr;
    float axis = 0.0f;
    float halfAngle = 0.0f;
    float length = 0.0f;
    if (!GetValionaBreathCone(bot, valiona, axis, halfAngle, length))
        return false;

    float const vx = valiona->GetPositionX();
    float const vy = valiona->GetPositionY();
    if (!InsideCone(vx, vy, axis, halfAngle + CONE_ANGLE_MARGIN, length, bot->GetPositionX(), bot->GetPositionY()))
        return false;

    if (FindConeExit(bot, vx, vy, axis, halfAngle, length, CONE_ANGLE_MARGIN, VALIONA_CONE_MAX_MOVE, x, y))
        return true;

    // The 150 degree Devouring Flames cone is too wide to walk round from range: go behind her instead.
    x = vx + 5.0f * std::cos(axis + PI_F);
    y = vy + 5.0f * std::sin(axis + PI_F);
    return true;
}

bool FlamePortalNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return FlamePortalTarget(bot, x, y);
}

bool FlamePortalTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetFlamePortalHazards(bot, hazards) || !InsideHazard(bot, hazards, FLAME_PORTAL_MARGIN))
        return false;

    Leash const none = { 0.0f, 0.0f, 0.0f };
    return FindSafeSpot(bot, hazards, FLAME_PORTAL_MARGIN, none, 15.0f, x, y);
}

bool SpiritKiteNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SpiritKiteTarget(bot, x, y);
}

bool SpiritKiteTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetSpiritHazards(bot, hazards) || !InsideHazard(bot, hazards, 0.0f))
        return false;

    // The spirit runs at 0.8x speed (boss_drahga_shadowburner.cpp:475), so a bot that keeps moving stays ahead of it.
    Leash const none = { 0.0f, 0.0f, 0.0f };
    return FindSafeSpot(bot, hazards, 0.0f, none, 24.0f, x, y);
}

bool SeepingTwilightNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return SeepingTwilightTarget(bot, x, y);
}

bool SeepingTwilightTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetSeepingTwilightHazards(bot, hazards) || !InsideHazard(bot, hazards, SEEPING_TWILIGHT_MARGIN))
        return false;

    Leash const none = { 0.0f, 0.0f, 0.0f };
    return FindSafeSpot(bot, hazards, SEEPING_TWILIGHT_MARGIN, none, 18.0f, x, y);
}

bool ShadowGaleNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return ShadowGaleTarget(bot, x, y) && bot->GetExactDist2d(x, y) > SHADOW_GALE_ARRIVE + 0.5f;
}

bool ShadowGaleTarget(Player* bot, float& x, float& y)
{
    if (!FindEngagedBoss(bot, NPC_ERUDAX))
        return false;

    // The stalker is summoned when the gale starts and despawned when the Faceless Corruptors come (boss_erudax.cpp:194-216).
    Creature* eye = FindNearestAlive(bot, NPC_SHADOW_GALE_STALKER, 120.0f);
    if (!eye)
        return false;

    x = eye->GetPositionX();
    y = eye->GetPositionY();
    return true;
}

bool BindingShadowsNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return BindingShadowsTarget(bot, x, y);
}

bool BindingShadowsTarget(Player* bot, float& x, float& y)
{
    Unit* target = nullptr;
    if (!GetBindingShadows(bot, target))
        return false;

    if (target != bot)
    {
        if (bot->GetExactDist2d(target) >= BINDING_SHADOWS_RADIUS + BINDING_SHADOWS_MARGIN)
            return false;

        std::vector<Hazard> hazards;
        hazards.push_back({ target->GetPositionX(), target->GetPositionY(), BINDING_SHADOWS_RADIUS });
        Leash const none = { 0.0f, 0.0f, 0.0f };
        return FindSafeSpot(bot, hazards, BINDING_SHADOWS_MARGIN, none, 15.0f, x, y);
    }

    // The target: run away from everybody else, so nobody is left inside the circle that lands on its spot.
    float cx = 0.0f;
    float cy = 0.0f;
    if (!OthersCentroid(bot, cx, cy))
    {
        cx = bot->GetPositionX() - std::cos(bot->GetOrientation());
        cy = bot->GetPositionY() - std::sin(bot->GetOrientation());
    }

    float const away = std::atan2(bot->GetPositionY() - cy, bot->GetPositionX() - cx);
    for (float delta : { 0.0f, 0.7f, -0.7f, 1.4f, -1.4f })
    {
        float const px = bot->GetPositionX() + BINDING_SHADOWS_RUN * std::cos(away + delta);
        float const py = bot->GetPositionY() + BINDING_SHADOWS_RUN * std::sin(away + delta);
        if (ClearStep(bot, px, py))
        {
            x = px;
            y = py;
            return true;
        }
    }

    return false;
}

bool AnyMovementLock(Player* bot)
{
    return GroundSiegeNeeded(bot) || BlitzNeeded(bot) || PhalanxNeeded(bot) || CaveInNeeded(bot) || FirePatchNeeded(bot) ||
           FlamePortalNeeded(bot) || SpiritKiteNeeded(bot) || SeepingTwilightNeeded(bot) || ValionaBreathNeeded(bot) ||
           ShadowGaleNeeded(bot) || BindingShadowsNeeded(bot);
}

Creature* FlamingSpirit(Player* bot)
{
    if (!FindEngagedBoss(bot, NPC_DRAHGA_SHADOWBURNER))
        return nullptr;

    std::vector<Creature*> spirits;
    CollectAlive(bot, { NPC_INVOKED_FLAMING_SPIRIT }, 80.0f, spirits);
    return LowestGuid(spirits);
}

Creature* FacelessCorruptor(Player* bot)
{
    if (!FindEngagedBoss(bot, NPC_ERUDAX))
        return nullptr;

    std::vector<Creature*> corruptors;
    CollectAlive(bot, { NPC_FACELESS_CORRUPTOR, NPC_FACELESS_CORRUPTOR_H }, 100.0f, corruptors);
    return LowestGuid(corruptors);
}
}
