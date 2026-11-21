/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Mgr/Guild/PlayerbotGuildMgr.cpp (random-bot guild cache, guild assignment,
 * random-bot guild deletion, the hourly cache validation world script). 5.4.8 / fork adaptations:
 *  - LOG_* {} -> TC_LOG_* %s; sPlayerbotAIConfig. -> ->; Field::Get<T>() -> GetUInt32()/GetString(); TeamId from the
 *    leader's race through Player::TeamForRace (5.4.8 has no Player::TeamIdForRace).
 *  - SetGuildEmblem: AC writes the random tabard into the guild table with a raw UPDATE, so the loaded guild keeps a
 *    blank emblem until the next restart. Here the same random values go through EmblemInfo (filled with ReadPacket, it
 *    has no value constructor on 5.4.8) into the loaded guild and EmblemInfo::SaveToDB (the same UPDATE).
 *  - CreateGuild also marks the new guild's name as used. AC marks it only on the next OnGuildUpdate, so until then
 *    AssignToGuild hands the same name to a bot of the other faction, which then joins the new guild (InitGuild finds
 *    it by name).
 *  - ValidateGuildCache skips a guild whose leader is not in the CharacterCache (AC dereferences the null entry).
 *  - DeleteRandomBotGuilds deletes the disbanded Guild objects (Guild::Disband only removes them from GuildMgr; the
 *    core's own disband paths delete them afterwards, Guild::HandleDisband).
 *  - The cache is guarded by a shared mutex: IsRealGuild runs on the map-update threads (PlayerbotAI::IsInRealGuild),
 *    the rest on the world thread.
 */

#include "PlayerbotGuildMgr.h"

#include "CharacterCache.h"
#include "DatabaseEnv.h"
#include "Guild.h"
#include "GuildMgr.h"
#include "Log.h"
#include "Player.h"
#include "PlayerbotAIConfig.h"
#include "Random.h"
#include "ScriptMgr.h"
#include "WorldPacket.h"

#include <algorithm>
#include <mutex>
#include <random>

namespace
{
// AC Player::TeamIdForRace
uint8 TeamIdForRace(uint8 race)
{
    switch (Player::TeamForRace(race))
    {
        case ALLIANCE:
            return TEAM_ALLIANCE;
        case HORDE:
            return TEAM_HORDE;
        default:
            return TEAM_NEUTRAL;
    }
}

// Read access to protected Guild data (see PlayerbotGuildAccess in the header). Forming a pointer to a protected member
// through a derived class is allowed; the class is never instantiated.
struct GuildPeek : public Guild
{
    static auto RanksMember() -> decltype(&GuildPeek::m_ranks) { return &GuildPeek::m_ranks; }
    static auto MembersMember() -> decltype(&GuildPeek::m_members) { return &GuildPeek::m_members; }
    static auto EmblemMember() -> decltype(&GuildPeek::m_emblemInfo) { return &GuildPeek::m_emblemInfo; }
};
}  // namespace

void PlayerbotGuildMgr::Init()
{
    {
        std::unique_lock<std::shared_mutex> lock(_cacheLock);
        _guildCache.clear();
    }
    if (sPlayerbotAIConfig->deleteRandomBotGuilds)
        DeleteRandomBotGuilds();

    LoadGuildNames();
    ValidateGuildCache();
}

bool PlayerbotGuildMgr::CreateGuild(Player* player, std::string guildName)
{
    Guild* guild = new Guild();
    if (!guild->Create(player, guildName))
    {
        TC_LOG_ERROR("playerbots", "Error creating guild [ %s ] with leader [ %s ]", guildName.c_str(),
                     player->GetName().c_str());
        delete guild;
        return false;
    }
    sGuildMgr->AddGuild(guild);

    TC_LOG_DEBUG("playerbots", "Guild created: id=%u name='%s'", uint32(guild->GetId()), guildName.c_str());
    SetGuildEmblem(guild->GetId());

    GuildCache entry;
    entry.name = guildName;
    entry.memberCount = 1;
    entry.status = 1;
    entry.maxMembers = sPlayerbotAIConfig->randomBotGuildSizeMax;
    entry.faction = player->GetTeamId();

    std::unique_lock<std::shared_mutex> lock(_cacheLock);
    _guildCache[guild->GetId()] = entry;
    // See the file header: the name is taken now.
    auto nameItr = _guildNames.find(guildName);
    if (nameItr != _guildNames.end())
        nameItr->second = false;
    return true;
}

bool PlayerbotGuildMgr::SetGuildEmblem(uint32 guildId)
{
    Guild* guild = sGuildMgr->GetGuildById(guildId);
    if (!guild)
        return false;

    // create random emblem
    uint32 st, cl, br, bc, bg;
    bg = urand(0, 51);
    bc = urand(0, 17);
    cl = urand(0, 17);
    br = urand(0, 7);
    st = urand(0, 180);

    TC_LOG_DEBUG("playerbots",
                 "[TABARD] new guild id=%u random -> style=%u, color=%u, borderStyle=%u, borderColor=%u, bgColor=%u",
                 uint32(guild->GetId()), st, cl, br, bc, bg);

    // 5.4.8: EmblemInfo::ReadPacket order is borderStyle, backgroundColor, borderColor, color, style (Guild.cpp).
    WorldPacket emblemData;
    emblemData << br << bg << bc << cl << st;
    EmblemInfo emblemInfo;
    emblemInfo.ReadPacket(emblemData);

    // populate guild table with a random tabard design
    PlayerbotGuildAccess::SetEmblem(guild, emblemInfo);
    TC_LOG_DEBUG("playerbots", "[TABARD] UPDATE done for guild id=%u", uint32(guild->GetId()));
    return true;
}

std::string PlayerbotGuildMgr::AssignToGuild(Player* player)
{
    if (!player)
        return "";

    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    uint8_t playerFaction = player->GetTeamId();
    std::vector<GuildCache*> partiallyfilledguilds;
    partiallyfilledguilds.reserve(_guildCache.size());

    for (auto& keyValue : _guildCache)
    {
        GuildCache& cached = keyValue.second;
        if (!cached.hasRealPlayer && cached.status == 1 && cached.faction == playerFaction)
            partiallyfilledguilds.push_back(&cached);
    }

    if (!partiallyfilledguilds.empty())
    {
        size_t idx = static_cast<size_t>(urand(0, static_cast<int>(partiallyfilledguilds.size()) - 1));
        return (partiallyfilledguilds[idx]->name);
    }

    size_t count = std::count_if(_guildCache.begin(), _guildCache.end(),
                                 [](std::pair<const uint32, GuildCache> const& pair)
                                 {
                                     return !pair.second.hasRealPlayer;
                                 });

    if (count < sPlayerbotAIConfig->randomBotGuildCount)
    {
        for (auto& key : _shuffled_guild_keys)
        {
            if (_guildNames[key])
            {
                TC_LOG_INFO("playerbots", "Assigning player [%s] to guild [%s]", player->GetName().c_str(), key.c_str());
                return key;
            }
        }
        TC_LOG_ERROR("playerbots", "No available guild names left.");
    }
    return "";
}

void PlayerbotGuildMgr::OnGuildUpdate(Guild* guild)
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    auto it = _guildCache.find(guild->GetId());
    if (it == _guildCache.end())
        return;

    GuildCache& entry = it->second;
    entry.memberCount = guild->GetMembersCount();
    if (entry.memberCount < entry.maxMembers)
        entry.status = 1;
    else if (entry.memberCount >= entry.maxMembers)
        entry.status = 2;  // Full
    std::string guildName = guild->GetName();
    for (auto& it : _guildNames)
    {
        if (it.first == guildName)
        {
            it.second = false;
            break;
        }
    }
}

