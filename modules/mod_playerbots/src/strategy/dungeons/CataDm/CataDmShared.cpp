/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources.

#include "CataDmShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Group.h"
#include "MopDungeonShared.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Vehicle.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <string>
#include <vector>
#include <mutex>
#include <unordered_map>
#include "Timer.h"

namespace CataDm
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

Unit* PickFocus(PlayerbotAI* botAI, Player* bot, std::initializer_list<uint32> priority, float maxDist,
                bool engagedOnly)
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

        if (engagedOnly && !unit->IsInCombat())
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
bool BlossomLanding(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = NearestCreature(bot, { NPC_GLUBTOK }, SCAN_RANGE, SCAN_RANGE);
    if (!boss)
        return false;

    Spell* spell = FindCast(boss, { SPELL_FIRE_BLOSSOM, SPELL_FROST_BLOSSOM });
    if (!spell)
        return false;

    // #667: 88129 / 88169 are instant TRIGGER_MISSILE spells (5.4.8 SpellEffect.dbc effect 32 -> 88173 / 88177, target 63 =
    // DEST_TARGET_ANY; Wowhead tooltip API 2026-10-15: "Instant"), so the landing point may be a destination, not a unit.
    // "dm blossom landing" never fired in w65 / w67 / w68 (0 trigger ticks) while the heroic blossom adds kept spawning:
    // take the destination when there is no unit target.
    Position landing;
    if (Unit* target = spell->m_targets.GetUnitTarget())
        landing = target->GetPosition();
    else if (spell->m_targets.HasDst())
        landing = spell->m_targets.GetDstPos()->GetPosition();
    else
        return false;

    if (bot->GetExactDist2d(landing.GetPositionX(), landing.GetPositionY()) > BLOSSOM_RADIUS)
        return false;

    pos = landing;
    radius = BLOSSOM_RADIUS + FLEE_MARGIN;
    return true;
}

Unit* BlossomAddFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_FIRE_BLOSSOM, NPC_FROST_BLOSSOM }, FOCUS_RANGE);
}

bool StickyBombDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    // #667 w68: live bombs only. npc_sticky_bomb arms / explodes only from its UpdateAI, which returns at !UpdateVictim()
    // (boss_helix_gearbreaker.cpp npc_sticky_bombAI::UpdateAI) - i.e. a bomb that never entered combat never goes off. Helix's
    // bombs are zoned in (BossAI::JustSummoned -> DoZoneInCombat, ScriptedCreature.cpp:779-784); the heroic crew's bombs are not
    // (npc_helix_crewAI::JustSummoned only records them) and sit inert for their 30 s, yet they kept every bot "in danger".
    Creature* bomb = nullptr;
    float bestDist = STICKY_BOMB_RADIUS;
    for (Creature* candidate : FindCreatures(bot, { NPC_STICKY_BOMB }, SCAN_RANGE))
    {
        if (!candidate->IsInCombat())
            continue;

        float const dist = bot->GetExactDist2d(candidate);
        if (dist <= bestDist)
        {
            bomb = candidate;
            bestDist = dist;
        }
    }

    if (!bomb)
        return false;

    pos = bomb->GetPosition();
    radius = STICKY_BOMB_RADIUS + FLEE_MARGIN;
    return true;
}

namespace
{
// One kind of bomb hazard: the points to keep away from and how far.
struct BombHazards
{
    std::vector<Position> points;
    float safe = 0.0f;
};

// The live Sticky Bombs, and the Chest Bomb carriers (for a carrier: every other alive member), within ESCAPE_SCAN.
std::vector<BombHazards> CollectBombHazards(Player* bot)
{
    std::vector<BombHazards> sets;

    BombHazards bombs;
    bombs.safe = STICKY_BOMB_RADIUS + FLEE_MARGIN;
    for (Creature* bomb : FindCreatures(bot, { NPC_STICKY_BOMB }, ESCAPE_SCAN))
    {
        if (bomb->IsInCombat())  // see StickyBombDanger
            bombs.points.push_back(bomb->GetPosition());
    }

    if (!bombs.points.empty())
        sets.push_back(bombs);

    Group* group = bot->GetGroup();
    if (!group)
        return sets;

    bool const carrier = bot->HasAura(SPELL_CHEST_BOMB);
    BombHazards chest;
    chest.safe = CHEST_BOMB_RADIUS + FLEE_MARGIN;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        if (bot->GetExactDist2d(member) > ESCAPE_SCAN)
            continue;

        if (carrier || member->HasAura(SPELL_CHEST_BOMB))
            chest.points.push_back(member->GetPosition());
    }

