/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/World/Rpg/Action/NewRpgAction.h)
 * into the Legends of Azeroth Pandaria fork.
 */

#ifndef _PLAYERBOT_NEWRPGACTION_H
#define _PLAYERBOT_NEWRPGACTION_H

#include "Duration.h"
#include "MovementActions.h"
#include "NewRpgBaseAction.h"
#include "NewRpgInfo.h"
#include "NewRpgStrategy.h"
#include "ObjectGuid.h"
#include "OutdoorPvP.h"
#include "PlayerbotAI.h"
#include "WorldPosition.h"

#include <string>

class Player;

// #536: AC Ai/World/Rpg/Action/NewRpgAction.h:25-44 - the "rpg status" / "rpg do quest" chat commands.
class TellRpgStatusAction : public NewRpgBaseAction
{
public:
    TellRpgStatusAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "rpg status") {}

    bool Execute(Event event) override;

private:
    static constexpr char const* RPG_STATUS_CHANGED_KEY = "rpg_status_changed";
    static constexpr char const* RPG_STATUS_CHANGED_DEFAULT = "rpg status -> %status";

    void WhisperStatusChange(Player* owner, std::string const& statusName);
};

class StartRpgDoQuestAction : public Action
{
public:
    StartRpgDoQuestAction(PlayerbotAI* botAI) : Action(botAI, "start rpg do quest") {}

    bool Execute(Event event) override;
};

class NewRpgStatusUpdateAction : public NewRpgBaseAction
{
public:
    NewRpgStatusUpdateAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg status update") {}
    bool Execute(Event event) override;

protected:
    const int32 statusWanderRandomDuration = 5 * MINUTE * IN_MILLISECONDS;
    // AC parity (issue #61): upstream NewRpgAction.h:65 uses 5 minutes, not 15 as the previous
    // comment here claimed.
    const int32 statusWanderNpcDuration = 5 * MINUTE * IN_MILLISECONDS;
    const int32 statusDoQuestDuration = 30 * MINUTE * IN_MILLISECONDS;
    const int32 statusRestDuration = 30 * IN_MILLISECONDS;
    // AC parity (issue #61): AC NewRpgAction.h:69 uses 1 hour.
    const int32 statusOutDoorPvPDuration = HOUR * IN_MILLISECONDS;
    // #201 (#612): a natural (not seeded) duel-spot visit lasts 30 minutes (DESIGN 4.4; AC NewRpgAction.h:69 pattern).
    const int32 statusDuelSpotDuration = 30 * MINUTE * IN_MILLISECONDS;
};

class NewRpgGoGrindAction : public NewRpgBaseAction
{
public:
    NewRpgGoGrindAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go grind") {}
    bool Execute(Event event) override;
};

class NewRpgGoCampAction : public NewRpgBaseAction
{
public:
    NewRpgGoCampAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg go camp") {}
    bool Execute(Event event) override;
};

/// Drive the bot to the outdoor-pvp capture points of whatever OutdoorPvP event owns the current
/// zone, then patrol around the one it picked until the capture bar flips or the status expires.
class NewRpgOutdoorPvpAction : public NewRpgBaseAction
{
public:
    NewRpgOutdoorPvpAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg outdoor pvp") {}

    bool Execute(Event event) override;
    OPvPCapturePoint* SelectNewObjective(OutdoorPvP::OPvPCapturePointMap const& capturePointMap);

protected:
    bool PatrolCapturePoint(GameObject* objectiveGO, float radius);
};

class NewRpgWanderRandomAction : public NewRpgBaseAction
{
public:
    NewRpgWanderRandomAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg wander random") {}
    bool Execute(Event event) override;
};

/// Walk from interactable NPC to interactable NPC, stopping at each for a while and taking or
/// handing in quests. This is the behaviour that makes cities and camps look inhabited.
class NewRpgWanderNpcAction : public NewRpgBaseAction
{
public:
    NewRpgWanderNpcAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg wander npc") {}
    bool Execute(Event event) override;

protected:
    const uint32 npcStayTime = 8 * IN_MILLISECONDS;
};

/// Work a quest in the log: head for the quest POI of an incomplete objective, stay there while
/// the grind/loot strategies make progress, then head back to hand it in.
class NewRpgDoQuestAction : public NewRpgBaseAction
{
public:
    NewRpgDoQuestAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg do quest") {}
    bool Execute(Event event) override;

protected:
    bool DoIncompleteQuest(NewRpgInfo::DoQuest& data);
    bool DoCompletedQuest(NewRpgInfo::DoQuest& data);

    const uint32 poiStayTime = 5 * MINUTE * IN_MILLISECONDS;
};

/// Walk to a flight master and take a real taxi to a level-appropriate zone.
class NewRpgTravelFlightAction : public NewRpgBaseAction
{
public:
    NewRpgTravelFlightAction(PlayerbotAI* botAI) : NewRpgBaseAction(botAI, "new rpg travel flight") {}
    bool Execute(Event event) override;

protected:
    /// A taxi path whose next hop is on another map does not continue on its own for a bot: the
    /// client normally triggers the map change. Do it here instead.
    void ContinueCrossMapTaxi();
};

#endif
