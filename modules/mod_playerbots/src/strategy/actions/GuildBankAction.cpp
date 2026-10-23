/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GuildBankAction.cpp (#258).
 *  - parseItems(text) -> InventoryAction::Find(text).
 *  - chat->FormatItem -> ChatHelper::FormatItem (static on this fork).
 *  - AC passes uint32 playerSlot/playerBag into Guild::SwapItemsWithInventory's uint8 parameters
 *    (core Guild.h:902); cast explicitly here so the same narrowing does not depend on the build's
 *    warning level.
 *  - Guild::MemberHasTabRights / SwapItemsWithInventory take an ObjectGuid on 5.4.8 (core Guild.h:1061
 *    / :902), not AC's low-guid form.
 */

#include "GuildBankAction.h"

#include "AiObjectContext.h"
#include "ChatHelper.h"
#include "Event.h"
#include "GameObject.h"
#include "GuildMgr.h"
#include "Item.h"
#include "ObjectGuid.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

#include <sstream>
#include <string>
#include <vector>

// AC GuildBankAction.cpp:12-36
bool GuildBankAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text.empty())
        return false;

    if (!bot->GetGuildId() || (GetMaster() && GetMaster()->GetGuildId() != bot->GetGuildId()))
    {
        botAI->TellMaster("I'm not in your guild!");
        return false;
    }

    GuidVector gos = AI_VALUE(GuidVector, "nearest game objects");
    for (ObjectGuid const goGuid : gos)
    {
        GameObject* go = botAI->GetGameObject(goGuid);
        if (!go || !bot->GetGameObjectIfCanInteractWith(go->GetGUID(), GAMEOBJECT_TYPE_GUILD_BANK))
            continue;

        return Execute(text, go);
    }

    botAI->TellMaster("Cannot find the guild bank nearby");
    return false;
}

// AC GuildBankAction.cpp:38-54
bool GuildBankAction::Execute(std::string const text, GameObject* bank)
{
    bool result = true;

    std::vector<Item*> found = Find(text);
    if (found.empty())
        return false;

    for (Item* item : found)
        if (item)
            result &= MoveFromCharToBank(item, bank);

    return result;
}

// AC GuildBankAction.cpp:56-79
bool GuildBankAction::MoveFromCharToBank(Item* item, GameObject* /*bank*/)
{
    uint32 playerSlot = item->GetSlot();
    uint32 playerBag = item->GetBagSlot();

    std::ostringstream out;

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());

    // #258: AC calls `guild->MemberHasTabRights(bot->GetGUID(), 0, GUILD_BANK_RIGHT_DEPOSIT_ITEM)` here, but
    // on this core that member is **private** (Guild.h:1061 sits under the `private:` at :963) - so the
    // pre-check is not portable. The core validates the rights itself inside `SwapItemsWithInventory`, so the
    // deposit is still refused when the bot has no rights; only the friendlier "I have no rights" message is
    // lost. Do not reintroduce the call without a public accessor on Guild.
    {
        out << ChatHelper::FormatItem(item->GetTemplate()) << " put to guild bank";
        guild->SwapItemsWithInventory(bot, false, 0, 255, uint8(playerBag), uint8(playerSlot), 0);
    }

    botAI->TellMaster(out);

    return true;
}