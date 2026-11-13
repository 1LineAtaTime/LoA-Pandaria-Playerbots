/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Bot/Cmd/ChatFilter.cpp. Deviations forced by the MoP core:
 * - roles come from PlayerBotSpec (AC asks PlayerbotAI::IsTank/IsHeal/IsRanged/IsMelee);
 * - monks: "@monk", melee for "@melee"/"@ranged" except Mistweaver, and the specs "@brew", "@mist", "@ww";
 * - "@spec" names are matched on the MoP Specializations enum instead of AC's talent tab numbers. Feral and
 *   Guardian are separate specs here, so "@cat" and "@bear" follow the spec rather than the tank strategy.
 */

#include "ChatFilter.h"
#include "Playerbots.h"
#include "AiFactory.h"
#include "Group.h"
#include "PlayerbotSpec.h"
#include "RtiTargetValue.h"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <string>

static std::string ToLower(std::string const& str)
{
    std::string out = str;
    std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c){ return std::tolower(c); });
    return out;
}

std::string const ChatFilter::Filter(std::string& message)
{
    if (message.find("@") == std::string::npos)
        return message;

    return message.substr(message.find(" ") + 1);
}

class StrategyChatFilter : public ChatFilter
{
public:
    StrategyChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);

        bool tank = msgLower.find("@tank") == 0;
        if (tank && !PlayerBotSpec::IsTank(bot))
            return "";

        bool dps = msgLower.find("@dps") == 0;
        if (dps && (PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot)))
            return "";

        bool heal = msgLower.find("@heal") == 0;
        if (heal && !PlayerBotSpec::IsHeal(bot))
            return "";

        bool ranged = msgLower.find("@ranged") == 0;
        if (ranged && !PlayerBotSpec::IsRanged(bot))
            return "";

        bool melee = msgLower.find("@melee") == 0;
        if (melee && PlayerBotSpec::IsRanged(bot))
            return "";

        bool rangeddps = msgLower.find("@rangeddps") == 0;
        if (rangeddps && (!PlayerBotSpec::IsRanged(bot) || PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot)))
            return "";

        bool meleedps = msgLower.find("@meleedps") == 0;
        if (meleedps && (!PlayerBotSpec::IsMelee(bot) || PlayerBotSpec::IsTank(bot) || PlayerBotSpec::IsHeal(bot)))
            return "";

        if (tank || dps || heal || ranged || melee)
            return ChatFilter::Filter(message);

        return message;
    }
};

class LevelChatFilter : public ChatFilter
{
public:
    LevelChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        if (msgLower[0] != '@')
            return message;

        if (msgLower.find("-") != std::string::npos)
        {
            uint32 fromLevel = atoi(message.substr(message.find("@") + 1, message.find("-")).c_str());
            uint32 toLevel = atoi(message.substr(message.find("-") + 1, message.find(" ")).c_str());

            if (bot->GetLevel() >= fromLevel && bot->GetLevel() <= toLevel)
                return ChatFilter::Filter(message);

            return message;
        }

        uint32 level = atoi(message.substr(message.find("@") + 1, message.find(" ")).c_str());
        if (bot->GetLevel() == level)
            return ChatFilter::Filter(message);

        return message;
    }
};

class CombatTypeChatFilter : public ChatFilter
{
public:
    CombatTypeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);

        bool melee = msgLower.find("@melee") == 0;
        bool ranged = msgLower.find("@ranged") == 0;

        if (!melee && !ranged)
            return message;

        switch (bot->GetClass())
        {
            case CLASS_WARRIOR:
            case CLASS_PALADIN:
            case CLASS_ROGUE:
            case CLASS_DEATH_KNIGHT:
                if (ranged)
                    return "";
                break;
            case CLASS_HUNTER:
            case CLASS_PRIEST:
            case CLASS_MAGE:
            case CLASS_WARLOCK:
                if (melee)
                    return "";
                break;
            case CLASS_DRUID:
                if (ranged && PlayerBotSpec::IsTank(bot))
                    return "";
                if (melee && !PlayerBotSpec::IsTank(bot))
                    return "";
                break;
            case CLASS_SHAMAN:
            case CLASS_MONK:
                if (melee && PlayerBotSpec::IsHeal(bot))
                    return "";
                if (ranged && !PlayerBotSpec::IsHeal(bot))
                    return "";
                break;
        }

        return ChatFilter::Filter(message);
    }
};

