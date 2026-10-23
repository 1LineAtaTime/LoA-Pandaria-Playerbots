/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataHooMultipliers.h"
#include "Action.h"
#include "CataHooActions.h"
#include "CataHooShared.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include "GenericSpellActions.h"

float HooMovementLockMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<HooMoveAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (now - lastCheck >= 150)
    {
        lastCheck = now;
        locked = CataHoo::AnyMovementLock(bot);
    }

    return locked ? 0.0f : 1.0f;
}

float HooSupernovaMultiplier::GetValue(Action* action)
{
    if (dynamic_cast<HooIsisetSupernovaAction*>(action) || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastSpellAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (now - lastCheck >= 150)
    {
        lastCheck = now;
        casting = CataHoo::SupernovaCasting(bot);
    }

    return casting ? 0.0f : 1.0f;
}
