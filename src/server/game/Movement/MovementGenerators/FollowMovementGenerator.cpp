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
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "FollowMovementGenerator.h"
#include "Creature.h"
#include "CreatureAI.h"
#include "MoveSpline.h"
#include "MoveSplineInit.h"
#include "Optional.h"
#include "PathGenerator.h"
#include "Pet.h"
#include "Unit.h"
#include "Util.h"

#include <cmath>

static void DoMovementInform(Unit* owner, Unit* target)
{
    if (owner->GetTypeId() != TYPEID_UNIT)
        return;

    if (CreatureAI* AI = owner->ToCreature()->AI())
        AI->MovementInform(FOLLOW_MOTION_TYPE, target->GetGUID().GetCounter());
}

FollowMovementGenerator::FollowMovementGenerator(Unit* target, float range, ChaseAngle angle) : AbstractFollower(ASSERT_NOTNULL(target)), _range(range), _angle(angle), _checkTimer(CHECK_INTERVAL)
{
    Priority = MOTION_PRIORITY_NORMAL;
    Flags = MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING;
    BaseUnitState = UNIT_STATE_FOLLOW;
}
FollowMovementGenerator::~FollowMovementGenerator() = default;

static bool PositionOkay(Unit* owner, Unit* target, float range, Optional<ChaseAngle> angle = {})
{
    float const xx = owner->GetCombatReach() + target->GetCombatReach() + range;
    if (owner->GetExactDistSq(target) > xx * xx)
        return false;

    return !angle || angle->IsAngleOkay(target->GetRelativeAngle(owner));
}

// issue #221 / AC parity (TargetedMovementGenerator.cpp:490-496): a follower has to close the gap at the
// target's CURRENT speed, not the speed type it happens to be standing in.
static float GetTargetSpeedInMotion(Unit* target)
{
    if (!target->movespline->Finalized())
        return target->movespline->Velocity();

    // ADAPTED: AC reads target->m_movementInfo.GetSpeedType(). This tree's MovementInfo has no speed-type
    // accessor (Entities/Object/MovementInfo.h exposes only zspeed/sinAngle/cosAngle/xyspeed), so derive the
    // walk/run split the same way the launch path below does via SetWalk(target->IsWalking()).
    return target->GetSpeed(target->IsWalking() ? MOVE_WALK : MOVE_RUN);
}

// issue #221 / AC parity (TargetedMovementGenerator.cpp:498-523, applied at :747-749): pets and guardians
// sync with the target's speed, and a pet behind its master gets a 1 + dist/10 boost so it can actually keep
// up on a slope instead of being left at the ramp foot. MoP core only ever called SetWalk() here.
static Optional<float> GetFollowVelocity(Unit* owner, Unit* target, float destX, float destY, bool playerPet)
{
    Optional<float> speed = {};
    // ADAPTED: AC uses UNIT_FLAG_POSSESSED; this tree exposes Unit::isPossessed() (Unit.h:1821)
    if (owner->IsInCombat() || owner->IsVehicle() || owner->isPossessed())
        return speed;

    bool isPetLike = owner->IsPet() || owner->IsGuardian() || owner->GetGUID() == target->GetCritterGUID() ||
        owner->GetCharmerOrOwnerGUID() == target->GetGUID();

    if (isPetLike || (owner->IsCreature() && target->IsCreature()))
    {
        speed = GetTargetSpeedInMotion(target);

        if (playerPet)
        {
            // ADAPTED: AC uses owner->GetDistance2d(dest.x, dest.y); this tree's Unit.h does not declare it,
            // so the 2D distance is computed directly - same value, no API risk.
            float const ddx = owner->GetPositionX() - destX;
            float const ddy = owner->GetPositionY() - destY;
            float distance = std::sqrt(ddx * ddx + ddy * ddy) - target->GetObjectSize() - (*speed / 2.f);
            if (distance > 0.f)
            {
                float multiplier = 1.f + (distance / 10.f);
                *speed *= multiplier;
            }
        }
    }

    return speed;
}

bool FollowMovementGenerator::Initialize(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_INITIALIZATION_PENDING | MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    AddFlag(MOVEMENTGENERATOR_FLAG_INITIALIZED | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

    _path = nullptr;
    _lastTargetPosition.reset();
    return true;
}

bool FollowMovementGenerator::Reset(Unit* owner)
{
    RemoveFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);

    return Initialize(owner);
}

