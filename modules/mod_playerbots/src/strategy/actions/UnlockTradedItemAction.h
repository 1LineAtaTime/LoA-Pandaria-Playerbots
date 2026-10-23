/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/UnlockTradedItemAction.{h,cpp} for issue #193 (S7 of
 * epic #163). See UnlockTradedItemAction.cpp for the 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_UNLOCKTRADEDITEMACTION_H
#define PLAYERBOTS_UNLOCKTRADEDITEMACTION_H

#include "Action.h"

class Item;
class PlayerbotAI;

class UnlockTradedItemAction : public Action
{
public:
    UnlockTradedItemAction(PlayerbotAI* botAI) : Action(botAI, "unlock traded item") {}

    bool Execute(Event event) override;

private:
    bool CanUnlockItem(Item* item);
    void UnlockItem(Item* item);
};

#endif
