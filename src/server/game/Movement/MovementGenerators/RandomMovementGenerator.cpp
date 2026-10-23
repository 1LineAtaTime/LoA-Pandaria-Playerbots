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

#include "Creature.h"
#include "MapManager.h"
#include "RandomMovementGenerator.h"
#include "ObjectAccessor.h"
#include "Map.h"
#include "Util.h"
#include "CreatureGroups.h"
#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "SmartAI.h"
#include "ScriptedCreature.h"
#include "World.h"

#define RUNNING_CHANCE_RANDOMMV 20                                  //will be "1 / RUNNING_CHANCE_RANDOMMV"

#ifdef MAP_BASED_RAND_GEN
#define rand_norm() creature.rand_norm()
#endif

namespace
{
    // MoP-only walk/run selection, unchanged from the pre-port generator (walk_mode column + SmartAI run flag).
    // AC picks it from creature->GetMovementTemplate().GetRandom() (AC RandomMovementGenerator.cpp:62-73 and 195-206);
    // MoP never consumed CreatureMovementData::Random here, so the existing behaviour is kept.
    void SetRandomMoveWalk(Creature* creature, Movement::MoveSplineInit& init)
    {
        bool run = false;

        if (creature->GetAIName() == "SmartAI")
            run = CAST_AI(SmartAI, creature->AI()) && CAST_AI(SmartAI, creature->AI())->IsCreatureRun();

        if (creature->GetWalkMode() == 1)
            init.SetWalk(run);
        else if (creature->GetWalkMode() == 2)
            init.SetWalk((irand(0, 5) > 0) ? !run : run);
        else
            init.SetWalk(!run);
    }
}

