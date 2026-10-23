/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/PvpValues.h)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 *
 * Only BgTypeValue ("bg type", AC PvpValues.h:17-21) and RpgBgTypeValue ("rpg bg type", AC PvpValues.h:52-58)
 * live here; FlagCarrierValue is in FlagCarrierValue.h and BgRoleValue in NearestGameObjects.h.
 */

#ifndef _PLAYERBOT_PVPVALUES_H
#define _PLAYERBOT_PVPVALUES_H

#include "NamedObjectContext.h"
#include "SharedDefines.h"
#include "Value.h"

class PlayerbotAI;

class BgTypeValue : public ManualSetValue<uint32>
{
public:
    BgTypeValue(PlayerbotAI* botAI) : ManualSetValue<uint32>(botAI, 0, "bg type") {}
};

class RpgBgTypeValue : public CalculatedValue<BattlegroundTypeId>
{
public:
    RpgBgTypeValue(PlayerbotAI* botAI) : CalculatedValue(botAI, "rpg bg type") {}

    BattlegroundTypeId Calculate() override;
};

#endif
