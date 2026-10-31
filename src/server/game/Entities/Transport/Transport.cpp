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

#include "Common.h"
#include "Transport.h"
#include "MapManager.h"
#include "ObjectMgr.h"
#include "Path.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"
#include "UpdateData.h"
#include "DBCStores.h"
#include "World.h"
#include "Config.h"
#include "GameObjectAI.h"
#include "Vehicle.h"
#include "MapReference.h"
#include "Player.h"
#include "Cell.h"
#include "CellImpl.h"
#include "GameObjectModel.h"
#include "Totem.h"
#include "MoveSpline.h"
#include "ZoneScript.h"
#include "MMapFactory.h"
#include "G3D/Matrix4.h"
#include <atomic>

Transport::Transport() : GameObject(),
    _transportInfo(nullptr), _isMoving(true), _pendingStop(false),
    _triggeredArrivalEvent(false), _triggeredDepartureEvent(false),
    _passengerTeleportItr(_passengers.begin()), _delayedAddModel(false), _delayedTeleport(false),
    _stopFrameStartTime(0), _stopFrameEndTime(0), _stopFrameStartProgress(0), _stopFrameTargetProgress(0),
    _stopFrameMode(0), _stopFrameLogAt(0)
{
    m_updateFlag = UPDATEFLAG_TRANSPORT | UPDATEFLAG_LOWGUID | UPDATEFLAG_STATIONARY_POSITION | UPDATEFLAG_ROTATION;
}
Transport::~Transport()
{
    ASSERT(_passengers.empty());
    UnloadStaticPassengers();
}

// #851 (owner 2026-10-20, real 5.4.8 client, build 72: "the 2 boats/ships are not even sailing anymore to the deck"):
// the Strand of the Ancients boats are sent as MoP 5.4.8 sends them (where they sail):
//   the boat is created with GAMEOBJECT_FIELD_LEVEL = data0 (pause 60133), GO state READY (startOpen 0),
//   PathProgress = TotalTime - 1; no entry special case for 193182-193185; the movement update writes a stop-frame
//   count of 0 (WriteBits(0, 22)) and ServerTime = PathProgress for type 11/15; the values update sends dynFlags 0 +
//   pathProgress = PathProgress % period for a transport; BattlegroundSA StartShips only sets ShipsStarted - no
//   DoorOpen, no state change, ever. That is this CreateLocal without EnableStopFrames (the stoppable-transport code below then holds the server boat in READY).
// The #736 stop-frame encoding (GO state 25, stop-frame list, Transport.StopFrameClientEncoding clocks) is what the real
// client drew frozen; it stays behind Transport.SotaBoatStopFrames = 1 for rollback only. Read per boat creation.
// #851 (owner 2026-10-20, build 73: the MoP 5.4.8 boats sail but loop - "they reset after they get to their destination
// (deck)... restart from the origin"): Transport.SotaBoatStopFrames = 2 sends the boats as a two-floor elevator -
// TrinityCore's type-11 stop-frame encoding (sniff-based: TC 6.0.2 83668e8f8f "fixes ... all stoppable elevators";
// semantics as rewritten in TC 630b60eb0d / 37c374c49f, GameObjectType::Transport::Update / OnStateChanged):
//   create block: stop frames [data0 = 60133 = the dock frame], ServerTime = getMSTime() (the absolute clock)
//   hold at sea:  GO state 24 (GO_STATE_TRANSPORT_ACTIVE = "go to / stay at frame 0"), LEVEL = the time it got there
//                 (<= now, so the client draws it arrived = at sea), dynamic field = dynFlags 0 + path progress 0
//   sail:         GO state 25 (stop frame 0 = the dock) + LEVEL = now + |60133 - 0| ms in ONE values update: the client
//                 moves the boat from its current progress to the dock frame, arriving at LEVEL
//   at the dock:  nothing more is sent; once LEVEL has passed the client holds state 25 at the dock frame (no loop:
//                 a stopped state never wraps; only the no-stop-frame regime loops the TransportAnimation path)
// No GO_DYNFLAG_LO_STOPPED and no per-tick progress pushes (TC never sends them for type 11). Why #736 froze under
// the same model: mode 0 gave a relative ServerTime (0) against an absolute LEVEL (arrival hours away), mode 1 never
// changed the GO state (25 -> 25) and sent a path progress derived from LEVEL (0xE9AE = 91 %) as the source.
int32 TransportSotaBoatMode()
{
    int32 const mode = sConfigMgr->GetIntDefault("Transport.SotaBoatStopFrames", 0, true);
    return mode == 1 || mode == 2 ? mode : 0;
}

// #851: Transport.SotaBoatHoldSeconds - mode 2 only: round 1 boats hold at sea this long after the battleground is set
// up (BattlegroundSA TotalTime), then sail. Default 60 = AC BG_SA_BOAT_START (the boats leave at 1:00 of the 2:00
// warm-up and dock as round 1 starts). Round 2 sails at once, as AC. Used by BattlegroundSA.cpp (extern, no header).
uint32 TransportSotaBoatHoldMs()
{
    int32 const secs = sConfigMgr->GetIntDefault("Transport.SotaBoatHoldSeconds", 60, true);
    return uint32(std::max<int32>(secs, 0)) * IN_MILLISECONDS;
}

