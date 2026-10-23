/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/WhoAction.cpp (#173 "who" format, #177 follow-up):
 *  - bare "who"   -> QuerySpec()  : "<race> [M|F] <spec> (<lvl> lvl), <GS> GS (<item quality counts>), (<area>), playing with <master>"
 *  - "who <skill>" -> QuerySkill(): the profession trade link + value/max, then area and master.
 *
 * Deviations from AC (all .cpp-local; PlayerbotAI.h / ChatHelper.h are locked full-rebuild headers):
 *  - Race/class names come from ChrRaces.dbc/ChrClasses.dbc instead of AC's hand-written WotLK name tables
 *    (AC ChatHelper.cpp:174-185): those tables have the wrong race/class set for 5.4.8 (no Pandaren).
 *  - AC's ChatHelper::FormatClass(bot, spec) prints "<spec> (<c0>/<c1>/<c2>)|r <class>" - three talent-tab
 *    point counts. 5.4.8 has no talent tabs (one pick per tier), so the "(c0/c1/c2)" part is omitted and
 *    the spec name is AiFactory::GetPlayerSpecName().
 *  - botAI->GetEquipGearScore() (AC Bot/PlayerbotAI.cpp:5079-5111, the Blizzard GetAverageItemLevel rule)
 *    is not a PlayerbotAI member on this fork; it is carried here as a static helper, 1:1 apart from the
 *    Titan's Grip spell id (AC 49152; 5.4.8 46917).
 *  - ChatHelper::parseSkill/FormatSkill are carried here (same table as ListSpellsAction.cpp ParseSkill()).
 *  - QueryTrade() ("who <item>" for random bots: "Selling <item> for <price>", #883) is ported 1:1 below. WhoAction
 *    derives InventoryAction as on AC; AC's InventoryAction::parseItems(text) is InventoryAction::Find(text) here
 *    (its item-link branch is the first thing Find() does, as in AC). RandomPlayerbotMgr::GetSellMultiplier() is
 *    not exposed on this fork, so it is carried file-local with the same "sellmultiplier" event value
 *    TradeStatusAction.cpp uses (one shared multiplier per bot).
 */

#include "WhoAction.h"

#include "AiFactory.h"
#include "DBCStores.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "Playerbots.h"

#include <map>

namespace
{
    // AC Bot/Cmd/ChatHelper.cpp:105-117,663-680 (skills map, parseSkill, FormatSkill).
    std::map<std::string, uint32> const& WhoSkills()
    {
        static std::map<std::string, uint32> const skills = {
            {"first aid", SKILL_FIRST_AID},
            {"fishing", SKILL_FISHING},
            {"cooking", SKILL_COOKING},
            {"alchemy", SKILL_ALCHEMY},
            {"enchanting", SKILL_ENCHANTING},
            {"engineering", SKILL_ENGINEERING},
            {"leatherworking", SKILL_LEATHERWORKING},
            {"blacksmithing", SKILL_BLACKSMITHING},
            {"tailoring", SKILL_TAILORING},
            {"herbalism", SKILL_HERBALISM},
            {"mining", SKILL_MINING},
            {"skinning", SKILL_SKINNING},
            {"jewelcrafting", SKILL_JEWELCRAFTING},
        };
        return skills;
    }

    // AC RandomPlayerbotMgr.cpp:3044-3056 GetSellMultiplier (same file-local form as TradeStatusAction.cpp:140).
    double WhoSellMultiplier(Player* bot)
    {
        uint32 value = sRandomPlayerbotMgr->GetValue(bot, "sellmultiplier");
        if (!value)
        {
            value = urand(80, 250);
            sRandomPlayerbotMgr->SetValue(bot, "sellmultiplier", value);
        }

        return (double)value / 100.0;
    }

    uint32 WhoParseSkill(std::string const& text)
    {
        auto const itr = WhoSkills().find(text);
        return itr != WhoSkills().end() ? itr->second : uint32(SKILL_NONE);
    }

    std::string WhoFormatSkill(uint32 skill)
    {
        for (auto const& entry : WhoSkills())
            if (entry.second == skill)
                return entry.first;

        return "";
    }

    // AC Bot/PlayerbotAI.cpp:5079-5111 PlayerbotAI::GetEquipGearScore(Player*): Blizzard's
    // GetAverageItemLevel rule (17 slots, 16 when the off hand is ignored).
    uint32 WhoEquipGearScore(Player* player)
    {
        constexpr uint8 TOTAL_SLOTS = 17;       // every slot except Body & Tabard
        constexpr uint32 SPELL_TITAN_GRIP = 46917;  // 5.4.8 (AC 49152)
        uint32 sumLevel = 0;

        Item* main = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_MAINHAND);
        Item* off = player->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_OFFHAND);

        bool ignoreOffhand = false;  // true -> divisor = 16
        if (main)
        {
            bool twoHand = (main->GetTemplate()->InventoryType == INVTYPE_2HWEAPON);
            if (twoHand && !player->HasAura(SPELL_TITAN_GRIP))
                ignoreOffhand = true;  // classic 2-hander
        }
        else if (!off)  // both hands empty
            ignoreOffhand = true;

        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (slot == EQUIPMENT_SLOT_BODY || slot == EQUIPMENT_SLOT_TABARD)
                continue;

            if (ignoreOffhand && slot == EQUIPMENT_SLOT_OFFHAND)
                continue;

            if (Item* it = player->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                sumLevel += it->GetTemplate()->ItemLevel;  // missing items add 0
        }

        uint8 const divisor = ignoreOffhand ? TOTAL_SLOTS - 1 : TOTAL_SLOTS;
        return sumLevel / divisor;
    }

    // AC WhoAction.cpp:94-131 QuerySkill().
    std::string WhoQuerySkill(PlayerbotAI* botAI, Player* bot, std::string const& text)
    {
        std::ostringstream out;
        uint32 skill = WhoParseSkill(text);
        if (!skill || !botAI->HasSkill((SkillType)skill))
            return "";

        std::string const skillName = WhoFormatSkill(skill);
        uint32 spellId = botAI->GetAiObjectContext()->GetValue<uint32>("spell id", skillName)->Get();
        uint16 value = bot->GetSkillValue(skill);
        uint16 maxSkill = bot->GetMaxSkillValue(skill);
        ObjectGuid guid = bot->GetGUID();

        std::string const data = "0";
        out << "|cFFFFFF00|Htrade:" << spellId << ":" << value << ":" << maxSkill << ":" << std::hex << std::uppercase
            << guid.GetRawValue() << std::nouppercase << std::dec << ":" << data << "|h[" << skillName << "]|h|r"
            << " |h|cff00ff00" << value << "|h|cffffffff/"
            << "|h|cff00ff00" << maxSkill << "|h|cffffffff ";

        return out.str();
    }

    // AC WhoAction.cpp:133-186 QuerySpec().
    std::string WhoQuerySpec(Player* bot)
    {
        std::ostringstream out;

        out << "|h|cffffffff";
        if (ChrRacesEntry const* raceEntry = sChrRacesStore.LookupEntry(bot->GetRace()))
            out << raceEntry->name;
        out << " [" << (bot->GetGender() == GENDER_MALE ? "M" : "F") << "] ";

        out << AiFactory::GetPlayerSpecName(bot) << "|r ";
        if (ChrClassesEntry const* classEntry = sChrClassesStore.LookupEntry(bot->GetClass()))
            out << classEntry->name;

        out << " (|h|cff00ff00" << (uint32)bot->GetLevel() << "|h|cffffffff lvl), ";
        out << "|h|cff00ff00" << WhoEquipGearScore(bot) << "|h|cffffffff GS (";

        ItemCountByQuality visitor;
        for (uint32 i = EQUIPMENT_SLOT_START; i < EQUIPMENT_SLOT_END; ++i)
            if (Item* item = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                visitor.Visit(item);

        bool needSlash = false;
        if (visitor.count[ITEM_QUALITY_LEGENDARY])
        {
            out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_LEGENDARY] << "|h|cffffffff";
            needSlash = true;
        }

        if (visitor.count[ITEM_QUALITY_EPIC])
        {
            out << "|h|cffff00ff" << visitor.count[ITEM_QUALITY_EPIC] << "|h|cffffffff";
            needSlash = true;
        }

        if (visitor.count[ITEM_QUALITY_RARE])
        {
            if (needSlash)
                out << "/";

            out << "|h|cff8080ff" << visitor.count[ITEM_QUALITY_RARE] << "|h|cffffffff";
            needSlash = true;
        }

        if (visitor.count[ITEM_QUALITY_UNCOMMON])
        {
            if (needSlash)
                out << "/";

            out << "|h|cff00ff00" << visitor.count[ITEM_QUALITY_UNCOMMON] << "|h|cffffffff";
            needSlash = true;
        }

        out << ")";

        return out.str();
    }
}

