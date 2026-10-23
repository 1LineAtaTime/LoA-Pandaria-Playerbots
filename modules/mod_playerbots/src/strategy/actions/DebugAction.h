/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/DebugAction.h (#611, build 13). FakeSpell is declared under #if 0 because its
 * 3.3.5 packet body is (see DebugAction.cpp).
 */

#ifndef _PLAYERBOT_DEBUGACTION_H
#define _PLAYERBOT_DEBUGACTION_H

#include "Action.h"
#include "ObjectGuid.h"
#include "TravelMgr.h"

class PlayerbotAI;
class Unit;

class DebugAction : public Action
{
public:
    DebugAction(PlayerbotAI* botAI) : Action(botAI, "Debug") {}

    bool Execute(Event event) override;

#if 0
    void FakeSpell(uint32 spellId, Unit* truecaster, Unit* caster, ObjectGuid target = ObjectGuid::Empty,
                   GuidVector otherTargets = {}, GuidVector missTargets = {}, WorldPosition source = WorldPosition(),
                   WorldPosition dest = WorldPosition(), bool forceDest = false);
#endif
    void addAura(uint32 spellId, Unit* target);
};

#endif
