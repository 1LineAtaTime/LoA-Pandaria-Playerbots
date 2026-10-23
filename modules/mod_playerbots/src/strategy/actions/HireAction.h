/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * #258 lane Q: AC Ai/Base/Actions/HireAction.h. Chat command "hire" (AC ChatActionContext.h:187). AC leaves the
 * chat trigger itself disabled (ChatTriggerContext.h:130 "Not correctly implemented at this time, would cause crash
 * and other issues") while keeping the action registered and "hire" in ChatCommandHandlerStrategy's supported list
 * (:152); this port keeps exactly that dormant shape.
 */

#ifndef _PLAYERBOT_HIREACTION_H
#define _PLAYERBOT_HIREACTION_H

#include "Action.h"

class PlayerbotAI;

class HireAction : public Action
{
public:
    HireAction(PlayerbotAI* botAI) : Action(botAI, "hire") {}

    bool Execute(Event event) override;
};

#endif
