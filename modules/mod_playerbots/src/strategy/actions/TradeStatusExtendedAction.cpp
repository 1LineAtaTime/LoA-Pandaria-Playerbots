/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeStatusExtendedAction.cpp for issue #193 (S7 of
 * epic #163).
 *
 * 5.4.8 adaptation vs AC (the only one this file needs):
 *  - SMSG_TRADE_STATUS_EXTENDED's wire format (Handlers/TradeHandler.cpp:87-191,
 *    WorldSession::SendUpdateTrade) is a fully bit-packed item list on this core: two conditional
 *    ObjectGuids per item (giftCreatorGuid always, creatorGuid only when the item is "not wrapped"),
 *    with the item's fields split between the leading bit stream and a separate byte-aligned buffer
 *    appended afterwards, addressed by real trade slot index rather than by loop position. That bears
 *    no resemblance to AC's plain sequential WotLK layout
 *    (`p >> itemId; p >> displayId; p >> count; ...` etc. for a fixed TRADE_SLOT_COUNT run) this
 *    action used to parse.
 *
 *    Everything AC actually acts on here - whether the "do not trade" slot (TRADE_SLOT_NONTRADED)
 *    holds a locked item the bot could pick - is already exposed directly by the core through
 *    TradeData/Item (Player::GetTrader()->GetTradeData()->GetItem(TRADE_SLOT_NONTRADED),
 *    Item::IsLocked(), ItemTemplate::LockID), so this reads that instead of re-deriving the same
 *    information from the packet body (the brief's "prefer the core's own object state over a hand
 *    parsed packet" rule, same as the trade-discount helpers in TradeStatusAction.cpp). The event's
 *    packet payload is intentionally never touched.
 */

#include "TradeStatusExtendedAction.h"

#include "Event.h"
#include "Player.h"
#include "PlayerbotAI.h"
#include "TradeData.h"

namespace
{
constexpr uint32 SPELL_PICK_LOCK = 1804;
}

bool TradeStatusExtendedAction::Execute(Event /*event*/)
{
    Player* trader = bot->GetTrader();
    if (!trader)
        return false;

    TradeData* tradeData = trader->GetTradeData();
    if (!tradeData)
        return false;

    Item* lockbox = tradeData->GetItem(TRADE_SLOT_NONTRADED);
    if (!lockbox)
        return false;

    // Check for a locked item in the "Do Not Trade" slot.
    if (lockbox->GetTemplate() && lockbox->GetTemplate()->LockID > 0 && lockbox->IsLocked())
    {
        if (bot->GetClass() == CLASS_ROGUE && bot->HasSpell(SPELL_PICK_LOCK))
        {
            botAI->DoSpecificAction("unlock traded item");
            botAI->SetNextCheckDelay(4000); // Delay before accepting trade
        }
        else
        {
            botAI->TellMaster("I can't unlock this item.");
        }
    }

    return true;
}
