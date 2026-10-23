/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "HunterActions.h"

#include "Event.h"
#include "GenericSpellActions.h"
#include "Playerbots.h"

bool CastHuntersMarkAction::isUseful() { return CastDebuffSpellAction::isUseful(); }

// #380: 5.4.8 Kill Command 34026 "The pet must be within 25 yards of the target to Kill Command" (Wowhead mop-classic
// tooltip spell 34026, 2026-10-05). AC has no such gate (WotLK), but the BM trail showed 34026 BAD_TARGETS (13) x16.
// Round 3: the 5.4.8 spell is cast ON THE ENEMY (100 yd range, pet "instantly inflicts damage to its target and applies
// Hunter's Mark" - same tooltip), not on the pet as in AC/WotLK, so the pet target (BAD_TARGETS 13 x16 in wave 11) is
// now GetTargetName() = "current target" (HunterActions.h); the pet is read from the "pet target" value here.
bool CastKillCommandAction::isUseful()
{
    Unit* pet = AI_VALUE(Unit*, "pet target");
    Unit* target = GetTarget();
    if (!pet || !target || !pet->IsAlive() || pet->GetDistance(target) > 25.0f)
        return false;

    return CastAuraSpellAction::isUseful();
}

bool CastViperStingAction::isUseful()
{
    return AI_VALUE2(uint8, "mana", "self target") < 50 && AI_VALUE2(uint8, "mana", "current target") >= 30;
}

bool CastAspectOfTheCheetahAction::isUseful()
{
    return !botAI->HasAnyAuraOf(GetTarget(), "aspect of the cheetah", "aspect of the pack", nullptr);
}

// Rotation fix lane A (#856), A Murder of Crows, "always"): w73 Survival cast it 7x and
// every cast failed NO_POWER (result 88) - the cost is 60 focus and Explosive Shot / Cobra Shot spent the focus below the cost on
// every tick, so it never reached 60. When the spell is ready and usable but short of focus by <= 25 (about 5 s of regeneration),
// this tick is spent waiting (returns true, casts nothing) so the focus pools and the next tick casts it.
bool CastMurderOfCrowsAction::Execute(Event event)
{
    uint32 const spellId = AI_VALUE2(uint32, "spell id", "a murder of crows");
    if (SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr)
    {
        int32 const cost = spellInfo->CalcPowerCost(bot, spellInfo->GetSchoolMask());
        int32 const focus = int32(bot->GetPower(POWER_FOCUS));
        if (cost > 0 && focus < cost && focus + 25 >= cost && !bot->HasSpellCooldown(spellId))
            return true;
    }

    return CastDebuffSpellAction::Execute(event);
}

bool CastExplosiveShotRank1Action::Execute(Event /*event*/) { return botAI->CastSpell(53301, GetTarget()); }
bool CastExplosiveShotRank2Action::Execute(Event /*event*/) { return botAI->CastSpell(53301, GetTarget()); }
bool CastExplosiveShotRank3Action::Execute(Event /*event*/) { return botAI->CastSpell(53301, GetTarget()); }
bool CastExplosiveShotRank4Action::Execute(Event /*event*/) { return botAI->CastSpell(53301, GetTarget()); }

bool CastExplosiveShotRank1Action::isUseful()
{
    Unit* target = GetTarget();
    return target && !target->HasAura(53301);
}

bool CastExplosiveShotRank2Action::isUseful()
{
    Unit* target = GetTarget();
    return target && !target->HasAura(53301);
}

bool CastExplosiveShotRank3Action::isUseful()
{
    Unit* target = GetTarget();
    return target && !target->HasAura(53301);
}

bool CastExplosiveShotRank4Action::isUseful()
{
    Unit* target = GetTarget();
    return target && !target->HasAura(53301);
}

// #521 (AC HunterActions.cpp:77-95): face the target, then leap back. AC's isUseful() (no Disengage while the
// "trap weave" strategy is active) is not ported: that strategy is not registered on this fork (Immolation Trap
// is not learnable in 5.4.8, see HunterTriggers.h), so the check would always pass.
bool CastDisengageAction::Execute(Event event)
{
    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    bot->SetOrientation(bot->GetAngle(target));
    return CastSpellAction::Execute(event);
}

Value<Unit*>* CastFreezingTrap::GetTargetValue() { return context->GetValue<Unit*>("cc target", "freezing trap"); }

bool CastAutoShotAction::isUseful()
{
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    if (AI_VALUE(Unit*, "current target") && bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL) &&
        bot->GetCurrentSpell(CURRENT_AUTOREPEAT_SPELL)->m_targets.GetUnitTargetGUID() ==
            AI_VALUE(Unit*, "current target")->GetGUID())
    {
        return false;
    }
    return AI_VALUE(uint32, "active spell") != AI_VALUE2(uint32, "spell id", getName());
}

Value<Unit*>* CastScareBeastCcAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", "scare beast"); }

bool CastScareBeastCcAction::Execute(Event event) { return botAI->CastSpell("scare beast", GetTarget()); }

bool CastWingClipAction::isUseful() { return CastSpellAction::isUseful() && !botAI->HasAura(spell, GetTarget()); }

NextAction** CastWingClipAction::getPrerequisites() { return nullptr; }

// Counter Shot on any casting enemy, battleground/arena only (#856).
bool CastCounterShotOnEnemyCasterAction::isUseful()
{
    return AI_VALUE(bool, "in pvp instance") && CastSpellOnEnemyCasterAction::isUseful();
}
