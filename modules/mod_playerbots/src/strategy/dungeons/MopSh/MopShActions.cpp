/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.

#include "MopShActions.h"
#include "MopShShared.h"
#include "Creature.h"
#include "Map.h"
#include "MopDungeonShared.h"
#include "Playerbots.h"
#include "Spell.h"
#include <cmath>

using namespace MopScarletHalls;

bool MopShStepAction::StepAtAngle(float angle, float distance)
{
    float x = bot->GetPositionX() + std::cos(angle) * distance;
    float y = bot->GetPositionY() + std::sin(angle) * distance;
    float z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MopShLeaveKnifeLineAction::Execute(Event /*event*/)
{
    Unit* boss = FindBoss(bot, NPC_HOUNDMASTER_BRAUN);
    if (!boss)
        return false;

    Spell* spell = boss->FindCurrentSpellBySpellId(SPELL_PIERCING_THROW);
    if (!spell)
        return false;

    Unit* victim = botAI->GetUnit(spell->m_targets.GetUnitTargetGUID());
    if (!victim || victim == bot || !bot->IsInBetween(boss, victim, KNIFE_LINE_HALF_WIDTH))
        return false;

    // Step off the line to the side the bot is already on (shortest way out), perpendicular to boss->victim.
    float const lineAngle = boss->GetAngle(victim);
    float const rel = Position::NormalizeOrientation(boss->GetAngle(bot) - lineAngle);
    float const side = rel < float(M_PI) ? 1.0f : -1.0f;
    return StepAtAngle(lineAngle + side * float(M_PI_2), KNIFE_LINE_HALF_WIDTH + KNIFE_LINE_CLEARANCE);
}

bool MopShFleeBladesAction::Execute(Event /*event*/)
{
    Creature* boss = FindBoss(bot, NPC_ARMSMASTER_HARLAN);
    if (!boss)
        return false;

    if (BladesSafety(boss, bot->GetPositionX(), bot->GetPositionY()) >= 1.0f)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // #781: the spot that is safest from Harlan and his route ahead (scored 0..1+, see BladesSafety). Taken at once only
    // when it is really clear (>= 1), e.g. from the entrance landing.
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    auto const score = [boss](float px, float py) { return std::min(BladesSafety(boss, px, py), 1.5f); };
    bool const local = MopDungeon::FindBestSpot(bot, score, 0.1f, x, y, z);
    if (local && BladesSafety(boss, x, y) >= 1.0f &&
        MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
        return true;

    // #657 (w50): no clear spot nearby (the pit and the ring are all within 11 yd of the route): walk the path-found way
    // out to the entrance corridor. The old fallback below returned false whenever Harlan was 16+ yd away, which is
    // most of his lap: Beny (pit ring, 1211.8,428.6) failed 19 of 19 and was carried off.
    for (BladesEscapeSpot const& spot : BLADES_ESCAPE_SPOTS)
    {
        float sz = bot->GetMapWaterOrGroundLevel(spot.x, spot.y, spot.z + 1.0f);
        if (sz <= INVALID_HEIGHT)
            sz = spot.z;

        if (MoveTo(bot->GetMapId(), spot.x, spot.y, sz, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
            return true;
    }

    // no path out: the best partial spot nearby, then the old flee from his current position
    if (local && MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT))
        return true;

    float const dist = bot->GetExactDist2d(boss);
    if (dist >= BLADES_SAFE_DISTANCE)
        return false;

    if (FleePosition(boss->GetPosition(), BLADES_SAFE_DISTANCE, 500U))
        return true;

    return MoveAway(boss, BLADES_SAFE_DISTANCE - dist + 1.0f);
}

bool MopShKoeglerInterruptAction::Execute(Event /*event*/)
{
    Unit* boss = FindBoss(bot, NPC_FLAMEWEAVER_KOEGLER);
    if (!boss || !CurrentCastPriority(boss))
        return false;

    return !CastInterrupt(botAI, boss).empty();
}

bool MopShLeaveBurningBooksAction::Execute(Event /*event*/)
{
    float clearDistance = 0.0f;
    Creature* bookcase = FindBurningBookcase(bot, clearDistance);
    if (!bookcase)
        return false;

    if (FleePosition(bookcase->GetPosition(), clearDistance, 500U))
        return true;

    return MoveAway(bookcase, clearDistance - bot->GetExactDist2d(bookcase) + 1.0f);
}

bool MopShInterceptBookBurnerAction::Execute(Event /*event*/)
{
    Unit* boss = nullptr;
    Unit* bookcase = nullptr;
    if (!BookBurnerAimed(bot, boss, bookcase))
        return false;

    // #683 w106: the nearest point of the beam (it was a fixed spot 5 yd from Koegler, 20+ yd from a tank on an add), at FORCED
    // priority so a pending combat move (charge, reach) does not hold it for its whole delay - the cast is 3 s.
    float x = 0.0f;
    float y = 0.0f;
    BookBurnerBeamPoint(bot, boss, bookcase, x, y);
    float z = bot->GetMapWaterOrGroundLevel(x, y, boss->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = boss->GetPositionZ();

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_FORCED);
}

bool MopShBehindKoeglerAction::Execute(Event /*event*/)
{
    Unit* boss = nullptr;
    if (!InDragonBreathCone(bot, boss))
        return false;

    // The point BREATH_BEHIND_DISTANCE behind Koegler's current facing; recomputed every tick.
    float const behind = boss->GetOrientation() + float(M_PI);
    float const x = boss->GetPositionX() + std::cos(behind) * BREATH_BEHIND_DISTANCE;
    float const y = boss->GetPositionY() + std::sin(behind) * BREATH_BEHIND_DISTANCE;
    float z = bot->GetMapWaterOrGroundLevel(x, y, boss->GetPositionZ());
    if (z <= INVALID_HEIGHT)
        z = boss->GetPositionZ();

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
}
