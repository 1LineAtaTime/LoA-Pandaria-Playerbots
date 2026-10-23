/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataHooTriggers.h"
#include "CataHooShared.h"
#include "Playerbots.h"

bool HooAnhuurBeaconsTrigger::IsActive()
{
    return CataHoo::BeaconNeeded(bot);
}

bool HooAnhuurPitReturnTrigger::IsActive()
{
    return CataHoo::PitReturnNeeded(bot);
}

bool HooAnhuurSearingLightTrigger::IsActive()
{
    return CataHoo::SearingLightNeeded(bot);
}

bool HooAnhuurReckoningTrigger::IsActive()
{
    return CataHoo::ReckoningNeeded(bot);
}

bool HooPtahQuicksandTrigger::IsActive()
{
    return CataHoo::QuicksandNeeded(bot);
}

bool HooAnraphetAlphaBeamTrigger::IsActive()
{
    return CataHoo::AlphaBeamNeeded(bot);
}

bool HooIsisetSupernovaTrigger::IsActive()
{
    return CataHoo::SupernovaCasting(bot);
}

bool HooIsisetImageTrigger::IsActive()
{
    return CataHoo::PriorityImage(bot) != nullptr;
}

bool HooAmmunaePodTrigger::IsActive()
{
    return CataHoo::PriorityPod(bot) != nullptr;
}

bool HooAmmunaeSporeCloudTrigger::IsActive()
{
    return CataHoo::SporeCloudNeeded(bot);
}

bool HooSeteshPriorityTrigger::IsActive()
{
    return CataHoo::PrioritySetesh(bot) != nullptr;
}

bool HooRajhSolarWindsTrigger::IsActive()
{
    return CataHoo::SolarWindsNeeded(bot);
}

bool HooSeteshReignOfChaosTrigger::IsActive()
{
    return CataHoo::ReignOfChaosNeeded(bot);
}

bool HooSeteshChaosBlastTrigger::IsActive()
{
    return CataHoo::ChaosBlastNeeded(bot);
}

bool HooRajhSunOrbTrigger::IsActive()
{
    return CataHoo::SunOrbNeeded(bot);
}

bool HooRajhInfernoLeapTrigger::IsActive()
{
    return CataHoo::InfernoLeapNeeded(bot);
}
