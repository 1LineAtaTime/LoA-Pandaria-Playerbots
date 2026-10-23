/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Trigger/FishingTriggers.h (issue #412).
 */

#ifndef _PLAYERBOT_FISHINGTRIGGERS_H
#define _PLAYERBOT_FISHINGTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

class CanFishTrigger : public Trigger
{
public:
    CanFishTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can fish") {}

    bool IsActive() override;
};

class CanUseFishingBobberTrigger : public Trigger
{
public:
    CanUseFishingBobberTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can use fishing bobber") {}

    bool IsActive() override;
};

#endif
