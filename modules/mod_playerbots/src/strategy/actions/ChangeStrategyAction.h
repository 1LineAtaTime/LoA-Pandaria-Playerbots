#ifndef _PLAYERBOT_CHANGESTRATEGYACTION_H
#define _PLAYERBOT_CHANGESTRATEGYACTION_H

#include "Action.h"

class PlayerbotAI;

class ChangeCombatStrategyAction : public Action
{
public:
    ChangeCombatStrategyAction(PlayerbotAI* botAI, std::string const name = "co") : Action(botAI, name) {}

    bool Execute(Event event) override;
};

class ChangeNonCombatStrategyAction : public Action
{
public:
    ChangeNonCombatStrategyAction(PlayerbotAI* botAI) : Action(botAI, "nc") {}

    bool Execute(Event event) override;
};

class ChangeDeadStrategyAction : public Action
{
public:
    ChangeDeadStrategyAction(PlayerbotAI* botAI) : Action(botAI, "de") {}

    bool Execute(Event event) override;
};

/// Ported from AC mod-playerbots CombatActions.h/.cpp (#149). AC's isUseful() also has a
/// CLASS_HUNTER branch gated on "item count"::"ammo" (ammo does not exist on 5.4.8, removed in
/// Cataclysm) - dropped here, so hunters fall through to the same rule as every other ranged class.
class SwitchToMeleeAction : public ChangeCombatStrategyAction
{
public:
    SwitchToMeleeAction(PlayerbotAI* botAI) : ChangeCombatStrategyAction(botAI, "-ranged,+close") {}

    bool isUseful() override;
};

class SwitchToRangedAction : public ChangeCombatStrategyAction
{
public:
    SwitchToRangedAction(PlayerbotAI* botAI) : ChangeCombatStrategyAction(botAI, "-close,+ranged") {}

    bool isUseful() override;
};

#endif
