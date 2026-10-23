/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Throne of the Tides (epic #373, issue #661). See CataTottShared.h for sources.

#include "CataTottMultipliers.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "CataTottShared.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "GenericSpellActions.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include <string>

namespace
{
// Every action of this dungeon is named "cata tott <what>" (CataTottActionContext.h).
bool IsOwnAction(Action* action)
{
    return action->getName().rfind("cata tott ", 0) == 0;
}
}  // namespace

// Same shape as fork strategy/dungeons/HoS/HoSMultipliers.cpp KrystallusMultiplier: while the mechanic applies, every
// movement action except the strategy's own is zeroed (plus the casters' "walk into spell range" action, as in Mech).
float CataTottEvadeMultiplier::GetValue(Action* action)
{
    bool const mover = dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!mover || IsOwnAction(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_checkedAt || now - _checkedAt >= 100)
    {
        _danger = CataTott::InEvadeDanger(bot);
        _checkedAt = now;
    }

    return _danger ? 0.0f : 1.0f;
}

// Same shape as fork strategy/dungeons/MgT/MgTMultipliers.cpp MgTFocusOrderMultiplier
float CataTottFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);
    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;
    if (!picker && !drop)
        return 1.0f;

    if (!CataTott::IsFocusDps(bot))
        return 1.0f;

    Unit* focus = CataTott::GetAnyFocusTarget(bot);
    if (!focus)
        return 1.0f;

    if (picker)
        return 0.0f;

    // never drop the focus target itself; dropping any other target is fine
    return AI_VALUE(Unit*, "current target") == focus ? 0.0f : 1.0f;
}

// Absorb Magic (3 s): spell damage on Ghur'sha heals him for three times the amount. Casters stop casting at him for
// the duration; heals, buffs on others and melee are untouched.
float CataTottAbsorbMagicMultiplier::GetValue(Action* action)
{
    if (action->getThreatType() == Action::ActionThreatType::None || dynamic_cast<CastHealingSpellAction*>(action))
        return 1.0f;

    if (!PlayerBotSpec::IsCaster(bot) || PlayerBotSpec::IsHeal(bot))
        return 1.0f;

    if (!bot->IsInCombat() || !CataTott::GhurshaAbsorbing(bot))
        return 1.0f;

    Creature* ghursha = CataTott::FindCreature(bot, CataTott::NPC_GHURSHA, CataTott::SCAN_RANGE);
    if (!ghursha || AI_VALUE(Unit*, "current target") != ghursha)
        return 1.0f;

    return 0.0f;
}
