/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license, you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/TradeStatusAction.cpp for issue #193 (S7 of epic #163).
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *  - SMSG_TRADE_STATUS is bit-packed on this core (Handlers/TradeHandler.cpp:35-75,
 *    WorldSession::SendTradeStatus: 1 unk bit + 5-bit status), not the plain `p >> status` AC's WotLK
 *    client received; Execute() reads it with ReadBit()/ReadBits(5).
 *  - TradeStatus enum values/names differ completely from AC's WotLK enum (SharedDefines.h:4238-4266
 *    here vs SharedDefines.h:3804-3830 in the AC tree) - mapped by what the core actually sends, not
 *    by name or value:
 *      AC TRADE_STATUS_BEGIN_TRADE  (the trade window opens)      -> split across two statuses on
 *          this core, because WorldSession::HandleInitiateTradeOpcode() (TradeHandler.cpp:636-782)
 *          does NOT send TRADE_STATUS_INITIATED like the note here previously claimed - it sends
 *          TRADE_STATUS_PROPOSED (=24) to the trade TARGET only (TradeHandler.cpp:758-782), and only
 *          WorldSession::HandleBeginTradeOpcode() (TradeHandler.cpp:611-619, triggered by the
 *          client's own CMSG_BEGIN_TRADE) sends TRADE_STATUS_INITIATED, to both sides. A real 5.4.8
 *          client reacts to a PROPOSED invite by popping a "will you trade with X" confirm and,
 *          when the player clicks OK, sending CMSG_BEGIN_TRADE - AC's WotLK client has no such
 *          confirm step and gets BEGIN_TRADE (the "the window is open, list your stuff" signal)
 *          directly. So here PROPOSED is where the bot accepts the invite (calls
 *          HandleBeginTradeOpcode() itself, mirroring the confirm click) and INITIATED is where it
 *          actually reacts to the window opening (facing + BeginTrade()'s inventory dump) - AC's
 *          single BEGIN_TRADE branch is split across the two on this core so a bot that receives
 *          only one of them (either side of a trade, see Execute()/BeginTrade() below) still reacts
 *          exactly once.
 *      AC TRADE_STATUS_TRADE_ACCEPT (the trade partner just accepted) -> TRADE_STATUS_ACCEPTED
 *          (sent to the *other* side in HandleAcceptTradeOpcode, TradeHandler.cpp:500,598)
 *      AC TRADE_STATUS_BACK_TO_TRADE: this core never sends an equivalent status for "trade state
 *          changed while the trader is still accepted" - TRADE_STATUS_STATE_CHANGED (=9) is declared
 *          in SharedDefines.h but not emitted anywhere in Handlers/TradeHandler.cpp or
 *          Entities/Player/TradeData.cpp (grepped both, and the whole src/ tree). Every real
 *          state-change instead re-sends TRADE_STATUS_UNACCEPTED (TradeData::SetAccepted,
 *          TradeData.cpp:120-130) to whichever side just had its own accept flag cleared. That branch
 *          has nothing to bind to and is dropped rather than guessed at.
 *  - IsSelfBot()/HasGameClientMaster(): no such concepts on this fork (see e.g.
 *    strategy/generic/WaitForAttackStrategy.cpp's own note on the same AC gap, and
 *    strategy/actions/LeaveGroupAction.h's LeaveFarAwayAction comment). "traderIsGameClientPlayer" is
 *    just "trader has no PlayerbotAI at all" (!GET_PLAYERBOT_AI(trader)); HasGameClientMaster() ->
 *    HasRealPlayerMaster() (established mapping, see strategy/actions/TaxiAction.cpp's file header).
 *  - PlayerbotTextMgr::GetBotTextOrDefault(): not ported on this fork (same as every other chat/tell
 *    string ported so far, e.g. strategy/actions/UseMeetingStoneAction.cpp) - plain default strings
 *    are used directly.
 *  - chat->FormatItem()/FormatWorldobject()/formatMoney(): FormatItem and formatMoney were ported in #331
 *    (AC Bot/Cmd/ChatHelper.cpp:190 and :432-457) and are used here since #334. proto->Name1 and
 *    Player::GetName() still stand in for AC's FormatWorldobject, which is not ported. Note TradeData's
 *    money is uint64 here (Entities/Player/TradeData.h:56) vs AC's uint32, so CheckTrade() still uses int64
 *    money math to avoid overflow - only the *rendering* was aligned to AC's uint32 formatter.
 *  - botAI->PlaySound(): PlayerbotAI has no PlaySound() on this fork (PlayerbotAI.cpp:793-843 shows it
 *    commented out everywhere it would be used, and PlayerbotAI.h is off-limits for this issue) - every
 *    PlaySound() call AC makes here is dropped.
 *  - CraftValue/GuildTaskMgr/SetCraftAction: no crafting-order system exists on this fork at all - the
 *    craft-fee/reserved-reagent carve-outs in AC's CheckTrade()/CalculateCost() and the
 *    craft-progress bookkeeping in Execute() are dropped outright (not stubbed).
 *  - sPlayerbotAIConfig.x -> sPlayerbotAIConfig->x, sRandomPlayerbotMgr.x -> sRandomPlayerbotMgr->x
 *    (both pointer singletons on this fork).
 *  - RandomPlayerbotMgr::IsAddclassBot()/GetBuyMultiplier()/GetSellMultiplier(): not exposed on this
 *    fork's Manager/RandomPlayerbotMgr.h (only IsRandomBot(Player*) is public there). IsAddclassBot()
 *    checks are dropped (PlayerbotAI.cpp:1371-1372 already documents this fork has no alt/addclass-bot
 *    security concept). GetBuyMultiplier()/GetSellMultiplier() are reimplemented below as file-local
 *    helpers on top of the same public RandomPlayerbotMgr::GetValue()/SetValue() API the trade-discount
 *    helpers use, mirroring AC's RandomPlayerbotMgr.cpp:3009-3037 logic - the only difference being the
 *    persisted value's expiry: AC uses its own minRandomBotsPriceChangeInterval/
 *    maxRandomBotsPriceChangeInterval config (not ported on this fork), so this reuses
 *    sPlayerbotAIConfig->maxRandomBotInWorldTime, the same expiry SetValue()/SetTradeDiscount() already
 *    use for the discount. Flagged in the #193 report as a judgment call, since it revives a small piece
 *    of AC's random-bot vendor economy that was never explicitly asked for by this issue.
 *  - RandomPlayerbotMgr::Get/Set/AddTradeDiscount(): not exposed either (same header). Reimplemented
 *    below as file-local helpers on top of the public GetValue(uint32, string)/SetValue(uint32, string,
 *    uint32, string) (Manager/RandomPlayerbotMgr.h:134-138), which already wrap
 *    GetEventValue()/SetEventValue() exactly the way AC's own helpers do (RandomPlayerbotMgr.cpp:921-938
 *    on this fork vs AC's RandomPlayerbotMgr.cpp:3039-3075) - same "trade_discount_<masterGuidLow>" key.
 *  - AC's InventoryAction::TellItems() (Ai/Base/Actions/InventoryAction.h:24) does not exist on this
 *    fork's cut-down InventoryAction (strategy/actions/InventoryAction.h) - a small local
 *    TellInventoryList() replaces it, listing proto->Name1 (itemId) xCount instead of a clickable link
 *    (same simplification as LootStrategyAction.cpp's "?" query listing).
 *  - QueryItemUsageAction (AC's base class here) does not exist on this fork; nothing this action uses
 *    needs it (no QueryItem()/QueryItemPrice() call sites), so TradeStatusAction is based on
 *    InventoryAction directly, for IterateItems()/ListItemsVisitor only.
 */

#include "TradeStatusAction.h"

#include "Event.h"
#include "ItemUsageValue.h"
#include "ItemVisitors.h"
#include "ChatHelper.h"
#include "Playerbots.h"
#include "BotArc.h"
#include "TradeData.h"

namespace
{
// 5.4.8: see the RandomPlayerbotMgr::Get/Set/AddTradeDiscount() note in the file header.
uint32 GetTradeDiscount(Player* bot, Player* master)
{
    if (!master)
        return 0;

    std::ostringstream name;
    name << "trade_discount_" << master->GetGUID().GetCounter();
    return sRandomPlayerbotMgr->GetValue(bot, name.str());
}

void SetTradeDiscount(Player* bot, Player* master, uint32 value)
{
    if (!master)
        return;

    std::ostringstream name;
    name << "trade_discount_" << master->GetGUID().GetCounter();
    sRandomPlayerbotMgr->SetValue(bot, name.str(), value);
}

void AddTradeDiscount(Player* bot, Player* master, int32 value)
{
    if (!master)
        return;

    uint32 discount = GetTradeDiscount(bot, master);
    int32 result = (int32)discount + value;
    SetTradeDiscount(bot, master, result < 0 ? 0 : uint32(result));
}

// 5.4.8: see the GetBuyMultiplier()/GetSellMultiplier() note in the file header.
double GetBuyMultiplier(Player* bot)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, "buymultiplier");
    if (!value)
    {
        value = urand(50, 120);
        sRandomPlayerbotMgr->SetValue(bot, "buymultiplier", value);
    }

    return (double)value / 100.0;
}

double GetSellMultiplier(Player* bot)
{
    uint32 value = sRandomPlayerbotMgr->GetValue(bot, "sellmultiplier");
    if (!value)
    {
        value = urand(80, 250);
        sRandomPlayerbotMgr->SetValue(bot, "sellmultiplier", value);
    }

    return (double)value / 100.0;
}

// 5.4.8: see the chat->formatMoney() note in the file header. #334: the local copy that used to live here
// took a uint64 because TradeData::GetMoney() is int32/uint64 at its sources; ChatHelper::formatMoney is AC's
// uint32 (AC ChatHelper.h:38) and every call site below passes a value that fits it - a uint32 trade discount,
// and a negated int32 diff - so the cast is safe and the module now has ONE money rendering (AC's).
// 
// 5.4.8: see the AC InventoryAction::TellItems() note in the file header.
void TellInventoryList(PlayerbotAI* botAI, std::map<uint32, uint32> const& items, std::map<uint32, bool> const& soulbound)
{
    std::ostringstream out;
    for (auto const& kv : items)
    {
        ItemTemplate const* proto = sObjectMgr->GetItemTemplate(kv.first);
        if (!proto)
            continue;

        out << proto->Name1 << " (" << kv.first << ") x" << kv.second;

        auto sb = soulbound.find(kv.first);
        if (sb != soulbound.end() && sb->second)
            out << " [soulbound]";

        out << "; ";
    }

    if (!out.str().empty())
        botAI->TellMaster(out);
}
}

bool TradeStatusAction::Execute(Event event)
{
    if (!bot->GetSession())
        return false;

    Player* trader = bot->GetTrader();
    if (!trader)
        return false;

    // 5.4.8: see the IsSelfBot()/traderIsGameClientPlayer note in the file header.
    bool const traderIsGameClientPlayer = !GET_PLAYERBOT_AI(trader);
    Player* master = GetMaster();

    // Bots refuse to trade with a person (whether active or selfbotting) who is neither their
    // master nor a group member. Bot traders are handled further down.
    if (trader != master && traderIsGameClientPlayer &&
        (!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())))
    {
        bot->Whisper("I'm kind of busy now", LANG_UNIVERSAL, trader);
        CancelTrade();
        return false;
    }

    // 5.4.8: see the IsAddclassBot() note in the file header.
    if (sPlayerbotAIConfig->enableRandomBotTrading == 0 && sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        bot->Whisper("Trading is disabled", LANG_UNIVERSAL, trader);
        CancelTrade();
        return false;
    }

    // Bots also refuse their own master when ungrouped and security withholds full access.
    if ((!bot->GetGroup() || !bot->GetGroup()->IsMember(trader->GetGUID())) &&
        (trader != master || !botAI->GetSecurity()->CheckLevelFor(PLAYERBOT_SECURITY_ALLOW_ALL, true, master)) &&
        traderIsGameClientPlayer)
    {
        CancelTrade();
        return false;
    }

    // 5.4.8: SMSG_TRADE_STATUS bit-packs its status field (see the file header) instead of AC's plain
    // `p >> status`.
    WorldPacket p(event.getPacket());
    p.rpos(0);
    p.ReadBit(); // unk bit (TradeHandler.cpp:40)
    uint32 status = p.ReadBits(5);

    if (status == TRADE_STATUS_ACCEPTED)
    {
        // 5.4.8: HandleAcceptTradeOpcode() never reads recvPacket (TradeHandler.cpp:333), so an empty
        // packet is enough - no need to build AC's dummy `uint32 status` payload.
        WorldPacket accept;

        uint32 discount = GetTradeDiscount(bot, trader);
        if (CheckTrade())
        {
            bot->GetSession()->HandleAcceptTradeOpcode(accept);
            if (bot->GetTradeData())
            {
                SetTradeDiscount(bot, trader, discount);
                return false;
            }

            // 5.4.8: no craft-order system on this fork (see the file header) - AC's
            // CraftData/GuildTaskMgr bookkeeping over givenItemIds/takenItemIds is dropped.
            return true;
        }
    }
    else if (status == TRADE_STATUS_PROPOSED)
    {
        // 5.4.8: see the file header - this core sends PROPOSED (not INITIATED) to the trade target
        // when someone else calls CMSG_INITIATE_TRADE against the bot. Accept the invite immediately
        // (same as a real client's "do you want to trade with X" confirm); HandleBeginTradeOpcode()
        // broadcasts TRADE_STATUS_INITIATED to both sides, which is what BeginTrade() below reacts to.
        WorldPacket accept;
        bot->GetSession()->HandleBeginTradeOpcode(accept);
        return true;
    }
    else if (status == TRADE_STATUS_INITIATED)
    {
        if (!BotHasInArcAC(bot, CAST_ANGLE_IN_FRONT, trader, sPlayerbotAIConfig->sightDistance))
            bot->SetFacingToObject(trader);

        BeginTrade();
        return true;
    }
    return false;
}

void TradeStatusAction::BeginTrade()
{
    Player* trader = bot->GetTrader();
    // 5.4.8: see the IsSelfBot() note in the file header - AC's selfbot exception is dropped, so any
    // bot-controlled trader skips the inventory dump.
    if (!trader || GET_PLAYERBOT_AI(trader))
        return;

    // 5.4.8: no HandleBeginTradeOpcode() call here - the PROPOSED branch in Execute() already made
    // it once (see the file header); calling it again would just re-broadcast TRADE_STATUS_INITIATED
    // to both sides for no reason.
    ListItemsVisitor visitor;
    IterateItems(&visitor);

    botAI->TellMaster("=== Inventory ===");
    TellInventoryList(botAI, visitor.items, visitor.soulbound);

    if (sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        uint32 discount = GetTradeDiscount(bot, botAI->GetMaster());
        if (discount)
        {
            std::ostringstream out;
            out << "Discount up to: " << ChatHelper::formatMoney(discount);
            botAI->TellMaster(out);
        }
    }
}

void TradeStatusAction::CancelTrade()
{
    // 5.4.8: HandleCancelTradeOpcode() never reads recvPacket (TradeHandler.cpp:629).
    WorldPacket p;
    bot->GetSession()->HandleCancelTradeOpcode(p);
}

bool TradeStatusAction::CheckTrade()
{
    Player* trader = bot->GetTrader();
    if (!bot->GetTradeData() || !trader || !trader->GetTradeData())
        return false;

    // 5.4.8: HasGameClientMaster() -> HasRealPlayerMaster(); IsSelfBot() dropped (file header).
    if (!botAI->HasRealPlayerMaster() && GET_PLAYERBOT_AI(bot->GetTrader()))
    {
        bool isGettingItem = false;
        for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
        {
            Item* item = trader->GetTradeData()->GetItem((TradeSlots)slot);
            if (item)
            {
                isGettingItem = true;
                break;
            }
        }

        if (isGettingItem)
        {
            // 5.4.8: chat->FormatWorldobject() has no equivalent here (file header) - the trader's
            // plain name replaces the clickable player link.
            if (bot->GetGroup() && bot->GetGroup()->IsMember(bot->GetTrader()->GetGUID()) &&
                botAI->HasRealPlayerMaster())
            {
                botAI->TellMasterNoFacing("Thank you " + bot->GetTrader()->GetName());
            }
            else
            {
                bot->Say("Thank you " + bot->GetTrader()->GetName(),
                         (bot->GetTeamId() == TEAM_ALLIANCE ? LANG_COMMON : LANG_ORCISH));
            }
        }
        return isGettingItem;
    }

    uint32 accountId = bot->GetSession()->GetAccountId();
    if (!sPlayerbotAIConfig->IsInRandomAccountList(accountId))
    {
        // A bot on a real player's own account always accepts. AC additionally plays a flavor sound
        // here (TEXT_EMOTE_SIGH/TEXT_EMOTE_THANK depending on who comes out ahead); botAI->PlaySound()
        // does not exist on this fork (file header), so that cosmetic step - and the cost computation
        // that only fed it - is dropped.
        return true;
    }

    int64 botItemsMoney = CalculateCost(bot, true);
    int64 botMoney = int64(bot->GetTradeData()->GetMoney()) + botItemsMoney;
    int64 playerItemsMoney = CalculateCost(trader, false);
    int64 playerMoney = int64(trader->GetTradeData()->GetMoney()) + playerItemsMoney;

    if (botItemsMoney > 0 && sPlayerbotAIConfig->enableRandomBotTrading == 2 && sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        bot->Whisper("Selling is disabled.", LANG_UNIVERSAL, trader);
        return false;
    }
    if (playerItemsMoney && sPlayerbotAIConfig->enableRandomBotTrading == 3 && sRandomPlayerbotMgr->IsRandomBot(bot))
    {
        bot->Whisper("Buying is disabled.", LANG_UNIVERSAL, trader);
        return false;
    }

    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = bot->GetTradeData()->GetItem((TradeSlots)slot);
        if (item && !item->GetTemplate()->SellPrice && !item->GetTemplate()->IsConjuredConsumable())
        {
            botAI->TellMaster(item->GetTemplate()->Name1 + " - This is not for sale");
            return false;
        }

        item = trader->GetTradeData()->GetItem((TradeSlots)slot);
        if (item)
        {
            std::string itemIdStr = std::to_string(item->GetTemplate()->ItemId);
            ItemUsage usage = AI_VALUE2(ItemUsage, "item usage", itemIdStr);
            if ((botMoney && !item->GetTemplate()->BuyPrice) || usage == ITEM_USAGE_NONE)
            {
                botAI->TellMaster(item->GetTemplate()->Name1 + " - I don't need this");
                return false;
            }
        }
    }

    if (!botMoney && !playerMoney)
        return true;

    if (!botItemsMoney && !playerItemsMoney)
    {
        botAI->TellError("There are no items to trade");
        return false;
    }

    int64 discount = int64(GetTradeDiscount(bot, trader));
    int64 delta = playerMoney - botMoney;
    int64 moneyDelta = int64(trader->GetTradeData()->GetMoney()) - int64(bot->GetTradeData()->GetMoney());
    bool success = false;
    if (delta < 0)
    {
        if (delta + discount >= 0)
        {
            if (moneyDelta < 0)
            {
                botAI->TellError("You can use discount to buy items only");
                return false;
            }
            success = true;
        }
    }
    else
        success = true;

    if (success)
    {
        AddTradeDiscount(bot, trader, int32(delta));
        switch (urand(0, 4))
        {
            case 0:
                botAI->TellMaster("A pleasure doing business with you");
                break;
            case 1:
                botAI->TellMaster("Fair trade");
                break;
            case 2:
                botAI->TellMaster("Thanks");
                break;
            case 3:
                botAI->TellMaster("Off with you");
                break;
        }

        return true;
    }

    botAI->TellMaster("I want " + ChatHelper::formatMoney(uint32(-(delta + discount))) + " for this");
    return false;
}

int32 TradeStatusAction::CalculateCost(Player* player, bool sell)
{
    TradeData* data = player->GetTradeData();
    if (!data)
        return 0;

    uint32 sum = 0;
    for (uint32 slot = 0; slot < TRADE_SLOT_TRADED_COUNT; ++slot)
    {
        Item* item = data->GetItem((TradeSlots)slot);
        if (!item)
            continue;

        ItemTemplate const* proto = item->GetTemplate();
        if (!proto)
            continue;

        if (proto->Quality < ITEM_QUALITY_NORMAL)
            return 0;

        // 5.4.8: no craft system on this fork (file header) - AC's craft-fee/reserved-reagent
        // carve-outs are dropped here.
        if (sell)
            sum += uint32(item->GetCount() * proto->SellPrice * GetSellMultiplier(bot));
        else
            sum += uint32(item->GetCount() * proto->BuyPrice * GetBuyMultiplier(bot));
    }

    return sum;
}
