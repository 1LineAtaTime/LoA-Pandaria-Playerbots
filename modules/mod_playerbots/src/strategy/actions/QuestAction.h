/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/QuestAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #257 slice B (the master-commanded quest layer).
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h and WorldPacketHandlerStrategy.cpp
 * are owned by the single later registration slice of #257.
 */

#ifndef _PLAYERBOT_QUESTACTION_H
#define _PLAYERBOT_QUESTACTION_H

#include "Action.h"
// #322: QuestDef.h declares Quest and needs Field declared first, or it fails to parse on its own
// (same ordering note as AI/ChatHelper.cpp:8-10).
#include "Field.h"
#include "Object.h"
#include "QuestDef.h"

class ObjectGuid;
class Quest;
class Player;
class PlayerbotAI;
class WorldObject;

class QuestAction : public Action
{
public:
    QuestAction(PlayerbotAI* botAI, std::string const name) : Action(botAI, name) {}
    bool Execute(Event event) override;

protected:
    bool CompleteQuest(Player* player, uint32 entry);
    virtual bool ProcessQuest(Quest const* quest, Object* questGiver) = 0;
    bool AcceptQuest(Quest const* quest, ObjectGuid questGiver);
    bool ProcessQuests(ObjectGuid questGiver);
    bool ProcessQuests(WorldObject* questGiver);
};

// #257 (AC QuestAction.h:35-40): binds SMSG_QUEST_UPDATE_COMPLETE (5.4.8 name of AC's SMSG_QUESTUPDATE_COMPLETE).
class QuestUpdateCompleteAction : public Action
{
public:
    QuestUpdateCompleteAction(PlayerbotAI* botAI) : Action(botAI, "quest update complete") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddKillAction : public Action
{
public:
    QuestUpdateAddKillAction(PlayerbotAI* botAI) : Action(botAI, "quest update add kill") {}
    bool Execute(Event event) override;
};

class QuestUpdateAddItemAction : public Action
{
public:
    QuestUpdateAddItemAction(PlayerbotAI* botAI) : Action(botAI, "quest update add item") {}
    bool Execute(Event event) override;
};

class QuestUpdateFailedAction : public Action
{
public:
    QuestUpdateFailedAction(PlayerbotAI* botAI) : Action(botAI, "quest update failed") {}
    bool Execute(Event event) override;
};

class QuestUpdateFailedTimerAction : public Action
{
public:
    QuestUpdateFailedTimerAction(PlayerbotAI* botAI) : Action(botAI, "quest update failed timer") {}
    bool Execute(Event event) override;
};

class QuestItemPushResultAction : public Action
{
public:
    QuestItemPushResultAction(PlayerbotAI* botAI) : Action(botAI, "quest item push result") {}
    bool Execute(Event event) override;
};

#endif