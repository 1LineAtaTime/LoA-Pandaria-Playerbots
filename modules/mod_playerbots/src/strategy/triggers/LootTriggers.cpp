/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Trigger/LootTriggers.cpp:41-50 (issue #151). LootAvailableTrigger
 * (AC :12-39) is ported into GenericTriggers.cpp instead - see the class comment in GenericTriggers.h.
 */

#include "LootTriggers.h"

#include "LootObjectStack.h"
#include "Playerbots.h"

bool FarFromCurrentLootTrigger::IsActive()
{
    LootObject loot = AI_VALUE(LootObject, "loot target");
    if (!loot.IsLootPossible(bot))
        return false;

    return AI_VALUE2(float, "distance", "loot target") >= INTERACTION_DISTANCE - 2.0f;
}

bool CanLootTrigger::IsActive() { return AI_VALUE(bool, "can loot"); }
