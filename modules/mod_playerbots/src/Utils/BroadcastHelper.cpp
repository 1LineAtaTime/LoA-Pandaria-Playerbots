/*
 * Ported from AC mod-playerbots Util/BroadcastHelper.cpp (#322). 5.4.8 adaptations, all file-local so no
 * widely included header (PlayerbotAI.h, PlayerbotAIConfig.h, ChatHelper.h) is touched:
 *
 *  - Config: AC reads ~45 AiPlayerbot.Broadcast* / ToxicLinks* / EnableBroadcasts keys into PlayerbotAIConfig
 *    members. Here BroadcastConfig() reads them once, locally, with the SAME keys; defaults are the ones
 *    in modules/mod_playerbots/config/playerbots.conf.dist (they match AC's, except
 *    BroadcastChanceLootingItemNormal = 150 on the dist vs 300 in AC's code). AC PlayerbotAIConfig.cpp:296-380.
 *  - PlayerbotAI::SayToGuild/SayToWorld/SayToChannel (AC Bot/PlayerbotAI.cpp:2983-3070), GetCurrentArea/
 *    GetCurrentZone/GetLocalizedAreaName (:2877-2900) and GetInventoryAndEquippedItems (:5869) do not exist
 *    on this fork's PlayerbotAI; they are re-implemented below as file-local helpers (SayToGuild, SayToWorld,
 *    SayToChannel, CurrentArea, CurrentZone, AreaName, InventoryAndEquippedItems).
 *      * 5.4.8 ChannelMgr has no GetChannels(); SayToChannel rebuilds the channel name exactly the way
 *        Player::UpdateLocalChannels does (Player.cpp:5622-5680) and looks it up with
 *        ChannelMgr::GetChannel(name, bot, false). The zone-name containment rule of AC is kept literally.
 *      * 5.4.8 AreaTableEntry::area_name and ChatChannelsEntry::pattern are single strings (no locale array).
 *      * SayToWorld: AC joins eligible bots to the custom "World" channel in PlayerbotMgr (AC
 *        Bot/PlayerbotMgr.cpp:604-619, level >= 10, random bot, non-SOLO grouper). On this fork that block is
 *        commented out (Manager/PlayerbotMgr.cpp ~598-619), so SayToWorld performs the same join lazily with the
 *        same level/random-bot gate (GrouperType does not exist on this fork).
 *  - ChatHelper::FormatClass(uint8)/FormatRace(uint8)/FormatClass(Player*, spec) (AC Bot/Cmd/ChatHelper.cpp:
 *    128-185, 639-661) are not on this fork's ChatHelper: FormatClassName/FormatRaceName/FormatRole below.
 *    5.4.8 has no talent trees, so %my_role is "<spec name> <class>" (AiFactory::GetPlayerSpecName) instead
 *    of AC's "<tree> (c0/c1/c2) <class>".
 *  - AC BroadcastKill assigns creature->GetLevel() (a uint8) straight into a std::string placeholder (a
 *    char, not the number). Fixed with std::to_string.
 */

#include "BroadcastHelper.h"

#include "AiFactory.h"
#include "Bag.h"
#include "Channel.h"
#include "ChannelMgr.h"
#include "ChatHelper.h"
#include "Config.h"
#include "Creature.h"
#include "DBCStores.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Item.h"
#include "Language.h"
#include "ObjectMgr.h"
#include "Playerbots.h"
#include "PlayerbotTextMgr.h"
#include "QuestDef.h"
#include "World.h"

#include <algorithm>
#include <map>
#include <sstream>

namespace
{
    // AC Bot/PlayerbotAI.h:158-165 ChatChannelId (kept local; the fork has no such enum yet).
    enum BroadcastChatChannelId : uint32
    {
        CH_GENERAL = 1,
        CH_TRADE = 2,
        CH_LOCAL_DEFENSE = 22,
        CH_WORLD_DEFENSE = 23,
        CH_GUILD_RECRUITMENT = 25,
        CH_LOOKING_FOR_GROUP = 26
    };

    // AC PlayerbotAIConfig.cpp:296-380 (+ PlayerbotAIConfig.h). Same keys, dist defaults.
    struct BroadcastCfg
    {
        bool enableBroadcasts;
        int32 broadcastChanceMaxValue;
        int32 toGuild, toWorld, toGeneral, toTrade, toLFG, toLocalDefense, toWorldDefense, toGuildRecruitment;
        int32 lootPoor, lootNormal, lootUncommon, lootRare, lootEpic, lootLegendary, lootArtifact;
        int32 questAccepted, questObjCompleted, questObjProgress, questFailedTimer, questComplete, questTurnedIn;
        int32 killNormal, killElite, killRareelite, killWorldboss, killRare, killUnknown, killPet, killPlayer;
        int32 levelupGeneric, levelupTenX, levelupMaxLevel;
        int32 suggestInstance, suggestQuest, suggestGrindMaterials, suggestGrindReputation, suggestSell,
            suggestSomething, suggestSomethingToxic, suggestToxicLinks, suggestThunderfury;
        int32 guildManagement;
        std::string toxicLinksPrefix;
    };

