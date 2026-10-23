/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Deadmines (epic #373, issue #667). See CataDmShared.h for sources and ids.

#include "CataDmMultipliers.h"
#include "CataDmActions.h"
#include "CataDmShared.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

float DmAvoidMultiplier::GetValue(Action* action)
{
    // #667 w68: attack actions pass (AC Ai/Dungeon/UB/UBMultipliers.cpp:19-26, HungarfenFoulSporesMultiplier lets AttackAction
    // through). AttackAction is a MovementAction, so this multiplier zeroed "dm attack helix" (930 IMPOSSIBLE = 930 "made action
    // dm attack helix useless"), "tank assist" (907) and "dps assist" (1479) whenever a bot stood within 4 yd of a bomb: the tank
    // never picked the Oaf up. Picking a target does not walk the bot into the bomb (Attack() only stops a move below
    // MOVEMENT_COMBAT, AttackActions.cpp:170-178); reach / chase / follow stay blocked.
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<DmAvoidAction*>(action) || dynamic_cast<AttackAction*>(action))
        return 1.0f;

    // The ground-effect scan is a grid search: do it at most every 150 ms per bot, not once per candidate action
    uint32 const now = getMSTime();
    if (!_lastCheck || GetMSTimeDiffToNow(_lastCheck) >= 150)
    {
        _danger = CataDm::InDanger(botAI, bot);
        _lastCheck = now;
    }

    return _danger ? 0.0f : 1.0f;
}
