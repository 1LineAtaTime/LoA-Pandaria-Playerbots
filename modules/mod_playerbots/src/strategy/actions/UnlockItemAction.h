/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UnlockItemAction.h (issue #535).
 */

#ifndef PLAYERBOTS_UNLOCKITEMACTION_H
#define PLAYERBOTS_UNLOCKITEMACTION_H

#include "Action.h"

class Item;
class PlayerbotAI;

// AC UnlockItemAction.h:14-24.
class UnlockItemAction : public Action
{
public:
    UnlockItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock item") {}

    bool Execute(Event event) override;

private:
    void UnlockItem(Item* item);
};

#endif
