/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PaladinActions.h"

#include "AiFactory.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SharedDefines.h"


bool CastSealSpellAction::isUseful() { return AI_VALUE2(bool, "combat", "self target"); }

Value<Unit*>* CastTurnUndeadAction::GetTargetValue() { return context->GetValue<Unit*>("cc target", getName()); }

// #163 S8: same movement-impaired check as PlayerbotAI::IsMovementImpaired on AC
// (Bot/PlayerbotAI.cpp:2109-2112); not added to PlayerbotAI.h/.cpp on this fork, so duplicated here
// and in PartyMemberSnaredTargetValue.cpp (the two are small enough that sharing isn't worth a new
// header).
static bool IsSnared(Unit* unit)
{
    // 5.4.8: Unit::IsRooted() doesn't exist on this fork; HasUnitState(UNIT_STATE_ROOT) is the
    // equivalent (root effects apply this state regardless of source).
    return unit &&
        (unit->HasAuraType(SPELL_AURA_MOD_ROOT) || unit->HasUnitState(UNIT_STATE_ROOT) || unit->GetSpeedRate(MOVE_RUN) < 1.0f);
}

Unit* CastHandOfFreedomOnPartyAction::GetTarget()
{
    bool const selfImpaired = IsSnared(bot) && !botAI->HasAura("hand of freedom", bot);

    if (!bot->GetGroup())
        return selfImpaired ? bot : nullptr;

    if (selfImpaired)
        return bot;

    return CastBuffSpellAction::GetTarget();
}

Value<Unit*>* CastHandOfFreedomOnPartyAction::GetTargetValue()
{
    return context->GetValue<Unit*>("party member snared target");
}

bool CastHandOfFreedomOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    return CastBuffSpellAction::isUseful() && !botAI->HasAura("hand of freedom", target);
}

Unit* CastRighteousDefenseAction::GetTarget()
{
    Unit* current_target = AI_VALUE(Unit*, "current target");
    if (!current_target)
    {
        return NULL;
    }
    return current_target->GetVictim();
}


bool CastDivineSacrificeAction::isUseful()
{
    return GetTarget() && (GetTarget() != nullptr) && CastSpellAction::isUseful() &&
           !botAI->HasAura("divine guardian", GetTarget(), false, false, -1, true);
}

bool CastCancelDivineSacrificeAction::Execute(Event event)
{
    botAI->RemoveAura("divine sacrifice");
    return true;
}

bool CastCancelDivineSacrificeAction::isUseful()
{
    return botAI->HasAura("divine sacrifice", GetTarget(), false, true, -1, true);
}

bool CastCrusaderStrikeAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target) return false;

    return !botAI->HasAura("weakened blows", GetTarget(), false, true, -1, false) || bot->GetPower(Powers::POWER_HOLY_POWER) < 3;
}

// ---- rotation lane 6 (#861) ----
#include "PlayerbotSpec.h"

// Divine Shield only when NOT carrying a PvP objective (flag/orb); shared value from lane 0.
bool CastDivineShieldAction::isUseful()
{
    return CastBuffSpellAction::isUseful() && !AI_VALUE(bool, "carrying objective");
}

static bool HasHolyPowerForSpender(Player* bot)
{
    // 3 Holy Power, or the Divine Purpose proc (90174) which makes the next spender free
    return bot->GetPower(POWER_HOLY_POWER) >= 3 || bot->HasAura(90174);
}

// Prot self and holy party Eternal Flame: holy power value >= 3
bool CastEternalFlameAction::isUseful() { return HasHolyPowerForSpender(bot) && CastHealingSpellAction::isUseful(); }

bool CastEternalFlameOnPartyAction::isUseful()
{
    return HasHolyPowerForSpender(bot) && HealPartyMemberAction::isUseful();
}

// Execution Sentence on the tank when it is <= 60% health
bool CastExecutionSentenceOnPartyAction::isUseful()
{
    Unit* target = GetTarget();
    if (!target || !target->ToPlayer() || target->GetHealthPct() > 60.0f || !PlayerBotSpec::IsTank(target->ToPlayer()))
        return false;

    return HealPartyMemberAction::isUseful();
}