    if (!chest.points.empty())
        sets.push_back(chest);

    return sets;
}

// How far (x, y) lies beyond the safe distance of the closest hazard, capped at ESCAPE_MARGIN_CAP (negative = inside one).
float BombMargin(std::vector<BombHazards> const& sets, float x, float y)
{
    float margin = ESCAPE_MARGIN_CAP;
    for (BombHazards const& set : sets)
    {
        for (Position const& point : set.points)
            margin = std::min(margin, std::hypot(x - point.GetPositionX(), y - point.GetPositionY()) - set.safe);
    }

    return margin;
}
}

bool BombEscapeSpot(Player* bot, float& x, float& y, float& z)
{
    std::vector<BombHazards> const sets = CollectBombHazards(bot);
    if (sets.empty() || BombMargin(sets, bot->GetPositionX(), bot->GetPositionY()) >= 0.0f)
        return false;

    auto const score = [&sets](float px, float py) { return BombMargin(sets, px, py); };
    return MopDungeon::FindBestSpot(bot, score, 1.0f, x, y, z);
}

bool BombSpotSafe(Player* bot, float x, float y)
{
    return BombMargin(CollectBombHazards(bot), x, y) >= 0.0f;
}

bool ChestBombDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Group* group = bot->GetGroup();
    if (!group)
        return false;

    bool const carrier = bot->HasAura(SPELL_CHEST_BOMB);
    Player* nearest = nullptr;
    float nearestDist = CHEST_BOMB_RADIUS;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || member->GetMapId() != bot->GetMapId())
            continue;

        // the carrier must clear everybody; the others only need to clear the carrier
        if (!carrier && !member->HasAura(SPELL_CHEST_BOMB))
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist <= nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    if (!nearest)
        return false;

    pos = nearest->GetPosition();
    radius = CHEST_BOMB_RADIUS + FLEE_MARGIN;
    return true;
}

bool HarvestDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* bunny = NearestCreature(bot, { NPC_TARGETING_BUNNY }, SCAN_RANGE, HARVEST_RADIUS);
    if (!bunny)
        return false;

    pos = bunny->GetPosition();
    radius = HARVEST_RADIUS + FLEE_MARGIN;
    return true;
}

bool OverdriveDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = NearestCreature(bot, { NPC_FOE_REAPER_5000 }, SCAN_RANGE, OVERDRIVE_RADIUS);
    if (!boss || !boss->HasAura(SPELL_OVERDRIVE))
        return false;

    pos = boss->GetPosition();
    radius = OVERDRIVE_RADIUS + FLEE_MARGIN;
    return true;
}

Unit* HelixFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || PlayerBotSpec::IsHeal(bot))
        return nullptr;

    // #667 w68: engaged units only. The group arrived at the Helix staging point next to an Oaf Lackey (spawn 319124, 9 yd), the
    // lackey fight put the bots in combat, and this focus picked the idle Oaf 29 yd away ("T:dm helix focus" -> "A:dm attack
    // helix - OK" -> target 47297, Server.log of the w68 helix leg): the trash gate pulled the boss, wiped, and left a released
    // Helix and orphan crews behind for the real pull. AC pattern: a focus scan over "possible targets" takes only units already
    // in combat (AC Ai/Dungeon/HoL/HoLTriggers.cpp:11-24, Stormforged Lieutenant).
    return PickFocus(botAI, bot, { NPC_HELIX_GEARBREAKER, NPC_LUMBERING_OAF }, FOCUS_RANGE, true);
}

Unit* VaporFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || PlayerBotSpec::IsHeal(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_VAPOR }, FOCUS_RANGE);
}

Unit* VanessaAddFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_DEFIAS_BLOOD_WIZARD, NPC_DEFIAS_SHADOWGUARD, NPC_DEFIAS_ENFORCER }, FOCUS_RANGE);
}


