/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/GoAction.cpp. 5.4.8 / fork adaptations:
 *  - ChatHelper::parseGameobjects -> ChatLinkParse::ParseGameobjects and ChatHelper::FormatGameobject ->
 *    RpgFormatWorldobject (same string; see UseAction.cpp), the helpers this fork keeps outside ChatHelper.
 *  - Map2ZoneCoordinates / Zone2MapCoordinates are DBCManager methods (sDBCManager).
 *  - Map::IsInWater() takes no collision height on this core (AC passes bot->GetCollisionHeight()).
 *  - sPlayerbotAIConfig and sServerFacade are pointers; TravelMgr::instance() is a reference.
 *  - The "debug move" strategy branch (AC GoAction.cpp:127-160) still reports the computed path to the master but does
 *    not spawn the waypoint markers (AC CreateWp over every path point): the marker helper is not part of this module.
 */

#include "GoAction.h"

#include "ChatLinkParse.h"
#include "ChooseTravelTargetAction.h"
#include "DBCStores.h"
#include "Event.h"
#include "Formations.h"
#include "GameObject.h"
#include "Helper.h"
#include "PathGenerator.h"
#include "Playerbots.h"
#include "PositionValue.h"
#include "RpgAction.h"
#include "ServerFacade.h"

char* strstri(char const* haystack, char const* needle);

