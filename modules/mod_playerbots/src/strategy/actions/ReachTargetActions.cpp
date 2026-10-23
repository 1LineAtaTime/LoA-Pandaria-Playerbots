/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ReachTargetActions.h"

#include "Event.h"
#include "PlayerbotAIConfig.h"
#include "Playerbots.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "ServerFacade.h"

#include "Log.h"

bool ReachTargetAction::Execute(Event event)
{
    return ReachCombatTo(AI_VALUE(Unit*, GetTargetName()), distance);
}

bool ReachTargetAction::isUseful()
{
    // do not move while staying (AC ReachTargetActions.cpp, #827)
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    // do not move while casting
    if (bot->GetCurrentSpell(CURRENT_CHANNELED_SPELL) != nullptr)
    {
        return false;
    }
    Unit* target = GetTarget();
    // float dis = distance + CONTACT_DISTANCE;
    return target &&
        !bot->IsWithinCombatRange(target, distance);  // sServerFacade->IsDistanceGreaterThan(AI_VALUE2(float,
    // "distance", GetTargetName()), distance);
}

std::string const ReachTargetAction::GetTargetName() { return "current target"; }

CastReachTargetSpellAction::CastReachTargetSpellAction(PlayerbotAI* botAI, std::string const spell, float distance)
    : CastSpellAction(botAI, spell), distance(distance)
{
}
bool CastReachTargetSpellAction::isUseful()
{
    // do not move while staying (AC ReachTargetActions.cpp, #827)
    if (botAI->HasStrategy("stay", botAI->GetState()))
    {
        return false;
    }

    Unit* target = GetTarget();
    if (!target)
        return false;

    float const dist = AI_VALUE2(float, "distance", "current target");
    if (!sServerFacade->IsDistanceGreaterThan(dist, (distance + sPlayerbotAIConfig->contactDistance)))
        return false;

    // issue #98: there was no upper bound here, and PlayerbotAI::CanCastSpell deliberately lets
    // SPELL_FAILED_OUT_OF_RANGE through (for casts broken by movement). So a bot whose target was
    // out of a gap-closer's range kept re-picking it every tick and never reached melee - measured
    // 35 charge attempts and 35 heroic-leap attempts for one bot in five minutes, with no effect.
    // Bound it by the spell's own maximum range.
    uint32 const spellId = AI_VALUE2(uint32, "spell id", spell);
    SpellInfo const* spellInfo = spellId ? sSpellMgr->GetSpellInfo(spellId) : nullptr;
    if (!spellInfo)
        return false;

    float const maxRange = spellInfo->GetMaxRange(false, bot);
    return !(maxRange > 0.0f && dist > maxRange);
}

ReachSpellAction::ReachSpellAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach spell", botAI->GetRange("spell"))
{
}

ReachPartyMemberToHealAction::ReachPartyMemberToHealAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to heal", botAI->GetRange("heal"))
{
}

std::string const ReachPartyMemberToHealAction::GetTargetName() { return "party member to heal"; }

ReachPartyMemberToResurrectAction::ReachPartyMemberToResurrectAction(PlayerbotAI* botAI)
    : ReachTargetAction(botAI, "reach party member to resurrect", botAI->GetRange("spell"))
{
}

std::string const ReachPartyMemberToResurrectAction::GetTargetName() { return "party member to resurrect"; }