    BroadcastCfg const& Cfg()
    {
        static BroadcastCfg const c = []
        {
            auto I = [](char const* key, int32 def) { return int32(sConfigMgr->GetIntDefault(key, def)); };
            BroadcastCfg v;
            v.enableBroadcasts = sConfigMgr->GetBoolDefault("AiPlayerbot.EnableBroadcasts", true);
            // AC PlayerbotAIConfig.cpp:305
            v.broadcastChanceMaxValue = v.enableBroadcasts ? 30000 : 0;
            v.toGuild = I("AiPlayerbot.BroadcastToGuildGlobalChance", 30000);
            v.toWorld = I("AiPlayerbot.BroadcastToWorldGlobalChance", 30000);
            v.toGeneral = I("AiPlayerbot.BroadcastToGeneralGlobalChance", 30000);
            v.toTrade = I("AiPlayerbot.BroadcastToTradeGlobalChance", 30000);
            v.toLFG = I("AiPlayerbot.BroadcastToLFGGlobalChance", 30000);
            v.toLocalDefense = I("AiPlayerbot.BroadcastToLocalDefenseGlobalChance", 30000);
            v.toWorldDefense = I("AiPlayerbot.BroadcastToWorldDefenseGlobalChance", 30000);
            v.toGuildRecruitment = I("AiPlayerbot.BroadcastToGuildRecruitmentGlobalChance", 30000);
            v.lootPoor = I("AiPlayerbot.BroadcastChanceLootingItemPoor", 30);
            v.lootNormal = I("AiPlayerbot.BroadcastChanceLootingItemNormal", 150);
            v.lootUncommon = I("AiPlayerbot.BroadcastChanceLootingItemUncommon", 10000);
            v.lootRare = I("AiPlayerbot.BroadcastChanceLootingItemRare", 20000);
            v.lootEpic = I("AiPlayerbot.BroadcastChanceLootingItemEpic", 30000);
            v.lootLegendary = I("AiPlayerbot.BroadcastChanceLootingItemLegendary", 30000);
            v.lootArtifact = I("AiPlayerbot.BroadcastChanceLootingItemArtifact", 30000);
            v.questAccepted = I("AiPlayerbot.BroadcastChanceQuestAccepted", 6000);
            v.questObjCompleted = I("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveCompleted", 300);
            v.questObjProgress = I("AiPlayerbot.BroadcastChanceQuestUpdateObjectiveProgress", 300);
            v.questFailedTimer = I("AiPlayerbot.BroadcastChanceQuestUpdateFailedTimer", 300);
            v.questComplete = I("AiPlayerbot.BroadcastChanceQuestUpdateComplete", 1000);
            v.questTurnedIn = I("AiPlayerbot.BroadcastChanceQuestTurnedIn", 10000);
            v.killNormal = I("AiPlayerbot.BroadcastChanceKillNormal", 30);
            v.killElite = I("AiPlayerbot.BroadcastChanceKillElite", 300);
            v.killRareelite = I("AiPlayerbot.BroadcastChanceKillRareelite", 3000);
            v.killWorldboss = I("AiPlayerbot.BroadcastChanceKillWorldboss", 20000);
            v.killRare = I("AiPlayerbot.BroadcastChanceKillRare", 10000);
            v.killUnknown = I("AiPlayerbot.BroadcastChanceKillUnknown", 100);
            v.killPet = I("AiPlayerbot.BroadcastChanceKillPet", 10);
            v.killPlayer = I("AiPlayerbot.BroadcastChanceKillPlayer", 30);
            v.levelupGeneric = I("AiPlayerbot.BroadcastChanceLevelupGeneric", 20000);
            v.levelupTenX = I("AiPlayerbot.BroadcastChanceLevelupTenX", 30000);
            v.levelupMaxLevel = I("AiPlayerbot.BroadcastChanceLevelupMaxLevel", 30000);
            v.suggestInstance = I("AiPlayerbot.BroadcastChanceSuggestInstance", 5000);
            v.suggestQuest = I("AiPlayerbot.BroadcastChanceSuggestQuest", 10000);
            v.suggestGrindMaterials = I("AiPlayerbot.BroadcastChanceSuggestGrindMaterials", 5000);
            v.suggestGrindReputation = I("AiPlayerbot.BroadcastChanceSuggestGrindReputation", 5000);
            v.suggestSell = I("AiPlayerbot.BroadcastChanceSuggestSell", 300);
            v.suggestSomething = I("AiPlayerbot.BroadcastChanceSuggestSomething", 30000);
            v.suggestSomethingToxic = I("AiPlayerbot.BroadcastChanceSuggestSomethingToxic", 0);
            v.suggestToxicLinks = I("AiPlayerbot.BroadcastChanceSuggestToxicLinks", 0);
            v.toxicLinksPrefix = sConfigMgr->GetStringDefault("AiPlayerbot.ToxicLinksPrefix", "gnomes");
            v.suggestThunderfury = I("AiPlayerbot.BroadcastChanceSuggestThunderfury", 1);
            v.guildManagement = I("AiPlayerbot.BroadcastChanceGuildManagement", 30000);
            return v;
        }();
        return c;
    }

