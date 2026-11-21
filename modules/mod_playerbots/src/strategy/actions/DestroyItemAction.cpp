/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/DestroyItemAction.cpp (DestroyItemAction, lines 12-37), 1:1 apart
 * from the chat helper being static (ChatHelper::parseItems / FormatItem). SmartDestroyItemAction (AC :39-106) is
 * not a chat command and is not ported.
 */

#include "DestroyItemAction.h"

#include "Event.h"
#include "Playerbots.h"

bool DestroyItemAction::Execute(Event event)
{
    std::string const text = event.getParam();
    ItemIds ids = ChatHelper::parseItems(text);

    for (ItemIds::iterator i = ids.begin(); i != ids.end(); i++)
    {
        FindItemByIdVisitor visitor(*i);
        DestroyItem(&visitor);
    }

    return true;
}

void DestroyItemAction::DestroyItem(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    for (Item* item : items)
    {
        std::ostringstream out;
        out << ChatHelper::FormatItem(item->GetTemplate()) << " destroyed";
        botAI->TellMaster(out);

        bot->DestroyItem(item->GetBagSlot(), item->GetSlot(), true);
    }
}
