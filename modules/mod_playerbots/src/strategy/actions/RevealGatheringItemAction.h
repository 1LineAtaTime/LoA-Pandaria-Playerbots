/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RevealGatheringItemAction.h (issue #377).
 */

#ifndef PLAYERBOTS_REVEALGATHERINGITEMACTION_H
#define PLAYERBOTS_REVEALGATHERINGITEMACTION_H

#include "Action.h"

class PlayerbotAI;

// AC RevealGatheringItemAction.h:14-20.
class RevealGatheringItemAction : public Action
{
public:
    RevealGatheringItemAction(PlayerbotAI* botAI) : Action(botAI, "reveal gathering item") {}

    bool Execute(Event event) override;
};

#endif