// ---------------------------------------------------------------------------------------------------------------
// #681 / #826
// ---------------------------------------------------------------------------------------------------------------
namespace
{
// Angle of (x, y) seen from the Fire Wall pivot
float PivotAngle(float x, float y)
{
    return Position::NormalizeOrientation(std::atan2(y - FIRE_WALL_PIVOT_Y, x - FIRE_WALL_PIVOT_X));
}

// The wall's two halves seen from the pivot: angle and length (the farthest dummy on each side). False = no wall.
// #681 w106: every dummy counts, lit or not. The dummies exist only for the heroic wall (boss_glubtok.cpp SummonFireWall) and now
// appear with the announce, 1.4 s before they burn (CPP timing): a bot on the line steps off before the first tick instead of
// taking it (w106 Frannor: two neighbouring dummies, 99k in the wall's first second, dead).
bool FireWallHalves(Player* bot, float (&angle)[2], float (&length)[2])
{
    std::vector<Creature*> const dummies = FindCreatures(bot, { NPC_FIRE_WALL_DUMMY }, 80.0f);
    Creature* farthest = nullptr;
    float farDist = 0.0f;
    for (Creature* dummy : dummies)
    {
        float const d = std::hypot(dummy->GetPositionX() - FIRE_WALL_PIVOT_X, dummy->GetPositionY() - FIRE_WALL_PIVOT_Y);
        if (d > farDist)
        {
            farDist = d;
            farthest = dummy;
        }
    }
    if (!farthest)
        return false;

    angle[0] = PivotAngle(farthest->GetPositionX(), farthest->GetPositionY());
    angle[1] = Position::NormalizeOrientation(angle[0] + float(M_PI));
    length[0] = farDist;
    length[1] = 0.0f;
    for (Creature* dummy : dummies)
    {
        float const a = PivotAngle(dummy->GetPositionX(), dummy->GetPositionY());
        float delta = std::fabs(Position::NormalizeOrientation(a - angle[1]));
        if (delta > float(M_PI))
            delta = 2.0f * float(M_PI) - delta;
        if (delta < 0.5f)
            length[1] = std::max(length[1], std::hypot(dummy->GetPositionX() - FIRE_WALL_PIVOT_X, dummy->GetPositionY() - FIRE_WALL_PIVOT_Y));
    }
    return true;
}
}

bool FireWallDanger(Player* bot)
{
    if (!bot->IsInCombat() || bot->GetMapId() != MAP_ID)
        return false;

    float angle[2];
    float length[2];
    if (!FireWallHalves(bot, angle, length))
        return false;

    float const r = std::hypot(bot->GetPositionX() - FIRE_WALL_PIVOT_X, bot->GetPositionY() - FIRE_WALL_PIVOT_Y);
    float const beta = PivotAngle(bot->GetPositionX(), bot->GetPositionY());
    float const omega = 2.0f * float(M_PI) / FIRE_WALL_PERIOD;
    float const width = FIRE_WALL_HALF_WIDTH + FLEE_MARGIN;
    float const halfWidthAngle = r > width ? std::asin(width / r) : float(M_PI);
    for (uint8 h = 0; h < 2; ++h)
    {
        if (r > length[h] + width)
            continue;

        // clockwise: the half's angle decreases; `ahead` = how far it still turns before it reaches the bot
        float const ahead = Position::NormalizeOrientation(angle[h] - beta);
        if (ahead <= halfWidthAngle + omega * FIRE_WALL_WARN || ahead >= 2.0f * float(M_PI) - halfWidthAngle)
            return true;
    }
    return false;
}

