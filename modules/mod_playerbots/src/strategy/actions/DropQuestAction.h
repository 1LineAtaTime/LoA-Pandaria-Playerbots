/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/DropQuestAction.h) into the
 * Legends of Azeroth Pandaria fork, issue #257 slice E (the quest-log pruning layer: "drop quest" and
 * "clean quest log").
 *
 * No action is registered here - Factory/AiFactory.cpp, ActionContext.h, ChatActionContext.h and
 * WorldPacketHandlerStrategy.cpp are owned by the single later registration slice of #257.
 */

#ifndef _PLAYERBOT_DROPQUESTACTION_H
#define _PLAYERBOT_DROPQUESTACTION_H

#include "Action.h"

class Player;
class PlayerbotAI;
class Quest;

class DropQuestAction : public Action
{
public:
    DropQuestAction(PlayerbotAI* botAI) : Action(botAI, "drop quest") {}

    bool Execute(Event event) override;
};

class CleanQuestLogAction : public Action
{
public:
    CleanQuestLogAction(PlayerbotAI* botAI) : Action(botAI, "clean quest log") {}

    bool Execute(Event event) override;
    void DropQuestType(uint8& numQuest, uint8 wantNum = 100, bool isGreen = false, bool hasProgress = false,
                       bool isComplete = false);

    // 5.4.8: kept as a static member so the header stays free of QuestDef.h (see the .cpp header);
    // AC's signature (Ai/Base/Actions/DropQuestAction.h:33) is unchanged.
    static bool HasProgress(Player* bot, Quest const* quest);
};

#endif
