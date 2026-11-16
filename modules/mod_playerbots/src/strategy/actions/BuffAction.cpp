/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BuffAction.cpp, 1:1 apart from the 5.4.8 consumable subclass name
 * (ITEM_SUBCLASS_FOOD is ITEM_SUBCLASS_FOOD_DRINK here) and the chat helper being static (ChatHelper::FormatItem).
 */

#include "BuffAction.h"

#include "Event.h"
#include "Playerbots.h"

class FindBuffVisitor : public IterateItemsVisitor
{
public:
    FindBuffVisitor(Player* bot) : IterateItemsVisitor(), bot(bot) {}

    bool Visit(Item* item) override
    {
        if (bot->CanUseItem(item->GetTemplate()) != EQUIP_ERR_OK)
            return true;

        ItemTemplate const* proto = item->GetTemplate();

        if (proto->Class != ITEM_CLASS_CONSUMABLE)
            return true;

        if (proto->SubClass != ITEM_SUBCLASS_ELIXIR && proto->SubClass != ITEM_SUBCLASS_FLASK &&
            proto->SubClass != ITEM_SUBCLASS_SCROLL && proto->SubClass != ITEM_SUBCLASS_FOOD_DRINK &&
            proto->SubClass != ITEM_SUBCLASS_CONSUMABLE_OTHER && proto->SubClass != ITEM_SUBCLASS_ITEM_ENHANCEMENT)
            return true;

        for (uint8 i = 0; i < MAX_ITEM_PROTO_SPELLS; i++)
        {
            uint32 spellId = proto->Spells[i].SpellId;
            if (!spellId)
                continue;

            if (bot->HasAura(spellId))
                return true;

            Item* itemForSpell =
                *GET_PLAYERBOT_AI(bot)->GetAiObjectContext()->GetValue<Item*>("item for spell", int32(spellId));
            if (itemForSpell && itemForSpell->IsInWorld() && itemForSpell->GetEnchantmentId(TEMP_ENCHANTMENT_SLOT))
                return true;

            if (items.find(proto->SubClass) == items.end())
                items[proto->SubClass] = std::vector<Item*>();

            items[proto->SubClass].push_back(item);
            break;
        }

        return true;
    }

    std::map<uint32, std::vector<Item*> > items;

private:
    Player* bot;
};

void BuffAction::TellHeader(uint32 subClass)
{
    switch (subClass)
    {
        case ITEM_SUBCLASS_ELIXIR:
            botAI->TellMaster("--- Elixir ---");
            return;
        case ITEM_SUBCLASS_FLASK:
            botAI->TellMaster("--- Flask ---");
            return;
        case ITEM_SUBCLASS_SCROLL:
            botAI->TellMaster("--- Scroll ---");
            return;
        case ITEM_SUBCLASS_FOOD_DRINK:
            botAI->TellMaster("--- Food ---");
            return;
        case ITEM_SUBCLASS_ITEM_ENHANCEMENT:
            botAI->TellMaster("--- Enchant ---");
            return;
    }
}

bool BuffAction::Execute(Event /*event*/)
{
    FindBuffVisitor visitor(bot);
    IterateItems(&visitor);

    uint32 oldSubClass = -1;
    for (std::map<uint32, std::vector<Item*> >::iterator i = visitor.items.begin(); i != visitor.items.end(); ++i)
    {
        std::vector<Item*> items = i->second;

        uint32 subClass = i->first;
        if (oldSubClass != subClass)
        {
            if (!items.empty())
                TellHeader(subClass);

            oldSubClass = subClass;
        }

        for (std::vector<Item*>::iterator j = items.begin(); j != items.end(); ++j)
        {
            Item* item = *j;
            std::ostringstream out;
            out << ChatHelper::FormatItem(item->GetTemplate(), item->GetCount());
            botAI->TellMaster(out);
        }
    }

    return true;
}
