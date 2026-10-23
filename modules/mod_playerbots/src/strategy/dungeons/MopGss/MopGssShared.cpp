/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655) - hazard / focus / interrupt computations.
// See MopGssShared.h for the sources of every mechanic and id.

#include "MopGssShared.h"

#include "AttackersValue.h"
#include "Creature.h"
#include "Group.h"
#include "InstanceScript.h"
#include "ObjectAccessor.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <iterator>
#include <list>
#include <string>
#include <vector>

namespace MopGss
{
namespace
{
// Nearest living creature of `entry` within `range` (centre-to-centre) that satisfies `pred`.
template <class Pred>
Creature* NearestCreature(Player* bot, uint32 entry, float range, Pred pred)
{
    std::list<Creature*> found;
    // 5.4.8: WorldObject::GetCreatureListWithEntryInGrid(list&, entry, range), same call as UB/UBShared.cpp.
    bot->GetCreatureListWithEntryInGrid(found, entry, range);

    Creature* best = nullptr;
    float bestDist = range + 1.0f;
    for (Creature* creature : found)
    {
        if (!creature || !creature->IsAlive() || !pred(creature))
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist < bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

// The Spell* of `spellId` while `unit` is casting or channeling it, else nullptr. Same lookup as
// HoS/HoSTriggers.cpp:23 (FindCurrentSpellBySpellId); the spell state keeps a finished / in-flight spell out.
Spell* CurrentCast(Unit* unit, uint32 spellId)
{
    if (!unit)
        return nullptr;

    Spell* spell = unit->FindCurrentSpellBySpellId(spellId);
    if (!spell)
        return nullptr;

    uint32 const state = spell->getState();
    if (state != SPELL_STATE_PREPARING && state != SPELL_STATE_CASTING)
        return nullptr;

    return spell;
}

// Largest effect radius of a spell (UB/UBShared.cpp MaxEffectRadius), clamped to a sane band.
float ClampedRadius(uint32 spellId, float fallback, float lo, float hi)
{
    float radius = 0.0f;
    if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId))
    {
        for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
            radius = std::max(radius, spellInfo->Effects[i].CalcRadius());
    }

    if (radius <= 0.0f)
        radius = fallback;

    return std::clamp(radius, lo, hi);
}

// A point `safeDist` away from (ox, oy) on the line through the bot. False when the bot is already clear.
bool SetAway(Player* bot, float ox, float oy, float safeDist, HazardKind kind, Hazard& out)
{
    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float const dist = std::hypot(bx - ox, by - oy);
    if (dist >= safeDist)
        return false;

    float ux, uy;
    if (dist < 0.1f)
    {
        ux = std::cos(bot->GetOrientation());
        uy = std::sin(bot->GetOrientation());
    }
    else
    {
        ux = (bx - ox) / dist;
        uy = (by - oy) / dist;
    }

    float const step = safeDist - dist + 1.0f;
    out.kind = kind;
    out.x = bx + ux * step;
    out.y = by + uy * step;
    return true;
}

// Sideways step out of a cone (halfAngle > 0) or a lane (halfAngle 0) that starts at (ox, oy) and points along
// `facing`. `halfWidth` is the lane half width / the cone's width at its tip, `length` the reach (0 = unlimited).
// False when the bot is outside the area.
bool EscapeFromArea(float ox, float oy, float facing, float halfAngle, float halfWidth, float length, float margin,
                    float bx, float by, float& outX, float& outY)
{
    float const dx = bx - ox;
    float const dy = by - oy;
    float const fx = std::cos(facing);
    float const fy = std::sin(facing);
    float const forward = dx * fx + dy * fy;
    float const lateral = -dx * fy + dy * fx;  // positive = left of the facing

    if (forward < -halfWidth)
        return false;

    if (length > 0.0f && forward > length + margin)
        return false;

    float const reach = std::max(forward, 0.0f);
    float const width = halfWidth + reach * std::tan(halfAngle) + margin;
    float const absLateral = std::fabs(lateral);
    if (absLateral >= width)
        return false;

    float const side = lateral >= 0.0f ? 1.0f : -1.0f;
    float const step = width - absLateral + 1.0f;
    outX = bx - fy * side * step;
    outY = by + fx * side * step;
    return true;
}

bool AreaHazard(Player* bot, float ox, float oy, float facing, float halfAngle, float halfWidth, float length,
                float margin, HazardKind kind, Hazard& out)
{
    float ex, ey;
    if (!EscapeFromArea(ox, oy, facing, halfAngle, halfWidth, length, margin, bot->GetPositionX(),
                        bot->GetPositionY(), ex, ey))
        return false;

    out.kind = kind;
    out.x = ex;
    out.y = ey;
    return true;
}

// Kip'tilak's Sabotage on THIS bot: the bombs explode after 5 s (148500 Fire on everyone within 2 yd), and the
// explosion lights every Stable Munition nearby. Wowpedia: "get away from everyone in that general area".
bool SabotageSelfHazard(Player* bot, Hazard& out)
{
    if (!bot->HasAura(GSS_SPELL_SABOTAGE))
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    float rx = 0.0f;
    float ry = 0.0f;
    bool any = false;

    if (Group* group = bot->GetGroup())
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                continue;

            float const dist = bot->GetExactDist2d(member);
            if (dist >= GSS_SABOTAGE_SPREAD)
                continue;

            float const d = std::max(dist, 0.5f);
            float const weight = (GSS_SABOTAGE_SPREAD - dist + 1.0f) / d;
            rx += (bx - member->GetPositionX()) * weight;
            ry += (by - member->GetPositionY()) * weight;
            any = true;
        }
    }