class RtiChatFilter : public ChatFilter
{
public:
    RtiChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        rtis.push_back("@star");
        rtis.push_back("@circle");
        rtis.push_back("@diamond");
        rtis.push_back("@triangle");
        rtis.push_back("@moon");
        rtis.push_back("@square");
        rtis.push_back("@cross");
        rtis.push_back("@skull");
    }

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        Group* group = bot->GetGroup();
        if (!group)
            return message;

        bool found = false;
        for (std::vector<std::string>::iterator i = rtis.begin(); i != rtis.end(); i++)
        {
            std::string const rti = *i;
            std::string rtiLower = ToLower(rti);
            bool isRti = msgLower.find(rtiLower) == 0;
            if (!isRti)
                continue;

            ObjectGuid rtiTarget = group->GetTargetIcon(RtiTargetValue::GetRtiIndex(rti.substr(1)));
            if (bot->GetGUID() == rtiTarget)
                return ChatFilter::Filter(message);

            Unit* target = *botAI->GetAiObjectContext()->GetValue<Unit*>("current target");
            if (!target)
                return "";

            if (target->GetGUID() != rtiTarget)
                return "";

            found |= isRti;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::vector<std::string> rtis;
};

class ClassChatFilter : public ChatFilter
{
public:
    ClassChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        classNames["@dk"] = CLASS_DEATH_KNIGHT;
        classNames["@druid"] = CLASS_DRUID;
        classNames["@hunter"] = CLASS_HUNTER;
        classNames["@mage"] = CLASS_MAGE;
        classNames["@monk"] = CLASS_MONK;
        classNames["@paladin"] = CLASS_PALADIN;
        classNames["@priest"] = CLASS_PRIEST;
        classNames["@rogue"] = CLASS_ROGUE;
        classNames["@shaman"] = CLASS_SHAMAN;
        classNames["@warlock"] = CLASS_WARLOCK;
        classNames["@warrior"] = CLASS_WARRIOR;
    }

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);

        bool found = false;
        for (std::map<std::string, uint8>::iterator i = classNames.begin(); i != classNames.end(); i++)
        {
            bool isClass = msgLower.find(ToLower(i->first)) == 0;
            if (isClass && bot->GetClass() != i->second)
                return "";

            found |= isClass;
            if (found)
                break;
        }

        if (found)
            return ChatFilter::Filter(message);

        return message;
    }

private:
    std::map<std::string, uint8> classNames;
};

class SubGroupChatFilter : public ChatFilter
{
public:
    SubGroupChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        if (msgLower.find("@group") == 0)
        {
            size_t spacePos = message.find(" ");
            if (spacePos == std::string::npos)
                return message;

            std::string pnum = message.substr(6, spacePos - 6);
            std::string actualMessage = message.substr(spacePos + 1);

            std::set<uint32> targets;
            std::istringstream ss(pnum);
            std::string token;

            while (std::getline(ss, token, ','))
            {
                size_t dashPos = token.find("-");
                if (dashPos != std::string::npos)
                {
                    uint32 from = atoi(token.substr(0, dashPos).c_str());
                    uint32 to = atoi(token.substr(dashPos + 1).c_str());
                    if (from > to) std::swap(from, to);
                    for (uint32 i = from; i <= to; ++i)
                        targets.insert(i);
                }
                else
                {
                    uint32 index = atoi(token.c_str());
                    targets.insert(index);
                }
            }

            if (!bot->GetGroup())
                return message;

            uint32 sg = bot->GetSubGroup() + 1;
            if (targets.find(sg) != targets.end())
                return ChatFilter::Filter(actualMessage);
        }

        return message;
    }
};

