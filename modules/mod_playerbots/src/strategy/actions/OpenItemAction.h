/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/OpenItemAction.h (issue #535).
 */

#ifndef PLAYERBOTS_OPENITEMACTION_H
#define PLAYERBOTS_OPENITEMACTION_H

#include "Action.h"

class Player;
class Item;
class Event;

// AC OpenItemAction.h:16-27.
class OpenItemAction : public Action
{
public:
    OpenItemAction(PlayerbotAI* botAI) : Action(botAI, "open item") {}

    bool Execute(Event event) override;

private:
    void OpenItem(Item* item, uint8 bag, uint8 slot);
};

#endif
