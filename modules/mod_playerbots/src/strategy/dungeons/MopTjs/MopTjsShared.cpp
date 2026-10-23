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

#include "MopTjsShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "Playerbots.h"
#include "SpellAuras.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <vector>

namespace MopTjs
{
namespace
{
// Sha Residue (106653) puddles: the dying Corrupt Living Waters leave them and so do the trash waters before the boss.
void CollectShaResidue(Player* bot, std::vector<Hazard>& out)
{
    for (Position const& pos : EncounterHelpers::GetDynamicObjectPositions(bot, DYNOBJ_SCAN_RANGE, SPELL_SHA_RESIDUE))
        out.push_back(Hazard::MakeCircle(pos.GetPositionX(), pos.GetPositionY(), SHA_RESIDUE_RADIUS));
}

void CollectWiseMari(Player* bot, std::vector<Hazard>& out)
{
    Creature* mari = bot->FindNearestCreature(NPC_WISE_MARI, SCAN_RANGE, true);
    if (!mari)
        return;

    float const marix = mari->GetPositionX();
    float const mariy = mari->GetPositionY();
    float const facing = mari->GetOrientation();

    // Hydrolance: Mari turns to one of three directions, casts 106055 (4 s) and then a line of 3.5 yd pulses runs
    // away along her facing (spell_wise_hydrolance_pulse, boss_wise_mari.cpp). The lane is live from the cast start.
    if (mari->HasAura(SPELL_HYDROLANCE_AURA) ||
        (mari->HasUnitState(UNIT_STATE_CASTING) && mari->FindCurrentSpellBySpellId(SPELL_HYDROLANCE_CAST)))
    {
        float const startX = marix + HYDROLANCE_START * std::cos(facing);
        float const startY = mariy + HYDROLANCE_START * std::sin(facing);
        out.push_back(Hazard::MakeLane(startX, startY, facing, HYDROLANCE_HALF_WIDTH, HYDROLANCE_LENGTH, 0.0f));
    }

    // #816 Corrupted Waters: the fountain water damages a player whose z is in the water band, inside the half discs the
    // script tests (boss_wise_mari.cpp:731-760). Live for the whole fight (the stalker casts it on combat start). A bot on the
    // dry rim or the centre platform is outside the z window and not in danger.
    if (mari->IsInCombat())
    {
        float const z = bot->GetPositionZ();
        if (z > CORRUPTED_Z_MIN && z < CORRUPTED_Z_MAX)
        {
            out.push_back(Hazard::MakeHalfDisc(MARI_ROOM_X, MARI_ROOM_Y, MARI_ROOM_FACING, CORRUPTED_FRONT_RADIUS, true));
            out.push_back(Hazard::MakeHalfDisc(MARI_ROOM_X, MARI_ROOM_Y, MARI_ROOM_FACING, CORRUPTED_BACK_RADIUS, false));
        }
        else if (z > CORRUPTED_POOL_Z_MIN && z < CORRUPTED_POOL_Z_MAX)
            out.push_back(Hazard::MakeCircle(MARI_ROOM_X, MARI_ROOM_Y, CORRUPTED_POOL_RADIUS));
    }

    // Wash Away (phase 2): a 12 degree beam that turns with Mari. Players above WASH_AWAY_SAFE_Z are dropped from the
    // target list by the fork's script (spell_tjs_wash_away_proc); everyone else keeps just ahead of the beam.
    if (mari->HasAura(SPELL_WASH_AWAY) && bot->GetPositionZ() <= WASH_AWAY_SAFE_Z)
        out.push_back(Hazard::MakeCone(marix, mariy, facing, WASH_AWAY_HALF_ANGLE, WASH_AWAY_LENGTH));
}

// Liu Flameheart: every Serpent Wave / Jade Serpent Wave is a Dragon Wave trigger that waits 1.5 s and then runs
// 100 yd along its facing, damaging everything within 3 yd (spells 106938 / 107053, boss_liu_flameheart.cpp).
void CollectLiuWaves(Player* bot, std::vector<Hazard>& out)
{
    std::list<Creature*> waves;
    bot->GetCreatureListWithEntryInGrid(waves, NPC_DRAGON_WAVE, WAVE_SCAN_RANGE);
    for (Creature* wave : waves)
    {
        if (!wave || !wave->IsAlive())
            continue;

        out.push_back(Hazard::MakeLane(wave->GetPositionX(), wave->GetPositionY(), wave->GetOrientation(),
                                       WAVE_HALF_WIDTH, WAVE_FORWARD, WAVE_BACK));
    }
}

// Sha of Doubt: Touch of Nothingness (106113) ticks 106228 on the carrier and every ally within 6 yd. The carrier
// walks away from the group; everyone else keeps clear of the carrier.
void CollectTouchOfNothingness(Player* bot, std::vector<Hazard>& out)
{
    Group* group = bot->GetGroup();
    if (!group)
        return;

    bool anyCarrier = bot->HasAura(SPELL_TOUCH_OF_NOTHINGNESS);
    if (!anyCarrier)
    {
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            if (member && member != bot && member->HasAura(SPELL_TOUCH_OF_NOTHINGNESS))
            {
                anyCarrier = true;
                break;
            }
        }
    }

