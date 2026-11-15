/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

// Ported from AC mod-playerbots Ai/Base/Actions/SayAction.cpp; see SayAction.h for the 5.4.8 adaptations.

#include "SayAction.h"

#include "AiFactory.h"
#include "BroadcastHelper.h"
#include "ChannelMgr.h"
#include "ChatHelper.h"
#include "DBCStores.h"
#include "Event.h"
#include "Group.h"
#include "Helper.h"
#include "ObjectAccessor.h"
#include "ObjectMgr.h"
#include "PlayerbotAIConfig.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"
#include "QuestDef.h"
#include "World.h"

#include <algorithm>
#include <atomic>
#include <charconv>
#include <list>
#include <mutex>
#include <regex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
    // Per-bot chat-reply state (see SayAction.h): AC PlayerbotAI::chatReplies and the "last said"::"chat" value.
    struct BotChatReplyState
    {
        std::list<ChatQueuedReply> replies;
        time_t lastChat = 0;
        bool lastChatSet = false;
    };

    std::mutex chatReplyLock;
    std::unordered_map<uint64, BotChatReplyState> chatReplyStates;
    std::atomic<uint32> queuedChatReplies{0};

    // A reply still queued this long after it fell due belongs to a bot that logged out meanwhile; AC's queue dies
    // with the bot's PlayerbotAI, so such a reply is dropped rather than sent hours later.
    constexpr time_t STALE_CHAT_REPLY_SECONDS = 120;

    Language SayLanguage(Player* bot) { return bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH; }

    // AC Bot/Cmd/ChatHelper.cpp:696-723
    std::set<uint32> extractGeneric(std::string_view text, std::string_view prefix)
    {
        std::set<uint32> ids;
        std::string_view text_view = text;

        size_t pos = 0;
        while ((pos = text_view.find(prefix, pos)) != std::string::npos)
        {
            // skip "Hquest:/Hitem:"
            pos += prefix.size();

            // extract everything after "Hquest:/Hitem:"
            size_t end_pos = text_view.find_first_not_of("0123456789", pos);
            std::string_view number_str = text_view.substr(pos, end_pos - pos);

            uint32 number = 0;

            auto [ptr, ec] = std::from_chars(number_str.data(), number_str.data() + number_str.size(), number);

            if (ec == std::errc())
            {
                ids.insert(number);
            }
            pos = end_pos;
        }

        return ids;
    }

    // AC Bot/PlayerbotAI.cpp:6797-6812 (GetAllCurrentQuestIds)
    std::set<uint32> GetAllCurrentQuestIds(Player* bot)
    {
        std::set<uint32> result;

        for (uint16 slot = 0; slot < MAX_QUEST_LOG_SIZE; ++slot)
        {
            uint32 questId = bot->GetQuestSlotQuestId(slot);
            if (!questId)
                continue;

            result.insert(questId);
        }

        return result;
    }

    // AC Bot/PlayerbotAI.cpp:6001-6045 / 6047-6090 (GetInventoryItemsCountWithId / HasItemInInventory). Same slots:
    // bags, backpack (5.4.8 has no keyring).
    uint32 GetInventoryItemsCountWithId(Player* bot, uint32 itemId)
    {
        uint32 count = 0;

        for (Item* pItem : BroadcastHelper::BotInventoryAndEquippedItems(bot))
        {
            if (pItem->GetSlot() < EQUIPMENT_SLOT_END && pItem->GetBagSlot() == INVENTORY_SLOT_BAG_0)
                continue;  // equipped, AC counts only bags and backpack

            if (pItem->GetEntry() == itemId)
                count += pItem->GetCount();
        }

        return count;
    }

    bool HasItemInInventory(Player* bot, uint32 itemId) { return GetInventoryItemsCountWithId(bot, itemId) > 0; }
}