bool Transport::CreateLocal(uint32 guidlow, uint32 entry, Map* map, float x, float y, float z, float ang, uint32 animprogress)
{
    SetMap(map);

    Relocate(x, y, z, ang);
    RelocateStationaryPosition(x, y, z, ang);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)", guidlow, x, y);
        return false;
    }

    SetZoneScript();
    if (m_zoneScript)
    {
        entry = m_zoneScript->GetGameObjectEntry(guidlow, entry);
        if (!entry)
            return false;
    }

    Object::_Create(sObjectMgr->GetGenerator<HighGuid::Mo_Transport>().Generate(), 0, HighGuid::Mo_Transport);

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    ASSERT(goinfo->type == GAMEOBJECT_TYPE_TRANSPORT);

    if (!goinfo)
    {
        TC_LOG_ERROR("entities.transport", "Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, map->GetId(), x, y, z, ang);
        return false;
    }

    m_goInfo = sObjectMgr->GetGameObjectTemplate(entry);
    m_goTemplateAddon = sObjectMgr->GetGameObjectTemplateAddon(entry);

    if (!(m_goValue.Transport.AnimationInfo = sTransportMgr->GetTransportAnimInfo(entry)))
    {
        TC_LOG_ERROR("entities.transport", "Transport %u (name: %s) will not be created, missing data in TransportAnimation.dbc", entry, goinfo->name.c_str());
        return false;
    }

    // initialize waypoints
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    m_goValue.Transport.PathProgress = 0;
    SetObjectScale(goinfo->size);
    if (m_goTemplateAddon)
    {
        SetFaction(m_goTemplateAddon->faction);
        SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, m_goTemplateAddon->flags);
    }
    SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, goinfo->transport.pause);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    SetGoType(GAMEOBJECT_TYPE_TRANSPORT);
    SetGoAnimProgress(animprogress);
    SetName(goinfo->name);
    SetWorldRotationAngles(ang, 0, 0);
    SetParentRotation({ });
    SetGoState(goinfo->transport.startOpen ? GO_STATE_ACTIVE : GO_STATE_READY);
    if (goinfo->transport.pause)
        m_goValue.Transport.PathProgress = goinfo->transport.startOpen ? goinfo->transport.pause : m_goValue.Transport.AnimationInfo->TotalTime - 1;

    CreateModel();
    LastUsedScriptID = GetGOInfo()->ScriptId;
    AIM_Initialize();

    // I ain't porting fixes to GO rotations and cleaning up all the mess that's now in `gameobject` table just for a few transports.
    // The following data came from sniffs.
    auto const SetRotations = [this](float x, float y, float z, float w)
    {
        SetParentRotation({ x, y, z, w });
    };
    switch (GetEntry())
    {
        case 218203: // Subway
        case 218204: // Subway
        case 218205: // Subway
        case 218206: // Subway
        case 218207: // Subway
        case 218208: // Subway
        case 196840: // Doodad_icecrown_elevator02
        case 202220: // Lady Deathwhisper Elevator
        case 202234: // Doodad_icecrown_elevator02
            SetRotations(0, 0, 1, 0);
            break;
        case 188360: // Doodad_Vrykul_Gondola01
        case 190587: // Doodad_Vrykul_Gondola_01
            SetRotations(0, 0, 0.9999905f, 0.004363241f);
            break;
        case 183177: // Telredor elevator (#218): no sniff row; the spawn's own rotation (gameobject guid 22015 rotation2/3),
                     // otherwise the animation offset is applied unrotated and the platform stops ~35 yd off the AC graph node
            SetRotations(0, 0, -0.694658f, 0.71934f);
            break;
        case 186452: // Doodad_VR_Elevator_Gate01
        case 190118: // Doodad_VR_Elevator_Lift02
        case 190119: // Doodad_VR_Elevator_Pulley01
            switch (guidlow)
            { //   186452      190118      190119
                case 52199: case 43852: case 43824: SetRotations(0, 0, 0.9153115f, 0.40274660f); break;
                case 52241: case 43823: case 43853: SetRotations(0, 0, -0.3705573f, 0.92880960f); break;
                case 52267: case 43825: case 43855: SetRotations(0, 0, -0.7604060f, 0.64944800f); break;
                case 52316: case 43854: case 43857: SetRotations(0, 0, -0.7489557f, 0.66262010f); break;
                case 52335: case 43856: case 43826: SetRotations(0, 0, 0.9958049f, 0.09150153f); break;
                case 53540: case 43822: /*missing*/ SetRotations(0, 0, 0.9153115f, 0.40274660f); break;
            }
            break;
        case 186758: // Doodad_HF_Elevator_Gate01
        case 186759: // Doodad_HF_Elevator_Gate02
        case 186760: // Doodad_HF_Elevator_Gate03
        case 186761: // Doodad_HF_Elevator_Lift_01
        case 186762: // Doodad_HF_Elevator_Lift01
            SetRotations(0, 0, 0.9999905f, -0.004363328f);
            break;
        case 188515: // Doodad_ID_elevator01
        case 188516: // Doodad_ID_elevator02
        case 188517: // Doodad_ID_elevator03
            SetRotations(0, 0, 0.3338069f, 0.9426414f);
            break;
        case 188136: // Doodad_LogRun_PumpElevator04
        case 188428: // Doodad_LogRun_PumpElevator05
        case 191278: // Doodad_LogRun_PumpElevator02
        case 191329: // Doodad_LogRun_PumpElevator03
        case 192588: // Doodad_LogRun_PumpElevator08
        case 192717: // Doodad_LogRun_PumpElevator01
            SetRotations(0, 0, 0.9990482f, 0.04361934f);
            break;
        case 189991: // Doodad_LogRun_PumpElevator04
        case 191276: // Doodad_LogRun_PumpElevator01
        case 191277: // Doodad_LogRun_PumpElevator02
            SetRotations(0, 0, 0.9510565f, 0.3090171f);
            break;
        case 188521: // Elevator
            SetRotations(0, 0, 0.9320078f, -0.3624381f);
            break;
        case 188514: // Doodad_Nexus_Elevator_BaseStructure_01
            switch (guidlow)
            {
                case 42149: case 51417: SetRotations(0, 0, -0.2630311f, 0.96478740f); break;
                case 46898: case 47839: SetRotations(0, 0, 0.9969173f, -0.07845917f); break;
                case 46899: case 42272: SetRotations(0, 0, 0.5224987f, 0.85264010f); break;
            }
            break;
        case 194675: // Doodad_Nexus_Elevator_BaseStructure_01
            SetRotations(0, 0, 0, 1);
            break;
        // #736 Strand of the Ancients boats (BattlegroundSA::ResetObjs repeats the rotation, "MAD props for Kiper").
        // Set here because TransportMgr::CreateLocalTransport -> Map::AddToMap(Transport*) sends the create block to
        // every player already on the map before AddObject returns - in round 2 every player, in round 1 whoever
        // triggered SetupBattleground. Doing it in ResetObjs after AddObject was too late: those clients got a boat
        // without stop frames (they loop the 60633 ms TransportAnimation path: sea -> dock -> back to sea) and with
        // the identity path rotation.
        case 193182: // The Graceful Maiden (BG_SA_BOAT_ONE, Alliance attackers)
        case 193184: // The Coffin Carrier  (BG_SA_BOAT_ONE, Horde attackers)
            SetRotations(0.0f, 0.0f, 1.0f, 0.0002f);
            if (int32 mode = TransportSotaBoatMode())   // #851: 0 = MoP 5.4.8 (no stop frames, GO state READY)
                EnableStopFrames(uint8(mode));
            break;
        case 193185: // The Frostbreaker    (BG_SA_BOAT_TWO, Alliance attackers)
        case 193183: // The Blightbringer   (BG_SA_BOAT_TWO, Horde attackers)
            SetRotations(0.0f, 0.0f, 1.0f, 0.00001f);
            if (int32 mode = TransportSotaBoatMode())   // #851: 0 = MoP 5.4.8 (no stop frames, GO state READY)
                EnableStopFrames(uint8(mode));
            break;
    }
    return true;
}

bool Transport::Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress)
{
    Relocate(x, y, z, ang);

    if (!IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Transport (GUID: %u) not created. Suggested coordinates isn't valid (X: %f Y: %f)",
            guidlow, x, y);
        return false;
    }

    Object::_Create(guidlow, 0, HighGuid::Mo_Transport);

    GameObjectTemplate const* goinfo = sObjectMgr->GetGameObjectTemplate(entry);
    if (!goinfo)
    {
        TC_LOG_ERROR("sql.sql", "Transport not created: entry in `gameobject_template` not found, guidlow: %u map: %u  (X: %f Y: %f Z: %f) ang: %f", guidlow, mapid, x, y, z, ang);
        return false;
    }

    m_goInfo = goinfo;
    m_goTemplateAddon = sObjectMgr->GetGameObjectTemplateAddon(entry);

    TransportTemplate const* tInfo = sTransportMgr->GetTransportTemplate(entry);
    if (!tInfo)
    {
        TC_LOG_ERROR("sql.sql", "Transport %u (name: %s) will not be created, missing `transport_template` entry.", entry, goinfo->name.c_str());
        return false;
    }

    _transportInfo = tInfo;

    // initialize waypoints
    _nextFrame = tInfo->keyFrames.begin();
    _currentFrame = _nextFrame++;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;
    _delayedTeleport = false;
    _pendingStop = false;

    m_goValue.Transport.PathProgress = 0;
    SetObjectScale(goinfo->size);
    if (m_goTemplateAddon)
    {
        SetFaction(m_goTemplateAddon->faction);
        SetUInt32Value(GAMEOBJECT_FIELD_FLAGS, m_goTemplateAddon->flags);
    }
    SetPeriod(tInfo->pathTime);
    SetEntry(goinfo->entry);
    SetDisplayId(goinfo->displayId);
    SetGoState(!goinfo->moTransport.canBeStopped ? GO_STATE_READY : GO_STATE_ACTIVE);
    SetGoType(GAMEOBJECT_TYPE_MO_TRANSPORT);
    SetGoAnimProgress(animprogress);
    SetName(goinfo->name);
    SetWorldRotationAngles(ang, 0, 0);
    SetParentRotation({ });

    CreateModel();
    LastUsedScriptID = GetGOInfo()->ScriptId;
    AIM_Initialize();

    // Load MMAPs for transport map
    std::string basePath = (sWorld->GetDataPath() + "mmaps").c_str();
    for (uint32 x = 31; x <= 32; ++x)
        for (uint32 y = 31; y <= 32; ++y)
            MMAP::MMapFactory::createOrGetMMapManager()->loadMap(GetGOInfo()->moTransport.mapID, x, y);

    return true;
}

