/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// Well of Eternity bot strategy "cata-woe" - actions (issue #672, epic #373).

#include "CataWoeActions.h"
#include "AiObjectContext.h"
#include "Creature.h"
#include "GameObject.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

// Same call as MgT's escape move (MgTActions.cpp MgTEscapeAction::Execute): combat priority, short delay.
bool CataWoeMovementAction::StepTo(float x, float y, float z)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true, false);
}

bool CataWoeMovementAction::LeaveHazards(std::vector<WellOfEternity::Hazard> const& hazards)
{
    float x = 0.0f, y = 0.0f, z = 0.0f;
    if (!WellOfEternity::FindHazardEscape(bot, hazards, x, y, z))
        return false;

    return StepTo(x, y, z);
}

bool CataWoeEscapeAction::Execute(Event /*event*/)
{
    std::vector<WellOfEternity::Hazard> hazards;
    WellOfEternity::CollectHazards(bot, _kind, hazards);
    return LeaveHazards(hazards);
}

bool CataWoeObedienceInterruptAction::Execute(Event /*event*/)
{
    Unit* azshara = WellOfEternity::GetObedienceTarget(bot);
    if (!azshara)
        return false;

    if (!WellOfEternity::CastInterrupt(botAI, azshara).empty())
        return true;

    // No interrupt could be cast from here: a melee class closes in so its kick / pummel / mind freeze / rebuke is in range
    // on the next tick (the cast takes 10 s).
    if (PlayerBotSpec::IsMelee(bot) && bot->GetExactDist2d(azshara) > 4.0f)
        return ReachCombatTo(azshara, 0.0f);

    return false;
}

bool CataWoeKillHandAction::Execute(Event /*event*/)
{
    Creature* hand = WellOfEternity::GetHandOfTheQueen(bot);
    if (!hand)
        return false;

    return Attack(hand);
}

bool CataWoeUsePortalFocusAction::Execute(Event /*event*/)
{
    GameObject* go = WellOfEternity::GetUsablePortalFocus(bot);
    if (!go)
        return false;

    // the #818 click trap: MoveTo(target, d) stops on the SURFACE distance, so the use test takes the same measure
    // (CataEtFragmentAction, CataEtActions.cpp)
    if (!bot->IsWithinDistInMap(go, INTERACTION_DISTANCE - 1.0f))
        return MoveTo(go, 2.0f, MovementPriority::MOVEMENT_NORMAL);

    // GameObject::Use is what HandleGameObjectUseOpcode reaches (the headless master's CMSG_GAMEOBJ_USE released the
    // w105 crystals this way); the GOOBER's deactivation runs go_npc_well_of_eternity_portal_energy_focus::OnLootStateChanged.
    go->Use(bot);
    return true;
}

bool CataWoeKillDebilitatorAction::Execute(Event /*event*/)
{
    Creature* debilitator = WellOfEternity::GetNearestDebilitator(bot);
    if (!debilitator)
        return false;

    return Attack(debilitator);
}
