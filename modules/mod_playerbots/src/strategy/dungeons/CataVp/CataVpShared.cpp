/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataVpShared.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "GameObject.h"
#include "Group.h"
#include "InstanceScript.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "Unit.h"
#include <algorithm>
#include <cmath>
#include <list>

namespace CataVp
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

namespace CataVp
{
namespace
{
struct RingState
{
    Creature* boss = nullptr;
    float shortest = 1000.0f;    // the nearest cyclone to Ertan
    float nearVortex = 1000.0f;  // the nearest cyclone to the bot
};

bool GetRingState(Player* bot, RingState& state)
{
    state.boss = FindEngagedBoss(bot, NPC_GRAND_VIZIER_ERTAN);
    if (!state.boss)
        return false;

    std::vector<Creature*> vortexes;
    CollectAlive(bot, { NPC_ERTAN_VORTEX }, 100.0f, vortexes);
    if (vortexes.empty())
        return false;

    for (Creature* vortex : vortexes)
    {
        state.shortest = std::min(state.shortest, state.boss->GetExactDist2d(vortex));
        state.nearVortex = std::min(state.nearVortex, bot->GetExactDist2d(vortex));
    }

    return true;
}

bool GetBreathCone(Player* bot, Creature*& boss, float& axis)
{
    boss = FindEngagedBoss(bot, NPC_ALTAIRUS);
    if (!boss || !IsCasting(boss, SPELL_CHILLING_BREATH))
        return false;

    // The boss turns to the player it breathes on; that player cannot get out of its own breath.
    Unit* target = GetCastTarget(bot, boss, SPELL_CHILLING_BREATH);
    if (target == bot)
        return false;

    axis = target ? boss->GetAngle(target) : boss->GetOrientation();
    return true;
}

bool GetTwisterHazards(Player* bot, std::vector<Hazard>& hazards)
{
    if (!FindEngagedBoss(bot, NPC_ALTAIRUS))
        return false;

    std::vector<Creature*> twisters;
    CollectAlive(bot, { NPC_ALTAIRUS_TWISTER }, 80.0f, twisters);
    for (Creature* twister : twisters)
        hazards.push_back({ twister->GetPositionX(), twister->GetPositionY(), TWISTER_RADIUS });

    return !hazards.empty();
}
}

bool ErtanRingNeeded(Player* bot)
{
    RingState state;
    if (!GetRingState(bot, state))
        return false;

    // w29 (#663): stack directly under Ertan for the whole fight. Storm's Edge only hits players farther from her than a
    // cyclone (boss_grand_vizier_ertan.cpp:178-202) and the called-in cyclones stop 9 yd from her (:241-253, #663 w64), so
    // within ERTAN_STACK_ARRIVE of her is safe in both ring states (see CataVpShared.h). A bot already on her spot cannot
    // get any farther from a cyclone, so the touch clause only moves a bot that is not.
    float const dist = bot->GetExactDist2d(state.boss);
    return dist > ERTAN_STACK_ARRIVE || (state.nearVortex < ERTAN_VORTEX_TOUCH && dist > 1.0f);
}

bool ErtanStackActive(Player* bot)
{
    RingState state;
    return GetRingState(bot, state);
}

bool ErtanRingTarget(Player* bot, float& x, float& y)
{
    RingState state;
    if (!GetRingState(bot, state))
        return false;

    // w29 (#663): always her own position (was: ring - 9 yd on the bot's bearing until the ring was called in)
    x = state.boss->GetPositionX();
    y = state.boss->GetPositionY();
    return true;
}

bool WindNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return WindTarget(bot, x, y);
}

