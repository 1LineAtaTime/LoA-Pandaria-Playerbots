/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/WtsAction.cpp, 1:1 apart from the chat helpers being static and
 * sRandomPlayerbotMgr being a pointer. RandomPlayerbotMgr does not expose GetBuyMultiplier() on this module, so the
 * same formula (AC RandomPlayerbotMgr.cpp) is kept file-local, as in QueryItemUsageAction.cpp.
 */

#include "WtsAction.h"

#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

namespace
{
// AC RandomPlayerbotMgr.cpp GetBuyMultiplier.
double WtsBuyMultiplier(Player* bot)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        sRandomPlayerbotMgr->SetValue(bot, "buymultiplier", value);
    }

    return (double)value / 100.0;
}
}  // namespace

bool WtsAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    std::string const link = event.getParam();

    ItemIds itemIds = ChatHelper::parseItems(link);
    if (itemIds.empty())
        return false;

    for (ItemIds::iterator i = itemIds.begin(); i != itemIds.end(); i++)
    {
        uint32 itemId = *i;
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        std::ostringstream out;
        out << itemId;
        ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", out.str());
        if (usage == ITEM_USAGE_NONE)
            continue;

        int32 buyPrice = proto->BuyPrice * WtsBuyMultiplier(bot);
        if (!buyPrice)
            continue;

        if (urand(0, 15) > 2)
            continue;

        std::ostringstream tell;
        tell << "I'll buy " << ChatHelper::FormatItem(proto) << " for " << ChatHelper::formatMoney(buyPrice);

        // ignore random bot chat filter
        bot->Whisper(tell.str(), LANG_UNIVERSAL, owner);
    }

    return true;
}
