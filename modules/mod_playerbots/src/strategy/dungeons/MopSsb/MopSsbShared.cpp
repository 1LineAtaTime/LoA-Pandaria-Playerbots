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

#include "MopSsbShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

namespace MopSsb
{
namespace
{
// The living, hostile creature of `entry` nearest to the bot (a direct grid search, so friendly-faction stalkers and
// triggers that the "nearest hostile npcs" value skips are not an issue).
Creature* NearestAliveHostile(Player* bot, uint32 entry, float range)
{
    std::list<Creature*> found;
    bot->GetCreatureListWithEntryInGrid(found, entry, range);

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* creature : found)
    {
        if (!creature || !creature->IsAlive() || !creature->IsHostileTo(bot))
            continue;

        float const dist = bot->GetExactDist2d(creature->GetPositionX(), creature->GetPositionY());
        if (!best || dist < bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

// Ook-Ook: Ground Pound is channeled with the boss locked on the victim (PrepareChanneledCast), so the cone stays put
// for the ~4.5 s. Rolling Barrels roll along their facing until they touch a unit and then explode (npc_barrel).
void CollectOokOok(Player* bot, std::vector<Hazard>& out)
{
    std::list<Creature*> barrels;
    bot->GetCreatureListWithEntryInGrid(barrels, NPC_ROLLING_BARREL, BARREL_SCAN_RANGE);
    for (Creature* barrel : barrels)
    {
        if (!barrel || !barrel->IsAlive())
            continue;

        out.push_back(Hazard::MakeLane(barrel->GetPositionX(), barrel->GetPositionY(), barrel->GetOrientation(),
                                       BARREL_HALF_WIDTH, BARREL_FORWARD, BARREL_BACK));
    }

    Creature* ook = bot->FindNearestCreature(NPC_OOK_OOK, SCAN_RANGE, true);
    if (!ook)
        return;

    if (ook->HasAura(SPELL_GROUND_POUND) ||
        (ook->HasUnitState(UNIT_STATE_CASTING) && ook->FindCurrentSpellBySpellId(SPELL_GROUND_POUND)))
    {
        out.push_back(Hazard::MakeCone(ook->GetPositionX(), ook->GetPositionY(), ook->GetOrientation(),
                                       GROUND_POUND_HALF_ANGLE, GROUND_POUND_LENGTH));
    }
}

// Hoptallus: Furlwind is a self-centred 8 yd spin that the boss cannot melee through, Carrot Breath a slowly turning
// 20 degree cone, and every Explosive Hopper blows up 5 s after it lights its fuse.
void CollectHoptallus(Player* bot, std::vector<Hazard>& out)
{
    if (Creature* hoptallus = bot->FindNearestCreature(NPC_HOPTALLUS, SCAN_RANGE, true))
    {
        if (hoptallus->HasAura(SPELL_FURLWIND) ||
            (hoptallus->HasUnitState(UNIT_STATE_CASTING) && hoptallus->FindCurrentSpellBySpellId(SPELL_FURLWIND)))
        {
            out.push_back(Hazard::MakeCircle(hoptallus->GetPositionX(), hoptallus->GetPositionY(), FURLWIND_RADIUS));
        }

        if (hoptallus->HasAura(SPELL_CARROT_BREATH) ||
            (hoptallus->HasUnitState(UNIT_STATE_CASTING) && hoptallus->FindCurrentSpellBySpellId(SPELL_CARROT_BREATH)))
        {
            out.push_back(Hazard::MakeCone(hoptallus->GetPositionX(), hoptallus->GetPositionY(),
                                           hoptallus->GetOrientation(), CARROT_HALF_ANGLE, CARROT_LENGTH));
        }
    }

    std::list<Creature*> hoppers;
    bot->GetCreatureListWithEntryInGrid(hoppers, NPC_EXPLOSIVE_HOPPER, MARK_SCAN_RANGE);
    for (Creature* hopper : hoppers)
    {
        if (!hopper || !hopper->IsAlive())
            continue;

        // The script lights the fuse with a triggered aura and then starts the 5 s Explosive Brew cast.
        if (hopper->HasAura(SPELL_LIT_FUSE) || hopper->FindCurrentSpellBySpellId(SPELL_EXPLOSIVE_BREW))
        {
            out.push_back(Hazard::MakeCircle(hopper->GetPositionX(), hopper->GetPositionY(), HOPPER_BLAST_RADIUS));
        }
    }
}

// Yan-Zhu: the two stalkers riding a Bloated player each breathe Gushing Brew along their own facing (the core's cone
// check uses the stalker's orientation, so this mirrors it exactly). The bloated bot's own stalkers are skipped.
void CollectBloat(Player* bot, std::vector<Hazard>& out)
{
    std::list<Creature*> stalkers;
    bot->GetCreatureListWithEntryInGrid(stalkers, NPC_BLOATED_STALKER, STALKER_SCAN_RANGE);
    for (Creature* stalker : stalkers)
    {
        if (!stalker || !stalker->IsAlive() || !stalker->HasAura(SPELL_GUSHING_BREW) ||
            stalker->GetVehicleBase() == bot)
        {
            continue;
        }

        out.push_back(Hazard::MakeCone(stalker->GetPositionX(), stalker->GetPositionY(), stalker->GetOrientation(),
                                       BLOAT_HALF_ANGLE, BLOAT_LENGTH));
    }
}
}

void CollectHazards(Player* bot, std::vector<Hazard>& out)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB || bot->GetVehicle())
        return;

