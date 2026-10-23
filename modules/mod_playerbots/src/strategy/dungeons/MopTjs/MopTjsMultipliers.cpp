/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopTjs multipliers (epic #373, issue #651).

#include "MopTjsMultipliers.h"
#include "MopTjsActions.h"
#include "MopTjsShared.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

float MopTjsAvoidMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopTjsAvoidAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (static_cast<int32>(now - _nextCheck) >= 0)
    {
        _nextCheck = now + MopTjs::DANGER_CACHE_MS;
        _inDanger = MopTjs::CanEscapeDanger(bot);
    }

    return _inDanger ? 0.0f : 1.0f;
}
