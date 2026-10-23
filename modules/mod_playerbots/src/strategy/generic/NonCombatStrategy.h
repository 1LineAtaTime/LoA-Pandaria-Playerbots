#ifndef _PLAYERBOT_NONCOMBATSTRATEGY_H
#define _PLAYERBOT_NONCOMBATSTRATEGY_H

#include "PassTroughStrategy.h"

class PlayerbotAI;
class NonCombatStrategy : public Strategy
{
public:
    NonCombatStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    uint32 GetType() const override { return STRATEGY_TYPE_NONCOMBAT; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// Marker strategy (upstream parity, #529): no triggers of its own; CheckMountStateAction::isUseful
/// checks that it is present. "check mount state" runs from the "nc" strategy, as on AC.
class MountStrategy : public Strategy
{
public:
    MountStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "mount"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// issue #62 / #151: harvest herb/mining nodes. The "loot" *Strategy* that used to live here (issue
/// #62) is now LootNonCombatStrategy (strategy/generic/LootNonCombatStrategy.h) - AC's item-loot
/// interface class is also named LootStrategy (strategy/value/LootObjectStack.h), and both headers
/// are pulled into the same AiObjectContext.cpp translation unit, so one of the two had to move
/// (loot-stack-port.md section 2 step 6).
class GatherStrategy : public Strategy
{
public:
    GatherStrategy(PlayerbotAI* botAI) : Strategy(botAI) {}

    std::string const getName() override { return "gather"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// AC GrindingStrategy (Ai/Base/Strategy/GrindingStrategy.*): attack the grind target when there is none.
class GrindingStrategy : public NonCombatStrategy
{
public:
    GrindingStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "grind"; }
    uint32 GetType() const override { return STRATEGY_TYPE_DPS; }
    NextAction** getDefaultActions() override;  // #527: AC "drink" 4.2 / "food" 4.1
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

/// AC MoveRandomStrategy: used only when new rpg is disabled.
class MoveRandomStrategy : public NonCombatStrategy
{
public:
    MoveRandomStrategy(PlayerbotAI* botAI) : NonCombatStrategy(botAI) {}
    std::string const getName() override { return "move random"; }
    void InitTriggers(std::vector<TriggerNode*>& triggers) override;
};

#endif
