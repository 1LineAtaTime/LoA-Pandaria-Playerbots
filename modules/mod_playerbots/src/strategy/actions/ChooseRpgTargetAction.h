/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/ChooseRpgTargetAction.h)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 */

#ifndef PLAYERBOTS_CHOOSERPGTARGETACTION_H
#define PLAYERBOTS_CHOOSERPGTARGETACTION_H

#include "ObjectGuid.h"
#include "RpgAction.h"
#include "WorldPosition.h"
#include <string>
#include <unordered_map>

class GuidPosition;
class Player;
class PlayerbotAI;
class WorldObject;
class WorldPosition;

class ChooseRpgTargetAction : public Action
{
public:
    ChooseRpgTargetAction(PlayerbotAI* botAI, std::string const name = "choose rpg target") : Action(botAI, name) {}

    bool Execute(Event event) override;
    bool isUseful() override;

    static bool isFollowValid(Player* bot, WorldObject* target);
    static bool isFollowValid(Player* bot, WorldPosition pos);

private:
    float getMaxRelevance(GuidPosition guidP);
    bool  HasSameTarget(ObjectGuid guid, uint32 max, GuidVector const& nearGuids);

    std::unordered_map <ObjectGuid, std::string> rgpActionReason;
};

class ClearRpgTargetAction : public ChooseRpgTargetAction
{
public:
    ClearRpgTargetAction(PlayerbotAI* botAI) : ChooseRpgTargetAction(botAI, "clear rpg target") {}

    bool Execute(Event event) override;
    bool isUseful() override;
};

#endif
