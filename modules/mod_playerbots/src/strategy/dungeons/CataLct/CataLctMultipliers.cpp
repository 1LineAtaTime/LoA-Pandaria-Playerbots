/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Lost City of the Tol'vir (epic #373, issue #666). See CataLctShared.h for sources and ids.

#include "CataLctMultipliers.h"
#include "CataLctActions.h"
#include "CataLctShared.h"
#include "ChooseTargetActions.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "Timer.h"

float LctAvoidMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<LctAvoidAction*>(action))
        return 1.0f;

    // The ground-effect scan is a grid search: do it at most every 150 ms per bot, not once per candidate action
    uint32 const now = getMSTime();
    if (!_lastCheck || GetMSTimeDiffToNow(_lastCheck) >= 150)
    {
        _danger = CataLct::InDanger(botAI, bot);
        _lastCheck = now;
    }

    return _danger ? 0.0f : 1.0f;
}

// #666 w69 barim.log: the mage's target alternated Soul Fragment / Harbinger every probe ("lct kill soul fragment" set the
// fragment, the next tick "dps assist" put the Harbinger back - the fragment is passive, so it is never in "attackers"), it
// finished almost no cast, 14 of 16 fragments merged (82263 cast=14: +41.6k heal and +20% damage each) and the group wiped
// at 49%. The tank ("lct kill harbinger" 197 OK / 276 FAILED) ping-ponged the same way with "tank assist".
float LctFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);
    if (!picker && !dynamic_cast<DropTargetAction*>(action))
        return 1.0f;

    if (bot->GetMapId() != CataLct::MAP_ID)
        return 1.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target || !target->IsAlive())
        return 1.0f;

    uint32 const entry = target->GetEntry();
    return (entry == CataLct::NPC_SOUL_FRAGMENT || entry == CataLct::NPC_HARBINGER_OF_DARKNESS) ? 0.0f : 1.0f;
}
