/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Gurub (Cataclysm 5-man) triggers, MoP-only lane L7 (#670). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.cpp.

#include "CataZgTriggers.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "Playerbots.h"
#include <vector>

namespace Zg = CataZulGurub;

bool CataZgHazardTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    std::vector<Zg::Hazard> hazards;
    Zg::CollectHazards(bot, _kind, hazards);
    if (hazards.empty())
        return false;

    return Zg::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < Zg::STAND_MARGIN;
}

bool CataZgToxicLinkTrigger::IsActive()
{
    return Zg::GetToxicPartner(bot) != nullptr;
}

bool CataZgVenoxisTankSpotTrigger::IsActive()
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    return Zg::GetVenoxisTankSpot(bot, x, y, z);
}

bool CataZgShadowsTrigger::IsActive()
{
    float centreX = 0.0f;
    float centreY = 0.0f;
    float radius = 0.0f;
    return Zg::GetDeadzoneToEnter(bot, centreX, centreY, radius);
}

bool CataZgFocusTrigger::IsActive()
{
    return Zg::GetFocusTarget(bot) != nullptr;
}

bool CataZgCauldronTrigger::IsActive()
{
    Zg::CauldronPlan plan;
    return Zg::GetCauldronPlan(bot, plan);
}

bool CataZgGraveyardGasTrigger::IsActive()
{
    return Zg::GetToxicTormentCauldron(bot) != nullptr;
}

bool CataZgInterruptTrigger::IsActive()
{
    return Zg::GetInterruptTarget(bot) != nullptr;
}

bool CataZgSpiritPullTrigger::IsActive()
{
    return Zg::GetSpiritToPull(bot) != nullptr;
}

bool CataZgChainStandTrigger::IsActive()
{
    Creature* chain = Zg::GetChainToStandAt(bot);
    if (!chain)
        return false;

    // The Gaze target goes onto the chain; everybody else waits on its own spot out of the slam radius (CataZgShared.h CHAIN_WAIT_*).
    if (Zg::IsGazeTarget(bot))
        return bot->GetExactDist2d(chain) > Zg::CHAIN_STAND_RANGE;

    float x = 0.0f;
    float y = 0.0f;
    Zg::GetChainWaitSpot(bot, chain, x, y);
    return bot->GetExactDist2d(x, y) > Zg::CHAIN_WAIT_SLACK;
}

bool CataZgSpareSpiritTrigger::IsActive()
{
    return Zg::ShouldSpareSpirit(bot, AI_VALUE(Unit*, "current target"));
}
