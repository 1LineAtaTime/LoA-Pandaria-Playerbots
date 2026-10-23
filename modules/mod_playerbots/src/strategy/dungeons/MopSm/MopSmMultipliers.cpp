/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-sm" (Scarlet Monastery, epic #373, issue #658). See MopSmShared.h for sources.
// Same shape as AC Ai/Dungeon/HoS/HoSMultipliers.cpp:14-45: veto every MovementAction except the strategy's own.

#include "MopSmMultipliers.h"
#include "Action.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "MopSmActions.h"
#include "MopSmShared.h"
#include "MovementActions.h"
#include "Playerbots.h"

using namespace MopScarletMonastery;

float MopSmFirestormMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopSmLeaveFirestormAction*>(action))
        return 1.0f;

    Creature* boss = nullptr;
    return FirestormKickNear(bot, boss) ? 0.0f : 1.0f;
}

float MopSmFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);
    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;
    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    Unit* add = FindFocusAdd(bot);
    if (!add)
        return 1.0f;

    if (!drop)
        return 0.0f;

    // Dropping is only allowed for a target that is not the focus add (or is no longer a valid target).
    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || current != add)
        return 1.0f;

    if (!current->IsAlive() || !AttackersValue::IsValidTarget(current, bot))
        return 1.0f;

    return 0.0f;
}

float MopSmReviveGuardMultiplier::GetValue(Action* action)
{
    // Name test first: it is cheap, the creature lookup is not.
    if (!IsInterruptActionName(action->getName()))
        return 1.0f;

    return WhitemaneIsResurrecting(bot) ? 0.0f : 1.0f;
}

float MopSmScorchedEarthMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopSmLeaveScorchedEarthAction*>(action) ||
        dynamic_cast<MopSmLeaveFirestormAction*>(action))
        return 1.0f;

    float clearDistance = 0.0f;
    return FindNearestHazard(bot, HAZARD_SCORCHED_EARTH, clearDistance) ? 0.0f : 1.0f;
}
