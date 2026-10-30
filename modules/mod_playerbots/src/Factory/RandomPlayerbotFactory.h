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

    // team: ALLIANCE or HORDE for a character of that faction (the addclass accounts), 0 for either
    Player* CreateRandomBot(WorldSession* session, uint8 cls,
        std::unordered_map<NameRaceAndGender, std::vector<std::string>>& names, uint32 team = 0);
    static void CreateRandomBots();
    static uint32 CalculateTotalAccountCount();
    static uint32 CalculateAvailableCharsPerAccount();
    // Beyond AC: AiPlayerbot.RandomBotAccountSlackPct (0 = AC) is included
    static uint32 CalculateNeededRndBotAccounts();
    // #611 item 4: AC Bot/Factory/RandomPlayerbotFactory.h:56
    static std::string const CreateRandomGuildName();

private:
    static bool IsValidRaceClassCombination(uint8 race, uint8 cls, uint32 expansion);
    static bool IsPlayableClass(uint8 cls);
    static uint32 GetCharsPerAccount(bool withDeathKnight);
    std::string const CreateRandomBotName(NameRaceAndGender raceAndGender);

    uint32 accountId;
};

#endif
