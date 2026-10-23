/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from the mod-playerbots module for AzerothCore (Ai/Base/Actions/ChooseRpgTargetAction.cpp)
 * into the Legends of Azeroth Pandaria fork, issue #272.
 *
 * 5.4.8 / this-fork adaptations vs AC (each is also marked at the site):
 *  - getMaxRelevance walks TriggerNode handlers: AC `std::vector<NextAction> getHandlers()`, here the old
 *    `NextAction**` array API (strategy/Trigger.h:56, a fresh owned array each call, destroyed with
 *    NextAction::destroy). getFirstRelevance() exists (strategy/Trigger.h:58).
 *  - botAI->HasGameClientMaster() -> botAI->HasRealPlayerMaster() (AI/PlayerbotAI.cpp:2261);
 *    botAI->GetGroupLeader() -> botAI->GetGroupMaster() (AI/PlayerbotAI.cpp:2438);
 *    AC free IsRealPlayer(botAI->GetMaster()) -> botAI->HasActivePlayerMaster() (AI/PlayerbotAI.cpp:2271).
 *  - TravelMgr::instance().weighted_shuffle -> sTravelMgr.weighted_shuffle (Travel/TravelMgr.h:795; the template
 *    itself is being added to TravelMgr.h by another lane, AC TravelMgr.h:886-901).
 *  - chat->FormatWorldobject -> RpgFormatWorldobject (RpgAction.h).
 *  - sPlayerbotAIConfig.x -> sPlayerbotAIConfig->x ; LOG_DEBUG -> TC_LOG_DEBUG printf style.
 *  - AC includes "GuildCreateActions.h" (not on this fork, nothing from it is used) - dropped.
 *  - Carried AC quirks: HasSameTarget is never called (its call site is commented out in AC); getMaxRelevance
 *    returns before restoring "rpg target" when "rpg" strategy is missing (AC line `if (!rpgStrategy) return 0.0f;`).
 */

#include "ChooseRpgTargetAction.h"
#include "BudgetValues.h"
#include "ChatHelper.h"
#include "Event.h"
#include "Formations.h"
#include "Playerbots.h"
#include "PossibleRpgTargetsValue.h"
#include "RpgSubActions.h"
#include "ServerFacade.h"
#include "GameObject.h"
#include "Group.h"
#include "Map.h"
#include "ObjectAccessor.h"
#include "TravelMgr.h"
#include "WorldPosition.h"
#include <random>

bool ChooseRpgTargetAction::HasSameTarget(ObjectGuid guid, uint32 max, GuidVector const& nearGuids)
{
    if (botAI->HasRealPlayerMaster())
        return false;

    uint32 num = 0;

    for (auto& i : nearGuids)
    {
        Player* player = ObjectAccessor::FindPlayer(i);
        if (!player)
            continue;

        PlayerbotAI* botAI = GET_PLAYERBOT_AI(player);
        if (!botAI)
            continue;

        if (!botAI->AllowActivity(GRIND_ACTIVITY))
            continue;

        if (PAI_VALUE(GuidPosition, "rpg target") != guid)
            continue;

        num++;
        if (num >= max)
            break;
    }

    return num > 0;
}

float ChooseRpgTargetAction::getMaxRelevance(GuidPosition guidP)
{
    GuidPosition currentRpgTarget = AI_VALUE(GuidPosition, "rpg target");
    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    Strategy* rpgStrategy = botAI->GetAiObjectContext()->GetStrategy("rpg");
    if (!rpgStrategy) return 0.0f;

    std::vector<TriggerNode*> triggerNodes;
    rpgStrategy->InitTriggers(triggerNodes);

    float maxRelevance = 0.0f;

    for (auto triggerNode : triggerNodes)
    {
        Trigger* trigger = context->GetTrigger(triggerNode->getName());

        if (trigger)
        {
            triggerNode->setTrigger(trigger);

            if (triggerNode->getFirstRelevance() < maxRelevance || triggerNode->getFirstRelevance() > 2.0f)
                continue;

            // AC redeclares `Trigger* trigger = triggerNode->getTrigger();` here (same pointer as the outer one, set
            // above); dropped to avoid the shadowing warning.

            if (!trigger->IsActive())
                continue;

            // 5.4.8: NextAction** (old API) instead of AC's std::vector<NextAction>; owned, destroyed below.
            NextAction** nextActions = triggerNode->getHandlers();

            bool isRpg = false;

            for (uint32 i = 0; nextActions && nextActions[i]; ++i)
            {
                NextAction* nextAction = nextActions[i];
                Action* action = botAI->GetAiObjectContext()->GetAction(nextAction->getName());

                if (dynamic_cast<RpgEnabled*>(action))
                    isRpg = true;
            }

            NextAction::destroy(nextActions);

            if (isRpg)
            {
                maxRelevance = triggerNode->getFirstRelevance();
                rgpActionReason[guidP] = triggerNode->getName();
            }
        }
    }

    for (auto trigger : triggerNodes)
    {
        delete trigger;
    }
    triggerNodes.clear();

    SET_AI_VALUE(GuidPosition, "rpg target", currentRpgTarget);

    if (!maxRelevance)
        return 0.0;

    return floor((maxRelevance - 1.0) * 1000.0f);
}

