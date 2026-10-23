/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/RpgValues.h)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 */

#ifndef PLAYERBOTS_RPGVALUES_H
#define PLAYERBOTS_RPGVALUES_H

#include "Value.h"

class PlayerbotAI;

class NextRpgActionValue : public ManualSetValue<std::string>
{
public:
    NextRpgActionValue(PlayerbotAI* botAI, std::string const defaultValue = "",
                       std::string const name = "next rpg action")
        : ManualSetValue(botAI, defaultValue, name){};
};

#endif
