/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/QueryQuestAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #257 slice F.
 *
 * No action is registered here - Factory/AiFactory.cpp and ActionContext.h are owned by the single
 * later registration slice of #257.
 */

#ifndef _PLAYERBOT_QUERYQUESTACTION_H
#define _PLAYERBOT_QUERYQUESTACTION_H

#include "Action.h"

class PlayerbotAI;

class QueryQuestAction : public Action
{
public:
    QueryQuestAction(PlayerbotAI* botAI) : Action(botAI, "query quest") {}

    bool Execute(Event event) override;

private:
    void TellObjectives(uint32 questId);
    void TellObjective(std::string const name, uint32 available, uint32 required);
};

#endif