bool GoAction::Execute(Event event)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    std::string const param = event.getParam();
    if (param == "?")
    {
        float x = bot->GetPositionX();
        float y = bot->GetPositionY();
        sDBCManager.Map2ZoneCoordinates(x, y, bot->GetZoneId());

        std::ostringstream out;
        out << "I am at " << x << "," << y;
        botAI->TellMaster(out.str());
        return true;
    }

    if (param.find("travel") != std::string::npos && param.size() > 7)
    {
        WorldPosition botPos(bot);

        std::string const destination = param.substr(7);

        TravelTarget* target = context->GetValue<TravelTarget*>("travel target")->Get();

        if (TravelDestination* dest = ChooseTravelTargetAction::FindDestination(bot, destination))
        {
            std::vector<WorldPosition*> points = dest->nextPoint(const_cast<WorldPosition*>(&botPos), true);
            if (points.empty())
                return false;

            target->setTarget(dest, points.front());
            target->setForced(true);

            std::ostringstream out;
            out << "Traveling to " << dest->getTitle();
            botAI->TellMasterNoFacing(out.str());

            return true;
        }
        else
        {
            botAI->TellMasterNoFacing("Clearing travel target");
            target->setTarget(TravelMgr::instance().nullTravelDestination, TravelMgr::instance().nullWorldPosition);
            target->setForced(false);
            return true;
        }
    }

    GuidVector gos = ChatLinkParse::ParseGameobjects(param);
    if (!gos.empty())
    {
        for (ObjectGuid const guid : gos)
        {
            if (GameObject* go = botAI->GetGameObject(guid))
                if (go->isSpawned())
                {
                    if (sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, go),
                                                             sPlayerbotAIConfig->reactDistance))
                    {
                        botAI->TellError("It is too far away");
                        return false;
                    }

                    std::ostringstream out;
                    out << "Moving to " << RpgFormatWorldobject(go);
                    botAI->TellMasterNoFacing(out.str());
                    return MoveNear(bot->GetMapId(), go->GetPositionX(), go->GetPositionY(), go->GetPositionZ() + 0.5f,
                                    sPlayerbotAIConfig->followDistance);
                }
        }
        return false;
    }

    GuidVector units;
    GuidVector npcs = AI_VALUE(GuidVector, "nearest npcs");
    units.insert(units.end(), npcs.begin(), npcs.end());
    GuidVector players = AI_VALUE(GuidVector, "nearest friendly players");
    units.insert(units.end(), players.begin(), players.end());
    for (ObjectGuid const guid : units)
    {
        if (Unit* unit = botAI->GetUnit(guid))
            if (strstri(unit->GetName().c_str(), param.c_str()))
            {
                std::ostringstream out;
                out << "Moving to " << unit->GetName();
                botAI->TellMasterNoFacing(out.str());
                return MoveNear(bot->GetMapId(), unit->GetPositionX(), unit->GetPositionY(),
                                unit->GetPositionZ() + 0.5f, sPlayerbotAIConfig->followDistance);
            }
    }

    if (param.find(";") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ';');
        // AC reads coords[0] and coords[1] unchecked; a lone ";" would index past the end, so require two parts.
        if (coords.size() < 2)
            return false;

        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());
        float z;
        if (coords.size() > 2)
            z = atof(coords[2].c_str());
        else
            z = bot->GetPositionZ();

        if (botAI->HasStrategy("debug move", BOT_STATE_NON_COMBAT))
        {
            PathGenerator path(bot);

            path.CalculatePath(x, y, z, false);

            G3D::Vector3 end = path.GetEndPosition();
            G3D::Vector3 aend = path.GetActualEndPosition();

            Movement::PointsArray const& points = path.GetPath();
            PathType type = path.GetPathType();

            std::ostringstream out;

            out << x << ";" << y << ";" << z << " =";

            out << "path is: ";

            out << type;

            out << " of length ";

            out << points.size();

            out << " with offset ";

            out << (end - aend).length();

            botAI->TellMaster(out);
        }

        if (bot->IsWithinLOS(x, y, z))
            return MoveNear(bot->GetMapId(), x, y, z, 0);
        else
            return MoveTo(bot->GetMapId(), x, y, z, false, false);

        return true;
    }

    if (param.find(",") != std::string::npos)
    {
        std::vector<std::string> coords = split(param, ',');
        // Same guard as the ";" form above.
        if (coords.size() < 2)
            return false;

        float x = atof(coords[0].c_str());
        float y = atof(coords[1].c_str());
        sDBCManager.Zone2MapCoordinates(x, y, bot->GetZoneId());

        Map* map = bot->GetMap();
        float z = bot->GetPositionZ();
        bot->UpdateAllowedPositionZ(x, y, z);

        if (sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, x, y),
                                                 sPlayerbotAIConfig->reactDistance))
        {
            botAI->TellMaster("It is too far away");
            return false;
        }

        if (map->IsInWater(bot->GetPhaseMask(), x, y, z))
        {
            botAI->TellError("It is in water");
            return false;
        }

        float ground = map->GetHeight(x, y, z + 0.5f);
        if (ground <= INVALID_HEIGHT)
        {
            botAI->TellError("I can't go there");
            return false;
        }

        float x1 = x, y1 = y;
        sDBCManager.Map2ZoneCoordinates(x1, y1, bot->GetZoneId());

        std::ostringstream out;
        out << "Moving to " << x1 << "," << y1;
        botAI->TellMasterNoFacing(out.str());

        return MoveNear(bot->GetMapId(), x, y, z + 0.5f, sPlayerbotAIConfig->followDistance);
    }

    PositionInfo pos = context->GetValue<PositionMap&>("position")->Get()[param];
    if (pos.isSet())
    {
        if (sServerFacade->IsDistanceGreaterThan(sServerFacade->GetDistance2d(bot, pos.x, pos.y),
                                                 sPlayerbotAIConfig->reactDistance))
        {
            botAI->TellError("It is too far away");
            return false;
        }

        std::ostringstream out;
        out << "Moving to position " << param;
        botAI->TellMasterNoFacing(out.str());
        return MoveNear(bot->GetMapId(), pos.x, pos.y, pos.z + 0.5f, sPlayerbotAIConfig->followDistance);
    }

    botAI->TellMaster("Whisper 'go x,y', 'go [game object]', 'go unit' or 'go position' and I will go there");
    return false;
}