    CollectOokOok(bot, out);
    CollectHoptallus(bot, out);
    CollectBloat(bot, out);
}

bool NeedsBlackoutStep(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB || bot->HasUnitState(UNIT_STATE_STUNNED))
        return false;

    Aura* aura = bot->GetAura(SPELL_BLACKOUT_BREW);
    if (!aura || aura->GetStackAmount() < BLACKOUT_MOVE_STACKS)
        return false;

    return bot->FindNearestCreature(NPC_YAN_ZHU, SCAN_RANGE, true) != nullptr;
}

Unit* FindFizzyBubble(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB)
        return nullptr;

    std::list<Creature*> bubbles;
    bot->GetCreatureListWithEntryInGrid(bubbles, NPC_FIZZY_BUBBLE, FIZZY_SCAN_RANGE);

    Creature* best = nullptr;
    float bestDist = 0.0f;
    for (Creature* bubble : bubbles)
    {
        if (!bubble || !bubble->IsAlive())
            continue;

        float const dist = bot->GetExactDist(bubble);
        if (!best || dist < bestDist)
        {
            best = bubble;
            bestDist = dist;
        }
    }

    return best;
}

bool NeedsFizzyFlight(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB || bot->GetVehicle() || bot->HasUnitState(UNIT_STATE_STUNNED))
        return false;

    Creature* boss = bot->FindNearestCreature(NPC_YAN_ZHU, SCAN_RANGE, true);
    if (!boss)
        return false;

    if (bot->HasAura(SPELL_FIZZY_FLIGHT))
        return bot->GetPositionZ() < boss->GetPositionZ() + FLIGHT_ABOVE_BOSS - 1.5f;

    return FindFizzyBubble(bot) != nullptr;
}

Unit* FindWallToJump(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB || bot->GetVehicle() || !bot->HasAura(SPELL_SUDSY))
        return nullptr;

    return NearestAliveHostile(bot, NPC_WALL_OF_SUDS, WALL_JUMP_DIST);
}

