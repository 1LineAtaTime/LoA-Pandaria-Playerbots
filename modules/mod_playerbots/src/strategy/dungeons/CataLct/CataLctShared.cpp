/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources.

#include "CataLctShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <vector>

namespace CataLct
{
// ---------------------------------------------------------------------------------------------------------------
// Generic helpers
// ---------------------------------------------------------------------------------------------------------------
std::vector<Creature*> FindCreatures(Player* bot, std::initializer_list<uint32> entries, float range)
{
    std::vector<Creature*> out;
    for (uint32 const entry : entries)
    {
        std::list<Creature*> found;
        bot->GetCreatureListWithEntryInGrid(found, entry, range);
        for (Creature* creature : found)
        {
            if (creature && creature->IsAlive())
                out.push_back(creature);
        }
    }

    return out;
}

Creature* NearestCreature(Player* bot, std::initializer_list<uint32> entries, float range, float maxDist)
{
    Creature* best = nullptr;
    float bestDist = maxDist;
    for (Creature* creature : FindCreatures(bot, entries, range))
    {
        float const dist = bot->GetExactDist2d(creature);
        if (dist <= bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

Spell* FindCast(Unit* caster, std::initializer_list<uint32> spellIds)
{
    if (!caster)
        return nullptr;

    for (uint32 const spellId : spellIds)
    {
        if (Spell* spell = caster->FindCurrentSpellBySpellId(spellId))
            return spell;
    }

    return nullptr;
}

bool IsCastingOrAura(Unit* unit, std::initializer_list<uint32> spellIds)
{
    if (!unit)
        return false;

    if (FindCast(unit, spellIds))
        return true;

    for (uint32 const spellId : spellIds)
    {
        if (unit->HasAura(spellId))
            return true;
    }

    return false;
}

namespace
{
// Absolute angle between two headings, 0..pi
float AngleDelta(float a, float b)
{
    float delta = std::fabs(Position::NormalizeOrientation(a - b));
    if (delta > float(M_PI))
        delta = 2.0f * float(M_PI) - delta;

    return delta;
}
}

bool InCone(Unit* source, Unit* unit, float halfAngle, float maxDist, bool rear)
{
    if (!source || !unit || source->GetExactDist2d(unit) > maxDist)
        return false;

    float facing = source->GetOrientation();
    if (rear)
        facing += float(M_PI);

    return AngleDelta(source->GetAngle(unit), facing) <= halfAngle;
}

bool LateralPoint(Player* bot, Unit* source, float& x, float& y, float& z)
{
    float const facing = source->GetOrientation();
    float const toBot = Position::NormalizeOrientation(source->GetAngle(bot) - facing);
    float const side = toBot < float(M_PI) ? 1.0f : -1.0f;  // 1 = left of the facing, -1 = right
    float const dist = std::max(6.0f, std::min(bot->GetExactDist2d(source), 14.0f));
    float const angle = facing + side * LATERAL_ANGLE;

    x = source->GetPositionX() + std::cos(angle) * dist;
    y = source->GetPositionY() + std::sin(angle) * dist;
    z = bot->GetPositionZ();

    // 5.4.8: Map::CheckCollisionAndGetValidCoords signature identical to the UB use (src/server/game/Maps/Map.h:662)
    return bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                          bot->GetPositionZ(), x, y, z);
}

Unit* PickFocus(PlayerbotAI* botAI, Player* bot, std::initializer_list<uint32> priority, float maxDist)
{
    size_t const none = priority.size();
    auto const rankOf = [&](Unit* unit) -> size_t
    {
        size_t rank = 0;
        for (uint32 const entry : priority)
        {
            if (unit->GetEntry() == entry)
                return rank;

            ++rank;
        }

        return none;
    };

    AiObjectContext* context = botAI->GetAiObjectContext();
    GuidVector const targets = context->GetValue<GuidVector>("possible targets")->Get();

    Unit* best = nullptr;
    size_t bestRank = none;
    float bestDist = maxDist;
    for (ObjectGuid const guid : targets)
    {
        Unit* unit = botAI->GetUnit(guid);
        if (!unit || !unit->IsAlive())
            continue;

        size_t const rank = rankOf(unit);
        if (rank == none)
            continue;

        float const dist = bot->GetExactDist2d(unit);
        if (dist > maxDist)
            continue;

        if (rank < bestRank || (rank == bestRank && dist < bestDist))
        {
            best = unit;
            bestRank = rank;
            bestDist = dist;
        }
    }

    if (!best)
        return nullptr;

    Unit* current = context->GetValue<Unit*>("current target")->Get();
    if (current && current->IsAlive() && rankOf(current) <= bestRank)
        return nullptr;

    return best;
}

Unit* InterruptCaster(Player* bot, std::initializer_list<uint32> casterEntries, std::initializer_list<uint32> spellIds,
                      float range)
{
    Unit* best = nullptr;
    float bestDist = range;
    for (Creature* creature : FindCreatures(bot, casterEntries, range))
    {
        if (!FindCast(creature, spellIds))
            continue;

        float const dist = bot->GetExactDist2d(creature);
        if (dist <= bestDist)
        {
            best = creature;
            bestDist = dist;
        }
    }

    return best;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsNonMeleeSpellCasted(true))
        return {};

    // Dedicated interrupts only (the MgT list minus its stun/silence "control" tier), plus the 5.x monk/paladin ones
    static char const* const interrupts[] = { "kick",         "pummel",       "wind shear",         "mind freeze",
                                              "counterspell", "spell lock",   "rebuke",             "spear hand strike",
                                              "skull bash" };

    for (char const* interrupt : interrupts)
    {
        std::string const name = interrupt;

        // unknown to this class / not learned => spell id 0 (same guard as MgTShared.cpp:1189)
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId)
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

// ---------------------------------------------------------------------------------------------------------------
// Edges
// ---------------------------------------------------------------------------------------------------------------
bool LandMineDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    for (Creature* mine : FindCreatures(bot, { NPC_LAND_MINE }, SCAN_RANGE))
    {
        float r = 0.0f;
        if (mine->HasAura(SPELL_LAND_MINE_COUNTDOWN))
            r = MINE_BLAST_RADIUS;
        else if (mine->HasAura(SPELL_TOLVIR_LAND_MINE_VISUAL))
            r = MINE_ARMED_RADIUS;

        if (r > 0.0f && bot->GetExactDist2d(mine) <= r)
        {
            pos = mine->GetPosition();
            radius = r + FLEE_MARGIN;
            return true;
        }
    }

    return false;
}

namespace
{
// Signed angle a - b in (-pi, pi]
float SignedDelta(float a, float b)
{
    float delta = Position::NormalizeOrientation(a - b);
    if (delta > float(M_PI))
        delta -= 2.0f * float(M_PI);

    return delta;
}

// Inside a Shockwave line of heading `line` from `origin` (along -half width .. length, across < half width)
bool InShockwaveLine(float ox, float oy, float line, float px, float py)
{
    float const dx = px - ox;
    float const dy = py - oy;
    float const dist = std::sqrt(dx * dx + dy * dy);
    if (dist < 0.1f)
        return true;

    float const delta = SignedDelta(std::atan2(dy, dx), line);
    float const along = dist * std::cos(delta);
    float const across = std::fabs(dist * std::sin(delta));
    return along > -SHOCKWAVE_HALF_WIDTH && along < SHOCKWAVE_LENGTH && across < SHOCKWAVE_HALF_WIDTH;
}
}

bool ShockwaveDanger(PlayerbotAI* /*botAI*/, Player* bot, float& x, float& y, float& z)
{
    if (!bot->IsInCombat() || bot->GetVehicle())
        return false;

    Creature* husam = NearestCreature(bot, { NPC_HUSAM }, 80.0f, 80.0f);
    if (!husam)
        return false;

    float const ox = husam->GetPositionX();
    float const oy = husam->GetPositionY();

    // The line headings: the running stalkers while Husam casts Shockwave (he stands still for the 5 s cast), and every armed
    // visual (the trail outlives the cast; it hits at 4.6 s after it appears, the aura goes at 7 s)
    std::vector<float> lines;
    auto const addLine = [&lines](float heading)
    {
        for (float const known : lines)
        {
            if (std::fabs(SignedDelta(heading, known)) < 0.15f)
                return;
        }

        lines.push_back(heading);
    };

    if (FindCast(husam, { SPELL_SHOCKWAVE }))
    {
        for (Creature* stalker : FindCreatures(bot, { NPC_SHOCKWAVE_STALKER }, 80.0f))
        {
            if (husam->GetExactDist2d(stalker) > 1.0f)
                addLine(husam->GetAngle(stalker));
        }
    }

    bool onVisual = false;
    for (Creature* visual : FindCreatures(bot, { NPC_SHOCKWAVE_VISUAL }, 80.0f))
    {
        if (!visual->HasAura(SPELL_SHOCKWAVE_ARMED))
            continue;

        if (husam->GetExactDist2d(visual) > 2.0f)
            addLine(husam->GetAngle(visual));

        if (bot->GetExactDist2d(visual) < SHOCKWAVE_HALF_WIDTH)
            onVisual = true;
    }

    if (lines.empty())
        return false;

    float const px = bot->GetPositionX();
    float const py = bot->GetPositionY();
    float const dist = husam->GetExactDist2d(bot);
    float const heading = husam->GetAngle(bot);

    // the line the bot is in (smallest angle to it)
    float hitLine = 0.0f;
    float hitDelta = 10.0f;
    for (float const line : lines)
    {
        float const delta = SignedDelta(heading, line);
        if (InShockwaveLine(ox, oy, line, px, py) && std::fabs(delta) < std::fabs(hitDelta))
        {
            hitLine = line;
            hitDelta = delta;
        }
    }

    if (hitDelta > 9.0f && !onVisual)
        return false;

    if (hitDelta > 9.0f)  // on an armed visual whose line heading drifted (Husam moved after the cast): use its nearest line
    {
        for (float const line : lines)
        {
            float const delta = SignedDelta(heading, line);
            if (std::fabs(delta) < std::fabs(hitDelta))
            {
                hitLine = line;
                hitDelta = delta;
            }
        }
    }

    float const side = hitDelta < 0.0f ? -1.0f : 1.0f;
    auto const isClear = [&](float tx, float ty)
    {
        for (float const line : lines)
        {
            if (InShockwaveLine(ox, oy, line, tx, ty))
                return false;
        }

        return true;
    };

    // far out: the shortest way out is across the line; near Husam: the 45 deg gap between two lines
    float tx = 0.0f;
    float ty = 0.0f;
    bool found = false;
    if (dist >= SHOCKWAVE_NEAR_RADIUS + 0.5f)
    {
        float const across = std::fabs(dist * std::sin(hitDelta));
        float const step = std::max(0.5f, SHOCKWAVE_SAFE_OFFSET - across);
        float const perp = hitLine + side * float(M_PI) / 2.0f;
        tx = px + std::cos(perp) * step;
        ty = py + std::sin(perp) * step;
        found = isClear(tx, ty);
    }

    for (float const s : { side, -side })
    {
        if (found)
            break;

        float const gap = hitLine + s * float(M_PI) / 4.0f;
        float const radius = std::max(SHOCKWAVE_NEAR_RADIUS, std::min(dist, 20.0f));
        tx = ox + std::cos(gap) * radius;
        ty = oy + std::sin(gap) * radius;
        found = isClear(tx, ty);
    }

    if (!found)
        return false;

    x = tx;
    y = ty;
    z = bot->GetPositionZ();
    // 5.4.8: Map::CheckCollisionAndGetValidCoords signature identical to the UB use (src/server/game/Maps/Map.h:662)
    bot->GetMap()->CheckCollisionAndGetValidCoords(bot, px, py, bot->GetPositionZ(), x, y, z);
    return true;
}

bool ThrownOnWall(PlayerbotAI* /*botAI*/, Player* bot, float& x, float& y, float& z)
{
    if (!bot->IsInCombat() || bot->GetVehicle())
        return false;

    Creature* husam = NearestCreature(bot, { NPC_HUSAM }, 60.0f, 45.0f);
    if (!husam || !husam->IsInCombat() || bot->GetPositionZ() <= husam->GetPositionZ() + THROWN_HEIGHT)
        return false;

    float const angle = bot->GetAngle(husam);
    x = bot->GetPositionX() + std::cos(angle) * THROWN_STEP;
    y = bot->GetPositionY() + std::sin(angle) * THROWN_STEP;
    z = husam->GetPositionZ();
    return true;
}

bool FellOffPlatform(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsAlive() || bot->GetMapId() != MAP_ID || bot->GetVehicle() || bot->GetPositionZ() >= PLATFORM_FALL_Z)
        return false;

    float const dx = bot->GetPositionX() - SIAMAT_HOME_X;
    float const dy = bot->GetPositionY() - SIAMAT_HOME_Y;
    if (dx * dx + dy * dy > PLATFORM_AREA_RADIUS * PLATFORM_AREA_RADIUS)
        return false;

    // only while the encounter runs: before the release the group may fight trash on the ground below the tower
    Creature* siamat = NearestCreature(bot, { NPC_SIAMAT }, 100.0f, 100.0f);
    return siamat && siamat->IsInCombat();
}

Unit* DustFlailBoss(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat() || PlayerBotSpec::IsTank(bot))
        return nullptr;

