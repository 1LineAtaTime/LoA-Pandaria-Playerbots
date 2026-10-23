/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "ChatHelper.h"

// #322: QuestDef.h declares Quest(Field*) and the Load* helpers, so Field must be declared before it.
// Including QuestDef.h alone fails with "syntax error: identifier 'Field'" (QuestDef.h:313).
#include "Field.h"
#include "QuestDef.h"
#include "DBCStructure.h"
#include "ItemPrototype.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "SharedDefines.h"
#include "SpellInfo.h"
#include "World.h"
#include <cstdio>
#include <cstdlib>
#include <map>
#include <sstream>

// ================================================================================================
// #331 - the money / spell / item-quality / item-class / slot half of AC's ChatHelper.
//
// AC keeps these name tables as private static members (AC Bot/Cmd/ChatHelper.h:78-82) filled from the
// ChatHelper constructor (AC Bot/Cmd/ChatHelper.cpp:42-117). They are function-local statics here
// instead: the entries are identical and the tables are read-only, and this keeps the members out of
// ChatHelper.h, which PlayerbotAI.h pulls into most of the module. The idiom is the one
// SellAction.cpp:62-73 already used for the same table.
// ================================================================================================
namespace
{
// AC Bot/Cmd/ChatHelper.cpp:42-59 (itemQualities), consumed by parseItemQuality (AC :556-562).
// #331: AC calls this table itemQualities; renamed here to ItemQualitiesTable because the core already
// declares an "enum class ItemQualities" (ItemPrototype.h) and the unqualified name is ambiguous (C2872).
// 5.4.8 divergence: none - ITEM_QUALITY_* and MAX_ITEM_QUALITY are unchanged (core
// Miscellaneous/SharedDefines.h:300-310).
std::map<std::string, uint32> const& ItemQualitiesTable()
{
    static std::map<std::string, uint32> const qualities = {
        {"poor", ITEM_QUALITY_POOR},        {"gray", ITEM_QUALITY_POOR},        {"grey", ITEM_QUALITY_POOR},
        {"normal", ITEM_QUALITY_NORMAL},    {"common", ITEM_QUALITY_NORMAL},   {"white", ITEM_QUALITY_NORMAL},
        {"uncommon", ITEM_QUALITY_UNCOMMON},{"green", ITEM_QUALITY_UNCOMMON},
        {"rare", ITEM_QUALITY_RARE},        {"blue", ITEM_QUALITY_RARE},
        {"epic", ITEM_QUALITY_EPIC},        {"violet", ITEM_QUALITY_EPIC},     {"purple", ITEM_QUALITY_EPIC},
        {"legendary", ITEM_QUALITY_LEGENDARY}, {"yellow", ITEM_QUALITY_LEGENDARY}, {"orange", ITEM_QUALITY_LEGENDARY},
        {"artifact", ITEM_QUALITY_ARTIFACT},{"heirloom", ITEM_QUALITY_HEIRLOOM}};
    return qualities;
}

// AC Bot/Cmd/ChatHelper.cpp:61-83 (consumableSubClasses / tradeSubClasses / projectileSubClasses),
// consumed by parseItemClass (AC :589-620). Entries verbatim, one 5.4.8 rename:
// "food" is ITEM_SUBCLASS_FOOD_DRINK here (core ItemPrototype.h:350); AC's ITEM_SUBCLASS_FOOD does
// not exist on this core. Same value (5), same class (ITEM_CLASS_CONSUMABLE).
// Note: gems/jewels are ITEM_CLASS_GEM, not a trade-goods subclass, so they are not mapped here on
// either core (a JEWELCRAFTING trade-goods match would miss actual gems) - AC's comment, kept.
std::map<std::string, uint32> const& ConsumableSubClasses()
{
    static std::map<std::string, uint32> const subClasses = {
        {"potion", ITEM_SUBCLASS_POTION},       {"elixir", ITEM_SUBCLASS_ELIXIR},
        {"flask", ITEM_SUBCLASS_FLASK},         {"scroll", ITEM_SUBCLASS_SCROLL},
        {"food", ITEM_SUBCLASS_FOOD_DRINK},     {"bandage", ITEM_SUBCLASS_BANDAGE},
        {"enchant", ITEM_SUBCLASS_CONSUMABLE_OTHER}};
    return subClasses;
}

std::map<std::string, uint32> const& TradeSubClasses()
{
    static std::map<std::string, uint32> const subClasses = {
        {"cloth", ITEM_SUBCLASS_CLOTH},     {"leather", ITEM_SUBCLASS_LEATHER},
        {"metal", ITEM_SUBCLASS_METAL_STONE}, {"stone", ITEM_SUBCLASS_METAL_STONE},
        {"ore", ITEM_SUBCLASS_METAL_STONE}, {"meat", ITEM_SUBCLASS_MEAT},
        {"herb", ITEM_SUBCLASS_HERB},       {"elemental", ITEM_SUBCLASS_ELEMENTAL},
        {"disenchants", ITEM_SUBCLASS_ENCHANTING}, {"enchanting", ITEM_SUBCLASS_ENCHANTING}};
    return subClasses;
}

std::map<std::string, uint32> const& ProjectileSubClasses()
{
    static std::map<std::string, uint32> const subClasses = {
        {"arrows", ITEM_SUBCLASS_ARROW}, {"bullets", ITEM_SUBCLASS_BULLET}};
    return subClasses;
}

// AC Bot/Cmd/ChatHelper.cpp:85-103 (slots), consumed by parseSlot (AC :622-628). EQUIPMENT_SLOT_* is
// unchanged on 5.4.8 (core Player.h:688-708), including EQUIPMENT_SLOT_END = 19, so the table is AC's.
std::map<std::string, uint32> const& Slots()
{
    static std::map<std::string, uint32> const slots = {
        {"head", EQUIPMENT_SLOT_HEAD},       {"neck", EQUIPMENT_SLOT_NECK},
        {"shoulder", EQUIPMENT_SLOT_SHOULDERS},{"shirt", EQUIPMENT_SLOT_BODY},
        {"chest", EQUIPMENT_SLOT_CHEST},     {"waist", EQUIPMENT_SLOT_WAIST},
        {"legs", EQUIPMENT_SLOT_LEGS},       {"feet", EQUIPMENT_SLOT_FEET},
        {"wrist", EQUIPMENT_SLOT_WRISTS},    {"hands", EQUIPMENT_SLOT_HANDS},
        {"finger 1", EQUIPMENT_SLOT_FINGER1},{"finger 2", EQUIPMENT_SLOT_FINGER2},
        {"trinket 1", EQUIPMENT_SLOT_TRINKET1},{"trinket 2", EQUIPMENT_SLOT_TRINKET2},
        {"back", EQUIPMENT_SLOT_BACK},       {"main hand", EQUIPMENT_SLOT_MAINHAND},
        {"off hand", EQUIPMENT_SLOT_OFFHAND},{"ranged", EQUIPMENT_SLOT_RANGED},
        {"tabard", EQUIPMENT_SLOT_TABARD}};
    return slots;
}
}  // namespace