    // urand(1, broadcastChanceMaxValue) <= chance. AC calls urand(1, max) directly; with broadcasts disabled
    // max is 0 and urand(1, 0) is undefined, so the roll is made safe here (result: never passes).
    bool RollChance(int32 chance)
    {
        int32 const max = Cfg().broadcastChanceMaxValue;
        if (max < 1 || chance < 1)
            return false;
        return int32(urand(1, uint32(max))) <= chance;
    }

    // AC Bot/Cmd/ChatHelper.cpp:128-185 (names), :659/:661
    std::string FormatClassName(uint8 cls)
    {
        switch (cls)
        {
            case CLASS_DRUID: return "druid";
            case CLASS_HUNTER: return "hunter";
            case CLASS_MAGE: return "mage";
            case CLASS_PALADIN: return "paladin";
            case CLASS_PRIEST: return "priest";
            case CLASS_ROGUE: return "rogue";
            case CLASS_SHAMAN: return "shaman";
            case CLASS_WARLOCK: return "warlock";
            case CLASS_WARRIOR: return "warrior";
            case CLASS_DEATH_KNIGHT: return "dk";
            case CLASS_MONK: return "monk";  // Mists-only (extends AC's table)
            default: return "";
        }
    }

    std::string FormatRaceName(uint8 race)
    {
        switch (race)
        {
            case RACE_DWARF: return "Dwarf";
            case RACE_GNOME: return "Gnome";
            case RACE_HUMAN: return "Human";
            case RACE_NIGHTELF: return "Night Elf";
            case RACE_ORC: return "Orc";
            case RACE_TAUREN: return "Tauren";
            case RACE_TROLL: return "Troll";
            case RACE_UNDEAD_PLAYER: return "Undead";
            case RACE_BLOODELF: return "Blood Elf";
            case RACE_DRAENEI: return "Draenei";
            // Mists-only races (extend AC's table)
            case RACE_GOBLIN: return "Goblin";
            case RACE_WORGEN: return "Worgen";
            case RACE_PANDAREN_NEUTRAL:
            case RACE_PANDAREN_ALLIANCE:
            case RACE_PANDAREN_HORDE: return "Pandaren";
            default: return "";
        }
    }

    // AC Bot/Cmd/ChatHelper.cpp:639-657 FormatClass(Player*, spec); see the file header for the 5.4.8 form.
    std::string FormatRole(Player* bot)
    {
        std::ostringstream out;
        out << AiFactory::GetPlayerSpecName(bot) << " " << FormatClassName(bot->GetClass());
        return out.str();
    }

    // AC Bot/PlayerbotAI.cpp:2877-2900
    AreaTableEntry const* CurrentArea(Player* bot) { return sAreaTableStore.LookupEntry(bot->GetAreaId()); }
    AreaTableEntry const* CurrentZone(Player* bot) { return sAreaTableStore.LookupEntry(bot->GetZoneId()); }

    std::string AreaName(AreaTableEntry const* entry)
    {
        return (entry && entry->area_name) ? std::string(entry->area_name) : std::string();
    }

    // The area/zone/class/race/level placeholder set AC fills at the top of every broadcast.
    void AddAreaPlaceholders(Player* bot, std::map<std::string, std::string>& placeholders)
    {
        AreaTableEntry const* current_area = CurrentArea(bot);
        AreaTableEntry const* current_zone = CurrentZone(bot);
        placeholders["%area_name"] = current_area ? AreaName(current_area) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
        placeholders["%zone_name"] = current_zone ? AreaName(current_zone) : PlayerbotTextMgr::instance().GetBotText("string_unknown_area");
    }