    Creature* boss = NearestCreature(bot, { NPC_LOCKMAW }, SCAN_RANGE, SCAN_RANGE);
    if (!boss || !IsCastingOrAura(boss, { SPELL_DUST_FLAIL, SPELL_DUST_FLAIL_AURA }))
        return nullptr;

    return InCone(boss, bot, DUST_FLAIL_REAR_ARC, DUST_FLAIL_RANGE, true) ? boss : nullptr;
}

Unit* CrocoliskFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_FRENZIED_CROCOLISK }, FOCUS_RANGE);
}

bool HeavensFuryDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* fury = NearestCreature(bot, { NPC_HEAVENS_FURY }, SCAN_RANGE, HEAVENS_FURY_RADIUS);
    if (!fury)
        return false;

    pos = fury->GetPosition();
    radius = HEAVENS_FURY_RADIUS + FLEE_MARGIN;
    return true;
}

Unit* SoulFragmentFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_SOUL_FRAGMENT }, FOCUS_RANGE);
}

Unit* HarbingerFocus(PlayerbotAI* botAI, Player* bot)
{
    // tank and damage dealers; a healer keeps healing
    if (!bot->IsInCombat() || (!PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsDps(bot)))
        return nullptr;

    if (PlayerBotSpec::IsTank(bot))
        return PickFocus(botAI, bot, { NPC_HARBINGER_OF_DARKNESS }, FOCUS_RANGE);

    // damage dealers: the same ranking as "lct kill soul fragment" (fragment first), so a bot on a fragment is never pulled
    // back to the Harbinger before the fragment dies (no target ping-pong between the two actions)
    return PickFocus(botAI, bot, { NPC_SOUL_FRAGMENT, NPC_HARBINGER_OF_DARKNESS }, FOCUS_RANGE);
}

