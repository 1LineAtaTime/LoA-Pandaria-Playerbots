/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TaxiAction.h (#179, spec section 3 "taxi with the
 * master"). Boards the flight remembered by RememberTaxiAction, staggering follower bots so they do
 * not all take off from the flight master on the same tick.
 */

#ifndef _PLAYERBOT_TAXIACTION_H
#define _PLAYERBOT_TAXIACTION_H

#include "Action.h"

class PlayerbotAI;

class TaxiAction : public Action
{
public:
    TaxiAction(PlayerbotAI* botAI) : Action(botAI, "taxi") {}

    bool Execute(Event event) override;
};

#endif
