/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TellItemCountAction.h: the "c" / "items" / "inv" /
 * "inventory" chat commands, which list the bot's items matching the text after the command.
 */

#ifndef _PLAYERBOT_TELLITEMCOUNTACTION_H
#define _PLAYERBOT_TELLITEMCOUNTACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class TellItemCountAction : public InventoryAction
{
public:
    // AC names the action "c"; the chat strategy reaches it as "item count" (AC ChatActionContext.h:127).
    TellItemCountAction(PlayerbotAI* botAI) : InventoryAction(botAI, "c") {}

    bool Execute(Event event) override;
};

#endif
