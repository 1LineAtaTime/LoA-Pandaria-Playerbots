/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Shadowfang Keep (epic #373, issue #668). See CataSfkShared.h for sources and ids.

#include "CataSfkActions.h"
#include "CataSfkShared.h"
#include "EncounterHelpers.h"
#include "Map.h"
#include "Playerbots.h"
#include "Random.h"
#include <cmath>

using namespace EncounterHelpers;

bool SfkInterruptPainAndSufferingAction::Execute(Event /*event*/)
{
    Unit* caster = CataSfk::AshburyInterruptTarget(botAI, bot);
    if (!caster)
        return false;

    return !CataSfk::CastInterrupt(botAI, caster).empty();
}

bool SfkKillWorgenSpiritAction::Execute(Event /*event*/)
{
    Unit* target = CataSfk::WorgenSpiritFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataSfk::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}

bool SfkInterruptWorgenAction::Execute(Event /*event*/)
{
    Unit* caster = CataSfk::WorgenInterruptTarget(botAI, bot);
    if (!caster)
        return false;

    return !CataSfk::CastInterrupt(botAI, caster).empty();
}

bool SfkLeaveDesecrationAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataSfk::DesecrationDanger(botAI, bot, pos, radius))
        return false;

    // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69
    return FleePosition(pos, radius);
}

bool SfkInterruptEmpowermentAction::Execute(Event /*event*/)
{
    Unit* caster = CataSfk::EmpowermentInterruptTarget(botAI, bot);
    if (!caster)
        return false;

    return !CataSfk::CastInterrupt(botAI, caster).empty();
}

bool SfkLeaveShieldConeAction::Execute(Event /*event*/)
{
    Unit* source = CataSfk::ShieldConeBoss(botAI, bot);
    if (!source)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataSfk::LateralPoint(bot, source, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool SfkLeavePoisonMixtureAction::Execute(Event /*event*/)
{
    Position pos;
    float radius = 0.0f;
    if (!CataSfk::PoisonMixtureDanger(botAI, bot, pos, radius))
        return false;

    // A 16.5 yd splash and a 1.5 s cast: FleePosition's steps (AiPlayerbot.FleeDistance = 5, once a second) cannot get
    // out in time, so one straight run to `radius` yd from the danger point, above the keep-moving hops (FORCED).
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataSfk::AwayPoint(bot, pos, radius, x, y, z))
        return FleePosition(pos, radius);  // AC idiom: FoSActions.cpp:15-18, UBActions.cpp:61-69

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool SfkKeepMovingAction::Execute(Event /*event*/)
{
    if (!CataSfk::KeepMovingDanger(botAI, bot))
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // Keep moving: a short hop in a random direction (the trigger only fires while the bot stands still)
    float const angle = frand(0.0f, 2.0f * float(M_PI));
    float x = bot->GetPositionX() + std::cos(angle) * CataSfk::KEEP_MOVING_STEP;
    float y = bot->GetPositionY() + std::sin(angle) * CataSfk::KEEP_MOVING_STEP;
    float z = bot->GetPositionZ();
    if (!bot->GetMap()->CheckCollisionAndGetValidCoords(bot, bot->GetPositionX(), bot->GetPositionY(),
                                                        bot->GetPositionZ(), x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

// AC HodirBitingColdJumpAction::Execute / isUseful (UldActions.cpp:1452-1489): with the raid cheat the stacks are dropped
bool SfkCoagulantJumpAction::Execute(Event /*event*/)
{
    bot->RemoveAurasDueToSpell(CataSfk::SPELL_TOXIC_COAGULANT);
    return true;
}

bool SfkCoagulantJumpAction::isUseful()
{
    return CataSfk::HasRaidCheat();
}

bool SfkInterruptCursedBulletsAction::Execute(Event /*event*/)
{
    Unit* caster = CataSfk::CursedBulletsInterruptTarget(botAI, bot);
    if (!caster)
        return false;

    return !CataSfk::CastInterrupt(botAI, caster).empty();
}

bool SfkLeaveBarrageConeAction::Execute(Event /*event*/)
{
    Unit* source = CataSfk::BarrageConeBoss(botAI, bot);
    if (!source)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataSfk::ConeEscapePoint(bot, source, CataSfk::BARRAGE_CONE_HALF_ANGLE, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_COMBAT);
}

bool SfkKillGhoulAction::Execute(Event /*event*/)
{
    Unit* target = CataSfk::GhoulFocus(botAI, bot);
    if (!target)
        return false;

    // AC FoSActions.cpp:24-52: one bot marks the priority target with the skull, then everyone attacks it
    if (IsMechanicTrackerBot(bot, CataSfk::MAP_ID))
        MarkTargetWithSkull(bot, target);

    return Attack(target);
}
