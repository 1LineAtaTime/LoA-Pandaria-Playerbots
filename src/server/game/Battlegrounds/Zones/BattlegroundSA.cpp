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

#include "BattlegroundSA.h"
#include "Language.h"
#include "Player.h"
#include "GameObject.h"
#include "ObjectMgr.h"
#include "WorldPacket.h"
#include "WorldSession.h"
#include "Chat.h"
#include "Transport.h"

// #851: defined in Transport.cpp (Transport.SotaBoatHoldSeconds, no header change)
uint32 TransportSotaBoatHoldMs();

// Achievements
enum
{
    WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_A = 6450,
    WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_H = 6451,
    WORLD_STATE_NOT_EVEN_A_SCRATCH_A      = 6449,
    WORLD_STATE_NOT_EVEN_A_SCRATCH_H      = 6448,
    WORLD_STATE_STORM_THE_BEACH_A         = 5668,
    WORLD_STATE_STORM_THE_BEACH_H         = 5669,
};

BattlegroundSA::BattlegroundSA()
{
    // #736 (owner 2026-10-08): the 2 min preparation of AC (Battleground.cpp:199-202 defaults 2M/1M/30S/NONE, :515-517
    // "Strand of the Ancients - always use 120 seconds due to boat timing mechanics"); was a 1 min "temp hack for ships".
    // The messages below already said 2 min / 1 min / 30 s.
    StartDelayTimes[BG_STARTING_EVENT_FIRST]  = BG_START_DELAY_2M;
    StartDelayTimes[BG_STARTING_EVENT_SECOND] = BG_START_DELAY_1M;
    StartDelayTimes[BG_STARTING_EVENT_THIRD]  = BG_START_DELAY_30S;
    StartDelayTimes[BG_STARTING_EVENT_FOURTH] = BG_START_DELAY_NONE;

    StartMessageIds[BG_STARTING_EVENT_FIRST]  = LANG_BG_SA_START_TWO_MINUTES;
    StartMessageIds[BG_STARTING_EVENT_SECOND] = LANG_BG_SA_START_ONE_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_THIRD]  = LANG_BG_SA_START_HALF_MINUTE;
    StartMessageIds[BG_STARTING_EVENT_FOURTH] = LANG_BG_SA_HAS_BEGUN;
    BgObjects.resize(BG_SA_MAXOBJ);
    BgCreatures.resize(BG_SA_MAXNPC + BG_SA_MAX_GY);
    TimerEnabled = false;
    UpdateWaitTimer = 0;
    EndTimestamp = 0;
    SignaledRoundTwo = false;
    SignaledRoundTwoHalfMin = false;
    InitSecondRound = false;
    _boatHoldMs = BG_SA_BOAT_START;
    _boatDockedAt[0] = _boatDockedAt[1] = 0;
    _boatSafetyDone[0] = _boatSafetyDone[1] = false;

    //! This is here to prevent an uninitialised variable warning
    //! The warning only occurs when SetUpBattleGround fails though.
    //! In the future this function should be called BEFORE sending initial worldstates.
    memset(&GraveyardStatus, 0, sizeof(GraveyardStatus));
}

BattlegroundSA::~BattlegroundSA() { }

void BattlegroundSA::Reset()
{
    TotalTime = 0;
    Attackers = ((urand(0, 1)) ? TEAM_ALLIANCE : TEAM_HORDE);
    for (uint8 i = 0; i <= 5; i++)
        GateStatus[i] = BG_SA_GATE_OK;
    ShipsStarted = false;
    ShipsLaunched = false;
    _nextShipIsEast = true;
    _boatInitAt.clear();
    _boatDockedAt[0] = _boatDockedAt[1] = 0;
    _boatSafetyDone[0] = _boatSafetyDone[1] = false;
    gateDestroyed = false;
    _allVehiclesAlive[TEAM_ALLIANCE] = true;
    _allVehiclesAlive[TEAM_HORDE] = true;
    Status = BG_SA_WARMUP;
}

bool BattlegroundSA::SetupBattleground()
{
    return ResetObjs();
}

