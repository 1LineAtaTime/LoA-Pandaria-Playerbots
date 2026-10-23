/*
 * Ported from AC mod-playerbots Util/BroadcastHelper.h (#322). Same public surface as AC.
 * uint8_t/uint32_t in the AC header are kept as-is; see BroadcastHelper.cpp for the 5.4.8 adaptations.
 */

#ifndef PLAYERBOTS_BROADCASTHELPER_H
#define PLAYERBOTS_BROADCASTHELPER_H

#include "Define.h"
#include <cstdint>
#include <list>
#include <string>
#include <utility>
#include <vector>

class PlayerbotAI;
class Player;
class Quest;
class Creature;
class Group;

struct ItemTemplate;

// AC Util/BroadcastHelper.h:23-163
class BroadcastHelper
{
public:
    BroadcastHelper();

public:
    enum ToChannel
    {
        TO_GUILD = 1,
        TO_WORLD = 2,
        TO_GENERAL = 3,
        TO_TRADE = 4,
        TO_LOOKING_FOR_GROUP = 5,
        TO_LOCAL_DEFENSE = 6,
        TO_WORLD_DEFENSE = 7,
        TO_GUILD_RECRUITMENT = 8
    };

    static uint8_t GetLocale();
    static bool BroadcastTest(
        PlayerbotAI* ai,
        Player* bot
    );
    static bool BroadcastToChannelWithGlobalChance(
        PlayerbotAI* ai,
        std::string message,
        std::list<std::pair<ToChannel, uint32_t>> toChannels
    );
    static bool BroadcastLootingItem(
        PlayerbotAI* ai,
        Player* bot,
        ItemTemplate const* proto
    );
    static bool BroadcastQuestAccepted(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestUpdateAddKill(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest,
        uint32_t availableCount,
        uint32_t requiredCount,
        std::string obectiveName
    );
    static bool BroadcastQuestUpdateAddItem(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest,
        uint32_t availableCount,
        uint32_t requiredCount,
        ItemTemplate const* proto
    );
    static bool BroadcastQuestUpdateFailedTimer(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestUpdateComplete(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastQuestTurnedIn(
        PlayerbotAI* ai,
        Player* bot,
        Quest const* quest
    );
    static bool BroadcastKill(
        PlayerbotAI* ai,
        Player* bot,
        Creature* creature
    );
    static bool BroadcastLevelup(
        PlayerbotAI* ai,
        Player* bot
    );
    static bool BroadcastGuildMemberPromotion(
        PlayerbotAI* ai,
        Player* bot,
        Player* player
    );
    static bool BroadcastGuildMemberDemotion(
        PlayerbotAI* ai,
        Player* bot,
        Player* player
    );
    static bool BroadcastGuildGroupOrRaidInvite(
        PlayerbotAI* ai,
        Player* bot,
        Player* player,
        Group* group
    );
    static bool BroadcastSuggestInstance(
        PlayerbotAI* ai,
        std::vector<std::string>& allowedInstances,
        Player* bot
    );
    static bool BroadcastSuggestQuest(
        PlayerbotAI* ai,
        std::vector<uint32>& quests,
        Player* bot
    );
    static bool BroadcastSuggestGrindMaterials(
        PlayerbotAI* ai,
        std::string item,
        Player* bot
    );
    static bool BroadcastSuggestGrindReputation(
        PlayerbotAI* ai,
        std::vector<std::string> levels,
        std::vector<std::string> allowedFactions,
        Player* bot
    );
    static bool BroadcastSuggestSell(
        PlayerbotAI* ai,
        ItemTemplate const* proto,
        uint32_t count,
        uint32_t price,
        Player* bot
    );
    static bool BroadcastSuggestSomething(
        PlayerbotAI* ai,
        Player* bot
    );
    static bool BroadcastSuggestSomethingToxic(
        PlayerbotAI* ai,
        Player* bot
    );
    static bool BroadcastSuggestToxicLinks(
        PlayerbotAI* ai,
        Player* bot
    );
    static bool BroadcastSuggestThunderfury(
        PlayerbotAI* ai,
        Player* bot
    );
};

#endif