bool HallowedGroundDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* barim = NearestCreature(bot, { NPC_BARIM }, SCAN_RANGE, HALLOWED_GROUND_RADIUS);
    if (!barim || !barim->HasAura(SPELL_REPENTANCE_START))
        return false;

    pos = barim->GetPosition();
    radius = HALLOWED_GROUND_RADIUS + FLEE_MARGIN;
    return true;
}

void CollectBlazeExclusions(Player* bot, GuidSet& out, GuidSet& tankOut)
{
    if (!bot->IsInCombat())
        return;

    // Barim alive and fighting; Repentance = his 82320 self aura (immune, the group fights the Harbinger)
    Creature* barim = NearestCreature(bot, { NPC_BARIM }, SCAN_RANGE, SCAN_RANGE);
    if (!barim || !barim->IsInCombat())
        return;

    bool const repentance = barim->HasAura(SPELL_REPENTANCE_START);
    if (!repentance && barim->HasUnitFlag(UNIT_FLAG_NON_ATTACKABLE))
        return;

    for (Creature* blaze : FindCreatures(bot, { NPC_BLAZE_OF_THE_HEAVENS }, SCAN_RANGE))
    {
        out.insert(blaze->GetGUID());
        if (repentance)
            tankOut.insert(blaze->GetGUID());
    }

    // #666 w69: the tank's own pick took Soul Fragments (tank target=43934 with lastact=tank) - they are the damage dealers'
    // job (SoulFragmentFocus), the tank holds the Harbinger
    if (repentance)
        for (Creature* fragment : FindCreatures(bot, { NPC_SOUL_FRAGMENT }, SCAN_RANGE))
            tankOut.insert(fragment->GetGUID());
}

