/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "TellMasterAction.h"

#include "Event.h"
#include "Playerbots.h"

// AC TellMasterAction.cpp:11-15, unchanged.
bool TellMasterAction::Execute(Event /*event*/)
{
    botAI->TellMaster(text);
    return true;
}
