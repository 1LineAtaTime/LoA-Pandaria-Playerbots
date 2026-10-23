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

#include "SpellIdValue.h"

#include "Playerbots.h"
#include "Vehicle.h"
#include "World.h"

SpellIdValue::SpellIdValue(PlayerbotAI* botAI) : CalculatedValue<uint32>(botAI, "spell id", 20 * 1000) {}

VehicleSpellIdValue::VehicleSpellIdValue(PlayerbotAI* botAI) : CalculatedValue<uint32>(botAI, "vehicle spell id")
{
}

uint32 SpellIdValue::Calculate()
{
    std::string namepart = qualifier;
    //ItemIds itemIds = ChatHelper::parseItems(namepart);

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(extractedSpellId))
            namepart = spellInfo->SpellName;

    std::wstring wnamepart;
    if (!Utf8toWStr(namepart, wnamepart))
    {
        TC_LOG_ERROR("playerbots", "Utf8 to WStr failed on Calculate SpellIdValue");
        return 0;
    }

    wstrToLower(wnamepart);
    char firstSymbol = tolower(namepart[0]);
    int spellLength = wnamepart.length();

    LocaleConstant loc = LOCALE_enUS;

    std::set<uint32> spellIds;
    for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
    {
        uint32 spellId = itr->first;

        if (itr->second->state == PLAYERSPELL_REMOVED || !itr->second->active)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive())
            continue;

        if (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL)
            continue;

        // #735 (5.4.8 data, extends AC's LEARN_SPELL skip above, AC SpellIdValue.cpp:49): a 5.4.8 glyph's apply spell
        // (SPELL_EFFECT_APPLY_GLYPH 74) is non-passive and carries the ABILITY's name (55112 "Templar's Verdict",
        // 89927 "Inferno Blast", 56268 "Hand of Gul'dan"; WotLK names them "Glyph of ..."). A bot that knows it (BotFactory
        // InitGlyphs learns it) resolves the ability to the glyph, because unranked matches resolve to the lowest id,
        // and CheckCast rejects it (result=1). Never a castable ability.
        if (spellInfo->Effects[0].Effect == SPELL_EFFECT_APPLY_GLYPH)
            continue;

        bool useByItem = false;
        /*for (uint8 i = 0; i < 3; ++i)
        {
            if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM &&
                itemIds.find(spellInfo->Effects[i].ItemType) != itemIds.end())
            {
                useByItem = true;
                break;
            }
        }*/

        char const* spellName = spellInfo->SpellName;
        if (!useByItem && (tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength ||
            !Utf8FitTo(spellName, wnamepart)))
            continue;

        spellIds.insert(spellId);
    }

    Pet* pet = bot->GetPet();
    if (spellIds.empty() && pet)
    {
        for (PetSpellMap::const_iterator itr = pet->m_spells.begin(); itr != pet->m_spells.end(); ++itr)
        {
            if (itr->second.state == PETSPELL_REMOVED)
                continue;

            uint32 spellId = itr->first;
            SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!spellInfo)
                continue;

            if (spellInfo->Effects[0].Effect == SPELL_EFFECT_LEARN_SPELL)
                continue;

            char const* spellName = spellInfo->SpellName;
            if (tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength ||
                !Utf8FitTo(spellName, wnamepart))
                continue;

            spellIds.insert(spellId);
        }
    }

    if (spellIds.empty())
    {
        // Phase 1.1 diagnostic (issue #59): the bot has no known spell whose name matches the
        // qualifier, so every action that uses it silently degrades to its alternative (usually
        // "shoot" or auto-attack). Logging the qualifier tells us whether the class spell names used
        // by the 3.3.5 strategies still exist in the 5.4.8 spellbook. The sampled name proves
        // whether SpellInfo::SpellName is actually populated in this client-data set.
        char const* sample = "<none>";
        uint32 sampleId = 0;
        size_t known = 0;
        for (PlayerSpellMap::iterator itr = bot->GetSpellMap().begin(); itr != bot->GetSpellMap().end(); ++itr)
        {
            if (itr->second->state == PLAYERSPELL_REMOVED || !itr->second->active)
                continue;
            SpellInfo const* si = sSpellMgr->GetSpellInfo(itr->first);
            if (!si)
                continue;
            if (known == 0)
            {
                sample = si->SpellName ? si->SpellName : "<null>";
                sampleId = itr->first;
            }
            ++known;
        }
        // AC SpellIdValue.cpp:98 (#530 item 6)
        if (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster()))
            TC_LOG_DEBUG("playerbots", "[SID] %s qualifier='%s' resolved=0 known=%zu sample=%u:'%s'", bot->GetName().c_str(),
                         qualifier.c_str(), known, sampleId, sample);
        return 0;
    }

    int32 saveMana = (int32)round(AI_VALUE(double, "mana save level"));
    uint32 rank = 1;
    uint32 highestRank = 0;
    uint32 highestSpellId = 0;
    uint32 lowestRank = 0;
    uint32 lowestSpellId = 0;

    if (saveMana <= 1)
    {
        for (auto it = spellIds.rbegin(); it != spellIds.rend(); ++it)
        {
            auto spellId = *it;
            SpellInfo const* pSpellInfo = sSpellMgr->GetSpellInfo(spellId);
            if (!pSpellInfo)
                continue;

            std::string spellName = pSpellInfo->Rank;

            // For atoi, the input string has to start with a digit, so lets search for the first digit
            size_t i = 0;
            for (; i < spellName.length(); i++)
            {
                if (isdigit(spellName[i]))
                    break;
            }

            // remove the first chars, which aren't digits
            spellName = spellName.substr(i, spellName.length() - i);

            // convert the remaining text to an integer
            int id = atoi(spellName.c_str());

            if (!id)
            {
                highestSpellId = spellId;
                continue;
            }

            if (!highestRank || (uint32)id > highestRank)
            {
                highestRank = id;
                highestSpellId = spellId;
            }

            if (!lowestRank || (lowestRank && (uint32)id < lowestRank))
            {
                lowestRank = id;
                lowestSpellId = spellId;
            }
        }
    }
    else
    {
        for (auto it = spellIds.rbegin(); it != spellIds.rend(); ++it)
        {
            auto spellId = *it;
            if (!highestSpellId)
                highestSpellId = spellId;
            if (saveMana == (int32)rank)
                return spellId;
            lowestSpellId = spellId;
            rank++;
        }
    }

    uint32 castSpellId = saveMana > 1 ? lowestSpellId : highestSpellId;

    if (!castSpellId && (!sPlayerbotAIConfig->logInGroupOnly || (bot->GetGroup() && botAI->HasRealPlayerMaster())))
        TC_LOG_DEBUG("playerbots", "[SID] %s qualifier='%s' resolved=0 (rank filter dropped every match)",
                     bot->GetName().c_str(), qualifier.c_str());

    return castSpellId;
}

