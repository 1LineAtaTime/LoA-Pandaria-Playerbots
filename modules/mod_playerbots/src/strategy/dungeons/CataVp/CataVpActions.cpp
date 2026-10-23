/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataVpActions.h"
#include "CataVpShared.h"
#include "LastMovementValue.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include <cmath>

bool VpMoveAction::GoTo(float x, float y, MovementPriority priority)
{
    return MoveTo(bot->GetMapId(), x, y, CataVp::GroundZ(bot, x, y), false, false, false, true, priority);
}

bool VpErtanRingAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataVp::ErtanRingTarget(bot, x, y))
        return false;

    // w64 (#663, 124 FAILED / 70 OK): the target is always her own spot, so MoveTo refused every repeat as a duplicate
    // (MovementActions.cpp IsDuplicateMove: same point within MaxWaitForMove 5 s) or as still waiting for the last move.
    // A bot still walking there is on its way: report that, not a failure. A bot standing still away from her was put
    // there by something else (blink 17-20 yd out, a cyclone's -60% slow cutting the walk short): drop the stale record
    // so the walk is re-issued at once instead of up to 5 s later (w64: the mage stood 6 s at 17.2 yd under Storm's Edge).
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (std::hypot(last.lastMoveToX - x, last.lastMoveToY - y) < 1.0f)
    {
        if (bot->isMoving())
            return true;

        last.clear();
    }

    return GoTo(x, y);
}

bool VpAltairusBreathAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataVp::BreathTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool VpAltairusTwisterAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataVp::TwisterTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool VpAltairusDownwindAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataVp::WindTarget(bot, x, y))
        return false;

    return GoTo(x, y, MovementPriority::MOVEMENT_COMBAT);
}

bool VpAsaadStackAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataVp::StackTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool VpSlipstreamAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    Creature* slipstream = nullptr;
    if (!CataVp::SlipstreamTarget(bot, x, y, z, slipstream))
        return false;

    // click range by surface distance (IsWithinDistInMap), the way MoveTo arrives - an exact-distance test never clicks
    if (slipstream && bot->IsWithinDistInMap(slipstream, CataVp::SLIPSTREAM_CLICK_RANGE))
    {
        if (bot->isMoving())
            bot->StopMoving();

        // no npc_spellclick_spells row: the script's OnSpellClick sets the result when it seats the bot on a carrier
        return slipstream->HandleSpellClick(bot);
    }

    // as VpErtanRingAction: a bot still walking to the same spot is on its way (MoveTo refuses the repeat as a duplicate)
    LastMovement& last = AI_VALUE(LastMovement&, "last movement");
    if (bot->isMoving() && std::hypot(last.lastMoveToX - x, last.lastMoveToY - y) < 1.0f)
        return true;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}
