/*
 * Ported from AC mod-playerbots Ai/Base/Actions/SuggestWhatToDoAction.cpp (#322). 5.4.8 adaptations:
 *  - AC keeps the "last said"/"suggest what to do" cooldown in the LastSaidValue (AC Ai/Base/Value/
 *    LastSaidValue.h:15-22, default time(nullptr) - 120). This fork has no "last said" value, and
 *    registering one needs ValueContext.h (a shared header), so the same per-bot timestamp is kept in a
 *    file-local map keyed by the bot's GUID (same default, same Set after every Execute).
 *  - AiPlayerbot.RandomBotSuggestDungeons is read locally (AC PlayerbotAIConfig.cpp:299, default 1 on
 *    both AC and this fork's playerbots.conf.dist) instead of being added to PlayerbotAIConfig.h.
 *  - RandomPlayerbotMgr::GetSellMultiplier is not exposed on this fork; the same reimplementation
 *    TradeStatusAction.cpp:140 uses (AC RandomPlayerbotMgr.cpp "sellmultiplier", urand(80,250)/100) is
 *    kept file-local.
 *  - AC's instance / faction tables are copied verbatim (WotLK-era); Mists-only dungeons/factions are not
 *    added (follow-up, would extend AC's pattern).
 */

#include "SuggestWhatToDoAction.h"

#include "AiFactory.h"
#include "BroadcastHelper.h"
#include "Config.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "QuestDef.h"

#include <functional>
#include <map>
#include <mutex>

namespace
{
    // See the file header: stand-in for AC's "last said" value, qualifier "suggest what to do".
    std::mutex lastSaidLock;
    std::map<uint64, time_t> lastSaidSuggest;

    time_t GetLastSaid(Player* bot)
    {
        std::lock_guard<std::mutex> guard(lastSaidLock);
        auto itr = lastSaidSuggest.find(bot->GetGUID().GetRawValue());
        // LastSaidValue.h:18 default
        return itr != lastSaidSuggest.end() ? itr->second : time(nullptr) - 120;
    }

    void SetLastSaid(Player* bot, time_t value)
    {
        std::lock_guard<std::mutex> guard(lastSaidLock);
        lastSaidSuggest[bot->GetGUID().GetRawValue()] = value;
    }

    // AC PlayerbotAIConfig.cpp:299
    bool RandomBotSuggestDungeons()
    {
        static bool const v = sConfigMgr->GetBoolDefault("AiPlayerbot.RandomBotSuggestDungeons", true);
        return v;
    }

    // AC RandomPlayerbotMgr::GetSellMultiplier; see TradeStatusAction.cpp:140 for the 5.4.8 form.
    double SellMultiplier(Player* bot)
    {
        uint32 value = sRandomPlayerbotMgr->GetValue(bot, "sellmultiplier");
        if (!value)
        {
            value = urand(80, 250);
            sRandomPlayerbotMgr->SetValue(bot, "sellmultiplier", value);
        }

        return (double)value / 100.0;
    }
}

std::map<std::string, uint8> SuggestDungeonAction::instances;
std::map<std::string, uint8> SuggestWhatToDoAction::factions;

// AC SuggestWhatToDoAction.cpp:36-45
SuggestWhatToDoAction::SuggestWhatToDoAction(PlayerbotAI* botAI, std::string const name)
    : InventoryAction{botAI, name}
{
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::specificQuest, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::grindReputation, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::something, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::grindMaterials, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::somethingToxic, this));
    suggestions.push_back(std::bind(&SuggestWhatToDoAction::toxicLinks, this));
}

// AC SuggestWhatToDoAction.cpp:47-55
bool SuggestWhatToDoAction::isUseful()
{
    if (!sRandomPlayerbotMgr->IsRandomBot(bot) || bot->GetGroup() || bot->GetInstanceId() || bot->GetBattleground())
        return false;

    time_t lastSaid = GetLastSaid(bot);
    return (time(0) - lastSaid) > 30;
}

// AC SuggestWhatToDoAction.cpp:57-67
bool SuggestWhatToDoAction::Execute(Event /*event*/)
{
    uint32 index = rand() % suggestions.size();
    auto fnct_ptr = suggestions[index];
    fnct_ptr();

    SetLastSaid(bot, time(nullptr) + urand(1, 60));

    return true;
}