void Transport::CleanupsBeforeDelete(bool finalCleanup)
{
    _delayedTeleport = false;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;
    _pendingStop = false;
    
    UnloadStaticPassengers();
    while (!_passengers.empty())
    {
        WorldObject* obj = *_passengers.begin();
        RemovePassenger(obj);
    }

    GameObject::CleanupsBeforeDelete(finalCleanup);
}

void Transport::Update(uint32 diff)
{
    uint32 const positionUpdateDelay = 0; // 200;

    if (AI())
        AI()->UpdateAI(diff);
    else if (!AIM_Initialize())
        TC_LOG_ERROR("entities.transport", "Could not initialize GameObjectAI for Transport");

    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
    {
        uint32& timer = m_goValue.Transport.PathProgress;

        // #736 stop-frame mode: the progress follows the commanded leg (GAMEOBJECT_FIELD_LEVEL is its arrival time)
        if (!_stopFrames.empty())
            UpdateStopFrameProgress();
        // Stoppable transport case
        else if (uint32 pauseTime = GetUInt32Value(GAMEOBJECT_FIELD_LEVEL))
        {
            uint32 periodTime = GetTransportPeriod();

            if (GetGoState() == GO_STATE_ACTIVE ? timer < pauseTime || timer == periodTime - 1 : timer < periodTime - 1)
            {
                timer += diff;

                if (GetGoState() == GO_STATE_ACTIVE) // Should transition from periodTime - 1 back to 0
                    timer %= GetTransportPeriod();

                if (timer >= pauseTime && GetGoState() == GO_STATE_ACTIVE)
                {
                    timer = pauseTime;
                    _triggeredDepartureEvent = false;
                    uint32 eventid = GetGOInfo()->transport.pause2EventID;
                    if (!_triggeredArrivalEvent && eventid)
                    {
                        _triggeredArrivalEvent = true;
                        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
                        EventInform(eventid);
                        //sScriptMgr->OnEventInform(this, eventid);
                    }
                }

                if (timer >= periodTime - 1 && GetGoState() == GO_STATE_READY)
                {
                    timer = periodTime - 1; // Client expects 65535 in dynamic field to draw the transport as stopped on first frame, otherwise (if 0 is sent) it will play the animation backwards
                    _triggeredArrivalEvent = false;
                    uint32 eventid = GetGOInfo()->transport.pause1EventID;
                    if (!_triggeredDepartureEvent && eventid)
                    {
                        _triggeredDepartureEvent = true;
                        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
                        EventInform(eventid);
                        //sScriptMgr->OnEventInform(this, eventid);
                    }
                }
            }
        }
        // Unstoppable transport case
        else
            timer += diff;

        timer %= GetTransportPeriod();

        // Ugly workaround. We want the timer to be 0 when the transport is stopped to properly calculate position, but client expects 65535 in dynamic field
        uint32 realTimer = timer % (GetTransportPeriod() - 1);

        if (_delayedAddModel)
        {
            _delayedAddModel = false;
            if (m_model)
                GetMap()->InsertGameObjectModel(*m_model);
        }

        G3D::Vector3 origin{ m_stationaryPosition.GetPositionX(), m_stationaryPosition.GetPositionY(), m_stationaryPosition.GetPositionZ() };

        G3D::Vector3 pos;
        {
            TransportAnimationEntry const* prev = nullptr;
            TransportAnimationEntry const* next = nullptr;
            for (auto itr = m_goValue.Transport.AnimationInfo->Path.begin(); itr != m_goValue.Transport.AnimationInfo->Path.end(); prev = (itr++)->second)
            {
                next = itr->second;
                if (realTimer <= next->TimeIndex)
                    break;
            }
            if (prev == next)
                prev = nullptr;

            if (prev)
                pos = G3D::Vector3{ prev->X, prev->Y, prev->Z }.lerp({ next->X, next->Y, next->Z }, (float)(realTimer - prev->TimeIndex) / (next->TimeIndex - prev->TimeIndex));
            else if (next)
                pos = G3D::Vector3{ next->X, next->Y, next->Z };

            if (prev || next)
            {
                // GAMEOBJECT_PARENTROTATION contains path rotation, while GameObject's own rotation (sent as part of movement update) allows it to spin around its axis while still maintaining the same path
                G3D::Quat parentRotation{ GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 0), GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 1), GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 2), GetFloatValue(GAMEOBJECT_FIELD_PARENT_ROTATION + 3) };
                G3D::Matrix4 parentRotationMatrix{ parentRotation.toRotationMatrix() };
                G3D::Matrix4 translationMatrix = G3D::Matrix4::translation(pos);
                pos = origin + (parentRotationMatrix * translationMatrix).column(3).xyz(); // Last column of 4x4 transformation matrix stores translation transformation, we only need it
            }
            else
                pos = G3D::Vector3{ GetPositionX(), GetPositionY(), GetPositionZ() };
        }

        G3D::Quat rot;
        float o;
        {
            TransportRotationEntry const* prev = nullptr;
            TransportRotationEntry const* next = nullptr;
            for (auto itr = m_goValue.Transport.AnimationInfo->Rotations.begin(); itr != m_goValue.Transport.AnimationInfo->Rotations.end(); prev = (itr++)->second)
            {
                next = itr->second;
                if (realTimer <= next->TimeIndex)
                    break;
            }
            if (prev == next)
                prev = nullptr;

            if (prev)
                rot = G3D::Quat{ prev->X, prev->Y, prev->Z, prev->W }.slerp({ next->X, next->Y, next->Z, next->W }, (float)(realTimer - prev->TimeIndex) / (next->TimeIndex - prev->TimeIndex));
            else if (next)
                rot = G3D::Quat{ next->X, next->Y, next->Z, next->W };

            if (prev || next)
            {
                float pitch, roll; // Currently unused, a few transport animations use them, but these GameObjects are not present in game
                rot.toRotationMatrix().toEulerAnglesZYX(o, pitch, roll);
                o = m_stationaryPosition.GetOrientation() + o;
            }
            else
                o = GetOrientation();
        }

        UpdatePosition(pos.x, pos.y, pos.z, o);

        if (!_stopFrames.empty())
            LogStopFramePosition();     // #851 boat monitoring

        sScriptMgr->OnTransportUpdate(this, diff);
        return;
    }


    if (GetKeyFrames().size() <= 1)
        return;

    if (IsMoving() || !_pendingStop)
        m_goValue.Transport.PathProgress += diff;

    uint32 timer = m_goValue.Transport.PathProgress % GetTransportPeriod();
    bool justStopped = false;

    // Set current waypoint
    // Desired outcome: _currentFrame->DepartureTime < timer < _nextFrame->ArriveTime
    // ... arrive | ... delay ... | departure
    //      event /         event /
    for (;;)
    {
        if (timer >= _currentFrame->ArriveTime)
        {
            if (!_triggeredArrivalEvent)
            {
                DoEventIfAny(*_currentFrame, false);
                _triggeredArrivalEvent = true;
            }

            if (timer < _currentFrame->DepartureTime)
            {
                SetMoving(false);
                justStopped = true;
                if (_pendingStop && GetGoState() != GO_STATE_READY)
                {
                    SetGoState(GO_STATE_READY);
                    m_goValue.Transport.PathProgress = (m_goValue.Transport.PathProgress / GetTransportPeriod());
                    m_goValue.Transport.PathProgress *= GetTransportPeriod();
                    m_goValue.Transport.PathProgress += _currentFrame->ArriveTime;
                }
                break;  // its a stop frame and we are waiting
            }
        }

        if (timer >= _currentFrame->DepartureTime && !_triggeredDepartureEvent)
        {
            DoEventIfAny(*_currentFrame, true); // departure event
            _triggeredDepartureEvent = true;
        }

        // not waiting anymore
        SetMoving(true);

        // Enable movement
        if (GetGOInfo()->moTransport.canBeStopped)
            SetGoState(GO_STATE_ACTIVE);

        if (timer >= _currentFrame->DepartureTime && timer < _currentFrame->NextArriveTime)
            break;  // found current waypoint

        MoveToNextWaypoint();

        sScriptMgr->OnRelocate(this, _currentFrame->Node->NodeIndex, _currentFrame->Node->MapId, _currentFrame->Node->LocX, _currentFrame->Node->LocY, _currentFrame->Node->LocZ);

        TC_LOG_DEBUG("entities.transport", "Transport %u (%s) moved to node %u %u %f %f %f", GetEntry(), GetName().c_str(), _currentFrame->Node->NodeIndex, _currentFrame->Node->MapId, _currentFrame->Node->LocX, _currentFrame->Node->LocY, _currentFrame->Node->LocZ);

        // Departure event
        if (_currentFrame->IsTeleportFrame())
            if (TeleportTransport(_nextFrame->Node->MapId, _nextFrame->Node->LocX, _nextFrame->Node->LocY, _nextFrame->Node->LocZ, _nextFrame->InitialOrientation))
                return; // Update more in new map thread
    }

    // Add model to map after we are fully done with moving maps
    if (_delayedAddModel)
    {
        _delayedAddModel = false;
        if (m_model)
            GetMap()->InsertGameObjectModel(*m_model);
    }

    // Set position
    _positionChangeTimer.Update(diff);
    if (_positionChangeTimer.Passed())
    {
        _positionChangeTimer.Reset(positionUpdateDelay);
        if (IsMoving())
        {
            float t = !justStopped ? CalculateSegmentPos(float(timer) * 0.001f) : 1.0f;
            G3D::Vector3 pos, dir;
            _currentFrame->Spline->evaluate_percent(_currentFrame->Index, t, pos);
            _currentFrame->Spline->evaluate_derivative(_currentFrame->Index, t, dir);
            UpdatePosition(pos.x, pos.y, pos.z, std::atan2(dir.y, dir.x) + float(M_PI));
        }
        else if (justStopped)
            UpdatePosition(_currentFrame->Node->LocX, _currentFrame->Node->LocY, _currentFrame->Node->LocZ, _currentFrame->InitialOrientation);
        else
        {
            /* There are four possible scenarios that trigger loading/unloading passengers:
              1. transport moves from inactive to active grid
              2. the grid that transport is currently in becomes active
              3. transport moves from active to inactive grid
              4. the grid that transport is currently in unloads
            */
            bool gridActive = GetMap()->IsGridLoaded(GetPositionX(), GetPositionY());
            if (!gridActive && sWorld->getBoolConfig(CONFIG_TRANSPORT_LOAD_GRIDS))
            {
                GetMap()->LoadGrid(GetPositionX(), GetPositionY());
                gridActive = true;
            }
            if (_staticPassengers.empty() && gridActive) // 2.
                LoadStaticPassengers();
            else if (!_staticPassengers.empty() && !gridActive)
                // 4. - if transports stopped on grid edge, some passengers can remain in active grids
                //      unload all static passengers otherwise passengers won't load correctly when the grid that transport is currently in becomes active
                UnloadStaticPassengers();
        }
    }

    sScriptMgr->OnTransportUpdate(this, diff);
}

