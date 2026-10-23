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
//                     #780: Ball of Fire spheres (59225): avoid the lane ahead of each one
//   Sha of Violence   none: Sha Spike / Disorienting Smash / Smoke Blades are instant and untelegraphed on this core
//   Taran Zhu         Ring of Malice (damages 8.5-15 yd from him), Haze of Hate -> Meditate, skull on Gripping Hatred

#include "MopSpmShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

namespace MopSpm
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

// Gu Cloudstrike: Static Field leaves a stalker with a 9 yd damage aura for 15 s; Invoke Lightning hits a random player
// and everybody within 5 yd of them, so the whole group spreads while Gu is casting it; the Azure Serpent's
// Lightning Breath is a 45 degree cone that reaches 80 yd.
void CollectGuCloudstrike(Player* bot, std::vector<Hazard>& out)
{
    std::list<Creature*> fields;
    bot->GetCreatureListWithEntryInGrid(fields, NPC_STATIC_FIELD, NEAR_SCAN_RANGE);
    for (Creature* field : fields)
    {
        if (field && field->IsAlive() && field->HasAura(SPELL_STATIC_FIELD_AURA))
            out.push_back(Hazard::MakeCircle(field->GetPositionX(), field->GetPositionY(), STATIC_FIELD_RADIUS));
    }

    if (Creature* gu = bot->FindNearestCreature(NPC_GU_CLOUDSTRIKE, SCAN_RANGE, true))
    {
        if (gu->HasUnitState(UNIT_STATE_CASTING) && gu->FindCurrentSpellBySpellId(SPELL_INVOKE_LIGHTNING))
        {
            if (Group* group = bot->GetGroup())
            {
                for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
                {
                    Player* member = ref->GetSource();
                    if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
                        continue;

                    out.push_back(Hazard::MakeCircle(member->GetPositionX(), member->GetPositionY(),
                                                     INVOKE_LIGHTNING_RADIUS));
                }
            }
        }
    }

    if (Creature* serpent = bot->FindNearestCreature(NPC_AZURE_SERPENT, SCAN_RANGE, true))
    {
        if (serpent->HasAura(SPELL_LIGHTNING_BREATH) ||
            (serpent->HasUnitState(UNIT_STATE_CASTING) && serpent->FindCurrentSpellBySpellId(SPELL_LIGHTNING_BREATH)))
        {
            out.push_back(Hazard::MakeCone(serpent->GetPositionX(), serpent->GetPositionY(),
                                           serpent->GetOrientation(), LIGHTNING_BREATH_HALF_ANGLE,
                                           LIGHTNING_BREATH_LENGTH));
        }
    }
}

// Master Snowdrift: Fists of Fury is channeled with the boss locked on his victim (PrepareChanneledCast) and hits a
// 60 degree cone for 10 yd; Tornado Kick hits everyone within 8 yd; the player he fixates with Chase Down runs.
// Flying Snow (the first senior student) whirls for 3 yd around herself.
void CollectMasterSnowdrift(Player* bot, std::vector<Hazard>& out)
{
    if (Creature* boss = bot->FindNearestCreature(NPC_MASTER_SNOWDRIFT, SCAN_RANGE, true))
    {
        if (boss->HasAura(SPELL_FISTS_OF_FURY) ||
            (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_FISTS_OF_FURY)))
        {
            out.push_back(Hazard::MakeCone(boss->GetPositionX(), boss->GetPositionY(), boss->GetOrientation(),
                                           FISTS_HALF_ANGLE, FISTS_LENGTH));
        }

        if (boss->HasAura(SPELL_TORNADO_KICK) ||
            (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_TORNADO_KICK)))
        {
            out.push_back(Hazard::MakeCircle(boss->GetPositionX(), boss->GetPositionY(), TORNADO_KICK_RADIUS));
        }

        // The fixated player is chased by the boss: keep running (he slams only within 1.5 yd).
        if (bot->HasAura(SPELL_CHASE_DOWN))
            out.push_back(Hazard::MakeCircle(boss->GetPositionX(), boss->GetPositionY(), CHASE_DOWN_KITE_RADIUS));
    }

    std::list<Creature*> students;
    bot->GetCreatureListWithEntryInGrid(students, NPC_FLYING_SNOW, NEAR_SCAN_RANGE);
    for (Creature* student : students)
    {
        if (student && student->IsAlive() && student->HasAura(SPELL_WHIRLING_STEEL))
        {
            out.push_back(Hazard::MakeCircle(student->GetPositionX(), student->GetPositionY(),
                                             WHIRLING_STEEL_RADIUS));
        }
    }

    // #780: Ball of Fire (Wowpedia Master_Snowdrift_(tactics), read 2026-10-14: "Fragrant Lotus summons damaging spheres that are
    // also to be avoided"; stage 2 "Ball of Fire inflicts Fire damage to all players in its path"). Each sphere flies straight on
    // along its own facing, so the strip ahead of it is the hazard; a sphere that has stopped is a small circle.
    std::list<Creature*> spheres;
    bot->GetCreatureListWithEntryInGrid(spheres, NPC_BALL_OF_FIRE, NEAR_SCAN_RANGE);
    for (Creature* sphere : spheres)
    {
        if (!sphere || !sphere->IsAlive())
            continue;

        if (sphere->isMoving())
            out.push_back(Hazard::MakeLane(sphere->GetPositionX(), sphere->GetPositionY(), sphere->GetOrientation(),
                                           BALL_OF_FIRE_HALF_WIDTH, BALL_OF_FIRE_AHEAD, BALL_OF_FIRE_BEHIND));
        else
            out.push_back(Hazard::MakeCircle(sphere->GetPositionX(), sphere->GetPositionY(), BALL_OF_FIRE_HALF_WIDTH));
    }
}

