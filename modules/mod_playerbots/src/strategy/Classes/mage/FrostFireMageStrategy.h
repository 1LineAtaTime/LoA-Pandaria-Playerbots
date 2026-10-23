/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license; you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 */

// #408 (AC parity, AC Strategy/FrostFireMageStrategy.h). The AC Frostfire specialisation does not
// exist on 5.4.8 (#385 lane M: Frostfire Bolt 44614 is a level-1 5.4.8 mage spell, but AC's Ice Shards talent
// gate in AiFactory has no 5.4.8 talent - cls.py mage 'ice shards' -> NOT-LEARNABLE), so this
// strategy is never selected by AiFactory; "co +frostfire" still loads it. It is ported 1:1 so the AiFactory frostfire branch and
// the MageBoostStrategy fire branch stay structurally identical to AC.
#ifndef _PLAYERBOT_FROSTFIREMAGESTRATEGY_H
#define _PLAYERBOT_FROSTFIREMAGESTRATEGY_H

#include "GenericMageStrategy.h"

class PlayerbotAI;

class FrostFireMageStrategy : public GenericMageStrategy
{
public:
    FrostFireMageStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frostfire"; }
    NextAction** getDefaultActions() override;
};

#endif