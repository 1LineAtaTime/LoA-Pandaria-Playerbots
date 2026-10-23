/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ItemCountValue.h"

#include "Playerbots.h"

// ItemCountValue/InventoryItemValue multiply-inherit a *second*, separate InventoryAction/Action
// (-> AiNamedObject) base chain through InventoryItemValueBase, alongside the
// Uint32CalculatedValue/CalculatedValue<T> (-> AiNamedObject) chain - the same shape as AC's
// original ItemCountValue/InventoryItemValue (Ai/Base/Value/ItemCountValue.h). That makes `bot`,
// `botAI` and `context` ambiguous if referenced unqualified from these two classes; `qualifier`
// (from the single Qualified base) is not, so Calculate() only ever uses qualifier and Find().

uint32 ItemCountValue::Calculate()
{
    uint32 count = 0;
    for (Item* item : Find(qualifier))
        count += item->GetCount();

    return count;
}

std::vector<Item*> InventoryItemValue::Calculate() { return Find(qualifier); }
