/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RememberTaxiAction.h (#179, spec section 3 "taxi with
 * the master"). Parses the master's CMSG_ACTIVATE_TAXI(_EXPRESS), forwarded via
 * masterIncomingPacketHandlers, into the "last taxi" LastMovement value so TaxiAction can act on it.
 */

#ifndef _PLAYERBOT_REMEMBERTAXIACTION_H
#define _PLAYERBOT_REMEMBERTAXIACTION_H

#include "Action.h"

class PlayerbotAI;

class RememberTaxiAction : public Action
{
public:
    RememberTaxiAction(PlayerbotAI* botAI) : Action(botAI, "remember taxi") {}

    bool Execute(Event event) override;
};

#endif