bool FireWallSpot(Player* bot, float& x, float& y, float& z)
{
    float angle[2];
    float length[2];
    if (!FireWallHalves(bot, angle, length))
        return false;

    float const r = std::hypot(bot->GetPositionX() - FIRE_WALL_PIVOT_X, bot->GetPositionY() - FIRE_WALL_PIVOT_Y);
    float const beta = PivotAngle(bot->GetPositionX(), bot->GetPositionY());
    float const omega = 2.0f * float(M_PI) / FIRE_WALL_PERIOD;

    // #681 w106: a bot already inside a half's band (the wall on it, or lit under it) goes BEHIND that half - the wedge it just
    // left, against the turn - with no lead: the half moves away from it, so it is out after a step or two. Racing ahead of a
    // half that is already over the bot (the nearer perpendicular when the half's centre line has not passed yet) keeps it in
    // the fire for seconds at a 2.3 yd/s gain (run 7 vs the wall's 4.7 yd/s at 15 yd) - w106: 13 wall hits on four players.
    float const width = FIRE_WALL_HALF_WIDTH + FLEE_MARGIN;
    float const halfWidthAngle = r > width ? std::asin(width / r) : float(M_PI);
    for (uint8 h = 0; h < 2; ++h)
    {
        if (r > length[h] + width)
            continue;

        float off = Position::NormalizeOrientation(beta - angle[h]);   // > 0: the bot is behind the half (counter-clockwise)
        if (off > float(M_PI))
            off -= 2.0f * float(M_PI);
        if (std::fabs(off) > halfWidthAngle)
            continue;

        float const behind = Position::NormalizeOrientation(angle[h] + float(M_PI) / 2.0f);
        float const radius = std::max(FIRE_WALL_MIN_RADIUS, std::min(r, FIRE_WALL_MAX_RADIUS));
        float const tx = FIRE_WALL_PIVOT_X + radius * std::cos(behind);
        float const ty = FIRE_WALL_PIVOT_Y + radius * std::sin(behind);
        float const dist = std::hypot(tx - bot->GetPositionX(), ty - bot->GetPositionY());
        if (dist < 1.0f)
            return false;

        return EncounterHelpers::CanTakeStepTowards(bot, tx, ty, dist, x, y, z);
    }

    // the wall is still coming: the two perpendiculars to the wall line; the nearer one is the middle of the bot's own wedge
    float best = 0.0f;
    float bestDelta = 10.0f;
    for (float const side : { -1.0f, 1.0f })
    {
        float const cand = Position::NormalizeOrientation(angle[0] + side * float(M_PI) / 2.0f);
        float delta = std::fabs(Position::NormalizeOrientation(cand - beta));
        if (delta > float(M_PI))
            delta = 2.0f * float(M_PI) - delta;
        if (delta < bestDelta)
        {
            bestDelta = delta;
            best = cand;
        }
    }
    float const target = Position::NormalizeOrientation(best - omega * FIRE_WALL_LEAD);
    float const radius = std::max(FIRE_WALL_MIN_RADIUS, std::min(r, FIRE_WALL_MAX_RADIUS));
    float const tx = FIRE_WALL_PIVOT_X + radius * std::cos(target);
    float const ty = FIRE_WALL_PIVOT_Y + radius * std::sin(target);
    float const dist = std::hypot(tx - bot->GetPositionX(), ty - bot->GetPositionY());
    if (dist < 1.0f)
        return false;

    return EncounterHelpers::CanTakeStepTowards(bot, tx, ty, dist, x, y, z);
}

bool MoltenSlagDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* slag = NearestCreature(bot, { NPC_MOLTEN_SLAG }, SCAN_RANGE, MOLTEN_SHIELD_RADIUS + FLEE_MARGIN);
    if (!slag)
        return false;

    pos = slag->GetPosition();
    radius = MOLTEN_SHIELD_RADIUS + FLEE_MARGIN + 1.0f;
    return true;
}

Unit* MoltenSlagFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsRangedDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_MOLTEN_SLAG }, FOCUS_RANGE, true);
}

Unit* ReaperTankFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsTank(bot))
        return nullptr;

    // PickFocus returns nothing while the current target already is the Reaper
    return PickFocus(botAI, bot, { NPC_FOE_REAPER_5000 }, FOCUS_RANGE, true);
}

bool VanessaFiresOnDeck(Player* bot)
{
    return !FindCreatures(bot, { NPC_NIGHTMARE_FIRE_BUNNY }, ROPE_SCAN).empty() &&
           NearestCreature(bot, { NPC_VANESSA_VANCLEEF }, 80.0f, 80.0f) != nullptr;
}

bool VanessaBlazeActive(Player* bot)
{
    for (Creature* fire : FindCreatures(bot, { NPC_NIGHTMARE_FIRE_BUNNY }, ROPE_SCAN))
        if (fire->HasAura(SPELL_FIERY_BLAZE))
            return true;
    return false;
}

bool VanessaBlazeDanger(Player* bot)
{
    for (Creature* fire : FindCreatures(bot, { NPC_NIGHTMARE_FIRE_BUNNY, NPC_GPB_JMF }, 20.0f))
        if (fire->HasAura(SPELL_FIERY_BLAZE) && bot->GetExactDist2d(fire) <= FIERY_BLAZE_RADIUS + FLEE_MARGIN)
            return true;
    return false;
}