class SpecChatFilter : public ChatFilter
{
public:
    SpecChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
    {
        // AC's names, keyed on the MoP specialization instead of a talent tab
        specNames[SPEC_PALADIN_HOLY] = "hpal";
        specNames[SPEC_PALADIN_PROTECTION] = "ppal";
        specNames[SPEC_PALADIN_RETRIBUTION] = "rpal";
        specNames[SPEC_PRIEST_DISCIPLINE] = "disc";
        specNames[SPEC_PRIEST_HOLY] = "hpr";
        specNames[SPEC_PRIEST_SHADOW] = "spr";
        specNames[SPEC_MAGE_ARCANE] = "arc";
        specNames[SPEC_MAGE_FROST] = "frost";
        specNames[SPEC_MAGE_FIRE] = "fire";
        specNames[SPEC_WARRIOR_ARMS] = "arms";
        specNames[SPEC_WARRIOR_FURY] = "fury";
        specNames[SPEC_WARRIOR_PROTECTION] = "pwar";
        specNames[SPEC_WARLOCK_AFFLICTION] = "affl";
        specNames[SPEC_WARLOCK_DEMONOLOGY] = "demo";
        specNames[SPEC_WARLOCK_DESTRUCTION] = "dest";
        specNames[SPEC_SHAMAN_ELEMENTAL] = "ele";
        specNames[SPEC_SHAMAN_ENHANCEMENT] = "enh";
        specNames[SPEC_SHAMAN_RESTORATION] = "rsha";
        specNames[SPEC_DRUID_BALANCE] = "bal";
        specNames[SPEC_DRUID_FERAL] = "cat";
        specNames[SPEC_DRUID_GUARDIAN] = "bear";
        specNames[SPEC_DRUID_RESTORATION] = "rdru";
        specNames[SPEC_HUNTER_BEAST_MASTERY] = "bmh";
        specNames[SPEC_HUNTER_MARKSMANSHIP] = "mmh";
        specNames[SPEC_HUNTER_SURVIVAL] = "svh";
        specNames[SPEC_ROGUE_ASSASSINATION] = "mut";
        specNames[SPEC_ROGUE_COMBAT] = "comb";
        specNames[SPEC_ROGUE_SUBTLETY] = "sub";
        // See below for blood death knight
        specNames[SPEC_DEATH_KNIGHT_FROST] = "fdk";
        specNames[SPEC_DEATH_KNIGHT_UNHOLY] = "udk";
        specNames[SPEC_MONK_BREWMASTER] = "brew";
        specNames[SPEC_MONK_MISTWEAVER] = "mist";
        specNames[SPEC_MONK_WINDWALKER] = "ww";
        // Matched by MatchesSpec, not by a spec of their own
        allNames.push_back("bdkt");
        allNames.push_back("bdkd");
        for (auto const& entry : specNames)
            allNames.push_back(entry.second);
    }

    std::string const Filter(std::string& message) override
    {
        std::string specPrefix;
        std::string rest;
        if (!ParseSpecPrefix(message, specPrefix, rest))
        {
            return message;
        }

        Player* bot = botAI->GetBot();
        if (!MatchesSpec(bot, specPrefix))
        {
            return "";
        }

        std::string result = ChatFilter::Filter(rest);
        return result;
    }

private:
    std::map<uint32, std::string> specNames;
    std::vector<std::string> allNames;

    bool ParseSpecPrefix(std::string const& message, std::string& specPrefix, std::string& rest)
    {
        std::string msgLower = ToLower(message);
        for (std::string const& name : allNames)
        {
            std::string prefix = "@" + name;
            if (msgLower.find(prefix) == 0)
            {
                specPrefix = name;
                size_t spacePos = message.find(' ');
                rest = (spacePos != std::string::npos) ? message.substr(spacePos + 1) : "";
                return true;
            }
        }
        return false;
    }

    bool MatchesSpec(Player* bot, std::string const& specPrefix)
    {
        uint32 spec = bot->GetSpecialization();
        std::string botSpecClass;
        // For death knights, blood is told apart into tank/dps at runtime by role
        if (spec == SPEC_DEATH_KNIGHT_BLOOD)
        {
            botSpecClass = PlayerBotSpec::IsTank(bot) ? "bdkt" : "bdkd";
        }
        else
        {
            auto it = specNames.find(spec);
            if (it != specNames.end())
                botSpecClass = it->second;
        }
        return botSpecClass == specPrefix;
    }
};

class AuraChatFilter : public ChatFilter
{
public:
    AuraChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        const std::string auraPrefix = "@aura";
        const std::string noAuraPrefix = "@noaura";
        size_t prefixLen = 0;
        bool isNoAura = false;
        if (msgLower.find(auraPrefix) == 0)
        {
            prefixLen = auraPrefix.length();
            isNoAura = false;
        }
        else if (msgLower.find(noAuraPrefix) == 0)
        {
            prefixLen = noAuraPrefix.length();
            isNoAura = true;
        }
        else
        {
            return message;
        }

        // Trim any leading spaces after @aura or @noaura (can use space between prefix and spell ID if desired, but not required)
        std::string auraIdOrName = message.substr(prefixLen);
        auraIdOrName.erase(0, auraIdOrName.find_first_not_of(' '));
        if (auraIdOrName.empty())
        {
            return message;
        }

