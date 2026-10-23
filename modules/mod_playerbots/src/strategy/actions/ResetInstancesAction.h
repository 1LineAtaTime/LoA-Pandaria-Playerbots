/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RESETINSTANCESACTION_H
#define _PLAYERBOT_RESETINSTANCESACTION_H

#include "Action.h"

class PlayerbotAI;

/// Ported from AC mod-playerbots ResetInstancesAction.h/.cpp (#149), adapted to this core's
/// WorldSession::HandleResetInstancesOpcode(WorldPacket&) signature (this core has no
/// WorldPackets::Instance::ResetInstances wrapper).
class ResetInstancesAction : public Action
{
public:
    ResetInstancesAction(PlayerbotAI* botAI) : Action(botAI, "reset instances") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
