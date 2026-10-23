/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/GuildValues.h/.cpp (#611).
 * "petition signs": GuildCreateActions' PetitionOfferAction / PetitionOfferNearbyAction / PetitionTurnInAction read and set it.
 */

#ifndef PLAYERBOTS_GUILDVALUES_H
#define PLAYERBOTS_GUILDVALUES_H

#include "Value.h"

class PlayerbotAI;

class PetitionSignsValue : public SingleCalculatedValue<uint8>
{
public:
    PetitionSignsValue(PlayerbotAI* botAI) : SingleCalculatedValue<uint8>(botAI, "petition signs") {}

    uint8 Calculate() override;
};

#endif