namespace
{
// #681: when each bot was last seen on a rope. A bot that lands back on the deck still sees burning fires (the outer ones burn
// for the rest of the fight) and Vanessa still unselectable for ~1 s - without this it took the fifth rope again. Bots of
// different maps update on different threads, hence the lock.
std::mutex ropeRideMutex;
std::unordered_map<uint64, uint32> ropeRideSeen;

void NoteRopeRide(Player* bot)
{
    std::lock_guard<std::mutex> lock(ropeRideMutex);
    ropeRideSeen[bot->GetGUID().GetRawValue()] = getMSTime();
}

bool RecentlyRoped(Player* bot)
{
    std::lock_guard<std::mutex> lock(ropeRideMutex);
    auto const itr = ropeRideSeen.find(bot->GetGUID().GetRawValue());
    return itr != ropeRideSeen.end() && GetMSTimeDiffToNow(itr->second) < ROPE_RIDE_COOLDOWN_MS;
}
}

Creature* VanessaRope(Player* bot)
{
    if (bot->GetMapId() != MAP_ID)
        return nullptr;

    if (Unit* base = bot->GetVehicleBase())
    {
        if (base->GetEntry() == NPC_VANESSA_ROPE)
            NoteRopeRide(bot);
        return nullptr;
    }

    if (RecentlyRoped(bot))
        return nullptr;

    Creature* best = nullptr;
    float bestDist = ROPE_SCAN;
    for (Creature* rope : FindCreatures(bot, { NPC_VANESSA_ROPE }, ROPE_SCAN))
    {
        if (!rope->HasAura(SPELL_ROPE_CLICK_ME) || !rope->GetVehicleKit() || !rope->GetVehicleKit()->HasEmptySeat(0))
            continue;
        float const dist = bot->GetExactDist2d(rope);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = rope;
        }
    }
    return best;
}

bool PowderKegDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    Creature* vanessa = NearestCreature(bot, { NPC_VANESSA_VANCLEEF }, SCAN_RANGE, POWDER_RADIUS + FLEE_MARGIN + 1.0f);
    if (!vanessa || !(vanessa->HasAura(SPELL_COSMETIC_BOMB) || vanessa->GetHealthPct() <= 2.0f))
        return false;

    pos = vanessa->GetPosition();
    radius = POWDER_RADIUS + FLEE_MARGIN + 1.0f;
    return true;
}

Creature* SteamValveToClick(Player* bot)
{
    Unit* base = bot->GetVehicleBase();
    if (!base || base->GetEntry() != NPC_TRAP_BUNNY)
        return nullptr;

    Creature* best = nullptr;
    float bestDist = VALVE_RANGE;
    for (Creature* valve : FindCreatures(bot, { NPC_STEAM_VALVE }, VALVE_RANGE))
    {
        if (valve->HasUnitFlag(UNIT_FLAG_NOT_SELECTABLE) || !valve->HasNpcFlag(UNIT_NPC_FLAG_SPELLCLICK))
            continue;
        float const dist = bot->GetExactDist(valve);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = valve;
        }
    }
    return best;
}

namespace
{
struct GroundHazard
{
    float x;
    float y;
    float safe;
};

std::vector<GroundHazard> CollectGroundHazards(Player* bot)
{
    std::vector<GroundHazard> out;
    float const margin = FLEE_MARGIN;
    for (Creature* c : FindCreatures(bot, { NPC_NIGHTMARE_FIRE_BUNNY, NPC_GPB_JMF }, ESCAPE_SCAN))
    {
        if (c->HasAura(SPELL_NIGHTMARE_FLAMES))
            out.push_back({ c->GetPositionX(), c->GetPositionY(), NIGHTMARE_FLAMES_RADIUS + margin });
        else if (c->HasAura(SPELL_FIERY_BLAZE))
            out.push_back({ c->GetPositionX(), c->GetPositionY(), FIERY_BLAZE_RADIUS + margin });
    }
    for (Creature* c : FindCreatures(bot, { NPC_COLLAPSING_ICICLE }, ESCAPE_SCAN))
        out.push_back({ c->GetPositionX(), c->GetPositionY(), ICE_SHARDS_RADIUS + margin });
    for (Creature* c : FindCreatures(bot, { NPC_LIGHTNING_STALKER, NPC_LIGHTNING_PLATTER }, ESCAPE_SCAN))
        if (c->HasAura(SPELL_SPARK) || c->HasAura(SPELL_SPARK_PLATTER))
            out.push_back({ c->GetPositionX(), c->GetPositionY(), SPARK_RADIUS + margin });
    return out;
}

float GroundHazardMargin(std::vector<GroundHazard> const& hazards, float x, float y)
{
    float margin = ESCAPE_MARGIN_CAP;
    for (GroundHazard const& h : hazards)
        margin = std::min(margin, std::hypot(x - h.x, y - h.y) - h.safe);
    return margin;
}
}

