/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/NearestAdsValue.cpp) into
 * the Legends of Azeroth Pandaria fork (issue #149).
 */

#include "NearestAddsValue.h"

#include "Playerbots.h"

bool NearestAddsValue::AcceptUnit(Unit* unit)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    return unit != target;
}
