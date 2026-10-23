/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Trigger/NewRpgTriggers.h)
 * into the Legends of Azeroth Pandaria fork.
 */

#ifndef _PLAYERBOT_NEWRPGTRIGGERS_H
#define _PLAYERBOT_NEWRPGTRIGGERS_H

#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "Trigger.h"

class NewRpgStatusTrigger : public Trigger
{
public:
    NewRpgStatusTrigger(PlayerbotAI* botAI, NewRpgStatus status = RPG_IDLE)
        : Trigger(botAI, "new rpg status"), status(status)
    {
    }
    bool IsActive() override;

protected:
    NewRpgStatus status;
};

#endif