void Transport::DelayedUpdate(uint32)
{
    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
        return;

    if (GetKeyFrames().size() <= 1)
        return;

    DelayedTeleportTransport();
}

void Transport::AddPassenger(WorldObject* passenger, bool withAll)
{
    if (!IsInWorld())
        return;

    std::lock_guard<std::recursive_mutex> guard(_passengersLock);  // #231, AC Transport.cpp:351
    if (_passengers.insert(passenger).second)
    {
        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnAddPassenger(this, plr);

        if (withAll)
        {
            // Ported from the mod-playerbots AzerothCore fork: also store the passenger's offset
            // from the transport, the same thing the core does when an object boards one (see
            // Object.cpp, "GetPosition / AddPassenger / CalculatePassengerOffset / relocate").
            // Without it the passenger sits at the transport's origin instead of on its deck.
            if (Transport* t = passenger->GetTransport())  // should never happen
                t->RemovePassenger(passenger, false);

            float x, y, z, o;
            passenger->GetPosition(x, y, z, o);

            passenger->SetTransport(this);
            passenger->m_movementInfo.transport.guid = GetGUID();
            CalculatePassengerOffset(x, y, z, &o);
            passenger->m_movementInfo.transport.pos.Relocate(x, y, z, o);

            if (Unit* unit = passenger->ToUnit())
                unit->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        }
        else
        {
            passenger->SetTransport(this);
            passenger->m_movementInfo.transport.guid = GetGUID();

            if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
                if (Unit* unit = passenger->ToUnit())
                    unit->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);
        }

        TC_LOG_DEBUG("entities.transport", "Object %s boarded transport %s.", passenger->GetName().c_str(), GetName().c_str());
    }
}

void Transport::RemovePassenger(WorldObject* passenger, bool withAll)
{
    std::lock_guard<std::recursive_mutex> guard(_passengersLock);  // #231, AC Transport.cpp:380
    bool erased = false;
    if (_passengerTeleportItr != _passengers.end())
    {
        PassengerSet::iterator itr = _passengers.find(passenger);
        if (itr != _passengers.end())
        {
            if (itr == _passengerTeleportItr)
                ++_passengerTeleportItr;

            _passengers.erase(itr);
            erased = true;
        }
    }
    else
        erased = _passengers.erase(passenger) > 0;

    // withAll == false only detaches from this transport's passenger set; the caller (AddPassenger
    // re-parenting a passenger onto another transport) resets the position itself.
    if (!withAll)
        return;

    if (erased || _staticPassengers.erase(passenger)) // static passenger can remove itself in case of grid unload
    {
        passenger->m_movementInfo.transport.Reset();
        TC_LOG_DEBUG("entities.transport", "Object %s removed from transport %s.", passenger->GetName().c_str(), GetName().c_str());

        if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
            if (Unit* unit = passenger->ToUnit())
                unit->ClearUnitState(UNIT_STATE_IGNORE_PATHFINDING);

        // Needs to be done, otherwise next spline update will put us next to world-space 0,0,0 coordinates, since we're no longer on a transport
        if (Unit* unit = passenger->ToUnit())
            if (unit->movespline->Initialized() && unit->movespline->onTransport && !unit->movespline->IsEnteringTransport())
                unit->StopMoving(); // m_transport is removed later, so UpdateSplinePosition won't screw up position, MOVEMENTFLAG_ONTRANSPORT is removed earlier, so new spline won't stick to transport

        passenger->SetTransport(nullptr);

        if (Player* plr = passenger->ToPlayer())
            sScriptMgr->OnRemovePassenger(this, plr);
    }
}

Creature* Transport::CreateNPCPassenger(uint32 guid, CreatureData const* data)
{
    Map* map = GetMap();
    Creature* creature = new Creature();

    if (!creature->LoadCreatureFromDB(guid, map, false))
    {
        delete creature;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = data->orientation;

    creature->SetTransport(this);
    creature->m_movementInfo.transport.guid = GetGUID();
    creature->m_movementInfo.transport.pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);
    creature->Relocate(x, y, z, o);
    creature->SetHomePosition(creature->GetPositionX(), creature->GetPositionY(), creature->GetPositionZ(), creature->GetOrientation());
    creature->SetTransportHomePosition(creature->m_movementInfo.transport.pos);

    /// @HACK - transport models are not added to map's dynamic LoS calculations
    ///         because the current GameObjectModel cannot be moved without recreating
    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
        creature->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

    if (!creature->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "Creature (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)",
            creature->GetGUID().GetCounter(), creature->GetEntry(), creature->GetPositionX(), creature->GetPositionY());
        delete creature;
        return NULL;
    }

    if (!map->AddToMap(creature))
    {
        delete creature;
        return NULL;
    }

    _staticPassengers.insert(creature);
    sScriptMgr->OnAddCreaturePassenger(this, creature);
    return creature;
}

