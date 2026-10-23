/*
 * Copyright (C) 2016+ AzerothCore <www.azerothcore.org>, released under GNU GPL v2 license you may redistribute it
 * and/or modify it under version 2 of the License, or (at your option), any later version.
 *
 * Ported from AC mod-playerbots Ai/Base/Actions/SellAction.cpp for issue #258 (NPC interaction layer).
 * Gives bots a vendor sell path so they can empty their bags.
 *
 * 5.4.8 / this-fork adaptations vs AC:
 *
 *  - chat->FormatItem(proto) -> ChatHelper::FormatItem(proto): this fork's ChatHelper formatters are
 *    static (AI/ChatHelper.h:48, landed by #257/#322).
 *
 *  - CMSG_SELL_ITEM: AC pushes a synthetic packet through WorldPackets::Item::SellItem. This core
 *    has no such packet class and its WorldSession::HandleSellItemOpcode (Handlers/ItemHandler.cpp:
 *    411-452) reads a *bit-packed* guid pair (mask 16 bits, then 16 ReadByteSeq bytes) - the same
 *    layout LootAction.cpp:157-180 / ReadyCheckAction.cpp:58-79 already hand-build here. The
 *    packet is therefore written field-by-field in exactly the core reader's order, using
 *    bot->GetSession()->HandleSellItemOpcode() (this fork's established idiom for faking an inbound
 *    client packet, e.g. LootAction.cpp:398-399, LootRollAction.cpp:219-221).
 *
 *  - ChatHelper::parseItemQuality(text) (#331, replacing the file-local ParseItemQuality() this file
 *    carried): the quality half of AC's ChatHelper landed in AI/ChatHelper.h in #331, so
 *    "sell gray"/"sell green"/"sell purple" still resolve through AC's itemQualities table
 *    (AC ChatHelper.cpp:42-59, :556-562) - now the single copy, with byte-identical behaviour.
 *
 *  - InventoryAction::parseItems(text, ITERATE_ITEMS_IN_BAGS) for the "<item link>" form of the
 *    command: this fork's InventoryAction::Find() (InventoryAction.cpp:52-127) is a cut-down
 *    replacement with the same semantics for the qualifiers it knows (numeric item id / bare item
 *    name / food / drink / potions) and it iterates bags by default, which is what AC's
 *    ITERATE_ITEMS_IN_BAGS asked for here.
 *
 *  - botAI->HasCheat(BotCheatMask::gold) around the sell (AC :174-177): the whole bot-cheat-mask
 *    feature is dormant on this fork (PlayerbotAI.h:319 has "cheatMask" commented out; there is no
 *    HasCheat()/BotCheatMask), exactly as recorded in RepairAllAction.cpp:10-15. The gold is simply
 *    not refunded - a sale costs the player nothing here, so this cannot alter behaviour. Flagged
 *    as an AC feature that cannot be ported without touching PlayerbotAI.h.
 *
 *  - BroadcastHelper: not ported to this fork (see #322). AC's SellAction has no BroadcastHelper
 *    call; nothing omitted here.
 */

#include "SellAction.h"

#include "ChatHelper.h"
#include "Event.h"
#include "ItemVisitors.h"
#include "ObjectGuid.h"
#include "Playerbots.h"
#include "WorldPacket.h"
#include "WorldSession.h"

#include <algorithm>
#include <sstream>

// AC SellAction.cpp:15-28
class SellItemsVisitor : public IterateItemsVisitor
{
public:
    SellItemsVisitor(SellAction* action) : IterateItemsVisitor(), action(action) {}

    bool Visit(Item* item) override
    {
        action->Sell(item);
        return true;
    }

private:
    SellAction* action;
};

// AC SellAction.cpp:30-72
class SellQualityItemsVisitor : public SellItemsVisitor
{
public:
    SellQualityItemsVisitor(SellAction* action, uint32 maxQuality, bool allClasses)
        : SellItemsVisitor(action), maxQuality(maxQuality), allClasses(allClasses)
    {
    }

    bool Visit(Item* item) override
    {
        ItemTemplate const* proto = item->GetTemplate();
        if (proto->Quality > maxQuality)
            return true;

        if (IsProfessionTool(proto))
            return true;

        if (!allClasses && proto->Quality > ITEM_QUALITY_POOR && !IsEquipment(proto))
            return true;

        return SellItemsVisitor::Visit(item);
    }

private:
    static bool IsEquipment(ItemTemplate const* proto)
    {
        return proto->Class == ITEM_CLASS_ARMOR || proto->Class == ITEM_CLASS_WEAPON;
    }

    static bool IsProfessionTool(ItemTemplate const* proto)
    {
        if (proto->Class != ITEM_CLASS_WEAPON)
            return false;

        // 5.4.8: this core spells the miscellaneous-weapon subclass ITEM_SUBCLASS_WEAPON_MISCELLANEOUS
        // (ItemPrototype.h:391); AC's ITEM_SUBCLASS_WEAPON_MISC is the same value (14).
        if (proto->SubClass == ITEM_SUBCLASS_WEAPON_MISCELLANEOUS || proto->SubClass == ITEM_SUBCLASS_WEAPON_FISHING_POLE)
            return true;

        return proto->TotemCategory != 0;
    }

    uint32 maxQuality;
    bool allClasses;
};