bool ChooseRpgTargetAction::Execute(Event /*event*/)
{
    Player* master = botAI->GetMaster();
    GuidPosition masterRpgTarget;
    if (master && master != bot && GET_PLAYERBOT_AI(master) && master->GetMapId() == bot->GetMapId() && !master->IsBeingTeleported())
    {
        Player* player = master;
        masterRpgTarget = PAI_VALUE(GuidPosition, "rpg target");
    }
    else
        master = nullptr;

    std::unordered_map<ObjectGuid, uint32> targets;
    GuidVector possibleTargets = AI_VALUE(GuidVector, "possible rpg targets");
    GuidVector possibleObjects = AI_VALUE(GuidVector, "nearest game objects no los");
    GuidVector possiblePlayers = AI_VALUE(GuidVector, "nearest friendly players");
    GuidSet& ignoreList = AI_VALUE(GuidSet&, "ignore rpg target");

    for (auto target : possibleTargets)
        targets[target] = 0.0f;

    for (auto target : possibleObjects)
        targets[target] = 0.0f;

    for (auto target : possiblePlayers)
        targets[target] = 0.0f;

    if (targets.empty())
    {
        return false;
    }

    if (urand(0, 9))
    {
        for (auto target : ignoreList)
            targets.erase(target);
    }

    SET_AI_VALUE(std::string, "next rpg action", this->getName());

    bool hasGoodRelevance = false;

    for (auto& target : targets)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, target.first);
        if (!unit)
            continue;

        GuidPosition guidP(unit);
        if (!guidP || !guidP.getMap())
            continue;

        // float priority = 1; //not used, line marked for removal.

        if (guidP.GetWorldObject() && !isFollowValid(bot, guidP.GetWorldObject()))
            continue;

        if (guidP.IsGameObject())
        {
            GameObject* go = guidP.GetGameObject();
            if (!go || !go->isSpawned() || go->GetGoState() != GO_STATE_READY)
                continue;
        }
        else if (guidP.IsPlayer())
        {
            Player* player = guidP.GetPlayer();
            if (!player)
                continue;

            if (GET_PLAYERBOT_AI(player))
            {
                GuidPosition guidPP = PAI_VALUE(GuidPosition, "rpg target");
                if (guidPP.IsPlayer())
                {
                    continue;
                }
            }
        }

        // if (possiblePlayers.size() > 200 || HasSameTarget(guidP, urand(5, 15), possiblePlayers))
        //     continue;

        float relevance = getMaxRelevance(guidP);

        if (!hasGoodRelevance || relevance > 1)
            target.second = relevance;

        if (target.second > 1)
            hasGoodRelevance = true;
    }

    SET_AI_VALUE(std::string, "next rpg action", "");

    // Only fires when following a playerbot master.
    for (auto it = begin(targets); it != end(targets);)
    {
        //Remove empty targets.
        if (it->second == 0)
            it = targets.erase(it);
        //Remove useless targets if there's any good ones
        else if (hasGoodRelevance && it->second <= 1.0)
            it = targets.erase(it);
        //Remove useless targets if it's not masters target.
        else if (!hasGoodRelevance && master && (!masterRpgTarget || it->first != masterRpgTarget))
            it = targets.erase(it);
        else
            ++it;
    }

    if (targets.empty())
    {
        TC_LOG_DEBUG("playerbots", "%s can't choose RPG target: all %u targets are not available", bot->GetName().c_str(),
                     static_cast<uint32>(possibleTargets.size()));
        RESET_AI_VALUE(GuidSet&, "ignore rpg target");
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    std::vector<GuidPosition> guidps;
    std::vector<int32> relevances;

    for (auto& target : targets)
    {
        Unit* unit = ObjectAccessor::GetUnit(*bot, target.first);
        if (!unit)
            continue;

        GuidPosition guidP(unit);
        if (!guidP)
            continue;

        guidps.push_back(guidP);
        relevances.push_back(target.second);
    }

    std::mt19937 gen(time(0));
    sTravelMgr.weighted_shuffle(guidps.begin(), guidps.end(), relevances.begin(), relevances.end(), gen);

    GuidPosition guidP(guidps.front());
    if (!guidP)
    {
        RESET_AI_VALUE(GuidPosition, "rpg target");
        return false;
    }

    if (botAI->HasStrategy("debug", BOT_STATE_NON_COMBAT) && guidP.GetWorldObject())
    {
        std::ostringstream out;
        out << "found: ";
        out << RpgFormatWorldobject(guidP.GetWorldObject());
        out << " " << relevances.front();

        botAI->TellMasterNoFacing(out);
    }

    SET_AI_VALUE(GuidPosition, "rpg target", guidP);

    return true;
}

