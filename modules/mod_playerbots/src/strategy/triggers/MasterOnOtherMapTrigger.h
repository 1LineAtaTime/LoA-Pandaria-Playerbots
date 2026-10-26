/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_MASTERONOTHERMAPTRIGGER_H
#define _PLAYERBOT_MASTERONOTHERMAPTRIGGER_H

#include "Trigger.h"

class PlayerbotAI;

// Active when the master has been on another map or instance than the bot for AiPlayerbot.FollowTeleportDelay,
// e.g. after taking a dungeon portal. Only the bot reads the master here; nothing is written to the master.
class MasterOnOtherMapTrigger : public Trigger
{
public:
    MasterOnOtherMapTrigger(PlayerbotAI* botAI) : Trigger(botAI, "master on other map") {}

    bool IsActive() override;

private:
    // Where the master was last seen and since when, to wait for the master to finish loading in
    bool tracking = false;
    uint32 masterMapId = 0;
    uint32 masterInstanceId = 0;
    uint32 masterArrivalTime = 0;
};

#endif