void PlayerbotGuildMgr::ResetGuildCache()
{
    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    _guildCache.clear();

    for (auto& nameEntry : _guildNames)
        nameEntry.second = true;
}

void PlayerbotGuildMgr::LoadGuildNames()
{
    TC_LOG_INFO("playerbots", "Loading guild names from playerbots_guild_names...");

    QueryResult result = CharacterDatabase.Query("SELECT name_id, name FROM playerbots_guild_names");

    if (!result)
    {
        TC_LOG_ERROR("playerbots", "No entries found in playerbots_guild_names. List is empty.");
        return;
    }

    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    do
    {
        Field* fields = result->Fetch();
        _guildNames[fields[1].GetString()] = true;
    } while (result->NextRow());

    for (auto& pair : _guildNames)
        _shuffled_guild_keys.push_back(pair.first);

    std::random_device rd;
    std::mt19937 g(rd());

    std::shuffle(_shuffled_guild_keys.begin(), _shuffled_guild_keys.end(), g);
    TC_LOG_INFO("playerbots", "Loaded %u guild entries from playerbots_guild_names table.", uint32(_guildNames.size()));
}

void PlayerbotGuildMgr::ValidateGuildCache()
{
    QueryResult result = CharacterDatabase.Query("SELECT guildid, name FROM guild");
    if (!result)
    {
        TC_LOG_ERROR("playerbots", "No guilds found in database, resetting guild cache");
        ResetGuildCache();
        return;
    }

    std::unordered_map<uint32, std::string> dbGuilds;
    do
    {
        Field* fields = result->Fetch();
        uint32 guildId = fields[0].GetUInt32();
        std::string guildName = fields[1].GetString();
        dbGuilds[guildId] = guildName;
    } while (result->NextRow());

    std::unique_lock<std::shared_mutex> lock(_cacheLock);

    for (auto it = dbGuilds.begin(); it != dbGuilds.end(); it++)
    {
        uint32 guildId = it->first;
        GuildCache cache;
        cache.name = it->second;
        cache.maxMembers = sPlayerbotAIConfig->randomBotGuildSizeMax;

        Guild* guild = sGuildMgr->GetGuildById(guildId);
        if (!guild)
            continue;

        cache.memberCount = guild->GetMembersCount();
        ObjectGuid leaderGuid = guild->GetLeaderGUID();
        CharacterCacheEntry const* leaderEntry = sCharacterCache->GetCharacterCacheByGuid(leaderGuid);
        if (!leaderEntry)  // see the file header
            continue;
        uint32 leaderAccount = leaderEntry->AccountId;
        cache.hasRealPlayer = !(sPlayerbotAIConfig->IsInRandomAccountList(leaderAccount));
        cache.faction = TeamIdForRace(leaderEntry->Race);
        if (cache.memberCount == 0)
            cache.status = 0;  // empty
        else if (cache.memberCount < cache.maxMembers)
            cache.status = 1;  // partially filled
        else
            cache.status = 2;  // full

        _guildCache.insert_or_assign(guildId, cache);
        for (auto& it : _guildNames)
        {
            if (it.first == cache.name)
            {
                it.second = false;
                break;
            }
        }
    }
}