    std::list<Creature*> munitions;
    bot->GetCreatureListWithEntryInGrid(munitions, GSS_NPC_STABLE_MUNITION, GSS_SABOTAGE_MUNITION + 2.0f);
    for (Creature* munition : munitions)
    {
        if (!munition || !munition->IsAlive())
            continue;

        float const dist = bot->GetExactDist2d(munition);
        if (dist >= GSS_SABOTAGE_MUNITION)
            continue;

        float const d = std::max(dist, 0.5f);
        float const weight = (GSS_SABOTAGE_MUNITION - dist + 1.0f) / d;
        rx += (bx - munition->GetPositionX()) * weight;
        ry += (by - munition->GetPositionY()) * weight;
        any = true;
    }

    if (!any)
        return false;

    float len = std::hypot(rx, ry);
    if (len < 0.01f)
    {
        rx = std::cos(bot->GetOrientation());
        ry = std::sin(bot->GetOrientation());
        len = 1.0f;
    }

    out.kind = HazardKind::SabotageSelf;
    out.x = bx + rx / len * 8.0f;
    out.y = by + ry / len * 8.0f;
    return true;
}

// Kip'tilak's Sabotage on SOMEBODY ELSE: four directional explosions leave the victim along the north / south /
// east / west axes, so stay off both axis lanes through the victim until it has gone off.
bool SabotageCrossHazard(Player* bot, Hazard& out)
{
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    float const bx = bot->GetPositionX();
    float const by = bot->GetPositionY();
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId() ||
            !member->HasAura(GSS_SPELL_SABOTAGE))
            continue;

        float const mx = member->GetPositionX();
        float const my = member->GetPositionY();
        float const dx = bx - mx;
        float const dy = by - my;
        if (std::hypot(dx, dy) > 45.0f)
            continue;

        bool const inNorthSouthLane = std::fabs(dx) < GSS_LANE_HALF_WIDTH;
        bool const inEastWestLane = std::fabs(dy) < GSS_LANE_HALF_WIDTH;
        if (!inNorthSouthLane && !inEastWestLane)
            continue;

        out.kind = HazardKind::SabotageCross;
        out.x = inNorthSouthLane ? mx + (dx >= 0.0f ? GSS_LANE_CLEAR : -GSS_LANE_CLEAR) : bx;
        out.y = inEastWestLane ? my + (dy >= 0.0f ? GSS_LANE_CLEAR : -GSS_LANE_CLEAR) : by;
        return true;
    }

    return false;
}

// The explosion bunnies (Mantid Munitions) run a straight 40 yd line at speed 10 and burn everything within 2 yd.
// #655 w68: World in Flames sends four bunnies from every munition in the room at once; the sidestep out of the FIRST lane found
// walked into the next one (the tank died at 29% while dodging, 65 -> 0 in one probe; 107215 = 228k Fire per 0.5 s tick, Wowhead
// 2026-10-15). So: a spot clear of every running lane (the WindsHazard / ClearOfAll shape), the single sidestep only as a fallback.
struct BunnyLane
{
    float ox;
    float oy;
    float fx;
    float fy;
};

constexpr float GSS_BUNNY_HALF_WIDTH = 3.0f;  // the old AreaHazard band: 2 yd burn + 1 yd
constexpr float GSS_BUNNY_MARGIN = 1.5f;
constexpr float GSS_BUNNY_REACH = 45.0f;      // they run 40 yd

// Lateral distance from (x, y) to the nearest running lane that still has (x, y) ahead of its bunny, capped at 8 yd.
float BunnyLaneClearance(std::vector<BunnyLane> const& lanes, float x, float y)
{
    float clearance = 8.0f;
    for (BunnyLane const& lane : lanes)
    {
        float const dx = x - lane.ox;
        float const dy = y - lane.oy;
        float const forward = dx * lane.fx + dy * lane.fy;
        if (forward < -GSS_BUNNY_HALF_WIDTH || forward > GSS_BUNNY_REACH)
            continue;

        clearance = std::min(clearance, std::fabs(-dx * lane.fy + dy * lane.fx));
    }

    return clearance;
}

