/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - multipliers (issue #672, epic #373).

#include "CataWoeMultipliers.h"
#include "Action.h"
#include "CataWoeActions.h"
#include "CataWoeShared.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

namespace
{
constexpr uint32 CACHE_MS = 100;
// A heal on the Fel Decay target is still allowed when it is this low (a life-saving heal beats the 50% backlash).
constexpr float FEL_DECAY_HEAL_BELOW_PCT = 35.0f;
}

bool CataWoeHazardMultiplier::NeedsMove()
{
    uint32 const now = getMSTime();
    if (_stamp && now - _stamp < CACHE_MS)
        return _cached;

    _stamp = now;
    _cached = WellOfEternity::InHazard(bot, WellOfEternity::HazardKind::FelFlames) ||
              WellOfEternity::InHazard(bot, WellOfEternity::HazardKind::EyeOfPerotharn) ||
              WellOfEternity::InHazard(bot, WellOfEternity::HazardKind::ArcaneBomb) ||
              WellOfEternity::InHazard(bot, WellOfEternity::HazardKind::FelFirestorm);
    return _cached;
}

float CataWoeHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<CataWoeMovementAction*>(action))
        return 1.0f;

    return NeedsMove() ? 0.0f : 1.0f;
}

float CataWoeFelDecayMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    Unit* target = action->GetTarget();
    if (!target || !target->HasAura(WellOfEternity::WOE_SPELL_FEL_DECAY))
        return 1.0f;

    if (target->GetHealthPct() < FEL_DECAY_HEAL_BELOW_PCT)
        return 1.0f;

    return 0.0f;
}
