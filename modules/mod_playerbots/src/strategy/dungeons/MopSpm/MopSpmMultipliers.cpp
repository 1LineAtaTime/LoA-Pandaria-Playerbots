/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm multipliers (epic #373, issue #653).

#include "MopSpmMultipliers.h"
#include "MopSpmActions.h"
#include "MopSpmShared.h"
#include "AttackActions.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

float MopSpmAvoidMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopSpmAvoidAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (static_cast<int32>(now - _nextCheck) >= 0)
    {
        _nextCheck = now + MopSpm::DANGER_CACHE_MS;
        _inDanger = MopSpm::CanEscapeDanger(bot);
    }

    return _inDanger ? 0.0f : 1.0f;
}

float MopSpmParryMultiplier::GetValue(Action* action)
{
    uint32 const now = getMSTime();
    if (static_cast<int32>(now - _nextCheck) >= 0)
    {
        _nextCheck = now + MopSpm::DANGER_CACHE_MS;
        _parry = MopSpm::IsParryStanceActive(bot);
    }

    if (!_parry)
        return 1.0f;

    bool const melee = dynamic_cast<AttackAction*>(action) != nullptr;
    bool const harmfulSpell = dynamic_cast<CastSpellAction*>(action) &&
                              !dynamic_cast<CastHealingSpellAction*>(action) &&
                              !dynamic_cast<CastBuffSpellAction*>(action);

    return (melee || harmfulSpell) ? 0.0f : 1.0f;
}