    if (!anyCarrier || !bot->FindNearestCreature(NPC_SHA_OF_DOUBT, SCAN_RANGE, true))
        return;

    bool const selfCarrier = bot->HasAura(SPELL_TOUCH_OF_NOTHINGNESS);
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (selfCarrier || member->HasAura(SPELL_TOUCH_OF_NOTHINGNESS))
            out.push_back(Hazard::MakeCircle(member->GetPositionX(), member->GetPositionY(), TOUCH_RADIUS));
    }
}

uint32 IntensityStacks(Creature* creature)
{
    Aura* aura = creature->GetAura(SPELL_INTENSITY);
    return aura ? aura->GetStackAmount() : 0;
}
}

void CollectHazards(Player* bot, std::vector<Hazard>& out)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_TJS || bot->GetVehicle())
        return;

    CollectShaResidue(bot, out);
    CollectWiseMari(bot, out);
    CollectLiuWaves(bot, out);
    CollectTouchOfNothingness(bot, out);
}

void CollectExclusions(Player* bot, bool tankType, GuidSet& out)
{
    // Wise Mari is immune (and knocks players back) while the Water Bubble is up: the bots deal with the Corrupt
    // Living Waters and their droplets, which attack the group on their own.
    if (Creature* mari = bot->FindNearestCreature(NPC_WISE_MARI, SCAN_RANGE, true))
    {
        if (mari->HasAura(SPELL_WATER_BUBBLE))
            out.insert(mari->GetGUID());
    }

    // Sha of Doubt is immune for as long as a Figment of Doubt lives (Bounds of Reality); every group member has
    // a figment of their own that fights them.
    if (Creature* sha = bot->FindNearestCreature(NPC_SHA_OF_DOUBT, SCAN_RANGE, true))
    {
        if (sha->HasAura(SPELL_BOUNDS_OF_REALITY))
            out.insert(sha->GetGUID());
    }

    // The tank keeps both Strife and Peril on him; the DPS balance their damage (Wowpedia: "periodically switching
    // between the bosses"). Focusing one stacks Intensity on it, ten stacks give 15 s of Ultimate Power (immune).
    if (tankType)
        return;

    Creature* strife = bot->FindNearestCreature(NPC_TRIAL_STRIFE, SCAN_RANGE, true);
    Creature* peril = bot->FindNearestCreature(NPC_TRIAL_PERIL, SCAN_RANGE, true);
    bool const strifeImmune = strife && strife->HasAura(SPELL_ULTIMATE_POWER);
    bool const perilImmune = peril && peril->HasAura(SPELL_ULTIMATE_POWER);

    if (strifeImmune)
        out.insert(strife->GetGUID());
    if (perilImmune)
        out.insert(peril->GetGUID());

    if (strife && peril && !strifeImmune && !perilImmune)
    {
        uint32 const strifeStacks = IntensityStacks(strife);
        uint32 const perilStacks = IntensityStacks(peril);
        if (strifeStacks >= INTENSITY_SWITCH_STACKS && strifeStacks > perilStacks)
            out.insert(strife->GetGUID());
        else if (perilStacks >= INTENSITY_SWITCH_STACKS && perilStacks > strifeStacks)
            out.insert(peril->GetGUID());
    }
}

