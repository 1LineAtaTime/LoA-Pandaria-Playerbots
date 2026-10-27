/*
* This file is part of the Pandaria 5.4.8 Project. See THANKS file for Copyright information
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

#include "MoveSplineInit.h"
#include "MoveSpline.h"
#include "MovementPacketBuilder.h"
#include "Unit.h"
#include "Transport.h"
#include "Vehicle.h"
#include "WorldPacket.h"
#include "Opcodes.h"
#include "Map.h"
#include "BattlegroundSA.h"
#include "Log.h"
#include "Config.h"
#include "MapReference.h"
#include "Player.h"
#include "WorldSession.h"
#include <cmath>

namespace Movement
{
    UnitMoveType SelectSpeedType(uint32 moveFlags)
    {
        if (moveFlags & MOVEMENTFLAG_FLYING)
        {
            if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.flight >= speed_obj.flight_back*/)
                return MOVE_FLIGHT_BACK;
            else
                return MOVE_FLIGHT;
        }
        else if (moveFlags & MOVEMENTFLAG_SWIMMING)
        {
            if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.swim >= speed_obj.swim_back*/)
                return MOVE_SWIM_BACK;
            else
                return MOVE_SWIM;
        }
        else if (moveFlags & MOVEMENTFLAG_WALKING)
        {
            //if (speed_obj.run > speed_obj.walk)
            return MOVE_WALK;
        }
        else if (moveFlags & MOVEMENTFLAG_BACKWARD /*&& speed_obj.run >= speed_obj.run_back*/)
            return MOVE_RUN_BACK;

        // Flying creatures use MOVEMENTFLAG_CAN_FLY or MOVEMENTFLAG_DISABLE_GRAVITY
        // Run speed is their default flight speed.
        return MOVE_RUN;
    }

    // #747: SMSG_MONSTER_MOVE sends the intermediate points of a linear (compressed) path as offsets from the middle
    // of path[1] and the destination (MovementPacketBuilder.cpp WriteLinearPath), each packed by
    // ByteBuffer::appendPackXYZ into 11/11/10 signed bits of quarter yards: +-255.75 yd in x/y, +-127.75 yd in z.
    // A larger offset wraps to the other side (a point 300 yd east of the middle is drawn ~212 yd west), so the client
    // draws a zig-zag hundreds of yards longer than the server's path in the same duration: the unit flies over the
    // terrain at many times its speed and snaps back. MoveSplineInitArgs::_checkPathBounds (MoveSpline.cpp:219) is
    // disabled and tests yards against raw units. With MAX_POINT_PATH_LENGTH 200 x SMOOTH_PATH_STEP_SIZE 4 a path can
    // be ~800 yd (AC: 148 points); any path shorter than 512 yd always fits (no point is farther than half the path
    // length from the middle of its ends).
    static bool PackedPathOffsetsFit(PointsArray const& path, size_t last)
    {
        Vector3 const middle = (path[1] + path[last]) / 2.f;
        for (size_t i = 2; i < last; ++i)
        {
            Vector3 const offset = middle - path[i];
            if (std::fabs(offset.x) >= 255.0f || std::fabs(offset.y) >= 255.0f || std::fabs(offset.z) >= 127.0f)
                return false;
        }
        return true;
    }

    // Player movers only (playerbots; a real client moves itself and its charge/jump splines are short; creature
    // scripts keep their arrival semantics): end the spline at the farthest point the packet can carry. Server and
    // client then move along the same, shorter path and the bot re-paths from there.
    static void ClipPathToPackedRange(Unit* unit, PointsArray& path)
    {
        if (unit->GetTypeId() != TYPEID_PLAYER || path.size() < 4)
            return;

        size_t last = path.size() - 1;
        if (PackedPathOffsetsFit(path, last))
            return;

        size_t const before = path.size();
        while (last > 2 && !PackedPathOffsetsFit(path, last))
            --last;
        path.resize(last + 1);

        TC_LOG_DEBUG("movement", "MoveSplineInit::Launch: %s map %u - path of %u points clipped to %u (packed offset > 255 yd, #747)",
            unit->GetName().c_str(), unit->GetMapId(), uint32(before), uint32(path.size()));
    }

    // #747 Twin Peaks: in a real 5.4.8 client the bots "fly around, teleport and move extremely fast" while the server
    // positions and the splines on the wire are right (wave 72 headless capture: the track a client builds from the
    // SMSG_MONSTER_MOVE splines equals the server [PROBE] track within 0.1 yd, median, for every bot in view). Real
    // players are never drawn from splines: the client draws them from the movement status the server relays
    // (SMSG_PLAYER_MOVE: position, facing, FORWARD, timestamp) and walks them itself - its own speed table, its own
    // ground. MoP 5.4.8 playerbots, whose bots look right in this client, move ground bots exactly that way: the bot
    // chase/follow/point generators hand the path to a fake-player step queue, one FakePlayerMovementInProgress step per
    // path point, which turns the bot to the next point and steps it like a client (speed x dt, ground height per step,
    // falls), broadcasting SMSG_PLAYER_MOVE (opcode wire 0x1A32) at every start / turn / landing / stop and every 500 ms.
    // It keeps SMSG_MONSTER_MOVE splines (the same Launch as ours) for flying / vehicle / jump moves, in
    // Orgrimmar (zone 1637), for Warsong Gulch point moves (zone 3277) and within 20 yd of the Twin Peaks Alliance mid
    // graveyard island (1877,439,-4) (the island has no walkable exit, a spline crosses the gap).
    // Here the server keeps its spline (every movement generator, arrival check and position update is unchanged) and
    // only what observers receive changes: a ground, linear spline of a playerbot at its own run speed is announced as
    // client movement status - start here, turns and a 500 ms heartbeat from Unit::UpdateSplineMovement, stop at
    // arrival / Stop() / Unit::DisableSpline().
    // Movement.BotStatusMovement (worldserver.conf, read once): 0 = off (SMSG_MONSTER_MOVE as before), 1 = battleground
    // and arena maps (default), 2 = every map.
    static int32 BotStatusMovementMode()
    {
        static int32 const mode = sConfigMgr->GetIntDefault("Movement.BotStatusMovement", 1);
        return mode;
    }

    static bool SendsBotMoveStatus(Unit* unit, MoveSplineInitArgs const& args, uint32 moveFlags)
    {
        Player* player = unit->ToPlayer();
        if (!player || !player->IsInWorld() || !player->GetSession() || !player->GetSession()->IsBot())
            return false;

        int32 const mode = BotStatusMovementMode();
        if (mode <= 0 || (mode == 1 && !unit->GetMap()->IsBattlegroundOrArena()))
            return false;

        if (unit->GetTransGUID() || unit->GetVehicle() || args.TransformForTransport)
            return false;

        // Twin Peaks Alliance mid graveyard island: no walkable exit, the path crosses the gap - a spline, as in MoP 5.4.8
        if (unit->GetMapId() == 726 && unit->GetExactDist2d(1877.0f, 439.0f) < 20.0f)
            return false;

        // only what a client walks by itself: a linear ground path along its own heading, no scripted effect
        if (args.flags & (MoveSplineFlag::Catmullrom | MoveSplineFlag::Flying | MoveSplineFlag::Falling |
            MoveSplineFlag::FallingSlow | MoveSplineFlag::Parabolic | MoveSplineFlag::Animation | MoveSplineFlag::Cyclic |
            MoveSplineFlag::Enter_Cycle | MoveSplineFlag::Frozen | MoveSplineFlag::UncompressedPath |
            MoveSplineFlag::OrientationFixed | MoveSplineFlag::OrientationInversed | MoveSplineFlag::TransportEnter |
            MoveSplineFlag::TransportExit) || args.walk)
            return false;

        if (moveFlags & (MOVEMENTFLAG_ROOT | MOVEMENTFLAG_WALKING | MOVEMENTFLAG_SWIMMING | MOVEMENTFLAG_FLYING |
            MOVEMENTFLAG_DISABLE_GRAVITY | MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_FALLING | MOVEMENTFLAG_FALLING_FAR |
            MOVEMENTFLAG_HOVER | MOVEMENTFLAG_BACKWARD | MOVEMENTFLAG_STRAFE_LEFT | MOVEMENTFLAG_STRAFE_RIGHT))
            return false;

        // the client moves a FORWARD player at the run speed it knows: a charge or a scripted velocity stays a spline
        float const runSpeed = unit->GetSpeed(MOVE_RUN);
        return std::fabs(args.velocity - runSpeed) <= 0.01f * runSpeed + 0.01f;
    }

    void SendBotMoveStatus(Unit* unit)
    {
        if (!unit->IsInWorld())
            return;

        WorldPacket data(SMSG_PLAYER_MOVE, 64);
        unit->WriteMovementInfo(data);

        Map* map = unit->GetMap();
        if (!map->IsBattlegroundOrArena())
        {
            unit->SendMessageToSet(&data, false);
            return;
        }

        // a battleground holds at most 80 players: only the real clients that have the unit in view get the status
        // (bots draw nothing, so the other bots of the map are skipped)
        Map::PlayerList const& players = map->GetPlayers();
        for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
            if (Player* viewer = itr->GetSource())
                if (viewer != unit && viewer->GetSession() && !viewer->GetSession()->IsBot() && viewer->HaveAtClient(unit))
                    viewer->SendDirectMessage(&data);
    }

    int32 MoveSplineInit::Launch()
    {
        MoveSpline& move_spline = *unit->movespline;

        Location real_position(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZMinusOffset(), unit->GetOrientation());
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        if (unit->GetTransGUID())
        {
            real_position.x = unit->GetTransOffsetX();
            real_position.y = unit->GetTransOffsetY();
            real_position.z = unit->GetTransOffsetZ();
            real_position.orientation = unit->GetTransOffsetO();
        }

        // there is a big chance that current position is unknown if current state is not finalized, need compute it
        // this also allows calculate spline position and update map position in much greater intervals
        // Don't compute for transport movement if the unit is in a motion between two transports
        if (!move_spline.Finalized() && move_spline.onTransport == (unit->GetTransGUID() != 0))
            real_position = move_spline.ComputePosition();

        // should i do the things that user should do? - no.
        if (args.path.empty())
            return 0;

        // correct first vertex
        args.path[0] = real_position;
        args.initialOrientation = real_position.orientation;
        args.flags.enter_cycle = args.flags.cyclic;

        // #747: keep every packed path offset inside the SMSG_MONSTER_MOVE range (see ClipPathToPackedRange)
        if (!(args.flags & MoveSplineFlag::UncompressedPath) && !args.flags.cyclic)
            ClipPathToPackedRange(unit, args.path);
        move_spline.onTransport = (unit->GetTransGUID() != 0);

        uint32 moveFlags = unit->m_movementInfo.GetMovementFlags();
        moveFlags |= MOVEMENTFLAG_FORWARD;

        if (moveFlags & MOVEMENTFLAG_ROOT)
            moveFlags &= ~MOVEMENTFLAG_MASK_MOVING;

        if (!args.HasVelocity)
        {
            if (unit->GetTotalAuraEffectValue(SPELL_AURA_MOD_DECREASE_SPEED) < -99.0f)
                return 0;

            // If spline is initialized with SetWalk method it only means we need to select
            // walk move speed for it but not add walk flag to unit
            uint32 moveFlagsForSpeed = moveFlags;
            if (args.walk)
                moveFlagsForSpeed |= MOVEMENTFLAG_WALKING;
            else
                moveFlagsForSpeed &= ~MOVEMENTFLAG_WALKING;

            args.velocity = unit->GetSpeed(SelectSpeedType(moveFlagsForSpeed));
        }

        args.velocity = std::min(args.velocity, args.flags.catmullrom || args.flags.flying || args.flags.falling || args.flags.parabolic ? 50.0f : std::max(28.0f, unit->GetSpeed(MOVE_RUN) * 4.0f));

        if (!args.Validate(unit))
            return 0;

        // #747: client-style status movement (see SendsBotMoveStatus); the previous spline's mode is still on move_spline
        bool const wasStatusMoving = move_spline.statusMode && !move_spline.Finalized();
        bool const statusMove = SendsBotMoveStatus(unit, args, moveFlags);
        if (wasStatusMoving && !statusMove)
        {
            // observers dead-reckon a status-moving bot along its heading: stop it where it is before a spline takes over
            unit->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
            SendBotMoveStatus(unit);
        }

        unit->m_movementInfo.SetMovementFlags(moveFlags);
        move_spline.Initialize(args);
        move_spline.statusMode = statusMove;
        move_spline.statusTimer = 0;

        if (statusMove)
        {
            // start (or a new path while running): face the first segment and run forward, like MSG_MOVE_START_FORWARD
            Location const start = move_spline.ComputePosition();
            unit->SetOrientation(start.orientation);
            move_spline.statusO = start.orientation;
            SendBotMoveStatus(unit);
            return move_spline.Duration();
        }

        WorldPacket data(SMSG_MONSTER_MOVE, 64);
        PacketBuilder::WriteMonsterMove(move_spline, data, unit);
        unit->SendMessageToSet(&data, true);

        return move_spline.Duration();
    }

    void MoveSplineInit::Stop()
    {
        MoveSpline& move_spline = *unit->movespline;

        // No need to stop if we are not moving
        if (move_spline.Finalized())
            return;

        Location loc = move_spline.ComputePosition();
        args.flags = MoveSplineFlag::Done;
        unit->m_movementInfo.RemoveMovementFlag(MOVEMENTFLAG_FORWARD);
        bool const wasStatusMoving = move_spline.statusMode;    // #747
        move_spline.Initialize(args);
        move_spline.statusMode = false;

        if (wasStatusMoving)
        {
            // #747: a status-moving bot stops like a client (MSG_MOVE_STOP): its status without FORWARD
            SendBotMoveStatus(unit);
            return;
        }

        WorldPacket data(SMSG_MONSTER_MOVE, 64);

        PacketBuilder::WriteStopMovement(loc, args.splineId, data, unit);
        unit->SendMessageToSet(&data, true);
    }

    MoveSplineInit::MoveSplineInit(Unit* m) : unit(m)
    {
        args.splineId = splineIdGen.NewId();
        // Elevators also use MOVEMENTFLAG_ONTRANSPORT but we do not keep track of their position changes
        args.TransformForTransport = unit->GetTransGUID();
        // mix existing state into new
        args.walk = unit->m_movementInfo.HasMovementFlag(MOVEMENTFLAG_WALKING);
        args.flags.canSwim = unit->CanSwim();
        args.flags.flying = unit->m_movementInfo.HasMovementFlag(MovementFlags(MOVEMENTFLAG_CAN_FLY | MOVEMENTFLAG_DISABLE_GRAVITY));
        args.flags.smoothGroundPath = true; // enabled by default, CatmullRom mode or client config "pathSmoothing" will disable this
    }

    void MoveSplineInit::SetFacing(const Unit* target)
    {
        args.flags.EnableFacingTarget();
        args.facing.target = target->GetGUID();
    }

    void MoveSplineInit::SetFacing(float angle)
    {
        if (args.TransformForTransport)
        {
            if (Unit* vehicle = unit->GetVehicleBase())
                angle -= vehicle->GetOrientation();
            else if (Transport* transport = unit->GetTransport())
                angle -= transport->GetOrientation();
        }

        args.facing.angle = G3D::wrap(angle, 0.f, (float)G3D::twoPi());
        args.flags.EnableFacingAngle();
    }

    // #736: Strand of the Ancients - a path the server builds for a player-controlled mover (a playerbot, the
    // demolisher it drives, a pet) stops in front of a gate that is not destroyed (BattlegroundSA::ClipPathAtClosedGates;
    // the navmesh has no gates in it). Real clients move themselves and collide with the gate model.
    static void ClipPathAtClosedGates(Unit* unit, PointsArray& path)
    {
        if (unit->GetMapId() != 607 || unit->GetTransGUID() || !unit->GetCharmerOrOwnerPlayerOrPlayerItself())
            return;

        if (BattlegroundMap* bgMap = unit->GetMap()->ToBattlegroundMap())
            if (Battleground* bg = bgMap->GetBG())
                static_cast<BattlegroundSA*>(bg)->ClipPathAtClosedGates(unit, path);
    }

    void MoveSplineInit::MoveTo(const Vector3& dest, bool generatePath, bool forceDestination)
    {
        if (generatePath)
        {
            PathGenerator path(unit);
            bool result = path.CalculatePath(dest.x, dest.y, dest.z, forceDestination);
            if (result && !(path.GetPathType() & PATHFIND_NOPATH))
            {
                PointsArray points = path.GetPath();
                ClipPathAtClosedGates(unit, points);
                MovebyPath(points);
                return;
            }
        }

        args.path_Idx_offset = 0;
        args.path.resize(2);
        TransportPathTransform transform(unit, args.TransformForTransport);
        // #736: the straight-line fallback (no navmesh path) is held at a closed SotA gate too - world coordinates only
        // (a vehicle seat entry, Vehicle.cpp, passes a seat offset with the transform disabled)
        PointsArray line = { Vector3(unit->GetPositionX(), unit->GetPositionY(), unit->GetPositionZ()), dest };
        if (args.TransformForTransport)
            ClipPathAtClosedGates(unit, line);
        args.path[1] = transform(line.back());
    }

    void MoveSplineInit::SetFall()
    {
        args.flags.EnableFalling();
        args.flags.fallingSlow = unit->HasUnitMovementFlag(MOVEMENTFLAG_FALLING_SLOW);
    }

    Vector3 TransportPathTransform::operator()(Vector3 input)
    {
        if (_transformForTransport)
            if (TransportBase* transport = _owner->GetDirectTransport())
                transport->CalculatePassengerOffset(input.x, input.y, input.z);

        return input;
    }
}