bool MunitionLaneHazard(Player* bot, Hazard& out)
{
    static uint32 const bunnies[] = {GSS_NPC_BUNNY_N_M, GSS_NPC_BUNNY_S_M, GSS_NPC_BUNNY_E_M, GSS_NPC_BUNNY_W_M,
                                     GSS_NPC_BUNNY_N_P, GSS_NPC_BUNNY_S_P, GSS_NPC_BUNNY_E_P, GSS_NPC_BUNNY_W_P};
    std::vector<BunnyLane> lanes;
    for (uint32 const entry : bunnies)
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, GSS_SCAN);
        for (Creature* bunny : found)
        {
            if (!bunny || !bunny->IsAlive() || !bunny->HasAura(GSS_SPELL_MUNITION_EXPLOSION_AURA))
                continue;

            float const o = bunny->GetOrientation();
            lanes.push_back({bunny->GetPositionX(), bunny->GetPositionY(), std::cos(o), std::sin(o)});
        }
    }

    float const safe = GSS_BUNNY_HALF_WIDTH + GSS_BUNNY_MARGIN;
    if (lanes.empty() || BunnyLaneClearance(lanes, bot->GetPositionX(), bot->GetPositionY()) >= safe)
        return false;

    auto const score = [&lanes](float px, float py) { return BunnyLaneClearance(lanes, px, py); };
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z))
    {
        out.kind = HazardKind::MunitionLane;
        out.x = x;
        out.y = y;
        return true;
    }

    // no better walkable spot: the old sideways step out of one lane that has the bot in it
    for (BunnyLane const& lane : lanes)
    {
        if (AreaHazard(bot, lane.ox, lane.oy, std::atan2(lane.fy, lane.fx), 0.0f, GSS_BUNNY_HALF_WIDTH, GSS_BUNNY_REACH,
                       GSS_BUNNY_MARGIN, HazardKind::MunitionLane, out))
            return true;
    }

    return false;
}

// #781 Kip'tilak's World in Flames: distance from (x, y) to the nearest munition axis lane (capped at 10 yd).
float MunitionLaneClearance(std::vector<Position> const& munitions, float x, float y)
{
    float clearance = 10.0f;
    for (Position const& m : munitions)
    {
        float const dx = std::fabs(x - m.GetPositionX());
        float const dy = std::fabs(y - m.GetPositionY());
        if (dx <= GSS_WIF_LANE_LENGTH)
            clearance = std::min(clearance, dy);  // the lane along the x axis (orientation 0 / pi bunnies)
        if (dy <= GSS_WIF_LANE_LENGTH)
            clearance = std::min(clearance, dx);  // the lane along the y axis (orientation pi/2 / 3pi/2 bunnies)
    }

    return clearance;
}

bool WorldInFlamesHazard(Player* bot, Hazard& out)
{
    Creature* kiptilak = bot->FindNearestCreature(GSS_NPC_KIPTILAK, 80.0f);
    if (!kiptilak || !kiptilak->IsInCombat())
        return false;

    float const pct = kiptilak->GetHealthPct();
    // #655 w68: GSS_WIF_AFTER keeps it on past the crossing, while the 5 new munitions land and everything goes off
    bool const imminent = (pct > 70.0f - GSS_WIF_AFTER && pct <= 70.0f + GSS_WIF_WINDOW) ||
                          (pct > 30.0f - GSS_WIF_AFTER && pct <= 30.0f + GSS_WIF_WINDOW);
    if (!imminent)
        return false;

    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, GSS_NPC_STABLE_MUNITION, GSS_WIF_LANE_LENGTH + 5.0f);
    std::vector<Position> munitions;
    for (Creature* munition : found)
    {
        if (munition && munition->IsAlive())
            munitions.push_back(munition->GetPosition());
    }

    if (munitions.empty() || MunitionLaneClearance(munitions, bot->GetPositionX(), bot->GetPositionY()) >= GSS_WIF_LANE_CLEAR)
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    auto const score = [&munitions](float px, float py) { return MunitionLaneClearance(munitions, px, py); };
    if (!MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z))
        return false;

    out.kind = HazardKind::WorldInFlames;
    out.x = x;
    out.y = y;
    return true;
}

// Ga'dok's Strafing Run: the lane stalkers carry the damage aura while the run is on; the boss itself flies the lane.
bool StrafeHazard(Player* bot, Hazard& out)
{
    float const safe = ClampedRadius(GSS_SPELL_STRAFING_RUN_DMG, 6.0f, 3.0f, 10.0f) + 1.5f;
    auto const lit = [](Creature* creature) { return creature->HasAura(GSS_SPELL_STRAFING_RUN_DMG); };

    Creature* nearest = NearestCreature(bot, GSS_NPC_STALKER_NORTH_SOUTH, GSS_SCAN, lit);
    Creature* other = NearestCreature(bot, GSS_NPC_STALKER_WEST_EAST, GSS_SCAN, lit);
    if (!nearest || (other && bot->GetExactDist2d(other) < bot->GetExactDist2d(nearest)))
        nearest = other;

    if (nearest && SetAway(bot, nearest->GetPositionX(), nearest->GetPositionY(), safe, HazardKind::Strafe, out))
        return true;

    // The boss's own flight line (a 5.4.8 flying boss strafes along its facing).
    Creature* gadok = bot->FindNearestCreature(GSS_NPC_GADOK, 160.0f);
    if (gadok && gadok->HasAura(GSS_SPELL_STRAFING_RUN))
    {
        return AreaHazard(bot, gadok->GetPositionX(), gadok->GetPositionY(), gadok->GetOrientation(), 0.0f, 6.0f,
                          140.0f, 2.0f, HazardKind::Strafe, out);
    }

    return false;
}