// Taran Zhu: Ring of Malice damages everything between 8.5 and 15 yd from him (the fork's script drops targets within
// 8.5 yd), so a bot in that band moves in (melee, tank) or out (ranged, healers).
void CollectTaranZhu(Player* bot, std::vector<Hazard>& out)
{
    Creature* taran = bot->FindNearestCreature(NPC_TARAN_ZHU, SCAN_RANGE, true);
    if (taran && taran->HasAura(SPELL_RING_OF_MALICE))
        out.push_back(Hazard::MakeRing(taran->GetPositionX(), taran->GetPositionY(), RING_INNER, RING_OUTER));
}
}

void CollectHazards(Player* bot, std::vector<Hazard>& out)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_SPM || bot->GetVehicle())
        return;

    CollectGuCloudstrike(bot, out);
    CollectMasterSnowdrift(bot, out);
    CollectTaranZhu(bot, out);
}

bool IsParryStanceActive(Player* bot)
{
    if (!bot || bot->GetMapId() != MAP_SPM)
        return false;

    Creature* boss = bot->FindNearestCreature(NPC_MASTER_SNOWDRIFT, SCAN_RANGE, true);
    if (!boss)
        return false;

    return boss->HasAura(SPELL_PARRY_STANCE) || boss->HasAura(SPELL_PARRY_STANCE_TRIGGERED) ||
           (boss->HasUnitState(UNIT_STATE_CASTING) && boss->FindCurrentSpellBySpellId(SPELL_PARRY_STANCE));
}

bool NeedsMeditate(Player* bot)
{
    return bot && bot->IsAlive() && bot->GetMapId() == MAP_SPM && bot->HasAura(SPELL_HAZE_OF_HATE) &&
           !bot->HasAura(SPELL_MEDITATE);
}

Unit* GetPriorityMarkTarget(Player* bot)
{
    if (!bot || bot->GetMapId() != MAP_SPM)
        return nullptr;

    // Taran Zhu: each Gripping Hatred pulls the whole group to it 10 s after it appears unless it dies first.
    if (Creature* grip = NearestAliveHostile(bot, NPC_GRIPPING_HATRED, MARK_SCAN_RANGE))
        return grip;

    // Gu Cloudstrike, phase 2: Gu cannot be hurt, the Azure Serpent (landed, stationary) is the fight.
    Creature* gu = bot->FindNearestCreature(NPC_GU_CLOUDSTRIKE, SCAN_RANGE, true);
    if (gu && gu->HasAura(SPELL_CHARGING_SOUL))
    {
        Creature* serpent = NearestAliveHostile(bot, NPC_AZURE_SERPENT, SCAN_RANGE);
        if (serpent && serpent->IsInCombat() && !serpent->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
            return serpent;
    }

    return nullptr;
}

void CollectExclusions(Player* bot, GuidSet& out)
{
    Creature* gu = bot->FindNearestCreature(NPC_GU_CLOUDSTRIKE, SCAN_RANGE, true);
    if (!gu || !gu->HasAura(SPELL_CHARGING_SOUL))
        return;

    // Only drop Gu once the serpent can really be fought, so a bot is never left without a target.
    Creature* serpent = NearestAliveHostile(bot, NPC_AZURE_SERPENT, SCAN_RANGE);
    if (serpent && serpent->IsInCombat() && !serpent->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        out.insert(gu->GetGUID());
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
