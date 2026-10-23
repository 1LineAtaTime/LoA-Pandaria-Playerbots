/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * #611: AC ChatHelper::parseGameobjects (Bot/Cmd/ChatHelper.cpp:505-545) and ChatHelper::eraseAllSubStr (:684-694),
 * carried verbatim as free functions. This fork's ChatHelper (AI/ChatHelper.h) has neither (the same gap
 * TradeAction.cpp:8 notes) and is outside the lane that ported them; CastCustomSpellAction and UseAction share them.
 * If ChatHelper::parseGameobjects/eraseAllSubStr are ported later, every ChatLinkParse::X(...) call can become chat->x(...).
 */

#ifndef _PLAYERBOT_CHATLINKPARSE_H
#define _PLAYERBOT_CHATLINKPARSE_H

#include "ObjectGuid.h"

#include <sstream>
#include <string>
#include <vector>

namespace ChatLinkParse
{
inline std::vector<ObjectGuid> ParseGameobjects(std::string const& text)
{
    std::vector<ObjectGuid> gos;
    //    Link format
    //    |cFFFFFF00|Hfound:" << guid << ':'  << entry << ':'  <<  "|h[" << gInfo->name << "]|h|r";
    //    |cFFFFFF00|Hfound:9582:1731|h[Copper Vein]|h|r

    size_t pos = 0;
    while (true)
    {
        // extract GO guid
        auto i = text.find("Hfound:", pos);
        if (i == std::string::npos)
            break;

        pos = i + 7;
        auto endPos = text.find(':', pos);
        if (endPos == std::string::npos)
            break;

        std::istringstream stream(text.substr(pos, endPos - pos));
        uint64 guid;
        stream >> guid;

        // extract GO entry
        pos = endPos + 1;
        endPos = text.find(':', pos);
        if (endPos == std::string::npos)
            break;

        ObjectGuid lootCurrent = ObjectGuid(guid);

        if (guid)
            gos.push_back(lootCurrent);
    }

    return gos;
}

inline void EraseAllSubStr(std::string& mainStr, std::string const& toErase)
{
    size_t pos = std::string::npos;

    // Search for the substring in std::string const in a loop untill nothing is found
    while ((pos = mainStr.find(toErase)) != std::string::npos)
    {
        // If found then erase it from std::string
        mainStr.erase(pos, toErase.length());
    }
}
}  // namespace ChatLinkParse

#endif
