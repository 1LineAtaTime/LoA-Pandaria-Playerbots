/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TellPvpStatsAction.cpp. 5.4.8 adaptations: no arena points; the
 * currency line reports Conquest and Honor points (Player::GetCurrency), and the team lines report the bot
 * personal rating per rated bracket (2v2, 3v3, 5v5, rated battleground), since arena teams do not exist on this
 * core (Player::GetArenaPersonalRating). The text keys keep the AC names; the "%arena_points" placeholder carries
 * the Conquest points so an AC-style text still renders.
 */

#include "TellPvpStatsAction.h"

#include "DBCEnums.h"
#include "Event.h"
#include "Language.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "PlayerbotTextMgr.h"
#include "Playerbots.h"

#include <map>

namespace
{
// Rated PvP slots as in RatedPvp.h (RatedPvpSlot); the header is not included because it needs DatabaseEnv.h first.
uint8 const PVP_SLOTS = 4;

inline char const* BracketName(uint8 slot)
{
    switch (slot)
    {
        case 0:
            return "2v2";
        case 1:
            return "3v3";
        case 2:
            return "5v5";
        default:
            return "rated BG";
    }
}
}  // namespace

bool TellPvpStatsAction::Execute(Event event)
{
    if (!bot)
        return false;

    // Prefer the actual chat sender (whisper / say / etc.) if available.
    Player* requester = event.getOwner();

    // Fallback to master if event owner is not available.
    if (!requester)
        requester = GetMaster();

    // If we still do not have a valid player to answer to, bail out.
    if (!requester)
        return false;

    // PVP currencies
    std::map<std::string, std::string> currencyPlaceholders;
    currencyPlaceholders["%arena_points"] = std::to_string(bot->GetCurrency(CURRENCY_TYPE_CONQUEST_POINTS, false));
    currencyPlaceholders["%conquest_points"] = currencyPlaceholders["%arena_points"];
    currencyPlaceholders["%honor_points"] = std::to_string(bot->GetCurrency(CURRENCY_TYPE_HONOR_POINTS, false));

    std::string const currencyText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
        "pvp_currency",
        "[PVP] Conquest points: %conquest_points | Honor Points: %honor_points",
        currencyPlaceholders);

    bot->Whisper(currencyText, LANG_UNIVERSAL, requester);

    // Personal rating per rated bracket.
    bool anyTeam = false;
    for (uint8 slot = 0; slot < PVP_SLOTS; ++slot)
    {
        uint32 const rating = bot->GetArenaPersonalRating(slot);
        if (!rating)
            continue;

        anyTeam = true;
        std::map<std::string, std::string> placeholders;
        placeholders["%bracket"] = BracketName(slot);
        placeholders["%team_name"] = bot->GetName();
        placeholders["%team_rating"] = std::to_string(rating);

        std::string const teamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pvp_arena_team", "[PVP] %bracket: rating %team_rating", placeholders);

        bot->Whisper(teamText, LANG_UNIVERSAL, requester);
    }

    if (!anyTeam)
    {
        std::string const noTeamText = PlayerbotTextMgr::instance().GetBotTextOrDefault(
            "pvp_no_arena_team", "[PVP] I have no rated PvP rating.", std::map<std::string, std::string>());

        bot->Whisper(noTeamText, LANG_UNIVERSAL, requester);
    }

    return true;
}