bool WindTarget(Player* bot, float& x, float& y)
{
    if (!bot->HasAura(SPELL_DOWNWIND_OF_ALTAIRUS) || PlayerBotSpec::IsTank(bot))
        return false;

    Creature* boss = FindEngagedBoss(bot, NPC_ALTAIRUS);
    Creature* current = FindNearestAlive(bot, NPC_AIR_CURRENT, 200.0f);
    if (!boss || !current)
        return false;

    float const heading = current->GetOrientation();
    float const diff = AngleDiff(boss->GetAngle(bot), heading);
    if (std::fabs(diff) <= WIND_WEDGE_NEEDED)
        return false;

    float const bearing = heading + (diff >= 0.0f ? WIND_WEDGE_AIM : -WIND_WEDGE_AIM);

    // Keep the bot's current distance from the boss (melee stay in melee, casters at range), pulled in until the
    // spot is on the platform.
    float radius = std::min(std::max(bot->GetExactDist2d(boss), 4.0f), 30.0f);
    for (; radius >= 4.0f; radius -= 4.0f)
    {
        x = boss->GetPositionX() + radius * std::cos(bearing);
        y = boss->GetPositionY() + radius * std::sin(bearing);
        if (std::hypot(x - ALTAIRUS_CENTER_X, y - ALTAIRUS_CENTER_Y) <= ALTAIRUS_PLATFORM_RADIUS)
            return true;
    }

    return false;
}

bool BreathNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return BreathTarget(bot, x, y);
}

bool BreathTarget(Player* bot, float& x, float& y)
{
    Creature* boss = nullptr;
    float axis = 0.0f;
    if (!GetBreathCone(bot, boss, axis))
        return false;

    return FindConeExit(bot, boss->GetPositionX(), boss->GetPositionY(), axis, BREATH_HALF_ANGLE, BREATH_LENGTH,
                        BREATH_ANGLE_MARGIN, BREATH_MAX_MOVE, x, y);
}

bool TwisterNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return TwisterTarget(bot, x, y);
}

bool TwisterTarget(Player* bot, float& x, float& y)
{
    std::vector<Hazard> hazards;
    if (!GetTwisterHazards(bot, hazards) || !InsideHazard(bot, hazards, TWISTER_MARGIN))
        return false;

    Leash const platform = { ALTAIRUS_CENTER_X, ALTAIRUS_CENTER_Y, ALTAIRUS_PLATFORM_RADIUS };
    return FindSafeSpot(bot, hazards, TWISTER_MARGIN, platform, 15.0f, x, y);
}

bool StackTarget(Player* bot, float& x, float& y)
{
    Creature* boss = FindEngagedBoss(bot, NPC_ASAAD);
    if (!boss)
        return false;

    // The field NPCs exist for the whole phase (spawned at its start, despawned after the storm), so the bots have
    // the 20 s of channelling to walk there.
    bool const phase = IsCasting(boss, SPELL_SUPREMACY_OF_THE_STORM) || IsCasting(boss, SPELL_UNSTABLE_GROUNDING_FIELD) ||
                       FindNearestAlive(bot, NPC_UNSTABLE_GROUNDING_FIELD, 100.0f) != nullptr;
    if (!phase)
        return false;

    if (boss->GetPositionZ() > ASAAD_LIFTED_Z)
    {
        x = boss->GetPositionX();
        y = boss->GetPositionY();
    }
    else
    {
        x = ASAAD_STACK_X;
        y = ASAAD_STACK_Y;
    }

    return true;
}

bool StackNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    return StackTarget(bot, x, y) && bot->GetExactDist2d(x, y) > ASAAD_STACK_ARRIVE;
}

namespace
{
struct PlatformAnchor
{
    float x;
    float y;
    float z;
    int32 platform;
};

// Creature spawn clusters of map 657 (MoP test world DB, 50 yd buckets, 2026-10-23) plus the slipstreams and landing zones.
// The platforms only meet through the slipstreams, so the nearest anchor names the platform.
PlatformAnchor const PLATFORM_ANCHORS[] =
{
    { -300.0f, 0.0f, 627.0f, 0 }, { -450.0f, -100.0f, 639.0f, 0 }, { -550.0f, -200.0f, 630.0f, 0 },
    { -650.0f, -50.0f, 620.0f, 0 }, { -719.97f, 4.33f, 635.67f, 0 }, { -765.44f, -41.37f, 639.93f, 0 },
    { -906.08f, -176.51f, 665.57f, 1 }, { -1000.0f, -100.0f, 690.0f, 1 }, { -1100.0f, 0.0f, 705.0f, 1 },
    { -1183.97f, 51.14f, 734.17f, 1 }, { -1198.79f, 107.05f, 740.71f, 1 },
    { -1193.67f, 472.84f, 634.76f, 2 }, { -1050.0f, 450.0f, 652.0f, 2 }, { -900.0f, 550.0f, 717.0f, 2 },
    { -800.0f, 500.0f, 682.0f, 2 }, { -700.0f, 500.0f, 648.0f, 2 }, { -622.7f, 503.5f, 646.7f, 2 },
};

struct SlipstreamSpot
{
    float x;
    float y;
    float z;
    uint32 gateBoss;   // the slipstream carries once this boss is DONE (the_vortex_pinnacle.cpp slipstreamRoutes)
    uint32 nextBoss;   // the boss on the platform it lands on
};

// index = the platform the slipstream starts on; it lands on the next one (navmesh surface z, mmaps 0657)
SlipstreamSpot const SLIPSTREAMS[] =
{
    { -765.44f, -41.37f, 639.95f, BOSS_DATA_ERTAN, BOSS_DATA_ALTAIRUS },
    { -1198.79f, 107.05f, 740.75f, BOSS_DATA_ALTAIRUS, BOSS_DATA_ASAAD },
};
}