GameObject* Transport::CreateGOPassenger(uint32 guid, GameObjectData const* data)
{
    Map* map = GetMap();
    GameObject* go = new GameObject();

    if (!go->LoadGameObjectFromDB(guid, map, false))
    {
        delete go;
        return NULL;
    }

    float x = data->posX;
    float y = data->posY;
    float z = data->posZ;
    float o = go->GetOrientation();//data->orientation;

    go->SetTransport(this);
    go->m_movementInfo.transport.guid = GetGUID();
    go->m_movementInfo.transport.pos.Relocate(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);
    go->Relocate(x, y, z, o);
    go->SetWorldRotation(go->GetWorldRotation());

    if (!go->IsPositionValid())
    {
        TC_LOG_ERROR("entities.transport", "GameObject (guidlow %d, entry %d) not created. Suggested coordinates aren't valid (X: %f Y: %f)", go->GetGUID().GetCounter(), go->GetEntry(), go->GetPositionX(), go->GetPositionY());
        delete go;
        return nullptr;
    }

    if (!map->AddToMap(go))
    {
        delete go;
        return nullptr;
    }

    _staticPassengers.insert(go);
    return go;
}

TempSummon* Transport::SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties, uint32 duration, Unit* summoner, uint32 spellId, uint32 vehId)
{
    Map* map = FindMap();
    if (!map)
        return nullptr;

    uint32 mask = UNIT_MASK_SUMMON;
    if (properties)
    {
        switch (properties->Category)
        {
            case SUMMON_CATEGORY_PET:
                mask = UNIT_MASK_GUARDIAN;
                break;
            case SUMMON_CATEGORY_PUPPET:
                mask = UNIT_MASK_PUPPET;
                break;
            case SUMMON_CATEGORY_VEHICLE:
                mask = UNIT_MASK_MINION;
                break;
            case SUMMON_CATEGORY_WILD:
            case SUMMON_CATEGORY_ALLY:
            case SUMMON_CATEGORY_UNK:
            {
                switch (properties->Type)
                {
                    case SUMMON_TYPE_MINION:
                    case SUMMON_TYPE_GUARDIAN:
                    case SUMMON_TYPE_GUARDIAN2:
                        mask = UNIT_MASK_GUARDIAN;
                        break;
                    case SUMMON_TYPE_TOTEM:
                        mask = UNIT_MASK_TOTEM;
                        break;
                    case SUMMON_TYPE_VEHICLE:
                    case SUMMON_TYPE_VEHICLE2:
                        mask = UNIT_MASK_SUMMON;
                        break;
                    case SUMMON_TYPE_MINIPET:
                        mask = UNIT_MASK_MINION;
                        break;
                    default:
                        if (properties->Flags & 512) // Mirror Image, Summon Gargoyle
                            mask = UNIT_MASK_GUARDIAN;
                        break;
                }
                break;
            }
            default:
                return nullptr;
        }
    }

    uint32 phase = PHASEMASK_NORMAL;
    uint32 team = 0;
    if (summoner)
    {
        phase = summoner->GetPhaseMask();
        if (summoner->GetTypeId() == TYPEID_PLAYER)
            team = summoner->ToPlayer()->GetTeam();
    }

    TempSummon* summon = nullptr;
    switch (mask)
    {
        case UNIT_MASK_SUMMON:
            summon = new TempSummon(properties, summoner, false);
            break;
        case UNIT_MASK_GUARDIAN:
            summon = new Guardian(properties, summoner, false);
            break;
        case UNIT_MASK_PUPPET:
            summon = new Puppet(properties, summoner);
            break;
        case UNIT_MASK_TOTEM:
            summon = new Totem(properties, summoner);
            break;
        case UNIT_MASK_MINION:
            summon = new Minion(properties, summoner, false);
            break;
    }

    float x, y, z, o;
    pos.GetPosition(x, y, z, o);
    CalculatePassengerPosition(x, y, z, &o);

    if (!summon->Create(map->GenerateLowGuid<HighGuid::Unit>(), map, phase, entry, vehId, team, x, y, z, o))
    {
        delete summon;
        return nullptr;
    }

    summon->SetUInt32Value(UNIT_FIELD_CREATED_BY_SPELL, spellId);

    summon->SetTransport(this);
    summon->m_movementInfo.transport.guid = GetGUID();
    summon->m_movementInfo.transport.pos.Relocate(pos);
    summon->Relocate(x, y, z, o);
    summon->SetHomePosition(x, y, z, o);
    summon->SetTransportHomePosition(pos);

    /// @HACK - transport models are not added to map's dynamic LoS calculations
    ///         because the current GameObjectModel cannot be moved without recreating
    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
        summon->AddUnitState(UNIT_STATE_IGNORE_PATHFINDING);

    summon->InitStats(duration);

    if (!map->AddToMap<Creature>(summon))
    {
        delete summon;
        return nullptr;
    }

    _staticPassengers.insert(summon);

    summon->InitSummon();
    summon->SetTempSummonType(summonType);

    return summon;
}

void Transport::UpdatePosition(float x, float y, float z, float o)
{
    GetMap()->SetMMapErrorReportEnabled(false);

    bool newActive = GetMap()->IsGridLoaded(x, y);
    if (!newActive && sWorld->getBoolConfig(CONFIG_TRANSPORT_LOAD_GRIDS))
    {
        GetMap()->LoadGrid(x, y);
        newActive = true;
    }
    Cell oldCell(GetPositionX(), GetPositionY());

    Relocate(x, y, z, o);
    SetWorldRotationAngles(o, 0, 0);
    UpdateModelPosition();

    UpdatePassengerPositions(_passengers);

    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
        return;

    /* There are four possible scenarios that trigger loading/unloading passengers:
      1. transport moves from inactive to active grid
      2. the grid that transport is currently in becomes active
      3. transport moves from active to inactive grid
      4. the grid that transport is currently in unloads
    */
    if (_staticPassengers.empty() && newActive) // 1. and 2.
        LoadStaticPassengers();
    else if (!_staticPassengers.empty() && !newActive && oldCell.DiffGrid(Cell(GetPositionX(), GetPositionY()))) // 3.
        UnloadStaticPassengers();
    else
        UpdatePassengerPositions(_staticPassengers);
    // 4. is handed by grid unload

    GetMap()->SetMMapErrorReportEnabled(true);
}

void Transport::LoadStaticPassengers()
{
    if (uint32 mapId = GetGOInfo()->moTransport.mapID)
    {
        CellObjectGuidsMap const& cells = sObjectMgr->GetMapObjectGuids(mapId, GetMap()->GetSpawnMode());
        CellGuidSet::const_iterator guidEnd;
        for (CellObjectGuidsMap::const_iterator cellItr = cells.begin(); cellItr != cells.end(); ++cellItr)
        {
            // Creatures on transport
            guidEnd = cellItr->second.creatures.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.creatures.begin(); guidItr != guidEnd; ++guidItr)
                CreateNPCPassenger(*guidItr, sObjectMgr->GetCreatureData(*guidItr));

            // GameObjects on transport
            guidEnd = cellItr->second.gameobjects.end();
            for (CellGuidSet::const_iterator guidItr = cellItr->second.gameobjects.begin(); guidItr != guidEnd; ++guidItr)
                CreateGOPassenger(*guidItr, sObjectMgr->GetGOData(*guidItr));
        }
    }
}

void Transport::UnloadStaticPassengers()
{
    while (!_staticPassengers.empty())
    {
        WorldObject* obj = *_staticPassengers.begin();
        obj->AddObjectToRemoveList();   // also removes from _staticPassengers
    }
}

void Transport::EnableMovement(bool enabled)
{
    if (GetGoType() == GAMEOBJECT_TYPE_TRANSPORT)
    {
        SetGoState(enabled ? GO_STATE_ACTIVE : GO_STATE_READY);
        return;
    }

    if (!GetGOInfo()->moTransport.canBeStopped)
        return;

    _pendingStop = !enabled;
}

