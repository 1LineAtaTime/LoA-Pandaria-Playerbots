/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sh" (Scarlet Halls, epic #373, issue #657). See MopShShared.h for sources.
// Same shape as AC Ai/Dungeon/HoS/HoSMultipliers.cpp:14-45: veto every MovementAction except the strategy's own.

#include "MopShMultipliers.h"
#include "Action.h"
#include "MopShActions.h"
#include "MopShShared.h"
#include "MovementActions.h"
#include "Playerbots.h"

using namespace MopScarletHalls;

float MopShBladesMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopShFleeBladesAction*>(action))
        return 1.0f;

    return BladesOfLightActive(bot) ? 0.0f : 1.0f;
}

float MopShBookBurnerMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopShInterceptBookBurnerAction*>(action) ||
        dynamic_cast<MopShBehindKoeglerAction*>(action) || dynamic_cast<MopShLeaveBurningBooksAction*>(action) ||
        dynamic_cast<MopShFleeBladesAction*>(action))
        return 1.0f;

    Unit* boss = nullptr;
    Unit* bookcase = nullptr;
    if (!BookBurnerAimed(bot, boss, bookcase))
        return 1.0f;

    return BookBurnerInterceptor(bot, boss, bookcase) == bot ? 0.0f : 1.0f;
}

float MopShDragonBreathMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopShBehindKoeglerAction*>(action))
        return 1.0f;

    Unit* boss = nullptr;
    return InDragonBreathCone(bot, boss) ? 0.0f : 1.0f;
}
