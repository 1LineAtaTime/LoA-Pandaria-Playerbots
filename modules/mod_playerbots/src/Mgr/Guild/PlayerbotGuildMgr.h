/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Mgr/Guild/PlayerbotGuildMgr.h. Same public surface as AC; 5.4.8 adaptations are in
 * PlayerbotGuildMgr.cpp. Additions:
 *  - The guild cache is read from the map-update threads (PlayerbotAI::IsInRealGuild, from AllowActive) while the
 *    world thread fills it (bot login, randomize, the hourly validation), so it is guarded by a shared mutex.
 *  - PlayerbotGuildAccess: on 5.4.8 Guild::GetMember / GetRankRights / HasRankRight and the Member / RankInfo classes are
 *    private (Guilds/Guild.h), so the guild management actions read a member's rank and a rank's rights through these
 *    helpers (protected Guild data, no core change).
 */

#ifndef PLAYERBOTS_PLAYERBOTGUILDMGR_H
#define PLAYERBOTS_PLAYERBOTGUILDMGR_H

#include "Common.h"
#include "ObjectGuid.h"

#include <shared_mutex>
#include <string>
#include <unordered_map>
#include <vector>

class EmblemInfo;
class Guild;
class Player;

class PlayerbotGuildMgr
{
public:
    static PlayerbotGuildMgr& instance()
    {
        static PlayerbotGuildMgr instance;

        return instance;
    }

    void Init();
    std::string AssignToGuild(Player* player);
    void LoadGuildNames();
    void ValidateGuildCache();
    void ResetGuildCache();
    bool CreateGuild(Player* player, std::string guildName);
    void OnGuildUpdate(Guild* guild);
    bool SetGuildEmblem(uint32 guildId);
    void DeleteRandomBotGuilds();
    bool IsRealGuild(uint32 guildId);
    bool IsRealGuild(Player* bot);

private:
    PlayerbotGuildMgr() = default;
    ~PlayerbotGuildMgr() = default;

    PlayerbotGuildMgr(PlayerbotGuildMgr const&) = delete;
    PlayerbotGuildMgr& operator=(PlayerbotGuildMgr const&) = delete;

    PlayerbotGuildMgr(PlayerbotGuildMgr&&) = delete;
    PlayerbotGuildMgr& operator=(PlayerbotGuildMgr&&) = delete;

    std::unordered_map<std::string, bool> _guildNames;

    struct GuildCache
    {
        std::string name;
        uint8 status = 0;
        uint32 maxMembers = 0;
        uint32 memberCount = 0;
        uint8 faction = 0;
        bool hasRealPlayer = false;
    };
    std::unordered_map<uint32, GuildCache> _guildCache;
    std::vector<std::string> _shuffled_guild_keys;

    std::shared_mutex _cacheLock;
};

void PlayerBotsGuildValidationScript();

// 5.4.8: read access to the private guild member / rank data the AC guild actions use (see the file header).
namespace PlayerbotGuildAccess
{
// Rights of a rank id (Guild::GetRankRights); 0 when the rank does not exist.
uint32 GetRankRights(Guild const* guild, uint32 rankId);
// The rank id of a guild member (Guild::GetMember(guid)->GetRankId()); false when guid is not a member.
bool GetMemberRankId(Guild const* guild, ObjectGuid guid, uint8& rankId);
// Guild::HasRankRight(player, right).
bool HasRankRight(Guild const* guild, Player const* player, uint32 right);
// Sets the guild's emblem in memory and in the guild table (EmblemInfo::SaveToDB), without Guild::HandleSetEmblem's
// leader / money checks.
void SetEmblem(Guild* guild, EmblemInfo const& emblemInfo);
}  // namespace PlayerbotGuildAccess

#endif