bool BattlegroundSA::ResetObjs()
{
    for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
            SendTransportsRemove(player);

    uint32 atF = BG_SA_Factions[Attackers];
    uint32 defF = BG_SA_Factions[Attackers ? TEAM_ALLIANCE : TEAM_HORDE];

    for (uint8 i = 0; i < BG_SA_MAXOBJ; i++)
        DelObject(i);

    for (uint8 i = 0; i < BG_SA_MAXNPC; i++)
        DelCreature(i);

    for (uint8 i = BG_SA_MAXNPC; i < BG_SA_MAXNPC + BG_SA_MAX_GY; i++)
        DelCreature(i);

    for (uint8 i = 0; i < 6; i++)
        GateStatus[i] = BG_SA_GATE_OK;

    for (uint8 i = 0; i <= BG_SA_PORTAL_DEFFENDER_RED; i++)
    {
        if (!AddObject(i, BG_SA_ObjEntries[i], BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1], BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY))
            return false;
    }

    for (uint8 i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; i++)
    {
        uint32 boatid = 0;
        switch (i)
        {
            case BG_SA_BOAT_ONE:
                boatid = Attackers ? BG_SA_BOAT_ONE_H : BG_SA_BOAT_ONE_A;
                break;
            case BG_SA_BOAT_TWO:
                boatid = Attackers ? BG_SA_BOAT_TWO_H : BG_SA_BOAT_TWO_A;
                break;
            default:
                break;
        }
        if (!AddObject(i, boatid, BG_SA_ObjSpawnlocs[i][0],
          BG_SA_ObjSpawnlocs[i][1],
          BG_SA_ObjSpawnlocs[i][2]+(Attackers ? -3.750f: 0),
          BG_SA_ObjSpawnlocs[i][3], 0, 0, 0, 0, RESPAWN_ONE_DAY))
            return false;
    }

    for (uint8 i = BG_SA_SIGIL_1; i <= BG_SA_LEFT_FLAGPOLE; i++)
    {
        if (!AddObject(i, BG_SA_ObjEntries[i],
            BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1],
            BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3],
            0, 0, 0, 0, RESPAWN_ONE_DAY))
        return false;
    }

    // MAD props for Kiper for discovering those values - 4 hours of his work.
    GetBGObject(BG_SA_BOAT_ONE)->SetParentRotation({ 0.0f, 0.0f, 1.0f, 0.0002f });
    GetBGObject(BG_SA_BOAT_TWO)->SetParentRotation({ 0.0f, 0.0f, 1.0f, 0.00001f });

    // #851 evidence (t736 driver): what the clients get for each boat - MoP 5.4.8 encoding: stopframes=0 state=1 (READY)
    // level=60133 progress=period-1; #736 stop-frame mode: stopframes=1 state=25; elevator mode (Transport.
    // SotaBoatStopFrames = 2): stopframes=1 state=24 level=<now> progress=0, held at sea for hold= ms (round 1)
    _boatHoldMs = std::min<uint32>(TransportSotaBoatHoldMs(), BG_SA_WARMUPLENGTH);
    _boatDockedAt[0] = _boatDockedAt[1] = 0;
    _boatSafetyDone[0] = _boatSafetyDone[1] = false;
    for (uint8 i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; ++i)
        if (GameObject* boat = GetBGObject(i))
        {
            Transport* transport = boat->ToTransport();
            std::vector<uint32> const* stops = transport ? transport->GetStopFrames() : nullptr;
            uint8 const mode = transport ? transport->GetStopFrameMode() : 0;
            bool const round2 = Status == BG_SA_SECOND_WARMUP;
            TC_LOG_DEBUG("entities.gameobject", "[851] boat-spawn entry=%u guid=%u mode=%s stopframes=%u state=%u level=%u progress=%u round=%u hold=%u now=%u",
                boat->GetEntry(), boat->GetGUID().GetCounter(), mode == 2 ? "elevator" : (stops ? "stopframe" : "default"),
                stops ? uint32(stops->size()) : 0u, uint32(boat->GetGoState()), boat->GetUInt32Value(GAMEOBJECT_FIELD_LEVEL),
                boat->GetGOValue()->Transport.PathProgress, round2 ? 2u : 1u, mode == 2 ? (round2 ? 0u : _boatHoldMs) : uint32(BG_SA_BOAT_START),
                getMSTime());
        }

    // #736: the boats wait at their ocean spot (stop-frame mode) until LaunchShips, instead of the client looping
    // their TransportAnimation path (ocean -> dock -> back to the ocean) for the whole battle. The stop frames are set
    // in Transport::CreateLocal (with the path rotation above): AddObject's Map::AddToMap already sent the create
    // block to the players on the map, so enabling them here was too late for those players (the round-2 loop).

    //Cannons and demolishers - NPCs are spawned
    //By capturing GYs.
    for (uint8 i = 0; i < BG_SA_DEMOLISHER_5; i++)
    {
        if (!AddCreature(BG_SA_NpcEntries[i], i, (Attackers == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE),
              BG_SA_NpcSpawnlocs[i][0], BG_SA_NpcSpawnlocs[i][1],
              BG_SA_NpcSpawnlocs[i][2], BG_SA_NpcSpawnlocs[i][3], i >= BG_SA_GUN_1 && i <= BG_SA_GUN_10 ? 1 : 600))
        return false;
    }

    // Hack to make cannons not miss because for some reason SPELL_ATTR3_IGNORE_HIT_RESULT is handled after miss chance handling for SPELL_DAMAGE_CLASS_MAGIC spells
    for (uint32 i = BG_SA_GUN_1; i <= BG_SA_GUN_10; ++i)
        if (Creature* cannon = GetBGCreature(i))
            cannon->m_modSpellHitChance = 1000.0f;

    OverrideGunFaction();
    DemolisherStartState(true);

    for (uint8 i = 0; i <= BG_SA_PORTAL_DEFFENDER_RED; i++)
    {
        SpawnBGObject(i, RESPAWN_IMMEDIATELY);
        GetBGObject(i)->SetFaction(defF);
    }

    GetBGObject(BG_SA_TITAN_RELIC)->SetFaction(atF);
    GetBGObject(BG_SA_TITAN_RELIC)->Refresh();

    for (uint8 i = 0; i <= 5; i++)
        GateStatus[i] = BG_SA_GATE_OK;

    TotalTime = 0;
    ShipsStarted = false;
    ShipsLaunched = false;
    _nextShipIsEast = true;

    //Graveyards
    for (uint8 i = 0; i < BG_SA_MAX_GY; i++)
    {
        WorldSafeLocsEntry const* sg = NULL;
        sg = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);

        if (!sg)
        {
            TC_LOG_ERROR("bg.battleground", "SOTA: Can't find GY entry %u", BG_SA_GYEntries[i]);
            return false;
        }

        if (i == BG_SA_BEACH_GY)
        {
            GraveyardStatus[i] = Attackers;
            AddSpiritGuide(i + BG_SA_MAXNPC, sg->x, sg->y, sg->z, BG_SA_GYOrientation[i], ((Attackers == TEAM_HORDE)? HORDE : ALLIANCE));
        }
        else
        {
            GraveyardStatus[i] = ((Attackers == TEAM_HORDE)? TEAM_ALLIANCE : TEAM_HORDE);
            if (!AddSpiritGuide(i + BG_SA_MAXNPC, sg->x, sg->y, sg->z, BG_SA_GYOrientation[i], ((Attackers == TEAM_HORDE)? ALLIANCE : HORDE)))
                TC_LOG_ERROR("bg.battleground", "SOTA: couldn't spawn GY: %u", i);
        }
    }

    //GY capture points
    for (uint8 i = BG_SA_CENTRAL_FLAG; i <= BG_SA_LEFT_FLAG; i++)
    {
        AddObject(i, (BG_SA_ObjEntries[i] - (Attackers == TEAM_ALLIANCE ? 1 : 0)),
            BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1],
            BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3],
            0, 0, 0, 0, RESPAWN_ONE_DAY);
        GetBGObject(i)->SetFaction(atF);
    }

    UpdateBombSpawns();

    //Player may enter BEFORE we set up BG - lets update his worldstates anyway...
    UpdateWorldState(BG_SA_RIGHT_GY_HORDE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    UpdateWorldState(BG_SA_LEFT_GY_HORDE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    UpdateWorldState(BG_SA_CENTER_GY_HORDE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);

    UpdateWorldState(BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    UpdateWorldState(BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    UpdateWorldState(BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);

    if (Attackers == TEAM_ALLIANCE)
    {
        UpdateWorldState(BG_SA_ALLY_ATTACKS, 1);
        UpdateWorldState(BG_SA_HORDE_ATTACKS, 0);

        UpdateWorldState(BG_SA_RIGHT_ATT_TOKEN_ALL, 1);
        UpdateWorldState(BG_SA_LEFT_ATT_TOKEN_ALL, 1);
        UpdateWorldState(BG_SA_RIGHT_ATT_TOKEN_HRD, 0);
        UpdateWorldState(BG_SA_LEFT_ATT_TOKEN_HRD, 0);

        UpdateWorldState(BG_SA_HORDE_DEFENCE_TOKEN, 1);
        UpdateWorldState(BG_SA_ALLIANCE_DEFENCE_TOKEN, 0);
    }
    else
    {
        UpdateWorldState(BG_SA_HORDE_ATTACKS, 1);
        UpdateWorldState(BG_SA_ALLY_ATTACKS, 0);

        UpdateWorldState(BG_SA_RIGHT_ATT_TOKEN_ALL, 0);
        UpdateWorldState(BG_SA_LEFT_ATT_TOKEN_ALL, 0);
        UpdateWorldState(BG_SA_RIGHT_ATT_TOKEN_HRD, 1);
        UpdateWorldState(BG_SA_LEFT_ATT_TOKEN_HRD, 1);

        UpdateWorldState(BG_SA_HORDE_DEFENCE_TOKEN, 0);
        UpdateWorldState(BG_SA_ALLIANCE_DEFENCE_TOKEN, 1);
    }

    UpdateWorldState(BG_SA_PURPLE_GATEWS, 1);
    UpdateWorldState(BG_SA_RED_GATEWS, 1);
    UpdateWorldState(BG_SA_BLUE_GATEWS, 1);
    UpdateWorldState(BG_SA_GREEN_GATEWS, 1);
    UpdateWorldState(BG_SA_YELLOW_GATEWS, 1);
    UpdateWorldState(BG_SA_ANCIENT_GATEWS, 1);

    for (int i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; i++)
        for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
            if (Player* player = ObjectAccessor::FindPlayer(itr->first))
                SendTransportInit(player);

    // set status manually so preparation is cast correctly in 2nd round too
    SetStatus(STATUS_WAIT_JOIN);

    TeleportPlayers();
    return true;
}

void BattlegroundSA::StartShips()
{
    if (ShipsStarted)
        return;

    // #736: AC StartShips (BattlegroundSA.cpp:297-317) = DoorOpen on both boats (StaticTransport sails to pauseAtTime
    // and holds there) + a values update to every player. Here: the stop-frame voyage (LaunchShips), timed to end as
    // the warm-up ends - round 1 at 1:00 of 2:00 (~60 s, the TransportAnimation voyage is 60.1 s), round 2 at once.
    // #851: MoP 5.4.8 boats (the default) - the original StartShips only sets ShipsStarted: no DoorOpen, no values update. The client already sails each boat from its
    // create block; LandBoatRiders puts the riders on the dock.
    // #851 elevator boats (Transport.SotaBoatStopFrames = 2): Transport::StartToFirstStopFrame ignores the travel time
    // asked for here and sails at the path's own speed (60133 ms, as the client draws it).
    if (BoatsUseStopFrames())
    {
        uint32 const warmup = Status == BG_SA_SECOND_WARMUP ? uint32(BG_SA_SECOND_WARMUPLENGTH) : uint32(BG_SA_WARMUPLENGTH);
        LaunchShips(TotalTime < warmup ? warmup - TotalTime : 0);
    }
    ShipsStarted = true;
}

// #851: the boats were created in #736 stop-frame mode (Transport::CreateLocal, Transport.SotaBoatStopFrames = 1)
bool BattlegroundSA::BoatsUseStopFrames()
{
    for (uint8 i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; ++i)
        if (GameObject* boat = GetBGObject(i))
            if (Transport* transport = boat->ToTransport())
                if (transport->GetStopFrames())
                    return true;

    return false;
}

// #851: the stop-frame mode the boats were created with (Transport::GetStopFrameMode: 0 MoP 5.4.8, 1 #736, 2 elevator)
uint8 BattlegroundSA::BoatsStopFrameMode()
{
    for (uint8 i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; ++i)
        if (GameObject* boat = GetBGObject(i))
            if (Transport* transport = boat->ToTransport())
                if (uint8 mode = transport->GetStopFrameMode())
                    return mode;

    return 0;
}

// #851 elevator boats: the boat docks for real (server and client), so the riders walk off by themselves (players;
// bots: BgTacticsSA BoatStep). Safety net only: an attacker still aboard BG_SA_BOAT_DOCKED_GRACE_MS after its boat
// reached the dock frame is put on that dock once (the MoP dock spots); "[851] boat-safety" shows how many.
void BattlegroundSA::LandDockedBoatRiders()
{
    uint32 const now = getMSTime();
    for (uint8 i = 0; i < 2; ++i)
    {
        if (_boatSafetyDone[i] || BgObjects[BG_SA_BOAT_ONE + i].IsEmpty())
            continue;

        GameObject* boat = GetBGObject(BG_SA_BOAT_ONE + i);
        Transport* transport = boat ? boat->ToTransport() : nullptr;
        std::vector<uint32> const* stops = transport ? transport->GetStopFrames() : nullptr;
        if (!stops || boat->GetGOValue()->Transport.PathProgress < stops->front())
            continue;

        if (!_boatDockedAt[i])
        {
            _boatDockedAt[i] = now ? now : 1;
            continue;
        }

        uint32 const docked = getMSTimeDiff(_boatDockedAt[i], now);
        if (docked < BG_SA_BOAT_DOCKED_GRACE_MS)
            continue;

        _boatSafetyDone[i] = true;
        uint32 landed = 0;
        for (auto&& itr : GetPlayers())
        {
            Player* player = ObjectAccessor::FindPlayer(itr.first);
            if (!player || !player->IsInWorld() || player->GetMapId() != GetMapId() || player->GetBGTeamId() != Attackers ||
                !player->IsAlive() || player->GetTransport() != transport)
                continue;

            // w75: real players (and headless clients) walk off a docked boat themselves; only stuck bots are landed.
            if (!player->GetSession() || !player->GetSession()->IsBot())
                continue;

            if (player->GetPositionY() < 0) // east dock
                player->TeleportTo(607, 1598.571f, -106.671f, 8.873f, 4.1359f, 0);
            else // west dock
                player->TeleportTo(607, 1607.596f, 49.766f, 7.5795f, 2.31865f, 0);
            ++landed;
            TC_LOG_DEBUG("entities.gameobject", "[851] boat-safety-land player=%s entry=%u docked_for=%u status=%u",
                player->GetName().c_str(), boat->GetEntry(), docked, uint32(Status));
        }

        TC_LOG_DEBUG("entities.gameobject", "[851] boat-safety entry=%u docked_for=%u landed=%u status=%u",
            boat->GetEntry(), docked, landed, uint32(Status));
    }
}

// #851 (owner 2026-10-20: "the 2 boats/ships are not even sailing anymore to the deck"): MoP 5.4.8 boats. The client sails
// the boat from the sea from its create block and draws it at the dock ~60 s later (then loops back to the sea); the
// server boat never moves. So, as in MoP 5.4.8 (round 1 TotalTime 59-60 s, round 2 53-60 s:
// every attacker on a transport or in the water -> TeleportTo(607, dock), the two spots below), each
// attacker still on a boat or out at sea is put on the dock BG_SA_BOAT_LANDING_MS after the create block it got -
// per rider instead of one round clock, because SendTransportInit restarts that rider's client boat when
// it joins. Bots ride the same clock (their BoatStep holds aboard until then; the server boat never "docks").
void BattlegroundSA::LandBoatRiders()
{
    if (BoatsStopFrameMode() == 2)
    {
        LandDockedBoatRiders();     // #851 elevator boats: safety net only
        return;
    }

    if (_boatInitAt.empty() || BoatsUseStopFrames())
        return;

    uint32 const now = getMSTime();
    for (auto itr = _boatInitAt.begin(); itr != _boatInitAt.end();)
    {
        uint32 const age = getMSTimeDiff(itr->second, now);
        if (age < BG_SA_BOAT_LANDING_MS)
        {
            ++itr;
            continue;
        }

        ObjectGuid const guid = itr->first;
        itr = _boatInitAt.erase(itr);

        if (GetPlayers().find(guid) == GetPlayers().end())
            continue;

        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player || !player->IsInWorld() || player->GetMapId() != GetMapId() || player->GetBGTeamId() != Attackers)
            continue;

        Transport* transport = player->GetTransport();
        bool const onBoat = transport && (transport->GetGUID() == BgObjects[BG_SA_BOAT_ONE] ||
                                          transport->GetGUID() == BgObjects[BG_SA_BOAT_TWO]);
        if (!onBoat && (!player->IsAlive() || player->GetPositionX() < BG_SA_OPEN_SEA_X))
            continue;   // already on the beach side, or dead

        float const x = player->GetPositionX();
        float const y = player->GetPositionY();
        if (y < 0) // east dock
            player->TeleportTo(607, 1598.571f, -106.671f, 8.873f, 4.1359f, 0);
        else // west dock
            player->TeleportTo(607, 1607.596f, 49.766f, 7.5795f, 2.31865f, 0);

        TC_LOG_DEBUG("entities.gameobject", "[851] boat-land player=%s age=%u aboard=%u from=%.1f,%.1f status=%u totaltime=%u",
            player->GetName().c_str(), age, uint32(onBoat), x, y, uint32(Status), TotalTime);
    }
}

// #736: AC drives the boats with DoorOpen (StaticTransport: GO_STATE_ACTIVE runs the path to pauseAtTime and holds -
// AC Transport.cpp StaticTransport::Update). On 5.4.8 the client needs the stop frame instead (Transport::
// StartToFirstStopFrame), so the boats leave the ocean spot here and arrive at the dock as the warm-up ends.
void BattlegroundSA::LaunchShips(uint32 travelTime)
{
    if (ShipsLaunched)
        return;

    for (uint8 i = BG_SA_BOAT_ONE; i <= BG_SA_BOAT_TWO; ++i)
        if (GameObject* boat = GetBGObject(i))
            if (Transport* transport = boat->ToTransport())
                transport->StartToFirstStopFrame(travelTime);

    ShipsLaunched = true;
}

// #736 (owner 2026-10-08, build 21: "attackers are teleported to the dock the moment the BG starts, instead of riding
// the ship"): the old "temp hack for ships" (also in MoP 5.4.8: 59000 / 53000)
// teleported every attacker who was on a transport OR in the water to the dock at 59 s / 53 s of the warm-up - the
// riders included. AC has no such teleport (BattlegroundSA::PostUpdateImpl only calls StartShips): the boats carry
// the attackers to the dock. Kept only as a rescue for an attacker who is NOT on a boat and still out at sea (fell
// off, or a bot that never boarded) - the beach is 1000 yd away. Riders are never touched.
void BattlegroundSA::RescueStrandedAttackers()
{
    for (auto&& itr : GetPlayers())
    {
        Player* player = ObjectAccessor::FindPlayer(itr.first);
        if (!player || player->GetBGTeamId() != Attackers || player->GetTransport() || !player->IsAlive())
            continue;

        if (player->GetPositionX() < BG_SA_OPEN_SEA_X)  // on the beach / at the docks already
            continue;

        if (player->GetPositionY() < 0) // east dock
            player->TeleportTo(607, 1598.571f, -106.671f, 8.873f, 4.1359f, 0);
        else // west dock
            player->TeleportTo(607, 1607.596f, 49.766f, 7.5795f, 2.31865f, 0);
    }
}

// #736 (owner 2026-10-08: "closed gates can be passed through by bots"): the 607 navmesh has walkable ground under
// every gate - the gates are destructible GOs, present only in the dynamic vmap tree (mmtile probe, data-mop-extract:
// polys 3755/5099/3965/1332/1261/3360 at the six gate centres) - so a path the server builds (MotionMaster::MovePoint
// -> MoveSplineInit::MoveTo: playerbots, the demolishers they drive, pets) ran straight through a closed gate, which a
// client cannot do (it collides with the gate model). Wave 27 probes: 24 walks through an intact gate in 6 rounds.
// A segment crossing a gate's plane (the GO faces the sea) within the width of the intact gate model
// (GameObjectDisplayInfo bounds) at ground height (not the wall walk above) is cut 1 yd in front of the gate on the
// mover's side, until the gate is destroyed; a damaged gate still blocks.
void BattlegroundSA::ClipPathAtClosedGates(Unit const* mover, std::vector<G3D::Vector3>& path)
{
    for (uint8 gate = BG_SA_GREEN_GATE; gate <= BG_SA_ANCIENT_GATE && path.size() >= 2; ++gate)
    {
        if (GateStatus[gate] == BG_SA_GATE_DESTROYED)
            continue;

        GameObject* go = GetBgMap()->GetGameObject(BgObjects[gate]);
        if (!go || go->HasFlag(GAMEOBJECT_FIELD_FLAGS, GO_FLAG_DESTROYED))
            continue;

        GameObjectDisplayInfoEntry const* info = sGameObjectDisplayInfoStore.LookupEntry(go->GetGOInfo()->displayId);
        if (!info)
            continue;

        float const cosO = std::cos(go->GetOrientation());
        float const sinO = std::sin(go->GetOrientation());
        float const scale = go->GetObjectScale();
        float const bottom = go->GetPositionZ() + info->minZ * scale;
        for (size_t i = 1; i < path.size(); ++i)
        {
            G3D::Vector3 const a = path[i - 1];
            G3D::Vector3 const b = path[i];
            float const ax = (a.x - go->GetPositionX()) * cosO + (a.y - go->GetPositionY()) * sinO;
            float const bx = (b.x - go->GetPositionX()) * cosO + (b.y - go->GetPositionY()) * sinO;
            if (ax * bx >= 0.0f)
                continue;

            G3D::Vector3 const hit = a + (b - a) * (ax / (ax - bx));
            float const hy = (hit.y - go->GetPositionY()) * cosO - (hit.x - go->GetPositionX()) * sinO;
            if (hy < info->minY * scale || hy > info->maxY * scale || hit.z < bottom - 2.0f || hit.z > bottom + 10.0f)
                continue;

            float const face = (ax > 0.0f ? info->maxX : -info->minX) * scale + 1.0f;
            float const stop = std::max(0.0f, (std::fabs(ax) - face) / std::fabs(ax - bx));
            path[i] = a + (b - a) * stop;
            path.resize(i + 1);
            TC_LOG_DEBUG("entities.gameobject", "[736] gate-clip mover=%s entry=%u gate=%u status=%u at=%.1f,%.1f,%.1f",
                mover->GetName().c_str(), mover->GetEntry(), uint32(gate), uint32(GateStatus[gate]), path[i].x, path[i].y, path[i].z);
            break;
        }
    }
}

// #736: the client's start countdown for the round-2 warm-up (SMSG_START_TIMER, as Battleground::ProcessJoin sends it
// for round 1), to one player or to everyone (player = nullptr)
void BattlegroundSA::SendRoundTwoStartTimer(Player* player)
{
    if (Status != BG_SA_SECOND_WARMUP || InitSecondRound || TotalTime >= BG_SA_SECOND_WARMUPLENGTH)
        return;

    uint32 const maxSecs = BG_SA_SECOND_WARMUPLENGTH / IN_MILLISECONDS;
    uint32 const secs = (BG_SA_SECOND_WARMUPLENGTH - TotalTime) / IN_MILLISECONDS;
    if (player)
    {
        player->SendStartTimer(secs, maxSecs, TIMER_PVP);
        return;
    }

    for (auto&& itr : GetPlayers())
        if (Player* member = ObjectAccessor::FindPlayer(itr.first))
            member->SendStartTimer(secs, maxSecs, TIMER_PVP);
}

void BattlegroundSA::PostUpdateImpl(uint32 diff)
{
    if (InitSecondRound)
    {
        if (UpdateWaitTimer < diff)
        {
            if (!SignaledRoundTwo)
            {
                SignaledRoundTwo = true;
                InitSecondRound = false;
                SendMessageToAll(LANG_BG_SA_ROUND_TWO_ONE_MINUTE, CHAT_MSG_BG_SYSTEM_NEUTRAL);
                // #736: the round-2 start countdown (Battleground::ProcessJoin's countdown only covers round 1)
                SendRoundTwoStartTimer(nullptr);
            }
        }
        else
        {
            UpdateWaitTimer -= diff;
            return;
        }
    }
    TotalTime += diff;

    if (Status == BG_SA_WARMUP)
    {
        // AC PostUpdateImpl (BattlegroundSA.cpp:338-353): the boats wait at sea, StartShips at BG_SA_BOAT_START (1:00)
        // #851 elevator boats: after Transport.SotaBoatHoldSeconds (default 60 = BG_SA_BOAT_START, capped at the warm-up)
        if (TotalTime >= (BoatsStopFrameMode() == 2 ? _boatHoldMs : uint32(BG_SA_BOAT_START)))
            StartShips();

        EndRoundTimer = BG_SA_ROUNDLENGTH;
        if (TotalTime >= BG_SA_WARMUPLENGTH)
        {
            TotalTime = 0;
            ToggleTimer();
            DemolisherStartState(false);
            Status = BG_SA_ROUND_ONE;
            StartCriteria(CRITERIA_START_TYPE_EVENT, (Attackers == TEAM_ALLIANCE) ? 23748 : 21702);

            EndTimestamp = time(nullptr) + BG_SA_ROUNDLENGTH / IN_MILLISECONDS;
            UpdateWorldState(BG_SA_ENABLE_TIMER, 1);
            UpdateWorldState(BG_SA_TIMER, EndTimestamp);
        }
        LandBoatRiders();   // #851 MoP 5.4.8 boats
        if (TotalTime >= BG_SA_WARMUPLENGTH - 1000)  // was 59 s of the old 60 s warm-up
            RescueStrandedAttackers();
        return;
    }
    else if (Status == BG_SA_SECOND_WARMUP)
    {
        if (RoundScores[0].time<BG_SA_ROUNDLENGTH)
            EndRoundTimer = RoundScores[0].time;
        else
            EndRoundTimer = BG_SA_ROUNDLENGTH;

        if (TotalTime >= BG_SA_SECOND_WARMUPLENGTH)
        {
            SendWarningToAll(LANG_BG_SA_HAS_BEGUN);
            TotalTime = 0;
            ToggleTimer();
            DemolisherStartState(false);
            gateDestroyed = false;
            Status = BG_SA_ROUND_TWO;

            EndTimestamp = time(nullptr) + EndRoundTimer / IN_MILLISECONDS;
            UpdateWorldState(BG_SA_ENABLE_TIMER, 1);
            UpdateWorldState(BG_SA_TIMER, EndTimestamp);

            StartCriteria(CRITERIA_START_TYPE_EVENT, (Attackers == TEAM_ALLIANCE) ? 23748 : 21702);
            // status was set to STATUS_WAIT_JOIN manually for Preparation, set it back now
            SetStatus(STATUS_IN_PROGRESS);
            for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
                if (Player* p = ObjectAccessor::FindPlayer(itr->first))
                    p->RemoveAurasDueToSpell(SPELL_PREPARATION);
        }
        if (TotalTime >= 30000)
        {
            if (!SignaledRoundTwoHalfMin)
            {
                SignaledRoundTwoHalfMin = true;
                SendMessageToAll(LANG_BG_SA_ROUND_TWO_START_HALF_MINUTE, CHAT_MSG_BG_SYSTEM_NEUTRAL);
            }
        }
        // round 2: every attacker was put on a boat by ResetObjs -> TeleportPlayers, so the boats sail at once (AC
        // calls StartShips on every SECOND_WARMUP tick, BattlegroundSA.cpp:388) and dock as round 2 starts
        StartShips();
        LandBoatRiders();   // #851 MoP 5.4.8 boats
        if (TotalTime >= 53000)
            RescueStrandedAttackers();
        return;
    }
    else if (GetStatus() == STATUS_IN_PROGRESS)
    {
        if (BoatsStopFrameMode() == 2)
            LandDockedBoatRiders();   // #851 elevator boats dock after the round started: the safety net runs here
        if (Status == BG_SA_ROUND_ONE)
        {
            if (TotalTime >= BG_SA_ROUNDLENGTH)
            {
                // Defense of the Ancients (1757 / 2200)
                if (!gateDestroyed)
                    GetBgMap()->SetWorldState(GetDefenerTeam() == TEAM_ALLIANCE ? WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_A : WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_H, 1);

                CastSpellOnTeam(SPELL_END_OF_ROUND, ALLIANCE);
                CastSpellOnTeam(SPELL_END_OF_ROUND, HORDE);

                RoundScores[0].winner = Attackers;
                RoundScores[0].time = BG_SA_ROUNDLENGTH;
                TotalTime = 0;
                Status = BG_SA_SECOND_WARMUP;
                EndTimestamp = 0;
                Attackers = (Attackers == TEAM_ALLIANCE) ? TEAM_HORDE : TEAM_ALLIANCE;
                UpdateWaitTimer = 5000;
                SignaledRoundTwo = false;
                SignaledRoundTwoHalfMin = false;
                InitSecondRound = true;
                ToggleTimer();
                ResetObjs();
                return;
            }
        }
        else if (Status == BG_SA_ROUND_TWO)
        {
            if (TotalTime >= EndRoundTimer)
            {
                // Defense of the Ancients (1757 / 2200)
                if (!gateDestroyed)
                    GetBgMap()->SetWorldState(GetDefenerTeam() == TEAM_ALLIANCE ? WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_A : WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_H, 1);

                CastSpellOnTeam(SPELL_END_OF_ROUND, ALLIANCE);
                CastSpellOnTeam(SPELL_END_OF_ROUND, HORDE);

                EndTimestamp = 0;
                UpdateWorldState(BG_SA_WorldStates::BG_SA_ENABLE_TIMER, 0);
                UpdateWorldState(BG_SA_WorldStates::BG_SA_TIMER, EndTimestamp);

                RoundScores[1].time = BG_SA_ROUNDLENGTH;
                RoundScores[1].winner = (Attackers == TEAM_ALLIANCE) ? TEAM_HORDE : TEAM_ALLIANCE;

                if (RoundScores[0].time == RoundScores[1].time)
                    EndBattleground(0);
                else if (RoundScores[0].time < RoundScores[1].time)
                    EndBattleground(RoundScores[0].winner == TEAM_ALLIANCE ? ALLIANCE : HORDE);
                else
                    EndBattleground(RoundScores[1].winner == TEAM_ALLIANCE ? ALLIANCE : HORDE);
                return;
            }
        }
        if (Status == BG_SA_ROUND_ONE || Status == BG_SA_ROUND_TWO)
        {
            timeUpdateTimer.Update(diff);
            if (timeUpdateTimer.Passed())
            {
                timeUpdateTimer.Reset(IN_MILLISECONDS);
                SendTime();
            }
            UpdateDemolisherSpawns();
        }
    }
}

void BattlegroundSA::StartingEventCloseDoors() { }

void BattlegroundSA::StartingEventOpenDoors() { }

void BattlegroundSA::FillInitialWorldStates(WorldStateBuilder& builder)
{
    uint32 ally_attacks = uint32(Attackers == TEAM_ALLIANCE ? 1 : 0);
    uint32 horde_attacks = uint32(Attackers == TEAM_HORDE ? 1 : 0);

    builder.AppendState(BG_SA_ANCIENT_GATEWS, GateStatus[BG_SA_ANCIENT_GATE]);
    builder.AppendState(BG_SA_YELLOW_GATEWS, GateStatus[BG_SA_YELLOW_GATE]);
    builder.AppendState(BG_SA_GREEN_GATEWS, GateStatus[BG_SA_GREEN_GATE]);
    builder.AppendState(BG_SA_BLUE_GATEWS, GateStatus[BG_SA_BLUE_GATE]);
    builder.AppendState(BG_SA_RED_GATEWS, GateStatus[BG_SA_RED_GATE]);
    builder.AppendState(BG_SA_PURPLE_GATEWS, GateStatus[BG_SA_PURPLE_GATE]);

    builder.AppendState(BG_SA_BONUS_TIMER, 0);

    builder.AppendState(BG_SA_HORDE_ATTACKS, horde_attacks);
    builder.AppendState(BG_SA_ALLY_ATTACKS, ally_attacks);

    //Time will be sent on first update...
    /*
    builder.AppendState(BG_SA_ENABLE_TIMER, TimerEnabled ? 1 : 0);
    builder.AppendState(BG_SA_TIMER_MINS, 0);
    builder.AppendState(BG_SA_TIMER_SEC_TENS, 0);
    builder.AppendState(BG_SA_TIMER_SEC_DECS, 0);
    */

    builder.AppendState(BG_SA_RIGHT_GY_HORDE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    builder.AppendState(BG_SA_LEFT_GY_HORDE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);
    builder.AppendState(BG_SA_CENTER_GY_HORDE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_HORDE ? 1 : 0);

    builder.AppendState(BG_SA_RIGHT_GY_ALLIANCE, GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    builder.AppendState(BG_SA_LEFT_GY_ALLIANCE, GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);
    builder.AppendState(BG_SA_CENTER_GY_ALLIANCE, GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] == TEAM_ALLIANCE ? 1 : 0);

    builder.AppendState(BG_SA_HORDE_DEFENCE_TOKEN, ally_attacks);
    builder.AppendState(BG_SA_ALLIANCE_DEFENCE_TOKEN, horde_attacks);

    builder.AppendState(BG_SA_LEFT_ATT_TOKEN_HRD, horde_attacks);
    builder.AppendState(BG_SA_RIGHT_ATT_TOKEN_HRD, horde_attacks);
    builder.AppendState(BG_SA_RIGHT_ATT_TOKEN_ALL, ally_attacks);
    builder.AppendState(BG_SA_LEFT_ATT_TOKEN_ALL, ally_attacks);

    if (EndTimestamp)
    {
        builder.AppendState(BG_SA_ENABLE_TIMER, 1);
        builder.AppendState(BG_SA_TIMER, EndTimestamp);
    }
}

// #736: spread the attackers over both boats. AC alternates the boat for every attacker it places
// (BattlegroundSA::TeleportToEntrancePosition, BattlegroundSA.cpp:569-590, _nextShipIsEast toggled at :585), so the
// round start (TeleportPlayers) and every late joiner (AddPlayer) split evenly. The old code sent late joiners to
// "the nearest mate" - always the first attacker found - so they all landed on one ship.
// East ship = y < 0 (2682.9, -830.4), west ship = y > 0 (2577.0, 980.3).
// As AC TeleportToEntrancePosition (BattlegroundSA.cpp:569-590): on a boat until the boats leave (!ShipsStarted -
// StartShips = the voyage), the toggle only when a boat is used; after that the one dock spot of AC (:588). A boat under
// way (~22 yd/s) would sail out from under a player dropped onto it with slow fall.
void BattlegroundSA::TeleportAttackerToShip(Player* player)
{
    if (ShipsStarted)
    {
        player->TeleportTo(607, 1600.381f, -106.263f, 8.8745f, 3.78f, 0);
        return;
    }

    player->CastSpell(player, 12438, true); // Without this player falls before boat loads...

    // boats still waiting at their ocean spot
    if (_nextShipIsEast)
        player->TeleportTo(607, 2682.936f, -830.368f, 15.0f, 2.895f, 0);
    else
        player->TeleportTo(607, 2577.003f, 980.261f, 15.0f, 0.807f, 0);

    _nextShipIsEast = !_nextShipIsEast;
}

void BattlegroundSA::AddPlayer(Player* player)
{
    Battleground::AddPlayer(player);
    //create score and add it to map, default values are set in constructor
    BattlegroundSAScore* sc = new BattlegroundSAScore{};
    sc->Team = player->GetBGTeam();

    if (player->GetBGTeamId() == Attackers)
        TeleportAttackerToShip(player);
    else
        player->TeleportTo(607, 1209.7f, -65.16f, 70.1f, 0.0f, 0);

    SendTransportInit(player);
    SendRoundTwoStartTimer(player);  // #736: a late joiner in the round-2 warm-up gets its countdown too
    PlayerScores[player->GetGUID()] = sc;
}

void BattlegroundSA::RemovePlayer(Player* player, ObjectGuid guid, uint32 team)
{
    if (player)
    {
        player->RemoveAura(52418);
    }
    Battleground::RemovePlayer(player, guid, team);
}

void BattlegroundSA::HandleAreaTrigger(Player* /*Source*/, uint32 /*Trigger*/, bool /*Entered*/)
{
    // this is wrong way to implement these things. On official it done by gameobject spell cast.
    if (GetStatus() != STATUS_IN_PROGRESS)
        return;
}

void BattlegroundSA::UpdatePlayerScore(Player* Source, uint32 type, uint32 value, bool doAddHonor)
{
    BattlegroundScoreMap::iterator itr = PlayerScores.find(Source->GetGUID());
    if (itr == PlayerScores.end())                         // player not found...
        return;

    if (type == SCORE_DESTROYED_DEMOLISHER)
        ((BattlegroundSAScore*)itr->second)->demolishers_destroyed += value;
    else if (type == SCORE_DESTROYED_WALL)
        ((BattlegroundSAScore*)itr->second)->gates_destroyed += value;
    else
        Battleground::UpdatePlayerScore(Source, type, value, doAddHonor);
}

void BattlegroundSA::TeleportPlayers()
{
    for (BattlegroundPlayerMap::const_iterator itr = GetPlayers().begin(); itr != GetPlayers().end(); ++itr)
    {
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
        {
            player->StopMoving();
            player->GetMotionMaster()->Clear();
            player->GetMotionMaster()->MoveIdle();

            // should remove spirit of redemption
            if (player->HasAuraType(SPELL_AURA_SPIRIT_OF_REDEMPTION))
                player->RemoveAurasByType(SPELL_AURA_MOD_SHAPESHIFT);

            // remove all negative auras before start new round
            for (auto itr = player->GetAppliedAuras().begin(); itr != player->GetAppliedAuras().end();)
            {
                AuraApplication const* aurApp = itr->second;
                ++itr;
                Aura* aura = aurApp->GetBase();
                if (!aurApp->IsPositive() && aura->GetDuration() <= 30 * IN_MILLISECONDS && !aura->IsPermanent())
                {
                    player->RemoveAura(aura);
                    itr = player->GetAppliedAuras().begin();
                }
            }

            if (!player->IsAlive())
            {
                player->ResurrectPlayer(1.0f);
                player->SpawnCorpseBones();
            }

            player->ResetAllPowers();
            player->CombatStopWithPets(true);

            player->RemoveAllControlled();
            player->CastSpell(player, SPELL_PREPARATION, true);

            if (player->GetBGTeamId() == Attackers)
                TeleportAttackerToShip(player);
            else
                player->TeleportTo(607, 1209.7f, -65.16f, 70.1f, 0.0f, 0);
        }
    }
}

void BattlegroundSA::EventPlayerDamagedGO(Player* /*player*/, GameObject* go, uint32 eventType)
{
    if (!go || !go->GetGOInfo())
        return;

    if (eventType == go->GetGOInfo()->building.damagedEvent)
    {
        uint32 i = getGateIdFromDamagedOrDestroyEventId(eventType);
        GateStatus[i] = BG_SA_GATE_DAMAGED;
        uint32 uws = getWorldStateFromGateId(i);
        if (uws)
            UpdateWorldState(uws, GateStatus[i]);
    }

    if (eventType == go->GetGOInfo()->building.destroyedEvent)
    {
        if (go->GetGOInfo()->building.destroyedEvent == BG_SA_EVENT_ANCIENT_GATE_DESTROYED)
            SendWarningToAll(LANG_BG_SA_CHAMBER_BREACHED);
        else
            SendGateDestroyed(LANG_BG_SA_WAS_DESTROYED, go->GetGOInfo()->entry);
    }

    if (eventType == go->GetGOInfo()->building.damagedEvent)
        SendGateDestroyed(LANG_BG_SA_IS_UNDER_ATTACK, go->GetGOInfo()->entry);
}

void BattlegroundSA::HandleKillUnit(Creature* creature, Player* killer)
{
    if (creature->GetEntry() == NPC_DEMOLISHER_SA)
    {
        UpdatePlayerScore(killer, SCORE_DESTROYED_DEMOLISHER, 1);
        _allVehiclesAlive[Attackers] = false;
    }
}

/*
  You may ask what the fuck does it do?
  Prevents owner overwriting guns faction with own.
 */
void BattlegroundSA::OverrideGunFaction()
{
    if (!BgCreatures[0])
        return;

    for (uint8 i = BG_SA_GUN_1; i <= BG_SA_GUN_10; i++)
    {
        if (Creature* gun = GetBGCreature(i))
            gun->SetFaction(BG_SA_Factions[Attackers ? TEAM_ALLIANCE : TEAM_HORDE]);
    }

    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_4; i++)
    {
        if (Creature* dem = GetBGCreature(i))
            dem->SetFaction(BG_SA_Factions[Attackers]);
    }
}

void BattlegroundSA::DemolisherStartState(bool start)
{
    if (!BgCreatures[0])
        return;

    // set flags only for the demolishers on the beach, factory ones dont need it
    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_4; i++)
    {
        if (Creature* dem = GetBGCreature(i))
        {
            if (start)
                dem->SetFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
            else
                dem->RemoveFlag(UNIT_FIELD_FLAGS, UNIT_FLAG_NON_ATTACKABLE | UNIT_FLAG_NOT_SELECTABLE);
        }
    }
}

void BattlegroundSA::DestroyGate(Player* player, GameObject* go)
{
    uint32 i = getGateIdFromDamagedOrDestroyEventId(go->GetGOInfo()->building.destroyedEvent);
    if (!GateStatus[i])
        return;

    if (GameObject* g = GetBGObject(i))
    {
        if (g->GetGOValue()->Building.Health == 0)
        {
            GateStatus[i] = BG_SA_GATE_DESTROYED;
            uint32 uws = getWorldStateFromGateId(i);
            if (uws)
                UpdateWorldState(uws, GateStatus[i]);
            bool rewardHonor = true;
            gateDestroyed = true;
            switch (i)
            {
                case BG_SA_GREEN_GATE:
                    if (GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_BLUE_GATE:
                    if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_RED_GATE:
                    if (GateStatus[BG_SA_PURPLE_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                case BG_SA_PURPLE_GATE:
                    if (GateStatus[BG_SA_RED_GATE] == BG_SA_GATE_DESTROYED)
                        rewardHonor = false;
                    break;
                default:
                    break;
            }
            // Delete teleport triggers
            switch (i)
            {
                case BG_SA_GREEN_GATE:
                case BG_SA_BLUE_GATE:
                    for (uint32 i = BG_SA_NPC_TRIGGER_1; i <= BG_SA_NPC_TRIGGER_2; ++i)
                        DelCreature(i);
                    break;
                case BG_SA_RED_GATE:
                case BG_SA_PURPLE_GATE:
                    for (uint32 i = BG_SA_NPC_TRIGGER_3; i <= BG_SA_NPC_TRIGGER_4; ++i)
                        DelCreature(i);
                    break;
                default:
                    break;
            }

            if (i < 5)
                DelObject(i + 14);

            if (player)
            {
                UpdatePlayerScore(player, SCORE_DESTROYED_WALL, 1);
                if (rewardHonor)
                    UpdatePlayerScore(player, SCORE_BONUS_HONOR, GetBonusHonorFromKill(1));
            }
        }
    }
}

WorldSafeLocsEntry const* BattlegroundSA::GetClosestGraveYard(Player* player)
{
    uint32 safeloc = 0;
    WorldSafeLocsEntry const* ret;
    WorldSafeLocsEntry const* closest;
    float dist, nearest;
    float x, y, z;

    player->GetPosition(x, y, z);

    if (player->GetBGTeamId() == Attackers)
        safeloc = BG_SA_GYEntries[BG_SA_BEACH_GY];
    else
        safeloc = BG_SA_GYEntries[BG_SA_DEFENDER_LAST_GY];

    closest = sWorldSafeLocsStore.LookupEntry(safeloc);
    nearest = sqrt((closest->x - x)*(closest->x - x) + (closest->y - y)*(closest->y - y) + (closest->z - z)*(closest->z - z));

    for (uint8 i = BG_SA_RIGHT_CAPTURABLE_GY; i < BG_SA_MAX_GY; i++)
    {
        if (GraveyardStatus[i] != player->GetBGTeamId())
            continue;

        ret = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);
        dist = sqrt((ret->x - x)*(ret->x - x) + (ret->y - y)*(ret->y - y) + (ret->z - z)*(ret->z - z));
        if (dist < nearest)
        {
            closest = ret;
            nearest = dist;
        }
    }

    return closest;
}

void BattlegroundSA::SendTime()
{
    // moved to BG_SA_TIMER in MoP
    /*
    uint32 end_of_round = (EndRoundTimer - TotalTime);
    UpdateWorldState(BG_SA_TIMER_MINS, end_of_round/60000);
    UpdateWorldState(BG_SA_TIMER_SEC_TENS, (end_of_round%60000)/10000);
    UpdateWorldState(BG_SA_TIMER_SEC_DECS, ((end_of_round%60000)%10000)/1000);
    */
}

void BattlegroundSA::EventPlayerClickedOnFlag(Player* Source, GameObject* target_obj)
{
    switch (target_obj->GetEntry())
    {
        case 191307:
        case 191308:
            if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                CaptureGraveyard(BG_SA_LEFT_CAPTURABLE_GY, Source);
            else
                target_obj->SendUpdateToPlayer(Source);
            break;
        case 191305:
        case 191306:
            if (GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED)
                CaptureGraveyard(BG_SA_RIGHT_CAPTURABLE_GY, Source);
            else
                target_obj->SendUpdateToPlayer(Source);
            break;
        case 191310:
        case 191309:
            if ((GateStatus[BG_SA_GREEN_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_BLUE_GATE] == BG_SA_GATE_DESTROYED) && (GateStatus[BG_SA_RED_GATE] == BG_SA_GATE_DESTROYED || GateStatus[BG_SA_PURPLE_GATE] == BG_SA_GATE_DESTROYED))
            {
                CaptureGraveyard(BG_SA_CENTRAL_CAPTURABLE_GY, Source);
                CaptureGraveyard(BG_SA_LEFT_CAPTURABLE_GY, Source);
                CaptureGraveyard(BG_SA_RIGHT_CAPTURABLE_GY, Source);
            }
            else
                target_obj->SendUpdateToPlayer(Source);
            break;
        default:
            return;
    };
}

void BattlegroundSA::CaptureGraveyard(BG_SA_Graveyards i, Player* Source)
{
    if (GraveyardStatus[i] == Attackers)
        return;

    ObjectGuid spiritGuideGUID = BgCreatures[BG_SA_MAXNPC + i];

    DelCreature(BG_SA_MAXNPC + i);
    GraveyardStatus[i] = Source->GetBGTeamId();
    WorldSafeLocsEntry const* sg = sWorldSafeLocsStore.LookupEntry(BG_SA_GYEntries[i]);
    if (!sg)
    {
        TC_LOG_ERROR("bg.battleground", "BattlegroundSA::CaptureGraveyard: non-existant GY entry: %u", BG_SA_GYEntries[i]);
        return;
    }

    AddSpiritGuide(i + BG_SA_MAXNPC, sg->x, sg->y, sg->z, BG_SA_GYOrientation[i], (GraveyardStatus[i] == TEAM_ALLIANCE?  ALLIANCE : HORDE));
    uint32 npc = 0;
    uint32 flag = 0;

    switch (i)
    {
        case BG_SA_LEFT_CAPTURABLE_GY:
            flag = BG_SA_LEFT_FLAG;
            DelObject(flag);
            AddObject(flag, (BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1)),
            BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1],
            BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            npc = BG_SA_NPC_RIGSPARK;
            AddCreature(BG_SA_NpcEntries[npc], npc, Attackers,
              BG_SA_NpcSpawnlocs[npc][0], BG_SA_NpcSpawnlocs[npc][1],
              BG_SA_NpcSpawnlocs[npc][2], BG_SA_NpcSpawnlocs[npc][3]);

            for (uint8 j = BG_SA_DEMOLISHER_7; j <= BG_SA_DEMOLISHER_8; j++)
            {
                AddCreature(BG_SA_NpcEntries[j], j, (Attackers == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE),
                    BG_SA_NpcSpawnlocs[j][0], BG_SA_NpcSpawnlocs[j][1],
                    BG_SA_NpcSpawnlocs[j][2], BG_SA_NpcSpawnlocs[j][3], 600);

                if (Creature* dem = GetBGCreature(j))
                    dem->SetFaction(BG_SA_Factions[Attackers]);
            }

            UpdateWorldState(BG_SA_LEFT_GY_ALLIANCE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0));
            UpdateWorldState(BG_SA_LEFT_GY_HORDE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1));
            if (Source->GetBGTeamId() == TEAM_ALLIANCE)
                SendWarningToAll(LANG_BG_SA_A_GY_WEST);
            else
                SendWarningToAll(LANG_BG_SA_H_GY_WEST);
            break;
        case BG_SA_RIGHT_CAPTURABLE_GY:
            flag = BG_SA_RIGHT_FLAG;
            DelObject(flag);
            AddObject(flag, (BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1)),
              BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1],
              BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            npc = BG_SA_NPC_SPARKLIGHT;
            AddCreature(BG_SA_NpcEntries[npc], npc, Attackers,
              BG_SA_NpcSpawnlocs[npc][0], BG_SA_NpcSpawnlocs[npc][1],
              BG_SA_NpcSpawnlocs[npc][2], BG_SA_NpcSpawnlocs[npc][3]);

            for (uint8 j = BG_SA_DEMOLISHER_5; j <= BG_SA_DEMOLISHER_6; j++)
            {
                AddCreature(BG_SA_NpcEntries[j], j, (Attackers == TEAM_ALLIANCE ? TEAM_HORDE : TEAM_ALLIANCE),
                    BG_SA_NpcSpawnlocs[j][0], BG_SA_NpcSpawnlocs[j][1],
                    BG_SA_NpcSpawnlocs[j][2], BG_SA_NpcSpawnlocs[j][3], 600);

                if (Creature* dem = GetBGCreature(j))
                    dem->SetFaction(BG_SA_Factions[Attackers]);
            }

            UpdateWorldState(BG_SA_RIGHT_GY_ALLIANCE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0));
            UpdateWorldState(BG_SA_RIGHT_GY_HORDE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1));
            if (Source->GetBGTeamId() == TEAM_ALLIANCE)
                SendWarningToAll(LANG_BG_SA_A_GY_EAST);
            else
                SendWarningToAll(LANG_BG_SA_H_GY_EAST);
            break;
        case BG_SA_CENTRAL_CAPTURABLE_GY:
            flag = BG_SA_CENTRAL_FLAG;
            DelObject(flag);
            AddObject(flag, (BG_SA_ObjEntries[flag] - (Source->GetBGTeamId() == TEAM_ALLIANCE ? 0 : 1)),
              BG_SA_ObjSpawnlocs[flag][0], BG_SA_ObjSpawnlocs[flag][1],
              BG_SA_ObjSpawnlocs[flag][2], BG_SA_ObjSpawnlocs[flag][3], 0, 0, 0, 0, RESPAWN_ONE_DAY);

            UpdateWorldState(BG_SA_CENTER_GY_ALLIANCE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 1 : 0));
            UpdateWorldState(BG_SA_CENTER_GY_HORDE, (GraveyardStatus[i] == TEAM_ALLIANCE ? 0 : 1));
            if (Source->GetBGTeamId() == TEAM_ALLIANCE)
                SendWarningToAll(LANG_BG_SA_A_GY_SOUTH);
            else
                SendWarningToAll(LANG_BG_SA_H_GY_SOUTH);
            break;
        default:
            ASSERT(false);
            break;
    };

    UpdateBombSpawns();

    // Those who are waiting to resurrect at this node are taken to the closest own node's graveyard
    WorldSafeLocsEntry const* grave = nullptr;
    std::vector<ObjectGuid> guids = m_ReviveQueue[spiritGuideGUID];
    for (auto&& guid : guids)
    {
        Player* player = ObjectAccessor::FindPlayer(guid);
        if (!player)
            continue;

        if (!grave)
            grave = GetClosestGraveYard(player);

        if (grave)
            player->TeleportTo(GetMapId(), grave->x, grave->y, grave->z, player->GetOrientation());
    }
}