// Ri'mok's Frenzied Assault: 9 yd, 90 degree cone in front of him for 6 s. Wowpedia: "stay away or behind".
bool FrenzyHazard(Player* bot, Hazard& out)
{
    Creature* rimok = bot->FindNearestCreature(GSS_NPC_RIMOK, 40.0f);
    if (!rimok)
        return false;

    // the 1 s cast, then the 6 s channel aura on Ri'mok (boss_commander_rimok.cpp:130-135)
    if (!CurrentCast(rimok, GSS_SPELL_FRENZIED_ASSAULT) && !rimok->HasAura(GSS_SPELL_FRENZIED_ASSAULT))
        return false;

    if (rimok->GetVictim() == bot)
        return false;

    return AreaHazard(bot, rimok->GetPositionX(), rimok->GetPositionY(), rimok->GetOrientation(),
                      float(M_PI) / 4.0f, 0.5f, 9.0f, 2.0f, HazardKind::Frenzy, out);
}

// Raigonn's Fixate (phase 2): the target flees for 15 s with +75% speed; the rest of the group must not be the one
// he catches, so the fixated bot keeps running away from him.
bool FixateHazard(Player* bot, Hazard& out)
{
    if (!bot->HasAura(GSS_SPELL_FIXATE))
        return false;

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 120.0f);
    if (!raigonn)
        return false;

    return SetAway(bot, raigonn->GetPositionX(), raigonn->GetPositionY(), GSS_FIXATE_RUN_DIST, HazardKind::Fixate,
                   out);
}

// The Engulfing Winds a Krik'thik Engulfer leaves wander the arena and burn what they touch.
// Live creatures of `entry` within `range` (with `pred`), as positions.
template <class Pred>
std::vector<Position> Positions(Player* bot, uint32 entry, float range, Pred pred)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, entry, range);
    std::vector<Position> out;
    for (Creature* creature : found)
    {
        if (creature && creature->IsAlive() && pred(creature))
            out.push_back(creature->GetPosition());
    }

    return out;
}

// Distance from (x, y) to the nearest of `points`, capped at `cap`.
float Clearance(std::vector<Position> const& points, float x, float y, float cap)
{
    float clearance = cap;
    for (Position const& p : points)
        clearance = std::min(clearance, std::hypot(x - p.GetPositionX(), y - p.GetPositionY()));

    return clearance;
}

// A walkable spot clear of every point in `points` (FindBestSpot), for `kind`. False when the bot is already clear.
bool ClearOfAll(Player* bot, std::vector<Position> const& points, float safe, HazardKind kind, Hazard& out)
{
    if (points.empty() || Clearance(points, bot->GetPositionX(), bot->GetPositionY(), safe) >= safe)
        return false;

    auto const score = [&points, safe](float px, float py) { return Clearance(points, px, py, safe + 1.5f); };
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z))
        return false;

    out.kind = kind;
    out.x = x;
    out.y = y;
    return true;
}

// w58: 3 Engulfers keep dropping wandering tornadoes; stepping away from the nearest one walked into the next (855
// FAILED escapes, 1226 trigger ticks). Now a spot clear of all of them.
bool WindsHazard(Player* bot, Hazard& out)
{
    std::vector<Position> const winds = Positions(bot, GSS_NPC_ENGULFING_WINDS, GSS_SCAN, [](Creature*) { return true; });
    return ClearOfAll(bot, winds, GSS_WINDS_SAFE_DIST, HazardKind::Winds, out);
}

// w58: the armed bomb stalkers on Ri'mok's ramp (see MopGssShared.h).
bool BombHazard(Player* bot, Hazard& out)
{
    if (bot->GetVehicle())
        return false;

    std::vector<Position> const armed = Positions(bot, GSS_NPC_BOMB_STALKER, GSS_SCAN,
                                                  [](Creature* stalker) { return stalker->HasAura(GSS_SPELL_BOMB_AURA); });
    return ClearOfAll(bot, armed, GSS_BOMB_CLEAR, HazardKind::Bomb, out);
}

// #655 Ri'mok's Viscous Fluid pools (Viscos Fluid Stalkers 56883, see MopGssShared.h).
std::vector<Position> FluidPools(Player* bot, float range)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, GSS_NPC_VISCOUS_FLUID, range);
    std::vector<Position> pools;
    for (Creature* pool : found)
    {
        if (pool && pool->IsAlive())
            pools.push_back(pool->GetPosition());
    }

    return pools;
}

// Distance from (x, y) to the nearest pool centre, capped at 15 yd.
float FluidClearance(std::vector<Position> const& pools, float x, float y)
{
    float clearance = 15.0f;
    for (Position const& pool : pools)
        clearance = std::min(clearance, std::hypot(x - pool.GetPositionX(), y - pool.GetPositionY()));

    return clearance;
}

