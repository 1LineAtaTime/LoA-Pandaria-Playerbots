/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "AttackActions.h"

#include "CreatureAI.h"
#include "Event.h"
#include "LastMovementValue.h"
#include "LootObjectStack.h"
#include "Map.h"           // INVALID_HEIGHT (#868 duel fall)
#include "MotionMaster.h"  // MoveFall (#868 duel fall)
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "PlayerbotSpec.h"
#include "ServerFacade.h"
#include "SharedDefines.h"
#include "Unit.h"
#include "WaitForAttackStrategy.h"

bool AttackAction::Execute(Event event)
{
    Unit* target = GetTarget();
    if (!target)
        return false;

    if (!target->IsInWorld())
    {
        return false;
    }
    return Attack(target);
}

bool AttackMyTargetAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    ObjectGuid guid = master->GetTarget();
    if (!guid)
    {
        if (verbose)
            botAI->TellError("You have no target");

        return false;
    }

    botAI->GetAiObjectContext()->GetValue<GuidVector>("prioritized targets")->Set({ guid });
    bool result = Attack(botAI->GetUnit(guid));
    if (result)
        context->GetValue<ObjectGuid>("pull target")->Set(guid);

    return result;
}

bool AttackAction::Attack(Unit* target, bool with_pet /*true*/)
{
    if (bot->GetMotionMaster()->GetCurrentMovementGeneratorType() == FLIGHT_MOTION_TYPE ||
        bot->HasUnitState(UNIT_STATE_IN_FLIGHT))
    {
        if (verbose)
            botAI->TellError("I cannot attack in flight");

        return false;
    }

    if (!target)
    {
        if (verbose)
            botAI->TellError("I have no target");

        return false;
    }

    if (!target->IsInWorld())
    {
        return false;
    }

    if (!bot->IsValidAttackTarget(target))
    {
        if (verbose)
            botAI->TellError("I cannot attack an invalid target");

        return false;
    }

    std::ostringstream msg;
    msg << target->GetName();

    if (bot->IsFriendlyTo(target))
    {
        msg << " is friendly to me";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    // issue #61: a neutral, non-attackable creature (e.g. a scripted city NPC) is neither
    // friendly nor hostile, so it slipped past the check above. Attacking it switched the bot to
    // the combat engine even though combat never started, which froze it permanently because the
    // non-combat engine (roaming/questing/travel) then never ran again.
    if (!bot->IsValidAttackTarget(target))
    {
        msg << " is not a valid attack target";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (!bot->IsWithinLOSInMap(target))
    {
        msg << " is not in my sight";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    if (target->isDead())
    {
        msg << " is dead";
        if (verbose)
            botAI->TellError(msg.str());

        return false;
    }

    // AC parity (AC AttackAction.cpp:87-100): the PvP-prohibited gate is waived for the bot's own duel
    // opponent, and it tests BOTH the bot's and the target's zone/area. #404 (attack-side half of #397).
    if ((target->IsPlayer() || target->IsPet()) &&
        (!bot->duel || bot->duel->opponent != target) &&
        (sPlayerbotAIConfig->IsPvpProhibited(bot->GetZoneId(), bot->GetAreaId()) ||
            sPlayerbotAIConfig->IsPvpProhibited(target->GetZoneId(), target->GetAreaId())))
    {
        if (verbose)
            botAI->TellError("I cannot attack others in PvP prohibited zones");

        return false;
    }

    // if (bot->IsMounted() && bot->IsWithinLOSInMap(target))
    // {
    //     WorldPacket emptyPacket;
    //     bot->GetSession()->HandleCancelMountAuraOpcode(emptyPacket);
    // }

    // Infantry attacks are not allowed from vehicles drivers (AC AttackAction::Attack).
    // Check is needed to stop some auto-attack situations.
    if (botAI->IsInVehicle() && !botAI->IsInVehicle(false, false, true))
        return false;

    ObjectGuid guid = target->GetGUID();
    bot->SetSelection(target->GetGUID());

    Unit* oldTarget = context->GetValue<Unit*>("current target")->Get();
    bool melee = bot->IsWithinMeleeRange(target) || PlayerBotSpec::IsMelee(bot);

    if (oldTarget == target && botAI->GetState() == BOT_STATE_COMBAT && bot->GetVictim() == target && (bot->HasUnitState(UNIT_STATE_MELEE_ATTACKING) == melee))
        return false;

    context->GetValue<Unit*>("old target")->Set(oldTarget);

    context->GetValue<Unit*>("current target")->Set(target);
    // #151 (AC AttackAction.cpp:178): re-enabled now that "available loot" is a real
    // LootObjectStack* (loot-stack-port.md section 2 step 4.6).
    context->GetValue<LootObjectStack*>("available loot")->Get()->Add(guid);

    LastMovement& lastMovement = AI_VALUE(LastMovement&, "last movement");
    // AC: never cancel a controlled movement (charge, jump, knockback - MOTION_SLOT_CONTROLLED).
    bool moveControlled = bot->GetMotionMaster()->GetMotionSlotType(MOTION_SLOT_CONTROLLED) != NULL_MOTION_TYPE;
    if (lastMovement.priority < MovementPriority::MOVEMENT_COMBAT && bot->isMoving() && !moveControlled)
    {
        AI_VALUE(LastMovement&, "last movement").clear();
        bot->GetMotionMaster()->Clear(false);
        bot->StopMoving();
    }


    if (IsMovingAllowed() && !bot->HasInArc(CAST_ANGLE_IN_FRONT, target))
    {
        sServerFacade->SetFacingTo(bot, target);
    }
    botAI->ChangeEngine(BOT_STATE_COMBAT);

    // AC parity (#141 item 5, AC AttackAction.cpp:194): let a real player master hold the
    // bot's opening attack via the opt-in "wait for attack" strategy. No-op for every bot that
    // does not have it active (bot->Attack() still runs unconditionally for them).
    if (WaitForAttackStrategy::ShouldWait(botAI))
    {
        TC_LOG_DEBUG("playerbots", "[WAITFORATTACK] %s: holding opening attack on %s",
                     bot->GetName().c_str(), target->GetName().c_str());
    }
    else
        bot->Attack(target, melee);
    return true;
}

bool AttackDuelOpponentAction::isUseful()
{
    return AI_VALUE(Unit*, "duel target");
}

// #868 (beyond AC, started duels only). AC's body is just
// `return Attack(duel target)` (AC AttackAction.cpp:222); it stays the first step. When it fails in a started duel the
// bot used to fail the same way every tick: wave 78 t613 Minarise (warlock) feared Asteri, both dropped the target
// ('invalid target': HasFearAura / IsValidTarget LOS), then stood 18-33 yd apart for 66 s, 'A:attack duel opponent -
// FAILED' x79 on 'is not in my sight' (AttackAction::Attack LOS check). Kalani, standing 6 yd above the ground at a
// saved mid-air position, never came down to the stealthed rogue Catheri, who could not reach it (68 s idle).
bool AttackDuelOpponentAction::Execute(Event event)
{
    Unit* opponent = AI_VALUE(Unit*, "duel target");
    if (Attack(opponent))
        return true;

    if (!opponent || !bot->duel || bot->duel->opponent != opponent || bot->duel->startTime == 0 ||
        !opponent->IsAlive() || !opponent->IsInWorld() || opponent->GetMapId() != bot->GetMapId())
        return false;

    // A bot left standing in mid-air (no client gravity) drops to the ground when its opponent stands below it. Same
    // test and fall as the Twin Peaks #747 check (BattleGroundTactics.cpp), limited to the duel case where the
    // opponent itself shows where the floor is.
    if (!bot->IsMounted() && !bot->HasFlyAura() && !bot->HasIncreaseMountedFlightSpeedAura() && !bot->isSwimming() &&
        !bot->IsFalling() && !bot->isMoving() && !bot->GetTransport() && !bot->GetVehicle() &&
        !bot->HasUnitState(UNIT_STATE_LOST_CONTROL))
    {
        float const surface =
            bot->GetMapWaterOrGroundLevel(bot->GetPositionX(), bot->GetPositionY(), bot->GetPositionZ());
        if (surface > INVALID_HEIGHT && bot->GetPositionZ() - surface > 3.0f &&
            opponent->GetPositionZ() < bot->GetPositionZ() - 3.0f)
        {
            TC_LOG_DEBUG("playerbots", "[DUELCHASE] %s fall %.1f yd to the ground (opponent %s below)",
                         bot->GetName().c_str(), bot->GetPositionZ() - surface, opponent->GetName().c_str());
            bot->GetMotionMaster()->MoveFall();
            return true;
        }
    }

    // A stealthed opponent: wait for its opener, as a player would (it cannot be seen, so it is not chased).
    if (!bot->CanSeeOrDetect(opponent))
        return false;

    // Out of line of sight: walk toward the opponent until Attack() succeeds on a later tick (it then stops this
    // MOVEMENT_NORMAL walk, AttackAction::Attack 'last movement' check).
    if (!bot->IsWithinLOSInMap(opponent))
    {
        TC_LOG_DEBUG("playerbots", "[DUELCHASE] %s close in on %s (no LOS, %.1f yd)", bot->GetName().c_str(),
                     opponent->GetName().c_str(), bot->GetExactDist(opponent));
        return MoveTo(opponent, 0.0f, MovementPriority::MOVEMENT_NORMAL);
    }

    return false;
}