void BattlegroundSA::EventPlayerUsedGO(Player* Source, GameObject* object)
{
    if (object->GetEntry() == BG_SA_ObjEntries[BG_SA_TITAN_RELIC] && GateStatus[BG_SA_ANCIENT_GATE] == BG_SA_GATE_DESTROYED)
    {
        if (Source->GetBGTeamId() == Attackers)
        {
            if (Source->GetBGTeamId() == TEAM_ALLIANCE)
                SendMessageToAll(LANG_BG_SA_ALLIANCE_CAPTURED_RELIC, CHAT_MSG_BG_SYSTEM_NEUTRAL);
            else SendMessageToAll(LANG_BG_SA_HORDE_CAPTURED_RELIC, CHAT_MSG_BG_SYSTEM_NEUTRAL);

            if (Status == BG_SA_ROUND_ONE)
            {
                // Defense of the Ancients (1757 / 2200)
                if (!gateDestroyed)
                    GetBgMap()->SetWorldState(GetDefenerTeam() == TEAM_ALLIANCE ? WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_A : WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_H, 1);

                CastSpellOnTeam(SPELL_END_OF_ROUND, ALLIANCE);
                CastSpellOnTeam(SPELL_END_OF_ROUND, HORDE);

                RoundScores[0].winner = Attackers;
                RoundScores[0].time = TotalTime;
                // Achievement Storm the Beach (1310)
                if (RoundScores[0].winner == Attackers && TotalTime <= 240000)
                {
                    GetBgMap()->SetWorldState(Attackers == TEAM_ALLIANCE ? WORLD_STATE_STORM_THE_BEACH_A : WORLD_STATE_STORM_THE_BEACH_H, 1);
                    for (auto&& itr : GetPlayers())
                        if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                            if (player->GetBGTeamId() == Attackers)
                                player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 65246);
                }

                Attackers = (Attackers == TEAM_ALLIANCE) ? TEAM_HORDE : TEAM_ALLIANCE;
                Status = BG_SA_SECOND_WARMUP;
                EndTimestamp = 0;
                TotalTime = 0;
                ToggleTimer();
                SendWarningToAll(LANG_BG_SA_ROUND_ONE_END);
                UpdateWaitTimer = 5000;
                SignaledRoundTwo = false;
                SignaledRoundTwoHalfMin = false;
                InitSecondRound = true;
                ResetObjs();
            }
            else if (Status == BG_SA_ROUND_TWO)
            {
                // Defense of the Ancients (1757 / 2200)
                if (!gateDestroyed)
                    GetBgMap()->SetWorldState(GetDefenerTeam() == TEAM_ALLIANCE ? WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_A : WORLD_STATE_DEFENCE_OF_THE_ANCIENTS_H, 1);

                CastSpellOnTeam(SPELL_END_OF_ROUND, ALLIANCE);
                CastSpellOnTeam(SPELL_END_OF_ROUND, HORDE);

                RoundScores[1].winner = Attackers;
                RoundScores[1].time = TotalTime;
                ToggleTimer();
                // Achievement Storm the Beach (1310)
                if (RoundScores[1].winner == Attackers && TotalTime <= 240000)
                {
                    GetBgMap()->SetWorldState(Attackers == TEAM_ALLIANCE ? WORLD_STATE_STORM_THE_BEACH_A : WORLD_STATE_STORM_THE_BEACH_H, 1);
                    for (auto&& itr : GetPlayers())
                        if (Player* player = ObjectAccessor::FindPlayer(itr.first))
                            if (player->GetBGTeamId() == Attackers)
                                player->UpdateAchievementCriteria(ACHIEVEMENT_CRITERIA_TYPE_BE_SPELL_TARGET, 65246);
                }

                EndTimestamp = 0;
                UpdateWorldState(BG_SA_ENABLE_TIMER, 0);
                UpdateWorldState(BG_SA_TIMER, 0);

                if (RoundScores[0].time == RoundScores[1].time)
                    EndBattleground(0);
                else if (RoundScores[0].time < RoundScores[1].time)
                    EndBattleground(RoundScores[0].winner == TEAM_ALLIANCE ? ALLIANCE : HORDE);
                else
                    EndBattleground(RoundScores[1].winner == TEAM_ALLIANCE ? ALLIANCE : HORDE);
            }
        }
    }
}

