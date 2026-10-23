/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataGbMultipliers.h"
#include "Action.h"
#include "CataGbActions.h"
#include "CataGbShared.h"
#include "FollowActions.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"

float GbCaveInApproachMultiplier::GetValue(Action* action)
{
    if (!action || dynamic_cast<GbMoveAction*>(action))
        return 1.0f;

    std::string const name = action->getName();
    bool const gapCloser = name == "charge" || name == "heroic leap" || name == "intercept" || name == "feral charge" ||
                           name == "feral charge - bear" || name == "feral charge - cat" || name == "wild charge" ||
                           name == "shadowstep" || name == "roll gap" || name == "killing spree";
    bool const follow = dynamic_cast<FollowAction*>(action) != nullptr;
    bool const approach = dynamic_cast<ReachTargetAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action) ||
                          dynamic_cast<CombatFormationMoveAction*>(action);
    if (!gapCloser && !follow && !approach)
        return 1.0f;

    uint32 const now = getMSTime();
    if (now - lastCheck >= 150)
    {
        lastCheck = now;
        approachBlocked = false;
        behindBlocked = false;
        ringInCircle = false;
        masterInCircle = false;
        Creature* boss = CataGb::FindEngagedBoss(bot, CataGb::NPC_FORGEMASTER_THRONGUS);
        if (boss)
        {
            Unit* target = AI_VALUE(Unit*, "current target");
            if (target == boss || bot->GetVictim() == boss)
            {
                approachBlocked = !PlayerBotSpec::IsRanged(bot) && CataGb::CaveInApproachBlocked(bot, boss); // ranged stop at spell range
                behindBlocked = CataGb::CaveInApproachBlocked(bot, boss, true);
                Creature* engaged = nullptr;
                ringInCircle = CataGb::ThrongusAtCaveIn(bot, engaged, boss->GetCombatReach() + bot->GetCombatReach());
            }

            Player* master = botAI->GetMaster();
            masterInCircle = master && master != bot && master->GetMapId() == bot->GetMapId() &&
                             CataGb::InCaveInCircle(bot, master->GetPositionX(), master->GetPositionY());
        }
    }

    if (follow)
        return masterInCircle ? 0.0f : 1.0f;

    if (name == "killing spree")
        return ringInCircle ? 0.0f : 1.0f;

    if (dynamic_cast<SetBehindTargetAction*>(action))
        return behindBlocked ? 0.0f : 1.0f;

    return approachBlocked ? 0.0f : 1.0f;
}

float GbMovementLockMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<GbMoveAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (now - lastCheck >= 150)
    {
        lastCheck = now;
        locked = CataGb::AnyMovementLock(bot);
    }

    return locked ? 0.0f : 1.0f;
}
