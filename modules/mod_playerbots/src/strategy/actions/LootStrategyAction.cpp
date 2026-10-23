/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/LootStrategyAction.cpp for issue #188 (S6 of epic
 * #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - chat->parseItems(text)/ChatHelper::FormatItem(proto)/chat->FormatItem(proto): this fork's
 *    ChatHelper (AI/ChatHelper.h) only has parseValue() - no item-link parser/formatter was ported.
 *    Replaced with a small local ParseItemIds() (accepts a plain numeric item id, or an item link's
 *    "item:<id>" segment) and proto->Name1 for display instead of a clickable item link.
 *  - StoreLootAction::IsLootAllowed(itemid, botAI) exists unchanged on this fork
 *    (strategy/actions/LootAction.h:57, ported for #151) and is used as-is.
 */

#include "LootStrategyAction.h"

#include "Event.h"
#include "LootAction.h"
#include "LootObjectStack.h"
#include "LootStrategyValue.h"
#include "Playerbots.h"

#include <cctype>
#include <cstring>

namespace
{
// 5.4.8: see the chat->parseItems() note in the file header. Accepts either a bare item id
// ("12345") or an item link containing "item:12345:..." and extracts the numeric id from it.
ItemIds ParseItemIds(std::string const& text)
{
    ItemIds ids;

    size_t pos = text.find("item:");
    if (pos != std::string::npos)
    {
        pos += strlen("item:");
        size_t end = pos;
        while (end < text.size() && isdigit((unsigned char)text[end]))
            ++end;
        if (end > pos)
            ids.insert(uint32(atoi(text.substr(pos, end - pos).c_str())));
        return ids;
    }

    bool allDigits = !text.empty();
    for (char c : text)
        if (!isdigit((unsigned char)c))
        {
            allDigits = false;
            break;
        }

    if (allDigits)
        ids.insert(uint32(atoi(text.c_str())));

    return ids;
}
}

bool LootStrategyAction::Execute(Event event)
{
    std::string const strategy = event.getParam();

    std::set<uint32>& alwaysLootItems = AI_VALUE(std::set<uint32>&, "always loot list");
    Value<LootStrategy*>* lootStrategy = context->GetValue<LootStrategy*>("loot strategy");

    if (strategy == "?")
    {
        {
            std::ostringstream out;
            out << "Loot strategy: ";
            out << lootStrategy->Get()->GetName();
            botAI->TellMaster(out);
        }

        {
            std::ostringstream out;
            out << "Always loot items: ";

            for (uint32 itemId : alwaysLootItems)
            {
                ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
                if (!proto)
                    continue;

                out << proto->Name1 << " (" << itemId << ") ";
            }

            botAI->TellMaster(out);
        }
    }
    else
    {
        ItemIds items = ParseItemIds(strategy);

        if (items.size() == 0)
        {
            lootStrategy->Set(LootStrategyValue::instance(strategy));

            std::ostringstream out;
            out << "Loot strategy set to " << lootStrategy->Get()->GetName();
            botAI->TellMaster(out);
            return true;
        }

        bool remove = strategy.size() > 1 && strategy.substr(0, 1) == "-";
        bool query = strategy.size() > 1 && strategy.substr(0, 1) == "?";
        for (uint32 itemid : items)
        {
            if (query)
            {
                if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemid))
                {
                    std::ostringstream out;
                    out << (StoreLootAction::IsLootAllowed(itemid, botAI) ? "Will loot " : "Won't loot ")
                        << proto->Name1 << " (" << itemid << ")";
                    botAI->TellMaster(out.str());
                }
            }
            else if (remove)
            {
                std::set<uint32>::iterator j = alwaysLootItems.find(itemid);
                if (j != alwaysLootItems.end())
                    alwaysLootItems.erase(j);

                botAI->TellMaster("Item(s) removed from always loot list");
            }
            else
            {
                alwaysLootItems.insert(itemid);
                botAI->TellMaster("Item(s) added to always loot list");
            }
        }
    }

    return true;
}
