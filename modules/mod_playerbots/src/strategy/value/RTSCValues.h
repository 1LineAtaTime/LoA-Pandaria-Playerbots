/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * #648: ported 1:1 from AC mod-playerbots Ai/Base/Value/RTSCValues.h. AC includes TravelMgr.h for WorldPosition; on
 * this fork WorldPosition lives in Travel/WorldPosition.h.
 */

#ifndef _PLAYERBOT_RTSCVALUES_H
#define _PLAYERBOT_RTSCVALUES_H

#include "NamedObjectContext.h"
#include "Value.h"
#include "WorldPosition.h"

class PlayerbotAI;

class SeeSpellLocationValue : public LogCalculatedValue<WorldPosition>
{
public:
    SeeSpellLocationValue(PlayerbotAI* botAI, std::string const name = "see spell location")
        : LogCalculatedValue(botAI, name){};

    bool EqualToLast(WorldPosition value) override;
    WorldPosition Calculate() override;
};

class RTSCSelectedValue : public ManualSetValue<bool>
{
public:
    RTSCSelectedValue(PlayerbotAI* botAI, bool defaultvalue = false, std::string const name = "RTSC selected")
        : ManualSetValue(botAI, defaultvalue, name){};
};

class RTSCNextSpellActionValue : public ManualSetValue<std::string>
{
public:
    RTSCNextSpellActionValue(PlayerbotAI* botAI, std::string const defaultvalue = "",
                             std::string const name = "RTSC next spell action")
        : ManualSetValue(botAI, defaultvalue, name){};
};

class RTSCSavedLocationValue : public ManualSetValue<WorldPosition>, public Qualified
{
public:
    RTSCSavedLocationValue(PlayerbotAI* botAI, WorldPosition defaultvalue = WorldPosition(),
                           std::string const name = "RTSC saved location")
        : ManualSetValue(botAI, defaultvalue, name){};

    std::string const Save() override;
    bool Load(std::string const text) override;
};

#endif