bool GroundHazardDanger(Player* bot)
{
    if (bot->GetMapId() != MAP_ID || bot->GetVehicle())
        return false;

    std::vector<GroundHazard> const hazards = CollectGroundHazards(bot);
    return !hazards.empty() && GroundHazardMargin(hazards, bot->GetPositionX(), bot->GetPositionY()) < 0.0f;
}

bool GroundHazardEscapeSpot(Player* bot, float& x, float& y, float& z)
{
    std::vector<GroundHazard> const hazards = CollectGroundHazards(bot);
    if (hazards.empty() || GroundHazardMargin(hazards, bot->GetPositionX(), bot->GetPositionY()) >= 0.0f)
        return false;

    auto const score = [&hazards](float px, float py) { return GroundHazardMargin(hazards, px, py); };
    return MopDungeon::FindBestSpot(bot, score, 0.5f, x, y, z);
}

namespace
{
// #826 w107: the live James within JAMES_SCAN. Only in Ripsnarl's nightmare: Erik's rescue, which summons James 0.2 s later, puts
// Adrenaline (60 s) on every player (w107 [PROBE] spdaura=92604:50 on all four bots), so the 150 yd grid search stays off the rest
// of the dungeon. A bot without it still follows its master, whom the driver moves to the ramp.
Creature* JamesHarrington(Player* bot)
{
    if (bot->GetMapId() != MAP_ID || bot->GetVehicle() || !bot->HasAura(SPELL_ADRENALINE))
        return nullptr;

    return NearestCreature(bot, { NPC_JAMES_HARRINGTON }, JAMES_SCAN, JAMES_SCAN);
}

bool JamesOutOfReach(Player* bot, Unit* james)
{
    if (bot->GetExactDist2d(JAMES_APPROACH_X, JAMES_APPROACH_Y) <= JAMES_APPROACH_ARRIVED)
        return false;   // at the ramp spot: fight from here (no walk loop if the sight line is clipped)

    return bot->GetExactDist2d(james) > JAMES_REACH || !bot->IsWithinLOSInMap(james);
}
}

bool JamesApproachSpot(Player* bot, float& x, float& y, float& z)
{
    Creature* james = JamesHarrington(bot);
    if (!james || !JamesOutOfReach(bot, james))
        return false;

    x = JAMES_APPROACH_X;
    y = JAMES_APPROACH_Y;
    z = JAMES_APPROACH_Z;
    return true;
}

Unit* HarringtonFocus(PlayerbotAI* botAI, Player* bot)
{
    // #826 w107: James out of reach first, for every role (the healer keeps the group in range); see JamesApproachSpot
    if (Creature* james = JamesHarrington(bot))
        if (JamesOutOfReach(bot, james))
            return james;

    if (PlayerBotSpec::IsHeal(bot))
        return nullptr;

    // James first: Calissa dies to her Choking in ~33 s (3% a second, 92608); the worgen only threaten Emme / Erik
    return PickFocus(botAI, bot, { NPC_JAMES_HARRINGTON, NPC_ENRAGED_WORGEN }, FOCUS_RANGE);
}

bool InDanger(PlayerbotAI* botAI, Player* bot)
{
    Position pos;
    float radius = 0.0f;
    return BlossomLanding(botAI, bot, pos, radius) || StickyBombDanger(botAI, bot, pos, radius) ||
           ChestBombDanger(botAI, bot, pos, radius) || HarvestDanger(botAI, bot, pos, radius) ||
           OverdriveDanger(botAI, bot, pos, radius) || FireWallDanger(bot) || MoltenSlagDanger(botAI, bot, pos, radius) ||
           PowderKegDanger(botAI, bot, pos, radius) || GroundHazardDanger(bot);
}
}
