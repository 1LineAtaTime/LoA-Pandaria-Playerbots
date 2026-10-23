/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeStatusExtendedAction.{h,cpp} for issue #193 (S7
 * of epic #163). See TradeStatusExtendedAction.cpp for the 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_TRADESTATUSEXTENDEDACTION_H
#define PLAYERBOTS_TRADESTATUSEXTENDEDACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;

class TradeStatusExtendedAction : public Action
{
public:
    TradeStatusExtendedAction(PlayerbotAI* botAI) : Action(botAI, "trade status extended") {}

    bool Execute(Event event) override;
};

#endif
