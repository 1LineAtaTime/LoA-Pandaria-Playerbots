/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/QuestConfirmAcceptAction.h) into
 * the Legends of Azeroth Pandaria fork, issue #257 slice D (the master-commanded quest layer).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h, WorldPacketActionContext.h and
 * WorldPacketHandlerStrategy.cpp are owned by the single later registration slice of #257.
 */

#ifndef _PLAYERBOT_QUESTCONFIRMACCEPTACTION_H
#define _PLAYERBOT_QUESTCONFIRMACCEPTACTION_H

#include "Action.h"

class ObjectGuid;
class Player;
class PlayerbotAI;
class Quest;
class WorldObject;

class QuestConfirmAcceptAction : public Action
{
public:
    QuestConfirmAcceptAction(PlayerbotAI* botAI) : Action(botAI, "quest confirm accept") {}
    bool Execute(Event event) override;
};

#endif