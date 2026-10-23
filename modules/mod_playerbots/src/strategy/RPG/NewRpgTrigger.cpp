/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Trigger/NewRpgTrigger.cpp)
 * into the Legends of Azeroth Pandaria fork.
 */

#include "NewRpgTriggers.h"

#include "PlayerbotAI.h"

bool NewRpgStatusTrigger::IsActive()
{
    return status == botAI->GetRpgInfo().GetStatus();
}