bool BlazeFireDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* fire = NearestCreature(bot, { NPC_BLAZE_FIRE }, SCAN_RANGE, BLAZE_FIRE_RADIUS);
    if (!fire)
        return false;

    pos = fire->GetPosition();
    radius = BLAZE_FIRE_RADIUS + FLEE_MARGIN;
    return true;
}

bool SiamatStormDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    if (Creature* cloud = NearestCreature(bot, { NPC_CLOUD_BURST }, SCAN_RANGE, CLOUD_BURST_RADIUS))
    {
        pos = cloud->GetPosition();
        radius = CLOUD_BURST_RADIUS + FLEE_MARGIN;
        return true;
    }

    if (Creature* storm = NearestCreature(bot, { NPC_TEMPEST_STORM }, SCAN_RANGE, TEMPEST_STORM_RADIUS))
    {
        pos = storm->GetPosition();
        radius = TEMPEST_STORM_RADIUS + FLEE_MARGIN;
        return true;
    }

    return false;
}

Unit* ServantFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    Creature* siamat = NearestCreature(bot, { NPC_SIAMAT }, 80.0f, 80.0f);
    if (!siamat || !siamat->HasAura(SPELL_DEFLECTING_WINDS))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_SERVANT_OF_SIAMAT_1, NPC_SERVANT_OF_SIAMAT_2, NPC_SERVANT_OF_SIAMAT_3 },
                     FOCUS_RANGE);
}