// AC SuggestWhatToDoAction.cpp:69-86
std::vector<uint32> SuggestWhatToDoAction::GetIncompletedQuests()
{
    std::vector<uint32> result;

    for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
    {
        uint32 questId = bot->GetQuestSlotQuestId(slot);
        if (!questId)
            continue;

        QuestStatus status = bot->GetQuestStatus(questId);
        if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
            result.push_back(questId);
    }

    return result;
}

// AC SuggestWhatToDoAction.cpp:88-95
void SuggestWhatToDoAction::specificQuest()
{
    std::vector<uint32> quests = GetIncompletedQuests();
    if (quests.empty())
        return;

    BroadcastHelper::BroadcastSuggestQuest(botAI, quests, bot);
}

// AC SuggestWhatToDoAction.cpp:97-139: the whole body is commented out in AC (ahbot category lookup), so
// this is an intentional no-op here too.
void SuggestWhatToDoAction::grindMaterials()
{
}

// AC SuggestWhatToDoAction.cpp:141-192
void SuggestWhatToDoAction::grindReputation()
{
    if (factions.empty())
    {
        factions["Argent Dawn"] = 60;
        factions["Bloodsail Buccaneers"] = 40;
        factions["Brood of Nozdormu"] = 60;
        factions["Cenarion Circle"] = 55;
        factions["Darkmoon Faire"] = 20;
        factions["Hydraxian Waterlords"] = 60;
        factions["Ravenholdt"] = 20;
        factions["Thorium Brotherhood"] = 40;
        factions["Timbermaw Hold"] = 50;
        factions["Wintersaber Trainers"] = 50;
        factions["Booty Bay"] = 30;
        factions["Everlook"] = 40;
        factions["Gadgetzan"] = 50;
        factions["Ratchet"] = 20;

        factions["Ashtongue Deathsworn"] = 70;
        factions["Cenarion Expedition"] = 62;
        factions["The Consortium"] = 65;
        factions["Honor Hold"] = 66;
        factions["Keepers of Time"] = 68;
        factions["Netherwing"] = 65;
        factions["Ogri'la"] = 65;
        factions["The Scale of the Sands"] = 65;
        factions["Sporeggar"] = 65;
        factions["Tranquillien"] = 10;
        factions["The Violet Eye"] = 70;

        factions["Argent Crusade"] = 75;
        factions["Ashen Verdict"] = 75;
        factions["The Kalu'ak"] = 72;
        factions["Kirin Tor"] = 75;
        factions["Knights of the Ebon Blade"] = 77;
        factions["The Sons of Hodir"] = 78;
        factions["The Wyrmrest Accord"] = 77;
    }

    std::vector<std::string> levels;
    levels.push_back("honored");
    levels.push_back("revered");
    levels.push_back("exalted");

    std::vector<std::string> allowedFactions;
    for (auto it : factions)
    {
        if ((int)bot->GetLevel() >= it.second)
            allowedFactions.push_back(it.first);
    }

    if (allowedFactions.empty())
        return;

    BroadcastHelper::BroadcastSuggestGrindReputation(botAI, levels, allowedFactions, bot);
}

// AC SuggestWhatToDoAction.cpp:194-212
void SuggestWhatToDoAction::something()
{
    BroadcastHelper::BroadcastSuggestSomething(botAI, bot);
}

void SuggestWhatToDoAction::somethingToxic()
{
    BroadcastHelper::BroadcastSuggestSomethingToxic(botAI, bot);
}

void SuggestWhatToDoAction::toxicLinks()
{
    BroadcastHelper::BroadcastSuggestToxicLinks(botAI, bot);
}

void SuggestWhatToDoAction::thunderfury()
{
    BroadcastHelper::BroadcastSuggestThunderfury(botAI, bot);
}

namespace
{
    // AC SuggestWhatToDoAction.cpp:214-245
    class FindTradeItemsVisitor : public IterateItemsVisitor
    {
    public:
        FindTradeItemsVisitor(uint32 quality) : IterateItemsVisitor(), quality(quality) {}

        bool Visit(Item* item) override
        {
            ItemTemplate const* proto = item->GetTemplate();
            if (proto->Quality != quality)
                return true;

            if (proto->Class == ITEM_CLASS_TRADE_GOODS && proto->Bonding == BIND_NONE)
            {
                if (proto->Quality == ITEM_QUALITY_NORMAL && item->GetCount() > 1 &&
                    item->GetCount() == item->GetMaxStackCount())
                    stacks.push_back(proto->ItemId);

                items.push_back(proto->ItemId);
                count[proto->ItemId] += item->GetCount();
            }

            return true;
        }

        std::map<uint32, uint32> count;
        std::vector<uint32> stacks;
        std::vector<uint32> items;

