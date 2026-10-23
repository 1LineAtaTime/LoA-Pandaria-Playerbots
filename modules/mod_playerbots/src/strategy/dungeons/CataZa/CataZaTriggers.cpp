/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) triggers, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSTriggers.cpp.

#include "CataZaTriggers.h"
#include "AiObjectContext.h"
#include "Playerbots.h"
#include <vector>

namespace Za = CataZulAman;

bool CataZaHazardTrigger::IsActive()
{
    if (!bot->IsInCombat())
        return false;

    std::vector<Za::Hazard> hazards;
    Za::CollectHazards(bot, _kind, hazards);
    if (hazards.empty())
        return false;

    return Za::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < Za::STAND_MARGIN;
}

bool CataZaStormTrigger::IsActive()
{
    float centreX = 0.0f;
    float centreY = 0.0f;
    float centreZ = 0.0f;
    float radius = 0.0f;
    if (!Za::GetStormEye(bot, centreX, centreY, centreZ, radius))
        return false;

    return bot->GetExactDist2d(centreX, centreY) > radius * 0.8f;
}

bool CataZaSpreadTrigger::IsActive()
{
    Player* nearest = nullptr;
    return Za::NeedsSpread(bot, nearest);
}

bool CataZaSurgeTrigger::IsActive()
{
    return Za::GetSurgeAnchor(bot) != nullptr;
}

bool CataZaWaterTotemTrigger::IsActive()
{
    Unit* totem = Za::GetWaterTotemToLeave(bot);
    return totem && bot->GetExactDist2d(totem) < Za::WATER_TOTEM_CLEAR + 6.0f;
}

bool CataZaFocusTrigger::IsActive()
{
    return Za::GetFocusTarget(bot) != nullptr;
}

bool CataZaInterruptTrigger::IsActive()
{
    return Za::GetInterruptTarget(bot) != nullptr;
}
