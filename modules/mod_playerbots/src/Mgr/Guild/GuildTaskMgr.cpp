/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Mgr/Guild/GuildTaskMgr.cpp: guild kill / item tasks advertised and rewarded by mail.
 * Dormant exactly as in AC: AiPlayerbot.EnableGuildTasks defaults to 0 (every Get*TaskValue returns 0 then), and the
 * only caller of Update, in RandomPlayerbotMgr::ProcessBot, is commented out in AC and here. The item / kill checks are
 * wired where AC calls them (CheckMailAction, StoreLootAction, TradeStatusAction, ItemUsageValue, the kill-task script
 * hook) and the ".playerbots gtask" console command is registered (cs_playerbots.cpp).
 *
 * 5.4.8 / fork adaptations:
 *  - LOG_* {} -> TC_LOG_* %s; Query("{}") -> PQuery("%u"); SetData() -> setUInt32()/setString(); Field::Get<T>() ->
 *    GetUInt32()/GetFloat()/GetString(); sPlayerbotAIConfig. / sRandomPlayerbotMgr. -> ->; ServerFacade::instance() ->
 *    sServerFacade; ChatHandler::PSendSysMessage(str) -> SendSysMessage(str); dynamic_cast<Creature*> -> ToCreature().
 *  - World DB columns are 5.4.8's (creature_template entry/name/minlevel/maxlevel/`rank`), AreaTableEntry::area_name is
 *    a single string.
 *  - The random-item lists AC takes from RandomItemMgr (RANDOM_ITEM_GUILD_TASK*, GetRandomItem, IsUsedBySkill,
 *    RandomItemMgr.cpp:15-82/143-160/947-956/1368-1441/1443-1556/1834-1872) do not exist in this fork's
 *    RandomItemManager. The same predicates, item filter and level buckets are kept file-local below, built in memory on
 *    first use (AC also stores the lists in playerbots_rnditem_cache; nothing else reads that table here). The level
 *    bucket is AC's ((level - 1) / 10 of the player level against the item level), carried over as is.
 *  - "gtask reset" runs a plain DELETE: the core's PlayerbotsDatabase has no PLAYERBOTS_DEL_GUILD_TASKS_ALL statement.
 *  - CheckKillTaskInternal looks the guild up only for the debug log and tolerates a missing one.
 */

#include "GuildTaskMgr.h"

#include "CharacterCache.h"
#include "Chat.h"
#include "ChatHelper.h"
#include "Creature.h"
#include "DBCStores.h"
#include "DatabaseEnv.h"
#include "Group.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Item.h"
#include "Log.h"
#include "Mail.h"
#include "Map.h"
#include "MapManager.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "PlayerbotSecurity.h"
#include "Playerbots.h"
#include "ServerFacade.h"
#include "SpellInfo.h"
#include "SpellMgr.h"
#include "Util.h"

#include <algorithm>
#include <ctime>
#include <mutex>
#include <sstream>
#include <unordered_map>
#include <unordered_set>

enum GuildTaskType
{
    GUILD_TASK_TYPE_NONE = 0,
    GUILD_TASK_TYPE_ITEM = 1,
    GUILD_TASK_TYPE_KILL = 2
};

namespace
{
// ---- AC RandomItemMgr pieces the guild tasks use (see the file header) ----

// AC Mgr/Item/RandomItemMgr.h:24-31
enum RandomItemType
{
    RANDOM_ITEM_GUILD_TASK,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE,
    RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE,
    RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE
};

// AC Mgr/Item/RandomItemMgr.h:95-101
class RandomItemPredicate
{
public:
    virtual ~RandomItemPredicate() {}

    virtual bool Apply(ItemTemplate const* proto) = 0;
};

// AC Mgr/Item/RandomItemMgr.cpp:15-39 (5.4.8 bind names: BIND_ON_ACQUIRE / BIND_ON_USE / BIND_QUEST)
class RandomItemGuildTaskPredicate : public RandomItemPredicate
{
public:
    bool Apply(ItemTemplate const* proto) override
    {
        if (!proto)
            return false;

        if (proto->Bonding == BIND_ON_ACQUIRE || proto->Bonding == BIND_ON_USE || proto->Bonding == BIND_QUEST)
            return false;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return false;

        if ((proto->Class == ITEM_CLASS_WEAPON || proto->Class == ITEM_CLASS_ARMOR) && proto->Quality >= ITEM_QUALITY_RARE)
            return true;

        if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_TRADE_GOODS)
            return true;

        return false;
    }
};

// AC Mgr/Item/RandomItemMgr.cpp:41-82
class RandomItemGuildTaskRewardPredicate : public RandomItemPredicate
{
public:
    RandomItemGuildTaskRewardPredicate(bool equip, bool rare) : equip(equip), rare(rare) {}

    bool Apply(ItemTemplate const* proto) override
    {
        if (!proto)
            return false;

        if (proto->Bonding == BIND_ON_ACQUIRE || proto->Bonding == BIND_ON_USE || proto->Bonding == BIND_QUEST)
            return false;

        if (proto->Class == ITEM_CLASS_QUEST)
            return false;

        if (equip)
        {
            uint32 desiredQuality = rare ? ITEM_QUALITY_RARE : ITEM_QUALITY_UNCOMMON;
            if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_EPIC)
                return false;

            if (proto->Class == ITEM_CLASS_WEAPON || proto->Class == ITEM_CLASS_ARMOR)
                return true;
        }
        else
        {
            uint32 desiredQuality = rare ? ITEM_QUALITY_UNCOMMON : ITEM_QUALITY_NORMAL;
            if (proto->Quality < desiredQuality || proto->Quality >= ITEM_QUALITY_RARE)
                return false;

            if (proto->Class == ITEM_CLASS_CONSUMABLE || proto->Class == ITEM_CLASS_TRADE_GOODS)
                return true;
        }

        return false;
    }

private:
    bool equip;
    bool rare;
};

// AC RandomItemMgr::IsInternalItem (RandomItemMgr.cpp:1368-1389)
bool IsInternalItem(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    if (proto->Flags & ITEM_PROTO_FLAG_DEPRECATED)
        return true;

    char const* name = proto->Name1.c_str();
    return strstr(name, "NPC ") || strstr(name, "Monster ") || strstr(name, "Test") || strstr(name, "Deprecated ") ||
           strstr(name, "[PH]") || strstr(name, "TEST") || strstr(name, "Unused ") || strstr(name, "zzOLD") ||
           strstr(name, "(test)") || strstr(name, "(OLD)") || strstr(name, "QR") || strstr(name, "2200 ");
}

