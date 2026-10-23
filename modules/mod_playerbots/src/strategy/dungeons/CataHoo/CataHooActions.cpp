/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataHooActions.h"
#include "CataHooShared.h"
#include "EncounterHelpers.h"
#include "GameObject.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

bool HooMoveAction::GoTo(float x, float y, MovementPriority priority)
{
    return MoveTo(bot->GetMapId(), x, y, CataHoo::GroundZ(bot, x, y), false, false, false, true, priority);
}

bool HooAnhuurBeaconsAction::Execute(Event /*event*/)
{
    GameObject* beacon = nullptr;
    if (!CataHoo::BeaconPlan(bot, beacon))
        return false;

    // Heroic: a beacon takes 10 s to use, so do not start it over on every tick.
    if (bot->IsNonMeleeSpellCasted(true))
        return true;

    if (beacon->IsWithinDistInMap(bot, CataHoo::BEACON_USE_RANGE))
    {
        beacon->Use(bot);
        return true;
    }

    float const x = beacon->GetPositionX();
    float const y = beacon->GetPositionY();
    float const z = beacon->GetPositionZ();
    if (bot->GetPositionZ() > z + CataHoo::BEACON_DROP_HEIGHT)
        return JumpTo(bot->GetMapId(), x, y, z + 0.5f, MovementPriority::MOVEMENT_FORCED);

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, true, MovementPriority::MOVEMENT_FORCED);
}

bool HooAnhuurPitReturnAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!CataHoo::PitReturnTarget(bot, x, y, z))
        return false;

    return JumpTo(bot->GetMapId(), x, y, z + 0.5f, MovementPriority::MOVEMENT_FORCED);
}

bool HooAnhuurSearingLightAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::SearingLightTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooAnhuurReckoningAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::ReckoningTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooPtahQuicksandAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::QuicksandTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooAnraphetAlphaBeamAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::AlphaBeamTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooIsisetSupernovaAction::Execute(Event /*event*/)
{
    if (!CataHoo::SupernovaNeeded(bot))
        return false;

    bot->SetFacingTo(CataHoo::SupernovaFacing(bot));
    return true;
}

bool HooIsisetImageAction::Execute(Event /*event*/)
{
    Creature* image = CataHoo::PriorityImage(bot);
    if (!image)
        return false;

    // Marking only: the bot's own target choice (dps assist) does the attacking, as in AC's FoS action.
    EncounterHelpers::MarkTargetWithSkull(bot, image);
    return false;
}

bool HooAmmunaePodAction::Execute(Event /*event*/)
{
    Creature* pod = CataHoo::PriorityPod(bot);
    if (!pod)
        return false;

    // Marking only: the bot's own target choice (dps assist) does the attacking, as in AC's FoS action.
    EncounterHelpers::MarkTargetWithSkull(bot, pod);
    return false;
}

bool HooAmmunaeSporeCloudAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::SporeCloudTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooSeteshPriorityAction::Execute(Event /*event*/)
{
    Creature* target = CataHoo::PrioritySetesh(bot);
    if (!target)
        return false;

    // Marking only: the bot's own target choice (dps assist) does the attacking, as in AC's FoS action.
    EncounterHelpers::MarkTargetWithSkull(bot, target);
    return false;
}

bool HooRajhSolarWindsAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::SolarWindsTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooSeteshReignOfChaosAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::ReignOfChaosTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooSeteshChaosBlastAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::ChaosBlastTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooRajhSunOrbAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::SunOrbTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}

bool HooRajhInfernoLeapAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    if (!CataHoo::InfernoLeapTarget(bot, x, y))
        return false;

    return GoTo(x, y);
}