// The non-tanks stand out of the pools. A melee bot only takes a spot from which it still reaches Ri'mok; when there is
// none it stays, and the tank walks him out ("mop gss tank fluid").
bool FluidHazard(Player* bot, Hazard& out)
{
    if (PlayerBotSpec::IsTank(bot))
        return false;

    Creature* rimok = bot->FindNearestCreature(GSS_NPC_RIMOK, 60.0f);
    if (!rimok || !rimok->IsInCombat())
        return false;

    std::vector<Position> const pools = FluidPools(bot, GSS_SCAN);
    if (pools.empty() || FluidClearance(pools, bot->GetPositionX(), bot->GetPositionY()) >= GSS_FLUID_CLEAR)
        return false;

    bool const melee = !PlayerBotSpec::IsRanged(bot);
    float const reach = rimok->GetCombatReach() + 3.5f;
    float const rx = rimok->GetPositionX();
    float const ry = rimok->GetPositionY();
    auto const score = [&pools, melee, reach, rx, ry](float px, float py)
    {
        float s = std::min(FluidClearance(pools, px, py), GSS_FLUID_CLEAR + 1.0f);
        if (melee && std::hypot(px - rx, py - ry) > reach)
            s -= 20.0f;
        return s;
    };

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z))
        return false;

    out.kind = HazardKind::Fluid;
    out.x = x;
    out.y = y;
    return true;
}

// #655 Raigonn's Battering Headbutt (phase 1 only: the charges stop when the Weak Spot dies, boss_raigonn.cpp:188-190):
// keep out of the impact zone while he charges at it and while he stands at the door (the hit lands 0.5 s after he
// arrives, :219-225, and he waits 3 s, :268-271). Once he backs away from the door (moving, still near it) the zone is
// safe until his next charge.
bool HeadbuttHazard(Player* bot, Hazard& out)
{
    if (bot->GetVehicle())
        return false;

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 150.0f);
    if (!raigonn || !raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE))
        return false;

    static float const impacts[][2] = {{GSS_HEADBUTT_INNER_X, GSS_HEADBUTT_INNER_Y},
                                       {GSS_HEADBUTT_OUTER_X, GSS_HEADBUTT_OUTER_Y}};
    for (auto const& impact : impacts)
    {
        if (raigonn->GetExactDist2d(impact[0], impact[1]) < GSS_HEADBUTT_SAFE && raigonn->isMoving())
            continue;

        if (SetAway(bot, impact[0], impact[1], GSS_HEADBUTT_SAFE, HazardKind::Headbutt, out))
            return true;
    }

    return false;
}

struct InterruptRule
{
    uint32 entry;
    uint32 spellId;
};

// Krik'thik Saboteur's Bombard (3 s cast, rapid explosives on random enemies).
InterruptRule const SABOTEUR_RULES[] = {{GSS_NPC_KRIKTHIK_SABOTEUR, GSS_SPELL_BOMBARD}};

// Krik'thik Swarm Bringer's Screeching Swarm (1.5 s cast; 80000 heal absorb + 25000 Shadow per second).
InterruptRule const SWARM_RULES[] = {{GSS_NPC_SWARM_BRINGER, GSS_SPELL_SCREECHING_SWARM}};

// MoP interrupt spell names the class strategies register (kick, pummel, wind shear, mind freeze, counterspell,
// spell lock, skull bash, spear hand strike, rebuke, silencing shot, silence).
std::vector<std::string> const& InterruptSpellNames()
{
    static std::vector<std::string> const names = {"kick",           "pummel",           "wind shear",
                                                   "mind freeze",    "counterspell",     "spell lock",
                                                   "skull bash",     "spear hand strike", "rebuke",
                                                   "silencing shot", "silence"};
    return names;
}
}  // namespace

bool ComputeHazard(Player* bot, Hazard& out)
{
    out = Hazard();
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return false;

    return SabotageSelfHazard(bot, out) || SabotageCrossHazard(bot, out) || MunitionLaneHazard(bot, out) ||
           WorldInFlamesHazard(bot, out) || StrafeHazard(bot, out) || FrenzyHazard(bot, out) ||
           BombHazard(bot, out) || FluidHazard(bot, out) || FixateHazard(bot, out) || HeadbuttHazard(bot, out) ||
           WindsHazard(bot, out);
}

// #655 w65: the boss the tank has to take back - alive, engaged and hitting somebody else (nullptr otherwise).
static Creature* BossOffTank(Player* bot, uint32 entry, float range)
{
    Creature* boss = bot->FindNearestCreature(entry, range);
    if (!boss || !boss->IsInCombat() || !AttackersValue::IsValidTarget(boss, bot))
        return nullptr;

    Unit* victim = boss->GetVictim();
    if (!victim || victim == bot)
        return nullptr;

    return boss;
}