static const std::unordered_set<std::string> noReplyMsgs = {
    "join",
    "leave",
    "follow",
    "attack",
    "pull",
    "flee",
    "reset",
    "reset ai",
    "all ?",
    "talents",
    "talents list",
    "talents auto",
    "talk",
    "stay",
    "stats",
    "who",
    "items",
    "leave",
    "join",
    "repair",
    "summon",
    "nc ?",
    "co ?",
    "de ?",
    "dead ?",
    "follow",
    "los",
    "guard",
    "do accept invitation",
    "stats",
    "react ?",
    "reset strats",
    "home",
};
static const std::unordered_set<std::string> noReplyMsgParts = {
    "+", "-", "@", "follow target", "focus heal", "cast ", "accept [", "e [", "destroy [", "go zone"};
static const std::unordered_set<std::string> noReplyMsgStarts = {"e ", "accept ", "cast ", "destroy "};

SayAction::SayAction(PlayerbotAI* botAI) : Action(botAI, "say"), Qualified() {}

// AC SayAction.cpp:56-142
bool SayAction::Execute(Event /*event*/)
{
    std::string text = "";
    std::map<std::string, std::string> placeholders;
    Unit* target = AI_VALUE(Unit*, "tank target");
    if (!target)
        target = AI_VALUE(Unit*, "current target");

    // set replace strings
    if (target)
        placeholders["<target>"] = target->GetName();
    placeholders["<randomfaction>"] = IsAlliance(bot->GetRace()) ? "Alliance" : "Horde";
    if (qualifier == "low ammo" || qualifier == "no ammo")
    {
        if (Item* const pItem = bot->GetItemByPos(INVENTORY_SLOT_BAG_0, EQUIPMENT_SLOT_RANGED))
        {
            switch (pItem->GetTemplate()->SubClass)
            {
                case ITEM_SUBCLASS_WEAPON_GUN:
                    placeholders["<ammo>"] = "bullets";
                    break;
                case ITEM_SUBCLASS_WEAPON_BOW:
                case ITEM_SUBCLASS_WEAPON_CROSSBOW:
                    placeholders["<ammo>"] = "arrows";
                    break;
            }
        }
    }

    // 5.4.8: AreaTableEntry::area_name is a single string (no locale array).
    if (bot->GetMap())
    {
        if (AreaTableEntry const* zone = sAreaTableStore.LookupEntry(bot->GetZoneId()))
            placeholders["<subzone>"] = zone->area_name ? zone->area_name : "";
    }

    // set delay before next say
    time_t nextTime = time(nullptr) + urand(1, 30);
    botAI->GetAiObjectContext()->GetValue<time_t>("last said", qualifier)->Set(nextTime);

    Group* group = bot->GetGroup();
    if (group)
    {
        std::vector<Player*> members;
        for (GroupReference* ref = group->GetFirstMember(); ref; ref = ref->next())
        {
            Player* member = ref->GetSource();
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                members.push_back(member);
        }

        uint32 count = members.size();
        if (count > 1)
        {
            for (uint32 i = 0; i < count * 5; i++)
            {
                int i1 = urand(0, count - 1);
                int i2 = urand(0, count - 1);

                Player* item = members[i1];
                members[i1] = members[i2];
                members[i2] = item;
            }
        }

        int index = 0;
        for (auto& member : members)
        {
            PlayerbotAI* memberAi = GET_PLAYERBOT_AI(member);
            if (memberAi)
                memberAi->GetAiObjectContext()
                    ->GetValue<time_t>("last said", qualifier)
                    ->Set(nextTime + (20 * ++index) + urand(1, 15));
        }
    }

    // load text based on chance
    if (!PlayerbotTextMgr::instance().GetBotText(qualifier, text, placeholders))
        return false;

    if (text.find("/y ") == 0)
        bot->Yell(text.substr(3), SayLanguage(bot));
    else
        bot->Say(text, SayLanguage(bot));

    return true;
}

// AC SayAction.cpp:144-154
bool SayAction::isUseful()
{
    if (!botAI->AllowActivity())
        return false;

    if (botAI->HasStrategy("silent", BotState::BOT_STATE_NON_COMBAT))
        return false;

    time_t lastSaid = AI_VALUE2(time_t, "last said", qualifier);
    return (time(nullptr) - lastSaid) > 30;
}

