/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H
#define _PLAYERBOT_TOTEMSSHAMANSTRATEGY_H

#include "GenericShamanStrategy.h"

class PlayerbotAI;

class TotemsShamanStrategy : public GenericShamanStrategy
{
public:
    TotemsShamanStrategy(PlayerbotAI* botAI);

    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "totems"; }
};

// #431 (AC parity, AC Strategy/TotemsShamanStrategy.h:176-364): one small combat strategy per totem. Each one
// is registered under the AC name (ShamanAiObjectContext.cpp) in its element's sibling context, so a bot holds at
// most one strategy per element (earth / fire / water / air). The per-totem NoXTotem triggers (ShamanTriggers.cpp)
// read these strategies through botAI->HasStrategy() to know which totem the slot should hold.

// Earth Totem Strategies
class StrengthOfEarthTotemStrategy : public GenericShamanStrategy
{
public:
    StrengthOfEarthTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "strength of earth"; }
};

class StoneskinTotemStrategy : public GenericShamanStrategy
{
public:
    StoneskinTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "stoneskin"; }
};

class EarthTotemStrategy : public GenericShamanStrategy
{
public:
    EarthTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "tremor"; }
};

class EarthbindTotemStrategy : public GenericShamanStrategy
{
public:
    EarthbindTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "earthbind"; }
};

// Fire Totem Strategies
class SearingTotemStrategy : public GenericShamanStrategy
{
public:
    SearingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "searing"; }
};

class MagmaTotemStrategy : public GenericShamanStrategy
{
public:
    MagmaTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "magma"; }
};

class FlametongueTotemStrategy : public GenericShamanStrategy
{
public:
    FlametongueTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "flametongue"; }
};

class TotemOfWrathStrategy : public GenericShamanStrategy
{
public:
    TotemOfWrathStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "wrath"; }
};

class FrostResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    FrostResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "frost resistance"; }
};

// Water Totem Strategies
class HealingStreamTotemStrategy : public GenericShamanStrategy
{
public:
    HealingStreamTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "healing stream"; }
};

class ManaSpringTotemStrategy : public GenericShamanStrategy
{
public:
    ManaSpringTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "mana spring"; }
};

class CleansingTotemStrategy : public GenericShamanStrategy
{
public:
    CleansingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "cleansing"; }
};

class FireResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    FireResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "fire resistance"; }
};

// Air Totem Strategies
class WrathOfAirTotemStrategy : public GenericShamanStrategy
{
public:
    WrathOfAirTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "wrath of air"; }
};

class WindfuryTotemStrategy : public GenericShamanStrategy
{
public:
    WindfuryTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "windfury"; }
};

class NatureResistanceTotemStrategy : public GenericShamanStrategy
{
public:
    NatureResistanceTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "nature resistance"; }
};

class GroundingTotemStrategy : public GenericShamanStrategy
{
public:
    GroundingTotemStrategy(PlayerbotAI* botAI);
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
    std::string const getName() override { return "grounding"; }
};

#endif
