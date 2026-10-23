/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/TravelAction.cpp) into the
 * Legends of Azeroth Pandaria fork, issue #269.
 */

#include "TravelAction.h"
#include "GridObject.h"  // must precede the grid notifiers (see NearestGameObjects.cpp)
#include "CellImpl.h"
#include "Event.h"
#include "GridNotifiers.h"
#include "GridNotifiersImpl.h"
#include "Opcodes.h"
#include "Playerbots.h"
#include "TravelMgr.h"  // 5.4.8: TravelTarget is not pulled in by Playerbots.h on this fork (see strategy/triggers/RpgTriggers.cpp)
#include "WorldPacket.h"

// AC TravelAction.cpp:13-55. 5.4.8 divergence: the searcher helpers are spelled Trinity:: not Acore::
// (see strategy/value/NearestNpcsValue.cpp:20-21), sPlayerbotAIConfig is a pointer on this fork, and
// Cell::VisitObjects (AC) is Cell::VisitAllObjects here (core Grids/Cells/Cell.h:115, same call as
// strategy/value/NearestNpcsValue.cpp:22). IsDuringRemoveFromWorld is core Unit.h:2500.
bool TravelAction::Execute(Event /*event*/)
{
    TravelTarget* target = AI_VALUE(TravelTarget*, "travel target");

    if (bot->isMoving())
        return false;

    target->setStatus(TRAVEL_STATUS_WORK);

    Unit* newTarget = nullptr;
    std::list<Unit*> targets;
    Trinity::AnyUnitInObjectRangeCheck u_check(bot, sPlayerbotAIConfig->sightDistance * 2);
    Trinity::UnitListSearcher<Trinity::AnyUnitInObjectRangeCheck> searcher(bot, targets, u_check);
    Cell::VisitAllObjects(bot, searcher, sPlayerbotAIConfig->sightDistance);

    for (Unit* unit : targets)
    {
        newTarget = unit;
        if (!newTarget || !newTarget->IsInWorld() || newTarget->IsDuringRemoveFromWorld())
            continue;

        if (newTarget->GetMapId() != bot->GetMapId())
            continue;

        if (!newTarget->IsAlive())
            continue;

        if (newTarget->GetEntry() == uint32(target->getDestination()->getEntry()))
            continue;

        if (newTarget->IsInCombat())
            continue;

        if (bot->IsHostileTo(newTarget))
            SET_AI_VALUE(ObjectGuid, "pull target", newTarget->GetGUID());
        else
            SET_AI_VALUE(GuidPosition, "rpg target", GuidPosition(newTarget));

        break;
    }

    return true;
}

// AC TravelAction.cpp:57-61 - kept verbatim, including the `false &&` that makes it dead in AC too.
bool TravelAction::isUseful()
{
    return false && AI_VALUE(TravelTarget*, "travel target")->isActive() &&
           (!AI_VALUE(GuidPosition, "rpg target") || !AI_VALUE(ObjectGuid, "pull target"));
}

// AC Bot/RandomPlayerbotMgr.cpp:3162-3172 RandomPlayerbotMgr::GetCreatureDataByEntry, reproduced as a
// file-local helper because the fork's Manager/RandomPlayerbotMgr.h has no such method. Same body over the
// 5.4.8 spawn store (core Globals/ObjectMgr.h:1499 GetAllCreatureData(), CreatureData::id at
// Creature.h:79; the same whole-store walk as Travel/TravelMgr.cpp:366).
static CreatureData const* GetCreatureDataByEntry(uint32 entry)
{
    if (entry != 0)
    {
        for (auto const& itr : sObjectMgr->GetAllCreatureData())
            if (itr.second.id == entry)
                return &itr.second;
    }

    return nullptr;
}

// AC TravelAction.cpp:63-111. 5.4.8 divergences: GET_PLAYERBOT_AI(bot)->GetGroupLeader() ->
// GetGroupMaster() (same mapping as Manager/PlayerbotSecurity.cpp:10, AI/PlayerbotAI.cpp:2438);
// sRandomPlayerbotMgr.GetCreatureDataByEntry -> the static helper above;
// GetCreatureBySpawnIdStore() is core Maps/Map.h:518 (multimap keyed by spawnId, CreatureData::spawnId
// Creature.h:78); Player::AddQuest(Quest const*, Object*) is core Player.h:1714.
bool MoveToDarkPortalAction::Execute(Event /*event*/)
{
    if (bot->GetGroup())
    {
        if (bot->GetGroup()->GetLeaderGUID() != bot->GetGUID() &&
            !GET_PLAYERBOT_AI(GET_PLAYERBOT_AI(bot)->GetGroupMaster()))
            return false;
    }

    if (bot->GetLevel() > 57)
    {
        if ((bot->GetTeamId() == TEAM_ALLIANCE && bot->GetQuestStatus(10119) == QUEST_STATUS_NONE) ||
            (bot->GetTeamId() == TEAM_HORDE && bot->GetQuestStatus(9407) == QUEST_STATUS_NONE))
        {
            if (!bot->IsInCombat())
            {
                if (bot->GetTeamId() == TEAM_ALLIANCE)
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(10119);
                    CreatureData const* creatureData = GetCreatureDataByEntry(16841);
                    if (quest && creatureData)
                    {
                        auto creatureBounds =
                            bot->GetMap()->GetCreatureBySpawnIdStore().equal_range(creatureData->spawnId);
                        if (creatureBounds.first != creatureBounds.second)
                            bot->AddQuest(quest, creatureBounds.first->second);
                    }
                }
                else
                {
                    Quest const* quest = sObjectMgr->GetQuestTemplate(9407);
                    CreatureData const* creatureData = GetCreatureDataByEntry(19254);
                    if (quest && creatureData)
                    {
                        auto creatureBounds =
                            bot->GetMap()->GetCreatureBySpawnIdStore().equal_range(creatureData->spawnId);
                        if (creatureBounds.first != creatureBounds.second)
                            bot->AddQuest(quest, creatureBounds.first->second);
                    }
                }
            }

            return MoveTo(0, -11815.1f, -3190.39f, -30.7338f, false, true);
        }

        return MoveTo(0, -11906.9f, -3208.53f, -14.8616f, false, true);
    }

    return false;
}

bool MoveToDarkPortalAction::isUseful() { return bot->GetLevel() > 54; }

// AC TravelAction.cpp:116-127. 5.4.8 divergence: AC uses GET_PLAYERBOT_AI(bot) (an AzerothCore
// macro that does not exist in this fork); the fork's own idiom for reaching the bot's own AI from
// an action is the `botAI` member (see BotScenario.cpp:606, BuyAction.cpp:225).
bool DarkPortalAzerothAction::Execute(Event /*event*/)
{
    if (bot->GetLevel() > 57)
    {
        WorldPacket packet(CMSG_AREATRIGGER);
        packet << 4354;

        return botAI->DoSpecificAction("reach area trigger", Event("travel action", packet));
    }

    return false;
}

bool DarkPortalAzerothAction::isUseful() { return bot->GetLevel() > 57; }

// AC TravelAction.cpp:131-138.
bool MoveFromDarkPortalAction::Execute(Event /*event*/)
{
    RESET_AI_VALUE(GuidPosition, "rpg target");

    if (bot->GetTeamId() == TEAM_ALLIANCE)
        return MoveTo(530, -319.261f, 1027.213, 54.172638f, false, true);

    return MoveTo(530, -180.444f, 1027.947, 54.181538f, false, true);
}