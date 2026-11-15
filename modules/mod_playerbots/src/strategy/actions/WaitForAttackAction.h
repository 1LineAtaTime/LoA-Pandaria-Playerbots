/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/WaitForAttackAction.h: the "wait for attack time" chat command that
 * sets how many seconds a "wait for attack" bot holds its opening attack.
 *
 * AC's WaitForAttackKeepSafeDistanceAction (the safe-distance retreat sub-feature) is not ported: see
 * strategy/generic/WaitForAttackStrategy.h.
 */

#ifndef _PLAYERBOT_WAITFORATTACKACTION_H
#define _PLAYERBOT_WAITFORATTACKACTION_H

#include "Action.h"

class PlayerbotAI;

class SetWaitForAttackTimeAction : public Action
{
public:
    SetWaitForAttackTimeAction(PlayerbotAI* botAI) : Action(botAI, "wait for attack time") {}

    bool Execute(Event event) override;
};

#endif