// AC SellAction.cpp:74-89
class SellVendorItemsVisitor : public SellItemsVisitor
{
public:
    SellVendorItemsVisitor(SellAction* action, AiObjectContext* con) : SellItemsVisitor(action) { context = con; }

    AiObjectContext* context;

    bool Visit(Item* item) override
    {
        ItemUsage usage = context->GetValue<ItemUsage>("item usage", item->GetEntry())->Get();
        if (usage != ITEM_USAGE_VENDOR && usage != ITEM_USAGE_AH)
            return true;

        return SellItemsVisitor::Visit(item);
    }
};

bool SellAction::Execute(Event event)
{
    std::string const text = event.getParam();
    if (text == "gray" || text == "*")
    {
        SellQualityItemsVisitor visitor(this, ITEM_QUALITY_POOR, false);
        IterateItems(&visitor);
        return true;
    }

    if (text == "vendor")
    {
        SellVendorItemsVisitor visitor(this, context);
        IterateItems(&visitor);
        return true;
    }

    std::string quality = text;
    bool allClasses = false;

    size_t const split = quality.rfind(' ');
    if (split != std::string::npos && quality.substr(split + 1) == "all")
    {
        quality.erase(split);
        allClasses = true;
    }

    uint32 const maxQuality = ChatHelper::parseItemQuality(quality);
    if (maxQuality != MAX_ITEM_QUALITY)
    {
        SellQualityItemsVisitor visitor(this, maxQuality, allClasses);
        IterateItems(&visitor);
        return true;
    }

    if (text != "")
    {
        // #883: AC's parseItems(text, ITERATE_ITEMS_IN_BAGS) (item-link branch first, AC InventoryAction.cpp:222-239)
        // is InventoryAction::Find() now; the #258/Q2 copy of the link branch that lived here is gone.
        std::vector<Item*> const items = Find(text);

        for (Item* item : items)
        {
            Sell(item);
        }
        return true;
    }

    botAI->TellError("Usage: s gray/*/vendor/<quality> [all]/[item link]");
    return false;
}

// AC SellAction.cpp:140-148
void SellAction::Sell(FindItemVisitor* visitor)
{
    IterateItems(visitor);
    std::vector<Item*> items = visitor->GetResult();
    for (Item* item : items)
    {
        Sell(item);
    }
}

// AC SellAction.cpp:150-185
void SellAction::Sell(Item* item)
{
    std::ostringstream out;

    GuidVector vendors = botAI->GetAiObjectContext()->GetValue<GuidVector>("nearest npcs")->Get();

    for (ObjectGuid const vendorguid : vendors)
    {
        Creature* pCreature = bot->GetNPCIfCanInteractWith(vendorguid, UNIT_NPC_FLAG_VENDOR);
        if (!pCreature)
            continue;

        ObjectGuid itemguid = item->GetGUID();
        uint32 count = item->GetCount();

        // 5.4.8 CMSG_SELL_ITEM layout, read by WorldSession::HandleSellItemOpcode
        // (core:Handlers/ItemHandler.cpp:411-452): uint32 count first, then the 16 guid-field mask
        // bits, then 16 bytes in ReadByteSeq order. Written here in exactly that order.
        WorldPacket p(CMSG_SELL_ITEM, 4 + 8);
        p << uint32(count);

        p.WriteBit(itemguid[4]);
        p.WriteBit(itemguid[3]);
        p.WriteBit(itemguid[7]);
        p.WriteBit(vendorguid[6]);
        p.WriteBit(vendorguid[5]);
        p.WriteBit(vendorguid[1]);
        p.WriteBit(itemguid[5]);
        p.WriteBit(itemguid[2]);
        p.WriteBit(itemguid[1]);
        p.WriteBit(vendorguid[2]);
        p.WriteBit(itemguid[6]);
        p.WriteBit(vendorguid[4]);
        p.WriteBit(vendorguid[0]);  // #258 lane Q2: was itemguid[0] - the core reads vendorGuid[0] here (ItemHandler.cpp HandleSellItemOpcode)
        p.WriteBit(vendorguid[7]);
        p.WriteBit(vendorguid[3]);
        p.WriteBit(itemguid[0]);
        p.FlushBits();

        p.WriteByteSeq(vendorguid[6]);
        p.WriteByteSeq(vendorguid[3]);
        p.WriteByteSeq(vendorguid[1]);
        p.WriteByteSeq(itemguid[1]);
        p.WriteByteSeq(vendorguid[2]);
        p.WriteByteSeq(itemguid[7]);
        p.WriteByteSeq(itemguid[5]);
        p.WriteByteSeq(vendorguid[7]);
        p.WriteByteSeq(itemguid[2]);
        p.WriteByteSeq(vendorguid[0]);
        p.WriteByteSeq(vendorguid[5]);
        p.WriteByteSeq(itemguid[3]);
        p.WriteByteSeq(itemguid[6]);
        p.WriteByteSeq(vendorguid[4]);
        p.WriteByteSeq(itemguid[4]);
        p.WriteByteSeq(itemguid[0]);

        bot->GetSession()->HandleSellItemOpcode(p);

        // #258: botAI->HasCheat(BotCheatMask::gold) { bot->SetMoney(botMoney); } (AC :174-177) is
        // omitted - BotCheatMask/HasCheat do not exist on this fork (see the file header).

        out << "Selling " << ChatHelper::FormatItem(item->GetTemplate());
        botAI->TellMaster(out.str());

        bot->PlayDistanceSound(120);
        break;
    }
}