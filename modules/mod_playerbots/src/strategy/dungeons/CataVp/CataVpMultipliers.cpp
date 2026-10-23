/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataVpMultipliers.h"
#include "Action.h"
#include "AttackActions.h"
#include "CataVpActions.h"
#include "CataVpShared.h"
#include "HunterActions.h"
#include "MageActions.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"

float VpMovementLockMultiplier::GetValue(Action* action)
{
    // #663 w64: Blink (both MoP's "blink" and AC's "blink back") and Disengage move the bot too
    bool const movementSpell = dynamic_cast<CastBlinkAction*>(action) || dynamic_cast<CastBlinkBackAction*>(action) ||
                               dynamic_cast<CastDisengageAction*>(action);
    if (!movementSpell && !dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<VpMoveAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (now - lastCheck >= 150)
    {
        lastCheck = now;
        locked = CataVp::AnyMovementLock(bot);
        ertanStack = CataVp::ErtanStackActive(bot);
    }

    if (locked)
        return 0.0f;

    // Ertan: hold the stack under her (AttackAction is a MovementAction but does not move, AC lets it through as well)
    if (ertanStack && !dynamic_cast<AttackAction*>(action))
        return 0.0f;

    return 1.0f;
}