void BattlegroundSA::ToggleTimer()
{
    TimerEnabled = !TimerEnabled;
    UpdateWorldState(BG_SA_ENABLE_TIMER, (TimerEnabled) ? 1 : 0);
}

void BattlegroundSA::EndBattleground(uint32 winner)
{
    // Not Even a Scratch (1762 / 2192)
    if (_allVehiclesAlive[winner == ALLIANCE ? TEAM_ALLIANCE : TEAM_HORDE])
        GetBgMap()->SetWorldState(winner == ALLIANCE ? WORLD_STATE_NOT_EVEN_A_SCRATCH_A : WORLD_STATE_NOT_EVEN_A_SCRATCH_H, 1);

    // honor reward for winning
    if (winner == ALLIANCE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), ALLIANCE);
    else if (winner == HORDE)
        RewardHonorToTeam(GetBonusHonorFromKill(1), HORDE);

    //complete map_end rewards (even if no team wins)
    RewardHonorToTeam(GetBonusHonorFromKill(2), ALLIANCE);
    RewardHonorToTeam(GetBonusHonorFromKill(2), HORDE);

    Battleground::EndBattleground(winner);
}

void BattlegroundSA::UpdateDemolisherSpawns()
{
    for (uint8 i = BG_SA_DEMOLISHER_1; i <= BG_SA_DEMOLISHER_8; i++)
    {
        if (i >= BG_SA_DEMOLISHER_1 && i <= BG_SA_DEMOLISHER_2 && GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] == Attackers)
            continue;
        if (i >= BG_SA_DEMOLISHER_3 && i <= BG_SA_DEMOLISHER_4 && GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] == Attackers)
            continue;

        if (BgCreatures[i])
        {
            if (Creature* Demolisher = GetBGCreature(i))
            {
                if (Demolisher->isDead())
                {
                    // Demolisher is not in list
                    if (DemoliserRespawnList.find(i) == DemoliserRespawnList.end())
                    {
                        DemoliserRespawnList[i] = getMSTime()+30000;
                    }
                    else
                    {
                        if (DemoliserRespawnList[i] < getMSTime())
                        {
                            Demolisher->Relocate(BG_SA_NpcSpawnlocs[i][0], BG_SA_NpcSpawnlocs[i][1],
                                BG_SA_NpcSpawnlocs[i][2], BG_SA_NpcSpawnlocs[i][3]);

                            Demolisher->Respawn();
                            DemoliserRespawnList.erase(i);
                        }
                    }
                }
            }
        }
    }
}

