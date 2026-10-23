/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/NewRpgBaseAction.h)
 * into the Legends of Azeroth Pandaria fork. Quest/NPC interaction helpers are not ported here;
 * only the runtime roaming movement and status-change helpers are implemented.
 */

#ifndef _PLAYERBOT_NEWRPGBASEACTION_H
#define _PLAYERBOT_NEWRPGBASEACTION_H

#include "LastMovementValue.h"
#include "MovementActions.h"
#include "NewRpgInfo.h"
#include "ObjectGuid.h"
#include "PlayerbotAI.h"
#include "Object.h"
#include "ObjectGuid.h"
#include "QuestDef.h"
#include "TravelMgr.h"
#include "WorldPosition.h"

#include <vector>

class Quest;
class WorldObject;

/// One quest-POI blob reduced to a position plus the objective it belongs to
/// (-1 for the hand-in location).
struct POIInfo
{
    G3D::Vector2 pos;
    int32 objectiveIdx;
};

/// A base (composition) class for all new rpg actions.
/// All functions that may be shared by multiple actions are declared here.
class NewRpgBaseAction : public MovementAction
{
public:
    NewRpgBaseAction(PlayerbotAI* botAI, std::string const name) : MovementAction(botAI, name) {}

protected:
    /* MOVEMENT RELATED */
    bool MoveFarTo(WorldPosition dest);
    bool MoveWorldObjectTo(ObjectGuid guid, float distance = INTERACTION_DISTANCE);
    bool MoveRandomNear(float moveStep = 50.0f, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    bool ForceToWait(uint32 duration, MovementPriority priority = MovementPriority::MOVEMENT_NORMAL);
    /// #218 (beyond AC, MoP only - see NewRpgRoamTransport.h): follow a travel-graph route with transport /
    /// flight / portal legs. Returns -1 when it does not handle this move (AC's MoveFarTo code runs), else 0/1
    /// as the move result. mayStart: build a route if no trip runs for dest; needSpecialLeg: only start a trip
    /// whose route has a transport, flight-path or portal leg.
    int32 RoamTransportMove(WorldPosition const& dest, bool mayStart, bool needSpecialLeg);

    /* QUEST RELATED CHECK */
    ObjectGuid ChooseNpcOrGameObjectToInteract(bool questgiverOnly = false, float distanceLimit = 0.0f);
    bool HasQuestToAcceptOrReward(WorldObject* object);
    bool InteractWithNpcOrGameObjectForQuest(ObjectGuid guid);
    bool CanInteractWithQuestGiver(Object* questGiver);
    bool IsWithinInteractionDist(Object* object);
    uint32 BestRewardIndex(Quest const* quest);
    bool IsQuestWorthDoing(Quest const* quest);
    bool IsQuestCapableDoing(Quest const* quest);

    /* QUEST RELATED ACTION */
    bool SearchQuestGiverAndAcceptOrReward();
    bool AcceptQuest(Quest const* quest, ObjectGuid guid);
    bool TurnInQuest(Quest const* quest, ObjectGuid guid);
    bool OrganizeQuestLog();
    bool GetQuestPOIPosAndObjectiveIdx(uint32 questId, std::vector<POIInfo>& poiInfo, bool toComplete = false);

    /* STATUS RELATED */
    bool RandomChangeStatus(std::vector<NewRpgStatus> candidateStatus);
    bool CheckRpgStatusAvailable(NewRpgStatus status);

    /* DESTINATION SELECTION */
    static WorldPosition SelectRandomGrindPos(Player* bot);
    static WorldPosition SelectRandomCampPos(Player* bot);
    bool SelectRandomFlightTaxiNode(uint32& flightMasterEntry, WorldPosition& flightMasterPos,
                                    std::vector<uint32>& path);

protected:
    /* FOR MOVE FAR */
    const float pathFinderDis = 70.0f;
    // Time without real progress toward dest before MoveFarTo
    // falls back to teleport recovery. Kept short enough that a
    // bot truly oscillating around an unreachable destination
    // (mmap returning non-progressing partial paths, or NOPATH +
    // cone fallback wandering) doesn't spin for 5 minutes before
    // the teleport fires, but long enough that a genuine long
    // walk that is slowly making progress never triggers it.
    const uint32 stuckTime = 90 * 1000;

};

#endif
