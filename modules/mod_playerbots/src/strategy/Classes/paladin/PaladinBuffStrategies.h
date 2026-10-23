/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PALADINBUFFSTRATEGIES_H
#define _PLAYERBOT_PALADINBUFFSTRATEGIES_H

#include "Strategy.h"

class PlayerbotAI;

class PaladinBuffManaStrategy : public Strategy
{
public:
    PaladinBuffManaStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bmana"; }
};

class PaladinBuffHealthStrategy : public Strategy
{
public:
    PaladinBuffHealthStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bhealth"; }
};

class PaladinBuffDpsStrategy : public Strategy
{
public:
    PaladinBuffDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bdps"; }
};

// #383 (AC PaladinAiObjectContext.cpp:76-77,83-84): AC key aliases "bmight" -> Dps blessing strategy,
// "bkings" -> Stats blessing strategy. Own getName so Engine::addStrategy/removeStrategy (keyed by
// getName) can remove them by the same key. Triggers are inherited from the aliased MoP strategy.
class PaladinBuffMightStrategy : public PaladinBuffDpsStrategy
{
public:
    PaladinBuffMightStrategy(PlayerbotAI* botAI) : PaladinBuffDpsStrategy(botAI) {}

    std::string const getName() override { return "bmight"; }
};

class PaladinBuffArmorStrategy : public Strategy
{
public:
    PaladinBuffArmorStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "barmor"; }
};

class PaladinBuffAoeStrategy : public Strategy
{
public:
    PaladinBuffAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "baoe"; }
};

class PaladinBuffCastStrategy : public Strategy
{
public:
    PaladinBuffCastStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bcast"; }
};

class PaladinBuffSpeedStrategy : public Strategy
{
public:
    PaladinBuffSpeedStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bspeed"; }
};

class PaladinBuffThreatStrategy : public Strategy
{
public:
    PaladinBuffThreatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bthreat"; }
};

class PaladinBuffStatsStrategy : public Strategy
{
public:
    PaladinBuffStatsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "bstats"; }
};

// #383: AC "bkings" alias (see PaladinBuffMightStrategy above)
class PaladinBuffKingsStrategy : public PaladinBuffStatsStrategy
{
public:
    PaladinBuffKingsStrategy(PlayerbotAI* botAI) : PaladinBuffStatsStrategy(botAI) {}

    std::string const getName() override { return "bkings"; }
};


// AC PaladinBuffStrategies.h:95-120 (1:1). Not wired into any default engine on AC either (only
// Priest gets "rshadow"), so they are registered for parity and manual/config use only.
class PaladinShadowResistanceStrategy : public Strategy
{
public:
    PaladinShadowResistanceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "rshadow"; }
};

class PaladinFrostResistanceStrategy : public Strategy
{
public:
    PaladinFrostResistanceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "rfrost"; }
};

class PaladinFireResistanceStrategy : public Strategy
{
public:
    PaladinFireResistanceStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "rfire"; }
};
#endif
