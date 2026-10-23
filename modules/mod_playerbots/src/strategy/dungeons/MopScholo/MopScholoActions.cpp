/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.

#include "MopScholoActions.h"
#include "Creature.h"
#include "EncounterHelpers.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"

using namespace MopScholomance;

bool MopScholoLeaveHazardAction::Execute(Event /*event*/)
{
    float clearDistance = 0.0f;
    Creature* carrier = FindNearestHazard(bot, _hazard, clearDistance);
    if (!carrier)
        return false;

    if (FleePosition(carrier->GetPosition(), clearDistance, 500U))
        return true;

    return MoveAway(carrier, clearDistance - bot->GetExactDist2d(carrier) + 1.0f);
}

bool MopScholoSpreadAction::Execute(Event /*event*/)
{
    Player* member = FindNearestGroupMember(bot, _spread);
    if (!member)
        return false;

    return MoveAway(member, _spread - bot->GetExactDist2d(member) + 1.0f);
}

bool MopScholoFixatedAction::Execute(Event /*event*/)
{
    Creature* soul = nullptr;
    if (!FixatedBySoul(bot, soul))
        return false;

    // Lilian's Soul is slow (Unleashed Anguish): keep running from it, tracing the room edge, while everyone else
    // burns it down.
    if (FleePosition(soul->GetPosition(), FIXATE_SAFE_RANGE, 500U))
        return true;

    return MoveAway(soul, FIXATE_SAFE_RANGE - bot->GetExactDist2d(soul) + 1.0f);
}

bool MopScholoInterruptAction::Execute(Event /*event*/)
{
    Unit* target = FindInterruptTarget(bot);
    if (!target)
        return false;

    return !CastInterrupt(botAI, target).empty();
}

bool MopScholoFocusAction::Execute(Event /*event*/)
{
    Unit* target = FindFocusTarget(bot);
    if (!target)
        return false;

    if (EncounterHelpers::IsMechanicTrackerBot(bot, MAP_SCHOLOMANCE))
        EncounterHelpers::MarkTargetWithSkull(bot, target);

    if (AI_VALUE(Unit*, "current target") == target)
        return false;

    // #659 w52: AttackAction::Attack refuses a target out of line of sight before it sets "current target", and the
    // focus multiplier vetoes the assist actions that would otherwise bring the bot closer, so nothing moved: after
    // Chillheart's fake death the group stood on the landing (z 132) with the Phylactery below on the main floor
    // (200.6,21.2,118.6) out of sight for 300 s (focus target 4318 FAILED, "target=-" in every probe). Walk to it
    // first (AC MovementAction::ReachCombatTo, the path-based approach the reach actions use); ranged stop at 10 yd.
    if (!bot->IsWithinLOSInMap(target))
        return ReachCombatTo(target, PlayerBotSpec::IsRanged(bot) ? 10.0f : 0.0f);

    return Attack(target);
}

bool MopScholoHarshLessonAction::Execute(Event /*event*/)
{
    Creature* subject = FindExplosivePainSubject(bot);
    if (!subject)
        return false;

    return CastFriendlyDispel(botAI, subject);
}

bool MopScholoLessonExitAction::Execute(Event /*event*/)
{
    Creature* gandling = nullptr;
    if (!LessonExitNeeded(bot, gandling))
        return false;

    return MoveTo(gandling, LESSON_RETURN_RANGE - 15.0f, MovementPriority::MOVEMENT_COMBAT);
}

bool MopScholoBoneArmorAction::Execute(Event /*event*/)
{
    if (!NeedsBoneArmor(bot))
        return false;

    Creature* pile = FindBonePile(bot);
    if (!pile)
        return false;

    // Spellclick range is the interaction distance (VehicleActions.cpp:93-106 does the same for vehicles); the pile's
    // script (npc_bone_pile::OnSpellClick, boss_rattlegore.cpp:280-283) puts Bone Armor on the clicker.
    if (!bot->IsWithinDistInMap(pile, INTERACTION_DISTANCE - 1.0f))   // surface distance, the measure MoveTo uses (#818 lane)
        return MoveTo(pile, 2.0f, MovementPriority::MOVEMENT_COMBAT);

    pile->HandleSpellClick(bot);
    return bot->HasAura(SPELL_BONE_ARMOR);
}

// AC Ai/Dungeon/UK/UKActions.cpp:48-60 (IngvarDodgeSmashAction).
bool MopScholoDodgeRapidityAction::isUseful() { return !AI_VALUE2(bool, "behind", "current target"); }
bool MopScholoDodgeRapidityAction::Execute(Event /*event*/)
{
    Creature* boss = FindEngagedBoss(bot, NPC_JANDICE_BAROV);
    if (!boss)
        return false;

    float const distance = bot->GetExactDist2d(boss->GetPosition());
    // AC: 2 yd past her midpoint, as a human would step.
    float const distanceExtra = 2.0f;
    return Move(bot->GetAngle(boss), distance + distanceExtra);
}

// AC Ai/Dungeon/UK/UKActions.cpp:62-69 (IngvarSmashReturnAction).
bool MopScholoRapidityReturnAction::isUseful() { return AI_VALUE2(bool, "behind", "current target"); }
bool MopScholoRapidityReturnAction::Execute(Event /*event*/)
{
    Creature* boss = FindEngagedBoss(bot, NPC_JANDICE_BAROV);
    if (!boss)
        return false;

    float const distance = bot->GetExactDist2d(boss->GetPosition());
    return Move(bot->GetAngle(boss), distance + bot->GetMeleeReach());
}

// #659 Ice Wave (sources in MopScholoShared.h): walk +Y (west, away from the wall) to the safe line; the tank drags
// Chillheart along, she chases her victim. Same "move to a computed spot" shape as MopScholoBoneArmorAction above.
bool MopScholoOutrunIceWallAction::Execute(Event /*event*/)
{
    float shortfall = 0.0f;
    float const safeY = IceWallSafeY(bot, shortfall);
    if (safeY == 0.0f || shortfall <= 0.0f)
        return false;

    // #659: +Y only while that stays on the main floor; past y ~38 the north wall stops a straight walk, so the spot is
    // the landing / corridor and the path climbs the side-lobe stairs (MopScholoShared.h geometry).
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    IceWallOutrunSpot(bot, safeY, x, y, z);
    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_COMBAT);
}

// #659: the tank walks its boss to the fight spot (TankSpot). Same "move to a fixed spot" shape as AC's tank position
// actions (AC Ai/Raid/ICC/Action/ICCActions_LM.cpp:96 IccLmTankPositionAction).
bool MopScholoTankSpotAction::Execute(Event /*event*/)
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
    if (!TankSpot(bot, x, y, z))
        return false;

    return MoveTo(bot->GetMapId(), x, y, z, false, false, true, false, MovementPriority::MOVEMENT_COMBAT);
}