// AC SayAction.cpp:156-220
void ChatReplyAction::ChatReplyDo(Player* bot, uint32& type, uint32& guid1, std::string& msg, std::string& chanName, std::string& name)
{
    std::string respondsText = "";

    // if we're just commanding bots around, don't respond...
    // first one is for exact word matches
    if (noReplyMsgs.find(msg) != noReplyMsgs.end())
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore exact blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    // second one is for partial matches like + or - where we change strats
    if (std::any_of(noReplyMsgParts.begin(), noReplyMsgParts.end(),
                    [&msg](std::string const& part) { return msg.find(part) != std::string::npos; }))
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore partial blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    if (std::any_of(noReplyMsgStarts.begin(), noReplyMsgStarts.end(),
                    [&msg](std::string const& start)
                    {
                        return msg.find(start) == 0;  // Check if the start matches the beginning of msg
                    }))
    {
        /*std::ostringstream out;
        out << "DEBUG ChatReplyDo decided to ignore start blocklist match" << msg;
        bot->Say(out.str(), LANG_UNIVERSAL);*/
        return;
    }

    ChatChannelSource chatChannelSource = GetChatChannelSource(bot, type, chanName);
    if ((msg.starts_with("LFG") || msg.starts_with("LFM")) && HandleLFGQuestsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    if (msg.starts_with("WTB") && HandleWTBItemsReply(bot, chatChannelSource, msg, name))
    {
        return;
    }

    //toxic links
    if (msg.starts_with(sPlayerbotAIConfig->toxicLinksPrefix)
        && (ExtractAllItemIds(msg).size() > 0 || ExtractAllQuestIds(msg).size() > 0))
    {
        HandleToxicLinksReply(bot, chatChannelSource);
        return;
    }

    //thunderfury
    if (ExtractAllItemIds(msg).count(19019))
    {
        HandleThunderfuryReply(bot, chatChannelSource);
        return;
    }

    auto messageRepy = GenerateReplyMessage(bot, msg, guid1, name);
    SendGeneralResponse(bot, chatChannelSource, messageRepy, name);
}

// AC SayAction.cpp:222-248
bool ChatReplyAction::HandleThunderfuryReply(Player* bot, ChatChannelSource chatChannelSource)
{
    std::map<std::string, std::string> placeholders;
    const auto thunderfury = sObjectMgr->GetItemTemplate(19019);
    if (!thunderfury)
        return false;
    placeholders["%thunderfury_link"] = ChatHelper::FormatItem(thunderfury);

    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("thunderfury_spam", placeholders);

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            BroadcastHelper::BotSayToWorld(bot, responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::GENERAL);
            break;
        }
        default:
            break;
    }

    SetLastChatTime(bot, time(0) + urand(5, 25));
    return true;
}

// AC SayAction.cpp:250-315
bool ChatReplyAction::HandleToxicLinksReply(Player* bot, ChatChannelSource chatChannelSource)
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
    std::vector<Item*> botItems = BroadcastHelper::BotInventoryAndEquippedItems(bot);

    std::map<std::string, std::string> placeholders;
    placeholders["%random_inventory_item_link"] = botItems.size() > 0 ? ChatHelper::FormatItem(botItems[rand() % botItems.size()]->GetTemplate()) : PlayerbotTextMgr::instance().GetBotText("string_empty_link");
    placeholders["%prefix"] = sPlayerbotAIConfig->toxicLinksPrefix;

    Quest const* quest = incompleteQuests.size() > 0
                             ? sObjectMgr->GetQuestTemplate(incompleteQuests[rand() % incompleteQuests.size()])
                             : nullptr;
    if (quest)
    {
        placeholders["%random_taken_quest_or_item_link"] = ChatHelper::FormatQuest(quest);
    }
    else
    {
        placeholders["%random_taken_quest_or_item_link"] = placeholders["%random_inventory_item_link"];
    }

    // %my_role, %area_name, %zone_name, %my_class, %my_race, %my_level
    BroadcastHelper::BotChatPlaceholders(bot, placeholders);

    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            BroadcastHelper::BotSayToWorld(bot, PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders));
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            BroadcastHelper::BotSayToChannel(bot, PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders), ChatChannelId::GENERAL);
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            BroadcastHelper::BotSayToGuild(bot, PlayerbotTextMgr::instance().GetBotText("suggest_toxic_links", placeholders));
            break;
        }
        default:
            break;
    }

    SetLastChatTime(bot, time(0) + urand(5, 60));

    return true;
}

