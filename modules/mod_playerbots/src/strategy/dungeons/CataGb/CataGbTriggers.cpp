/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

#include "CataGbTriggers.h"
#include "CataGbShared.h"
#include "Playerbots.h"

bool GbUmbrissGroundSiegeTrigger::IsActive()
{
    return CataGb::GroundSiegeNeeded(bot);
}

bool GbUmbrissBlitzTrigger::IsActive()
{
    return CataGb::BlitzNeeded(bot);
}

bool GbThrongusPhalanxTrigger::IsActive()
{
    return CataGb::PhalanxNeeded(bot);
}

bool GbThrongusCaveInTrigger::IsActive()
{
    return CataGb::CaveInNeeded(bot);
}

bool GbThrongusFirePatchTrigger::IsActive()
{
    return CataGb::FirePatchNeeded(bot);
}

bool GbValionaBreathTrigger::IsActive()
{
    return CataGb::ValionaBreathNeeded(bot);
}

bool GbDrahgaPortalTrigger::IsActive()
{
    return CataGb::FlamePortalNeeded(bot);
}

bool GbDrahgaKiteTrigger::IsActive()
{
    return CataGb::SpiritKiteNeeded(bot);
}

bool GbDrahgaSpiritTrigger::IsActive()
{
    return CataGb::FlamingSpirit(bot) != nullptr;
}

bool GbDrahgaSeepingTwilightTrigger::IsActive()
{
    return CataGb::SeepingTwilightNeeded(bot);
}

bool GbErudaxGaleTrigger::IsActive()
{
    return CataGb::ShadowGaleNeeded(bot);
}

bool GbErudaxShadowsTrigger::IsActive()
{
    return CataGb::BindingShadowsNeeded(bot);
}

bool GbErudaxCorruptorTrigger::IsActive()
{
    return CataGb::FacelessCorruptor(bot) != nullptr;
}