    private:
        uint32 quality;
    };
}

// AC SuggestWhatToDoAction.cpp:247-249
SuggestDungeonAction::SuggestDungeonAction(PlayerbotAI* botAI) : SuggestWhatToDoAction(botAI, "suggest dungeon") {}

// AC SuggestWhatToDoAction.cpp:251-312
bool SuggestDungeonAction::Execute(Event /*event*/)
{
    // TODO (AC): use PlayerbotDungeonRepository::instance()

    if (!RandomBotSuggestDungeons() || bot->GetGroup())
        return false;

    if (instances.empty())
    {
        instances["Ragefire Chasm"] = 15;
        instances["Deadmines"] = 18;
        instances["Wailing Caverns"] = 18;
        instances["Shadowfang Keep"] = 25;
        instances["Blackfathom Deeps"] = 20;
        instances["Stockade"] = 20;
        instances["Gnomeregan"] = 35;
        instances["Razorfen Kraul"] = 35;
        instances["Maraudon"] = 50;
        instances["Scarlet Monestery"] = 40;
        instances["Uldaman"] = 45;
        instances["Dire Maul"] = 58;
        instances["Scholomance"] = 59;
        instances["Razorfen Downs"] = 40;
        instances["Strathholme"] = 59;
        instances["Zul'Farrak"] = 45;
        instances["Blackrock Depths"] = 55;
        instances["Temple of Atal'Hakkar"] = 55;
        instances["Lower Blackrock Spire"] = 57;

        instances["Hellfire Citidel"] = 65;
        instances["Coilfang Reservoir"] = 65;
        instances["Auchindoun"] = 65;
        instances["Cavens of Time"] = 68;
        instances["Tempest Keep"] = 69;
        instances["Magister's Terrace"] = 70;

        instances["Utgarde Keep"] = 75;
        instances["The Nexus"] = 75;
        instances["Ahn'kahet: The Old Kingdom"] = 75;
        instances["Azjol-Nerub"] = 75;
        instances["Drak'Tharon Keep"] = 75;
        instances["Violet Hold"] = 80;
        instances["Gundrak"] = 77;
        instances["Halls of Stone"] = 77;
        instances["Halls of Lightning"] = 77;
        instances["Oculus"] = 77;
        instances["Utgarde Pinnacle"] = 77;
        instances["Trial of the Champion"] = 80;
        instances["Forge of Souls"] = 80;
        instances["Pit of Saron"] = 80;
        instances["Halls of Reflection"] = 80;
    }

    std::vector<std::string> allowedInstances;
    for (auto it : instances)
    {
        if (bot->GetLevel() >= it.second)
            allowedInstances.push_back(it.first);
    }

    if (allowedInstances.empty())
        return false;

    BroadcastHelper::BroadcastSuggestInstance(botAI, allowedInstances, bot);
    return true;
}

// AC SuggestWhatToDoAction.cpp:314-316
SuggestTradeAction::SuggestTradeAction(PlayerbotAI* botAI) : SuggestWhatToDoAction(botAI, "suggest trade") {}

// AC SuggestWhatToDoAction.cpp:318-368
bool SuggestTradeAction::Execute(Event /*event*/)
{
    uint32 quality = urand(0, 100);
    if (quality > 95)
        quality = ITEM_QUALITY_LEGENDARY;
    else if (quality > 90)
        quality = ITEM_QUALITY_EPIC;
    else if (quality > 75)
        quality = ITEM_QUALITY_RARE;
    else if (quality > 50)
        quality = ITEM_QUALITY_UNCOMMON;
    else
        quality = ITEM_QUALITY_NORMAL;

    uint32 item = 0, count = 0;
    while (quality-- > ITEM_QUALITY_POOR)
    {
        FindTradeItemsVisitor visitor(quality);
        IterateItems(&visitor);
        if (!visitor.stacks.empty())
        {
            uint32 index = urand(0, visitor.stacks.size() - 1);
            item = visitor.stacks[index];
        }

        if (!item)
        {
            if (!visitor.items.empty())
            {
                uint32 index = urand(0, visitor.items.size() - 1);
                item = visitor.items[index];
            }
        }

        if (item)
        {
            count = visitor.count[item];
            break;
        }
    }

    if (!item || !count)
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(item);
    if (!proto)
        return false;

    uint32 price = proto->SellPrice * SellMultiplier(bot) * count;
    if (!price)
        return false;

    BroadcastHelper::BroadcastSuggestSell(botAI, proto, count, price, bot);
    return true;
}
