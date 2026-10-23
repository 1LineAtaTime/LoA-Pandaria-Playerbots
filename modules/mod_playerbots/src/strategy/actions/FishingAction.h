/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/FishingAction.h (issue #412; #375 is the same gap):
 * FishingAction, EquipFishingPoleAction, MoveNearWaterAction, UseBobberAction, EndMasterFishingAction
 * and RemoveBobberStrategyAction. Declarations are 1:1; see FishingAction.cpp for the MoP API
 * adaptations (liquid query, equip path, fishing spell id, config constants).
 */

#ifndef _PLAYERBOT_FISHINGACTION_H
#define _PLAYERBOT_FISHINGACTION_H

#include "Action.h"
#include "Event.h"
#include "MovementActions.h"
#include "Playerbots.h"
#include "WorldPosition.h"

// AC FISHING_BOBBER (35591, "Fishing Bobber"); same entry used by the fork's fishing code path.
extern uint32 const FISHING_BOBBER;
// AC FISHING_POLE (6256, "Fishing Pole") is the item given to random bots that have none.
extern uint32 const FISHING_POLE;
// AC sPlayerbotAIConfig.fishingDistanceFromMaster (AiPlayerbot.FishingDistanceFromMaster, default 10,
// AC PlayerbotAIConfig.cpp:291). Not config-backed on this fork; defined in FishingAction.cpp. Exported for
// AC FollowActions.cpp:287-288 (follow only beyond this distance while "master fishing" is on, #412).
extern float const kFishingDistanceFromMaster;

WorldPosition FindWaterRadial(Player* bot, float x, float y, float z, Map* map, uint32 phaseMask, float minDistance,
                              float maxDistance, float increment, bool checkLOS = false, int numDirections = 16);

class PlayerbotAI;

class FishingAction : public Action
{
public:
    FishingAction(PlayerbotAI* botAI) : Action(botAI, "go fishing") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class EquipFishingPoleAction : public Action
{
public:
    EquipFishingPoleAction(PlayerbotAI* botAI) : Action(botAI, "equip fishing pole") {}

    bool Execute(Event event) override;
    bool isUseful() override;

private:
    Item* _pole = nullptr;
};

class MoveNearWaterAction : public MovementAction
{
public:
    MoveNearWaterAction(PlayerbotAI* botAI) : MovementAction(botAI, "move near water") {}

    bool Execute(Event event) override;
    bool isUseful() override;
    bool isPossible() override;
};

class UseBobberAction : public Action
{
public:
    UseBobberAction(PlayerbotAI* botAI) : Action(botAI, "use fishing bobber") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class EndMasterFishingAction : public Action
{
public:
    EndMasterFishingAction(PlayerbotAI* botAI) : Action(botAI, "end master fishing") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

class RemoveBobberStrategyAction : public Action
{
public:
    RemoveBobberStrategyAction(PlayerbotAI* botAI) : Action(botAI, "remove bobber strategy") {}

    bool Execute(Event event) override;
};

#endif
