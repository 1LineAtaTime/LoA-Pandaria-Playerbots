/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option) any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Value/GuildValues.cpp (#611).
 * 5.4.8: chat->FormatQItem(5863) -> "5863" (the fork's plain numeric item-id qualifier, same convention as
 * BudgetValues.cpp:212); CharacterDatabase.Query("{}") -> PQuery("%u") (this core's printf-style query).
 */

#include "GuildValues.h"
#include "Playerbots.h"

uint8 PetitionSignsValue::Calculate()
{
    if (bot->GetGuildId())
        return 0;

    std::vector<Item*> petitions = AI_VALUE2(std::vector<Item*>, "inventory items", std::to_string(5863));
    if (petitions.empty())
        return 0;

    QueryResult result = CharacterDatabase.PQuery("SELECT playerguid FROM petition_sign WHERE petitionguid = %u",
                                                  petitions.front()->GetGUID().GetCounter());
    return result ? (uint8)result->GetRowCount() : 0;
}