void Transport::MoveToNextWaypoint()
{
    // Clear events flagging
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;

    // Set frames
    _currentFrame = _nextFrame++;
    if (_nextFrame == GetKeyFrames().end())
        _nextFrame = GetKeyFrames().begin();
}

float Transport::CalculateSegmentPos(float now)
{
    KeyFrame const& frame = *_currentFrame;
    const float speed = float(m_goInfo->moTransport.moveSpeed);
    const float accel = float(m_goInfo->moTransport.accelRate);
    float timeSinceStop = frame.TimeFrom + (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float timeUntilStop = frame.TimeTo - (now - (1.0f/IN_MILLISECONDS) * frame.DepartureTime);
    float segmentPos, dist;
    float accelTime = _transportInfo->accelTime;
    float accelDist = _transportInfo->accelDist;
    // calculate from nearest stop, less confusing calculation...
    if (timeSinceStop < timeUntilStop)
    {
        if (timeSinceStop < accelTime)
            dist = 0.5f * accel * timeSinceStop * timeSinceStop;
        else
            dist = accelDist + (timeSinceStop - accelTime) * speed;
        segmentPos = dist - frame.DistSinceStop;
    }
    else
    {
        if (timeUntilStop < accelTime)
            dist = 0.5f * accel * timeUntilStop * timeUntilStop;
        else
            dist = accelDist + (timeUntilStop - accelTime) * speed;
        segmentPos = frame.DistUntilStop - dist;
    }

    return std::max(0.0f, std::min(1.0f, segmentPos / frame.NextDistFromPrev));
}

bool Transport::TeleportTransport(uint32 newMapid, float x, float y, float z, float o)
{
    Map const* oldMap = GetMap();

    if (oldMap->GetId() != newMapid)
    {
        _delayedTeleport = true;
        UnloadStaticPassengers();
        return true;
    }
    else
    {
        // Teleport players, they need to know it
        for (PassengerSet::iterator itr = _passengers.begin(); itr != _passengers.end(); ++itr)
        {
            if ((*itr)->GetTypeId() == TYPEID_PLAYER)
            {
                // will be relocated in UpdatePosition of the vehicle
                if (Unit* veh = (*itr)->ToUnit()->GetVehicleBase())
                    if (veh->GetTransport() == this)
                        continue;

                float destX, destY, destZ, destO;
                (*itr)->m_movementInfo.transport.pos.GetPosition(destX, destY, destZ, destO);
                CalculatePassengerPosition(destX, destY, destZ, &destO);

                (*itr)->ToPlayer()->TeleportTo(newMapid, destX, destY, destZ, destO,
                    TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_NOT_LEAVE_COMBAT | TELE_TO_NOT_UNSUMMON_PET | TELE_TO_TRANSPORT_TELEPORT);
            }
        }

        UpdatePosition(x, y, z, o);
        return false;
    }
}

void Transport::DelayedTeleportTransport()
{
    if (!_delayedTeleport)
        return;

    _delayedTeleport = false;
    _triggeredArrivalEvent = false;
    _triggeredDepartureEvent = false;
    
    Map* newMap = sMapMgr->CreateBaseMap(_nextFrame->Node->MapId);
    GetMap()->RemoveFromMap<Transport>(this, false);
    SetMap(newMap);

    float x = _nextFrame->Node->LocX,
        y = _nextFrame->Node->LocY,
        z = _nextFrame->Node->LocZ,
        o = _nextFrame->InitialOrientation;

    Relocate(x, y, z, o);
    SetWorldRotationAngles(o, 0, 0);

    for (_passengerTeleportItr = _passengers.begin(); _passengerTeleportItr != _passengers.end();)
    {
        WorldObject* obj = (*_passengerTeleportItr++);

        float destX, destY, destZ, destO;
        obj->m_movementInfo.transport.pos.GetPosition(destX, destY, destZ, destO);
        CalculatePassengerPosition(destX, destY, destZ, &destO);

        switch (obj->GetTypeId())
        {
            case TYPEID_UNIT:
                if (Creature* creature = obj->ToCreature())
                {
                    if (creature->IsPet())
                        break;
                    if (creature->IsSummon())
                        creature->ToTempSummon()->UnSummon();
                    else if (creature->IsAIEnabled)
                        creature->AI()->EnterEvadeMode();
                }
                break;
            case TYPEID_PLAYER:
                if (!obj->ToPlayer()->TeleportTo(_nextFrame->Node->MapId, destX, destY, destZ, destO, TELE_TO_NOT_LEAVE_TRANSPORT | TELE_TO_TRANSPORT_TELEPORT))
                    RemovePassenger(obj);
                break;
            case TYPEID_GAMEOBJECT:
                obj->ToGameObject()->Delete();
                break;
            case TYPEID_DYNAMICOBJECT:
                obj->AddObjectToRemoveList();
                break;
            default:
                RemovePassenger(obj);
                break;
        }
    }

    Relocate(x, y, z, o);
    SetWorldRotationAngles(o, 0, 0);
    GetMap()->AddToMap<Transport>(this);
}

void Transport::UpdatePassengerPositions(PassengerSet& passengers) 
{
    for (std::set<WorldObject*>::iterator itr = passengers.begin(); itr != passengers.end(); ++itr)
    {
        WorldObject* passenger = *itr;
        // transport teleported but passenger not yet (can happen for players)
        if (passenger->GetMap() != GetMap())
            continue;

        // if passenger is on vehicle we have to assume the vehicle is also on transport
        // and its the vehicle that will be updating its passengers
        if (Unit* unit = passenger->ToUnit())
            if (unit->GetVehicle())
                continue;

        // Do not use Unit::UpdatePosition here, we don't want to remove auras
        // as if regular movement occurred
        float x, y, z, o;
        passenger->m_movementInfo.transport.pos.GetPosition(x, y, z, o);
        CalculatePassengerPosition(x, y, z, &o);
        switch (passenger->GetTypeId())
        {
            case TYPEID_UNIT:
            {
                Creature* creature = passenger->ToCreature();
                GetMap()->CreatureRelocation(creature, x, y, z, o, false);
                if (_staticPassengers.find(passenger) == _staticPassengers.end())
                    break; // Don't update home position from creatures that were spawned not on transport
                creature->GetTransportHomePosition(x, y, z, o);
                CalculatePassengerPosition(x, y, z, &o);
                creature->SetHomePosition(x, y, z, o);
                break;
            }
            case TYPEID_PLAYER:
                //relocate only passengers in world and skip any player that might be still logging in/teleporting
                if (passenger->IsInWorld())
                    GetMap()->PlayerRelocation(passenger->ToPlayer(), x, y, z, o);
                break;
            case TYPEID_GAMEOBJECT:
                GetMap()->GameObjectRelocation(passenger->ToGameObject(), x, y, z, o, false);
                passenger->ToGameObject()->RelocateStationaryPosition(x, y, z, o);
                break;
            case TYPEID_DYNAMICOBJECT:
                GetMap()->DynamicObjectRelocation(passenger->ToDynObject(), x, y, z, o);
                break;
            default:
                break;
        }

        if (Unit* unit = passenger->ToUnit())
            if (Vehicle* vehicle = unit->GetVehicleKit())
                vehicle->RelocatePassengers();
    }
}



void Transport::DoEventIfAny(KeyFrame const& node, bool departure)
{
    if (uint32 eventid = departure ? node.Node->DepartureEventID : node.Node->ArrivalEventID)
    {
        TC_LOG_DEBUG("maps.script", "Taxi %s event %u of node %u of %s path", departure ? "departure" : "arrival", eventid, node.Node->NodeIndex, GetName().c_str());
        GetMap()->ScriptsStart(sEventScripts, eventid, this, this);
        EventInform(eventid);
    }
}

void Transport::BuildUpdate(UpdateDataMapType& data_map)
{
    Map::PlayerList const& players = GetMap()->GetPlayers();
    if (players.isEmpty())
        return;

    bool const phased = HasPhaseRule();
    for (Map::PlayerList::const_iterator itr = players.begin(); itr != players.end(); ++itr)
    {
        // a phased transport only updates the clients it was created at (TrinityCore master: InSamePhase)
        if (phased && itr->GetSource()->GetTransport() != this && !IsPhaseSeenBy(itr->GetSource()))
            continue;

        BuildFieldsUpdate(itr->GetSource(), data_map);
    }

    ClearUpdateMask(false);
}

bool Transport::CanBeSeenByPhase(WorldObject const* viewer) const
{
    if (!HasPhaseRule())
        return true;

    if (Player const* player = viewer->ToPlayer())
        if (player->IsGameMaster())
            return true;

    bool holdsPhase = false;
    for (uint32 phaseId : _transportInfo->phases)
    {
        if (viewer->IsPhased(phaseId))
        {
            holdsPhase = true;
            break;
        }
    }

    return _transportInfo->phaseInverse ? !holdsPhase : holdsPhase;
}

bool Transport::IsPhaseSeenBy(Player const* player) const
{
    return _phaseSeenBy.find(player->GetGUID()) != _phaseSeenBy.end();
}

void Transport::SetPhaseSeenBy(Player const* player, bool seen)
{
    if (seen)
        _phaseSeenBy.insert(player->GetGUID());
    else
        _phaseSeenBy.erase(player->GetGUID());
}

// TrinityCore master Map::SendUpdateTransportVisibility for one transport: create it at the client when the player
// entered its phase, destroy it when the player left it. Its own riders keep it (they were sent it with themselves).
void Transport::UpdatePhaseVisibilityFor(Player* player)
{
    if (!HasPhaseRule() || !IsInWorld() || !player->IsInWorld() || player->GetMap() != GetMap())
        return;

    bool const seen = IsPhaseSeenBy(player);
    if (player->GetTransport() == this)
    {
        if (!seen)
            SetPhaseSeenBy(player, true);
        return;
    }

    bool const visible = CanBeSeenByPhase(player);
    if (visible == seen)
        return;

    UpdateData data(GetMapId());
    if (visible)
        BuildCreateUpdateBlockForPlayer(&data, player);
    else
        BuildOutOfRangeUpdateBlock(&data);
    SetPhaseSeenBy(player, visible);

    WorldPacket packet;
    data.BuildPacket(&packet);
    player->SendDirectMessage(&packet);
}

// #736 - stop-frame mode for a local (type 11) transport with a pause time. The 5.4.8 create block carries the
// transport's stop frames (Object::BuildMovementUpdate, the 22-bit count + uint32 list after the first FlushBits);
// with them the client moves the transport to the stop frame of GO state 25 and holds it there, arriving at the
// server time in GAMEOBJECT_FIELD_LEVEL, and draws it held while OBJECT_FIELD_DYNAMIC_FLAGS has GO_DYNFLAG_LO_STOPPED.
// Without stop frames (this core until now) it loops the whole TransportAnimation path, whatever the GO state.
// Model: ProjectSkyfire/SkyFire_548 694499de5 + ccba7309a (2026-06-26 / 2026-07-19, "Fix/Stabilize Strand of the
// Ancients transport flow": InitializeLegacyTransport, StartLegacyTransportToFirstStop,
// StopLegacyTransportAtInitialStop) and TrinityCore 6.x GameObject::SetTransportState (PauseTimes, states 24/25),
// trimmed to the one stop frame the SotA boats have and kept opt-in, so every other transport is untouched.
static GOState const TRANSPORT_STOP_FRAME_STATE = GOState(25);  // GO_STATE_TRANSPORT_STOPPED + stop frame 0

// #736 A/B (owner 2026-10-14: "the boats do not move in the client", while the server moves boats and riders - t736 w33
// dock 54-57 s, riders 13/13): worldserver.conf Transport.StopFrameClientEncoding picks the clock the client gets for
// a stop-frame transport. Only the client encoding changes; the server-side progress and passengers never do.
//   0 = as before: the create block's ServerTime is the relative path progress (0 at sea, Object::BuildMovementUpdate)
//       while GAMEOBJECT_FIELD_LEVEL is absolute (getMSTime() + travel), the dynamic field pushes the progress every
//       tick, and GO_DYNFLAG_LO_STOPPED is sent while held.
//   1 = TrinityCore 6.x (sniff-based) for type 11: ServerTime = getMSTime() (TC6 Object.cpp BuildMovementUpdate - a type
//       11 GO is no Transport there, so it writes getMSTime()), GAMEOBJECT_LEVEL = getMSTime() + time to the stop frame
//       (TC6 GameObject::SetTransportState), no per-tick progress push, no dynamic flags for a transport (TC6
//       GameObject::BuildValuesUpdate: dynFlags 0, pathProgress = PathProgress % period). TC6 has no "held at the path
//       start" state: the hold before the launch is sent as an arrival a day away, so the client keeps the boat at sea
//       (with an absolute clock, LEVEL = now in GO state 25 reads "already at the stop frame" = at the dock).
//   2 = mode 1 with the hold sent literally as before (LEVEL = the hold time) - the plain TC6 reading.
// Read at use time (cached 10 s, so a config reload lands between matches); change it between matches only.
// Used by Object.cpp and GameObject.cpp through an extern declaration (no header change).
// #851 (owner 2026-10-16, build 71: "the SotA ship animation is completely frozen"): the default is mode 1, the only
// encoding proven in the client (t736 w51/w52 headless client view: boat sea -> dock in 57-58.8 s, held, riders
// anchored). Mode 0 fails every client row ("client never docked" = a frozen boat). The key lived only in the runtime
// worldserver.conf (never in the .dist or the testserver overrides), so the conf regeneration dropped it and the
// server fell back to the old default 0 (Server.log "[736] boat-launch ... mode=0").
int32 TransportStopFrameClientEncoding()
{
    static std::atomic<int32> s_mode{1};
    static std::atomic<uint32> s_readAt{0};
    static std::atomic<bool> s_read{false};
    uint32 const now = getMSTime();
    if (!s_read.load() || getMSTimeDiff(s_readAt.load(), now) >= 10000)
    {
        s_mode.store(sConfigMgr->GetIntDefault("Transport.StopFrameClientEncoding", 1, true));
        s_readAt.store(now);
        s_read.store(true);
    }

    return s_mode.load();
}

static uint32 const TRANSPORT_STOP_FRAME_HOLD_MS = 24 * HOUR * IN_MILLISECONDS;  // mode 1: "arrives in a day" = held

// #851 elevator encoding (Transport.SotaBoatStopFrames = 2): GO_STATE_TRANSPORT_ACTIVE = "go to / stay at frame 0"
static GOState const TRANSPORT_STOP_FRAME_START_STATE = GOState(24);

void Transport::EnableStopFrames(uint8 mode /*= 1*/)
{
    if (GetGOInfo()->type != GAMEOBJECT_TYPE_TRANSPORT)
        return;

    uint32 const period = GetTransportPeriod();
    uint32 const pause = GetGOInfo()->transport.pause;
    if (period < 2 || !pause)
        return;

    _stopFrames.assign(1, std::min(pause, period - 1));
    _stopFrameMode = mode == 2 ? 2 : 1;
    _stopFrameLogAt = 0;

    uint32 const now = getMSTime();
    m_goValue.Transport.PathProgress = 0;
    _stopFrameStartTime = now;
    _stopFrameEndTime = now;
    _stopFrameStartProgress = 0;
    _stopFrameTargetProgress = 0;       // held at the path start until StartToFirstStopFrame

    if (_stopFrameMode == 2)
    {
        // #851: TC GameObjectType::Transport::OnStateChanged initialisation - state 24 with LEVEL = now + |0 - 0|:
        // already at frame 0 (the sea), so every client (late joiners too: LEVEL stays in the past) draws it held there.
        // The dynamic field is built from PathProgress (0) with no flags - GameObject::BuildValuesUpdate.
        SetByteValue(GAMEOBJECT_BYTES_1, 0, TRANSPORT_STOP_FRAME_START_STATE);
        SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, now);
        return;
    }

    SetByteValue(GAMEOBJECT_BYTES_1, 0, TRANSPORT_STOP_FRAME_STATE);
    // #736 A/B: mode 1 sends the hold as an arrival a day away (see TransportStopFrameClientEncoding)
    SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, TransportStopFrameClientEncoding() == 1 ? now + TRANSPORT_STOP_FRAME_HOLD_MS : now);
    SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, GO_DYNFLAG_LO_STOPPED);
    ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);
}

