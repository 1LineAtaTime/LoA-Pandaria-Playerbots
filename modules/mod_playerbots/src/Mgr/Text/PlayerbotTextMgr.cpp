/*
 * Ported from AC mod-playerbots Mgr/Text/PlayerbotTextMgr.cpp (#322). 5.4.8 adaptations:
 *  - DB access: the fork's PlayerbotsDatabase already carries PLAYERBOTS_SEL_TEXT (columns name, text,
 *    say_type, reply_type, text_loc1..text_loc8 - the same 12-column layout AC's loader reads, verified
 *    against playerbots.ai_playerbot_texts). There is no PLAYERBOTS_SEL_TEXT_CHANCE statement on this
 *    fork, so LoadBotTextChance() uses an ad-hoc query (PlayerbotsDatabase.PQuery, as
 *    RandomPlayerbotMgr::PrepareTeleportCache does) instead of adding a statement to the core.
 *  - TOTAL_LOCALES is 12 on 5.4.8 (AC: 9) but the table only has 9 text columns (text + loc1..8), so the
 *    loader reads TEXT_COLUMNS locales; higher locales stay empty and fall back to text[0].
 *  - Lookups use find() instead of AC's operator[] (AC creates empty map entries on every miss, which is
 *    a data race when several map threads ask for a text at once). Results are identical.
 *  - Text loading tolerates a missing/empty table: logs and continues (GetBotText returns "").
 *  - sWorldSessionMgr->GetActiveSessionCount() is sWorld->GetActiveSessionCount() on 5.4.8.
 */

#include "PlayerbotTextMgr.h"

#include "DatabaseEnv.h"
#include "Log.h"
#include "QueryResult.h"
#include "Random.h"
#include "World.h"