// AC WhoAction.cpp:73-91 QueryTrade().
std::string const WhoAction::QueryTrade(std::string const& text)
{
    std::ostringstream out;

    std::vector<Item*> items = Find(text);
    for (Item* sell : items)
    {
        int32 sellPrice = sell->GetTemplate()->SellPrice * WhoSellMultiplier(bot) * sell->GetCount();
        if (!sellPrice)
            continue;

        out << "Selling " << ChatHelper::FormatItem(sell->GetTemplate(), sell->GetCount()) << " for "
            << ChatHelper::formatMoney(sellPrice);
        return out.str();
    }

    return "";
}

// AC WhoAction.cpp:25-70.
bool WhoAction::Execute(Event event)
{
    Player* owner = event.getOwner();
    if (!owner)
        return false;

    std::ostringstream out;

    std::string const text = event.getParam();
    if (!text.empty())
    {
        out << WhoQuerySkill(botAI, bot, text);

        if (sRandomPlayerbotMgr->IsRandomBot(bot))
            out << QueryTrade(text);
    }
    else
        out << WhoQuerySpec(bot);

    if (!out.str().empty())
    {
        if (AreaTableEntry const* areaEntry = sAreaTableStore.LookupEntry(bot->GetAreaId()))
            out << ", (|cffb04040" << areaEntry->area_name << "|r)";
    }

    if (botAI->GetMaster())
    {
        if (!out.str().empty())
            out << ", ";

        out << "playing with " << botAI->GetMaster()->GetName();
    }

    std::string const tell = out.str();
    if (tell.empty())
        return false;

    // ignore random bot chat filter
    bot->Whisper(tell, LANG_UNIVERSAL, owner);
    return true;
}