int32 PlatformOf(float x, float y, float z)
{
    int32 platform = -1;
    float best = 0.0f;
    for (PlatformAnchor const& anchor : PLATFORM_ANCHORS)
    {
        float const dx = anchor.x - x;
        float const dy = anchor.y - y;
        float const dz = anchor.z - z;
        float const d = dx * dx + dy * dy + dz * dz;
        if (platform < 0 || d < best)
        {
            best = d;
            platform = anchor.platform;
        }
    }

    return platform;
}

bool SlipstreamTarget(Player* bot, float& x, float& y, float& z, Creature*& slipstream)
{
    slipstream = nullptr;
    if (!bot->IsAlive() || bot->GetVehicle() || bot->IsBeingTeleported())
        return false;

    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance)
        return false;

    int32 const mine = PlatformOf(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
    if (mine < 0 || mine >= int32(sizeof(SLIPSTREAMS) / sizeof(SLIPSTREAMS[0])))
        return false;

    SlipstreamSpot const& spot = SLIPSTREAMS[mine];
    if (instance->GetBossState(spot.gateBoss) != DONE)
        return false;

    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Player* leader = botAI ? botAI->GetGroupMaster() : nullptr;
    int32 wanted = mine;
    if (leader && leader != bot)
    {
        if (!leader->IsInWorld() || leader->GetMapId() != bot->GetMapId() || leader->GetInstanceId() != bot->GetInstanceId())
            return false;

        if (Creature* carrier = leader->GetVehicleCreatureBase())
        {
            // the leader rides: route r (GetData r + 1, 0-based start platform r) lands on platform r + 1
            if (carrier->GetEntry() == NPC_SLIPSTREAM && carrier->AI())
                wanted = int32(carrier->AI()->GetData(SLIPSTREAM_DATA_CARRIER));
        }
        else if (leader->IsAlive())
            wanted = PlatformOf(leader->GetPositionX(), leader->GetPositionY(), leader->GetPositionZ());
    }
    else if (instance->GetBossState(spot.nextBoss) != DONE &&
             bot->GetExactDist(spot.x, spot.y, spot.z) <= SLIPSTREAM_LEADER_RANGE)
        wanted = mine + 1;   // a leader bot: on to the next boss

    if (wanted <= mine)
        return false;

    x = spot.x;
    y = spot.y;
    z = spot.z;
    if (bot->GetExactDist(x, y, z) > SLIPSTREAM_SEARCH_RANGE)
        return true;

    // the DB slipstream (it has a spawn id; a carrier or an entrance shortcut is a summon)
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, NPC_SLIPSTREAM, SLIPSTREAM_SEARCH_RANGE + 10.0f);
    for (Creature* creature : found)
    {
        if (creature && creature->IsAlive() && creature->GetDBTableGUIDLow() && creature->GetExactDist(x, y, z) < 10.0f)
        {
            slipstream = creature;
            break;
        }
    }

    return true;
}

bool SlipstreamNeeded(Player* bot)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Creature* slipstream = nullptr;
    return SlipstreamTarget(bot, x, y, z, slipstream);
}

bool AnyMovementLock(Player* bot)
{
    // #692: walking to the slipstream also locks: a follow toward a leader in the air or on another platform walks off the edge
    return ErtanRingNeeded(bot) || WindNeeded(bot) || BreathNeeded(bot) || TwisterNeeded(bot) || StackNeeded(bot) ||
           SlipstreamNeeded(bot);
}
}