bool LightningChargeDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    for (Creature* servant : FindCreatures(bot, { NPC_SERVANT_OF_SIAMAT_1, NPC_SERVANT_OF_SIAMAT_2, NPC_SERVANT_OF_SIAMAT_3 },
                                           SCAN_RANGE))
    {
        if (servant->GetHealthPct() <= SERVANT_CHARGE_HP_PCT && bot->GetExactDist2d(servant) <= LIGHTNING_CHARGE_RADIUS)
        {
            pos = servant->GetPosition();
            radius = LIGHTNING_CHARGE_RADIUS + FLEE_MARGIN;
            return true;
        }
    }

    return false;
}


bool InDanger(PlayerbotAI* botAI, Player* bot)
{
    Position pos;
    float radius = 0.0f;
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return LandMineDanger(botAI, bot, pos, radius) || ShockwaveDanger(botAI, bot, x, y, z) ||
           HeavensFuryDanger(botAI, bot, pos, radius) ||
           HallowedGroundDanger(botAI, bot, pos, radius) ||
           BlazeFireDanger(botAI, bot, pos, radius) || SiamatStormDanger(botAI, bot, pos, radius) ||
           LightningChargeDanger(botAI, bot, pos, radius) || DustFlailBoss(botAI, bot) != nullptr;
}
}