void Transport::StartToFirstStopFrame(uint32 travelTime)
{
    if (_stopFrames.empty())
        return;

    UpdateStopFrameProgress();

    uint32 const now = getMSTime();
    uint32 const requestedTravel = travelTime;
    // #851 elevator encoding: the client runs the leg at the path's own speed - TC OnStateChanged timeToStop =
    // |pathProgress - stopFrame| - so the server leg (and the riders) take exactly that long too
    if (_stopFrameMode == 2)
    {
        int64 const span = int64(_stopFrames.front()) - int64(m_goValue.Transport.PathProgress);
        travelTime = uint32(span < 0 ? -span : span);
    }

    _stopFrameStartTime = now;
    _stopFrameEndTime = now + travelTime;
    _stopFrameStartProgress = m_goValue.Transport.PathProgress;
    _stopFrameTargetProgress = _stopFrames.front();
    if (!travelTime)
        m_goValue.Transport.PathProgress = _stopFrameTargetProgress;

    if (_stopFrameMode == 2)
    {
        // #851: state 24 -> 25 and the arrival time in the same values update (TC SetGoState + SetLevel). No dynamic
        // field push: the source of the leg is the client's own drawn progress (frame 0), as TC (the progress there is
        // only written silently, for later create blocks).
        SetByteValue(GAMEOBJECT_BYTES_1, 0, TRANSPORT_STOP_FRAME_STATE);
        SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, _stopFrameEndTime);
        _stopFrameLogAt = 0;
        TC_LOG_DEBUG("entities.gameobject", "[736] boat-launch entry=%u level=%u servertime=%u mode=elevator travel=%u requested=%u progress=%u->%u now=%u state=%u",
            GetEntry(), GetUInt32Value(GAMEOBJECT_FIELD_LEVEL), now, travelTime, requestedTravel,
            _stopFrameStartProgress, _stopFrameTargetProgress, now, uint32(GetGoState()));
        return;
    }

    SetByteValue(GAMEOBJECT_BYTES_1, 0, TRANSPORT_STOP_FRAME_STATE);
    SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, _stopFrameEndTime);
    if (travelTime)
        RemoveFlag(OBJECT_FIELD_DYNAMIC_FLAGS, GO_DYNFLAG_LO_STOPPED);
    else
        SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, GO_DYNFLAG_LO_STOPPED);
    ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);

    // #736 A/B evidence: the clocks the client gets for this launch (servertime = the create block's ServerTime now)
    int32 const mode = TransportStopFrameClientEncoding();
    TC_LOG_DEBUG("entities.gameobject", "[736] boat-launch entry=%u level=%u servertime=%u mode=%d travel=%u progress=%u->%u now=%u",
        GetEntry(), GetUInt32Value(GAMEOBJECT_FIELD_LEVEL), mode ? now : m_goValue.Transport.PathProgress, mode, travelTime,
        _stopFrameStartProgress, _stopFrameTargetProgress, now);
}

