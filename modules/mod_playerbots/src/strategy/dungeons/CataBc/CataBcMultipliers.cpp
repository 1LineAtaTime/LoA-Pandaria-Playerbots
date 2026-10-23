/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy: Blackrock Caverns (epic #373, issue #660). See CataBcShared.h for sources.

#include "CataBcMultipliers.h"
#include "Action.h"
#include "AiObjectContext.h"
#include "CataBcShared.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"
#include "Timer.h"
#include <string>

namespace
{
// Every action of this dungeon is named "cata bc <what>" (CataBcActionContext.h).
bool IsOwnAction(Action* action)
{
    return action->getName().rfind("cata bc ", 0) == 0;
}
}  // namespace

// Same shape as fork strategy/dungeons/HoS/HoSMultipliers.cpp KrystallusMultiplier: while the mechanic applies, every
// movement action except the strategy's own is zeroed (plus the casters' "walk into spell range" action, as in Mech).
float CataBcEvadeMultiplier::GetValue(Action* action)
{
    bool const mover = dynamic_cast<MovementAction*>(action) || dynamic_cast<CastReachTargetSpellAction*>(action);
    if (!mover || IsOwnAction(action))
        return 1.0f;

    uint32 const now = getMSTime();
    if (!_checkedAt || now - _checkedAt >= 100)
    {
        _danger = CataBc::InEvadeDanger(bot);
        _checkedAt = now;
    }

    return _danger ? 0.0f : 1.0f;
}

// Same shape as fork strategy/dungeons/MgT/MgTMultipliers.cpp MgTFocusOrderMultiplier, for the chained bots
float CataBcChainsMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);
    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;
    if (!picker && !drop)
        return 1.0f;

    if (!CataBc::IsChained(bot))
        return 1.0f;

    Creature* chains = CataBc::GetChains(bot);
    if (!chains)
        return 1.0f;

    if (picker)
        return 0.0f;

    return AI_VALUE(Unit*, "current target") == chains ? 0.0f : 1.0f;
}

float CataBcKarshMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || IsOwnAction(action))
        return 1.0f;

    if (!PlayerBotSpec::IsTank(bot))
        return 1.0f;

    Creature* karsh = CataBc::GetKarsh(bot);
    if (!karsh || karsh->GetVictim() != bot)
        return 1.0f;

    // Heated armor: normal tanking. Cooled and Karsh already in the pillar: nothing to wait for either.
    if (CataBc::KarshSuperheatStacks(karsh) != 0 ||
        karsh->GetDistance(CataBc::KARSH_PILLAR_X, CataBc::KARSH_PILLAR_Y, CataBc::KARSH_PILLAR_Z) <=
            CataBc::KARSH_PILLAR_RADIUS + 0.5f)
    {
        _holdSince = 0;
        return 1.0f;
    }

    // Cooled, Karsh still coming: stay on the post until he is in melee range (at most KARSH_HOLD_MS)
    float tx = 0.0f;
    float ty = 0.0f;
    CataBc::GetKarshPost(karsh, tx, ty);
    if (bot->GetExactDist2d(tx, ty) < 4.0f && bot->GetExactDist2d(karsh) > 6.0f)
    {
        uint32 const now = getMSTime();
        if (!_holdSince)
            _holdSince = now;

        return now - _holdSince < CataBc::KARSH_HOLD_MS ? 0.0f : 1.0f;
    }

    _holdSince = 0;
    return 1.0f;
}
