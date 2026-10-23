/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - actions (issue #673, epic #373).

#include "CataHotActions.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>

// Same call as MgT's escape move (MgTActions.cpp MgTEscapeAction::Execute): combat priority, short delay.
bool CataHotMovementAction::StepTo(float x, float y, float z)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataHotMovementAction::LeaveHazards(std::vector<HourOfTwilight::Hazard> const& hazards)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!HourOfTwilight::FindHazardEscape(bot, hazards, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataHotEscapeAction::Execute(Event /*event*/)
{
    std::vector<HourOfTwilight::Hazard> hazards;
    HourOfTwilight::CollectHazards(bot, _kind, hazards);
    return LeaveHazards(hazards);
}

bool CataHotWaveDodgeAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!HourOfTwilight::GetWaveEscape(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

// AC PoS TyrannusAction::RangedSpread (PoSActions.cpp:292-312): small increments so the course can be corrected.
bool CataHotShearSpreadAction::Execute(Event /*event*/)
{
    constexpr float SPREAD_RADIUS = 11.0f;

    Player* neighbour = HourOfTwilight::GetNearestShearVictimNeighbour(bot);
    if (!neighbour)
        return false;

    float const dist = bot->GetExactDist2d(neighbour);
    float const moveDistance = std::min(4.0f, SPREAD_RADIUS - dist + 1.0f);
    return FleePosition(neighbour->GetPosition(), moveDistance, 250U);
}

bool CataHotKnifeCoverAction::Execute(Event /*event*/)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!HourOfTwilight::NeedsKnifeCover(bot) || !HourOfTwilight::GetKnifeCoverSpot(bot, x, y, z))
        return false;

    return StepTo(x, y, z);
}

// 1.5 yd short of Thrall on the bot's side, so the group stacks around him and not on one point (the pulse reaches 4 yd).
bool CataHotWaterShellAction::Execute(Event /*event*/)
{
    Creature* thrall = HourOfTwilight::GetShellThrall(bot);
    if (!thrall)
        return false;

    float const dx = bot->GetPositionX() - thrall->GetPositionX();
    float const dy = bot->GetPositionY() - thrall->GetPositionY();
    float const len = std::hypot(dx, dy);
    if (len < 2.5f)
        return false;

    return StepTo(thrall->GetPositionX() + dx / len * 1.5f, thrall->GetPositionY() + dy / len * 1.5f, thrall->GetPositionZ());
}

bool CataHotKillTombAction::Execute(Event /*event*/)
{
    Creature* tomb = HourOfTwilight::GetIcyTomb(bot);
    if (!tomb)
        return false;

    return Attack(tomb);
}

bool CataHotAssistThrallAction::Execute(Event /*event*/)
{
    Unit* ambusher = HourOfTwilight::GetEscortAmbusher(bot);
    if (!ambusher)
        return false;

    return Attack(ambusher);
}