void PlayerbotGuildMgr::DeleteRandomBotGuilds()
{
    TC_LOG_INFO("playerbots", "Deleting randombot guilds...");
    std::vector<uint32> guildsToDisband;

    if (QueryResult result = CharacterDatabase.Query("SELECT guildid, leaderguid FROM guild"))
    {
        do
        {
            Field* fields = result->Fetch();
            uint32 guildId = fields[0].GetUInt32();
            ObjectGuid leader = ObjectGuid::Create<HighGuid::Player>(fields[1].GetUInt32());

            // The leader's account is checked instead of the 'add' event or anything else that depends on the
            // bot being online.
            if (sPlayerbotAIConfig->IsInRandomAccountList(sCharacterCache->GetCharacterAccountIdByGuid(leader)))
                guildsToDisband.push_back(guildId);
        } while (result->NextRow());
    }

    for (uint32 guildId : guildsToDisband)
    {
        if (Guild* guild = sGuildMgr->GetGuildById(guildId))
        {
            guild->Disband();
            delete guild;  // see the file header
        }
    }
    TC_LOG_INFO("playerbots", "Randombot guilds deleted");
}

bool PlayerbotGuildMgr::IsRealGuild(Player* bot)
{
    if (!bot)
        return false;
    uint32 guildId = bot->GetGuildId();
    if (!guildId)
        return false;

    return IsRealGuild(guildId);
}

bool PlayerbotGuildMgr::IsRealGuild(uint32 guildId)
{
    if (!guildId)
        return false;

    std::shared_lock<std::shared_mutex> lock(_cacheLock);

    auto it = _guildCache.find(guildId);
    if (it == _guildCache.end())
        return false;

    // A "real guild" is one whose leader's account is not in the bot accounts list.
    // Guild membership by real players does not affect this, only the leader's account type does.
    return it->second.hasRealPlayer;
}

class BotGuildCacheWorldScript : public WorldScript
{
public:
    BotGuildCacheWorldScript() : WorldScript("BotGuildCacheWorldScript"), _validateTimer(0) {}

    void OnUpdate(uint32 diff) override
    {
        _validateTimer += diff;

        if (_validateTimer >= _validateInterval)  // Validate every hour
        {
            _validateTimer = 0;
            PlayerbotGuildMgr::instance().ValidateGuildCache();
            TC_LOG_INFO("playerbots", "Scheduled guild cache validation");
        }
    }

private:
    uint32 _validateInterval = HOUR * IN_MILLISECONDS;
    uint32 _validateTimer;
};

void PlayerBotsGuildValidationScript()
{
    new BotGuildCacheWorldScript();
}

namespace PlayerbotGuildAccess
{
uint32 GetRankRights(Guild const* guild, uint32 rankId)
{
    if (!guild)
        return 0;

    for (auto const& rank : guild->*GuildPeek::RanksMember())
        if (rank.GetId() == rankId)
            return rank.GetRights();

    return 0;
}

bool GetMemberRankId(Guild const* guild, ObjectGuid guid, uint8& rankId)
{
    if (!guild)
        return false;

    auto const& members = guild->*GuildPeek::MembersMember();
    auto itr = members.find(guid.GetCounter());
    if (itr == members.end() || !itr->second)
        return false;

    rankId = itr->second->GetRankId();
    return true;
}

bool HasRankRight(Guild const* guild, Player const* player, uint32 right)
{
    if (!guild || !player)
        return false;

    uint8 rankId = 0;
    if (!GetMemberRankId(guild, player->GetGUID(), rankId))
        return false;

    return (GetRankRights(guild, rankId) & right) != GR_RIGHT_EMPTY;
}

void SetEmblem(Guild* guild, EmblemInfo const& emblemInfo)
{
    if (!guild)
        return;

    guild->*GuildPeek::EmblemMember() = emblemInfo;
    emblemInfo.SaveToDB(guild->GetId());
}
}  // namespace PlayerbotGuildAccess
