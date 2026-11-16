/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/BuffAction.h: the "buff" chat command.
 */

#ifndef _PLAYERBOT_BUFFACTION_H
#define _PLAYERBOT_BUFFACTION_H

#include "InventoryAction.h"

class PlayerbotAI;

class BuffAction : public InventoryAction
{
public:
    BuffAction(PlayerbotAI* botAI) : InventoryAction(botAI, "buff") {}

    bool Execute(Event event) override;

private:
    void TellHeader(uint32 subClass);
};

#endif