// AC RandomItemMgr::IsValidItem (RandomItemMgr.cpp:1391-1441). The fork has no AiPlayerbot unobtainable-items list.
bool IsValidItem(ItemTemplate const* proto)
{
    if (!proto)
        return false;

    if (proto->ItemLevel == 0)
        return false;

    if (proto->Duration)
        return false;

    if (proto->Area || proto->Map)
        return false;

    if (proto->RequiredCityRank || proto->RequiredHonorRank)
        return false;

    if ((proto->AllowableClass & CLASSMASK_ALL_PLAYABLE) == 0)
        return false;

    if ((proto->AllowableRace & RACEMASK_ALL_PLAYABLE) == 0)
        return false;

    if (IsInternalItem(proto))
        return false;

    return true;
}

// AC RandomItemMgr.cpp:1443-1527 (IsCraftedBySpellInfo / IsCraftedBySpell / IsCraftedBySkill)
bool IsCraftedBySpellInfo(uint32 itemId, SpellInfo const* spellInfo)
{
    for (uint8 x = 0; x < MAX_SPELL_REAGENTS; ++x)
    {
        if (spellInfo->Reagent[x] <= 0)
            continue;

        if (itemId == static_cast<uint32>(spellInfo->Reagent[x]))
            return true;
    }

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        if (spellInfo->Effects[i].Effect == SPELL_EFFECT_CREATE_ITEM && spellInfo->Effects[i].ItemType == itemId)
            return true;
    }

    return false;
}

bool IsCraftedBySpell(uint32 itemId, uint32 spellId)
{
    SpellInfo const* spellInfo = sSpellMgr->GetSpellInfo(spellId);
    if (!spellInfo)
        return false;

    if (IsCraftedBySpellInfo(itemId, spellInfo))
        return true;

    for (uint8 i = 0; i < MAX_SPELL_EFFECTS; ++i)
    {
        SpellInfo const* triggerSpellInfo = sSpellMgr->GetSpellInfo(spellInfo->Effects[i].TriggerSpell);
        if (!triggerSpellInfo)
            continue;

        if (IsCraftedBySpellInfo(itemId, triggerSpellInfo))
            return true;
    }

    return false;
}

bool IsCraftedBySkill(uint32 itemId, uint32 skillId)
{
    // AC: GetSkillLineAbilitiesBySkillLine(skillId); 5.4.8 walks the SkillLineAbility store.
    for (uint32 j = 0; j < sSkillLineAbilityStore.GetNumRows(); ++j)
    {
        SkillLineAbilityEntry const* skillLine = sSkillLineAbilityStore.LookupEntry(j);
        if (!skillLine || skillLine->SkillLine != skillId)
            continue;

        if (IsCraftedBySpell(itemId, skillLine->Spell))
            return true;
    }

    for (auto const& itr : *sObjectMgr->GetItemTemplateStore())
    {
        ItemTemplate const* recipe = &itr.second;
        if (recipe->Class != ITEM_CLASS_RECIPE)
            continue;

        uint32 skillType = 0;
        switch (recipe->SubClass)
        {
            case ITEM_SUBCLASS_LEATHERWORKING_PATTERN: skillType = SKILL_LEATHERWORKING; break;
            case ITEM_SUBCLASS_TAILORING_PATTERN:      skillType = SKILL_TAILORING; break;
            case ITEM_SUBCLASS_ENGINEERING_SCHEMATIC:  skillType = SKILL_ENGINEERING; break;
            case ITEM_SUBCLASS_BLACKSMITHING:          skillType = SKILL_BLACKSMITHING; break;
            case ITEM_SUBCLASS_COOKING_RECIPE:         skillType = SKILL_COOKING; break;
            case ITEM_SUBCLASS_ALCHEMY_RECIPE:         skillType = SKILL_ALCHEMY; break;
            case ITEM_SUBCLASS_FIRST_AID_MANUAL:       skillType = SKILL_FIRST_AID; break;
            case ITEM_SUBCLASS_ENCHANTING_FORMULA:     skillType = SKILL_ENCHANTING; break;
            case ITEM_SUBCLASS_JEWELCRAFTING_RECIPE:   skillType = SKILL_JEWELCRAFTING; break;
            case ITEM_SUBCLASS_FISHING_MANUAL:         skillType = SKILL_FISHING; break;
            default: break;
        }

        if (!skillType)
            continue;

        for (uint32 i = 0; i < MAX_ITEM_PROTO_SPELLS; ++i)
        {
            if (recipe->Spells[i].SpellId > 0 && IsCraftedBySpell(itemId, recipe->Spells[i].SpellId))
                return true;
        }
    }

    return false;
}

std::mutex sUsedBySkillLock;
std::unordered_set<uint32> sUsedBySkillCache;  // AC RandomItemMgr::itemCache

// AC RandomItemMgr::IsUsedBySkill (RandomItemMgr.cpp:1529-1556)
bool IsUsedBySkill(ItemTemplate const* proto, uint32 skillId)
{
    if (!proto)
        return false;

    {
        std::lock_guard<std::mutex> lock(sUsedBySkillLock);
        if (sUsedBySkillCache.count(proto->ItemId))
            return true;
    }

    switch (proto->Class)
    {
        case ITEM_CLASS_TRADE_GOODS:
        case ITEM_CLASS_MISCELLANEOUS:  // AC ITEM_CLASS_MISC
        case ITEM_CLASS_REAGENT:
        case ITEM_CLASS_GEM:
            if (IsCraftedBySkill(proto->ItemId, skillId))
            {
                std::lock_guard<std::mutex> lock(sUsedBySkillLock);
                sUsedBySkillCache.insert(proto->ItemId);
                return true;
            }
            break;
        default:
            break;
    }

    return false;
}

// AC RandomItemMgr::BuildCacheRandomItem (RandomItemMgr.cpp:1834-1872), in memory only.
std::once_flag sRandomItemCacheOnce;
std::unordered_map<uint32, std::unordered_map<uint32, std::vector<uint32>>> sRandomItemCache;