namespace
{
    // ai_playerbot_texts has text + text_loc1..text_loc8 = 9 locale columns (AC TOTAL_LOCALES = 9).
    uint8 const TEXT_COLUMNS = 9;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:14-24
void PlayerbotTextMgr::replaceAll(std::string& str, std::string const& from, std::string const& to)
{
    if (from.empty())
        return;
    size_t start_pos = 0;
    while ((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length();  // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:26-52
void PlayerbotTextMgr::LoadBotTexts()
{
    TC_LOG_INFO("playerbots", "Loading playerbots texts...");

    botTexts.clear();

    uint32 count = 0;
    if (PreparedQueryResult result =
            PlayerbotsDatabase.Query(PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_TEXT)))
    {
        do
        {
            std::map<uint32, std::string> text;
            Field* fields = result->Fetch();
            std::string name = fields[0].GetString();
            text[0] = fields[1].GetString();
            uint8 sayType = fields[2].GetUInt8();
            uint8 replyType = fields[3].GetUInt8();
            for (uint8 i = 1; i < TEXT_COLUMNS && i < TOTAL_LOCALES; ++i)
            {
                text[i] = fields[i + 3].GetString();
            }

            botTexts[name].push_back(BotTextEntry(name, text, sayType, replyType));
            ++count;
        } while (result->NextRow());
    }
    else
        TC_LOG_ERROR("playerbots", "ai_playerbot_texts is missing or empty - bot chat texts unavailable");

    TC_LOG_INFO("playerbots", "%u playerbots texts loaded", count);
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:54-72 (PLAYERBOTS_SEL_TEXT_CHANCE = "SELECT name, probability FROM
// ai_playerbot_texts_chance"; absent on this fork, see the file header).
void PlayerbotTextMgr::LoadBotTextChance()
{
    if (botTextChance.empty())
    {
        QueryResult results = PlayerbotsDatabase.PQuery("SELECT `name`, `probability` FROM `ai_playerbot_texts_chance`");
        if (results)
        {
            do
            {
                Field* fields = results->Fetch();
                std::string name = fields[0].GetString();
                uint32 probability = uint32(fields[1].GetInt64());

                botTextChance[name] = probability;
            } while (results->NextRow());
        }
    }
}

// general texts

// AC Mgr/Text/PlayerbotTextMgr.cpp:76-92
std::string PlayerbotTextMgr::GetBotText(std::string name)
{
    if (botTexts.empty())
    {
        TC_LOG_ERROR("playerbots", "Can't get bot text %s! No bots texts loaded!", name.c_str());
        return "";
    }

    auto itr = botTexts.find(name);
    if (itr == botTexts.end() || itr->second.empty())
    {
        TC_LOG_ERROR("playerbots", "Can't get bot text %s! No bots texts for this name!", name.c_str());
        return "";
    }

    std::vector<BotTextEntry>& list = itr->second;
    BotTextEntry const& textEntry = list[urand(0, list.size() - 1)];
    uint32 const locale = GetLocalePriority();
    auto const l = textEntry.m_text.find(locale);
    if (l != textEntry.m_text.end() && !l->second.empty())
        return l->second;

    auto const d = textEntry.m_text.find(0);
    return d != textEntry.m_text.end() ? d->second : std::string();
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:94-104
std::string PlayerbotTextMgr::GetBotText(std::string name, std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name);
    if (botText.empty())
        return "";

    for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        replaceAll(botText, i->first, i->second);

    return botText;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:106-120
std::string PlayerbotTextMgr::GetBotTextOrDefault(std::string name, std::string defaultText,
                                                  std::map<std::string, std::string> placeholders)
{
    std::string botText = GetBotText(name, placeholders);
    if (botText.empty())
    {
        for (std::map<std::string, std::string>::iterator i = placeholders.begin(); i != placeholders.end(); ++i)
        {
            replaceAll(defaultText, i->first, i->second);
        }
        return defaultText;
    }

    return botText;
}

// chat replies

// AC Mgr/Text/PlayerbotTextMgr.cpp:124-157
std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::map<std::string, std::string> placeholders)
{
    if (botTexts.empty())
    {
        TC_LOG_ERROR("playerbots", "Can't get bot text reply %u! No bots texts loaded!", uint32(replyType));
        return "";
    }
    auto itr = botTexts.find("reply");
    if (itr == botTexts.end() || itr->second.empty())
    {
        TC_LOG_ERROR("playerbots", "Can't get bot text reply %u! No bots texts replies!", uint32(replyType));
        return "";
    }

    std::vector<BotTextEntry>& list = itr->second;
    std::vector<BotTextEntry const*> proper_list;
    for (BotTextEntry const& text : list)
    {
        if (text.m_replyType == uint32(replyType))
            proper_list.push_back(&text);
    }

    if (proper_list.empty())
        return "";

    BotTextEntry const& textEntry = *proper_list[urand(0, proper_list.size() - 1)];
    uint32 const locale = GetLocalePriority();
    std::string botText;
    auto const l = textEntry.m_text.find(locale);
    if (l != textEntry.m_text.end() && !l->second.empty())
        botText = l->second;
    else
    {
        auto const d = textEntry.m_text.find(0);
        if (d != textEntry.m_text.end())
            botText = d->second;
    }
    for (auto& placeholder : placeholders)
        replaceAll(botText, placeholder.first, placeholder.second);

    return botText;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:159-165
std::string PlayerbotTextMgr::GetBotText(ChatReplyType replyType, std::string name)
{
    std::map<std::string, std::string> placeholders;
    placeholders["%s"] = name;

    return GetBotText(replyType, placeholders);
}

// probabilities

// AC Mgr/Text/PlayerbotTextMgr.cpp:169-175 (a name with no/zero chance row always passes).
bool PlayerbotTextMgr::rollTextChance(std::string name)
{
    auto itr = botTextChance.find(name);
    if (itr == botTextChance.end() || !itr->second)
        return true;

    return urand(0, 100) < itr->second;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:177-184
bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name);
    return !text.empty();
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:186-193
bool PlayerbotTextMgr::GetBotText(std::string name, std::string& text, std::map<std::string, std::string> placeholders)
{
    if (!rollTextChance(name))
        return false;

    text = GetBotText(name, placeholders);
    return !text.empty();
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:195-203
void PlayerbotTextMgr::AddLocalePriority(uint32 locale)
{
    if (locale >= TEXT_COLUMNS || locale >= TOTAL_LOCALES)
    {
        TC_LOG_WARN("playerbots", "Ignoring locale %u for bot texts because it exceeds the text columns (%u)",
                    locale, uint32(TEXT_COLUMNS - 1));
        return;
    }

    botTextLocalePriority[locale]++;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:205-221
uint32 PlayerbotTextMgr::GetLocalePriority()
{
    // if no real players online, reset top locale
    uint32 const activeSessions = sWorld->GetActiveSessionCount();
    if (!activeSessions)
    {
        ResetLocalePriority();
        return 0;
    }

    uint32 topLocale = 0;
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        if (botTextLocalePriority[i] > botTextLocalePriority[topLocale])
            topLocale = i;
    }

    return topLocale;
}

// AC Mgr/Text/PlayerbotTextMgr.cpp:223-230
void PlayerbotTextMgr::ResetLocalePriority()
{
    for (uint8 i = 0; i < TOTAL_LOCALES; ++i)
    {
        botTextLocalePriority[i] = 0;
    }
}
