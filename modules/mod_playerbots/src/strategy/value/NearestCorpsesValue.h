/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/NearestCorpsesValue.h (issue #151). Feeds
 * AddAllLootAction ("add all loot" - AddLootAction.cpp) so a dead creature enters the loot stack
 * even before the bot has attacked or dropped-target on it.
 */

#ifndef _PLAYERBOT_NEARESTCORPSESVALUE_H
#define _PLAYERBOT_NEARESTCORPSESVALUE_H

#include "NearestUnitsValue.h"
#include "PlayerbotAIConfig.h"

class PlayerbotAI;

class NearestCorpsesValue : public NearestUnitsValue
{
public:
    NearestCorpsesValue(PlayerbotAI* botAI, float range = sPlayerbotAIConfig->sightDistance)
        : NearestUnitsValue(botAI, "nearest corpses", range, true)
    {
    }

protected:
    void FindUnits(std::list<Unit*>& targets) override;
    bool AcceptUnit(Unit* unit) override;
};

#endif