// #655 w65 tank focus. Ri'mok / Raigonn phase 2: the boss whenever he hits somebody else (the add pickups of "mop gss tank
// rimok adds" walked the tank off Ri'mok for 285 of 302 ticks; Raigonn's phase-2 AttackStart(random) killed the mage in
// two probes). Raigonn phase 1: he never melees (REACT_PASSIVE, DoMeleeAttackIfReady only in PHASE_VULNERABILITY,
// boss_raigonn.cpp:107/359) and his body takes -99%, yet the tank held him 95 ticks while 8-10 Protectorates / Swarm
// Bringers beat on him (attackers=10) and never switched: TankAssistTrigger only switches off a target he has aggro on
// (GenericTriggers.cpp:496-510). So the tank kills the add pile: the add he is on, else the nearest Swarm Bringer /
// Protectorate (Wowpedia "Raigonn": the tank "keep[s] aggro on ... random mantid adds").
static ObjectGuid ComputeTankFocus(Player* bot)
{
    if (Creature* rimok = BossOffTank(bot, GSS_NPC_RIMOK, 60.0f))
        return rimok->GetGUID();

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 130.0f);
    if (!raigonn || !raigonn->IsInCombat())
        return ObjectGuid::Empty;

    bool const phase1 = raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE);
    if (!phase1)
    {
        // #655 w68: Raigonn whether or not he is on the tank. With a focus only while he hit somebody else, the tank's own
        // pick ("tank target", FindTankTargetSmartStrategy: a unit without his aggro ranks first) took a Krik'thik Engulfer
        // (30 yd up) from the first phase-2 tick: melee FAILED for 27 s, rage 7-16, no Shield Block / Barrier, dead at Raigonn
        // 66% (leg log .t6xx-gss-mop.raigonn.log, Franuel target=56912 while atk=56877). Wowpedia "Raigonn": phase 2 is the
        // tank on Raigonn while the damage dealers clear the leftover adds and then focus him.
        if (AttackersValue::IsValidTarget(raigonn, bot))
            return raigonn->GetGUID();

        return ObjectGuid::Empty;
    }

    if (Unit* victim = bot->GetVictim())
    {
        uint32 const entry = victim->GetEntry();
        if (victim->IsAlive() && (entry == GSS_NPC_SWARM_BRINGER || entry == GSS_NPC_PROTECTORATE))
            return victim->GetGUID();
    }

    auto const engaged = [bot](Creature* creature) {
        return creature->IsInCombat() && AttackersValue::IsValidTarget(creature, bot);
    };
    if (Creature* bringer = NearestCreature(bot, GSS_NPC_SWARM_BRINGER, 40.0f, engaged))
        return bringer->GetGUID();

    if (Creature* protectorate = NearestCreature(bot, GSS_NPC_PROTECTORATE, 40.0f, engaged))
        return protectorate->GetGUID();

    return ObjectGuid::Empty;
}

ObjectGuid ComputeFocusTarget(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return ObjectGuid::Empty;

    if (PlayerBotSpec::IsTank(bot))
        return ComputeTankFocus(bot);

    if (PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return ObjectGuid::Empty;

    // #655: a bot the Artillery put on Raigonn's back sits on the Weak Spot (vehicle 1913, both seats CAN_ATTACK) and
    // hits it; while it rides nothing else is in reach.
    if (Unit* base = bot->GetVehicleBase())
    {
        if (base->GetEntry() == GSS_NPC_WEAK_SPOT && base->IsAlive())
            return base->GetGUID();

        return ObjectGuid::Empty;
    }

    auto const valid = [bot](Creature* creature) { return AttackersValue::IsValidTarget(creature, bot); };

    // Ri'mok (#655 w65): the damage dealers stay on Ri'mok. A Krik'thik Saboteur has 3.94M hp (Health_mod 10, world
    // creature_template 60447: 40% of Ri'mok's 9.78M) and one joins every ~45 s (Wowpedia "Commander Ri'mok"; w65 spellwatch
    // 120559: six Saboteur guids in 282 s); w62-w65 spent 201-457 target ticks on them, never killed one before the next
    // came, and Ri'mok lost 100->77% only in the ~40 s before the first one (then no bot hit him). His death despawns every
    // add (instance DATA_RIMOK DONE -> npc_add_generator DoAction(DONE), boss_commander_rimok.cpp:309-313); the Saboteurs'
    // Bombard is handled by "mop gss saboteur interrupt" only.
    if (Creature* rimok = bot->FindNearestCreature(GSS_NPC_RIMOK, 70.0f))
    {
        if (rimok->IsInCombat() && valid(rimok))
            return rimok->GetGUID();

        return ObjectGuid::Empty;
    }

    // Raigonn (#655): Wowpedia - "leave one DPS down to help the tank quickly kill Swarmbringers", the remaining DPS
    // ride the cannons to the Weak Spot ("mop gss cannon"; Impervious Carapace takes 99% off the boss until it dies).
    // On the ground: the Swarm Bringers first; the ranged then shoot the Weak Spot.
    if (Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 130.0f))
    {
        if (Creature* bringer = NearestCreature(bot, GSS_NPC_SWARM_BRINGER, 45.0f, valid))
            return bringer->GetGUID();

        // #655 w68 phase 2 (Broken Carapace, +300% damage taken): Wowpedia "Raigonn" - "kill the remaining adds, then focus on
        // Raigonn". No adds come in phase 2 (events.Reset(), boss_raigonn.cpp:188), so the leftover Protectorates (on the tank)
        // and, for the ranged, the Engulfers (81,742 hp, 30 yd up; their tornadoes) go first, then Raigonn. Without a focus the
        // generic pick sent the mage to an Engulfer and back (w68 probes 119-121) and left the boss to the rogue alone.
        if (raigonn->IsInCombat() && !raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE))
        {
            auto const engaged = [&valid](Creature* creature) { return creature->IsInCombat() && valid(creature); };
            if (Creature* protectorate = NearestCreature(bot, GSS_NPC_PROTECTORATE, 45.0f, engaged))
                return protectorate->GetGUID();

            if (PlayerBotSpec::IsRanged(bot))
            {
                // 2D range: they hover ~30 yd up
                if (Creature* engulfer = NearestCreature(bot, GSS_NPC_KRIKTHIK_ENGULFER, 30.0f, engaged))
                    return engulfer->GetGUID();
            }

            if (valid(raigonn))
                return raigonn->GetGUID();

            return ObjectGuid::Empty;
        }

        if (PlayerBotSpec::IsRanged(bot))
        {
            // #655 w69: phase 2 opened with 2-3 Protectorates (Hive Mind, +50% attack speed) still on the tank, who then died
            // 8 s in (w68, with none left: 27 s). Close to the end of phase 1 the ground ranged clear the Protectorates first
            // (Wowpedia "Raigonn": the tank and one damage dealer hold the adds); the rider keeps hitting the Weak Spot.
            Creature* weakSpotNow = bot->FindNearestCreature(GSS_NPC_WEAK_SPOT, 150.0f);
            if (weakSpotNow && weakSpotNow->GetHealthPct() <= GSS_WEAKSPOT_CLEAR_PCT)
            {
                auto const engaged = [&valid](Creature* creature) { return creature->IsInCombat() && valid(creature); };
                if (Creature* protectorate = NearestCreature(bot, GSS_NPC_PROTECTORATE, 45.0f, engaged))
                    return protectorate->GetGUID();
            }

            // the Weak Spot is friendly (faction 2137) until the fight starts: IsValidTarget skips it then
            // w59: with IsValidTarget's LoS test the Weak Spot (17 yd up on his back) was the mage's focus on 57 ticks only
            // and the rest went to Raigonn's body (-99%); the spell's own LoS check still applies
            auto const hostile = [bot](Creature* creature) {
                return creature->IsHostileTo(bot) && !creature->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
                       !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE);
            };
            if (Creature* weakSpot = NearestCreature(bot, GSS_NPC_WEAK_SPOT, 100.0f, hostile))
                return weakSpot->GetGUID();
        }
    }

    return ObjectGuid::Empty;
}

