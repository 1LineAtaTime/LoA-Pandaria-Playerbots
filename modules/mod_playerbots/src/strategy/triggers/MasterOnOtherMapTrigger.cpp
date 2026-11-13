/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "MasterOnOtherMapTrigger.h"

#include "Playerbots.h"
#include "Timer.h"

bool MasterOnOtherMapTrigger::IsActive()
{
    if (!sPlayerbotAIConfig->followTeleport)
        return false;

    // Random bots without a master never get here
    Player* master = botAI->GetMaster();
    if (!master || master == bot)
    {
        tracking = false;
        return false;
    }

    // A master in transit or dead (its ghost is outside at a graveyard) is not followed; the wait starts again
    // once the master is back in the world and alive
    if (!master->IsInWorld() || master->IsBeingTeleported() || !master->IsAlive())
    {
        tracking = false;
        return false;
    }

    uint32 const now = getMSTime();
    uint32 const mapId = master->GetMapId();
    uint32 const instanceId = master->GetInstanceId();
    if (!tracking || mapId != masterMapId || instanceId != masterInstanceId)
    {
        tracking = true;
        masterMapId = mapId;
        masterInstanceId = instanceId;
        masterArrivalTime = now;
    }

    if (bot->GetMapId() == mapId && bot->GetInstanceId() == instanceId)
        return false;

    // The dead engine always has the follow strategy; a bot put on stay stays behind when it dies too
    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return false;

    return getMSTimeDiff(masterArrivalTime, now) >= sPlayerbotAIConfig->followTeleportDelay;
}
