/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Hour of Twilight bot strategy "cata-hot" - multipliers (issue #673, epic #373).

#include "CataHotMultipliers.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "CataHotActions.h"
#include "CataHotShared.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

namespace
{
constexpr uint32 CACHE_MS = 100;
}

bool CataHotHazardMultiplier::NeedsMove()
{
    uint32 const now = getMSTime();
    if (_stamp && now - _stamp < CACHE_MS)
        return _cached;

    _stamp = now;
    _cached = HourOfTwilight::InHazard(bot, HourOfTwilight::HazardKind::SmokeBomb) ||
              HourOfTwilight::InHazard(bot, HourOfTwilight::HazardKind::LightOrb) || HourOfTwilight::InWavePath(bot) ||
              HourOfTwilight::InHazard(bot, HourOfTwilight::HazardKind::IcyBoulder) || HourOfTwilight::NeedsWaterShell(bot) ||
              HourOfTwilight::NeedsKnifeCover(bot);   // #696
    return _cached;
}

float CataHotHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<CataHotMovementAction*>(action))
        return 1.0f;

    return NeedsMove() ? 0.0f : 1.0f;
}

// Only the generic target pickers, only while the bot is already on the tomb: a bot that could not reach the tomb keeps its
// normal target choice (same picker set as CataTottMultipliers.cpp CataTottFocusMultiplier).
float CataHotIcyTombMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<DpsAssistAction*>(action) && !dynamic_cast<DpsAoeAction*>(action))
        return 1.0f;

    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || !current->IsAlive() || current->GetEntry() != HourOfTwilight::HOT_NPC_ICY_TOMB)
        return 1.0f;

    return HourOfTwilight::NeedsIcyTombKill(bot) ? 0.0f : 1.0f;
}