void CollectCarapaceExclusions(Player* bot, GuidSet& out)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || bot->GetMapId() != GSS_MAP_ID)
        return;

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 150.0f);
    if (!raigonn || !raigonn->IsInCombat() || !raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE))
        return;

    // a dead / missing Weak Spot leaves nothing else to hit: do not strip the group of every target then
    Creature* weakSpot = bot->FindNearestCreature(GSS_NPC_WEAK_SPOT, 150.0f);
    if (!weakSpot || !weakSpot->IsAlive())
        return;

    out.insert(raigonn->GetGUID());
}

bool TankLostBoss(Player* bot, uint32 bossEntry)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return false;

    if (bossEntry == GSS_NPC_RIMOK)
        return BossOffTank(bot, GSS_NPC_RIMOK, 60.0f) != nullptr;

    // Raigonn: phase 2 only (in phase 1 he never melees, the tank holds the adds). #655 w68: the whole of phase 2, on the tank or
    // not - "mop gss focus target" keeps the tank on him then (ComputeTankFocus), and a pickup would switch the target every tick.
    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 130.0f);
    return raigonn && raigonn->IsInCombat() && !raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE) &&
           AttackersValue::IsValidTarget(raigonn, bot);
}

void CollectTankExclusions(Player* bot, GuidSet& out)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || bot->GetMapId() != GSS_MAP_ID)
        return;

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 150.0f);
    if (!raigonn || !raigonn->IsInCombat())
        return;

    // w68: the tank stood under an Engulfer / the Weak Spot with melee FAILED in both phases (probes 3-34, 58-74, 105-131)
    static uint32 const unreachable[] = {GSS_NPC_KRIKTHIK_ENGULFER, GSS_NPC_WEAK_SPOT};
    for (uint32 const entry : unreachable)
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, 150.0f);
        for (Creature* creature : found)
        {
            if (creature && creature->IsAlive())
                out.insert(creature->GetGUID());
        }
    }
}

Unit* FindInterruptTarget(Player* bot, InterruptGroup group)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat())
        return nullptr;

    InterruptRule const* rules = group == InterruptGroup::Saboteur ? SABOTEUR_RULES : SWARM_RULES;
    size_t const count = group == InterruptGroup::Saboteur ? std::size(SABOTEUR_RULES) : std::size(SWARM_RULES);
    for (size_t i = 0; i < count; ++i)
    {
        uint32 const spellId = rules[i].spellId;
        Creature* caster = NearestCreature(bot, rules[i].entry, 40.0f, [spellId](Creature* creature) {
            return CurrentCast(creature, spellId) != nullptr;
        });
        if (caster)
            return caster;
    }

    return nullptr;
}

bool HasInterruptSpell(PlayerbotAI* botAI)
{
    for (std::string const& name : InterruptSpellNames())
    {
        if (botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get())
            return true;
    }

    return false;
}

bool CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    for (std::string const& name : InterruptSpellNames())
    {
        if (!botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get())
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return true;
    }

    return false;
}

