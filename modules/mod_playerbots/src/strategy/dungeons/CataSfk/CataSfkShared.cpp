/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources.

#include "CataSfkShared.h"
#include "AiObjectContext.h"
#include "Config.h"
#include "Creature.h"
#include "Group.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "Spell.h"
#include "SpellAuras.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cmath>
#include <list>
#include <sstream>
#include <string>
#include <vector>

namespace CataSfk
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

bool ConeEscapePoint(Player* bot, Unit* source, float halfAngle, float& x, float& y, float& z)
{
    float const facing = source->GetOrientation();
    float const toBot = Position::NormalizeOrientation(source->GetAngle(bot) - facing);
    float const ownSide = toBot < float(M_PI) ? 1.0f : -1.0f;  // 1 = left of the facing, -1 = right
    float const dist = std::max(6.0f, std::min(bot->GetExactDist2d(source), 14.0f));

    // the LateralPoint spot on the bot's own side first, then the mirrored one (w67 Godfrey: 103 of 110 "leave barrage
    // cone" runs FAILED in the narrow gallery; the mage's [PROBE] rows read mv=0 through the barrage that killed it)
    for (float const side : { ownSide, -ownSide })
    {
        float const angle = facing + side * LATERAL_ANGLE;
        x = source->GetPositionX() + std::cos(angle) * dist;
        y = source->GetPositionY() + std::sin(angle) * dist;
        z = bot->GetPositionZ();

        // 5.4.8: Map::CheckCollisionAndGetValidCoords signature identical to the UB use (src/server/game/Maps/Map.h:662)
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;

        // the collision check stops the run at a wall: only a point that really leaves the cone counts
        if (AngleDelta(source->GetAngle(x, y), facing) > halfAngle)
            return true;
    }

    return false;
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

namespace
{
// Dedicated interrupts only (the MgT list minus its stun/silence "control" tier), plus the 5.x monk/paladin ones
char const* const CLASS_INTERRUPTS[] = { "kick",         "pummel",     "wind shear", "mind freeze",      "counterspell",
                                         "spell lock",   "rebuke",     "spear hand strike", "skull bash" };
}

bool IsClassInterrupt(std::string const& spellName)
{
    for (char const* interrupt : CLASS_INTERRUPTS)
    {
        if (spellName == interrupt)
            return true;
    }

    return false;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target || !target->IsNonMeleeSpellCasted(true))
        return {};

    for (char const* interrupt : CLASS_INTERRUPTS)
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
Unit* AshburyInterruptTarget(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat())
        return nullptr;

    return InterruptCaster(bot, { NPC_ASHBURY }, { SPELL_PAIN_AND_SUFFERING }, INTERRUPT_SCAN);
}

Unit* WorgenSpiritFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot,
                     { NPC_RETHILGORE, NPC_ODO_BLINDWATCHER, NPC_RAZORCLAW_BUTCHER, NPC_WOLF_MASTER_NANDOS,
                       NPC_LUPINE_SPECTRE },
                     FOCUS_RANGE);
}

Unit* WorgenInterruptTarget(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat())
        return nullptr;

    return InterruptCaster(bot, { NPC_RETHILGORE, NPC_ODO_BLINDWATCHER }, { SPELL_SOUL_DRAIN, SPELL_BLINDING_SHADOWS },
                           INTERRUPT_SCAN);
}

bool DesecrationDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* stalker = NearestCreature(bot, { NPC_DESECRATION_STALKER }, SCAN_RANGE, DESECRATION_RADIUS);
    if (!stalker)
        return false;

    pos = stalker->GetPosition();
    radius = DESECRATION_RADIUS + FLEE_MARGIN;
    return true;
}

Unit* EmpowermentInterruptTarget(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat())
        return nullptr;

    return InterruptCaster(bot, { NPC_WAILING_GUARDSMAN, NPC_TORMENTED_OFFICER }, { SPELL_UNHOLY_EMPOWERMENT },
                           INTERRUPT_SCAN);
}

