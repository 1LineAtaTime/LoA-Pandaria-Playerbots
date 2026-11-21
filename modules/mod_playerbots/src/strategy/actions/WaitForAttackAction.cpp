/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/WaitForAttackAction.cpp (SetWaitForAttackTimeAction), 1:1 apart from
 * the PlayerbotTextMgr call, which takes its placeholder map without a default on this module.
 */

#include "WaitForAttackAction.h"

#include "Event.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

#include <algorithm>
#include <cctype>

bool SetWaitForAttackTimeAction::Execute(Event event)
{
    std::string newTimeStr = event.getParam();

    if (newTimeStr.empty())
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "wait_for_attack_provide_time", "Please provide a time to set (in seconds)",
            std::map<std::string, std::string>());
        botAI->TellMaster(text);
        return false;
    }

    if (!std::all_of(newTimeStr.begin(), newTimeStr.end(), ::isdigit))
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "wait_for_attack_invalid_time", "Please provide valid time to set (in seconds) between 0 and 99",
            std::map<std::string, std::string>());
        botAI->TellMaster(text);
        return false;
    }

    // AC runs std::stoi on the digit string unchecked; a very long run of digits throws out_of_range. Bound the length
    // first: anything of three or more digits is above the 99 limit checked next anyway.
    if (newTimeStr.size() > 3)
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "wait_for_attack_invalid_time", "Please provide valid time to set (in seconds) between 0 and 99",
            std::map<std::string, std::string>());
        botAI->TellMaster(text);
        return false;
    }

    int newTime = std::stoi(newTimeStr);
    if (newTime < 0 || newTime > 99)
    {
        std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "wait_for_attack_invalid_time", "Please provide valid time to set (in seconds) between 0 and 99",
            std::map<std::string, std::string>());
        botAI->TellMaster(text);
        return false;
    }

    context->GetValue<uint8>("wait for attack time")->Set(static_cast<uint8>(newTime));

    std::map<std::string, std::string> placeholders;
    placeholders["%new_time"] = std::to_string(newTime);
    std::string const text = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "wait_for_attack_time_set", "Wait for attack time set to %new_time seconds", placeholders);
    botAI->TellMaster(text);
    return true;
}
