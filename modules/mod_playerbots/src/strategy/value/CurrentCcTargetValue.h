/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/CurrentCcTargetValue.h)
 * into the Legends of Azeroth Pandaria fork (issue #149).
 */

#ifndef _PLAYERBOT_CURRENTCCTARGETVALUE_H
#define _PLAYERBOT_CURRENTCCTARGETVALUE_H

#include "NamedObjectContext.h"
#include "TargetValue.h"

class PlayerbotAI;
class Unit;

class CurrentCcTargetValue : public TargetValue, public Qualified
{
public:
    CurrentCcTargetValue(PlayerbotAI* botAI, std::string const name = "current cc target") : TargetValue(botAI, name) {}

    Unit* Calculate() override;
};

#endif