// #331 - AC Bot/Cmd/ChatHelper.cpp:190-229. No 5.4.8 divergence. The file-local copy MailAction.cpp and
// TrainerAction.cpp each carried was byte-identical, so the mail/trainer money output is unchanged by
// this move (see TrainerAction.cpp for the one local copy that did differ and is now aligned to AC).
std::string const ChatHelper::formatMoney(uint32 copper)
{
    std::ostringstream out;
    if (!copper)
    {
        out << "0";
        return out.str();
    }

    uint32 gold = uint32(copper / 10000);
    copper -= (gold * 10000);
    uint32 silver = uint32(copper / 100);
    copper -= (silver * 100);

    bool space = false;
    if (gold > 0)
    {
        out << gold << "g";
        space = true;
    }

    if (silver > 0 && gold < 50)
    {
        if (space)
            out << " ";

        out << silver << "s";
        space = true;
    }

    if (copper > 0 && gold < 10)
    {
        if (space)
            out << " ";

        out << copper << "c";
    }

    return out.str();
}

// #335 - AC Bot/Cmd/ChatHelper.cpp:251-284. No 5.4.8 divergence: the function only ever touches the
// text and atol(), so it is byte-identical to AC's (and to the file-local copies MailAction.cpp and
// TrainerAction.cpp used to carry - those are the same scan, so their output cannot change).
// It is the last piece SendMailAction's money branch was waiting on (#331 left it deliberately inert
// because there was no way to read an amount out of the command text without inventing a parser).
uint32 ChatHelper::parseMoney(std::string const text)
{
    // if user specified money in ##g##s##c format
    std::string acum = "";
    uint32 copper = 0;
    for (size_t i = 0; i < text.length(); i++)
    {
        if (text[i] == 'g')
        {
            copper += (atol(acum.c_str()) * 100 * 100);
            acum = "";
        }
        else if (text[i] == 'c')
        {
            copper += atol(acum.c_str());
            acum = "";
        }
        else if (text[i] == 's')
        {
            copper += (atol(acum.c_str()) * 100);
            acum = "";
        }
        else if (text[i] == ' ')
            break;
        else if (text[i] >= 48 && text[i] <= 57)
            acum += text[i];
        else
        {
            copper = 0;
            break;
        }
    }
    return copper;
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:286-310. No 5.4.8 divergence: it is a self-contained scan for
// "|Hitem:<id>:" links, so it reads the id out of the whisper text and nothing else. This is the
// piece that makes BuyAction's buy-by-link path and SendMailAction's item path resolve ids
// (previously they carried file-local copies - see BuyAction.cpp).
ItemIds ChatHelper::parseItems(std::string const text)
{
    ItemIds itemIds;

    size_t pos = 0;
    while (true)
    {
        auto i = text.find("Hitem:", pos);
        if (i == std::string::npos)
            break;

        pos = i + 6;
        auto endPos = text.find(':', pos);
        if (endPos == std::string::npos)
            break;

        std::string const idC = text.substr(pos, endPos - pos);
        auto id = atol(idC.c_str());
        pos = endPos;
        if (id)
            itemIds.insert(id);
    }

    return itemIds;
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:423-430.
//
// 5.4.8 divergence, the one TrainerAction.cpp already documented: AC indexes
// `spellInfo->SpellName[sWorld->GetDefaultDbcLocale()]` because AC's SpellInfo carries a per-locale
// ARRAY; on this core SpellInfo::SpellName is a single `char const*` (SpellInfo.h:353) holding the
// already-resolved name, so indexing it yields a `char` and the build fails. The link itself is
// byte-identical to AC's, and TrainerAction's file-local copy - which already worked around this - is
// byte-identical to the output below, so its "learn" list does not change.
std::string const ChatHelper::FormatSpell(SpellInfo const* spellInfo)
{
    std::ostringstream out;
    std::string spellName = spellInfo->SpellName;
    out << "|cffffffff|Hspell:" << spellInfo->Id << "|h[" << spellName << "]|h|r";
    return out.str();
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:499-503. Non-static on AC too, and for the same reason: it needs
// botAI->GetBot() to build the handler. Call it as botAI->GetChatHelper()->parseSpell(text); the
// Format*/parse* siblings above stay static.
uint32 ChatHelper::parseSpell(std::string const text)
{
    PlayerbotChatHandler handler(botAI->GetBot());
    return handler.extractSpellId(text);
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:556-562.
uint32 ChatHelper::parseItemQuality(std::string const text)
{
    std::map<std::string, uint32> const& qualities = ItemQualitiesTable();
    if (qualities.find(text) == qualities.end())
        return MAX_ITEM_QUALITY;

    return qualities.find(text)->second;
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:564-587.
std::string ChatHelper::FormatItemQuality(uint32 quality)
{
    switch (quality)
    {
        case ITEM_QUALITY_POOR:
            return "grey";
        case ITEM_QUALITY_NORMAL:
            return "white";
        case ITEM_QUALITY_UNCOMMON:
            return "green";
        case ITEM_QUALITY_RARE:
            return "blue";
        case ITEM_QUALITY_EPIC:
            return "purple";
        case ITEM_QUALITY_LEGENDARY:
            return "orange";
        case ITEM_QUALITY_ARTIFACT:
            return "artifact";
        case ITEM_QUALITY_HEIRLOOM:
            return "heirloom";
        default:
            return "unknown";
    }
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:589-620.
bool ChatHelper::parseItemClass(std::string const text, uint32* itemClass, uint32* itemSubClass)
{
    if (text == "questitem")
    {
        *itemClass = ITEM_CLASS_QUEST;
        *itemSubClass = ITEM_SUBCLASS_QUEST;
        return true;
    }

    std::map<std::string, uint32> const& consumables = ConsumableSubClasses();
    if (consumables.find(text) != consumables.end())
    {
        *itemClass = ITEM_CLASS_CONSUMABLE;
        *itemSubClass = consumables.find(text)->second;
        return true;
    }

    std::map<std::string, uint32> const& tradeGoods = TradeSubClasses();
    if (tradeGoods.find(text) != tradeGoods.end())
    {
        *itemClass = ITEM_CLASS_TRADE_GOODS;
        *itemSubClass = tradeGoods.find(text)->second;
        return true;
    }

    std::map<std::string, uint32> const& projectiles = ProjectileSubClasses();
    if (projectiles.find(text) != projectiles.end())
    {
        *itemClass = ITEM_CLASS_PROJECTILE;
        *itemSubClass = projectiles.find(text)->second;
        return true;
    }

    return false;
}

// #331 - AC Bot/Cmd/ChatHelper.cpp:622-628.
uint32 ChatHelper::parseSlot(std::string const text)
{
    std::map<std::string, uint32> const& slots = Slots();
    if (slots.find(text) != slots.end())
        return slots.find(text)->second;

    return EQUIPMENT_SLOT_END;
}

// Ported verbatim from AC Bot/Cmd/ChatHelper.cpp:231-249.
std::string ChatHelper::parseValue(std::string const& type, std::string const& text)
{
    std::string retString;

    std::string pattern = "Hvalue:" + type + ":";

    int pos = text.find(pattern, 0);
    if (pos == -1)
        return retString;

    pos += pattern.size();

    int endPos = text.find('|', pos);
    if (endPos == -1)
        return retString;

    retString = text.substr(pos, endPos - pos);
    return retString;
}

// #322 - ported from AC Bot/Cmd/ChatHelper.cpp:358-377.
//
// AC reads QuestLocale::Title[sWorld->GetDefaultDbcLocale()] first and falls back to quest->GetTitle().
// Neither exists on 5.4.8: QuestTemplateLocale (src/server/game/Quests/QuestDef.h:252) has no Title
// member (its fields are LogTitle/QuestDescription/...), sWorld->GetDefaultDbcLocale() is gone, and
// Quest has no GetTitle(). MoP's quest title is Quest::GetLogTitle() (QuestDef.h:365), which is the
// value AC's quest->GetTitle() would have produced - see GossipDef.cpp:212-216, which localizes the
// same field then hands it to AddQuestLevelToTitle. So AC's fallback branch is the one that survives;
// the link itself is byte-identical to AC.
std::string const ChatHelper::FormatQuest(Quest const* quest)
{
    if (!quest)
    {
        return "Invalid quest";
    }

    std::ostringstream out;
    std::string questTitle = quest->GetLogTitle();

    out << "|cFFFFFF00|Hquest:" << quest->GetQuestId() << ':' << quest->GetQuestLevel() << "|h[" << questTitle << "]|h|r";
    return out.str();
}

// #322 - ported verbatim from AC Bot/Cmd/ChatHelper.cpp:547-554 (no 5.4.8 divergence).
std::string const ChatHelper::FormatQuestObjective(std::string const name, uint32 available, uint32 required)
{
    std::ostringstream out;
    out << "|cFFFFFFFF" << name << (available >= required ? "|c0000FF00: " : "|c00FF0000: ") << available << "/"
        << required << "|r";

    return out.str();
}

// #257 slice B - ported from AC Bot/Cmd/ChatHelper.cpp:432-457.
//
// 5.4.8 divergence, one line: AC reads ItemLocale::Name[sWorld->GetDefaultDbcLocale()] directly, but on
// this core World::GetDefaultDbcLocale() returns a LocaleConstant and ItemLocale::Name is a
// StringVector (ItemPrototype.h:841), so the string is pulled with ObjectMgr::GetLocaleString() - the
// same call the core itself makes in Player::SendQuestConfirmAccept() (Player.cpp:18233-18236). The link
// body is byte-identical to AC's.
std::string const ChatHelper::FormatItem(ItemTemplate const* proto, uint32 count, uint32 total)
{
    char color[32];
    snprintf(color, sizeof(color), "%x", ItemQualityColors[proto->Quality]);

    std::string itemName;
    ItemLocale const* locale = sObjectMgr->GetItemLocale(proto->ItemId);

    if (locale && locale->Name.size() > uint32(sWorld->GetDefaultDbcLocale()))
        ObjectMgr::GetLocaleString(locale->Name, sWorld->GetDefaultDbcLocale(), itemName);

    if (itemName.empty())
        itemName = proto->Name1;

    std::ostringstream out;
    out << "|c" << color << "|Hitem:" << proto->ItemId << ":0:0:0:0:0:0:0"
        << "|h[" << itemName << "]|h|r";

    if (count > 1)
        out << "x" << count;

    if (total > 0)
        out << " (" << total << ")";

    return out.str();
}