bool ComputeTankDragSpot(Player* bot, float& x, float& y, float& z)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return false;

    Creature* rimok = bot->FindNearestCreature(GSS_NPC_RIMOK, 40.0f);
    if (!rimok || !rimok->IsInCombat() || rimok->GetVictim() != bot)
        return false;

    // he does not follow during Frenzied Assault (a channel aimed at the tank, boss_commander_rimok.cpp:129-137)
    if (CurrentCast(rimok, GSS_SPELL_FRENZIED_ASSAULT) || rimok->HasAura(GSS_SPELL_FRENZIED_ASSAULT))
        return false;

    // only while the tank holds him in melee (the generic reach action comes first)
    if (bot->GetExactDist2d(rimok) > rimok->GetCombatReach() + 6.0f)
        return false;

    std::vector<Position> const pools = FluidPools(bot, GSS_SCAN + 10.0f);
    if (pools.empty() || FluidClearance(pools, rimok->GetPositionX(), rimok->GetPositionY()) >= GSS_FLUID_CLEAR)
        return false;

    // he follows into melee reach of the tank, so the tank's own spot needs about his reach more than a pool radius
    auto const score = [&pools](float px, float py)
    {
        return std::min(FluidClearance(pools, px, py), GSS_FLUID_DRAG_CLEAR + 2.0f);
    };
    return MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z);
}

Creature* FindCannon(Player* bot)
{
    if (!bot || !bot->IsAlive() || !bot->IsInCombat() || bot->GetVehicle())
        return nullptr;

    // melee damage dealers only: the ranged shoot the Weak Spot from the ground, the tank and the healer stay down
    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot) ||
        PlayerBotSpec::IsRanged(bot))
        return nullptr;

    Creature* raigonn = bot->FindNearestCreature(GSS_NPC_RAIGONN, 150.0f);
    if (!raigonn || !raigonn->IsInCombat() || !raigonn->HasAura(GSS_SPELL_IMPERVIOUS_CARAPACE))
        return nullptr;

    // the Weak Spot turns hostile when the fight starts (boss_raigonn.cpp:143-149); it seats two riders
    Creature* weakSpot = NearestCreature(bot, GSS_NPC_WEAK_SPOT, 150.0f, [bot](Creature* creature) {
        return creature->IsHostileTo(bot) && !creature->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE) &&
               creature->GetVehicleKit() && creature->GetVehicleKit()->GetAvailableSeatCount() > 0;
    });
    if (!weakSpot)
        return nullptr;

    return NearestCreature(bot, GSS_NPC_ARTILLERY, 120.0f, [bot](Creature* creature) {
        return !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && creature->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK) &&
               creature->IsFriendlyTo(bot) && creature->GetVehicleKit() &&
               creature->GetVehicleKit()->GetAvailableSeatCount() > 0;
    });
}

FlakState ComputeFlak(Player* bot)
{
    FlakState state;
    if (!bot || !bot->IsAlive() || bot->GetMapId() != GSS_MAP_ID || bot->GetVehicle())
        return state;

    // the cannon only works once Ga'dok is DONE (npc_flak_cannon::SpellHit, boss_striker_gadok.cpp:794); Raigonn's
    // own fight has the cannon rides (MopGssCannonAction) and leaves no spare hands
    InstanceScript* instance = bot->GetInstanceScript();
    if (!instance || instance->GetBossState(GSS_BOSS_GADOK) != DONE)
        return state;

    EncounterState const raigonn = instance->GetBossState(GSS_BOSS_RAIGONN);
    if (raigonn == IN_PROGRESS || raigonn == DONE)
        return state;

    // never the tank or the healer (the group needs them), and never a real player
    if (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsDps(bot))
        return state;

    std::list<Creature*> bombers;
    bot->GetCreatureListWithEntryInGrid(bombers, GSS_NPC_KRIKTHIK_BOMBARDIER, GSS_FLAK_SCAN);
    for (Creature* bomber : bombers)
    {
        if (bomber && bomber->IsAlive())
            ++state.bombardiers;
    }

    if (!state.bombardiers)
        return state;

    // election: among the living damage dealers of the group the ranged ones first, then the lowest GUID
    if (Group* group = bot->GetGroup())
    {
        auto const rank = [](Player* member) {
            return (uint64(PlayerBotSpec::IsRanged(member) ? 0 : 1) << 32) | member->GetGUID().GetCounter();
        };

        Player* elected = nullptr;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (!member || !member->IsAlive() || member->GetMapId() != bot->GetMapId() || member->GetVehicle())
                continue;

            PlayerbotAI* memberAI = GET_PLAYERBOT_AI(member);
            if (!memberAI || memberAI->IsRealPlayer())
                continue;

            if (PlayerBotSpec::IsTank(member) || PlayerBotSpec::IsHeal(member) || !PlayerBotSpec::IsDps(member))
                continue;

            if (!elected || rank(member) < rank(elected))
                elected = member;
        }

        if (elected != bot)
            return state;
    }

    state.cannon = NearestCreature(bot, GSS_NPC_FLAK_CANNON, GSS_FLAK_SCAN, [bot](Creature* creature) {
        return !creature->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) && creature->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK) &&
               creature->IsFriendlyTo(bot);
    });
    return state;
}
}  // namespace MopGss
