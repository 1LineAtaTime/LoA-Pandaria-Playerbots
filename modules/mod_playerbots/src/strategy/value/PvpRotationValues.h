/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #855 (epic #854, rotation lane 0): shared values for the rotation ports. AC mod-playerbots has no
 * equivalent; they extend AC's value pattern (EnemyHealerTargetValue / IsMovingValue) with the MoP rotation-condition
 * semantics:
 *   "enemy caster target"  condition 17 CastingSpell p2 bit 1 (interruptible) on action target type 2 ("each valid enemy
 *                          around", the "random enemy" interrupt rows of priority group 10)
 *   "is cc"                condition 16 CC p1 = 0 (aura type 5/6/7/12/407) widened to the contract's mechanic mask
 *   "carrying objective"   condition 46 CarryingPvPObjective (flag / orb / cart)
 *   "in pvp instance"      condition 22 CheckMapType p1 3 (battleground) or 4 (arena)
 */

#ifndef _PLAYERBOT_PVPROTATIONVALUES_H
#define _PLAYERBOT_PVPROTATIONVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"

class PlayerbotAI;
class Unit;

// Nearest hostile unit within 30 yd and line of sight that is casting or channelling a spell an interrupt can stop
// (the core's own EffectInterruptCast test). Players first, then healers (a positive spell being cast, or a healer spec),
// then the nearest. Candidates: the current target + "attackers" (threat list, group attackers, and in battlegrounds /
// arenas the enemy players near the bot - AttackersValue). nullptr when none.
class EnemyCasterTargetValue : public UnitCalculatedValue
{
public:
    EnemyCasterTargetValue(PlayerbotAI* botAI) : UnitCalculatedValue(botAI, "enemy caster target") {}

    static bool IsCastingInterruptible(Unit* unit);

protected:
    Unit* Calculate() override;
};

// "is cc::<unit value>" (default "current target"): the unit is stunned, feared, polymorphed / incapacitated, charmed,
// rooted or silenced by an aura. Used for "only stun / fear targets that are not already CC'd".
class IsCcValue : public BoolCalculatedValue, public Qualified
{
public:
    IsCcValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "is cc") {}

    static bool IsCrowdControlled(Unit* unit);

    bool Calculate() override;
};

// "carrying objective" (the bot) or "carrying objective::<unit value>": the unit carries a battleground flag (WSG / TP /
// EotS), a Kotmogu orb or a Deepwind cart. Classes skip Divine Shield / Stealth / mount-breaking spells then.
class CarryingObjectiveValue : public BoolCalculatedValue, public Qualified
{
public:
    CarryingObjectiveValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "carrying objective") {}

    static bool IsCarrying(Unit* unit);

    bool Calculate() override;
};

// "in pvp instance": the bot is on a battleground or arena map.
class InPvpInstanceValue : public BoolCalculatedValue
{
public:
    InPvpInstanceValue(PlayerbotAI* botAI) : BoolCalculatedValue(botAI, "in pvp instance") {}

    bool Calculate() override;
};

#endif
