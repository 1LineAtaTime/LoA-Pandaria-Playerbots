/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MopSsb actions (epic #373, issue #652).

#include "MopSsbActions.h"
#include "MopSsbShared.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "MoveSplineInit.h"
#include "Playerbots.h"
#include <algorithm>
#include <cmath>
#include <vector>

bool MopSsbAvoidAction::Execute(Event /*event*/)
{
    if (IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    std::vector<MopSsb::Hazard> hazards;
    MopSsb::CollectHazards(bot, hazards);
    if (hazards.empty() ||
        !MopSsb::InAnyHazard(hazards, bot->GetPositionX(), bot->GetPositionY(), MopSsb::DANGER_GRACE))
    {
        return false;
    }

    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!MopSsb::FindSafePoint(bot, hazards, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, false, false, MovementPriority::MOVEMENT_COMBAT, true,
                  false);
}

bool MopSsbBlackoutStepAction::Execute(Event /*event*/)
{
    Creature* boss = bot->FindNearestCreature(MopSsb::NPC_YAN_ZHU, MopSsb::SCAN_RANGE, true);
    if (!boss || IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    float const bossX = boss->GetPositionX();
    float const bossY = boss->GetPositionY();
    float const botX = bot->GetPositionX();
    float const botY = bot->GetPositionY();
    float const radius = std::max(boss->GetExactDist2d(botX, botY), MopSsb::BLACKOUT_MIN_RADIUS);
    float const angle = boss->GetAngle(botX, botY);

    // An arc keeps the bot at its current distance from the (stationary) boss, so a melee bot stays in melee range.
    float const sweep = MopSsb::BLACKOUT_STEP / radius;
    for (int attempt = 0; attempt < 2; ++attempt)
    {
        float const used = attempt == 0 ? _direction : -_direction;
        float const destAngle = angle + sweep * used;
        float const destX = bossX + radius * std::cos(destAngle);
        float const destY = bossY + radius * std::sin(destAngle);

        float stepX = 0.0f;
        float stepY = 0.0f;
        float stepZ = 0.0f;
        if (!EncounterHelpers::CanTakeStepTowards(bot, destX, destY, MopSsb::BLACKOUT_STEP + 4.0f, stepX, stepY, stepZ))
            continue;

        _direction = -used;
        return MoveTo(bot->GetMapId(), stepX, stepY, stepZ, false, false, false, false,
                      MovementPriority::MOVEMENT_COMBAT, true, false);
    }

    return false;
}

bool MopSsbFizzyFlightAction::Execute(Event /*event*/)
{
    Creature* boss = bot->FindNearestCreature(MopSsb::NPC_YAN_ZHU, MopSsb::SCAN_RANGE, true);
    if (!boss)
        return false;

    // Flying (aura 114459 -> SetCanFly): climb straight up above the boss's floor + 4.9, hold there until the aura ends.
    if (bot->HasAura(MopSsb::SPELL_FIZZY_FLIGHT))
    {
        float const targetZ = boss->GetPositionZ() + MopSsb::FLIGHT_ABOVE_BOSS;
        if (bot->GetPositionZ() >= targetZ - 1.5f)
            return false;

        Movement::MoveSplineInit init(bot);
        init.MoveTo(bot->GetPositionX(), bot->GetPositionY(), targetZ, false, true);
        init.SetFly();
        init.Launch();
        return true;
    }

    Unit* bubble = MopSsb::FindFizzyBubble(bot);
    if (!bubble)
        return false;

    // MoveTo stops on the SURFACE distance, so the click test is the surface test too (IsWithinDistInMap), not an exact one.
    if (!bot->IsWithinDistInMap(bubble, MopSsb::FIZZY_CLICK_DIST))
        return MoveTo(bubble, 1.5f, MovementPriority::MOVEMENT_FORCED);

    bot->GetMotionMaster()->Clear();
    bot->StopMoving();
    // npc_spellclick_spells 59799 -> 114459; npc_fizzy_bubble::OnSpellClick adds the flight aura and removes the bubble.
    return bubble->HandleSpellClick(bot);
}

bool MopSsbWallJumpAction::Execute(Event /*event*/)
{
    Unit* wall = MopSsb::FindWallToJump(bot);
    if (!wall || IsWaitingForLastMove(MovementPriority::MOVEMENT_COMBAT))
        return false;

    // Hop away from the wall (alternating sideways so the destination is never a duplicate move).
    float const away = wall->GetAngle(bot->GetPositionX(), bot->GetPositionY());
    float const angle = away + 0.6f * _side;
    _side = -_side;
    float const destX = bot->GetPositionX() + MopSsb::WALL_JUMP_STEP * std::cos(angle);
    float const destY = bot->GetPositionY() + MopSsb::WALL_JUMP_STEP * std::sin(angle);
    return JumpTo(bot->GetMapId(), destX, destY, bot->GetPositionZ(), MovementPriority::MOVEMENT_COMBAT);
}

bool MopSsbBubbleAction::Execute(Event /*event*/)
{
    Unit* bubble = MopSsb::FindBubbleShieldTarget(bot);
    if (!bubble || AI_VALUE(Unit*, "current target") == bubble)
        return false;

    return Attack(bubble);
}

bool MopSsbMarkPriorityAction::Execute(Event /*event*/)
{
    Unit* target = MopSsb::GetPriorityMarkTarget(bot);
    return target && EncounterHelpers::MarkTargetWithSkull(bot, target);
}
