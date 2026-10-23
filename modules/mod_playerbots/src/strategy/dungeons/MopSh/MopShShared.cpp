/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#include "MopShShared.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "DungeonStrategyUtils.h"
#include "Group.h"
#include "Player.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ObjectAccessor.h"
#include "Spell.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include <algorithm>
#include <cfloat>
#include <cmath>
#include <list>
#include <vector>

namespace MopScarletHalls
{
namespace
{
// Same stop-a-cast test as the fork's MgT strategy (MgTShared.cpp:507): a silence-type interrupt, an explicit
// interrupt-cast effect, or a hard stun/silence aura.
bool StopsACast(uint32 spellId)
{
    SpellInfo const* info = sSpellMgr->GetSpellInfo(spellId);
    if (!info)
        return false;

    if ((info->InterruptFlags & SPELL_INTERRUPT_FLAG_INTERRUPT) &&
        info->PreventionType == SPELL_PREVENTION_TYPE_SILENCE)
        return true;

    for (uint8 i = EFFECT_0; i <= EFFECT_2; ++i)
    {
        if (info->Effects[i].Effect == SPELL_EFFECT_INTERRUPT_CAST)
            return true;

        if (info->Effects[i].Effect != SPELL_EFFECT_APPLY_AURA)
            continue;

        if (info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_SILENCE ||
            info->Effects[i].ApplyAuraName == SPELL_AURA_MOD_STUN)
            return true;
    }

    return false;
}

// Dedicated interrupts first, hard control last (MgTShared.cpp:545-566, plus the MoP-only names: Monk
// "spear hand strike", Paladin "rebuke", Druid "skull bash", Hunter "counter shot").
char const* const INTERRUPT_SPELLS[] = {
    "kick", "pummel", "shield bash", "wind shear", "mind freeze", "counterspell", "spell lock",
    "spear hand strike", "rebuke", "skull bash", "counter shot", "silencing shot",
    "shockwave", "shadowfury", "hammer of justice", "kidney shot", "silence", "bash", "strangulate",
    "arcane torrent"
};
}

Creature* FindBoss(Player* bot, uint32 entry)
{
    return bot->FindNearestCreature(entry, BOSS_SCAN_RANGE, true);
}

uint8 InterruptPriority(uint32 spellId)
{
    switch (spellId)
    {
        case SPELL_PYROBLAST:               // 195k + DoT on the tank
            return 1;
        case SPELL_BOOK_BURNER:             // lights a book case: 15 yd, 195k per 2 s for 30 s
            return 2;
        case SPELL_GREATER_DRAGONS_BREATH:  // 360 deg cone sweep, 243k + 48k/s + confusion (channel is interruptible)
            return 3;
        case SPELL_FIREBALL_VOLLEY:         // 219k to everyone within 35 yd
            return 4;
        default:
            return 0;
    }
}

uint8 CurrentCastPriority(Unit* caster)
{
    uint8 best = 0;
    for (CurrentSpellTypes slot : { CURRENT_GENERIC_SPELL, CURRENT_CHANNELED_SPELL })
    {
        Spell* spell = caster->GetCurrentSpell(slot);
        if (!spell || !spell->m_spellInfo)
            continue;

        uint8 const prio = InterruptPriority(spell->m_spellInfo->Id);
        if (prio && (!best || prio < best))
            best = prio;
    }

    return best;
}

std::string CastInterrupt(PlayerbotAI* botAI, Unit* target)
{
    if (!target->IsNonMeleeSpellCasted(true))
        return {};

    for (char const* spellName : INTERRUPT_SPELLS)
    {
        std::string const name = spellName;
        uint32 const spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", name)->Get();
        if (!spellId || !StopsACast(spellId))
            continue;

        if (!botAI->CanCastSpell(name, target))
            continue;

        if (botAI->CastSpell(name, target))
            return name;
    }

    return {};
}

bool BladesOfLightActive(Player* bot)
{
    // Already carried by the vortex: movement is not ours any more.
    if (bot->HasAura(SPELL_BLADES_OF_LIGHT_GRAB))
        return false;

    // #657 (w50): a holy priest in Spirit of Redemption (27827, MoP: "cannot move", 15 s) is alive to the trigger but
    // every MoveTo is refused (MovementActions.cpp:671 IsMovingAllowed): Zarany's 43 of the 62 FAILED flees.
    if (bot->HasSpiritOfRedemptionAura())
        return false;

    Creature* boss = FindBoss(bot, NPC_ARMSMASTER_HARLAN);
    if (!boss)
        return false;

    return boss->HasAura(SPELL_BLADES_OF_LIGHT) || boss->FindCurrentSpellBySpellId(SPELL_BLADES_OF_LIGHT);
}

namespace
{
// Harlan's Blades of Light route: BladeStormPoints, scarlet_halls.h:43-60 (fork), x / y only.
constexpr float BLADES_ROUTE[][2] = {
    {1201.52f, 438.68f}, {1198.41f, 445.94f}, {1205.38f, 445.08f}, {1213.15f, 453.57f},
    {1218.32f, 446.91f}, {1222.85f, 438.54f}, {1215.36f, 427.42f}, {1200.62f, 425.51f},
    {1188.55f, 436.41f}, {1188.44f, 449.05f}, {1198.83f, 462.78f}, {1218.94f, 459.59f},
    {1223.45f, 444.52f}, {1212.56f, 431.35f}, {1200.78f, 435.94f}, {1206.30f, 443.73f}};
constexpr size_t BLADES_ROUTE_POINTS = sizeof(BLADES_ROUTE) / sizeof(BLADES_ROUTE[0]);

float SegmentDistance(float px, float py, float ax, float ay, float bx, float by)
{
    float const vx = bx - ax;
    float const vy = by - ay;
    float const len2 = vx * vx + vy * vy;
    float t = len2 > 0.0001f ? ((px - ax) * vx + (py - ay) * vy) / len2 : 0.0f;
    t = std::clamp(t, 0.0f, 1.0f);
    return std::hypot(px - (ax + t * vx), py - (ay + t * vy));
}
}

float BladesSafety(Creature* harlan, float x, float y)
{
    float const toBoss = std::hypot(x - harlan->GetPositionX(), y - harlan->GetPositionY());
    float safety = toBoss / BLADES_SAFE_DISTANCE;

    // #657 (w43): every segment of the route counts, not only the next three. He runs the 194 yd route at run speed
    // and it covers the whole room (every spot of the pit, the ring and the entrance landing is within 11 yd of it),
    // so a spot clear of "the next three segments" was on his path a few seconds later: the rogue fled from the ring
    // (1200,465) onto route point 9 (1188.4,449.1) and the mage waited at (1184.9,442.9), 5 yd from it - both were
    // carried off and killed. The clear ground is the entrance corridor west of the room (x < ~1177), where the
    // healer stood and survived. Scanning every segment also drops the "route point nearest to him" lookup, which
    // jumped to the last points (14-15) whenever he crossed the room centre and then saw no route ahead at all.
    float toPath = FLT_MAX;
    for (size_t i = 1; i < BLADES_ROUTE_POINTS; ++i)
        toPath = std::min(toPath, SegmentDistance(x, y, BLADES_ROUTE[i - 1][0], BLADES_ROUTE[i - 1][1],
                                                  BLADES_ROUTE[i][0], BLADES_ROUTE[i][1]));

    return std::min(safety, toPath / BLADES_PATH_CLEAR);
}

Creature* FindBurningBookcase(Player* bot, float& clearDistance)
{
    clearDistance = 0.0f;
    if (bot->HasSpiritOfRedemptionAura())   // #657: cannot move (see BladesOfLightActive)
        return nullptr;

    std::list<Creature*> cases;
    bot->GetCreatureListWithEntryInGrid(cases, NPC_BOOK_CASE, BURNING_BOOKS_SCAN_RANGE);

    float const radius = DUNGEON_MODE(bot, BURNING_BOOKS_RADIUS, BURNING_BOOKS_RADIUS_HEROIC);
    float const clear = radius + BURNING_BOOKS_MARGIN;

    Creature* nearest = nullptr;
    float nearestDist = clear;
    for (Creature* bookcase : cases)
    {
        if (!bookcase || !bookcase->IsAlive() || !bookcase->HasAura(SPELL_BURNING_BOOKS))
            continue;

        float const dist = bot->GetExactDist2d(bookcase);
        if (dist < nearestDist)
        {
            nearest = bookcase;
            nearestDist = dist;
        }
    }

    clearDistance = clear;
    return nearest;
}

bool BookBurnerAimed(Player* bot, Unit*& boss, Unit*& bookcase)
{
    boss = FindBoss(bot, NPC_FLAMEWEAVER_KOEGLER);
    bookcase = nullptr;
    if (!boss)
        return false;

    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_BOOK_BURNER);
    if (!spell)
        return false;

