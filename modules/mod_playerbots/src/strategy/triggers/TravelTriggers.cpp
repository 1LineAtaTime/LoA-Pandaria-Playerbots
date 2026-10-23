/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Trigger/TravelTriggers.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #269.
 */

#include "TravelTriggers.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "TravelMgr.h"  // 5.4.8: TravelTarget is not pulled in by Playerbots.h on this fork (see strategy/triggers/RpgTriggers.cpp)

bool NoTravelTargetTrigger::IsActive() { return !context->GetValue<TravelTarget*>("travel target")->Get()->isActive(); }

bool FarFromTravelTargetTrigger::IsActive()
{
    return context->GetValue<TravelTarget*>("travel target")->Get()->isTraveling();
}

bool NearDarkPortalTrigger::IsActive() { return bot->GetAreaId() == 72; }

// 5.4.8 divergence: ServerFacade::instance().GetDistance2d -> sServerFacade->GetDistance2d
// (Utils/ServerFacade.h:29, Unit*, float, float).
bool AtDarkPortalAzerothTrigger::IsActive()
{
    if (bot->GetAreaId() == 72)
    {
        if (sServerFacade->GetDistance2d(bot, -11906.9f, -3208.53f) < 20.0f)
        {
            return true;
        }
    }

    return false;
}

bool AtDarkPortalOutlandTrigger::IsActive()
{
    if (bot->GetAreaId() == 3539)
    {
        if (sServerFacade->GetDistance2d(bot, -248.1939f, 921.919f) < 10.0f)
        {
            return true;
        }
    }

    return false;
}
