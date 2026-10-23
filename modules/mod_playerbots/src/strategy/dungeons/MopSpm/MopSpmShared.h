/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Shado-Pan Monastery (map 959) strategy "mop-spm" - MoP-only (epic #373, issue #653).
// AC WotLK mod-playerbots has no strategy for this dungeon, so this follows AC's dungeon-strategy PATTERN:
//   triggers -> avoid action (AC Ai/Dungeon/HoS/HoSStrategy.cpp:9-33, HoSActions.cpp:9-60),
//   movement lock while a mechanic is live (AC HoSMultipliers.cpp:14-45),
//   "do not attack the boss now" multiplier (AC Ai/Dungeon/VH/VHMultipliers.cpp:43-61, Zuramat's Shroud of Darkness),
//   target exclusions (AC Ai/Dungeon/UB/UBStrategy.cpp AppendTargetExclusions),
//   skull marking of priority adds by one tracker bot (fork Utils/EncounterHelpers.h MarkTargetWithSkull).
// Mechanics (read 2026-10-06):
//   - Wowpedia MediaWiki API: Gu_Cloudstrike, Master_Snowdrift_(tactics), Sha_of_Violence, Taran_Zhu_(tactics)
//   - Wowhead tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id> for the spell texts
//   - what THIS core casts: src/server/scripts/Pandaria/ShadopanMonastery/boss_*.cpp, plus the 5.4.8 DBC
//     (SpellEffect / SpellRadius / SpellTargetRestrictions) for radii, cone angles and cast times used below.
// Edges per boss (a mechanic the fork's script never casts has NO edge - see the issue comment for the gaps):
//   Gu Cloudstrike    Static Field pools (9 yd, 15 s), Invoke Lightning spread (5 yd), Azure Serpent's Lightning Breath
//                     cone (45 degrees, 80 yd), phase 2: Gu is immune (Charging Soul) so the serpent is the target
//   Master Snowdrift  Fists of Fury cone (60 degrees, 10 yd), Tornado Kick (8 yd), Parry Stance (stop all damage),
//                     Chase Down fixate (kite), Flying Snow's Whirling Steel (3 yd)
//                     #780: Ball of Fire spheres (59225, Fragrant Lotus and the phase-2/3 clones): avoid the lane ahead
//   Sha of Violence   none: Sha Spike / Disorienting Smash / Smoke Blades are instant and untelegraphed on this core
//   Taran Zhu         Ring of Malice (damages 8.5-15 yd from him), Haze of Hate -> Meditate, skull on Gripping Hatred

#ifndef PLAYERBOTS_MOPSPMSHARED_H
#define PLAYERBOTS_MOPSPMSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <algorithm>
#include <cmath>
#include <vector>

class Player;
class Unit;

namespace MopSpm
{
constexpr uint32 MAP_SPM = 959;

// Creatures (fork shadopan_monastery.h:46-83 and the boss scripts; the same entries are in the MoP world DB).
constexpr uint32 NPC_GU_CLOUDSTRIKE = 56747;
constexpr uint32 NPC_AZURE_SERPENT = 56754;
constexpr uint32 NPC_STATIC_FIELD = 56803;       // Static Field Stalker: carries the 15 s damage aura
constexpr uint32 NPC_MASTER_SNOWDRIFT = 56541;
constexpr uint32 NPC_FLYING_SNOW = 56473;        // senior student, casts Whirling Steel
constexpr uint32 NPC_BALL_OF_FIRE = 59225;       // #780: sphere summoned by Ball of Fire 113760, flies 100 yd straight on
constexpr uint32 NPC_TARAN_ZHU = 56884;
constexpr uint32 NPC_GRIPPING_HATRED = 59804;    // pulls the whole group in after a 10 s cast

// Spells (ids from the fork scripts; a heroic cast keeps the same id).
constexpr uint32 SPELL_INVOKE_LIGHTNING = 106984;  // 2 s cast on a random player, 5 yd splash (107202)
constexpr uint32 SPELL_STATIC_FIELD_AURA = 106941; // on the stalker: 128889 every second, 9 yd
constexpr uint32 SPELL_LIGHTNING_BREATH = 102573;  // serpent: 2 s aura, 102572 every 500 ms
constexpr uint32 SPELL_CHARGING_SOUL = 110945;     // Gu: immune, stunned in phase 2
constexpr uint32 SPELL_FISTS_OF_FURY = 106853;     // 1 s cast + 5 s aura, 106854 every 500 ms
constexpr uint32 SPELL_TORNADO_KICK = 106434;      // 1.5 s cast + 5 s aura, 106433 every second (8 yd)
constexpr uint32 SPELL_PARRY_STANCE = 106454;      // 2 s cast, 4 s aura: any hit is answered (Quivering Palm)
constexpr uint32 SPELL_PARRY_STANCE_TRIGGERED = 106844;
constexpr uint32 SPELL_CHASE_DOWN = 118961;        // fixate on the Tornado Slam target (11 s)
constexpr uint32 SPELL_WHIRLING_STEEL = 106646;    // 1 s cast + 10 s aura, 106645 every second (3 yd)
constexpr uint32 SPELL_RING_OF_MALICE = 131521;    // 15 s aura on Taran Zhu, 131522 every 500 ms (15 yd)
constexpr uint32 SPELL_HAZE_OF_HATE = 107087;      // at 100 Hatred: -100% hit, only Meditate clears it
constexpr uint32 SPELL_MEDITATE = 107200;          // the extra-action button: 5 s stun, resets Hatred

// Geometry from the 5.4.8 DBC (SpellRadius / SpellTargetRestrictions) plus a margin where the shape moves.
constexpr float STATIC_FIELD_RADIUS = 9.0f;          // 128889
constexpr float INVOKE_LIGHTNING_RADIUS = 5.0f;      // 107202
constexpr float LIGHTNING_BREATH_HALF_ANGLE = 0.60f; // 102572 is a 45 degree cone; +12 degrees
constexpr float LIGHTNING_BREATH_LENGTH = 85.0f;     // 80 yd
constexpr float FISTS_HALF_ANGLE = 0.73f;            // 106854 is a 60 degree cone; +12 degrees
constexpr float FISTS_LENGTH = 11.0f;                // 10 yd
constexpr float TORNADO_KICK_RADIUS = 8.0f;          // 106433
constexpr float CHASE_DOWN_KITE_RADIUS = 16.0f;
constexpr float WHIRLING_STEEL_RADIUS = 3.0f;        // 106645
// #780 Ball of Fire sphere (npc_snowdrift_fireball, boss_master_snowdrift.cpp:1264-1282: MovePoint 100 yd along its own
// orientation at run speed 1.14286 x 7 = 8 yd/s; damage only within 0.5 yd + combat reach, :1665-1686): the strip ~2.5 s ahead of it,
// or a circle once it has stopped.
constexpr float BALL_OF_FIRE_HALF_WIDTH = 3.0f;
constexpr float BALL_OF_FIRE_AHEAD = 20.0f;
constexpr float BALL_OF_FIRE_BEHIND = 1.5f;
constexpr float RING_INNER = 8.5f;                   // the fork's script spares everyone within 8.5 yd of Taran Zhu
constexpr float RING_OUTER = 15.0f;                  // 131522

constexpr float SCAN_RANGE = 150.0f;
constexpr float NEAR_SCAN_RANGE = 60.0f;
constexpr float MARK_SCAN_RANGE = 60.0f;

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
// Contains() takes a `grow` margin (yards) so a destination can be required to clear the shape with room to spare.
struct Hazard
{
    enum class Shape : uint8
    {
        Circle,
        Ring,
        Lane,
        Cone
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

// Master Snowdrift is in Parry Stance (casting it or carrying the aura): anything that hits him is answered with a
// stun and a flying kick, so the bot must not attack until it ends.
bool IsParryStanceActive(Player* bot);

// The bot carries Haze of Hate (Hatred reached 100) and is not already meditating.
bool NeedsMeditate(Player* bot);

// The add the whole group should kill first (Gripping Hatred, or the Azure Serpent during Gu's phase 2), or nullptr.
// One tracker bot marks it with the skull icon, which every bot's target selection ranks first.
Unit* GetPriorityMarkTarget(Player* bot);

// Guids the target values must skip: Gu Cloudstrike while Charging Soul makes him immune and the Azure Serpent can be
// fought instead.
void CollectExclusions(Player* bot, GuidSet& out);
}

#endif