void BattlegroundSA::UpdateBombSpawns()
{
    for (uint8 i = BG_SA_BOMB; i < BG_SA_MAXOBJ; i++)
    {
        if (GetBGObject(i))
            continue;

        if (bombsLocationWest.GetExactDist2d(BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1]) < 50.0f && GraveyardStatus[BG_SA_LEFT_CAPTURABLE_GY] != Attackers)
            continue;
        if (bombsLocationEast.GetExactDist2d(BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1]) < 50.0f && GraveyardStatus[BG_SA_RIGHT_CAPTURABLE_GY] != Attackers)
            continue;
        if (bombsLocationSouth.GetExactDist2d(BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1]) < 50.0f && GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] != Attackers)
            continue;
        if (bombsLocationRelic.GetExactDist2d(BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1]) < 50.0f && GraveyardStatus[BG_SA_CENTRAL_CAPTURABLE_GY] != Attackers) // Which graveyard?
            continue;

        AddObject(i, BG_SA_ObjEntries[BG_SA_BOMB],
            BG_SA_ObjSpawnlocs[i][0], BG_SA_ObjSpawnlocs[i][1],
            BG_SA_ObjSpawnlocs[i][2], BG_SA_ObjSpawnlocs[i][3],
            0,0,0,0,RESPAWN_ONE_DAY);
        GetBGObject(i)->SetUInt32Value(GAMEOBJECT_FIELD_FACTION_TEMPLATE, BG_SA_Factions[Attackers]);
    }
}

