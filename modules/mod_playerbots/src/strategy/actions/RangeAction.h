/*
 * This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/RangeAction.h (#177): the "range" chat command. The
 * qualified "range" AI value it reads/writes already exists on this fork (ValueContext.h); only this
 * chat wrapper was missing.
 */

#ifndef _PLAYERBOT_RANGEACTION_H
#define _PLAYERBOT_RANGEACTION_H

#include "Action.h"

class PlayerbotAI;

class RangeAction : public Action
{
public:
    RangeAction(PlayerbotAI* botAI) : Action(botAI, "range") {}

    bool Execute(Event event) override;

private:
    void PrintRange(std::string const type);
};

#endif
