/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 */

// MoP-only dungeon strategy "mop-scholo" (Scholomance, epic #373, issue #659). See MopScholoShared.h for sources.
// The movement veto has the shape of AC Ai/Dungeon/HoS/HoSMultipliers.cpp:14-45.

#include "MopScholoMultipliers.h"
#include "Action.h"
#include "AttackersValue.h"
#include "ChooseTargetActions.h"
#include "Creature.h"
#include "MopScholoActions.h"
#include "MopScholoShared.h"
#include "MovementActions.h"
#include "PlayerbotSpec.h"
#include "Playerbots.h"
#include "ReachTargetActions.h"

using namespace MopScholomance;

float MopScholoFocusMultiplier::GetValue(Action* action)
{
    bool const picker = dynamic_cast<DpsAssistAction*>(action) || dynamic_cast<DpsAoeAction*>(action) ||
                        dynamic_cast<TankAssistAction*>(action);
    bool const drop = !picker && dynamic_cast<DropTargetAction*>(action) != nullptr;
    if (!picker && !drop)
        return 1.0f;

    if (!bot->IsInCombat())
        return 1.0f;

    Unit* focus = FindFocusTarget(bot);
    if (!focus)
        return 1.0f;

    if (!drop)
        return 0.0f;

    // Dropping is only allowed for a target that is not the focus target (or is no longer a valid target).
    Unit* current = AI_VALUE(Unit*, "current target");
    if (!current || current != focus)
        return 1.0f;

    if (!current->IsAlive() || !AttackersValue::IsValidTarget(current, bot))
        return 1.0f;

    return 0.0f;
}

float MopScholoFixateMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopScholoFixatedAction*>(action))
        return 1.0f;

    Creature* soul = nullptr;
    return FixatedBySoul(bot, soul) ? 0.0f : 1.0f;
}

float MopScholoRapidityMultiplier::GetValue(Action* action)
{
    if (!dynamic_cast<MovementAction*>(action) || dynamic_cast<MopScholoDodgeRapidityAction*>(action))
        return 1.0f;

    if (!PlayerBotSpec::IsTank(bot))
        return 1.0f;

    Creature* boss = nullptr;
    return JandiceRapidityActive(bot, boss) ? 0.0f : 1.0f;
}

float MopScholoIceWallMultiplier::GetValue(Action* action)
{
    // #659: gap closers (warrior Charge, druid Wild Charge = CastReachTargetSpellAction; Heroic Leap) land the bot on its
    // target. w41: the tank charged Chillheart on her spawn (y 6.1, inside the 9.7 yd engage reach) and died at +13 s.
    bool const gapCloser = dynamic_cast<CastReachTargetSpellAction*>(action) || action->getName() == "heroic leap";
    // The outrun and the tank spot both lead away from the wall (the spot is only kept while it is ahead of the safe
    // line), so they are never vetoed - in particular not while the bot is fighting an add or the boss inside the wall.
    if ((!gapCloser && !dynamic_cast<MovementAction*>(action)) || dynamic_cast<MopScholoOutrunIceWallAction*>(action) ||
        dynamic_cast<MopScholoTankSpotAction*>(action))
        return 1.0f;

    float shortfall = 0.0f;
    float const safeY = IceWallSafeY(bot, shortfall);
    if (safeY == 0.0f)
        return 1.0f;

    if (shortfall > 0.0f)
        return 0.0f;

    Unit* target = AI_VALUE(Unit*, "current target");
    return (target && target->GetPositionY() < safeY - ICE_WALL_MARGIN) ? 0.0f : 1.0f;
}