void BuildRandomItemCache()
{
    RandomItemGuildTaskPredicate guildTask;
    RandomItemGuildTaskRewardPredicate equipGreen(true, false), equipBlue(true, true), trade(false, false),
        tradeRare(false, true);
    std::unordered_map<uint32, RandomItemPredicate*> predicates;
    predicates[RANDOM_ITEM_GUILD_TASK] = &guildTask;
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN] = &equipGreen;
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE] = &equipBlue;
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE] = &trade;
    predicates[RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE] = &tradeRare;

    uint32 count = 0;
    for (auto const& itr : *sObjectMgr->GetItemTemplateStore())
    {
        ItemTemplate const* proto = &itr.second;

        // skip items that fail basic validatoin (level, duration, flags, etc.)
        if (!IsValidItem(proto))
            continue;

        // skip items with no sell price
        if (proto->SellPrice == 0)
            continue;

        uint32 const bucket = (proto->ItemLevel - 1) / 10;

        for (uint32 type = RANDOM_ITEM_GUILD_TASK; type <= RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE; ++type)
        {
            if (predicates[type] && !predicates[type]->Apply(proto))
                continue;

            sRandomItemCache[bucket][type].push_back(itr.first);
            ++count;
        }
    }

    TC_LOG_INFO("playerbots", "Guild tasks: cached %u random items", count);
}

// AC RandomItemMgr::GetRandomItem + Query + NormalizeLevel (RandomItemMgr.cpp:143-160, 947-956, 3040-3045)
uint32 GetRandomItem(uint32 level, RandomItemType type, RandomItemPredicate* predicate = nullptr)
{
    std::call_once(sRandomItemCacheOnce, BuildRandomItemCache);

    level = std::min(level, std::min(sPlayerbotAIConfig->randomBotMaxLevel, static_cast<uint32>(DEFAULT_MAX_LEVEL)));

    auto const levelItr = sRandomItemCache.find((level - 1) / 10);
    if (levelItr == sRandomItemCache.end())
        return 0;

    auto const typeItr = levelItr->second.find(type);
    if (typeItr == levelItr->second.end())
        return 0;

    std::vector<uint32> list;
    for (uint32 const itemId : typeItr->second)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            continue;

        if (predicate && !predicate->Apply(proto))
            continue;

        list.push_back(itemId);
    }

    if (list.empty())
        return 0;

    return list[urand(0, list.size() - 1)];
}

// AC PlayerbotFactory::tradeSkills (Bot/Factory/PlayerbotFactory.cpp:57-61); AC's loop below reads the first 13.
uint32 const GuildTaskTradeSkills[] = {SKILL_ALCHEMY,     SKILL_ENCHANTING,    SKILL_SKINNING, SKILL_TAILORING,
                                       SKILL_LEATHERWORKING, SKILL_ENGINEERING, SKILL_HERBALISM, SKILL_INSCRIPTION,
                                       SKILL_MINING,      SKILL_BLACKSMITHING, SKILL_COOKING,  SKILL_FIRST_AID,
                                       SKILL_FISHING,     SKILL_JEWELCRAFTING};

class RandomItemBySkillGuildTaskPredicate : public RandomItemPredicate
{
public:
    RandomItemBySkillGuildTaskPredicate(Player* player) : RandomItemPredicate(), player(player) {}

    bool Apply(ItemTemplate const* proto) override
    {
        for (uint32 i = 0; i < 13; ++i)
        {
            uint32 skillId = GuildTaskTradeSkills[i];
            if (player->HasSkill(skillId) && IsUsedBySkill(proto, skillId))
                return true;
        }

        return false;
    }

private:
    Player* player;
};

std::string const formatTime(uint32 secs)
{
    std::ostringstream out;
    if (secs < 3600)
    {
        out << secs / 60 << " min";
    }
    else if (secs < 7200)
    {
        out << "1 hr " << (secs - 3600) / 60 << " min";
    }
    else if (secs < 3600 * 24)
    {
        out << secs / 3600 << " hr";
    }
    else
    {
        out << secs / 3600 / 24 << " days";
    }

    return out.str();
}

std::string const formatDateTime(uint32 secs)
{
    time_t rawtime = time(nullptr) + secs;
    tm* timeinfo = localtime(&rawtime);

    char buffer[256];
    strftime(buffer, sizeof(buffer), "%b %d, %H:%M", timeinfo);
    return std::string(buffer);
}

std::string const GetHelloText(uint32 owner)
{
    ObjectGuid ownerGUID = ObjectGuid::Create<HighGuid::Player>(owner);

    std::ostringstream body;
    body << "Hello";

    std::string playerName;
    sCharacterCache->GetCharacterNameByGuid(ownerGUID, playerName);
    if (!playerName.empty())
        body << ", " << playerName;

    body << ",\n\n";

    return body.str();
}
}  // namespace

