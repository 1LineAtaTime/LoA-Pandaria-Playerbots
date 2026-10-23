/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/QuestStrategies.h) into the
 * Legends of Azeroth Pandaria fork, issue #257 slice H (registration slice).
 */

#ifndef _PLAYERBOT_QUESTSTRATEGIES_H
#define _PLAYERBOT_QUESTSTRATEGIES_H

#include "PassTroughStrategy.h"

class PlayerbotAI;

class QuestStrategy : public PassTroughStrategy
{
public:
    QuestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

class DefaultQuestStrategy : public QuestStrategy
{
public:
    DefaultQuestStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "quest"; }
};

class AcceptAllQuestsStrategy : public QuestStrategy
{
public:
    AcceptAllQuestsStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "accept all quests"; }
};

#endif