/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Mogu'shan Palace strategy "mop-msp" (epic #373, issue #654). See MopMspShared.h for sources.

#include "MopMspMultipliers.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "MopMspActions.h"
#include "MopMspShared.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

float MopMspHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<MopMspEscapeAction*>(action))
        return 1.0f;

    auto const& hazard = AI_VALUE_REF(MopMsp::Hazard, "mop msp hazard");
    return hazard.kind == MopMsp::HazardKind::None ? 1.0f : 0.0f;
}

float MopMspFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);

    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;

    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    if (PlayerBotSpec::IsHeal(bot) || PlayerBotSpec::IsTank(bot) || !PlayerBotSpec::IsDps(bot))
        return 1.0f;

    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop msp focus target");
    if (focus.IsEmpty())
        return 1.0f;

    if (!drop)
        return 0.0f;

    // keep the focus target even if the generic code wants to drop it, but drop a dead / invalid one
    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || current->GetGUID() != focus)
        return 1.0f;

    if (!current->IsAlive() || !AttackersValue::IsValidTarget(current, bot))
        return 1.0f;

    return 0.0f;
}
