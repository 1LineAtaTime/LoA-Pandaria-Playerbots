/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSpm actions (epic #373, issue #653).

#include "MopSpmActions.h"
#include "MopSpmShared.h"
#include "EncounterHelpers.h"
#include "Playerbots.h"
#include <vector>

bool MopSpmAvoidAction::Execute(Event /*event*/)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    std::vector<MopSpm::Hazard> hazards;
    MopSpm::CollectHazards(bot, hazards);
    if (hazards.empty() ||
        !MopSpm::InAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), MopSpm::DANGER_GRACE))
    {
        return false;
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!MopSpm::FindSafePoint(bot, hazards, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
                  false);
}

bool MopSpmParryStopAction::Execute(Event /*event*/)
{
    bot->AttackStop();
    bot->InterruptSpell(CURRENT_AUTOREPEAT_SPELL);
    botAI->InterruptSpell();
    return true;
}

bool MopSpmMeditateAction::Execute(Event /*event*/)
{
    // The Meditate button is an action-bar override of the Haze of Hate aura, so the bot does not "know" the spell:
    // cast it as a triggered self cast (the same way the fork's own scripts apply it to players).
    bot->CastSpell(bot, MopSpm::SPELL_MEDITATE, true);
    return true;
}

bool MopSpmMarkPriorityAction::Execute(Event /*event*/)
{
    Unit* target = MopSpm::GetPriorityMarkTarget(bot);
    return target && EncounterHelpers::MarkTargetWithSkull(bot, target);
}