Unit* FindBubbleShieldTarget(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SSB || bot->GetVehicle() || !bot->IsInCombat() ||
        PlayerBotSpec::IsHeal(bot) || !PlayerBotSpec::IsRanged(bot))
    {
        return nullptr;
    }

    Creature* boss = bot->FindNearestCreature(NPC_YAN_ZHU, SCAN_RANGE, true);
    if (!boss || !boss->HasAura(SPELL_BUBBLE_SHIELD))
        return nullptr;

    return NearestAliveHostile(bot, NPC_BUBBLE_SHIELD, BUBBLE_ATTACK_RANGE);
}

Unit* GetPriorityMarkTarget(Player* bot)
{
    if (!bot || bot->GetMapId() != MAP_SSB)
        return nullptr;

    // Hoptallus: a Hopper that reaches its target blows up for 30k in 10 yd (knock down) - kill them first.
    if (Creature* hopper = NearestAliveHostile(bot, NPC_EXPLOSIVE_HOPPER, MARK_SCAN_RANGE))
        return hopper;

    // Yan-Zhu: Yeasty Brew Alementals feed him with Ferment ("kill them asap", Wowpedia).
    return NearestAliveHostile(bot, NPC_YEASTY_ALEMENTAL, MARK_SCAN_RANGE);
}

bool IsInDanger(Player* bot)
{
    std::vector<Hazard> hazards;
    CollectHazards(bot, hazards);
    return !hazards.empty() && InAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), DANGER_GRACE);
}

bool CanEscapeDanger(Player* bot)
{
    std::vector<Hazard> hazards;
    CollectHazards(bot, hazards);
    if (hazards.empty() || !InAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), DANGER_GRACE))
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return FindSafePoint(bot, hazards, x, y, z);
}

bool FindSafePoint(Player* bot, std::vector<Hazard> const& hazards, float& outX, float& outY, float& outZ)
{
    struct Candidate
    {
        float x;
        float y;
        float radius;
        float cost;
    };

    static constexpr float RADII[] = { 3.0f, 5.0f, 7.5f, 10.0f, 14.0f, 19.0f };
    static constexpr int BEARINGS = 16;
    static constexpr int MAX_WALK_CHECKS = 12;

    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();

    // Among equally distant points prefer the one nearest the main tank, who holds the boss in place.
    float anchorX = botX;
    float anchorY = botY;
    if (Player* tank = EncounterHelpers::GetGroupMainTank(bot))
    {
        anchorX = tank->GetPositionX();
        anchorY = tank->GetPositionY();
    }

    std::vector<Candidate> candidates;
    for (float const radius : RADII)
    {
        for (int i = 0; i < BEARINGS; ++i)
        {
            float const angle =
                (2.0f * static_cast<float>(M_PI) * static_cast<float>(i)) / static_cast<float>(BEARINGS);
            float const cx = botX + radius * std::cos(angle);
            float const cy = botY + radius * std::sin(angle);
            if (InAnyHazard(hazards, cx, cy, SAFE_CLEARANCE))
                continue;

            float const ax = cx - anchorX;
            float const ay = cy - anchorY;
            candidates.push_back(Candidate{ cx, cy, radius, radius + 0.25f * std::sqrt(ax * ax + ay * ay) });
        }
    }

    std::sort(candidates.begin(), candidates.end(),
              [](Candidate const& a, Candidate const& b) { return a.cost < b.cost; });

    int checked = 0;
    for (Candidate const& candidate : candidates)
    {
        if (++checked > MAX_WALK_CHECKS)
            break;

        float stepX = 0.0f;
        float stepY = 0.0f;
        float stepZ = 0.0f;
        if (!EncounterHelpers::CanTakeStepTowards(bot, candidate.x, candidate.y, candidate.radius, stepX, stepY,
                                                  stepZ))
        {
            continue;
        }

        // The helper can land short of the request: the landing spot still has to be outside every hazard.
        if (InAnyHazard(hazards, stepX, stepY, SAFE_CLEARANCE * 0.5f))
            continue;

        outX = stepX;
        outY = stepY;
        outZ = stepZ;
        return true;
    }

    return false;
}
}
