/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Strategy/NewRpgStrategy.h)
 * into the Legends of Azeroth Pandaria fork.
 */

#ifndef _PLAYERBOT_NEWRPGSTRATEGY_H
#define _PLAYERBOT_NEWRPGSTRATEGY_H

#include "NewRpgInfo.h"
#include "Strategy.h"

class PlayerbotAI;

class NewRpgStrategy : public Strategy
{
public:
    NewRpgStrategy(PlayerbotAI* botAI);

    std::string const getName() override { return "new rpg"; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    void InitMultipliers(std::vector<Multiplier*>& multipliers) override;
};

#endif
