/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Value/AoeValues.h) into the
 * Legends of Azeroth Pandaria fork (issue #149).
 */

#ifndef _PLAYERBOT_AOEVALUES_H
#define _PLAYERBOT_AOEVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

class AoePositionValue : public CalculatedValue<WorldLocation>
{
public:
    AoePositionValue(PlayerbotAI* botAI) : CalculatedValue<WorldLocation>(botAI, "aoe position") {}

    WorldLocation Calculate() override;
};

// Not one of the issue #149 names, but registered alongside "aoe position": CcTargetValue (issue
// #149 "cc target") reads it via AI_VALUE2(uint8, "aoe count"), so it must exist too or that ported
// code hits the exact same null-Value crash this issue is fixing.
class AoeCountValue : public CalculatedValue<uint8>
{
public:
    AoeCountValue(PlayerbotAI* botAI) : CalculatedValue<uint8>(botAI, "aoe count") {}

    uint8 Calculate() override;
};

class HasAreaDebuffValue : public BoolCalculatedValue, public Qualified
{
public:
    HasAreaDebuffValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "has area debuff") {}

    bool Calculate() override;
};

#endif
