/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#include "PlayerbotRepository.h"

#include <sstream>

#include "AiObjectContext.h"
#include "DatabaseEnv.h"
#include "Log.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"

namespace
{
    // playerbots_db_store.value is a varchar(255); a longer row would fail the whole save under strict SQL mode
    constexpr std::size_t DB_STORE_VALUE_MAX_LENGTH = 255;
}

// Runs on the world thread at login; the query uses a synchronous connection, as AzerothCore's. As there, a bot
// that logs in again right after logging out may read the rows from before its logout save, which is asynchronous.
void PlayerbotRepository::Load(PlayerbotAI* botAI)
{
    uint32 guid = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_SEL_DB_STORE);
    stmt->setUInt32(0, guid);
    if (PreparedQueryResult result = PlayerbotsDatabase.Query(stmt))
    {
        std::vector<std::string> values;
        do
        {
            Field* fields = result->Fetch();
            std::string const key = fields[0].GetString();
            std::string const value = fields[1].GetString();

            if (key == "value")
                values.push_back(value);
            else if (key == "co")
            {
                botAI->ClearStrategies(BOT_STATE_COMBAT);
                botAI->ChangeStrategy("+chat", BOT_STATE_COMBAT);
                botAI->ChangeStrategy(value, BOT_STATE_COMBAT);
            }
            else if (key == "nc")
            {
                botAI->ClearStrategies(BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy("+chat", BOT_STATE_NON_COMBAT);
                botAI->ChangeStrategy(value, BOT_STATE_NON_COMBAT);
            }
            else if (key == "dead")
                botAI->ChangeStrategy(value, BOT_STATE_DEAD);
        } while (result->NextRow());

        botAI->GetAiObjectContext()->Load(values);
    }
}

// Also called from the bot's map thread (the co/nc commands): it only reads the bot's own AI and queues the
// statements on the asynchronous connection, which is thread-safe. The delete and the inserts go in one
// transaction so that they run in order whatever PlayerbotsDatabase.WorkerThreads is.
void PlayerbotRepository::Save(PlayerbotAI* botAI)
{
    uint32 guid = botAI->GetBot()->GetGUID().GetCounter();

    std::vector<std::pair<std::string, std::string>> rows;

    std::vector<std::string> data = botAI->GetAiObjectContext()->Save();
    for (std::vector<std::string>::iterator i = data.begin(); i != data.end(); ++i)
        rows.emplace_back("value", *i);

    rows.emplace_back("co", FormatStrategies("co", botAI->GetStrategies(BOT_STATE_COMBAT)));
    rows.emplace_back("nc", FormatStrategies("nc", botAI->GetStrategies(BOT_STATE_NON_COMBAT)));
    rows.emplace_back("dead", FormatStrategies("dead", botAI->GetStrategies(BOT_STATE_DEAD)));

    PlayerbotsDatabaseTransaction trans = PlayerbotsDatabase.BeginTransaction();

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_DB_STORE);
    stmt->setUInt32(0, guid);
    trans->Append(stmt);

    for (std::pair<std::string, std::string> const& row : rows)
    {
        if (row.second.size() > DB_STORE_VALUE_MAX_LENGTH)
        {
            TC_LOG_ERROR("playerbots", "Bot %s: '%s' is too long to store (%u characters): %s",
                botAI->GetBot()->GetName().c_str(), row.first.c_str(), uint32(row.second.size()), row.second.c_str());
            continue;
        }

        stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_INS_DB_STORE);
        stmt->setUInt32(0, guid);
        stmt->setString(1, row.first);
        stmt->setString(2, row.second);
        trans->Append(stmt);
    }

    PlayerbotsDatabase.CommitTransaction(trans);
}

std::string const PlayerbotRepository::FormatStrategies(std::string const /*type*/, std::vector<std::string> strategies)
{
    std::ostringstream out;
    for (std::vector<std::string>::iterator i = strategies.begin(); i != strategies.end(); ++i)
        out << "+" << (*i).c_str() << ",";

    std::string const res = out.str();
    return res.substr(0, res.size() - 1);
}

void PlayerbotRepository::Reset(PlayerbotAI* botAI)
{
    uint32 guid = botAI->GetBot()->GetGUID().GetCounter();

    PlayerbotsPreparedStatement* stmt = PlayerbotsDatabase.GetPreparedStatement(PLAYERBOTS_DEL_DB_STORE);
    stmt->setUInt32(0, guid);
    PlayerbotsDatabase.Execute(stmt);
}