void GuildTaskMgr::Update(Player* player, Player* guildMaster)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
        return;

    if (!GetTaskValue(0, 0, "advert_cleanup"))
    {
        CleanupAdverts();
        RemoveDuplicatedAdverts();
        SetTaskValue(0, 0, "advert_cleanup", 1, sPlayerbotAIConfig->guildTaskAdvertCleanupTime);
    }

    PlayerbotAI* masterBotAI = GET_PLAYERBOT_AI(guildMaster);
    uint32 guildId = guildMaster->GetGuildId();
    if (!guildId || !masterBotAI || !guildMaster->GetGuildId())
        return;

    if (!player->IsFriendlyTo(guildMaster))
        return;

    Guild* guild = sGuildMgr->GetGuildById(guildMaster->GetGuildId());
    if (!guild)
        return;

    DenyReason reason = PLAYERBOT_DENY_NONE;
    PlayerbotSecurityLevel secLevel = masterBotAI->GetSecurity()->LevelFor(player, &reason);
    if (secLevel == PLAYERBOT_SECURITY_DENY_ALL || (secLevel == PLAYERBOT_SECURITY_TALK && reason != PLAYERBOT_DENY_FAR))
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: skipping guild task update - not enough security level, reason = %u",
                     guild->GetName().c_str(), player->GetName().c_str(), uint32(reason));
        return;
    }

    TC_LOG_DEBUG("playerbots", "%s: guild task update for player %s", guild->GetName().c_str(), player->GetName().c_str());

    ObjectGuid::LowType owner = player->GetGUID().GetCounter();

    uint32 activeTask = GetTaskValue(owner, guildId, "activeTask");
    if (!activeTask)
    {
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "itemTask", 0, 0);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "killTask", 0, 0);
        SetTaskValue(owner, guildId, "killCount", 0, 0);
        SetTaskValue(owner, guildId, "payment", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);

        uint32 task = CreateTask(player, guildId);

        if (task == GUILD_TASK_TYPE_NONE)
        {
            TC_LOG_ERROR("playerbots", "%s / %s: error creating guild task", guild->GetName().c_str(),
                         player->GetName().c_str());
        }

        uint32 time = urand(sPlayerbotAIConfig->minGuildTaskChangeTime, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "activeTask", task, time);
        SetTaskValue(owner, guildId, "advertisement", 1,
                     urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime,
                           sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));

        TC_LOG_DEBUG("playerbots", "%s / %s: guild task %u is set for %u secs", guild->GetName().c_str(),
                     player->GetName().c_str(), task, time);
        return;
    }

    CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();

    uint32 advertisement = GetTaskValue(owner, guildId, "advertisement");
    if (!advertisement)
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: sending advertisement", guild->GetName().c_str(), player->GetName().c_str());

        if (SendAdvertisement(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "advertisement", 1,
                         urand(sPlayerbotAIConfig->minGuildTaskAdvertisementTime,
                               sPlayerbotAIConfig->maxGuildTaskAdvertisementTime));
        }
        else
        {
            TC_LOG_DEBUG("playerbots", "%s / %s: error sending advertisement", guild->GetName().c_str(),
                         player->GetName().c_str());
        }
    }

    uint32 thanks = GetTaskValue(owner, guildId, "thanks");
    if (!thanks)
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: sending thanks", guild->GetName().c_str(), player->GetName().c_str());

        if (SendThanks(trans, owner, guildId, GetTaskValue(owner, guildId, "payment")))
        {
            SetTaskValue(owner, guildId, "thanks", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            TC_LOG_DEBUG("playerbots", "%s / %s: error sending thanks", guild->GetName().c_str(),
                         player->GetName().c_str());
        }
    }

    uint32 reward = GetTaskValue(owner, guildId, "reward");
    if (!reward)
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: sending reward", guild->GetName().c_str(), player->GetName().c_str());

        if (Reward(trans, owner, guildId))
        {
            SetTaskValue(owner, guildId, "reward", 1, 2 * sPlayerbotAIConfig->maxGuildTaskChangeTime);
            SetTaskValue(owner, guildId, "payment", 0, 0);
        }
        else
        {
            TC_LOG_DEBUG("playerbots", "%s / %s: error sending reward", guild->GetName().c_str(),
                         player->GetName().c_str());
        }
    }

    CharacterDatabase.CommitTransaction(trans);
}

uint32 GuildTaskMgr::CreateTask(Player* owner, uint32 guildId)
{
    switch (urand(0, 1))
    {
        case 0:
            CreateItemTask(owner, guildId);
            return GUILD_TASK_TYPE_ITEM;
        default:
            CreateKillTask(owner, guildId);
            return GUILD_TASK_TYPE_KILL;
    }
}

bool GuildTaskMgr::CreateItemTask(Player* player, uint32 guildId)
{
    if (!player || player->GetLevel() < 5)
        return false;

    Guild* guild = sGuildMgr->GetGuildById(guildId);
    std::string const guildName = guild ? guild->GetName() : "";

    RandomItemBySkillGuildTaskPredicate predicate(player);
    uint32 itemId = GetRandomItem(player->GetLevel() - 5, RANDOM_ITEM_GUILD_TASK, &predicate);
    if (!itemId)
    {
        TC_LOG_ERROR("playerbots", "%s / %s: no items avaible for item task", guildName.c_str(), player->GetName().c_str());
        return false;
    }

    uint32 count = GetMaxItemTaskCount(itemId);

    TC_LOG_DEBUG("playerbots", "%s / %s: item task %u (x%u)", guildName.c_str(), player->GetName().c_str(), itemId, count);

    SetTaskValue(player->GetGUID().GetCounter(), guildId, "itemCount", count, sPlayerbotAIConfig->maxGuildTaskChangeTime);
    SetTaskValue(player->GetGUID().GetCounter(), guildId, "itemTask", itemId, sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::CreateKillTask(Player* player, uint32 guildId)
{
    if (!player)
        return false;

    Guild* guild = sGuildMgr->GetGuildById(guildId);
    std::string const guildName = guild ? guild->GetName() : "";

    uint32 rank = !urand(0, 2) ? CREATURE_ELITE_RAREELITE : CREATURE_ELITE_RARE;

    std::vector<uint32> ids;

    uint32 level = player->GetLevel();
    QueryResult results = WorldDatabase.PQuery(
        "SELECT ct.entry, c.map, c.position_x, c.position_y, ct.name FROM creature_template ct "
        "JOIN creature c ON ct.entry = c.id WHERE ct.maxlevel < %u AND ct.minlevel > %u AND ct.`rank` = %u ",
        level + 4, level - 3, rank);
    if (results)
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 id = fields[0].GetUInt32();
            uint32 map = fields[1].GetUInt32();
            float x = fields[2].GetFloat();
            float y = fields[3].GetFloat();
            std::string const name = fields[4].GetString();

            if (strstr(name.c_str(), "UNUSED"))
                continue;

            float dist = sServerFacade->GetDistance2d(player, x, y);
            if (dist > sPlayerbotAIConfig->guildTaskKillTaskDistance || player->GetMapId() != map)
                continue;

            if (std::find(ids.begin(), ids.end(), id) == ids.end())
                ids.push_back(id);
        } while (results->NextRow());
    }

    if (ids.empty())
    {
        TC_LOG_ERROR("playerbots", "%s / %s: no rare creatures available for kill task", guildName.c_str(),
                     player->GetName().c_str());
        return false;
    }

    uint32 index = urand(0, ids.size() - 1);
    uint32 creatureId = ids[index];

    TC_LOG_DEBUG("playerbots", "%s / %s: kill task %u", guildName.c_str(), player->GetName().c_str(), creatureId);

    SetTaskValue(player->GetGUID().GetCounter(), guildId, "killTask", creatureId, sPlayerbotAIConfig->maxGuildTaskChangeTime);

    return true;
}

