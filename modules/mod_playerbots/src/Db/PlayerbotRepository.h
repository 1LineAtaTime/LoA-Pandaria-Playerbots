/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

#ifndef _PLAYERBOT_PLAYERBOTREPOSITORY_H
#define _PLAYERBOT_PLAYERBOTREPOSITORY_H

#include <string>
#include <vector>

#include "Define.h"

class PlayerbotAI;

// A bot's strategies and saved values (loot strategy, always-loot list, positions, rti, formation...) in
// playerbots_db_store: loaded when the bot logs in, saved when its strategies change and when it leaves its master
class PlayerbotRepository
{
public:
    static PlayerbotRepository& instance()
    {
        static PlayerbotRepository instance;

        return instance;
    }

    void Save(PlayerbotAI* botAI);
    void Load(PlayerbotAI* botAI);
    void Reset(PlayerbotAI* botAI);

private:
    PlayerbotRepository() = default;
    ~PlayerbotRepository() = default;

    PlayerbotRepository(PlayerbotRepository const&) = delete;
    PlayerbotRepository& operator=(PlayerbotRepository const&) = delete;

    PlayerbotRepository(PlayerbotRepository&&) = delete;
    PlayerbotRepository& operator=(PlayerbotRepository&&) = delete;

    std::string const FormatStrategies(std::string const type, std::vector<std::string> strategies);
};

#endif