    void AddSelfPlaceholders(Player* bot, std::map<std::string, std::string>& placeholders)
    {
        placeholders["%my_class"] = FormatClassName(bot->GetClass());
        placeholders["%my_race"] = FormatRaceName(bot->GetRace());
        placeholders["%my_level"] = std::to_string(bot->GetLevel());
    }

    // AC Bot/PlayerbotAI.cpp:5869-5910
    std::vector<Item*> InventoryAndEquippedItems(Player* bot)
    {
        std::vector<Item*> items;

        for (uint8 i = INVENTORY_SLOT_BAG_START; i < INVENTORY_SLOT_BAG_END; ++i)
        {
            if (Bag* pBag = (Bag*)bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
            {
                for (uint32 j = 0; j < pBag->GetBagSize(); ++j)
                {
                    if (Item* pItem = pBag->GetItemByPos(j))
                        items.push_back(pItem);
                }
            }
        }

        for (uint8 i = INVENTORY_SLOT_ITEM_START; i < INVENTORY_SLOT_ITEM_END; ++i)
        {
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, i))
                items.push_back(pItem);
        }

        // AC's KEYRING_SLOT loop has no 5.4.8 counterpart (no keyring).

        for (uint8 slot = EQUIPMENT_SLOT_START; slot < EQUIPMENT_SLOT_END; ++slot)
        {
            if (Item* pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, slot))
                items.push_back(pItem);
        }

        return items;
    }

    // AC Bot/PlayerbotAI.cpp:2983-3004
    bool SayToGuild(Player* bot, std::string const& msg)
    {
        if (msg.empty())
            return false;

        if (bot->GetGuildId())
        {
            if (Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId()))
            {
                // Guild::HasRankRight is private in 5.4.8; BroadcastToGuild applies the same GR_RIGHT_GCHATSPEAK check itself.
                guild->BroadcastToGuild(bot->GetSession(), false, msg, LANG_UNIVERSAL);
                return true;
            }
        }

        return false;
    }

    // AC Bot/PlayerbotAI.cpp:3006-3025 (+ the join from AC Bot/PlayerbotMgr.cpp:604-619, see file header)
    bool SayToWorld(Player* bot, std::string const& msg)
    {
        if (msg.empty())
            return false;

        ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeam());
        if (!cMgr)
            return false;

        if (bot->GetLevel() >= 10 && sRandomPlayerbotMgr->IsRandomBot(bot))
        {
            if (Channel* joinChannel = cMgr->GetJoinChannel("World", 0))
                joinChannel->JoinChannel(bot, "");
        }

        // no zone
        if (Channel* worldChannel = cMgr->GetChannel("World", bot, false))
        {
            worldChannel->Say(bot->GetGUID(), msg, LANG_UNIVERSAL);
            return true;
        }

        return false;
    }

    // AC Bot/PlayerbotAI.cpp:3027-3070. AC walks cMgr->GetChannels(); 5.4.8 hides the channel map, so the
    // candidate name is rebuilt like Player::UpdateLocalChannels (Player.cpp:5654-5680) does.
    bool BroadcastSayToChannel(Player* bot, std::string const& msg, uint32 chanId)
    {
        if (msg.empty())
            return false;

        ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeam());
        if (!cMgr)
            return false;

        AreaTableEntry const* current_zone = CurrentZone(bot);
        if (!current_zone)
            return false;

        std::string const current_str_zone = AreaName(current_zone);

        for (uint32 i = 0; i < sChatChannelsStore.GetNumRows(); ++i)
        {
            ChatChannelsEntry const* entry = sChatChannelsStore.LookupEntry(i);
            if (!entry || entry->ChannelID != chanId)
                continue;

            std::string channelName;
            if (!(entry->flags & CHANNEL_DBC_FLAG_GLOBAL))
            {
                char buf[200];
                char const* nameExt;
                if (entry->flags & CHANNEL_DBC_FLAG_CITY_ONLY)
                    nameExt = sObjectMgr->GetTrinityString(LANG_CHANNEL_CITY,
                        LocaleConstant(sWorld->getIntConfig(CONFIG_CHANNEL_CONSTANT_LOCALE)));
                else
                    nameExt = current_zone->area_name;

                snprintf(buf, sizeof(buf), entry->pattern, nameExt);
                channelName = buf;
            }
            else
                channelName = entry->pattern;

            Channel* channel = cMgr->GetChannel(channelName, bot, false);
            if (!channel || channel->GetName().empty())
                continue;

            // Checks if the channel name contains the current zone (AC :3050-3057)
            bool const does_contain = channel->GetName().find(current_str_zone) != std::string::npos;
            if (chanId != CH_LOOKING_FOR_GROUP && chanId != CH_WORLD_DEFENSE && !does_contain)
                continue;

            channel->Say(bot->GetGUID(), msg, LANG_UNIVERSAL);
            return true;
        }

        return false;
    }
}

