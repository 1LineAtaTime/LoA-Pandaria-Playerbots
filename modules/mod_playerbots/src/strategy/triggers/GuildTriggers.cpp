/*
 * This file is part of the mod-playerbots module for AzerothCore. See AUTHORS file for Copyright
 * information; released under GNU GPL v2 license, redistribute/modify under version 2 of the License,
 * or (at your option) any later version.
 *
 * Ported 1:1 from AC mod-playerbots Ai/Base/Trigger/GuildTriggers.cpp. Fork adaptations:
 *  - chat->FormatQItem(id) -> std::to_string(id) (the fork's numeric "item count" qualifier, as GuildCreateActions.cpp).
 *  - AC's free IsSelfBot(p) and PlayerbotAI::IsAltBot() (AC Bot/PlayerbotAI.cpp:4640, :4699) are file-local; the fork
 *    has no selfbots, so IsSelfBot is "the AI of p is its own master" (PlayerbotAI::IsRealPlayer).
 *  - Guild::GetMemberSize -> GetMembersCount.
 */

#include "GuildTriggers.h"

#include "Guild.h"
#include "GuildMgr.h"
#include "ObjectAccessor.h"
#include "Playerbots.h"
#include "World.h"

namespace
{
bool IsSelfBot(Player* player)
{
    PlayerbotAI* playerAI = player ? GET_PLAYERBOT_AI(player) : nullptr;
    return playerAI && playerAI->IsRealPlayer();
}

// AC PlayerbotAI::IsAltBot
bool IsAltBot(PlayerbotAI* botAI, Player* bot)
{
    return botAI->HasGameClientMaster() && !sRandomPlayerbotMgr->IsRandomBot(bot) && !IsSelfBot(bot);
}
}  // namespace

bool PetitionTurnInTrigger::IsActive()
{
    return !bot->GetGuildId() && AI_VALUE2(uint32, "item count", std::to_string(5863)) &&
           AI_VALUE(uint8, "petition signs") >= sWorld->getIntConfig(CONFIG_MIN_PETITION_SIGNS);
}

bool BuyTabardTrigger::IsActive()
{
    return bot->GetGuildId() && !AI_VALUE2(uint32, "item count", std::to_string(5976));
}

bool LeaveLargeGuildTrigger::IsActive()
{
    if (!bot->GetGuildId())
        return false;

    if (IsSelfBot(bot))
        return false;

    if (IsAltBot(botAI, bot))
        return false;

    if (botAI->IsInRealGuild())
        return false;

    GuilderType type = botAI->GetGuilderType();

    Guild* guild = sGuildMgr->GetGuildById(bot->GetGuildId());
    if (!guild)
        return false;

    Player* leader = ObjectAccessor::FindPlayer(guild->GetLeaderGUID());

    // Only leave the guild if the leader is a bot (but not a selfbot).
    PlayerbotAI* leaderBotAI = leader ? GET_PLAYERBOT_AI(leader) : nullptr;
    if (!leaderBotAI || IsSelfBot(leader))
        return false;

    if (type == GuilderType::SOLO && guild->GetLeaderGUID() != bot->GetGUID())
        return true;

    uint32 members = guild->GetMembersCount();
    uint32 maxMembers = uint8(type);

    return members > maxMembers;
}
