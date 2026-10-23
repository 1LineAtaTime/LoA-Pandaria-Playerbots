/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "PointMovementGenerator.h"
#include "Errors.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "World.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "Player.h"
#include "CreatureGroups.h"

//----- Point Movement Generator
template<class T>
bool PointMovementGenerator<T>::DoInitialize(T* unit)
{
    if (!unit->IsStopped())
        unit->StopMoving();

    unit->AddUnitState(UNIT_STATE_ROAMING|UNIT_STATE_ROAMING_MOVE);

    if (id == EVENT_CHARGE_PREPATH)
        return true;

    Movement::MoveSplineInit init(unit);
    init.MoveTo(i_x, i_y, i_z, m_generatePath);
    if (speed > 0.0f)
        init.SetVelocity(speed);
    init.Launch();

    // Call for creature group update
    if (Creature* creature = unit->ToCreature())
        if (creature->GetFormation() && creature->GetFormation()->GetLeader() == creature)
            creature->GetFormation()->LeaderStartedMoving();

    return true;
}

template<class T>
bool PointMovementGenerator<T>::DoUpdate(T* unit, uint32 /*diff*/)
{
    if (!unit)
        return false;

    if (unit->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED))
    {
        unit->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    unit->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    if (id != EVENT_CHARGE_PREPATH && i_recalculateSpeed && !unit->movespline->Finalized())
    {
        i_recalculateSpeed = false;
        Movement::MoveSplineInit init(unit);
        init.MoveTo(i_x, i_y, i_z, m_generatePath);
        if (speed > 0.0f) // Default value for point motion type is 0.0, if 0.0 spline will use GetSpeed on unit
            init.SetVelocity(speed);
        init.Launch();

        // Call for creature group update
        if (Creature* creature = unit->ToCreature())
            if (creature->GetFormation() && creature->GetFormation()->GetLeader() == creature)
                creature->GetFormation()->LeaderStartedMoving();
    }

    bool done = unit->movespline->Finalized();
    if (unit->GetTypeId() == TYPEID_UNIT && done)
        MovementInform(unit);

    return !done;
}

// DoInitialize() above sets UNIT_STATE_ROAMING_MOVE unconditionally, so something has to clear it
// again when the move ends. This used to clear it only for a charge, which meant an ordinary
// MovePoint() - by far the common case - left the state set for the rest of the unit's life: the
// generator was popped, GetCurrentMovementGeneratorType() went back to IDLE_MOTION_TYPE, and
// Unit::IsStopped() (which is just !HasUnitState(UNIT_STATE_MOVING)) still answered "moving"
// forever after. The two parameters were even left unnamed, which is how the handling went missing
// when this file was backported onto the newer MotionMaster.
//
// It was found from playerbots: battleground bots reached their objective, stopped, and then never
// issued another move, because every caller that asks "is this unit busy moving?" before ordering a
// new one was told yes. Measured over one Warsong Gulch match, 41159 of 41159 samples of a stopped
// bot read IDLE generator + UNIT_STATE_ROAMING_MOVE set.
//
// `active` is false when the generator is being finalized without having been the running one, in
// which case it never set the state and must not clear someone else's - this is the same condition
// upstream TrinityCore uses.
template<class T>
void PointMovementGenerator<T>::DoFinalize(T* unit, bool active, bool /*movementInform*/)
{
    if (unit->HasUnitState(UNIT_STATE_CHARGING))
        unit->ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);

    if (active)
        unit->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

/// Deactivating is the other way a running point move stops being the active generator (it is
/// pushed down the stack rather than popped), and it has to drop the movement state for the same
/// reason DoFinalize does.
template<class T>
void PointMovementGenerator<T>::DoDeactivate(T* unit)
{
    unit->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
}

template<class T>
bool PointMovementGenerator<T>::DoReset(T* unit)
{
    return true;
}

template<class T>
void PointMovementGenerator<T>::MovementInform(T* /*unit*/) { }

template <> void PointMovementGenerator<Creature>::MovementInform(Creature* unit)
{
    if (unit->AI())
        unit->AI()->MovementInform(POINT_MOTION_TYPE, id);
}

template bool PointMovementGenerator<Player>::DoInitialize(Player*);
template bool PointMovementGenerator<Creature>::DoInitialize(Creature*);
template void PointMovementGenerator<Player>::DoFinalize(Player*, bool, bool);
template void PointMovementGenerator<Creature>::DoFinalize(Creature*, bool, bool);
template void PointMovementGenerator<Player>::DoDeactivate(Player*);
template void PointMovementGenerator<Creature>::DoDeactivate(Creature*);
template bool PointMovementGenerator<Player>::DoReset(Player*);
template bool PointMovementGenerator<Creature>::DoReset(Creature*);
template bool PointMovementGenerator<Player>::DoUpdate(Player*, uint32);
template bool PointMovementGenerator<Creature>::DoUpdate(Creature*, uint32);

void AssistanceMovementGenerator::Finalize(Unit* unit, bool, bool)
{
    unit->ToCreature()->SetNoCallAssistance(false);
    unit->ToCreature()->CallAssistance();
    if (unit->IsAlive())
        unit->GetMotionMaster()->MoveSeekAssistanceDistract(sWorld->getIntConfig(CONFIG_CREATURE_FAMILY_ASSISTANCE_DELAY));
}

bool EffectMovementGenerator::Update(Unit* unit, uint32)
{
    bool done = unit->movespline->Finalized();

    if (unit->GetTypeId() == TYPEID_UNIT && done)
        if (unit->ToCreature()->AI())
            unit->ToCreature()->AI()->MovementInform(EFFECT_MOTION_TYPE, m_Id);

    return !done;
}

bool EffectMovementGenerator::Initialize(Unit* unit)
{
    if (Player* player = unit->ToPlayer())
        player->SetFallInformation(0, player->GetPositionZ());

    return true;
}

void EffectMovementGenerator::Finalize(Unit* unit, bool, bool)
{
    if (unit->GetTypeId() != TYPEID_UNIT)
        return;

    // Need restore previous movement since we have no proper states system
    if (unit->IsAlive() && !unit->HasUnitState(UNIT_STATE_CONFUSED | UNIT_STATE_FLEEING))
    {
        if (Unit* victim = unit->GetVictim())
            unit->GetMotionMaster()->MoveChase(victim);
        else
            unit->GetMotionMaster()->Initialize();
    }
}