    // GUID, not GetUnitTarget(): the target pointer is not safe before the spell's first update (Spell.h:665).
    bookcase = ObjectAccessor::GetUnit(*bot, spell->m_targets.GetUnitTargetGUID());
    return bookcase && bookcase->GetEntry() == NPC_BOOK_CASE;
}

float BookBurnerBeamPoint(Unit* who, Unit* boss, Unit* bookcase, float& x, float& y)
{
    float const bx = boss->GetPositionX();
    float const by = boss->GetPositionY();
    float const len = boss->GetExactDist2d(bookcase);
    float const angle = boss->GetAngle(bookcase);
    float const dx = std::cos(angle);
    float const dy = std::sin(angle);

    // projection of `who` onto the Koegler -> case line, clamped to the part of the segment the core counts
    float t = (who->GetPositionX() - bx) * dx + (who->GetPositionY() - by) * dy;
    float const hi = std::max(BOOK_BURNER_MIN_FROM_BOSS, len - BOOK_BURNER_MIN_FROM_CASE);
    t = std::min(std::max(t, BOOK_BURNER_MIN_FROM_BOSS), hi);

    x = bx + dx * t;
    y = by + dy * t;
    return who->GetExactDist2d(x, y);
}

Player* BookBurnerInterceptor(Player* bot, Unit* boss, Unit* bookcase)
{
    float const onBeam = BOOK_BURNER_BEAM_HALF_WIDTH - 0.5f;
    Group* group = bot->GetGroup();
    if (!group)
        return bot;

    Player* best = nullptr;
    float bestDist = FLT_MAX;
    Player* tank = nullptr;
    float tankDist = FLT_MAX;
    for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
    {
        Player* member = ref->GetSource();
        if (!member || !member->IsInWorld() || !member->IsAlive() || member->GetMap() != bot->GetMap() ||
            member->GetExactDist2d(boss) > BOOK_BURNER_ELECT_RANGE)
            continue;

        // somebody already covers it (the core shields the case for any living player on the line)
        if (member->IsInBetween(boss, bookcase, onBeam))
            return member;

        if (!GET_PLAYERBOT_AI(member) || member->HasSpiritOfRedemptionAura())
            continue;

        float x = 0.0f;
        float y = 0.0f;
        float const dist = BookBurnerBeamPoint(member, boss, bookcase, x, y);
        if (dist < bestDist)
        {
            best = member;
            bestDist = dist;
        }

        if (PlayerBotSpec::IsTank(member) && dist < tankDist)
        {
            tank = member;
            tankDist = dist;
        }
    }

    if (tank && tankDist <= bestDist + BOOK_BURNER_TANK_SLACK)
        return tank;

    return best;
}

bool InDragonBreathCone(Player* bot, Unit*& boss)
{
    boss = nullptr;
    if (bot->HasSpiritOfRedemptionAura())   // #657: cannot move (see BladesOfLightActive)
        return false;

    boss = FindBoss(bot, NPC_FLAMEWEAVER_KOEGLER);
    if (!boss || !boss->FindCurrentSpellBySpellId(SPELL_GREATER_DRAGONS_BREATH))
        return false;

    if (bot->GetExactDist2d(boss) > BREATH_REACT_RANGE)
        return false;

    // Smallest angle between the boss's facing and the direction to the bot, 0..pi. Read live each tick: whether the
    // sweep rotates the boss or not, "behind him" is the one place the frontal cone never reaches.
    float const toBot = boss->GetAngle(bot);
    float const diff = std::fabs(Position::NormalizeOrientation(toBot - boss->GetOrientation() + float(M_PI)) - float(M_PI));
    return diff < BREATH_FRONT_HALF_ANGLE;
}
}