        uint32 auraId = 0;
        size_t spacePos = auraIdOrName.find(' ');
        std::string idStr = (spacePos != std::string::npos) ? auraIdOrName.substr(0, spacePos) : auraIdOrName;
        std::string rest = (spacePos != std::string::npos) ? auraIdOrName.substr(spacePos + 1) : "";
        if (!idStr.empty())
        {
            bool isNumeric = std::all_of(idStr.begin(), idStr.end(), ::isdigit);
            if (isNumeric)
            {
                auraId = atoi(idStr.c_str());
            }
        }

        if (auraId == 0)
            return message;

        bool hasAura = bot->HasAura(auraId);
        bool match = isNoAura ? !hasAura : hasAura;
        std::string result = match ? ChatFilter::Filter(rest) : "";
        return result;
    }
};

class AggroByChatFilter : public ChatFilter
{
public:
    AggroByChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI) {}

    std::string const Filter(std::string& message) override
    {
        Player* bot = botAI->GetBot();
        std::string msgLower = ToLower(message);
        const std::string prefix = "@aggroby";
        size_t prefixLen = prefix.length();
        if (msgLower.find(prefix) != 0)
        {
            return message;
        }

        // Trim any leading spaces after @aggroby (can use space between prefix and entry ID/creature name if desired, but not required)
        std::string enemyStr = message.substr(prefixLen);
        enemyStr.erase(0, enemyStr.find_first_not_of(' '));
        if (enemyStr.empty())
        {
            return message;
        }

        // If creature name is more than one word, it must be enclosed in quotes, e.g. @aggroby "Scarlet Commander Mograine" flee
        std::string rest = "";
        std::string enemyName = "";
        bool isName = false;
        uint32 entryId = 0;

        if (enemyStr[0] == '"')
        {
            size_t endQuote = enemyStr.find('"', 1);
            if (endQuote != std::string::npos)
            {
                enemyName = enemyStr.substr(1, endQuote - 1);
                isName = true;
                size_t spacePos = enemyStr.find(' ', endQuote + 1);
                if (spacePos != std::string::npos)
                {
                    rest = enemyStr.substr(spacePos + 1);
                }
                else
                {
                    rest = "";
                }
            }
            else
            {
                enemyName = enemyStr.substr(1);
                isName = true;
                rest = "";
            }
        }
        else
        {
            size_t splitPos = enemyStr.find_first_of(" ");
            std::string idOrName = (splitPos != std::string::npos) ? enemyStr.substr(0, splitPos) : enemyStr;
            if (splitPos != std::string::npos)
            {
                rest = enemyStr.substr(splitPos + 1);
            }
            else
            {
                rest = "";
            }
            if (!idOrName.empty())
            {
                bool isNumeric = std::all_of(idOrName.begin(), idOrName.end(), ::isdigit);
                if (isNumeric)
                {
                    entryId = atoi(idOrName.c_str());
                }
                else
                {
                    enemyName = idOrName;
                    isName = true;
                }
            }
        }

        const float radius = 100.0f;
        GuidVector npcs = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();
        bool match = false;
        for (auto const& guid : npcs)
        {
            Creature* c = botAI->GetCreature(guid);
            if (!c)
            {
                continue;
            }

            bool nameMatch = isName && ToLower(c->GetName()) == ToLower(enemyName);
            bool idMatch = (entryId != 0) && c->GetEntry() == entryId;
            if ((nameMatch || idMatch) && c->GetDistance2d(bot) <= radius)
            {
                Unit* victim = c->GetVictim();
                if (victim && victim->GetGUID() == bot->GetGUID())
                {
                    match = true;
                    break;
                }
            }
        }
        std::string result = match ? ChatFilter::Filter(rest) : "";
        return result;
    }
};

CompositeChatFilter::CompositeChatFilter(PlayerbotAI* botAI) : ChatFilter(botAI)
{
    filters.push_back(new StrategyChatFilter(botAI));
    filters.push_back(new ClassChatFilter(botAI));
    filters.push_back(new RtiChatFilter(botAI));
    filters.push_back(new CombatTypeChatFilter(botAI));
    filters.push_back(new LevelChatFilter(botAI));
    filters.push_back(new SubGroupChatFilter(botAI));
    filters.push_back(new SpecChatFilter(botAI));
    filters.push_back(new AuraChatFilter(botAI));
    filters.push_back(new AggroByChatFilter(botAI));
}

CompositeChatFilter::~CompositeChatFilter()
{
    for (std::vector<ChatFilter*>::iterator i = filters.begin(); i != filters.end(); i++)
        delete (*i);
}

std::string const CompositeChatFilter::Filter(std::string& message)
{
    for (auto* filter : filters)
    {
        message = filter->Filter(message);
        if (message.empty())
            break;
    }

    return message;
}