BroadcastHelper::BroadcastHelper() {}

// AC Util/BroadcastHelper.cpp:14-21
uint8 BroadcastHelper::GetLocale()
{
    uint8 locale = sWorld->GetDefaultDbcLocale();
    // -- In case we're using auto detect on config file
    if (locale >= TOTAL_LOCALES)
        locale = LocaleConstant::LOCALE_enUS;
    return locale;
}

// AC Util/BroadcastHelper.cpp:23-65. AC returns false on the first line ("return something to ignore the
// logic"); everything after it is dead code there and is not ported.
bool BroadcastHelper::BroadcastTest(PlayerbotAI* /*ai*/, Player* /* bot */)
{
    return false;
}

/**
@param toChannels - map of (ToChannel, chance), where chance is in range 0-100 as uint32 (unless global chance is not 100%)

@return true if said to the channel, false otherwise

AC Util/BroadcastHelper.cpp:67-175
*/
bool BroadcastHelper::BroadcastToChannelWithGlobalChance(PlayerbotAI* ai, std::string message, std::list<std::pair<ToChannel, uint32>> toChannels)
{
    BroadcastCfg const& cfg = Cfg();
    if (!cfg.enableBroadcasts)
        return false;
    if (message.empty())
    {
        return false;
    }

    Player* bot = ai->GetBot();

    for (auto const& pair : toChannels)
    {
        uint32 roll = urand(1, 100);
        uint32 chance = pair.second;
        uint32 broadcastRoll = urand(1, uint32(std::max(cfg.broadcastChanceMaxValue, 1)));

        switch (pair.first)
        {
            case TO_GUILD:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toGuild)
                    && SayToGuild(bot, message))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toWorld)
                    && SayToWorld(bot, message))
                {
                    return true;
                }
                break;
            }
            case TO_GENERAL:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toGeneral)
                    && BroadcastSayToChannel(bot, message, CH_GENERAL))
                {
                    return true;
                }
                break;
            }
            case TO_TRADE:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toTrade)
                    && BroadcastSayToChannel(bot, message, CH_TRADE))
                {
                    return true;
                }
                break;
            }
            case TO_LOOKING_FOR_GROUP:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toLFG)
                    && BroadcastSayToChannel(bot, message, CH_LOOKING_FOR_GROUP))
                {
                    return true;
                }
                break;
            }
            case TO_LOCAL_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toLocalDefense)
                    && BroadcastSayToChannel(bot, message, CH_LOCAL_DEFENSE))
                {
                    return true;
                }
                break;
            }
            case TO_WORLD_DEFENSE:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toWorldDefense)
                    && BroadcastSayToChannel(bot, message, CH_WORLD_DEFENSE))
                {
                    return true;
                }
                break;
            }
            case TO_GUILD_RECRUITMENT:
            {
                if (roll <= chance
                    && broadcastRoll <= uint32(cfg.toGuildRecruitment)
                    && BroadcastSayToChannel(bot, message, CH_GUILD_RECRUITMENT))
                {
                    return true;
                }
                break;
            }
            default:
                break;
        }
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:177-278
bool BroadcastHelper::BroadcastLootingItem(PlayerbotAI* ai, Player* bot, ItemTemplate const* proto)
{
    BroadcastCfg const& cfg = Cfg();
    if (!cfg.enableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%item_link"] = ChatHelper::FormatItem(proto);
    AddAreaPlaceholders(bot, placeholders);
    AddSelfPlaceholders(bot, placeholders);

    char const* textName = nullptr;
    int32 chance = 0;
    switch (proto->Quality)
    {
        case ITEM_QUALITY_POOR:
            textName = "broadcast_looting_item_poor"; chance = cfg.lootPoor; break;
        case ITEM_QUALITY_NORMAL:
            textName = "broadcast_looting_item_normal"; chance = cfg.lootNormal; break;
        case ITEM_QUALITY_UNCOMMON:
            textName = "broadcast_looting_item_uncommon"; chance = cfg.lootUncommon; break;
        case ITEM_QUALITY_RARE:
            textName = "broadcast_looting_item_rare"; chance = cfg.lootRare; break;
        case ITEM_QUALITY_EPIC:
            textName = "broadcast_looting_item_epic"; chance = cfg.lootEpic; break;
        case ITEM_QUALITY_LEGENDARY:
            textName = "broadcast_looting_item_legendary"; chance = cfg.lootLegendary; break;
        case ITEM_QUALITY_ARTIFACT:
            textName = "broadcast_looting_item_artifact"; chance = cfg.lootArtifact; break;
        default:
            break;
    }

    if (textName && RollChance(chance))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText(textName, placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:280-304
bool BroadcastHelper::BroadcastQuestAccepted(PlayerbotAI* ai, Player* bot, Quest const* quest)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().questAccepted))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_accepted_generic", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:306-346
bool BroadcastHelper::BroadcastQuestUpdateAddKill(PlayerbotAI* ai, Player* bot, Quest const* quest, uint32 availableCount, uint32 requiredCount, std::string obectiveName)
{
    if (!Cfg().enableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    AddAreaPlaceholders(bot, placeholders);
    placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
    placeholders["%quest_obj_name"] = obectiveName;
    AddSelfPlaceholders(bot, placeholders);
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = ChatHelper::FormatQuestObjective(obectiveName, availableCount, requiredCount);

    if (availableCount < requiredCount && RollChance(Cfg().questObjProgress))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_kill_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount && RollChance(Cfg().questObjCompleted))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_kill_objective_completed", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:348-390
bool BroadcastHelper::BroadcastQuestUpdateAddItem(PlayerbotAI* ai, Player* bot, Quest const* quest, uint32 availableCount, uint32 requiredCount, ItemTemplate const* proto)
{
    if (!Cfg().enableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    AddAreaPlaceholders(bot, placeholders);
    placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
    std::string itemLinkFormatted = ChatHelper::FormatItem(proto);
    placeholders["%item_link"] = itemLinkFormatted;
    AddSelfPlaceholders(bot, placeholders);
    placeholders["%quest_obj_available"] = std::to_string(availableCount);
    placeholders["%quest_obj_required"] = std::to_string(requiredCount);
    placeholders["%quest_obj_missing"] = std::to_string(requiredCount - std::min(availableCount, requiredCount));
    placeholders["%quest_obj_full_formatted"] = ChatHelper::FormatQuestObjective(itemLinkFormatted, availableCount, requiredCount);

    if (availableCount < requiredCount && RollChance(Cfg().questObjProgress))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_item_objective_progress", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }
    else if (availableCount == requiredCount && RollChance(Cfg().questObjCompleted))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_add_item_objective_completed", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:392-416
bool BroadcastHelper::BroadcastQuestUpdateFailedTimer(PlayerbotAI* ai, Player* bot, Quest const* quest)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().questFailedTimer))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_failed_timer", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:418-442
bool BroadcastHelper::BroadcastQuestUpdateComplete(PlayerbotAI* ai, Player* bot, Quest const* quest)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().questComplete))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_update_complete", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:444-468
bool BroadcastHelper::BroadcastQuestTurnedIn(PlayerbotAI* ai, Player* bot, Quest const* quest)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().questTurnedIn))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_quest_turned_in", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:470-577
bool BroadcastHelper::BroadcastKill(PlayerbotAI* ai, Player* bot, Creature *creature)
{
    BroadcastCfg const& cfg = Cfg();
    if (!cfg.enableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%victim_name"] = creature->GetName();
    AddAreaPlaceholders(bot, placeholders);
    placeholders["%victim_level"] = std::to_string(creature->GetLevel());
    AddSelfPlaceholders(bot, placeholders);

    //if ((creature->IsElite() && !creature->GetMap()->IsDungeon())
    //if creature->IsWorldBoss()
    //if creature->GetLevel() > DEFAULT_MAX_LEVEL + 1
    //if creature->GetLevel() > bot->GetLevel() + 4

    if (creature->IsPet())
    {
        if (RollChance(cfg.killPet))
        {
            return BroadcastToChannelWithGlobalChance(
                ai,
                PlayerbotTextMgr::instance().GetBotText("broadcast_killed_pet", placeholders),
                { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }
    else if (creature->IsPlayer())
    {
        if (RollChance(cfg.killPlayer))
        {
            placeholders["%victim_class"] = FormatClassName(creature->GetClass());

            return BroadcastToChannelWithGlobalChance(
                ai,
                PlayerbotTextMgr::instance().GetBotText("broadcast_killed_player", placeholders),
                { {TO_WORLD_DEFENSE, 50}, {TO_LOCAL_DEFENSE, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }
    else
    {
        char const* textName = nullptr;
        int32 chance = 0;
        switch (creature->GetCreatureTemplate()->rank)
        {
            case CREATURE_ELITE_NORMAL:
                textName = "broadcast_killed_normal"; chance = cfg.killNormal; break;
            case CREATURE_ELITE_ELITE:
                textName = "broadcast_killed_elite"; chance = cfg.killElite; break;
            case CREATURE_ELITE_RAREELITE:
                textName = "broadcast_killed_rareelite"; chance = cfg.killRareelite; break;
            case CREATURE_ELITE_WORLDBOSS:
                textName = "broadcast_killed_worldboss"; chance = cfg.killWorldboss; break;
            case CREATURE_ELITE_RARE:
                textName = "broadcast_killed_rare"; chance = cfg.killRare; break;
            case CREATURE_UNKNOWN:
                textName = "broadcast_killed_unknown"; chance = cfg.killUnknown; break;
            default:
                break;
        }

        if (textName && RollChance(chance))
        {
            return BroadcastToChannelWithGlobalChance(
                ai,
                PlayerbotTextMgr::instance().GetBotText(textName, placeholders),
                { {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
            );
        }
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:579-626
bool BroadcastHelper::BroadcastLevelup(PlayerbotAI* ai, Player* bot)
{
    BroadcastCfg const& cfg = Cfg();
    if (!cfg.enableBroadcasts)
        return false;
    uint32 level = bot->GetLevel();

    std::map<std::string, std::string> placeholders;
    AddAreaPlaceholders(bot, placeholders);
    AddSelfPlaceholders(bot, placeholders);
    placeholders["%my_level"] = std::to_string(level);

    if (level == sPlayerbotAIConfig->randomBotMaxLevel && RollChance(cfg.levelupMaxLevel))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_max_level", placeholders),
            { {TO_GUILD, 30}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    // It's divisible by 10
    else if (level % 10 == 0 && RollChance(cfg.levelupTenX))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_10x", placeholders),
            { {TO_GUILD, 50}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }
    else if (RollChance(cfg.levelupGeneric))
    {
        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("broadcast_levelup_generic", placeholders),
            { {TO_GUILD, 90}, {TO_WORLD, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:628-644
bool BroadcastHelper::BroadcastGuildMemberPromotion(PlayerbotAI* ai, Player* /* bot */, Player* player)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().guildManagement))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = FormatClassName(player->GetClass());
        placeholders["%other_race"] = FormatRaceName(player->GetRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("broadcast_guild_promotion", placeholders));
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:646-660 (AC has no EnableBroadcasts check here; with broadcasts off the
// roll cannot pass, which matches AC's broadcastChanceMaxValue = 0 behaviour closely enough).
bool BroadcastHelper::BroadcastGuildMemberDemotion(PlayerbotAI* ai, Player* /* bot */, Player* player)
{
    if (RollChance(Cfg().guildManagement))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = player->GetName();
        placeholders["%other_class"] = FormatClassName(player->GetClass());
        placeholders["%other_race"] = FormatRaceName(player->GetRace());
        placeholders["%other_level"] = std::to_string(player->GetLevel());

        return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("broadcast_guild_demotion", placeholders));
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:662-698
bool BroadcastHelper::BroadcastGuildGroupOrRaidInvite(PlayerbotAI* ai, Player* /* bot */, Player* player, Group* group)
{
    if (!Cfg().enableBroadcasts)
        return false;
    std::map<std::string, std::string> placeholders;
    placeholders["%name"] = player->GetName();
    AddAreaPlaceholders(ai->GetBot(), placeholders);

    //TODO move texts to sql!
    if (group && group->isRaidGroup())
    {
        if (urand(0, 3))
        {
            return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("Hey anyone want to raid in %zone_name", placeholders));
        }
        else
        {
            return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("Hey %name I'm raiding in %zone_name do you wan to join me?", placeholders));
        }
    }
    else
    {
        //(bot->GetTeam() == ALLIANCE ? LANG_COMMON : LANG_ORCISH)
        if (urand(0, 3))
        {
            return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("Hey anyone wanna group up in %zone_name?", placeholders));
        }
        else
        {
            return SayToGuild(ai->GetBot(), PlayerbotTextMgr::instance().GetBotText("Hey %name do you want join my group? I'm heading for %zone_name", placeholders));
        }
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:700-726
bool BroadcastHelper::BroadcastSuggestInstance(PlayerbotAI* ai, std::vector<std::string>& allowedInstances, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestInstance))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = FormatRole(bot);

        std::ostringstream itemout;
        //itemout << "|c00b000b0" << allowedInstances[urand(0, allowedInstances.size() - 1)] << "|r";
        itemout << allowedInstances[urand(0, allowedInstances.size() - 1)];
        placeholders["%instance_name"] = itemout.str();

        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_instance", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:728-754
bool BroadcastHelper::BroadcastSuggestQuest(PlayerbotAI* ai, std::vector<uint32>& quests, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestQuest))
    {

        int index = rand() % quests.size();

        Quest const* quest = sObjectMgr->GetQuestTemplate(quests[index]);
        if (!quest)
            return false;

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = FormatRole(bot);
        placeholders["%quest_link"] = ChatHelper::FormatQuest(quest);
        placeholders["%quest_level"] = std::to_string(quest->GetQuestLevel());
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_quest", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:756-779
bool BroadcastHelper::BroadcastSuggestGrindMaterials(PlayerbotAI* ai, std::string item, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestGrindMaterials))
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = FormatRole(bot);
        placeholders["%category"] = item;

        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_trade", placeholders),
            { {TO_TRADE, 50}, {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:781-810
bool BroadcastHelper::BroadcastSuggestGrindReputation(PlayerbotAI* ai, std::vector<std::string> levels, std::vector<std::string> allowedFactions, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestGrindReputation))
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = FormatRole(bot);
        placeholders["%rep_level"] = levels[urand(0, 2)];
        std::ostringstream rnd; rnd << urand(1, 5) << "K";
        placeholders["%rndK"] = rnd.str();

        std::ostringstream itemout;
        //itemout << "|c004040b0" << allowedFactions[urand(0, allowedFactions.size() - 1)] << "|r";
        itemout << allowedFactions[urand(0, allowedFactions.size() - 1)];
        placeholders["%faction"] = itemout.str();

        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_faction", placeholders),
            { {TO_LOOKING_FOR_GROUP, 50}, {TO_GUILD, 50}, {TO_WORLD, 50}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:812-837
bool BroadcastHelper::BroadcastSuggestSell(PlayerbotAI* ai, ItemTemplate const* proto, uint32 count, uint32 price, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestSell))
    {

        std::map<std::string, std::string> placeholders;
        placeholders["%item_link"] = ChatHelper::FormatItem(proto, 0);
        placeholders["%item_formatted_link"] = ChatHelper::FormatItem(proto, count);
        placeholders["%item_count"] = std::to_string(count);
        placeholders["%cost_gold"] = ChatHelper::formatMoney(price);

        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_sell", placeholders),
            { {TO_TRADE, 90}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:839-863
bool BroadcastHelper::BroadcastSuggestSomething(PlayerbotAI* ai, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestSomething))
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%my_role"] = FormatRole(bot);

        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_something", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:865-896
bool BroadcastHelper::BroadcastSuggestSomethingToxic(PlayerbotAI* ai, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestSomethingToxic))
    {
        //items
        std::vector<Item*> botItems = InventoryAndEquippedItems(bot);

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ChatHelper::FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : PlayerbotTextMgr::instance().GetBotText("string_empty_link");

        placeholders["%my_role"] = FormatRole(bot);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_something_toxic", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:898-946
bool BroadcastHelper::BroadcastSuggestToxicLinks(PlayerbotAI* ai, Player* bot)
{
    if (!Cfg().enableBroadcasts)
        return false;
    if (RollChance(Cfg().suggestToxicLinks))
    {
        //quests
        std::vector<uint32> incompleteQuests;
        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            QuestStatus status = bot->GetQuestStatus(questId);
            if (status == QUEST_STATUS_INCOMPLETE || status == QUEST_STATUS_NONE)
                incompleteQuests.push_back(questId);
        }

        //items
        std::vector<Item*> botItems = InventoryAndEquippedItems(bot);

        //spells
        //?

        std::map<std::string, std::string> placeholders;

        placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ChatHelper::FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : PlayerbotTextMgr::instance().GetBotText("string_empty_link");
        placeholders["%prefix"] = Cfg().toxicLinksPrefix;

        if (incompleteQuests.size() > 0)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()]);
            placeholders["%random_taken_quest_or_item_link"] = quest ? ChatHelper::FormatQuest(quest) : placeholders["%random_inventory_item_link"];
        }
        else
        {
            placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
        }

        placeholders["%my_role"] = FormatRole(bot);
        AddAreaPlaceholders(bot, placeholders);
        AddSelfPlaceholders(bot, placeholders);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders),
            { {TO_GUILD, 10}, {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}

// AC Util/BroadcastHelper.cpp:948-963 (no EnableBroadcasts check in AC; RollChance() is false when the max is 0)
bool BroadcastHelper::BroadcastSuggestThunderfury(PlayerbotAI* ai, Player* /*bot*/)
{
    if (RollChance(Cfg().suggestThunderfury))
    {
        std::map<std::string, std::string> placeholders;
        ItemTemplate const* thunderfuryProto = sObjectMgr->GetItemTemplate(19019);
        if (!thunderfuryProto)
            return false;
        placeholders["%thunderfury_link"] = ChatHelper::FormatItem(thunderfuryProto);

        return BroadcastToChannelWithGlobalChance(
            ai,
            PlayerbotTextMgr::instance().GetBotText("thunderfury_spam", placeholders),
            { {TO_WORLD, 70}, {TO_GENERAL, 100} }
        );
    }

    return false;
}
