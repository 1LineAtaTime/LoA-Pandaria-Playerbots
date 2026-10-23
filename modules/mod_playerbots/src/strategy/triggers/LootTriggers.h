/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Trigger/LootTriggers.h (issue #151). LootAvailableTrigger
 * itself stays declared in GenericTriggers.h - see the class comment there - to avoid the ~21-TU
 * GenericTriggers.h rebuild a move would cost; only its .cpp body changes (GenericTriggers.cpp).
 */

#ifndef _PLAYERBOT_LOOTTRIGGERS_H
#define _PLAYERBOT_LOOTTRIGGERS_H

#include "Trigger.h"

class PlayerbotAI;

// AC LootTriggers.h:22-29.
class FarFromCurrentLootTrigger : public Trigger
{
public:
    FarFromCurrentLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "far from current loot") {}

    bool IsActive() override;
};

// AC LootTriggers.h:30-36.
class CanLootTrigger : public Trigger
{
public:
    CanLootTrigger(PlayerbotAI* botAI) : Trigger(botAI, "can loot") {}

    bool IsActive() override;
};

#endif
