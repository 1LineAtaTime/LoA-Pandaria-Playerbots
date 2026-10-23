/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * #258 lane Q: AC Ai/Base/Actions/TellMasterAction.h:13-20. Only TellMasterAction is ported; AC's
 * OutOfReactRangeAction ("tell out of react range", same header) is not part of #258.
 * Registered (by the PM) as "tell not enough money" / "tell not enough reputation"
 * (AC WorldPacketActionContext.h:147-148).
 */

#ifndef _PLAYERBOT_TELLMASTERACTION_H
#define _PLAYERBOT_TELLMASTERACTION_H

#include "Action.h"

class PlayerbotAI;

class TellMasterAction : public Action
{
public:
    TellMasterAction(PlayerbotAI* botAI, std::string const text) : Action(botAI, "tell master"), text(text) {}

    bool Execute(Event event) override;

private:
    std::string const text;
};

#endif
