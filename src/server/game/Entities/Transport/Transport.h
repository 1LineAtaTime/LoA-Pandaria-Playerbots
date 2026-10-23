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

#ifndef TRANSPORTS_H
#define TRANSPORTS_H

#include "GameObject.h"
#include "TransportMgr.h"
#include "VehicleDefines.h"
#include <mutex>

struct CreatureData;

class Transport : public GameObject, public TransportBase
{
        friend Transport* TransportMgr::CreateLocalTransport(uint32, Map*);
        friend Transport* TransportMgr::CreateLocalTransport(uint32, Map*, float, float, float, float, uint32);
        friend Transport* TransportMgr::CreateTransport(uint32, uint32, Map*);

        Transport();
    public:
        typedef std::set<WorldObject*> PassengerSet;

        ~Transport();

        bool CreateLocal(uint32 guidlow, uint32 entry, Map* map, float x, float y, float z, float ang, uint32 animprogress);
        bool Create(uint32 guidlow, uint32 entry, uint32 mapid, float x, float y, float z, float ang, uint32 animprogress);
        void CleanupsBeforeDelete(bool finalCleanup = true) override;

        void Update(uint32 diff) override;
        void DelayedUpdate(uint32 diff);
        
        void BuildUpdate(UpdateDataMapType& data_map) override;

    /// withAll also places the passenger on the deck (stores its offset from the transport), which
    /// is what a bot needs when it walks onto a boat/zeppelin. Ported from the mod-playerbots
    /// AzerothCore fork (issue #86).
    void AddPassenger(WorldObject* passenger, bool withAll = false);
    void RemovePassenger(WorldObject* passenger, bool withAll = true);
        PassengerSet const& GetPassengers() const { return _passengers; }
        PassengerSet const& GetStaticPassengers() const { return _staticPassengers; }

        Creature* CreateNPCPassenger(uint32 guid, CreatureData const* data);
        GameObject* CreateGOPassenger(uint32 guid, GameObjectData const* data);

        /**
         * @fn bool Transport::SummonPassenger(uint64, Position const&, TempSummonType, SummonPropertiesEntry const*, uint32, Unit*, uint32, uint32)
         *
         * @brief Temporarily summons a creature as passenger on this transport.
         *
         * @param entry Id of the creature from creature_template table
         * @param pos Initial position of the creature (transport offsets)
         * @param summonType
         * @param properties
         * @param duration Determines how long the creauture will exist in world depending on @summonType (in milliseconds)
         * @param summoner Summoner of the creature (for AI purposes)
         * @param spellId
         * @param vehId If set, this value overrides vehicle id from creature_template that the creature will use
         *
         * @return Summoned creature.
         */
        TempSummon* SummonPassenger(uint32 entry, Position const& pos, TempSummonType summonType, SummonPropertiesEntry const* properties = nullptr, uint32 duration = 0, Unit* summoner = nullptr, uint32 spellId = 0, uint32 vehId = 0);


        /// This method transforms supplied transport offsets into global coordinates
        void CalculatePassengerPosition(float& x, float& y, float& z, float* o = nullptr) const override
        {
            TransportBase::CalculatePassengerPosition(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        /// This method transforms supplied global coordinates into local offsets
        void CalculatePassengerOffset(float& x, float& y, float& z, float* o = nullptr) const override
        {
            TransportBase::CalculatePassengerOffset(x, y, z, o, GetPositionX(), GetPositionY(), GetPositionZ(), GetOrientation());
        }

        uint32 GetTransportPeriod() const override { return GetGOInfo()->type == GAMEOBJECT_TYPE_MO_TRANSPORT ? GetUInt32Value(GAMEOBJECT_FIELD_LEVEL) : GameObject::GetTransportPeriod(); }
        void SetPeriod(uint32 period) { SetUInt32Value(GAMEOBJECT_FIELD_LEVEL, period); }
        uint32 GetTimer() const { return GetGOValue()->Transport.PathProgress; }

        KeyFrameVec const& GetKeyFrames() const { return _transportInfo->keyFrames; }

        void UpdatePosition(float x, float y, float z, float o);

        //! Needed when transport moves from inactive to active grid
        void LoadStaticPassengers();

        //! Needed when transport enters inactive grid
        void UnloadStaticPassengers();

        void EnableMovement(bool enabled);

        void SetDelayedAddModelToMap() { _delayedAddModel = true; }

        TransportTemplate const* GetTransportTemplate() const { return _transportInfo; }

        // #736: stop-frame mode for a local (type 11) transport with a pause time - the Strand of the Ancients boats.
        // The 5.4.8 client only stops such a transport when the create block lists its stop frames; without them it
        // loops the whole TransportAnimation path (ocean -> dock -> jump back), which is what the boats did.
        // #851: mode = Transport.SotaBoatStopFrames: 1 = #736 encoding (GO state 25 throughout, rollback only),
        // 2 = elevator encoding (TrinityCore type-11 stop-frame states: 24 held at frame 0, 25 to/at stop frame 0)
        void EnableStopFrames(uint8 mode = 1);          // hold at the path start (progress 0)
        void StartToFirstStopFrame(uint32 travelTime);  // sail to the first stop frame in travelTime ms, then hold there
        std::vector<uint32> const* GetStopFrames() const { return _stopFrames.empty() ? nullptr : &_stopFrames; }
        uint8 GetStopFrameMode() const { return _stopFrames.empty() ? 0 : _stopFrameMode; }  // 0 = no stop frames

    private:
        void UpdateStopFrameProgress();
        void LogStopFramePosition();                    // #851 "[851] boat-pos" (1 s while sailing, 5 s while held)

        void MoveToNextWaypoint();
        float CalculateSegmentPos(float perc);
        bool TeleportTransport(uint32 newMapid, float x, float y, float z, float o);
        void DelayedTeleportTransport();
        void UpdatePassengerPositions(PassengerSet& passengers);
        void DoEventIfAny(KeyFrame const& node, bool departure);

        //! Helpers to know if stop frame was reached
        bool IsMoving() const { return _isMoving; }
        void SetMoving(bool val) { _isMoving = val; }

        TransportTemplate const* _transportInfo;

        KeyFrameVec::const_iterator _currentFrame;
        KeyFrameVec::const_iterator _nextFrame;

        TimeTrackerSmall _positionChangeTimer;
        bool _isMoving;
        bool _pendingStop;

        //! These are needed to properly control events triggering only once for each frame
        bool _triggeredArrivalEvent;
        bool _triggeredDepartureEvent;

        PassengerSet _passengers;
        PassengerSet::iterator _passengerTeleportItr;
        PassengerSet _staticPassengers;
        // #231: AC Transport.h:111 (`mutable std::mutex Lock`), taken by AddPassenger / RemovePassenger
        // (AC Transport.cpp:351,380) so boarding from another map-update thread (MapUpdate.Threads > 1)
        // cannot race the set. Recursive here: MoP's RemovePassenger runs scripts and StopMoving().
        mutable std::recursive_mutex _passengersLock;

        bool _delayedAddModel;
        bool _delayedTeleport;

        // #736 stop-frame mode (empty = the transport behaves as before)
        std::vector<uint32> _stopFrames;
        uint32 _stopFrameStartTime;
        uint32 _stopFrameEndTime;
        uint32 _stopFrameStartProgress;
        uint32 _stopFrameTargetProgress;
        uint8 _stopFrameMode;                           // #851: 1 = #736 encoding, 2 = elevator encoding
        uint32 _stopFrameLogAt;                         // #851: getMSTime() of the last "[851] boat-pos" line
};

#endif
