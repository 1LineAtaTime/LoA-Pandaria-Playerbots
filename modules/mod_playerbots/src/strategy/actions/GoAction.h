/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GoAction.h: the "go" chat command (go to a spot, unit, game object,
 * saved position or travel destination).
 */

#ifndef _PLAYERBOT_GOACTION_H
#define _PLAYERBOT_GOACTION_H

#include "MovementActions.h"

class PlayerbotAI;

class GoAction : public MovementAction
{
public:
    // AC registers the action under the name "Go" and the chat context maps the "go" creator to it
    // (AC ChatActionContext.h:182); the name only matters for logging.
    GoAction(PlayerbotAI* botAI) : MovementAction(botAI, "Go") {}

    bool Execute(Event event) override;
};

#endif
