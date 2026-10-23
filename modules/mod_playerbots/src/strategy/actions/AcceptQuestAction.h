/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/AcceptQuestAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #257 slice D (the master-commanded quest layer).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h, WorldPacketActionContext.h and
 * WorldPacketHandlerStrategy.cpp are owned by the single later registration slice of #257.
 */

#ifndef _PLAYERBOT_ACCEPTQUESTACTION_H
#define _PLAYERBOT_ACCEPTQUESTACTION_H

#include "QuestAction.h"

class Quest;
class PlayerbotAI;
class WorldObject;

class AcceptAllQuestsAction : public QuestAction
{
public:
    AcceptAllQuestsAction(PlayerbotAI* botAI, std::string const name = "accept all quests") : QuestAction(botAI, name)
    {
    }

protected:
    bool ProcessQuest(Quest const* quest, Object* questGiver) override;
};

class AcceptQuestAction : public AcceptAllQuestsAction
{
public:
    AcceptQuestAction(PlayerbotAI* botAI) : AcceptAllQuestsAction(botAI, "accept quest") {}
    bool Execute(Event event) override;
};

// AC binds "accept quest share" to a packet handler (AC Bot/PlayerbotAI.cpp, master incoming/outgoing
// handlers); the binding itself belongs to the registration slice of #257.
class AcceptQuestShareAction : public Action
{
public:
    AcceptQuestShareAction(PlayerbotAI* botAI) : Action(botAI, "accept quest share") {}
    bool Execute(Event event) override;
};

// AC binds "confirm quest" to CMSG_QUEST_CONFIRM_ACCEPT / SMSG_QUEST_CONFIRM_ACCEPT (AC
// Bot/PlayerbotAI.cpp:317/323); the binding itself belongs to the registration slice of #257.
class ConfirmQuestAction : public Action
{
public:
    ConfirmQuestAction(PlayerbotAI* botAI) : Action(botAI, "confirm quest") {}
    bool Execute(Event event) override;
};

#endif