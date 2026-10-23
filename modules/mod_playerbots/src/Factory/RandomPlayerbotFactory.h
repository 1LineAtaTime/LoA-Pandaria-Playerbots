/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H
#define _PLAYERBOT_RANDOMPLAYERBOTFACTORY_H

#include <map>
#include <unordered_map>
#include <vector>

#include "Common.h"
#include "DBCEnums.h"
#include "SharedDefines.h"

class Player;
class WorldSession;

class RandomPlayerbotFactory
{
public:
    // AC Bot/Factory/RandomPlayerbotFactory.h:26-47 - playerbots_names.gender is this race+gender code (#627)
    enum class NameRaceAndGender : uint8
    {
        // Generic is the category used for human & undead
        GenericMale = 0,
        GenericFemale,
        GnomeMale,
        GnomeFemale,
        DwarfMale,
        DwarfFemale,
        NightelfMale,
        NightelfFemale,
        DraeneiMale,
        DraeneiFemale,
        OrcMale,
        OrcFemale,
        TrollMale,
        TrollFemale,
        TaurenMale,
        TaurenFemale,
        BloodelfMale,
        BloodelfFemale
    };

    static constexpr NameRaceAndGender CombineRaceAndGender(uint8 race, uint8 gender);

    RandomPlayerbotFactory(uint32 accountId);
    virtual ~RandomPlayerbotFactory() {}

    Player* CreateRandomBot(WorldSession* session, Classes cls, std::unordered_map<NameRaceAndGender, std::vector<std::string>>& names);
    static void CreateRandomBots();
    // #611 item 4: AC Bot/Factory/RandomPlayerbotFactory.h:56
    static std::string const CreateRandomGuildName();

private:
    std::string const CreateRandomBotName(NameRaceAndGender raceAndGender);


    uint32 accountId;
    static std::map<uint8, std::vector<uint8>> availableRaces;
};

#endif
