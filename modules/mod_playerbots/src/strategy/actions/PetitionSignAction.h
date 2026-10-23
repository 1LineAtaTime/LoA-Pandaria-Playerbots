/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * #258 lane Q: AC Ai/Base/Actions/PetitionSignAction.h. Fired by the bot's own SMSG_PETITION_SHOW_SIGNATURES
 * (AC WorldPacketHandlerStrategy.cpp:50 "petition offer" -> "petition sign").
 */

#ifndef _PLAYERBOT_PETITIONSIGNACTION_H
#define _PLAYERBOT_PETITIONSIGNACTION_H

#include "Action.h"

class PlayerbotAI;

class PetitionSignAction : public Action
{
public:
    PetitionSignAction(PlayerbotAI* botAI) : Action(botAI, "petition sign") {}

    bool Execute(Event event) override;
};

#endif
