/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Stormstout Brewery (map 961) strategy "mop-ssb" - MoP-only (epic #373, issue #652).
// AC WotLK mod-playerbots has no strategy for this dungeon, so this follows AC's dungeon-strategy PATTERN:
//   triggers -> avoid action (AC Ai/Dungeon/HoS/HoSStrategy.cpp:9-33, HoSActions.cpp:9-60),
//   movement lock while a mechanic is live (AC HoSMultipliers.cpp:14-45),
//   skull marking of priority adds by one tracker bot (fork Utils/EncounterHelpers.h MarkTargetWithSkull /
//   IsMechanicTrackerBot, the AC idiom for "kill this first").
// Mechanics (read 2026-10-06):
//   - Wowpedia MediaWiki API: Ook-Ook, Hoptallus, Yan-Zhu_the_Uncasked
//   - Wowhead tooltip API nether.wowhead.com/mop-classic/tooltip/spell/<id> for the spell texts
//   - what THIS core casts: src/server/scripts/Pandaria/StormstoutBrewery/boss_*.cpp, plus the 5.4.8 DBC
//     (SpellEffect / SpellRadius / SpellTargetRestrictions) for radii, cone angles and cast times used below.
// Edges per boss (a mechanic the fork's script never casts has NO edge - see the issue comment for the gaps):
//   Ook-Ook      Ground Pound cone (35 degrees, 30 yd, channeled at the victim); Rolling Barrel lanes (50k explosion)
//   Hoptallus    Furlwind (8 yd spin), Carrot Breath (20 degree cone, 30 yd), lit Explosive Hoppers (10 yd, 5 s fuse),
//                skull on the nearest hopper
//   Yan-Zhu      Blackout Brew (keep moving: 10 stacks stun for 10 s), Bloat/Gushing Brew cones (10 degrees, 8 yd),
//                skull on the Yeasty Brew Alementals

#ifndef PLAYERBOTS_MOPSSBSHARED_H
#define PLAYERBOTS_MOPSSBSHARED_H

#include "Define.h"
#include "ObjectGuid.h"
#include <algorithm>
#include <cmath>
#include <vector>

class Player;
class Unit;

namespace MopSsb
{
constexpr uint32 MAP_SSB = 961;

// Creatures (fork stormstout_brewery.h:44-80 and the boss scripts; the same entries are in the MoP world DB).
constexpr uint32 NPC_OOK_OOK = 56637;
constexpr uint32 NPC_ROLLING_BARREL = 56682;     // boss_ook_ook.cpp npc_barrel (friendly faction, explodes on contact)
constexpr uint32 NPC_HOPTALLUS = 56717;
constexpr uint32 NPC_EXPLOSIVE_HOPPER = 56718;   // boss_hoptallus.cpp npc_explosive_hopper
constexpr uint32 NPC_YAN_ZHU = 59479;
constexpr uint32 NPC_YEASTY_ALEMENTAL = 59494;   // Yan-Zhu add that feeds him (Ferment)
constexpr uint32 NPC_BLOATED_STALKER = 59482;    // rides the Bloat target and breathes Gushing Brew (spell_bloat_aura)
// #690 Yan-Zhu brewmastery (boss_yanzhu_the_uncasked.cpp): Stout pair = Carbonation + Fizzy Bubbles or Wall of Suds + Sudsy;
// Ale pair = Yeasty adds or Bubble Shield. Wowpedia "Yan-Zhu the Uncasked" (2026-10-22): "grab a Fizzy Bubble to fly up,
// otherwise, you're silenced", "use Sudsy [jump] when he casts Wall of Suds", "take out the bubbles".
constexpr uint32 NPC_WALL_OF_SUDS = 59512;       // walks 90 yd along its facing at 0.9 x walk speed (npc_wall_of_suds)
constexpr uint32 NPC_FIZZY_BUBBLE = 59799;       // spellclick 114459 (world DB npc_spellclick_spells), 12 s lifetime
constexpr uint32 NPC_BUBBLE_SHIELD = 65522;      // 8 of them hover 8 yd above Yan-Zhu while his Bubble Shield is up

// Spells (ids from the fork scripts; a heroic cast keeps the same id).
constexpr uint32 SPELL_GROUND_POUND = 106807;    // 1.5 s cast + 3 s channel, trigger 106808 every second
constexpr uint32 SPELL_FURLWIND = 112992;        // 2.5 s cast + 10 s aura, trigger 112993 every second
constexpr uint32 SPELL_CARROT_BREATH = 112944;   // 3 s cast + 15 s aura, trigger 112945 every 500 ms
constexpr uint32 SPELL_LIT_FUSE = 114290;        // on a Hopper from the moment it starts Explosive Brew
constexpr uint32 SPELL_EXPLOSIVE_BREW = 114291;  // 5 s cast, 10 yd blast + knock down
constexpr uint32 SPELL_BLACKOUT_BREW = 106851;   // stacking aura: +1 per second standing still, -1 moving
constexpr uint32 SPELL_GUSHING_BREW = 106549;    // aura on the stalkers: 106560 every 500 ms (10 degree cone, 8 yd)
constexpr uint32 SPELL_CARBONATION = 115003;     // 3 s cast, then 20 s of silence + damage below caster z + 4.9 (spell_carbonation)
constexpr uint32 SPELL_FIZZY_FLIGHT = 114459;    // aura from clicking a Fizzy Bubble: 20 s of flight (spell_fizzi_buble)
constexpr uint32 SPELL_SUDSY = 114468;           // on every player when Yan-Zhu has the Sudsy brew: a jump becomes a high leap
constexpr uint32 SPELL_BUBBLE_SHIELD = 106563;   // Yan-Zhu's aura: 10% damage reduction per bubble left

// Geometry from the 5.4.8 DBC (SpellRadius / SpellTargetRestrictions) plus a margin where the shape moves.
constexpr float GROUND_POUND_HALF_ANGLE = 0.48f;    // 106808 is a 35 degree cone; +10 degrees
constexpr float GROUND_POUND_LENGTH = 32.0f;        // 30 yd
constexpr float BARREL_HALF_WIDTH = 3.5f;           // the barrel explodes within 3.3 yd of a unit; blast 3-5 yd
constexpr float BARREL_FORWARD = 14.0f;
constexpr float BARREL_BACK = 2.0f;
constexpr float FURLWIND_RADIUS = 8.0f;             // 112993
constexpr float CARROT_HALF_ANGLE = 0.49f;          // 112945 is a 20 degree cone; +18 degrees (the boss turns)
constexpr float CARROT_LENGTH = 32.0f;              // 30 yd
constexpr float HOPPER_BLAST_RADIUS = 10.0f;        // 114291
constexpr float BLOAT_HALF_ANGLE = 0.23f;           // 106560 is a 10 degree cone; +8 degrees
constexpr float BLOAT_LENGTH = 10.0f;               // 8 yd

constexpr uint32 BLACKOUT_MOVE_STACKS = 6;          // the 10th stack stuns for 10 s (spell_blackout_brew)
constexpr float BLACKOUT_STEP = 8.0f;               // ~1.1 s of running, so the bot is moving when the aura ticks
constexpr float BLACKOUT_MIN_RADIUS = 4.0f;

constexpr float FIZZY_SCAN_RANGE = 45.0f;           // the bubbles spawn within 20 yd of Yan-Zhu, 1.5-3 yd above the floor
constexpr float FIZZY_CLICK_DIST = 5.0f;            // INTERACTION_DISTANCE - 1 on the surface distance (the #818 click trap)
constexpr float FLIGHT_ABOVE_BOSS = 8.0f;           // Carbonation only hurts units below the caster z + 4.9
constexpr float WALL_JUMP_DIST = 8.0f;              // the wall walks ~2 yd/s: a 1 s Sudsy leap started this far out clears it
constexpr float WALL_JUMP_STEP = 2.5f;              // hop away from the wall; any arc above the floor triggers the Sudsy leap
constexpr float BUBBLE_ATTACK_RANGE = 40.0f;

constexpr float SCAN_RANGE = 150.0f;
constexpr float BARREL_SCAN_RANGE = 45.0f;
constexpr float STALKER_SCAN_RANGE = 60.0f;
constexpr float MARK_SCAN_RANGE = 50.0f;

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

// True while the bot carries enough Blackout Brew stacks that standing still for a few more seconds stuns it.
bool NeedsBlackoutStep(Player* bot);

// #690 Carbonation: a Fizzy Bubble is up (they only exist for the 12 s after the cast starts) or the bot is flying under
// the Fizzy aura but still low. True = the bot should click a bubble / climb.
bool NeedsFizzyFlight(Player* bot);

// The Fizzy Bubble this bot should click: the nearest live one, or nullptr.
Unit* FindFizzyBubble(Player* bot);

// #690 Wall of Suds: the bot has Sudsy and a Wall of Suds is within WALL_JUMP_DIST (the wall to hop, or nullptr).
Unit* FindWallToJump(Player* bot);

// #690 Bubble Shield: a ranged non-healer bot with a bubble to pop (nearest one, or nullptr). Melee cannot reach 8 yd up.
Unit* FindBubbleShieldTarget(Player* bot);

// The add the whole group should kill first (nearest lit/charging Explosive Hopper, else a Yeasty Brew Alemental), or
// nullptr. One tracker bot marks it with the skull icon, which every bot's target selection ranks first.
Unit* GetPriorityMarkTarget(Player* bot);
}

#endif