bool GuildTaskMgr::SendAdvertisement(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 validIn;
    GetTaskValue(owner, guildId, "activeTask", &validIn);

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
        return SendItemAdvertisement(trans, itemTask, owner, guildId, validIn);

    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (killTask)
        return SendKillAdvertisement(trans, killTask, owner, guildId, validIn);

    return false;
}

bool GuildTaskMgr::SendItemAdvertisement(CharacterDatabaseTransaction& trans, uint32 itemId, uint32 owner,
                                         uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "We are in a great need of " << proto->Name1 << ". If you could sell us ";
    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (count > 1)
        body << "at least " << count << " of them ";
    else
        body << "some ";
    body << "we'd really appreciate that and pay a high price.\n\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: " << proto->Name1;
    if (count > 1)
        subject << " (x" << count << ")";

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendKillAdvertisement(CharacterDatabaseTransaction& trans, uint32 creatureId, uint32 owner,
                                         uint32 guildId, uint32 validIn)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;
    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId);
    if (!proto)
        return false;

    QueryResult result =
        WorldDatabase.PQuery("SELECT map, position_x, position_y, position_z FROM creature WHERE id = %u", creatureId);
    if (!result)
        return false;

    std::string location;
    do
    {
        Field* fields = result->Fetch();
        uint32 mapid = fields[0].GetUInt32();
        float x = fields[1].GetFloat();
        float y = fields[2].GetFloat();
        float z = fields[3].GetFloat();
        Map* map = sMapMgr->FindMap(mapid, 0);
        if (!map)
            continue;

        AreaTableEntry const* entry = sAreaTableStore.LookupEntry(map->GetAreaId(PHASEMASK_NORMAL, x, y, z));
        if (!entry)
            continue;

        location = entry->area_name;
        break;
    } while (result->NextRow());

    std::ostringstream body;
    body << GetHelloText(owner);
    body << "As you probably know " << proto->Name << " is wanted dead for the crimes it did against our guild. If you should kill it ";
    body << "we'd really appreciate that.\n\n";
    if (!location.empty())
        body << proto->Name << "'s the last known location was " << location << ".\n";
    body << "The task will expire at " << formatDateTime(validIn) << "\n";
    body << "\n";
    body << "Best Regards,\n";
    body << guild->GetName() << "\n";
    body << leader->GetName() << "\n";

    std::ostringstream subject;
    subject << "Guild Task: ";
    if (proto->rank == CREATURE_ELITE_ELITE || proto->rank == CREATURE_ELITE_RAREELITE ||
        proto->rank == CREATURE_ELITE_WORLDBOSS)
        subject << "(Elite) ";
    subject << proto->Name;
    if (!location.empty())
        subject << ", " << location;

    MailDraft(subject.str(), body.str()).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    return true;
}

bool GuildTaskMgr::SendThanks(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId, uint32 payment)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        std::ostringstream body;
        body << GetHelloText(owner);
        body << "One of our guild members wishes to thank you for the " << proto->Name1 << "!";
        uint32 count = GetTaskValue(owner, guildId, "itemCount");
        if (count)
        {
            body << " If we have another ";
            body << count << " of them that would help us tremendously.\n";
        }
        body << "\n";
        body << "Thanks again,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";

        MailDraft("Thank You", body.str()).AddMoney(payment).SendMailTo(trans, MailReceiver(owner), MailSender(leader));

        Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
        if (player)
            SendCompletionMessage(player, "payed for");

        return true;
    }

    return false;
}

uint32 GuildTaskMgr::GetMaxItemTaskCount(uint32 itemId)
{
    ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
    if (!proto)
        return 0;

    if (!proto->Stackable || proto->GetMaxStackSize() == 1)
        return 1;

    if (proto->Quality == ITEM_QUALITY_NORMAL)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return urand(1, 3) * proto->GetMaxStackSize();
            case 10:
                return urand(2, 6) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(4, 12) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    if (proto->Quality < ITEM_QUALITY_RARE)
    {
        switch (proto->GetMaxStackSize())
        {
            case 5:
                return proto->GetMaxStackSize();
            case 10:
                return urand(1, 2) * proto->GetMaxStackSize() / 2;
            case 20:
                return urand(1, 4) * proto->GetMaxStackSize() / 4;
            default:
                return proto->GetMaxStackSize();
        }
    }

    return 1;
}

bool GuildTaskMgr::IsGuildTaskItem(uint32 itemId, uint32 guildId)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        return 0;
    }

    uint32 value = 0;

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_VALUE);
    stmt->setUInt32(0, itemId);
    stmt->setUInt32(1, guildId);
    stmt->setString(2, "itemTask");
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        value = fields[0].GetUInt32();
        uint32 lastChangeTime = fields[1].GetUInt32();
        uint32 validIn = fields[2].GetUInt32();
        if ((time(nullptr) - lastChangeTime) >= validIn)
            value = 0;
    }

    return value;
}

std::map<uint32, uint32> GuildTaskMgr::GetTaskValues(uint32 owner, std::string const type, uint32* /*validIn = nullptr*/)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        return std::map<uint32, uint32>();
    }

    std::map<uint32, uint32> results;

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER);
    stmt->setUInt32(0, owner);
    stmt->setString(1, type);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 value = fields[0].GetUInt32();
            uint32 lastChangeTime = fields[1].GetUInt32();
            uint32 secs = fields[2].GetUInt32();
            uint32 guildId = fields[3].GetUInt32();
            if ((time(nullptr) - lastChangeTime) >= secs)
                value = 0;

            results[guildId] = value;

        } while (result->NextRow());
    }

    return results;
}

uint32 GuildTaskMgr::GetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32* validIn /* = nullptr */)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        return 0;
    }

    uint32 value = 0;

    PlayerbotsPreparedStatement* stmt =
        PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_AND_TYPE);
    stmt->setUInt32(0, owner);
    stmt->setUInt32(1, guildId);
    stmt->setString(2, type);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        Field* fields = result->Fetch();
        value = fields[0].GetUInt32();
        uint32 lastChangeTime = fields[1].GetUInt32();
        uint32 secs = fields[2].GetUInt32();
        if ((time(nullptr) - lastChangeTime) >= secs)
            value = 0;

        if (validIn)
            *validIn = secs;
    }

    return value;
}