// AC SayAction.cpp:316-410
bool ChatReplyAction::HandleWTBItemsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    auto messageItemIds = ExtractAllItemIds(msg);

    if (messageItemIds.empty())
    {
        return false;
    }

    std::set<uint32> matchingItemIds;

    for (auto messageItemId : messageItemIds)
    {
        if (HasItemInInventory(bot, messageItemId))
        {
            matchingItemIds.insert(messageItemId);
        }
    }

    if (!matchingItemIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        // %area_name, %zone_name, %my_class, %my_race, %my_level, %my_role
        BroadcastHelper::BotChatPlaceholders(bot, placeholders);
        placeholders["%formatted_item_links"] = "";

        for (auto matchingItemId : matchingItemIds)
        {
            ItemTemplate const* proto = sObjectMgr->GetItemTemplate(matchingItemId);
            if (!proto)
                continue;
            placeholders["%formatted_item_links"] += ChatHelper::FormatItem(proto, GetInventoryItemsCountWithId(bot, matchingItemId));
            placeholders["%formatted_item_links"] += " ";
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_channel", placeholders);
                    BroadcastHelper::BotSayToWorld(bot, responseMessage);
                }
                else
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_whisper", placeholders);
                    botAI->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_channel", placeholders);
                    BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::GENERAL);
                }
                else
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_whisper", placeholders);
                    botAI->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_TRADE:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_channel", placeholders);
                    BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::TRADE);
                }
                else
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_wtb_items_whisper", placeholders);
                    botAI->Whisper(responseMessage, name);
                }
                break;
            }
            default:
            break;
        }
        SetLastChatTime(bot, time(0) + urand(5, 60));
    }

    return true;
}

// AC SayAction.cpp:411-496
bool ChatReplyAction::HandleLFGQuestsReply(Player* bot, ChatChannelSource chatChannelSource, std::string& msg, std::string& name)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    auto messageQuestIds = ExtractAllQuestIds(msg);

    if (messageQuestIds.empty())
    {
        return false;
    }

    auto botQuestIds = GetAllCurrentQuestIds(bot);
    std::set<uint32> matchingQuestIds;
    for (auto botQuestId : botQuestIds)
    {
        if (messageQuestIds.count(botQuestId) != 0)
        {
            matchingQuestIds.insert(botQuestId);
        }
    }

    if (!matchingQuestIds.empty())
    {
        std::map<std::string, std::string> placeholders;
        placeholders["%other_name"] = name;
        // %area_name, %zone_name, %my_class, %my_race, %my_level, %my_role
        BroadcastHelper::BotChatPlaceholders(bot, placeholders);
        placeholders["%quest_links"] = "";
        for (auto matchingQuestId : matchingQuestIds)
        {
            Quest const* quest = sObjectMgr->GetQuestTemplate(matchingQuestId);
            if (!quest)
                continue;
            placeholders["%quest_links"] += ChatHelper::FormatQuest(quest);
        }

        switch (chatChannelSource)
        {
            case ChatChannelSource::SRC_WORLD:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_channel", placeholders);
                    BroadcastHelper::BotSayToWorld(bot, responseMessage);
                }
                else
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_whisper", placeholders);
                    botAI->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_GENERAL:
            {
                //may reply to the same channel or whisper
                if (urand(0, 1))
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_channel", placeholders);
                    BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::GENERAL);
                }
                else
                {
                    std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_whisper", placeholders);
                    botAI->Whisper(responseMessage, name);
                }
                break;
            }
            case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
            {
                //do not reply to the chat
                //may whisper
                std::string responseMessage = PlayerbotTextMgr::instance().GetBotText("response_lfg_quests_whisper", placeholders);
                botAI->Whisper(responseMessage, name);
                break;
            }
            default:
            break;
        }
        SetLastChatTime(bot, time(0) + urand(5, 25));
    }

    return true;
}

