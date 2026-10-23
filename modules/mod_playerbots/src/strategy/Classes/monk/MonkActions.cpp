/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #194 (S8): the one Monk action that needs a hand-written isUseful() (CastUpliftOnPartyAction), so it
 * can't be a one-line macro in MonkActions.h like every other Monk action. Everything else stays in
 * the header.
 */

#include "MonkActions.h"

#include "Playerbots.h"

bool CastUpliftOnPartyAction::isUseful()
{
    return HealPartyMemberAction::isUseful() && AI_VALUE2(uint8, "chi", "self target") >= 2;
}

bool CastTigerPalmAction::isUseful()
{
    if (!CastMeleeSpellAction::isUseful())
        return false;

    // #526 (wave 11): Tiger Power 125359 does not stack in 5.4.8 (SpellAuraOptions.dbc StackAmount=1; Wowhead mop-classic
    // tooltip 100787, 2026-10-05: "ignore 30% of enemies' armor for 20 sec"). The old "stacks < 3" test was always true, so
    // Tiger Palm (1 Chi, +0.2) fired at every Chi and starved Blackout Kick / Rising Sun Kick (2 Chi). Refresh upkeep only.
    Aura* power = botAI->GetAura("tiger power", bot);
    return !power || power->GetDuration() < 4000;
}

bool CastPurifyingBrewAction::isUseful()
{
    // Wave-75 fix J (#862), MoP rotation row 26817 (brewmaster, prio group 5/1): Purifying Brew also at health <= 40%
    // (self, in combat) whatever the Stagger level, next to the Moderate (26810) / Heavy (26811) Stagger rows.
    return CastBuffSpellAction::isUseful() &&
           (bot->HasAura(124274) || bot->HasAura(124273) ||  // Moderate / Heavy Stagger
            bot->GetHealthPct() <= 40.0f);
}

bool CastShuffleAction::isUseful()
{
    if (!CastMeleeSpellAction::isUseful())
        return false;

    Aura* shuffle = botAI->GetAura("shuffle", bot);
    return !shuffle || shuffle->GetDuration() < 3000;
}

// #526 (AC pattern: BoostTrigger::IsActive, GenericTriggers.cpp:364-372 on this fork): the same
// "player target or balance <= 50" gate, with SpellCanBeCastTrigger replacing BuffTrigger's no-aura
// test (see MonkActions.h for why). Wave 10: CanCastSpell does not test the cast's CASTER_AURASTATE
// (result 24 x63), so the trigger itself requires a Tigereye Brew stack (aura 125195, Wowhead tooltip
// 2026-10-05) and no running 116740 buff.
bool TigereyeBrewTrigger::IsActive()
{
    if (!bot->HasAura(125195) || bot->HasAura(116740))
        return false;

    if (!SpellCanBeCastTrigger::IsActive())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (target && target->ToPlayer())
        return true;

    return AI_VALUE(uint8, "balance") <= 50.f;
}

// #526: Touch of Death 115080 tooltip (Wowhead, 2026-10-04): only usable on creatures with equal or less
// health than the monk, or on players at or below 10% health.
bool TouchOfDeathTrigger::IsActive()
{
    Unit* target = GetTarget();
    if (!target || !SpellCanBeCastTrigger::IsActive())
        return false;

    // Wave 10: the trigger stayed active every tick during the 90 s cooldown (cast result "cooldown" x30, action
    // PREREQ/FAILED); gate on the cooldown and on the 3 Chi it costs (tooltip 115080, 2026-10-05) as AC rogue
    // finishers gate on combo points.
    uint32 spellId = AI_VALUE2(uint32, "spell id", "touch of death");
    if (!spellId || bot->HasSpellCooldown(spellId) || AI_VALUE2(uint8, "chi", "self target") < 3)
        return false;

    if (target->GetTypeId() == TYPEID_PLAYER)
        return target->GetHealth() * 10 <= target->GetMaxHealth();

    return target->GetHealth() <= bot->GetHealth();
}

// #862 MoP rotation row 27013: Mana Tea needs >= 2 stacks of the stack aura 115867.
bool CastManaTeaAction::isUseful()
{
    if (!CastSpellAction::isUseful())
        return false;

    Aura* stacks = bot->GetAura(115867);
    return stacks && stacks->GetStackAmount() >= 2;
}

// #862 MoP rotation row 26809 / 26916: >= 7 stacks of 128939, buff 115308 not already running.
bool ElusiveBrewTrigger::IsActive()
{
    Aura* stacks = bot->GetAura(128939);
    if (!stacks || stacks->GetStackAmount() < 7 || bot->HasAura(115308))
        return false;

    return SpellCanBeCastTrigger::IsActive();
}

// #862 MoP rotation row 1001 (chi <= 2, all specs) / 26915 (Windwalker: energy <= 30%).
bool ChiBrewTrigger::IsActive()
{
    bool lowChi = AI_VALUE2(uint8, "chi", "self target") <= 2;
    bool lowEnergy = bot->GetSpecialization() == Specializations::SPEC_MONK_WINDWALKER &&
                     AI_VALUE2(uint8, "energy", "self target") <= 30;
    if (!lowChi && !lowEnergy)
        return false;

    return SpellCanBeCastTrigger::IsActive();
}

// rot-fix-F: Energizing Brew (MoP rotation row 26914) - Windwalker, Energy <= 75%.
bool EnergizingBrewTrigger::IsActive()
{
    if (bot->GetSpecialization() != Specializations::SPEC_MONK_WINDWALKER)
        return false;
    if (AI_VALUE2(uint8, "energy", "self target") > 75)
        return false;

    return SpellCanBeCastTrigger::IsActive();
}

// #862 MoP rotation row 26812 / 27006: cast the statue only when the bot has none standing nearby.
bool MonkStatueTrigger::IsActive()
{
    if (!SpellCanBeCastTrigger::IsActive())
        return false;

    if (Creature* statue = bot->FindNearestCreature(statueEntry, 100.0f, true))
        if (statue->GetOwnerGUID() == bot->GetGUID())
            return false;

    return true;
}

bool CastSpearHandStrikeOnEnemyCasterAction::isUseful()
{
    Unit* target = GetTarget();
    return target && CastSpellOnEnemyCasterAction::isUseful() && bot->IsWithinMeleeRange(target);
}

// #862 MoP rotation row 1010: not mounted, target 11+ yd away (Roll covers ~20 yd), PvP instances only.
bool CastRollGapAction::isUseful()
{
    if (!AI_VALUE(bool, "in pvp instance") || bot->IsMounted())
        return false;

    Unit* target = AI_VALUE(Unit*, "current target");
    if (!target)
        return false;

    float dist = bot->GetDistance(target);
    return dist >= 11.0f && dist <= 30.0f && CastSpellAction::isUseful();
}
