/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) actions, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSActions.cpp and
// MgTActions.cpp (escape step with MovementPriority::MOVEMENT_COMBAT, focus attack).

#include "CataZaActions.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <vector>

namespace Za = CataZulAman;

bool CataZaEscapeAction::Execute(Event /*event*/)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    std::vector<Za::Hazard> hazards;
    Za::CollectHazards(bot, _kind, hazards);
    if (hazards.empty())
        return false;

    if (Za::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) >= Za::STAND_MARGIN)
        return false;

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!Za::FindEscapeSpot(bot, hazards, Za::CLEAR_MARGIN, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZaJoinStormEyeAction::Execute(Event /*event*/)
{
    float centreX = 0.0f;
    float centreY = 0.0f;
    float centreZ = 0.0f;
    float radius = 0.0f;
    if (!Za::GetStormEye(bot, centreX, centreY, centreZ, radius))
        return false;

    float const dist = bot->GetExactDist2d(centreX, centreY);
    if (dist <= radius * 0.8f)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // Aim for half way into the eye, on the line from the centre to the bot.
    float const reach = radius * 0.5f;
    float const x = centreX + (bot->GetPositionX() - centreX) / dist * reach;
    float const y = centreY + (bot->GetPositionY() - centreY) / dist * reach;

    // #669 w67: the floor height is probed at the eye's own z (the DynamicObject sits on the platform under the victim).
    // Probing from the bot's z sent a bot standing on the stairs (z 57 vs 74) to the level below the platform and the
    // move failed (87 of 101 "za join storm eye" FAILED).
    float z = bot->GetMapHeight(x, y, centreZ + 3.0f, true, 12.0f);
    if (!(std::fabs(z - centreZ) <= 4.0f))
        z = centreZ;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZaSpreadAction::Execute(Event /*event*/)
{
    Player* nearest = nullptr;
    if (!Za::NeedsSpread(bot, nearest) || !nearest)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    Creature* boss = Za::FindBoss(bot, Za::NPC_AKILZON);
    if (!boss)
        return false;

    // #669 w67: MoveAway from the nearest member walked the ranged and the healer off Akil'zon's platform down the west
    // stairs (z 74 -> 57), out of the healer's reach and into the Amani'shi Tempest 24549 at their foot, which then
    // joined the fight. Pick a spot clear of every member instead, on the boss's level and within reach of him
    // (ring search, as the hazard escapes).
    std::vector<Za::Hazard> others;
    Za::CollectSpreadHazards(bot, others);

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    // #669 w68: the spread also has to stay compact. The ranged settled on opposite sides of Akil'zon (healer 400.8 1398.1,
    // mage 361.0 1386.5: 40 yd apart); when the mage was lifted the healer ran 5 s to the eye and took 4 storm pulses
    // (43657 also stacks +100% nature taken: 16k, 27k, 34k, 45k; 100% -> 1% health). Prefer a spot within
    // SPREAD_MAX_FROM_MEMBER of everybody, else fall back to any spot clear of everybody.
    if (!Za::FindEscapeSpot(bot, others, 0.5f, x, y, z, boss, Za::SPREAD_MAX_FROM_BOSS, Za::SPREAD_MAX_FROM_MEMBER) &&
        !Za::FindEscapeSpot(bot, others, 0.5f, x, y, z, boss, Za::SPREAD_MAX_FROM_BOSS))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZaSurgeCloseAction::Execute(Event /*event*/)
{
    Unit* boss = Za::GetSurgeAnchor(bot);
    if (!boss)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // #669: GetSurgeAnchor measures the CENTRE distance (GetExactDist2d > SURGE_HOLD_DISTANCE), but MoveTo(Unit*, d) stops at
    // a SURFACE distance d (WorldObject::GetDistance minus both combat reaches), which can leave the bot beyond the 10 yd
    // centre threshold for good. Walk to a point SURGE_CLOSE_DISTANCE from the boss centre, on the boss-to-bot line.
    float const dist = bot->GetExactDist2d(boss);
    if (dist < 0.1f)
        return false;

    float const x = boss->GetPositionX() + (bot->GetPositionX() - boss->GetPositionX()) / dist * Za::SURGE_CLOSE_DISTANCE;
    float const y = boss->GetPositionY() + (bot->GetPositionY() - boss->GetPositionY()) / dist * Za::SURGE_CLOSE_DISTANCE;

    float const bz = boss->GetPositionZ();
    float z = bot->GetMapHeight(x, y, bz + 5.0f, true, 12.0f);
    if (!(std::fabs(z - bz) <= 8.0f))
        z = bz;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataZaLeaveWaterTotemAction::Execute(Event /*event*/)
{
    Unit* totem = Za::GetWaterTotemToLeave(bot);
    if (!totem)
        return false;

    // The boss follows the tank, so the tank has to end up a melee reach beyond the totem's healing radius.
    float const need = Za::WATER_TOTEM_CLEAR + 6.0f - bot->GetExactDist2d(totem);
    if (need <= 0.5f)
        return false;

    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveAway(totem, std::min(need, 10.0f));
}

bool CataZaFocusAction::Execute(Event /*event*/)
{
    Unit* target = Za::GetFocusTarget(bot);
    if (!target)
        return false;

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    return Attack(target);
}

bool CataZaInterruptAction::Execute(Event /*event*/)
{
    Unit* target = Za::GetInterruptTarget(bot);
    if (!target)
        return false;

    return Za::CastDedicatedInterrupt(botAI, target);
}