void Transport::UpdateStopFrameProgress()
{
    uint32 const now = getMSTime();
    uint32 const travelTime = getMSTimeDiff(_stopFrameStartTime, _stopFrameEndTime);
    uint32 const elapsed = getMSTimeDiff(_stopFrameStartTime, now);

    if (!travelTime || elapsed >= travelTime)
    {
        m_goValue.Transport.PathProgress = _stopFrameTargetProgress;
        if (_stopFrameMode == 2)    // #851 elevator encoding: the client holds by itself once LEVEL has passed
            return;
        if (!HasFlag(OBJECT_FIELD_DYNAMIC_FLAGS, GO_DYNFLAG_LO_STOPPED))
        {
            SetFlag(OBJECT_FIELD_DYNAMIC_FLAGS, GO_DYNFLAG_LO_STOPPED);   // arrived: the client draws it held
            ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);
        }
        return;
    }

    // linear in time between the leg's start and target progress
    int64 const from = int64(_stopFrameStartProgress);
    int64 const to = int64(_stopFrameTargetProgress);
    uint32 const progress = uint32(from + (to - from) * int64(elapsed) / int64(travelTime));
    if (progress == m_goValue.Transport.PathProgress)
        return;

    m_goValue.Transport.PathProgress = progress;
    // #736 A/B: modes 1/2 (TC6) send no per-tick progress - the client runs the leg from GAMEOBJECT_FIELD_LEVEL
    // #851: neither does the elevator encoding (Transport.SotaBoatStopFrames = 2)
    if (_stopFrameMode == 2 || TransportStopFrameClientEncoding() != 0)
        return;

    // #736 (owner 2026-10-08, build 21: "the attackers stand on a non-moving ship, the ship is at the dock after the
    // round starts and never moves"): the client draws a stop-frame transport at the path progress of the dynamic
    // field (GameObject::BuildValuesUpdate, OBJECT_FIELD_DYNAMIC_FLAGS high half), so that field has to follow the
    // leg. Before, it was only sent at the launch (progress 0) and at the arrival (the stop frame): the boat sat at
    // sea for the whole warm-up and then jumped to the dock. SkyFire_548 GameObject::UpdateLegacyTransportPathProgress
    // -> MarkLegacyTransportPathProgressChanged does the same on every progress change.
    ForceValuesUpdateAtIndex(OBJECT_FIELD_DYNAMIC_FLAGS);
}

// #851 (owner 2026-10-20: "You should be able to monitor where the ships are at all times"): one line per stop-frame
// transport (the SotA boats) every 1 s while it sails and for 10 s after each leg ends, every 5 s while it holds -
// phase hold (at frame 0, the sea) / sail / dock (at the stop frame). Drivers locate every rider against these.
void Transport::LogStopFramePosition()
{
    uint32 const now = getMSTime();
    uint32 const travel = getMSTimeDiff(_stopFrameStartTime, _stopFrameEndTime);
    bool const sailing = travel && getMSTimeDiff(_stopFrameStartTime, now) < travel;
    bool const justEnded = !sailing && getMSTimeDiff(_stopFrameEndTime, now) < 10 * IN_MILLISECONDS;
    uint32 const interval = (sailing || justEnded) ? IN_MILLISECONDS : 5 * IN_MILLISECONDS;
    if (_stopFrameLogAt && getMSTimeDiff(_stopFrameLogAt, now) < interval)
        return;

    _stopFrameLogAt = now ? now : 1;
    size_t riders = 0;
    {
        std::lock_guard<std::recursive_mutex> guard(_passengersLock);
        riders = _passengers.size();
    }

    TC_LOG_DEBUG("entities.gameobject", "[851] boat-pos entry=%u guid=%u inst=%u mode=%s phase=%s progress=%u/%u state=%u level=%u now=%u x=%.2f y=%.2f z=%.2f o=%.3f riders=%u",
        GetEntry(), GetGUID().GetCounter(), GetMap() ? GetMap()->GetInstanceId() : 0u, _stopFrameMode == 2 ? "elevator" : "stopframe",
        sailing ? "sail" : (_stopFrameTargetProgress ? "dock" : "hold"), m_goValue.Transport.PathProgress, _stopFrames.front(),
        uint32(GetGoState()), GetUInt32Value(GAMEOBJECT_FIELD_LEVEL), now, GetPositionX(), GetPositionY(), GetPositionZ(),
        GetOrientation(), uint32(riders));
}
