/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #230 D2: AC's "attack tagged" strategy (AC Ai/Base/Strategy/NonCombatStrategy.h:43-50, registered at AC
 * StrategyContext.h:116/191). A marker strategy: AttackersValue::IsPossibleTarget (strategy/value/AttackersValue.cpp:255-258,
 * AC AttackersValue.cpp:241-242) lets a bot that has it attack creatures tagged by someone outside its group. AC adds it
 * only in battlegrounds/arenas (AC AiFactory.cpp:678-679; MoP Factory/AiFactory.cpp:936-937 already lists it), but it was
 * never registered on this fork, so addStrategiesNoInit dropped the name. Kept in its own header (not NonCombatStrategy.h,
 * which ~20 class strategy headers include) so registering it rebuilds StrategyContext.h's two includers only.
 */

#ifndef _PLAYERBOT_ATTACKTAGGEDSTRATEGY_H
#define _PLAYERBOT_ATTACKTAGGEDSTRATEGY_H

#include "Strategy.h"

class PlayerbotAI;

class AttackTaggedStrategy : public Strategy
{
public:
    AttackTaggedStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    std::string const getName() override { return "attack tagged"; }
};

#endif
