/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Strategy/LfgStrategy.h) into the
 * Legends of Azeroth Pandaria fork (#210 S10 G11 registration). #142: AC's "random" -> "lfg join" and
 * "seldom" -> "lfg leave" triggers are ported (LfgStrategy.cpp), next to "unknown dungeon" -> "give
 * leader in dungeon".
 *
 * Registered in StrategyContext.h as "lfg" and added to a random bot's non-combat engine by AiFactory
 * when AiPlayerbot.RandomBotJoinLfg is set (AC AiFactory.cpp:601-602).
 */

#ifndef _PLAYERBOT_LFGSTRATEGY_H
#define _PLAYERBOT_LFGSTRATEGY_H

#include "PassTroughStrategy.h"

class LfgStrategy : public PassTroughStrategy
{
public:
    LfgStrategy(PlayerbotAI* botAI);

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "lfg"; }
};

#endif
