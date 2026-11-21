/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/QueryItemUsageAction.cpp. 5.4.8 differences:
 *  - Quest item requirements live in Quest::GetObjectives() (QUEST_OBJECTIVE_ITEM, ObjectID / Amount) and the
 *    progress in Player::GetQuestObjectiveCounter(), instead of AC's RequiredItemId[] / RequiredItemCount[] /
 *    QuestStatusData::ItemCount[] arrays (QueryQuestItem, AC QueryItemUsageAction.cpp:176-190). The
 *    QuestStatusData parameter is kept for signature parity and not read.
 *  - RandomPlayerbotMgr does not expose GetBuyMultiplier()/GetSellMultiplier() on this module (see
 *    TradeStatusAction.cpp); the same two formulas (AC RandomPlayerbotMgr.cpp) are kept file-local here.
 *  - sRandomPlayerbotMgr is a pointer.
 */

#include "QueryItemUsageAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemUsageValue.h"
#include "Playerbots.h"
#include "QuestDef.h"

namespace
{
// AC RandomPlayerbotMgr.cpp GetBuyMultiplier (same file-local form as TradeStatusAction.cpp).
double QueryBuyMultiplier(Player* bot)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        sRandomPlayerbotMgr->SetValue(bot, "buymultiplier", value);
    }

    return (double)value / 100.0;
}

// AC RandomPlayerbotMgr.cpp GetSellMultiplier.
double QuerySellMultiplier(Player* bot)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        sRandomPlayerbotMgr->SetValue(bot, "sellmultiplier", value);
    }

    return (double)value / 100.0;
}
}  // namespace

bool QueryItemUsageAction::Execute(Event event)
{
    std::string param = event.getParam();
    if (param.empty())
    {
        return false;
    }

    // Use parseItems() to extract item IDs from the input
    ItemIds itemIds = chat->parseItems(param);
    if (itemIds.empty())
    {
        return false;
    }

    // Process each extracted item ID (assuming single-item queries for now)
    for (uint32 itemId : itemIds)
    {
        ItemTemplate const* itemTemplate = sObjectMgr->GetItemTemplate(itemId);
        if (!itemTemplate)
            continue;

        uint32 count = GetCount(itemTemplate);
        uint32 total = bot->GetItemCount(itemTemplate->ItemId, true);
        std::string itemInfo = QueryItem(itemTemplate, count, total);

        botAI->TellMaster(itemInfo);
        return true;  // Only process the first valid item
    }

    return false;
}

uint32 QueryItemUsageAction::GetCount(ItemTemplate const* item)
{
    uint32 total = 0;

    std::vector<Item*> items = InventoryAction::parseItems(item->Name1);
    if (!items.empty())
    {
        for (std::vector<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            total += (*i)->GetCount();
        }
    }

    return total;
}

std::string const QueryItemUsageAction::QueryItem(ItemTemplate const* item, uint32 count, uint32 total)
{
    std::ostringstream out;
    std::string usage = QueryItemUsage(item);
    std::string const quest = QueryQuestItem(item->ItemId);
    std::string const price = QueryItemPrice(item);
    if (usage.empty())
        usage = (quest.empty() ? "Useless" : "Quest");

    out << chat->FormatItem(item, count, total) << ": " << usage;
    if (!quest.empty())
        out << ", " << quest;

    if (!price.empty())
        out << ", " << price;

    return out.str();
}

std::string const QueryItemUsageAction::QueryItemUsage(ItemTemplate const* item)
{
    std::ostringstream out;
    out << item->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
    switch (usage)
    {
        case ITEM_USAGE_EQUIP:
            return "Equip";
        case ITEM_USAGE_REPLACE:
            return "Equip (replace)";
        case ITEM_USAGE_BAD_EQUIP:
            return "Equip (temporary)";
        case ITEM_USAGE_BROKEN_EQUIP:
            return "Broken Equip";
        case ITEM_USAGE_QUEST:
            return "Quest (other)";
        case ITEM_USAGE_SKILL:
            return "Tradeskill";
        case ITEM_USAGE_USE:
            return "Use";
        case ITEM_USAGE_GUILD_TASK:
            return "Guild task";
        case ITEM_USAGE_DISENCHANT:
            return "Disenchant";
        case ITEM_USAGE_VENDOR:
            return "Vendor";
        case ITEM_USAGE_AH:
            return "Auctionhouse";
        case ITEM_USAGE_AMMO:
            return "Ammunition";
        default:
            break;
    }

    return "";
}

std::string const QueryItemUsageAction::QueryItemPrice(ItemTemplate const* item)
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return "";

    if (item->Bonding == BIND_ON_ACQUIRE)
        return "";

    std::ostringstream msg;
    std::vector<Item*> items = InventoryAction::parseItems(item->Name1);
    int32 sellPrice = 0;
    if (!items.empty())
    {
        for (std::vector<Item*>::iterator i = items.begin(); i != items.end(); ++i)
        {
            Item* sell = *i;
            int32 price = sell->GetCount() * sell->GetTemplate()->SellPrice * QuerySellMultiplier(bot);
            if (!sellPrice || sellPrice > price)
                sellPrice = price;
        }
    }
    if (sellPrice)
        msg << "Sell: " << chat->formatMoney(sellPrice);

    std::ostringstream out;
    out << item->ItemId;
    ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
    if (usage == ITEM_USAGE_NONE)
        return msg.str();

    int32 buyPrice = item->BuyPrice * QueryBuyMultiplier(bot);
    if (buyPrice)
    {
        if (sellPrice)
            msg << " ";

        msg << "Buy: " << chat->formatMoney(buyPrice);
    }

    return msg.str();
}

std::string const QueryItemUsageAction::QueryQuestItem(uint32 itemId)
{
    Player* bot = botAI->GetBot();
    QuestStatusMap& questMap = bot->getQuestStatusMap();
    for (QuestStatusMap::const_iterator i = questMap.begin(); i != questMap.end(); i++)
    {
        Quest const* questTemplate = sObjectMgr->GetQuestTemplate(i->first);
        if (!questTemplate)
            continue;

        uint32 questId = questTemplate->GetQuestId();
        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE ||
            (status == QUEST_STATUS_COMPLETE && !bot->GetQuestRewardStatus(questId)))
        {
            QuestStatusData const& questStatus = i->second;
            std::string const usage = QueryQuestItem(itemId, questTemplate, &questStatus);
            if (!usage.empty())
                return usage;
        }
    }

    return "";
}

std::string const QueryItemUsageAction::QueryQuestItem(uint32 itemId, Quest const* questTemplate,
                                                       QuestStatusData const* /*questStatus*/)
{
    for (QuestObjective const& objective : questTemplate->GetObjectives())
    {
        if (objective.Type != QUEST_OBJECTIVE_ITEM || objective.ObjectID != itemId)
            continue;

        uint32 required = objective.Amount > 0 ? uint32(objective.Amount) : 0;
        uint32 available = bot->GetQuestObjectiveCounter(objective.ID);
        if (!required)
            continue;

        return chat->FormatQuestObjective(chat->FormatQuest(questTemplate), available, required);
    }

    return "";
}