void BattlegroundSA::SendTransportInit(Player* player)
{
    if (BgObjects[BG_SA_BOAT_ONE] ||  BgObjects[BG_SA_BOAT_TWO])
    {
        UpdateData transData(player->GetMapId());
        if (BgObjects[BG_SA_BOAT_ONE])

            GetBGObject(BG_SA_BOAT_ONE)->BuildCreateUpdateBlockForPlayer(&transData, player);
        if (BgObjects[BG_SA_BOAT_TWO])
            GetBGObject(BG_SA_BOAT_TWO)->BuildCreateUpdateBlockForPlayer(&transData, player);
        WorldPacket packet;
        transData.BuildPacket(&packet);
        player->SendDirectMessage(&packet);

        // #851: this player's client (re)starts both boats from the sea now (MoP 5.4.8 boats, see LandBoatRiders)
        if (!BoatsUseStopFrames())
            _boatInitAt[player->GetGUID()] = getMSTime();
    }
}

void BattlegroundSA::SendTransportsRemove(Player* player)
{
    if (BgObjects[BG_SA_BOAT_ONE] ||  BgObjects[BG_SA_BOAT_TWO])
    {
        UpdateData transData(player->GetMapId());
        if (BgObjects[BG_SA_BOAT_ONE])
            GetBGObject(BG_SA_BOAT_ONE)->BuildOutOfRangeUpdateBlock(&transData);
        if (BgObjects[BG_SA_BOAT_TWO])
            GetBGObject(BG_SA_BOAT_TWO)->BuildOutOfRangeUpdateBlock(&transData);
        WorldPacket packet;
        transData.BuildPacket(&packet);
        player->SendDirectMessage(&packet);
    }
}