Unit* ShieldConeBoss(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat() || PlayerBotSpec::IsTank(bot))
        return nullptr;

    Creature* boss = NearestCreature(bot, { NPC_SPRINGVALE }, SCAN_RANGE, SCAN_RANGE);
    if (!boss || !IsCastingOrAura(boss, { SPELL_SHIELD_OF_PERFIDIOUS }))
        return nullptr;

    return InCone(boss, bot, SHIELD_CONE_HALF_ANGLE, SHIELD_CONE_RANGE, false) ? boss : nullptr;
}

bool PoisonMixtureDanger(PlayerbotAI* /*botAI*/, Player* bot, Position& pos, float& radius)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = NearestCreature(bot, { NPC_WALDEN }, SCAN_RANGE, SCAN_RANGE);
    if (!boss)
        return false;

    Spell* spell = FindCast(boss, { SPELL_POISONOUS_MIXTURE });
    if (!spell)
        return false;

    Unit* target = spell->m_targets.GetUnitTarget();
    if (!target)
        return false;

    radius = POISON_MIXTURE_RADIUS + FLEE_MARGIN;
    if (target != bot)
    {
        if (bot->GetExactDist2d(target) > radius)
            return false;

        pos = target->GetPosition();
        return true;
    }

    // The bot is the target: the bottle follows it (TARGET_DEST_TARGET_ENEMY is resolved when the cast ends), so it
    // walks away from the nearest group member still inside the splash
    Group* group = bot->GetGroup();
    if (!group)
        return false;

    Player* nearest = nullptr;
    float nearestDist = radius;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || member == bot || !member->IsAlive() || !member->IsInWorld() ||
            member->GetMapId() != bot->GetMapId())
            continue;

        float const dist = bot->GetExactDist2d(member);
        if (dist < nearestDist)
        {
            nearest = member;
            nearestDist = dist;
        }
    }

    if (!nearest)
        return false;

    pos = nearest->GetPosition();
    return true;
}

bool AwayPoint(Player* bot, Position const& from, float dist, float& x, float& y, float& z)
{
    float base = from.GetAngle(bot);
    if (from.GetExactDist2d(bot) < 0.5f)
        base = bot->GetOrientation() + float(M_PI);

    // straight away first, then 45 / 90 deg off it on either side (a wall or the room edge behind the bot)
    static float const offsets[] = { 0.0f, 0.785f, -0.785f, 1.571f, -1.571f };
    for (float const offset : offsets)
    {
        float const angle = base + offset;
        x = from.GetPositionX() + std::cos(angle) * dist;
        y = from.GetPositionY() + std::sin(angle) * dist;
        z = bot->GetPositionZ();

        // 5.4.8: Map::CheckCollisionAndGetValidCoords signature identical to the UB use (src/server/game/Maps/Map.h:662)
        if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                            bot->GetPositionZ(), x, y, z))
            continue;

        // the collision check shortens the run at a wall: only a point that really leaves the splash counts
        if (from.GetExactDist2d(x, y) >= dist - FLEE_MARGIN)
            return true;
    }

    return false;
}

bool KeepMovingDanger(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || bot->isMoving())
        return false;

    // the red cauldron beats every reason to move (CatalystUp)
    if (CatalystUp(botAI, bot))
        return false;

    if (CoagulantTicking(bot))
        return true;

    if (NearestCreature(bot, { NPC_MYSTERY_TOXIN_GREEN }, TOXIN_RANGE, TOXIN_RANGE))
        return true;

    Creature* boss = NearestCreature(bot, { NPC_WALDEN }, SCAN_RANGE, SCAN_RANGE);
    return boss && IsCastingOrAura(boss, { SPELL_ICE_SHARDS });
}

bool CoagulantTicking(Player* bot)
{
    return bot->IsInCombat() && bot->HasAura(SPELL_TOXIC_COAGULANT);
}

