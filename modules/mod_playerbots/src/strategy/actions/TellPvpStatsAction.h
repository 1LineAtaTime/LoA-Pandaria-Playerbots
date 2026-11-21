/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TellPvpStatsAction.h: the "tell pvp stats" chat command.
 */

#ifndef _PLAYERBOT_TELLPVPSTATSACTION_H
#define _PLAYERBOT_TELLPVPSTATSACTION_H

#include "Action.h"

class PlayerbotAI;

class TellPvpStatsAction : public Action
{
public:
    TellPvpStatsAction(PlayerbotAI* botAI) : Action(botAI, "tell pvp stats") {}

    bool Execute(Event event) override;
};

#endif