// Port of AC RandomMovementGenerator.cpp:36-245 (issue #72)
template<>
void RandomMovementGenerator<Creature>::_setRandomLocation(Creature* creature)
{
    if (!creature)
        return;

    // AC RandomMovementGenerator.cpp:41-42 skips while creature->_moveState != MAP_OBJECT_CELL_MOVE_NONE.
    // MapObject::_moveState is private in MoP (friends: Map, ObjectGridLoader) and the access needs a friend line in
    // Object.h, which is included everywhere; omitted. The retry on the next tick is the same either way.

    if (_validPointsVector[_currentPoint].empty())
    {
        if (_currentPoint == RANDOM_POINTS_NUMBER) // cant go anywhere from initial position, lets stay
            return;
        // go back to initial position and will never return to this point
        _currentPoint = RANDOM_POINTS_NUMBER;
        _currDestPosition.Relocate(_initialPosition);
        creature->AddUnitState(UNIT_STATE_ROAMING_MOVE);
        Movement::MoveSplineInit init(creature);
        init.MoveTo(_currDestPosition.GetPositionX(), _currDestPosition.GetPositionY(), _currDestPosition.GetPositionZ());

        SetRandomMoveWalk(creature, init);
        init.Launch();
        // AC also requires CanLeaderStartMoving() (AC RandomMovementGenerator.cpp:76); MoP's CreatureGroup has no such method
        if (creature->GetFormation() && creature->GetFormation()->GetLeader() == creature)
            creature->GetFormation()->LeaderStartedMoving();
        return;
    }

    uint8 random = urand(0, _validPointsVector[_currentPoint].size() - 1);
    std::vector<uint8>::iterator randomIter = _validPointsVector[_currentPoint].begin() + random;
    uint8 newPoint = *randomIter;
    uint16 pathIdx = uint16(_currentPoint * RANDOM_POINTS_NUMBER + newPoint);

    // cant go anywhere from new point, so dont go there to not be stuck
    if (_validPointsVector[newPoint].empty())
    {
        _validPointsVector[_currentPoint].erase(randomIter);
        return;
    }

    Movement::PointsArray& finalPath = _preComputedPaths[pathIdx];
    if (finalPath.empty())
    {
        Map* map = creature->GetMap();
        float x = _destinationPoints[newPoint].x, y = _destinationPoints[newPoint].y, z = _destinationPoints[newPoint].z;
        // invalid coordinates
        if (!Trinity::IsValidMapCoord(x, y))
        {
            _validPointsVector[_currentPoint].erase(randomIter);
            _preComputedPaths.erase(pathIdx);
            return;
        }

        float ground = INVALID_HEIGHT;
        float levelZ = creature->GetMapWaterOrGroundLevel(x, y, z, &ground);
        float newZ = INVALID_HEIGHT;

        // flying creature
        if (creature->CanFly())
            newZ = std::max<float>(levelZ, z + rand_norm() * _wanderDistance / 2.0f);
        // point underwater
        else if (ground < levelZ)
        {
            if (!creature->CanEnterWater())
            {
                _validPointsVector[_currentPoint].erase(randomIter);
                _preComputedPaths.erase(pathIdx);
                return;
            }
            else
            {
                if (levelZ > INVALID_HEIGHT)
                    newZ = std::min<float>(levelZ - 2.0f, z + rand_norm() * _wanderDistance / 2.0f);
                newZ = std::max<float>(ground, newZ);
            }
        }
        // point on ground
        else
        {
            if (levelZ <= INVALID_HEIGHT || !creature->CanWalk())
            {
                _validPointsVector[_currentPoint].erase(randomIter);
                _preComputedPaths.erase(pathIdx);
                return;
            }
        }

        creature->UpdateAllowedPositionZ(x, y, newZ);

        if (newZ > INVALID_HEIGHT)
        {
            // flying / swiming creature - dest not in los
            if (!creature->IsWithinLOS(x, y, newZ))
            {
                _validPointsVector[_currentPoint].erase(randomIter);
                _preComputedPaths.erase(pathIdx);
                return;
            }

            finalPath.push_back(G3D::Vector3(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ()));
            finalPath.push_back(G3D::Vector3(x, y, newZ));
        }
        else // ground
        {
            // AC keeps one PathGenerator per generator and Clear()s it (AC RandomMovementGenerator.cpp:152-155);
            // MoP's Clear() is private (PathGenerator.h), so a fresh generator per cache miss gives the same clean state.
            PathGenerator pathGenerator(creature);

            bool result = pathGenerator.CalculatePath(x, y, levelZ, false);
            if (result && !(pathGenerator.GetPathType() & PATHFIND_NOPATH))
            {
                // generated path is too long
                float pathLen = pathGenerator.getPathLength();
                if (pathLen * pathLen > creature->GetExactDistSq(x, y, levelZ) * MAX_PATH_LENGHT_FACTOR * MAX_PATH_LENGHT_FACTOR)
                {
                    _validPointsVector[_currentPoint].erase(randomIter);
                    _preComputedPaths.erase(pathIdx);
                    return;
                }

                finalPath = pathGenerator.GetPath();

                // no valid path - AC checks this after the loop (RandomMovementGenerator.cpp:211-217); moved before it because
                // AC's `begin() + 1` on an empty path is out of range. Same outcome for every path of 2+ points.
                if (finalPath.size() < 2)
                {
                    _validPointsVector[_currentPoint].erase(randomIter);
                    _preComputedPaths.erase(pathIdx);
                    return;
                }

                Movement::PointsArray::iterator itr = finalPath.begin();
                Movement::PointsArray::iterator itrNext = finalPath.begin() + 1;
                float zDiff, distDiff;

                for (; itrNext != finalPath.end(); ++itr, ++itrNext)
                {
                    distDiff = std::sqrt(((*itr).x - (*itrNext).x) * ((*itr).x - (*itrNext).x) + ((*itr).y - (*itrNext).y) * ((*itr).y - (*itrNext).y));
                    zDiff = std::fabs((*itr).z - (*itrNext).z);

                    // Xinef: tree climbing, cut as much as we can
                    if (zDiff > 2.0f ||
                            (G3D::fuzzyNe(zDiff, 0.0f) && distDiff / zDiff < 2.15f)) // ~25 degrees
                    {
                        _validPointsVector[_currentPoint].erase(randomIter);
                        _preComputedPaths.erase(pathIdx);
                        return;
                    }

                    // AC passes LINEOFSIGHT_ALL_CHECKS before the ignore flags (RandomMovementGenerator.cpp:201); MoP's
                    // Map::isInLineOfSight has no LineOfSightChecks parameter
                    if (!map->isInLineOfSight((*itr).x, (*itr).y, (*itr).z + 2.f, (*itrNext).x, (*itrNext).y, (*itrNext).z + 2.f, creature->GetPhaseMask(),
                        VMAP::ModelIgnoreFlags::Nothing))
                    {
                        _validPointsVector[_currentPoint].erase(randomIter);
                        _preComputedPaths.erase(pathIdx);
                        return;
                    }
                }
            }
            else
            {
                _validPointsVector[_currentPoint].erase(randomIter);
                _preComputedPaths.erase(pathIdx);
                return;
            }
        }
    }

    _currentPoint = newPoint;
    ASSERT(!finalPath.empty());
    G3D::Vector3 finalPoint = finalPath.back();
    _currDestPosition.Relocate(finalPoint.x, finalPoint.y, finalPoint.z);

    creature->AddUnitState(UNIT_STATE_ROAMING_MOVE);

    Movement::MoveSplineInit init(creature);
    init.MovebyPath(finalPath);
    SetRandomMoveWalk(creature, init);
    init.Launch();

    ++_moveCount;
    if (roll_chance_i((int32) _moveCount * 25 + 10))
    {
        _moveCount = 0;
        _nextMoveTime.Reset(urand(4000, 8000));
    }

    //Call for creature group update (AC also requires CanLeaderStartMoving(), see above)
    if (creature->GetFormation() && creature->GetFormation()->GetLeader() == creature)
        creature->GetFormation()->LeaderStartedMoving();

    if (sWorld->getBoolConfig(CONFIG_DONT_CACHE_RANDOM_MOVEMENT_PATHS))
        _preComputedPaths.erase(pathIdx);
}

