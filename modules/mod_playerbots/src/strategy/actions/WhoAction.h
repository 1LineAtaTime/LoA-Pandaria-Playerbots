/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_WHOACTION_H
#define _PLAYERBOT_WHOACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class WhoAction : public InventoryAction
{
public:
    WhoAction(PlayerbotAI* botAI) : InventoryAction(botAI, "who") {}

    bool Execute(Event event) override;

private:
    // AC WhoAction.h: "who <item link>" for random bots, "Selling <item> for <price>".
    std::string const QueryTrade(std::string const& text);
};

#endif