void BattlegroundSA::SendGateDestroyed(uint32 text, uint32 entry)
{
    auto go = sObjectMgr->GetGameObjectTemplate(entry);
    if (!go)
        return;

    std::string name = go->name;

    std::map<uint32, WorldPacket> localizedPackets;

    for (BattlegroundPlayerMap::const_iterator itr = m_Players.begin(); itr != m_Players.end(); ++itr)
        if (Player* player = ObjectAccessor::FindPlayer(itr->first))
        {
            if (localizedPackets.find(player->GetSession()->GetSessionDbLocaleIndex()) == localizedPackets.end())
            {
                char const* format = sObjectMgr->GetTrinityString(text, player->GetSession()->GetSessionDbLocaleIndex());

                if (auto locale = sObjectMgr->GetGameObjectLocale(entry))
                    sObjectMgr->GetLocaleString(locale->Name, player->GetSession()->GetSessionDbLocaleIndex(), name);

                char buf[256];
                sprintf(buf, format, name.c_str());

                ChatHandler::BuildChatPacket(localizedPackets[player->GetSession()->GetSessionDbLocaleIndex()], CHAT_MSG_RAID_BOSS_EMOTE, LANG_UNIVERSAL, NULL, NULL, buf);
            }

            player->SendDirectMessage(&localizedPackets[player->GetSession()->GetSessionDbLocaleIndex()]);
        }
}
