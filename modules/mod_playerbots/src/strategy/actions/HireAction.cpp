/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 */

/*
 * Ported from AC mod-playerbots Ai/Base/Actions/HireAction.cpp for #258. Differences:
 *  - RandomPlayerbotMgr::GetTradeDiscount() does not exist on this fork's manager (Manager/RandomPlayerbotMgr.h);
 *    AC's body (RandomPlayerbotMgr.cpp:3084-3095) is a GetEventValue() of "trade_discount_<masterGuid>", reproduced over the public GetValue(bot, name) (same helper as TradeStatusAction.cpp:97),
 *    here file-locally so the manager header stays untouched.
 *  - QueryResult/Field API of this core: PQuery + GetUInt64, not AC's fmt Query + Get<uint64>.
 */

#include "HireAction.h"

#include "ChatHelper.h"
#include "DatabaseEnv.h"
#include "Event.h"
#include "PlayerbotAI.h"
#include "Playerbots.h"
#include "RandomPlayerbotMgr.h"
#include "WorldSession.h"

namespace
{
uint32 GetTradeDiscountLocal(Player* bot, Player* master)
{
    if (!master)
        return 0;

    std::ostringstream name;
    name << "trade_discount_" << master->GetGUID().GetCounter();
    return sRandomPlayerbotMgr->GetValue(bot, name.str());
}
}

bool HireAction::Execute(Event /*event*/)
{
    Player* master = GetMaster();
    if (!master)
        return false;

    if (!sRandomPlayerbotMgr->IsRandomBot(bot))
        return false;

    uint32 account = master->GetSession()->GetAccountId();
    QueryResult results = CharacterDatabase.PQuery("SELECT COUNT(*) FROM characters WHERE account = %u", account);

    uint32 charCount = 10;
    if (results)
    {
        Field* fields = results->Fetch();
        charCount = uint32(fields[0].GetUInt64());
    }

    if (charCount >= 10)
    {
        botAI->TellMaster("You already have the maximum number of characters");
        return false;
    }

    if (bot->GetLevel() > master->GetLevel())
    {
        botAI->TellMaster("You cannot hire higher level characters than you");
        return false;
    }

    uint32 discount = GetTradeDiscountLocal(bot, master);
    uint32 m = 1 + (bot->GetLevel() / 10);
    uint32 moneyReq = m * 5000 * bot->GetLevel();
    if (discount < moneyReq)
    {
        std::ostringstream out;
        out << "You cannot hire me - I barely know you. Make sure you have at least " << chat->formatMoney(moneyReq)
            << " as a trade discount";
        botAI->TellMaster(out.str());
        return false;
    }

    botAI->TellMaster("I will join you at your next relogin");

    bot->SetMoney(moneyReq);
    sRandomPlayerbotMgr->Remove(bot);
    CharacterDatabase.PExecute("UPDATE characters SET account = %u WHERE guid = %u", account,
                               bot->GetGUID().GetCounter());

    return true;
}
