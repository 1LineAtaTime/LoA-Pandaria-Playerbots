/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Temple of the Jade Serpent (map 960) strategy "mop-tjs" - MoP-only (epic #373, issue #651).
// AC WotLK mod-playerbots has no strategy for this dungeon, so this follows AC's dungeon-strategy PATTERN:
//   triggers -> avoid action (AC Ai/Dungeon/HoS/HoSStrategy.cpp:9-33, HoSActions.cpp:9-60),
//   movement lock while a mechanic is live (AC HoSMultipliers.cpp:14-45),
//   target exclusions (AC Ai/Dungeon/UB/UBStrategy.cpp AppendTargetExclusions; fork strategy/dungeons/UB, MgT).
// Mechanics (read 2026-10-06):
//   - Wowpedia MediaWiki API: Wise_Mari, Lorewalker_Stonestep and Lorewalker_Stonestep_(tactics), Liu_Flameheart, Sha_of_Doubt
//   - Wowhead tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id> for the spell texts
//   - what THIS core casts: src/server/scripts/Pandaria/TempleOfTheJadeSerpent/boss_*.cpp, plus the 5.4.8 DBC
//     (SpellEffect / SpellRadius / SpellTargetRestrictions) for the radii, cone angles and cast times used below.
// Edges per boss (a mechanic the fork's script never casts has NO edge - see the issue comment for the gaps):
//   Wise Mari          boss left alone while Water Bubble is up (kill the Corrupt Living Waters); leave Sha Residue puddles;
//                      leave the Hydrolance line; sidestep the rotating Wash Away beam
//   Lorewalker (trial) Strife/Peril: DPS switch off the one with Ultimate Power or high Intensity stacks
//   Liu Flameheart     step out of the Serpent / Jade Serpent wave lanes
//   Sha of Doubt       boss left alone during Bounds of Reality (kill the Figments of Doubt); spread from Touch of Nothingness

#ifndef PLAYERBOTS_MOPTJSSHARED_H
#define PLAYERBOTS_MOPTJSSHARED_H

#include "Define.h"
#include "MopDungeonShared.h"
#include "ObjectGuid.h"
#include <algorithm>
#include <cmath>
#include <vector>

class Player;

namespace MopTjs
{
constexpr uint32 MAP_TJS = 960;

// Creatures (fork temple_of_the_jade_serpent.h:42-82 and the boss scripts; the same entries are in the MoP world DB).
constexpr uint32 NPC_WISE_MARI = 56448;
constexpr uint32 NPC_SHA_OF_DOUBT = 56439;
constexpr uint32 NPC_DRAGON_WAVE = 56789;  // Liu's wave trigger (boss_liu_flameheart.cpp, npc_wave_trigger_liu_flameheart)
constexpr uint32 NPC_TRIAL_STRIFE = 59051; // the two trial enemies of the Lorewalker event (boss_lorewalker_stonestep.cpp)
constexpr uint32 NPC_TRIAL_PERIL = 59726;
// #651 Lorewalker Stonestep: the Corrupted Scroll's death picks one of two tales at random (instance_temple_of_the_jade_serpent.cpp:
// 298-359). Champion of the Five Suns: five Suns (56915) turn selectable ~30 s later (DATA_SUN DONE, :443-465), each dead Sun
// summons a Haunting Sha (58856, Extract Sha 111768) that attacks the group; five dead Sha corrupt Zao Sunseeker (58826,
// faction 1770 -> 14, boss_lorewalker_stonestep.cpp:363-376) and his death ends the encounter (:389-404, :500-506). None of
// the Suns or Zao ever puts a player on its threat list, so they never show up in "attackers" (AC UK Frost Tomb case).
constexpr uint32 NPC_LOREWALKER_SUN = 56915;
constexpr uint32 NPC_HAUNTING_SHA = 58856;
constexpr uint32 NPC_ZAO_SUNSEEKER = 58826;
constexpr float LOREWALKER_RANGE = 60.0f;  // the Suns, Zao and Strife/Peril are all within 25 yd of the room centre
// #781 Liu -> Yu'lon: at 30% Liu turns NOT_SELECTABLE | NON_ATTACKABLE and summons Yu'lon (boss_liu_flameheart.cpp:
// 259-265); Yu'lon (56762) starts at 30% health and her "death" makes her breathe on Liu (boss_yu_lon_tjs :327-395).
// Wowpedia "Liu Flameheart": "summoning the spirit of Yu'lon to defend her in battle". Every non-healer switches.
constexpr uint32 NPC_LIU_FLAMEHEART = 56732;
constexpr uint32 NPC_YULON = 56762;
// #815 Wise Mari phase 1: Call Water summons one Corrupt Living Water (56511, spell 106462) at one of the four
// fountains (boss_wise_mari.cpp:349-371, FountainTriggerSummonPosition temple_of_the_jade_serpent.h:152-158); a dead
// one splits into Corrupt Droplets (62358, npc_corrupt_living_water :528-543). Four dead waters = Bubble Burst.
constexpr uint32 NPC_CORRUPT_LIVING_WATER = 56511;
constexpr uint32 NPC_CORRUPT_DROPLET = 62358;
constexpr float LIVING_WATER_RANGE = 60.0f;  // the four fountains are 25-29 yd from Mari; the group stands south of him
constexpr float LIVING_WATER_RANGED_DIST = 15.0f;  // a ranged bot out of sight closes to this, then casts

inline MopDungeon::WaveSpec YulonSwitch()
{
    return {NPC_LIU_FLAMEHEART, {NPC_YULON}, MopDungeon::WaveRole::All, 80.0f};
}

// Spells (ids from the fork scripts; a heroic cast keeps the same id).
constexpr uint32 SPELL_WATER_BUBBLE = 106062;       // boss_wise_mari.cpp SPELL_WATER_BUBBLE_WISE: immune + knockback
constexpr uint32 SPELL_HYDROLANCE_CAST = 106055;    // 4 s cast, Mari turns to the lane first
constexpr uint32 SPELL_HYDROLANCE_AURA = 106098;    // 1.75 s aura: pulses (106104) every 250 ms, 2 yd further each time
constexpr uint32 SPELL_WASH_AWAY = 106331;          // aura: rotating beam, trigger 106334 every 250 ms
constexpr uint32 SPELL_SHA_RESIDUE = 106653;        // persistent area aura (dynamic object), 5 yd, 2 minutes
constexpr uint32 SPELL_BOUNDS_OF_REALITY = 117665;  // boss_sha_of_doubt.cpp: boss immune while the figments live
constexpr uint32 SPELL_TOUCH_OF_NOTHINGNESS = 106113;
constexpr uint32 SPELL_INTENSITY = 113315;          // stacking aura on Strife/Peril (spell_tjs_intensity)
constexpr uint32 SPELL_ULTIMATE_POWER = 113309;     // 10 stacks: 15 s of immunity

// Geometry from the 5.4.8 DBC (SpellRadius / SpellTargetRestrictions) plus a margin where the shape moves.
constexpr float SHA_RESIDUE_RADIUS = 5.0f;          // 106653 effect radius
constexpr float HYDROLANCE_HALF_WIDTH = 3.5f;       // 106104 pulse radius
constexpr float HYDROLANCE_START = 8.0f;            // the first pulse lands past Mari's 16 yd body radius
constexpr float HYDROLANCE_LENGTH = 34.0f;          // 7 pulses, 2 yd apart, from ~16 yd out
constexpr float WASH_AWAY_HALF_ANGLE = 0.35f;       // 106334 beam is a 12 degree cone (6 deg half); 20 deg leaves room for the 18 deg/s turn
constexpr float WASH_AWAY_LENGTH = 60.0f;
constexpr float WASH_AWAY_SAFE_Z = 174.7f;          // the script drops players above this height from the beam
// #816 Corrupted Waters (115165, npc_wise_stalker while Mari is in combat; CorruptedWatersTargetSelector in
// boss_wise_mari.cpp:731-760 keeps a player as a target when he is within 20 yd of roomCenter in its front half-plane
// (HasInArc(PI), facing 4.33) or within 14 yd behind it, AND his z is in 174.05..174.23; a second pool 170.19..170.215 within
// 30 yd). Wowpedia "Wise Mari": "be sure to not stand in any of the water ... stand only on the dry sections of floor".
// The dry rim (fountain triggers 56586) sits at z ~176, the centre platform at 174.96. Z windows widen the script band by the
// navmesh/position jitter (a standing bot reports the floor +-0.2).
constexpr float MARI_ROOM_X = 1046.941f;
constexpr float MARI_ROOM_Y = -2560.606f;
constexpr float MARI_ROOM_FACING = 4.33f;
constexpr float CORRUPTED_FRONT_RADIUS = 20.0f;
constexpr float CORRUPTED_BACK_RADIUS = 14.0f;
constexpr float CORRUPTED_POOL_RADIUS = 30.0f;
constexpr float CORRUPTED_Z_MIN = 173.65f;
constexpr float CORRUPTED_Z_MAX = 174.63f;          // below the 174.7 Wash Away line and the 174.96 platform
constexpr float CORRUPTED_POOL_Z_MIN = 169.7f;
constexpr float CORRUPTED_POOL_Z_MAX = 170.7f;
constexpr float WAVE_HALF_WIDTH = 3.0f;             // 106938 / 107053: 3 yd, 180 degree cone
constexpr float WAVE_FORWARD = 16.0f;               // the wave moves at 11.5 yd/s
constexpr float WAVE_BACK = 1.5f;
constexpr float TOUCH_RADIUS = 6.0f;                // 106228
constexpr uint32 INTENSITY_SWITCH_STACKS = 6;       // Wowpedia (tactics): switch when the stacks get above ~6

constexpr float SCAN_RANGE = 150.0f;
constexpr float DYNOBJ_SCAN_RANGE = 40.0f;
constexpr float WAVE_SCAN_RANGE = 80.0f;

// AppendTargetExclusions is asked several times per tick; the answer is cached for this long.
constexpr uint32 EXCLUSION_CACHE_MS = 200;

// How far inside a hazard (plus this margin) the bot counts as "in danger", and how far outside every hazard
// a destination has to be. The gap between the two keeps the bot from stepping out and straight back in.
constexpr float DANGER_GRACE = 1.0f;
constexpr float SAFE_CLEARANCE = 2.0f;

// Danger is re-evaluated at most this often inside the movement multiplier (it is asked once per candidate action).
constexpr uint32 DANGER_CACHE_MS = 150;

// Raid target icon index of the skull (Group::GetTargetIcon / SetTargetIcon, same value as RtiTargetValue's skull).
constexpr uint8 SKULL_ICON = 7;

// ---- ground hazards -------------------------------------------------------------------------------
// One damaging ground shape in world X/Y.
//   Circle : everything closer than `radius` to (x, y)
//   Ring   : the band between `inner` and `radius` around (x, y) (safe inside `inner` and outside `radius`)
//   Lane   : a strip of half width `width` from `inner` yards behind (x, y) to `radius` yards ahead along (dirX, dirY)
//   Cone   : a wedge of length `radius` around (dirX, dirY) whose half angle has the cosine `cosHalf`
//   Half   : #816 the half disc of `radius` around (x, y) on the (dirX, dirY) side (`width` > 0) or the opposite side (< 0)
// Contains() takes a `grow` margin (yards) so a destination can be required to clear the shape with room to spare.
struct Hazard
{
    enum class Shape : uint8
    {
        Circle,
        Ring,
        Lane,
        Cone,
        Half
    };

    Shape shape = Shape::Circle;
    float x = 0.0f;
    float y = 0.0f;
    float dirX = 1.0f;
    float dirY = 0.0f;
    float radius = 0.0f;
    float inner = 0.0f;
    float width = 0.0f;
    float cosHalf = 1.0f;

    static Hazard MakeCircle(float cx, float cy, float r)
    {
        Hazard h;
        h.shape = Shape::Circle;
        h.x = cx;
        h.y = cy;
        h.radius = r;
        return h;
    }

    static Hazard MakeRing(float cx, float cy, float innerRadius, float outerRadius)
    {
        Hazard h;
        h.shape = Shape::Ring;
        h.x = cx;
        h.y = cy;
        h.inner = innerRadius;
        h.radius = outerRadius;
        return h;
    }

    static Hazard MakeLane(float ox, float oy, float angle, float halfWidth, float forward, float back)
    {
        Hazard h;
        h.shape = Shape::Lane;
        h.x = ox;
        h.y = oy;
        h.dirX = std::cos(angle);
        h.dirY = std::sin(angle);
        h.width = halfWidth;
        h.radius = forward;
        h.inner = back;
        return h;
    }

    static Hazard MakeCone(float ox, float oy, float angle, float halfAngle, float length)
    {
        Hazard h;
        h.shape = Shape::Cone;
        h.x = ox;
        h.y = oy;
        h.dirX = std::cos(angle);
        h.dirY = std::sin(angle);
        h.radius = length;
        h.cosHalf = std::cos(halfAngle);
        return h;
    }

    static Hazard MakeHalfDisc(float cx, float cy, float angle, float r, bool front)
    {
        Hazard h;
        h.shape = Shape::Half;
        h.x = cx;
        h.y = cy;
        h.dirX = std::cos(angle);
        h.dirY = std::sin(angle);
        h.radius = r;
        h.width = front ? 1.0f : -1.0f;
        return h;
    }

    bool Contains(float px, float py, float grow) const
    {
        float const dx = px - x;
        float const dy = py - y;
        float const dist = std::sqrt(dx * dx + dy * dy);

        switch (shape)
        {
            case Shape::Circle:
                return dist < radius + grow;
            case Shape::Ring:
                return dist > inner - grow && dist < radius + grow;
            case Shape::Lane:
            {
                float const forward = dx * dirX + dy * dirY;
                float const lateral = -dx * dirY + dy * dirX;
                return std::fabs(lateral) < width + grow && forward > -inner - grow && forward < radius + grow;
            }
            case Shape::Half:
            {
                if (dist >= radius + grow)
                    return false;
                float const side = dx * dirX + dy * dirY;
                return width > 0.0f ? side > -grow : side < grow;
            }
            case Shape::Cone:
            {
                if (dist >= radius + grow)
                    return false;
                if (dist < 0.5f + grow)
                    return true;

                float const cosAngle = (dx * dirX + dy * dirY) / dist;
                float const halfAngle =
                    std::acos(std::max(-1.0f, std::min(1.0f, cosHalf))) + grow / dist;
                if (halfAngle >= 3.14159265f)
                    return true;

                return cosAngle > std::cos(halfAngle);
            }
        }

        return false;
    }
};

inline bool InAnyHazard(std::vector<Hazard> const& hazards, float x, float y, float grow)
{
    for (Hazard const& hazard : hazards)
    {
        if (hazard.Contains(x, y, grow))
            return true;
    }

    return false;
}

// Every ground shape the bot should stay out of right now (empty when no mechanic is live).
void CollectHazards(Player* bot, std::vector<Hazard>& out);

// True when the bot stands inside (or within DANGER_GRACE of) one of the CollectHazards() shapes.
bool IsInDanger(Player* bot);

// Nearest walkable point outside every hazard; each candidate is validated with
// EncounterHelpers::CanTakeStepTowards. False when no candidate qualifies.
bool FindSafePoint(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ);

// In danger AND a safe point exists. The movement multiplier uses this so a bot that cannot get out of a hazard (no
// walkable spot) is never frozen in it by the movement lock.
bool CanEscapeDanger(Player* bot);

// Guids the target values must skip (units that cannot be hurt right now). `tankType` is true for the tank target
// value, which keeps holding both Strife and Peril.
void CollectExclusions(Player* bot, bool tankType, GuidSet& out);

// #815: the Corrupt Living Water (else a Corrupt Droplet) this non-healer bot (tank included) should kill while Wise
// Mari is bubbled, or nullptr. Line of sight is NOT required: the bot walks to an add it cannot see.
Unit* FindLivingWater(Player* bot);

// #651: the Lorewalker Stonestep enemy this non-healer bot (tank included) should attack, or nullptr: corrupted Zao, else
// Strife/Peril (the attackable one with fewer Intensity stacks), else the nearest selectable Sun while no Haunting Sha is up.
// Line of sight is NOT required (a Sun hovers up to 7 yd above the floor).
Unit* FindLorewalkerTarget(Player* bot);

// #651: `unit` is a live enemy of the Lorewalker event (Sun, Haunting Sha, Zao, Strife, Peril).
bool IsLorewalkerEnemy(Unit* unit);
}

#endif