// #815: the waters must die (Wowpedia "Wise Mari", MediaWiki API 2026-10-14: "After the players defeat four Corrupt
// Living Waters, Wise Mari unleashes all of his remaining power"), but the generic "attackers" value only lists a unit
// the bot can see (AttackersValue::IsValidTarget = IsPossibleTarget + IsWithinLOSInMap). w54: the 4th water (fountain 3,
// 1075.2,-2561.3, the only fountain across the x=1066.67 navmesh tile seam) took the healer as victim and never reached
// her: an incomplete chase path is not "cannot reach" (ChaseMovementGenerator.cpp:194 masks out PATHFIND_INCOMPLETE),
// so the add parks at the end of the partial path (:142) and never evades. It stayed in the healer's attacker list and
// out of every bot's "attackers" for 270 s: "T:no attackers", Mari kept the bubble, no Bubble Burst (106612 cast=0). AC AttackFrostTombAction
// (UKActions.cpp:11-31) finds its add the same way, from "possible targets no los" instead of the threat lists.
Unit* FindLivingWater(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_TJS || bot->GetVehicle() ||
        !MopDungeon::WaveRoleMatches(bot, MopDungeon::WaveRole::All))
        return nullptr;

    Creature* mari = bot->FindNearestCreature(NPC_WISE_MARI, SCAN_RANGE, true);
    if (!mari || !mari->IsInCombat() || !mari->HasAura(SPELL_WATER_BUBBLE))
        return nullptr;

    for (uint32 const entry : { NPC_CORRUPT_LIVING_WATER, NPC_CORRUPT_DROPLET })
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, LIVING_WATER_RANGE);

        Unit* best = nullptr;
        float bestDist = LIVING_WATER_RANGE + 1.0f;
        for (Creature* add : found)
        {
            if (!add || !add->IsAlive() || !AttackersValue::IsPossibleTarget(add, bot))
                continue;

            float const dist = bot->GetExactDist2d(add);
            if (dist < bestDist)
            {
                best = add;
                bestDist = dist;
            }
        }

        if (best)
            return best;
    }

    return nullptr;
}

// #651 Lorewalker Stonestep (Wowpedia "Lorewalker Stonestep (tactics)", MediaWiki API 2026-10-14: "Players must extinguish
// the suns from the sky. Defeating each Sun causes Haunting Sha to appear ... The Haunting Sha have now possessed Zao
// Sunseeker. You must now defeat him"; Trial: "periodically switching between the bosses when the stacks get too high").
// AC AttackFrostTombAction (UKActions.cpp:11-31) finds its add by entry outside the threat lists the same way.
bool IsLorewalkerEnemy(Unit* unit)
{
    if (!unit || !unit->IsAlive())
        return false;

    switch (unit->GetEntry())
    {
        case NPC_LOREWALKER_SUN:
        case NPC_HAUNTING_SHA:
        case NPC_ZAO_SUNSEEKER:
        case NPC_TRIAL_STRIFE:
        case NPC_TRIAL_PERIL:
            return true;
        default:
            return false;
    }
}

Unit* FindLorewalkerTarget(Player* bot)
{
    if (!bot || !bot->IsAlive() || bot->GetMapId() != MAP_TJS || bot->GetVehicle() ||
        !MopDungeon::WaveRoleMatches(bot, MopDungeon::WaveRole::All))
        return nullptr;

    // Stage two of the Suns tale: Zao is a valid target only once corrupted (faction 14, selectable).
    if (Creature* zao = bot->FindNearestCreature(NPC_ZAO_SUNSEEKER, LOREWALKER_RANGE, true))
    {
        if (AttackersValue::IsPossibleTarget(zao, bot))
            return zao;
    }

    // Trial of the Yaungol: both are stationary; take the one with fewer Intensity stacks (Ultimate Power makes a unit
    // immune, which IsPossibleTarget already rejects). Before the unflag (EVENT_STRIFE_3) both are NON_ATTACKABLE.
    Creature* strife = bot->FindNearestCreature(NPC_TRIAL_STRIFE, LOREWALKER_RANGE, true);
    Creature* peril = bot->FindNearestCreature(NPC_TRIAL_PERIL, LOREWALKER_RANGE, true);
    bool const strifeOk = strife && AttackersValue::IsPossibleTarget(strife, bot);
    bool const perilOk = peril && AttackersValue::IsPossibleTarget(peril, bot);
    if (strifeOk && perilOk)
        return IntensityStacks(strife) <= IntensityStacks(peril) ? static_cast<Unit*>(strife) : static_cast<Unit*>(peril);
    if (strifeOk)
        return strife;
    if (perilOk)
        return peril;

    // Stage one of the Suns tale: a live Haunting Sha attacks the group by itself (npc_haunting_sha UpdateAI), so the
    // generic target values hold it; the next Sun waits until it is dead.
    if (Creature* sha = bot->FindNearestCreature(NPC_HAUNTING_SHA, LOREWALKER_RANGE, true))
    {
        if (AttackersValue::IsPossibleTarget(sha, bot))
            return nullptr;
    }

    std::list<Creature*> suns;
    bot->GetCreatureListWithEntryInGrid(suns, NPC_LOREWALKER_SUN, LOREWALKER_RANGE);
    Unit* best = nullptr;
    float bestDist = LOREWALKER_RANGE + 1.0f;
    for (Creature* sun : suns)
    {
        if (!sun || !sun->IsAlive() || !AttackersValue::IsPossibleTarget(sun, bot))
            continue;

        float const dist = bot->GetExactDist2d(sun);
        if (dist < bestDist)
        {
            best = sun;
            bestDist = dist;
        }
    }

    return best;
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