bool CatalystUp(PlayerbotAI* /*botAI*/, Player* bot)
{
    return bot->IsInCombat() && NearestCreature(bot, { NPC_MYSTERY_TOXIN_RED }, TOXIN_RANGE, TOXIN_RANGE) != nullptr;
}

bool CoagulantJump(PlayerbotAI* botAI, Player* bot)
{
    Aura* aura = bot->GetAura(SPELL_TOXIC_COAGULANT);
    if (!aura || aura->GetStackAmount() < COAGULANT_JUMP_STACKS)
        return false;

    // AC UldTriggers.cpp:529-543: the boss alive, the master alive and not at the same stack count
    if (!NearestCreature(bot, { NPC_WALDEN }, TOXIN_RANGE, TOXIN_RANGE))
        return false;

    Player* master = botAI->GetMaster();
    if (!master || !master->IsAlive())
        return false;

    Aura* masterAura = master->GetAura(SPELL_TOXIC_COAGULANT);
    return !masterAura || masterAura->GetStackAmount() < COAGULANT_JUMP_STACKS;
}

bool HasRaidCheat()
{
    static bool const value = []
    {
        std::stringstream ss(sConfigMgr->GetStringDefault("AiPlayerbot.BotCheats", "food,taxi,raid"));
        std::string token;
        while (std::getline(ss, token, ','))
        {
            token.erase(0, token.find_first_not_of(" \t"));
            token.erase(token.find_last_not_of(" \t") + 1);
            if (token == "raid")
                return true;
        }
        return false;
    }();
    return value;
}

Unit* CursedBulletsInterruptTarget(PlayerbotAI* /*botAI*/, Player* bot)
{
    if (!bot->IsInCombat())
        return nullptr;

    return InterruptCaster(bot, { NPC_GODFREY }, { SPELL_CURSED_BULLET }, INTERRUPT_SCAN);
}

Unit* BarrageConeBoss(PlayerbotAI* /*botAI*/, Player* bot)
{
    // The tank too (#668 w67: 22 of 56 barrage hits, 22275 each, on the tank): the cone points at a random player
    // (96344 picks any enemy, 96343 summons the aim stalker 52065 on it) and Godfrey stops attacking for the whole
    // barrage (Creature::PrepareChanneledCast: AttackStop + REACT_PASSIVE), then chases his old victim again
    // (boss_lord_godfrey.cpp DoAction -> RemoveChanneledCast), so stepping out costs the tank nothing.
    if (!bot->IsInCombat())
        return nullptr;

    Creature* boss = NearestCreature(bot, { NPC_GODFREY }, 70.0f, 70.0f);
    if (!boss || !IsCastingOrAura(boss, { SPELL_PISTOL_BARRAGE, SPELL_PISTOL_BARRAGE_DMG_AURA }))
        return nullptr;

    return InCone(boss, bot, BARRAGE_CONE_HALF_ANGLE, BARRAGE_CONE_RANGE, false) ? boss : nullptr;
}

bool BarrageCasting(Player* bot)
{
    if (!bot->IsInCombat())
        return false;

    Creature* boss = NearestCreature(bot, { NPC_GODFREY }, 70.0f, 70.0f);
    return boss && FindCast(boss, { SPELL_PISTOL_BARRAGE }) != nullptr;
}

Unit* GhoulFocus(PlayerbotAI* botAI, Player* bot)
{
    if (!bot->IsInCombat() || !PlayerBotSpec::IsDps(bot) || PlayerBotSpec::IsTank(bot))
        return nullptr;

    return PickFocus(botAI, bot, { NPC_BLOODTHIRSTY_GHOUL }, FOCUS_RANGE);
}


bool InDanger(PlayerbotAI* botAI, Player* bot)
{
    Position pos;
    float radius = 0.0f;
    return DesecrationDanger(botAI, bot, pos, radius) || PoisonMixtureDanger(botAI, bot, pos, radius) ||
           ShieldConeBoss(botAI, bot) != nullptr || BarrageConeBoss(botAI, bot) != nullptr;
}
}