// AC RandomMovementGenerator.cpp:247-272
template<>
bool RandomMovementGenerator<Creature>::DoInitialize(Creature* creature)
{
    if (!creature->IsAlive())
        return true;

    if (!_wanderDistance)
        _wanderDistance = creature->GetWanderDistance();

    _nextMoveTime.Reset(creature->GetSpawnId() && creature->GetWanderDistance() == _wanderDistance ? urand(1, 5000) : 0);
    _wanderDistance = std::max((creature->GetWanderDistance() == _wanderDistance && creature->GetInstanceId() == 0) ? (creature->CanFly() ? MIN_WANDER_DISTANCE_AIR : MIN_WANDER_DISTANCE_GROUND) : 0.0f, _wanderDistance);

    if (G3D::fuzzyEq(_initialPosition.GetExactDist2d(0.0f, 0.0f), 0.0f))
    {
        _initialPosition.Relocate(creature);
        _destinationPoints.clear();
        for (uint8 i = 0; i < RANDOM_POINTS_NUMBER; ++i)
        {
            float angle = (M_PI * 2.0f / (float)RANDOM_POINTS_NUMBER) * i;
            float factor = 0.5f + rand_norm() * 0.5f;
            _destinationPoints.push_back(G3D::Vector3(_initialPosition.GetPositionX() + _wanderDistance * cos(angle)*factor, _initialPosition.GetPositionY() + _wanderDistance * std::sin(angle)*factor, _initialPosition.GetPositionZ()));
        }
    }

    creature->AddUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    return true;
}

template<>
bool RandomMovementGenerator<Creature>::DoReset(Creature* creature)
{
    DoInitialize(creature);
    return true;
}

template<>
void RandomMovementGenerator<Creature>::DoFinalize(Creature* creature, bool, bool)
{
    creature->ClearUnitState(UNIT_STATE_ROAMING | UNIT_STATE_ROAMING_MOVE);
    creature->SetWalk(false);
}

template<>
bool RandomMovementGenerator<Creature>::DoUpdate(Creature* creature, const uint32 diff)
{
    if (creature->HasUnitState(UNIT_STATE_ROOT | UNIT_STATE_STUNNED | UNIT_STATE_DISTRACTED))
    {
        _nextMoveTime.Reset(0);  // Expire the timer
        creature->ClearUnitState(UNIT_STATE_ROAMING_MOVE);
        return true;
    }

    if (creature->movespline->Finalized())
    {
        _nextMoveTime.Update(diff);
        if (_nextMoveTime.Passed())
            _setRandomLocation(creature);
    }
    return true;
}

// AC RandomMovementGenerator.cpp:319-329 (MoP signature takes the owner, see MovementGenerator.h GetResetPosition)
template<>
bool RandomMovementGenerator<Creature>::GetResetPosition(Unit* /*owner*/, float& x, float& y, float& z)
{
    if (_currentPoint < RANDOM_POINTS_NUMBER)
        _currDestPosition.GetPosition(x, y, z);
    else if (G3D::fuzzyNe(_initialPosition.GetExactDist2d(0.0f, 0.0f), 0.0f)) // if initial position is not 0.0f, 0.0f
        _initialPosition.GetPosition(x, y, z);
    else
        return false;
    return true;
}