// AC SayAction.cpp:498-571
bool ChatReplyAction::SendGeneralResponse(Player* bot, ChatChannelSource chatChannelSource, std::string& responseMessage, std::string& name)
{
    PlayerbotAI* botAI = GET_PLAYERBOT_AI(bot);
    if (!botAI)
        return false;

    // send responds
    switch (chatChannelSource)
    {
        case ChatChannelSource::SRC_WORLD:
        {
            //may reply to the same channel or whisper
            BroadcastHelper::BotSayToWorld(bot, responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GENERAL:
        {
            //may reply to the same channel 80% or whisper
            if (urand(0, 100) < 80)
                BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::GENERAL);
            else
                botAI->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_TRADE:
        {
            //do not reply to the chat
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOCAL_DEFENSE:
        {
            //may reply to the same channel or whisper
            BroadcastHelper::BotSayToChannel(bot, responseMessage, ChatChannelId::LOCAL_DEFENSE);
            break;
        }
        case ChatChannelSource::SRC_WORLD_DEFENSE:
        {
            //may whisper
            break;
        }
        case ChatChannelSource::SRC_LOOKING_FOR_GROUP:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_GUILD_RECRUITMENT:
        {
            //do not reply to the chat
            break;
        }
        case ChatChannelSource::SRC_WHISPER:
        {
            botAI->Whisper(responseMessage, name);
            break;
        }
        case ChatChannelSource::SRC_SAY:
        {
            botAI->Say(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_YELL:
        {
            botAI->Yell(responseMessage);
            break;
        }
        case ChatChannelSource::SRC_GUILD:
        {
            BroadcastHelper::BotSayToGuild(bot, responseMessage);
            break;
        }
        default:
            break;
    }
    SetLastChatTime(bot, time(0) + urand(5, 25));

    return true;
}

// AC SayAction.cpp:573-1049 (verbatim, including AC's unreachable branches)
std::string ChatReplyAction::GenerateReplyMessage(Player* bot, std::string& incomingMessage, uint32& guid1, std::string& name)
{
    ChatReplyType replyType = REPLY_NOT_UNDERSTAND; // default not understand

    std::string respondsText = "";

    // Chat Logic
    int32 verb_pos = -1;
    int32 verb_type = -1;
    int32 is_quest = 0;
    bool found = false;
    std::stringstream text(incomingMessage);
    std::string segment;
    std::vector<std::string> word;
    while (std::getline(text, segment, ' '))
    {
        word.push_back(segment);
    }

    for (uint32 i = 0; i < 15; i++)
    {
        if (word.size() < i)
            word.push_back("");
    }

    if (incomingMessage.find("?") != std::string::npos)
        is_quest = 1;
    if (word[0].find("what") != std::string::npos)
        is_quest = 2;
    else if (word[0].find("who") != std::string::npos)
        is_quest = 3;
    else if (word[0] == "when")
        is_quest = 4;
    else if (word[0] == "where")
        is_quest = 5;
    else if (word[0] == "why")
        is_quest = 6;

    // Responds
    for (uint32 i = 0; i < 8; i++)
    {
        // blame gm with chat tag
        if (Player* plr = ObjectAccessor::FindPlayer(ObjectGuid::Create<HighGuid::Player>(guid1)))
        {
            if (plr->isGMChat())
            {
                replyType = REPLY_ADMIN_ABUSE;
                found = true;
                break;
            }
        }

        if (word[i] == "hi" || word[i] == "hey" || word[i] == "hello" || word[i] == "wazzup")
        {
            replyType = REPLY_HELLO;
            found = true;
            break;
        }

        if (verb_type < 4)
        {
            if (word[i] == "am" || word[i] == "are" || word[i] == "is")
            {
                verb_pos = i;
                verb_type = 2;  // present
                if (verb_pos == 0)
                    is_quest = 1;
            }
            else if (word[i] == "will")
            {
                verb_pos = i;
                verb_type = 3;  // future
            }
            else if (word[i] == "was" || word[i] == "were")
            {
                verb_pos = i;
                verb_type = 1;  // past
            }
            else if (word[i] == "shut" || word[i] == "noob")
            {
                if (incomingMessage.find(bot->GetName()) == std::string::npos)
                {
                    continue;  // not react
                    uint32 rnd = urand(0, 2);
                    std::string msg = "";
                    if (rnd == 0)
                        msg = "sorry %s, ill shut up now";
                    if (rnd == 1)
                        msg = "ok ok %s";
                    if (rnd == 2)
                        msg = "fine, i wont talk to you anymore %s";

                    msg = std::regex_replace(msg, std::regex("%s"), name);
                    respondsText = msg;
                    found = true;
                    break;
                }
                else
                {
                    replyType = REPLY_GRUDGE;
                    found = true;
                    break;
                }
            }
        }
    }
    if (verb_type < 4 && is_quest && !found)
    {
        switch (is_quest)
        {
        case 2:
        {
            uint32 rnd = urand(0, 3);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "i dont know what";
                break;
            case 1:
                msg = "i dont know %s";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afraid that was before i was around or paying attention";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 4);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "nobody";
                break;
            case 1:
                msg = "we all do";
                break;
            case 2:
                msg = "perhaps its you, %s";
                break;
            case 3:
                msg = "dunno %s";
                break;
            case 4:
                msg = "is it me?";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 4:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "soon perhaps %s";
                break;
            case 1:
                msg = "probably later";
                break;
            case 2:
                msg = "never";
                break;
            case 3:
                msg = "what do i look like, a psychic?";
                break;
            case 4:
                msg = "a few minutes, maybe an hour ... years?";
                break;
            case 5:
                msg = "when? good question %s";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 5:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "really want me to answer that?";
                break;
            case 1:
                msg = "on the map?";
                break;
            case 2:
                msg = "who cares";
                break;
            case 3:
                msg = "afk?";
                break;
            case 4:
                msg = "none of your buisiness where";
                break;
            case 5:
                msg = "yeah, where?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }

            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 6:
        {
            uint32 rnd = urand(0, 6);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "dunno %s";
                break;
            case 1:
                msg = "why? just because %s";
                break;
            case 2:
                msg = "why is the sky blue?";
                break;
            case 3:
                msg = "dont ask me %s, im just a bot";
                break;
            case 4:
                msg = "your asking the wrong person";
                break;
            case 5:
                msg = "who knows?";
                break;
            case 6:
                msg = "dunno %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        default:
        {
            switch (verb_type)
            {
            case 1:
            {
                uint32 rnd = urand(0, 3);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 4];
                    break;
                case 1:
                    msg = "ya %s but thats in the past";
                    break;
                case 2:
                    msg = "nah, but " + word[verb_pos + 1] + " will " + word[verb_pos + 3] + " again though %s";
                    break;
                case 3:
                    msg = "afraid that was before i was around or paying attention";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 2:
            {
                uint32 rnd = urand(0, 6);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "its true, " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 1:
                    msg = "ya %s thats true";
                    break;
                case 2:
                    msg = "maybe " + word[verb_pos + 1] + " " + word[verb_pos] + " " + word[verb_pos + 2] + " " + word[verb_pos + 3] + " " + word[verb_pos + 4] + " " + word[verb_pos + 5];
                    break;
                case 3:
                    msg = "dunno %s";
                    break;
                case 4:
                    msg = "i dont think so %s";
                    break;
                case 5:
                    msg = "yes";
                    break;
                case 6:
                    msg = "no";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            case 3:
            {
                uint32 rnd = urand(0, 8);
                std::string msg = "";

                switch (rnd)
                {
                case 0:
                    msg = "dunno %s";
                    break;
                case 1:
                    msg = "beats me %s";
                    break;
                case 2:
                    msg = "how should i know %s";
                    break;
                case 3:
                    msg = "dont ask me %s, im just a bot";
                    break;
                case 4:
                    msg = "your asking the wrong person";
                    break;
                case 5:
                    msg = "what do i look like, a psychic?";
                    break;
                case 6:
                    msg = "sure %s";
                    break;
                case 7:
                    msg = "i dont think so %s";
                    break;
                case 8:
                    msg = "maybe";
                    break;
                }
                msg = std::regex_replace(msg, std::regex("%s"), name);
                respondsText = msg;
                found = true;
                break;
            }
            }
        }
        }
    }
    else if (!found)
    {
        switch (verb_type)
        {
        case 1:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "yeah %s, the key word being " + word[verb_pos] + " " + word[verb_pos + 1];
                break;
            case 1:
                msg = "ya %s but thats in the past";
                break;
            case 2:
                msg = word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " again though %s";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 2:
        {
            uint32 rnd = urand(0, 2);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "%s, what do you mean " + word[verb_pos + 1] + "?";
                break;
            case 1:
                msg = "%s, what is a " + word[verb_pos + 1] + "?";
                break;
            case 2:
                msg = "yeah i know " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " is a " + word[verb_pos + 1];
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        case 3:
        {
            uint32 rnd = urand(0, 1);
            std::string msg = "";

            switch (rnd)
            {
            case 0:
                msg = "are you sure thats going to happen %s?";
                break;
            case 1:
                msg = "%s, what will happen %s?";
                break;
            case 2:
                msg = "are you saying " + word[verb_pos ? verb_pos - 1 : verb_pos + 1] + " will " + word[verb_pos + 1] + " " + word[verb_pos + 2] + " %s?";
                break;
            }
            msg = std::regex_replace(msg, std::regex("%s"), name);
            respondsText = msg;
            found = true;
            break;
        }
        }
    }

    if (!found)
    {
        // Name Responds
        if (incomingMessage.find(bot->GetName()) != std::string::npos)
        {
            replyType = REPLY_NAME;
            found = true;
        }
        else  // Does not understand
        {
            replyType = REPLY_NOT_UNDERSTAND;
            found = true;
        }
    }

    // load text if needed
    if (respondsText.empty())
    {
        respondsText = PlayerbotTextMgr::instance().GetBotText(replyType, name);
    }

    if (respondsText.size() > 255)
    {
        respondsText.resize(255);
    }

    return respondsText;
}

// AC Bot/PlayerbotAI.cpp:6632-6730. 5.4.8: ChannelMgr::forTeam takes the team (ALLIANCE/HORDE), not a TeamId.
ChatChannelSource ChatReplyAction::GetChatChannelSource(Player* bot, uint32 type, std::string channelName)
{
    if (type == CHAT_MSG_CHANNEL)
    {
        if (channelName == "World")
            return ChatChannelSource::SRC_WORLD;
        else
        {
            ChannelMgr* cMgr = ChannelMgr::forTeam(bot->GetTeam());
            if (!cMgr)
            {
                return ChatChannelSource::SRC_UNDEFINED;
            }

            Channel const* channel = cMgr->GetChannel(channelName, bot);
            if (channel)
            {
                switch (channel->GetChannelId())
                {
                    case ChatChannelId::GENERAL:
                    {
                        return ChatChannelSource::SRC_GENERAL;
                    }
                    case ChatChannelId::TRADE:
                    {
                        return ChatChannelSource::SRC_TRADE;
                    }
                    case ChatChannelId::LOCAL_DEFENSE:
                    {
                        return ChatChannelSource::SRC_LOCAL_DEFENSE;
                    }
                    case ChatChannelId::WORLD_DEFENSE:
                    {
                        return ChatChannelSource::SRC_WORLD_DEFENSE;
                    }
                    case ChatChannelId::LOOKING_FOR_GROUP:
                    {
                        return ChatChannelSource::SRC_LOOKING_FOR_GROUP;
                    }
                    case ChatChannelId::GUILD_RECRUITMENT:
                    {
                        return ChatChannelSource::SRC_GUILD_RECRUITMENT;
                    }
                    default:
                    {
                        return ChatChannelSource::SRC_UNDEFINED;
                    }
                }
            }
        }
    }
    else
    {
        switch (type)
        {
            case CHAT_MSG_WHISPER:
            {
                return ChatChannelSource::SRC_WHISPER;
            }
            case CHAT_MSG_SAY:
            {
                return ChatChannelSource::SRC_SAY;
            }
            case CHAT_MSG_YELL:
            {
                return ChatChannelSource::SRC_YELL;
            }
            case CHAT_MSG_GUILD:
            {
                return ChatChannelSource::SRC_GUILD;
            }
            case CHAT_MSG_PARTY:
            {
                return ChatChannelSource::SRC_PARTY;
            }
            case CHAT_MSG_RAID:
            {
                return ChatChannelSource::SRC_RAID;
            }
            case CHAT_MSG_EMOTE:
            {
                return ChatChannelSource::SRC_EMOTE;
            }
            case CHAT_MSG_TEXT_EMOTE:
            {
                return ChatChannelSource::SRC_TEXT_EMOTE;
            }
            default:
            {
                return ChatChannelSource::SRC_UNDEFINED;
            }
        }
    }
    return ChatChannelSource::SRC_UNDEFINED;
}

std::set<uint32> ChatReplyAction::ExtractAllQuestIds(std::string const& text) { return extractGeneric(text, "Hquest:"); }

std::set<uint32> ChatReplyAction::ExtractAllItemIds(std::string const& text) { return extractGeneric(text, "Hitem:"); }

// AC Bot/PlayerbotAI.cpp:6288
void ChatReplyAction::QueueChatResponse(Player* bot, ChatQueuedReply const& reply)
{
    std::lock_guard<std::mutex> guard(chatReplyLock);
    chatReplyStates[bot->GetGUID().GetRawValue()].replies.push_back(reply);
    ++queuedChatReplies;
}

// AC Bot/PlayerbotAI.cpp:644-656 (the "chat replies" loop of UpdateAIInternal), on the bot's own thread.
void ChatReplyAction::ProcessChatReplies(Player* bot)
{
    if (!queuedChatReplies.load(std::memory_order_relaxed))
        return;

    std::vector<ChatQueuedReply> due;
    time_t const now = time(nullptr);
    {
        std::lock_guard<std::mutex> guard(chatReplyLock);
        auto itr = chatReplyStates.find(bot->GetGUID().GetRawValue());
        if (itr == chatReplyStates.end())
            return;

        std::list<ChatQueuedReply>& replies = itr->second.replies;
        for (auto it = replies.begin(); it != replies.end();)
        {
            time_t checkTime = it->m_time;
            if (checkTime && now < checkTime)
            {
                ++it;
                continue;
            }

            if (!checkTime || now - checkTime <= STALE_CHAT_REPLY_SECONDS)
                due.push_back(*it);

            it = replies.erase(it);
            --queuedChatReplies;
        }

        if (replies.empty() && (!itr->second.lastChatSet || itr->second.lastChat <= now))
            chatReplyStates.erase(itr);
    }

    for (ChatQueuedReply& reply : due)
        ChatReplyDo(bot, reply.m_type, reply.m_guid1, reply.m_msg, reply.m_chanName, reply.m_name);
}

time_t ChatReplyAction::GetLastChatTime(Player* bot)
{
    std::lock_guard<std::mutex> guard(chatReplyLock);
    auto itr = chatReplyStates.find(bot->GetGUID().GetRawValue());
    if (itr == chatReplyStates.end() || !itr->second.lastChatSet)
        return time(nullptr) - 120;  // AC LastSaidValue default

    return itr->second.lastChat;
}

void ChatReplyAction::SetLastChatTime(Player* bot, time_t value)
{
    std::lock_guard<std::mutex> guard(chatReplyLock);
    BotChatReplyState& state = chatReplyStates[bot->GetGUID().GetRawValue()];
    state.lastChat = value;
    state.lastChatSet = true;
}
