/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Gate of the Setting Sun strategy "mop-gss" (epic #373, issue #655). See MopGssShared.h for sources.

#include "MopGssMultipliers.h"
#include "AiObjectContext.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "MopGssActions.h"
#include "MopGssShared.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"

float MopGssCannonMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<MopGssCannonAction*>(action) || dynamic_cast<MopGssEscapeAction*>(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_checkedAt || getMSTimeDiff(_checkedAt, now) >= 500)
    {
        _checkedAt = now;
        _wantCannon = MopGss::FindCannon(bot) != nullptr;
    }

    return _wantCannon ? 0.0f : 1.0f;
}

float MopGssHazardMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) && !dynamic_cast<CastReachTargetSpellAction*>(action))
        return 1.0f;

    if (dynamic_cast<MopGssEscapeAction*>(action))
        return 1.0f;

    auto const& hazard = AI_VALUE_REF(MopGss::Hazard, "mop gss hazard");
    return hazard.kind == MopGss::HazardKind::None ? 1.0f : 0.0f;
}

float MopGssFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);

    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;

    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    // #655 w65: the tank too - ComputeFocusTarget gives him a focus only when the boss is off him (Ri'mok, phase-2 Raigonn)
    // or in Raigonn's phase 1 (the add pile), and TankAssist must not take him back to the adds / the carapaced body then
    if (PlayerBotSpec::IsHeal(bot) || (!PlayerBotSpec::IsTank(bot) && !PlayerBotSpec::IsDps(bot)))
        return 1.0f;

    ObjectGuid const focus = AI_VALUE(ObjectGuid, "mop gss focus target");
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

float MopGssTankFaceMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<TankFaceAction*>(action))
        return 1.0f;

    Unit* current = AI_VALUE(Unit*, "current target");
    if (current && current->GetEntry() == MopGss::GSS_NPC_RAIGONN)
        return 0.0f;

    return 1.0f;
}
