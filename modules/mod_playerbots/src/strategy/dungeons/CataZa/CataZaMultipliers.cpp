/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Zul'Aman (Cataclysm 5-man) multipliers, MoP-only lane L7 (#669). Pattern: AC Ai/Dungeon/HoS/HoSMultipliers.cpp and
// MgTMultipliers.cpp (the same "competing movement" and "target picker" action sets).

#include "CataZaMultipliers.h"
#include "Action.h"
#include "CataZaShared.h"
#include "ChooseTargetActions.h"
#include "FollowActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include <initializer_list>
#include <vector>

namespace Za = CataZulAman;

namespace
{
// MgTMultipliers.cpp MgTDampeningFieldMultiplier / MgTGravityLapseMultiplier: the actions that walk a bot toward the
// group, its target or its formation slot.
bool ZaIsCompetingMovement(Action* action)
{
    return dynamic_cast<CombatFormationMoveAction*>(action) || dynamic_cast<FollowAction*>(action) ||
           dynamic_cast<AvoidAoeAction*>(action) || dynamic_cast<ReachTargetAction*>(action) ||
           dynamic_cast<CastReachTargetSpellAction*>(action);
}

bool ZaIsTargetPicker(Action* action)
{
    return dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
           dynamic_cast<TankAssistAction*>(action);
}
}

bool CataZaCachedMultiplier::Cached()
{
    if (!_valid || GetMSTimeDiffToNow(_stamp) >= 100)
    {
        _value = Compute();
        _stamp = getMSTime();
        _valid = true;
    }

    return _value;
}

bool CataZaHazardMultiplier::Compute()
{
    if (!bot->IsInCombat() || bot->GetMapId() != Za::MAP_ZUL_AMAN)
        return false;

    for (Za::HazardKind kind : { Za::HazardKind::FireBombs, Za::HazardKind::FlameBreath, Za::HazardKind::PillarOfFlame,
                                 Za::HazardKind::DaakaraWhirlwind, Za::HazardKind::MalacrassGround,
                                 Za::HazardKind::FeatherCyclone })
    {
        std::vector<Za::Hazard> hazards;
        Za::CollectHazards(bot, kind, hazards);
        if (!hazards.empty() &&
            Za::MinSlack(hazards, bot->GetPositionX(), bot->GetPositionY()) < Za::CLEAR_MARGIN)
            return true;
    }

    return false;
}

float CataZaHazardMultiplier::GetValue(Action* action)
{
    if (!ZaIsCompetingMovement(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZaStormMultiplier::Compute()
{
    // The victim (aura 43648) cannot move anyway; everybody else stays with the eye.
    return Za::IsStormActive(bot) && !bot->HasAura(Za::SPELL_ELECTRICAL_STORM);
}

float CataZaStormMultiplier::GetValue(Action* action)
{
    if (!ZaIsCompetingMovement(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZaSpreadMultiplier::Compute()
{
    Player* nearest = nullptr;
    return Za::NeedsSpread(bot, nearest);
}

float CataZaSpreadMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZaSurgeMultiplier::Compute()
{
    if (bot->GetMapId() != Za::MAP_ZUL_AMAN || !bot->IsInCombat() || PlayerBotSpec::IsTank(bot) ||
        !bot->HasAura(Za::SPELL_SURGE))
        return false;

    return Za::FindBoss(bot, Za::NPC_NALORAKK) || Za::FindBoss(bot, Za::NPC_DAAKARA);
}

float CataZaSurgeMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CombatFormationMoveAction*>(action) && !dynamic_cast<FollowAction*>(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}

bool CataZaFocusMultiplier::Compute()
{
    return Za::GetFocusTarget(bot) != nullptr;
}

float CataZaFocusMultiplier::GetValue(Action* action)
{
    if (!ZaIsTargetPicker(action))
        return 1.0f;

    return Cached() ? 0.0f : 1.0f;
}