uint32 GuildTaskMgr::SetTaskValue(uint32 owner, uint32 guildId, std::string const type, uint32 value, uint32 validIn)
{
    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_GUILD_TASKS);
    stmt->setUInt32(0, owner);
    stmt->setUInt32(1, guildId);
    stmt->setString(2, type);
    PlayerbotsDatabase.Execute(stmt);

    if (value)
    {
        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_GUILD_TASKS);
        stmt->setUInt32(0, owner);
        stmt->setUInt32(1, guildId);
        stmt->setUInt32(2, (uint32)time(nullptr));
        stmt->setUInt32(3, validIn);
        stmt->setString(4, type);
        stmt->setUInt32(5, value);
        PlayerbotsDatabase.Execute(stmt);
    }

    return value;
}

bool GuildTaskMgr::HandleConsoleCommand(ChatHandler* /* handler */, char const* args)
{
    if (!sPlayerbotAIConfig->guildTaskEnabled)
    {
        TC_LOG_ERROR("playerbots", "Guild task system is currently disabled!");
        return false;
    }

    if (!args || !*args)
    {
        TC_LOG_ERROR("playerbots", "Usage: gtask stats/reset");
        return false;
    }

    std::string const cmd = args;

    if (cmd == "reset")
    {
        // AC: PLAYERBOTS_DEL_GUILD_TASKS_ALL (see the file header)
        PlayerbotsDatabase.Execute("DELETE FROM playerbots_guild_tasks");
        TC_LOG_INFO("playerbots", "Guild tasks were reset for all players");
        return true;
    }

    if (cmd == "stats")
    {
        TC_LOG_INFO("playerbots", "Usage: gtask stats <player name>");
        return true;
    }

    if (cmd.find("stats ") != std::string::npos)
    {
        std::string const charName = cmd.substr(cmd.find("stats ") + 6);
        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(charName);
        if (!guid)
        {
            TC_LOG_ERROR("playerbots", "Player %s not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();

        PlayerbotsPreparedStatement* stmt =
            PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_ORDERED);
        stmt->setUInt32(0, owner);
        stmt->setString(1, "activeTask");
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            do
            {
                Field* fields = result->Fetch();
                uint32 value = fields[0].GetUInt32();
                uint32 lastChangeTime = fields[1].GetUInt32();
                uint32 validIn = fields[2].GetUInt32();
                if ((time(nullptr) - lastChangeTime) >= validIn)
                    value = 0;

                uint32 guildId = fields[3].GetUInt32();

                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                std::ostringstream name;
                if (value == GUILD_TASK_TYPE_ITEM)
                {
                    name << "ItemTask";
                    uint32 itemId = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "itemTask");
                    uint32 itemCount = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "itemCount");

                    if (ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId))
                    {
                        name << " (" << proto->Name1 << " x" << itemCount << ",";

                        switch (proto->Quality)
                        {
                            case ITEM_QUALITY_UNCOMMON:
                                name << "green";
                                break;
                            case ITEM_QUALITY_NORMAL:
                                name << "white";
                                break;
                            case ITEM_QUALITY_RARE:
                                name << "blue";
                                break;
                            case ITEM_QUALITY_EPIC:
                                name << "epic";
                                break;
                            case ITEM_QUALITY_LEGENDARY:
                                name << "yellow";
                                break;
                        }

                        name << ")";
                    }
                }
                else if (value == GUILD_TASK_TYPE_KILL)
                {
                    name << "KillTask";
                    uint32 creatureId = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "killTask");

                    if (CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(creatureId))
                    {
                        name << " (" << proto->Name << ",";

                        switch (proto->rank)
                        {
                            case CREATURE_ELITE_RARE:
                                name << "rare";
                                break;
                            case CREATURE_ELITE_RAREELITE:
                                name << "rare elite";
                                break;
                        }

                        name << ")";
                    }
                }
                else
                    continue;

                uint32 advertValidIn = 0;
                uint32 advert = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "advertisement", &advertValidIn);
                if (advert && advertValidIn < validIn)
                    name << " advert in " << formatTime(advertValidIn);

                uint32 thanksValidIn = 0;
                uint32 thanks = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "thanks", &thanksValidIn);
                if (thanks && thanksValidIn < validIn)
                    name << " thanks in " << formatTime(thanksValidIn);

                uint32 rewardValidIn = 0;
                uint32 reward = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "reward", &rewardValidIn);
                if (reward && rewardValidIn < validIn)
                    name << " reward in " << formatTime(rewardValidIn);

                uint32 paymentValidIn = 0;
                uint32 payment = GuildTaskMgr::instance().GetTaskValue(owner, guildId, "payment", &paymentValidIn);
                if (payment && paymentValidIn < validIn)
                    name << " payment " << ChatHelper::formatMoney(payment) << " in " << formatTime(paymentValidIn);

                TC_LOG_INFO("playerbots", "%s: %s valid in %s [%s]", charName.c_str(), name.str().c_str(),
                            formatTime(validIn).c_str(), guild->GetName().c_str());

            } while (result->NextRow());
        }

        return true;
    }

    if (cmd == "cleanup")
    {
        GuildTaskMgr::instance().CleanupAdverts();
        GuildTaskMgr::instance().RemoveDuplicatedAdverts();
        return true;
    }

    if (cmd == "reward")
    {
        TC_LOG_INFO("playerbots", "Usage: gtask reward <player name>");
        return true;
    }

    if (cmd == "advert")
    {
        TC_LOG_INFO("playerbots", "Usage: gtask advert <player name>");
        return true;
    }

    bool reward = cmd.find("reward ") != std::string::npos;
    bool advert = cmd.find("advert ") != std::string::npos;
    if (reward || advert)
    {
        std::string charName;
        if (reward)
            charName = cmd.substr(cmd.find("reward ") + 7);

        if (advert)
            charName = cmd.substr(cmd.find("advert ") + 7);

        ObjectGuid guid = sCharacterCache->GetCharacterGuidByName(charName);
        if (!guid)
        {
            TC_LOG_ERROR("playerbots", "Player %s not found", charName.c_str());
            return false;
        }

        uint32 owner = guid.GetCounter();

        PlayerbotsPreparedStatement* stmt =
            PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_GUILD_TASKS_BY_OWNER_DISTINCT);
        stmt->setUInt32(0, owner);
        if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
        {
            CharacterDatabaseTransaction trans = CharacterDatabase.BeginTransaction();
            do
            {
                Field* fields = result->Fetch();
                uint32 guildId = fields[0].GetUInt32();
                Guild* guild = sGuildMgr->GetGuildById(guildId);
                if (!guild)
                    continue;

                if (reward)
                    GuildTaskMgr::instance().Reward(trans, owner, guildId);

                if (advert)
                    GuildTaskMgr::instance().SendAdvertisement(trans, owner, guildId);
            } while (result->NextRow());

            CharacterDatabase.CommitTransaction(trans);
            return true;
        }
    }

    return false;
}

