/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #648: ported 1:1 from AC mod-playerbots Ai/Base/Strategy/RTSCStrategy.cpp.
 */

#include "RTSCStrategy.h"

#include "Playerbots.h"

RTSCStrategy::RTSCStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

void RTSCStrategy::InitTriggers(std::vector<TriggerNode*>& /*triggers*/) {}