bool ChooseRpgTargetAction::isUseful()
{
    if (!botAI->AllowActivity(RPG_ACTIVITY))
        return false;

    GuidPosition guidP = AI_VALUE(GuidPosition, "rpg target");

    if (guidP && guidP.distance(bot) < sPlayerbotAIConfig->reactDistance * 2)
        return false;

    // TravelTarget* travelTarget = AI_VALUE(TravelTarget*, "travel target"); //not used, line marked for removal.

    //if (travelTarget->isTraveling() && AI_VALUE2(bool, "can free move to", *travelTarget->getPosition()))
        //return false;

    if (AI_VALUE(GuidVector, "possible rpg targets").empty())
        return false;

    //Not stay, not guard, not combat, not trading and group ready.
    if (!AI_VALUE(bool, "can move around"))
        return false;

    return true;
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldObject* target)
{
    if (!target)
        return false;

    return isFollowValid(bot, WorldPosition(target));
}

bool ChooseRpgTargetAction::isFollowValid(Player* bot, WorldPosition pos)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    Player* groupLeader = botAI->GetGroupMaster();
    Player* realMaster = botAI->GetMaster();
    AiObjectContext* context = botAI->GetAiObjectContext();

    bool inDungeon = false;

    if (botAI->HasActivePlayerMaster())
    {
        if (realMaster->IsInWorld() && realMaster->GetMap()->IsDungeon() && bot->GetMapId() == realMaster->GetMapId())
            inDungeon = true;

        if (realMaster && realMaster->IsInWorld() && realMaster->GetMap()->IsDungeon() &&
            (realMaster->GetMapId() != pos.GetMapId()))
            return false;
    }

    if (!groupLeader || bot == groupLeader)
        return true;

    if (!botAI->HasStrategy("follow", BOT_STATE_NON_COMBAT))
        return true;

    if (bot->GetDistance(groupLeader) > sPlayerbotAIConfig->rpgDistance * 2)
        return false;

    Formation* formation = AI_VALUE(Formation*, "formation");
    float distance = groupLeader->GetDistance2d(pos.GetPositionX(), pos.GetPositionY());

    if (!botAI->HasActivePlayerMaster() && distance < 50.0f)
    {
        Player* player = groupLeader;
        if ((groupLeader && !groupLeader->isMoving()) ||
            PAI_VALUE(WorldPosition, "last long move").distance(pos) < sPlayerbotAIConfig->reactDistance)
            return true;
    }

    if ((inDungeon || !groupLeader->HasPlayerFlag(PLAYER_FLAGS_RESTING)) && realMaster == groupLeader && distance > 5.0f)
        return false;

    if (!groupLeader->isMoving() && distance < 25.0f)
        return true;

    if (distance < formation->GetMaxDistance())
        return true;

    return false;
}
