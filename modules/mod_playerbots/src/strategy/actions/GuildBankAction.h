/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GuildBankAction.h/.cpp (#258).
 *  - parseItems(text) -> InventoryAction::Find(text) (this fork's surviving resolver; same rules for
 *    the qualifiers a guild-bank deposit can carry - item id or item name).
 *  - chat->FormatItem -> ChatHelper::FormatItem (static on this fork).
 *  - PlayerbotTextMgr is not ported, and AC's BroadcastHelper is not ported (see #322); neither has a
 *    call site in this action, so nothing is omitted here.
 */

#ifndef _PLAYERBOT_GUILDBANKACTION_H
#define _PLAYERBOT_GUILDBANKACTION_H

#include "InventoryAction.h"

class GameObject;
class Item;
class PlayerbotAI;

class GuildBankAction : public InventoryAction
{
public:
    GuildBankAction(PlayerbotAI* botAI) : InventoryAction(botAI, "guild bank") {}

    bool Execute(Event event) override;

private:
    bool Execute(std::string const text, GameObject* bank);
    bool MoveFromCharToBank(Item* item, GameObject* bank);
};

#endif