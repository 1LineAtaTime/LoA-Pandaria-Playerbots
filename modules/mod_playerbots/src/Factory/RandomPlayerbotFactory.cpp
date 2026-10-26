/*
* This file is part of the Legends of Azeroth Pandaria Project. See THANKS file for Copyright information
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 2 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "Helper.h"
#include "RandomPlayerbotFactory.h"

#include "AccountMgr.h"
#include "DBCStore.h"
#include "DatabaseEnv.h"
#include "CharacterCache.h"
#include "GuildMgr.h"
#include "Playerbots.h"
#include "PlayerbotAIConfig.h"
#include "Random.h"
#include "RandomPlayerbotMgr.h"
#include "ScriptMgr.h"
#include "SocialMgr.h"
#include "Timer.h"

#include <future>
#include <algorithm>
#include <cctype>
#include <set>
#include <thread>

// AC Bot/Factory/RandomPlayerbotFactory.cpp:26-49 (#627). Cata/MoP races (goblin, worgen, pandaren) have no
// own name category in playerbots_names and fall to Generic, like AC's default.
constexpr RandomPlayerbotFactory::NameRaceAndGender RandomPlayerbotFactory::CombineRaceAndGender(uint8 race,
                                                                                                uint8 gender)
{
    NameRaceAndGender baseIndex;
    switch (race)
    {
        case RACE_ORC:        baseIndex = NameRaceAndGender::OrcMale; break;
        case RACE_DWARF:      baseIndex = NameRaceAndGender::DwarfMale; break;
        case RACE_NIGHTELF:   baseIndex = NameRaceAndGender::NightelfMale; break;
        case RACE_TAUREN:     baseIndex = NameRaceAndGender::TaurenMale; break;
        case RACE_GNOME:      baseIndex = NameRaceAndGender::GnomeMale; break;
        case RACE_TROLL:      baseIndex = NameRaceAndGender::TrollMale; break;
        case RACE_BLOODELF:   baseIndex = NameRaceAndGender::BloodelfMale; break;
        case RACE_DRAENEI:    baseIndex = NameRaceAndGender::DraeneiMale; break;
        case RACE_HUMAN:
        case RACE_UNDEAD_PLAYER:
        default:
            baseIndex = NameRaceAndGender::GenericMale;
            break;
    }

    return static_cast<NameRaceAndGender>(static_cast<uint8>(baseIndex) +
                                          ((gender >= GENDER_NONE) ? static_cast<uint8>(GENDER_MALE) : gender));
}

// The races come from the character creation data (playercreateinfo) rather than a hand-kept table: every race and
// class combination this server can create, pandaren and monks included
bool RandomPlayerbotFactory::IsValidRaceClassCombination(uint8 race, uint8 cls, uint32 expansion)
{
    // skip expansion races if not playing with expansion
    if (expansion < EXPANSION_THE_BURNING_CRUSADE && (race == RACE_BLOODELF || race == RACE_DRAENEI))
        return false;

    if (expansion < EXPANSION_CATACLYSM && (race == RACE_WORGEN || race == RACE_GOBLIN))
        return false;

    if (expansion < EXPANSION_MISTS_OF_PANDARIA && (race == RACE_PANDAREN_NEUTRAL || cls == CLASS_MONK))
        return false;

    // skip expansion classes if not playing with expansion
    if (expansion < EXPANSION_WRATH_OF_THE_LICH_KING && cls == CLASS_DEATH_KNIGHT)
        return false;

    return sObjectMgr->GetPlayerInfo(race, cls) != nullptr;
}

bool RandomPlayerbotFactory::IsPlayableClass(uint8 cls)
{
    // skip nonexistent classes
    if (!((1 << (cls - 1)) & CLASSMASK_ALL_PLAYABLE) || !sChrClassesStore.LookupEntry(cls))
        return false;

    // skip disabled with config classes
    if ((1 << (cls - 1)) & sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_CLASSMASK))
        return false;

    uint32 const expansion = sWorld->getIntConfig(CONFIG_EXPANSION);
    if (cls == CLASS_DEATH_KNIGHT && expansion < EXPANSION_WRATH_OF_THE_LICH_KING)
        return false;

    if (cls == CLASS_MONK && expansion < EXPANSION_MISTS_OF_PANDARIA)
        return false;

    return true;
}

RandomPlayerbotFactory::RandomPlayerbotFactory(uint32 accountId) : accountId(accountId)
{
}

Player* RandomPlayerbotFactory::CreateRandomBot(WorldSession* session, uint8 cls,
    std::unordered_map<NameRaceAndGender, std::vector<std::string>>& nameCache, uint32 team)
{
    TC_LOG_DEBUG("playerbots", "Creating new random bot for class %s", ClassToString((Classes)cls).c_str());

    uint8 gender = rand() % 2 ? GENDER_MALE : GENDER_FEMALE;
    bool const factionGiven = team == ALLIANCE || team == HORDE;
    bool const alliance = factionGiven ? team == ALLIANCE : urand(0, 1) != 0;

    std::vector<uint8> raceOptions;
    for (uint8 race = RACE_HUMAN; race < MAX_RACES; ++race)
    {
        // skip disabled with config races
        if ((1 << (race - 1)) & sWorld->getIntConfig(CONFIG_CHARACTER_CREATING_DISABLED_RACEMASK))
            continue;

        if (factionGiven)
        {
            // A character of the faction from the start, a pandaren included
            if (race == RACE_PANDAREN_NEUTRAL || alliance != IsAlliance(race))
                continue;
        }
        else
        {
            // A pandaren starts neutral and picks its faction on its first login (RandomPlayerbotMgr::ProcessBot)
            if (race == RACE_PANDAREN_ALLIANCE || race == RACE_PANDAREN_HORDE)
                continue;

            // Try to get 50/50 faction distribution for random bot population balance.
            // Without this check, races from the faction with more class options would dominate.
            if (race != RACE_PANDAREN_NEUTRAL && alliance != IsAlliance(race))
                continue;
        }

        if (IsValidRaceClassCombination(race, cls, sWorld->getIntConfig(CONFIG_EXPANSION)))
            raceOptions.push_back(race);
    }

    if (raceOptions.empty())
    {
        TC_LOG_ERROR("playerbots", "No races available for class: %s", ClassToString((Classes)cls).c_str());
        return nullptr;
    }

    uint8 race = raceOptions[urand(0, raceOptions.size() - 1)];
    // AC RandomPlayerbotFactory.cpp:95-121: names keyed by race+gender (#627)
    const auto raceAndGender = CombineRaceAndGender(race, gender);

    std::string name;
    if (!nameCache.empty())
    {
        if (nameCache[raceAndGender].empty())
        {
            TC_LOG_ERROR("playerbots", "No names found for the specified race: %u and gender: %u", race, gender);
            return nullptr;
        }

        uint32 i = urand(0, nameCache[raceAndGender].size() - 1);
        name = nameCache[raceAndGender][i];
        swap(nameCache[raceAndGender][i], nameCache[raceAndGender].back());
        nameCache[raceAndGender].pop_back();
    }
    else
    {
        name = CreateRandomBotName(raceAndGender);
    }

    if (name.empty())
    {
        TC_LOG_ERROR("playerbots", "Failed to get a valid random bot name");
        return nullptr;
    }

    std::vector<uint8> facialHairTypes;
    std::vector<std::pair<uint8, uint8>> faces, hairs;

    for (uint32 index = 0; index < sChrSectionStore.GetNumRows(); ++index)
    {
        const CharSectionsEntry* charSection = sChrSectionStore.LookupEntry(index);

        if (!charSection) continue;
        if (charSection->RaceID != race || charSection->SexID != gender) continue;

        switch (charSection->BaseSection)
        {
            case SECTION_TYPE_FACE:
                faces.push_back(std::pair<uint8, uint8>(charSection->VariationIndex, charSection->ColorIndex));
                break;
            case SECTION_TYPE_FACIAL_HAIR:
                facialHairTypes.push_back(charSection->VariationIndex);
                break;
            case SECTION_TYPE_HAIR:
                hairs.push_back(std::pair<uint8, uint8>(charSection->VariationIndex, charSection->ColorIndex));
                break;
        }
    }

    if (faces.empty() || hairs.empty())
    {
        TC_LOG_ERROR("playerbots", "No CharSections faces or hairs for race %s gender %u", RaceToString((Races)race).c_str(), uint32(gender));
        return nullptr;
    }

    std::pair<uint8, uint8> face = faces[urand(0, faces.size() - 1)];
    std::pair<uint8, uint8> hair = hairs[urand(0, hairs.size() - 1)];

    bool excludeCheck = (race == RACE_TAUREN) || (race == RACE_DRAENEI) ||
        (gender == GENDER_FEMALE && race != RACE_NIGHTELF && race != RACE_UNDEAD_PLAYER);
    uint8 facialHair = (excludeCheck || facialHairTypes.empty()) ? 0 : facialHairTypes[urand(0, facialHairTypes.size() - 1)];

    WorldPacket p;
    std::unique_ptr<CharacterCreateInfo> characterInfo = std::make_unique<CharacterCreateInfo>(
        name, race, cls, gender, face.second, face.first, hair.first, hair.second, facialHair, 0, p);

    Player* player = new Player(session);
    player->GetMotionMaster()->Initialize();
    if (!player->Create(sObjectMgr->GetGenerator<HighGuid::Player>().Generate(), characterInfo.get()))
    {
        player->CleanupsBeforeDelete();
        delete player;

        TC_LOG_ERROR("playerbots", "Unable to create random bot for account %u - name: \"%s\"; race: %s; class: %s",
            accountId, name.c_str(), RaceToString((Races)race).c_str(), ClassToString((Classes)cls).c_str());
        return nullptr;
    }

    player->setCinematic(2);
    player->SetAtLoginFlag(AT_LOGIN_NONE);

    if (player->GetClass() == CLASS_DEATH_KNIGHT)
    {
        player->LearnSpell(50977, false);
    }
    TC_LOG_DEBUG("playerbots", "Random bot created for account %u - name: \"%s\"; race: %s; class: %s", accountId,
        name.c_str(), RaceToString((Races)race).c_str(), ClassToString((Classes)cls).c_str());

    return player;
}

// AC Bot/Factory/RandomPlayerbotFactory.cpp:184-314 (#627): every candidate (pool, conlang, true random) is
// also checked against characters.name (CHAR_SEL_CHECK_NAME) before it is returned.
std::string const RandomPlayerbotFactory::CreateRandomBotName(NameRaceAndGender raceAndGender)
{
    std::string botName = "";
    int tries = 3;
    while (--tries)
    {
        QueryResult result = CharacterDatabase.PQuery(
            "SELECT n.name "
            "FROM playerbots_names n "
            "LEFT OUTER JOIN characters c ON c.name = n.name "
            "WHERE c.guid IS NULL and n.gender = '%u' "
            "ORDER BY RAND() LIMIT 1",
            static_cast<uint32>(raceAndGender));
        if (!result)
        {
            break;
        }

        Field* fields = result->Fetch();
        botName = fields[0].GetString();
        if (ObjectMgr::CheckPlayerName(botName) == ResponseCodes::CHAR_NAME_SUCCESS)  // Checks for reservation & profanity, too
        {
            CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
            stmt->setString(0, botName);

            if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
                continue;

            return botName;
        }
    }

    // CONLANG NAME GENERATION
    TC_LOG_ERROR("playerbots", "No more names left for random bots. Attempting conlang name generation.");
    const std::string groupCategory = "SCVKRU";
    const std::string groupFormStart[2][4] = { {"SV", "SV", "VK", "RV"}, {"V", "SU", "VS", "RV"} };
    const std::string groupFormMid[2][6] = { {"CV", "CVC", "CVC", "CVK", "VC", "VK"},
                                            {"CV", "CVC", "CVK", "KVC", "VC", "KV"} };
    const std::string groupFormEnd[2][4] = { {"CV", "VC", "VK", "CV"}, {"RU", "UR", "VR", "V"} };
    const std::string groupLetter[2][6] = {
        // S           C                            V               K           R         U
        {"dtspkThfS", "bcCdfghjkmnNqqrrlsStTvwxyz", "aaeeiouA", "ppttkkbdg", "lmmnrr", "AEO"},
        {"dtskThfS", "bcCdfghjkmmnNqrrlssStTvwyz", "aaaeeiiuAAEIO", "ppttkbbdg", "lmmnrrr", "AEOy"} };
    const std::string replaceRule[2][17] = {
        {"ST", "ka", "ko", "ku", "kr", "S", "T", "C", "N", "jj", "AA", "AI", "A", "E", "O", "I", "aa"},
        {"sth", "ca", "co", "cu", "cr", "sh", "th", "ch", "ng", "dg", "A", "ayu", "ai", "ei", "ou", "iu", "ae"} };

    const auto gender = static_cast<uint8>(raceAndGender) % 2;

    tries = 10;
    while (--tries)
    {
        botName.clear();
        // Build name from groupForms
        // Pick random start group
        botName = groupFormStart[gender][rand() % 4];
        // Pick up to 2 and then up to 1 additional middle group
        for (int i = 0; i < rand() % 3 + rand() % 2; i++)
        {
            botName += groupFormMid[gender][rand() % 6];
        }
        // Pick up to 1 end group
        botName += rand() % 2 ? groupFormEnd[gender][rand() % 4] : "";
        // If name is single letter add random end group
        botName += (botName.size() < 2) ? groupFormEnd[gender][rand() % 4] : "";

        // Replace Catagory value with random Letter from that Catagory's Letter string for a given bot gender
        for (int i = 0; i < botName.size(); i++)
        {
            botName[i] = groupLetter[gender][groupCategory.find(botName[i])]
                [rand() % groupLetter[gender][groupCategory.find(botName[i])].size()];
        }

        // Itterate over replace rules
        for (int i = 0; i < 17; i++)
        {
            int j = botName.find(replaceRule[0][i]);
            while (j > -1)
            {
                botName.replace(j, replaceRule[0][i].size(), replaceRule[1][i]);
                j = botName.find(replaceRule[0][i]);
            }
        }

        // Capitalize first letter
        botName[0] -= 32;

        if (ObjectMgr::CheckPlayerName(botName) != ResponseCodes::CHAR_NAME_SUCCESS) // Checks for reservation & profanity, too
        {
            botName.clear();
            continue;
        }
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, botName);

        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            botName.clear();
            continue;
        }
        return botName;
    }

    // TRUE RANDOM NAME GENERATION
    TC_LOG_ERROR("playerbots", "Conlang name generation failed. True random name fallback.");
    tries = 10;
    while (--tries)
    {
        for (uint8 i = 0; i < 10; i++)
        {
            botName += (i == 0 ? 'A' : 'a') + rand() % 26;
        }
        if (ObjectMgr::CheckPlayerName(botName) != ResponseCodes::CHAR_NAME_SUCCESS)  // Checks for reservation & profanity, too
        {
            botName.clear();
            continue;
        }
        CharacterDatabasePreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHAR_SEL_CHECK_NAME);
        stmt->setString(0, botName);

        if (PreparedQueryResult result = CharacterDatabase.Query(stmt))
        {
            botName.clear();
            continue;
        }
        return botName;
    }
    TC_LOG_ERROR("playerbots", "Random name generation failed.");
    botName.clear();
    return botName;
}

// AzerothCore's CalculateTotalAccountCount: the accounts needed for MaxRandomBots and AddClassAccountPoolSize, after the
// unassigned accounts of playerbots_account_type are used up, or RandomBotAccountCount when that is higher (0, the
// default, means calculate)
uint32 RandomPlayerbotFactory::CalculateTotalAccountCount()
{
    // Reset account types if features are disabled
    // (synchronously: AzerothCore queues these and then polls up to a second for them to land)
    if (sPlayerbotAIConfig->maxRandomBots == 0)
    {
        PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE_UNASSIGN);
        stmt->setUInt8(0, 1);
        PlayerbotsDatabase.DirectExecute(stmt);
        TC_LOG_INFO("playerbots", "MaxRandomBots set to 0, any RNDbot accounts (type 1) will be unassigned (type 0)");
    }
    if (sPlayerbotAIConfig->addClassAccountPoolSize == 0)
    {
        PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_UPD_ACCOUNT_TYPE_UNASSIGN);
        stmt->setUInt8(0, 2);
        PlayerbotsDatabase.DirectExecute(stmt);
        TC_LOG_INFO("playerbots", "AddClassAccountPoolSize set to 0, any AddClass accounts (type 2) will be unassigned (type 0)");
    }

    // Check existing account types
    uint32 existingRndBotAccounts = 0;
    uint32 existingAddClassAccounts = 0;
    uint32 existingUnassignedAccounts = 0;
    if (PreparedQueryResult typeCheck = PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_ACCOUNT_TYPE_COUNT_BY_TYPE)))
    {
        do
        {
            Field* fields = typeCheck->Fetch();
            uint8 accountType = fields[0].GetUInt8();
            uint32 count = uint32(fields[1].GetUInt64());

            if (accountType == 0)
                existingUnassignedAccounts = count;
            else if (accountType == 1)
                existingRndBotAccounts = count;
            else if (accountType == 2)
                existingAddClassAccounts = count;
        } while (typeCheck->NextRow());
    }

    uint32 divisor = CalculateAvailableCharsPerAccount();
    if (!divisor)
        return 0;

    // Calculate max bots, taking periodic online/offline into account
    uint32 maxBots = sPlayerbotAIConfig->maxRandomBots;
    if (sPlayerbotAIConfig->enablePeriodicOnlineOffline)
        maxBots = uint32(maxBots * sPlayerbotAIConfig->periodicOnlineOfflineRatio);

    // Result is rounded up for maxBots not cleanly divisible by the divisor
    uint32 neededRndBotAccounts = (maxBots + divisor - 1) / divisor;
    uint32 neededAddClassAccounts = sPlayerbotAIConfig->addClassAccountPoolSize;

    uint32 existingTotal = existingRndBotAccounts + existingAddClassAccounts + existingUnassignedAccounts;

    // Calculate shortfalls after using unassigned accounts
    uint32 availableUnassigned = existingUnassignedAccounts;
    uint32 additionalAccountsNeeded = 0;
    for (auto [needed, existing] : { std::pair(neededRndBotAccounts, existingRndBotAccounts),
                                     std::pair(neededAddClassAccounts, existingAddClassAccounts) })
    {
        if (needed <= existing)
            continue;

        uint32 shortfall = needed - existing;
        if (shortfall <= availableUnassigned)
            availableUnassigned -= shortfall;
        else
        {
            additionalAccountsNeeded += shortfall - availableUnassigned;
            availableUnassigned = 0;
        }
    }

    uint32 calculatedTotal = existingTotal + additionalAccountsNeeded;

    // Manually set randomBotAccountCount meets the requirements
    if (sPlayerbotAIConfig->randomBotAccountCount >= calculatedTotal)
        return sPlayerbotAIConfig->randomBotAccountCount;

    // Manually set randomBotAccountCount doesn't meet the requirements. Using calculated value
    if (sPlayerbotAIConfig->randomBotAccountCount > 0)
        TC_LOG_WARN("playerbots", "RandomBotAccountCount (%u) is lower than the required calculated value (%u). Using the calculated value instead.",
            sPlayerbotAIConfig->randomBotAccountCount, calculatedTotal);

    return calculatedTotal;
}

// The characters CreateRandomBots gives an account: one per playable class (11 in Mists of Pandaria, death knights
// and monks included), capped by CharactersPerRealm
uint32 RandomPlayerbotFactory::GetCharsPerAccount(bool withDeathKnight)
{
    uint32 chars = 0;
    for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES; ++cls)
        if (IsPlayableClass(cls) && (withDeathKnight || cls != CLASS_DEATH_KNIGHT))
            ++chars;

    return std::min(chars, sWorld->getIntConfig(CONFIG_CHARACTERS_PER_REALM));
}

// AzerothCore's: the characters of an account that can log in as random bots, without the death knight
// (AiPlayerbot.DisableDeathKnightLogin) and less the share an uneven faction ratio leaves unused
uint32 RandomPlayerbotFactory::CalculateAvailableCharsPerAccount()
{
    uint32 availableChars = GetCharsPerAccount(!sPlayerbotAIConfig->disableDeathKnightLogin);

    uint32 hordeRatio = sPlayerbotAIConfig->randomBotHordeRatio;
    uint32 allianceRatio = sPlayerbotAIConfig->randomBotAllianceRatio;
    // AzerothCore divides by zero when both are 0
    if (!std::max(hordeRatio, allianceRatio))
        return availableChars;

    // horde : alliance = 50 : 50 -> 0%
    // horde : alliance = 0 : 50 -> 50%
    // horde : alliance = 10 : 50 -> 40%
    float unavailableRatio =
        static_cast<float>((std::max(hordeRatio, allianceRatio) - std::min(hordeRatio, allianceRatio))) /
        (std::max(hordeRatio, allianceRatio) * 2);

    // Conservative floor to ensure enough characters (may result in more accounts than needed).
    if (unavailableRatio != 0)
        availableChars = availableChars - availableChars * unavailableRatio;

    return availableChars;
}

void RandomPlayerbotFactory::CreateRandomBots()
{
    TC_LOG_INFO("playerbots", "Creating random bot accounts...");

    std::unordered_map<NameRaceAndGender, std::vector<std::string>> nameCache;
    int account_creation = 0;

    // Calculates the total number of required accounts.
    uint32 totalAccountCount = CalculateTotalAccountCount();
    uint32 charsPerAccount = GetCharsPerAccount(true);
    uint32 timer = getMSTime();

    for (uint32 accountNumber = 0; accountNumber < totalAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string const accountName = out.str();

        // Account names are stored in upper case (AccountMgr::CreateAccount normalizes them)
        std::string storedName = accountName;
        std::transform(storedName.begin(), storedName.end(), storedName.begin(), ::toupper);

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->setString(0, storedName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (result)
            continue;

        account_creation++;
        std::string password = "";
        if (sPlayerbotAIConfig->randomBotRandomPassword)
        {
            for (int i = 0; i < 10; i++)
                password += static_cast<char>(urand('!', 'z'));
        }
        else
            password = accountName;

        AccountOpResult res = sAccountMgr->CreateAccount(accountName, password, "");
        if (AccountOpResult::AOR_OK == res)
            TC_LOG_INFO("playerbots", "Account %s created for random bots", accountName.c_str());
        else
            TC_LOG_ERROR("playerbots", "Could not create random bot account %s (error %u)", accountName.c_str(), uint32(res));
    }

    if (account_creation)
    {
        // AC RandomPlayerbotFactory.cpp:647-656 (#627): wait until the login DB queue is drained instead of a
        // fixed sleep
        TC_LOG_INFO("playerbots", "Waiting for %u accounts loading into database (%u queries)...", account_creation, uint32(LoginDatabase.QueueSize()));
        /* wait for async accounts create to make character create correctly */
        while (LoginDatabase.QueueSize())
            std::this_thread::sleep_for(std::chrono::seconds(1));

        TC_LOG_INFO("playerbots", ">> %u Accounts loaded into database in %u ms", account_creation, GetMSTimeDiffToNow(timer));
    }

    // AzerothCore assigns the account types after CreateRandomBots; here before the characters are made, so that an
    // addclass account gets pandaren of a faction (see below)
    sRandomPlayerbotMgr->AssignAccountTypes();

    TC_LOG_INFO("playerbots", "Creating random bot characters...");
    uint32 totalRandomBotChars = 0;
    std::vector<WorldSession*> sessionBots;
    int bot_creation = 0;
    uint32 expectedChars = 0;
    timer = getMSTime();
    bool nameCached = false;
    for (uint32 accountNumber = 0; accountNumber < totalAccountCount; ++accountNumber)
    {
        std::ostringstream out;
        out << sPlayerbotAIConfig->randomBotAccountPrefix << accountNumber;
        std::string accountName = out.str();
        std::transform(accountName.begin(), accountName.end(), accountName.begin(), ::toupper);

        LoginDatabasePreparedStatement* stmt = LoginDatabase.GetPreparedStatement(LOGIN_GET_ACCOUNT_ID_BY_USERNAME);
        stmt->setString(0, accountName);
        PreparedQueryResult result = LoginDatabase.Query(stmt);
        if (!result)
        {
            // AccountMgr::CreateAccount writes the account synchronously: it failed
            TC_LOG_ERROR("playerbots", "Random bot account %s is missing, its characters are not created", accountName.c_str());
            continue;
        }

        Field* fields = result->Fetch();
        uint32 accountId = fields[0].GetUInt32();

        sPlayerbotAIConfig->randomBotAccounts.push_back(accountId);

        // The classes the account already has, so a top-up creates only the missing ones (the loop ran up to
        // MAX_CLASSES - characters, so an account with some characters skipped the last classes instead)
        std::set<uint8> existingClasses;
        if (QueryResult classes = CharacterDatabase.PQuery("SELECT class FROM characters WHERE account = %u", accountId))
        {
            do
            {
                existingClasses.insert(classes->Fetch()[0].GetUInt8());
            } while (classes->NextRow());
        }

        uint32 count = AccountMgr::GetCharactersCount(accountId);
        if (count >= charsPerAccount)
        {
            expectedChars += count;
            continue;
        }

        // AC RandomPlayerbotFactory.cpp:688-713 (#627): build the name cache only when an account needs
        // characters, and leave out every name already taken
        if (!nameCached)
        {
            nameCached = true;
            TC_LOG_INFO("playerbots", "Creating cache for names per gender and race...");
            QueryResult result = CharacterDatabase.Query(
                "SELECT n.name, n.gender FROM playerbots_names n "
                "LEFT OUTER JOIN characters c ON c.name = n.name WHERE c.guid IS NULL");
            if (!result)
            {
                TC_LOG_ERROR("playerbots", "No more unused names left (is the characters table playerbots_names imported?)");
                return;
            }
            do
            {
                Field* fields = result->Fetch();
                std::string name = fields[0].GetString();
                NameRaceAndGender raceAndGender = static_cast<NameRaceAndGender>(fields[1].GetUInt8());
                if (sObjectMgr->CheckPlayerName(name) == ResponseCodes::CHAR_NAME_SUCCESS)
                    nameCache[raceAndGender].push_back(name);
            } while (result->NextRow());
        }

        TC_LOG_INFO("playerbots", "Creating random bot characters for account: [%u/%u]", accountNumber + 1, totalAccountCount);
        RandomPlayerbotFactory factory(accountId);
        bool const addClassAccount = sRandomPlayerbotMgr->IsAccountType(accountId, 2);

        WorldSession* session = new WorldSession(accountId, accountName, nullptr, AccountTypes::SEC_PLAYER, EXPANSION_MISTS_OF_PANDARIA,
            time_t(0), LOCALE_enUS, 0, false, false, true, true /*isBot, as AC*/);
        sessionBots.push_back(session);

        // One character for each class the account does not have yet, while it has room
        for (uint8 cls = CLASS_WARRIOR; cls < MAX_CLASSES && count < charsPerAccount; ++cls)
        {
            if (!IsPlayableClass(cls) || existingClasses.count(cls))
                continue;

            // A random bot's pandaren picks its faction on its first login, which an addclass bot never has before
            // its master's: an addclass account's characters are of a faction from the start
            uint32 const team = addClassAccount ? (urand(0, 1) ? ALLIANCE : HORDE) : 0;
            Player* playerBot = factory.CreateRandomBot(session, cls, nameCache, team);
            if (!playerBot)
            {
                TC_LOG_ERROR("playerbots", "Fail to create character for account %u", accountId);
                continue;
            }

            playerBot->SaveToDB(true);

            sWorld->AddCharacterNameData(playerBot->GetGUID(), playerBot->GetName(), playerBot->GetGender(), playerBot->GetRace(), playerBot->GetClass(), playerBot->GetLevel());
            sCharacterCache->AddCharacterCacheEntry(playerBot->GetGUID(), accountId, playerBot->GetName(), playerBot->GetGender(), playerBot->GetRace(), playerBot->GetClass(), playerBot->GetLevel());

            playerBot->CleanupsBeforeDelete();

            delete playerBot;
            playerBot = nullptr;
            session->SetPlayer(nullptr);

            ++count;
            bot_creation++;
        }

        expectedChars += count;
    }

    if (bot_creation)
    {
        // AC RandomPlayerbotFactory.cpp:757-764 (#627): wait for the character DB queue, not a fixed sleep
        TC_LOG_INFO("playerbots", "Waiting for %u characters loading into database (%u queries)...", bot_creation, uint32(CharacterDatabase.QueueSize()));
        /* wait for characters load into database, or characters will fail to loggin */
        while (CharacterDatabase.QueueSize())
            std::this_thread::sleep_for(std::chrono::seconds(1));

        // and until the rows are there: the saves are transactions on the async workers
        std::ostringstream accounts;
        for (uint32 accountId : sPlayerbotAIConfig->randomBotAccounts)
            accounts << (accounts.tellp() ? "," : "") << accountId;

        uint32 savedChars = 0;
        for (uint32 waited = 0; waited <= 60; ++waited)
        {
            if (QueryResult result = CharacterDatabase.PQuery("SELECT COUNT(*) FROM characters WHERE account IN (%s)", accounts.str().c_str()))
                savedChars = uint32(result->Fetch()[0].GetUInt64());

            if (savedChars >= expectedChars)
                break;

            std::this_thread::sleep_for(std::chrono::seconds(1));
        }

        if (savedChars < expectedChars)
            TC_LOG_ERROR("playerbots", "Only %u of %u random bot characters are in the database after waiting a minute", savedChars,
                expectedChars);

        TC_LOG_INFO("playerbots", ">> %u Characters loaded into database in %u ms", bot_creation, GetMSTimeDiffToNow(timer));
    }

    for (WorldSession* session : sessionBots)
        delete session;

    for (uint32 accountId : sPlayerbotAIConfig->randomBotAccounts)
        totalRandomBotChars += AccountMgr::GetCharactersCount(accountId);

    TC_LOG_INFO("server.loading", ">> %u random bot accounts with %u characters available", uint32(sPlayerbotAIConfig->randomBotAccounts.size()), totalRandomBotChars);
}

// #611 item 4: AC Bot/Factory/RandomPlayerbotFactory.cpp:774-803 (Query("{}") -> PQuery("%u"); Get<T>() -> GetUInt32()/GetString()).
std::string const RandomPlayerbotFactory::CreateRandomGuildName()
{
    std::string guildName = "";

    QueryResult result = CharacterDatabase.Query("SELECT MAX(name_id) FROM playerbots_guild_names");
    if (!result)
    {
        TC_LOG_ERROR("playerbots", "No more names left for random guilds");
        return guildName;
    }

    Field* fields = result->Fetch();
    uint32 maxId = fields[0].GetUInt32();

    uint32 id = urand(0, maxId);
    result = CharacterDatabase.PQuery(
        "SELECT n.name FROM playerbots_guild_names n "
        "LEFT OUTER JOIN guild e ON e.name = n.name WHERE e.guildid IS NULL AND n.name_id >= %u LIMIT 1",
        id);
    if (!result)
    {
        TC_LOG_ERROR("playerbots", "No more names left for random guilds");
        return guildName;
    }

    fields = result->Fetch();
    guildName = fields[0].GetString();

    return guildName;
}
