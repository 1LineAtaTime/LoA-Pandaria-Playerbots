/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Monk class strategies for the Legends of Azeroth Pandaria fork (issue #67). Modeled on the other
 * class strategies in this fork; casts are resolved by spell name through SpellIdValue.
 */

#ifndef _PLAYERBOT_MONKSTRATEGIES_H
#define _PLAYERBOT_MONKSTRATEGIES_H

#include "CombatStrategy.h"
#include "NonCombatStrategy.h"
#include "PullStrategy.h"
#include "Strategy.h"

class PlayerbotAI;

/// #140: shared monk combat strategy, a CombatStrategy like GenericWarriorStrategy / GenericPaladinStrategy
/// (so every spec, Mistweaver included, gets the CombatStrategy base triggers): melee filler plus the
/// interrupt (#526) and defensive edges every spec shares.
class MonkStrategy : public CombatStrategy
{
public:
    MonkStrategy(PlayerbotAI* botAI) : CombatStrategy(botAI) {}
    std::string const getName() override { return "monk"; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Windwalker (dps). Registered as "melee", which is the name AiFactory uses for the spec.
class WindwalkerMonkStrategy : public MonkStrategy
{
public:
    WindwalkerMonkStrategy(PlayerbotAI* botAI) : MonkStrategy(botAI) {}
    std::string const getName() override { return "melee"; }
    // #140 (AC ArmsWarriorStrategy parity, MoP ArmsWarriorStrategy.h:22).
    uint32 GetType() const override { return STRATEGY_TYPE_COMBAT | STRATEGY_TYPE_DPS | STRATEGY_TYPE_MELEE; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Brewmaster (tank). Registered as "tank".
class BrewmasterMonkStrategy : public MonkStrategy
{
public:
    BrewmasterMonkStrategy(PlayerbotAI* botAI) : MonkStrategy(botAI) {}
    std::string const getName() override { return "tank"; }
    // #140 (MoP TankWarriorStrategy.h:22 / TankPaladinStrategy.h:21).
    uint32 GetType() const override { return STRATEGY_TYPE_TANK | STRATEGY_TYPE_MELEE; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Mistweaver (healer). Registered as "heal".
class MistweaverMonkStrategy : public MonkStrategy
{
public:
    MistweaverMonkStrategy(PlayerbotAI* botAI) : MonkStrategy(botAI) {}
    std::string const getName() override { return "heal"; }
    // #140 (MoP HealPaladinStrategy.h:21 / HealDruidStrategy.h:21: healers are HEAL | RANGED).
    uint32 GetType() const override { return STRATEGY_TYPE_HEAL | STRATEGY_TYPE_RANGED; }
    NextAction** getDefaultActions() override;
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Spinning Crane Kick. Registered as "aoe" (Brewmaster) and, via MonkMeleeAoeStrategy, "melee aoe" (Windwalker).
class MonkAoeStrategy : public Strategy
{
public:
    MonkAoeStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "aoe"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// #526: Mistweaver damage when nobody needs a heal (AC PriestHealerDpsStrategy shape). Registered as "healer dps".
class MonkHealerDpsStrategy : public Strategy
{
public:
    MonkHealerDpsStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "healer dps"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// #526: AiFactory asks the windwalker for "melee aoe"; same behaviour, registered under that name.
class MonkMeleeAoeStrategy : public MonkAoeStrategy
{
public:
    MonkMeleeAoeStrategy(PlayerbotAI* botAI) : MonkAoeStrategy(botAI) {}
    std::string const getName() override { return "melee aoe"; }
};

class GenericMonkNonCombatStrategy : public NonCombatStrategy
{
public:
    GenericMonkNonCombatStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "nc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Detox, for the "cure" slot AiFactory asks every class for.
class MonkCureStrategy : public Strategy
{
public:
    MonkCureStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "cure"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// #526: Tigereye Brew / Touch of Death (AC PaladinBoostStrategy / RogueBoostStrategy slot).
class MonkBoostStrategy : public Strategy
{
public:
    MonkBoostStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "boost"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// #526: Paralysis (AC PaladinCcStrategy / RogueCcStrategy slot).
class MonkCcStrategy : public Strategy
{
public:
    MonkCcStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}
    std::string const getName() override { return "cc"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// #526: brewmaster pull. A monk has no ranged weapon, so the generic "shoot" pull can never run;
/// like AC PaladinPullStrategy (Ai/Class/Paladin/Strategy/PaladinPullStrategy.cpp) the tank pulls with
/// its ranged taunt, Provoke 115546 (40 yd).
class MonkPullStrategy : public PullStrategy
{
public:
    MonkPullStrategy(PlayerbotAI* botAI) : PullStrategy(botAI, "shoot") {}

    std::string GetPullActionName() const override;
};

#endif