bool GuildTaskMgr::CheckItemTask(uint32 itemId, uint32 obtained, Player* ownerPlayer, Player* bot, bool byMail)
{
    if (!bot || !ownerPlayer)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    uint32 owner = ownerPlayer->GetGUID().GetCounter();
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    TC_LOG_DEBUG("playerbots", "%s / %s: checking guild task", guild->GetName().c_str(), ownerPlayer->GetName().c_str());

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    if (itemTask != itemId)
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: item %u is not guild task item (%u)", guild->GetName().c_str(),
                     ownerPlayer->GetName().c_str(), itemId, itemTask);

        if (byMail)
            SendCompletionMessage(ownerPlayer, "made a mistake with");

        return false;
    }

    uint32 count = GetTaskValue(owner, guildId, "itemCount");
    if (!count)
    {
        return false;
    }

    uint32 rewardTime = urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime);
    if (byMail)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemId);
        if (!proto)
            return false;

        uint32 money = GetTaskValue(owner, guildId, "payment");
        SetTaskValue(owner, guildId, "payment", money + proto->BuyPrice * obtained, rewardTime + 300);
    }

    if (obtained >= count)
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: guild task complete", guild->GetName().c_str(), ownerPlayer->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1, rewardTime - 15);
        SetTaskValue(owner, guildId, "itemCount", 0, 0);
        SetTaskValue(owner, guildId, "thanks", 0, 0);
        SendCompletionMessage(ownerPlayer, "completed");
    }
    else
    {
        TC_LOG_DEBUG("playerbots", "%s / %s: guild task progress %u/%u", guild->GetName().c_str(),
                     ownerPlayer->GetName().c_str(), obtained, count);
        SetTaskValue(owner, guildId, "itemCount", count - obtained, sPlayerbotAIConfig->maxGuildTaskChangeTime);
        SetTaskValue(owner, guildId, "thanks", 1, rewardTime - 30);
        SendCompletionMessage(ownerPlayer, "made a progress with");
    }
    return true;
}

bool GuildTaskMgr::Reward(CharacterDatabaseTransaction& trans, uint32 owner, uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());
    if (!leader)
        return false;

    Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (!player)
        return false;

    uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
    uint32 killTask = GetTaskValue(owner, guildId, "killTask");
    if (!itemTask && !killTask)
        return false;

    std::ostringstream body;
    body << GetHelloText(owner);

    RandomItemType rewardType;
    uint32 itemId = 0;
    uint32 itemCount = 1;
    if (itemTask)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(itemTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name1
             << " you provided so kindly. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->Quality > ITEM_QUALITY_NORMAL ? RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_BLUE
                                                          : RANDOM_ITEM_GUILD_TASK_REWARD_EQUIP_GREEN;
        itemId = GetRandomItem(player->GetLevel() - 5, rewardType);
    }
    else if (killTask)
    {
        CreatureTemplate const* proto = sObjectMgr->GetCreatureTemplate(killTask);
        if (!proto)
            return false;

        body << "We wish to thank you for the " << proto->Name
             << " you've killed recently. We really appreciate this and may this small gift bring you our thanks!\n";
        body << "\n";
        body << "Many thanks,\n";
        body << guild->GetName() << "\n";
        body << leader->GetName() << "\n";
        rewardType = proto->rank == CREATURE_ELITE_RARE ? RANDOM_ITEM_GUILD_TASK_REWARD_TRADE
                                                        : RANDOM_ITEM_GUILD_TASK_REWARD_TRADE_RARE;
        itemId = GetRandomItem(player->GetLevel(), rewardType);
        if (itemId)
        {
            ItemTemplate const* itemProto = sObjectMgr->GetItemTemplate(itemId);
            if (itemProto)
            {
                if (itemProto->Quality == ITEM_QUALITY_NORMAL)
                    itemCount = itemProto->GetMaxStackSize();

                if (proto->rank != CREATURE_ELITE_RARE && itemProto->Quality > ITEM_QUALITY_NORMAL)
                    itemCount = urand(1, itemProto->GetMaxStackSize());
            }
        }
    }

    uint32 payment = GetTaskValue(owner, guildId, "payment");
    if (payment)
        SendThanks(trans, owner, guildId, payment);

    MailDraft draft("Thank You", body.str());

    if (itemId)
    {
        Item* item = Item::CreateItem(itemId, itemCount);
        if (item)
        {
            item->SaveToDB(trans);
            draft.AddItem(item);
        }
    }

    draft.SendMailTo(trans, MailReceiver(owner), MailSender(leader));

    player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(owner));
    if (player)
        SendCompletionMessage(player, "rewarded for");

    SetTaskValue(owner, guildId, "activeTask", 0, 0);
    SetTaskValue(owner, guildId, "payment", 0, 0);

    return true;
}

void GuildTaskMgr::CheckKillTask(Player* player, Unit* victim)
{
    if (!player)
        return;

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            if (Player* member = gr->GetSource())
                CheckKillTaskInternal(member, victim);
        }
    }
    else
    {
        CheckKillTaskInternal(player, victim);
    }
}