uint32 VehicleSpellIdValue::Calculate()
{
    // #581, AC Ai/Base/Value/SpellIdValue.cpp:190-243.
    Vehicle* vehicle = bot->GetVehicle();
    if (!vehicle)
        return 0;

    // do not allow if no spells
    VehicleSeatEntry const* seat = vehicle->GetSeatForPassenger(bot);
    if (!seat || !(seat->m_flags & VEHICLE_SEAT_FLAG_CAN_CAST))
        return 0;

    Unit* vehicleBase = vehicle->GetBase();
    if (!vehicleBase || !vehicleBase->IsAlive())
        return 0;

    std::string namepart = qualifier;

    PlayerbotChatHandler handler(bot);
    uint32 extractedSpellId = handler.extractSpellId(namepart);
    if (extractedSpellId)
        if (SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(extractedSpellId))
            namepart = spellInfo->SpellName;

    std::wstring wnamepart;
    if (!Utf8toWStr(namepart, wnamepart))
        return 0;

    wstrToLower(wnamepart);
    char firstSymbol = tolower(namepart[0]);
    size_t spellLength = wnamepart.length();

    Creature* creature = vehicleBase->ToCreature();
    if (!creature)
        return 0;

    for (uint32 x = 0; x < CREATURE_MAX_SPELLS; ++x)
    {
        uint32 spellId = creature->m_spells[x];
        if (spellId == 2)
            continue;

        SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
        if (!spellInfo || spellInfo->IsPassive())
            continue;

        char const* spellName = spellInfo->SpellName;
        if (!spellName || tolower(spellName[0]) != firstSymbol || strlen(spellName) != spellLength ||
            !Utf8FitTo(spellName, wnamepart))
            continue;

        return spellId;
    }

    return 0;
}