bool FollowMovementGenerator::Update(Unit* owner, uint32 diff)
{
    // owner might be dead or gone
    if (!owner || !owner->IsAlive())
        return false;

    // our target might have gone away
    Unit* const target = GetTarget();
    if (!target || !target->IsInWorld())
        return false;

    // issue #221 (AC parity, TargetedMovementGenerator.cpp:637-648): a pet following its own master is allowed
    // to cheat the mmap path, exactly like its master. Without this the pet stops on the first PATHFIND_NOPATH
    // (stairs, towers, doorways) and is left behind instead of walking up to the follow point.
    // MoP stores the summoner in UNIT_FIELD_CREATED_BY (Player::SummonPet / Pet::LoadPetFromDB call
    // SetCreatorGUID), so accept that field as well as the legacy UNIT_FIELD_SUMMONED_BY one.
    bool followingMaster = false;
    if (Pet* oPet = owner->ToPet())
        followingMaster = target->GetGUID() == oPet->GetCreatorGUID() || target->GetGUID() == oPet->GetOwnerGUID();

    // issue #221 re-anchor: a pet wedged in terrain the mmap mesh does not cover can never path out, so once it
    // falls far enough behind its master we put it back at the master's feet rather than re-pathing every tick.
    // Threshold is 10 yd of 3D separation: far enough that a pet legitimately following at follow range never
    // trips it, close enough to catch a pet stranded at the foot of a ramp. Deliberately not gated on
    // UNIT_STATE_NOT_MOVE / IsMovementPreventedByCasting - a pet that cannot path out never clears those.
    if (followingMaster && !owner->GetTransport() && !target->GetTransport() && owner->GetDistance(target) > 10.0f)
    {
        float teleX, teleY, teleZ;
        target->GetClosePoint(teleX, teleY, teleZ, owner->GetCombatReach());
        // Master's feet, verbatim. GetMapHeight() is a DOWNWARD search seeded from the master's Z, so on a slope
        // it can resolve to the terrain UNDER the ramp; the pet then lands under the master again and the branch
        // re-fires forever. The master's own Z is always a legal standing height for the pet.
        teleZ = target->GetPositionZ();
        // Convergence guard (#221): measure the destination against the MASTER, not against the pet. The pet is
        // >10 yd away by the branch condition, so a pet-relative test can never pass - it is unsatisfiable by
        // construction, which is what made the previous revision dead code. GetClosePoint lands within a combat
        // reach of the master, so this is satisfied whenever the master itself has a sane close point.
        //
        // #252 (pets/minions vanish while the owner flies - taxi and flying mount; ground mount is fine; the
        // flying-mount half is filed separately as #289): the
        // test is 2D, not 3D, and that IS the flight-state predicate. GetClosePoint's Z comes from
        // UpdateAllowedPositionZ, i.e. the ground under the master, so for a master in the air the 3D distance
        // measured here is essentially the master's altitude - 50+ yd on any taxi or flight. The 3D form could
        // therefore never pass for an airborne master, the whole re-anchor block was dead for the entire flight,
        // and the pet was left at the departure point. Once the master's 3D separation passes the map visibility
        // range, Pet::Update (Pet.cpp:559) and Player::Update (Player.cpp:1517) both remove the pet for good -
        // IsWithinDistInMap() defaults to is3D=true (Object.h:495) and PET_REMOVE_DISMISS|PET_REMOVE_FLAG_RESET_CURRENT
        // drops the character_pet row, so it does not come back on landing. A master on a ground mount keeps its
        // Z on the mesh, GetClosePoint resolves within a combat reach in 3D too, and the guard passes - which is
        // exactly the reported ground-vs-air split. The destination is a spot on the ground beside the master, so
        // 2D is the meaningful measure of it.
        if (target->GetDistance2d(teleX, teleY) < 6.0f)
        {
            // Only re-anchor a pet that is not already at the spot. The branch trigger is 3D, so an airborne
            // master keeps it above the threshold for the whole flight; without this the block would
            // NearTeleportTo the pet on every tick of the flight instead of once.
            if (owner->GetDistance2d(teleX, teleY) > 2.0f)
            {
                owner->StopMoving();
                owner->NearTeleportTo(teleX, teleY, teleZ, target->GetOrientation());
                _lastTargetPosition.reset();
                // ONLY on a real re-anchor. The previous revision returned here unconditionally, so on any tick
                // where the pet was over the threshold the whole of the normal follow logic below was skipped -
                // the pet was frozen by this guard, not by the terrain. Falling through instead lets normal
                // following run.
                return true;
            }
        }
        // else: fall through to normal following below.
    }

    if (owner->HasUnitState(UNIT_STATE_NOT_MOVE) || owner->IsMovementPreventedByCasting())
    {
        _path = nullptr;
        owner->StopMoving();
        _lastTargetPosition.reset();
        return true;
    }

    _checkTimer.Update(diff);
    if (_checkTimer.Passed())
    {
        _checkTimer.Reset(CHECK_INTERVAL);
        if (HasFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED) && PositionOkay(owner, target, _range, _angle))
        {
            RemoveFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
            _path = nullptr;
            owner->StopMoving();
            _lastTargetPosition.reset();
            DoMovementInform(owner, target);
            return true;
        }
    }

    if (owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE) && owner->movespline->Finalized())
    {
        RemoveFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
        _path = nullptr;
        owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
        DoMovementInform(owner, target);
    }

    bool const targetPositionChanged = !_lastTargetPosition ||
        _lastTargetPosition->GetExactDistSq(target->GetPosition()) > 0.0f;
    if (targetPositionChanged)
        _lastTargetPosition = target->GetPosition();

    if (targetPositionChanged ||
        (!owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE) && !PositionOkay(owner, target, _range + FOLLOW_RANGE_TOLERANCE)))
    {
        if (owner->HasUnitState(UNIT_STATE_FOLLOW_MOVE) || !PositionOkay(owner, target, _range + FOLLOW_RANGE_TOLERANCE))
        {
            if (!_path)
                _path = std::make_unique<PathGenerator>(owner);

            float x, y, z;

            // select angle
            float tAngle;
            float const curAngle = target->GetRelativeAngle(owner);
            if (_angle.IsAngleOkay(curAngle))
                tAngle = curAngle;
            else
            {
                float const diffUpper = Position::NormalizeOrientation(curAngle - _angle.UpperBound());
                float const diffLower = Position::NormalizeOrientation(_angle.LowerBound() - curAngle);
                if (diffUpper < diffLower)
                    tAngle = _angle.UpperBound();
                else
                    tAngle = _angle.LowerBound();
            }

            if (followingMaster)
            {
                // Use the master's collision height; GetNearPoint otherwise clamps the pet's destination to the
                // ground beneath it.
                Position targetPosition = target->GetPosition();
                target->MovePositionToFirstCollision(targetPosition, owner->GetCombatReach() + _range,
                    target->ToAbsoluteAngle(tAngle) - target->GetOrientation());
                targetPosition.GetPosition(x, y, z);
                if (owner->IsHovering())
                    owner->UpdateAllowedPositionZ(x, y, z);
            }
            else
                target->GetNearPoint(owner, x, y, z, _range, target->ToAbsoluteAngle(tAngle));

            bool success = _path->CalculatePath(x, y, z, followingMaster);
            if (!success || ((_path->GetPathType() & PATHFIND_NOPATH) && !followingMaster))
            {
                owner->StopMoving();

                return true;
            }

            owner->AddUnitState(UNIT_STATE_FOLLOW_MOVE);
            AddFlag(MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);

            Movement::MoveSplineInit init(owner);
            init.MovebyPath(_path->GetPath());
            init.SetWalk(target->IsWalking());
            if (followingMaster)
                if (Optional<float> velocity = GetFollowVelocity(owner, target, x, y, followingMaster))
                    init.SetVelocity(*velocity);
            init.SetFacing(target->GetOrientation());
            init.Launch();
        }
    }
    return true;
}

void FollowMovementGenerator::Deactivate(Unit* owner)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_DEACTIVATED);
    RemoveFlag(MOVEMENTGENERATOR_FLAG_TRANSITORY | MOVEMENTGENERATOR_FLAG_INFORM_ENABLED);
    owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
}

void FollowMovementGenerator::Finalize(Unit* owner, bool active, bool/* movementInform*/)
{
    AddFlag(MOVEMENTGENERATOR_FLAG_FINALIZED);
    if (active)
        owner->ClearUnitState(UNIT_STATE_FOLLOW_MOVE);
}
