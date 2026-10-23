/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CoSMultipliers.h"
#include "Action.h"
#include "ChooseTargetActions.h"
#include "CoSActions.h"
#include "CoSTriggers.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "Playerbots.h"  // AI_VALUE2 (AC gets it transitively)

// AC Ai/Dungeon/CoS/CoSMultipliers.cpp:15
float EpochMultiplier::GetValue(Action* action)
{
    Unit* boss = AI_VALUE2(Unit*, "find target", "chrono-lord epoch");
    if (!boss) { return 1.0f; }

    if (bot->GetClass() == CLASS_HUNTER) { return 1.0f; }

    if (dynamic_cast<FleeAction*>(action)) { return 0.0f; }

    return 1.0f;
}