void GuildTaskMgr::SendCompletionMessage(Player* player, std::string const verb)
{
    std::ostringstream out;
    out << player->GetName() << " has " << verb << " a guild task";

    if (Group* group = player->GetGroup())
    {
        for (GroupReference* gr = group->GetFirstMember(); gr; gr = gr->next())
        {
            Player* member = gr->GetSource();
            if (member && member != player)
                ChatHandler(member->GetSession()).SendSysMessage(out.str().c_str());
        }
    }
    else
    {
        if (PlayerbotAI* botAI = GET_PLAYERBOT_AI(player))
            if (Player* master = botAI->GetMaster())
                ChatHandler(master->GetSession()).SendSysMessage(out.str().c_str());
    }

    std::ostringstream self;
    self << "You have " << verb << " a guild task";
    ChatHandler(player->GetSession()).SendSysMessage(self.str().c_str());
}

void GuildTaskMgr::CheckKillTaskInternal(Player* player, Unit* victim)
{
    ObjectGuid::LowType owner = player->GetGUID().GetCounter();
    if (!victim || !victim->IsCreature())
        return;

    Creature* creature = victim->ToCreature();
    if (!creature)
        return;

    std::map<uint32, uint32> tasks = GetTaskValues(owner, "killTask");
    for (std::map<uint32, uint32>::iterator i = tasks.begin(); i != tasks.end(); ++i)
    {
        uint32 guildId = i->first;
        uint32 value = i->second;
        Guild* guild = sGuildMgr->GetGuildById(guildId);

        if (value != creature->GetEntry())
            continue;

        TC_LOG_DEBUG("playerbots", "%s / %s: guild task complete", guild ? guild->GetName().c_str() : "",
                     player->GetName().c_str());
        SetTaskValue(owner, guildId, "reward", 1,
                     urand(sPlayerbotAIConfig->minGuildTaskRewardTime, sPlayerbotAIConfig->maxGuildTaskRewardTime));

        SendCompletionMessage(player, "completed");
    }
}

void GuildTaskMgr::CleanupAdverts()
{
    uint32 deliverTime = time(nullptr) - sPlayerbotAIConfig->minGuildTaskChangeTime;
    QueryResult result = CharacterDatabase.PQuery(
        "SELECT id, receiver FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= %u", deliverTime);
    if (!result)
        return;

    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt32())))
            player->RemoveMail(id);

        ++count;
    } while (result->NextRow());

    if (count > 0)
    {
        CharacterDatabase.PExecute("DELETE FROM mail WHERE subject LIKE 'Guild Task%%' AND deliver_time <= %u",
                                   deliverTime);
        TC_LOG_INFO("playerbots", "%u old gtask adverts removed", count);
    }
}

void GuildTaskMgr::RemoveDuplicatedAdverts()
{
    uint32 deliverTime = time(nullptr);
    QueryResult result = CharacterDatabase.PQuery(
        "SELECT m.id, m.receiver FROM (SELECT MAX(id) AS id, subject, receiver FROM mail WHERE subject LIKE 'Guild "
        "Task%%' "
        "AND deliver_time <= %u GROUP BY subject, receiver) q JOIN mail m ON m.subject = q.subject WHERE m.id <> q.id "
        "AND m.deliver_time <= %u",
        deliverTime, deliverTime);

    if (!result)
        return;

    std::vector<uint32> ids;
    uint32 count = 0;
    do
    {
        Field* fields = result->Fetch();
        uint32 id = fields[0].GetUInt32();

        if (Player* player = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt32())))
            player->RemoveMail(id);

        ++count;
        ids.push_back(id);
    } while (result->NextRow());

    if (count > 0)
    {
        std::vector<uint32> buffer;
        for (std::vector<uint32>::iterator i = ids.begin(); i != ids.end(); ++i)
        {
            buffer.push_back(*i);
            if (buffer.size() > 50)
            {
                DeleteMail(buffer);
                buffer.clear();
            }
        }

        DeleteMail(buffer);
        TC_LOG_INFO("playerbots", "%u duplicated gtask adverts removed", count);
    }
}

void GuildTaskMgr::DeleteMail(std::vector<uint32> buffer)
{
    if (buffer.empty())
        return;

    std::ostringstream sql;
    sql << "DELETE FROM mail WHERE id IN (";

    bool first = true;
    for (std::vector<uint32>::iterator j = buffer.begin(); j != buffer.end(); ++j)
    {
        if (first)
            first = false;
        else
            sql << ",";

        sql << *j;
    }

    sql << ")";

    CharacterDatabase.Execute(sql.str().c_str());
}

bool GuildTaskMgr::CheckTaskTransfer(std::string const text, Player* ownerPlayer, Player* bot)
{
    if (!bot)
        return false;

    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    ObjectGuid::LowType owner = ownerPlayer->GetGUID().GetCounter();
    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    if (text.empty())
        return false;

    TC_LOG_DEBUG("playerbots", "%s / %s: checking guild task transfer", guild->GetName().c_str(),
                 ownerPlayer->GetName().c_str());

    uint32 account = ownerPlayer->GetSession()->GetAccountId();

    if (QueryResult results = CharacterDatabase.PQuery("SELECT guid, name FROM characters WHERE account = %u", account))
    {
        do
        {
            Field* fields = results->Fetch();
            uint32 newOwner = fields[0].GetUInt32();
            std::string const name = fields[1].GetString();
            if (newOwner == bot->GetGUID().GetCounter())
                continue;

            std::wstring wnamepart;
            if (!Utf8toWStr(name, wnamepart))
                return false;

            wstrToLower(wnamepart);

            if (Utf8FitTo(text, wnamepart))
            {
                uint32 validIn;
                uint32 activeTask = GetTaskValue(owner, guildId, "activeTask", &validIn);
                uint32 itemTask = GetTaskValue(owner, guildId, "itemTask");
                uint32 killTask = GetTaskValue(owner, guildId, "killTask");

                if (itemTask || killTask)
                {
                    SetTaskValue(newOwner, guildId, "activeTask", activeTask, validIn);
                    SetTaskValue(newOwner, guildId, "advertisement", 1, 15);

                    if (itemTask)
                        SetTaskValue(newOwner, guildId, "itemTask", itemTask, validIn);

                    if (killTask)
                        SetTaskValue(newOwner, guildId, "killTask", killTask, validIn);

                    SetTaskValue(owner, guildId, "activeTask", 0, 0);
                    SetTaskValue(owner, guildId, "payment", 0, 0);

                    SendCompletionMessage(ownerPlayer, "transfered");
                }
            }
        } while (results->NextRow());
    }

    return true;
}
