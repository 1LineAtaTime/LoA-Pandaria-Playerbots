/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.

#include "MopSmActions.h"
#include "MopSmShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "Map.h"
#include "Playerbots.h"
#include <cmath>

using namespace MopScarletMonastery;

bool MopSmLeaveSpiritGaleAction::StepAtAngle(float angle, float distance)
{
    float x = bot->GetPositionX() + std::cos(angle) * distance;
    float y = bot->GetPositionY() + std::sin(angle) * distance;
    float z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, true, MovementPriority::MOVEMENT_COMBAT);
}

bool MopSmLeaveSpiritGaleAction::Execute(Event /*event*/)
{
    if (!bot->HasAura(SPELL_SPIRIT_GALE_AURA))
        return false;

    // The gale centre is not exposed; it is 2 yd wide and was dropped on a player, so a tangent step around Thalnos
    // (keeps the bot's melee / casting distance to the boss) leaves it. Each bot keeps its own side by guid parity,
    // the other side is the fallback, then straight on.
    Creature* boss = FindBoss(bot, NPC_THALNOS_THE_SOULRENDER);
    float const base = boss ? boss->GetAngle(bot) : bot->GetOrientation();
    float const side = (bot->GetGUID().GetCounter() & 1) ? 1.0f : -1.0f;

    if (StepAtAngle(base + side * float(M_PI_2), SPIRIT_GALE_STEP))
        return true;

    if (StepAtAngle(base - side * float(M_PI_2), SPIRIT_GALE_STEP))
        return true;

    return StepAtAngle(bot->GetOrientation(), SPIRIT_GALE_STEP);
}

bool MopSmPriorityInterruptAction::Execute(Event /*event*/)
{
    Unit* target = FindInterruptTarget(bot);
    if (!target)
        return false;

    return !CastInterrupt(botAI, target).empty();
}

bool MopSmDispelWhitemaneShieldAction::Execute(Event /*event*/)
{
    Unit* target = FindShieldedWhitemane(bot);
    if (!target)
        return false;

    return !CastShieldDispel(botAI, target).empty();
}

bool MopSmFocusAddAction::Execute(Event /*event*/)
{
    Unit* target = FindFocusAdd(bot);
    if (!target)
        return false;

    if (EncounterHelpers::IsMechanicTrackerBot(bot, MAP_SCARLET_MONASTERY))
        EncounterHelpers::MarkTargetWithSkull(bot, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool MopSmLeaveFirestormAction::Execute(Event /*event*/)
{
    Creature* boss = nullptr;
    if (!FirestormKickNear(bot, boss))
        return false;

    float const safe = FIRESTORM_KICK_RADIUS + FIRESTORM_KICK_MARGIN;
    float const dist = bot->GetExactDist2d(boss);
    if (dist >= safe)
        return false;

    if (FleePosition(boss->GetPosition(), safe, 500U))
        return true;

    return MoveAway(boss, safe - dist + 1.0f);
}

bool MopSmLeaveScorchedEarthAction::Execute(Event /*event*/)
{
    float clearDistance = 0.0f;
    Creature* fire = FindNearestHazard(bot, HAZARD_SCORCHED_EARTH, clearDistance);
    if (!fire)
        return false;

    if (FleePosition(fire->GetPosition(), clearDistance, 500U))
        return true;

    return MoveAway(fire, clearDistance - bot->GetExactDist2d(fire) + 1.0f);
}
